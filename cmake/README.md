简体中文 | [English](README.en.md)

# nim_duilib CMake 工程配置说明

本文档详细介绍 `nim_duilib\cmake` 目录中 CMake 配置文件（*.cmake）的用途及使用方法，供维护人员参考。

## 目录文件概览

```
cmake/
├── README.md / README.en.md        # 本文件（目录说明、开关汇总与排障）
├── third_party_versions.json       # Skia / SDL3 / CEF 版本清单（唯一维护点）
├── i18n/messages.json              # 构建消息目录（中英文，唯一维护点）
├── macos/Info.plist.in             # macOS App Bundle 的 Info.plist 模板
│
├─ 公共层（库子项目、示例与根工程共用）
│  ├── duilib_common.cmake                 # 公共配置（操作系统/编译器/CPU检测，路径设置）
│  ├── duilib_bin.cmake                    # 可执行程序基础配置（C++20 标准、路径与输出目录）
│  ├── duilib_bin_windows.cmake            # Windows 平台特定配置（运行库、manifest、WebView2）
│  ├── duilib_bin_linux.cmake              # Linux 平台特定配置
│  ├── duilib_bin_macos.cmake              # macOS 平台特定配置（框架与 App Bundle 入口）
│  ├── duilib_bin_freebsd.cmake            # FreeBSD 平台特定配置
│  ├── duilib_compiles.cmake               # 编译选项检测（AVX/AVX2）
│  ├── duilib_compiler_flags.cmake         # GCC/Clang 公共编译参数（Unix 库与示例共用）
│  ├── duilib_unix_font_libs.cmake         # Linux/FreeBSD 字体依赖（Freetype、Fontconfig）
│  ├── duilib_lib_output.cmake             # 库输出目录规则（lib/<arch>/<Config>，配置只由目录表达）
│  ├── duilib_log_variables.cmake          # DUILIB_LOG 变量打印（由 duilib_common.cmake 末尾包含）
│  ├── duilib_example.cmake                # 示例公共实现：示例只需「声明开关 + include 一次」
│  ├── duilib_write_build_stamp.cmake      # POST_BUILD 写 bin/.build-config/<目标>（识别配置切换）
│  ├── duilib_cef_macos.cmake              # macOS CEF 特殊配置（Framework、Helper、资源、rpath）
│  ├── duilib_macos_bundle.cmake           # macOS 普通示例的 App Bundle 配置
│  ├── duilib_copy_bundle_resources.cmake  # App Bundle 资源按清单增量同步（每次构建执行）
│  └── duilib_copy_sdl_bundle.cmake        # 复制 SDL3 动态库到 Contents/Frameworks 并设置 rpath
│
├─ 根工程编排（仓库根 CMakeLists.txt 使用）
│  ├── duilib_i18n.cmake                   # 语言解析与消息目录（include 与 cmake -P 均可用）
│  ├── duilib_bootstrap.cmake              # 工具自举：PATH 上没有 ninja 时取用 VS 目录或补丁归档
│  ├── duilib_root_options.cmake           # 平台/编译器/架构识别与统一缓存选项（project() 之后）
│  ├── duilib_preflight_tools.cmake        # 阶段 A 预检（无编译器）：检查工具与系统库，不改文件
│  ├── duilib_preflight.cmake              # 阶段 B 预检（有编译器）：确定性的工具链/架构校验
│  ├── duilib_deps.cmake                   # 依赖检查与准备目标（DUILIB_DEPS_MODE=off/prepare/auto）
│  ├── duilib_deps_run.cmake               # 依赖准备脚本（cmake -P）：Skia/SDL3/CEF 下载、构建与安装
│  ├── duilib_download.cmake               # 下载工具（curl 优先、断点续传与重试，回退 file(DOWNLOAD)）
│  └── duilib_cef.cmake                    # CEF 运行时下载/校验/解压/复制（Windows 与 Linux 共用）
│
└─ 目录名规则（依赖准备、配置期解析、off 模式检查与打包共用）
   ├── duilib_skia_variants.cmake         # Skia 变体目录名唯一规则
   ├── duilib_skia_toolchain.cmake        # Skia 编译工具链选择 + duilib_detect_msys2()（MSYS2 判定）
   ├── duilib_sdl_variants.cmake          # SDL3 变体前缀唯一规则（架构 × 配置 × 运行库）
   └── duilib_crt.cmake                   # CRT 标识唯一推导（msvc-md / msvc-mt / <env>[-msvcrt] / system）
```

最后一组里，`duilib_skia_variants.cmake`、`duilib_sdl_variants.cmake`、
`duilib_crt.cmake` 这三个「目录名规则」文件是**唯一实现点**：配置期解析
（`duilib_common.cmake`、`duilib_root_options.cmake`）与依赖检查/准备
（`duilib_deps*.cmake`）都必须复用它们，禁止在其他 CMake 文件里再拼目录名；
`duilib_skia_toolchain.cmake` 负责 Skia 编译工具链选择和 `duilib_detect_msys2()`
（MSYS2 与独立 MinGW-w64 判定），同样只有这一处实现。
打包侧（`build/pack.py`）与 VS 属性表（`msvc/PropertySheets/*.props`）无法直接
include CMake 脚本，只能按同一套规则实现；`build/tests/skia_variants_check.cmake`
（`cmake -P` 夹具）与 `build/test_cmake_scripts.py`（Python 侧实现）用同一组用例
逐项比对，防止两侧再次漂移。

---

## 配置文件详解

### 1. duilib_common.cmake（公共配置）

**功能：** 定义跨平台的公共配置变量和开关选项，是所有 CMake 工程的基础配置文件。

**主要检测内容：**

#### 1.1 操作系统检测
```cmake
DUILIB_OS_WINDOWS  # Windows 系统
DUILIB_OS_LINUX    # Linux 系统
DUILIB_OS_MACOS    # macOS 系统
DUILIB_OS_FREEBSD  # FreeBSD 系统
```

#### 1.2 编译器检测
```cmake
DUILIB_COMPILER_MSVC    # MSVC 编译器
DUILIB_COMPILER_GCC     # GCC 编译器
DUILIB_COMPILER_LLVM    # Clang/LLVM 编译器
DUILIB_COMPILER_NAME    # 编译器名称（如 "msvc", "mingw64-gcc", "mingw64-llvm"；MSYS2 为 "gcc", "llvm"）
```

#### 1.3 CPU 架构检测
```cmake
DUILIB_SYSTEM_PROCESSOR  # 处理器类型：x86, x64, arm32, arm64
DUILIB_BITS_64           # 是否 64 位系统
```

#### 1.4 编译类型检测
```cmake
DUILIB_BUILD_TYPE        # "debug" 或 "release"
```

**主要开关选项：**

| 选项名称 | 默认值 | 说明 |
|----------|--------|------|
| `DUILIB_LOG` | OFF | 打印 duilib 调试日志 |
| `DUILIB_SKIA_LIB_SUBPATH` | 空 | Skia 库子目录；空表示按平台/配置自动拼接（见下方规则） |
| `DUILIB_ENABLE_SDL` | Windows=OFF, 其他=ON | 启用 SDL 输入支持 |
| `DUILIB_SDL_AUTOBUILD` | OFF | 依赖自动准备模式（`DUILIB_DEPS_MODE=auto`）：跳过 SDL3 存在性校验，按变体规则预判路径 |
| `DUILIB_ENABLE_CEF` | Windows MSVC/Linux/macOS=ON，MinGW/FreeBSD=OFF | 启用 CEF 浏览器支持（库编译 CEF 控件源码）；不支持的平台会被强制关闭并告警 |
| `DUILIB_EXAMPLE_CEF` | OFF | 示例程序链接 CEF（仅 `cef`/`CefBrowser` 设置） |
| `DUILIB_CEF_109` | OFF | 使用 CEF 109 版本（支持 Win7）；已弃用，等价于 `-DDUILIB_CEF_VERSION=109`，设置时会打印告警 |
| `DUILIB_WEBVIEW2_EXE` | OFF | WebView2 可执行程序（仅 Windows） |
| `DUILIB_MINGW_STATIC` | ON | MinGW 静态链接 |

**主要路径变量：**

| 变量名 | 说明 |
|--------|------|
| `DUILIB_ROOT` | duilib 源码根目录 |
| `DUILIB_LIB_PATH` | duilib 库输出目录 |
| `DUILIB_BIN_PATH` | duilib 可执行程序输出目录 |
| `DUILIB_SKIA_SRC_ROOT_DIR` | Skia 源码目录 |
| `DUILIB_SKIA_LIB_PATH` | Skia 库文件目录 |
| `DUILIB_SDL_ROOT` | SDL3 安装根目录；Windows 下按 `<根>/<变体>` 选择前缀，显式传入的值仍作为根与单前缀回退 |
| `DUILIB_SDL_INCLUDE_DIR` | SDL3 头文件目录（取实际选中的变体前缀下的 `include/`） |
| `DUILIB_SDL_SRC_ROOT_DIR` | SDL3 安装前缀目录（由实际选中的变体前缀推导） |
| `DUILIB_SDL_LIB_PATH` | SDL3 库文件目录（多配置生成器下为按配置切换的生成器表达式） |
| `DUILIB_CEF_SRC_ROOT_DIR` | libCEF 源码目录 |
| `DUILIB_CEF_LIB_PATH` | libCEF 库文件目录 |

**主要库列表变量：**

| 变量名 | 说明 |
|--------|------|
| `DUILIB_LIBS` | duilib 基础库列表 |
| `DUILIB_SKIA_LIBS` | Skia 库列表（svg, skshaper, skottie, sksg, jsonreader, skia） |
| `DUILIB_SDL_LIBS` | SDL3 库列表 |
| `DUILIB_CEF_LIBS` | libCEF 库列表 |

**Skia 路径拼接规则：**

`DUILIB_SKIA_LIB_SUBPATH` 默认**为空**，由 `duilib_common.cmake` 按平台与配置自动拼接，各工程无需再自行设置：

```cmake
# 默认值：空（不再预先写死 llvm.<arch>.<构建类型>）
# 外部用 -DDUILIB_SKIA_LIB_SUBPATH=... 传入时以外部值为准（固定路径，不随配置变化）。

# 子目录名为空时，自动规则（解析结果必须真实存在，否则配置阶段直接报错；
# MinGW 构建还要求目录内存在 *.a 静态库，避免把同名目录中的 MSVC .lib 误判为可用库）：
#   Windows MSVC / clang-cl : llvm.<CPU_ARCH>.debug / llvm.<CPU_ARCH>.release
#             Debug 必须匹配 debug 变体（Debug 版本无法链接 release Skia），缺失直接报错
#   Windows MinGW gcc       : mingw64-gcc.<CPU_ARCH>.debug / .release（MSYS2 为 gcc.<arch>.*）
#             Debug 变体缺失时回退同编译器的 release 变体；
#             gcc 与 llvm 的 Skia 不能混用（libstdc++/libc++ 运行时不兼容，链接会失败）
#   Windows MinGW llvm      : mingw64-llvm.<CPU_ARCH>.debug / .release（MSYS2 为 llvm.<arch>.*）
#             Debug 变体缺失时回退 release 变体
#   Linux/macOS/FreeBSD : <编译器>.<CPU_ARCH>.release
#             Skia 在 Unix 只编译 release 变体（D-3）

# MSYS2 与独立 MinGW-w64 按编译器安装位置区分（MSYS2 安装根目录下存在 usr/bin/pacman.exe），
# 不使用 MSYSTEM 变量（Git Bash 也会设置 MSYSTEM=MINGW64）；
# 本工具链的前缀优先，另一组前缀作为回退（只有一组 Skia 时仍可构建）。

# 最终 Skia 库目录：
${DUILIB_SKIA_SRC_ROOT_DIR}/out/<子目录名>

# 示例：
# Windows x64 + Debug          -> skia/out/llvm.x64.debug
# Windows x64 + Release        -> skia/out/llvm.x64.release
# Windows -A Win32 + Debug     -> skia/out/llvm.x86.debug
# MinGW gcc + Debug（无 gcc debug） -> skia/out/mingw64-gcc.x64.release
# MinGW gcc + Debug（无 gcc Skia）  -> 配置期报错（不能链接 llvm 版 Skia）
# Linux/macOS/FreeBSD          -> skia/out/llvm.x64.release（clang）
#                                skia/out/gcc.x64.release（gcc）
# MSYS2 Windows + Debug        -> skia/out/gcc.<arch>.debug 优先，缺失时回退 gcc.<arch>.release
# 外部指定 -DDUILIB_SKIA_LIB_SUBPATH=llvm.x64.release -> 两个配置都固定用该目录（同样校验存在性）
# MinGW 指向只有 .lib 的 MSVC 目录（如 llvm.x64.release 的 MSVC 产物） -> 配置期报错
```

**SDL3 安装前缀拼接规则：**

Windows 上 SDL3 的静态库必须按“架构 × 配置 × MSVC 运行库”分开存放，否则 Debug/Release
同名覆盖、`/MT` 与 `/MD` 混链报 LNK2038。`duilib_deps` 把每种组合安装到自己的完整前缀
（`<deps-root>/SDL3/<变体>`，含 `include/`、`lib/`、`share/licenses`），规则集中在
[duilib_sdl_variants.cmake](duilib_sdl_variants.cmake)，`duilib_common.cmake`、依赖检查、
`build/pack.py` 与 VS 属性表都复用同一套命名：

```cmake
#   Windows MSVC  : msvc.<arch>.debug / msvc.<arch>.release
#                   /MD、/MDd（DUILIB_MD，DLL 模式隐含）时追加 .mdd / .md
#   Windows MinGW : 复用 Skia 的 release 变体名（mingw64-<gcc|llvm>.<arch>.release 等），
#                   Debug/Release 共用同一份静态库（CRT 由工具链决定）
#   非 Windows    : <deps-root>/SDL3 单前缀（只有 Release 共享库）

# 候选顺序：<root>/<变体> 优先，<root> 作为旧布局或自备单前缀的回退；
# build.py deps 按上表安装，pack.py 默认也按同一顺序选择；
# 显式 --sdl-dir 视为单前缀（不派生变体目录）
```

---

### 2. duilib_bin.cmake（可执行程序基础配置）

**功能：** 可执行程序的通用 CMake 配置，适用于所有平台。

**主要配置内容：**

#### 2.1 C++ 标准设置
```cmake
set(CMAKE_CXX_STANDARD 20)             # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # 强制要求 C++20
```

#### 2.2 路径配置
```cmake
include_directories(${DUILIB_ROOT})              # duilib 根目录
include_directories(${DUILIB_PROJECT_SRC_DIR})   # 项目源码目录
link_directories("${DUILIB_LIB_PATH}")           # duilib 库目录
link_directories("${DUILIB_SKIA_LIB_PATH}")      # Skia 库目录
link_directories("${DUILIB_SDL_LIB_PATH}")       # SDL 库目录（如果启用）
```

#### 2.3 输出目录
```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_BIN_PATH}")
```

**库输出目录（CM-5 / D-2 方案 B）：**

各库在 `project()` 之后 `include` [duilib_lib_output.cmake](duilib_lib_output.cmake)，
统一按平台与配置输出，配置只由目录表达（库名不再带 `_d`）：

| 平台 | 库目录 |
| :--- | :--- |
| Windows | `<root>/lib/<x64\|x86\|arm64>/<Debug\|Release\|RelWithDebInfo\|MinSizeRel>/` |
| Linux / macOS / FreeBSD | `<root>/lib/<Debug\|Release\|RelWithDebInfo\|MinSizeRel>/` |

- Windows 的 arch 由 `CMAKE_GENERATOR_PLATFORM` 推导（`Win32 → x86`，缺省 x64）。
- `DUILIB_LIB_PATH` 为 `${DUILIB_LIB_BASE}/$<CONFIG>`，由生成器展开。
- 同一 `<arch>|<Config>` 约定一次只由一套构建体系（CMake 或 VS）完整构建，后构建者覆盖前者。

#### 2.4 源码收集
```cmake
aux_source_directory(${DUILIB_PROJECT_SRC_DIR} SRC_FILES)
# 支持子目录：DUILIB_SRC_SUB_DIRS 变量
```

#### 2.5 平台分发
自动根据操作系统包含对应的平台配置文件：
```cmake
if(DUILIB_OS_WINDOWS)
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_windows.cmake")
elseif(DUILIB_OS_LINUX)
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_linux.cmake")
elseif(DUILIB_OS_MACOS)
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_macos.cmake")
elseif(DUILIB_OS_FREEBSD)
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_freebsd.cmake")
endif()
```

---

### 3. duilib_bin_windows.cmake（Windows 平台配置）

**功能：** Windows 平台特有的编译和链接配置。

**主要配置内容：**

#### 3.1 MSVC 运行库配置
```cmake
if(DUILIB_MD)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")  # MD/MDd
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")     # MT/MTd
endif()
```

#### 3.2 MSVC 编译选项
```cmake
add_compile_options("/utf-8")                      # UTF-8 源码编码
add_compile_options($<$<COMPILE_LANGUAGE:C>:/MP${CPU_CORES}>)   # 多核编译
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP${CPU_CORES}>)
```

#### 3.3 Unicode 编码
```cmake
add_definitions(-DUNICODE -D_UNICODE)
```

#### 3.4 MinGW-w64 特殊处理
```cmake
if(DUILIB_MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows ${CMAKE_EXE_LINKER_FLAGS}")    # Windows 程序
    if(DUILIB_MINGW_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")  # 静态链接
    endif()
endif()
```

#### 3.5 Manifest 文件配置
```cmake
if(DUILIB_TARGET_ARCH_RESOLVED STREQUAL "arm64")
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.arm64.manifest")
elseif(DUILIB_BITS_64)
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x64.manifest")
else()
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x86.manifest")
endif()
```

#### 3.6 MSVC 子系统设置
```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
```

#### 3.7 CEF 延迟加载
```cmake
# 只有 CEF 示例链接 libcef，因此也只为它们设置延迟加载
if(DUILIB_EXAMPLE_CEF)
    target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:libcef.dll")
endif()
```

#### 3.8 WebView2 支持
```cmake
if(DUILIB_WEBVIEW2_EXE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_WEBVIEW2=1)
    # 自动复制 WebView2Loader.dll
endif()
```

#### 3.9 Windows 系统依赖库
```cmake
set(DUILIB_WINDOWS_LIBS Comctl32 Imm32 Opengl32 User32 shlwapi)
# 可选：Version.lib Winmm.lib Setupapi.lib（SDL 依赖）
```

---

### 4. duilib_bin_linux.cmake（Linux 平台配置）

**功能：** Linux 平台特有的编译和链接配置。

**主要配置内容：**

```cmake
# 仅 CEF 示例链接 CEF（cef/CefBrowser）；其他示例不应产生 libcef.so 依赖
set(DUILIB_EXAMPLE_CEF_LIBS "")
if(DUILIB_ENABLE_CEF AND DUILIB_EXAMPLE_CEF)
    list(APPEND DUILIB_TARGET_INCLUDE_DIRS "${DUILIB_CEF_SRC_ROOT_DIR}")
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_CEF_LIB_PATH}")
    set(DUILIB_EXAMPLE_CEF_LIBS ${DUILIB_CEF_LIBS})
endif()

# Linux 系统依赖库
set(DUILIB_LINUX_LIBS X11 freetype fontconfig pthread dl)

# 链接命令
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_EXAMPLE_CEF_LIBS} ${DUILIB_LINUX_LIBS})
```

---

### 5. duilib_bin_macos.cmake（macOS 平台配置）

**功能：** macOS 平台特有的编译和链接配置。

**主要配置内容：**

#### 5.1 系统框架查找
```cmake
find_library(ACCELERATE Accelerate)
find_library(COREFOUNDATION CoreFoundation)
find_library(CORETEXT CoreText)
find_library(COREGRAPHICS CoreGraphics)
```

#### 5.2 编译器参数设置
```cmake
set(DUILIB_COMPILER_FLAGS
    -fno-strict-aliasing
    -fstack-protector
    -funwind-tables
    -fvisibility=hidden
    -Wall
    -Wextra
    # ... 更多参数
)

set(DUILIB_CXX_COMPILER_FLAGS
    -fno-threadsafe-statics
    -fvisibility-inlines-hidden
    -frtti
    # ... 更多参数
)
```

#### 5.3 链接命令
```cmake
target_link_libraries(${PROJECT_NAME}
    ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS}
    ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
    "-framework AppKit" "-framework Foundation" "-framework Metal" "-framework Cocoa"
)
```

#### 5.4 普通示例的 App Bundle

`duilib_macos_bundle.cmake` 在示例设置 `DUILIB_EXAMPLE_MACOS_BUNDLE=ON` 时被调用，
把目标创建为 `MACOSX_BUNDLE`，并完成：

- `Info.plist`（模板 `cmake/macos/Info.plist.in`）、Bundle 标识与版本号；
- `bin/resources` → `Contents/Resources/duilib/resources`（与普通程序的
  `<exe 目录>/resources/` 同构，`GetResourceRootPath(true)` 在 macOS 下定位该目录，
  不需要改 C++ 代码）；资源清单按示例名（`project()` 名称）自动推导，示例无需配置；
  目标名与资源目录名不一致（如 `ChildWindow`→`child_window`）时也可显式设置
  `DUILIB_THEME_DIR_NAME`，显式值优先且会校验目录存在；需要覆盖清单时设置
  `DUILIB_MACOS_BUNDLE_RESOURCES`；
- SDL3 动态库 → `Contents/Frameworks`，rpath 设为
  `@executable_path/../Frameworks`；
- 切换 Bundle/普通程序时清理残留的另一种产物。

目标列表也可以用根选项 `DUILIB_MACOS_BUNDLE_EXAMPLES` 指定。完整说明见
[build/macos-app-bundle.md](../build/macos-app-bundle.md)。

---

### 6. duilib_bin_freebsd.cmake（FreeBSD 平台配置）

**功能：** FreeBSD 平台特有的编译和链接配置。

**主要配置内容：**

```cmake
# FreeBSD 系统依赖库
set(DUILIB_FREEBSD_LIBS pthread dl)

find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(X11 REQUIRED)

target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_FREEBSD_LIBS} ${X11_LIBRARIES} Freetype::Freetype Fontconfig::Fontconfig)
```

---

### 7. duilib_compiles.cmake（编译选项检测）

**功能：** 检测目标平台的 CPU 特性支持（如 AVX/AVX2 指令集）。

**检测内容：**

```cmake
# AVX 支持检测
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256 a = _mm256_set1_ps(0.0f);
        return 0;
    }
" DUILIB_HAVE_AVX)

# AVX2 支持检测
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256i a = _mm256_set1_epi32(0);
        return 0;
    }
" DUILIB_HAVE_AVX2)
```

---

### 8. duilib_cef_macos.cmake（macOS CEF 特殊配置）

**功能：** macOS 平台 CEF 浏览器支持的特殊配置。

**主要配置内容：**

- CEF_ROOT 路径设置
- CEF Framework 配置
- Helper 应用配置（多进程支持）
- 资源文件复制（主题、语言包、字体等）
- App Bundle 配置

---

## 根工程脚本（依赖准备与公共设施）

下面这些文件由**仓库根 `CMakeLists.txt`** 使用，库子项目（`duilib/`）与示例
（`examples/*`）不需要关心（示例通过 `duilib_example.cmake` 间接使用公共层）。
按执行顺序：

| 文件 | 何时执行 | 职责与关键入口 |
| :--- | :--- | :--- |
| `duilib_i18n.cmake` | `project()` 之前 | 语言解析（`--lang`/`DUILIB_LANG` → `LC_ALL`/`LC_MESSAGES`/`LANG` → 系统区域 → en；只有 `zh`/`zh_CN`/`zh_Hans` 用中文）与消息目录加载。`include` 与 `cmake -P` 均可用，入口 `duilib_t()` / `duilib_msg()` / `duilib_warn()` / `duilib_error()` |
| `duilib_bootstrap.cmake` | `project()` 之前 | 工具自举：PATH 上没有 ninja 时由 `duilib_bootstrap_ninja(<patch_dir> <patch_zip>)` 依次从 VS 安装目录与 Skia 补丁归档的 `bin/ninja.exe` 取用 |
| `duilib_preflight_tools.cmake` | `project()` 之前（`cmake -P`） | 阶段 A 预检：只探测工具（cmake/ninja/git/python、`vswhere`）与系统库，不下载、不安装、不改文件 |
| `duilib_root_options.cmake` | `project()` 之后 | 平台/编译器/架构识别，写入统一缓存选项（覆盖子项目 `option()` 的默认值）；同时前移 `DUILIB_MINGW`/`DUILIB_MSYS2_ENV`，并按配置解析 SDL3 变体前缀 |
| `duilib_preflight.cmake` | `project()` 之后 | 阶段 B 预检：编译器确定后的确定性校验（架构、工具链、系统库），错误一次性汇总输出 |
| `duilib_deps.cmake` | `project()` 之后 | 建立依赖检查与准备目标（`duilib_configure_dependencies()` / `duilib_link_dependency_targets()`）；`DUILIB_DEPS_MODE=off/prepare/auto` 三档 |
| `duilib_deps_run.cmake` | 构建期（`cmake -P`） | 真正的依赖准备：Skia/SDL3/CEF 的下载、校验、构建与安装；SDL3 每个变体装到自己的前缀并写 `.duilib-sdl`/`.duilib-crt` 标记；`duilib_deps` 目标与 `build.py deps` 共用 |
| `duilib_download.cmake` | `cmake -P` | 下载工具函数：curl 优先（支持断点续传与重试），不存在时回退 `file(DOWNLOAD)`；入口 `duilib_download_file()` / `duilib_verify_sha256()` / `duilib_verify_sha1()` |
| `duilib_cef.cmake` | `cmake -P` | CEF 运行时（Release/Resources）的下载、校验、解压与按平台复制；写指纹 `.duilib-cef` |

依赖目录（默认 `<deps-root>` 为仓库同级目录，可用 `-DDUILIB_DEPS_ROOT=` 或
`build.py --deps-root` 指定）：

| 目录 | 内容 |
| :--- | :--- |
| `<deps-root>/skia/` | Skia 源码与 `out/<变体>/`（规则见 `duilib_skia_variants.cmake`） |
| `<deps-root>/SDL/` | SDL3 源码（Windows 默认不启用 SDL 时不下载） |
| `<deps-root>/SDL3/<变体>/` | SDL3 每个变体的完整安装前缀（`include/`、`lib/`、`share/licenses`）；非 Windows 为 `<deps-root>/SDL3/` 单前缀 |
| `<deps-root>/SDL3_build/` | SDL3 构建目录（按架构/配置；stamp 不匹配时清空重建） |
| `<deps-root>/cef_binary/` | CEF 下载缓存与解压结果 |

**构建消息目录（`i18n/messages.json`）：** CMake 侧（`duilib_i18n.cmake`）与 Python
侧（`build/i18n.py`，供 `build.py`、`pack.py`、`release.py` 等使用）都从这一份
文件取文案，新增或修改提示时只改这里：

- 结构为 `{"messages": {"<键>": {"en": "...", "zh_CN": "..."}}}`，每个键**必须同时
  提供 `en` 与 `zh_CN`；英文是兜底语言，缺 `zh_CN` 时两侧都会回退到 `en`。
- 键名用 `区域.用途` 形式（如 `preflight.cmake_too_old`、
  `preflight.cmake_too_old_hint`），错误文案配套一个 `_hint` 键，
  `duilib_error()` / `Messages.hint()` 会拼成第二行 `Hint:`。
- CMake 侧用 `duilib_msg()` / `duilib_warn()` 输出，动态内容作为第二个参数追加在
  文案之后（这类文案不要留 `%s` 占位符）；Python 侧（`Messages.t()`）沿用
  `%s` 占位符，由调用方 `% (...)` 插值（如 `pkg.cef_lib_version_hint`）。查不到
  键时两侧都直接输出键名本身，便于发现漏配。
- 语言判定顺序两侧一致：`--lang`/`DUILIB_LANG` → `LC_ALL`/`LC_MESSAGES`/`LANG`
  → 系统区域 → `en`；只有 `zh`/`zh_CN`/`zh_Hans` 用中文，`zh_TW`/`zh_HK`/
  `zh_Hant` 用英文。Windows 控制台不是 UTF-8（`chcp 65001`）时回退英文并打印
  `lang.non_utf8_fallback`；Python 侧入口会先调用 `set_console_utf8()` 把控制台
  代码页切到 65001 再输出（它只改编码、不改语言判定；切换失败时中文可能显示为
  乱码，可先执行 `chcp 65001`）。

命令行统一走 `python3 build/build.py`（`deps` 完成准备），参数与排障见
[build/build-reference.md](../build/build-reference.md)。

---

## CMake 使用示例

### 基本使用方法

#### 1. 创建 CMakeLists.txt

在项目根目录创建 `CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_duilib_app)

# 设置项目源码目录
set(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# 包含 duilib CMake 配置
include(${DUILIB_ROOT}/cmake/duilib_bin.cmake)
```

#### 2. 编译命令

```bash
# 创建构建目录（禁止在源码目录编译）
mkdir build
cd build

# 配置项目
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release
```

**Unix 下的 Debug 构建**（单配置生成器，构建目录与产物目录都与 Release 分离）：

```bash
cmake -S .. -B ./build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build_debug
```

日常构建推荐使用统一入口（自动准备 Skia/CEF；Windows 上 SDL 默认关闭，需要时加 `--sdl`）：

```bash
python3 build/build.py --config Debug
```

构建树固定为 `out/<arch>/<Config>`；详见 `build/build-reference.md`。

---

### 高级使用示例

#### 1. 启用 SDL 支持

```bash
cmake -S .. -B . -DDUILIB_ENABLE_SDL=ON -DCMAKE_BUILD_TYPE=Release
```

#### 2. 启用 CEF 支持

```bash
cmake -S .. -B . -DDUILIB_ENABLE_CEF=ON -DCMAKE_BUILD_TYPE=Release
```

#### 3. 使用 CEF 109 版本（支持 Win7）

```bash
cmake -S .. -B . -DDUILIB_ENABLE_CEF=ON -DDUILIB_CEF_109=ON -DCMAKE_BUILD_TYPE=Release
```

#### 4. 指定 Skia 库路径

```bash
cmake -S .. -B . -DDUILIB_SKIA_LIB_SUBPATH=llvm.x64.release -DCMAKE_BUILD_TYPE=Release
```

#### 5. 启用调试日志

```bash
cmake -S .. -B . -DDUILIB_LOG=ON -DCMAKE_BUILD_TYPE=Debug
```

#### 6. MSVC 动态运行库与 DLL 模式

```bash
# 动态运行库（静态库）
cmake -S .. -B . -DDUILIB_MD=ON -DCMAKE_BUILD_TYPE=Release

# duilib 编译为 DLL（隐含 /MD），输出 bin/duilib*.dll + 导入库
cmake -S .. -B . -DDUILIB_DLL=ON -DCMAKE_BUILD_TYPE=Release
```

`DUILIB_DLL` 仅支持 Windows MSVC，会自动打开 `DUILIB_MD`。duilib 共享库自行
链接 Skia/SDL/CEF 与第三方静态库；示例只链接导入库。输出命名与 MSBuild 的
DLL 模式一致（x86 为 `duilib32*.dll`，Debug 带 `_d`，导入库统一为
`duilib.lib`）。切换 `MT`/`MD`/DLL 模式后需要重新编译 CEF wrapper 等全部目标；
使用 `build.py --dll` 时会自动记录并清理构建目录，直接使用 `cmake` 时请删除
构建目录后重新配置。

---

### MinGW-w64 编译示例

#### 1. 使用 GCC 编译器

```bash
cmake -S .. -B ./build_gcc -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER=gcc ^
    -DCMAKE_CXX_COMPILER=g++ ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DDUILIB_ENABLE_SDL=ON

cmake --build ./build_gcc
```

#### 2. 使用 Clang 编译器

```bash
cmake -S .. -B ./build_llvm -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DDUILIB_ENABLE_SDL=ON

cmake --build ./build_llvm
```

---

## 编译开关选项汇总

| 选项名称 | 类型 | 默认值 | 说明 |
|----------|------|--------|------|
| `DUILIB_LOG` | BOOL | OFF | 打印调试日志 |
| `DUILIB_SKIA_LIB_SUBPATH` | STRING | 空 | Skia 库子目录；空表示按平台/配置自动拼接 |
| `DUILIB_ENABLE_SDL` | BOOL | Windows=OFF, 其他=ON | 启用 SDL 支持 |
| `DUILIB_ENABLE_CEF` | BOOL | Windows MSVC/Linux/macOS=ON，MinGW/FreeBSD=OFF | 启用 CEF 支持 |
| `DUILIB_CEF_109` | BOOL | OFF | CEF 109 版本（Win7）；已弃用，等价于 `DUILIB_CEF_VERSION=109` |
| `DUILIB_WEBVIEW2_EXE` | BOOL | OFF | WebView2 可执行程序 |
| `DUILIB_MD` | BOOL | OFF | MSVC 动态运行库（/MD） |
| `DUILIB_DLL` | BOOL | OFF | duilib 编译为 DLL（仅 Windows MSVC，隐含 `DUILIB_MD=ON`） |
| `DUILIB_MINGW_STATIC` | BOOL | ON | MinGW 静态链接 |
| `DUILIB_SDL_AUTOBUILD` | BOOL | OFF | 内部信号（非用户选项）：`DUILIB_DEPS_MODE=auto` 时由 `duilib_deps.cmake` 置位，跳过 SDL3 存在性校验 |

**根工程选项（`cmake/duilib_root_options.cmake`；`DUILIB_LANG`、`DUILIB_PREFLIGHT`、
`DUILIB_PREFLIGHT_ONLY` 在根 `CMakeLists.txt` 中声明，因为 `project()` 之前就要用）：**

| 选项名称 | 类型 | 默认值 | 说明 |
|----------|------|--------|------|
| `DUILIB_LANG` | STRING | auto | 交互语言：`auto`/`zh_CN`/`en`（`auto` 先看同名环境变量，再按 `LC_ALL`/`LC_MESSAGES`/`LANG` 与系统区域判定） |
| `DUILIB_PREFLIGHT` | BOOL | ON | 两阶段环境预检（阶段 A/B）；关闭可跳过命令探测，但架构与编译器校验仍会执行 |
| `DUILIB_PREFLIGHT_ONLY` | BOOL | OFF | 只跑预检后退出，不生成构建目标 |
| `DUILIB_TARGET_ARCH` | STRING | auto | 目标架构：`auto`/`x86`/`x64`/`arm64`（`auto` 取当前工具链） |
| `DUILIB_BUILD_THIRD_PARTY` | BOOL | ON | 编译 zlib/libpng/cximage/libwebp |
| `DUILIB_BUILD_EXAMPLES` | BOOL | ON | 是否编译示例程序 |
| `DUILIB_EXAMPLES` | STRING | 按平台推导 | 参与构建的示例名（分号/逗号分隔，`all` 表示全部）；被 CEF 开关变化改写时会重新推导，避免旧缓存值把已关闭的 CEF 示例留在列表里 |
| `DUILIB_MACOS_BUNDLE_EXAMPLES` | STRING | 空 | 编译为 `.app` 的示例（仅 macOS；`all`/`*` 为全部） |
| `DUILIB_DEPS_MODE` | STRING | off | `off` 只检查；`prepare` 只建依赖目标（两阶段推荐流程）；`auto` 构建期先准备 |
| `DUILIB_DEPS_ROOT` | PATH | `<源根>/..` | 依赖根目录（Skia/SDL3/CEF 都在其下） |
| `DUILIB_SDL_ROOT` | PATH | `<deps-root>/SDL3` | SDL3 安装根；Windows 下按 `<根>/<变体>` 选择前缀，见上一节规则 |
| `DUILIB_SDL_BUILD_ROOT` | PATH | `<deps-root>/SDL3_build` | SDL3 构建目录（按架构/配置） |
| `DUILIB_SKIA_REF` / `DUILIB_SDL_REF` | STRING | 清单默认 | `cmake/third_party_versions.json` 中的版本键（commit/branch/tag） |
| `DUILIB_CEF_VERSION` | STRING | 清单默认 | `109` / `156`；`DUILIB_CEF_109=ON` 等价于选 109 |
| `DUILIB_OFFLINE` | BOOL | OFF | 只用已缓存的依赖源码/归档（不联网） |
| `DUILIB_DEPS_AUTO_CLONE` | BOOL | ON | 准备依赖时允许 `git clone` |
| `DUILIB_DEPS_FORCE` | BOOL | OFF | 忽略 stamp 强制重建依赖 |
| `DUILIB_DEPS_JOBS` | STRING | 6 | Skia/SDL3 依赖构建的并行数 |
| `DUILIB_DEPS_LOCK_TIMEOUT` | STRING | 5 | 依赖准备文件锁超时（秒），避免多进程同时准备 |
| `DUILIB_DOWNLOAD_RETRIES` / `DUILIB_DOWNLOAD_RETRY_DELAY` | STRING | 清单 `retry` 段（0=不限 / 20） | 网络重试次数与间隔；每次配置都按 `third_party_versions.json` 的 `retry` 强制写入，改清单即可 |
| `DUILIB_DOWNLOAD_CONNECT_TIMEOUT` | STRING | 30 | 下载连接超时（秒） |
| `DUILIB_SKIA_REQUIRE_PATCH` | BOOL | ON | 选中的 Skia ref 必须有配套 `patchZip` |
| `DUILIB_SKIA_AUTOBUILD` | BOOL | OFF | 跳过 Skia 存在性校验（`deps` 先构建时使用） |
| `DUILIB_SDL_RPATH` / `DUILIB_RPATH_ABSOLUTE` | BOOL | ON | Unix rpath：先写相对路径，另含绝对路径回退 |
| `DUILIB_LLVM_ROOT` | PATH | 自动探测 | Skia 在 Windows 使用的 LLVM 根（`gn` 的 `clang_win`） |
| `DUILIB_GN_EXECUTABLE` / `DUILIB_NINJA_EXECUTABLE` | FILEPATH | 空 | 覆盖 `gn`/`ninja` 的位置 |
| `DUILIB_CEF_ARCHIVE` | FILEPATH | 空 | 本地 CEF 归档（跳过下载） |
| `DUILIB_CEF_REQUIRED` | BOOL | ON | CEF 运行时准备失败即报错 |

---

## 平台差异说明

### Windows vs Linux vs macOS vs FreeBSD

| 配置项 | Windows | Linux | macOS | FreeBSD |
|--------|---------|-------|-------|---------|
| C++ 标准 | C++20 | C++20 | C++20 | C++20 |
| 编码 | Unicode | UTF-8 | UTF-8 | UTF-8 |
| 图形库 | Skia + GDI | Skia + X11 | Skia + Metal | Skia + X11 |
| 输入支持 | Win32/SDL | X11/SDL | Cocoa/SDL | X11/SDL |
| 浏览器 | CEF/WebView2 | CEF | CEF | ❌ |
| SDL 默认 | 关闭 | 开启 | 开启 | 开启 |

### MSVC vs MinGW-w64

| 配置项 | MSVC | MinGW-w64 |
|--------|------|-----------|
| 运行库 | MT/MD | 静态链接 |
| 子系统 | WINDOWS | WINDOWS |
| Manifest | 内嵌 | RC 文件 |
| CEF 延迟加载 | /DELAYLOAD | ❌ |

---

## 维护注意事项

### 1. 修改开关选项后需要清除缓存

CMake 会缓存选项值，修改后需要清除缓存：

```bash
rm -rf CMakeCache.txt CMakeFiles/
# 或者删除整个构建目录重新配置
```

### 2. 禁止在源码目录编译

duilib CMake 配置强制禁止在源码目录编译：

```cmake
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build...")
endif()
```

### 3. Skia 库路径要求

确保 Skia 编译输出在正确目录：
```
skia/out/
├── llvm.x64.debug/             # MSVC / clang-cl（/MD 时追加 .mdd）
├── llvm.x64.release/           # MSVC / clang-cl（/MD 时追加 .md）
├── gcc.msvcrt.x64.release/     # MSYS2 MINGW64
├── gcc.x64.release/            # MSYS2 UCRT64 等
├── mingw64-gcc.x64.release/    # 独立 MinGW-w64
└── ...
```

SDL3 安装前缀（Windows）由同一套架构/配置/运行库规则决定：
`<deps-root>/SDL3/msvc.<arch>.<debug|release>[.mdd|.md]`（每个变体是完整安装
前缀），MinGW 复用 Skia 的 release 变体名，非 Windows 为 `<deps-root>/SDL3` 单前缀。
两类依赖都由 `python3 build/build.py deps` 生成，规则见
[duilib_sdl_variants.cmake](duilib_sdl_variants.cmake)。

### 4. 第三方库依赖

编译前确保以下依赖可用：

| 平台 | 必需依赖 |
|------|----------|
| Windows | Skia |
| Linux | Skia, X11, Freetype, Fontconfig |
| macOS | Skia, Cocoa/Metal 框架 |
| FreeBSD | Skia, X11, Freetype, Fontconfig |

---

## 常见问题排查

### Q1: CMake 报 "Unknown OS"

**原因：** 操作系统不被支持。

**解决方案：** 检查 `duilib_common.cmake` 中的操作系统检测逻辑。

---

### Q2: Skia 库找不到

**原因：** Skia 未编译或路径不正确。

**解决方案：**
1. 先执行 `python3 build/build.py deps`（或 `build.py deps --force`）重新准备依赖
2. 确认 Skia 已编译：`ls skia/out/`
3. 使用 `DUILIB_SKIA_LIB_SUBPATH` 指定正确路径

---

### Q3: MinGW 编译报 linker error

**原因：** 可能缺少依赖库或链接顺序问题。

**解决方案：**
1. 确保 `DUILIB_MINGW_STATIC=ON`
2. 检查 `CMAKE_EXE_LINKER_FLAGS`

---

### Q4: CEF 程序启动失败

**原因：** libcef.dll 未找到或版本不匹配。

**解决方案：**
1. 确认 CEF 库已正确下载
2. 检查 `DUILIB_CEF_LIB_PATH` 路径
3. 将 libcef.dll 复制到可执行程序目录

---

### Q5: WebView2 程序无法运行

**原因：** WebView2Loader.dll 未找到。

**解决方案：**
1. 确认使用 `DUILIB_WEBVIEW2_EXE=ON`
2. 检查 WebView2Loader.dll 是否在 bin 目录

---

### Q6: macOS 编译报框架未找到

**原因：** Cocoa/AppKit 等框架未正确链接。

**解决方案：**
1. 确保使用 Xcode 或支持框架的编译器
2. 检查 `duilib_bin_macos.cmake` 中的框架配置
