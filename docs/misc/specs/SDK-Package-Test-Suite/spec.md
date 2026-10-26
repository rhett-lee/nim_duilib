# 发布 SDK 包测试用例与测试方法（规格说明）

## 1. 背景与目标

现有自动化测试覆盖的是**构建/打包过程**：

- `build/test_build.py`：构建入口的参数解析、目录推导等纯函数；
- `build/test_pack.py` / `build/autotest.py`：打包脚本的收集规则、结构与 `--verify`；
- `build/test-matrix.md`：人工回归矩阵。

缺少的是对**成品 SDK 包**的独立验证：解包后能否被下游工程正确消费。下游最常见的
失败并非打包脚本报错，而是包内元数据、接入文件、头文件、库或运行期文件组合错误，
直到用户新建工程时才暴露。

本次交付一套可重复执行的 SDK 包测试用例：

1. 以解包后的发布包为唯一输入，校验布局、校验和、元数据与接入文件；
2. 用一个真实的 CMake 消费者工程完成「配置 → 编译 → 链接 → 运行」闭环；
3. 覆盖静态/DLL、CEF 有/无、SDL 有/无、MSVC `/MT` `/MD`、Debug/Release、架构、包格式；
4. 覆盖损坏包、篡改包、架构/配置/运行库/工具链不匹配等负向场景；
5. 输出 PASS/FAIL/SKIP 结果与 JSON 报告，并配套一份中文测试方法文档。

不在范围：修改 `pack.py` 的打包规则、修改 `duilib/` 源码、接入 CI、生成新的 SDK 包
（测试只消费已有包；需要新包时仍走 `build.py pack`）。

## 2. 交付物

| 文件 | 作用 |
| :--- | :--- |
| `build/sdk_test/CMakeLists.txt` | 下游消费者用例工程：只依赖解包后的 SDK，不引用源码树 |
| `build/sdk_test/main.cpp` | 运行期用例：启动/关闭 `GlobalManager`，输出实际编译特性 |
| `build/sdk_test/header_duilib.cpp` | 头文件自包含用例：单独包含 `duilib/duilib.h` |
| `build/sdk_test/header_cef.cpp` | CEF 头文件用例（仅 `HAS_CEF=1` 时参与编译） |
| `build/sdk_test/header_webview2.cpp` | WebView2 头文件用例（仅 `HAS_WEBVIEW2=1` 时参与编译） |
| `build/sdk_package_test.py` | 用例编排器：解包、Python 结构用例、CMake 消费用例、负向用例、报告 |
| `build/test_sdk_package.py` | 编排器纯函数单元测试（由 `build.py test` 自动发现） |
| `build/sdk-package-test.md` | 测试方法与用途文档（中文） |
| `build/README.md`（修改） | 文件概览表登记新增文件 |

所有临时产物（解包目录、CMake 构建树、报告）默认写到
`<系统临时目录>/nim_duilib_sdk_test/<时间戳>/`（`tempfile.gettempdir()`），
不写入仓库的 `dist/`、`lib/`、`bin/`、`out/`。

## 3. 测试架构

### 3.1 消费者用例工程 `build/sdk_test`

输入变量（由 Runner 传入）：

| 变量 | 含义 |
| :--- | :--- |
| `NIM_DUILIB_SDK_DIR` | 解包后的 SDK 根目录（Runner 传绝对路径；手工使用时可省略或写相对路径，工程会自动探测） |
| `SDK_EXPECT_VERSION` | 期望版本（空表示不校验） |
| `SDK_EXPECT_SYSTEM` / `ARCH` / `CONFIG` / `TOOLCHAIN` | 期望平台元数据 |
| `SDK_EXPECT_IS_DLL` / `HAS_SDL` / `HAS_CEF` / `HAS_WEBVIEW2` / `HAS_JPEG` | 期望能力开关 |
| `SDK_EXPECT_MSVC_RUNTIME` | 期望 MSVC 运行库（`MT`/`MD`/空） |

流程：

1. 先解析 `NIM_DUILIB_SDK_DIR`（绝对路径直接用；相对路径以 `sdk_test` 自身、它的
   上一级/上两级、构建目录及其上一级/上两级为基准解析，取第一个命中；省略时探测
   `sdk_test` 的上一级等常见布局），找到含
   `lib/cmake/nim_duilib/nim_duilibConfig.cmake` 的目录后加入 `CMAKE_PREFIX_PATH`，
   再 `find_package(nim_duilib CONFIG REQUIRED)`；找不到时报错并列出尝试过的目录；
2. 用 `sdk_expect()` 逐项比对 `NIM_DUILIB_PACKAGE_*` 变量与期望值，不一致直接
   `FATAL_ERROR`（这是「元数据自洽」用例的编译期部分）；
3. 目标 `nim_duilib_sdk_consumer` 链接 `nim_duilib::duilib`；
4. `nim_duilib_copy_runtime(... RESOURCES)` 复制 `bin/*.dll`（DLL 模式）与
   `bin/resources/` 到可执行文件同级；
5. `HAS_CEF=1` 时追加 `header_cef.cpp`，`HAS_WEBVIEW2=1` 时追加
   `header_webview2.cpp`。头文件缺失会导致编译失败，从而暴露「声明有但没随包」。

`main.cpp` 输出（供 Runner 断言）：

```
SDK_TEST_BEGIN
SDK_TEST_FEATURES sdl=<0|1> cef=<0|1> webview2=<0|1> jpeg=<0|1> dll=<0|1>
SDK_TEST_RESOURCE_ROOT=<路径>
SDK_TEST_RESULT=ok
```

- 特性值取自 SDK 注入的编译宏（`DUILIB_SDL` / `DUILIB_CEF` / `DUILIB_WEBVIEW2` /
  `DUILIB_JPEG_TURBO` / `DUILIB_DLL`），与 `nim_duilibPackageInfo.cmake` 交叉验证；
- 运行期调用
  `GlobalManager::Instance().Startup(LocalFilesResParam(GetResourceRootPath(false)))`，
  成功后 `Shutdown()`；失败返回非 0。该路径会初始化 Skia 渲染工厂、字体管理器、
  解码器与内部线程并加载主题资源，是静态库完整链接的最强单项证明。

### 3.2 编排器 `build/sdk_package_test.py`

主要纯函数（可单测）：

| 函数 | 作用 |
| :--- | :--- |
| `parse_package_name(name)` | 拆分版本、系统、架构、`-sdl/-cef/-cef109/-dll/-examples/-debug` |
| `parse_cmake_info(text)` | 解析 `nim_duilibPackageInfo.cmake` 的 `set()` |
| `parse_manifest_fields(text)` | 解析 `MANIFEST.txt` 的「版本/平台/配置/工具链/包含示例」 |
| `infer_capabilities(files)` | 由实际文件推断 `is_dll/has_sdl/sdl_kind/has_cef/cef_version/...` |
| `select_archive_root(names)` | 校验单一包根目录 |
| `scenario_applicability(info)` | 决定每个场景是执行还是 SKIP 及原因 |

执行流程：

1. 对每个 `--package` 解包到 `<out>/extract/<序号>/`；
2. 解析元数据并推断实际能力；
3. 依次执行适用场景，异常转为 FAIL 而不是中断；
4. 汇总打印表格，写出 `report.json` 与每个场景的日志文件；
5. 退出码：`0` 全部通过（允许 SKIP）、`1` 存在 FAIL、`2` 全部 SKIP。

## 4. 场景矩阵

| 编号 | 场景 | 适用条件 | 判定依据 |
| :--- | :--- | :--- | :--- |
| P01 | `package.extract` | 全部 | 归档可解包、包根目录唯一 |
| P02 | `package.layout` | 全部 | `include/duilib/duilib.h`、基础库、`lib/cmake/nim_duilib/*`、`MANIFEST.txt`、`SHA256SUMS`、`VERSION`、`docs/*.md`、`LICENSES/`、`LICENSE`、`THIRD-PARTY-NOTICES.md` 全部存在 |
| P03 | `package.checksums` | 全部 | `SHA256SUMS` 覆盖除 `MANIFEST.txt`/`VERSION`/自身外的全部文件，且哈希全部匹配 |
| P04 | `package.metadata` | 全部 | 包名、`VERSION`、`MANIFEST.txt`、`nim_duilibPackageInfo.cmake`、实际文件四者一致（系统/架构/配置/工具链/DLL/SDL/CEF/WebView2/JPEG/MSVC 运行库） |
| P05 | `package.integration` | 全部 | CMake 接入文件无未替换 `@...@`；`win+msvc` 包含 `msvc/nim_duilib.props/.targets/.sdk.props` 与三份 manifest，XML 可解析；`ConfigVersion` 含 `PACKAGE_VERSION` |
| P06 | `package.forbidden` | 全部 | 不含 `.pdb/.ilk/.exp/.obj/.o`、`cef_sandbox.lib`、`bin/resources.zip`；`include/` 下无 `.cpp`；未启用 CEF 时无 CEF 头与 wrapper |
| P07 | `package.verify-cli` | 全部 | `python build/pack.py --verify <包>` 返回 0 |
| P08 | `package.docs` | 全部 | `docs/` 至少一份文档；`LICENSES/` 非空；`bin/resources/global.xml` 存在（运行必需） |
| C01 | `consumer.static` | `IS_DLL=0` 且本机可编译同平台/架构 | 配置、编译、运行通过；运行输出 `SDK_TEST_RESULT=ok`，特性值与元数据一致 |
| C02 | `consumer.dll` | `IS_DLL=1` 且本机可编译同平台/架构 | 同上；额外确认运行时 DLL 被复制到可执行文件同级并能加载 |
| C03 | `negative.config` | 单配置生成器 | 用相反 `CMAKE_BUILD_TYPE` 配置必须失败，错误含「配置为」 |
| C04 | `negative.arch` | MSVC 有 VS 生成器，或包架构≠主机架构 | 用相反架构配置必须失败，错误含「架构为」 |
| C05 | `negative.msvc-runtime` | Windows + MSVC + 元数据为 `MT`/`MD` | 显式设置与包相反的 `CMAKE_MSVC_RUNTIME_LIBRARY` 必须失败，错误含「运行库」 |
| C06 | `negative.toolchain` | 包工具链与另一套编译器均可用 | 用相反工具链配置必须失败，错误含「该 nim_duilib SDK 是」 |
| C07 | `negative.incomplete` | 全部 | 副本删除 `lib` 主库或 `include/duilib/duilib.h` 后 `pack.py --verify` 必须失败 |
| C08 | `negative.tampered` | 全部 | 副本改动一个文件字节后 `pack.py --verify` 必须报告 SHA 不匹配 |
| C09 | `package.tar-formats` | 全部 | 由解包内容重新生成 `.tgz` 与 `.tar.bz2` 后 `pack.py --verify` 仍通过 |
| C10 | `examples.smoke` | 显式 `--run-examples` 且包含示例 | 启动 `bin/basic[.exe]`，3 秒后仍在运行，随后终止 |

SKIP 必须写明原因（例如「DLL 包但当前平台不支持运行」「未安装 MinGW」），计入报告。

## 5. CLI 约定

```
python build/sdk_package_test.py --package dist/<包> [--package <包2>] \
       [--scenarios all|P01,C01,...] [--no-build] [--run-examples] \
       [--out-dir DIR] [--keep] [--jobs N] [--dry-run] [--list-scenarios]
```

- `--list-scenarios`：只打印场景表并退出；
- `--no-build`：只执行 `P*` 场景（无编译器环境可用）；
- `--dry-run`：打印将执行的命令，不真正配置/编译/运行；
- `--keep`：保留解包与构建目录便于排查；
- 默认使用 Ninja（可用时），否则交给 CMake 选择默认生成器；`--jobs` 传给
  `cmake --build --parallel`。

## 6. 文档 `build/sdk-package-test.md`

章节：目的与定位（与其它测试的分工）、前置条件、快速开始、场景说明表、
消费者工程单独用法、结果解读与报告结构、常见失败排查、扩展新场景的方法、
按包类型的覆盖矩阵（静态/DLL × CEF/SDL × 配置 × 架构）。

## 7. 验收步骤

1. `python -m py_compile build/sdk_package_test.py build/test_sdk_package.py`；
2. `python build/build.py test`：新增单测与既有单测全部通过；
3. `python build/sdk_package_test.py --list-scenarios` 正确列出 P01–P08、C01–C10；
4. 用现有 Debug 静态包执行：
   `python build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-win-x64-examples-debug.zip`
   —— P* 与 C01 通过，负向场景（C03/C05/C07/C08/C09）通过；
5. 用现有 DLL Debug 包执行同一命令：C02 通过、C01 SKIP；
6. 若某负向场景因环境缺失无法执行，必须输出 SKIP 及原因，不得计为通过；
7. 不提交代码、不生成正式包（按用户约定由用户手动执行提交与 Release 编译）。

## 8. 风险与假设

- 消费者工程按 SDK 元数据选择行为，不读取仓库源码；若包本身缺少 CMake 接入文件，
  C01/C02 会按失败处理，符合预期。
- 运行期启动依赖 `bin/resources/`；`nim_duilib_copy_runtime(RESOURCES)` 已覆盖。
- GUI 示例冒烟（C10）默认关闭，避免无人值守环境弹窗干扰。
- 负向场景依赖具体错误文案（`配置为`/`架构为`/`运行库`/`该 nim_duilib SDK 是`），
  这些文案来自 `packaging/cmake/nim_duilibConfig.cmake`，若上游文案调整需同步
  测试断言。
