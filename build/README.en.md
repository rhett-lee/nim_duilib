English | [简体中文](README.md)

> Last synced: 2026-09-29

# nim_duilib MSVC Projects and Build Scripts Guide

This document details the purpose and usage of the Visual Studio solution files (*.sln) and build script files (*.bat/*.sh) in the `nim_duilib\build` directory, for reference by maintainers.

## Directory File Overview

```
build/
├── Solution files (Visual Studio)
│   ├── duilib.sln                   # Basic solution (includes lib library, no example programs)
│   ├── duilib_no_cef.sln            # Basic solution (no CEF)
│   ├── examples.sln                 # Complete solution (includes lib library and example programs)
│   └── examples_no_cef.sln          # Complete solution (no CEF)
│
├── Build scripts (Windows/Batch)
│   ├── detect_vs_version.bat         # Automatically detect VS version
│   ├── msvc_build.bat                # CMake + MSVC build script
│   ├── build_duilib_all_in_one.bat   # One-click build script (MSVC + LLVM)
│   ├── gcc-mingw-w64_build.bat       # CMake + MinGW-w64 gcc build script
│   ├── llvm-mingw-w64_build.bat      # CMake + MinGW-w64 clang build script
│   └── build_duilib_all_in_one_mingw-w64.bat  # One-click build script (MinGW-w64)
│
├── Build scripts (Linux/macOS/other platforms)
│   ├── build_duilib_all_in_one.sh   # Linux/macOS/FreeBSD one-click build script
│   ├── linux_build.sh               # Linux system build script
│   ├── msys2_build.sh               # MSYS2 system build script
│   ├── macos_build.sh               # macOS system build script
│   └── freebsd_build.sh             # FreeBSD system build script
│
└── build.md                         # Project build documentation
```

---

## Solution Files Explained

### 1. duilib.sln

**Function:** Basic Visual Studio solution file that includes the duilib core library project.

**Included content:**
- duilib core library project
- Third-party dependency library projects (zlib, libpng, cximage, libwebp)

**Features:**
- Does not include example programs
- Includes CEF and WebView2 support
- Requires Skia to be downloaded and built in advance

**Use cases:**
- Developing using only the duilib core library
- No need to reference example programs

---

### 2. duilib_no_cef.sln

**Function:** Basic solution that does not include the CEF module.

**Differences from duilib.sln:**
- Does not support the CEF (libCEF) module
- WebView2 support remains available

---

### 3. examples.sln

**Function:** Complete example program solution, including the duilib library and all example programs.

**Included content:**
- duilib core library
- Third-party dependency libraries
- Complete set of example programs:
  - `basic` - Basic example
  - `controls` - Control example
  - `ColorPicker` - Color picker
  - `DpiAware` - DPI awareness example
  - `chat` - Chat UI example
  - `layout` - Layout example
  - `ListBox` - List box example
  - `ListCtrl` - List control example
  - `MoveControl` - Move control example
  - `MultiLang` - Multi-language example
  - `render` - Render example
  - `RichEdit` - Rich text editor example
  - `VirtualListBox` - Virtual list box example
  - `threads` - Multi-threading example
  - `TreeView` - Tree view example
  - `cef` - CEF browser example
  - `CefBrowser` - Full CEF browser example
  - `WebView2` - WebView2 example
  - `WebView2Browser` - Full WebView2 browser example
  - `ChildWindow` - Child window example
  - `XmlPreview` - XML preview example
  - `ColorTheme` - Theme color preview and generation example

**Note:** The CEF module requires manual download and configuration; see [docs/CEF.md](../docs/CEF.en.md).

---

### 4. examples_no_cef.sln

**Function:** Complete example program solution that does not include CEF-related examples.

**Differences from examples.sln:**
- Does not include the `cef` and `CefBrowser` example programs
- All other example programs and features remain the same

---

## Build Scripts Explained

### 1. detect_vs_version.bat (VS version detection)

**Function:** Automatically detects the Visual Studio version installed in the system.

**Detection priority:**
1. **Prefer vswhere.exe:** Obtain the latest version via the VS installer tool
2. **Directory traversal fallback:** Traverse common installation directories from highest to lowest version

**Detected VS versions:**
- VS2026 (v145)
- VS2022 (v143)
- VS2019 (v142)
- VS2017 (v141)

**Output variables:**
| Variable | Description |
|--------|------|
| `VS_VERSION` | VS version identifier (e.g. vs2022) |
| `VS_PATH` | VS installation path |
| `VS_MAJOR_VERSION` | Major version number (e.g. 17) |

**Usage:**
```batch
call detect_vs_version.bat
echo Detected: %VS_VERSION% (%VS_MAJOR_VERSION%)
```

---

### 2. msvc_build.bat (CMake + MSVC build)

**Function:** Build the project using CMake and the MSVC compiler from the command line.

**Prerequisites:**
- Visual Studio 2022 (17.0) or later
- CMake 3.21+ (CMake 4.2+ required if using VS2026)
- Download and build Skia in advance

**Usage:**
```batch
cd nim_duilib\build
msvc_build.bat
```

**Build contents:**
1. Third-party libraries: zlib, libpng, cximage, libwebp, libcef
2. duilib core library
3. All example programs

**Skia path requirement:**
- Expected Skia build output at `..\..\skia\out\llvm.x64.release`

---

### 3. build_duilib_all_in_one.bat (one-click build - MSVC)

**Function:** Fully automated build script that automatically downloads all dependencies (including Skia) and completes the build.

**Prerequisites:**
- git.exe
- python3.exe
- LLVM Clang compiler (located at `C:\LLVM\bin`)
- Visual Studio 2022 (17.0) or later

**Usage:**
```batch
cd nim_duilib\build
build_duilib_all_in_one.bat        # Static runtime (/MT)
build_duilib_all_in_one.bat /MD    # Dynamic runtime (/MD)
```

**Build process:**
1. Detect required software (git, python3, clang)
2. Detect VS version
3. Clone the nim_duilib repository (if not present)
4. Clone the skia_compile tool repository
5. Clone the skia repository
6. Apply Skia patches
7. Build Skia (Debug/Release x64/x86)
8. Set the duilib runtime library mode
9. Build the entire solution using devenv

**Build output directories:**
| Platform | Output directory |
|------|----------|
| x64 Release | `..\bin\` |
| x64 Debug | `..\bin\` |
| x86 Release | `..\bin\` |
| x86 Debug | `..\bin\` |

**Skia build options:**
```
llvm.x64.debug
llvm.x64.release
llvm.x86.debug
llvm.x86.release
```

---

### 4. gcc-mingw-w64_build.bat (CMake + MinGW gcc)

**Function:** Build the project using CMake and the MinGW-w64 GCC compiler.

**Prerequisites:**
- MinGW-w64 GCC/G++ compiler
- CMake 3.21+

**Usage:**
```batch
cd nim_duilib\build
gcc-mingw-w64_build.bat        # SDL disabled
gcc-mingw-w64_build.bat -sdl   # SDL enabled
```

**Features:**
- Does not support the CEF module
- Supports WebView2
- Supports optional SDL feature

---

### 5. llvm-mingw-w64_build.bat (CMake + MinGW clang)

**Function:** Build the project using CMake and the MinGW-w64 Clang/LLVM compiler.

**Prerequisites:**
- MinGW-w64 Clang/LLVM compiler
- CMake 3.21+

**Usage:**
```batch
cd nim_duilib\build
llvm-mingw-w64_build.bat        # SDL disabled
llvm-mingw-w64_build.bat -sdl   # SDL enabled
```

**Features:**
- Uses the Clang compiler, better performance
- Does not support the CEF module
- Supports WebView2
- Supports optional SDL feature

---

### 6. build_duilib_all_in_one_mingw-w64.bat (one-click build - MinGW)

**Function:** Fully automated build script that uses the MinGW-w64 compiler to automatically download dependencies and build.

**Prerequisites:**
- git.exe
- python3.exe
- cmake.exe
- GCC/G++ or Clang/Clang++ compiler (choose one)

**Usage:**
```batch
cd nim_duilib\build
build_duilib_all_in_one_mingw-w64.bat      # SDL disabled
build_duilib_all_in_one_mingw-w64.bat -sdl # SDL enabled
```

**Build process:**
1. Detect required software (git, python3, cmake, gcc/clang)
2. Clone the nim_duilib repository
3. Clone the skia_compile tool repository
4. Clone the skia repository
5. Apply Skia patches
6. Build Skia (choose gcc or clang version based on compiler)
7. Optional: clone and build SDL3
8. Call the corresponding MinGW build script

---

## Build Configuration Comparison

| Feature | MSVC | MinGW GCC | MinGW Clang |
|------|------|-----------|-------------|
| Build tool | devenv.exe | mingw32-make.exe | mingw32-make.exe |
| CEF support | ✅ Supported | ❌ Not supported | ❌ Not supported |
| WebView2 support | ✅ Supported | ✅ Supported | ✅ Supported |
| SDL support | ✅ Optional | ✅ Optional | ✅ Optional |
| Static runtime | ✅ /MT | ✅ | ✅ |
| Dynamic runtime | ✅ /MD | ✅ | ✅ |
| One-click build script | ✅ | ❌ | ❌ |

---

## Quick Start

### Method 1: Using the Visual Studio IDE

1. **Build the library only:**
   ```
   Double-click to open duilib.sln or duilib_no_cef.sln
   Select the configuration (Debug/Release) and platform (x64/Win32)
   Build → Rebuild Solution
   ```

2. **Build the complete examples:**
   ```
   Double-click to open examples.sln or examples_no_cef.sln
   Select the configuration and platform
   Build → Rebuild Solution
   ```

### Method 2: Using the one-click build script (recommended for beginners)

```batch
cd nim_duilib\build
build_duilib_all_in_one.bat
```

The script automatically handles:
- Download all dependencies
- Build Skia
- Build duilib and all examples

### Method 3: Using the CMake command line

```batch
cd nim_duilib\build
msvc_build.bat
```

---

## Build Output Description

### Output Directory Structure

```
nim_duilib/
├── bin/                         # Build output directory
│   ├── duilib.dll               # duilib DLL file
│   ├── basic.exe                # Example program
│   ├── controls.exe
│   └── ...
│
├── lib/                         # Static library directory
│   ├── x64/
│   │   ├── duilib.lib
│   │   └── ...
│   └── Win32/
│       └── ...
│
├── build_temp/                   # Build temp directory (safe to delete)
│   ├── msvc/
│   └── ...
│
└── .vs/                          # VS cache directory (clean periodically)
```

### Build Target Files Description

| File type | Path | Description |
|----------|------|------|
| Dynamic library | `bin\duilib.dll` | duilib dynamic-link library (DLL) |
| Static library | `lib\x64\duilib.lib` | duilib static library |
| Executable program | `bin\*.exe` | Example programs |
| Resource files | `bin\resources\` | UI resource files |

---

## Common Issues / Troubleshooting

### Q1: Build reports "Visual Studio 2022 or newer is required"

**Cause:** The detected VS version installed in the system is below 2022.

**Solution:**
1. Install Visual Studio 2022 or later
2. Ensure `detect_vs_version.bat` correctly detects the new version

---

### Q2: Build reports "Skia not found"

**Cause:** Skia is not built or the path is incorrect.

**Solution:**
1. Use `build_duilib_all_in_one.bat` to automatically download and build Skia
2. Or build Skia manually, ensuring output is at `..\..\skia\out\llvm.x64.release`

---

### Q3: Build reports "clang.exe not found"

**Cause:** LLVM Clang is not installed or not in the PATH.

**Solution:**
1. Install LLVM Clang (recommended version 16+)
2. Ensure `C:\LLVM\bin` is in the system PATH
3. Or modify the LLVM path in `build_duilib_all_in_one.bat`

---

### Q4: Build reports "CMake version is too old"

**Cause:** The CMake version is below the requirement.

**Solution:**
- MSVC build requires CMake 3.21+ (4.2+ for VS2026)
- Download the latest CMake: https://cmake.org/download/

---

### Q5: The CEF example program cannot run

**Cause:** The CEF runtime files are not configured correctly.

**Solution:**
1. Download libCEF manually and configure it; see [docs/CEF.md](../docs/CEF.en.md)
2. Copy libcef.dll and related resources to `bin\libcef_win` or `bin\libcef_win_109`

---

## Maintenance Notes

### 1. Cleaning the build cache

Temporary files generated during the build process can be safely cleaned:

```batch
# Clean VS cache
rmdir /s /q nim_duilib\build\.vs

# Clean build temp files
rmdir /s /q nim_duilib\build\build_temp

# Clean CEF runtime cache
rmdir /s /q nim_duilib\cef_cache

# Clean WebView2 cache
rmdir /s /q nim_duilib\webview2_cache
```

### 2. Switching the runtime library mode

**Static runtime (/MT):**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseStaticRuntime.bat
```

**Dynamic runtime (/MD):**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseDynamicRuntime.bat
```

**Important:** After switching, you must close VS and reopen the solution.

### 3. Adding a new example program

1. Create a new project under the `examples\` directory
2. Edit the example's `.vcxproj` file and add the property sheet reference:
   ```xml
   <Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettings.props" />
   ```
3. Add the new project to `examples.sln` and `examples_no_cef.sln`

### 4. Skia build version requirement

duilib depends on a specific version of Skia; the build output directory must be:
```
skia/out/
├── llvm.x64.debug/
├── llvm.x64.release/
├── llvm.x86.debug/
├── llvm.x86.release/
├── mingw64-gcc.x64.release/   # MinGW GCC build
├── mingw64-llvm.x64.release/  # MinGW Clang build
└── ...
```

### 5. Supported compiler versions

| Compiler | Minimum version | Recommended version |
|--------|----------|----------|
| Visual Studio | VS2022 (17.0) | VS2022 / VS2026 |
| LLVM Clang    |      | 16.0+ |
| MinGW GCC     |      | 13.2+ |
| MinGW Clang   |      | 16.0+ |

Note: VS2017/VS2019 are supported in the develop-cpp17 branch, but not in the main branch.

---

## Version Compatibility Matrix

| Feature | MSVC + VS2022/2026 | MinGW GCC | MinGW Clang | Linux GCC/Clang | macOS Clang |
|----------|-------------------|-----------|-------------|-----------------|-------------|
| Core library | ✅ | ✅ | ✅ | ✅ | ✅ |
| Skia rendering | ✅ | ✅ | ✅ | ✅ | ✅ |
| CEF browser | ✅ | ❌ | ❌ | ✅ | ✅ |
| WebView2 | ✅ | ✅ | ✅ | ❌ | ❌ |
| SDL input | ✅ | ✅ | ✅ | ✅ | ✅ |
| Static runtime | ✅ | ✅ | ✅ | ✅ | ✅ |
| Dynamic runtime | ✅ | ❌ | ❌ | ❌ | ❌ |
| C++20 standard | ✅ (VS2022+) | ✅ | ✅ | ✅ | ✅ |
| C++17 standard | ✅ (VS2017/VS2019) | ✅ | ✅ | ✅ | ✅ |

Note: VS2017/VS2019 are supported in the develop-cpp17 branch, but not in the main branch.
