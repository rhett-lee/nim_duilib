# 发布打包待办事项（Follow-up）

本文件记录发布打包流程已经完成的部分、尚未完成的工作及验收标准，供后续逐项跟进。

> 本文件位于 `docs/misc/`，不随发布包分发。

## 一、当前基线

已完成并验证的内容（Windows：MSVC x64/x86 Debug、x64/x86 Release 静态包、MSVC x64/x86 DLL（`/MD`）Debug/Release 包、arm64 Release/Debug 静态与 DLL、arm64 CEF142/CEF109 包；MinGW x64 Release；MSYS2 UCRT64 gcc / CLANG64 llvm / MINGW64 gcc(msvcrt) x64 Release；MSVC x64 `--with-sdl`、`--with-examples` 与 MSVC / MinGW 示例组合；Linux x64 Release/Debug：编译、出包与包测试；macOS x64 Release/Debug：构建、出包、包测试与 Bundle 运行；FreeBSD x64 Release/Debug：构建、出包、包测试与示例启动（不支持 CEF）。Linux/macOS 于 2026-10-28、FreeBSD 于 2026-10-29 真机复测，Windows arm64 于 2026-10-28/29 完成）：

| 领域 | 状态 |
| :--- | :--- |
| 产物收集 | 头文件、库、Skia、SDL、CEF、WebView2、JPEG、运行期 DLL、资源、许可证、文档 |
| 包内布局 | `include/`、`lib/`（含 `lib/cmake/`）、`bin/`（含 `bin/resources/`）、`docs/`、`LICENSES/`、`msvc/` |
| 元数据与校验 | `MANIFEST.txt`（含文档清单）、`SHA256SUMS`、`VERSION` |
| 下游接入 | VS props/targets、CMake Config/Helper、x64 + x86 + arm64 manifest（三份均随包） |
| 文档处理 | 只打顶层使用文档，源码链接按包布局重写，无断链 |
| 已验证 | `--list`、XML 解析、CMake 配置、SHA/断链检查、真实 zip 抽查；x64/x86 Debug 与 Release 出包、解包及 SHA 校验；MinGW（gcc / llvm）5 库 + 21 示例构建与运行；VS/CMake 端到端（x64/x86 Debug、x64/x86 Release、MinGW x64 gcc/llvm 包、MSVC DLL 包）；MSYS2 真实环境（UCRT64 gcc、CLANG64 llvm、MINGW64 gcc/msvcrt）全量构建、出包、解包与 CMake 端到端；MSVC DLL 模式（`/MD`）构建、出包与端到端；MSVC x64/x86 与 MinGW x64 的 SDL/示例组合出包、`--verify` 与解包运行；Windows arm64 Release/Debug（静态、DLL）与 CEF142/109 出包、`--verify`、SDK 包测试；Linux x64 Release/Debug、macOS x64 Release/Debug 与 FreeBSD x64 Release/Debug 的构建、出包、包测试与示例运行（Linux/macOS 2026-10-28、FreeBSD 2026-10-29 真机复测；FreeBSD 不含 CEF） |

尚未覆盖：Windows on ARM 真机运行（无设备，未测、不跟进）；Windows x86 SDL
（缺 x86 SDL3 库）；MSYS2 MINGW32 x86 与 libpag 的 ARM64 支持（未测、不跟进）。

### 平台与特性约束

| 平台 | 架构 | CEF | 说明 |
| :--- | :--- | :--- | :--- |
| Windows | x64 / x86 / arm64 | 支持 | CEF 二进制按版本与架构位于 `bin/libcef_win/{x64,Win32,arm64}/`、`bin/libcef_win_109/{x64,Win32,arm64}/`；arm64 Release/Debug（静态与 DLL）与 CEF142/109 出包已实测（2026-10-28/29），真机运行无设备、未测不跟进 |
| Linux | 仅 x64（不支持 x86） | 支持 | CEF 运行时位于 `bin/libcef_linux/`，与 Windows 的分架构结构不同；Release/Debug 编译、出包与包测试（含示例运行）已实测（2026-10-25 / 2026-10-28） |
| macOS | x64 | 以 `.app` 内嵌 | CEF 随示例生成 `cef.app`、`CefBrowser.app`；`--with-examples` 时由 `--with-cef` 控制是否随包（未指定则跳过并告警），随包时整目录（含 Framework/Helper）收集；CEF 运行时目录不单独收集；x64 Release/Debug 构建、出包、包测试与 Bundle 运行已实测（2026-10-28），arm64 无真机不安排 |
| FreeBSD | 仅 x64（本次实测） | 不支持 | 目录结构沿用 Linux 分支，包内不包含 CEF；Release/Debug 的构建、默认 SDK 包与 `--with-examples` 出包、包测试（含示例启动）已实测（2026-10-29），`--with-cef` 直接报错 |

说明：上表是平台能力与目录约束，包内最终布局以打包脚本为准；CEF 运行时的收集与落包见 P1-3。

## 二、状态说明

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成
- `[-]` 不处理（明确关闭，并记录理由）
- 优先级：P0 发布阻断，P1 发布质量，P2 维护与体验

## 三、待办清单

### P0：发布前必须处理

#### P0-1 打包矩阵实测

- [x] 覆盖“平台 × 配置 × 模式 × 特性”组合，每个组合至少执行一次 `--list` 与解包检查。
- 现状：已验证 Windows x64、x86 两个架构的 Debug 与 Release 静态包
  （2026-10-24 补齐 x86 与 x64 Release）；
  x86 另以 `--with-cef --list` 核对 CEF 运行时按 `bin/libcef_win/Win32/` 收集。
  MinGW x64 Release 已完成 gcc 与 llvm 两套工具链验证：5 库 + 21 示例构建、
  `basic.exe` 运行、llvm 包解包/SHA 校验（2026-10-24）。
  MinGW 包已按编译器区分：包名追加 `-mingw-gcc` / `-mingw-llvm`，MANIFEST
  工具链注明 gcc（libstdc++）/ llvm-mingw（libc++），并自动选择同源 Skia 目录。
  MSYS2 构建/打包链路已支持（2026-10-24）：`msys2_build.sh` 与
  `build_duilib_all_in_one.sh` 可在 MSYS2 运行（失败即停、无需 `bc`、SDL 构建目录隔离）；
  `pack.py` 识别 MSYS2 的 `gcc.<arch>.release` / `llvm.<arch>.release`，按 `.a` 内容
  与同名 MSVC 目录区分；MSYS/Cygwin Python 也能正确判定为 Windows 主机；
  MinGW 包禁止 `--with-cef`。
- MSYS2 真实环境实测（2026-10-24，`E:\msys64`）：
  UCRT64（gcc 14.2.0）与 CLANG64（clang 19.1.7）分别运行
  `build_duilib_all_in_one.sh`：先编译 Skia（`out/gcc.x64.release` /
  `out/llvm.x64.release`），再构建 5 库 + 21 示例，全部成功（22 / 14 分钟），
  21 个示例均可启动；`pack.py` 产出 `-win-x64-mingw-gcc.zip`
  （536 文件/90.6 MB，zip 21.3 MB）与 `-win-x64-mingw-llvm.zip`
  （536 文件/77.0 MB，zip 20.3 MB），解包 539 条目 SHA 全匹配。
- MSYS2 MINGW64（msvcrt，2026-10-24）：`msys2_build.sh` 全量构建成功
  （Skia `out/gcc.msvcrt.x64.release`、5 库 + 21 示例），`basic.exe` 启动正常；
  `pack.py` 产出 `-win-x64-mingw-gcc.zip`（536 文件/90.5 MB），Skia 目录为
  `gcc.msvcrt.x64.release`，`--verify` 与解包 SHA 全匹配。MINGW64 与 UCRT64 的
  gcc 静态库 C 运行库不同（msvcrt / ucrt），Skia 使用独立目录；两者包名相同，
  不可互换，也不能归档到同一目录。
- MSVC DLL 模式（Release|x64，2026-10-24）：创建 `msvc/PropertySheets/duilib_dll.flag`
  后 MSBuild 产出 `bin/duilib.dll`（18.4 MB）与 `lib/x64/Release/duilib.lib`
  导入库（4.3 MB），其余库仍为静态库；duilib 使用 `/MD`，Skia 必须同 CRT，
  `SkiaSettings.props` 检测到 flag 后改用 `skia/out/llvm.x64.release.md`
  （需用相同 GN 参数、把 `extra_cflags` 的 `/MT` 换成 `/MD` 自行构建）。
  `pack.py` 自动收集 `duilib.dll`、按包元数据选择 `.md` Skia，包名为
  `-win-x64-dll`（542 文件/94.9 MB，zip 27.3 MB），`--verify` 545 条目 SHA 全匹配。
- MSVC DLL 模式补齐 Debug/x86（2026-10-24）：构建 Skia `/MDd`、x86 `/MD` 变体
  （`llvm.x64.debug.mdd`、`llvm.x86.release.md`、`llvm.x86.debug.mdd`），
  `SkiaSettings.props` 增加 Debug 的 `.mdd` 路径。MSBuild 产出
  `bin/duilib_d.dll`（24.7 MB）、`bin/duilib32.dll`（14.8 MB）、
  `bin/duilib32_d.dll`（19.9 MB）及对应导入库；`pack.py` 产出
  `-win-x64-dll-debug`（542 文件/100.4 MB，zip 27.8 MB）、
  `-win-x86-dll`（542 文件/83.8 MB，zip 24.3 MB）、
  `-win-x86-dll-debug`（542 文件/86.1 MB，zip 24.5 MB），三个包 `--verify`
  均为 545 条目 SHA 全匹配。DLL 模式依赖 MSVC 动态运行库，x86 Release 在
  系统 `MSVCP140.dll` 过旧（14.29 vs 编译器 14.44）时启动崩溃，随程序附带
  新版运行库后正常；该限制已写入发布文档。
- MSVC SDL 与示例组合（2026-10-24）：在独立构建目录 `build_temp/msvc_sdl_x64`
  以 `-DDUILIB_ENABLE_SDL=ON` 重建 duilib 与 21 个示例（SDL3 为
  `../SDL3/lib/SDL3-static.lib`），随后分别出包：
  `--with-sdl`（`-win-x64-sdl`，621 文件/181.4 MB，zip 38.5 MB，624 条目）、
  `--with-examples`（`-win-x64-examples`，757 文件/562.9 MB，zip 165.6 MB，760 条目）、
  `--with-sdl --with-examples`（`-win-x64-sdl-examples`，837 文件/426.0 MB，
  zip 146.0 MB，840 条目）；三个包 `--verify` 均为 SHA 全匹配，包内
  `lib/SDL3-static.lib`、`include/SDL3/SDL.h` 与
  `NIM_DUILIB_PACKAGE_SDL_KIND "static"` 正确。解包后 `basic.exe`、
  `ChildWindow.exe`（唯一直接包含 SDL 头的示例）、`controls.exe` 均可启动。
  x86 `--with-examples`（`-win-x86-examples`，757 文件/294.7 MB，zip 107.2 MB，
  760 条目）使用 32 位示例，`--verify` 通过、解包后 `basic.exe` 为 x86 且运行正常。
  MinGW（MINGW64 msvcrt）`-examples`（`-win-x64-mingw-gcc-examples`，752 文件/
  739.1 MB，zip 208.5 MB，755 条目）与 `-sdl-examples`（832 文件/748.8 MB，
  zip 211.1 MB，835 条目）`--verify` 通过，包内 `libSDL3.a` 与元数据为静态 SDL，
  解包后 `basic.exe` 独立运行。
  原未覆盖的 x86 SDL 组合已于 2026-10-29 补齐：`build.py build --config Release
  --arch x86 --sdl` 重建 x86 `SDL3-static.lib`（786/786，Skia `llvm.x86.release`），
  出包 `nim_duilib-1.0.2026.1023-win-x86-sdl-examples.zip`（844 条目 / 128.2 MB，
  SDL3 79 项 + 21 个示例），`--verify` 844 条目 SHA 全匹配，SDK 测试
  PASS=16 / FAIL=0 / SKIP=3，`bin/basic.exe` 启动通过（W-P27）。
- Linux x64 Release 实测（2026-10-25）：编译与打包流程均通过；
  `pack.py` 分别产出默认 SDK 包、`--with-cef`、`--with-examples` 三种组合。
  2026-10-28 真机复测补齐 Debug 配置（再出 3 个 Debug 包）与解包后示例启动运行，
  8 次 `sdk_package_test.py` 全部通过（含下游工程接入与两次 `--run-examples`）。
  此前 `ldd` 检查发现非 CEF 示例也依赖 `libcef.so`，已改为示例级
  `DUILIB_EXAMPLE_CEF` 控制（仅 `cef`/`CefBrowser` 链接 CEF）。
- FreeBSD x64 全量实测（2026-10-29 真机复测）：Release/Debug 编译与打包流程均通过，
  测试内容与 Linux 相同但不含 CEF（FreeBSD 上游无 Chromium 构建）；`pack.py` 产出
  Release/Debug 的默认 SDK 包与 `--with-examples` 包共 4 个，4 次
  `sdk_package_test.py` 全部通过（含 Release/Debug 示例包的 `--run-examples`，C10 启动
  `bin/basic`），`build.py doctor/test`、`test_build.py`、`test_pack.py`、
  `test_sdk_package.py`、`test_autotest.py`、`autotest.py` 亦全部通过；
  `--with-cef` 按不支持的平台直接报错（属预期的负向结果）。命令与结果见
  `build/test-matrix.md` §4。
- macOS x64 实测（2026-10-28 真机复测）：Release/Debug 构建、6 个出包、8 次包测试
  （含两次 `--run-examples`）与全部脚本测试通过；`basic.app` 的 `Info.plist`、
  仅改资源的增量同步与 CEF 示例 Bundle 均已验证。arm64 无真机，不安排。
- 待覆盖：
  - 配置：Linux 与 FreeBSD 均已覆盖 Debug/Release（2026-10-28 / 2026-10-29）；
  - 架构：Windows x64/x86；Linux、FreeBSD 仅 x64（不支持 x86）；arm64 的 Windows
    Release/Debug、CEF142 与 CEF109 已完成（2026-10-28），arm64 真机运行未测、不跟进；
  - 工具链：MSVC（Debug/Release 已完成）/ MinGW（gcc、llvm 已完成）/
    MSYS2（UCRT64、CLANG64、MINGW64 均已完成，2026-10-24；MINGW32 x86 未测、不跟进）；
  - 模式：静态已完成；DLL（`EnableDuilibDll`）x64/x86 与 arm64 × Debug/Release 均已完成
    （2026-10-24 / 2026-10-28）；
  - 平台：Windows、Linux、macOS、FreeBSD 均已完成（Linux/macOS 2026-10-28、
    FreeBSD 2026-10-29 真机复测）；
  - 特性：`--with-sdl`（MSVC x64/x86、MSYS2 UCRT64 gcc 已完成，2026-10-24 / 10-29）、
    `--with-cef`（Windows、Linux、macOS 已完成；FreeBSD 不支持，负向验证通过）、
    `--with-examples`（MSVC x64/x86、MinGW x64、Linux/macOS/FreeBSD Release/Debug
    已完成，2026-10-25/28/29）。
- 2026-10-29 收口：最后一个空格（MSVC x86 + SDL）已补齐、出包并通过 C10，
  本项改为 `[x]`；剩余未覆盖项仅为明确「未测、不跟进」的组合（MINGW32 x86、
  arm64 真机运行）。
- 验收：矩阵中每个格子都有记录（命令、文件数、关键文件存在性、告警），异常项有结论。

#### P0-2 关键产物缺失时阻止出包

- [x] **已实施**（2026-10-25，取代 2026-10-23 的“不处理”决策）。
- 关键产物：核心头文件 `include/duilib/duilib.h`、5 个基础库、Skia 库
  （后两者原本已直接失败）、`bin/resources/`、Windows 三份 manifest、
  显式请求时的 SDL3 运行期 DLL / CEF 运行时 / 示例程序。
- 行为：收集完成后若存在关键缺失，逐条打印原因并以退出码 1 拒绝出包；
  非关键项（许可证、文档、JPEG、WebView2 loader、PE 架构不匹配等）仍只告警。
- 排查通道：`--allow-missing` 把关键缺失降级为告警继续出包（发布流程不应使用）；
  `--list` 不受影响，只打印清单。
- 验收：`test_pack.py` 覆盖各关键项缺失、`--allow-missing` 降级与真实仓库无缺失；
  `arm64-synthetic` 与 MSVC x64 实际出包不受影响。

#### P0-3 接入层端到端验证

- [x] Windows x64、x86（Debug、Release）、MSYS2 UCRT64/CLANG64/MINGW64
  与 MSVC DLL 模式（x64/x86 × Debug/Release）已完成（2026-10-24）；
  Linux、macOS、FreeBSD 的包测试（含下游 CMake 工程与示例运行）已于 2026-10-28/29 通过。
- 已完成：
  - MSBuild：导入 `msvc/nim_duilib.targets` 的 Debug|x64 工程编译、链接、运行通过；
    `NimDuilibCopyResources=1` 复制 `resources/`，`bin/*.dll` 复制 `WebView2Loader.dll`；
    嵌入 manifest 已验证（Common Controls 6.0.0.0、AMD64、supportedOS 列表）。
  - CMake：`find_package(nim_duilib CONFIG REQUIRED)` +
    `nim_duilib_copy_runtime(... RESOURCES)` 的 Debug|x64 工程编译、链接、运行通过。
  - 运行验证：`GlobalManager::Startup` 成功加载 `resources/themes/default/global.xml`，
    程序输出 `duilib version: 1.0.2026.1023`。
  - 负向验证：x64 包 + x86 工程在 CMake 配置阶段按架构不匹配拒绝，并给出明确提示。
  - Windows x86（2026-10-24）：解包 `-win-x86-debug` / `-win-x86` 包后分别用 VS 与 CMake
    建独立工程，Debug/Release 均为 32 位启动、加载资源、退出码 0；
    VS 嵌入 manifest 为 X86（Common Controls 6.0.0.0、supportedOS 列表），
    `NimDuilibCopyResources=1` 复制 214 个资源文件；CMake
    `nim_duilib_copy_runtime(... RESOURCES)` 复制 `WebView2Loader.dll`（包内为 14C x86）
    与 214 个资源文件；64 位工程配 x86 包在 CMake 配置阶段与 MSBuild 校验阶段均被架构拒绝。
  - Windows x64 Release（2026-10-24）：解包 `-win-x64.zip` 后分别用 VS（`Release|x64`）与
    CMake（VS2022 `-A x64`，`find_package` + `nim_duilib_copy_runtime`）建独立工程，
    均为 64 位启动、加载资源、退出码 0；VS 嵌入 manifest 为 AMD64（Common Controls
    6.0.0.0、supportedOS 列表），两条路径均复制 214 个资源文件与 `WebView2Loader.dll`
    （包内为 8664 x64）；32 位工程配 x64 包在 CMake 配置阶段与 MSBuild 校验阶段均被架构拒绝。
  - MinGW x64（2026-10-24）：源码树内 gcc / llvm 均已构建运行；已分别用解包后的
    `-mingw-gcc` 与 `-mingw-llvm` 包通过 `find_package(nim_duilib)` +
    `nim_duilib_copy_runtime` 建独立工程，两套工具链均构建运行成功（64 位启动、
    资源加载、退出码 0；分别导入 `libstdc++-6.dll` / `libc++.dll`）。
  - MSYS2（UCRT64 / CLANG64，2026-10-24）：解包 `-win-x64-mingw-gcc` 与
    `-win-x64-mingw-llvm` 包后，分别用 UCRT64 gcc 14.2.0 与 CLANG64 clang 19.1.7
    以 CMake（`find_package` + `nim_duilib_copy_runtime`）建独立工程，均构建运行成功
    （bits=64、startup=1、退出码 0，复制 214 个资源文件）；包内 `WebView2Loader.dll`
    为 x64，静态库为 `coff-x86-64`；MSVC 工程误用两个 MinGW 包均被工具链校验拒绝，
    MinGW 工程误用 MSVC `-win-x64` 包同样被拒。
  - MSYS2 MINGW64（2026-10-24）：解包 `-win-x64-mingw-gcc`（msvcrt 构建）后，
    用 MINGW64 gcc 14.2.0 以 CMake（`find_package` + `nim_duilib_copy_runtime`）
    建独立工程，构建运行成功（bits=64、startup=1、dll=0、退出码 0，复制 214 个
    资源文件）；包内 Skia 为 `gcc.msvcrt.x64.release` 的 `.a`。
  - MSVC DLL 模式（2026-10-24）：解包 `-win-x64-dll` 后用 VS2022 以 CMake
    （`find_package` + `nim_duilib_copy_runtime`，`-A x64`）建独立工程，
    包元数据自动设置 `DUILIB_DLL` 与 `MultiThreadedDLL`，构建后自动复制
    `duilib.dll` / `WebView2Loader.dll` 与 214 个资源文件；运行输出
    bits=64、startup=1、dll=1、退出码 0。VS props 路径同样按元数据跳过静态库、
    只链 `duilib.lib` 导入库并定义 `DUILIB_DLL`（未单独建 VS 工程，已核对属性表条件）。
  - MSVC DLL 模式补齐（2026-10-24）：分别解包 `-win-x64-dll-debug`、`-win-x86-dll`、
    `-win-x86-dll-debug`，用 VS2022 CMake（`-A x64` / `-A Win32`）建独立工程，
    包元数据自动设置 `DUILIB_DLL` 与 `MultiThreadedDLL`；运行输出依次为
    bits=64/32/32、startup=1、dll=1、退出码 0，并自动复制对应
    `duilib_d.dll` / `duilib32.dll` / `duilib32_d.dll`、`WebView2Loader.dll`
    与 214 个资源文件。x86 Release 首次运行因系统 `MSVCP140.dll` 为 14.29
    （编译器 14.44）而崩溃，随程序附带新版 VC++ 运行库后正常，属运行环境依赖。
- 发现并修复：接入层 Debug 未屏蔽 release CRT，下游链接出现 LNK4098；
  已在 `packaging/msvc/nim_duilib.props` 与 `packaging/cmake/nim_duilibConfig.cmake`
  中补 `/NODEFAULTLIB:"libcmt.lib"`（与仓库内 `DuilibSettings.props` /
  `BinCommonSettings.props` 一致）。修复后 VS 与 CMake 两条路径均无 LNK4098/LNK2038。
- 发现并修复（2026-10-24）：`build/gcc-mingw-w64_build.bat`、
  `build/llvm-mingw-w64_build.bat` 只把 `DUILIB_SKIA_LIB_SUBPATH` 设为环境变量，
  未以 `-D` 传给示例的 CMake 配置，示例因此链接到 MSVC 的 `llvm.x64.release`
  （`skia.lib`），出现全量 `undefined reference`。两个脚本的示例配置行已补
  `-DDUILIB_SKIA_LIB_SUBPATH=%DUILIB_SKIA_LIB_SUBPATH%`，复测全部链接通过。
- 未覆盖及原因：
  - 当前无：Windows x64/x86、Linux、macOS 与 FreeBSD 的下游接入（`find_package`
    配置、编译、链接、运行）均已通过包测试（Linux/macOS 2026-10-28，
    FreeBSD 2026-10-29）；macOS 的 CEF Bundle 仍按“下游手动配置”流程验证。
- 验收（Windows x64）：同上（Debug/Release）；另确认 CMake 与 MSBuild 的架构校验均能拒绝 Win32 工程误用 x64 包。
- 验收（Windows x86）：同上（Debug/Release）；另确认 CMake 与 MSBuild 的架构校验均能拒绝 x64 工程误用 x86 包。
- 验收（MinGW x64）：同上；另确认 MSVC 工程误用 MinGW 包会被工具链校验拒绝。
- 验收（MSYS2）：UCRT64 gcc、CLANG64 llvm、MINGW64 gcc(msvcrt) 三条链路的出包、解包 SHA
  与 CMake 端到端均通过；另确认 MSVC 与 MinGW 包双向误用会被工具链校验拒绝。
- 验收（DLL 模式）：x64/x86 × Debug/Release 四个包内均有对应
  `bin/duilib*d.dll` 与 `lib/<arch>/<Config>/duilib.lib` 导入库；
  CMake 与 VS props 按包元数据只链导入库并定义 `DUILIB_DLL`，四个组合端到端运行通过。

#### P0-4 签名与公证

- [-] **不处理**（2026-10-23 决策：不具备条件）。
- 理由：当前没有 Windows 代码签名证书与 macOS 公证环境；SDK 以头文件和静态库/导入库形式分发，通常不需要对库本身签名。
- 说明：如后续需要对外分发可执行示例或安装包，再单独准备证书与公证流程。

#### P0-5 发布流程闭环

> 方案见 [Release-Process-Closure-Plan.md](Release-Process-Closure-Plan.md)，已于 2026-10-23 实施。
> 版本号单一源为 `duilib/duilib_version.h`；`BUILD` 段不补零。

- [x] 版本递增检查、CHANGELOG / Release Notes、git tag、产物归档。
- [x] `dist/` 旧产物清理策略。
- 现状：`build/release.py` 提供 check / package / finalize / clean；中英
  `CHANGELOG.md` / `CHANGELOG.en.md` 与 `docs/misc/Release-Checklist.md` 已建立；
  包名追加 `-sdl/-cef/-cef109/-examples/-debug` 特性后缀，产物按 `dist/<版本>/` 归档。
- 验收：按 checklist 可从干净工作区产出唯一版本包，步骤可复现。

### P1：发布质量

#### P1-1 Unix 归档语义

- [x] tar 内文件权限按类型设置，不再统一 `0o644`。
- [x] 符号链接按 `lstat` 处理，避免 zip 展开 `.so` 版本链产生重复实体。
- 现状（2026-10-23 已实施）：`_tar_add_file()` 用 `lstat` 生成条目，普通文件权限取自
  源文件（清除组/其他可写位，保留可执行与 setuid 等特殊位）；Windows 源没有 POSIX
  权限位，按扩展名给 `0755`/`0644`。`_zip_add_file()` 把符号链接写成 `S_IFLNK`
  条目（内容为链接目标），普通文件按权限写 `external_attr`，解包端可还原权限。
  `iter_files()` 额外产出目录型符号链接（如 macOS Framework 的 `Versions/Current`），
  否则这类链接会丢失。符号链接不计入 `SHA256SUMS`，避免 `sha256sum -c` 跟随链接后
  校验失败。
- 实测结构（Linux SDL3）：`libSDL3.so -> libSDL3.so.0 -> libSDL3.so.0.5.0`，
  打包需保留完整符号链接链，不能只留实体文件。
- 其它受影响文件：Linux `bin/` 下的示例程序（无扩展名，可执行）、
  macOS `bin/` 下的示例程序与 `.app` bundle 内的可执行文件、
  CEF 的 `chrome-sandbox`（需要可执行权限）。
- macOS `bin/` 的示例为无扩展名可执行文件（`-rwxr-xr-x`），CEF 示例为
  `cef.app`、`CefBrowser.app`；打包需保留 bundle 目录结构与内部符号链接。
- 依据：`build/pack.py` 的 `build_package()` 与 `iter_files()`。
- 验收：Linux/macOS 包内示例程序与 `chrome-sandbox` 可直接执行；
  `libSDL3.so*` 等链接结构与构建目录一致。
  Windows 侧已回归：Debug x64 zip 539 个文件，`SHA256SUMS` 全部匹配，
  `.dll` 条目 `0755`、`.lib` 条目 `0644`，`zip` 可完整解压；
  Linux、FreeBSD 侧已真机出包（FreeBSD Release/Debug，2026-10-29；Linux 2026-10-28），
  符号链接与权限位逻辑随包生效，示例程序可直接执行；
  macOS 侧真实包已验证：2026-10-28 的 `--run-examples` 从解包产物直接启动
  `basic.app`（首次因解包丢可执行位失败，修复后通过并补 5 项单元测试）；
  构造用例（zip 写 `S_IFLNK` 条目、tar 写 `SYMTYPE`，链接目标与模式均正确）
  继续覆盖归档层。

#### P1-2 可复现构建

- [x] 支持 `SOURCE_DATE_EPOCH`，固定 zip 条目时间与 tar mtime/gzip 头。
- 现状（2026-10-23 已实施）：`read_source_date_epoch()` 读取并校验环境变量；
  设置后所有归档条目、gzip 头与 `MANIFEST.txt` 的“打包时间”统一使用该值
  （按 UTC 渲染）。未设置时普通条目取源文件 mtime，生成文件与 gzip 头取收集
  产物中的最新 mtime；条目按包内路径排序。`--format tarbz2` 无时间戳头，
  同样稳定。
- 实测：Debug x64 包连续两次打包，zip 摘要
  `616D5FDB…36FAD36`、tgz 摘要 `C78FAF5F…0B171E` 两次一致；
  `SOURCE_DATE_EPOCH=1700000000` 时 zip 摘要 `BEEF0E1A…0A9EDC`、tgz 摘要
  `2DBDFB19…C52473` 两次一致，条目时间全部为 `2023-11-14 22:13:20`，
  gzip 头 mtime 同为 `1700000000` 且不写文件名；非法值退出码 1。
- 验收：同一输入、同一环境连续两次打包，`SHA256SUMS` 与包文件摘要一致。

#### P1-3 CEF 运行时打包

- [x] Windows：运行时二进制位于 `bin/libcef_win/{x64,Win32}/`、
  `bin/libcef_win_109/{x64,Win32}/`，含 `libcef.dll`、`*.pak`、`icudtl.dat`、
  `v8_context_snapshot.bin`、`locales/`。
- [x] Linux：运行时位于 `bin/libcef_linux/`（扁平，不再分架构），含
  `libcef.so`、`libEGL.so`、`libGLESv2.so`、`libvk_swiftshader.so`、
  `libvulkan.so.1`、`chrome-sandbox`、`vk_swiftshader_icd.json`、
  `resources.pak`、`chrome_100_percent.pak`、`chrome_200_percent.pak`、
  `icudtl.dat`、`v8_context_snapshot.bin`、`locales/*.pak`。
- [x] 打包脚本按上述结构收集对应平台/架构的运行时，保持相对结构放入包内 `bin/`。
  - [x] Windows（2026-10-23）：`--with-cef` 时从 `bin/libcef_win[_109]/{x64,Win32}/`
    收集运行时，包内保持 `bin/libcef_win[_109]/<架构>/` 结构；
    `libcef.lib`、`cef_sandbox.lib` 等库文件不随包（前者在 `lib/` 已提供，
    后者本项目不使用沙箱且体积 80 MB 级）。
  - [x] MinGW 包不支持 CEF（2026-10-24）：`pack.py` 对 MinGW 工具链的
    `--with-cef` 直接报错，避免把 MSVC 的 `libcef.lib` 与 wrapper 打进 MinGW 包。
  - [x] Linux（2026-10-25）：`--with-cef` 接入 `bin/libcef_linux/`，包内保持
    扁平结构 `bin/libcef_linux/`；`libcef.so` 不重复放进 `lib/`（避免上百 MB
    重复），链接方式为 `-L<SDK>/bin/libcef_linux -lcef -lcef_dll_wrapper`，
    `find_package(nim_duilib)` 按绝对路径自动链接；头文件收集到
    `include/duilib/third_party/libcef/libcef_linux/include`；
    `--cef-version 109` 仅 Windows（Linux 只有 latest），
    `pack.py` 对非 Windows 传入 109 直接报错。仓库缺少 `bin/libcef_linux/libcef.so`
    时按关键产物缺失处理（`PackageError`）。
  - macOS 按既定设计不自动收集（见下条）。
- 现状（2026-10-25）：Windows 运行时已随包；MinGW 包禁止 CEF；Linux 已接入
  `bin/libcef_linux/` 并完成真机出包（Release，含 `--with-cef`）；FreeBSD 不支持
  CEF（上游无 Chromium 构建，`pack.py` 对 `--with-cef` 直接报错）；macOS 仍由下游手动
  配置，`docs/Release.md` 与 `Release.en.md` 已补充手动部署流程。
- [x] macOS：结构已确认，CEF 随示例以 `.app` 形式生成：
  `Contents/MacOS/<exe>`、`Contents/Frameworks/Chromium Embedded Framework.framework`
  （`Versions/A/...` 及顶层符号链接）、`Contents/Frameworks/<name> Helper*.app`
  （Helper / GPU / Plugin / Renderer）、`Contents/Resources/`。
  按既定设计，SDK 包不自动收集 macOS 的框架与 Helper，由下游手动配置。
- [x] macOS 的 CEF 二进制分发包位于与仓库平级的 `cef_binary/`（可用 `-DCEF_ROOT`
  或环境变量 `CEF_ROOT` 覆盖），内含 `include/`、`libcef_dll/`、`cmake/`、
  `Release/Chromium Embedded Framework.framework`；构建时由 `COPY_MAC_FRAMEWORK`
  把框架与 Helper 拷入 `.app` 的 `Contents/Frameworks/`。
  注意：仓库内 `duilib/third_party/libcef/libcef_macos` 只用于编译 wrapper，
  不是运行时分发包，两者不能混用。
- [x] FreeBSD：不支持 CEF，包内不出现任何 CEF 内容。
- 验收：带 CEF 的 Windows/Linux 示例程序在干净机器上无需额外拷贝即可启动；
  `chrome-sandbox` 的可执行权限正确；macOS 按手动配置流程验证。
  Windows 侧已满足运行时随包（`--with-cef --list`：x64 latest 235 个、109 个 69 个
  运行时文件；x86 latest 233 个，且不含任何 `.lib`）；示例程序已在
  W-P01～W-P09 等示例包内构建完成。**包内示例启动（C10）已于 2026-10-29 验证**：
  x64 Release 的静态 CEF 包、SDL 包、无 CEF 包与 x64 Debug/Release 的 DLL 包
  五个示例包全部 `[PASS] C10 examples.smoke`（见 P3-1）。
  Linux 侧由 `test_pack.py` 的 `CefLinuxPackagingTest` 覆盖收集、元数据、MANIFEST
  与拒绝场景（5 个用例）；真机出包已验证（Release，含 `--with-cef` 与
  `--with-examples` 组合，2026-10-25），解包后示例运行已于 2026-10-28 的
  `--run-examples` 验证通过；macOS 手动部署流程见
  `docs/Release.md` 的“macOS：下游手动配置 CEF”。

#### P1-4 WebView2 运行时说明

- [x] 在 `MANIFEST.txt` 与接入文档中明确“依赖系统 Evergreen Runtime”。
- 现状（2026-10-23 已实施）：`write_manifest()` 的“运行期”章节在包内含
  WebView2 loader（`WebView2LoaderStatic.lib`、`WebView2Loader.dll.lib` 或
  `bin/WebView2Loader.dll`）时，注明依赖系统已安装的 Microsoft Edge WebView2
  Evergreen Runtime、本包不含浏览器内核、`0x80070002` 错误与安装地址。
  `docs/Release.md` / `Release.en.md` 新增“WebView2 运行时”章节，说明
  静态/动态 loader 的差异、Fixed Version Runtime 方式与常见问题。
- 验收：Windows 包的 `MANIFEST.txt` 与文档均有说明，常见问题覆盖运行时缺失、
  `WebView2Loader.dll` 缺失与架构不匹配三种情况。

#### P1-5 校验方式对 Windows 友好

- [x] 提供 `verify.py` 或 PowerShell `Get-FileHash` 示例。
- 现状（2026-10-23 已实施）：`MANIFEST.txt` 的“校验”章节与
  `docs/Release.md` / `Release.en.md` 的“校验发布包”章节给出 PowerShell 5+
  校验 `SHA256SUMS` 的脚本（全部通过输出 `OK`，否则列出失败项并 `exit 1`），
  并补充 `Get-FileHash` / `certutil -hashfile` 的单文件用法；
  `release.py finalize` 生成的压缩包级 `SHA256SUMS.txt` 格式相同，可在
  `dist/<版本>/` 直接复用同一脚本。
- 验收：Windows 用户无需 `sha256sum`，在包根目录执行文档中的命令即可完成
  全包校验；已在解压后的实测包上执行该脚本，输出 `OK`。

#### P1-6 许可合规复查

- [x] 逐项确认随包分发的第三方组件许可：Skia 及其第三方（expat、skcms、bentleyottmann、skresources 等）、CEF、WebView2、SDL、libjpeg-turbo、libwebp、libpng、zlib。
- 现状（2026-10-24 已实施）：新增 [License-Compliance.md](License-Compliance.md)，
  按“组件 → 许可证 → 是否允许随包分发 → 包内文件”逐项列出 25 个组件/模块
  （含字体资源）。随包组件均为 MIT / BSD-2/3 / zlib / libpng / Apache-2.0
  （含 LLVM 例外）等宽松许可，允许二进制分发；未发现 GPL/AGPL/LGPL。
- 复查发现两处许可文本缺失：Skia 静态链接的 expat（MIT）、资源字体
  Roboto Mono（Apache-2.0）。已新增 `licenses/expat.LICENSE.txt` 与
  `licenses/RobotoMono.LICENSE.txt`，并在 README 第三方库表中登记；
  打包脚本会随 `licenses/` 一并收集。
- 确认 Skia 实际仅启用 expat 一个 third_party（`args.gn` 关闭了 libpng、zlib、
  libjpeg-turbo、libwebp、ICU），因此无需补充其余 Skia 第三方许可。
- 验收：对照表覆盖全部随包二进制与许可文本；`--list` 中 `lib/expat.lib`、
  `licenses/expat.LICENSE.txt` 与 `licenses/RobotoMono.LICENSE.txt` 均已随包。

### P2：维护与体验

#### P2-1 pack.py 自动化测试与 CI

- [x] 为链接重写、MANIFEST、SHA256SUMS、元数据推断、接入文件生成编写测试。
- [x] 不接入 CI（用户决策）；改为提供可本地重复执行的自动化脚本。
- 现状（2026-10-25）：`build/test_pack.py` 扩展到 39 个用例（含 P0-2 关键产物
  缺失与 `--allow-missing` 降级），覆盖包体检正/负向
  （zip 与 tar.gz）、文档断链、包名（含 arm64/DLL）、MSVC 运行库自动推断、
  MANIFEST 生成与时间确定性、接入文件生成、arm64 manifest/Skia/CEF/JPEG 行为、
  防回归；工作区 Python 与 MSYS2 UCRT64 Python 全部通过。
  新增 `build/autotest.py`：预设化执行「环境检测 →（可选）构建 → 打包 →
  `--verify` → 结构断言 →（可选）冒烟」，输出 `report.json` / `autotest.log`
  到 `C:\develop\temp\autotest\<时间戳>/`，默认不写仓库 `bin/`、`lib/`、`dist/`；
  支持 `unit`、`pack-msvc-x64-release/debug`、`pack-msvc-x86-release`、
  `pack-mingw-x64-release`、`arm64-synthetic`、`all-ready` 与 `--dry-run`。
  新增 `build/test_autotest.py`（17 个用例）。实测 `unit`、`arm64-synthetic`、
  `pack-msvc-x64-release` 预设全部通过；CI 接入按决策不做。

#### P2-2 包体检命令

- [x] 增加 `pack.py --verify <包路径>`：校验布局、SHA、文档断链、头文件完整性。
- 验收：损坏包与缺文件包能给出明确错误；正常包返回 0。
- 实施（2026-10-24）：支持 zip / tgz / tarbz2；检查单根目录、`MANIFEST.txt` 与
  `SHA256SUMS` 存在、`include/duilib/` 与 `lib/*.a|*.lib`、逐文件 SHA256
  （上限 20 条报错）、SDK 文档相对链接（跳过 `licenses/` 下逐字复制的上游文本）；
  `MANIFEST.txt` / `VERSION` 按设计未登记 SHA，单独豁免。
  实测 4 个 MinGW 包（gcc / llvm / Debug / SDL）全部 `[OK]` 返回 0；
  篡改文件、缺 `MANIFEST.txt`、截断 zip、路径不存在四类坏包均给出明确错误并返回 1。

#### P2-3 文档补充

- [x] 安装、升级、迁移说明。
- [x] 常见问题（运行期 DLL 缺失、资源目录、架构/配置不匹配、CEF 运行时）。
- [x] 发布 checklist。
- 现状：发布 checklist 见 [Release-Checklist.md](Release-Checklist.md)
  （版本递增、CHANGELOG、构建矩阵、出包与体检、归档与 tag 的勾选步骤）。
- 实施（2026-10-29）：新增双语 `docs/Deployment.md` / `Deployment.en.md`
  （包内随文档，链接已按包布局改写并纳入 `Summary.md` 索引）：包目录结构与包名含义、
  免安装解包与 `SHA256SUMS` 校验、Windows VS 接入（`nim_duilib.targets`）与
  CMake 接入（`find_package` + `nim_duilib_copy_runtime`）、运行期文件清单
  （`resources/`、DLL、CEF 运行时目录、MSVC/MinGW 运行库）、升级步骤
  （矩阵核对、替换目录、清理旧产物、重编）、迁移表（源码树 → SDK、静态 → DLL、
  CEF142 → CEF109、单配置 → Debug/Release、x64 → arm64）与 11 条常见问题的
  「现象 / 原因 / 处理」。包内 `MANIFEST.txt` 仍是由 `pack.py` 按实际收集结果
  生成的权威清单，文档说明两者不一致时以 `MANIFEST.txt` 为准。
- 验收：新用户按文档可从下载走到可运行程序（解包 → 校验 → 接入 → 资源与运行期
  文件 → 启动）。

#### P2-4 arm64 策略

- [~] 明确 arm64 的支持范围，并补齐 manifest 与相关产物。
- 现状（2026-10-24）：新增 `msvc/manifest/duilib.arm64.manifest`（ARM64 +
  Common Controls + supportedOS）；`pack.py` 的 Windows 包统一收集 x64/x86/arm64
  三份 manifest，并校验 `bin/*.dll` 的实际 PE 架构（交叉出包时跳过不匹配项）；
  `nim_duilib.props` 按 `Platform` 选择 arm64 manifest；`nim_duilibConfig.cmake`
  与 `duilib_common.cmake` 在无 generator platform 时用 `CMAKE_SYSTEM_PROCESSOR`
  识别 ARM64；`SkiaSettings.props` 补 `llvm.arm64.release/debug` 与
  `.md/.mdd` 目录。`arm64-synthetic` 预设用临时假库产出 arm64 包并通过
  `--verify` 与结构断言（三份 manifest、arm64 架构、CMake config）。
- 支持范围：接入文件与包结构**已就绪**；libjpeg-turbo 的 Windows arm64 预编译库
  已入库（`libjpeg-turbo6-win-vc-arm64`），`pack.py` 与 `JpegTurboSettings.props`
  已接入，arm64 包随包 `lib/turbojpeg-static.lib`；**arm64 真机编译、打包、运行验证
  未测、不跟进**（无测试设备，不再列入待办）。
- 现状（2026-10-28）：CEF 的 Windows arm64 随包放开。上游 `cef_builds` 对 142 与 109
  都发布 `windowsarm64` 预编译包，仓库 `libcef_win[_109]/lib/arm64/libcef.lib` 已入库；
  `pack.py` 按 `lib/arm64` 收集导入库、按 `bin/libcef_win[_109]/arm64/` 收集运行时，
  `third_party_versions.json` 补 109 的 `windows/arm64` 归档，`test_pack.py` 由
  「报错」改为「收集」用例；执行步骤见 `build/arm64-test-plan.md`（W-A06/W-A07）。
- 现状（2026-10-28，范围收敛）：arm64 本轮**只验证 Windows**——macOS / Linux / FreeBSD
  均无真机，不安排测试。Windows 侧 VS 2022 已装 MSVC 14.44.35207 ARM64 交叉工具，
  `build.py doctor --arch arm64` 通过；`build.py` 的 `vcvarsall` 参数此前固定传 `arm64`
  （要求 Hostarm64 原生工具集）导致误报“工具未安装”，已改为按主机选择
  `amd64_arm64` / `x86_arm64` / `arm64`。
- 现状（2026-10-28，Windows 实测）：`deps` 自动构建 `skia/out/llvm.arm64.release` 与
  `llvm.arm64.debug`；`build.py --arch arm64` Release（25/25）与 Debug（802/802，23 个
  示例）全量构建均通过，产物均为 PE `0xAA64`，嵌入清单为 `ARM64`；出包
  `-win-arm64-examples.zip`（760 条目 / 109.0 MB）、
  `-win-arm64-cef-examples.zip`（1314 条目 / 311.7 MB，CEF 运行时 `0xAA64`）、
  `-win-arm64-cef-examples-debug.zip`（1314 条目 / 394.5 MB）与
  `-win-arm64-cef109-examples.zip`（1131 条目 / 252.1 MB，109 运行时 `0xAA64`），
  `pack.py --verify` 全匹配；`sdk_package_test.py` 四包均 PASS=14 FAIL=0 SKIP=5
  （C01 完成配置/编译/链接，因 x64 主机无法运行 ARM64 程序而按架构 SKIP）。
  同轮修复：ARM64 无 WGL（Skia 恒走 CPU）、manifest 选错、`nim_duilibConfig.cmake`
  交叉架构识别。
- 验收：Windows Release/Debug 构建、VS ARM64 静态与 DLL 模式、四个包的出包/包测试
  均已通过；DLL 模式（W-B28/W-B29）与 CEF109（W-A07/W-P25）已补齐。Windows on ARM
  真机运行验证无设备，标记未测、不跟进（不再列入待办）。计划与结果见
  `build/arm64-test-plan.md`。

#### P2-5 Unix 共享库（.so/.dylib）

- [-] **暂不实施**（2026-10-28 决策：收益不足，Unix 保持静态库）。
- 评估记录见 [Unix-Shared-Library-Feasibility.md](Unix-Shared-Library-Feasibility.md)：
  符号导出（`-fvisibility=hidden` + Unix `DUILIB_API` 为空）、第三方静态库 PIC、
  CEF 无 delay-load 语义、rpath/install_name 与打包/下游接入是主要改造点；
  技术上可行，但不等同于 Windows 的 `--dll`。
- 重新评估条件：多程序共享同一份库、插件热更新、分发包体积或链接时间有明确收益，
  或下游明确要求动态库形态。

### P3：本轮剩余跟进项（2026-10-29）

> 建立依据：对 P0～P2 的逐项复核。功能与验证层面只剩以下 6 项，其余条目均已
> `[x]`，或按决策 `[-]` / `[~]` 关闭。截至 2026-10-29，P3-1、P3-2、P3-4 与 P3-6
> 已完成，剩余 P3-3、P3-5。

#### P3-1 Windows 示例包的包内启动冒烟（C10）

- [x] 已完成（2026-10-29）：五个示例包的包内启动（C10）全部通过
  （覆盖静态/SDL/无 CEF 与 DLL 的 Debug/Release）。
  - W-P04（x64 Release + CEF，1318 条目 / 328.2 MB）：
    `nim_duilib-1.0.2026.1023-win-x64-cef-examples.zip`；
  - W-P09（+SDL，1398 条目 / 352.9 MB）：
    `...-win-x64-sdl-cef-examples.zip`；
  - W-P10（无 CEF，764 条目 / 121.8 MB）：`...-win-x64-examples.zip`；
  - W-P18（x64 Debug DLL + SDL + CEF，1398 条目 / 219.6 MB）：
    `...-win-x64-sdl-cef-examples-dll-debug.zip`（`duilib_d.dll` 36,236,800 B）；
  - W-P19（x64 Release DLL + SDL + CEF，1398 条目 / 210.1 MB）：
    `...-win-x64-sdl-cef-examples-dll.zip`（`duilib.dll` 13,602,816 B）。
  每个包均先 `pack.py --verify`（SHA 全匹配），再
  `sdk_package_test.py --scenarios C10 --run-examples`，结果均为
  `[PASS] C10 examples.smoke`（`bin/basic.exe` 启动 3 秒后仍在运行，
  PASS=1 / FAIL=0 / SKIP=0）；证据已回填 `build/test-matrix.md` §2/§7。
  DLL 回归后已执行 DLL → 静态恢复构建（`bin/duilib*.dll` 由 `build.py` 自动清理）。

#### P3-2 x86 + SDL3 出包组合

- [x] 已完成（2026-10-29）：`python build\build.py build --config Release --arch x86
  --sdl`（786/786，SDL3 按 x86 重建为 `../SDL3/lib/SDL3-static.lib`，Skia
  `llvm.x86.release`）→ `python build\pack.py --config Release --arch x86
  --with-examples --with-sdl --out-dir dist` 产出
  `nim_duilib-1.0.2026.1023-win-x86-sdl-examples.zip`（844 条目 / 128.2 MB，
  SDL3 79 项、21 个示例、CEF 库按未请求跳过）→ `pack.py --verify` 844 条目
  SHA 全匹配 → `sdk_package_test.py --run-examples`：PASS=16 / FAIL=0 / SKIP=3
  （含 `[PASS] C10 examples.smoke`，`bin/basic.exe` 启动 3 秒后仍在运行）。
  证据已回填 `build/test-matrix.md` 的 W-B30、W-P27、§2/§7 与已知缺口第 10 条；
  P0-1 随之收口。测试后已恢复 x64 静态基线，并重新构建 x64 `SDL3-static.lib`。

#### P3-3 `build/` 脚本控制台输出英文化

- [ ] 按 `docs/misc/specs/English-Console-Output/tasks.md` 的 T1～T7 执行
  （`pack.py`、`release.py`、`autotest.py`、两个 `.sh`，并同步测试断言）。
  现状：T1～T7 全部未勾选；`build/pack.py:389` 等处仍输出中文 `[ERROR]`，
  `autotest.py` 的预设说明仍为中文，纯英文环境会显示乱码/方块。
- 约束：只改运行期输出，注释、docstring 与 `MANIFEST.txt` 内容保持中文。

#### P3-4 `build/legacy/` 旧脚本清理

- [x] 已完成（2026-10-29）：删除 `build/legacy/` 下 11 个旧脚本
  （`build_duilib_all_in_one.*`、`msvc_build.bat`、`linux_build.sh`、`macos_build.sh`、
  `freebsd_build.sh`、`msys2_build.sh`、MinGW/VS 辅助脚本）；`README.md` / `README.en.md`、
  `build/README*.md`、`build/build-reference*.md`、`CHANGELOG*.md` 与本文档同步为
  「已删除」，仓库内已无对该目录的引用。

#### P3-5 `release.py` 全流程演练

- [ ] 发版前完整执行 `check → package → finalize → clean --list` 并记录结果。
  现状：`build/test_*.py` 没有 release 相关的自动化用例，文档只给出步骤
  （P0-5 的验收未附执行记录）。

#### P3-6 许可合规遗留项

- [x] 已完成（2026-10-29）：`docs/misc/License-Compliance.md`「遗留事项」4 条全部处理，
  文档复核日期改为 2026-10-29 并新增「遗留事项处理结果」与更新记录。
  - 新增组件同步：登记流程写入 `Release-Checklist.md` 第 1 节（`pack.py --list`
    对照、`licenses/`、`THIRD-PARTY-NOTICES.md`、对照表与更新记录）与第 5 节
    finalize 复核；复核时随包组件与 `licenses/` 的 20 个许可文件完全对应；
  - libpag：确认 SDK 包固定 `DUILIB_LIB_PAG=0`、不收集 libpag 库；写明启用时须随程序
    分发 `duilib/third_party/libpag/licenses/**`（`libpag.LICENSE.txt` 118,099 B、
    `tgfx.LICENSE.txt` 49,968 B），且未使用 lz4/Qt；
  - `bin/resources`：按来源分三层记录（首提交 `02c0f6e9` 的 144 个上游资源、
    后续项目新增的控件/示例资源、Lottie/PAG 演示素材）；其中 `loading5.json`/
    `loading6.json` 含 `LottieFiles AE` 导出元数据，`lottie.json`/`pag_logo.pag`
    的提交未记录授权；已在 `THIRD-PARTY-NOTICES.md` 增加「示例动画与图片素材」说明，
    约定仅作演示、下游商用前自行确认或替换；
  - CEF：实测官方 109/142 二进制包均无 `LICENSES.chromium.html`，按其生成方式估算体积
    为 10 MB 量级（zip 后 1–2 MB，占 CEF 运行时不足 1%），决定默认不随包，
    需要严格覆盖时按同版本 Chrome/Chromium 手工补充到 `LICENSES/CEF/`。
  性质为发布前检查项，不改变当前出包行为；`python build\build.py test` 复跑通过。

## 四、建议推进顺序

1. P3-1 Windows 示例包 C10 冒烟已完成（2026-10-29：六个示例包全部通过）；
2. P3-2 x86 SDL3 库与出包组合已完成（2026-10-29：W-P27，P0-1 收口）；
3. P3-3 脚本控制台英文化（P3-4 的 `build/legacy/` 清理已于 2026-10-29 完成）；
4. P3-6 许可合规遗留已核对完成（2026-10-29）；发版前执行 P3-5 `release.py` 全流程演练。

> P0-2 已实施（2026-10-25）；P0-1 已于 2026-10-29 收口（x86 SDL 组合补齐）；
> P0-3、P0-5、P1、P2 均已收口；P0-4、P2-5 已明确关闭，不再排期。
> P3-1、P3-2、P3-4、P3-6 已完成，剩余 P3-3、P3-5。

## 五、更新记录

| 日期 | 变更 |
| :--- | :--- |
| 2026-10-23 | 创建，整理当前基线与待办清单 |
| 2026-10-23 | P0-2、P0-4 明确关闭；P1-3 补充 CEF 二进制目录现状与平台范围 |
| 2026-10-23 | 补充平台约束（Linux 无 x86、FreeBSD 无 CEF）；记录 Windows/Linux CEF 运行时结构与 SDL3 符号链接结构 |
| 2026-10-23 | 修复 macOS 编译错误：仓库根 `VERSION` 重命名为 `VERSION.txt`，避免与标准头 `<version>` 冲突 |
| 2026-10-23 | 记录 macOS `bin/` 结构：示例为可执行文件，CEF 示例为 `cef.app`/`CefBrowser.app`（含 Framework 与 Helper）；明确 macOS CEF 仍由下游手动配置 |
| 2026-10-23 | 记录 macOS CEF 分发包位置与结构：同级 `cef_binary/`，含 `Release/Chromium Embedded Framework.framework`；与仓库内 wrapper 目录用途不同 |
| 2026-10-23 | P0-5 方案定稿并存入 [Release-Process-Closure-Plan.md](Release-Process-Closure-Plan.md)，暂不实施 |
| 2026-10-23 | 确定版本号单一源方案：`duilib/duilib_version.h` 为唯一编辑点，删除 `VERSION.txt`，`msvc/dllRes/version.h` 改为适配层，`BUILD` 段不补零 |
| 2026-10-23 | 版本号单一源已实施：`pack.py` 解析新头文件，`--list` 版本为 `1.0.2026.1023`；`VERSION.txt` 已删除 |
| 2026-10-23 | P1-3 Windows 运行时已随包：`collect_cef_runtime()` 收集 `bin/libcef_win[_109]/<架构>/`，排除 `.lib`；Linux 待 `--with-cef` 支持，macOS 按设计手动 |
| 2026-10-23 | P1-1 已实施：tar 按源权限写模式（保留可执行/setuid），zip 用 `S_IFLNK` 条目保存符号链接，`iter_files()` 补目录链接，符号链接不计入 `SHA256SUMS` |
| 2026-10-23 | P1-2 已实施：支持 `SOURCE_DATE_EPOCH`，统一归档条目/gzip 头/MANIFEST 时间；未设置时用源文件与产物最新 mtime，条目排序，两次打包摘要一致 |
| 2026-10-23 | P1-4 已实施：MANIFEST 运行期与 Release 文档标注 WebView2 依赖系统 Evergreen Runtime，补充 loader 用法与常见问题 |
| 2026-10-23 | P1-5 已实施：MANIFEST 与 Release 文档补充 PowerShell `Get-FileHash` 全包校验脚本及 `SHA256SUMS.txt` 用法 |
| 2026-10-24 | P1-6 已实施：新增许可合规对照表；补充 expat（MIT）与 Roboto Mono 字体（Apache-2.0）许可文本并在 README 登记；确认随包组件均为宽松许可、无 GPL/AGPL/LGPL |
| 2026-10-24 | Windows x86 Debug 出包验证：补齐 4 个基础库与 CEF wrapper，`pack.py --arch x86` 产出 541 文件/242.1 MB；解包后 SHA256 全包通过，包内 20 个二进制均为 x86（14C machine）；`--with-cef --list` 收集 233 个 Win32 运行时文件 |
| 2026-10-24 | Windows x86 接入层端到端验证：用解包 SDK 分别以 VS（`nim_duilib.targets`）与 CMake（`find_package` + `nim_duilib_copy_runtime`）构建 Debug 程序，32 位启动成功并加载资源（退出码 0）；x64 工程配 x86 包被架构校验拒绝 |
| 2026-10-24 | MinGW x64 Release 验证：修复 gcc/llvm 构建脚本未向 CMake 传 `DUILIB_SKIA_LIB_SUBPATH`（此前示例链接 MSVC Skia，全量 undefined reference）；两套工具链均成功构建 5 库 + 21 示例，`basic.exe` 运行正常；llvm 包 538 文件/88.9 MB，zip 22.9 MB，解包 SHA256 通过 |
| 2026-10-24 | MinGW 包区分编译器：`pack.py` 从静态库符号自动判断 gcc / llvm-mingw（可用 `--mingw-variant` 指定），包名追加 `-mingw-<变体>`，MANIFEST 工具链与 CMake/props 元数据同步；Skia 只选同源目录。MinGW 包不再收集 MSVC 专用的 `WebView2LoaderStatic.lib` 与 `turbojpeg-static.lib`，WebView2 只保留 `WebView2Loader.dll.lib` + DLL |
| 2026-10-24 | MSYS2 构建脚本补齐：`msys2_build.sh` / `build_duilib_all_in_one.sh` 启用 `set -e`、去掉 `bc` 依赖、SDL 构建目录按生成器隔离；MSYS2 依赖安装命令写入 `build/README.md` |
| 2026-10-24 | `pack.py` 支持 MSYS2 打包链路：识别 `gcc.<arch>.release` / `llvm.<arch>.release` 并按 `.a` 校验目录内容，显式 `--skia-lib-dir` 校验变体；新增 MSYS/Cygwin Python 的 Windows 主机判定；MinGW 禁止 `--with-cef` |
| 2026-10-24 | MANIFEST 补充 MinGW 运行库（libstdc++ / libc++）说明与 MSYS2 MINGW64/UCRT64 包不可互换提示；`docs/Release.md` / `Release.en.md` 同步包名说明 |
| 2026-10-24 | MinGW x64 接入层端到端验证：解包 `-mingw-llvm` 包后用 CMake `find_package` + `nim_duilib_copy_runtime` 建独立工程，llvm-mingw 构建运行成功（bits=64、startup=1、退出码 0，导入 `libc++.dll`）；MSVC 工程误用 MinGW 包被工具链校验拒绝 |
| 2026-10-24 | 修复打包 CMake Config 的 Skia 静态库顺序：`libskia.a` 移到 `skresources` 等依赖库之后，解决 GNU ld 单遍扫描下 skottie/skresources/jsonreader 的 undefined reference（MSVC 与 lld 不受影响） |
| 2026-10-24 | MinGW x64 gcc 包出包与验证：用 gcc 重建 5 个静态库后产出 536 文件/91.3 MB（zip 22.1 MB）；解包 SDK 用 gcc 建独立工程并运行成功（bits=64、startup=1、退出码 0，导入 `libstdc++-6.dll`）；修复顺序后 llvm 包复测同样通过 |
| 2026-10-24 | Windows x86 Release 出包与端到端验证：构建 5 个 MSVC Win32 Release 静态库后产出 `-win-x86.zip`（541 文件/123.3 MB，zip 27.4 MB）；解包 SDK 分别以 VS（`nim_duilib.targets`，Release\|Win32）与 CMake（VS2022 `-A Win32`，`find_package` + `nim_duilib_copy_runtime`）建独立工程，均构建运行成功（bits=32、startup=1、退出码 0，复制 214 个资源文件；VS manifest 为 X86）；x64 工程配 x86 包在 CMake 配置与 MSBuild 校验阶段均被拒绝 |
| 2026-10-24 | Windows x64 Release 出包与端到端验证：构建 5 个 MSVC x64 Release 静态库后产出 `-win-x64.zip`（541 文件/163.1 MB，zip 34.0 MB），解包 SHA256 全部匹配、`WebView2Loader.dll` 为 x64（8664）；解包 SDK 分别以 VS（`nim_duilib.targets`，Release\|x64）与 CMake（VS2022 `-A x64`，`find_package` + `nim_duilib_copy_runtime`）建独立工程，均构建运行成功（bits=64、startup=1、退出码 0，复制 214 个资源文件，manifest 为 AMD64）；Win32 工程配 x64 包在 CMake 配置与 MSBuild 校验阶段均被拒绝 |
| 2026-10-24 | MSYS2 UCRT64（gcc）全链路实测：`build_duilib_all_in_one.sh` 编译 Skia（`out/gcc.x64.release`）+ 5 库 + 21 示例（22 分钟），21 个示例全部可启动；`pack.py` 产出 `-win-x64-mingw-gcc.zip`（536 文件/90.6 MB，zip 21.3 MB），解包 539 条目 SHA 全匹配；解包 SDK 用 gcc 建独立工程运行成功（bits=64、startup=1、退出码 0，214 个资源文件）；MSVC 工程误用该包、MinGW 工程误用 MSVC 包均被工具链校验拒绝；`--with-cef` 对 MinGW 报错退出 |
| 2026-10-24 | MSYS2 CLANG64（llvm）全链路实测：Skia 输出 `out/llvm.x64.release`，5 库 + 21 示例构建成功（14 分钟），21 个示例全部可启动；`pack.py` 产出 `-win-x64-mingw-llvm.zip`（536 文件/77.0 MB，zip 20.3 MB），解包 539 条目 SHA 全匹配、静态库为 `coff-x86-64`、`WebView2Loader.dll` 为 x64；解包 SDK 用 clang 19.1.7 建独立工程运行成功（bits=64、startup=1、退出码 0，214 个资源文件）；MSVC 工程误用该包被工具链校验拒绝 |
| 2026-10-24 | `msys2_build.sh` 增强：新增 `DUILIB_BUILD_TYPE`（Debug/Release，非法值直接报错退出）、构建目录按“工具链+配置”隔离到 `build_temp/<工具链>_build/<配置>/`、并行数默认取 `nproc`（可用 `DUILIB_MAKE_JOBS` 覆盖）；`--fresh` 改为默认关闭（它会清空含 Ninja 依赖文件的 `CMakeFiles`，把增量构建退化为全量重编），需要时用 `DUILIB_CMAKE_FRESH=1`；README 中英文同步 |
| 2026-10-24 | MSYS2 Debug 链路验证（UCRT64 gcc）：Debug 全量构建 22 分钟，`lib/x64/Debug` 产出 5 库（`libduilib.a` 380.7 MB），21 个示例均可启动；`pack.py --config Debug` 产出 `-win-x64-mingw-gcc-debug.zip`（536 文件/429.8 MB，zip 82.5 MB），解包 SHA 536/536 通过；解包 SDK 用 CMake 建独立工程运行成功（bits=64、startup=1、退出码 0、214 资源）；Debug SDK + Release 工程被 FATAL 拒绝（可 `-DNIM_DUILIB_ALLOW_CONFIG_MISMATCH=ON` 强行继续） |
| 2026-10-24 | `build_duilib_all_in_one.sh` 无人值守增强：克隆/更新失败按 `GIT_RETRY_MAX`（默认 5）重试；已有仓库时 `git pull` 失败只告警并沿用本地代码（实测 GitHub 连接被重置时构建未中断）；Skia 补丁解压增加 `.patch_stamp` 幂等判断，且以 `gn/is_mingw.py` + `BUILDCONFIG.gn` 实际内容校验补丁是否真的落地（避免工作区被 git 操作还原后仍跳过解压、gn 退回 MSVC `cl.exe` 导致失败）；本地已含基线提交时跳过 stash/checkout/pull，避免刷新 mtime 触发 Skia 全量重编 |
| 2026-10-24 | 修复 MSYS2/gcc 下 `-sdl` 链接失败：SDL3 静态库在 Windows 上还引用 `hid`/`mincore`/`dinput8` 符号，而仓库只传了 `-lSDL3`。已在 `cmake/duilib_bin_windows.cmake`、`packaging/cmake/nim_duilibConfig.cmake`、`packaging/msvc/nim_duilib.props` 补齐系统库；21 个示例全部链接成功 |
| 2026-10-24 | 新增 `pack.py --verify <包路径>`（P2-2 完成）：支持 zip/tgz/tarbz2，校验单根目录、`MANIFEST.txt`/`SHA256SUMS`、`include/duilib/` 与 `lib/` 布局、逐文件 SHA256、SDK 文档相对链接；4 个既有 MinGW 包全部通过，篡改/缺 MANIFEST/截断 zip/路径不存在均返回 1；`release.py package` 出包后自动调用该体检，坏包不进入 `dist/<版本>/`；`docs/Release.md` / `Release.en.md` 选项表同步 |
| 2026-10-24 | 新增 `build/test_pack.py`（P2-1 部分完成）：11 个用例覆盖包体检正负向（zip/tar.gz）、坏 zip、缺文件、包名组合与 `SOURCE_DATE_EPOCH`，UCRT64 Python 运行通过；CI 接入仍待办 |
| 2026-10-24 | CLANG64 增量复测：网络中断时 `git pull` 失败仅告警并继续（实测 GitHub 连接重置未中断构建）；随后自动发现 Skia 补丁缺失并重新解压，`gn gen` 恢复 clang 工具链；本地已有基线提交时跳过 stash/checkout/pull（日志确认 skip update）。5 库 + 21 示例重建成功，`basic.exe` 可启动，`lib/x64/Release` 为 clang 产物；tgz 出包后用 `--verify` 校验 539 条目全部通过 |
| 2026-10-24 | SDL 特性出包与端到端验证（UCRT64 gcc）：`pack.py --with-sdl` 产出 `-win-x64-mingw-gcc-sdl.zip`（619 个条目 / 616 个登记 SHA，SDL3 79 项：头文件 + `libSDL3.a` + 许可证），解包 SHA 616/616 通过；解包 SDK 的 CMake 工程链接命令包含 `libSDL3.a` 与 `-lHid -lMincore -lDinput8`，构建运行成功（bits=64、startup=1、退出码 0）；`pack.py` 同时修复 SDL3 许可证路径（CMake 安装位于 `share/licenses/SDL3`） |
| 2026-10-24 | MSYS2 MINGW64（msvcrt）全链路实测：新增 `gcc.msvcrt.x64.release` Skia 目录（MINGW64 与 UCRT64 的 gcc 静态库 CRT 不同，不可互换），`build_duilib_all_in_one.sh` 在 MINGW64 下自动选用；全量构建成功（5 库 + 21 示例），`basic.exe` 启动正常；`pack.py` 产出 `-win-x64-mingw-gcc.zip`（536 文件/90.5 MB，zip 27.3 MB），`--verify` 与解包 SHA 536/536 匹配；解包 SDK 用 MINGW64 gcc 建 CMake 工程运行成功（bits=64、startup=1、dll=0、退出码 0） |
| 2026-10-24 | MSVC DLL 模式（`/MD`）Release\|x64 全链路：新增 `msvc/PropertySheets/duilib_dll.flag` 触发的 DLL 构建，修复导出类强制实例化导致的 C2280/C2338/C2027（`ListCtrlData` 禁止拷贝、`TabBox` 析构移入 cpp）；新建 `/MD` 版 Skia `skia/out/llvm.x64.release.md` 消除 LNK2038；`SkiaSettings.props` 按 flag 切换 Skia 目录，`pack.py` 新增 `detect_dll_mode()` 自动选择 `.md` Skia 并产出 `-win-x64-dll.zip`（542 文件/94.9 MB，zip 27.3 MB），`--verify` 545 条目 SHA 全匹配；解包 SDK 用 VS2022 CMake 工程运行成功（bits=64、startup=1、dll=1、退出码 0，自动复制 `duilib.dll`/`WebView2Loader.dll` 与 214 个资源） |
| 2026-10-24 | `build/test_pack.py` 增补 DLL 模式包名后缀用例（共 12 个），MINGW64 与工作区 Python 均通过；`--skia-lib-dir` 帮助与 `docs/Release.md`、`build/README.md` 补充 `.md/.mdd` Skia 说明 |
| 2026-10-24 | MSVC DLL 模式补齐 Debug/x86：新建 Skia 变体 `llvm.x64.debug.mdd`、`llvm.x86.release.md`、`llvm.x86.debug.mdd`（GN 参数与普通版本一致，仅 `/MT`→`/MD`、`/MTd`→`/MDd`）；`SkiaSettings.props` 增加 Debug 的 `.mdd` 路径；MSBuild 产出 `duilib_d.dll`（24.7 MB）、`duilib32.dll`（14.8 MB）、`duilib32_d.dll`（19.9 MB）与导入库 |
| 2026-10-24 | DLL 模式 Debug/x86 出包与端到端：`pack.py` 分别产出 `-win-x64-dll-debug.zip`（542 文件/100.4 MB，zip 27.8 MB）、`-win-x86-dll.zip`（542 文件/83.8 MB，zip 24.3 MB）、`-win-x86-dll-debug.zip`（542 文件/86.1 MB，zip 24.5 MB），`--verify` 均 545 条目 SHA 全匹配；解包 SDK 用 VS2022 CMake（`-A x64`/`-A Win32`）建工程，运行分别为 bits=64/32/32、startup=1、dll=1、退出码 0；x86 Release 在系统 `MSVCP140.dll` 14.29 下启动崩溃，附带 14.4x 运行库后正常，`Release.md`/`Release.en.md` 已注明该依赖 |
| 2026-10-24 | P0-1 特性组合实测（MSVC）：以 `-DDUILIB_ENABLE_SDL=ON` 重建 duilib 与 21 个示例，出包 `-win-x64-sdl.zip`（621 文件，624 条目）、`-win-x64-examples.zip`（757 文件，760 条目）、`-win-x64-sdl-examples.zip`（837 文件，840 条目）；x86 示例重建后出包 `-win-x86-examples.zip`（757 文件，760 条目，示例为 32 位）。各包 `--verify` SHA 全匹配，解包后 `basic.exe`/`ChildWindow.exe`/`controls.exe` 均正常启动 |
| 2026-10-24 | P0-1 特性组合实测（MinGW）：MINGW64 msvcrt 产物出包 `-win-x64-mingw-gcc-examples.zip`（752 文件，755 条目）与 `-win-x64-mingw-gcc-sdl-examples.zip`（832 文件，835 条目），`--verify` 通过，包内 `libSDL3.a` 为静态 SDL，解包后 `basic.exe` 独立运行；至此 P0-1 仅剩 arm64 与 Linux/macOS/FreeBSD 未覆盖，x86 SDL 因缺少 x86 SDL3 库暂未覆盖 |
| 2026-10-24 | P2-1 自动化：`build/test_pack.py` 扩展到 30 个用例（链接重写、MANIFEST、MSVC 运行库推断、接入文件、arm64、防回归），工作区与 UCRT64 Python 全部通过；新增 `build/autotest.py`（预设化检测/可选构建/打包/`--verify`/结构断言/可选冒烟/JSON 报告）与 `build/test_autotest.py`（17 个用例）；`unit`、`arm64-synthetic`、`pack-msvc-x64-release` 实测通过；按用户决策不接入 CI |
| 2026-10-24 | P2-4 arm64 接入：新增 `msvc/manifest/duilib.arm64.manifest`；`pack.py` 的 Windows 包收集 x64/x86/arm64 三份 manifest，并对 `bin/*.dll` 做 PE 架构校验（交叉出包时跳过不匹配 DLL）；`nim_duilib.props`/`nim_duilibConfig.cmake`/`duilib_common.cmake`/`SkiaSettings.props` 增加 ARM64 识别与目录映射；`arm64-synthetic` 预设用临时假库出包并通过 `--verify` 与结构断言。arm64 真机编译/运行验证未做（无设备），CEF/JPEG 无 arm64 预编译包 |
| 2026-10-25 | P0-2 改为实施：`pack.py` 增加关键产物缺失阻断（核心头文件、`bin/resources/`、Windows manifest、显式请求的 SDL3 运行期 DLL / CEF 运行时 / 示例程序；基础库与 Skia 原本已失败），以退出码 1 拒绝出包；新增 `--allow-missing` 仅供排查降级，`--list` 不受影响。`test_pack.py` 增至 39 个用例，覆盖各关键项缺失、降级与真实仓库无缺失；`unit`、`arm64-synthetic` 与 `pack-msvc-x64-release` 预设复测通过 |
| 2026-10-25 | P1-3 Linux 已接入：`collect_cef` 收集 `bin/libcef_linux/` 运行时与头文件，`libcef.so` 不进 `lib/`，CMake 按绝对路径链接；`--cef-version 109` 仅 Windows；`test_pack.py` 新增 5 个 Linux CEF 用例（共 44 个），`test_autotest.py` 17 个通过 |
| 2026-10-25 | macOS CEF 文档补齐：`docs/Release.md` 与 `Release.en.md` 新增“macOS：下游手动配置 CEF”（CEF_ROOT 与分发包准备、构建命令、`MACOSX_BUNDLE`/`COPY_MAC_FRAMEWORK`/Helper/sandbox、`Contents/Resources/duilib` 资源布局、成品结构、签名公证与自检命令、常见问题表） |
| 2026-10-25 | arm64 libjpeg-turbo 接入：上游 arm64 预编译库入库后，`collect_jpeg_turbo()` 收集 `libjpeg-turbo6-win-vc-arm64/lib/turbojpeg-static.lib`，`JpegTurboSettings.props` 增加 ARM64 的 include/lib 目录；MANIFEST 的 arm64 说明改为仅 CEF 缺席，测试与 README/Checklist 同步；真机编译/运行验证仍未做 |
| 2026-10-25 | macOS 示例 Bundle 打包支持：`collect_examples()` 识别目录型 `bin/*.app`，逐文件收集（保留 `Contents/MacOS`、`Contents/Frameworks` 内的 CEF Framework/Helper 与原权限、符号链接）；`is_cef_example()` 支持 `.app`；是否随包与其他平台一致由 `--with-cef` 控制（未指定则跳过并告警）。macOS 的 `collect_cef()` 不再报错，只提示不收集 CEF 开发文件/运行时；`collect_libs()` 在 macOS 跳过 wrapper 校验；MANIFEST、`build-reference*.md`、`docs/Release*.md` 与 `test_pack.py` 同步 |
| 2026-10-25 | Linux x64 Release 编译与打包实测通过：`pack.py` 产出默认 SDK 包、`--with-cef`、`--with-examples` 三种组合；非 CEF 示例不再链接 `libcef.so`（新增示例级 `DUILIB_EXAMPLE_CEF`，仅 `cef`/`CefBrowser` 链接 CEF，Windows/macOS 同步）。未覆盖：Linux Debug、解包后示例启动运行 |
| 2026-10-25 | FreeBSD x64 Release 编译与打包实测通过（除 CEF 外与 Linux 覆盖相同）：`pack.py` 产出默认 SDK 包与 `--with-examples` 包，`--with-cef` 明确报错；FreeBSD 走 Linux/X11 分支（CM-10/CM-13 的 FreeBSD 部分已实机验证）。未覆盖：FreeBSD Debug、解包后示例启动运行 |
| 2026-10-28 | DLL 模式 turbojpeg 修正：`JpegTurboSettings.props` 在 `EnableDuilibDll=1` 时改用 `turbojpeg.lib` 导入库（不再用按 `/MT` 构建的 `turbojpeg-static.lib`，消除 LNK4098），并在 duilib 构建后把 `bin/turbojpeg.dll` 复制到 `bin/`；`DuilibSettings.props`/`BinCommonSettings.props` 的 Debug `/NODEFAULTLIB:libcmt.lib` 只保留给静态模式；`pack.py` 的 `collect_jpeg_turbo()` 改为 DLL 模式按 duilib DLL 是否实际引用 `turbojpeg.dll` 决定收集，DLL 包不再带静态库；`packaging/msvc/nim_duilib.targets` 校验 DLL 包必须有 `bin/turbojpeg.dll` |
| 2026-10-28 | DLL 模式入口与自动重建：`build.py` 新增 `--dll`（隐含 `--md`，自动准备 Skia `.md`/`.mdd`；Windows 上 SDL 默认关闭，`--dll` 也不自动开启，关闭时 `deps` 不下载、不构建 SDL3），`_cmake_defines` 显式传 `DUILIB_DLL`，并同步 `msvc/PropertySheets/duilib_dll.flag`；`out/<arch>/<Config>/.duilib-build-mode` 记录模式，静态↔DLL 切换时自动删除构建树（含旧 CEF wrapper `.obj`），DLL→静态还清理 `bin/duilib*.dll`；MSBuild 侧由 `CommonSettings.props` 的 `$(IntDir)duilib_crt_mode.txt` 触发同样的全量重编，不再需要手工删 `build_temp` |
| 2026-10-28 | DLL 包特性一致性：`pack.py` 从 duilib DLL 导出表探测 SDL（`WindowBase::GetVideoDriverName`），DLL 以 SDL 编译而打包未加 `--with-sdl` 时直接报错，避免下游 `DUILIB_SDL=0` 与 DLL 的 Control/WindowBase 类布局不一致；DLL 以 CEF 编译但未加 `--with-cef` 时告警。`test_pack.py` 增至 72 个用例 |
| 2026-10-28 | `sdk_package_test.py` Windows 长路径修复：解包时去掉归档顶层包目录（`root_name` 仅用于 P01/P04 校验），包工作目录改用 `01`/`02` 短名，避免 CEF 深头文件与 MSBuild `.tlog` 超过 MAX_PATH（`FileNotFoundError`/`MSB6003`）；`--out-dir` 与 `--keep` 语义同步写入 `sdk-package-test.md` |
| 2026-10-28 | Windows DLL 模式回归（第一轮）：`build.py --dll` x64 Debug（802/802）与 Release 全量示例构建通过，basic/ChildWindow 启动正常；出包 `nim_duilib-...-win-x64-sdl-cef-examples-dll[-debug].zip`（均 1396 条目，210.1/219.6 MB），`--verify` 通过，`sdk_package_test.py` 均 PASS=15 / SKIP=4 |
| 2026-10-28 | Windows DLL 模式回归（收尾）：x86 Debug DLL 802/802（`bin/duilib32_d.dll` 29,417,472 B）、x86 Release DLL（`duilib32.dll` 11,598,336 B）；x86 Debug/Release 包 1394 条目（191.6/183.4 MB）；MSBuild DLL 包 `win-x64-cef-dll` 1099 条目 / 200.2 MB，含 `bin/turbojpeg.dll`、无 `turbojpeg-static.lib`；修正 `infer_capabilities()` 对 DLL 包 `HAS_JPEG` 的误判（P04/C02），单测 207 项通过；五个 DLL 包均通过 `--verify` 与 SDK 测试（PASS=15 / SKIP=4）；回归后删除 `duilib_dll.flag`、`bin/duilib*.dll` 并恢复静态库基线 |
| 2026-10-28 | P2-5 Unix 共享库评估：新增 [Unix-Shared-Library-Feasibility.md](Unix-Shared-Library-Feasibility.md)，分析 Linux/macOS/FreeBSD 把 duilib 改为 `.so/.dylib` 的可行性、导出可见性/PIC/CEF 运行期依赖/rpath/打包改造点；结论为技术上可行但不等同于 Windows DLL，决定暂不实施，Unix 保持静态库 |
| 2026-10-29 | P2-4 arm64 补齐：`build.py --arch arm64 --dll`（Release/Debug）与 VS `duilib.sln` ARM64 DLL 模式产出 `duilib.dll`/`duilib_d.dll`（均 PE `AA64`）；CEF109 包 `-win-arm64-cef109-examples.zip`（1131 条目 / 252.1 MB）通过 `--verify` 与 `sdk_package_test.py`（PASS=14 / SKIP=5）；Windows on ARM 真机运行无设备，标记未测、不跟进 |
| 2026-10-29 | P2-3 完成：新增双语 `docs/Deployment.md` / `Deployment.en.md`（安装、升级、迁移、运行期部署与 FAQ），并入 `docs/Summary*.md` 索引，作为顶层文档随包收录；包内 `MANIFEST.txt` 仍是权威清单 |
| 2026-10-29 | libjpeg-turbo 3.2.x 静态库与 duilib-zlib 的 `z_errmsg` 符号冲突（LNK2005）：duilib-zlib 增加 `z_errmsg=duilib_z_errmsg` 编译宏（CMake 与 VS 两套构建），最小工程同链 `turbojpeg-static.lib` 验证通过 |
| 2026-10-29 | FreeBSD x64 全量实测通过：Release/Debug 构建、4 个出包（默认 SDK 与 `--with-examples`）、4 次 `sdk_package_test.py`（含两次 `--run-examples` 的 C10 示例启动）与全部脚本测试（`doctor`/`test`/`test_build`/`test_pack`/`test_sdk_package`/`test_autotest`/`autotest`）通过；`pack.py --with-cef` 按预期报错。此前“FreeBSD Debug 与示例运行未测、不跟进”的结论已撤销，P0-1/P0-3 的 FreeBSD 缺口关闭 |
| 2026-10-29 | 剩余跟进项收敛为 P3-1～P3-6（Windows 示例包 C10 冒烟、x86 SDL3 出包、脚本控制台英文化、`build/legacy/` 清理、`release.py` 全流程演练、许可合规遗留）；P0-3 改为完成；修正 P1-1 macOS 与 P1-3 Windows 的过期表述，建议推进顺序改为按 P3 排期 |
| 2026-10-29 | P3-4 完成：删除 `build/legacy/` 下 11 个旧分平台脚本（`build_duilib_all_in_one.*`、`msvc_build.bat`、`linux_build.sh`、`macos_build.sh`、`freebsd_build.sh`、`msys2_build.sh` 与 MinGW/VS 辅助脚本）；`README(.en).md`、`build/README(.en).md`、`build/build-reference(.en).md`、`CHANGELOG(.en).md` 与 `CMake-Improvement-Plan.md` 统一改为「已删除」，仓库中无残留引用 |
| 2026-10-29 | P3-1 部分完成：Windows 首次执行包内示例启动（C10）。x64 Release + CEF 示例包 `nim_duilib-1.0.2026.1023-win-x64-cef-examples.zip`（1318 条目 / 328.2 MB）`--verify` SHA 全匹配，`sdk_package_test.py --scenarios C10 --run-examples` 报 `[PASS] C10 examples.smoke`（`bin/basic.exe` 启动 3 秒后仍在运行，PASS=1 / FAIL=0 / SKIP=0）；SDL、DLL、无 CEF 三个变体待补 |
| 2026-10-29 | P3-1 完成：补齐 SDL、无 CEF 与 DLL 组合，连同前一轮 x64 CEF 包共 5 个示例包的 C10 全部通过 —— `win-x64-sdl-cef-examples.zip`（1398 条目 / 352.9 MB）、`win-x64-examples.zip`（764 / 121.8 MB）、`win-x64-sdl-cef-examples-dll.zip`（1398 / 210.1 MB）、`win-x64-sdl-cef-examples-dll-debug.zip`（1398 / 219.6 MB），四包 `--verify` SHA 全匹配、C10 均 `[PASS]`；DLL 回归后已恢复静态构建并自动清理 `bin/duilib*.dll` |
| 2026-10-29 | P3-2 完成：`build.py build --config Release --arch x86 --sdl`（786/786，SDL3 按 x86 重建、Skia `llvm.x86.release`）→ `pack.py --arch x86 --with-examples --with-sdl` 产出 `nim_duilib-1.0.2026.1023-win-x86-sdl-examples.zip`（844 条目 / 128.2 MB）；`--verify` 844 条目 SHA 全匹配，SDK 测试 PASS=16 / FAIL=0 / SKIP=3（含 C10 `bin/basic.exe` 启动）；W-B30/W-P27 回填后 P0-1 收口；测试后恢复 x64 静态基线并重建 x64 `SDL3-static.lib` |
| 2026-10-29 | P3-6 完成：`License-Compliance.md` 的 4 条遗留事项全部处理 —— 新增组件登记流程写入 `Release-Checklist.md`；确认 SDK 包固定 `DUILIB_LIB_PAG=0` 并写明启用 libpag 时须随程序分发 `duilib/third_party/libpag/licenses/**`（含 `tgfx.LICENSE.txt`）；`bin/resources` 的 409 个文件按来源分层记录（首提交 144 个上游资源、项目新增控件/示例资源、Lottie/PAG 演示素材），`loading5/6.json` 含 `LottieFiles AE` 元数据已在 `THIRD-PARTY-NOTICES.md` 增加演示素材说明；CEF 官方 109/142 包实测无 `LICENSES.chromium.html`，估算 10 MB 量级（zip 后 1–2 MB），决定默认不随包、需要时按同版本 Chrome/Chromium 手工补充 |
