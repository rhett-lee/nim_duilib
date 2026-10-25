# 编译参考手册

本文是 nim_duilib 的唯一编译参考手册，适用于命令行构建、依赖准备、出包与环境排查。
旧的分平台构建脚本（`build_duilib_all_in_one.*`、`msvc_build.bat`、`linux_build.sh` 等）
已归档到 `build/legacy/`、不再参与构建（待全平台验证后删除），统一使用 `python build/build.py`。

英文版：[build-reference.en.md](build-reference.en.md)

## 1. 环境要求

| 项目 | 要求 |
| :--- | :--- |
| CMake | 3.24 及以上（根工程使用 CMakePresets） |
| Python | **Python 3（必需）**，Skia 的构建流程依赖 `python3` |
| Git | 用于获取依赖源码 |
| Ninja | Windows 由 Visual Studio 自带；其他平台用包管理器安装 |
| Windows | Visual Studio 2022 及以上（含 C++ 工作负载），或 MSYS2 / MinGW-w64 |
| Linux | gcc 10+ 或 clang 12+，以及 X11/字体等开发包 |
| macOS | Xcode 命令行工具 13.5+，Homebrew 安装 cmake/ninja/gn |
| FreeBSD | clang、cmake、ninja、gn、fontconfig、freetype2 |

Python 3 缺失时 `build.bat` / `build.py` 会直接报错并给出安装提示，不提供绕过
Python 的构建路径。

## 2. 快速开始

```bash
python build/build.py                  # Release：自动准备依赖 + 编译库与示例
python build/build.py --config Debug   # Debug（独立构建目录）
python build/build.py doctor           # 环境自检（无副作用）
python build/build.py pack --with-cef  # 出 SDK 包
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
| `build.py doctor` | 环境自检：工具链、版本清单、依赖目录、根工程预检 |
| `build.py clean` | 清理构建树；`--deps` 同时清理依赖产物 |

一次调用只构建一个架构 + 一个配置。需要 Debug 与 Release 时调用两次：

```bash
python build/build.py --config Debug
python build/build.py --config Release
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

## 5. 依赖准备

依赖目录默认位于仓库同级（可用 `--deps-root` 覆盖）：

```
<deps-root>/
  skia/            # Skia 源码（由 skia_compile 补丁构建）
  skia_compile/    # Skia 补丁归档（如 skia.2026-09-16.src.zip）
  SDL/             # SDL3 源码
  SDL3/            # SDL3 安装前缀
  SDL3_build/      # SDL3 构建目录
  cef_binary/      # CEF 下载/解压缓存（macOS 为完整分发目录）
```

版本由 [third_party_versions.json](../cmake/third_party_versions.json) 统一锁定：

- **Skia**：固定 commit（`skia.default`）+ 对应补丁 zip；Windows 使用 VS 自带
  clang/ninja，Unix 使用系统 LLVM。Debug 复用 Release 编译模式，仅切换运行库
  （`/MTd` 或 `/MDd`），因此每种配置需要各自的 Skia 变体目录。
- **SDL3**：固定 tag（`sdl.default`）。Windows 默认不启用；显式启用时由 `deps`
  自动构建 Debug + Release 静态库。Unix 构建动态库，并写入 rpath。
- **CEF**：固定版本分支（`cef.default`，可选 `109` 兼容 Windows 7）。只下载
  `Release/*` 与 `Resources/*` 运行时，放到：
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

## 7. 出包

SDK 只由 `pack.py` 生成，根 CMake 工程不提供 `install()` 目标。

```bash
python build/build.py pack                    # Release SDK
python build/build.py pack --config Debug     # Debug SDK（包名带 -debug）
python build/build.py pack --with-cef         # 包含 CEF 运行时
python build/build.py pack --with-examples    # 包含示例程序
python build/build.py pack --out-dir dist     # 指定输出目录
```

包内包含 `include/`、`lib/<arch>/<Config>/`、`bin/`、`lib/cmake/nim_duilib/`、
`LICENSES/` 与 `THIRD-PARTY-NOTICES.md`。合规要求：分发 SDK 时必须保留
`LICENSES/` 与 `THIRD-PARTY-NOTICES.md`。

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

## 9. 运行库切换

- 默认使用静态运行库（/MT、/MTd）。
- `--md` 切换为动态运行库（/MD、/MDd）；`pack.py` 端为 `--msvc-runtime md`。
- Skia 与 nim_duilib 的运行库必须一致；`deps` 会按配置构建对应的 Skia 变体。
- 若 nim_duilib 最终编译为 DLL，必须使用动态运行库。

## 10. 排障

| 现象 | 处理 |
| :--- | :--- |
| `Visual Studio not found` | 安装 VS 2022+ 与 C++ 工作负载；或用 `--vs-version` 指定版本 |
| `Skia not found` | 首次执行 `python build/build.py deps`，或检查 `--deps-root` |
| `gn/ninja not found` | Windows 使用 VS 自带目录；Unix 安装 `ninja` 与 `gn`。Unix 不会使用 Skia 目录里的 `gn.exe`（Windows 版无法运行），只有 `gn.exe` 时必须另装原生 `gn` |
| `CMake version is too old` | 升级到 CMake 3.24+ |
| CEF 示例无法运行 | 确认 `bin/libcef_win/<arch>`（或 `bin/libcef_linux`）包含运行时；`pack --with-cef` 会带上 |
| CEF 下载失败 | 用 `--cef-archive <file>` 指定本地归档，或 `--offline` 使用缓存 |
| Linux 文本不显示 | 安装 fontconfig 与中文字体，确认 `bin/resources/fonts` 存在 |
| 构建目录损坏 | `python build/build.py clean` 后重试 |
| 切换编译器后配置失败（如 cl.exe 找不到 `kernel32.lib`） | 无需手动清理：`build.py` 检测到 `out/<arch>/<Config>` 缓存中的编译器变化会自动清空该目录 |

更细的旧脚本时代问题排查记录保留在 [build/README.md](README.md)。

## 11. 与旧脚本的差异

| 项目 | 旧脚本 | 现在 |
| :--- | :--- | :--- |
| 入口 | 11 个 .bat/.sh | `python build/build.py` |
| 配置 | 一次构建 Debug + Release | 一次一个 `--config`，调用两次 |
| 构建树 | `build/build_temp` | `out/<arch>/<Config>` |
| 依赖 | all-in-one 脚本内部处理 | `deps` 子命令，按版本清单准备 |
| SDK | 手工整理 | `build.py pack`（含 `LICENSES/`、`THIRD-PARTY-NOTICES.md`） |

VS 解决方案文件（`build/duilib.sln`、`examples.sln` 等）仍保留，使用前先运行
`python build/build.py deps` 准备依赖。
