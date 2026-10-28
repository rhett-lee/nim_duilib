# 构建与打包测试矩阵

本文档汇总 nim_duilib 统一构建入口（`build/build.py`）与打包脚本（`build/pack.py`）的
全部测试组合，供后续版本回归测试直接参照。矩阵按平台、工具链、架构、配置和功能开关
划分，并记录当前验证状态。

- 更新日期：2026-10-28
- 适用版本：新构建系统（`out/<arch>/<Config>` + `lib/<arch>/<Config>` + `bin/`）
- 命令示例中 Windows 用 `python`，Linux/macOS/FreeBSD 用 `python3`
- 每个组合出包后都必须执行 `pack.py --verify <包>` 校验条目数、文件 SHA 与文档链接

## 状态标记

| 标记 | 含义 |
| :--- | :--- |
| 通过 | 2026-10-26 ~ 28 本轮实际执行并通过 |
| 历史通过 | 新构建系统验证阶段（2026-10-23 ~ 25）执行并通过，本轮未复测 |
| 部分 | 已执行但存在已知限制，或仅完成构建/打包中的一项 |
| 未测 | 尚未验证 |
| 不适用 | 项目不支持该组合 |

## 0. 测试环境基线

| 项目 | 本轮环境 |
| :--- | :--- |
| 操作系统 | Windows 10/11 x64 |
| Visual Studio | 2022 17.14.37710.0（MSVC x64/x86；无 arm64 工具集） |
| CMake / Ninja | CMake 4.3.0，Ninja（VS 自带） |
| Python | 3.14.4 |
| MSYS2 | `E:\msys64`（MINGW64、UCRT64、CLANG64；MINGW32 缺 gcc/make） |
| 独立 MinGW | `C:\mingw64\llvm-mingw-20250430-ucrt-x86_64` |
| Skia | `6f559bafbed4c8323a899df4008aa073df4eccc6` |
| SDL3 | `main`（`release-3.4.18` tag 源码编译报错，已不再作为默认） |
| CEF | 默认 142；109 可选（Windows 7） |

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
| W-B01 | MSVC | x64 / Debug | 默认（/MT、CEF 142、无 SDL） | 库 + 全部示例 | `python build/build.py build --config Debug` | 通过 | 861/861；过滤其他平台入口后增量构建为 21 次链接，新生成的 exe 均写入 `bin/.build-config/` |
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
| W-B17 | MSYS2 MINGW32 | x86 / Release | 默认 | 库 + 示例 | — | 未测 | 环境缺 gcc/make，需先补齐工具链 |
| W-B18 | MSVC | arm64 / Debug/Release | 默认 | 库 + 示例 | `... --arch arm64` | 未测 | VS 未安装 arm64 工具集 |
| W-B19 | MSVC | x64 / Debug | `--dry-run` | 仅打印命令 | `... --config Debug --dry-run` | 通过 | 不执行配置/编译，不输出“编译完成” |
| W-B20 | MSVC | x64 / Debug | 复用构建目录 | 有状态选项回归 | 先 `--no-examples --md --sdl`，再默认构建 | 通过 | 默认值显式传入，不继承旧选项 |
| W-B21 | MSVC | x64 / Debug | `--dll` | 库 + 全部示例 | `... build --dll --config Debug` | 通过 | 802/802；`bin/duilib_d.dll` 36,225,024 B；basic/ChildWindow 启动正常（窗口标题正确） |
| W-B22 | MSVC | x64 / Release | `--dll` | 库 + 全部示例 | `... build --dll --config Release` | 通过 | 24 个增量链接；`bin/duilib.dll` 13,598,720 B、`lib/x64/Release/duilib.lib` 导入库；basic/ChildWindow 启动正常 |
| W-B23 | MSVC | x86 / Debug | `--dll` | 库 + 全部示例 | `... build --dll --config Debug --arch x86` | 通过 | 日志含 `mt → dll` 自动清理；802/802；`bin/duilib32_d.dll` 29,417,472 B、`lib/x86/Debug/duilib.lib` 导入库；basic/ChildWindow 启动正常（Debug CRT 14.44 已在 SysWOW64） |
| W-B24 | MSVC | x86 / Release | `--dll` | 库 + 全部示例 | `... build --dll --config Release --arch x86` | 通过 | 802/802；`bin/duilib32.dll` 11,598,336 B、`lib/x86/Release/duilib.lib` 导入库；basic/ChildWindow 用 app-local CRT 启动正常 |
| W-B25 | MSVC | x64 / Debug ↔ Release | 静态 ↔ DLL 模式切换 | 模式切换自动清理 | 依次 `build --config Debug`、`build --dll --config Debug`、再静态 | 通过 | 日志含 `dll → mt`，构建树重建、`bin/duilib_d.dll` 删除、`duilib_dll.flag` 同步为 OFF；307/307 得到 `lib/x64/Debug/duilib.lib`（256,775,712 B 静态库）。反向 `mt → dll` 见 W-B23 |
| W-B26 | MSBuild | x64 / Release | `duilib_dll.flag` | duilib + 第三方 + wrapper | 先无 flag 构建 `build/duilib.sln`，再创建 flag 重建 | 通过 | 静态库 870,484,530 B；加 flag 后 6 个工程 `duilib_crt_mode.txt` 变为 `dll`，wrapper/109/第三方与 duilib 全部重编；无 LNK4098/LNK2038；`bin/duilib.dll` 17,366,016 B，导入 `turbojpeg.dll`、`MSVCP140.dll`、`libcef.dll`，导入库 4,349,026 B，`bin/turbojpeg.dll` 自动复制 |

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
  `python build/build.py build --config Debug` 重新链接 x64 Debug 静态示例
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
| W-P01 | x64 Debug + 示例 + CEF 142 | `python build/build.py pack --config Debug --with-examples --with-cef` | 1316 / 412.3 MB | 通过 |
| W-P02 | x64 Debug + 示例 + CEF 109 | `... --config Debug --with-examples --with-cef --cef-version 109` | 1134 / 373.5 MB | 通过 |
| W-P03 | x86 Debug + 示例 + CEF 142 | `... --config Debug --arch x86 --with-examples --with-cef` | 1314 / 368.1 MB | 通过 |
| W-P04 | x64 Release + 示例 + CEF 142 | `... --config Release --with-examples --with-cef` | 1316 / 328.1 MB | 通过 |
| W-P05 | x86 Release + 示例 + CEF 142 | `... --config Release --arch x86 --with-examples --with-cef` | 1314 / 287.8 MB | 通过 |
| W-P06 | x64 Release + 示例 + CEF 109 | `... --config Release --with-examples --with-cef --cef-version 109` | 1134 / 280.8 MB | 通过 |
| W-P07 | x64 Debug + 示例 + SDL + MD | `... --config Debug --md --with-examples --with-sdl --with-cef` | 1396 / 416.5 MB | 通过 |
| W-P08 | x64 Release + 示例 + MD | `... --config Release --md --with-examples --with-cef` | 1316 / 304.2 MB | 通过 |
| W-P09 | x64 Release + 示例 + SDL | `... --config Release --with-examples --with-sdl --with-cef` | 1396 / 352.8 MB | 通过 |
| W-P10 | x64 Release + 示例，无 CEF | `... --config Release --with-examples` | 762 / 121.8 MB | 通过 |
| W-P11 | x64 Debug，仅库无示例 | `... --config Debug` | 546 / 63.5 MB | 通过 |
| W-P12 | MinGW gcc Release + 示例（无 CEF） | `... --config Release --with-examples` | 757 / 208.2 MB | 通过 |
| W-P13 | llvm-mingw Release + 示例（无 CEF） | 同上（MinGW 环境） | 757 / 147.0 MB | 通过 |
| W-P14 | MinGW gcc Release + 示例 + SDL（无 CEF） | `... --config Release --with-examples --with-sdl` | 837 / 243.0 MB | 通过 |
| W-P15 | MinGW 动态链接 Release + 示例（无 CEF） | `... --config Release --with-examples`（源码关闭静态链接） | 757 / 195.4 MB | 通过，有限制 |
| W-P16 | x64 Debug，zip / tgz / tar.bz2 | `pack.py --config Debug --format zip\|tgz\|tarbz2` | 546 / 63.5 MB 各一份 | 通过 |
| W-P17 | 显式 MSVC 运行库 MT / MD | `pack.py --msvc-runtime mt` / `md` | 元数据与库一致 | 通过 |
| W-P18 | x64 Debug DLL + 示例 + SDL + CEF | `pack.py --config Debug --with-examples --with-sdl --with-cef` | 1396 / 219.6 MB | 通过 |
| W-P19 | x64 Release DLL + 示例 + SDL + CEF | `pack.py --config Release --with-examples --with-sdl --with-cef` | 1396 / 210.1 MB | 通过 |
| W-P20 | x86 Debug DLL + 示例 + SDL + CEF | `pack.py --config Debug --arch x86 --with-examples --with-sdl --with-cef` | 1394 / 191.6 MB | 通过 |
| W-P21 | x86 Release DLL + 示例 + SDL + CEF | `pack.py --config Release --arch x86 --with-examples --with-sdl --with-cef` | 1394 / 183.4 MB | 通过 |
| W-P22 | MSBuild DLL（动态 turbojpeg，无 SDL） | MSBuild 构建后 `pack.py --config Release`（不加 `--with-sdl`） | 1099 / 200.2 MB | 通过 | 包名 `win-x64-cef-dll`；`JPEG=1`、`SDL3=0`；含 `bin/duilib.dll` + `bin/turbojpeg.dll`，无 `turbojpeg-static.lib`；`--verify` 与 SDK 测试（PASS=15 FAIL=0 SKIP=4）通过；修复 P04/C02 对 DLL 包 `HAS_JPEG` 的误判 |

打包专项说明：

- W-P15：关闭静态链接后示例依赖 MinGW 运行库，当前构建与打包不会自动收集这些 DLL；
  分发前需手工复制到 exe 同级。包内 README 已提示。
- W-P18 ~ W-P21：DLL 包由 `build.py build --dll`（CMake `DUILIB_DLL=ON`）产物打出，
  不再依赖历史/外部 DLL 包。包名带 `-dll`，元数据 `IS_DLL=1`、`MSVC_RUNTIME=MD`，
  `lib/` 只要求 `duilib.lib` 导入库。`pack.py` 通过 duilib DLL 导出表判断 SDL
  支持，DLL 以 SDL 编译时必须加 `--with-sdl`，否则直接报错（避免下游
  `DUILIB_SDL=0` 与 DLL 的 Control/WindowBase 类布局不一致）。
- W-P18 ~ W-P21 四个 DLL 包均已通过 `pack.py --verify` 与 `sdk_package_test.py`
  （PASS=15 FAIL=0 SKIP=4：C02 覆盖 DLL 消费者，C01/C06/C10/C11 按平台或参数跳过）。
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
- 包名模式：`nim_duilib-<版本>-win-<arch>[-sdl][-cef|-cef109]-examples[-dll][-debug].zip`。

## 3. Linux 构建与打包矩阵

Unix 平台一次只构建一个配置，库输出到 `lib/<Config>`；`--md` 仅 MSVC 有效。

| 编号 | 平台 / 工具链 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| L-B01 | Linux gcc | x64 / Release | 默认（SDL 默认开启） | `python3 build/build.py --config Release` | 历史通过 | 构建 + 出包已验证 |
| L-B02 | Linux clang | x64 / Release | 默认 | 同上，`CC/CXX=clang/clang++` | 未测 | 预检与 Skia 工具链优先 clang |
| L-B03 | Linux | x64 / Debug | 默认 | `python3 build/build.py --config Debug` | 未测 | |
| L-B04 | Linux | arm64 / Release | 默认 | `--arch arm64` | 未测 | 缺真机/交叉工具链 |
| L-B05 | Linux gcc | x64 / Release | 关闭 SDL | `--no-sdl` | 未测 | |
| L-B06 | Linux | x64 / Release | `--with-cef` 打包 | `pack --with-cef` | 未测 | 代码路径有单元测试，缺实机包 |
| L-B07 | Linux | x64 / Release | 运行验证 | 解包运行示例 | 未测 | 依赖系统 X11/字体 |

## 4. FreeBSD 构建与打包矩阵

FreeBSD 复用 Linux/X11 分支；不支持 CEF，`--with-cef` 直接报错。

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| F-B01 | x64 / Release | 默认 | `python3 build/build.py --config Release` | 历史通过 | 构建 + 出包 |
| F-B02 | x64 / Debug | 默认 | `--config Debug` | 未测 | |
| F-B03 | x64 / Release | `--with-cef` | `pack --with-cef` | 历史通过（负向） | 预期报错，CEF 不支持 |
| F-B04 | x64 / Release | 运行验证 | 解包运行示例 | 未测 | |

## 5. macOS 构建与打包矩阵

本轮环境：macOS x64、AppleClang 21、CMake 4.0.2、Ninja、Python 3.13.3；
Skia `llvm.x64.release`、SDL3 `main`、CEF 142/x64。

2026-10-28 macOS 真机复测：Release/Debug 构建、6 个出包、8 次包测试（含两次
`--run-examples`），以及 `build.py doctor/test`、`test_build.py`、`test_pack.py`、
`test_sdk_package.py`、`test_autotest.py`、`autotest.py` 全部通过；只改资源的增量同步
（M-B07）手测通过；当前仅 arm64 无测试环境（命令清单见 5.3）。

### 5.1 构建

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| M-B01 | x64 / Release | 默认（CEF 142、SDL 默认开启） | `python3 build/build.py` | 通过 | 全量构建通过；`bin/` 示例与 `.app` 正常更新 |
| M-B02 | x64 / Debug | 默认 | `python3 build/build.py --config Debug` | 通过 | 全量构建通过；切换配置后示例重新链接 |
| M-B03 | arm64 / Release | 默认 | `--arch arm64` | 未测 | 无 arm64 测试环境 |
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
（修改、新增、删除 XML 三种改动均由每次构建同步进 `basic.app`）。唯一未覆盖项为
`M-B03`（arm64），当前没有 arm64 测试环境。

## 6. 跨平台通用命令矩阵

| 编号 | 命令 | 覆盖内容 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- |
| G-01 | `build.py doctor` | 工具链、版本清单、依赖目录、预检 | 通过 | Windows 本轮；Linux/FreeBSD 历史；macOS 2026-10-28 真机通过 |
| G-02 | `build.py deps --offline` | 离线采纳已有 Skia/CEF/SDL3；本地源码不在清单 ref 上时先切到本地已有的提交 | 通过 | 不访问网络，stamp 校验；Windows 实测 SDL `main` ↔ `release-3.4.18` 双向切换均可用 |
| G-03 | `build.py clean` | 删除整个 `out/`；`--build-dir` 可只清理指定构建目录 | 通过 | dry-run 仅打印路径 |
| G-04 | `build.py clean --deps --dry-run` | 展示依赖清理范围 | 通过 | 不会删除源码克隆 |
| G-05 | `build.py build/pack/test --dry-run` | 只打印命令 | 通过 | build/clean 不再误报完成 |
| G-06 | `build.py test` | 构建/打包单元测试 | 通过 | 202 项全部通过（含 `bin/.build-config/` 新路径、旧标记兼容、标记目录不被误收、Debug/Release 切换清理）；macOS 2026-10-28 复测 `build.py test`、`test_build.py`、`test_pack.py`、`test_sdk_package.py`、`test_autotest.py` 均通过 |
| G-07 | 负向校验 | 混合库、架构不一致、CEF 版本不匹配、错误 ref/archive | 通过 | 均在配置/打包阶段明确报错 |

## 7. 当前测试进展（2026-10-28）

| 范围 | 已完成 | 结果 |
| :--- | :--- | :--- |
| Windows P1（关键出包组合） | 7 / 7 | 全部通过 |
| Windows P2（参数分支与工程逻辑） | 9 / 9 | 全部通过 |
| Windows 自动测试 | 158 项 | 全部通过 |
| Linux x64 Release 构建与打包 | 历史验证 | 通过（运行时未验证） |
| FreeBSD x64 Release 构建与打包 | 历史验证 | 通过（CEF 不支持，运行时未验证） |
| macOS x64 Release/Debug 构建 | 2 / 2 | 全部通过（含 CEF `.app`，见 5.3） |
| macOS 普通示例 Bundle（`basic.app`） | 2 / 2 | Bundle 生成、`Info.plist` 保留、切换配置后首次启动正常（M-B06）；只改资源的增量同步手测通过，修改/新增/删除 XML 均生效（M-B07） |
| macOS x64 出包 | 6 个包 | 全部通过（Release/Debug × 仅库/示例/CEF 示例） |
| macOS 包测试（`sdk_package_test.py`） | 8 / 8 | 全部通过（含 Release/Debug 示例包的 `--run-examples`） |
| macOS 脚本测试（`doctor`/`test`/`test_*`/`autotest`） | 7 项 | 全部通过（见 5.3） |
| macOS 示例启动/运行验证（`--run-examples`） | 2 次 | 通过：Release、Debug 示例包均 `[PASS] C10 examples.smoke`（M-T07/M-T08） |
| arm64（Windows/Linux/macOS） | 0 | 未测（无 arm64 测试环境） |

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

### 2026-10-27 依赖版本一致性（Windows 已验证）

| 问题 | 处理 | 验证 |
| :--- | :--- | :--- |
| `<deps-root>/SDL` 停在 `main`，清单为 `release-3.4.18`，脚本仍编译本地源码并复用旧 stamp | `deps` 先按清单切换 tag/commit（本地已有提交不联网，`--offline` 可用）；`.duilib-sdl` 记录源码提交，切换后旧产物重建 | 当时清单为 `release-3.4.18`：`build.py deps --sdl --offline` 使 HEAD `3b2bc95` → `829a65d`，SDL3 重建，stamp 含 `829a65d...`；二次运行 `[OK] SDL3 is ready` |
| 清单默认 ref 为 `release-3.4.18` tag，但其源码编译报错 | 默认 ref 改为 `main` 分支；`release-3.4.18` 保留在 `refs` 中，需要时用 `--sdl-ref` 选择 | `build.py deps --sdl --offline`：HEAD `829a65d`（tag）→ `3b2bc95`（main），SDL3 重建，stamp 为 `main|3b2bc95...`；二次运行 `[OK] SDL3 is ready`，未再重建 |
| `skia_compile` 同名补丁归档被替换成旧版本后仍直接复用，Skia 产物不更新 | 先 `git status` 核对归档与当前提交，必要时 `git checkout -- <zip>` 恢复，再 `git pull`；补丁指纹变化时重新解压并重建 Skia | 篡改本地 zip 后 `doctor` 输出 `[WARN] skia patch differs from skia_compile HEAD`；`deps --offline` 恢复归档（`hash-object` 与 `HEAD:zip` 一致），Skia 未无谓重建 |
| `--skia-ref` 选中的 tag/branch 不检查本地工作区 | 与 commit ref 一样按清单对齐；branch 只要求当前在该分支，不追远端 | 临时仓库隔离验证：tag、branch 均可切换；离线缺 ref 时明确报错 |

### 已知缺口与风险

1. MinGW `DUILIB_MINGW_STATIC=OFF` 的包不包含 `libstdc++-6.dll` 等运行库，分发前需手工补齐。
2. DLL 模式已由 `build.py --dll` 与 `duilib_dll.flag` 两个入口产出，x64/x86 ×
   Debug/Release 的构建、打包与 SDK 接入已覆盖（见 W-B21 ~ W-B26、W-P18 ~ W-P22）；
   2026-10-24 生成的旧 `dist/` DLL 包（含 `/MT` 的 `turbojpeg-static.lib`）已删除，
   `dist/` 中只保留本轮通过校验的新 DLL 包（W-P18 ~ W-P21、W-P22）。
3. Linux/FreeBSD 仅验证 Release 构建与打包，Debug、解包运行、下游工程接入未验证。
4. macOS x64 的 Release/Debug 构建、`basic.app`（`Info.plist` 保留、切换配置后首次启动
   正常、只改资源的增量同步）、6 个出包、8 次包测试（含两次 `--run-examples`）与全部脚本
   测试均已通过（2026-10-28 真机复测）；仅 arm64 未覆盖（无测试环境）。
5. Windows/Linux/macOS 的 arm64 均未实测；Windows 缺 arm64 MSVC 工具集。
6. `bin/` 与 `lib/<arch>/<Config>` 被多架构/多工具链共用，切换后必须清理，否则打包
   会因 PE 架构或混合库校验失败（这是预期保护，不是缺陷）。
7. MinGW 默认并行度在内存较小的机器上可能 OOM，回归时优先 `--jobs 2`/`--jobs 3`。
8. Unix 的 SDL3 stamp 不再含配置分量（修复前切换 Debug/Release 会无谓重建 SDL3）；
   需在 Linux/macOS 真机验证：先 Release 后 Debug 各跑一次 `build.py deps`，第二次
   不应重新编译 SDL3。
9. Unix 的 Skia 静态库改为绝对路径链接（与 Windows 一致）；需在 Linux/macOS 真机
   验证：重新编译 Skia 后 `ninja` 能触发示例程序重新链接，且链接命令不再依赖
   `link_directories` 的搜索顺序。
10. `_duilib_cef_tracked_dir` 已按平台返回跟踪目录（修复前 Linux/macOS 错对
    `libcef_win` 的跟踪头）；需在 Linux/macOS 真机验证一次 `build.py deps` 的 CEF
    下载与迁移采纳路径。
11. SDL3 源码 ref 对齐与源码提交 stamp 仅在 Windows 验证（`main` ↔
    `release-3.4.18` 双向切换并重建，清单默认已改为 `main`）；需在
    Linux/macOS/FreeBSD 真机验证：本地 `SDL/` 不在清单分支时能离线切换，
    切换后 `.duilib-sdl` 记录新提交并触发一次重建，二次运行直接复用。
12. `skia_compile` 补丁归档刷新（旧归档恢复 + `git pull` + 补丁指纹触发重建）仅在
    Windows 验证；需在 Unix 真机验证 `git status` 少文件路径在子目录下的行为一致，
    以及本地归档缺失时 `deps` 仍能从仓库恢复而不联网。
13. Skia tag/branch ref 对齐在临时仓库中隔离验证；需在真实 `skia/` 工作区验证
    `--skia-ref` 选 branch 时不会误 fetch 或切到 detached HEAD。

## 8. 回归执行顺序建议

1. `build.py test`，确认构建/打包脚本自身无回归。
2. Windows 默认 x64 Debug 全量构建，作为基线。
3. 按 W-B02 ~ W-B15 逐项构建，每项后立即出包并 `pack.py --verify`。
4. 切换工具链/架构前清理 `bin/*.exe`、`bin/*.dll` 与 `lib/<arch>/<Config>`。
5. Windows 完成后执行 G-01 ~ G-05 通用命令分支。
6. Linux、FreeBSD、macOS 按各自矩阵执行，Release 优先；有条件再补 Debug 与运行验证。
7. 回归结束恢复默认 x64 Debug 构建，并重新生成最终包校验。
