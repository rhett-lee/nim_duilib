# Unix 平台共享库（.so/.dylib）可行性分析

> 状态：已评估，**暂不实施**（2026-10-28 决策）
> 范围：Linux / macOS / FreeBSD 下把 duilib 由静态库改为共享库（对应 Windows 的 `--dll` 模式）
> 依据：仓库源码、构建脚本与打包脚本的静态审查；**未执行构建，未修改代码**
> 本文件位于 `docs/misc/`，不随发布包分发。

## 一、背景

Windows 已支持 `build.py --dll`：duilib 编译为 `bin/duilib*.dll`，下游只链接导入库；
第三方库按需链接（SDL 在 Windows 上默认关闭），CEF 通过 `/DELAYLOAD` 延迟加载。
Linux / macOS / FreeBSD 目前仍编译为静态库（`libduilib*.a`），`--dll` 在非 Windows
平台被明确拒绝。

需要回答的问题是：Unix 平台能否在 `--dll` 模式下同样把 `.a` 改成 `.so` / `.dylib`？
本文件记录评估结论、代码证据、主要障碍和建议的实施路线，供以后需要时参考。

## 二、结论

**技术上可行，但不等同于 Windows 的 `--dll`。**

- CMake 的 `SHARED` 目标、ELF `.so` 与 Mach-O `.dylib` 机制都支持；
- 但不能只删除 `DUILIB_DLL` 的 Windows/MSVC 限制，当前 Unix 构建是按静态库设计的，
  直接改成共享库会得到一个“几乎不导出符号”的空壳 `.so`；
- 真正的改造量在导出可见性、PIC、CEF 运行期语义、rpath/install_name 以及
  打包/下游接入，而不是 `add_library(... SHARED)` 这一行。

**决策：2026-10-28 讨论后暂不实施，Unix 继续使用静态库；Windows `--dll` 保持不变。**

## 三、当前代码现状（证据）

以下行号为 2026-10-28 时的快照；代码变动后以函数名/章节为准。

| 位置 | 现状 |
| :--- | :--- |
| `cmake/duilib_root_options.cmake:291-299` | `DUILIB_DLL` 只允许 `Windows + MSVC`，否则 `FATAL_ERROR`；并强制 `DUILIB_MD=ON` |
| `duilib/CMakeLists.txt:280-291` | `if(FreeBSD)` 分支排在 `elseif(DUILIB_DLL)` 之前；即使放开选项，FreeBSD 仍会走静态库分支 |
| `duilib/CMakeLists.txt:323-373` | DLL 分支的输出名、`/DELAYLOAD:libcef.dll`、`WinInet/Comctl32/...` 全部是 Windows 专用 |
| `cmake/duilib_compiler_flags.cmake:16,28` | Unix 的 duilib 目标统一加 `-fvisibility=hidden` 与 `-fvisibility-inlines-hidden` |
| `duilib/duilib_config_linux.h:8`、`duilib_config_macos.h:10`、`duilib_config_freebsd.h:8` | `DUILIB_API` 为空，没有任何 `visibility("default")` 标注 |
| `cmake/duilib_bin_linux.cmake:26-32`、`duilib_bin_macos.cmake:43-51`、`duilib_bin_freebsd.cmake:15-22` | X11、字体库、macOS 框架、`pthread/dl` 只链接到示例程序，而不是 duilib 库 |
| `build/build.py` 的 `--dll` 分支 | 仅 Windows MSVC 可用，且隐含 `--md`；Unix 上 `--md` 没有语义 |
| `build/pack.py:313` | `.so` / `.dylib` 被列为排除扩展名 |
| `build/pack.py:437-444`、`:1714` | “DLL 模式”只从 `bin/duilib*.dll` 判断，Unix `.so` 不会被识别 |
| `packaging/cmake/nim_duilibConfig.cmake` | 只查找 `.a` / `.dll.a`，并默认要求 5 个基础静态库同时存在 |

## 四、必须解决的硬问题

### 1. 符号导出（最大障碍）

当前 Unix 编译选项是 `-fvisibility=hidden`，而 `DUILIB_API` 在三个 Unix 平台配置中都是空宏。
这意味着：直接 `add_library(duilib SHARED ...)` 生成的 `.so/.dylib` **几乎不会导出任何
duilib 公开符号**，下游链接会出现大面积 undefined symbol。

必须做：

- 在 Unix 平台配置里把 `DUILIB_API` 定义为 `__attribute__((visibility("default")))`，
  可只在共享构建宏下启用，避免影响静态产物；
- 审计公开 API：`duilib` 头文件中约 331 处 `DUILIB_API`（其中 325 处用于
  `class/struct/enum`，6 处是平台宏定义），**没有自由函数/全局变量标注**；
  其中 2 处标在 `enum` 上，`visibility` 属性对枚举是否生效需编译实测；
  公开的非类符号（自由函数、全局变量、显式模板实例化）需要另行审计并补标注；
- 用 `nm -D --defined-only libduilib.so` 与静态库的 `nm --defined-only libduilib.a`
  对比导出集合，确认无缺失；
- 特别注意导出类的基类、异常类、`typeinfo/vtable`：`-fvisibility-inlines-hidden` +
  隐藏基类可能影响跨库 `dynamic_cast` 与异常捕获；
- 示例在共享模式下应只链接 duilib（直接调用 SDL/CEF 的示例再显式链接），
  否则示例会继续链接 Skia/第三方静态库，**掩盖导出缺失**，测试结果不可信。

### 2. PIC（机械但必须做）

把静态库链接进 `.so` 要求所有静态库都是 PIC：

- **Skia**：非 Windows 构建已带 `-fPIC`（补丁归档的 `gn/skia/BUILD.gn` 非 Windows 分支），
  且带 `-fvisibility=hidden`，无 PIC 问题；
- **CEF wrapper**：`libcef_linux/cmake/cef_variables.cmake` 已加 `-fPIC`；
- **仓库内四个静态库**：`duilib-zlib`、`duilib-png`、`duilib-cximage`、`duilib-webp`
  的 CMakeLists 没有设置 `POSITION_INDEPENDENT_CODE`，默认非 PIC，
  链接进 `.so` 会报 `relocation ... can not be used when making a shared object`。

建议在 DLL 模式下打开 `CMAKE_POSITION_INDEPENDENT_CODE=ON`（或对相关 target 单独设置），
静态构建保持不变。若 Unix 还链接系统静态 libjpeg 等，也要一并检查 PIC。

### 3. CMake 结构需要重构

- 选项门控改为平台中立（至少允许 Linux/macOS/FreeBSD，Windows 语义保持不变）；
- `duilib/CMakeLists.txt` 的分支顺序要调整：`if(DUILIB_DLL)` 必须先于 FreeBSD 分支，
  Windows 专用属性/链接选项要包进 `if(DUILIB_OS_WINDOWS)`；
- Unix 共享库要自己链接 `DUILIB_LIBS`（zlib/png/cximage/webp）、`DUILIB_SKIA_LIBS`、
  `DUILIB_SDL_LIBS`、字体库、`X11`、`pthread/dl`；macOS 还要链接
  `Accelerate/CoreFoundation/CoreText/CoreGraphics/AppKit/Foundation/Metal/Cocoa`；
- Unix 上不应因 `--dll` 强制 `DUILIB_MD`（`--md` 只是 MSVC CRT 概念；
  `DUILIB_CRT_ID` 在 Unix 是 `system`）。

### 4. 符号污染 / ODR

Windows DLL 默认只导出 `__declspec(dllexport)` 的符号，第三方静态库符号天然私有；
ELF 默认导出所有全局符号。因此：

- zlib/png/webp/cximage、甚至 CEF wrapper 的符号会从 `libduilib.so` 导出，
  可能与宿主程序自带的同名库发生符号抢占；
- 建议 Linux/FreeBSD 加 `-Wl,--exclude-libs,ALL` 隐藏静态库符号，
  或给这些静态目标设置 `C_VISIBILITY_PRESET hidden` / `CXX_VISIBILITY_PRESET hidden`；
- macOS 没有 `--exclude-libs`，需要用 `-fvisibility=hidden` 编译第三方库，
  或 `-Wl,-unexported_symbols_list`。

## 五、运行期依赖差异（比“编出 .so”更难）

### 1. SDL3

Unix 上 SDL 始终开启，`libduilib.so` 会产生 `NEEDED libSDL3.so`。包内本来就随 SDL3，
配合 rpath 可行；但意味着所有使用共享版的应用都必须带 SDL3 运行库，
即使应用不使用 SDL。

### 2. CEF（最关键的架构差异）

Windows DLL 模式使用 `/DELAYLOAD:libcef.dll`（`duilib/CMakeLists.txt:363-367`），
因此不使用 CEF 的示例在 `bin/libcef_win` 不存在时也能启动。

Unix 没有等价的 delay-load：

- Linux：`libduilib.so` 若包含 CEF wrapper，会记录 `NEEDED libcef.so`；
  **即使应用完全不用 CEF，启动时也必须能找到 libcef.so**；
- macOS：CEF 是 `Chromium Embedded Framework.framework`，把 CEF 代码放进
  `libduilib.dylib` 会强迫每个 app bundle 都携带数百 MB 的 CEF Framework，通常不可接受。

可选方案：

1. **共享版先只支持 CEF 关闭构建**（最务实）；CEF 示例继续用静态库或单独构建；
2. 把 CEF 拆成独立共享模块（`duilib_cef.so` / `duilib_cef.dylib`），核心库不引用 CEF；
   但公开头文件与类层次要重新划分，改造量较大；
3. 接受依赖、共享包始终附带 CEF 运行库（Linux 尚可，macOS 代价高）。

注：当前 Linux 已做到“仅 CEF 示例链接 CEF”，非 CEF 示例不依赖 `libcef.so`
（见 `Packaging-Follow-Up-Items.md` 的记录）；共享库会把这条隔离重新合并。

### 3. rpath / SONAME / install_name

- Linux/FreeBSD：`libduilib.so` 需设置 `SONAME`（建议 `libduilib.so.<major>`）、
  `RPATH/RUNPATH`（`$ORIGIN`、`$ORIGIN/../bin/libcef_linux` 等）；
- macOS：`install_name` 应为 `@rpath/libduilib.dylib`，示例/Bundle 需要
  `@loader_path` 或 `@executable_path/../Frameworks`；Bundle 时要把 dylib 复制进
  `Contents/Frameworks` 并重新签名；
- 现有全局 rpath 是按“可执行文件在 `bin/`”设计的，`libduilib.so` 放在 `lib/` 时
  需要单独设置 target 级 rpath，否则相对路径会错位。

### 4. ABI

Windows MSVC 的 C++ ABI 相对稳定；Unix 共享 C++ 库对编译器/标准库更敏感：

- Linux：`libstdc++` 版本、`_GLIBCXX_USE_CXX11_ABI` 必须一致；
- macOS：必须同为 libc++；
- 下游若用不同编译器（GCC vs Clang）链接同一个 `.so`，可能二进制不兼容。

共享包只能保证“同工具链/同标准库”使用，需要写入文档。

## 六、打包与下游接入的改造量

目前整条链路都按静态库假设处理：

- `build/pack.py:313` 明确把 `.so` / `.dylib` 列为排除扩展名；Unix 的库扫描只接受 `.a`；
- `pack.py:437-444`、`:1714` 的 DLL 模式判断只看 `bin/duilib*.dll`，Unix `.so` 不会被识别；
- `packaging/cmake/nim_duilibConfig.cmake` 的库查找只支持 `.dll.a` / `.a`，不支持 `.so/.dylib`；
- 同文件还要求 `duilib / duilib-cximage / duilib-png / duilib-webp / duilib-zlib`
  五个库同时存在；共享模式下只需 `libduilib.so`，组件静态库不应再作为包内容要求；
- Unix 的导入目标无条件追加 X11/字体/框架，共享模式下应改为由 `libduilib.so` 自己承担，
  避免下游被迫 `find_package(X11/Freetype/Fontconfig)`；
- `nim_duilib_copy_runtime` 目前主要处理 Windows DLL 与 macOS Framework，
  Unix 需要新增复制 `libduilib.so` + SDL3 并设置 rpath；
- `sdk_package_test.py`、`test-matrix.md`、`docs/Release*.md` 都需要新增 Unix 共享模式
  用例与布局说明；
- 包名/元数据建议沿用 `-dll`，或统一改为 `-shared`（需要定一个语义）。

建议的共享包布局（若未来实施）：

- `lib/libduilib.so`（链接 + 运行；建议带 SONAME 与版本软链）；
- `lib/libSDL3.so*`、`lib/libcef.so`（Linux）；
- 通过 CMake config 提供导入目标与 rpath/运行时复制；
- macOS Bundle：`Contents/Frameworks/libduilib.dylib` + CEF Framework。

## 七、平台差异与工作量

| 平台 | 动态库形式 | 主要额外工作 | 风险 |
| :--- | :--- | :--- | :--- |
| Linux | `.so` | 导出、PIC、rpath、CEF 依赖、打包/CMake config | 中 |
| FreeBSD | `.so` | 同 Linux，无 CEF，范围较小 | 低-中 |
| macOS | `.dylib` | `install_name`、Bundle 复制、签名/公证、CEF Framework | 高 |

粗略工作量（单人，基于现有构建/测试设施）：

- Linux x64 Release 可行性 spike（导出 + PIC + CMake + 跑 basic/ChildWindow）：**1–2 天**；
- Linux 完整（CEF、打包、SDK 测试、文档）：**再加 3–4 天**；
- FreeBSD：**1 天左右**；
- macOS（含 Bundle、CEF、签名）：**3–5 天**；
- 全平台完整覆盖约 **1.5–2.5 周**；若只做“Linux 非 CEF 共享版”，可压缩到 **3–5 天**。

## 八、若未来实施：建议路线

1. **先做 Linux x64 Release 的非 CEF spike**，验证三件事：
   - `DUILIB_API` 可见性改造后，示例只链接 `libduilib.so` 能正常链接、启动；
   - `nm -D` 导出集合与静态库对比无明显缺失；
   - Skia / CEF wrapper / 第三方静态库 PIC 无链接错误。
2. spike 通过后，再决定 CEF 策略：**优先“共享版仅 CEF-off”**；若要 CEF 共享，
   评估拆分独立模块。
3. 然后接 `pack.py` / CMake config / SDK 测试，保持静态包完全不变，共享包只作为 opt-in。
4. FreeBSD 复用 Linux 流程；macOS 最后做，重点处理 install_name、Bundle、签名。
5. `build.py` 上建议：
   - `--dll` 保持 Windows 语义不变；
   - Unix 增加 `--shared`（或让 `--dll` 平台中立但**不隐含 `--md`**）；
   - 模式标记继续用 `dll`，但 `sync_msvc_dll_flag` 只走 Windows。

## 九、决策与重新评估条件

- **2026-10-28：暂不实施。** Unix 继续使用静态库，Windows `--dll` 保持不变。
- 理由：改造面涉及导出可见性、PIC、CEF 运行期语义、rpath/install_name 与打包/下游接入；
  在没有明确收益前，风险和成本高于收益；Unix 生态本身也更习惯静态链接。
- 重新评估的触发条件（满足任一即可重新立项）：
  - 多个程序需要共享同一份 duilib 运行库，或需要插件化/热更新；
  - 分发包体积、链接时间或内存占用出现可量化的显著收益；
  - macOS/Linux 的下游接入方明确要求动态库形态；
  - 上游 CEF/Skia 的构建产物或链接方式出现变化，降低了本次评估中的主要障碍。

## 十、关键代码位置索引

| 主题 | 位置 |
| :--- | :--- |
| `DUILIB_DLL` 选项与 Windows/MSVC 限制 | `cmake/duilib_root_options.cmake:291-299` |
| duilib 目标创建与平台分支 | `duilib/CMakeLists.txt:280-291` |
| Windows DLL 专用链接与 `/DELAYLOAD` | `duilib/CMakeLists.txt:323-373` |
| Unix 可见性编译选项 | `cmake/duilib_compiler_flags.cmake:16,28` |
| Unix `DUILIB_API` 定义 | `duilib/duilib_config_{linux,macos,freebsd}.h` |
| 示例的平台库链接 | `cmake/duilib_bin_{linux,macos,freebsd}.cmake` |
| `build.py --dll` 入口 | `build/build.py`（`--dll` 参数与 `_cmake_defines`） |
| 打包的静态库假设 | `build/pack.py:313,437-444,1714` |
| SDK CMake 库查找与平台依赖 | `packaging/cmake/nim_duilibConfig.cmake` |
| 非 CEF 示例的 CEF 隔离现状 | `docs/misc/Packaging-Follow-Up-Items.md`（P1-3 相关记录） |
