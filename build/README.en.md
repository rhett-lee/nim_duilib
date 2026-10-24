English | [简体中文](README.md)

> Last synced: 2026-10-24

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
│   ├── build_duilib_all_in_one.sh   # One-click build script (Linux/macOS/FreeBSD/MSYS2)
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

### 7. msys2_build.sh (MSYS2 build)

**Function:** Build the third-party libraries, duilib and all examples in MSYS2 using mingw-w64 gcc/g++ or clang/clang++.

**Prerequisites:**
- Run inside an MSYS2 MINGW64 / UCRT64 / CLANG64 shell
- A mingw-w64-target compiler: gcc/g++ or clang/clang++ (`x86_64-w64-mingw32-*` or `x86_64-w64-windows-gnu-*`)
- CMake (3.24+ supports the optional `DUILIB_CMAKE_FRESH=1` reconfigure; older versions still build)
- One of ninja, make or mingw32-make

**Dependencies (pacman):**

Run these in the matching MSYS2 shell. `$MINGW_PACKAGE_PREFIX` is set automatically by
MSYS2 (`mingw-w64-x86_64`, `mingw-w64-ucrt-x86_64` or `mingw-w64-clang-x86_64` for
MINGW64 / UCRT64 / CLANG64), so the environment name does not need to be hardcoded:

```bash
# msys2_build.sh (gcc environments: MINGW64 / UCRT64)
pacman -S --needed $MINGW_PACKAGE_PREFIX-gcc \
                   $MINGW_PACKAGE_PREFIX-cmake \
                   $MINGW_PACKAGE_PREFIX-ninja

# CLANG64: replace gcc with clang, everything else is the same
pacman -S --needed $MINGW_PACKAGE_PREFIX-clang \
                   $MINGW_PACKAGE_PREFIX-cmake \
                   $MINGW_PACKAGE_PREFIX-ninja

# build_duilib_all_in_one.sh also clones dependencies and builds Skia and SDL
pacman -S --needed git unzip curl python \
                   $MINGW_PACKAGE_PREFIX-gn \
                   $MINGW_PACKAGE_PREFIX-ninja
```

- `build_duilib_all_in_one.sh` also runs in MSYS2: it clones the dependencies, builds
  Skia (and optionally SDL), then calls `msys2_build.sh`.
- MSYS2 `base` does not ship cmake / ninja / gn / git / unzip / python3; the scripts
  abort at the startup check or a later step if any of them is missing.
- `python` is an MSYS package providing `/usr/bin/python3`; install it if the `python3`
  command is not available.
- `bc` is no longer required: the CMake version check uses shell built-ins.
- `--fresh` is off by default: it wipes `CMakeFiles` (including Ninja depfiles), turning
  incremental builds into full rebuilds. Build directories are already split per
  compiler and configuration. Set `DUILIB_CMAKE_FRESH=1 ./msys2_build.sh` to force a
  reconfigure (CMake 3.24+ only; older versions skip the flag).

**Usage:**
```bash
cd nim_duilib/build
./msys2_build.sh                            # Release (default), SDL disabled
DUILIB_BUILD_TYPE=Debug ./msys2_build.sh    # Debug
DUILIB_MAKE_JOBS=8 ./msys2_build.sh         # override parallel jobs (default: nproc)
DUILIB_CMAKE_FRESH=1 ./msys2_build.sh       # force reconfigure (full rebuild)
./msys2_build.sh -sdl                       # SDL enabled
```

**Notes:**
- The build type is controlled by `DUILIB_BUILD_TYPE` (`Debug` / `Release` only). Temporary
  build directories are split by configuration under
  `build/build_temp/<compiler>_build/<Debug|Release>/`, so switching configurations never
  reuses a stale generator cache.
- Parallel jobs default to the CPU count (`nproc`), overridable with `DUILIB_MAKE_JOBS`;
  it falls back to 6 when `nproc` is unavailable.
- `build_duilib_all_in_one.sh` also honors `DUILIB_BUILD_TYPE`. Clone/update failures retry
  at most `GIT_RETRY_MAX` times (default 5) so unattended runs cannot loop forever.
- The script pins the CMake generator (Ninja first, then MSYS Makefiles, then MinGW Makefiles).
  A native Windows CMake (e.g. `/mingw64/bin/cmake`) defaults to the Visual Studio
  generator, which silently ignores `-DCMAKE_C_COMPILER` and builds MSVC artifacts.
- The compiler target is validated; MSYS/Cygwin-target gcc/clang are ignored and the
  other compiler is used when available.
- Skia directory: `gcc.<arch>.release` for gcc builds and `llvm.<arch>.release` for
  clang builds; only release variants exist, so Debug builds fall back to release.
- With `-sdl`, the Windows SDL build directory is `SDL.build.msys2.gcc.ninja` or
  `SDL.build.msys2.llvm.ninja`, also pinned to the Ninja generator.
- The script stops at the first failed configure/build (`set -e`) instead of continuing
  with the remaining examples.
- CEF is not supported; WebView2 and optional SDL are supported.

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

### Method 4: Linux / macOS / FreeBSD build scripts

```bash
./build/linux_build.sh                    # Release (default)
DUILIB_BUILD_TYPE=Debug ./build/linux_build.sh   # Debug
```

`macos_build.sh` and `freebsd_build.sh` work the same way. Temporary build
directories are split by configuration under
`build_temp/<compiler>_build/<Debug|Release>/`, so Debug and Release never
overwrite each other. MSYS2's `msys2_build.sh` follows the same layout.
Skia still uses only the release variant on Unix.

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
├── lib/                         # Static library directory (config in subdirectory)
│   ├── x64/
│   │   ├── Release/
│   │   │   ├── duilib.lib
│   │   │   ├── duilib-zlib.lib
│   │   │   └── ...
│   │   └── Debug/
│   │       └── ...
│   └── x86/
│       ├── Release/
│       └── Debug/
│
├── build_temp/                   # Build temp directory (safe to delete)
│   ├── x64/                      # VS
│   ├── msvc/                     # CMake
│   └── ...
│
└── .vs/                          # VS cache directory (clean periodically)
```

Linux / macOS / FreeBSD have no architecture level: `lib/Debug/`, `lib/Release/`.

### Build Target Files Description

| File type | Path | Description |
|----------|------|------|
| Dynamic library | `bin\duilib.dll` | duilib dynamic-link library (DLL) |
| Static library | `lib\x64\<Release\|Debug>\duilib.lib` | duilib static library |
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

### Q6: No text is rendered on Linux (font list is empty)

**Cause:** Skia was not built with the fontconfig font backend enabled. On platforms that are
neither Windows nor macOS, `FontMgr_Skia.cpp` calls `SkFontMgr_New_FontConfig`; if Skia was built
without that backend it **silently returns an empty font set** — no error, no crash, just no glyphs
at all. This is easily mistaken for a layout or theme-color problem.

The Linux branches of `build_duilib_all_in_one.sh` (both the clang and gcc paths) pass
`skia_enable_fontmgr_fontconfig=true skia_use_freetype=true` explicitly, so the build does not rely
on Skia's upstream defaults. If you compile Skia yourself, include both flags.

**How to diagnose:** search the startup log for `[WARN] FontManager::GetFontNameList`. That warning
confirms the font backend is the cause (on Windows it goes to the debugger; on Linux it is written to
stderr via `SDL_Log`).

---

## Release packaging

After building, package with `build/pack.py`. The script only collects
files that have already been built; it does not trigger compilation.
Official releases use `build/release.py`, which orchestrates `pack.py` and
verifies versions and artifacts, but never runs commit / tag / push.

```
# SDK only (recommended, Release by default)
python build/pack.py

# Debug SDK (package name gains -debug)
python build/pack.py --config Debug

# SDK plus example programs (package name gains -examples)
python build/pack.py --with-examples

# list what would be collected without packaging
python build/pack.py --list

# choose the MSVC runtime recorded in package metadata
# (auto by default: MD for DLL mode, otherwise MT)
python build/pack.py --msvc-runtime md

# release: check, package, verify (artifacts go to dist/<version>/)
python build/release.py check
python build/release.py package --config Release --arch x64 --dry-run
python build/release.py finalize
python build/release.py clean --list
```

Running `pack.py` directly writes to `dist/`; `release.py package` always writes
to `dist/<version>/` (`dist/` is already in .gitignore). Package names are
`nim_duilib-<version>-<system>-<arch>[-sdl][-cef|-cef109][-examples][-debug]`,
containing `include/duilib/`, `lib/` (the 5 duilib libraries plus all Skia libraries
and WebView2/JPEG link libraries), `bin/` (runtime DLLs and `bin/resources/`),
`licenses/`, `docs/` (top-level user documentation with links rewritten for the
package layout), plus `MANIFEST.txt` and `SHA256SUMS`.

The package also ships a **downstream integration layer**, so consumers do
not have to assemble include paths, link lists and defines manually:

- `msvc/` (Windows + MSVC only): `nim_duilib.props`, `nim_duilib.targets`,
  `nim_duilib.sdk.props` and `manifest/` (both x64 and x86). Import
  `nim_duilib.targets` once at the end of a `.vcxproj` to get compile/link
  settings, the manifest, and runtime DLL copying.
- `lib/cmake/nim_duilib/` (all platforms): `find_package(nim_duilib CONFIG
  REQUIRED)` provides `nim_duilib::duilib` and
  `nim_duilib_copy_runtime(<target> [RESOURCES])`.
- Both validate architecture/configuration/toolchain. Packages are
  single-arch and single-config, so Debug and Release builds must use
  their own packages. The MSVC runtime is aligned to the package by default
  (can be disabled).

Minimal integration examples (see
[Release and Packaging](../docs/Release.en.md#downstream-integration) for the
full examples and property reference):

Visual Studio, placed at the end of the `.vcxproj` before `Microsoft.Cpp.targets`:

```xml
<PropertyGroup>
  <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  <NimDuilibCopyResources>1</NimDuilibCopyResources>
</PropertyGroup>
<Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />
```

CMake:

```cmake
list(APPEND CMAKE_PREFIX_PATH "C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug")
find_package(nim_duilib CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)
nim_duilib_copy_runtime(my_app)
```

Easy mistakes to avoid:

- Headers must keep the `include/duilib/xxx.h` hierarchy, since downstream
  code uses `#include "duilib/duilib.h"`. Flattening breaks every build.
- Image decoder implementation headers are not packaged
  (`Image/ImageDecoder_*.h`, `Image/Image_*.h`, `APngDecoder.h`,
  `FrameSequence_gif.h`, etc.), and neither are the internal
  `Core/ZipStreamIO.h`, `Core/ControlDropTarget*`, `Core/WindowDropTarget_*`
  or `Core/SDL_MacOS.h`, nor the `.cpp`-only `Core/SDL_Linux.h`,
  `Core/MessageLoop_*`, `Utils/ProcessSingleton_*`, `Utils/BitmapHelper_*`,
  `Utils/ApiWrapper_Windows.h`, `Utils/Macros_Windows.h`, `Utils/DiskUtils_Windows.h`,
  `Utils/DllManager_Windows.h`, `Utils/InlineHook_Windows.h` and
  `Control/RichEdit*_Windows/_SDL`. From `third_party`, only WebView2 (Windows only)
  headers are forwarded; `zlib`, `libpng`, `giflib` and `libjpeg-turbo`
  headers are no longer forwarded (their static libraries are still shipped).
- `Core/DragWindowFilter_SDL.h` is the exception: always kept on non-Windows,
  and on Windows packaged only with `--with-sdl` (`ControlDragable.h` includes
  it under `DUILIB_BUILD_FOR_SDL`).
- Windows packages carry `bin/WebView2Loader.dll` for the dynamic loader; packaging
  fails when any of `duilib`, `duilib-cximage`, `duilib-png`, `duilib-webp` or
  `duilib-zlib` is missing (no longer a warning).
- MinGW builds produce `libduilib*.a` (static) or `libduilib*.dll.a` (import)
  libraries, and Skia lives in `mingw64-gcc.<arch>.release` /
  `mingw64-llvm.<arch>.release`. MSYS2 builds (`msys2_build.sh`) use
  `gcc.<arch>.release` / `llvm.<arch>.release` instead, and the MSYS2 MINGW64
  environment (msvcrt) uses `gcc.msvcrt.<arch>.release` because its gcc
  artifacts cannot be mixed with UCRT64 ones. Packaging probes these namings
  and uses the presence of `.a` files to tell a MinGW directory apart
  from the identically named MSVC `llvm.<arch>.release` (a `.lib`-only
  directory is skipped). When several directories of the same variant exist, the
  standalone MinGW name wins, and the msvcrt one wins under MINGW64; pass
  `--skia-lib-dir` to force a specific directory.
  The script detects the toolchain (MSVC/MinGW) from the library directory and
  writes `-lxxx` links for MinGW in the manifest.
- In DLL mode (`msvc/PropertySheets/duilib_dll.flag` exists) `bin/duilib*.dll`
  (`duilib[_d].dll` on x64/arm64, `duilib32[_d].dll` on x86) is packaged and
  `lib/` holds the matching import library; static builds have no such file and
  are skipped automatically. The package name gains `-dll` so it never
  overwrites the static package. DLL mode links with `/MD`, so Skia must match:
  Release uses `llvm.<arch>.release.md`, Debug uses
  `llvm.<arch>.debug.mdd` (not provided yet); the packer picks them
  automatically.
- With `--with-sdl`, SDL3 is collected based on the actual artifact: MSVC static
  `SDL3-static.lib`; dynamic import `SDL3.lib` / `libSDL3.dll.a` plus
  `bin/SDL3.dll`; or MinGW static `libSDL3.a`.
- There is only one library layout: `lib/<arch>/<Release|Debug>/` on Windows
  and `lib/<Release|Debug>/` on Unix. The configuration comes from `--config`,
  not from a `_d` suffix.
- Debug libraries carry the debug CRT and may only serve Debug downstream
  builds; mixing them into a Release SDK breaks linking.
- SDL3 is not packaged on Windows by default (use `--with-sdl`); CEF (headers,
  libraries and runtime) requires `--with-cef`, with the runtime kept under
  `bin/libcef_win[/_109]/<arch>/` in the package.
- Archive permissions and symlinks: tar entries use the source permissions
  (executable/setuid preserved) and zip stores symlinks as `S_IFLNK`; `.so`
  chains and executable bits survive extraction, and symlinks are excluded from
  `SHA256SUMS`.
- Reproducible builds: `SOURCE_DATE_EPOCH` normalizes entry times, the gzip
  header and the MANIFEST build time; without it, entries use source/artifact
  mtimes and are sorted, so unchanged input packs byte-identically.

See [docs/Release.en.md](../docs/Release.en.md) for the full description.

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
├── llvm.x64.debug/            # MSVC Debug
├── llvm.x64.release/          # MSVC Release (same name as the MSYS2 clang dir, can coexist)
├── llvm.x86.debug/
├── llvm.x86.release/
├── mingw64-gcc.x64.release/   # standalone MinGW GCC build
├── mingw64-llvm.x64.release/  # standalone MinGW Clang build
├── gcc.x64.release/           # MSYS2 GCC build
└── llvm.x64.release/          # MSYS2 Clang build
```
Notes:
- MSYS2 clang and MSVC both use `llvm.<arch>.release`, but the library file names
  differ (`lib*.a` vs `*.lib`), so both can coexist in that directory.
- MSYS2 only produces release variants; Debug builds fall back to the release
  directory of the same compiler.
- When no matching directory is found, CMake fails at configure time instead of
  proceeding to compilation.

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
