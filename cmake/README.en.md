English | [简体中文](README.md)

> Last synced: 2026-10-31

# nim_duilib CMake Project Configuration Guide

This document details the purpose and usage of the CMake configuration files (*.cmake) in the `nim_duilib\cmake` directory, for reference by maintainers.

## Directory File Overview

```
cmake/
├── README.md / README.en.md        # this document (overview, options, troubleshooting)
├── third_party_versions.json       # Skia / SDL3 / CEF version manifest (single source of truth)
├── i18n/messages.json              # build message catalog (English + Chinese, single source)
├── macos/Info.plist.in             # Info.plist template for macOS app bundles
│
├─ Shared layer (library subprojects, examples and the root project)
│  ├── duilib_common.cmake                 # common configuration (OS/compiler/CPU detection, paths)
│  ├── duilib_bin.cmake                    # executable base configuration (C++20, paths, output dirs)
│  ├── duilib_bin_windows.cmake            # Windows specifics (runtime library, manifest, WebView2)
│  ├── duilib_bin_linux.cmake              # Linux specifics
│  ├── duilib_bin_macos.cmake              # macOS specifics (frameworks, app-bundle entry)
│  ├── duilib_bin_freebsd.cmake            # FreeBSD specifics
│  ├── duilib_compiles.cmake               # compile feature detection (AVX/AVX2)
│  ├── duilib_compiler_flags.cmake         # shared GCC/Clang flags (Unix libraries and examples)
│  ├── duilib_unix_font_libs.cmake         # Linux/FreeBSD font dependencies (Freetype, Fontconfig)
│  ├── duilib_lib_output.cmake             # library output rule (lib/<arch>/<Config>; config in the path)
│  ├── duilib_log_variables.cmake          # DUILIB_LOG variable dump (included at the end of common)
│  ├── duilib_example.cmake                # shared example implementation ("declare + include once")
│  ├── duilib_write_build_stamp.cmake      # POST_BUILD stamp in bin/.build-config/<target>
│  ├── duilib_cef_macos.cmake              # macOS CEF specifics (framework, helpers, resources, rpath)
│  ├── duilib_macos_bundle.cmake           # macOS app-bundle setup for plain examples
│  ├── duilib_copy_bundle_resources.cmake  # incremental resource sync into the bundle (every build)
│  └── duilib_copy_sdl_bundle.cmake        # copy SDL3 dylibs into Contents/Frameworks and set rpath
│
├─ Root project orchestration (used by the repository-root CMakeLists.txt)
│  ├── duilib_i18n.cmake                   # language resolution + message catalog (include or cmake -P)
│  ├── duilib_bootstrap.cmake              # tool bootstrap: find ninja in VS dirs or the patch archive
│  ├── duilib_root_options.cmake           # platform/compiler/arch detection + cached options (after project())
│  ├── duilib_preflight_tools.cmake        # phase A preflight (no compiler): tools and system libs, read-only
│  ├── duilib_preflight.cmake              # phase B preflight (with compiler): deterministic checks
│  ├── duilib_deps.cmake                   # dependency check/prepare targets (DUILIB_DEPS_MODE=off/prepare/auto)
│  ├── duilib_deps_run.cmake               # dependency script (cmake -P): download, build, install Skia/SDL3/CEF (plan mode prints commands only)
│  ├── duilib_download.cmake               # download helpers (curl first, resume/retry, file(DOWNLOAD) fallback)
│  └── duilib_cef.cmake                    # CEF runtime download/verify/extract/copy (Windows and Linux)
│
└─ Naming rules (dependency preparation, configure-time resolution, off-mode checks, packaging)
   ├── duilib_skia_variants.cmake         # single rule for Skia variant directory names
   ├── duilib_skia_toolchain.cmake        # Skia toolchain selection + duilib_detect_msys2() (MSYS2 detection)
   ├── duilib_sdl_variants.cmake          # single rule for SDL3 variant prefixes (arch × config × runtime)
   └── duilib_crt.cmake                   # single rule for the CRT id (msvc-md / msvc-mt / <env>[-msvcrt] / system)
```

In the last group, the three "naming rules" files `duilib_skia_variants.cmake`,
`duilib_sdl_variants.cmake` and `duilib_crt.cmake` are the **only implementations**:
configure-time resolution (`duilib_common.cmake`, `duilib_root_options.cmake`) and
dependency checks/preparation (`duilib_deps*.cmake`) must reuse them — do not build
directory names anywhere else. `duilib_skia_toolchain.cmake` is likewise the single
implementation point for Skia build-toolchain selection and `duilib_detect_msys2()`
(distinguishing MSYS2 from standalone MinGW-w64). The packaging side (`build/pack.py`)
and the VS property sheets (`msvc/PropertySheets/*.props`) cannot include CMake
scripts directly, so they mirror the same rules;
`build/tests/skia_variants_check.cmake` (a `cmake -P` fixture)
together with `build/test_cmake_scripts.py` (the Python implementation) compare both
sides case by case so they cannot drift apart again.

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
DUILIB_COMPILER_NAME    # Compiler name (e.g. "msvc", "mingw64-gcc", "mingw64-llvm"; "gcc"/"llvm" on MSYS2)
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
| `DUILIB_SKIA_LIB_SUBPATH` | empty | Skia library subdirectory; empty means auto-concatenated per platform and configuration (see below) |
| `DUILIB_ENABLE_SDL` | Windows=OFF, others=ON | Enable SDL input support |
| `DUILIB_SDL_AUTOBUILD` | OFF | Dependency auto-prepare mode (`DUILIB_DEPS_MODE=auto`): skip the SDL3 existence check and predict the variant path |
| `DUILIB_ENABLE_CEF` | Windows MSVC/Linux/macOS=ON, MinGW/FreeBSD=OFF | Enable CEF browser support (the library compiles CEF control sources); unsupported platforms are forced off with a warning |
| `DUILIB_EXAMPLE_CEF` | OFF | Example links CEF (set only by `cef`/`CefBrowser`) |
| `DUILIB_CEF_109` | OFF | Use CEF 109 (supports Win7); deprecated, equivalent to `-DDUILIB_CEF_VERSION=109` and prints a warning when set |
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
| `DUILIB_SDL_ROOT` | SDL3 installation root; on Windows prefixes are selected as `<root>/<variant>`, and an explicit value still acts as the root with a single-prefix fallback |
| `DUILIB_SDL_INCLUDE_DIR` | SDL3 include directory (taken from the selected variant prefix) |
| `DUILIB_SDL_SRC_ROOT_DIR` | SDL3 installation prefix directory (derived from the selected variant prefix) |
| `DUILIB_SDL_LIB_PATH` | SDL3 library directory (a configuration-dependent generator expression for multi-config generators) |
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

`DUILIB_SKIA_LIB_SUBPATH` defaults to **empty**; `duilib_common.cmake` concatenates the path
from the platform and configuration, so individual projects no longer need to set it:

```cmake
# Default: empty (no longer pre-set to llvm.<arch>.<build type>)
# An external -DDUILIB_SKIA_LIB_SUBPATH=... always wins (a fixed path that does not follow the configuration).

# When the subdirectory name is empty, the automatic rule is
# (the resolved directory must exist, otherwise configuration fails immediately;
#  MinGW builds also require *.a static libraries in the directory, so a same-named
#  directory holding MSVC .lib files is not mistaken for a usable one):
#   Windows MSVC / clang-cl : llvm.<CPU_ARCH>.debug / llvm.<CPU_ARCH>.release
#             Debug must match the debug variant (a Debug build cannot link release
#             Skia); a missing directory is a hard error
#   Windows MinGW gcc       : mingw64-gcc.<CPU_ARCH>.debug / .release (gcc.<arch>.* on MSYS2)
#             falls back to the same compiler's release variant when debug is missing;
#             gcc- and llvm-built Skia cannot be mixed (libstdc++/libc++ are incompatible
#             at link time)
#   Windows MinGW llvm      : mingw64-llvm.<CPU_ARCH>.debug / .release (llvm.<arch>.* on MSYS2)
#             falls back to the release variant
#   Linux/macOS/FreeBSD : <compiler>.<CPU_ARCH>.release
#             Skia is only built as release on Unix (D-3)

# MSYS2 and standalone MinGW-w64 are distinguished by the compiler installation path
# (an MSYS2 root contains usr/bin/pacman.exe); MSYSTEM is not used because Git Bash
# sets MSYSTEM=MINGW64 too. The matching prefix is preferred, the other one is a fallback
# (so a build still works when only one set of Skia libraries is present).

# Final Skia library directory:
${DUILIB_SKIA_SRC_ROOT_DIR}/out/<subdirectory>

# Examples:
# Windows x64 + Debug      -> skia/out/llvm.x64.debug
# Windows x64 + Release    -> skia/out/llvm.x64.release
# Windows -A Win32 + Debug -> skia/out/llvm.x86.debug
# MinGW gcc + Debug (no gcc debug)  -> skia/out/mingw64-gcc.x64.release
# MinGW gcc + Debug (no gcc Skia)   -> configuration error (llvm-built Skia cannot be linked)
# Linux/macOS/FreeBSD      -> skia/out/llvm.x64.release (clang)
#                            skia/out/gcc.x64.release (gcc)
# MSYS2 Windows + Debug    -> skia/out/gcc.<arch>.debug first, then gcc.<arch>.release
# -DDUILIB_SKIA_LIB_SUBPATH=llvm.x64.release -> both configurations use that fixed directory
#                                              (its existence is validated as well)
# MinGW pointing at an MSVC-only directory (.lib, e.g. the MSVC llvm.x64.release) -> configure error
```

**SDL3 installation prefix rule:**

On Windows the SDL3 static libraries must be separated by architecture × configuration ×
MSVC runtime; otherwise Debug/Release overwrite each other (same file names) and mixing
`/MT` with `/MD` fails at link time with LNK2038. `duilib_deps` installs each combination
into its own complete prefix (`<deps-root>/SDL3/<variant>`, with `include/`, `lib/` and
`share/licenses`). The naming lives in
[duilib_sdl_variants.cmake](duilib_sdl_variants.cmake) and is reused by
`duilib_common.cmake`, the dependency checks, `build/pack.py` and the VS property sheet:

```cmake
#   Windows MSVC  : msvc.<arch>.debug / msvc.<arch>.release
#                   plus .mdd / .md when /MD, /MDd is used (DUILIB_MD, implied by DLL mode)
#   Windows MinGW : reuses the Skia release variant names (mingw64-<gcc|llvm>.<arch>.release, ...);
#                   Debug/Release share one static library (the CRT comes from the toolchain)
#   non-Windows   : a single <deps-root>/SDL3 prefix (Release shared library only)

# Candidate order: <root>/<variant> first, then <root> as the legacy/self-built single prefix;
# build.py deps installs as described above and pack.py selects in the same order;
# an explicit --sdl-dir is treated as a single prefix (no variant derivation)
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

**Library output directories (CM-5 / D-2 option B):**

Each library includes [duilib_lib_output.cmake](duilib_lib_output.cmake) after `project()`.
Configuration is expressed by the directory only (library names no longer carry `_d`):

| Platform | Library directory |
| :--- | :--- |
| Windows | `<root>/lib/<x64\|x86\|arm64>/<Debug\|Release\|RelWithDebInfo\|MinSizeRel>/` |
| Linux / macOS / FreeBSD | `<root>/lib/<Debug\|Release\|RelWithDebInfo\|MinSizeRel>/` |

- The Windows arch comes from `CMAKE_GENERATOR_PLATFORM` (`Win32 → x86`, default x64).
- `DUILIB_LIB_PATH` is `${DUILIB_LIB_BASE}/$<CONFIG>`, expanded by the generator.
- A given `<arch>|<Config>` is fully built by only one build system (CMake or VS) at a time;
  the later build overwrites the earlier one.

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
if(DUILIB_MD)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")  # MD/MDd
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
if(DUILIB_TARGET_ARCH_RESOLVED STREQUAL "arm64")
    set(DUILIB_WIN_MANIFEST "${DUILIB_ROOT}/msvc/manifest/duilib.arm64.manifest")
elseif(DUILIB_BITS_64)
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
# Only CEF examples (cef/CefBrowser) link CEF; others must not import libcef.so
set(DUILIB_EXAMPLE_CEF_LIBS "")
if(DUILIB_ENABLE_CEF AND DUILIB_EXAMPLE_CEF)
    list(APPEND DUILIB_TARGET_INCLUDE_DIRS "${DUILIB_CEF_SRC_ROOT_DIR}")
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_CEF_LIB_PATH}")
    set(DUILIB_EXAMPLE_CEF_LIBS ${DUILIB_CEF_LIBS})
endif()

# Linux system dependency libraries
set(DUILIB_LINUX_LIBS X11 freetype fontconfig pthread dl)

# Link command
target_link_libraries(${PROJECT_NAME} ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_EXAMPLE_CEF_LIBS} ${DUILIB_LINUX_LIBS})
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

#### 5.4 App Bundles for plain examples

`duilib_macos_bundle.cmake` is called when an example sets
`DUILIB_EXAMPLE_MACOS_BUNDLE=ON`. It creates the target as a `MACOSX_BUNDLE` and:

- generates `Info.plist` (template `cmake/macos/Info.plist.in`), identifier and versions;
- copies `bin/resources` into `Contents/Resources/duilib/resources` (isomorphic to
  the plain `<exe directory>/resources/`; `GetResourceRootPath(true)` locates it on
  macOS, so no C++ changes are needed). The resource list is derived from the
  example name (`project()`), so nothing has to be configured; when the target
  name differs from the resource directory (`ChildWindow`→`child_window`), set
  `DUILIB_THEME_DIR_NAME` explicitly — it takes precedence and is validated; use
  `DUILIB_MACOS_BUNDLE_RESOURCES` to override the whole list;
- copies SDL3 dylibs into `Contents/Frameworks` and sets the rpath to
  `@executable_path/../Frameworks`;
- removes the stale artifact of the other mode when switching between bundle and
  plain executable.

The root option `DUILIB_MACOS_BUNDLE_EXAMPLES` can also select targets. See
[build/macos-app-bundle.en.md](../build/macos-app-bundle.en.md) for details.

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

## Root Project Scripts (dependency preparation and shared infrastructure)

These files are used by the **repository-root `CMakeLists.txt`**; library subprojects
(`duilib/`) and examples (`examples/*`) do not need them directly (examples reach the
shared layer through `duilib_example.cmake`). In execution order:

| File | When | Responsibilities and entry points |
| :--- | :--- | :--- |
| `duilib_i18n.cmake` | before `project()` | Language resolution (`--lang`/`DUILIB_LANG` → `LC_ALL`/`LC_MESSAGES`/`LANG` → system locale → en; only `zh`/`zh_CN`/`zh_Hans` are Chinese) and message catalog loading. Works with `include` and with `cmake -P`; entry points `duilib_t()` / `duilib_msg()` / `duilib_warn()` / `duilib_error()` |
| `duilib_bootstrap.cmake` | before `project()` | Tool bootstrap: when ninja is not on PATH, `duilib_bootstrap_ninja(<patch_dir> <patch_zip>)` picks it up from the Visual Studio install or the Skia patch archive's `bin/ninja.exe` |
| `duilib_preflight_tools.cmake` | before `project()` (`cmake -P`) | Phase A preflight: probes tools (cmake/ninja/git/python, `vswhere`) and system libraries only — no downloads, no installs, no file changes |
| `duilib_root_options.cmake` | after `project()` | Platform/compiler/architecture detection and the unified cache options (they override the subprojects' `option()` defaults); also hoists `DUILIB_MINGW`/`DUILIB_MSYS2_ENV` and resolves the SDL3 variant prefix per configuration |
| `duilib_preflight.cmake` | after `project()` | Phase B preflight: deterministic checks once the compiler is known (architecture, toolchain, system libraries), all errors reported together |
| `duilib_deps.cmake` | after `project()` | Creates the dependency check/prepare targets (`duilib_configure_dependencies()` / `duilib_link_dependency_targets()`); `DUILIB_DEPS_MODE=off/prepare/auto`. Also provides the read-only `duilib_deps_plan` target (`duilib_add_dependency_plan_target()`), created in every mode |
| `duilib_deps_run.cmake` | build time (`cmake -P`) | The actual preparation: download, verify, build and install Skia/SDL3/CEF; each SDL3 variant is installed into its own prefix and stamped with `.duilib-sdl`/`.duilib-crt`; shared by the `duilib_deps` target and `build.py deps`. With `-DDUILIB_DEPS_PLAN=ON` it only prints the actual Skia/SDL3 command lines (no directories, no lock, no clone, no compile) |
| `duilib_download.cmake` | `cmake -P` | Download helpers: curl first (resume and retries), otherwise `file(DOWNLOAD)`; entry points `duilib_download_file()` / `duilib_verify_sha256()` / `duilib_verify_sha1()` |
| `duilib_cef.cmake` | `cmake -P` | Downloads, verifies, extracts and copies the CEF runtime (Release/Resources) per platform; writes the `.duilib-cef` fingerprint |

Dependency directories (`<deps-root>` defaults to the repository's parent; override with
`-DDUILIB_DEPS_ROOT=` or `build.py --deps-root`):

| Directory | Contents |
| :--- | :--- |
| `<deps-root>/skia/` | Skia sources and `out/<variant>/` (rules in `duilib_skia_variants.cmake`) |
| `<deps-root>/SDL/` | SDL3 sources (not downloaded when SDL is off, the Windows default) |
| `<deps-root>/SDL3/<variant>/` | Complete install prefix of each SDL3 variant (`include/`, `lib/`, `share/licenses`); a single `<deps-root>/SDL3/` prefix on non-Windows |
| `<deps-root>/SDL3_build/` | SDL3 build trees (per architecture/configuration; rebuilt from scratch when the stamp does not match) |
| `<deps-root>/cef_binary/` | CEF download cache and extracted runtime |

**Plan mode (print dependency commands only, execute nothing):** `build.py deps
--dry-run` additionally runs the read-only `duilib_deps_plan` target (equivalent to
`cmake -P duilib_deps_run.cmake -DDUILIB_DEPS_PLAN=ON`) and prints the real Skia and
SDL3 command lines, which makes manual reproduction and troubleshooting easier:

```text
[plan]   cd <deps-root>/skia
[plan]   <gn> gen out/<variant> "--args=<gn args>"
[plan]   <ninja> -C out/<variant> -j <jobs>
[plan]   <cmake> -S <SDL sources> -B <SDL3_build>/<arch>-<cfg> -DCMAKE_INSTALL_PREFIX=...
[plan]   <cmake> --build <SDL3_build>/<arch>-<cfg> --parallel <jobs>
[plan]   <cmake> --install <SDL3_build>/<arch>-<cfg> --config <cfg>
```

- The arguments are identical to the real preparation (both come from the same set
  of configure-time `-D` values); no naming rule is duplicated.
- Plan mode is read-only: it does not create the dependency directory, lock a file,
  clone/switch sources, compile anything or write stamps. Already-ready variants are
  labelled "skipped when actually running", but their commands are still listed.
- It works even when the build tree was configured with `DUILIB_DEPS_MODE=off`
  (plan mode ignores that mode).
- Values come from the **last real configuration** cached in the build tree
  (`DUILIB_MD`, `DUILIB_SDL_ROOT`, ...); `--dry-run` does not reconfigure. When the
  current command line differs from the cache, `deps.plan_cache_stale` is printed with
  the differences (e.g. `DUILIB_MD=ON (now OFF)`), along with a hint to run `build.py
  deps` once without `--dry-run` first.
- The CEF runtime is handled by download/extract, so plan mode only prints a hint
  instead of the download commands.

**Build message catalog (`i18n/messages.json`):** both the CMake side
(`duilib_i18n.cmake`) and the Python side (`build/i18n.py`, used by `build.py`,
`pack.py`, `release.py`, ...) read messages from this single file; add or change
a message here only:

- Layout: `{"messages": {"<key>": {"en": "...", "zh_CN": "..."}}}`. Every key **must
  provide both `en` and `zh_CN`; English is the fallback language, so a missing
  `zh_CN` entry falls back to `en` on both sides.
- Keys use the `area.purpose` form (e.g. `preflight.cmake_too_old`,
  `preflight.cmake_too_old_hint`); an error message comes with a `_hint` key, and
  `duilib_error()` / `Messages.hint()` render it as a second `Hint:` line.
- On the CMake side `duilib_msg()` / `duilib_warn()` append dynamic content as their
  second argument (such messages must not contain `%s`); on the Python side
  (`Messages.t()`) messages keep `%s` placeholders and the caller interpolates with
  `% (...)` (e.g. `pkg.cef_lib_version_hint`). A missing key prints the key itself
  on both sides, which makes gaps easy to spot.
- Language resolution is identical on both sides: `--lang`/`DUILIB_LANG` →
  `LC_ALL`/`LC_MESSAGES`/`LANG` → system locale → `en`; only `zh`/`zh_CN`/`zh_Hans`
  use Chinese, while `zh_TW`/`zh_HK`/`zh_Hant` use English. A non-UTF-8 Windows
  console (no `chcp 65001`) falls back to English and prints
  `lang.non_utf8_fallback`; the Python entry points call `set_console_utf8()` first
  to switch the console code page to 65001 (this only changes the encoding, not the
  language; if the switch fails Chinese may render as mojibake, so run `chcp 65001`
  beforehand).

The command line is always `python3 build/build.py` (`deps` prepares everything); see
[build/build-reference.md](../build/build-reference.md) for options and troubleshooting.

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

**Debug builds on Unix** (single-config generator; the build and artifact directories are
separate from Release):

```bash
cmake -S .. -B ./build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build_debug
```

For everyday builds use the unified entry (it prepares Skia/CEF automatically; SDL is off by default on Windows, pass `--sdl` if needed):

```bash
python3 build/build.py --config Debug
```

The build tree is always `out/<arch>/<Config>`; see `build/build-reference.en.md`.

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

#### 6. MSVC dynamic runtime and DLL mode

```bash
# Dynamic runtime (static libraries)
cmake -S .. -B . -DDUILIB_MD=ON -DCMAKE_BUILD_TYPE=Release

# Build duilib as a DLL (implies /MD), producing bin/duilib*.dll + import lib
cmake -S .. -B . -DDUILIB_DLL=ON -DCMAKE_BUILD_TYPE=Release
```

`DUILIB_DLL` supports Windows MSVC only and enables `DUILIB_MD` automatically.
The shared duilib links Skia/SDL/CEF and the third-party static libraries itself;
examples link only the import library. Output names match MSBuild DLL mode (x86:
`duilib32*.dll`, Debug adds `_d`, the import library is always `duilib.lib`).
After switching between `MT`/`MD`/DLL modes, every target including the CEF
wrapper must be recompiled. `build.py --dll` records and cleans the build tree
automatically; when using `cmake` directly, delete the build directory and
reconfigure.

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
| `DUILIB_SKIA_LIB_SUBPATH` | STRING | empty | Skia library subdirectory; empty means auto-concatenated per platform/configuration |
| `DUILIB_ENABLE_SDL` | BOOL | Windows=OFF, others=ON | Enable SDL support |
| `DUILIB_ENABLE_CEF` | BOOL | Windows MSVC/Linux/macOS=ON, MinGW/FreeBSD=OFF | Enable CEF support |
| `DUILIB_CEF_109` | BOOL | OFF | CEF 109 (Win7); deprecated, equivalent to `DUILIB_CEF_VERSION=109` |
| `DUILIB_WEBVIEW2_EXE` | BOOL | OFF | WebView2 executable program |
| `DUILIB_MD` | BOOL | OFF | MSVC dynamic runtime (/MD) |
| `DUILIB_DLL` | BOOL | OFF | Build duilib as a DLL (Windows MSVC only, implies `DUILIB_MD=ON`) |
| `DUILIB_MINGW_STATIC` | BOOL | ON | MinGW static linking |
| `DUILIB_SDL_AUTOBUILD` | BOOL | OFF | Internal signal (not a user option): set by `duilib_deps.cmake` when `DUILIB_DEPS_MODE=auto` to skip the SDL3 existence check |

**Root project options (`cmake/duilib_root_options.cmake`; `DUILIB_LANG`,
`DUILIB_PREFLIGHT` and `DUILIB_PREFLIGHT_ONLY` are declared in the root `CMakeLists.txt`
because they are needed before `project()`):**

| Option | Type | Default value | Description |
|----------|------|--------|------|
| `DUILIB_LANG` | STRING | auto | Interaction language: `auto`/`zh_CN`/`en` (`auto` reads the environment variable of the same name, then falls back to `LC_ALL`/`LC_MESSAGES`/`LANG` and the system locale) |
| `DUILIB_PREFLIGHT` | BOOL | ON | Two-stage environment preflight (phases A/B); turning it off skips tool probing, but architecture and compiler checks still run |
| `DUILIB_PREFLIGHT_ONLY` | BOOL | OFF | Run the preflight only, then exit without generating build targets |
| `DUILIB_TARGET_ARCH` | STRING | auto | Target architecture: `auto`/`x86`/`x64`/`arm64` (`auto` follows the current toolchain) |
| `DUILIB_BUILD_THIRD_PARTY` | BOOL | ON | Build zlib/libpng/cximage/libwebp |
| `DUILIB_BUILD_EXAMPLES` | BOOL | ON | Build the example applications |
| `DUILIB_EXAMPLES` | STRING | derived per platform | Examples to build (semicolon/comma separated, `all` = every one); re-derived when the CEF switch changes so stale cache values cannot keep CEF-only examples in the list |
| `DUILIB_MACOS_BUNDLE_EXAMPLES` | STRING | empty | Examples built as `.app` bundles (macOS only; `all`/`*` = every one) |
| `DUILIB_DEPS_MODE` | STRING | off | `off` checks only; `prepare` creates dependency targets only (the recommended two-phase flow); `auto` prepares during the build |
| `DUILIB_DEPS_PLAN` | BOOL | OFF | Internal switch (not a user option): the `duilib_deps_plan` target uses it to print the Skia/SDL3 commands without doing anything |
| `DUILIB_DEPS_ROOT` | PATH | `<source root>/..` | Dependency root (Skia/SDL3/CEF live below it) |
| `DUILIB_SDL_ROOT` | PATH | `<deps-root>/SDL3` | SDL3 install root; on Windows the prefix is chosen as `<root>/<variant>` (rule above) |
| `DUILIB_SDL_BUILD_ROOT` | PATH | `<deps-root>/SDL3_build` | SDL3 build trees (per architecture/configuration) |
| `DUILIB_SKIA_REF` / `DUILIB_SDL_REF` | STRING | manifest default | Version keys in `cmake/third_party_versions.json` (commit/branch/tag) |
| `DUILIB_CEF_VERSION` | STRING | manifest default | `109` / `156`; `DUILIB_CEF_109=ON` is equivalent to selecting 109 |
| `DUILIB_OFFLINE` | BOOL | OFF | Use only cached dependency sources/archives (no network) |
| `DUILIB_DEPS_AUTO_CLONE` | BOOL | ON | Allow `git clone` while preparing dependencies |
| `DUILIB_DEPS_FORCE` | BOOL | OFF | Rebuild dependencies even when the stamps match |
| `DUILIB_DEPS_JOBS` | STRING | 6 | Parallel jobs for the Skia/SDL3 dependency builds |
| `DUILIB_DEPS_LOCK_TIMEOUT` | STRING | 5 | Dependency lock timeout in seconds (prevents concurrent preparation) |
| `DUILIB_DOWNLOAD_RETRIES` / `DUILIB_DOWNLOAD_RETRY_DELAY` | STRING | manifest `retry` (0 = unlimited / 20) | Network retry count and interval; forced from the `retry` section of `third_party_versions.json` on every configure |
| `DUILIB_DOWNLOAD_CONNECT_TIMEOUT` | STRING | 30 | Download connect timeout (seconds) |
| `DUILIB_SKIA_REQUIRE_PATCH` | BOOL | ON | Require the manifest `patchZip` for the selected Skia ref |
| `DUILIB_SKIA_AUTOBUILD` | BOOL | OFF | Skip the Skia existence check (used when `deps` builds it first) |
| `DUILIB_SDL_RPATH` / `DUILIB_RPATH_ABSOLUTE` | BOOL | ON | Unix rpath: relative paths first, plus an absolute fallback |
| `DUILIB_LLVM_ROOT` | PATH | auto-detected | LLVM root used by Skia on Windows (`gn`'s `clang_win`) |
| `DUILIB_GN_EXECUTABLE` / `DUILIB_NINJA_EXECUTABLE` | FILEPATH | empty | Override the `gn`/`ninja` location |
| `DUILIB_CEF_ARCHIVE` | FILEPATH | empty | Local CEF archive (skips the download) |
| `DUILIB_CEF_REQUIRED` | BOOL | ON | Fail when the CEF runtime cannot be prepared |

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
├── llvm.x64.debug/             # MSVC / clang-cl (adds .mdd for /MD)
├── llvm.x64.release/           # MSVC / clang-cl (adds .md for /MD)
├── gcc.msvcrt.x64.release/     # MSYS2 MINGW64
├── gcc.x64.release/            # MSYS2 UCRT64 and friends
├── mingw64-gcc.x64.release/    # standalone MinGW-w64
└── ...
```

SDL3 install prefixes on Windows follow the same architecture/configuration/runtime
rule: `<deps-root>/SDL3/msvc.<arch>.<debug|release>[.mdd|.md]` (each variant is a
complete prefix); MinGW reuses the Skia release variant name, and non-Windows keeps
the single `<deps-root>/SDL3` prefix. Both are produced by
`python3 build/build.py deps`; the rule lives in
[duilib_sdl_variants.cmake](duilib_sdl_variants.cmake).

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
1. Run `python3 build/build.py deps` (or `build.py deps --force`) to prepare the dependencies again
2. Confirm Skia is built: `ls skia/out/`
3. Use `DUILIB_SKIA_LIB_SUBPATH` to specify the correct path

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
