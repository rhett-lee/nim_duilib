# Linux 发行版系统依赖

在 Linux 上从源码构建 nim_duilib 之前，需要先安装编译器、构建工具以及窗口/字体/图形相关的开发包。构建脚本会自动准备 Skia、SDL3、CEF 等第三方依赖，但这些系统包必须由使用者提前安装。

本文列出的发行版命令来自仓库中已验证的构建文档（[build-reference.md](build-reference.md) 的“平台 × 编译器 × 依赖 × 可选模块”一节）；仓库中没有对应命令的发行版会明确标注“待确认”，请不要按其他发行版的经验直接套用包名。

## 1. 需要安装什么

| 类别 | 内容 | 用途 |
| :--- | :--- | :--- |
| 编译器 | gcc/g++ 10+ 或 clang/clang++ 12+ | 编译界面库、示例与 Skia |
| 构建工具 | CMake 3.24+、Ninja、gn（Ubuntu 系包名为 `generate-ninja`）、make、Git、Python 3、unzip（部分发行版还需 bzip2、wget） | 生成并执行构建；Skia 依赖 gn/ninja |
| 窗口与字体 | X11、Freetype、Fontconfig 的开发包 | CMake 配置阶段通过 `find_package(X11 REQUIRED)`、`find_package(Freetype REQUIRED)`、`find_package(Fontconfig REQUIRED)` 查找，见 [cmake/duilib_bin_linux.cmake](../cmake/duilib_bin_linux.cmake) 与 [cmake/duilib_unix_font_libs.cmake](../cmake/duilib_unix_font_libs.cmake) |
| 图形与输入 | OpenGL/GLES/EGL、Vulkan、DBus、ibus 开发包 | 渲染后端与输入法支持 |
| Wayland（可选） | wayland、libxkbcommon 开发包 | Wayland 桌面环境下构建与运行 |

## 2. 各发行版安装命令

下表是仓库构建文档中列出的“必选”安装命令，“桌面”列注明了该命令对应的桌面环境（X11 与 Wayland 的差异体现在是否包含 wayland/xkbcommon 包）。

| 发行版 | 桌面 | 安装命令 |
| :--- | :--- | :--- |
| Ubuntu | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip bzip2 libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| Debian | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| UbuntuKylin（优麒麟） | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| 中科方德 | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| 统信 UOS | X11 | `sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| OpenKylin（开放麒麟） | Wayland | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| OpenEuler | UKUI/DDE (X11) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel` |
| Fedora | GNOME (Wayland) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel` |
| openSUSE | KDE (X11) | `sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel` |

## 3. 安装后如何验证

1. 运行环境自检（只读，不修改文件）：

   ```bash
   python3 build/build.py doctor
   ```

2. 开始构建：

   ```bash
   python3 build/build.py
   ```

如果缺少系统包，问题通常会在 CMake 配置阶段暴露，典型报错如下：

- `Could not find X11`、`Could not find Freetype`、`Could not find Fontconfig`：补装对应的 `-dev` / `-devel` 开发包。
- Skia 配置或编译阶段找不到 `gn` / `ninja` / `clang`：补装构建工具；Ubuntu 系包名为 `generate-ninja`，Fedora/openSUSE 为 `gn`。
- 构建成功但界面文字不显示：这不是构建依赖问题，通常需要安装 fontconfig 与中文字体，见根目录 README 的“常见问题”。

## 4. 待确认事项

- **Ubuntu 之外的发行版实测**：测试矩阵（[test-matrix.md](test-matrix.md)）记录的是 Linux x64（VMware 虚拟机）的构建与打包结果，未按发行版逐一记录；Ubuntu 之外的发行版命令来自构建文档，未逐一实测。

## 5. 相关文档

- 完整构建参数与排障：[build-reference.md](build-reference.md)
- macOS / FreeBSD 依赖与构建说明：见根目录 README 的“环境与依赖要求”
- 构建结果与运行方式：见根目录 README 的“快速开始”

---

[English](linux-dependencies.en.md)
