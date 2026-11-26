# Linux Distribution System Dependencies

Before building nim_duilib from source on Linux, you need to install a compiler,
the build tools, and the development packages for windowing, fonts and graphics.
The build script prepares Skia, SDL3, CEF and the other bundled third-party
libraries automatically, but these system packages must be installed by hand.

The distribution commands in this document are taken from the build reference
that has been verified in this repository (the
"platform × compiler × dependencies × optional modules" section of
[build-reference.en.md](build-reference.en.md)). Distributions that have no command in
the repository are marked "to be confirmed" — do not copy package names from a
different distribution.

## 1. What to install

| Category | Content | Purpose |
| :--- | :--- | :--- |
| Compiler | gcc/g++ 10+ or clang/clang++ 12+ | Compile the UI library, examples and Skia |
| Build tools | CMake 3.24+, Ninja, gn (package name `generate-ninja` on Ubuntu-based distributions), make, Git, Python 3.8+, unzip (some distributions also need bzip2 and wget) | Generate and run the build; Skia requires gn/ninja |
| Windowing and fonts | Development packages for X11, Freetype and Fontconfig | Looked up during the CMake configure step through `find_package(X11 REQUIRED)`, `find_package(Freetype REQUIRED)` and `find_package(Fontconfig REQUIRED)`, see [cmake/duilib_bin_linux.cmake](../cmake/duilib_bin_linux.cmake) and [cmake/duilib_unix_font_libs.cmake](../cmake/duilib_unix_font_libs.cmake) |
| Graphics and input | Development packages for OpenGL/GLES/EGL, Vulkan, DBus and ibus | Rendering backend and input method support |
| Wayland (optional) | Development packages for wayland and libxkbcommon | Building and running on Wayland desktops |

### Compiler and CMake version requirements

| Purpose | Minimum | Notes |
| :--- | :--- | :--- |
| Main project (Linux/macOS/FreeBSD) | gcc 10+ or clang 12+ | Checked by the preflight at configure time |
| Building Skia (Linux/macOS/FreeBSD) | clang 12+ or gcc 11+ | Skia's `ml3`/`ml4` targets always use `-march=x86-64-v3` / `-march=x86-64-v4`, which only clang 12+ / gcc 11+ understand; clang is preferred and falls back to gcc when older than 12, and configuration fails early when both are too old |
| CMake | 3.24+ (CMakePresets is version 6, so 3.25+ in practice) | The root project is configured via `cmake --preset` from `build.py` |

Domestic distributions (NFSChina/中科方德 and UnionTech UOS): **the compilers shipped with both systems cannot build Skia** (NFSChina ships clang 11.0.1 and gcc 10.2.1; UOS ships gcc older than 11), so users must install **clang 12+ (recommended) or gcc 11+** themselves. In addition, **the CMake 3.18.4 bundled with NFSChina does not meet the 3.24+ requirement** — upgrade CMake manually (3.25 or newer is recommended). Put the new compiler's `bin` directory first in `PATH`; `build.py doctor` prints the CMake version and the Skia toolchain actually in use.

## 2. Installation commands per distribution

The table lists the "required" commands from the repository build reference. The
desktop column records the desktop environment the command was written for
(X11 and Wayland differ only in whether the wayland/xkbcommon packages are
included).

| Distribution | Desktop | Installation command |
| :--- | :--- | :--- |
| Ubuntu | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip bzip2 libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| Debian | GNOME (Wayland) | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| UbuntuKylin | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| Zhongke Fangde | X11 | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| UnionTech UOS | X11 | `sudo apt install -y gcc g++ gdb make git cmake python3 ninja-build wget unzip libfontconfig1-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev` |
| OpenKylin | Wayland | `sudo apt install -y gcc g++ gdb make git ninja-build generate-ninja python3 cmake llvm clang unzip libfontconfig-dev libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev libvulkan-dev libxext-dev libxcursor-dev libxi-dev libxrandr-dev libdbus-1-dev libibus-1.0-dev libwayland-dev libxkbcommon-dev` |
| OpenEuler | UKUI/DDE (X11) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel` |
| Fedora | GNOME (Wayland) | `sudo dnf install -y gcc g++ gdb make git ninja-build gn python cmake llvm clang unzip fontconfig-devel mesa-libGL-devel mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-devel ibus-devel wayland-devel libxkbcommon-devel` |
| openSUSE | KDE (X11) | `sudo zypper install -y gcc gcc-c++ gdb make git ninja gn python cmake llvm clang unzip fontconfig-devel Mesa-libGL-devel Mesa-libEGL-devel Mesa-libGLESv3-devel glu-devel vulkan-devel libXext-devel libXcursor-devel libXi-devel libXrandr-devel dbus-1-devel ibus-devel` |

## 3. Verifying the installation

1. Run the environment self-check (read-only, it does not modify the repository):

   ```bash
   python3 build/build.py doctor
   ```

2. Start the build:

   ```bash
   python3 build/build.py
   ```

If a system package is missing, the problem usually shows up during the CMake
configure step. Typical messages:

- `Could not find X11`, `Could not find Freetype` or `Could not find Fontconfig`:
  install the matching `-dev` / `-devel` development package.
- `[ERROR] required tool not found: fontconfig freetype2 x11 ...`: the root
  project preflight (`cmake/duilib_preflight_tools.cmake`) lists every missing
  system library found through `pkg-config`; install them and configure again.
  Without `pkg-config` the check only prints `[WARN]` and is skipped.
- `gn`, `ninja` or `clang` not found during the Skia configure or compile step:
  install the missing build tool; the package is named `generate-ninja` on
  Ubuntu-based distributions and `gn` on Fedora/openSUSE.
- The build succeeds but window text does not show up: this is not a build
  dependency problem. It normally means fontconfig and a CJK font are missing;
  see the "Troubleshooting" section in the root README.

## 4. To be confirmed

- **Distributions other than Ubuntu**: the test matrix ([test-matrix.md](test-matrix.md), Chinese)
  records build and packaging results for Linux x64 (a VMware virtual machine)
  without recording the distribution per case. The commands for distributions
  other than Ubuntu come from the build reference and have not been tested one by
  one.

## 5. Related documents

- Full build parameters and troubleshooting: [build-reference.en.md](build-reference.en.md)
- macOS / FreeBSD dependencies and build instructions: see "Environment and
  dependencies" in the root README
- Build outputs and how to run them: see "Quick start" in the root README

---

[简体中文](linux-dependencies.md)
