# 编译参考手册

本文是 nim_duilib 的唯一编译参考手册，适用于命令行构建、依赖准备、出包与环境排查。
旧的分平台构建脚本（`build_duilib_all_in_one.*`、`msvc_build.*`、`linux_build.sh` 等）
已于 2026-10-29 删除（四个平台均验证过新入口），统一使用 `python3 build/build.py`。

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

Python 3 缺失时 `build.py` 会直接报错并给出安装提示，不提供绕过 Python 的构建
路径。Skia 的构建脚本固定调用 `python3`；若 PATH 中只有 `python` / `py -3` 而没有
`python3`，预检会给出告警，并在实际编译 Skia 前报错终止。

本文命令统一写作 `python3 build/build.py`：Windows 与 Unix 使用同一形式，参数与
子命令完全一致。macOS/Linux/FreeBSD 通常只提供 `python3`；Windows 上没有 `python3`
命令时的替代写法见根 README 的「为什么统一使用 `python3 build/build.py`」。由于
Skia 的构建脚本固定调用 `python3`，各平台都必须保证 `python3` 在 PATH 中。

## 2. 快速开始

```bash
python3 build/build.py                  # Release：自动准备依赖 + 编译库与示例
python3 build/build.py --config Debug   # Debug（独立构建目录）
python3 build/build.py doctor           # 环境自检（无副作用）
python3 build/build.py pack --with-cef  # 出 SDK 包
```

Windows 与 Unix 使用同一条命令；Windows 上的 `python3`
替代写法见根 README 的「为什么统一使用 `python3 build/build.py`」。

构建目录固定为 `out/<arch>/<Config>`（例如 `out/x64/Debug`），与 `build/` 平级，
不写入 `build/build_temp`。可执行文件与运行时资源始终输出到仓库根 `bin/`，
库文件输出到 `lib/<arch>/<Config>/`（Unix 为 `lib/<Config>/`）。

## 3. 子命令

| 命令 | 说明 |
| :--- | :--- |
| `build.py`（默认） | 等价于 `build.py build`：先确保依赖就绪，再编译一个配置 |
| `build.py deps` | 只准备当前配置所需的 Skia / SDL3 / CEF |
| `build.py build` | 编译一个配置；默认先准备依赖，`--no-deps` 跳过 |
| `build.py test` | 运行 `build/` 下的标准库 unittest（含 CMake 规则用例与一次真实 configure 冒烟）；PATH 上没有 cmake 时自动使用 `DUILIB_CMAKE_EXECUTABLE` 或 VS 自带 CMake |
| `build.py pack` | 调用 `pack.py` 出 SDK 包 |
| `build.py doctor` | 环境自检：工具链、版本清单、依赖目录、根工程预检（含编译器、`gn`） |
| `build.py clean` | 清理构建树；`--deps` 同时清理依赖产物 |

一次调用只构建一个架构 + 一个配置。需要 Debug 与 Release 时调用两次：

```bash
python3 build/build.py --config Debug
python3 build/build.py --config Release
```

Debug 与 Release 的示例输出同名，都写在共享的 `bin/`，而目标文件按配置分开存放。
切换配置时 `build.py` 会先删除另一种配置的示例产物及其
`bin/.build-config/<目标>` 标记，强制当前配置重新链接；否则 Ninja 会因 `bin/`
中的文件比本配置的目标文件新而跳过链接（`ninja: no work to do`）。

## 4. 常用参数

| 参数 | 说明 |
| :--- | :--- |
| `--config Release\|Debug` | 构建配置，默认 `Release`；不提供 `--debug` 别名 |
| `--arch auto\|x86\|x64\|arm64` | 目标架构，默认主机架构；Windows + Ninja 交叉编译 arm64 时必须显式指定 |
| `--lang auto\|zh_CN\|en` | 交互语言，默认跟随系统；也读取环境变量 `DUILIB_LANG` |
| `--build-dir <dir>` | 覆盖构建目录，允许放到仓库外 |
| `--deps-root <dir>` | 依赖源码/安装根目录，默认仓库同级目录（与 `nim_duilib` 平级）；Skia、SDL3、CEF 都安装到这里 |
| `--jobs N` | 并行编译任务数，默认 CPU 逻辑核心数 |
| `--offline` | 只使用已缓存的依赖源码/归档，不联网 |
| `--dry-run` | 只打印将执行的命令 |
| `--cmake-arg <arg>` / `--build-arg <arg>` | 透传给 configure / build（可重复）；值以 `-` 开头时写成 `--cmake-arg=-DFOO=1` |
| `--vs-version N` | 显式指定 Visual Studio 主版本（如 `17`）；仅 Windows 有效 |
| `--cef-version 109\|latest` | CEF 版本：`109` 为旧版，`latest` 取版本清单默认值（当前为 156，清单升级后自动跟随） |
| `--skia-ref` / `--sdl-ref` | 选择版本清单中的 ref，如 `chrome/m142`、`main`、`release-3.4.18` |
| `--sdl` / `--no-sdl` | 仅 Windows 有效；Windows 默认关闭，其他平台强制开启 |
| `--md` | 使用 MSVC 动态运行库（/MD、/MDd）；仅 Windows 有效 |
| `--dll` | Windows MSVC DLL 模式：隐式 `--md`，自动准备 Skia `.md/.mdd`，输出 `bin/duilib*.dll`；同步 `msvc/PropertySheets/duilib_dll.flag`。SDL 不会自动开启（见 `--sdl`） |
| `--no-absolute-rpath` | 仅 Unix 有效；产物只写相对 rpath，不附加 SDL3 等绝对兜底路径 |
| `--cef-archive <file>` | 使用本地 CEF 归档，跳过下载（仍按官方 `.sha1` 校验） |
| `--no-deps`（build） | 跳过依赖准备，复用已就绪的 Skia/SDL3/CEF |
| `--target <name>`（build） | 只构建指定 CMake 目标，如 `duilib` |
| `--no-examples`（build） | 不编译示例程序（仍配置工程） |
| `--force`（deps） | 忽略完成标记，强制重建依赖 |
| `--with-cef` / `--with-sdl` / `--with-examples`（pack） | 包内包含对应内容（Unix 始终打包 SDL3，无需 `--with-sdl`） |
| `--out-dir`（pack） | 指定包输出目录（默认 `dist/`） |

`--sdl` 只是编译开关；`pack` 要在包内包含 SDL3 时必须另加 `--with-sdl`
（只写 `--sdl` 时 `build.py` 会打印提示，打包内容保持不变）。

`out/<arch>/<Config>` 会复用 CMake 缓存，而 `DUILIB_MD`、`DUILIB_BUILD_EXAMPLES`、
`DUILIB_ENABLE_SDL`、`DUILIB_ENABLE_CEF`、`DUILIB_CEF_VERSION`、`DUILIB_SKIA_REF`
等都是有状态选项。`build.py` 每次都会显式传入解析后的取值，所以先跑
`--no-examples --md`、再跑默认命令不会沿用旧选项；`DUILIB_ENABLE_CEF` 也会按平台
重新传入（Windows MSVC、Linux、macOS 为 `ON`，MinGW/MSYS2、FreeBSD 为 `OFF`）。
只有绕过 `build.py` 直接用 `cmake` 配置同一目录时才需要自己清理缓存（删除构建
目录后重新配置）。用 `--cmake-arg` 临时关掉 CEF 时，`DUILIB_EXAMPLES` 中残留的
`cef`/`CefBrowser` 会被自动剔除或跳过，不会再去链接不存在的 CEF 符号。
`DUILIB_MACOS_BUNDLE_EXAMPLES`（macOS App Bundle 目标列表）只通过
`--cmake-arg` 设置，会保留在缓存中；关闭时传空值，见
[macos-app-bundle.md](macos-app-bundle.md)。

Unix 上示例程序默认写入相对 rpath（`$ORIGIN/../lib`、`@loader_path/../lib` 等），
并追加 SDL3 依赖前缀作为构建树兜底。要生成完全不含构建机绝对路径的产物，可加
`--no-absolute-rpath`（等价于 `--cmake-arg=-DDUILIB_RPATH_ABSOLUTE=OFF`）；此时
构建树中的示例需自行设置 `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`，打包后的程序仍
通过相对 rpath 找到包内 SDL3。

## 5. 依赖准备

依赖目录默认位于仓库同级（可用 `--deps-root` 覆盖）：

```
<deps-root>/
  skia/            # Skia 源码（解压补丁后编译）
  SDL/             # SDL3 源码
  SDL3/            # SDL3 安装前缀
  SDL3_build/      # SDL3 构建目录
  cef_binary/      # CEF 下载/解压缓存（macOS 为完整分发目录）
```

Skia 的补丁归档随仓库提交，位于 `build/skia_patch/`（由清单 `skia.patchDir` 指定），
归档内的 `SHA1.txt` 记录它对应的 Skia 提交。`build.py deps`（以及默认的 `build`）
与 CMake 依赖脚本在使用前都会校验 `SHA1.txt` 与清单 `skia.default` 是否一致，
不一致时直接报错，避免用错版本的补丁编译。补丁内容变化会让对应的 Skia 产物
重新编译。切换到清单指定的提交、或重新解压补丁之前，依赖脚本会先用 `git stash`
保存 `skia/` 工作区里已跟踪文件的修改（旧版本补丁的改动不会被删除），需要时可用
`git stash list` / `git stash pop` 找回。这一步不需要联网，也不再克隆外部补丁
仓库；归档缺失时用
`git checkout -- build/skia_patch` 恢复（仅有 Skia 产物、无需重建时只告警，不中断）。
`doctor` 会显示归档是否存在、SHA1 是否与清单一致。

归档同时提供 Windows 版 `bin/gn.exe` 与 `bin/ninja.exe`：全新环境里归档尚未解压时，
根工程预检只提示由 `deps` 解压这两个工具，不会中断配置；解压后仍是硬性检查。
重新解压补丁时，源码树里已存在且与归档内容一致的 `bin/gn.exe`、`bin/ninja.exe`
会原样保留（正在运行的 `ninja.exe` 无法被覆盖）；内容不同时先解到临时目录再替换，
仍被占用则只告警并继续使用现有文件，源码文件照旧覆盖。

某些 Visual Studio 安装不含 ninja（“适用于 Windows 的 C++ CMake 工具”是可选组件）。
`build.py` 依次在 PATH、VS 安装目录、已解压的 `skia/bin/ninja.exe`、补丁归档里查找
ninja，仍找不到时从归档解压到 `out/.duilib-tools/bin/ninja.exe` 并继续使用 Ninja
生成器；根工程在 `project()` 之前执行同样的自举（解压到
`<构建树>/.duilib-tools/bin/ninja.exe` 并写入 `CMAKE_MAKE_PROGRAM`），因此直接运行
`cmake --preset x64-release` 也不要求系统装有 ninja。两者都不会写入 `skia/` 源码树；
需要指定其它 ninja 时用环境变量或缓存项 `DUILIB_NINJA_EXECUTABLE`。

所有网络操作（git clone/fetch、CEF 下载）共用一套重试参数，维护在
[third_party_versions.json](../cmake/third_party_versions.json) 的 `retry` 段：

```json
"retry": { "maxAttempts": 0, "delaySeconds": 20 }
```

`maxAttempts=0` 表示无限重试（默认），`delaySeconds` 为两次尝试之间的固定间隔。
`git pull` 例外：只尝试一次，失败仅告警并继续使用本地内容（与旧脚本一致）。
把依赖源码切到清单 ref 时，只有本地缺少目标提交才会 fetch；本地已有该提交时直接
`git checkout`，离线构建同样可用。

版本由 [third_party_versions.json](../cmake/third_party_versions.json) 统一锁定：

- **Skia**：固定 commit（`skia.default`）+ 对应补丁 zip；Windows 使用 VS 自带
  clang/ninja。非 Windows 平台优先用 clang/clang++ 编译（GCC 版 Skia 运行性能明显
  较差），只有 clang 不可用时才回退 gcc/g++；产物目录相应为
  `out/llvm.<arch>.release` 或 `out/gcc.<arch>.release`。Debug 复用 Release 编译
  模式，仅切换运行库（`/MTd` 或 `/MDd`），因此每种配置需要各自的 Skia 变体目录。
  本地 `skia/` 源码不在清单 commit（或 `--skia-ref` 指定的 tag/branch）上时，
  `deps` 会把工作区切到清单指定的提交；branch ref 只要求当前就在该分支上，
  不会每次都去追远端新提交。
- **SDL3**：默认 ref 为 `main` 分支（`sdl.default`；`refs` 中也保留
  `release-3.4.18` tag，源码编译报错时不要选用，需要时可用 `--sdl-ref` 切换）。
  Windows 默认不启用；显式启用时由 `deps` 自动构建 Debug + Release 静态库。
  Unix 构建动态库，并写入 rpath（相对条目在前，绝对兜底由
  `DUILIB_RPATH_ABSOLUTE` 控制）。本地 `SDL/` 源码停在别的分支/标签时，`deps`
  会先切到清单指定的 ref 再编译（branch ref 只要求当前就在该分支上，不追远端）；
  SDL3 的安装 stamp 记录源码提交，切到清单版本后旧产物会自动重建（升级到本版本
  时会重建一次）。
- **CEF**：固定版本分支（`cef.default`，可选 `109` 兼容 Windows 7）。统一使用
  `_minimal` 发行包（体积约为标准包的一半，含 `include/`、`Release/`、
  `Resources/`），只安装 `Release/*` 与 `Resources/*` 运行时，放到：
  - Windows：`bin/libcef_win/<arch>`（156）或 `bin/libcef_win_109/<arch>`（109）
  - Linux：`bin/libcef_linux`
  - macOS：`<deps-root>/cef_binary`
  编译使用仓库中已跟踪的头文件与导入库；下载包内的 `CEF_VERSION` 必须与跟踪
  头文件一致，否则报错。Debug 使用 Release 运行时与框架（CEF 分发包通常只在
  `Release/` 下提供 `Chromium Embedded Framework.framework`，CMake 会自动回退）。
  MinGW 不支持 CEF。

下载的 CEF 归档会按官方发布的 `<下载地址>.sha1` 校验完整性。哈希文件与归档一起
缓存，离线时可复用；拿不到时 `deps` 回退版本清单中的 `sha256`，两者都没有时
只告警并跳过校验。

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

### macOS App Bundle

macOS 下 `basic` 默认编译为 `bin/basic.app`。其他示例可以在自己的
`CMakeLists.txt` 中加一行 `set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)`，或用
`-DDUILIB_MACOS_BUNDLE_EXAMPLES=<目标名列表>` 指定；示例 C++ 代码无需修改，
资源按 macOS 标准布局复制到 `Contents/Resources/duilib/resources`（与普通程序的
`<exe 目录>/resources/` 同构），SDL3 动态库复制到 `Contents/Frameworks` 并设置相对
rpath。`GlobalManager::GetResourceRootPath(true)`（默认参数）在 macOS 下返回该目录，
传 `false` 时回退到可执行文件同目录的 `resources/`。

完整说明（开关、目录结构、图标、验证、签名、打包）见
[macos-app-bundle.md](macos-app-bundle.md)。

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
`LICENSES/`、`THIRD-PARTY-NOTICES.md` 与 `THIRD-PARTY-NOTICES.en.md`。合规要求：
分发 SDK 时必须保留 `LICENSES/` 与中英文两份第三方声明。

只给 `--with-examples` 而不给 `--with-cef` 时，会自动跳过 `cef` 与 `CefBrowser`
（Windows 为 `cef.exe`/`CefBrowser.exe`），避免包内留下缺少 libcef 运行时、
必然启动失败的程序；需要可运行的 CEF 示例请同时加 `--with-cef`。

macOS 例外：CEF 示例是 App Bundle（`bin/cef.app`、`bin/CefBrowser.app` 目录），
普通示例也可以按 [macos-app-bundle.md](macos-app-bundle.md) 编译为
`bin/<示例名>.app`。`.app` 目录会整体收集（含 `Contents/` 下的可执行文件、
资源与 Frameworks）。
`--with-examples` 时同样以 `--with-cef` 决定 CEF 示例是否随包：未指定则跳过并告警，
指定后整个目录（含 `Contents/Frameworks` 下的 CEF Framework 与 Helper）按原结构打包。
Bundle 自带运行时，因此 macOS 的 `--with-cef` 只控制示例，不收集 CEF 开发文件与
运行时目录（SDK 不含 macOS CEF 开发文件，需按 [Release.md](../docs/Release.md)
的 macOS 章节从源码树构建）。

## 8. 平台 × 编译器 × 依赖 × 可选模块

| 平台 | 编译器 | Skia | SDL3 | CEF | WebView2 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Windows | MSVC 2022+ | 必须 | 默认关闭，可 `--sdl` 启用 | 默认启用（109/156） | 支持 |
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

各发行版的系统包命令见上表；Skia 由 `build.py deps` 自动编译，无需手工准备。
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

| 环境 | `MSYSTEM` | 工具链目录 | 建议的 `CC`/`CXX` | Skia 目录候选（`skia/out/`，按顺序取第一个） | 包名后缀 |
| --- | --- | --- | --- | --- | --- |
| MINGW64（msvcrt） | `MINGW64` | `E:\msys64\mingw64\bin` | `gcc` / `g++` | `gcc.msvcrt.<arch>.release` → `gcc.<arch>.release` → `mingw64-gcc.<arch>.release` | `-mingw-gcc` |
| UCRT64 | `UCRT64` | `E:\msys64\ucrt64\bin` | `gcc` / `g++` | `gcc.<arch>.release` → `mingw64-gcc.<arch>.release` | `-mingw-gcc` |
| CLANG64 | `CLANG64` | `E:\msys64\clang64\bin` | `clang` / `clang++` | `llvm.<arch>.release` → `mingw64-llvm.<arch>.release` | `-mingw-llvm` |

MINGW64（msvcrt）与 UCRT64（ucrt）的 gcc 静态库 C 运行库不同，Skia 也必须用同源目录；
MINGW64 会优先选带 `.msvcrt` 的目录（只在该环境提供，其他环境不会误选）。
两者的包名都是 `-mingw-gcc`，**不能归档到同一目录**。
独立版 llvm-mingw 使用 `mingw64-llvm.<arch>.release`（libc++），与 MSYS2 CLANG64 的
`llvm.<arch>.release` 也不是同一份产物，`pack.py` 会按库内符号（`libstdc++` / `libc++`）区分。
上表规则由 `cmake/duilib_skia_variants.cmake` 统一实现（配置期解析、off 模式检查、
依赖准备共用），`build/pack.py` 的打包侧候选与之保持一致，并由
`build/test_cmake_scripts.py` 逐项比对。
需要强制指定某个 Skia 目录时用 `--skia-lib-dir <目录>`（编译侧对应
`-DDUILIB_SKIA_LIB_SUBPATH=<目录>`）。

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
- `--dll` = `--md` + `DUILIB_DLL=ON`：duilib 目标改为 `SHARED`，输出
  `bin/duilib*.dll`（x86 为 `duilib32*.dll`，Debug 带 `_d`）与
  `lib/<arch>/<Config>/duilib.lib` 导入库；示例只链接导入库。
- `--dll` 不会开启 SDL：Windows 上 SDL 默认关闭，DLL 模式也不隐式启用；
  关闭时 `deps` 不 clone SDL 源码、也不构建 SDL3，需要时显式 `--sdl`。
- **模式切换自动清理**：`build.py` 在 `out/<arch>/<Config>/.duilib-build-mode`
  记录上次模式（`mt`/`md`/`dll`）。在 `mt`/`md`/`dll` 之间切换时自动删除整个
  构建目录，CEF wrapper 与所有目标按新 CRT 重新编译；从 DLL 切回静态时还会删除
  `bin/duilib*.dll`，避免 `pack.py` 误判。MSBuild 路径由
  `CommonSettings.props` 的 `IntDir` 标记完成同样的自动重编。
- libjpeg-turbo：DLL 模式使用 `turbojpeg.lib` 导入库并复制
  `bin/turbojpeg.dll`；静态模式仍为 `turbojpeg-static.lib`（/MT）。

## 10. 排障

| 现象 | 处理 |
| :--- | :--- |
| `Visual Studio not found` | 安装 VS 2022+ 与 C++ 工作负载；或用 `--vs-version` 指定版本 |
| `Skia not found` | 首次执行 `python3 build/build.py deps`，或检查 `--deps-root` |
| `gn/ninja not found` | Windows 使用 VS 自带目录，或执行 `python3 build/build.py deps` 从 Skia 补丁归档解压自带的 `bin/gn.exe`；VS 未装 ninja（可选组件）时 `build.py` 与根工程都会自动从归档解压自带的 `bin/ninja.exe`（`out/.duilib-tools` 或构建树 `.duilib-tools`），无需单独安装。Unix 安装 `ninja` 与 `gn`。Unix 不会使用 Skia 目录里的 `gn.exe`（Windows 版无法运行），只有 `gn.exe` 时必须另装原生 `gn` |
| `python: command not found`（macOS/Linux） | 这些系统通常只有 `python3`：改用 `python3 build/build.py ...`，参数不变 |
| `python3 not found`（编译 Skia 时报错） | Skia 构建脚本固定调用 `python3`，不认 `python` / `py -3`；安装 Python 3 并确认 `python3 --version` 可用（Windows 可把 Python 安装目录加入 PATH） |
| `CMake version is too old` | 升级到 CMake 3.24+ |
| CEF 示例无法运行 | 确认 `bin/libcef_win/<arch>`（或 `bin/libcef_linux`）包含运行时；`pack --with-cef` 会带上 |
| macOS Debug 构建复制 `Chromium Embedded Framework.framework` 失败 | CEF 分发包通常只带 Release 框架；现已在配置期检测并自动回退到 `Release/`，日志会打印 `CEF Debug framework not found; using Release framework`。若 Debug/Release 都没有框架，配置期会直接报错并提示检查 `CEF_ROOT` |
| 打包的示例程序加载了构建机的 `.so` | 示例程序已写入相对 rpath（`$ORIGIN/../lib`、`$ORIGIN/libcef_linux`），优先使用包内库；Linux 示例依赖 libcef 时请用 `pack --with-cef`，否则包内缺少 CEF 运行时 |
| 产物中仍有构建机绝对路径（`otool`/`readelf` 可见） | 这些路径来自 `DUILIB_RPATH_ABSOLUTE=ON` 的 SDL3 兜底；发布构建可加 `--cmake-arg=-DDUILIB_RPATH_ABSOLUTE=OFF`，或改用 `DUILIB_SDL_RPATH=OFF` 完全关闭 rpath |
| CEF 下载失败 | 用 `--cef-archive <file>` 指定本地归档，或 `--offline` 使用缓存 |
| Linux 文本不显示 | 安装 fontconfig 与中文字体，确认 `bin/resources/fonts` 存在 |
| 构建目录损坏 | `python3 build/build.py clean` 后重试 |
| `clean` 删除失败（权限/占用） | 关闭占用构建树的程序（编辑器、运行中的示例、杀毒软件）后重试；错误信息会给出无法删除的具体路径 |
| 切换编译器后配置失败（如 cl.exe 找不到 `kernel32.lib`） | 无需手动清理：`build.py` 检测到 `out/<arch>/<Config>` 缓存中的编译器变化会自动清空该目录 |
| 先编译 Debug 再编译 Release 时直接结束（`ninja: no work to do`），`bin/` 仍是 Debug 示例 | 两个配置的示例都写到共享的 `bin/`，切换配置时 `build.py` 会删除另一种配置的示例产物与 `bin/.build-config` 标记后重新链接；macOS 的 `.app` 只删 `Contents/MacOS` 下的可执行文件，保留 configure 阶段生成的 `Contents/Info.plist`；手工调用 CMake 时需自行清理 `bin/` 中的同名示例 |
| macOS 切换 Debug/Release 后第一次启动即崩溃，栈顶为 `+[UNUserNotificationCenter currentNotificationCenter]` 断言 | 旧版 `build.py` 在 configure 之后整个删除 `bin/<目标>.app`，重新链接只重建可执行文件，`Contents/Info.plist` 缺失导致 LaunchServices 无法解析进程的 bundle | 已修复：切换配置时只删 `.app` 内的可执行文件。已有产物可 `rm -rf bin/<目标>.app` 后重新构建（configure 会重新生成 `Info.plist`），用 `plutil -p bin/<目标>.app/Contents/Info.plist` 确认 |
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
| SDK | 手工整理 | `build.py pack`（含 `LICENSES/`、`THIRD-PARTY-NOTICES.md` / `.en.md`） |

VS 解决方案文件（`build/duilib.sln`、`examples.sln` 等）仍保留，使用前先运行
`python3 build/build.py deps` 准备依赖。

回归测试的全部构建/打包组合、各平台状态与执行顺序见 [test-matrix.md](test-matrix.md)。
