# nim_duilib 发布 SDK 包测试（sdk_package_test.py）

本文说明 `build/sdk_package_test.py` 的用途、用法、场景判定、结果解读与扩展方式。
该工具以**解包后的成品 SDK 包**为唯一输入，不读取 `lib/`、`bin/` 或源码树，也不构建
nim_duilib 自身；它验证的是下游用户实际拿到的东西。

## 1. 目的与定位

`build/sdk_package_test.py` 做三件事：

1. **包级校验**：解包、目录布局、SHA256SUMS、元数据、CMake/MSVC 接入文件、禁止内容、
   `pack.py --verify`、文档与许可证、tar 格式重打包；
2. **真实消费者验证**：用 `build/sdk_test` 这个仅依赖解包 SDK 的 CMake 工程完成配置、
   编译、链接、运行，并核对运行期编译特性；macOS 上还会用同一工程生成 App Bundle，
   验证 `FRAMEWORKS`/rpath/`Contents/Resources/duilib/resources` 的部署是否正确；
3. **负向用例**：配置、架构、MSVC 运行库、工具链不匹配时必须被 SDK 拒绝；缺库/缺头
   文件、被篡改的包必须被 `pack.py --verify` 拒绝。

它与构建、打包、自动化工具的分工如下：

| 工具 | 输入 | 主要职责 | 与本工具的关系 |
| :--- | :--- | :--- | :--- |
| `build/build.py` / `test_build.py` | 源码、构建目录 | 依赖准备、编译、测试、出包的统一入口；`test_build.py` 测入口逻辑 | 本工具不构建仓库，只复用 `build.py` 的 `detect_toolchain()` 探测编译环境 |
| `build/pack.py` / `test_pack.py` | 已构建的 `lib/<arch>/<Config>`、`bin/` | 收集产物并生成发布包；`--verify` 校验已有包 | 本工具在 P07、C07、C08、C09 中调用 `pack.py --verify`，但不负责出包 |
| `build/autotest.py` | 环境 + 现有产物（可选先构建） | 环境检测 → 构建 → 打包 → 体检 → 可选冒烟 → 报告 | 本工具从“包已经存在”开始，额外做下游消费者编译/运行和负向配置用例 |
| `build/test-matrix.md` | 人工维护 | 构建与打包组合的回归矩阵、命令和状态记录 | 矩阵提供“待测包”的来源与组合清单；本工具提供包级与消费者级自动判断 |
| `build/test_sdk_package.py` | 无（纯函数） | 覆盖包名解析、元数据解析、能力推断、场景适用性、场景选择 | 不需要真实发布包；随 `python3 build/build.py test` 一起执行 |

> 两个脚本名字相近，注意区分：真正测试发布包的是
> `python3 build/sdk_package_test.py --package <包>`；`build/test_sdk_package.py`
> 只跑纯函数单测。若误把 `--package`、`--list-scenarios` 等编排器参数传给后者，
> 它会打印提示并自动转交给 `sdk_package_test.py`。

本工具**不做**这些事：

- 不生成发布包、不编译 nim_duilib 库，也不修改源码；
- 不替代 `pack.py --verify` 本身，只是把它的结果纳入 P07/C07/C08/C09；
- 不验证示例程序的完整功能，C10 只做“启动后存活 3 秒”的冒烟；
- C11 只验证 App Bundle 的部署与启动，不创建窗口、不验证界面渲染；
- 不做跨平台运行：消费者场景要求包平台与本机一致，负向场景也受工具链和生成器限制。

## 2. 前置条件

### 2.1 通用要求

| 项目 | 要求 | 说明 |
| :--- | :--- | :--- |
| Python | Python 3 | 脚本仅用标准库；P07/C07/C08/C09 会用当前 Python 解释器调用 `pack.py` |
| 待测包 | 已由 `build.py pack` 或 `build/pack.py` 生成 | 支持 zip、tgz、tar.bz2 等 `shutil.unpack_archive` 可解格式；归档必须只有一个顶层目录 |
| 磁盘空间 | 至少能容纳解包内容与消费者构建树 | 带示例/CEF 的包可达数百 MB，Debug 消费者构建还会额外占用空间；C07/C08/C09 只写几 KB 的代表性小归档。编译器的临时文件写在 `--out-dir` 下的 `tmp/`，不占用系统临时目录；启动时会按包大小估算并在空间不足时打印 `[WARN]` |
| CMake | 3.18+（`build/sdk_test/CMakeLists.txt` 的最低版本） | 只影响 C01–C06、C11；找不到 `cmake` 时记为 SKIP |
| 生成器/编译器 | CMake 可用的 Ninja、Makefiles 或 Visual Studio 生成器 | 只影响 C01–C06、C11；工具链经 `build.py` 的 `detect_toolchain()` 探测 |
| 桌面会话 | 能启动 GUI 程序 | 只影响 C10；无桌面环境时示例可能启动即退出，从而判为 FAIL |

### 2.2 编译类场景的环境要求

`SCENARIO_ORDER` 把 C01–C06、C10、C11 登记为“需编译”，但实际行为有区别：

| 场景 | 环境要求 | 是否真正编译 |
| :--- | :--- | :--- |
| C01 / C02 | 包平台 = 本机平台；CMake + 生成器 + 可用编译器；Windows MSVC 需 VS 2022+（C++ 工作负载），MinGW 需 MSYS2 且设置 `MSYSTEM`，`mingw-llvm` 还需 `CC`/`CXX` 指向 clang | 是：配置、编译、链接、运行 |
| C03–C06 | 同上，但不要求能正常编译本包；C03 还要求生成器不是 Visual Studio 多配置生成器 | 否：只执行 CMake 配置并期待失败 |
| C10 | 包平台 = 本机平台；工具链探测通过；包内含示例；指定 `--run-examples`；可启动 GUI | 否：直接运行包内 `bin/basic` |
| C11 | macOS 本机 + macOS 包 + 包内有 SDL3 动态库；CMake + Ninja/Makefiles + 可用编译器；系统 `otool` | 是：配置、编译、链接、检查 bundle、迁移后运行 |

因此：

- **完全不需要编译器**：P01–P08、C07–C09。它们只做归档读取、哈希、重打包和
  `pack.py --verify`，适合在任何机器上先跑一遍。
- **需要编译环境**：C01、C02、C11（C11 仅 macOS）。
- **需要工具链环境但不编译**：C03–C06。
- **需要同平台且能启动 GUI**：C10。
- `--no-build` 会跳过 C01–C06、C10、C11，包括 C03–C06 的配置阶段。

平台与工具链的前置判断集中在 `build_block_reason()`：包平台与本机不同、MSVC 包不在
Windows、MinGW 包未设置 `MSYSTEM`、`mingw-llvm` 包的 `CC`/`CXX` 不含 `clang`、
工具链探测失败，都会让 C01/C02/C10/C11 直接 SKIP。C03–C06 在此基础上还有各自的额外门禁。

## 3. 快速开始

### 3.1 生成待测包

发布包只由 `pack.py` 生成，默认输出到 `dist/`：

```bash
# 默认 Release、仅 SDK、Windows 输出 zip
python3 build/pack.py

# Debug + CEF + 示例：覆盖静态消费者、CEF 头文件与 C10
python3 build/build.py pack --config Debug --with-cef --with-examples

# 指定 SDL、CEF、示例与输出目录
python3 build/pack.py --config Release --with-sdl --with-cef --with-examples --out-dir dist

# 指定归档格式（Windows 默认 zip，其他平台默认 tgz）
python3 build/pack.py --config Debug --format tarbz2 --out-dir dist
```

生成后建议先自查一次：

```bash
python3 build/pack.py --verify dist/nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip
```

打包前要确认 `lib/<arch>/<Config>` 与 `bin/` 中只有目标架构、目标工具链、目标配置的
产物；混入旧库或旧示例会让 P04/C01/C02 失败。包名规则见 `pack.py:build_package_name`：

```text
nim_duilib-<版本>-<系统>-<架构>[-mingw-gcc|-mingw-llvm][-sdl][-cef|-cef109][-examples][-dll][-debug]
```

`-mingw-gcc` / `-mingw-llvm` 用于区分 C++ 运行库；Unix 包默认带 SDL，包名不追加
`-sdl`；`-dll` 表示 DLL 模式，`-debug` 表示 Debug 配置。

### 3.2 常用命令

以下命令在仓库根目录执行；Windows 上可把 `python3` 换成 `python`。

| 命令 | 作用 |
| :--- | :--- |
| `python3 build/sdk_package_test.py --list-scenarios` | 列出全部场景编号、名称、类型和说明后退出 |
| `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip` | 对单个包执行全部适用场景 |
| `python3 build/sdk_package_test.py --package dist/<包> --no-build` | 只做包级场景（P01–P08、C07–C09） |
| `python3 build/sdk_package_test.py --package dist/<包> --scenarios P01,C01` | 只跑指定场景，编号大小写不敏感 |
| `python3 build/sdk_package_test.py --package dist/<macOS包> --scenarios C11 --keep` | 只验证 macOS App Bundle（`FRAMEWORKS`/rpath/资源布局），保留构建目录与日志 |
| `python3 build/sdk_package_test.py --package dist/a.zip --package dist/b.zip --jobs 4` | 多包依次执行；每个包独立输出目录与报告条目 |
| `python3 build/sdk_package_test.py --package dist/<包> --keep` | 保留解包目录、消费者构建目录和日志 |
| `python3 build/sdk_package_test.py --package dist/<包> --dry-run` | 不执行编译类场景；包级场景仍真实执行 |
| `python3 build/sdk_package_test.py --package dist/<包> --run-examples` | 允许执行 C10 示例冒烟 |
| `python3 build/sdk_package_test.py --package dist/<包> --out-dir <输出目录>/manual` | 指定输出目录（不再追加时间戳子目录） |

`--package` 可重复指定，且每个路径必须是已存在的文件。`--scenarios` 默认为 `all`；
空值、`all` 都表示全部场景；未知编号会直接报错退出。

### 3.3 参数与运行模式

| 参数 | 默认 | 行为 |
| :--- | :--- | :--- |
| `--package ARCHIVE` | 无（除非 `--list-scenarios`） | 待测发布包，可重复；路径会 `resolve()` 为绝对路径 |
| `--scenarios LIST` | `all` | 逗号分隔的场景编号；`all` 或空串表示全部 |
| `--no-build` | 关 | 跳过所有“需编译”场景（C01–C06、C10、C11） |
| `--run-examples` | 关 | 解除 C10 的开关门禁；仍需包内含示例且环境满足 |
| `--out-dir DIR` | 平台默认目录 | 输出目录；显式指定时不再追加时间戳；解包、编译临时文件与重打包都在此目录，需预留足够空间 |
| `--keep` | 关 | 保留 `extract/`、`work/`、`logs/` |
| `--jobs N` | CPU 数，取不到时 4 | 传给 `cmake --build --parallel` |
| `--dry-run` | 关 | 跳过编译类场景；包级场景照常执行；保留输出目录 |
| `--list-scenarios` | 关 | 打印场景表后以 0 退出，不需要 `--package` |

三个开关的准确行为：

- **`--no-build`**：`scenario_applicability()` 对每个编译类场景返回不适用，原因为
  “指定了 --no-build”。C01–C06、C10、C11 全部 SKIP；P01–P08、C07–C09 仍执行。
- **`--dry-run`**：`run_package()` 在进入场景前就把编译类场景记为
  `SKIP / dry-run：未执行`，因此 **不会调用 CMake，也不会把构建命令写进日志**。
  CLI 帮助中的“只打印将执行的命令”在本工具里实际表现为“跳过编译类场景”。
  包级场景仍会真实解包、计算哈希，并用代表性小归档调用 `pack.py --verify`。
  该模式下不做任何清理，输出目录会保留。
- **`--run-examples`**：只是 C10 的开关门禁。未指定时 C10 SKIP “未指定
  --run-examples”；指定后还必须满足“包内含示例”和平台/工具链前置条件。
- **`--keep`**：不加时，每个包跑完会删除 `extract/` 与 `work/`，写报告后还会删除
  `<out>/<NN>-<包名>` 整个目录，最终只留下 `report.json`；`--keep` 时才保留日志和
  构建目录。`--dry-run` 同样保留。

## 4. 场景说明

场景按 `SCENARIO_ORDER` 的顺序执行和展示，编号是稳定标识。类型中的“包级”表示不需要
编译；“需编译”表示由 `--no-build`/`--dry-run` 统一控制，但 C03–C06 实际只做配置。

| 编号 | 名称 | 类型 | 适用条件 | 判定依据 | 常见 SKIP 原因 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| P01 | `package.extract` | 包级 | 任意可加载的包 | 归档只有一个顶层目录；解包后恰好得到一个根目录；报告文件数与根目录名 | 无；失败通常在加载阶段变成 P00 FAIL |
| P02 | `package.layout` | 包级 | 任意包 | 必需文件 10 项齐全；`lib/` 下有 `duilib` 主库；静态包还需 `duilib-cximage`、`duilib-webp`、`duilib-png`、`duilib-zlib`、`skia`；`bin/resources/themes/default/global.xml` 存在 | 无 |
| P03 | `package.checksums` | 包级 | 任意包 | `SHA256SUMS` 每行可解析；除 `MANIFEST.txt`、`VERSION`、`SHA256SUMS` 外的普通文件全部在册且无多余条目；实际 SHA256 与登记值一致 | 无 |
| P04 | `package.metadata` | 包级 | 包名符合命名规则 | 包名中的版本/平台/架构/配置与 CMake、`VERSION`、`MANIFEST.txt` 一致；`IS_DLL`/`HAS_SDL`/`HAS_CEF`/`HAS_WEBVIEW2`/`HAS_JPEG` 在包名、CMake 元数据、实际文件三处一致；示例标志；MinGW 变体；MSVC 运行库规则。macOS 例外：`-cef` 表示 CEF 示例 App Bundle（内嵌 Framework）随包，`HAS_CEF` 必须为 0（SDK 不提供 CEF 开发文件） | 无；包名无法解析也判 FAIL |
| P05 | `package.integration` | 包级 | 任意包 | `*.cmake` 无未替换的 `@...@` 占位符；`nim_duilibConfigVersion.cmake` 含 `PACKAGE_VERSION`；Windows 包有 `msvc/manifest/duilib.{x64,x86,arm64}.manifest` 且 XML 可解析；MSVC 包还有 `msvc/nim_duilib.props`、`.targets`、`.sdk.props` 及 4 个属性键 | 无 |
| P06 | `package.forbidden` | 包级 | 任意包 | 不含 `.pdb`/`.ilk`/`.exp`/`.obj`/`.o`；不含 `cef_sandbox.lib`；`include/` 下无 `.cpp`；无 `bin/resources.zip`；未声明 CEF 时不含 CEF 头；未声明 SDL 时不含 `include/SDL3/` | 无 |
| P07 | `package.verify-cli` | 包级 | 任意包，且 `pack.py` 可运行 | `python build/pack.py --verify <原包>` 返回 0 | 无；`pack.py` 缺失或无法执行会 FAIL |
| P08 | `package.docs` | 包级 | 任意包 | `docs/` 至少一份 `.md`；`LICENSES/` 非空；`THIRD-PARTY-NOTICES.md` 与 `.en.md` 均存在；运行资源 `global.xml` 存在 | 无 |
| C01 | `consumer.static` | 需编译 | 静态包；未 `--no-build`；平台与工具链可用 | 用 `build/sdk_test` 配置（`find_package(nim_duilib CONFIG)` + `SDK_EXPECT_*` 自检）→ 编译 → 找到 `sdk_consumer` → 可执行文件同级有 `resources/` → 运行返回 0 且输出 `SDK_TEST_RESULT=ok` → `SDK_TEST_FEATURES` 与包能力一致 | DLL 包改由 C02；`--no-build`；平台不同；MSVC 包不在 Windows；MinGW 包无 `MSYSTEM`；`mingw-llvm` 的 `CC`/`CXX` 不含 `clang`；工具链不可用 |
| C02 | `consumer.dll` | 需编译 | DLL 包；其余条件同 C01 | 同 C01，并额外检查 `bin/` 下所有顶层 DLL 已复制到可执行文件同级 | 静态包改由 C01；其余同 C01 |
| C03 | `negative.config` | 需编译（仅配置） | 平台与工具链可用；生成器不是 Visual Studio 多配置 | 用相反配置（Debug↔Release）配置必须失败，stdout 含“配置为” | 构建前置不满足；工具链不可用；生成器为 Visual Studio，无法用 `CMAKE_BUILD_TYPE` 触发校验 |
| C04 | `negative.arch` | 需编译（仅配置） | 能构造与包不同的架构 | 用相反架构配置必须失败，stdout 含“架构为”；Windows 优先用 VS 生成器的 `-A`（x64↔x86，arm64→x64），非 Windows 用主机架构与包架构不一致来触发 | 构建前置不满足；工具链不可用；非 Windows 且包架构 = 主机架构；Windows 无 VS 且包架构 = 主机架构 |
| C05 | `negative.msvc-runtime` | 需编译（仅配置） | Windows MSVC 包，且元数据声明了 `MT`/`MD` | 传入相反的 `CMAKE_MSVC_RUNTIME_LIBRARY` 后配置必须失败，stdout 含“运行库” | 非 Windows 或非 MSVC 包；包未声明 MSVC 运行库；工具链不可用 |
| C06 | `negative.toolchain` | 需编译（仅配置） | Windows 包，且本机存在“对立”工具链 | MSVC 包改用 Ninja + `g++` 配置；MinGW 包改用本机 MSVC 配置；必须失败且 stdout 含“该 nim_duilib SDK 是” | 非 Windows 包；工具链不可用；MSVC 包但无 `g++` 或无 Ninja；MinGW 包但无 VS 或 `cl.exe` |
| C07 | `negative.incomplete` | 包级 | 任意包；需要生成小归档 | 从真实包挑最小头文件与最小静态库，分别生成“只含头文件（缺库）”和“只含库（缺头文件）”的小 tgz，两次 `--verify` 都必须非 0，且输出分别含 `缺少 lib/ 静态库`、`缺少 include/duilib/ 头文件` | 包内既无头文件也无静态库时 FAIL |
| C08 | `negative.tampered` | 包级 | 任意包；需要生成小归档 | 篡改 `bin/resources/themes/default/global.xml`（不存在时取 SHA256SUMS 中体积最小的文件），小归档的 SHA256SUMS 仍写原内容哈希，`--verify` 必须失败且输出含 `SHA256` | 无 |
| C09 | `package.tar-formats` | 包级 | 任意包；需要生成小归档 | 用最小头文件、最小静态库与运行资源生成 `.tgz` 与 `.tar.bz2`，两者分别通过 `pack.py --verify` | 无 |
| C10 | `examples.smoke` | 需编译（不编译） | 指定 `--run-examples`；包内含示例；平台与工具链前置满足 | 启动 `bin/basic.exe`（Unix 为 `bin/basic`，macOS 为 `bin/basic.app/Contents/MacOS/basic`），3 秒后进程仍存活 | 未指定 `--run-examples`；包内无示例；平台/工具链前置不满足；找不到 `bin/basic` |
| C11 | `consumer.macos-bundle` | 需编译 | 仅 macOS 本机；macOS 包；包内有 SDL3 动态库；平台与工具链前置满足 | 用 `build/sdk_test` 构建 `sdk_bundle.app`（`MACOSX_BUNDLE` + `RESOURCES FRAMEWORKS`）→ 检查 `Contents/Frameworks/libSDL3*.dylib`、`Contents/Resources/duilib/resources/themes/default/global.xml` → `otool -L` 引用 `@rpath/libSDL3*` 且无 SDK 绝对路径 → `otool -l` 含 `LC_RPATH @executable_path/../Frameworks` → `otool -D` 为 `@rpath/...` → 把 `.app` 复制到构建树外运行，要求返回 0、输出 `SDK_TEST_RESULT=ok` 与正的 `SDK_TEST_SDL_VERSION` | 非 macOS；非 macOS 包；包内无 SDL3 动态库（静态 SDL 或无 SDL）；`--no-build`；工具链不可用；找不到 `otool` |

### 4.1 包级场景细节

- P01 的加载发生在场景调度之前：`load_package()` 先读取归档成员、用
  `select_archive_root()` 确认顶层目录唯一，再解包并收集普通文件（不跟随符号链接）。
  加载失败时，该包会写入一条 `P00`/`package.load` 的 FAIL 记录，后续场景不再执行。
- P03 把 `MANIFEST.txt`、`VERSION` 视为虚拟文件，不要求登记进 `SHA256SUMS`；
  `SHA256SUMS` 自身也不登记。符号链接不参与 `collect_files()`，因此不计入哈希集合。
- P04 的 `IS_DLL`/`HAS_SDL`/`HAS_CEF`/`HAS_WEBVIEW2`/`HAS_JPEG` 不是只看包名：
  包名、`nim_duilibPackageInfo.cmake` 元数据、实际文件推断三者必须一致。MSVC 运行库
  还要求：DLL 包必须是 `MD`；静态包可为 `MT` 或 `MD`；非 Windows/MSVC 包不得声明
  MSVC 运行库。`HAS_JPEG` 按实际文件推断：静态包看 `lib/turbojpeg*.lib`，
  DLL 包看运行期 `bin/turbojpeg.dll`。macOS 的 CEF 语义不同：SDK 不提供 CEF
  开发文件（`HAS_CEF` 恒为 0），包名中的 `-cef` 表示 CEF 示例 App Bundle
  （内嵌 Chromium Embedded Framework）随包。
- P05 对 Windows 包统一要求三份架构 manifest（即使 MinGW 包也一样）；只有 MSVC 包
  额外要求 `msvc/nim_duilib.props`、`nim_duilib.targets`、`nim_duilib.sdk.props`。
- C07/C09 用 `_make_sample_package()` 生成只含 `MANIFEST.txt`、`VERSION`、一个最小
  头文件、一个最小静态库（C09 另加 `global.xml`）、少量符号链接的小归档，并重建
  `SHA256SUMS`。因此重打包耗时与包体积无关；完整包的结构与校验和仍由 P01–P03、
  P07 覆盖。C07 还要求两次失败原因分别命中“缺少库”“缺少头文件”，避免因其他错误
  误判。
- C08 优先篡改 `global.xml`，因为它在任何包里都存在且通常较小；篡改方式是在原内容
  末尾追加一段注释，而小归档的 `SHA256SUMS` 仍写原内容哈希，因此失败原因必须是
  `SHA256` 不匹配；归档同时带上头文件与静态库，避免“缺文件”掩盖哈希问题。
- C09 只验证 tgz/tar.bz2 的写出与 `--verify` 接受；原始包如果是 zip，其 zip 路径由
  P07 覆盖。

### 4.2 编译类场景细节

C01/C02 的实际步骤与限制：

1. 通过 `build.py` 探测工具链；Visual Studio 生成器会按包元数据加 `-A`
   （`x86`→`Win32`、`x64`→`x64`、`arm64`→`ARM64`），单配置生成器使用
   `-DCMAKE_BUILD_TYPE=<包配置>`；
2. 把包元数据转换成 `-DSDK_EXPECT_*` 传给消费者工程，先验证 CMake 包元数据自洽；
3. 编译 `build/sdk_test` 的 `nim_duilib_sdk_consumer` 目标（macOS 工程里还有 bundle
   目标，但这里用 `--target` 只构建消费者），搜索输出名为 `sdk_consumer` 的可执行文件；
4. 检查可执行文件同级是否有 `resources/themes/default/global.xml`；DLL 包还要求
   `bin/` 下所有顶层 DLL 都已复制过来；
5. 运行程序，要求返回码为 0 且输出含 `SDK_TEST_RESULT=ok`；
6. 解析 `SDK_TEST_FEATURES`，核对 `sdl`/`cef`/`webview2`/`jpeg`/`dll` 与包能力一致。

超时限制：配置 1800 秒、编译 3600 秒、运行 180 秒；负向配置 600 秒。超时会判 FAIL。

C01/C02 的“消费者”覆盖范围：

- 始终编译 `main.cpp` 与 `header_duilib.cpp`；
- 包声明 `HAS_CEF=1` 时追加编译 `header_cef.cpp`，声明 `HAS_WEBVIEW2=1` 时追加
  `header_webview2.cpp`；声明支持却未随包转发头文件会在这里编译失败；
- 程序会调用 `GlobalManager::Startup()` 加载资源并初始化，能正常 `Shutdown()` 说明
  静态库（或导入库 + DLL）至少可链接、可启动；
- CEF 运行时目录（如 `bin/libcef_win*/`）不在 C01/C02 的复制与断言范围内，也不在
  程序启动路径中，需要真实 CEF 包配合示例或手工验证。

C03–C06 只执行配置，并检查失败原因关键字：

| 场景 | 注入的错误条件 | 期望配置失败输出含 |
| :--- | :--- | :--- |
| C03 | `CMAKE_BUILD_TYPE` 取相反配置 | `配置为` |
| C04 | Visual Studio 的 `-A` 取相反架构，或主机架构与包架构不一致 | `架构为` |
| C05 | `CMAKE_MSVC_RUNTIME_LIBRARY` 取相反的 `MT`/`MD` | `运行库` |
| C06 | MSVC 包用 Ninja + `g++`，MinGW 包用 MSVC 环境 | `该 nim_duilib SDK 是` |

C10 只负责启动存活判断：找到示例程序后 `Popen` 启动，等待 3 秒；进程已退出则 FAIL，
仍存活则 PASS，随后终止进程。它不检查窗口内容、不点击、不验证渲染结果。

C11 的步骤（仅 macOS）：

1. 用同一个 `build/sdk_test` 工程配置到独立的 `work/consumer_bundle` 目录，只构建
   `nim_duilib_sdk_bundle` 目标；
2. 由 CMake 生成 `sdk_bundle.app`：`bundle_main.cpp` 调用
   `GlobalManager::GetResourceRootPath(true)`（默认参数，macOS 下返回 bundle 内的
   标准目录），`nim_duilib_copy_runtime(... RESOURCES
   FRAMEWORKS)` 把 SDL3 dylib 放进 `Contents/Frameworks`、把主题资源放进
   `Contents/Resources/duilib/resources`，并把可执行文件的 rpath 设为
   `@executable_path/../Frameworks`（使用 `INSTALL_RPATH`，避免构建树的 SDK
   绝对路径抢先命中同名库）；
3. 检查 bundle 布局：`Contents/MacOS/sdk_bundle`、`Contents/Frameworks/libSDL3*.dylib`、
   `Contents/Resources/duilib/resources/themes/default/global.xml`；
4. 用 `otool` 检查链接与加载信息：`-L` 必须是 `@rpath/libSDL3*` 且不含 SDK 根目录的
   绝对路径；`-l` 必须出现 `LC_RPATH @executable_path/../Frameworks`；`-D` 的
   install_name 必须以 `@rpath/` 开头；
5. 把整个 `.app` 复制到 `work/bundle-relocated/`（构建树之外）再运行，避免构建目录
   的绝对 rpath 掩盖问题；运行环境带 `DYLD_PRINT_LIBRARIES=1`，要求 dyld 打印的
   SDL3 路径位于迁移后 `.app` 的 `Contents/Frameworks`，且不出现 SDK 的 `lib/`
   路径；同时要求返回 0、输出 `SDK_TEST_RESULT=ok`、资源根含
   `Contents/Resources/duilib/resources`、`SDK_TEST_SDL_VERSION` 为正数，并核对
   `SDK_TEST_FEATURES` 与包能力一致。

C11 不创建窗口，因此无桌面会话的 macOS 也能执行；它也不负责签名/公证，
`.app` 未签名时直接从 `Contents/MacOS` 启动即可。

## 5. 消费者工程单独用法

`build/sdk_test` 是一个独立的 CMake 工程，只依赖解包后的 SDK，不引用 nim_duilib
源码树。

| 文件 | 作用 |
| :--- | :--- |
| `CMakeLists.txt` | `find_package(nim_duilib CONFIG REQUIRED)`、`SDK_EXPECT_*` 自检、按包能力选择头文件用例、复制运行期文件；macOS 上额外登记 App Bundle 目标 |
| `main.cpp` | 运行期用例：输出特性、检查资源目录、`Startup`/`Shutdown`、输出 `SDK_TEST_RESULT` |
| `bundle_main.cpp` | macOS App Bundle 运行期用例：`GetResourceRootPath(true)` 定位 `Contents/Resources/duilib/resources`、调用 `SDL_GetVersion()` 验证 Frameworks/rpath、`Startup`/`Shutdown` |
| `header_duilib.cpp` | 伞形头文件自包含用例，始终参与编译 |
| `header_cef.cpp` | CEF 头文件转发用例，仅 `HAS_CEF=1` 时编译 |
| `header_webview2.cpp` | WebView2 头文件转发用例，仅 `HAS_WEBVIEW2=1` 时编译 |

目标名是 `nim_duilib_sdk_consumer`，输出文件名为 `sdk_consumer`。macOS 上还会创建
`nim_duilib_sdk_bundle`（`MACOSX_BUNDLE`），输出 `sdk_bundle.app`，内部调用
`nim_duilib_copy_runtime(nim_duilib_sdk_bundle RESOURCES FRAMEWORKS)`；其他平台不创建
该目标。

### 5.1 手工配置与运行

`NIM_DUILIB_SDK_DIR` 指向解包后的 SDK 根目录（同时含 `include/` 与 `lib/` 的那层，
不是归档文件）。它有三种写法，工程都会自动解析：

| 写法 | 示例 | 解析规则 |
| :--- | :--- | :--- |
| 省略 | `cmake -S ./sdk_test -B ./build` | 探测常见布局：`sdk_test` 的上一级、构建目录的上两级等 |
| 相对路径 | `-DNIM_DUILIB_SDK_DIR=./`<br>`-DNIM_DUILIB_SDK_DIR=./nim_duilib-1.0.2026.1023-win-x64` | 分别以 `sdk_test` 自身、它的上一级/上两级、构建目录及其上一级/上两级为基准解析，取第一个命中 |
| 绝对路径 | `-DNIM_DUILIB_SDK_DIR=C:/sdk/nim_duilib-1.0.2026.1023-win-x64` | 直接使用 |

CMake 不提供“调用 cmake 的目录”，所以相对路径靠上面这组基准近似；解析成功时会打印
`nim_duilib SDK 根目录（自动探测）：...`，一个都没找到时报错并列出所有尝试过的目录。
也可以直接指定包配置目录（`<SDK>/lib/cmake/nim_duilib`），或用
`-Dnim_duilib_DIR=<包配置目录>` 交给 `find_package` 自行处理。

把 `sdk_test` 拷进解包后的 SDK 根目录，在根目录里执行（`./` 指的是 `sdk_test`
自己，工程会自动找到上一级的 SDK 根）：

```bash
cmake -S ./sdk_test -B ./sdk_consumer -DNIM_DUILIB_SDK_DIR=./
cmake --build ./sdk_consumer --config Release --parallel 4
./sdk_consumer/Release/sdk_consumer.exe
```

`sdk_test` 与解包目录并列时同样可以写相对路径（以 `sdk_test` 的上一级为基准）：

```bash
cmake -S ./sdk_test -B ./sdk_consumer -DNIM_DUILIB_SDK_DIR=./nim_duilib-1.0.2026.1023-win-x64
cmake --build ./sdk_consumer --config Release --parallel 4
./sdk_consumer/Release/sdk_consumer.exe
```

`--config` 必须与包配置一致（Debug 包用 `Debug`，Release 包用 `Release`）。Visual
Studio 是多配置生成器，而发布包是单配置的，配置阶段会打印一条
`当前是多配置生成器…` 告警，这是预期行为：构建时用对 `--config` 即可。x86 包需要
加 `-A Win32`，arm64 包加 `-A ARM64`（x64 不用加）。

使用源码树里的 `build/sdk_test`（不在 SDK 根目录内）时要显式给出 SDK 根目录：

Visual Studio 生成器：

```bash
cmake -S build/sdk_test -B <临时目录>/sdk_consumer -G "Visual Studio 17 2022" -A x64 -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根目录> -DSDK_EXPECT_ARCH=x64 -DSDK_EXPECT_CONFIG=Debug
cmake --build <临时目录>/sdk_consumer --config Debug --parallel 4
<临时目录>/sdk_consumer/Debug/sdk_consumer.exe
```

Ninja / 单配置生成器：

```bash
cmake -S build/sdk_test -B <临时目录>/sdk_consumer_ninja -G Ninja -DCMAKE_BUILD_TYPE=Debug -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根目录> -DSDK_EXPECT_ARCH=x64 -DSDK_EXPECT_CONFIG=Debug
cmake --build <临时目录>/sdk_consumer_ninja --parallel 4
<临时目录>/sdk_consumer_ninja/sdk_consumer.exe
```

Linux/macOS 上去掉 `.exe`；`NIM_DUILIB_SDK_DIR` 用实际解包路径。`SDK_EXPECT_*` 都可以
省略，省略时不校验对应项。

macOS 上专门跑 App Bundle 用例（构建 `sdk_bundle.app`，再按 C11 的方式检查）：

```bash
cmake -S build/sdk_test -B <临时目录>/sdk_bundle -DNIM_DUILIB_SDK_DIR=<解包后的 SDK 根目录>
cmake --build <临时目录>/sdk_bundle --target nim_duilib_sdk_bundle --parallel 4

# 产物：<临时目录>/sdk_bundle/sdk_bundle.app
find <临时目录>/sdk_bundle/sdk_bundle.app/Contents -maxdepth 5 \
     \( -name 'libSDL3*.dylib' -o -name global.xml \)
otool -L <临时目录>/sdk_bundle/sdk_bundle.app/Contents/MacOS/sdk_bundle | grep SDL3
otool -l <临时目录>/sdk_bundle/sdk_bundle.app/Contents/MacOS/sdk_bundle | grep -A2 LC_RPATH
cp -R <临时目录>/sdk_bundle/sdk_bundle.app /tmp/ && /tmp/sdk_bundle.app/Contents/MacOS/sdk_bundle
```

### 5.2 `SDK_EXPECT_*` 变量

| 变量 | 校验的 CMake 元数据 | 取值示例 |
| :--- | :--- | :--- |
| `SDK_EXPECT_VERSION` | `NIM_DUILIB_PACKAGE_VERSION` | `1.0.2026.1023` |
| `SDK_EXPECT_SYSTEM` | `NIM_DUILIB_PACKAGE_SYSTEM` | `win`、`linux`、`macos`、`freebsd` |
| `SDK_EXPECT_ARCH` | `NIM_DUILIB_PACKAGE_ARCH` | `x64`、`x86`、`arm64` |
| `SDK_EXPECT_CONFIG` | `NIM_DUILIB_PACKAGE_CONFIG` | `Debug`、`Release` |
| `SDK_EXPECT_TOOLCHAIN` | `NIM_DUILIB_PACKAGE_TOOLCHAIN` | `msvc`、`mingw`、空 |
| `SDK_EXPECT_IS_DLL` | `NIM_DUILIB_PACKAGE_IS_DLL` | `1`、`0` |
| `SDK_EXPECT_HAS_SDL` | `NIM_DUILIB_PACKAGE_HAS_SDL` | `1`、`0` |
| `SDK_EXPECT_HAS_CEF` | `NIM_DUILIB_PACKAGE_HAS_CEF` | `1`、`0` |
| `SDK_EXPECT_HAS_WEBVIEW2` | `NIM_DUILIB_PACKAGE_HAS_WEBVIEW2` | `1`、`0` |
| `SDK_EXPECT_HAS_JPEG` | `NIM_DUILIB_PACKAGE_HAS_JPEG` | `1`、`0` |
| `SDK_EXPECT_MSVC_RUNTIME` | `NIM_DUILIB_PACKAGE_MSVC_RUNTIME` | `MT`、`MD`、空 |

规则：

- 变量未定义或为空字符串时不做校验；
- 定义了就按字符串精确比较，不一致时配置阶段直接 `FATAL_ERROR`，
  消息形如 `SDK 测试失败：<变量> 期望 '<期望值>'，实际 '<实际值>'`；
- 它比较的是 **CMake 元数据**，不是包内实际文件；三者一致性由 P04 负责。

### 5.3 输出与退出码

正常运行的 stdout：

```text
SDK_TEST_BEGIN
SDK_TEST_FEATURES sdl=0 cef=1 webview2=0 jpeg=1 dll=0
SDK_TEST_RESOURCE_ROOT=<可执行文件同级的 resources 路径>
SDK_TEST_RESULT=ok
```

| 输出/代码 | 含义 |
| :--- | :--- |
| `SDK_TEST_BEGIN` | 程序开始标记 |
| `SDK_TEST_FEATURES ...` | 由 SDK 头文件宏 `DUILIB_SDL`、`DUILIB_CEF`、`DUILIB_WEBVIEW2`、`DUILIB_JPEG_TURBO`、`DUILIB_DLL` 计算出的运行期特性 |
| `SDK_TEST_RESOURCE_ROOT=...` | 资源根目录，便于排查部署问题 |
| `SDK_TEST_RESULT=ok` | 启动与关闭成功 |
| `SDK_TEST_RESULT=failed: ...` | 资源目录不存在、`Startup` 返回 false、抛异常等 |
| 返回码 0 | 成功 |
| 返回码 3 | 资源目录不存在 |
| 返回码 4 | `Startup` 返回 false |
| 返回码 5 | 捕获到 `std::exception` |
| 返回码 6 | 捕获到未知异常 |

`sdk_bundle.app`（C11）的 stdout 在同样格式上多两行：

```text
SDK_TEST_BUNDLE=1
SDK_TEST_SDL_VERSION=30201
```

`SDK_TEST_SDL_VERSION` 是 `SDL_GetVersion()` 从 `Contents/Frameworks` 的动态库读到的
版本号；能打印它说明 `@rpath` 已生效。C11 的判定见第 4.2 节，另外还要求
`SDK_TEST_RESOURCE_ROOT` 位于 `Contents/Resources/duilib/resources`。

编排器判定 C01/C02 成功需要同时满足：进程返回码为 0、输出含
`SDK_TEST_RESULT=ok`、`SDK_TEST_FEATURES` 的 `sdl`/`cef`/`webview2`/`jpeg`/`dll`
全部与包能力推断一致。任一不满足都判 FAIL。

`nim_duilib_copy_runtime(<target> RESOURCES)` 会把 SDK 的 `bin/*.dll` 和
`bin/resources/` 复制到可执行文件同级；目标是 macOS `MACOSX_BUNDLE` 且未指定
`DESTINATION` 时，资源改复制到 `<target>.app/Contents/Resources/duilib/resources/`。
手工运行时若跳过这一步，资源目录检查会失败。

## 6. 结果解读与 report.json

### 6.1 PASS / FAIL / SKIP

| 状态 | 产生方式 | 含义 |
| :--- | :--- | :--- |
| PASS | check 函数正常返回说明字符串 | 该场景通过 |
| FAIL | 抛 `AssertionError`，或抛其他未预期异常（记录前 3 层 traceback） | 包或环境不满足判定条件；不会中断同包其他场景，也不会中断后续包 |
| FAIL（P00） | 归档无法加载 | 该包没有进入场景执行，只记录一条 `P00/package.load` |
| SKIP | 场景适用性判断为否，或 check 抛 `ScenarioSkipped` | 当前环境/包类型不适用，不计入失败 |

SKIP 是“没测到”，不是通过。`--no-build`、`--dry-run`、包类型不匹配、编译器缺失、
生成器限制等都会产生 SKIP。报告会保留具体原因，便于判断是真不适用还是环境没配好。

### 6.2 退出码

| 退出码 | 条件 |
| :--- | :--- |
| 0 | 没有 FAIL；允许存在 SKIP（但至少有一个 PASS） |
| 1 | 存在任意 FAIL（包括 P00 加载失败） |
| 2 | 没有 FAIL，且 PASS 数为 0（全部 SKIP） |

`--list-scenarios` 直接返回 0。参数解析错误（缺少 `--package`、包不存在、未知场景、
`--scenarios` 只含分隔符）由 `argparse`/`SystemExit` 以非零码退出，不属于上面的测试
结果码。

### 6.3 输出目录与日志

默认输出根目录（所有平台一致）：

- `<系统临时目录>/nim_duilib_sdk_test/<时间戳>`（Windows 的临时目录通常为
  `%TEMP%`，Linux/macOS 通常为 `/tmp`）；
- 时间戳格式为 `%Y%m%d-%H%M%S`；
- 显式指定 `--out-dir` 时使用该目录，不再追加时间戳。

每个包一个子目录，命名为两位序号（如 `01`），不再拼接包名：

```text
<系统临时目录>/nim_duilib_sdk_test/20261026-153000/
├─ report.json
├─ 01/
│  ├─ extract/...                  # 包内容（已去掉归档顶层包目录）
│  ├─ work/consumer/...            # 消费者构建树
│  ├─ work/consumer_bundle/...     # macOS App Bundle 构建树（仅 C11）
│  ├─ work/bundle-relocated/...    # 迁移后的 .app 副本（仅 C11，用于运行）
│  ├─ work/tmp/...                 # 编译/运行的 TMPDIR（见下方说明）
│  ├─ work/repack.tgz / repack.tar.bz2 / bad-*.tgz / tampered.tgz
│  └─ logs/consumer.log, bundle.log, neg-config.log, neg-arch.log, neg-runtime.log, neg-toolchain.log
└─ 02/                            # 下一个包
```

`work/tmp/` 是通过 `TMPDIR`/`TEMP`/`TMP` 交给编译器与示例程序的临时目录。Linux 上
`/tmp` 常常是小容量 tmpfs，写满时 gcc 会报 `error closing ...s: 设备上没有空间`，
进而被误判成「C++ 编译器不可用」（C01）或「配置失败原因不含 …」（C03）；把临时目录
放到 `--out-dir` 下即可避免这类环境性失败。

路径与 Windows 长路径：

- 归档顶层包目录只用于 P01/P04 校验（`root_name`），解包时其内容直接落到
  `extract/`；否则“输出目录 + 包名 + 包名 + CEF 深头文件”很容易超过 260 字符，
  Windows 未启用长路径支持时 `shutil.unpack_archive` 会直接抛
  `FileNotFoundError`。
- 子目录只用 `01`、`02` 这样的短名，也是为了给 `work/CMakeFiles/CMakeScratch`
  与 MSBuild 的 `.tlog` 路径留出余量（C04 的 VS 生成器最容易触发
  `MSB6003 DirectoryNotFoundException`）。
- 需要保留解包结果排查时加 `--keep`；默认运行结束只删 `extract/`、`work/`
  与包子目录，`report.json` 保留。

日志规则：

- C01/C02 写 `consumer.log`（配置、编译、运行输出追加在同一文件）；
- C11 写 `bundle.log`（配置、编译、`otool` 输出与迁移后运行追加在同一文件）；
- C03 写 `neg-config.log`，C04 写 `neg-arch.log`，C05 写 `neg-runtime.log`，
  C06 写 `neg-toolchain.log`；
- P 系列和 C07–C09 不单独写日志，失败原因在 `detail` 和终端输出中；
- FAIL 结果里的 `log` 字段指向该包日志目录中当时最新的 `*.log`，没有日志时为空串；
- 不加 `--keep` 且不是 `--dry-run` 时，`extract/`、`work/` 以及整个包目录都会在结束后
  删除，只保留 `report.json`。要保留日志必须加 `--keep`。

终端会逐条打印：

```text
[PASS] P01  package.extract           解包成功：...
[FAIL] C01  consumer.static           配置失败（rc=1）：...
       日志：<输出目录>/.../logs/consumer.log
```

最后打印 `结果：PASS=x FAIL=y SKIP=z` 与报告路径。

每个场景开始时还会打印一行 `>> C07 ...`，结束后立即打印该场景结果（`flush`），
因此把输出重定向到日志文件时也能实时看到进度。C07/C08/C09 只生成代表性小归档，
耗时与包体积无关（Debug CEF 包也不再出现长时间 100% CPU）。

### 6.4 report.json 结构

报告使用 UTF-8、两空格缩进写入：

```json
{
  "generatedAt": "2026-10-26 15:30:00",
  "host": {
    "system": "win",
    "arch": "x64",
    "python": "3.x.x ..."
  },
  "packages": [
    {
      "archive": "C:\\path\\to\\nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip",
      "package": "nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip",
      "metadata": {
        "version": "1.0.2026.1023",
        "system": "win",
        "arch": "x64",
        "config": "Debug",
        "toolchain": "msvc",
        "is_dll": false,
        "has_sdl": false,
        "has_cef": true,
        "has_examples": true,
        "msvc_runtime": "MT"
      },
      "results": [
        {
          "id": "P01",
          "name": "package.extract",
          "status": "PASS",
          "detail": "解包成功：1316 个文件，包根 nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug",
          "log": ""
        }
      ]
    }
  ]
}
```

| 字段 | 说明 |
| :--- | :--- |
| `generatedAt` | 报告生成时间，格式 `%Y-%m-%d %H:%M:%S` |
| `host.system` / `host.arch` | 主机平台与架构（来自 `host_system()`/`host_arch()`） |
| `host.python` | 当前解释器的 `sys.version` 完整字符串 |
| `packages[]` | 每个待测包一个条目，顺序与 `--package` 一致 |
| `archive` / `package` | 包的绝对路径与文件名 |
| `metadata` | 从包元数据与文件推断出的摘要；字段可能为空 |
| `results[]` | 每个已执行/被跳过的场景一条记录 |
| `id` / `name` | 场景编号与注册名 |
| `status` | `PASS`、`FAIL` 或 `SKIP` |
| `detail` | 通过说明、失败原因或 SKIP 原因 |
| `log` | 编译类场景的日志路径；没有日志时为空串 |

加载失败的包条目没有 `metadata`，`results` 只有一条
`{"id": "P00", "name": "package.load", "status": "FAIL", ...}`。

## 7. 常见失败排查

| 现象 / 场景 | 可能原因 | 排查方向 |
| :--- | :--- | :--- |
| P00 无法加载发布包 | 归档损坏、格式不支持、顶层目录不唯一、空包 | 先 `python3 build/pack.py --verify <包>`；确认归档只有一个顶层目录；必要时重新出包 |
| P02 缺少必需文件或主库 | 产物不全、库文件名不符合预期、包被手工裁剪 | 对照 P02 的必需文件清单；静态包检查 `duilib-cximage`/`webp`/`png`/`zlib`/`skia`；重新构建并出包 |
| P03 `SHA256` 不匹配、未登记或多登记 | 出包后归档内容被改动、`SHA256SUMS` 与文件不同步、手工增删成员 | 用干净的 `lib/`、`bin/` 重新出包；不要直接编辑归档或 `SHA256SUMS`；P07 会再次复核 |
| P04 元数据不一致 | 包名特性与产物不符（如 `-dll` 但只有静态库、`-debug` 但产物是 Release、`-sdl` 但无 SDL、未声明 CEF 却带 CEF 头） | 按实际产物选择 `pack.py` 参数；清理 `bin/`、`lib/<arch>/<Config>` 中的旧工具链/旧架构残留后重出包 |
| P05 接入文件不完整 | CMake 模板占位符未替换、缺 `PACKAGE_VERSION`、缺 manifest 或 MSVC 属性表、manifest XML 非法 | 检查 `lib/cmake/nim_duilib/` 与 `msvc/` 内容；用当前 `pack.py` 重新生成，避免手工改包 |
| P06 出现禁止内容 | `lib/` 或 `bin/` 混入 `.pdb`/`.obj`、旧 `cef_sandbox.lib`、`include/` 下有 `.cpp`、未声明 CEF/SDL 却带相应头文件 | 清理解包前的构建产物；确认打包开关与实际产物一致；发布不要用 `--allow-missing` 掩盖缺失 |
| P07 `pack.py --verify` 失败 | 包本身有问题，或旧包的文档相对链接失效、库/架构混合 | 手动执行同一命令看 `[ERROR]` 行；`test-matrix.md` 提到旧 `dist/` DLL 包存在历史链接失效，属于旧产物 |
| P02/P05/P08 同时失败（缺 `THIRD-PARTY-NOTICES.md`、`LICENSES/` 为空、缺某份架构 manifest） | 包由旧版 `pack.py` 生成：旧包用 `licenses/`（小写）、无第三方声明、`msvc/manifest/` 只有部分架构 | 用当前 `pack.py` 重新出包后再测；早期 `dist/` 中的 DLL 包即属于这种情况，`C02` 仍可通过，但结构类场景应按新包复核 |
| C01/C02 配置失败 | CMake 找不到 `nim_duilib`、包里架构/配置/工具链与当前生成器不符、`SDK_EXPECT_*` 与元数据不符、`NIM_DUILIB_SDK_DIR` 指错 | 看 `consumer.log` 的 CMake 错误；确认指向解包后的 SDK 根目录；使用匹配的 VS 架构、配置或 Ninja |
| C01/C02 编译失败 | 头文件未随包、CEF/WebView2 头未转发、运行库/架构不匹配、链接库缺失 | 看 `consumer.log` 的首个编译/链接错误；`HAS_CEF=1` 时检查 `header_cef.cpp`；重新出包，不要手工拼 include/lib 路径 |
| C01/C02 未找到 `sdk_consumer` | 编译实际失败、输出目录被清理、生成器输出名变化 | 检查构建日志与 `work/consumer` 下的输出；确认目标 `OUTPUT_NAME` 未被改动 |
| C01/C02 资源未复制到可执行文件同级 | `nim_duilib_copy_runtime` 未执行、包内 `global.xml` 缺失 | 检查 `CMakeLists.txt` 的复制调用；确认包内 `bin/resources/themes/default/global.xml` 存在 |
| C02 DLL 未复制到可执行文件同级 | 包内缺少顶层 DLL、复制步骤失败 | 检查 `bin/` 顶层 DLL 与 `info.caps["bin_dlls"]`；查看构建后复制日志 |
| C01/C02 运行失败（rc=3/4/5/6） | 资源目录不存在、`Startup` 返回 false、异常、CEF/SDL/CRT 运行库缺失 | 手工运行 `sdk_consumer` 看完整输出；确认资源、DLL 与 CRT 已部署；CEF 运行时目录需自行复制 |
| C01/C02 特性不一致 | SDK 头文件宏（`DUILIB_SDL` 等）与实际包能力不符 | 对照 P04；检查 `nim_duilibPackageInfo.cmake` 与 CMake 目标宏；重新构建/出包 |
| C11 `Contents/Frameworks` 没有 dylib | `nim_duilib_copy_runtime` 未加 `FRAMEWORKS`、包内 `lib/libSDL3*.dylib` 缺失、或 `NIM_DUILIB_SDL_DYLIBS` 未登记 | 检查 `CMakeLists.txt` 的调用与 `bundle.log`；确认 P04 的 `HAS_SDL` 与实际文件；macOS 包应由当前 `nim_duilibConfig.cmake` 生成 |
| C11 `otool -L` 仍是绝对路径 | SDL3 的 `install_name` 不是 `@rpath`（例如未启用 `MACOSX_RPATH`） | 用 `otool -D lib/libSDL3*.dylib` 检查；重新构建依赖，或 `install_name_tool -id @rpath/<文件名>` 后重新出包 |
| C11 缺少 `LC_RPATH` | 目标不是 `MACOSX_BUNDLE`，或 `nim_duilib_copy_runtime` 未执行到设置 rpath 的分支（无动态 SDL） | 检查 `add_executable(... MACOSX_BUNDLE)` 与包内 SDL 形态；用 `otool -l` 查看实际 rpath |
| C11 迁移后运行失败（`Library not loaded`） | `Contents/Frameworks` 缺少 dylib、符号链接链缺失、或签名被破坏 | 不解包直接运行构建树里的 `.app` 对比；确认 `libSDL3.dylib -> libSDL3.<abi>.dylib` 符号链接；归档解包需保留符号链接 |
| C11 资源目录检查失败 | `RESOURCES` 未生效，或目标不是 `MACOSX_BUNDLE`（资源落到了可执行文件同级的 `resources`，而不是 `Contents/Resources/duilib/resources`） | 检查 `bundle.log`；确认调用为 `nim_duilib_copy_runtime(<bundle> RESOURCES FRAMEWORKS)`，且未显式传 `DESTINATION` |
| C03 配置没有被拒绝 | 生成器是多配置的 Visual Studio（本应 SKIP）；或 SDK 未校验配置 | 先用 Ninja/Makefiles 生成器复核；若仍通过，检查 SDK 的配置校验逻辑与错误文案“配置为” |
| C04 架构没有被拒绝 | 本机没有相反架构工具集（本应 SKIP）；或 SDK 未校验架构 | Windows 安装 VS 的 x86/x64 工具集后重试；检查错误文案“架构为” |
| C05 运行库没有被拒绝 | 包不是 Windows MSVC，或未声明 `MT`/`MD`（本应 SKIP）；DLL 包未使用 `/MD` | 检查 P04 是否已报运行库问题；确认 DLL 包元数据为 `MD`，静态包默认 `MT` |
| C06 SKIP | MSVC 包缺 `g++` 或 Ninja；MinGW 包缺 VS/`cl.exe`；非 Windows 包 | 这是环境不满足，不是包缺陷；按需安装对立工具链，或接受 SKIP |
| C06 FAIL | 配置未被拒绝，或失败原因不是“该 nim_duilib SDK 是” | 检查 SDK 是否错误接受了异种工具链；查看 `neg-toolchain.log` 中的实际错误文本 |
| C07/C08 `--verify` 接受了坏包 | `pack.py --verify` 回归，或小归档生成时跳文件/篡改未生效 | 手工检查 `work/bad-*.tgz`、`work/tampered.tgz`；确认 `pack.py` 是当前版本 |
| C09 小归档校验失败 | 生成的 tgz/tar.bz2 内容或 `SHA256SUMS` 有问题 | 查看失败输出；先跑 P07 排除原包问题，再用 `--keep` 保留 `work/repack.*` 检查 |
| 所有场景 SKIP、退出码 2 | 只选了编译类场景但包类型/环境不适用；`--no-build` 后只剩不适用场景；工具链/CMake 缺失 | 看每个 SKIP 的 `detail`；补跑 P 系列确认包本身；安装缺失工具或改用适用包 |
| C01–C06、C11 全部 SKIP“找不到 cmake” | `cmake` 不在 `PATH` | 把 CMake 加入 `PATH`；包级场景不受影响 |
| 磁盘空间不足 | 解包 + 消费者构建 | 换用空间充足的 `--out-dir`；清理旧的运行目录；排查时先用 `--scenarios P01,P02,P03` 缩小范围 |
| C01/C03 报“C++ 编译器不可用”“配置失败原因不含 …”，日志里是 `error closing ...s: 设备上没有空间` | 系统临时目录（Linux 常为 1–2 GiB 的 `/tmp` tmpfs）被写满，gcc 的临时汇编文件写不进去；与包和工具链无关 | 现在的版本已把 `TMPDIR`/`TEMP`/`TMP` 指到 `--out-dir/<NN>/work/tmp`，不会再落到系统 `/tmp`；若仍有告警，用 `--out-dir` 指到剩余空间更大的磁盘（启动时的 `[WARN]` 会给出估算值） |

## 8. 扩展新场景的方法

1. **登记场景**：在 `sdk_package_test.py` 的 `SCENARIO_ORDER` 中追加
   `(编号, 注册名, 说明, 是否需要编译)`。`SCENARIO_NAMES` 与 `BUILD_SCENARIOS` 会自动
   派生，`--list-scenarios`、`--scenarios` 也会自动识别。编号保持稳定，不要复用旧编号。
2. **判断适用性**：
   - 包级场景不需要改 `scenario_applicability()`，默认始终适用；
   - 编译类场景在该函数中增加分支，基于 `caps`、`meta`、`build_block_reason` 返回
     `(False, "原因")`；`--no-build` 和 `--dry-run` 已在调度层统一处理；
   - 平台/工具链的前置判断放在 `build_block_reason()`，让 C 类场景共享同一套规则。
3. **实现检查函数**：新增 `check_xxx(info, work_dir, log_dir, options)`（包级可只收
   `info`，需要重打包时加 `work_dir`），并在 `_scenario_runner()` 中按编号分发。
   契约如下：

   | 行为 | 结果 |
   | :--- | :--- |
   | 正常返回说明字符串 | PASS，字符串写入 `detail` |
   | 抛 `ScenarioSkipped("原因")` | SKIP，原因写入 `detail` |
   | 抛 `AssertionError("原因")` | FAIL，原因写入 `detail`，并附最新日志路径 |
   | 抛其他异常 | FAIL，记录异常类型、消息和前 3 层 traceback |

   约定返回非空中文说明，便于报告直接阅读；不要在 check 内捕获所有异常后返回成功。
4. **记录日志**：外部命令统一走 `run_command()`（传入 `cwd`、`env`、`log_path`、
   `dry_run`、`timeout`）或 `_log_command()`，日志命名保持
   `neg-*.log`/`consumer.log` 风格。
5. **补单元测试**：在 `test_sdk_package.py` 中为新增的纯函数、适用性判断、场景选择补
   `unittest` 用例。单测不应依赖真实发布包；用构造的 `caps`/`meta` 覆盖分支即可。
6. **验证**：

   ```bash
   python3 build/test_sdk_package.py -v
   python3 build/build.py test          # 会 discover build/test_*.py
   python3 build/sdk_package_test.py --list-scenarios
   ```

   改动 CLI 或场景语义时，同步更新本文的场景表和 README 的文件概览。

## 9. 按包类型的覆盖矩阵

P01–P08、C07–C09 对任何可加载的包都会执行（其中 P02/P04/P05 的判定内容随库形态、
功能开关、配置、架构变化）。C01/C02 按 `IS_DLL` 二选一；其余编译类场景由环境和
生成器决定 PASS/FAIL/SKIP。

### 9.1 维度对场景的影响

| 维度 | 取值 | 主要影响 |
| :--- | :--- | :--- |
| 归档格式 | zip / tgz / tar.bz2 | P07 校验原始归档；C09 额外验证由代表性小文件生成的 tgz/tar.bz2 |
| 库形态 | 静态 / DLL | P02 的库清单、P04 的 `IS_DLL` 与运行库规则；C01 与 C02 互斥 |
| CEF | 无 / CEF 142 / CEF 109 | P04 的 `HAS_CEF`、P06 的反向检查；消费者编译 `header_cef.cpp`，`FEATURES cef=1` |
| SDL | 无 / 静态 / 动态 | P04 的 `HAS_SDL`、P06 的反向检查；消费者链接 SDL，`FEATURES sdl=1` |
| 配置 | Debug / Release | P04 的配置一致性；C03 的相反配置；消费者 `CMAKE_BUILD_TYPE` 或 `--config` |
| 工具链 | MSVC / MinGW gcc / MinGW llvm / Unix | P04 的工具链与 MinGW 变体；P05 的 MSVC 接入文件；C06 的对立面；消费者编译环境 |
| 架构 | x64 / x86 / arm64 | P04/P05 的架构；C04 的相反架构；消费者 `-A` 与可用工具集 |
| 示例 | 无 / 有 | P04 的示例标志；C10 的 `--run-examples` 门禁与 `bin/basic` |
| MSVC 运行库 | MT / MD | P04 的合法性（DLL 必须 `MD`）；C05 的相反运行库 |

### 9.2 代表组合

| 代表组合 | 包级场景（P01–P08、C07–C09） | 消费者/负向场景 | 真实包与环境要求 |
| :--- | :--- | :--- | :--- |
| Windows MSVC 静态，无 CEF，无 SDL，Release，x64 | 全部执行 | C01 PASS；C02 SKIP；C03/C04/C05/C06 视生成器与工具而定，通常 C01/C03/C05 可跑，C04 需 VS 的相反架构工具集，C06 需 `g++`+Ninja；C10 可选 | 真实 MSVC 静态包；VS 2022+；Ninja 优先 |
| Windows MSVC 静态，CEF + SDL，Debug，x64 | 全部执行 | C01 PASS（编译 `header_cef.cpp`，`FEATURES cef=1 sdl=1`）；C02 SKIP；C03/C04/C06 视生成器与工具而定；C05 用相反运行库；C10 指定 `--run-examples` 后可跑 | 真实 CEF/SDL 静态包；CEF 运行时部署不由 C01 覆盖 |
| Windows MSVC DLL，CEF，Release，x64 | 全部执行（P02 只要求 `duilib` 主库） | C02 PASS（检查顶层 DLL 复制）；C01 SKIP；C05 用相反运行库；CEF 头文件参与编译 | 真实 DLL 产物：`build/build.py build --dll`（或 `duilib_dll.flag` + MSBuild）生成 `bin/duilib.dll` 与 `lib/x64/Release/duilib.lib` |
| Windows MSVC 静态，无 CEF，Debug，x86 | 全部执行 | C01 需 x86 编译器；C04 用 x64 作相反架构；C05/C06 视工具而定；C10 可选 | 真实 x86 包 + x86 工具集 |
| Windows MinGW gcc/llvm 静态，无 CEF，Release，x64 | 全部执行 | C01 需 MSYS2 + `MSYSTEM`（llvm 变体还需 `CC`/`CXX` 指向 clang）；C03 用 Ninja；C04 通常 SKIP；C05/C06 SKIP；C10 可选 | 真实 MinGW 包；MSYS2/MinGW-w64 环境；CEF 不支持 |
| Linux gcc/clang 静态，SDL，Release，x64 | 全部执行 | C01 PASS；C03 用 Ninja/Makefiles；C04 SKIP（包架构=主机）；C05/C06 SKIP；C10 需桌面会话且包含示例 | 真实 Linux 包；X11/字体等运行依赖 |
| macOS 静态，SDL（动态 dylib），Release，x64，含示例 | 全部执行 | C01 PASS；C11 PASS（`sdk_bundle.app` 的 Frameworks/rpath/资源布局）；C03 PASS；C04 SKIP；C05/C06 SKIP；C10 需 `bin/basic.app` 与桌面会话 | 真实 macOS 包（`.app` 示例）；macOS 本机 + `otool` |
| Windows/Linux/macOS arm64 任意静态包 | 全部执行 | C01 需要 arm64 编译器或可用交叉/仿真环境，否则 SKIP；C04 在 Windows 需要 arm64 工具集或 x64 生成器才能构造相反架构，否则 SKIP；C05/C06 视 Windows 环境；C10 需能运行 arm64 程序 | 真实 arm64 包；Windows 交叉工具已就绪（`Hostx64\arm64`），运行验证仍需 arm64 设备；Linux/macOS 不安排 |
| 非 Windows 或 MinGW 的 DLL 包 | 结构上可执行包级场景 | C02 需要平台一致且工具链支持；C05/C06 不适用 | 当前构建不产出该组合，属于不适用/需外部产物 |

### 9.3 必须用真实包或真实环境才能验证的部分

- **C01/C02/C10/C11**：需要真实库文件、真实可执行文件和匹配的编译器/运行环境。只含
  元数据和目录结构的“合成包”可以通过 P 系列，但无法完成消费者编译运行；C11 还需要
  macOS 本机、真实 SDL3 dylib 与 `otool`。
- **CEF/WebView2 运行期**：消费者只编译头文件并核对宏，不启动浏览器、不复制
  `bin/libcef_win*/` 等 CEF 运行时目录。CEF 运行期部署需要真实包配合示例（C10）或
  手工验证。
- **DLL 模式**：用 `build/build.py build --dll`（或 VS 工程的
  `msvc/PropertySheets/duilib_dll.flag`）生成真实 DLL 产物，再运行 C02 与 P04
  的 `MD` 规则；纯合成包只能验证结构。DLL 包需要 `bin/duilib*.dll`；若 duilib
  DLL 引用 `turbojpeg.dll`（MSBuild 默认启用 libjpeg-turbo），还需要
  `bin/turbojpeg.dll`，`pack.py` 会自动收集；CMake 构建当前未启用 JPEG，不
  产生该依赖。duilib DLL 以 SDL 编译时打包必须加 `--with-sdl`。
- **arm64**：C01 需要 arm64 工具链，C10/C11 需要能运行 arm64 程序的环境。Windows 上
  VS 2022 的 ARM64 交叉工具（`Hostx64\arm64`）可完成 C01 的配置/编译/链接；x64 主机
  无法运行 ARM64 产物（`WinError 216`），因此 `check_consumer()` 在链接后按
  `_can_run_arch()` 判为不可运行，C01 记为 SKIP 并注明「运行验证需 arm64 环境」，
  不再误判为 FAIL。C10 同样会 SKIP；Linux/macOS arm64 无真机时不安排（见
  `build/arm64-test-plan.md`）。C04 在 Windows 用 `-A x64` 即可构造相反架构。
  2026-10-28 实测：arm64 的 Release 无 CEF、Release CEF142、Debug CEF142 三个包均为
  `PASS=14 FAIL=0 SKIP=5`。
- **Linux/macOS 运行**：C10 需要桌面会话、X11/字体或 macOS 窗口环境；纯 headless
  环境只能验证包级场景。
- **MinGW llvm 变体**：需要 MSYS2 环境且 `CC`/`CXX` 指向 clang，否则 C01/C02 会 SKIP。
- **MT/MD 两种运行库**：C05 要求包元数据声明 `MT`/`MD`，并需要对应的真实库与 MSVC
  环境；否则 SKIP。
- **工具链对立面**：C06 需要本机同时具备 `g++`+Ninja 或 VS/`cl.exe`。只装一种工具链时
  C06 会是 SKIP，这不代表 SDK 的工具链校验有问题。

包级场景的 SKIP 极少，P01–P08、C07–C09 的 FAIL 通常直接指向包本身；编译类场景的
SKIP 多数是环境限制，应结合 `detail` 区分“未测到”和“测试失败”。
