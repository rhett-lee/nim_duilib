# 根 CMake 工程实现方案

本文给出在仓库根目录新增 `CMakeLists.txt`、实现「只用 CMake 完成依赖准备 + 全量构建」
的设计方案。依赖准备部分对齐 `build/build_duilib_all_in_one.sh` / `.bat` 的既有行为：
Skia 源码克隆与预编译、SDL 源码克隆与预编译、CEF 下载解压等。

## 1. 背景与约束

现状：

- 仓库没有根 `CMakeLists.txt`，`duilib/`、`duilib/third_party/*`、`examples/*`
  各自是独立 `project()`，由 `build/*_build.sh|bat` 逐个
  `cmake -S <子目录> -B <构建目录>` 构建；
- 第三方依赖的源码与预编译产物放在**仓库同级目录**：
  `../skia/`（含 `out/<变体>/`）、`../skia_compile/`（补丁 zip）、
  `../SDL3/`（安装前缀）、`../cef_binary/`（macOS）；
- 依赖准备逻辑目前只在 `build_duilib_all_in_one.sh/.bat` 中，且依赖
  `git`、`gn`、`ninja`、`unzip`、`curl`/`wget` 等外部工具；
- 子项目普遍假设产物已经存在（Skia 库目录、SDL3 安装目录、CEF 运行时），
  `cmake/duilib_common.cmake` 会在配置期对 Skia 做存在性校验并
  `FATAL_ERROR`。

设计目标：

1. `cmake -S . -B build` + `cmake --build build` 能完成从依赖准备到库、示例的全量构建；
2. 保留现有目录布局与脚本入口，不破坏 `pack.py`、`autotest.py`、`msvc/` 接入层；
3. 各平台（Windows MSVC / MinGW、Linux、macOS、FreeBSD）行为与原脚本一致；
4. 依赖准备幂等、可离线、可重复执行，不因重复解压/克隆触发全量重编；
5. 架构支持范围明确：Windows 支持 x86 / x64 / arm64，其他平台支持 x64 / arm64，超出范围在配置期报错；
6. 构建系统面向用户的交互输出支持简体中文与英文双语，按系统/区域语言自动选择，英文兜底；
7. Windows MSVC 最低版本为 Visual Studio 2022（含 MSVC v143 工具集），低版本在配置期报错。

### 1.1 平台与架构支持矩阵

| 平台 | 支持架构 | 工具链 | Skia 变体目录 |
| :--- | :--- | :--- | :--- |
| Windows | x86 / x64 / arm64 | MSVC（VS 2022+；Win32 / x64 / ARM64）、MinGW-w64（x86 / x64）、llvm-mingw（x64 / arm64）、MSYS2（mingw64 / ucrt64 / clang64 / clangarm64） | `llvm.<arch>.debug\|release`、MinGW 的 `gcc\|llvm.<arch>.*` |
| Linux | x64 / arm64 | gcc / clang | `llvm.<arch>.release` / `gcc.<arch>.release` |
| macOS | x64 / arm64 | Xcode clang（Ninja 或 Xcode 生成器） | `llvm.<arch>.release` |
| FreeBSD | x64 / arm64 | gcc / clang | `llvm.<arch>.release` / `gcc.<arch>.release` |

不支持：Windows arm32、Unix 系 x86 / arm32、未识别的处理器。命中不支持项时，
配置期以当前语言 `FATAL_ERROR` 结束，并给出“支持的架构列表 + 当前检测值”。

**架构识别与覆盖**：

- 新增缓存选项 `DUILIB_TARGET_ARCH`（`auto` / `x86` / `x64` / `arm64`，默认 `auto`）；
- Windows 多配置生成器（VS）从 `CMAKE_GENERATOR_PLATFORM` 取值（`Win32` → `x86`，
  其余转小写）；Ninja 生成器没有 generator platform，**交叉编译时必须显式**
  `-DDUILIB_TARGET_ARCH=arm64`，否则 `CMAKE_SIZEOF_VOID_P` 只能区分 32/64 位，
  会把 arm64 误判为 x64；
- Unix 默认取 `CMAKE_SYSTEM_PROCESSOR`，允许用 `DUILIB_TARGET_ARCH` 显式覆盖；
- 根工程在 `add_subdirectory` 之前解析并校验架构，结果写入
  `DUILIB_SKIA_CPU_ARCH`（供 `duilib_common.cmake` 与 `duilib_lib_output.cmake` 使用），
  避免各子项目重复推导；
- 校验规则：`x86` 必须 `CMAKE_SIZEOF_VOID_P=4`，`x64`/`arm64` 必须为 8，
  `DUILIB_SKIA_CPU_ARCH` 必须与解析结果一致，不一致即报错。

**各架构的依赖可用性**（决定默认开关与提示文案）：

| 依赖 | x86 | x64 | arm64 | 处理 |
| :--- | :--- | :--- | :--- | :--- |
| Skia | Windows | 全平台 | 全平台 | 必须有对应变体；缺失时报错并提示用 `deps` 目标预编译 |
| SDL3 | 可选（仅 Windows） | Windows 可选；Unix 必选 | Windows 可选；Unix 必选 | 非 Windows 平台强制启用；版本按清单固定（tag 优先）；**Windows 也由 `deps` 自动构建（Debug+Release 静态库）**，不再依赖手工产物；交叉编译时 `DUILIB_SDL_ROOT` 按架构分目录 |
| libjpeg-turbo | Windows MSVC | Windows MSVC | Windows MSVC | 仅 MSVC 预编译包；MinGW 不收集 |
| CEF | Windows MSVC | Windows MSVC / Linux / macOS | Windows MSVC / macOS | Windows 三架构由构建期按清单自动下载 `Release/*` + `Resources/*` 到 `bin/`，编译期仍用仓库头文件；MinGW/MSYS2 默认关闭（MSVC ABI 不兼容）；Linux arm64 以分发包实际可用性为准 |
| WebView2 | Windows | Windows | Windows | arm64 模块已提供，默认启用 |

`DUILIB_ENABLE_CEF` 在缺少可用运行时的平台默认 `OFF`：FreeBSD、MinGW/MSYS2
（CEF 为 MSVC ABI，链接必然失败）以及 Linux arm64 分发包缺失的情况，并在配置期
输出 `[WARN]` 说明原因；显式 `-DDUILIB_ENABLE_CEF=ON` 且依赖缺失时才 `FATAL_ERROR`。

**产物布局**：沿用现状——Windows `lib/<arch>/<Config>` 与 `bin/`，
Unix `lib/<Config>`；不同架构必须使用不同的构建目录，避免缓存互相污染。

### 1.2 构建系统交互语言（简体中文 + 英文）

目标：默认跟随系统语言（简体中文 → 中文，其他 → 英文），既降低中文用户门槛，
又保留英文作为兜底与问题反馈的共同语言。

**语言解析顺序**（`auto` 为默认）：

1. `--lang en|zh_CN`（`build.py`）或 `-DDUILIB_LANG=en|zh_CN|auto`；
2. 环境变量 `DUILIB_LANG`；
3. `LC_ALL` / `LC_MESSAGES` / `LANG`（Unix、MSYS2；`zh_CN*`、`zh-Hans*` → 中文）；
4. Windows：Python 用 `GetUserDefaultUILanguage`，CMake 读注册表 `LocaleName`；
   `zh-CN` / `zh-Hans` → 中文；
5. 无法判定或控制台不支持 UTF-8 → 英文。

**消息目录（唯一维护点）**：`cmake/i18n/messages.json`：

```json
{
  "preflight.missing_tool": {
    "en": "[ERROR] {tool} not found",
    "zh_CN": "[ERROR] 未找到 {tool}"
  }
}
```

- CMake：`cmake/duilib_i18n.cmake` 在配置期载入目录，通过 `duilib_msg(<key> ...)` 输出；
- Python：`build/i18n.py` 读取同一文件，`build.py`、`pack.py`、`release.py`、`autotest.py`
  统一调用；
- 缺翻译 → 回退英文并继续（不中断构建）；`en` 为基准语言，新增消息必须同时提供 `en`。

**书写规范**：

1. 状态标记与阶段前缀固定英文：`[OK]`、`[WARN]`、`[ERROR]`、`[check]`、`[deps]`、
   `[build]`、`[pack]`、`[test]`，便于日志检索与测试断言；其后正文按语言翻译；
2. 错误固定两行：`[ERROR] <what failed>` + `Hint: <one actionable command>`，
   `Hint:` 本身不翻译；
3. 命令、参数、路径、变量名、工具名、版本号不翻译；
4. 不翻译 CMake、编译器、git、ninja 等外部工具的原始输出，只翻译本项目自己的消息；
5. 只用返回码与文件存在性做判断，不解析本地化文本。

**编码与 Windows 控制台**：

- 消息文件与源码统一 UTF-8（无 BOM）；中文只出现在消息目录中，CMake 模块内不内联中文字符串；
- `build.bat` / `build.py` 在中文模式下切到 UTF-8（`chcp 65001` 或
  `SetConsoleOutputCP(65001)`）；
- 直接使用 CMake 且控制台不是 UTF-8（`chcp` ≠ 65001）时，CMake 自动降级为英文并输出一行
  `[WARN] Non-UTF-8 console; falling back to English. Run 'chcp 65001' or use build.bat.`；
- 无法切换代码页（重定向/CI）时保持英文，避免乱码。

**不适用范围**：代码注释（维持现状）、文档（中文/英文并存）、`pack.py` 生成的 MANIFEST/报告内容
（`test_pack.py` 有中文断言，如要本地化需单独统一并同步测试）。

**测试约束**：`test_pack.py` / `test_autotest.py` 断言 `[ERROR]`、`[OK]` 标记、退出码与关键字，
不断言整句；中英文两种语言下运行结果一致。

**成本评估**：

- 一次性成本：把现有用户可见字符串抽到目录（主要是 13 个 cmake 模块与 4 个 Python 脚本），
  属机械工作；新增目录载入器（CMake + Python 各一份，约 50~80 行）与语言探测；
- 持续成本：每条新消息提供 `en` + `zh_CN` 两个值；漏写自动回退英文，不影响构建；
- 主要风险在 Windows 代码页（`chcp 65001` + 自动降级已覆盖），逻辑判断仍依赖标记与返回码；
- 结论：成本可控、收益明显，建议采用双语方案。

**存量文案**：现有中文 `message()` 逐条迁入目录（`en` + `zh_CN` 两份），旧脚本删除后不再维护旧文案。

### 1.3 配置期环境预检（Preflight）

预检分为**两阶段**，避免“没有编译器/没有 CMake 时无法给出可读错误”：

- **阶段 A（工具与系统库）**：`cmake/duilib_preflight_tools.cmake` 以 `cmake -P` 脚本模式运行，
  不需要 `project()` 与编译器；根 `CMakeLists.txt` 在 `project()` **之前**用
  `execute_process` 调用，失败即 `FATAL_ERROR`；
- **阶段 B（编译器与架构）**：`cmake/duilib_preflight.cmake` 在 `project()` 之后运行，
  检查编译器身份/版本、目标架构、CRT、生成器平台。

`build.py doctor`：有 CMake 时调用 `cmake -P` 复用阶段 A 与 B 的清单；
没有 CMake 时退化为 **Python 内置探测**（扫描 PATH、注册表、`pkg-config`），
只给出最小可读清单，不因缺少 CMake 而完全无法自检。

**检测分级**：

- **REQUIRED**：缺失即 `FATAL_ERROR`，例如编译器、CMake 版本、VS 2022+；
- **CONDITIONAL**：当前配置需要时才检查，例如 `deps` 模式下的 `gn`/`ninja`/`git`；
- **OPTIONAL**：缺失只 `[WARN]` 并自动关闭对应功能，例如未启用 CEF 时不检查 CEF 包。

**通用检查**：CMake ≥ 3.24（`cmake_minimum_required` 已强制）、Python 3 ≥ 3.8、
Git、生成器对应的构建工具（Ninja 生成器 → `ninja`；Makefiles → `make`/`gmake`）、
`DUILIB_DEPS_MODE != off` 时的 `gn`、`unzip`/`tar`、`curl`/`wget`（CEF 下载）。

**平台检查清单**：

| 平台 | 必检项 | 条件/可选 |
| :--- | :--- | :--- |
| Windows MSVC | VS 2022+（MSVC v143、`vcvarsall`/`vswhere`）、Windows SDK、Python 3、Git | LLVM（Skia 的 `clang_win`，有预编译 Skia 时可跳过）、Windows 11 SDK（启用 CEF 时警告 10 SDK） |
| Windows MinGW / MSYS2 | 目标前缀的 gcc/clang（拒绝 MSYS/Cygwin 目标）、Ninja 或 make、CMake、Python 3、Git | MSYS2 下用 `pacman -Q` 检查对应前缀的 gcc 或 clang、cmake、ninja、gn、git、python、unzip，缺失时打印 `pacman -S --needed ...` |
| Linux | gcc 或 clang（支持 C++20）、`ninja` 或 `make`、CMake、Python 3、Git、`gn`、`unzip`、`bzip2`、`curl`/`wget` | `pkg-config` 检查 `x11`、`fontconfig`、`freetype2`、`gl`、`egl`、`glesv2`、`vulkan`、`xext`、`xcursor`、`xi`、`xrandr`、`dbus-1`、`ibus-1.0`；Wayland 会话追加 `wayland-client`、`xkbcommon` |
| macOS | Xcode Command Line Tools（`xcode-select -p`、`clang++`）、macOS ≥ 12、CMake、Ninja、`gn` | 启用 CEF 时校验 `CEF_ROOT` 下 `Release/`、`Resources/` 是否齐全 |
| FreeBSD | clang 或 gcc、CMake、Ninja、`gn`、Git、Python 3、`unzip`、`fontconfig`、`freetype2` | 启用 CEF 不支持（固定 `OFF`） |

**系统库缺失时的提示**：读取 `/etc/os-release` 的 `ID`/`ID_LIKE`，映射到
`apt` / `dnf` / `zypper` / `pkg`，直接打印 `README.md` 中对应的完整安装命令，
而不是只报“找不到 xxx”。示例：

```
[check] platform=linux arch=x64 compiler=GNU 13.2.0 generator=Ninja
[OK]    cmake 3.28.3
[OK]    python3 3.12.3
[WARN]  gn not found (required only for 'deps'; prebuilt Skia found)
[ERROR] system libraries missing: fontconfig freetype2
  Hint: sudo apt install -y libfontconfig-dev libfreetype-dev
```

**约束**：

1. 预检无副作用：不下载、不安装、不改文件，只做命令探测与 `pkg-config`/`vswhere` 查询；
2. 所有错误在本次配置结束前汇总输出，一次给出完整修复清单；
3. `DUILIB_PREFLIGHT=OFF` 可跳过（面向高级用户与已有完整环境的 CI 缓存），
   但不支持跳过架构与编译器校验；
4. 输出遵循 1.2 节双语规范（`[ERROR]`/`[WARN]`/`Hint:` 标记保持英文）；

## 2. 总体架构

根工程只做**编排**，不重写已有子项目：

```
根 CMakeLists.txt
  ├─ cmake/duilib_i18n.cmake           语言解析 + 消息目录加载（en/zh_CN）
  ├─ cmake/i18n/messages.json          构建系统消息目录（唯一维护点）
  ├─ cmake/duilib_root_options.cmake   平台/编译器/架构识别，统一缓存选项
  ├─ cmake/duilib_deps.cmake           依赖检查与准备（Skia / SDL / CEF）
  ├─ cmake/duilib_cef.cmake            CEF 下载/解压/复制（Windows + Linux 共用）
  ├─ cmake/third_party_versions.json   Skia / SDL / CEF 版本清单（唯一维护点）
  └─ 构建阶段
       ├─ add_subdirectory(duilib/third_party/zlib)
       ├─ add_subdirectory(duilib/third_party/libpng)
       ├─ add_subdirectory(duilib/third_party/cximage)
       ├─ add_subdirectory(duilib/third_party/libwebp)
       ├─ add_subdirectory(duilib/third_party/libcef/libcef_linux)   # 仅 Linux + CEF
       ├─ add_subdirectory(duilib)
       └─ add_subdirectory(examples/<name>)                          # 按平台筛选
```

依赖准备分三种模式，通过 `DUILIB_DEPS_MODE` 选择：

| 模式 | 行为 | 适用 |
| :--- | :--- | :--- |
| `off`（默认） | 只做检查，缺失时按现状报错 | 依赖已由脚本/上一次构建准备好 |
| `prepare` | 只配置依赖目标，`--target duilib_deps` 完成准备 | 推荐的两阶段流程 |
| `auto` | 配置期跳过依赖存在性校验，构建期先跑依赖目标 | 单命令体验（macOS CEF 除外） |

两阶段是推荐路径，原因是 macOS 的 `cmake/duilib_cef_macos.cmake` 在**配置期**就
`find_package(CEF)`，CEF 分发包必须先解压到位；Skia/SDL 则可以在构建期用
`add_dependencies` 保证顺序。

## 3. 依赖准备设计

### 3.1 统一变量

| 变量 | 默认值 | 说明 |
| :--- | :--- | :--- |
| `DUILIB_DEPS_ROOT` | `${CMAKE_SOURCE_DIR}/..` | 与现有脚本一致的同级目录 |
| `DUILIB_SKIA_ROOT` | `${DUILIB_DEPS_ROOT}/skia` | Skia 源码与 `out/` |
| `DUILIB_SKIA_COMPILE_ROOT` | `${DUILIB_DEPS_ROOT}/skia_compile` | 补丁 zip 来源 |
| `DUILIB_VERSIONS_FILE` | `${CMAKE_SOURCE_DIR}/cmake/third_party_versions.json` | Skia / SDL / CEF 版本清单（唯一维护点） |
| `DUILIB_SKIA_REPO` | 清单 `skia.repo` | 可替换为镜像 |
| `DUILIB_SKIA_REF` | 清单 `skia.default` | 清单中的键；支持 commit / branch / tag |
| `DUILIB_SKIA_PATCH_ZIP` | 该 ref 的 `patchZip` | 相对 `skia_compile/`，按版本配套 |
| `DUILIB_SDL_SRC` | `${DUILIB_DEPS_ROOT}/SDL` | SDL 源码 |
| `DUILIB_SDL_REPO` | 清单 `sdl.repo` | 可替换为镜像 |
| `DUILIB_SDL_REF` | 清单 `sdl.default` | 清单中的键；tag（推荐）或 branch |
| `DUILIB_SDL_ROOT` | `${DUILIB_DEPS_ROOT}/SDL3` | SDL 安装前缀（脚本同名） |
| `DUILIB_SDL_BUILD_ROOT` | `${DUILIB_DEPS_ROOT}/SDL3_build` | Windows 自动构建 SDL 的中间目录（按工具链/架构/配置分） |
| `DUILIB_CEF_ROOT` | `${DUILIB_DEPS_ROOT}/cef_binary` | macOS 分发包根目录 |
| `DUILIB_CEF_VERSION` | 清单 `cef.default` | 可选 `109` / `142` |
| `DUILIB_CEF_URL_BASE` | 清单中的 `baseUrl` | 官方 CDN，可替换为镜像 |
| `DUILIB_CEF_CACHE` | `${DUILIB_DEPS_ROOT}/cef_binary` | 下载包与解压暂存目录 |
| `DUILIB_CEF_ARCHIVE` | 空 | 指定的本地 CEF 包，跳过下载（按当前平台/架构） |
| `DUILIB_CEF_REQUIRED` | `ON` | 开启 CEF 时下载失败是否中断；`OFF` 则告警并关闭 CEF |
| `DUILIB_DEPS_AUTO_CLONE` | `ON` | 允许 git clone；关闭时只检查 |
| `DUILIB_DEPS_JOBS` | 6 | Skia/SDL 并行度（对齐脚本 `-j 6`） |
| `DUILIB_DEPS_LOCK_TIMEOUT` | 5 | 依赖准备锁的等待秒数；超时报“已有实例在运行” |
| `DUILIB_DOWNLOAD_RETRIES` | `0` | `0` = 无限重试；`N>0` = 最多重试 N 次 |
| `DUILIB_DOWNLOAD_RETRY_DELAY` | 3 | 首次重试间隔秒数；指数退避，上限 60 秒 |
| `DUILIB_DOWNLOAD_CONNECT_TIMEOUT` | 30 | 连接超时秒数 |
| `DUILIB_LLVM_ROOT` | `C:/LLVM` | Windows Skia 的 `clang_win` 根目录 |
| `DUILIB_GN_EXECUTABLE` | 空 | 覆盖 `gn` 可执行文件路径（默认用 Skia 自带或 PATH） |
| `DUILIB_NINJA_EXECUTABLE` | 空 | 覆盖 `ninja` 可执行文件路径 |
| `DUILIB_MD` | `OFF` | `ON` → Skia 用 `/MD`、`/MDd`；`OFF` → `/MT`、`/MTd` |
| `DUILIB_SKIA_REQUIRE_PATCH` | `ON` | 所选 Skia ref 缺少 `patchZip` 时报错 |
| `DUILIB_SDL_RPATH` | `ON` | Unix 产物写入 rpath（相对条目在前） |
| `DUILIB_RPATH_ABSOLUTE` | `ON` | rpath 是否追加 `${DUILIB_SDL_ROOT}/lib` 等绝对兜底路径；`OFF` 时只保留相对条目 |

所有路径都用 `CACHE PATH`，便于下游复用已有的 skia/SDL3/cef_binary 缓存。

### 3.2 Skia

复用脚本的输出目录命名规则（`cmake/duilib_common.cmake` 也按此查找）：

| 平台/工具链 | 变体目录 |
| :--- | :--- |
| Windows MSVC | `out/llvm.<x64\|x86>.release`、`out/llvm.<x64\|x86>.debug` |
| Linux/macOS clang | `out/llvm.<arch>.release` |
| Linux/macOS gcc | `out/gcc.<arch>.release` |
| MSYS2 MINGW64（msvcrt） | `out/gcc.msvcrt.<arch>.release` |

实现要点：

1. **版本配置**：版本来自 `cmake/third_party_versions.json` 的 `skia` 段；
   `DUILIB_SKIA_REF` 是清单中的键，每个键给出 `type`（`commit` / `branch` / `tag`）、
   `value` 和配套的 `patchZip`。默认值取 `skia.default`（保持当前基线
   `6f559bafbed4c8323a899df4008aa073df4eccc6`），可用
   `-DDUILIB_SKIA_REF=<键>` 临时切换；
2. **就绪判定**：`out/<变体>/` 下存在 `libskia.a`（Unix）或 `skia.lib`（Windows），
   且 `out/<变体>/.duilib-ref` 记录与当前 ref + 补丁组合一致时跳过；不一致则重新
   `gn gen` + `ninja`（原地重建，不删除 `out/`）；
3. **源码获取**：`ExternalProject_Add(skia_ext GIT_REPOSITORY ${DUILIB_SKIA_REPO}
   GIT_TAG <value>)`；`tag`/`branch` 用 `GIT_SHALLOW TRUE`，`commit` 用
   `GIT_SHALLOW FALSE`。已存在 `.git` 时先 `git fetch --tags --prune` 再
   `git checkout <value>`（固定 ref 不做 `pull`，避免漂移）；
4. **补丁**：解压该 ref 配套的 `patchZip`（`cmake -E tar xf`，替代 `unzip`/`miniunz`），
   沿用脚本幂等判断（`gn/is_mingw.py` 与 `gn/BUILDCONFIG.gn` 的 `is_mingw`），
   并在 `out/<变体>/.duilib-patch` 记录 zip 哈希；ref 没有配置 `patchZip` 且
   `DUILIB_SKIA_REQUIRE_PATCH=ON`（默认）时报错；
5. **gn/ninja**：优先使用 Skia 自带的 `bin/gn`、`bin/ninja`（Windows），
   否则用 PATH 中的可执行文件；变量 `DUILIB_GN_EXECUTABLE`、
   `DUILIB_NINJA_EXECUTABLE` 可覆盖；
6. **gn args**：按 平台 × 编译器 × CRT 固定参数，不再依赖脚本分支逻辑，见下表；
   **不调用 `tools/git-sync-deps`**：源码树默认已包含 `third_party/externals`
   （由现有 skia 仓库与补丁包提供），预检只校验关键目录存在；
7. **构建**：`CONFIGURE_COMMAND <gn> gen out/<变体> --args=...`，
   `BUILD_COMMAND <ninja> -C out/<变体>`，`BUILD_IN_SOURCE TRUE`，
   `INSTALL_COMMAND ""`；
8. 若 `DUILIB_DEPS_MODE=auto`，`cmake/duilib_common.cmake` 的 Skia 存在性校验
   需要在 `DUILIB_SKIA_AUTOBUILD=ON` 时跳过，并直接按变体名设置
   `DUILIB_SKIA_LIB_SUBPATH`（MinGW 的 `_duilib_check_mingw_skia_lib` 同样跳过）。

**gn args 矩阵**（公共参数 + 平台差异）：

公共参数（所有平台）：

```
is_official_build=true
is_debug=false
is_trivial_abi=false
skia_use_libwebp_encode=false
skia_use_libwebp_decode=false
skia_use_libpng_encode=false
skia_use_libpng_decode=false
skia_use_zlib=false
skia_use_libjpeg_turbo_encode=false
skia_use_libjpeg_turbo_decode=false
skia_enable_fontmgr_win_gdi=false
skia_use_icu=false
skia_use_expat=true
skia_use_system_expat=false
skia_use_xps=false
skia_enable_pdf=false
skia_use_wuffs=false
skia_enable_svg=true
skia_use_partition_alloc=false
extra_cflags=["-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER"]
```

| 平台 / 编译器 | `target_cpu` | 额外参数 | 变体目录 |
| :--- | :--- | :--- | :--- |
| Windows MSVC（clang-cl 前端） | `x64` / `x86` / `arm64` | `cc="clang" cxx="clang++" clang_win="${DUILIB_LLVM_ROOT}"`；`extra_cflags` 追加 CRT：`/MT`、`/MTd`、`/MD`、`/MDd` | `out/llvm.<arch>.debug`、`out/llvm.<arch>.release` |
| Linux clang | `x64` / `arm64` | `cc="clang" cxx="clang++" skia_enable_fontmgr_fontconfig=true skia_use_freetype=true` | `out/llvm.<arch>.release` |
| Linux gcc | `x64` / `arm64` | `cc="gcc" cxx="g++" skia_enable_fontmgr_fontconfig=true skia_use_freetype=true` | `out/gcc.<arch>.release` |
| MSYS2 gcc（mingw64） | `x64` | 同上，另用 `gcc.msvcrt.<arch>.release` | `out/gcc.msvcrt.x64.release` |
| MSYS2 gcc（ucrt64/clang64） | `x64` | 同上 | `out/gcc.<arch>.release` |
| FreeBSD clang | `x64` / `arm64` | `ar="llvm-ar"`；`extra_ldflags=["-L/usr/local/lib"]`；`extra_cflags` 追加 `-I/usr/local/include/freetype2`、`-I/usr/local/include`；字体后端同 Linux | `out/llvm.<arch>.release` |
| macOS clang | `x64` / `arm64` | `cc="clang" cxx="clang++"`；字体后端使用 Skia 原生 CoreText（不设 fontconfig） | `out/llvm.<arch>.release` |

说明：

- Windows 的 `debug` / `release` 变体**都用 `is_debug=false`**，差异只在
  `extra_cflags` 的 CRT 开关（`/MT` vs `/MTd`、`/MD` vs `/MDd`），与现有脚本一致；
- `DUILIB_LLVM_ROOT` 默认 `C:/LLVM`（对齐脚本），预检校验 `clang.exe` 存在；
- `DUILIB_MD=ON` 时 CRT 用 `/MD`/`/MDd`，否则 `/MT`/`/MTd`，同一构建必须与
  `DUILIB_MD`、CEF wrapper 保持一致（见 10.4 的 CRT 矩阵）。

**升级方式**：在清单中新增一个 ref 条目（或把 `skia.default` 指向新条目），
重新执行 `deps` 即可；不需要改 CMake 逻辑。Skia 上游没有正式 release tag，
因此支持固定 `commit` 与里程碑分支（如 `chrome/m142`），建议先验证再在清单中
记录解析后的 commit，保证可复现。

### 3.3 SDL

- **可选性**：Windows 下 `DUILIB_ENABLE_SDL` 默认 `OFF`（可选）；Linux/macOS/FreeBSD
  下为必选，忽略 `-DDUILIB_ENABLE_SDL=OFF` 并输出 `[WARN]`；
- **版本配置**：来自清单 `sdl` 段的 `sdl.default`，默认固定为 `release-3.4.18` tag，
  也允许 `main`（`type=branch`）；`DUILIB_SDL_REF` 可临时切换；
- **源码获取**：`ExternalProject_Add(sdl_ext GIT_REPOSITORY ${DUILIB_SDL_REPO}
  GIT_TAG <value>)`，tag/branch 用 `GIT_SHALLOW TRUE`；固定 tag 时不做 `git pull`；
- **就绪判定**：`${DUILIB_SDL_ROOT}/include/SDL3/SDL.h` 存在，且
  `${DUILIB_SDL_ROOT}/.duilib-ref`（ref + 架构）与 `.duilib-crt`（运行库/配置）均与
  当前配置一致时跳过；不一致则重新配置、构建并安装到同一前缀；
- **构建参数**：

  | 平台 / 工具链 | 关键参数 | 说明 |
  | :--- | :--- | :--- |
  | Windows MSVC | `SDL_SHARED=OFF`、`SDL_STATIC=ON`、`SDL_TEST_LIBRARY=OFF` | 静态库，不产生 DLL，无需复制运行库 |
  | Windows MinGW/MSYS2 | 同上 | 与 duilib 使用同一编译器前缀 |
  | Linux / macOS / FreeBSD | `SDL_SHARED=ON`、`SDL_STATIC=OFF`、`SDL_TEST_LIBRARY=OFF` | 安装到 `${DUILIB_SDL_ROOT}`；沿用现有 `SDL_X11_XSCRNSAVER=OFF`、`SDL_X11_XTEST=OFF`，X11/Wayland/音频后端保持上游默认 |
  | 交叉编译 | 每架构独立 `DUILIB_SDL_ROOT`（如 `SDL3-<arch>`） | 避免架构间污染 |

**Windows 自动构建（新增能力）**：

- **现状**：`build_duilib_all_in_one.bat` 不构建 SDL；只有 MSYS2 的 `-sdl` 路径会在
  `.sh` 里构建 Release 版，MSVC/MinGW 的 SDL 一直是手工构建。根工程把这条流程补齐；
- **触发条件**：`DUILIB_ENABLE_SDL=ON` 时，Windows（MSVC / MinGW / MSYS2）由
  `duilib_deps` 自动获取源码、构建并安装；默认 `OFF` 时跳过（保持现状）；
- **中间目录**：`${DUILIB_SDL_BUILD_ROOT}/<toolchain>/<arch>/<config>`，
  与安装前缀分开，避免源树/安装目录混用；
- **配置覆盖**：一次 `deps` 同时构建 **Release 与 Debug**（`CMAKE_DEBUG_POSTFIX=d`），
  产物为 `SDL3-static.lib` / `SDL3-staticd.lib`（MSVC）或 `libSDL3.a` / `libSDL3d.a`
  （MinGW），避免 Debug 误链 Release 库导致的 CRT 冲突；
- **运行库**：MSVC 通过 `CMAKE_MSVC_RUNTIME_LIBRARY` 与 `DUILIB_MD` 保持一致
  （`MultiThreaded[Debug]` 或 `MultiThreaded[Debug]DLL`）；MinGW/MSYS2 必须使用与
  主工程相同的 MSYS2 前缀（`mingw64`/`ucrt64`/`clang64`），msvcrt 与 ucrt 不可混用，
  `deps` 把 `MSYSTEM` 与编译器 triple 写入 stamp 供配置期比对；
- **生成器**：沿用根工程生成器与工具链；MSYS2 固定 `Ninja`（原生 CMake 默认 VS
  生成器会静默忽略 `-DCMAKE_C_COMPILER`）；
- **其他参数**：`SDL_SHARED=OFF`、`SDL_STATIC=ON`、`SDL_TEST_LIBRARY=OFF`，
  X11/Wayland/音频选项在 Windows 上无意义，不再传入；
- **就绪判定**：`${DUILIB_SDL_ROOT}/include/SDL3/SDL.h`、当前配置对应库文件、
  `.duilib-ref`（ref + 架构）与 `.duilib-crt`（运行库/`MSYSTEM`）同时匹配才跳过；
  缺 Debug 库时提示重新执行 `deps`，不静默回退到 Release 库。

- **运行库搜索路径（rpath）**：Unix 默认开启（`DUILIB_SDL_RPATH=ON`），
  先写入相对条目（macOS `@loader_path`、其他 `$ORIGIN`；Linux 另含 CEF 的
  `libcef_linux`、SDL 的 `../lib`），保证打包后优先加载包内库；
  `DUILIB_RPATH_ABSOLUTE=ON`（默认）时再追加 `${DUILIB_SDL_ROOT}/lib` 绝对路径，
  让构建树中的示例无需环境变量即可运行。`DUILIB_RPATH_ABSOLUTE=OFF` 时只保留
  相对条目，产物完全可迁移，构建树运行需自行设置 `LD_LIBRARY_PATH`/
  `DYLD_LIBRARY_PATH`。FreeBSD 的 `/usr/local/lib` 也受该开关控制。
  实现上统一设置 `CMAKE_INSTALL_RPATH` 与 `CMAKE_BUILD_RPATH`，并设置
  `CMAKE_BUILD_WITH_INSTALL_RPATH=TRUE`：构建树直接使用安装期 rpath，避免 CMake
  把 `target_link_directories` 里的绝对路径（静态库目录、Skia 输出目录）自动写进
  `BUILD_RPATH`。`DUILIB_SDL_RPATH=OFF` 仍表示完全不写 rpath。
- **验证**：Linux/FreeBSD `readelf -d bin/<example> | grep -E 'RPATH|RUNPATH'`、
  macOS `otool -L bin/<example>`，确认指向 `${DUILIB_SDL_ROOT}/lib`（FreeBSD 另含
  `/usr/local/lib`）而非 `/usr/lib`；若系统已安装同名 SDL3，`ldd` 结果必须指向
  构建前缀。
- **升级方式**：清单中新增 tag 条目或更新 `sdl.default`，重新执行 `deps` 即完成升级。

### 3.4 CEF

三个平台统一走“版本清单 → 按平台/架构下载 → 解压 → 按约定布局复制”的流程，
Windows 与 Linux 共用同一套实现，只是复制目标不同。

#### 3.4.1 版本清单（唯一维护点）

新增 `cmake/third_party_versions.json`，顶层包含 `skia`、`sdl`、`cef` 三段；
新增或升级任一依赖时**只改这个文件**，构建逻辑不动。`cef` 段结构如下：

```json
"cef": {
  "default": "142",
  "baseUrl": "https://cef-builds.spotifycdn.com",
  "versions": {
    "109": {
      "archives": {
        "windows/x86":   { "file": "..._windows32_minimal.tar.bz2",   "sha256": "" },
        "windows/x64":   { "file": "..._windows64_minimal.tar.bz2",   "sha256": "" },
        "windows/arm64": { "file": "..._windowsarm64_minimal.tar.bz2", "sha256": "" }
      }
    },
    "142": {
      "archives": {
        "windows/x86":   { "file": "..._windows32_minimal.tar.bz2",   "sha256": "" },
        "windows/x64":   { "file": "..._windows64_minimal.tar.bz2",   "sha256": "" },
        "windows/arm64": { "file": "..._windowsarm64_minimal.tar.bz2", "sha256": "" },
        "linux/x64":     { "file": "..._linux64_minimal.tar.bz2",     "sha256": "" },
        "linux/arm64":   { "file": "..._linuxarm64_minimal.tar.bz2",   "sha256": "" },
        "macos/x64":     { "file": "..._macosx64_minimal.tar.bz2",     "sha256": "" },
        "macos/arm64":   { "file": "..._macosarm64_minimal.tar.bz2",   "sha256": "" }
      }
    }
  }
}
```

- `file` 为完整文件名（`baseUrl + "/" + file` 组成下载地址），`sha256` 为空时首次下载后
  告警提示补齐，不阻断；`url` 可选，用于指定镜像或历史版本地址；
- 支持版本 `/ 平台 / 架构` 任意组合，缺失组合视为“该版本不支持该架构”，预检阶段报错；
- `DUILIB_CEF_VERSION` 默认取 `default`；旧选项 `DUILIB_CEF_109=ON` 映射为 `109`
  并输出弃用提示；
- CMake 用 `string(JSON ...)` 读取，`build.py` 也可直接读同一文件做 `doctor` 展示。

`skia` / `sdl` 段示例（同一文件）：

```json
"skia": {
  "default": "6f559baf...",
  "repo": "https://skia.googlesource.com/skia.git",
  "refs": {
    "6f559baf...": {
      "type": "commit",
      "value": "6f559bafbed4c8323a899df4008aa073df4eccc6",
      "patchZip": "skia.2026-09-16.src.zip"
    }
  }
},
"sdl": {
  "default": "release-3.4.18",
  "repo": "https://github.com/libsdl-org/SDL.git",
  "refs": {
    "release-3.4.18": { "type": "tag" },
    "main":           { "type": "branch" }
  }
}
```

- `skia.refs` 的键用于 `DUILIB_SKIA_REF`，`sdl.refs` 的键用于 `DUILIB_SDL_REF`；
- 每次升级只需新增一个键并更新 `default`，不新增代码分支。

#### 3.4.2 Windows（自动下载运行时）

设计原则：**`duilib/third_party/libcef` 只提供编译期文件（头文件、wrapper、CMake 脚本），
已提交 git 管理，构建系统不得修改它**；自动下载只把运行期文件补齐到 `bin/`，
用于解决示例程序无法启动（缺 DLL、`Resources`、`locales`）的问题。

| 项 | 规则 |
| :--- | :--- |
| 编译期目录（不复制） | 142 → `duilib/third_party/libcef/libcef_win`；109 → `.../libcef_win_109` |
| 运行期目录 | 142 → `bin/libcef_win/<arch>`；109 → `bin/libcef_win_109/<arch>`（`.gitignore` 已忽略） |
| 架构目录名 | `x86` → `Win32`，`x64` → `x64`，`arm64` → `arm64` |
| 复制内容 | 仅 `Release/*` 与 `Resources/*`（DLL、`.pak`、`locales`、`*.bin` 等运行所需文件） |
| 不复制 | `cmake/`、`include/`、`libcef_dll/` 等开发文件，编译一律使用仓库内版本 |
| 版本匹配 | 下载包 `include/cef_version.h` 的 `CEF_VERSION` 必须与仓库对应版本目录的同名宏完全一致 |

**导入库说明**：`Release/libcef.lib` 会随 `Release/*` 一并复制到 `bin/.../<arch>/`，
但**链接不依赖它**：三架构（`Win32` / `x64` / `arm64`）均使用仓库内
`duilib/third_party/libcef/libcef_win[_109]/lib/<arch>/libcef.lib`，arm64 导入库已提交，
`.gitignore` 的 `arm64/` 忽略规则已调整。

实现要点：

1. **版本预检（先于复制）**：CMake 解析仓库
   `libcef_win[_109]/include/cef_version.h` 与解压包 `include/cef_version.h` 中的
   `CEF_VERSION`，字符串不一致即 `[ERROR] CEF runtime/header version mismatch`，
   并提示改用匹配的 `DUILIB_CEF_VERSION` 或更新仓库头文件；版本号必须与
   `third_party_versions.json` 中选定的版本一致；
2. **就绪判定**：`bin/<运行时目录>/<arch>/libcef.dll`、
   `Resources/locales/*.pak` 与 `.cef-version`（内容为 `版本 + 架构`）同时存在才跳过；
   缺 stamp 时重新解压并覆盖当前架构，不信任“目录已存在”；
3. **下载**：`file(DOWNLOAD ... EXPECTED_HASH SHA256=...)` 到
   `${DUILIB_CEF_CACHE}`；已有文件且哈希匹配时不重复下载；
4. **解压与复制**：解压到 `${DUILIB_CEF_CACHE}/cef_binary_<ver>_windows<arch>`，
   只将 `Release/*`、`Resources/*` 复制到 `bin/.../<arch>/`；其他架构目录不受影响；
5. **切换版本/架构**：只更新目标目录与 stamp，旧版本目录保留，便于回切；
6. **离线**：`-DDUILIB_CEF_ARCHIVE=<本地包>` 跳过下载，仍执行版本校验、解压与复制；
7. **失败策略**：`DUILIB_CEF_REQUIRED=ON`（默认）→ `FATAL_ERROR`；
   `OFF` → `[WARN]` 并自动关闭 CEF；
8. **Debug**：CEF 只用 Release 的 `libcef.lib` 与运行时，Debug 配置同样链接它；
   需要 Debug 版 CEF 的用户自行手工部署，构建系统不提供；
9. 计划在 P1 实现 `cmake/duilib_cef.cmake`，Windows/Linux 共用下载、校验、解压与复制逻辑；
   macOS 仍走两阶段（先 `deps` 解压到 `CEF_ROOT`，再配置）。

#### 3.4.3 Linux 与 macOS

- Linux：按清单下载 `linux/x64` 或 `linux/arm64` 包，只复制 `Release/*`、`Resources/*` 到
  `bin/libcef_linux`（`.gitignore` 已忽略，`duilib_common.cmake` 的
  `DUILIB_CEF_LIB_PATH` 已指向该目录）；编译期头文件与 wrapper 仍用仓库内
  `duilib/third_party/libcef/libcef_linux`，版本匹配校验同 3.4.2；
- macOS：解压到 `${DUILIB_CEF_ROOT}` 并导出 `CEF_ROOT`，由 `duilib_cef_macos.cmake`
  在配置期 `find_package(CEF)`，因此清单中的 macOS 包必须先准备（两阶段）；
- Linux/macOS 的版本同样由清单控制，当前只提供 142；新增 109 时补一条 `archives` 记录即可。

#### 3.4.4 WebView2 与 libjpeg-turbo（保持现状）

- **WebView2**：SDK 与三架构模块由维护者手工升级并提交到仓库，根工程**不下载、不做版本清单**；
  CMake 继续按仓库现有路径与架构目录选择，不新增自动化变量。
- **libjpeg-turbo**：以仓库已提交的预编译包为准，按架构选择 `.lib`/头文件，
  Debug/Release 与 `/MD`、`/MT` 的匹配沿用现有 props 与 CMake 逻辑，不纳入本次改造。

### 3.5 下载与并发控制

**下载器**：新增 `cmake/duilib_download.cmake`（`cmake -P` 脚本模式），
被 CEF 以及需要下载的依赖复用：

1. **实现优先级**：优先调用 `curl -fL --connect-timeout ... --continue-at - -o <file>`，
   支持**断点续传**；没有 `curl` 时回退 `file(DOWNLOAD)`（无续传，预检提示安装 curl）；
2. **重试**：`DUILIB_DOWNLOAD_RETRIES` 默认 `0` 表示**无限重试**，设为 `N>0` 时最多重试 N 次；
   重试间隔从 `DUILIB_DOWNLOAD_RETRY_DELAY`（默认 3 秒）开始指数退避，上限 60 秒；
   连接超时由 `DUILIB_DOWNLOAD_CONNECT_TIMEOUT`（默认 30 秒）控制；
3. **完整性**：下载完成后校验 `sha256`；不匹配时删除文件并重试，**最多 3 次**后
   `FATAL_ERROR`（哈希错误通常是清单问题，无限重试没有意义）；
   已存在的文件先校验哈希，匹配则跳过，不匹配则重新下载；
4. **不处理代理**：不探测、不配置代理，`curl` 自然继承 `http_proxy`/`https_proxy`
   环境变量；镜像通过清单的 `url` 或 `DUILIB_CEF_URL_BASE` 指定；
5. **磁盘与路径**：`doctor` 检查可用空间（Skia + SDL + CEF 首次准备建议 ≥ 10 GB），
   不足只 `[WARN]`；所有路径加引号，Windows 下依赖根路径超过 150 字符时 `[WARN]`
   提示缩短，避免长路径问题；中文/非 ASCII 路径本身不做转码（CMake、curl、ninja
   均按 UTF-8 传递），仅在 Windows 上遇到 `clang`/`ninja` 报路径错误时提示改用
   ASCII 路径，并优先把缓存目录放在 `DUILIB_DEPS_ROOT`（而非用户目录）；
6. **进度**：`curl` 保留进度输出；CMake 侧按阶段打印目标 URL、缓存路径与重试次数。

**并发控制**：依赖目录（`../skia`、`../SDL3`、`../cef_binary`）是跨构建目录共享的，
多个 `deps` 同时执行会互相破坏：

- 在 `duilib_deps` 准备开始时获取
  `file(LOCK "${DUILIB_DEPS_ROOT}/.duilib_deps.lock" GUARD PROCESS TIMEOUT <n>)`；
- 获取失败 → `[ERROR] another dependency preparation is already running` +
  `Hint: wait for it to finish or remove a stale lock file`；
- 锁只覆盖源码 checkout、补丁、`gn gen`/`ninja`、下载/解压/复制，不覆盖主工程编译；
- `DUILIB_DEPS_LOCK_TIMEOUT` 默认 5 秒；`GUARD PROCESS` 在进程退出时自动释放，
  异常退出留下的锁文件按提示手工删除；
- `deps --force` 不绕过锁。

### 3.6 版本清单校验与升级流程

**结构约束**（由 `doctor` 与配置期共同校验）：

- 必须存在 `schemaVersion`、`skia`、`sdl`、`cef`；未知顶层键只 `[WARN]`（向前兼容）；
- `skia.default` / `sdl.default` 必须是对应 `refs` 中的键，`cef.default` 必须存在于
  `versions`；缺失 → `FATAL_ERROR`；
- `refs.<key>.type` 只能是 `commit` / `branch` / `tag`；`value` 非空；
  `skia` 条目缺少 `patchZip` 且 `DUILIB_SKIA_REQUIRE_PATCH=ON` → 报错；
- `cef.versions.<ver>.archives.<platform>/<arch>` 的 `file` 必填；
  `url` 可选（镜像）；`sha256` 必须是 64 位十六进制，为空时 `[WARN]` 并跳过校验
  （首次接入允许，发布前必须补齐）；
- `doctor` 打印解析结果：版本、ref 类型、平台/架构、最终 URL、缓存路径、哈希状态。

**升级流程**（Skia / SDL / CEF 通用）：

1. 在清单中新增 ref 或 version 条目，保留旧条目用于回滚；
2. 本地执行 `deps --force`，确认下载、补丁、编译全部通过；
3. 补齐/更新 `sha256`，必要时更新 `patchZip`；
4. 全量构建并运行 `basic` 与对应 CEF 示例；
5. `pack --verify` 与 `test_pack.py` / `test_autotest.py` 通过；
6. 更新 `default` 指向新条目并提交清单；
7. **回滚**：把 `default` 改回旧条目并重新 `deps`，不删除旧下载缓存。

## 4. 根 CMakeLists 设计

### 4.1 选项

| 选项 | 默认 | 说明 |
| :--- | :--- | :--- |
| `DUILIB_TARGET_ARCH` | `auto` | `auto` / `x86` / `x64` / `arm64`；Windows + Ninja 交叉编译必须显式指定 |
| `DUILIB_LANG` | `auto` | `auto` / `en` / `zh_CN`；交互语言，见 1.2 节 |
| `DUILIB_PREFLIGHT` | `ON` | `OFF` 跳过工具/系统库预检（架构与编译器校验仍生效） |
| `DUILIB_DEPS_MODE` | `off` | `off` / `prepare` / `auto` |
| `DUILIB_BUILD_THIRD_PARTY` | `ON` | 构建 zlib/libpng/cximage/libwebp |
| `DUILIB_BUILD_EXAMPLES` | `ON` | 构建示例程序 |
| `DUILIB_EXAMPLES` | 平台默认列表 | 可用分号列表裁剪 |
| `DUILIB_ENABLE_SDL` | Windows `OFF`（可选），其他平台强制 `ON` | `-D...=OFF` 在非 Windows 被忽略并告警 |
| `DUILIB_SKIA_REF` | 清单 `skia.default` | 清单键；commit / branch / tag 均可 |
| `DUILIB_SDL_REF` | 清单 `sdl.default`（`release-3.4.18`） | 清单键；tag 优先，可选 `main` |
| `DUILIB_DEPS_LOCK_TIMEOUT` | `5` | 依赖准备并发锁等待秒数 |
| `DUILIB_DOWNLOAD_RETRIES` | `0` | `0` = 无限重试；`N>0` = 有限重试次数 |
| `DUILIB_DOWNLOAD_RETRY_DELAY` | `3` | 重试起始间隔（秒），指数退避上限 60 秒 |
| `DUILIB_DOWNLOAD_CONNECT_TIMEOUT` | `30` | 下载连接超时（秒） |
| `DUILIB_LLVM_ROOT` | `C:/LLVM` | Windows Skia `clang_win` 根目录 |
| `DUILIB_GN_EXECUTABLE` / `DUILIB_NINJA_EXECUTABLE` | 空 | 覆盖 `gn` / `ninja` 路径 |
| `DUILIB_MD` | `OFF` | Windows CRT：`OFF` → `/MT`、`/MTd`；`ON` → `/MD`、`/MDd` |
| `DUILIB_SKIA_REQUIRE_PATCH` | `ON` | Skia ref 缺少 `patchZip` 时报错 |
| `DUILIB_SDL_RPATH` | `ON` | Unix 写入 rpath（相对条目在前） |
| `DUILIB_RPATH_ABSOLUTE` | `ON` | rpath 是否追加 `${DUILIB_SDL_ROOT}/lib` 等绝对兜底路径 |
| `DUILIB_ENABLE_CEF` | Windows MSVC 三架构 / Linux / macOS `ON`；MinGW、MSYS2、FreeBSD `OFF` | 与脚本构建的示例集合一致；MinGW 因 MSVC ABI 不兼容默认关闭 |
| `DUILIB_CEF_VERSION` | 清单 `default`（当前 `142`） | `109` / `142`；只改 `third_party_versions.json` 即可升级 |
| `DUILIB_CEF_109` | `OFF` | 旧选项，`ON` 等价于 `DUILIB_CEF_VERSION=109` |
| `DUILIB_CEF_REQUIRED` | `ON` | 下载失败是否中断；`OFF` 时告警并关闭 CEF |
| `DUILIB_BUILD_TYPE` | `CMAKE_BUILD_TYPE` 或 `Release` | 多配置生成器的兜底 |

平台示例列表（与 `linux_build.sh`、`msys2_build.sh` 对齐）：

- Linux：`basic controls ... cef CefBrowser ...`（含 SDL）
- Windows：`basic controls ... WebView2 WebView2Browser ...`（CEF 示例可加）
- macOS：非 CEF 示例 + `cef` / `CefBrowser`（每个都是独立 `.app`）

### 4.2 执行流程

```cmake
cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/duilib_i18n.cmake)           # 语言解析（不依赖 project）

# 阶段 A：无编译器环境预检（脚本模式）
execute_process(
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/duilib_preflight_tools.cmake"
    RESULT_VARIABLE _preflight_rc)
if(NOT _preflight_rc EQUAL 0)
    message(FATAL_ERROR "Environment preflight failed. See messages above.")
endif()

project(nim_duilib_all LANGUAGES C CXX)

include(cmake/duilib_root_options.cmake)   # 平台/架构识别与选项
include(cmake/duilib_preflight.cmake)      # 阶段 B：编译器/架构/CRT
if(DUILIB_PREFLIGHT_ONLY)
    return()                               # 只跑预检
endif()
include(cmake/duilib_deps.cmake)           # 依赖检查/准备函数

if(DUILIB_DEPS_MODE STREQUAL "prepare")
    duilib_add_dependency_targets()        # 只生成 duilib_deps 目标
    return()
endif()

duilib_check_or_schedule_dependencies()    # off: 严格检查；auto: 配置期跳过

if(DUILIB_BUILD_THIRD_PARTY)
    add_subdirectory(duilib/third_party/zlib)
    add_subdirectory(duilib/third_party/libpng)
    add_subdirectory(duilib/third_party/cximage)
    add_subdirectory(duilib/third_party/libwebp)
endif()
if(DUILIB_ENABLE_CEF AND UNIX AND NOT APPLE)
    add_subdirectory(duilib/third_party/libcef/libcef_linux)
endif()
add_subdirectory(duilib)

if(DUILIB_BUILD_EXAMPLES)
    foreach(_name IN LISTS DUILIB_EXAMPLES)
        add_subdirectory(examples/${_name})
    endforeach()
endif()

if(DUILIB_DEPS_MODE STREQUAL "auto")
    duilib_link_dependency_order()         # add_dependencies 串起依赖
endif()
```

### 4.3 目标顺序与链接

- 第三方库目标名与库文件名保持一致（`duilib-png`、`duilib-zlib`、`duilib-webp`、
  `duilib-cximage`），`DUILIB_LIBS` 直接按目标名链接即可，CMake 会自动解析绝对路径
  并建立构建依赖，避免并行构建时先链接后生成库文件；
- `duilib`、`duilib-cximage`、`duilib-webp` 的目标名与库名一致，CMake 会自动解析；
  `libcef_dll_wrapper`（Linux 目标名）与链接名 `cef_dll_wrapper` 不同，需要显式依赖；
- 输出目录继续由 `cmake/duilib_lib_output.cmake` / `duilib_bin.cmake` 决定，
  根工程不覆盖，保证产物仍是 `lib/<arch>/<Config>` 与 `bin/`；
- **可执行文件与运行时统一输出到仓库根 `bin/`**：示例、CEF 运行时
  （`bin/libcef_win[_109]/<arch>`、`bin/libcef_linux`）、`resources/` 同目录，
  保证从 `bin/` 直接运行即可找到依赖；
- **构建树放在仓库根 `out/`**：按架构与配置分为 `out/<arch>/<Config>`
  （如 `out/x64/Release`、`out/arm64/Debug`），与 `build/` 平级；
  `build/` 中已纳入 git 的脚本与 `.sln` 不被污染，`build/build_temp` 仍只属于
  `build/*.sln` 的既有 VS 工程，根工程**不写入、不清理**；
  一次只配置/构建一个架构+配置组合，切换时不污染其他缓存；
  `--build-dir` / `-B` 可指向任意目录（含仓库外）；
- **`clean` 范围**：默认删除整个 `out/`（其中只有生成物，可安全清理），
  可用 `--build-dir` 收窄到单个架构/配置；**不删除**依赖缓存
  （`../skia`、`../SDL3`、`../cef_binary`）；`clean --deps` 才清理依赖的
  构建产物与下载缓存，源码 clone 默认保留，避免重复下载与全量重编。

### 4.4 对现有 cmake 模块的最小改动

1. `cmake/duilib_common.cmake`：新增
   `if(DUILIB_SKIA_AUTOBUILD)` 分支，跳过 Skia 库目录存在性校验，直接使用
   预期的变体路径；其余逻辑不变；
2. `cmake/duilib_common.cmake` 的 SDL 段：按配置选择
   `SDL3-static.lib` / `SDL3-staticd.lib`（MSVC）或 `SDL3` / `SDL3d`（MinGW），
   并把 `DUILIB_SDL_LIB_PATH` 的 `lib64`/`lib` 探测保持现状；
3. `duilib/CMakeLists.txt`、`examples/*/CMakeLists.txt` 无需改动（`project()` 可嵌套，
   `duilib_common` 的 `DUILIB_ROOT` 由 `CMAKE_CURRENT_LIST_DIR` 推导，仍正确）；
4. `cmake/duilib_root_options.cmake` 负责在 `add_subdirectory` 之前把
   `DUILIB_ENABLE_CEF/SDL`、`DUILIB_SKIA_REF`、`DUILIB_SDL_REF`、`DUILIB_CEF_VERSION`、
   `DUILIB_CEF_109`、`DUILIB_MD` 等写入缓存，同时读取
   `third_party_versions.json`，避免子项目 `option()` 默认值覆盖；
5. `cmake/duilib_common.cmake` 的 Windows CEF 路径按版本与架构选择：
   `DUILIB_CEF_SRC_ROOT_DIR` 跟随 `DUILIB_CEF_VERSION`（`libcef_win` / `libcef_win_109`），
   `DUILIB_CEF_LIB_PATH` 跟随 `DUILIB_TARGET_ARCH`（`Win32` / `x64` / `arm64`，三架构导入库均已提交），
   并校验 `libcef.lib` 存在；wrapper 名仍为 `libcef_dll_wrapper[_109]`。

### 4.5 平台差异

- **Linux / FreeBSD**：仅 x64 / arm64，其他架构在配置期报错；gcc 或 clang；
  Skia 变体 `llvm.*` / `gcc.*`；SDL 动态；CEF 仅 Linux（arm64 可用性以分发包为准）；
- **macOS**：仅 x64 / arm64（不提供 universal binary）；Xcode/Ninja；
  `CEF_ROOT` 必须在配置期存在 → 必须两阶段；CEF 示例输出 `.app`，签名仍由下游完成；
- **Windows MSVC**：x86 / x64 / arm64 三架构，**最低 VS 2022（MSVC v143）**，更低版本在
  配置期报错。推荐 Ninja + `vcvarsall`（`x86` / `x64` / `arm64`）单配置，
  保证 Skia 变体与 `DUILIB_BUILD_TYPE` 一致；VS 多配置生成器走
  `CMAKE_GENERATOR_PLATFORM`（P2 支持，需按 `$<CONFIG>` 选择 Skia 变体与运行库开关）；
  CEF 与 WebView2 在 arm64 上默认开启（运行时/模块已入库）；
  CEF 编译期用仓库 `libcef_win[_109]` 头文件，运行期由构建期下载到
  `bin/libcef_win[_109]/<arch>`；
- **MinGW-w64 / llvm-mingw**：x86 / x64（llvm-mingw 另支持 arm64），每架构独立构建目录；
  **CEF 默认 `OFF`**（`msvc` ABI 不兼容，显式开启会在预检阶段报错）；
- **MSYS2**：生成器固定 Ninja；`mingw64` / `ucrt64` / `clang64` 为 x64，
  `clangarm64` 为 arm64；gcc 与 clang 各自使用对应 Skia 变体，
  MINGW64 使用 `gcc.msvcrt.*`；CEF 默认 `OFF`。

### 4.6 CRT 与配置一致性

**适用范围**：只约束 **Skia 与 SDL**；CEF 是预编译运行时（DLL/动态库），
自带运行库，**不参与** `/MT` / `/MD` 校验，也不按 CRT 切换分发包。
libjpeg-turbo/WebView2 沿用仓库既有配置（见 3.4.4）。

| 平台 / 工具链 | Debug | Release | 说明 |
| :--- | :--- | :--- | :--- |
| Windows MSVC，`DUILIB_MD=OFF` | `/MTd` | `/MT` | Skia `extra_cflags`、SDL `MultiThreadedDebug`/`MultiThreaded`、duilib 静态运行库 props |
| Windows MSVC，`DUILIB_MD=ON` | `/MDd` | `/MD` | 同上改为 DLL 运行库；对应 `DuilibUseDynamicRuntime.bat` |
| Windows MinGW/MSYS2 | 同一前缀 | 同一前缀 | 无 `/MT`；msvcrt（`mingw64`）与 ucrt（`ucrt64`）不可混用，Debug/Release 必须由同一 `MSYSTEM` 构建 |
| Linux / macOS / FreeBSD | 系统 libc/libc++ | 同左 | 无 `/MT` 概念；Skia 变体统一 `is_debug=false` |

**一致性规则**：

1. 同一构建树内 `DUILIB_MD` 不可切换；切换视为不同配置，需使用新的构建目录；
2. Skia 的 `out/<变体>` 与 SDL 安装前缀各写一份 `.duilib-crt`
   （内容如 `msvc-mt` / `msvc-md` / `mingw64-msvcrt` / `ucrt64-ucrt` / `system`），
   配置期与当前 `DUILIB_MD`、`MSYSTEM`、编译器 triple 比对；
3. 不一致时报 `[ERROR] CRT mismatch`，`Hint:` 指向用相同 `DUILIB_MD` 重新执行
   `build.py deps`（或改用匹配的构建目录），而不是静默链接；
4. Windows 下 Debug 配置找不到 Debug 版 SDL 库时同样报错（旧手工包只有 Release），
   提示 `build.py deps` 生成 Debug 库；
5. `cmake/duilib_common.cmake` 按配置选择库名：MSVC `SDL3-static.lib` /
   `SDL3-staticd.lib`，MinGW `SDL3` / `SDL3d`；多配置生成器用 `$<CONFIG:Debug>`
   表达式，单配置用 `CMAKE_BUILD_TYPE`；
6. 渲染/窗口后端只要 CRT 与编译器一致即可，不额外校验 STL 版本（Skia 为 C++ 库，
   跨 CRT 混用会直接触发链接器 `LNK2038`，无需重复检测）。

## 5. CMakePresets 与使用方式

新增 `CMakePresets.json`（P1），按架构 × 配置拆分构建树：

```json
{
  "version": 6,
  "configurePresets": [
    { "name": "base", "hidden": true, "generator": "Ninja" },
    { "name": "x64-debug", "inherits": "base",
      "binaryDir": "${sourceDir}/out/x64/Debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "DUILIB_TARGET_ARCH": "x64" } },
    { "name": "x64-release", "inherits": "base",
      "binaryDir": "${sourceDir}/out/x64/Release",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "DUILIB_TARGET_ARCH": "x64" } },
    { "name": "x86-debug", "inherits": "base",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" },
      "binaryDir": "${sourceDir}/out/x86/Debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "DUILIB_TARGET_ARCH": "x86" } },
    { "name": "x86-release", "inherits": "base",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" },
      "binaryDir": "${sourceDir}/out/x86/Release",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "DUILIB_TARGET_ARCH": "x86" } },
    { "name": "arm64-debug", "inherits": "base",
      "binaryDir": "${sourceDir}/out/arm64/Debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "DUILIB_TARGET_ARCH": "arm64" } },
    { "name": "arm64-release", "inherits": "base",
      "binaryDir": "${sourceDir}/out/arm64/Release",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "DUILIB_TARGET_ARCH": "arm64" } }
  ],
  "buildPresets": [
    { "name": "x64-debug",     "configurePreset": "x64-debug" },
    { "name": "x64-release",   "configurePreset": "x64-release" },
    { "name": "x86-debug",     "configurePreset": "x86-debug" },
    { "name": "x86-release",   "configurePreset": "x86-release" },
    { "name": "arm64-debug",   "configurePreset": "arm64-debug" },
    { "name": "arm64-release", "configurePreset": "arm64-release" }
  ]
}
```

推荐命令（`build.py` 内部按同样规则选择 preset）：

```
# 推荐入口
python build/build.py --config Release          # 原生架构
python build/build.py --config Debug

# 直接使用 CMake（x64 Release，单命令模式）
cmake --preset x64-release -DDUILIB_DEPS_MODE=auto
cmake --build --preset x64-release

# macOS（CEF 必须在配置期就绪，使用两阶段）
cmake --preset x64-release -DDUILIB_DEPS_MODE=prepare
cmake --build out/x64/Release --target duilib_deps
cmake --preset x64-release -DDUILIB_DEPS_MODE=off
cmake --build --preset x64-release
```

- `binaryDir` 固定为 `out/<arch>/<Config>`；Presets 只提供结构性参数
  （生成器、`binaryDir`、`CMAKE_BUILD_TYPE`、`DUILIB_TARGET_ARCH`），
  `DUILIB_DEPS_MODE` 由命令行或 `build.py` 传入；
- 依赖版本、CEF/SDL 开关等业务默认值由 `build.py` 以 `-D` 传入，
  避免两处维护（见 10.4 第 19 条）；
- `x86` preset 仅 Windows 可用（`condition` 已限制）；交叉编译 arm64 时由
  preset 写入 `DUILIB_TARGET_ARCH=arm64`，其余工具链参数用 `--cmake-arg` 传入；
- 需要自定义目录（仓库外、其他工具链）时用 `-B <dir>` 覆盖；
- 根工程不写入 `build/build_temp`，该目录只属于 `build/*.sln` 的既有 VS 工程；
- 可执行文件与库仍由 `duilib_bin.cmake` / `duilib_lib_output.cmake` 输出到
  仓库根 `bin/` 与 `lib/`，因此从 `bin/` 运行即可找到 `resources/` 与 CEF 运行时。

## 6. 与现有脚本的关系

- 根工程成为构建逻辑的唯一实现；旧的 `build/*_build.sh|bat` 与
  `build_duilib_all_in_one.*` 在新入口完成全平台验证后**直接删除**，
  不保留兼容层（见 `Build-Entry-Simplification-Plan.md`）；
- 删除前，`all_in_one` 的依赖准备段先行替换为
  `cmake -DDUILIB_DEPS_MODE=prepare`，确认行为一致后再移除脚本；
- `build/autotest.py` 的预设（`unit`、`arm64-synthetic`、
  `pack-msvc-x64-release`）改为调用 `build.py` / 根工程；新增预设
  `root-cmake-release` 覆盖根工程参数组合；
- `build/pack.py` 仍只收集产物，不感知构建方式。

## 7. 验收标准

1. Linux（x64 / arm64）：干净环境只用 CMake 完成 Skia/SDL/CEF 准备与全量构建，
   `bin/<示例>` 可启动，`pack.py` 能出包（x64 Release 编译与出包已实测，
   2026-10-25；示例启动与 arm64 待验证）；
2. Windows MSVC（Ninja，x86 / x64 / arm64，VS 2022+）：仅需 `vcvarsall` + CMake，
   产物结构与 `msvc_build.bat` 一致；三架构默认构建 CEF 与 WebView2（arm64 导入库已入库）；CEF 由构建期按
   清单只下载 `Release/*` + `Resources/*` 到 `bin/`（109 / 142 可切换），
   下载包与仓库头文件版本不一致时报错，重复配置不重复下载；
   `--with-cef` 出包通过 `--verify`；
3. macOS（x64 / arm64）：两阶段构建出 `cef.app`，`Contents/Frameworks` 结构符合
   `Release.md` 的 macOS 章节；
4. FreeBSD（x64 / arm64）：配置与构建通过；x64 Release 已真机验证（编译与出包，
   2026-10-25，不含 CEF），arm64 待真机验证；
5. 架构校验：不支持的架构（如 Unix x86、Windows arm32）在配置期报错；
   Windows + Ninja 未显式指定 arm64 时给出明确提示而非静默产出 x64 产物；
6. 语言：`zh_CN` 环境（`LANG=zh_CN.UTF-8`、Windows 简体中文系统）下输出中文，
   `LC_ALL=C` / 英文系统下输出英文；Windows 中文模式需先切到 UTF-8（`chcp 65001`），
   否则自动回退英文；禁用翻译（`-DDUILIB_LANG=en`）时输出与英文系统一致；
7. 版本切换：修改 `third_party_versions.json` 的 `skia.default` / `sdl.default`
   （或用 `-DDUILIB_SKIA_REF` / `-DDUILIB_SDL_REF` 临时指定）后重新 `deps`，
   Skia/SDL 按 stamp 自动重建，构建无需其他改动；
8. 环境预检：在缺少 Python 3 / Git / 编译器 / VS / 系统库的干净机器上，一次配置即
   汇总列出全部缺失项与安装命令；`DUILIB_PREFLIGHT=OFF` 可跳过且无副作用；
9. 重复执行不触发 Skia 全量重编；断网时 `off` 模式可用；
10. `test_pack.py` / `test_autotest.py` 全绿；
11. rpath：Linux/macOS/FreeBSD 的示例可执行文件在系统已安装同名 SDL3 的环境下
    仍加载 `${DUILIB_SDL_ROOT}/lib` 的版本（`readelf -d` / `otool -L` / `ldd` 验证）；
    `DUILIB_SDL_RPATH=OFF` 时退回环境变量控制且可正常运行；
12. 下载：模拟断网/中断后重新执行 `deps` 能从断点续传；`DUILIB_DOWNLOAD_RETRIES`
    设为有限值时可观察到达到上限后报错；哈希不匹配时删除重下并在 3 次后失败；
13. 并发：两个构建目录同时执行 `deps` 时，第二个在 `DUILIB_DEPS_LOCK_TIMEOUT`
    内等待，超时后给出 `[ERROR]` 与 `Hint:`，不会同时写 `../skia`/`../SDL3`/`../cef_binary`；
14. 清单校验：`third_party_versions.json` 缺少 `default`、未知 ref 类型、CEF 缺 `file`
    时配置期报错；`sha256` 为空时 `[WARN]` 但流程可继续；`doctor` 打印的版本/架构/URL
    与实际下载一致；
15. Windows SDL 自动化：MSVC（x64）与 MSYS2（mingw64/ucrt64）下 `build.py deps`
    自动完成源码获取、Debug+Release 静态库构建与安装；`DUILIB_ENABLE_SDL=ON` 时
    两种配置均可链接并运行，不再依赖手工构建的 `../SDL3`；
16. CRT 一致性：`DUILIB_MD` 与 Debug/Release 组合下，Skia、SDL、duilib 使用同一
    运行库（`/MT`、`/MTd` 或 `/MD`、`/MDd`）；切换 `DUILIB_MD` 或在 Debug 下缺
    Debug SDL 库时报 `[ERROR] CRT mismatch` 并给出重跑 `deps` 的提示；
    **CEF 不参与 CRT 校验**；
17. 构建目录与清理：默认构建树为 `out/<arch>/<Config>`（如 `out/x64/Release`），
    与 `build/` 平级，`build/` 中已跟踪文件与 `build_temp` 不被根工程写入；
    `clean` 默认删除整个 `out/`，不删除 `../skia`、`../SDL3`、`../cef_binary`
    依赖缓存，`clean --deps` 才清理依赖产物；
18. 文档：`build-reference.md` 与 `build-reference.en.md` 内容对应、同步维护，
    命名与 `docs/` 下其他 `*.md` / `*.en.md` 一致；`build/build.md` 已删除且
    全仓库引用已更新；根 README 首屏只使用 `python build/build.py` /
    `--config Debug`；
19. `build.py` 语义：`--config Debug|Release` 一次只构建一个配置（默认 `Release`）；
    `build` 默认自动确保依赖就绪（`--no-deps` 跳过）；Debug/Release 分别调用一次
    即可得到两种配置的产物；`release.py` / `autotest.py` 不再单次构建两种配置；
20. Python 3：环境缺少 `python3`/`py -3` 时预检报 `[ERROR]`，并给出安装提示；
    不存在绕过 Python 的构建路径；
21. 测试：`python -m unittest build/test_build.py` 通过；中英文两种语言下断言一致；
    下载重试/哈希测试不访问网络；
22. SDK：根工程不提供 `install()` 目标；`pack.py` 产出的 `lib/cmake/nim_duilib`
    可被 `find_package(nim_duilib)` 使用；
23. 合规：`pack.py` 产物含 `LICENSES/` 与 `THIRD-PARTY-NOTICES.md`，列出的组件、
    版本、来源 URL 与清单一致。

## 8. 分阶段实施

- **P0（最小可用）**：根 `CMakeLists.txt` + `duilib_root_options.cmake`（含
  `DUILIB_TARGET_ARCH` 解析与校验、VS 2022+ 版本校验）+ 两阶段预检
  （`cmake -P` 工具/系统库检查 + `project()` 后编译器/架构检查，含 CMake/Python 3/Git/生成器）；
  仅支持 `DUILIB_DEPS_MODE=off`，Linux/Windows(MSVC x64) 走通；根工程与新增模块接入
  `duilib_i18n.cmake` 消息目录（en/zh_CN）；
- **P1（依赖自动化）**：`duilib_deps.cmake` 实现 Skia/SDL/CEF 准备、
  `third_party_versions.json` 驱动 Skia ref（commit/branch/tag）与 SDL ref（tag/branch）
  的切换与 stamp 判定、**Windows SDL 自动构建（MSVC/MinGW/MSYS2，Debug+Release，
  CRT 与 `DUILIB_MD` 对齐）**、`cmake/duilib_cef.cmake` + 同一清单实现 CEF 运行时
  下载/解压/复制（Windows 三架构、Linux、109/142 可切换，含仓库头文件版本匹配校验）、
  `prepare`/`auto` 模式、幂等与离线参数、`CMakePresets.json`；macOS 两阶段打通；
  预检补齐平台系统库与 `pacman`/发行版安装提示；把存量 cmake 模块的用户可见
  `message()` 迁入消息目录；
- **P2（收口）**：脚本改为调用根工程、多配置生成器支持、Windows x86 / arm64 与
  Linux/macOS/FreeBSD arm64 验证、SDK 仅由 `pack.py` 产出（不新增 `install()`）、
  根构建树统一为 `out/<arch>/<Config>` 并完善 `.gitignore`（整体忽略 `/out/`）、
  `build-reference.en.md` 双语文档、
  新增根工程自动化预设；`pack.py` / `release.py` / `autotest.py` 接入同一消息目录；
  `build/test_build.py`（unittest）与 `LICENSES/`、`THIRD-PARTY-NOTICES.md`。

## 9. 风险与开放问题

| 风险 | 影响 | 缓解 |
| :--- | :--- | :--- |
| 网络不可达（Skia 仓库大、CEF 走 CDN） | 首次构建失败 | 固定 commit/tag + `URL_HASH`、`DUILIB_CEF_ARCHIVE`、`DUILIB_DEPS_AUTO_CLONE=OFF` |
| 目标机器没有 Python 3 | Skia 的构建脚本无法运行，入口不可用 | Python 3 作为硬性前置；预检 `[ERROR]` + 安装提示；不提供绕过路径 |
| Skia 全量重编 | 构建时长急剧上升 | 固定 commit、补丁 stamp、跳过 update、Ninja 增量 |
| macOS CEF 配置期依赖 | 单命令模式不可用 | 明确两阶段，`prepare` 为默认推荐 |
| Visual Studio 多配置 | Skia 变体与 `/MT`、`/MD` 选择错误 | P0/P1 仅支持 Ninja 单配置；VS 多配置在 P2 补齐后再删除旧脚本 |
| Windows 手工 SDL 包只有 Release | Debug 链接 `/MT` 版 SDL 与 `/MTd` 主工程冲突（`LNK2038`） | 新流程 `deps` 自动构建 Debug 库；stamp 记录 CRT，缺失或失配时报错 |
| 根构建树使用 `out/<arch>/<Config>` | 与仓库内 `build/` 混淆；`.gitignore` 漏配导致误提交 | `out/` 整体忽略；`build/` 保持只放已跟踪脚本；README 明确“构建树在 `out/`，运行时在 `bin/`” |
| Windows + Ninja 交叉编译架构误判 | arm64 被当作 x64，链接到错误 Skia/JPEG | 强制显式 `DUILIB_TARGET_ARCH`，配置期校验 Skia 变体与指针宽度 |
| 多架构共用构建目录 / 依赖前缀 | 错误架构的库混入产物 | 每架构独立构建目录；`DUILIB_SDL_ROOT` 按架构分目录并在配置期校验 |
| 中文输出在非 UTF-8 控制台乱码 | 提示不可读 | 中文模式先 `chcp 65001`（`build.bat`/`build.py` 自动执行）；检测不到 UTF-8 时自动回退英文；`LC_ALL=C` 与英文 Windows 实测 |
| 预检误报（`pkg-config` 缺失、工具不在 PATH） | 正常环境被阻断 | 缺 `pkg-config` 时降级为 `[WARN]` 并给出手动安装提示；提供 `DUILIB_PREFLIGHT=OFF`；CI 同时覆盖“最小环境”与“完整环境” |
| CEF 清单配置错误（文件名/哈希） | 下载或解压失败 | `sha256` 首次接入后固定；`doctor` 打印解析出的版本/架构/URL；缺失的版本+架构组合在预检阶段报错 |
| CEF 上游删除旧版本包 | 109 无法重新下载 | 保留 `cef_binary/` 缓存与 `DUILIB_CEF_ARCHIVE` 离线包；必要时在清单中改用镜像 `url` |
| CEF Windows 包体积较大 | 首次构建耗时增加 | 只下载当前平台+架构；缓存命中（含哈希校验）直接复用；`.cef-version` 避免重复复制 |
| 与现有脚本逻辑漂移 | 行为不一致 | 根工程成为唯一实现，脚本按简化方案直接删除 |
| FreeBSD / arm64 依赖 | gn/字体库/Skia 变体差异 | 复用脚本参数，列为后续真机验证项 |
| SDL 使用 `main` 分支 | 版本漂移、构建不可复现 | 默认固定 `release-3.x.y` tag；`main` 仅显式选择；`.duilib-ref` 记录实际 ref |
| Skia ref 与补丁不配套 | 补丁应用失败或源码不一致 | 清单按 ref 绑定 `patchZip` 并校验哈希；失败时报错且不留下半成品 |
| 切换 ref 未触发重建 | 继续使用旧二进制 | `.duilib-ref` / `.duilib-patch` stamp 不匹配即重新 `gn gen` + `ninja`；`deps --force` 可强制 |

## 10. 复盘：待补充事项

以下按“是否影响实现正确性”排序。**P0（1～6 项）、P1（7～12 项）与 P2 的
13～25 项均已决策**（其中 21 CI 暂缓，待后续单独评估）。

### 10.1 已决策（2026-10-25）

1. **CEF 落盘位置**：`duilib/third_party/libcef` 中的头文件、wrapper、CMake 脚本已由 git 管理，
   构建系统**不修改**；自动下载只把 `Release/*`、`Resources/*` 复制到
   `bin/libcef_win[_109]/<arch>`（Windows）与 `bin/libcef_linux`（Linux）。
   编译期使用仓库头文件；下载包 `include/cef_version.h` 与仓库同版本头文件的
   `CEF_VERSION` 不一致时立即报错。
2. **MinGW / MSYS2 的 CEF**：默认 `OFF`；显式开启时在预检阶段报错并说明 MSVC ABI 不兼容。
3. **预检拆分**：阶段 A 用 `cmake -P`（无编译器）检查工具/系统库，阶段 B 在 `project()`
   后检查编译器/架构/CRT；`doctor` 在无 CMake 时回退 Python 内置探测。
4. **Debug 与 CEF**：只使用 Release 的 `libcef.lib`/运行时；需要 Debug CEF 的用户手工部署。
5. **Windows ARM64 探测**：同时参考 `PROCESSOR_ARCHITEW6432` 与
   `CMAKE_HOST_SYSTEM_PROCESSOR`，避免把 arm64 主机识别成 x64。
6. **Presets**：补 `binaryDir` 与 `buildPresets`；`binaryDir` 按架构 × 配置区分
   （`out/<arch>/<Config>`）。
7. **运行时输出**：可执行文件、`resources/`、CEF 运行时统一在仓库根 `bin/`；
   CMake 构建树放在 `out/<arch>/<Config>`，不放进 `bin/`，也不放进 `build/`。

### 10.2 已确认（2026-10-25）

- **Windows arm64 的 CEF 导入库**：已提交 `libcef_win[_109]/lib/arm64/libcef.lib`，
  `.gitignore` 已调整；三架构统一链接仓库内导入库，下载包只提供运行时。
- **`binaryDir` 语义**：以“可执行文件/运行时输出目录 = 仓库根 `bin/`”为准。
  构建树默认 `out/<arch>/<Config>`（如 `out/x64/Release`），与 `build/` 平级；
  `build.py --build-dir <dir>` 或 Presets 覆盖可放到仓库外。

### 10.3 P1：依赖与工具链细节（已写入正文）

7. **Skia gn 参数表**：已在 3.2 给出“公共参数 + 平台 × 编译器 × CRT”矩阵，
   明确 Windows debug/release 差异仅为 CRT（`/MT`、`/MTd`、`/MD`、`/MDd`），
   `DUILIB_LLVM_ROOT` 可覆盖 `clang_win`；**不调用 `tools/git-sync-deps`**，
   预检只校验 `third_party/externals` 关键目录存在。
8. **SDL 构建与运行时**：已在 3.3 给出参数表——Windows/MinGW/MSYS2 静态（无 DLL
   复制问题）、Unix 动态；X11/Wayland/音频保持上游默认；`DUILIB_SDL_RPATH=ON`
   写入 `${DUILIB_SDL_ROOT}/lib` 绝对 rpath（FreeBSD 追加 `/usr/local/lib`），
   避免加载系统同名 SDL；验证方式为 `readelf -d` / `otool -L` / `ldd`。
9. **WebView2 / libjpeg-turbo**：已在 3.4.4 明确**保持现状**——WebView2 由维护者
   手工升级并提交 git，不下载、不纳入清单、不新增自动化变量；libjpeg-turbo 以仓库
   已提交的预编译包为准，架构与 `/MD`、`/MT` 匹配沿用现有 props/CMake 逻辑。
10. **下载健壮性**：已在 3.5 落地——不处理代理（继承 `http_proxy`/`https_proxy`）；
    `DUILIB_DOWNLOAD_RETRIES=0` 默认无限重试、可改有限次数；`curl --continue-at -`
    断点续传；连接超时与指数退避；哈希不匹配删除重下最多 3 次；镜像走清单
    `url`/`DUILIB_CEF_URL_BASE`；`doctor` 检查磁盘与长路径；非 ASCII 路径不转码。
11. **共享依赖并发**：已在 3.5 采用锁文件方案——
    `${DUILIB_DEPS_ROOT}/.duilib_deps.lock` + `file(LOCK ... GUARD PROCESS)`，
    默认等待 5 秒（`DUILIB_DEPS_LOCK_TIMEOUT`），超时报错并提示等待或删除陈旧锁；
    锁不覆盖主工程编译，`deps --force` 不绕过锁。
12. **版本清单校验与升级流程**：已在 3.6 给出 schema 必填项、`default` 必须存在于
    `refs`/`versions`、ref 类型限制、`patchZip` 规则、CEF `file` 必填与 `sha256`
    为空时的 `[WARN]` 策略、`doctor` 打印解析结果，以及“新增条目 → `deps --force` →
    补哈希 → 构建示例 → 出包/测试 → 改 `default` → 回滚”的 7 步流程。

### 10.4 P1/P2：构建产物与下游

13. **CRT 与配置矩阵**（已决策）：**CEF 无 CRT 约束**，不参与校验；Skia 与 SDL
    必须与 nim_duilib 一致。矩阵与校验规则见 4.6（`/MT`、`/MTd` 或 `/MD`、`/MDd`；
    MinGW 按 msvcrt/ucrt 前缀；stamp `.duilib-crt` 在配置期比对）。
    **补充能力**：Windows 的 SDL 自动构建流程已写入 3.3（MSVC/MinGW/MSYS2，
    Debug+Release 静态库，`CMAKE_MSVC_RUNTIME_LIBRARY` 跟随 `DUILIB_MD`）。
14. **`build.py` 配置语义**（已决策）：用 `--config Debug|Release`，**一次只构建
    一个配置**，默认 `Release`；不再提供 `--debug` 别名（文档统一写 `--config`）。
    默认构建示例（`--no-examples` 可关）、`--target` 透传；`--jobs` 默认 CPU 核数；
    `--force` 只属于 `deps`；`--offline` 只用缓存；`--dry-run` 只打印命令；
    退出码 `0` 成功 / `1` 构建或预检失败 / `2` 参数错误。`build` 默认自动确保依赖
    就绪（stamp 命中即跳过），`--no-deps` 跳过。
15. **构建目录与清理**（已决策）：构建树放在仓库根 `out/<arch>/<Config>`，与 `build/`
    平级，`build/` 中已跟踪的脚本与 `.sln` 不受污染；`build/build_temp` 仅供
    `build/*.sln` 的既有 VS 工程使用，根工程不写入、不清理。`clean` 默认删除整个
    `out/`（只含生成物），可用 `--build-dir` 收窄；依赖缓存默认保留，
    `clean --deps` 才清理依赖产物（见 4.3、5）。
16. **macOS 两阶段自动化**（已决策）：`build.py` 在配置前**自动**确保依赖就绪
    （全平台统一，macOS 不再要求手动两步）；`--no-deps` 供已手工准备好的用户跳过；
    直接使用 CMake 时仍走两阶段：`-DDUILIB_DEPS_MODE=prepare` 构建
    `duilib_deps`，再以 `off` 重新配置。
17. **`install()` / export 与 SDK 布局**（已决策）：**不新增 `install()`**，
    SDK 只由 `pack.py` 产出（`include/`、`lib/<arch>/<Config>`、`bin/`、
    `lib/cmake/nim_duilib/`），避免根 CMake 与 `pack.py` 两套布局各说各话；
    `find_package(nim_duilib)` 的兼容性以 `pack.py` 产物为准，后续如需
    `make install` 再单独设计。
18. **`pack.py` / `autotest.py` 与新产物的映射**（已决策）：产物布局保持不变
    （`lib/<arch>/<Config>`、`bin/`、CEF 运行时目录），只替换脚本中的构建命令；
    `Release+Debug` 不再一次完成，改为分别调用
    `build.py --config Debug` 与 `build.py --config Release` 各一次；
    新增 `x64-release`（及对应 arm64）预设并验证 `pack --verify`。
19. **Presets 与 `build.py` 的唯一事实源**（已决策）：Presets 负责结构性参数
    （生成器、`binaryDir`、`CMAKE_BUILD_TYPE`、`DUILIB_TARGET_ARCH`）；
    业务默认值（依赖版本、CEF/SDL 开关、`DUILIB_DEPS_MODE`）由 `build.py` 以 `-D`
    传入；优先级为 `CLI > 环境变量 > build.py 默认 > CMake 默认`。文档明确
    “推荐入口是 `build.py`，Presets 是直接使用 CMake 的便捷入口”。

### 10.5 P2：测试、CI、文档与合规

20. **测试**（已决策）：用 Python 标准库 `unittest`，**不引入 pytest**；
    新增 `build/test_build.py` 覆盖参数解析、`--config` 默认值、语言选择、
    清单校验、依赖就绪判定与预检模拟；中英文测试只断言标记、退出码与关键字；
    下载相关只测重试/哈希逻辑（假 curl），不访问真实网络。
21. **CI 矩阵**（暂缓）：Windows/Linux/macOS/FreeBSD、架构与缓存策略留待后续
    单独评估；当前以本地 `build.py test` + `autotest.py` 验证，不阻塞本次设计。
22. **Python 依赖**（已决策）：**Python 3 是硬性要求**（Skia 的构建流程依赖
    `python3`），不提供无 Python 的备用入口；`build.bat` 依次尝试 `py -3` /
    `python3`，都找不到时报 `[ERROR]` 并给出安装提示。旧 `.bat`/`.sh` 编译脚本
    全部删除，不保留兼容路径。
23. **文档**（已决策）：`build-reference.md` + `build-reference.en.md`（命名同
    `docs/` 下其他 `*.en.md`）；`build/build.md` **删除**并更新全部引用
    （根 README、WORKBUDDY、TRAE、CLAUDE、Release 文档等）；`build/README.md` /
    `.en.md` 保留为“各平台前置依赖 + 发布打包 + 排障索引”，删除旧脚本对照表并
    指向 `build-reference.md`；根 README 快速开始只用
    `python build/build.py`（默认 Release）与 `python build/build.py --config Debug`，
    不再出现 `--debug` 或 `build` 子命令形式。
24. **合规**（已决策）：`pack.py` 随包生成 `LICENSES/` 与
    `THIRD-PARTY-NOTICES.md`，收集 CEF / SDL / Skia / libjpeg-turbo / WebView2
    的许可证与版权声明，并标注来源 URL 与版本；仅在发布包中生效，不影响构建。
25. **`.gitignore` 与本地状态**（已决策）：构建树统一在仓库根 `out/`，
    `.gitignore` 直接忽略 `/out/`；根目录 `CMakeUserPresets.json` 一并忽略；
    `build/` 中已跟踪的脚本与 `.sln` 不受影响，`build/build_temp` 继续由现有
    VS 工程使用。另需审计全局 `arm64/` 忽略规则，改为精确路径，避免误伤
    `duilib/third_party/libcef/**/arm64` 等源码目录。
