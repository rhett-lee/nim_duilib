# 构建与打包测试矩阵

本文档汇总 nim_duilib 统一构建入口（`build/build.py`）与打包脚本（`build/pack.py`）的
全部测试组合，供后续版本回归测试直接参照。矩阵按平台、工具链、架构、配置和功能开关
划分，并记录当前验证状态。

- 更新日期：2026-10-30
- 适用版本：新构建系统（`out/<arch>/<Config>` + `lib/<arch>/<Config>` + `bin/`）
- 命令示例统一使用 `python3 build/build.py`
- 每个组合出包后都必须执行 `pack.py --verify <包>` 校验条目数、文件 SHA 与文档链接

## 状态标记

| 标记 | 含义 |
| :--- | :--- |
| 通过 | 2026-10-26 ~ 30 本轮实际执行并通过 |
| 历史通过 | 新构建系统验证阶段（2026-10-23 ~ 25）执行并通过，本轮未复测 |
| 部分 | 已执行但存在已知限制，或仅完成构建/打包中的一项 |
| 未测 | 尚未验证 |
| 不安排 | 明确不纳入本轮测试（如无对应真机环境），保留编号备查 |
| 不适用 | 项目不支持该组合 |

## 0. 测试环境基线

| 项目 | 本轮环境 |
| :--- | :--- |
| 操作系统 | Windows 10/11 x64 |
| Visual Studio | 2022 17.14.37710.0（MSVC 14.44.35207：x64/x86 + ARM64 交叉工具 `Hostx64\arm64`；Windows SDK 10.0.26100.0 含 arm64 库） |
| CMake / Ninja | CMake 4.3.0，Ninja（VS 自带） |
| Python | 3.14.4 |
| MSYS2 | `E:\msys64`（MINGW64、UCRT64、CLANG64；MINGW32 缺 gcc/make，未测、不跟进） |
| 独立 MinGW | `C:\mingw64\llvm-mingw-20250430-ucrt-x86_64` |
| Skia | `e46ba84356043800523c401eaec9ecfd5cb67539` |
| SDL3 | `main`（`release-3.4.18` tag 源码编译报错，已不再作为默认） |
| CEF | 默认 156（`156.0.14+g4fa4ab5+chromium-156.0.8078.48`）；109 可选（Windows 7） |

Linux、FreeBSD 与 macOS 的本轮环境记录在各章开头（§3、§4、§5）。

MinGW 环境变量模板（每次新终端都要设置）：

```powershell
$env:MSYSTEM='MINGW64'
$env:PATH='E:\msys64\mingw64\bin;' + $env:PATH
$env:CC='E:\msys64\mingw64\bin\gcc.exe'
$env:CXX='E:\msys64\mingw64\bin\g++.exe'
```

独立 llvm-mingw 还需把 `E:\msys64\mingw64\bin` 放在 PATH 中提供 ninja/make，
`CC`/`CXX` 仍指向 llvm-mingw 的编译器。

## 1. Windows 构建矩阵

| 编号 | 工具链 | 架构/配置 | 关键开关 | 范围 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W-B01 | MSVC | x64 / Debug | 默认（/MT、CEF 142、无 SDL） | 库 + 全部示例 | `python3 build/build.py build --config Debug` | 通过 | 861/861；过滤其他平台入口后增量构建为 21 次链接，新生成的 exe 均写入 `bin/.build-config/` |
| W-B02 | MSVC | x64 / Debug | `--cef-version 109` | 库 + 全部示例 | `... --config Debug --cef-version 109` | 通过 | 838/838，109 目标更少 |
| W-B03 | MSVC | x86 / Debug | 默认 | 库 + 全部示例 | `... --config Debug --arch x86` | 通过 | 861/861 |
| W-B04 | MSVC | x64 / Release | 默认 | 库 + 全部示例 | `... --config Release` | 通过 | 861/861 |
| W-B05 | MSVC | x86 / Release | 默认 | 库 + 全部示例 | `... --config Release --arch x86` | 通过 | 861/861 |
| W-B06 | MSVC | x64 / Release | `--cef-version 109` | 库 + 全部示例 | `... --config Release --cef-version 109` | 通过 | 537/537 |
| W-B07 | MSVC | x64 / Debug | `--md --sdl` | 库 + 全部示例 | `... --config Debug --md --sdl` | 通过 | 861/861；单配置生成器只构建/安装 Debug 版 SDL3（`/MDd`） |
| W-B08 | MSVC | x64 / Release | `--md` | 库 + 全部示例 | `... --config Release --md` | 通过 | 861/861（Skia 用 `.md` 变体） |
| W-B09 | MSVC | x64 / Release | `--sdl`（/MT） | 库 + 全部示例 | `... --config Release --sdl` | 通过 | 763/763，示例启动验证通过 |
| W-B10 | MSVC | x64 / Release | 关闭 CEF | 库 + 非 CEF 示例 | `... --config Release --cmake-arg=-DDUILIB_ENABLE_CEF=OFF` | 通过 | 328/328，21 个示例；缓存中残留的 cef/CefBrowser 已自动剔除 |
| W-B11 | MSVC | x64 / Debug | `--target duilib --no-examples --lang en` | 仅库 | `... --config Debug --target duilib --no-examples --lang en` | 通过 | 307/307，英文提示生效 |
| W-B12 | MSYS2 gcc | x64 / Release | 默认（CEF 不支持） | 库 + 非 CEF 示例 | `... --config Release --build-dir <临时目录>` | 通过 | 571/571 |
| W-B13 | llvm-mingw | x64 / Release | 默认 | 库 + 非 CEF 示例 | 同上，`CC/CXX` 指向独立 llvm-mingw | 通过 | 571/571，basic.exe 冒烟启动正常 |
| W-B14 | MSYS2 gcc | x64 / Release | `--sdl` | 库 + 非 CEF 示例 | `... --config Release --sdl` | 通过 | 571/571，SDL3 按 gcc 重建（CRT stamp 触发） |
| W-B15 | MSYS2 gcc | x64 / Release | `-DDUILIB_MINGW_STATIC=OFF` | 库 + 非 CEF 示例 | `... --config Release --cmake-arg=-DDUILIB_MINGW_STATIC=OFF --jobs 3` | 通过 | 571/571；objdump 确认示例依赖 libstdc++-6/libgcc_s_seh-1/libwinpthread-1.dll |
| W-B16 | MSYS2 gcc/llvm | x64 / Debug | 默认 | 库 | `... --config Debug --target duilib --no-examples` | 通过 | MINGW64 本轮实测 285/285（libduilib.a 333.2 MB）；UCRT64/CLANG64 历史通过；示例未复测 |
| W-B17 | MSYS2 MINGW32 | x86 / Release | 默认 | 库 + 示例 | — | 未测（不跟进） | 环境缺 gcc/make，且无跟进计划；编号保留备查 |
| W-B18 | MSVC | arm64 / Release | 默认 | 库 + 示例 | `... --arch arm64` | 通过 | 25/25 目标、21 个示例链接完成；`bin/*.exe` 23 个与 `lib/arm64/Release` 6 个库均为 PE `0xAA64`；Skia 由 `deps` 自动构建 `skia/out/llvm.arm64.release`；ARM64 无 WGL，Skia 渲染走 CPU（见本节修复） |
| W-B18b | MSVC | arm64 / Debug | 默认 | 库 + 示例 | `... --arch arm64 --config Debug` | 通过 | `deps` 自动构建 `skia/out/llvm.arm64.debug`（1078 目标，约 6 分钟）；802/802 目标、23 个示例链接完成；`bin/*.exe` 均为 `0xAA64`，嵌入清单为 `processorArchitecture="ARM64"`；`lib/arm64/Debug` 6 个库 409.4 MB（`duilib.lib` 254 MB） |
| W-B19 | MSVC | x64 / Debug | `--dry-run` | 仅打印命令 | `... --config Debug --dry-run` | 通过 | 不执行配置/编译，不输出“编译完成” |
| W-B20 | MSVC | x64 / Debug | 复用构建目录 | 有状态选项回归 | 先 `--no-examples --md --sdl`，再默认构建 | 通过 | 默认值显式传入，不继承旧选项 |
| W-B21 | MSVC | x64 / Debug | `--dll` | 库 + 全部示例 | `... build --dll --config Debug` | 通过 | 802/802；`bin/duilib_d.dll` 36,225,024 B；basic/ChildWindow 启动正常（窗口标题正确） |
| W-B22 | MSVC | x64 / Release | `--dll` | 库 + 全部示例 | `... build --dll --config Release` | 通过 | 24 个增量链接；`bin/duilib.dll` 13,598,720 B、`lib/x64/Release/duilib.lib` 导入库；basic/ChildWindow 启动正常 |
| W-B23 | MSVC | x86 / Debug | `--dll` | 库 + 全部示例 | `... build --dll --config Debug --arch x86` | 通过 | 日志含 `mt → dll` 自动清理；802/802；`bin/duilib32_d.dll` 29,417,472 B、`lib/x86/Debug/duilib.lib` 导入库；basic/ChildWindow 启动正常（Debug CRT 14.44 已在 SysWOW64） |
| W-B24 | MSVC | x86 / Release | `--dll` | 库 + 全部示例 | `... build --dll --config Release --arch x86` | 通过 | 802/802；`bin/duilib32.dll` 11,598,336 B、`lib/x86/Release/duilib.lib` 导入库；basic/ChildWindow 用 app-local CRT 启动正常 |
| W-B25 | MSVC | x64 / Debug ↔ Release | 静态 ↔ DLL 模式切换 | 模式切换自动清理 | 依次 `build --config Debug`、`build --dll --config Debug`、再静态 | 通过 | 日志含 `dll → mt`，构建树重建、`bin/duilib_d.dll` 删除、`duilib_dll.flag` 同步为 OFF；307/307 得到 `lib/x64/Debug/duilib.lib`（256,775,712 B 静态库）。反向 `mt → dll` 见 W-B23 |
| W-B26 | MSBuild | x64 / Release | `duilib_dll.flag` | duilib + 第三方 + wrapper | 先无 flag 构建 `build/duilib.sln`，再创建 flag 重建 | 通过 | 静态库 870,484,530 B；加 flag 后 6 个工程 `duilib_crt_mode.txt` 变为 `dll`，wrapper/109/第三方与 duilib 全部重编；无 LNK4098/LNK2038；`bin/duilib.dll` 17,366,016 B，导入 `turbojpeg.dll`、`MSVCP140.dll`、`libcef.dll`，导入库 4,349,026 B，`bin/turbojpeg.dll` 自动复制 |
| W-B27 | MSBuild | arm64 / Release + Debug | VS 解决方案 ARM64 配置（4 个 `.sln` 及 30 个工程） | 库 + 23 个示例 | `msbuild build\duilib.sln /p:Configuration=Release /p:Platform=ARM64 /m`；`msbuild build\examples.sln /p:Configuration=Release /p:Platform=ARM64 /m:2`（Debug 相同） | 通过 | Release：`lib/arm64/Release` 7 个库（`duilib.lib` 828.5 MB，LTCG）、23 个示例全部链接；Debug：`lib/arm64/Debug` 7 个库、`bin/*_d.exe` 23 个；Release/Debug 共 46 个 exe 均 PE `0xAA64`、清单 `processorArchitecture="ARM64"`；4 个 `.sln` 的 ARM64 配置经 `ValidateSolutionConfiguration` 与映射检查（30/30、24/24、7/7、5/5）；`duilib_no_cef.sln` / `examples_no_cef.sln` 的 Release/Debug 也已实际编译通过，各 19 个示例 |
| W-B28 | MSVC | arm64 / Release + Debug | `--dll` | 库 + 全部示例 | `python3 build/build.py --arch arm64 --dll`（Debug 加 `--config Debug`） | 通过 | `deps` 自动构建 `llvm.arm64.release.md` / `debug.mdd`；`bin/duilib.dll` 9.9 MB、`duilib_d.dll` 36 MB 均为 PE `0xAA64`；导入库 4.15 / 4.17 MB；46 个 exe 全 `0xAA64` |
| W-B29 | MSBuild | arm64 / Release + Debug | `duilib_dll.flag` | duilib + 第三方 + wrapper | 创建 `msvc/PropertySheets/duilib_dll.flag` 后 `msbuild build\duilib.sln /p:Configuration=Release /p:Platform=ARM64 /m:2`（Debug 同） | 通过 | Release `bin/duilib.dll` 17.3 MB（LTCG）、Debug `bin/duilib_d.dll` 27.3 MB，均为 PE `0xAA64`；导入库 `lib/arm64/<Config>/duilib.lib` 4.15 / 4.17 MB；Debug 首次因 E: 盘写满报 `C2471`/`FTK1005`，清理中间产物与 PDB 后重跑 0 error；flag 已删除 |
| W-B30 | MSVC | x86 / Release | `--sdl` | 库 + 全部示例 | `python3 build/build.py build --config Release --arch x86 --sdl` | 通过 | 786/786；Skia 用 `llvm.x86.release`；SDL3 按 x86 重建并安装到 `../SDL3`（stamp 含架构，x64 下次 `--sdl` 会自动重建）；`bin/` 23 个示例均为 PE `0x014C` |

构建矩阵执行要点：

- 在同一个 `out/<arch>/<Config>` 中切换工具链会被自动清理；但 `lib/<arch>/<Config>`、
  `bin/` 不会自动清理，切换工具链或架构前需手工清理，否则打包会报混合库/架构不一致。
- 手工清理 `bin/` 时只删 `*.exe`、`*.dll` 与 `.build-config/` 目录（升级前旧构建
  可能在根目录留下 `*.build-config` 文件，一并删除）；**`bin/resources.zip` 是仓库
  跟踪的 748 字节占位文件**（示例 `.rc` 通过 `IDR_THEME THEME "..\..\bin\resources.zip"`
  内嵌引用），误删会导致 windres 报 `can't open file '..\..\bin\resources.zip'`，
  需 `git checkout -- bin/resources.zip` 恢复。
- `--md`、`--sdl`、`--no-examples`、`--cef-version` 等是有状态选项，`build.py` 每次都会
  显式传入解析后的值；直接调用 `cmake` 复用目录时才需要手工清缓存。
- `--dll`（Windows MSVC）隐含 `--md`；`build.py` 会自动准备 Skia 的 `.md`
  （Release）/.mdd（Debug）变体，无需手工转运行库。SDL 在 Windows 上默认关闭，
  `--dll` 也不自动开启；关闭时 `deps` 不下载 SDL 源码、不构建 SDL3，需要时显式
  `--sdl`。
- 本轮 DLL 回归结束后恢复静态基线：删除 `msvc/PropertySheets/duilib_dll.flag`、
  `bin/duilib*.dll` 及对应 `.pdb/.ilk`，并用测试前备份覆盖
  `lib/x64/Release`、`lib/x86/Release`、`lib/x86/Debug`；随后
  `python3 build/build.py build --config Debug` 重新链接 x64 Debug 静态示例
  （basic/ChildWindow 启动正常，`bin/` 中无 `duilib*.dll`）。
- 静态 ↔ DLL 模式切换时，`build.py` 读取 `out/<arch>/<Config>/.duilib-build-mode`，
  模式变化会删除整个构建树（含 `libcef_dll_wrapper` 旧 `.obj`）并重新配置；
  DLL → 静态还会清理 `bin/duilib*.dll`。MSBuild 侧对应
  `$(IntDir)duilib_crt_mode.txt`，切换 `duilib_dll.flag` 后会自动删除旧
  `.obj/.pch`，不必手工清理 `build_temp`。
- MinGW Debug 示例单个体积约 200–280 MB，全部示例约 10 GB；日常回归优先用 Release。
- MinGW 编译/链接内存不足时用 `--jobs 2` 或 `--jobs 3`，默认并行度可能触发
  `cc1plus/ld out of memory`。
- `DUILIB_MINGW_STATIC=OFF` 时示例依赖 `libstdc++-6.dll`、`libgcc_s_seh-1.dll`
  （x86 为 `libgcc_s_dw2-1.dll`）、`libwinpthread-1.dll`，构建不会自动复制到 `bin/`。

## 2. Windows 打包矩阵

打包前置：确认 `lib/<arch>/<Config>` 只有一种工具链的库（MSVC `.lib` 或 MinGW `.a`），
`bin/` 中只有目标架构的示例（Windows 各架构共用 `bin/`）。

| 编号 | 内容组合 | 命令示例 | 结果（条目 / 大小） | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| W-P01 | x64 Debug + 示例 + CEF 142 | `python3 build/build.py pack --config Debug --with-examples --with-cef` | 1316 / 412.3 MB | 通过 |
| W-P02 | x64 Debug + 示例 + CEF 109 | `... --config Debug --with-examples --with-cef --cef-version 109` | 1134 / 373.5 MB | 通过 |
| W-P03 | x86 Debug + 示例 + CEF 142 | `... --config Debug --arch x86 --with-examples --with-cef` | 1314 / 368.1 MB | 通过 |
| W-P04 | x64 Release + 示例 + CEF 142 | `... --config Release --with-examples --with-cef` | 1318 / 328.2 MB | 通过（含 C10） |
| W-P05 | x86 Release + 示例 + CEF 142 | `... --config Release --arch x86 --with-examples --with-cef` | 1314 / 287.8 MB | 通过 |
| W-P06 | x64 Release + 示例 + CEF 109 | `... --config Release --with-examples --with-cef --cef-version 109` | 1134 / 280.8 MB | 通过 |
| W-P07 | x64 Debug + 示例 + SDL + MD | `... --config Debug --md --with-examples --with-sdl --with-cef` | 1396 / 416.5 MB | 通过 |
| W-P08 | x64 Release + 示例 + MD | `... --config Release --md --with-examples --with-cef` | 1316 / 304.2 MB | 通过 |
| W-P09 | x64 Release + 示例 + SDL | `... --config Release --with-examples --with-sdl --with-cef` | 1398 / 352.9 MB | 通过（含 C10） |
| W-P10 | x64 Release + 示例，无 CEF | `... --config Release --with-examples` | 764 / 121.8 MB | 通过（含 C10） |
| W-P11 | x64 Debug，仅库无示例 | `... --config Debug` | 546 / 63.5 MB | 通过 |
| W-P12 | MinGW gcc Release + 示例（无 CEF） | `... --config Release --with-examples` | 757 / 208.2 MB | 通过 |
| W-P13 | llvm-mingw Release + 示例（无 CEF） | 同上（MinGW 环境） | 757 / 147.0 MB | 通过 |
| W-P14 | MinGW gcc Release + 示例 + SDL（无 CEF） | `... --config Release --with-examples --with-sdl` | 837 / 243.0 MB | 通过 |
| W-P15 | MinGW 动态链接 Release + 示例（无 CEF） | `... --config Release --with-examples`（源码关闭静态链接） | 757 / 195.4 MB | 通过，有限制 |
| W-P16 | x64 Debug，zip / tgz / tar.bz2 | `pack.py --config Debug --format zip\|tgz\|tarbz2` | 546 / 63.5 MB 各一份 | 通过 |
| W-P17 | 显式 MSVC 运行库 MT / MD | `pack.py --msvc-runtime mt` / `md` | 元数据与库一致 | 通过 |
| W-P18 | x64 Debug DLL + 示例 + SDL + CEF | `pack.py --config Debug --with-examples --with-sdl --with-cef` | 1398 / 219.6 MB | 通过（含 C10） |
| W-P19 | x64 Release DLL + 示例 + SDL + CEF | `pack.py --config Release --with-examples --with-sdl --with-cef` | 1398 / 210.1 MB | 通过（含 C10） |
| W-P20 | x86 Debug DLL + 示例 + SDL + CEF | `pack.py --config Debug --arch x86 --with-examples --with-sdl --with-cef` | 1394 / 191.6 MB | 通过 |
| W-P21 | x86 Release DLL + 示例 + SDL + CEF | `pack.py --config Release --arch x86 --with-examples --with-sdl --with-cef` | 1394 / 183.4 MB | 通过 |
| W-P22 | MSBuild DLL（动态 turbojpeg，无 SDL） | MSBuild 构建后 `pack.py --config Release`（不加 `--with-sdl`） | 1099 / 200.2 MB | 通过 | 包名 `win-x64-cef-dll`；`JPEG=1`、`SDL3=0`；含 `bin/duilib.dll` + `bin/turbojpeg.dll`，无 `turbojpeg-static.lib`；`--verify` 与 SDK 测试（PASS=15 FAIL=0 SKIP=4）通过；修复 P04/C02 对 DLL 包 `HAS_JPEG` 的误判 |
| W-P23 | arm64 Release + 示例（无 CEF） | `pack.py --arch arm64 --with-examples` | 760 / 109.0 MB | 通过 | `--verify` 760 条目全匹配；SDK 测试 PASS=14 FAIL=0 SKIP=5（C01 配置/编译/链接通过，因本机 x64 无法运行 arm64 程序而 SKIP） |
| W-P24 | arm64 Release + 示例 + CEF 142 | `pack.py --arch arm64 --with-examples --with-cef` | 1314 / 311.7 MB | 通过 | CEF 开发文件 288 项 + 运行时 238 项；包内 `bin/libcef_win/arm64/libcef.dll` 为 PE `0xAA64`，三份 manifest 齐全；`--verify` 1314 条目全匹配；SDK 测试 PASS=14 FAIL=0 SKIP=5 |
| W-P25 | arm64 Release + 示例 + CEF 109 | `pack.py --arch arm64 --with-examples --with-cef --cef-version 109` | 1131 / 252.1 MB | 通过 | 包内 `bin/libcef_win_109/arm64/libcef.dll` 为 `0xAA64`（165.5 MB）；开发库仅 `lib/libcef_dll_wrapper_109.lib`（119.2 MB），无 142 混入；`--verify` 1131 条目全匹配；SDK 测试 PASS=14 FAIL=0 SKIP=5（C01 按架构 SKIP）；WebView2 无 arm64 loader，按预期跳过 |
| W-P26 | arm64 Debug + 示例 + CEF 142 | `pack.py --arch arm64 --config Debug --with-examples --with-cef` | 1314 / 394.5 MB | 通过 | Skia 目录 `llvm.arm64.debug`；CEF 288 + 运行时 238；`--verify` 1314 条目全匹配；SDK 测试 PASS=14 FAIL=0 SKIP=5（C01 按架构 SKIP）；WebView2 无 arm64 loader，按预期跳过 |
| W-P27 | x86 Release + 示例 + SDL（无 CEF） | `pack.py --config Release --arch x86 --with-examples --with-sdl` | 844 / 128.2 MB | 通过（含 C10） | 2026-10-29 补齐 x86 SDL 组合：SDL3 79 项、21 个示例、Skia `llvm.x86.release`；`--verify` 844 条目 SHA 全匹配；`sdk_package_test.py --run-examples` PASS=16 FAIL=0 SKIP=3（C10 通过，C02/C06/C11 按平台跳过）；未加 `--with-cef`，包内 CEF 库与 `cef.exe`/`CefBrowser.exe` 按预期跳过 |

打包专项说明：

- W-P15：关闭静态链接后示例依赖 MinGW 运行库，当前构建与打包不会自动收集这些 DLL；
  分发前需手工复制到 exe 同级。包内 README 已提示。
- W-P18 ~ W-P21：DLL 包由 `build.py build --dll`（CMake `DUILIB_DLL=ON`）产物打出，
  不再依赖历史/外部 DLL 包。包名带 `-dll`，元数据 `IS_DLL=1`、`MSVC_RUNTIME=MD`，
  `lib/` 只要求 `duilib.lib` 导入库。`pack.py` 通过 duilib DLL 导出表判断 SDL
  支持，DLL 以 SDL 编译时必须加 `--with-sdl`，否则直接报错（避免下游
  `DUILIB_SDL=0` 与 DLL 的 Control/WindowBase 类布局不一致）。
- W-P18 ~ W-P21 四个 DLL 包均已通过 `pack.py --verify` 与 `sdk_package_test.py`
  （PASS=15 FAIL=0 SKIP=4：C02 覆盖 DLL 消费者，C01/C06/C11 按平台或参数跳过）。
  其中 x64 的 W-P18/W-P19 已于 2026-10-29 追加执行 C10（`--run-examples`）并通过；
  x86 的 W-P20/W-P21 未执行 C10（本轮不带 `--run-examples`）。
  x86 消费者使用 `/MD` 时由测试脚本把 VS 2022 运行库复制到 exe 同级，
  避免本机 `SysWOW64` 旧 `msvcp140.dll`（14.29）导致崩溃。
- DLL 包的 JPEG 规则：CMake 构建当前不启用 libjpeg-turbo，包内 `JPEG=0`；
  MSBuild 构建默认启用，DLL 模式随包 `bin/turbojpeg.dll`（导入库
  `turbojpeg.lib`），`pack.py` 只在 duilib DLL 实际引用 `turbojpeg.dll` 时收集，
  不会再收集按 `/MT` 构建的 `turbojpeg-static.lib`。
- DLL 包的 SDL3/Skia 已静态链接进 `duilib.dll`；包内 SDL3 头文件与库只用于
  下游编译宏与（静态模式）链接，DLL 消费者只需链接 `duilib.lib`。
- `--with-examples` 未加 `--with-cef` 时会跳过 `cef.exe`/`CefBrowser.exe`，避免包内
  留下必然启动失败的二进制。
- Windows 示例包的**包内启动冒烟（C10）**：2026-10-29 首次执行并覆盖 6 个示例包，
  全部为 `[PASS] C10 examples.smoke`（`bin/basic.exe` 启动 3 秒后仍在运行）：
  - x64 Release + CEF：`nim_duilib-1.0.2026.1023-win-x64-cef-examples.zip`
    （1318 条目 / 328.2 MB，`--verify` 全匹配）；
  - x64 Release + SDL + CEF：`...-win-x64-sdl-cef-examples.zip`
    （1398 条目 / 352.9 MB，`--verify` 全匹配）；
  - x64 Release 无 CEF：`...-win-x64-examples.zip`
    （764 条目 / 121.8 MB，`--verify` 全匹配）；
  - x64 Release DLL + SDL + CEF：`...-win-x64-sdl-cef-examples-dll.zip`
    （1398 条目 / 210.1 MB，`duilib.dll` 13,602,816 B，`--verify` 全匹配）；
  - x64 Debug DLL + SDL + CEF：`...-win-x64-sdl-cef-examples-dll-debug.zip`
    （1398 条目 / 219.6 MB，`duilib_d.dll` 36,236,800 B，`--verify` 全匹配）；
  - x86 Release + SDL（无 CEF，W-P27）：`nim_duilib-1.0.2026.1023-win-x86-sdl-examples.zip`
    （844 条目 / 128.2 MB，SDL3 79 项 + 21 个示例，`--verify` 全匹配，SDK 测试 PASS=16）。
  均以 `sdk_package_test.py --scenarios C10 --run-examples` 执行（PASS=1 / FAIL=0 / SKIP=0）。
- 包内条目数较本表历史值 +2：2026-10-29 起 `docs/Deployment*.md` 随包收录，
  同一组合的文档条目从 32 增至 34。
- 包名模式：`nim_duilib-<版本>-win-<arch>[-sdl][-cef|-cef109]-examples[-dll][-debug].zip`。

## 3. Linux 构建与打包矩阵

Unix 平台一次只构建一个配置，库输出到 `lib/<Config>`；`--md` 仅 MSVC 有效。

本轮环境：Linux x64（VMware 虚拟机）、CMake 3.31、Ninja、gcc（另验证 clang）；
`/tmp` 为 1.9 GiB tmpfs 且已写满，包测试改将临时目录放到 `--out-dir` 下（见 §7 修复表）。

2026-10-28 Linux 虚拟机复测：Release/Debug 构建、6 个出包、8 次包测试（含两次
`--run-examples`），以及 `build.py doctor/test`、`test_build.py`、`test_pack.py`、
`test_sdk_package.py`、`test_autotest.py`、`autotest.py` 全部通过（命令见 3.3）。

### 3.1 构建

| 编号 | 平台 / 工具链 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| L-B01 | Linux gcc | x64 / Release | 默认（SDL 默认开启） | `python3 build/build.py --config Release` | 通过 | 2026-10-28 虚拟机；`python3 build/build.py` 同义 |
| L-B02 | Linux clang | x64 / Release | 默认 | `CC=clang CXX=clang++ python3 build/build.py` | 通过 | 2026-10-28 虚拟机；仅验证构建，未做 clang 出包与包测试（预检与 Skia 工具链优先 clang） |
| L-B03 | Linux | x64 / Debug | 默认 | `python3 build/build.py --config Debug` | 通过 | 2026-10-28 虚拟机；切换配置后重新链接 |
| L-B04 | Linux | arm64 / Release | 默认 | `--arch arm64` | 不安排 | 无 arm64 真机（2026-10-28 决策：arm64 只测 Windows） |
| L-B05 | Linux | x64 / Release | `--with-cef` 打包 | `pack --with-examples --with-cef` | 通过 | 2026-10-28 虚拟机；CEF 示例包生成并通过包测试 |
| L-B06 | Linux | x64 / Release | 运行验证 | 解包运行示例（C10） | 通过 | `--run-examples` 启动 `bin/basic` 3 秒后仍在运行 |

### 3.2 打包与包测试

包名统一为 `nim_duilib-1.0.2026.1023-linux-x64[-examples[-cef]][-debug].tar.gz`；
所有包测试都带 `--out-dir ./temp`（系统 `/tmp` 空间不足时的显式落盘位置）。

| 编号 | 包 | 生成命令 | 包测试 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| L-P01 | Release 仅库 | `python3 build/pack.py` | `--package ...-linux-x64.tar.gz` | 通过 |
| L-P02 | Release 示例 | `python3 build/pack.py --with-examples` | `--package ...-linux-x64-examples.tar.gz` | 通过 |
| L-P03 | Release CEF 示例 | `python3 build/pack.py --with-examples --with-cef` | `--package ...-linux-x64-cef-examples.tar.gz` | 通过 |
| L-P04 | Debug 仅库 | `python3 build/pack.py --config Debug` | `--package ...-linux-x64-debug.tar.gz` | 通过 |
| L-P05 | Debug 示例 | `python3 build/pack.py --config Debug --with-examples` | `--package ...-linux-x64-examples-debug.tar.gz` | 通过 |
| L-P06 | Debug CEF 示例 | `python3 build/pack.py --config Debug --with-examples --with-cef` | `--package ...-linux-x64-cef-examples-debug.tar.gz` | 通过 |
| L-T01 | Release 示例 + 启动示例 | — | `--package ...-linux-x64-examples.tar.gz --run-examples` | 通过：C10 `examples.smoke` |
| L-T02 | Debug 示例 + 启动示例 | — | `--package ...-linux-x64-examples-debug.tar.gz --run-examples` | 通过：C10 `examples.smoke` |

Linux 上 C02（DLL）、C04（架构）、C05/C06（MSVC 工具链）、C11（macOS Bundle）按平台
SKIP，属预期。

### 3.3 2026-10-28 Linux 虚拟机复测命令（全部通过）

| 序号 | 命令 | 覆盖 |
| ---: | :--- | :--- |
| 1 | `python3 build/build.py doctor` | 工具链 / 依赖 / 预检 |
| 2 | `python3 build/build.py test` | 构建与打包脚本单元测试 |
| 3 | `python3 build/test_build.py` | 构建脚本测试 |
| 4 | `python3 build/build.py` | Release 全量构建（L-B01） |
| 5 | `python3 build/pack.py` | Release 仅库出包（L-P01） |
| 6 | `python3 build/pack.py --with-examples` | Release 示例出包（L-P02） |
| 7 | `python3 build/pack.py --with-examples --with-cef` | Release CEF 示例出包（L-P03、L-B05） |
| 8 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64.tar.gz --out-dir ./temp` | L-P01 |
| 9 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-examples.tar.gz --out-dir ./temp` | L-P02 |
| 10 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-cef-examples.tar.gz --out-dir ./temp` | L-P03 |
| 11 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-examples.tar.gz --run-examples --out-dir ./temp` | L-T01（C10） |
| 12 | `python3 build/build.py --config Debug` | Debug 全量构建（L-B03） |
| 13 | `python3 build/pack.py --config Debug` | Debug 仅库出包（L-P04） |
| 14 | `python3 build/pack.py --config Debug --with-examples` | Debug 示例出包（L-P05） |
| 15 | `python3 build/pack.py --config Debug --with-examples --with-cef` | Debug CEF 示例出包（L-P06） |
| 16 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-debug.tar.gz --out-dir ./temp` | L-P04 |
| 17 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-examples-debug.tar.gz --out-dir ./temp` | L-P05 |
| 18 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-cef-examples-debug.tar.gz --out-dir ./temp` | L-P06 |
| 19 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-linux-x64-examples-debug.tar.gz --run-examples --out-dir ./temp` | L-T02（C10） |
| 20 | `python3 build/test_pack.py` | 打包脚本测试 |
| 21 | `python3 build/test_sdk_package.py` | 包测试脚本测试 |
| 22 | `python3 build/test_autotest.py` | 自动化脚本测试 |
| 23 | `python3 build/autotest.py` | 自动化（默认 `unit` 预设） |
| 24 | `CC=clang CXX=clang++ python3 build/build.py` | clang Release 构建（L-B02） |

复测结论：上述命令全部通过，无遗留问题。`--out-dir ./temp` 是必要的：该机 `/tmp` 为
1.9 GiB tmpfs 且已写满，旧版脚本会把编译器临时文件写到 `/tmp`，导致 C01/C03 误报
「C++ 编译器不可用」；当前版本已把 `TMPDIR` 指到输出目录下（见 §7 修复表）。

## 4. FreeBSD 构建与打包矩阵

FreeBSD 复用 Linux/X11 分支；一次只构建一个配置，库输出到 `lib/<Config>`。
不支持 CEF，`--with-cef` 直接报错。

本轮环境：FreeBSD x64 虚拟机、系统 clang、cmake、ninja、`python3`；Skia 为
`llvm.x64.release`（Unix 侧只构建一份 release 版，Debug 构建共用）。

2026-10-29 FreeBSD 虚拟机复测：Release/Debug 构建、4 个出包、4 次包测试（含两次
`--run-examples`），以及 `build.py doctor/test`、`test_build.py`、`test_pack.py`、
`test_sdk_package.py`、`test_autotest.py`、`autotest.py` 全部通过（命令见 4.3）；
`pack.py --with-cef` 按预期报错（CEF 不支持 FreeBSD）。

### 4.1 构建

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| F-B01 | x64 / Release | 默认（SDL 默认开启） | `python3 build/build.py --config Release` | 通过 | 2026-10-29 虚拟机；`python3 build/build.py` 同义 |
| F-B02 | x64 / Debug | 默认 | `--config Debug` | 通过 | 2026-10-29 虚拟机；切换配置后重新链接 |
| F-B03 | x64 / Release | `--with-cef` | `python3 build/pack.py --with-cef` | 通过（负向） | 按预期报错：FreeBSD 上游无 Chromium 构建，不支持 CEF |
| F-B04 | x64 / Release + Debug | 运行验证 | 解包运行示例（C10） | 通过 | `--run-examples` 启动 `bin/basic` 3 秒后仍在运行 |

### 4.2 打包与包测试

包名统一为 `nim_duilib-1.0.2026.1023-freebsd-x64[-examples][-debug].tar.gz`；
包测试均带 `--out-dir ./temp`（与 Linux 相同的显式落盘位置）。

| 编号 | 包 | 生成命令 | 包测试 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| F-P01 | Release 仅库 | `python3 build/pack.py` | `--package ...-freebsd-x64.tar.gz` | 通过 |
| F-P02 | Release 示例 | `python3 build/pack.py --with-examples` | `--package ...-freebsd-x64-examples.tar.gz` | 通过 |
| F-P03 | Debug 仅库 | `python3 build/pack.py --config Debug` | `--package ...-freebsd-x64-debug.tar.gz` | 通过 |
| F-P04 | Debug 示例 | `python3 build/pack.py --config Debug --with-examples` | `--package ...-freebsd-x64-examples-debug.tar.gz` | 通过 |
| F-T01 | Release 示例 + 启动示例 | — | `--package ...-freebsd-x64-examples.tar.gz --run-examples` | 通过：C10 `examples.smoke` |
| F-T02 | Debug 示例 + 启动示例 | — | `--package ...-freebsd-x64-examples-debug.tar.gz --run-examples` | 通过：C10 `examples.smoke` |

FreeBSD 上 C02（DLL）、C04（架构）、C05/C06（MSVC 工具链）与 C11（macOS Bundle）
按平台 SKIP，与 Linux 一致，属预期；此前 FreeBSD 无 CEF，CEF 相关用例同样跳过。

### 4.3 2026-10-29 FreeBSD 虚拟机复测命令（全部通过）

| 序号 | 命令 | 覆盖 |
| ---: | :--- | :--- |
| 1 | `python3 build/build.py doctor` | 工具链 / 依赖 / 预检 |
| 2 | `python3 build/build.py test` | 构建与打包脚本单元测试 |
| 3 | `python3 build/test_build.py` | 构建脚本测试 |
| 4 | `python3 build/build.py` | Release 全量构建（F-B01） |
| 5 | `python3 build/pack.py` | Release 仅库出包（F-P01） |
| 6 | `python3 build/pack.py --with-examples` | Release 示例出包（F-P02） |
| 7 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-freebsd-x64.tar.gz --out-dir ./temp` | F-P01 |
| 8 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-freebsd-x64-examples.tar.gz --run-examples --out-dir ./temp` | F-P02 + F-T01（C10） |
| 9 | `python3 build/build.py --config Debug` | Debug 全量构建（F-B02） |
| 10 | `python3 build/pack.py --config Debug` | Debug 仅库出包（F-P03） |
| 11 | `python3 build/pack.py --config Debug --with-examples` | Debug 示例出包（F-P04） |
| 12 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-freebsd-x64-debug.tar.gz --out-dir ./temp` | F-P03 |
| 13 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-freebsd-x64-examples-debug.tar.gz --run-examples --out-dir ./temp` | F-P04 + F-T02（C10） |
| 14 | `python3 build/test_pack.py` | 打包脚本测试 |
| 15 | `python3 build/test_sdk_package.py` | 包测试脚本测试 |
| 16 | `python3 build/test_autotest.py` | 自动化脚本测试 |
| 17 | `python3 build/autotest.py` | 自动化（默认 `unit` 预设） |
| 18 | `python3 build/pack.py --with-cef` | 负向验证：报错退出（F-B03，预期结果） |

复测结论：Release/Debug 构建与 4 个包全部通过，两次 `--run-examples` 均输出
`[PASS] C10 examples.smoke`；`--with-cef` 报错属预期（FreeBSD 不支持 CEF），
不计为失败。该轮未发现遗留问题。

## 5. macOS 构建与打包矩阵

本轮环境：macOS x64、AppleClang 21、CMake 4.0.2、Ninja、Python 3.13.3；
Skia `llvm.x64.release`、SDL3 `main`、CEF 142/x64。

2026-10-28 macOS 真机复测：Release/Debug 构建、6 个出包、8 次包测试（含两次
`--run-examples`），以及 `build.py doctor/test`、`test_build.py`、`test_pack.py`、
`test_sdk_package.py`、`test_autotest.py`、`autotest.py` 全部通过；只改资源的增量同步
（M-B07）手测通过；arm64 按 2026-10-28 决策不安排（无 arm64 真机，只测 Windows）。

### 5.1 构建

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| M-B01 | x64 / Release | 默认（CEF 142、SDL 默认开启） | `python3 build/build.py` | 通过 | 全量构建通过；`bin/` 示例与 `.app` 正常更新 |
| M-B02 | x64 / Debug | 默认 | `python3 build/build.py --config Debug` | 通过 | 全量构建通过；切换配置后示例重新链接 |
| M-B03 | arm64 / Release | 默认 | `--arch arm64` | 不安排 | 无 arm64 真机（2026-10-28 决策：arm64 只测 Windows） |
| M-B04 | x64 / Release | CEF 示例 / `.app` | 构建 `cef`、`CefBrowser` | 通过 | Helper/Framework/wrapper 正常，Bundle 随 `--with-cef` 进包 |
| M-B05 | x64 / Release ↔ Debug | 同一 `bin/` 切换配置 | 连续执行 `python3 build/build.py` 与 `--config Debug` | 通过 | 两次构建均通过，切换后示例重新链接；单元测试覆盖两个方向 |
| M-B06 | x64 / Release | 普通示例 `.app`（`basic` 默认开启） | `python3 build/build.py` 后检查 `bin/basic.app`（`Contents/Info.plist`、`Contents/Resources/duilib/resources` 只含按示例名自动推导的子集） | 通过 | macOS 真机：`.app` 生成正常、`Info.plist` 存在、Debug/Release 切换后首次启动不再崩溃。资源改由 `<目标>_bundle_resources` 每次构建同步（按内容比较），实现见 `cmake/duilib_copy_bundle_resources.cmake` |
| M-B07 | x64 / Release | 仅改资源、不改源码 | 修改/新增/删除 `bin/resources/themes/default/basic/*.xml` 后 `python3 build/build.py`，`open bin/basic.app` | 通过 | 真机手测通过：三种改动（改内容、删文件、加文件）后构建都会同步到 `basic.app`，启动即生效，不需要删 `.app` 或强制重链；日志出现 `Syncing duilib resources into basic.app`，有变化时再出现 `Bundle 资源已同步` |

### 5.2 打包与包测试

打包命令与 `build.py pack` 等价。6 个包随后都由 `sdk_package_test.py` 完成包级、
消费者与负向场景校验，全部通过。

| 编号 | 配置 / 内容 | 打包命令 | 包文件 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| M-P01 | Release，仅库 | `python3 build/pack.py` | `nim_duilib-1.0.2026.1023-macos-x64.tar.gz` | 通过 |
| M-P02 | Release，含示例，无 CEF | `python3 build/pack.py --with-examples` | `nim_duilib-1.0.2026.1023-macos-x64-examples.tar.gz` | 通过 |
| M-P03 | Release，含示例 + CEF | `python3 build/pack.py --with-examples --with-cef` | `nim_duilib-1.0.2026.1023-macos-x64-cef-examples.tar.gz` | 通过 |
| M-P04 | Debug，含示例，无 CEF | `python3 build/pack.py --config Debug --with-examples` | `nim_duilib-1.0.2026.1023-macos-x64-examples-debug.tar.gz` | 通过 |
| M-P05 | Debug，含示例 + CEF | `python3 build/pack.py --config Debug --with-examples --with-cef` | `nim_duilib-1.0.2026.1023-macos-x64-cef-examples-debug.tar.gz` | 通过 |
| M-P06 | Debug，仅库 | `python3 build/pack.py --config Debug` | `nim_duilib-1.0.2026.1023-macos-x64-debug.tar.gz` | 通过 |

包测试命令与结果：

| 编号 | 包 | 命令示例 | 状态 |
| :--- | :--- | :--- | :--- |
| M-T01 | Release 仅库 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64.tar.gz` | 通过 |
| M-T02 | Release 示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples.tar.gz` | 通过 |
| M-T03 | Release CEF 示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-cef-examples.tar.gz` | 通过 |
| M-T04 | Debug 仅库 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-debug.tar.gz` | 通过 |
| M-T05 | Debug 示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples-debug.tar.gz` | 通过 |
| M-T06 | Debug CEF 示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-cef-examples-debug.tar.gz` | 通过 |
| M-T07 | Release 示例 + 启动示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples.tar.gz --run-examples` | 通过 | C10 启动 `bin/basic.app/Contents/MacOS/basic` 3 秒后仍在运行（首次执行曾因解包丢可执行位失败，修复后通过，见 §7） |
| M-T08 | Debug 示例 + 启动示例 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples-debug.tar.gz --run-examples` | 通过 | 同上 |

### 5.3 2026-10-28 macOS 真机复测命令（全部通过）

按执行顺序，同一 `bin/`、`lib/` 上完成 Release → Debug 的构建、出包与包测试：

| 序号 | 命令 | 覆盖 |
| ---: | :--- | :--- |
| 1 | `python3 build/build.py doctor` | 工具链 / 依赖 / 预检 |
| 2 | `python3 build/build.py test` | 构建与打包脚本单元测试 |
| 3 | `python3 build/test_build.py` | 构建脚本测试（含 `bin/.build-config/` 清理） |
| 4 | `python3 build/build.py` | Release 全量构建（含 `basic.app`、CEF `.app`） |
| 5 | `python3 build/pack.py` | Release 仅库出包（M-P01） |
| 6 | `python3 build/pack.py --with-examples` | Release 示例出包（M-P02） |
| 7 | `python3 build/pack.py --with-examples --with-cef` | Release CEF 示例出包（M-P03） |
| 8 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64.tar.gz` | M-T01 |
| 9 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples.tar.gz` | M-T02 |
| 10 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-cef-examples.tar.gz` | M-T03 |
| 11 | `python3 build/build.py --config Debug` | Debug 全量构建（切换配置，验证 Info.plist 保留与重链） |
| 12 | `python3 build/pack.py --config Debug` | Debug 仅库出包（M-P06） |
| 13 | `python3 build/pack.py --config Debug --with-examples` | Debug 示例出包（M-P04） |
| 14 | `python3 build/pack.py --config Debug --with-examples --with-cef` | Debug CEF 示例出包（M-P05） |
| 15 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-debug.tar.gz` | M-T04 |
| 16 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples-debug.tar.gz` | M-T05 |
| 17 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-cef-examples-debug.tar.gz` | M-T06 |
| 18 | `python3 build/test_pack.py` | 打包脚本测试 |
| 19 | `python3 build/test_sdk_package.py` | 包测试脚本测试 |
| 20 | `python3 build/test_autotest.py` | 自动化脚本测试 |
| 21 | `python3 build/autotest.py` | 自动化（默认 `unit` 预设） |
| 22 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples.tar.gz --run-examples` | M-T07（C10 启动示例） |
| 23 | `python3 build/sdk_package_test.py --package dist/nim_duilib-1.0.2026.1023-macos-x64-examples-debug.tar.gz --run-examples` | M-T08（C10 启动示例，Debug） |

复测结论：上述命令全部通过，无遗留问题。`M-B07`（只改资源、不改源码）已手测通过
（修改、新增、删除 XML 三种改动均由每次构建同步进 `basic.app`）。`M-B03`（arm64）
按 2026-10-28 决策不安排（无 arm64 真机，arm64 只测 Windows）。

## 6. 跨平台通用命令矩阵

| 编号 | 命令 | 覆盖内容 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- |
| G-01 | `build.py doctor` | 工具链、版本清单、依赖目录、预检 | 通过 | Windows 本轮；Linux 2026-10-28 虚拟机通过；macOS 2026-10-28 真机通过；FreeBSD 2026-10-29 虚拟机通过 |
| G-02 | `build.py deps --offline` | 离线采纳已有 Skia/CEF/SDL3；本地源码不在清单 ref 上时先切到本地已有的提交 | 通过 | 不访问网络，stamp 校验；Windows 实测 SDL `main` ↔ `release-3.4.18` 双向切换均可用 |
| G-03 | `build.py clean` | 删除整个 `out/`；`--build-dir` 可只清理指定构建目录 | 通过 | dry-run 仅打印路径 |
| G-04 | `build.py clean --deps --dry-run` | 展示依赖清理范围 | 通过 | 不会删除源码克隆 |
| G-05 | `build.py build/pack/test --dry-run` | 只打印命令 | 通过 | build/clean 不再误报完成 |
| G-06 | `build.py test` | 构建/打包单元测试 | 通过 | 2026-10-31 Windows 复测：262 项通过、2 项按平台跳过（POSIX 权限位相关）；含 `test_cmake_scripts.py` 的 CMake 规则用例（Skia 变体候选、CMake 与 `pack.py` 候选一致性、CRT 推导、依赖准备、多配置准备）与一次真实 configure 冒烟（PATH 无 cmake 时回退 `DUILIB_CMAKE_EXECUTABLE` / VS 自带 CMake）；macOS 2026-10-28 与 FreeBSD 2026-10-29 复测 `build.py test`、`test_build.py`、`test_pack.py`、`test_sdk_package.py`、`test_autotest.py` 均通过 |
| G-07 | 负向校验 | 混合库、架构不一致、CEF 版本不匹配、错误 ref/archive | 通过 | 均在配置/打包阶段明确报错 |

## 7. 当前测试进展（2026-10-29）

| 范围 | 已完成 | 结果 |
| :--- | :--- | :--- |
| Windows P1（关键出包组合） | 7 / 7 | 全部通过 |
| Windows P2（参数分支与工程逻辑） | 9 / 9 | 全部通过 |
| Windows 自动测试 | 158 项 | 全部通过 |
| Windows 示例启动（C10） | 6 次 | 通过（2026-10-29）：x64 Release 的 CEF、SDL、无 CEF 三包 + x64 Debug/Release 的 DLL 两包 + x86 Release SDL 包，均 `[PASS] C10 examples.smoke` |
| Linux x64 Release/Debug 构建 | 2 / 2 | 通过（含 CEF 示例，见 3.3） |
| Linux x64 clang 构建（Release） | 1 / 1 | 通过（`CC=clang CXX=clang++ python3 build/build.py`，仅构建，未出包） |
| Linux x64 出包 | 6 个包 | 全部通过（Release/Debug × 仅库/示例/CEF 示例） |
| Linux 包测试（`sdk_package_test.py`） | 8 / 8 | 全部通过（含 Release/Debug 示例包的 `--run-examples`，`--out-dir ./temp`） |
| Linux 脚本测试（`doctor`/`test`/`test_*`/`autotest`） | 7 项 | 全部通过（见 3.3） |
| Linux 示例启动/运行验证（`--run-examples`） | 2 次 | 通过：Release、Debug 示例包均 `[PASS] C10 examples.smoke`（L-T01/L-T02） |
| FreeBSD x64 Release/Debug 构建 | 2 / 2 | 通过（2026-10-29 虚拟机，命令见 4.3） |
| FreeBSD x64 出包 | 4 个包 | 全部通过（Release/Debug × 仅库/示例） |
| FreeBSD 包测试（`sdk_package_test.py`） | 4 / 4 | 全部通过（含 Release/Debug 示例包的 `--run-examples`，`--out-dir ./temp`） |
| FreeBSD 脚本测试（`doctor`/`test`/`test_*`/`autotest`） | 7 项 | 全部通过（见 4.3） |
| FreeBSD 示例启动（`--run-examples`） | 2 次 | 通过：Release、Debug 示例包均 `[PASS] C10 examples.smoke`（F-T01/F-T02） |
| FreeBSD CEF 负向验证 | 1 项 | `pack.py --with-cef` 按预期报错（不支持 CEF，F-B03） |
| macOS x64 Release/Debug 构建 | 2 / 2 | 全部通过（含 CEF `.app`，见 5.3） |
| macOS 普通示例 Bundle（`basic.app`） | 2 / 2 | Bundle 生成、`Info.plist` 保留、切换配置后首次启动正常（M-B06）；只改资源的增量同步手测通过，修改/新增/删除 XML 均生效（M-B07） |
| macOS x64 出包 | 6 个包 | 全部通过（Release/Debug × 仅库/示例/CEF 示例） |
| macOS 包测试（`sdk_package_test.py`） | 8 / 8 | 全部通过（含 Release/Debug 示例包的 `--run-examples`） |
| macOS 脚本测试（`doctor`/`test`/`test_*`/`autotest`） | 7 项 | 全部通过（见 5.3） |
| macOS 示例启动/运行验证（`--run-examples`） | 2 次 | 通过：Release、Debug 示例包均 `[PASS] C10 examples.smoke`（M-T07/M-T08） |
| arm64（仅 Windows） | 8 / 8 | Release/Debug 构建（W-B18/W-B18b）、DLL 模式（W-B28/W-B29）、VS 解决方案 ARM64（W-B27）、Release 无 CEF / CEF142 / CEF109 与 Debug CEF142 出包 + 包测试（W-P23/W-P24/W-P25/W-P26）均通过；示例启动未测（不跟进：无 Windows on ARM 设备）；Linux/macOS/FreeBSD 的 arm64 不安排（无真机） |

本轮验证过的代表性产物（中间包已清理，仅保留最终包）：

| 产物 | 条目 | 大小 |
| :--- | ---: | ---: |
| win-x64 Debug CEF 示例（最终保留） | 1316 | 420.2 MB |
| win-x64 Debug CEF109 示例 | 1134 | 373.5 MB |
| win-x86 Debug CEF 示例 | 1314 | 368.1 MB |
| win-x64 Release CEF 示例 | 1316 | 328.1 MB |
| win-x86 Release CEF 示例 | 1314 | 287.8 MB |
| win-x64 Release CEF109 示例 | 1134 | 280.8 MB |
| win-x64 Debug MD + SDL 示例 | 1396 | 421.1 MB |
| win-x64 Release MD 示例 | 1316 | 304.2 MB |
| win-x64 Release SDL 示例 | 1396 | 352.8 MB |
| win-x64 Release 无 CEF 示例 | 762 | 121.8 MB |
| win-x64 Debug 仅库 | 546 | 63.5 MB |
| win-x64 MinGW gcc Release 示例 | 757 | 214.6 MB |
| win-x64 llvm-mingw Release 示例 | 757 | 151.8 MB |
| win-x64 MinGW gcc Release SDL 示例 | 837 | 248.2 MB |
| win-x64 MinGW 动态链接 Release 示例 | 757 | 201.2 MB |
| win-arm64 Release 无 CEF 示例 | 760 | 109.0 MB |
| win-arm64 Release CEF 示例 | 1314 | 311.7 MB |
| win-arm64 Debug CEF 示例 | 1314 | 394.5 MB |

最终包位置：`E:\packtest\final\nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip`
（420.2 MB，1316 条目，`pack.py --verify` 全部匹配）。

macOS x64 的 6 个包保留在仓库 `dist/`，包名与包测试结果见 5.2 节。

### 本轮发现与修复

| 问题 | 处理 |
| :--- | :--- |
| 复用 `out/<arch>/<Config>` 时 CMake 有状态选项残留（`--no-examples`、`--md`、`--sdl`、`--cef-version`） | 每次显式传入解析后的值，新增 3 项回归测试 |
| SDL3 构建目录被其他工具链/运行库使用后仍复用旧缓存（MinGW 下调用 `cl.exe`） | 按 `.duilib-sdl` stamp 校验 CRT，不一致时清空重建 |
| `build`/`clean` 的 `--dry-run` 仍打印“编译完成/已删除” | dry-run 下不再输出完成提示，新增 1 项测试 |
| MinGW 禁用静态链接后示例缺运行库 DLL | 已验证构建/打包，文档补充运行库清单；不由构建自动收集 |
| 切换 Debug/Release 后 Ninja 判定链接已最新，`bin/` 不更新 | `build.py` 链接前按 `bin/.build-config` 删除另一配置的示例及标记，强制重链；macOS 本轮 Release → Debug 实测通过，单元测试覆盖两个方向 |
| macOS 切换 Debug/Release 后，后编译的那一版首次启动崩溃（栈顶 `+[UNUserNotificationCenter currentNotificationCenter]` 断言） | 根因是跨配置清理整删 `.app`，丢掉 configure 阶段生成、链接阶段不重建的 `Contents/Info.plist`（缺 `CFBundleIdentifier`，LaunchServices 解析不到 bundle）。`build.py` 的清理改为对 `.app` 只删 `Contents/MacOS` 内容，保留 plist 与资源；macOS 真机 Release ↔ Debug 反复切换后启动正常，单元测试覆盖 |
| 只改资源、不改源码时 Bundle 内仍是旧资源（测试看不到修改） | 资源复制原先挂在 `POST_BUILD`，不重链就不执行。新增 `cmake/duilib_copy_bundle_resources.cmake`，由每次构建都运行的 `<目标>_bundle_resources` 目标调用：按 SHA256 比较、增量复制、删除清单外文件；普通示例与 CEF 示例共用 |
| 新增资源目标后 configure 报循环依赖（`<目标>_bundle_resources` ↔ `<目标>`） | 自定义目标的 `COMMAND` 中引用了 `$<TARGET_BUNDLE_DIR:${target}>`，目标相关生成表达式会隐式加一条依赖目标的边（CMP0112），与 `add_dependencies` 成环。改为字面路径 `${DUILIB_BIN_PATH}/<目标>.app/...`；CEF 分支本就是字面路径，无此问题 |
| macOS `--run-examples` 的 C10 报 `PermissionError: [Errno 13]`（`bin/basic.app/Contents/MacOS/basic`） | `sdk_package_test.py` 的 `extract_package` 用 `open(..., "wb")` 解包，权限固定 0644，Unix 上丢可执行位。tar 按 `member.mode`、zip 按 `external_attr >> 16` 恢复模式（Windows 跳过），新增 5 项单元测试（POSIX 分支用 mock 断言 `chmod` 参数） |
| Windows arm64 无法随包 CEF（`collect_cef()` 直接报错，`third_party_versions.json` 的 109 缺 `windows/arm64` 条目） | 上游已发布 Windows arm64 预编译包（142 与 109 都有），`pack.py` 现按 `lib/arm64` 收集 `libcef.lib`、按 `bin/libcef_win[_109]/arm64` 收集运行时；清单补 109 的 `windows/arm64` 归档；`test_pack.py` 由「拒绝」改为「收集」用例 |
| `doctor --arch arm64` 报 `Visual Studio C++ tools for arm64 are not installed`，但 VS 已装 ARM64 生成工具 | `vcvarsall.bat` 原先固定传 `arm64`，该参数要求 `Hostarm64` 原生工具集，x64 主机默认只装 `Hostx64\arm64`。改为按主机选择 `amd64_arm64` / `x86_arm64` / `arm64`（新增 `_vcvars_platform` 与 4 项单元测试）；`doctor --arch arm64` 复查通过（Ninja + VS + gn + curl 全 OK） |
| Windows arm64 链接失败：`SkCreateWGLContext`、`SkWGLExtensions::*` 共 5 个 LNK2019 | Skia 上游在 `_M_ARM64` 下把 `SkWGL_win.cpp` 整体排除，`GrGLMakeNativeInterface()` 返回 nullptr，ARM64 没有 WGL 后端。`SkGLWindowContext_Windows.cpp` 改用 `#if defined(DUILIB_BUILD_FOR_WIN) && !defined(_M_ARM64)`；`Render_Skia_Windows` 在 ARM64 的 `MakeGLForWin` 直接返回 nullptr，绘制固定走 CPU（`PaintAndSwapBuffers` 的 GL 分支同步排除），非 ARM64 行为不变 |
| arm64 示例链接命令嵌入的是 `duilib.x64.manifest` | `cmake/duilib_bin_windows.cmake` 原先只按 32/64 位选清单，ARM64 进程用 x64 清单无法启动；改为按 `DUILIB_TARGET_ARCH_RESOLVED` 优先匹配 `duilib.arm64.manifest`，`cmake/README*.md` 同步代码片段 |
| Ninja 交叉编译 arm64 时 `nim_duilibConfig.cmake` 把消费者判成 x64，C01/C03/C05 配置阶段报「SDK 架构为 arm64，当前工程为 x64」 | 单配置生成器下 `CMAKE_SYSTEM_PROCESSOR` 仍是主机架构；新增 `CMAKE_CXX_COMPILER_ARCHITECTURE_ID MATCHES "ARM64"` 回退分支，重打包后 C03/C05 转 PASS、C01 配置与编译通过 |
| x64 主机上 arm64 消费者运行失败（`OSError: [WinError 216]`），C01 被误判为 FAIL | 交叉编译产物无法在本机执行。`sdk_package_test.py` 新增 `_can_run_arch()`（x64 主机可经 WOW64 运行 x86，反之不可），配置/编译/链接通过后按架构 SKIP 并注明「运行验证需 arm64 环境」；新增 5 项单元测试 |
| Linux 上 C01/C03 报「C++ 编译器不可用」「配置失败原因不含 …」，日志为 `error closing ...s: 设备上没有空间` | `/tmp` 是小容量 tmpfs 且已写满，gcc 临时汇编文件写不进去，被误判成包/工具链问题。`sdk_package_test.py` 现在把 `TMPDIR`/`TEMP`/`TMP` 指到 `--out-dir/<NN>/work/tmp`；启动时按包大小估算空间并打印 `[WARN]`；新增 4 项单元测试 |

### 2026-10-28 VS 工程 ARM64 支持（已验证）

| 问题 | 处理 | 验证 |
| :--- | :--- | :--- |
| VS 解决方案只有 Win32/x64，无法在 VS 配置管理器里选 ARM64 | `CommonSettings.props` 把 `ARM64` 映射为 `arm64`（`OutDir` 与 CMake 共用 `lib/arm64/<Config>`），`CEFSettings`/`WebView2Settings`/`BinManifestSettings` 增加 ARM64 分支，`BinOutSettings`/`DuilibSettings` 增加 ARM64 输出目录与 `_d` 名称，`SDLSettings`/`LibPagSettings` 在 ARM64 强制关闭（无预编译库）；30 个 `.vcxproj` 复制 x64 配置为 `Debug|ARM64`/`Release|ARM64`（`/machine:x64`→`/machine:arm64`），4 个 `.sln` 补平台与工程映射；`SkGLWindowContext_Windows.cpp` 在 ARM64 下 `ExcludedFromBuild`（无 WGL） | 4 个 `.sln` 的 Release/Debug ARM64 均通过 `ValidateSolutionConfiguration`，映射 30/30、24/24、7/7、5/5 无缺失；Release/Debug 各 23 个示例与 7 个库构建成功，46 个 exe 均 PE `0xAA64`、清单 `processorArchitecture="ARM64"`；`duilib_no_cef.sln` / `examples_no_cef.sln` 的 ARM64 Release/Debug 也实际编译通过（各 19 个示例） |
| ARM64 链接 `duilib-webp.lib` 报 7 个 `*NEON` 符号 LNK2001（`VP8DspInitNEON`、`WebPInitUpsamplersNEON` 等） | libwebp 在 `_M_ARM64` 下定义 `WEBP_HAVE_NEON`，而 VCXPROJ 只列了 SSE/AVX 源；补入 `src\dsp\alpha_processing_neon.c`、`dec_neon.c`、`filters_neon.c`、`lossless_neon.c`、`rescaler_neon.c`、`upsampling_neon.c`、`yuv_neon.c`（非 NEON 平台由 `WEBP_DSP_INIT_STUB` 生成空实现，可无条件列出） | `examples.sln` ARM64 Release 不再出现该 LNK2001 |
| `examples.sln` 链接 `cef.exe`/`CefBrowser.exe` 报 20 个 `cef_v8value_*`、`cef_create_context_shared` 等 LNK2001 | 解决方案把 `libcef_dll_wrapper_109` 声明为 `cef`/`duilib`/`CefBrowser` 的工程依赖，sln 构建会把依赖工程的输出注入链接行，且 109 封装库排在 142 之前被优先拉取；此时 `LibCefVersion109=0` 只提供 CEF 142 的 `libcef.lib`（已移除相关 C API）。移除 3 处（`examples.sln`）与 1 处（`duilib.sln`）冗余依赖；109 封装库仍随解决方案编译，`LibCefVersion109=1` 时由 `CEFSettings.props` 显式链接 | `cef.exe`、`CefBrowser.exe` 链接通过；`duilib.sln` ARM64 Release 增量构建 0 错误 |
| 全并行 `/m` 构建示例并链接 LTCG 静态库时出现 `C1060 编译器堆空间不足` | 与平台无关的并行度/内存峰值问题（`duilib.lib` 828.5 MB，`/GL`）；文档建议示例解决方案用 `/m:2` 或在 VS 里降低“最大并行项目生成数” | `/m:2` 下 Release（`/GL`）与 Debug 的 23 个示例全部通过 |

### 2026-10-30 CEF 156 升级（Windows 已验证）

| 问题 / 验证点 | 处理 | 验证 |
| :--- | :--- | :--- |
| 156 发行包取消了 `lib/` 目录，导入库改放在 `Release/` | `duilib/third_party/libcef/libcef_win` 的 `include/`、`libcef_dll/`、`cmake/`、`CMakeLists.txt` 整体替换为 156；`lib/x64`、`lib/Win32`、`lib/arm64` 的 `libcef.lib` 分别取自各 Windows 包的 `Release/libcef.lib` | Release/Debug 构建均正常链接 `libcef.lib` 与 `libcef_dll_wrapper` |
| 封装库源码有增减 | wrapper 新增 4 个 `.cc`（`component_update_callback_cpptoc`、`component_ctocpp`、`component_updater_ctocpp`、`v8_backing_store_ctocpp`）与 12 个头文件，删除 6 个已移除头文件 | `libcef_dll_wrapper.vcxproj` / `.filters` 按 CMake 156 包重新生成，文件集合与生成工程完全一致（cc 213、h 476），保留 `CEFSettings.props` 接入与 `CEF_USE_BOOTSTRAP` 定义 |
| 156 wrapper 要求 C++20；Windows 沙箱路径定义 `CEF_API_VERSION_LAST=15600` | 根工程对 Windows 全部版本强制 `USE_SANDBOX=OFF`（109 另有迭代器调试冲突），wrapper 的 C++17 降级只保留给 `libcef_dll_wrapper_109` | Release/Debug 编译无错误；`/std:c++20` 与 wrapper 的 `-std=c++20` 一致 |
| `CefRegisterExtension` 在 156 被移除（`CEF_API_REMOVED(15400)`），`CefClientAppRender.cpp` 编译报 `C3861` | `OnContextCreated` 中向页面全局对象绑定 native 函数 `__nimCefNativeCall` / `__nimCefNativeRegister`，再 `Eval` 注入与原扩展等价的 `NimCefWebInstance`（`call` / `register`）；109 分支保留原扩展注册 | `cef.exe --remote-debugging-port=9335` 通过 DevTools `Runtime.evaluate` 实测：`typeof NimCefWebInstance` = `object`、`typeof __nimCefNativeCall` = `function`；调用 `NimCefWebInstance.call('ShowMessageBox', {message}, cb)` 弹出标题为 `C++ receives the message sent by JavaScript.` 的对话框，JS 回调收到 `[false,{"message":"Success."}]`，Web 端调用方式未变 |
| `deps` 把 142 运行时写成 `156|x64|windows` 指纹（Windows 无法从运行时读取版本） | 采纳规则收紧：仅 Linux/macOS 在 `include/cef_version.h` 版本匹配时采纳；Windows 一律按归档重新解压覆盖 | 删除 `bin/libcef_win/x64` 后 `deps --offline`：`libcef.dll` 294,670,848 字节、指纹 `156|x64|windows`；Win32/arm64 运行时同步刷新为 156（`156|x86|windows`、`156|arm64|windows`） |
| Debug 构建链接报 `SkPathBuilder::addPolygon`、`SkDashPathEffect::Make`、`SkCanvas::drawGlyphs`、`SkFont::unicharsToGlyphs` 等 `LNK2019` | 与 CEF 升级无关：`E:\develop\skia\out\llvm.x64.debug` 仍是旧 Skia 提交的产物（Release 的 `llvm.x64.release` 已随 Skia 升级重编）；`build.py deps --config Debug --offline` 重编 `llvm.x64.debug`（1084/1084） | 重跑 Debug：23/23 链接通过，日志 0 告警 |
| 完整构建回归 | Release / Debug 全量 `build.py build --no-deps` | Release 42/42 通过；Debug 23/23 通过；`bin/cef.exe`、`bin/CefBrowser.exe` 重新生成；日志无 warning；两个 CEF 示例启动后持续运行（冒烟正常） |
| 单元测试与环境检查 | `build.py test` / `build.py doctor` | `test` 236 个用例通过（skipped=2）；`doctor` 显示 `cef default 156`、`cef runtime 156 ...bin\libcef_win\x64`，其余项 OK |

辅助结果：版本清单、VS 工程文件列表、脚本默认版本提示（`pack.py` 的 arm64 文案、`sdk_package_test.py` 包名解析）与文档（README、`build/build-reference.md`、`docs/CEF.md`、`docs/Deployment.md`、`docs/Differences.md`、`duilib/third_party/README.md`、CHANGELOG）已同步到 156；109 仍保留。

### 2026-10-30 Skia 补丁归档机制（Windows 已验证）

| 问题 / 验证点 | 处理 | 验证 |
| :--- | :--- | :--- |
| 全新环境里阶段 A 预检在补丁解压前要求 `skia/bin/gn`，配置直接失败 | 预检读取清单 `skia.patchDir` / `patchZip`，归档存在时把缺失 gn 降级为告警，由 `deps` 解压后校验 | 归档存在、gn 缺失：`[WARN] 未找到 gn；依赖准备会从 Skia 补丁归档中解压 (skia.2026-10-30.src.zip)`，rc=0；归档缺失：仍报 `[ERROR] 未找到必需的工具: gn` + Hint，rc=1；`DUILIB_DEPS_MODE=off`：`[WARN] gn not found (deps: off)`。直接 `cmake -S . -B`（不传 `-DDUILIB_SKIA_REF`）时发现未加引号的 `if(NOT DUILIB_SKIA_REF STREQUAL "")` 在变量未定义时按字面量比较，导致 `patchZip` 传空、预检误报；改为带引号比较后复测通过（配置成功且输出上述告警） |
| Skia 源码停在旧基线 `6f559baf`，需要真实切换提交、打补丁并编译 | `build.py deps` 按清单切换提交、校验 `SHA1.txt`、解压补丁，再用归档自带的 gn/ninja 编译 | HEAD `6f559baf` → `e46ba84`（本地已有该提交，未走镜像）；归档 66 条目解压后 `bin/gn.exe`、`bin/ninja.exe` 就绪；`llvm.x64.release` 1084 个目标编译完成，`out/llvm.x64.release` 生成 `skia.lib` 等 10 个静态库；`.duilib-patch` 为 `e46ba84...:fbe0d0...`（zip 的 sha256），`.duilib-ref` 为 `e46ba84...|e46ba84...|msvc-mt|x64` |
| 二次执行是否重复解压/编译 | 补丁指纹与 ref 未变时直接复用 | 再跑 `deps` 只有 `[OK] Skia 已就绪 (llvm.x64.release)`，无解压与编译输出 |
| 归档缺失时 CMake 侧此前静默复用，与 build.py 的策略不一致 | 按“需重建 / 已有产物”区分：需要重建时报错，已有产物时告警复用 | 临时移走归档后 `deps`：Python 与 CMake 各输出一次 `[WARN] 未找到 Skia 补丁包；继续复用已有 Skia 产物`，rc=0；恢复归档后 sha256 仍为 `fbe0d0...` |
| 归档与清单不配套（`SHA1.txt` 记录的提交不一致） | 两个入口都在解压前校验 | 用记录旧提交的副本替换归档：`build.py deps` 报 `[ERROR] Skia 补丁包的 SHA1 与版本清单不一致` + Hint（rc=1）；`cmake --build --target duilib_deps` 同样报错，两侧已共用同一组消息键 |
| 归档内缺少 `SHA1.txt` 时 CMake 侧直接抛出 `tar: SHA1.txt: Not found in archive` | 提取前先用 `cmake -E tar tf` 列出条目，缺失则按空 SHA-1 走统一错误 | Python 报同一错误；CMake 改为 `[ERROR] Skia 补丁包的 SHA1 与版本清单不一致` + `SHA1.txt missing` + Hint |
| 旧构建树缓存残留 `DUILIB_SKIA_COMPILE_ROOT` / `DUILIB_SKIA_COMPILE_REPO` | 配置期清理（对应选项已删除） | 重新配置后 `CMakeCache.txt` 只剩 `DUILIB_SKIA_PATCH_DIR=E:/develop/nim_duilib/build/skia_patch` |
| 中文环境下 `project()` 之后的构建消息变成英文（重复 include 把语言重置为 en） | 默认语言只在首次初始化时写入 | 重新配置：`[OK] 目标架构  x64`、`[OK] Skia 已就绪`、`[OK] 依赖已就绪` 均为中文，`build.ninja` 中 `duilib_deps` 命令为 `-DDUILIB_LANG=zh_CN` |
| 新机制下的完整编译回归 | `build.py clean` 后全量 `build.py build` | 802/802 目标完成，23 个示例 exe 重新链接（含 cef/CefBrowser），无告警，`[OK] 编译完成` |
| VS 未安装可选组件 ninja 时，`-G Ninja` / `cmake --preset` 配置直接失败（归档里已有 `bin/ninja.exe`） | 根工程在 `project()` 之前自举：按 `DUILIB_NINJA_EXECUTABLE` → `skia/bin` → PATH → 补丁归档取用，解压到 `<构建树>/.duilib-tools/bin/ninja.exe` 并写入 `CMAKE_MAKE_PROGRAM`；`build.py` 查找链加入 `skia/bin` 与归档（解压到 `out/.duilib-tools`） | 探针项目（PATH 无 ninja 且 `skia/bin/ninja.exe` 临时改名）：`cmake -G Ninja` 输出 `[OK] ninja .../build/.duilib-tools/bin/ninja.exe (from skia.2026-10-30.src.zip)`，随后 `cmake --build` 实际执行成功（rc=0）；恢复 `skia/bin/ninja.exe` 后同一探针输出 `[OK] ninja E:/develop/skia/bin/ninja.exe (skia/bin)`，重复配置直接走缓存；真实工程 `cmake -S . -B <临时目录> -G Ninja`（PATH 无 ninja、`skia/bin/ninja.exe` 缺失）同样先输出自举成功行，预检随后 `[OK] ninja <同一路径>`（该次未进入 vcvars 环境，`project()` 因找不到 cl 失败，属探针预期） |
| 阶段 A 预检只按 PATH 查 ninja，与 gn 的归档引导不一致 | 预检按 `DUILIB_NINJA_EXECUTABLE` → `skia/bin` → PATH 查找；Ninja 生成器且确实没有 ninja 时报错并提示 VS 组件，VS 生成器且归档存在时降级为告警 | VS 生成器、PATH 与 `skia/bin` 均无 ninja：`[WARN] 未找到 ninja；依赖准备会从 Skia 补丁归档中解压自带的 ninja（用于编译 Skia） (skia.2026-10-30.src.zip)`，rc=0；Ninja 生成器同样条件：`[ERROR] 未找到必需的工具: ninja` + 新 Hint，rc=1 |
| `build.py` 找不到 ninja 时退回 MSBuild 生成器，丢失 Ninja 预设与 Skia 工具链 | `_find_ninja` 增加“已解压的 `skia/bin`”与“补丁归档解压到 `out/.duilib-tools/bin/ninja.exe`”两个来源，并支持 `DUILIB_NINJA_EXECUTABLE` 环境变量覆盖 | 新增 7 项单元测试（环境变量 / PATH / VS / skia/bin / 归档解压 / 归档缺失 / 全部缺失）；`build.py test` 236 个用例通过（skipped=2）；按覆盖方式指定归档解压出的 ninja 后 `build.py deps --build-dir <临时目录>` 配置 + 依赖目标成功（`CMAKE_GENERATOR=Ninja`，`CMAKE_MAKE_PROGRAM` 指向该副本） |

辅助结果：`build.py test` 236 个用例通过（skipped=2）；`build.py doctor` 显示
`[OK] skia patch ...skia.2026-10-30.src.zip`、`[OK] gn`，其余检查项全部 OK。

### 2026-10-29 libjpeg-turbo 3.2.x 与 duilib-zlib 的 `z_errmsg` 冲突（Windows 已验证）

| 问题 | 处理 | 验证 |
| :--- | :--- | :--- |
| 静态链接 `turbojpeg-static.lib` 时报 `LNK2005`：`z_errmsg` 在 `zutil.obj`（libjpeg-turbo 内置 zlib）与 `zutil.obj`（duilib-zlib）重复定义 | libjpeg-turbo 3.2 起上游预编译库内嵌 spng + zlib（`tjLoadImage`/`tjSaveImage` 的 PNG 支持），链接 `turbojpeg.c.obj` 会一并拉入内置 `zutil.c.obj`；其内置 zlib 虽按 `Z_PREFIX` 改名，但 `z_errmsg` 不在 `zconf.h` 的前缀列表内，仍以原名导出。为 duilib-zlib 增加编译宏 `z_errmsg=duilib_z_errmsg`（内部符号，不在 `zlib.h` 公开 API 中），CMake 与 VS 工程两套构建同步修改 | Debug x64 重编 duilib-zlib 后符号表为 `duilib_z_errmsg`；最小工程同时链接 `duilib-zlib.lib` 与 `turbojpeg-static.lib`，修复前可复现 `LNK2005`、修复后链接通过；`build.py test` 不受影响。2026-10-29 复验：`lib/x64/Release/duilib-zlib.lib` 是修复提交（09:22）之前构建的旧库，出包后 SDK 测试 C01 仍报 `LNK2005`；用 `build.py build --config Release --target duilib-zlib` 重编后重新出包，`pack.py --verify` 1100 条目 SHA 全匹配、`sdk_package_test.py` `PASS=15 FAIL=0 SKIP=4`（C01 由 FAIL 转 PASS）。x86 与 arm64 的 `duilib-zlib.lib` 同日重编，三架构符号表均为 `duilib_z_errmsg`、无裸 `z_errmsg` |

### 2026-10-27 依赖版本一致性（Windows 已验证）

| 问题 | 处理 | 验证 |
| :--- | :--- | :--- |
| `<deps-root>/SDL` 停在 `main`，清单为 `release-3.4.18`，脚本仍编译本地源码并复用旧 stamp | `deps` 先按清单切换 tag/commit（本地已有提交不联网，`--offline` 可用）；`.duilib-sdl` 记录源码提交，切换后旧产物重建 | 当时清单为 `release-3.4.18`：`build.py deps --sdl --offline` 使 HEAD `3b2bc95` → `829a65d`，SDL3 重建，stamp 含 `829a65d...`；二次运行 `[OK] SDL3 is ready` |
| 清单默认 ref 为 `release-3.4.18` tag，但其源码编译报错 | 默认 ref 改为 `main` 分支；`release-3.4.18` 保留在 `refs` 中，需要时用 `--sdl-ref` 选择 | `build.py deps --sdl --offline`：HEAD `829a65d`（tag）→ `3b2bc95`（main），SDL3 重建，stamp 为 `main|3b2bc95...`；二次运行 `[OK] SDL3 is ready`，未再重建 |
| 补丁归档与清单 Skia 提交不配套时仍被使用 | 归档改为随仓库提交到 `build/skia_patch/`，`SHA1.txt` 记录对应提交；`deps` / CMake 依赖脚本解压前校验，不一致直接报错 | `doctor` 输出 `[OK] skia patch ...`；`test_build.py` 覆盖 SHA1 不一致与归档缺失的报错路径 |
| `--skia-ref` 选中的 tag/branch 不检查本地工作区 | 与 commit ref 一样按清单对齐；branch 只要求当前在该分支，不追远端 | 临时仓库隔离验证：tag、branch 均可切换；离线缺 ref 时明确报错 |

### 已知缺口与风险

1. MinGW `DUILIB_MINGW_STATIC=OFF` 的包不包含 `libstdc++-6.dll` 等运行库，分发前需手工补齐。
2. DLL 模式已由 `build.py --dll` 与 `duilib_dll.flag` 两个入口产出，x64/x86 ×
   Debug/Release 的构建、打包与 SDK 接入已覆盖（见 W-B21 ~ W-B26、W-P18 ~ W-P22）；
   2026-10-24 生成的旧 `dist/` DLL 包（含 `/MT` 的 `turbojpeg-static.lib`）已删除，
   `dist/` 中只保留本轮通过校验的新 DLL 包（W-P18 ~ W-P21、W-P22）。
3. Linux x64 已完成 Release/Debug 构建、6 个出包、8 次包测试（含下游工程接入 C01 与
   `--run-examples` 的 C10）与全部脚本测试（2026-10-28 虚拟机复测）；clang 工具链已验证
   Release 构建（未出包）；arm64 不安排（无真机）。FreeBSD x64 已完成 Release/Debug
   构建、4 个出包、4 次包测试（含两次 `--run-examples`）与全部脚本测试
   （2026-10-29 虚拟机复测），CEF 按平台不支持（`--with-cef` 报错属预期）。
4. macOS x64 的 Release/Debug 构建、`basic.app`（`Info.plist` 保留、切换配置后首次启动
   正常、只改资源的增量同步）、6 个出包、8 次包测试（含两次 `--run-examples`）与全部脚本
   测试均已通过（2026-10-28 真机复测）；arm64 不安排（无 arm64 真机）。
5. arm64 本轮只测 Windows（2026-10-28 决策：macOS/Linux/FreeBSD 均无真机）。Windows 已
   完成 Release/Debug 构建（W-B18/W-B18b）、VS 解决方案 ARM64（W-B27）、Release 无 CEF /
   CEF142 与 Debug CEF142 出包与包测试（W-P23/W-P24/W-P26）。DLL 模式（W-B28/W-B29）
   与 CEF109 出包（W-A07/W-P25）也已补齐。Windows on ARM 真机运行验证不具备条件
   （无设备），标记未测、不跟进；x64 主机运行 ARM64 程序报 `WinError 216`，C01 因此
   按架构 SKIP。ARM64 没有 WGL，Skia 恒走 CPU 绘制；步骤与剩余项见
   `build/arm64-test-plan.md`。
6. `bin/` 与 `lib/<arch>/<Config>` 被多架构/多工具链共用，切换后必须清理，否则打包
   会因 PE 架构或混合库校验失败（这是预期保护，不是缺陷）。
7. MinGW 默认并行度在内存较小的机器上可能 OOM，回归时优先 `--jobs 2`/`--jobs 3`。
8. Unix 台账第 8～13 条（2026-10-29 复核）：**代码修复均已就位，运行期验证缺少 Unix 环境
   （未测、不跟进，不再列入待办）**。逐条结论：
   - 第 8 条 SDL3 stamp 去掉配置分量：已确认 `cmake/duilib_deps_run.cmake`
     `_duilib_sdl_stamp_value()` 仅 Windows 追加 `${DUILIB_CONFIG}`，Unix 分支不带配置；
     Unix 的 SDL3 固定按 Release 构建共享库，产物与 duilib 的 Debug/Release 无关。
   - 第 9 条 Skia 绝对路径链接：已确认 `cmake/duilib_common.cmake` 在库目录不含生成器
     表达式时（含 Unix 单配置）调用 `_duilib_resolve_static_libs()`，把
     `svg/skshaper/skottie/sksg/jsonreader/skia` 解析为绝对路径，Ninja 因此把库文件
     记为链接依赖；多配置生成器下仍保留库名交给链接器解析。
   - 第 10 条 CEF 跟踪目录：已确认 `cmake/duilib_cef.cmake` 的 `_duilib_cef_tracked_dir()`
     按平台返回（Windows 为 `libcef_win[_109]`，Unix 为 `libcef_linux`）。
   - 第 11 条 SDL ref 对齐：已确认 `_duilib_ensure_git_ref()` 在构建前把本地仓库切到清单
     ref（本地已有分支/远端跟踪分支时不联网），stamp 含源码提交，切换后必然重建一次。
   - 第 12 条补丁归档刷新：补丁归档改为随仓库提交（`build/skia_patch/`），不再从
     外部仓库恢复或拉取；`_duilib_skia_patch_sha1()` 读取归档内 `SHA1.txt`，
     `_duilib_skia_patch_verify()` 与清单 Skia 提交比对，不一致直接报错；归档缺失且
     需要重建时报错并提示用 `git checkout -- build/skia_patch` 恢复（已有产物时仅告警）。
   - 第 13 条 `--skia-ref` 分支：已确认 `build.py doctor` 的 `_check_source_ref()` 只读
     `rev-parse` 做状态报告，不 fetch、不 checkout；实际切换只在 `deps`/构建阶段由
     `_duilib_ensure_git_ref()` 完成，不会留在 detached HEAD。
   说明：2026-10-28 的 Linux 虚拟机、macOS 真机复测跑过 Release/Debug 双配置构建与出包
   （第 8/9 条的运行场景已被覆盖），但当时没有单独记录「第二次 `deps` 不重建 SDL3」
   「改 Skia 后示例重新链接」这两个观测点；Windows 侧对应逻辑（补丁归档校验、ref 切换、
   stamp 复用）已在 §7 的回归中实测。本轮起不再把这几条列入待办，后续有 Unix 环境时
   可按上述命令顺带复测。
9. **Windows 示例包的包内启动冒烟（C10）**：已关闭。2026-10-29 的 6 个示例包
   （x64 Release 静态 CEF / +SDL / 无 CEF，x64 Debug 与 Release 的 DLL + SDL + CEF，
   x86 Release + SDL）全部 `[PASS] C10 examples.smoke`，`--verify` 均 SHA 全匹配；
   DLL 回归后已恢复静态构建并清理 `bin/duilib*.dll`（P3-1、P3-2 完成）。
10. **x86 + SDL3 组合缺库**：已关闭（2026-10-29）。`build.py build --config Release
    --arch x86 --sdl` 会按 x86 重建 SDL3（`../SDL3` 的 stamp 含架构与 CRT），
    出包 W-P27（844 条目 / 128.2 MB），`--verify` 与 SDK 测试（PASS=16 FAIL=0 SKIP=3）
    通过，C10 启动成功（P3-2 完成，P0-1 收口）。
11. **`build/` 脚本控制台输出**：已关闭（2026-10-29，P3-3）。`build.py`、`pack.py`、
    `release.py`、`autotest.py` 与 `sdk_package_test.py` 的运行期消息与 `--help`
    改为简易双语：简体（`zh`/`zh_CN`/`zh_Hans`）显示中文，其余区域（含
    `zh_TW`/`zh_HK`/`zh_Hant`）显示英文；文案集中在 `cmake/i18n/messages.json`，
    五个入口均支持 `--lang`，语言在 argparse 之前解析。`PYTHONIOENCODING=ascii`
    下 `--help` 与列表命令（加 `--lang en`）无中文，`python3 build/build.py test`
    229 个用例通过（补丁机制调整后移除 7 个旧用例）；原任务中的两个 `.sh` 已随 `build/legacy/` 清理删除。
    `release.py` 的 `check`/`package`/`finalize`/`clean` 全流程已于 2026-10-29 实测
    通过（含失败路径，P3-5 完成，记录见 `Packaging-Follow-Up-Items.md`）。

## 8. 回归执行顺序建议

1. `build.py test`，确认构建/打包脚本自身无回归。
2. Windows 默认 x64 Debug 全量构建，作为基线。
3. 按 W-B02 ~ W-B15 逐项构建，每项后立即出包并 `pack.py --verify`；示例包再执行一次
   `sdk_package_test.py --run-examples`（Windows 的 C10 已覆盖 Release 静态 CEF / SDL /
   无 CEF 与 Debug/Release DLL 五包，新增组合仍应按同一口径补测）。
4. 切换工具链/架构前清理 `bin/*.exe`、`bin/*.dll` 与 `lib/<arch>/<Config>`。
5. Windows 完成后执行 G-01 ~ G-05 通用命令分支。
6. Linux、FreeBSD、macOS 按各自矩阵执行；三者的 Release/Debug 构建、出包、包测试与
   示例启动均已覆盖（见 §3、§4、§5）；FreeBSD 的 CEF 负向验证（`--with-cef` 报错）同样
   纳入回归。
7. 回归结束恢复默认 x64 Debug 构建，并重新生成最终包校验。
