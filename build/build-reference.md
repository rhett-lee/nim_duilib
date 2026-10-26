# 编译参考手册

本文是 nim_duilib 的唯一编译参考手册，适用于命令行构建、依赖准备、出包与环境排查。
旧的分平台构建脚本（`build_duilib_all_in_one.*`、`msvc_build.bat`、`linux_build.sh` 等）
已归档到 `build/legacy/`、不再参与构建（待全平台验证后删除），统一使用 `python3 build/build.py`。

英文版：[build-reference.en.md](build-reference.en.md)

## 1. 环境要求

| 项目 | 要求 |
| :--- | :--- |
| CMake | 3.24 及以上（根工程使用 CMakePresets） |
| Python | **Python 3（必需）**；Skia 的构建脚本固定调用 `python3`，`python` / `py -3` 不能替代 |
| Git | 用于获取依赖源码 |
| Ninja | Windows 由 Visual Studio 自带；其他平台用包管理器安装 |
| Windows | Visual Studio 2022 及以上（含 C++ 工作负载），或 MSYS2 / MinGW-w64 |
| Linux | gcc 10+ 或 clang 12+，以及 X11/字体等开发包 |
| macOS | Xcode 命令行工具 13.5+，Homebrew 安装 cmake/ninja/gn |
| FreeBSD | clang、cmake、ninja、gn、fontconfig、freetype2 |

Python 3 缺失时 `build.bat` / `build.py` 会直接报错并给出安装提示，不提供绕过
Python 的构建路径。只有 `python` / `py -3`、PATH 中没有 `python3` 时，预检给出
告警，并在实际编译 Skia 前报错终止。

本文命令统一写作 `python3`：macOS/Linux/FreeBSD 通常没有 `python` 命令；Windows
可运行 `build\build.bat`（参数相同，无需 `python3`），或把命令中的 `python3`
换成 `python`。

## 2. 快速开始

```bash
python3 build/build.py                  # Release：自动准备依赖 + 编译库与示例
python3 build/build.py --config Debug   # Debug（独立构建目录）
python3 build/build.py doctor           # 环境自检（无副作用）
python3 build/build.py pack --with-cef  # 出 SDK 包
```

Windows 也可以直接运行：

```bat
build\build.bat --config Debug
```

构建目录固定为 `out/<arch>/<Config>`（例如 `out/x64/Debug`），与 `build/` 平级，
不写入 `build/build_temp`。可执行文件与运行时资源始终输出到仓库根 `bin/`，
库文件输出到 `lib/<arch>/<Config>/`（Unix 为 `lib/<Config>/`）。

## 3. 子命令

| 命令 | 说明 |
| :--- | :--- |
| `build.py`（默认） | 等价于 `build.py build`：先确保依赖就绪，再编译一个配置 |
| `build.py deps` | 只准备当前配置所需的 Skia / SDL3 / CEF |
| `build.py build` | 编译一个配置；默认先准备依赖，`--no-deps` 跳过 |
| `build.py test` | 运行 `build/` 下的标准库 unittest（含 `test_build.py`） |
| `build.py pack` | 调用 `pack.py` 出 SDK 包 |
| `build.py doctor` | 环境自检：工具链、版本清单、依赖目录、根工程预检（含编译器、`gn`） |
| `build.py clean` | 清理构建树；`--deps` 同时清理依赖产物 |

一次调用只构建一个架构 + 一个配置。需要 Debug 与 Release 时调用两次：

```bash
python3 build/build.py --config Debug
python3 build/build.py --config Release
```

## 4. 常用参数

| 参数 | 说明 |
| :--- | :--- |
| `--config Release\|Debug` | 构建配置，默认 `Release`；不提供 `--debug` 别名 |
| `--arch auto\|x86\|x64\|arm64` | 目标架构，默认主机架构；Windows + Ninja 交叉编译 arm64 时必须显式指定 |
| `--lang auto\|zh_CN\|en` | 交互语言，默认跟随系统；也读取环境变量 `DUILIB_LANG` |
| `--build-dir <dir>` | 覆盖构建目录，允许放到仓库外 |
| `--deps-root <dir>` | 依赖源码/安装根目录，默认仓库同级目录 |
| `--jobs N` | 并行编译任务数 |
| `--offline` | 只使用已缓存的依赖源码/归档，不联网 |
| `--dry-run` | 只打印将执行的命令 |
| `--cmake-arg <arg>` / `--build-arg <arg>` | 透传给 configure / build；值以 `-` 开头时写成 `--cmake-arg=-DFOO=1` |
| `--vs-version N` | 显式指定 Visual Studio 主版本（如 `17`） |
| `--cef-version 109\|142` | CEF 版本，默认取版本清单 |
| `--skia-ref` / `--sdl-ref` | 选择版本清单中的 ref |
| `--sdl` / `--no-sdl` | 显式启用/关闭 SDL；Windows 默认关闭，其他平台强制开启 |
| `--md` | 使用 MSVC 动态运行库（/MD、/MDd） |
| `--cef-archive <file>` | 使用本地 CEF 归档，跳过下载 |
| `--no-deps`（build） | 跳过依赖准备，直接构建 |
| `--target <name>`（build） | 只构建指定目标，如 `duilib` |
| `--no-examples`（build） | 不编译示例程序 |
| `--force`（deps） | 强制重建依赖 |
| `--with-cef` / `--with-sdl` / `--with-examples`（pack） | 包内包含对应内容 |
| `--out-dir`（pack） | 指定包输出目录 |

`--sdl` 只是编译开关；`pack` 要在包内包含 SDL3 时必须另加 `--with-sdl`
（只写 `--sdl` 时 `build.py` 会打印提示，打包内容保持不变）。

`out/<arch>/<Config>` 会复用 CMake 缓存，而 `DUILIB_MD`、`DUILIB_BUILD_EXAMPLES`、
`DUILIB_ENABLE_SDL`、`DUILIB_CEF_VERSION`、`DUILIB_SKIA_REF` 等都是有状态选项。
`build.py` 每次都会显式传入解析后的取值，所以先跑 `--no-examples --md`、再跑默认
命令不会沿用旧选项；只有绕过 `build.py` 直接用 `cmake` 配置同一目录时才需要自己
清理缓存（删除构建目录后重新配置）。

## 5. 依赖准备

依赖目录默认位于仓库同级（可用 `--deps-root` 覆盖）：

```
<deps-root>/
  skia/            # Skia 源码（由 skia_compile 补丁构建）
  skia_compile/    # skia_compile 仓库（缺少补丁归档时由 deps 自动克隆）
  SDL/             # SDL3 源码
  SDL3/            # SDL3 安装前缀
  SDL3_build/      # SDL3 构建目录
  cef_binary/      # CEF 下载/解压缓存（macOS 为完整分发目录）
```

Skia 的补丁归档由 `build.py deps`（以及默认的 `build`）自动获取：缺少时克隆整个
[skia_compile](https://github.com/rhett-lee/skia_compile) 仓库到
`<deps-root>/skia_compile`；仓库已存在则执行 `git pull` 更新，更新失败时继续使用
本地已有内容（与旧脚本一致）。直接用 CMake 构建（`DUILIB_DEPS_MODE=auto` 或
`duilib_deps` 目标）时由依赖脚本执行同样的克隆/更新逻辑。`--offline` 下不联网，
需要提前自备补丁归档；`doctor` 会显示该归档是否存在。

所有网络操作（git clone/fetch、CEF 下载）共用一套重试参数，维护在
[third_party_versions.json](../cmake/third_party_versions.json) 的 `retry` 段：

```json
"retry": { "maxAttempts": 0, "delaySeconds": 20 }
```

`maxAttempts=0` 表示无限重试（默认），`delaySeconds` 为两次尝试之间的固定间隔。
`git pull` 例外：只尝试一次，失败仅告警并继续使用本地内容（与旧脚本一致）。

版本由 [third_party_versions.json](../cmake/third_party_versions.json) 统一锁定：

- **Skia**：固定 commit（`skia.default`）+ 对应补丁 zip；Windows 使用 VS 自带
  clang/ninja。非 Windows 平台优先用 clang/clang++ 编译（GCC 版 Skia 运行性能明显
  较差），只有 clang 不可用时才回退 gcc/g++；产物目录相应为
  `out/llvm.<arch>.release` 或 `out/gcc.<arch>.release`。Debug 复用 Release 编译
  模式，仅切换运行库（`/MTd` 或 `/MDd`），因此每种配置需要各自的 Skia 变体目录。
- **SDL3**：固定 tag（`sdl.default`）。Windows 默认不启用；显式启用时由 `deps`
  自动构建 Debug + Release 静态库。Unix 构建动态库，并写入 rpath。
- **CEF**：固定版本分支（`cef.default`，可选 `109` 兼容 Windows 7）。统一使用
  `_minimal` 发行包（体积约为标准包的一半，含 `include/`、`Release/`、
  `Resources/`），只安装 `Release/*` 与 `Resources/*` 运行时，放到：
  - Windows：`bin/libcef_win/<arch>`（142）或 `bin/libcef_win_109/<arch>`（109）
  - Linux：`bin/libcef_linux`
  - macOS：`<deps-root>/cef_binary`
  编译使用仓库中已跟踪的头文件与导入库；下载包内的 `CEF_VERSION` 必须与跟踪
  头文件一致，否则报错。Debug 使用 Release 运行时。MinGW 不支持 CEF。

已有依赖产物但没有 stamp 文件时，`deps` 会校验版本后直接采纳（Skia 校验源码
HEAD，CEF 校验跟踪头文件主版本），避免重复编译或重复下载。

依赖模式由 `DUILIB_DEPS_MODE` 控制：`off`（只检查）、`prepare`（由构建目标
准备）、`auto`（配置阶段准备）。`build.py` 默认使用 `prepare`，`clean` 默认
不删除依赖缓存。

## 6. 构建产物

| 路径 | 内容 |
| :--- | :--- |
| `out/<arch>/<Config>/` | CMake 构建树（可随时删除） |
| `lib/<arch>/<Config>/` | Windows 静态库；Unix 为 `lib/<Config>/` |
| `bin/` | 示例程序可执行文件、`resources/`、CEF 运行时 |
| `bin/resources/` | XML 皮肤、图片、`fonts/`、`lang/`、`themes/` |

`bin/resources` 的发布裁剪规则：`fonts/`（未用自定义字体可删）、`lang/`（未用
多语言可删）、`themes/default/`（只保留 `public/` 与 `global.xml`）。需要把资源
打包进 exe 时，将整理后的 `resources/` 压成 `resources.zip` 替换 `bin/resources.zip`
后重新编译，参见 `examples/basic`。

CEF 运行时目录需要随程序一起发布；CEF 的用法见 [docs/CEF.md](../docs/CEF.md)。
只有 `cef` 与 `CefBrowser` 示例链接 CEF（`DUILIB_EXAMPLE_CEF`），其余示例的可执行
文件不导入 `libcef`；`DUILIB_ENABLE_CEF` 只决定库是否编译 CEF 控件源码。

## 7. 出包

SDK 只由 `pack.py` 生成，根 CMake 工程不提供 `install()` 目标。

```bash
python3 build/build.py pack                    # Release SDK
python3 build/build.py pack --config Debug     # Debug SDK（包名带 -debug）
python3 build/build.py pack --with-cef         # 包含 CEF 运行时
python3 build/build.py pack --with-examples    # 包含示例程序
python3 build/build.py pack --out-dir dist     # 指定输出目录
```

包内包含 `include/`、`lib/<arch>/<Config>/`、`bin/`、`lib/cmake/nim_duilib/`、
`LICENSES/` 与 `THIRD-PARTY-NOTICES.md`。合规要求：分发 SDK 时必须保留
`LICENSES/` 与 `THIRD-PARTY-NOTICES.md`。

只给 `--with-examples` 而不给 `--with-cef` 时，会自动跳过 `cef` 与 `CefBrowser`
（Windows 为 `cef.exe`/`CefBrowser.exe`），避免包内留下缺少 libcef 运行时、
必然启动失败的程序；需要可运行的 CEF 示例请同时加 `--with-cef`。

macOS 例外：CEF 示例是 App Bundle（`bin/cef.app`、`bin/CefBrowser.app` 目录）。
`--with-examples` 时同样以 `--with-cef` 决定是否随包：未指定则跳过并告警，指定后
整个目录（含 `Contents/Frameworks` 下的 CEF Framework 与 Helper）按原结构打包。
Bundle 自带运行时，因此 macOS 的 `--with-cef` 只控制示例，不收集 CEF 开发文件与
运行时目录（SDK 不含 macOS CEF 开发文件，需按 [Release.md](../docs/Release.md)
的 macOS 章节从源码树构建）。

## 8. 平台 × 编译器 × 依赖 × 可选模块

| 平台 | 编译器 | Skia | SDL3 | CEF | WebView2 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Windows | MSVC 2022+ | 必须 | 默认关闭，可 `--sdl` 启用 | 默认启用（109/142） | 支持 |
| Windows | MSYS2 / MinGW-w64 gcc、clang | 必须 | 默认关闭，可启用 | 不支持 | 支持 |
| Linux | gcc 10+ / clang 12+ | 必须 | 强制启用 | 默认启用 | 不支持 |
| macOS | clang（Xcode） | 必须 | 强制启用 | 默认启用 | 不支持 |
| FreeBSD | clang | 必须 | 强制启用 | 不支持 | 不支持 |

Linux 各发行版所需的系统包（字体、GL、X11、ibus 等）参考下表：

| 发行版 | 桌面 | 安装命令（必选） |
| :--- | :--- | :--- |
| OpenEuler | UKUI/DDE (X11) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel` |
| OpenKylin | Wayland | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| UbuntuKylin | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| 中科方德 | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| 统信UOS | X11 | `sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| Ubuntu | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip bzip2 libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| Debian | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| Fedora | GNOME (Wayland) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel` |
| OpenSuse | KDE (X11) | `sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel` |

各发行版的 Skia 编译细节见 skia_compile 仓库中的 `compile_skia_on_<platform>.md`。
macOS 需要 `brew install cmake ninja` 并从源码安装 `gn`；FreeBSD 执行
`sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2`。

### 强制指定编译器

默认由 CMake 选择编译器：`CC`/`CXX` 环境变量优先，否则使用 `cc`/`c++`
（Linux 上通常指向 gcc/g++）。需要固定工具链时用以下任一方式：

```bash
# 方式一：环境变量（推荐，对 doctor/deps/build 全部生效）
CC=clang CXX=clang++ python3 build/build.py
CC=gcc   CXX=g++     python3 build/build.py

# 方式二：通过 --cmake-arg 传给 CMake configure
python3 build/build.py --cmake-arg -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
python3 build/build.py --cmake-arg -DCMAKE_C_COMPILER=gcc   -DCMAKE_CXX_COMPILER=g++
```

- 非 Windows 平台的 `build.py doctor` 会按 `CC`/`CXX` 报告实际将使用的编译器。
- 切换编译器后无需手动删目录：`build.py` 发现 `out/<arch>/<Config>` 缓存中的编译器
  变化时，会自动清空该构建目录后重新配置。
- Windows/MSVC 不使用 `CC`/`CXX`，工具链由 vcvars 提供；MSYS2 中可用
  `CC=gcc CXX=g++` 或 `CC=clang CXX=clang++` 指定。
- 非 Windows 的 Skia 与主工程工具链解耦：Skia 始终优先 clang/clang++（GCC 版 Skia
  运行性能明显较差），因此主工程用 gcc 时也不会改变 Skia 使用 clang 的选择。

MSYS2 各环境使用各自目录下的工具链（MSYS2 终端中 `MSYSTEM` 与 `PATH` 已就绪）：

| 环境 | `MSYSTEM` | 工具链目录 | 建议的 `CC`/`CXX` | Skia 目录前缀 |
| --- | --- | --- | --- | --- |
| MINGW64（msvcrt） | `MINGW64` | `E:\msys64\mingw64\bin` | `gcc` / `g++` | `gcc.<arch>` |
| UCRT64 | `UCRT64` | `E:\msys64\ucrt64\bin` | `gcc` / `g++` | `gcc.<arch>` |
| CLANG64 | `CLANG64` | `E:\msys64\clang64\bin` | `clang` / `clang++` | `mingw64-llvm.<arch>`（回退） |

从 PowerShell 调用时同样导出 `MSYSTEM`，并把对应 `bin` 目录放到 `PATH` 最前面；
只要设置了 `MSYSTEM`，`build.py` 就不会执行 vcvars，而是使用 `CC`/`CXX` 指定的编译器。
独立版 llvm-mingw 也可用同样方式：设置 `MSYSTEM`，`CC`/`CXX` 指向其 `clang.exe` /
`clang++.exe` 的绝对路径，并保证使用的 Skia 是 `mingw64-llvm.<arch>` 变体。

## 9. 运行库切换

- 默认使用静态运行库（/MT、/MTd）。
- `--md` 切换为动态运行库（/MD、/MDd）；`pack.py` 端为 `--msvc-runtime md`。
- Skia 与 nim_duilib 的运行库必须一致；`deps` 会按配置构建对应的 Skia 变体：
  `/MT` 使用 `llvm.<arch>.debug` / `llvm.<arch>.release`，`/MD` 使用
  `llvm.<arch>.debug.mdd` / `llvm.<arch>.release.md`。
- 若 nim_duilib 最终编译为 DLL，必须使用动态运行库。

## 10. 排障

| 现象 | 处理 |
| :--- | :--- |
| `Visual Studio not found` | 安装 VS 2022+ 与 C++ 工作负载；或用 `--vs-version` 指定版本 |
| `Skia not found` | 首次执行 `python3 build/build.py deps`，或检查 `--deps-root` |
| `gn/ninja not found` | Windows 使用 VS 自带目录；Unix 安装 `ninja` 与 `gn`。Unix 不会使用 Skia 目录里的 `gn.exe`（Windows 版无法运行），只有 `gn.exe` 时必须另装原生 `gn` |
| `python: command not found`（macOS/Linux） | 这些系统通常只有 `python3`：改用 `python3 build/build.py ...`；Windows 直接运行 `build\build.bat` |
| `python3 not found`（编译 Skia 时报错） | Skia 构建脚本固定调用 `python3`，不认 `python` / `py -3`；安装 Python 3 并确认 `python3 --version` 可用（Windows 可把 Python 安装目录加入 PATH） |
| `CMake version is too old` | 升级到 CMake 3.24+ |
| CEF 示例无法运行 | 确认 `bin/libcef_win/<arch>`（或 `bin/libcef_linux`）包含运行时；`pack --with-cef` 会带上 |
| 打包的示例程序加载了构建机的 `.so` | 示例程序已写入相对 rpath（`$ORIGIN/../lib`、`$ORIGIN/libcef_linux`），优先使用包内库；Linux 示例依赖 libcef 时请用 `pack --with-cef`，否则包内缺少 CEF 运行时 |
| CEF 下载失败 | 用 `--cef-archive <file>` 指定本地归档，或 `--offline` 使用缓存 |
| Linux 文本不显示 | 安装 fontconfig 与中文字体，确认 `bin/resources/fonts` 存在 |
| 构建目录损坏 | `python3 build/build.py clean` 后重试 |
| `clean` 删除失败（权限/占用） | 关闭占用构建树的程序（编辑器、运行中的示例、杀毒软件）后重试；错误信息会给出无法删除的具体路径 |
| 切换编译器后配置失败（如 cl.exe 找不到 `kernel32.lib`） | 无需手动清理：`build.py` 检测到 `out/<arch>/<Config>` 缓存中的编译器变化会自动清空该目录 |
| `Visual Studio C++ tools for <arch> are not installed` | 在 Visual Studio Installer 中为对应架构安装 MSVC 生成工具（如 ARM64）；仅有 x64/x86 工具集时不要构建 arm64 |
| MSYS2 CLANG64 配置期报 Skia 目录没有 `.a` | 已按候选顺序自动回退到 `mingw64-llvm.<arch>.release`；若仍无可用的 `.a` 目录，请用 `-DDUILIB_SKIA_LIB_SUBPATH=<目录>` 指定 clang64 编译的 Skia |
| `--cef-version 109` 构建报找不到 `libcef_dll_wrapper_109` | 已修复：wrapper 的源码目录与目标名现在跟随 `DUILIB_CEF_VERSION`；旧构建树需 `build.py clean` 后重新配置 |
| `--cef-version 109` Debug 链接报 `_ITERATOR_DEBUG_LEVEL` 0 vs 2（LNK2038） | 已修复：109 配置期强制 `USE_SANDBOX=OFF`，避免 wrapper 的 `_HAS_ITERATOR_DEBUGGING=0` 与 Debug 主工程冲突 |
| `pack` 报“库目录同时包含 MSVC（.lib）与 MinGW（.a）产物” | 切换工具链后旧库仍留在 `lib/<arch>/<Config>`；删除该目录（或 `build.py clean`）后用单一工具链重新构建 |
| `pack --with-cef` 报缺少所选版本的 wrapper | 编译与打包的 CEF 版本必须一致（如 `build --cef-version 109` 对应 `pack --with-cef --cef-version 109`）；错误信息会指出库目录中实际存在的版本 |
| `pack --with-examples` 报“bin 的架构与目标 x86/x64 不一致” | Windows 的 `bin/` 被多个架构共用；清理 `bin/` 中其它架构的 exe/dll，或只保留目标架构的构建产物后重新打包 |
| MinGW 编译或链接（尤其 Debug）报 `out of memory allocating ...` | 降低并行度：`--jobs 2`；链接阶段 GNU ld 已追加 `-Wl,--no-keep-memory`（clang/CLANG64 的 lld 不需要该参数） |
| MinGW 关闭静态链接（`DUILIB_MINGW_STATIC=OFF`）后示例启动缺 `libstdc++-6.dll` 等 | 该模式依赖编译器运行库，需把 `libstdc++-6.dll`、`libgcc_s_seh-1.dll`（x86 为 `libgcc_s_dw2-1.dll`）、`libwinpthread-1.dll` 复制到 exe 同级；默认静态链接无此问题 |
| `deps` 构建 SDL3 时出现 `cl.exe`/`LNK1104`，或切换 MSVC/MinGW 后 `--sdl` 失败 | SDL3 构建目录被其它工具链或运行库使用过；依赖准备会在 CRT 不一致时自动清理，直接重试即可 |
| MinGW Debug 链接报 `No space left on device` | MinGW Debug 示例单个约 200–280 MB，全部示例约 10 GB；清理 `bin/` 后重试，或改用 Release 配置 |

更细的旧脚本时代问题排查记录保留在 [build/README.md](README.md)。

## 11. 与旧脚本的差异

| 项目 | 旧脚本 | 现在 |
| :--- | :--- | :--- |
| 入口 | 11 个 .bat/.sh | `python3 build/build.py` |
| 配置 | 一次构建 Debug + Release | 一次一个 `--config`，调用两次 |
| 构建树 | `build/build_temp` | `out/<arch>/<Config>` |
| 依赖 | all-in-one 脚本内部处理 | `deps` 子命令，按版本清单准备 |
| SDK | 手工整理 | `build.py pack`（含 `LICENSES/`、`THIRD-PARTY-NOTICES.md`） |

VS 解决方案文件（`build/duilib.sln`、`examples.sln` 等）仍保留，使用前先运行
`python3 build/build.py deps` 准备依赖。

回归测试的全部构建/打包组合、各平台状态与执行顺序见 [test-matrix.md](test-matrix.md)。
