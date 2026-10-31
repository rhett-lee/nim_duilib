简体中文 | [English](README.en.md)

> 英文版 `README.en.md` 最近同步于 2026-10-29。

# nim_duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) 是一款基于 **C++20** 的跨平台桌面 UI 库，源于经典 duilib（经 NIM_Duilib_Framework 演进）：用 XML 描述界面结构与样式，用 C++ 编写业务逻辑，渲染由 Skia 完成。同一套界面代码可以构建到 Windows、Linux、macOS 与 FreeBSD，适合既需要原生窗口能力（托盘、网页控件、系统通知、DPI 感知）、又不想为每个平台重写一套界面的桌面客户端。

- 语言标准：`main`、`develop` 为 C++20；`develop-cpp17` 分支为 C++17（仅用于 Windows VS2017/VS2019）
- 支持平台：Windows 7/10/11（x86/x64/arm64；Windows 7 使用 CEF 109）、Linux x64、macOS x64/arm64、FreeBSD x64
- 版本信息：版本号见 [duilib/duilib_version.h](duilib/duilib_version.h)，发布包见 GitHub Releases，变更记录见 [CHANGELOG.md](CHANGELOG.md)
- 许可证：[MIT](LICENSE)

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## 项目定位与能力边界

动手之前先确认这个库能覆盖哪些需求、哪些还需要额外工作，可以避免选型后返工。

**定位**：面向桌面客户端的 C++ 界面库。适合需要深度定制界面、同时覆盖 Windows/Linux/macOS 的桌面应用，也适合从 duilib / NIM_Duilib_Framework 迁移的项目。它提供的是"界面层"：窗口、布局、控件、渲染、资源与事件；网络、存储、业务逻辑仍由你自己的代码或第三方库负责。

**能力边界**（与上手直接相关的几点）：

- **渲染**：Skia 是必需的后端，负责矢量绘制、文字与动画。构建脚本会按 [版本清单](cmake/third_party_versions.json) 自动准备并编译它，不需要手工编译；如果这一步失败，通常是网络或工具链问题，`doctor` 会指出缺少哪一项。
- **开发方式**：以 XML + C++ 为主，仓库不包含可视化设计器；另有 `.claude` AI 技能辅助生成界面代码。
- **平台专属**：WebView2 仅 Windows；"DPI 独立设置"仅 Windows；系统窗口阴影仅 Windows/macOS。
- **CEF**：Windows/Linux/macOS 默认启用，FreeBSD 与 MinGW 不支持。默认使用 156（要求 Windows 10+）；要在 Windows 7 上运行网页控件，构建时加 `--cef-version 109`。
- **动画**：GIF/APNG/WebP/Lottie 默认可用；PAG 默认关闭，仅在 Windows + MSVC 解决方案下可启用（需自行编译 libpag）。
- **SDL3**：Windows 默认关闭（加 `--sdl` 开启），Linux/macOS/FreeBSD 强制开启，用于窗口管理与输入。
- **运行库**：Windows 默认静态运行库（/MT、/MTd）；需要动态运行库时加 `--md`（/MD、/MDd）。
- **DLL 模式**：Windows MSVC 下加 `--dll` 可把库编译为动态库，产物为 `bin/duilib.dll`（Debug 为 `bin/duilib_d.dll`），导入库在 `lib/<arch>/<Config>/duilib.lib`。适合需要动态加载、缩小可执行体量，或让多个程序共享同一份库的场景；同一程序只能选用静态库或 DLL 中的一种，发布时必须随包分发 DLL。

## 主要特性

以下能力覆盖桌面客户端最常见的需求，多数可以直接在示例程序中运行验证。

- **XML 布局驱动**：界面结构与业务逻辑分离，调整布局或换肤时只需改 XML，不必重新组织 C++ 代码。
- **丰富的控件与容器**：基础控件、虚表列表、树、菜单、属性表、标签页等，并支持自定义控件与自定义容器。
- **Skia 无窗口渲染**：绝大多数控件不依赖系统原生窗口句柄，后端可选择 CPU 或 GPU 绘制，复杂界面也能保持流畅。
- **事件驱动**：既可在 C++ 中绑定回调，也可在 XML 中直接配置事件响应代码，交互逻辑集中且易读。
- **图片格式**：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO。
- **动画格式**：GIF、APNG、WEBP、Lottie JSON、PAG（默认关闭）。
- **多语言、动态换肤、浅色/深色主题与强调色**：字符串与颜色集中管理，切换语言或主题不需要重启。
- **窗口阴影**：圆角/直角阴影，Windows 与 macOS 还可使用系统阴影。
- **DPI 感知**：Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2（Windows）。
- **网页控件**：CEF（Windows/Linux/macOS）与 WebView2（仅 Windows）。

## 界面预览

下图是 `examples/controls` 的运行效果，用于直观了解常用控件的默认外观。界面中的字体、配色与图片都来自 `bin/resources` 下的 XML 皮肤资源，替换资源即可得到完全不同的视觉风格，这也是本项目支持动态换肤的基础。

<img width="760" src="docs/Images/controls.png"/>

其余示例（basic、render、layout、ListCtrl、TreeView、CEF 等）的截图与说明见 [docs/Examples.md](docs/Examples.md)。

## 环境与依赖要求

构建分为两部分：编译器与基础工具由你准备，第三方源码依赖（Skia/SDL3/CEF）由 `build.py` 按 [版本清单](cmake/third_party_versions.json) 自动获取、编译并缓存到仓库同级目录。因此本机不需要预装这些依赖的库文件；但系统开发包缺失时，CMake 配置阶段会直接报错，按报错补装即可。

### 语言与编译器

| 平台 | 架构 | 编译器 | 说明 |
| :--- | :--- | :--- | :--- |
| Windows | x86 / x64 / arm64 | Visual Studio 2022（v143）、Visual Studio 2026（v145）、MSYS2 / MinGW-w64（gcc 或 clang） | 支持 Windows 7 及以上；VS2017/2019 请用 `develop-cpp17` 分支 |
| Linux | x64 | gcc 10+、clang 12+ | arm64 见下方“待确认” |
| macOS | x64 / arm64 | Xcode 16+（AppleClang，CEF wrapper 使用 C++20） | 构建机需 macOS 14.5+；产物最低支持 macOS 13.0（CEF 156 的部署目标） |
| FreeBSD | x64 | 系统自带 clang | |

Windows 7 的兼容性来自库自身配置（[duilib/duilib_config.h](duilib/duilib_config.h) 默认兼容 Win7 以上）与 CEF 109；CEF 156 要求 Windows 10 及以上。

### 构建工具

| 工具 | 要求 | 用途与说明 |
| :--- | :--- | :--- |
| CMake | 3.24+ | 生成构建文件；根工程使用 CMakePresets |
| Python | 3.8+ | 运行 `build/build.py`；Skia 的构建脚本还要求 PATH 中存在 `python3` |
| Git | 任意较新版本 | 获取 Skia/SDL3 源码与 CEF 归档 |
| Ninja | 构建后端 | Windows 使用 Visual Studio 自带的版本 |
| macOS 额外 | `brew install cmake ninja`，并从源码安装 `gn` | Skia 构建需要 `gn` |
| FreeBSD 额外 | `sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2` | 一次性装齐工具链与字体依赖 |

### Linux 系统依赖

Linux 上还需要发行版提供的 X11、字体、OpenGL/EGL、DBus/ibus 等开发包。各发行版包名差异较大，装错或漏装通常表现为 CMake 阶段报 `Could not find X11`、`Could not find Freetype` 或 `Could not find Fontconfig`。按发行版划分的安装命令、必需包清单和验证方法见单独文档：[build/linux-dependencies.md](build/linux-dependencies.md)。其中的命令取自仓库构建文档，覆盖 Ubuntu、Debian、Fedora、openSUSE、OpenEuler、OpenKylin、UbuntuKylin、中科方德、统信 UOS；除 Ubuntu 外未逐一实测，文档中已说明验证范围。

### 平台验证状态

下表只记录仓库中能找到证据的验证结果；没有验证记录的组合不会被写成“支持”。

| 平台 / 架构 | 验证环境 | 状态 |
| :--- | :--- | :--- |
| Windows 7 x64 | 真机手动测试（CEF 109） | 已验证 |
| Windows 10/11 x64 | VS 2022 17.14（MSVC 14.44）、VS 2026、CMake 4.3、Python 3.14；另测 MSYS2、llvm-mingw、x86 | 已验证 |
| Windows arm64 | VS 2022 交叉工具链；构建、DLL 模式与出包 | 构建/出包已验证；无 arm64 真机，未做启动验证 |
| macOS x64 | AppleClang 21、CMake 4.0.2、Python 3.13 | 已验证 |
| macOS arm64 | Apple Silicon 真机 | 已验证 |
| Linux x64 | VMware 虚拟机、CMake 3.31、gcc（另测 clang） | 已验证；Ubuntu 之外的发行版待确认 |
| FreeBSD x64 | FreeBSD 虚拟机、系统 clang | 已验证 |

仍标注 **待确认** 的项目：Linux arm64、Ubuntu 之外 Linux 发行版的实际验证、macOS 最低系统版本。详细用例、命令与结果见 [build/test-matrix.md](build/test-matrix.md)。

## 快速开始

以下步骤假设机器上已装好上表中的编译器与构建工具，并且能访问 GitHub。首次构建需要从源码编译 Skia 并下载 CEF 运行时，耗时从十几分钟到一小时不等；依赖会缓存在仓库同级目录（可用 `--deps-root` 修改），之后的增量构建不再重复下载和编译。

### 为什么统一使用 `python3 build/build.py`

仓库不再为各平台提供不同的构建脚本，文档里的命令也统一写成 `python3 build/build.py`。这样做的原因有三点：Skia 的构建脚本固定调用 `python3`，缺少 `python3` 时会在编译 Skia 前直接失败，`python` 不能替代；macOS/Linux 的常见环境只提供 `python3`，命令若写成 `python` 会报 command not found；统一成同一条命令后，文档只需维护一套命令，新用户不必先判断自己该用哪个脚本，参数、子命令与产物路径在各平台完全一致，学习成本最低。`build.py` 会自动识别当前平台与编译器、选择生成器、准备依赖，并附带 `doctor`（环境自检）、`deps`（只准备依赖）、`test`（单元测试）、`pack`（出包）、`clean`（清理）等子命令。

Windows 上如果没有 `python3` 命令，可以把命令开头的 `python3` 换成 `py -3`，或换成环境里指向 Python 3 的 `python`（先用 `--version` 确认版本）；也可以直接用薄封装 `build\build.bat`（它按 `py -3` → `python` → `python3` 的顺序找解释器），参数与 `build.py` 完全相同，例如 `build\build.bat deps`。前两种写法只适用于 Windows，本文其他命令示例一律使用 `python3`。需要注意，它们只改变 `build.py` 的启动方式，Skia 构建阶段仍要求 PATH 中存在 `python3`，否则会在编译 Skia 前报错终止。

### Windows（已验证：Windows 10/11 x64 + VS 2022/VS 2026）

前置：已安装 Visual Studio 2022 或 2026（含“使用 C++ 的桌面开发”工作负载）、Git、Python 3。

```bat
git clone https://github.com/rhett-lee/nim_duilib.git
cd nim_duilib
python3 build/build.py doctor
python3 build/build.py
bin\basic.exe
```

- `doctor` 只做环境自检，不修改仓库。预期看到编译器、CMake、Python、依赖目录的摘要；若某项缺失或版本过低，会给出提示，先按提示修好环境再继续。
- 首次构建会先准备依赖再编译。成功时日志以 `[OK] 编译完成` 结束；失败时以 `[ERROR] 编译失败` 结束并提示查看上方的编译输出。
- 产物：示例程序在 `bin\`（Debug/Release 同名），静态库在 `lib\x64\Release`，构建树在 `out\x64\Release`。`bin\basic.exe` 能正常打开窗口即说明工具链、依赖与运行资源都已就绪。
- 只想先确认库能编译通过：`python3 build/build.py build --no-examples --target duilib`。
- 暂时跳过 CEF 相关示例：`python3 build/build.py --cmake-arg=-DDUILIB_ENABLE_CEF=OFF`（其余示例照常构建）。
- Debug 构建：`python3 build/build.py --config Debug`。一次调用只构建一个配置，需要两种配置时执行两次。

### Linux / macOS / FreeBSD

前置：按上文安装编译器、CMake、Ninja、Git、Python 3；Linux 还需按 [build/linux-dependencies.md](build/linux-dependencies.md) 安装系统开发包。

```bash
git clone https://github.com/rhett-lee/nim_duilib.git
cd nim_duilib
python3 build/build.py doctor
python3 build/build.py
./bin/basic            # macOS 为 open bin/basic.app
```

- macOS 下 `basic` 默认编译为 `bin/basic.app`，示例 C++ 代码无需修改。
- 构建成功的表现与 Windows 相同：日志以 `[OK] 编译完成` 结束，`bin/basic`（或 `bin/basic.app`）可以启动。
- 若在 CMake 配置阶段报找不到 X11/字体相关包，按 [build/linux-dependencies.md](build/linux-dependencies.md) 补装；若 Skia 编译阶段报找不到 `gn`/`ninja`/`clang`，补装对应构建工具。

### 使用 Visual Studio 解决方案

如果团队需要在 Visual Studio IDE 中调试，解决方案仍然可用，但它不会自动准备依赖，必须先执行一次依赖准备：

```bat
python3 build/build.py deps
```

然后打开 `build/examples.sln` 构建（不需要 CEF 时用 `build/examples_no_cef.sln`）。解决方案的 Debug 产物带 `_d` 后缀（如 `basic_d.exe`），与 `build.py` 输出的 `bin\basic.exe` 不同；两者中间目录也相互独立（解决方案为 `build/build_temp`，命令行入口为 `out/<arch>/<Config>`）。

### DLL 模式（仅 Windows MSVC）

默认构建产出静态库。如果希望把界面库做成可以动态加载的组件、缩小主程序的体积，或让多个程序共享同一份库，可以在 Windows MSVC 下使用 `--dll`：

```bat
python3 build/build.py --dll --config Release
```

- `--dll` 会隐式开启 `--md`（/MD、/MDd），并把 Skia 换成对应的 `.md`/`.mdd` 变体；`deps` 会自动准备，不需要手工处理。
- 预期产物：`bin/duilib.dll`（Debug 为 `bin/duilib_d.dll`，x86 为 `duilib32*.dll`）与导入库 `lib/<arch>/<Config>/duilib.lib`；示例程序只链接导入库。构建成功的日志同样以 `[OK] 编译完成` 结束。
- 发布时 `bin/duilib*.dll` 必须与可执行文件一起分发；漏发 DLL 的典型表现是程序启动阶段报“找不到 duilib.dll”。
- 该模式只支持 Windows + MSVC；Linux/macOS/FreeBSD 仍构建静态库。`--dll` 不会顺带开启 SDL，需要时另加 `--sdl`。
- 打包时 `pack.py` 会自动收集 `bin/duilib*.dll`，包名追加 `-dll`；在 DLL 与静态模式之间切换时构建目录会自动清理，避免混用旧产物。

## 目录结构

了解目录划分有助于快速定位源码与文档；生成目录（`out/`、`lib/`、`dist/`）可以随时删除，重新构建即可恢复。

| 目录 | 说明 |
| :--- | :--- |
| `duilib/` | 库源码，按模块划分：`Core`、`Control`、`Box`、`Layout`、`Render`、`RenderSkia`、`Image`、`Animation`、`CEFControl`、`WebView2`、`Utils`、`third_party` |
| `examples/` | 仓库附带多个可直接运行的示例，覆盖基础窗口、控件演示、渲染、布局与 WebView 等场景；清单见 [docs/Examples.md](docs/Examples.md) |
| `bin/` | 示例程序输出目录，包含 `resources/` 皮肤与语言资源、CEF 运行时（构建后） |
| `docs/` | 控件、布局、事件、部署等文档与截图，索引见 [docs/Summary.md](docs/Summary.md) |
| `build/` | 统一构建入口 `build.py`、出包与发布脚本、VS 解决方案、[构建参考手册](build/build-reference.md)、[Linux 系统依赖](build/linux-dependencies.md) |
| `cmake/` | 公共 CMake 模块与第三方版本清单 |
| `msvc/` | Windows 清单文件与 VS 工程公共配置（如 libpag 开关） |
| `packaging/` | SDK 接入层（CMake 包与 MSVC props/targets） |
| `licenses/` | 第三方组件的许可证文件 |
| `out/<arch>/<Config>` | 构建树（生成） |
| `lib/<arch>/<Config>` | 库产物（生成） |
| `dist/<版本>` | 发布包（`pack` 生成） |
| `.claude/` | AI 技能定义与注册脚本 |

## 常用构建与运行命令

所有命令都在仓库根目录执行，且一次调用只构建一个架构 + 一个配置；需要 Debug 与 Release 时执行两次。Windows 与 Unix 使用同一条 `python3 build/build.py` 命令。

| 目的 | 命令 |
| :--- | :--- |
| 环境自检 | `python3 build/build.py doctor` |
| 构建 Release（库 + 示例） | `python3 build/build.py` |
| 构建 Debug | `python3 build/build.py --config Debug` |
| 只准备依赖（Skia/SDL3/CEF） | `python3 build/build.py deps` |
| 只编译库 | `python3 build/build.py build --no-examples --target duilib` |
| 运行单元测试 | `python3 build/build.py test` |
| 生成 SDK 包 | `python3 build/build.py pack --with-cef`（需要示例时再加 `--with-examples`） |
| 清理构建树 | `python3 build/build.py clean`（`clean --deps` 同时清理依赖） |
| 启用 SDL3（Windows） | `python3 build/build.py --sdl` |
| 动态运行库 | `python3 build/build.py --md` |
| DLL 模式（仅 Windows MSVC） | `python3 build/build.py --dll`（隐式 `--md`；产物 `bin/duilib*.dll` + `lib/<arch>/<Config>/duilib.lib` 导入库） |
| 指定架构 | `python3 build/build.py --arch arm64` |
| 使用 CEF 109（Windows 7） | `python3 build/build.py --cef-version 109` |
| 离线构建 | `python3 build/build.py --offline` |
| 完整参数说明 | [build/build-reference.md](build/build-reference.md) |

## 进阶用法

下面这些路径对应更深入的开发场景，按需查阅即可；新手先按“快速开始”跑通再深入会更顺畅。

- **写第一个窗口**：[examples/basic](examples/basic) 是最小工程；分步教程见 [docs/Getting-Started.md](docs/Getting-Started.md)。该教程以 VS 解决方案为例，依赖准备请以 `python3 build/build.py deps` 为准，构建入口以本文“快速开始”为准。
- **界面开发文档**：[窗口](docs/Window.md)、[容器/布局](docs/Box.md)、[控件](docs/Control.md)、[菜单](docs/Menu.md)、[全局样式](docs/Global.md)、[事件](docs/Events.md)、[XML 节点](docs/XmlNode.md)、[XML 事件](docs/XmlEvents.md)。
- **构建与发布**：完整参数见 [build/build-reference.md](build/build-reference.md)；依赖目录与发布打包说明见 [build/README.md](build/README.md)。
- **内置对话框与通知**：[MessageBoxWnd](docs/MessageBox.md)、[ToastWnd](docs/Toast.md)、[SystemNotification](docs/SystemNotification.md)。
- **网页控件**：[CEF 控件](docs/CEF.md)、[WebView2 示例](examples/WebView2)；CEF 示例运行需要 `bin/libcef_win/<arch>` 下的运行时。
- **接入 SDK**：[docs/Deployment.md](docs/Deployment.md)、[cmake/README.md](cmake/README.md)（CMake 包接入与排障，含英文版）。
- **macOS App Bundle**：[build/macos-app-bundle.md](build/macos-app-bundle.md)。
- **启用 PAG**：仅 Windows + MSVC 解决方案。将 [msvc/PropertySheets/LibPagSettings.props](msvc/PropertySheets/LibPagSettings.props) 中的 `LibPagEnabled` 改为 `1`，并按 [libpag-build.md](duilib/third_party/libpag/windows/libpag-build.md) 编译 libpag；CMake 构建不包含 PAG。
- **AI 辅助开发**：仓库同时内置 Claude、WorkBuddy 与 TRAE 三套配置，技能内容同源，只是加载方式不同：Claude 使用 `.claude/skills/` 与注册脚本，WorkBuddy 使用 `.workbuddy/skills/nim-duilib/`，TRAE 使用 `.trae/skills/nim-duilib/`；三份指南分别为 [CLAUDE.md](CLAUDE.md)、[WORKBUDDY.md](WORKBUDDY.md)、[TRAE.md](TRAE.md)。Claude 需先执行 `.claude/register.bat`（Windows）或 `bash .claude/register.sh`（Linux/macOS）注册技能，然后在任意项目中运行 `/nim-init`；WorkBuddy 与 TRAE 的项目级技能无需注册脚本，按语义触发即可。可从 `/nim-duilib-create-window`、`/nim-duilib-xml-layout`、`/nim-duilib-add-control`、`/nim-duilib-event-handler`、`/nim-duilib-theme`、`/nim-duilib-resource-pack` 开始；参考手册见 [.claude/docs/nim-duilib-llm-reference.md](.claude/docs/nim-duilib-llm-reference.md)。
- **变更记录**：[CHANGELOG.md](CHANGELOG.md)。

## 常见问题

下表中列出了最常见的现象与处理方式；如果表中没有，`doctor` 的输出与 [build/build-reference.md](build/build-reference.md) 的排障章节通常能定位到具体环节。

| 现象 | 处理 |
| :--- | :--- |
| 首次构建很慢 / 失败于下载 | 首次需要编译 Skia 并下载 CEF；已有缓存或本地归档时用 `--offline`、`--cef-archive <file>` |
| `doctor` 报找不到 Visual Studio | 安装 VS 2022 或 2026 的 C++ 桌面开发工作负载；多版本并存时用 `--vs-version 17`（或 `18`）显式指定 |
| 报 CMake 版本过低 | 升级到 3.24+ |
| 找不到 `python3` 命令 | 安装 Python 3 并确保 `python3` 在 PATH 中；Windows 的特殊处理见「为什么统一使用 `python3 build/build.py`」 |
| 切换 Debug/Release 后 `bin/` 仍是旧示例 | `build.py` 会自动清理另一种配置的示例；手工调用 CMake 时需自行清理 `bin/` |
| CEF 示例无法运行 | 确认 `bin/libcef_win/<arch>`（Linux 为 `bin/libcef_linux`）包含运行时；出包用 `pack --with-cef`，且编译与打包的 CEF 版本要一致 |
| Linux 下界面文字不显示 | 安装 fontconfig 与中文字体，见 [build/linux-dependencies.md](build/linux-dependencies.md) |
| WebView2 示例启动失败 | 确认 `bin/WebView2Loader.dll` 已生成，详见 [cmake/README.md](cmake/README.md) 的常见问题 |
| 其他构建问题 | [build/build-reference.md §10 排障](build/build-reference.md) |

## 许可证与贡献

- **许可证**：本项目使用 MIT 协议，见 [LICENSE](LICENSE)。分发衍生程序时请保留版权声明与许可文本。
- **第三方组件**：汇总见 [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)，逐项许可证在 [licenses/](licenses)；SDK 包内为 `LICENSES/`，分发 SDK 时必须一并保留。
- **反馈与贡献**：通过 GitHub Issue/PR，仓库提供 [bug 报告](.github/ISSUE_TEMPLATE/bug_report.md)、[功能建议](.github/ISSUE_TEMPLATE/feature_request.md) 与 [PR 模板](.github/pull_request_template.md)。提交前建议运行 `python3 build/build.py test`，并确保至少一个平台的 Release 构建通过。
- **当前开发计划**：丰富控件与功能；持续测试和完善基于 SDL3 的跨平台窗口引擎（X11/XWayland 较稳定，纯 Wayland 问题较多）；修复缺陷。

## 与上游 duilib / NIM_Duilib_Framework 的差异

本项目在 NIM_Duilib_Framework 基础上做了系统性改造：代码按功能模块拆分、XML 解析替换为 pugixml、移除 base 库依赖、集成 Skia 与 SDL3、重写布局体系，并新增 50+ 个控件/容器。因此它与上游在 API、XML 属性与控件行为上都可能存在差异，上游文档不能直接当作本项目文档使用；从上游迁移时建议先按清单核对。

完整的差异清单（十余项整体改进、二十余项功能完善、50+ 个新增控件/容器、性能优化、示例程序与文档）以及上游项目链接，见 [docs/Differences.md](docs/Differences.md)。控件属性与用法以 [docs/Control.md](docs/Control.md) 为准。
