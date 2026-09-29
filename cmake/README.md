# nim_duilib CMake 工程配置说明

本文档详细介绍 `nim_duilib\cmake` 目录中 CMake 配置文件（*.cmake）的用途及使用方法，供维护人员参考。

## 目录文件概览

```
cmake/
├── duilib_common.cmake        # 公共配置（操作系统/编译器/CPU检测，路径设置）
├── duilib_bin.cmake           # 可执行程序基础配置（C++标准，路径配置）
├── duilib_bin_windows.cmake   # Windows 平台特定配置
├── duilib_bin_linux.cmake     # Linux 平台特定配置
├── duilib_bin_macos.cmake     # macOS 平台特定配置
├── duilib_bin_freebsd.cmake   # FreeBSD 平台特定配置
├── duilib_compiles.cmake      # 编译选项检测（AVX/AVX2）
└── duilib_cef_macos.cmake     # macOS CEF 特殊配置
```

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
DUILIB_COMPILER_NAME    # 编译器名称（如 "msvc", "mingw64-gcc", "mingw64-llvm"）
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
| `DUILIB_SKIA_LIB_SUBPATH` | OFF | Skia 库子目录（OFF 表示自动拼接） |
| `DUILIB_ENABLE_SDL` | Windows=OFF, 其他=ON | 启用 SDL 输入支持 |
| `DUILIB_ENABLE_CEF` | OFF | 启用 CEF 浏览器支持 |
| `DUILIB_CEF_109` | OFF | 使用 CEF 109 版本（支持 Win7） |
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
| `DUILIB_SDL_SRC_ROOT_DIR` | SDL3 源码目录 |
| `DUILIB_SDL_LIB_PATH` | SDL3 库文件目录 |
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
```cmake
# DUILIB_SKIA_LIB_SUBPATH 为 OFF 时，按规则自动拼接：
${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_COMPILER_NAME}.${DUILIB_SYSTEM_PROCESSOR}.${DUILIB_BUILD_TYPE}

# 示例：
# skia/out/msvc.x64.release
# skia/out/mingw64-gcc.x64.release
# skia/out/llvm.x86.debug
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
if("${DUILIB_MD}" STREQUAL "ON")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")  # MD/MDd
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
if(DUILIB_BITS_64)
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
if(DUILIB_ENABLE_CEF)
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
# CEF 支持
if(DUILIB_ENABLE_CEF)
    include_directories(${DUILIB_CEF_SRC_ROOT_DIR})
    link_directories("${DUILIB_CEF_LIB_PATH}")
endif()

# Linux 系统依赖库
set(DUILIB_LINUX_LIBS X11 freetype fontconfig pthread dl)

# 链接命令
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${DUILIB_LINUX_LIBS})
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

#### 6. MSVC 动态运行库

```bash
cmake -S .. -B . -DDUILIB_MD=ON -DCMAKE_BUILD_TYPE=Release
```

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
| `DUILIB_SKIA_LIB_SUBPATH` | STRING | OFF | Skia 库子目录 |
| `DUILIB_ENABLE_SDL` | BOOL | Windows=OFF, 其他=ON | 启用 SDL 支持 |
| `DUILIB_ENABLE_CEF` | BOOL | OFF | 启用 CEF 支持 |
| `DUILIB_CEF_109` | BOOL | OFF | CEF 109 版本（Win7） |
| `DUILIB_WEBVIEW2_EXE` | BOOL | OFF | WebView2 可执行程序 |
| `DUILIB_MD` | BOOL | OFF | MSVC 动态运行库（/MD） |
| `DUILIB_MINGW_STATIC` | BOOL | ON | MinGW 静态链接 |

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
├── msvc.x64.release/
├── mingw64-gcc.x64.release/
├── mingw64-llvm.x64.release/
└── ...
```

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
1. 确认 Skia 已编译：`ls skia/out/`
2. 使用 `DUILIB_SKIA_LIB_SUBPATH` 指定正确路径

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
