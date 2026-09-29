English | [简体中文](README.md)

> Last synced: 2026-09-29

# nim_duilib CMake Project Configuration Guide

This document details the purpose and usage of the CMake configuration files (*.cmake) in the `nim_duilib\cmake` directory, for reference by maintainers.

## Directory File Overview

```
cmake/
├── duilib_common.cmake        # Common configuration (OS/compiler/CPU detection, path settings)
├── duilib_bin.cmake           # Executable program base configuration (C++ standard, path config)
├── duilib_bin_windows.cmake   # Windows platform-specific configuration
├── duilib_bin_linux.cmake     # Linux platform-specific configuration
├── duilib_bin_macos.cmake     # macOS platform-specific configuration
├── duilib_bin_freebsd.cmake   # FreeBSD platform-specific configuration
├── duilib_compiles.cmake      # Compile option detection (AVX/AVX2)
└── duilib_cef_macos.cmake     # macOS CEF special configuration
```

---

## Configuration Files Explained

### 1. duilib_common.cmake (common configuration)

**Function:** Defines cross-platform common configuration variables and switch options; it is the base configuration file for all CMake projects.

**Main detection content:**

#### 1.1 Operating system detection
```cmake
DUILIB_OS_WINDOWS  # Windows system
DUILIB_OS_LINUX    # Linux system
DUILIB_OS_MACOS    # macOS system
DUILIB_OS_FREEBSD  # FreeBSD system
```

#### 1.2 Compiler detection
```cmake
DUILIB_COMPILER_MSVC    # MSVC compiler
DUILIB_COMPILER_GCC     # GCC compiler
DUILIB_COMPILER_LLVM    # Clang/LLVM compiler
DUILIB_COMPILER_NAME    # Compiler name (e.g. "msvc", "mingw64-gcc", "mingw64-llvm")
```

#### 1.3 CPU architecture detection
```cmake
DUILIB_SYSTEM_PROCESSOR  # Processor type: x86, x64, arm32, arm64
DUILIB_BITS_64           # Whether it is a 64-bit system
```

#### 1.4 Build type detection
```cmake
DUILIB_BUILD_TYPE        # "debug" or "release"
```

**Main switch options:**

| Option | Default value | Description |
|----------|--------|------|
| `DUILIB_LOG` | OFF | Print duilib debug logs |
| `DUILIB_SKIA_LIB_SUBPATH` | OFF | Skia library subdirectory (OFF means auto-concatenated) |
| `DUILIB_ENABLE_SDL` | Windows=OFF, others=ON | Enable SDL input support |
| `DUILIB_ENABLE_CEF` | OFF | Enable CEF browser support |
| `DUILIB_CEF_109` | OFF | Use CEF 109 version (supports Win7) |
| `DUILIB_WEBVIEW2_EXE` | OFF | WebView2 executable program (Windows only) |
| `DUILIB_MINGW_STATIC` | ON | MinGW static linking |

**Main path variables:**

| Variable | Description |
|--------|------|
| `DUILIB_ROOT` | duilib source root directory |
| `DUILIB_LIB_PATH` | duilib library output directory |
| `DUILIB_BIN_PATH` | duilib executable program output directory |
| `DUILIB_SKIA_SRC_ROOT_DIR` | Skia source directory |
| `DUILIB_SKIA_LIB_PATH` | Skia library file directory |
| `DUILIB_SDL_SRC_ROOT_DIR` | SDL3 source directory |
| `DUILIB_SDL_LIB_PATH` | SDL3 library file directory |
| `DUILIB_CEF_SRC_ROOT_DIR` | libCEF source directory |
| `DUILIB_CEF_LIB_PATH` | libCEF library file directory |

**Main library list variables:**

| Variable | Description |
|--------|------|
| `DUILIB_LIBS` | duilib base library list |
| `DUILIB_SKIA_LIBS` | Skia library list (svg, skshaper, skottie, sksg, jsonreader, skia) |
| `DUILIB_SDL_LIBS` | SDL3 library list |
| `DUILIB_CEF_LIBS` | libCEF library list |

**Skia path concatenation rule:**
```cmake
# When DUILIB_SKIA_LIB_SUBPATH is OFF, auto-concatenate by rule:
${DUILIB_SKIA_SRC_ROOT_DIR}/out/${DUILIB_COMPILER_NAME}.${DUILIB_SYSTEM_PROCESSOR}.${DUILIB_BUILD_TYPE}

# Examples:
# skia/out/msvc.x64.release
# skia/out/mingw64-gcc.x64.release
# skia/out/llvm.x86.debug
```

---

### 2. duilib_bin.cmake (executable program base configuration)

**Function:** Common CMake configuration for executable programs, applicable to all platforms.

**Main configuration content:**

#### 2.1 C++ standard setting
```cmake
set(CMAKE_CXX_STANDARD 20)             # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)    # Force C++20
```

#### 2.2 Path configuration
```cmake
include_directories(${DUILIB_ROOT})              # duilib root directory
include_directories(${DUILIB_PROJECT_SRC_DIR})   # Project source directory
link_directories("${DUILIB_LIB_PATH}")           # duilib library directory
link_directories("${DUILIB_SKIA_LIB_PATH}")      # Skia library directory
link_directories("${DUILIB_SDL_LIB_PATH}")       # SDL library directory (if enabled)
```

#### 2.3 Output directory
```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_BIN_PATH}")
```

#### 2.4 Source collection
```cmake
aux_source_directory(${DUILIB_PROJECT_SRC_DIR} SRC_FILES)
# Supports subdirectories: DUILIB_SRC_SUB_DIRS variable
```

#### 2.5 Platform dispatch
Automatically include the corresponding platform configuration file based on the operating system:
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

### 3. duilib_bin_windows.cmake (Windows platform configuration)

**Function:** Windows-specific compilation and linking configuration.

**Main configuration content:**

#### 3.1 MSVC runtime library configuration
```cmake
if("${DUILIB_MD}" STREQUAL "ON")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")  # MD/MDd
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")     # MT/MTd
endif()
```

#### 3.2 MSVC compile options
```cmake
add_compile_options("/utf-8")                      # UTF-8 source encoding
add_compile_options($<$<COMPILE_LANGUAGE:C>:/MP${CPU_CORES}>)   # Multi-core compilation
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP${CPU_CORES}>)
```

#### 3.3 Unicode encoding
```cmake
add_definitions(-DUNICODE -D_UNICODE)
```

#### 3.4 MinGW-w64 special handling
```cmake
if(DUILIB_MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows ${CMAKE_EXE_LINKER_FLAGS}")    # Windows program
    if(DUILIB_MINGW_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "-static ${CMAKE_EXE_LINKER_FLAGS}")  # Static linking
    endif()
endif()
```

#### 3.5 Manifest file configuration
```cmake
if(DUILIB_BITS_64)
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x64.manifest")
else()
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.x86.manifest")
endif()
```

#### 3.6 MSVC subsystem setting
```cmake
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
```

#### 3.7 CEF delay load
```cmake
if(DUILIB_ENABLE_CEF)
    target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:libcef.dll")
endif()
```

#### 3.8 WebView2 support
```cmake
if(DUILIB_WEBVIEW2_EXE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DUILIB_WEBVIEW2=1)
    # Automatically copy WebView2Loader.dll
endif()
```

#### 3.9 Windows system dependency libraries
```cmake
set(DUILIB_WINDOWS_LIBS Comctl32 Imm32 Opengl32 User32 shlwapi)
# Optional: Version.lib Winmm.lib Setupapi.lib (SDL dependency)
```

---

### 4. duilib_bin_linux.cmake (Linux platform configuration)

**Function:** Linux-specific compilation and linking configuration.

**Main configuration content:**

```cmake
# CEF support
if(DUILIB_ENABLE_CEF)
    include_directories(${DUILIB_CEF_SRC_ROOT_DIR})
    link_directories("${DUILIB_CEF_LIB_PATH}")
endif()

# Linux system dependency libraries
set(DUILIB_LINUX_LIBS X11 freetype fontconfig pthread dl)

# Link command
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS} ${DUILIB_LINUX_LIBS})
```

---

### 5. duilib_bin_macos.cmake (macOS platform configuration)

**Function:** macOS-specific compilation and linking configuration.

**Main configuration content:**

#### 5.1 System framework search
```cmake
find_library(ACCELERATE Accelerate)
find_library(COREFOUNDATION CoreFoundation)
find_library(CORETEXT CoreText)
find_library(COREGRAPHICS CoreGraphics)
```

#### 5.2 Compiler parameter setting
```cmake
set(DUILIB_COMPILER_FLAGS
    -fno-strict-aliasing
    -fstack-protector
    -funwind-tables
    -fvisibility=hidden
    -Wall
    -Wextra
    # ... more parameters
)

set(DUILIB_CXX_COMPILER_FLAGS
    -fno-threadsafe-statics
    -fvisibility-inlines-hidden
    -frtti
    # ... more parameters
)
```

#### 5.3 Link command
```cmake
target_link_libraries(${PROJECT_NAME}
    ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS}
    ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
    "-framework AppKit" "-framework Foundation" "-framework Metal" "-framework Cocoa"
)
```

---

### 6. duilib_bin_freebsd.cmake (FreeBSD platform configuration)

**Function:** FreeBSD-specific compilation and linking configuration.

**Main configuration content:**

```cmake
# FreeBSD system dependency libraries
set(DUILIB_FREEBSD_LIBS pthread dl)

find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(X11 REQUIRED)

target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_FREEBSD_LIBS} ${X11_LIBRARIES} Freetype::Freetype Fontconfig::Fontconfig)
```

---

### 7. duilib_compiles.cmake (compile option detection)

**Function:** Detect CPU feature support (such as AVX/AVX2 instruction sets) on the target platform.

**Detection content:**

```cmake
# AVX support detection
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256 a = _mm256_set1_ps(0.0f);
        return 0;
    }
" DUILIB_HAVE_AVX)

# AVX2 support detection
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256i a = _mm256_set1_epi32(0);
        return 0;
    }
" DUILIB_HAVE_AVX2)
```

---

### 8. duilib_cef_macos.cmake (macOS CEF special configuration)

**Function:** Special configuration for CEF browser support on macOS.

**Main configuration content:**

- CEF_ROOT path setting
- CEF Framework configuration
- Helper application configuration (multi-process support)
- Resource file copying (themes, language packs, fonts, etc.)
- App Bundle configuration

---

## CMake Usage Examples

### Basic usage

#### 1. Create CMakeLists.txt

Create `CMakeLists.txt` in the project root directory:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_duilib_app)

# Set the project source directory
set(DUILIB_PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Include the duilib CMake configuration
include(${DUILIB_ROOT}/cmake/duilib_bin.cmake)
```

#### 2. Build commands

```bash
# Create the build directory (in-tree build is prohibited)
mkdir build
cd build

# Configure the project
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

---

### Advanced usage examples

#### 1. Enable SDL support

```bash
cmake -S .. -B . -DDUILIB_ENABLE_SDL=ON -DCMAKE_BUILD_TYPE=Release
```

#### 2. Enable CEF support

```bash
cmake -S .. -B . -DDUILIB_ENABLE_CEF=ON -DCMAKE_BUILD_TYPE=Release
```

#### 3. Use CEF 109 version (supports Win7)

```bash
cmake -S .. -B . -DDUILIB_ENABLE_CEF=ON -DDUILIB_CEF_109=ON -DCMAKE_BUILD_TYPE=Release
```

#### 4. Specify the Skia library path

```bash
cmake -S .. -B . -DDUILIB_SKIA_LIB_SUBPATH=llvm.x64.release -DCMAKE_BUILD_TYPE=Release
```

#### 5. Enable debug log

```bash
cmake -S .. -B . -DDUILIB_LOG=ON -DCMAKE_BUILD_TYPE=Debug
```

#### 6. MSVC dynamic runtime

```bash
cmake -S .. -B . -DDUILIB_MD=ON -DCMAKE_BUILD_TYPE=Release
```

---

### MinGW-w64 build examples

#### 1. Using the GCC compiler

```bash
cmake -S .. -B ./build_gcc -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER=gcc ^
    -DCMAKE_CXX_COMPILER=g++ ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DDUILIB_ENABLE_SDL=ON

cmake --build ./build_gcc
```

#### 2. Using the Clang compiler

```bash
cmake -S .. -B ./build_llvm -G "MinGW Makefiles" ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DDUILIB_ENABLE_SDL=ON

cmake --build ./build_llvm
```

---

## Build Switch Options Summary

| Option | Type | Default value | Description |
|----------|------|--------|------|
| `DUILIB_LOG` | BOOL | OFF | Print debug logs |
| `DUILIB_SKIA_LIB_SUBPATH` | STRING | OFF | Skia library subdirectory |
| `DUILIB_ENABLE_SDL` | BOOL | Windows=OFF, others=ON | Enable SDL support |
| `DUILIB_ENABLE_CEF` | BOOL | OFF | Enable CEF support |
| `DUILIB_CEF_109` | BOOL | OFF | CEF 109 version (Win7) |
| `DUILIB_WEBVIEW2_EXE` | BOOL | OFF | WebView2 executable program |
| `DUILIB_MD` | BOOL | OFF | MSVC dynamic runtime (/MD) |
| `DUILIB_MINGW_STATIC` | BOOL | ON | MinGW static linking |

---

## Platform Differences

### Windows vs Linux vs macOS vs FreeBSD

| Config item | Windows | Linux | macOS | FreeBSD |
|--------|---------|-------|-------|---------|
| C++ standard | C++20 | C++20 | C++20 | C++20 |
| Encoding | Unicode | UTF-8 | UTF-8 | UTF-8 |
| Graphics library | Skia + GDI | Skia + X11 | Skia + Metal | Skia + X11 |
| Input support | Win32/SDL | X11/SDL | Cocoa/SDL | X11/SDL |
| Browser | CEF/WebView2 | CEF | CEF | ❌ |
| SDL default | Off | On | On | On |

### MSVC vs MinGW-w64

| Config item | MSVC | MinGW-w64 |
|--------|------|-----------|
| Runtime library | MT/MD | Static linking |
| Subsystem | WINDOWS | WINDOWS |
| Manifest | Embedded | RC file |
| CEF delay load | /DELAYLOAD | ❌ |

---

## Maintenance Notes

### 1. Clearing the cache after modifying switch options

CMake caches option values; after modification, the cache must be cleared:

```bash
rm -rf CMakeCache.txt CMakeFiles/
# Or delete the entire build directory and reconfigure
```

### 2. In-tree build is prohibited

The duilib CMake configuration forcibly prohibits building in the source directory:

```cmake
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build...")
endif()
```

### 3. Skia library path requirement

Ensure the Skia build output is in the correct directory:
```
skia/out/
├── msvc.x64.release/
├── mingw64-gcc.x64.release/
├── mingw64-llvm.x64.release/
└── ...
```

### 4. Third-party library dependencies

Ensure the following dependencies are available before building:

| Platform | Required dependencies |
|------|----------|
| Windows | Skia |
| Linux | Skia, X11, Freetype, Fontconfig |
| macOS | Skia, Cocoa/Metal frameworks |
| FreeBSD | Skia, X11, Freetype, Fontconfig |

---

## Common Issues / Troubleshooting

### Q1: CMake reports "Unknown OS"

**Cause:** The operating system is not supported.

**Solution:** Check the OS detection logic in `duilib_common.cmake`.

---

### Q2: Skia library not found

**Cause:** Skia is not built or the path is incorrect.

**Solution:**
1. Confirm Skia is built: `ls skia/out/`
2. Use `DUILIB_SKIA_LIB_SUBPATH` to specify the correct path

---

### Q3: MinGW build reports linker error

**Cause:** Missing dependency libraries or link order issue.

**Solution:**
1. Ensure `DUILIB_MINGW_STATIC=ON`
2. Check `CMAKE_EXE_LINKER_FLAGS`

---

### Q4: CEF program fails to start

**Cause:** libcef.dll not found or version mismatch.

**Solution:**
1. Confirm the CEF library is downloaded correctly
2. Check the `DUILIB_CEF_LIB_PATH` path
3. Copy libcef.dll to the executable program directory

---

### Q5: WebView2 program cannot run

**Cause:** WebView2Loader.dll not found.

**Solution:**
1. Confirm `DUILIB_WEBVIEW2_EXE=ON` is used
2. Check whether WebView2Loader.dll is in the bin directory

---

### Q6: macOS build reports framework not found

**Cause:** Cocoa/AppKit and other frameworks are not linked correctly.

**Solution:**
1. Ensure you use Xcode or a compiler that supports frameworks
2. Check the framework configuration in `duilib_bin_macos.cmake`
