# Spec：Linux/FreeBSD/macOS 编译与打包脚本复查修复

日期：2026-10-27
背景：Windows 手工测试曾发现大量脚本缺陷；本次对 Unix 三平台做同类复查，发现 5 项问题（见上表）。本规格定义修复方案与验收标准。

## 修复 1：pack.py 兼容无 lib 前缀的 CEF wrapper 产物名（高）

**根因**：`duilib/third_party/libcef/*/libcef_dll/CMakeLists.txt` 对 wrapper 目标
`set_target_properties(... PREFIX "")`，Unix 产物为 `cef_dll_wrapper.a`；
而 pack.py 白名单 `CEF_WRAPPER_BY_VERSION = {'latest': 'libcef_dll_wrapper', '109': 'libcef_dll_wrapper_109'}`
与 `_lib_base_name` 归一逻辑无法把 `cef_dll_wrapper` 映射到白名单名。

**方案**：修改 `build/pack.py` 的 `_lib_base_name`：当剥离 `lib` 前缀后的 stem
等于 `cef_dll_wrapper` 或 `cef_dll_wrapper_109` 时，统一归一为带 `lib` 前缀的
白名单名（`libcef_dll_wrapper` / `libcef_dll_wrapper_109`）。包内仍按原文件名复制，
仅逻辑名归一；下游 `packaging/cmake/nim_duilibConfig.cmake` 的 `_nim_duilib_lib_path`
候选已含 `${BASE_NAME}.a`，无需改动。

**验收**：test_pack.py 新增用例——构造 Unix `lib/<Config>` 目录含
`cef_dll_wrapper.a`，Linux + with_cef 场景下 `collect_libs` 通过、库清单含
`libcef_dll_wrapper`。

## 修复 2：Unix SDL stamp 去掉配置分量（中）

**根因**：`_duilib_sdl_stamp_value` 为
`ref|arch|crt|platform|config`；Unix 的 SDL3 固定 `-DCMAKE_BUILD_TYPE=Release` 单次
构建到 `${DUILIB_ARCH}-release`，与 duilib 的 Debug/Release 无关，config 分量导致
切换配置必重建。

**方案**：`cmake/duilib_deps_run.cmake` 中 stamp 的 `${DUILIB_CONFIG}` 分量仅
Windows 保留；Unix 省略该分量（与构建目录命名 `${DUILIB_ARCH}-release` 一致）。
代价：已有 Unix 构建树 stamp 不匹配，首次运行重建一次 SDL3，可接受。

**验收**：代码审查确认 stamp 组成；无法本机实测，记入 build/test-matrix.md 已知缺口。

## 修复 3：Skia 静态库绝对路径解析扩展到 Unix（中）

**根因**：`duilib_common.cmake` 中 `_duilib_resolve_static_libs` 仅在
`if(DUILIB_OS_WINDOWS AND NOT ... MATCHES "\$<")` 下生效；Unix 以裸名链接 Skia
静态库，Ninja 不把裸名记为依赖，deps 重编 Skia 后示例不重链接。注释所称
"避免绝对路径写进 dylib install_name/rpath"仅适用于共享库（SDL3），对静态 .a 不成立。

**方案**：移除 `DUILIB_OS_WINDOWS` 限制，所有平台对 Skia 静态库解析绝对路径；
SDL3 共享库保持裸名 `SDL3` 不变。同步更正注释：静态库用绝对路径以便 Ninja 跟踪
重链接；共享库保留裸名以避免污染 install_name/rpath。
前置确认：`_duilib_resolve_static_libs` 对库目录中不存在的名字保留原名
（系统库名不受影响），执行时核对实现后修改。

**验收**：代码审查 + 记入 test-matrix.md 已知缺口（需真机验证：touch Skia .a 后
ninja 示例是否重链接）。

## 修复 4：_duilib_cef_tracked_dir 按平台返回跟踪目录（低）

**根因**：`duilib_cef.cmake` 的 `_duilib_cef_tracked_dir` 只按 CEF 版本返回
`libcef_win${_suffix}`，用于迁移采纳路径与解压后版本比对；Linux/macOS 会对错跟踪头。
当前三平台跟踪头同为 142.0.10，问题潜伏。

**方案**：按 `DUILIB_DEPS_PLATFORM` 返回 `libcef_linux` / `libcef_macos` /
`libcef_win[_109]`（FreeBSD 无 CEF 不涉及）。

**验收**：代码审查；记入 test-matrix.md 已知缺口。

## 修复 5：更正 duilib_compiler_flags.cmake 头注释（轻微）

**方案**：注释改为"macOS 的 duilib 库与示例程序（含 CEF 分支）共用这一份定义"，
删除"Linux/FreeBSD 的示例程序"字样。

## 单元测试

- `build/test_pack.py`：新增 Linux wrapper 名归一用例（修复 1）。
- 修复 2/3/4 为 CMake 逻辑，pytest 无法覆盖，记入 `build/test-matrix.md` 已知缺口。

## 文档同步

- `CHANGELOG.md` / `CHANGELOG.en.md`：修复区追加 4 条（#1~#4）。
- `build/build-reference.md` / `.en.md`：如涉及 wrapper 产物名或 SDL stamp 行为的
  描述，同步更正；不涉及则不强行添加。

## 验证

- `python build/build.py test` 全部通过（158+ 项）。
- 代码由用户手动提交，助手不执行 git commit。
- 本机为 Windows，三平台无法实测；需真机验证项写入 test-matrix.md。
