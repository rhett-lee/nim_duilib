# Release and Packaging

This document explains how to package nim_duilib into a distributable
artifact, and covers versioning and signing during a release.

The packaging script is `build/pack.py`. It **only collects files that have
already been built; it does not trigger any compilation**.

## Why not CPack

This repository has no root `CMakeLists.txt`. `duilib/`, `examples/*/` and
`duilib/third_party/*/` are each independent `project()` definitions,
configured and built one by one by scripts such as `build/msvc_build.bat`,
`build/linux_build.sh` and so on. CPack must be attached to a top-level
project, so "adding `install()` rules" is not a viable path here.

Moreover Windows has two build paths — `build/*.sln` (MSBuild) and
`msvc_build.bat` (CMake). Their library artifacts now share one layout:

| Platform | Library directory |
| :--- | :--- |
| Windows | `lib/<x64\|x86\|arm64>/<Release\|Debug>/` |
| Linux / macOS / FreeBSD | `lib/<Release\|Debug>/` |

Configuration is expressed purely by the directory; library names no longer
carry a `_d` suffix. The convention is that **a given `<arch>|<Config>` is
fully built by only one build system at a time**; the later build overwrites
the earlier one (including `.pdb`). `pack.py` reads only the single directory
selected by `--config` — it no longer merges candidate directories or guesses
by priority.

## Quick start

You must complete a build of the matching configuration before packaging; the
script does not trigger compilation:

```
# Windows (MSBuild path)
build\duilib.sln      # Configuration: Release or Debug

# Windows (CMake path)
build\msvc_build.bat

# Linux / macOS / FreeBSD
./build/linux_build.sh
./build/macos_build.sh
./build/freebsd_build.sh
```

Then package:

```
# SDK only (recommended, Release by default)
python build/pack.py

# Debug SDK (package name gains -debug)
python build/pack.py --config Debug

# SDK plus example programs
python build/pack.py --with-examples
```

Output goes to `dist/`, with package names of the form
`nim_duilib-<version>-<system>-<arch>[-debug].zip`:

```
dist/nim_duilib-1.0.2026.0416-win-x64.zip
dist/nim_duilib-1.0.2026.0416-win-x64-debug.zip
```

## Package layout

```
nim_duilib-<version>-<system>-<arch>/
  VERSION               Version number, identical to the root VERSION file
  MANIFEST.txt          Package contents and caveats
  SHA256SUMS            SHA256 of every file, verifiable with sha256sum -c
  LICENSE               Main license
  include/duilib/       Public headers (plus include/SDL3/ when SDL is used)
  lib/                  Static libraries (5 duilib libs + all Skia libs + third-party link libs)
  resources/            Theme, language and font resources
  licenses/             Third-party licenses
  docs/                 User documentation (top level only; links rewritten for the package layout)
  msvc/                 Downstream integration: props/targets/manifest (both x64 and x86)
  lib/cmake/nim_duilib/ Downstream integration: CMake find_package config
  bin/                  Runtime DLLs; example programs with --with-examples
```

### SDK usage contract

To let downstream builds **compile, link and run**, `MANIFEST.txt` records:

1. **Include path**: `include/` (`#include "duilib/duilib.h"`). When CEF is
   included, also add `include/duilib/third_party/libcef/<libcef_win|libcef_win_109>`.
2. **Library path**: `lib/` (flat).
3. **Link list** (MSVC uses `xxx.lib`; MinGW artifacts are `.a` / `.dll.a`,
   written as `-lxxx`):
   - Core: `duilib`, `duilib-cximage`, `duilib-webp`, `duilib-png`, `duilib-zlib`
   - Skia: `skia`, `svg`, `skshaper`, `skottie`, `sksg`, `jsonreader`
     (`bentleyottmann`, `expat`, `skcms`, `skresources` are shipped; add them
     if unresolved symbols appear)
   - WebView2: `WebView2LoaderStatic` (VS) or `WebView2Loader.dll` (CMake,
     requires the DLL at runtime)
   - JPEG (Windows): `turbojpeg-static`
   - SDL (optional): `SDL3-static` (MSVC static) / `SDL3` import library plus
     `bin/SDL3.dll` (dynamic) / `libSDL3.a` (MinGW static); on Unix `SDL3`
   - CEF (optional): `libcef` + `libcef_dll_wrapper[_109]`
4. **Platform system libraries**: with SDL on Windows add
   `Version/Winmm/Imm32/Setupapi` (`-lversion -lwinmm -limm32 -lsetupapi`
   for MinGW); on Unix `X11`/`freetype`/`fontconfig` come from the system and
   are not packaged.
5. **Runtime**: `resources/` must sit next to the executable; the CEF runtime
   is external; `bin/duilib*.dll` must sit next to the executable when duilib
   is built in DLL mode; copy `bin/SDL3.dll` for dynamic SDL3 on Windows;
   on Unix `libSDL3.so*` must be findable (`LD_LIBRARY_PATH`
   etc.).
6. **Configuration consistency**: a Debug package serves Debug downstream
   builds only, and a Release package serves Release only. Never mix the two.
   The MSVC runtime is fixed by the package: `/MT` (Debug `/MTd`) for static
   mode, `/MD` (Debug `/MDd`) for DLL mode. The integration layer aligns it
   for consumers by default.

### Downstream integration

The SDK ships two integration layers driven by the same package metadata
(arch, configuration, toolchain, DLL mode, SDL/WebView2/JPEG/CEF, MSVC
runtime, runtime DLLs). Packages are single-arch and single-config: a Debug
build must use the `-debug` package and a Release build the normal package.

#### Visual Studio (MSVC)

**Minimal setup**: import once near the end of the `.vcxproj` (before
`Microsoft.Cpp.targets`):

```xml
<Import Project="C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\msvc\nim_duilib.targets" />
```

The property sheets handle:

- include paths: `include` (plus the CEF wrapper root when present);
- defines: `DUILIB_SDL/CEF/WEBVIEW2/JPEG_TURBO/LIB_PAG`, `UNICODE`, and
  `DUILIB_DLL` in DLL mode;
- language/encoding: `stdcpp20`, `/utf-8`;
- linking: the full static set or the DLL import library plus system libraries;
- MSVC runtime alignment to the package;
- manifest: `msvc/manifest/duilib.x64.manifest` or `duilib.x86.manifest`,
  selected by the project's Platform (both ship, so x86 programs can be built
  on an x64 machine);
- post-build copy of `bin/*.dll` into `$(OutDir)`.

**When the SDK path is not fixed**, define the property before the import:

```xml
<PropertyGroup>
  <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  <NimDuilibCopyResources>1</NimDuilibCopyResources>
</PropertyGroup>
<Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />
```

`NimDuilibDir` must end with a backslash. It can also live in
`Directory.Build.props` or an environment variable of the same name.

**Key project-file fragment** (order matters):

```xml
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <!-- usual ProjectConfigurations / Globals settings -->

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />

  <!-- your sources and project settings -->
  <ItemGroup>
    <ClCompile Include="main.cpp" />
  </ItemGroup>

  <!-- nim_duilib integration: before Microsoft.Cpp.targets -->
  <PropertyGroup>
    <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  </PropertyGroup>
  <Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
```

**Compile/link settings without the copy step**: import only the props file:

```xml
<Import Project="$(NimDuilibDir)msvc\nim_duilib.props" />
```

Available properties:

| Property | Default | Meaning |
| :--- | :--- | :--- |
| `NimDuilibDir` | `<SDK root>` | SDK root directory, trailing `\` required |
| `NimDuilibCopyRuntime` | `1` | Copy `bin/*.dll` after build |
| `NimDuilibCopyResources` | `0` | Copy `resources/` after build |
| `NimDuilibForceRuntimeLibrary` | `1` | Align MSVC runtime; set `0` to opt out |
| `NimDuilibSkipValidation` | `0` | Skip toolchain/arch/config/library checks |

Common errors:

| Error | Cause | Fix |
| :--- | :--- | :--- |
| `duilib\duilib.h` not found | `NimDuilibDir` is not the package root, or misses the trailing `\` | Point it at the extracted `nim_duilib-...` directory |
| SDK arch/config mismatch | Wrong arch or wrong configuration package | Use the matching package; Debug needs the `-debug` one |
| "SDK is a MinGW build" | An MSVC project imported a MinGW package | Use the MSVC package |
| LNK2038 / runtime mismatch | The project overrides `RuntimeLibrary` | Keep the default alignment, or set `NimDuilibForceRuntimeLibrary=0` and align manually |

#### CMake (all platforms)

**Minimal CMakeLists.txt**:

```cmake
cmake_minimum_required(VERSION 3.18)
project(my_app LANGUAGES CXX)

# Extracted SDK root (not the lib/cmake subdirectory)
list(APPEND CMAKE_PREFIX_PATH "C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug")
find_package(nim_duilib CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)

# Copy bin/*.dll next to the executable after build
nim_duilib_copy_runtime(my_app)
```

**Configure and build**:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug
cmake --build build
```

Alternatively point directly at the config directory:

```
cmake -S . -B build -Dnim_duilib_DIR=C:/SDK/nim_duilib-...-debug/lib/cmake/nim_duilib
```

**Copy resources on demand / choose the destination**:

```cmake
# Also copy resources/ (overwrites an existing resources/ in the target dir)
nim_duilib_copy_runtime(my_app RESOURCES)

# Share one output directory between targets
nim_duilib_copy_runtime(my_app DESTINATION "${CMAKE_BINARY_DIR}/bin")
nim_duilib_copy_runtime(my_tool RESOURCES DESTINATION "${CMAKE_BINARY_DIR}/bin")
```

**Validation and switches**: `find_package` validates platform, toolchain,
architecture, configuration and the MSVC runtime, and fails with
`FATAL_ERROR` by default on mismatch:

| Variable | Default | Meaning |
| :--- | :--- | :--- |
| `NIM_DUILIB_SET_MSVC_RUNTIME` | `ON` | Set `CMAKE_MSVC_RUNTIME_LIBRARY` from the package |
| `NIM_DUILIB_ALLOW_ARCH_MISMATCH` | `OFF` | Warn only on arch mismatch (not recommended) |
| `NIM_DUILIB_ALLOW_CONFIG_MISMATCH` | `OFF` | Warn only on config mismatch (not recommended) |

**Multi-config generators (VS)**: packages are single-config.
`CMAKE_BUILD_TYPE` has no effect with the Visual Studio generator, so
building a configuration other than the package's (for example Debug with a
Release package) links an incompatible runtime. Use a separate build
directory and the matching package per configuration:

```
cmake -S . -B build-debug   -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-...-debug
cmake -S . -B build-release -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-...
```

The CEF runtime (`libcef.dll`, `*.pak`, `icudtl.dat`) still has to be placed
next to the executable manually; `nim_duilib_copy_runtime` only copies the
SDK's `bin/*.dll` and `resources/`.

### The header path layout must not change

Headers keep the `include/duilib/xxx.h` hierarchy, matching their path
relative to the repository root. Downstream code includes them as:

```cpp
#include "duilib/duilib.h"
```

Therefore there must be a `duilib/` level under `include/`. Flattening it to
`include/duilib.h` breaks every downstream build.

`RenderSkia/` is not exposed: the rendering backend is an implementation
detail and `duilib.h` does not reference it.

### Only the required third_party headers are forwarded

The packaged `Image/` directory keeps only the public base headers
(`Image.h`, `ImageAttribute.h`, `ImageInfo.h`, `ImageLoadParam.h`,
`ImageDecoder.h`, `ImageDecoderFactory.h`, `ImageUtil.h`, `StateImage.h`,
`StateImageMap.h`, `ImagePlayer.h`).

Format-specific decoder implementation headers are **not packaged**: they are
used only by `.cpp` files inside the library, and the `duilib.h` include chain
never references them:

* `Image/ImageDecoder_*.h` (PNG / GIF / JPEG / WEBP / SVG / ICO / Icon /
  LOTTIE / PAG / Common)
* `Image/Image_*.h` (PNG / GIF / JPEG / WEBP / SVG / ICO / LOTTIE / PAG /
  Bitmap / Animation)
* `Image/APngDecoder.h`, `Image/FrameSequence_gif.h`, `Image/ImageDecoderUtil.h`

`Core/ZipStreamIO.h` is likewise internal (only `ZipManager.cpp` includes it;
neither `duilib.h` nor `ZipManager.h` references it), so it is not packaged
either.

The following platform implementation headers are included only by `.cpp` /
`.mm` files inside the library and are not packaged:

* `Core/ControlDropTargetImpl_SDL.h`
* `Core/ControlDropTargetImpl_Windows.h`
* `Core/ControlDropTargetUtils.h`
* `Core/SDL_MacOS.h`
* `Core/WindowDropTarget_SDL.h`
* `Core/WindowDropTarget_Windows.h`
* `Core/SDL_Linux.h`, `Core/MessageLoop_SDL.h`, `Core/MessageLoop_Windows.h`
* `Utils/ProcessSingleton_Linux.h`, `Utils/ProcessSingleton_MacOS.h`, `Utils/ProcessSingleton_Windows.h`
* `Utils/BitmapHelper_SDL.h`, `Utils/BitmapHelper_Windows.h`
* `Utils/ApiWrapper_Windows.h`, `Utils/Macros_Windows.h`
* `Utils/DiskUtils_Windows.h`, `Utils/DllManager_Windows.h`, `Utils/InlineHook_Windows.h`
* `Control/RichEditHost_Windows.h`, `Control/RichEditDragSource_Windows.h`,
  `Control/RichEditDropTarget_Windows.h`, `Control/RichEditDropTarget_SDL.h`

Exception: `Core/ControlDragable.h` conditionally includes
`Core/DragWindowFilter_SDL.h` under `DUILIB_BUILD_FOR_SDL`. That macro is always
defined on non-Windows platforms, so Unix packages keep this header; Windows
packages omit it unless `--with-sdl` is used.

Public headers depend on only one kind of `third_party` headers, which is
forwarded with the package:

| Public header | Depends on | Notes |
| :--- | :--- | :--- |
| `WebView2/WebView2Control.h`, `WebView2/WebView2EnvironmentOptions.h` | `Microsoft.Web.WebView2` | Forwarded on Windows only |

Accordingly, `zlib`, `libpng`, `giflib` and `libjpeg-turbo` headers are no
longer forwarded. The `duilib-zlib` and `turbojpeg-static` libraries are still
shipped because linking needs them; only their headers are omitted.

These include relationships were verified one by one: every
`#include "duilib/..."` in the packaged `duilib/` headers resolves, so
downstream code only needs `#include "duilib/duilib.h"` to compile.

## Resource scope differences with --with-examples

This is the easiest thing to get wrong. `resources/themes/default/` holds
both shared resources and per-example private theme directories, and the two
modes differ in what they collect:

| Mode | `resources/` scope |
| :--- | :--- |
| Default (SDK) | Only `themes/default/public/` and `themes/default/global.xml` |
| `--with-examples` | The entire `resources/`, no exclusions |

`global.xml` **must** be kept in SDK mode. It is required at framework
startup: in local-file mode `GlobalManager.cpp` returns `false` outright when
the file is missing, so initialization fails.

Excluded in SDK mode are the 22 per-example private theme directories:
`MultiLang`, `basic`, `cef`, `cef_browser`, `chat`, `child_window`,
`color_theme`, `controls`, `dpi_aware`, `layout`, `list_box`, `list_ctrl`,
`move_control`, `panel`, `render`, `rich_edit`, `threads`, `tree_view`,
`virtual_list_box`, `webview2`, `webview2_browser`, `xml_preview`.

`bin/resources.zip` is never packaged — at 748 bytes it is an empty shell.

## Command-line options

| Option | Description |
| :--- | :--- |
| `--config Release\|Debug` | Library configuration; defaults to `Release` (case-insensitive) |
| `--arch x64\|x86\|arm64` | Library architecture; selects the library directory on Windows and affects the package name everywhere |
| `--lib-dir <path>` | Fully explicit library directory, overriding `--arch`/`--config` path computation |
| `--skia-lib-dir <path>` | Skia static library directory; defaults to probing `../skia/out/llvm.<arch>.<config>` |
| `--sdl-dir <path>` | SDL3 root directory; defaults to `../SDL3` |
| `--with-sdl` | Additionally package SDL3 on Windows (Unix always packages it) |
| `--with-cef` | Package the CEF development set: headers + `libcef.lib` + wrapper (large) |
| `--cef-version latest\|109` | CEF version for `--with-cef`; defaults to `latest` |
| `--msvc-runtime auto\|mt\|md` | MSVC runtime recorded in package metadata; `auto` (MD for DLL mode, otherwise MT) by default |
| `--with-examples` | Include example programs and all per-example resources |
| `--format zip\|tgz\|tarbz2` | Archive format; defaults to `zip` on Windows, `tgz` elsewhere |
| `--out-dir <path>` | Output directory; defaults to `dist/` |
| `--list` | Print the file list that would be collected without packaging |

## Debug packages

Configuration is determined solely by the directory (`lib/<arch>/Debug` or
`lib/Debug`); the `_d` suffix is no longer inspected.

- With `--config Debug` the package name gains `-debug` and `MANIFEST.txt`
  records `Configuration : Debug`.
- Debug libraries carry the debug CRT (`/MTd`, or `/MDd` in DLL mode) and may
  be used only by Debug downstream builds — **never mix them into a Release
  SDK**.
- With `--with-examples`: Release packages skip `*_d.exe`; Debug packages
  cannot distinguish CMake Debug executables (same name as Release), so the
  script warns that Release executables may be mixed in.
- If the library directory is missing or the main `duilib` library is absent,
  the script exits with code 1 instead of producing a package with no linkable
  libraries.

## CEF limitations

The CEF runtime (`libcef.dll`, `*.pak`, `icudtl.dat`) is **not part of this
repository**; users must obtain it from the official libcef release.
Consequently:

- The CEF-based examples (`cef`, `CefBrowser`) cannot run straight from the
  package
- `MANIFEST.txt` lists those examples and notes the limitation
- The CEF development set is excluded by default; pass `--with-cef` to include
  it (use `--cef-version 109` for the older version)

## Versioning

The single source of truth for the version is the `VERSION` file at the
repository root, formatted as `major.minor.patch.build`, for example
`1.0.2026.0416`.

`pack.py` reads that file and uses it for the package name, the in-package
`VERSION` and `MANIFEST.txt`. An invalid format (anything other than four
numeric segments) makes the script exit with an error rather than producing a
half-finished package.

The `DUILIB_VER_*` macros in `msvc/dllRes/version.h` are currently
**maintained independently**. They are used only by the `.vcxproj` version
resource and are invisible to the CMake side. A version bump must update both
places — this is a known gap that should later be resolved by deriving
`version.h` from `VERSION`.

Git tags are recommended in `v<version>` form so they line up with package
names. The existing tags in this repository (such as `2026-06-08(main)`) use a
legacy date-based format that contains parentheses and therefore cannot be
used directly as a package name or CPack version.

## Release process

1. Confirm the working tree is clean and that build artifacts are Release
2. Update `VERSION` and `msvc/dllRes/version.h`
3. Package and verify:

   ```
   python build/pack.py --list
   python build/pack.py
   ```

4. Verify artifact integrity:

   ```
   cd dist && unzip -q nim_duilib-<version>-<system>-<arch>.zip -d /tmp/check
   cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS
   ```

5. Tag: `git tag -a v<version> -m "release <version>"`
6. Push the tag and artifacts to the distribution channel

## Signing and notarization

Currently **not automated**. The following describes the manual procedure.

### Windows

Only executables and DLLs can be signed; static libraries (`.lib`) do not need
it. This project produces a pure static library (`add_library` in
`duilib/CMakeLists.txt` is not specified as `SHARED`, and
`msvc/dllRes/dllmain.cpp` only participates in `.vcxproj` builds), so the SDK
package itself needs no signing.

If you publish a package that includes example programs, signing the
executables with Authenticode is recommended:

```
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 ^
    /f <certificate>.pfx /p <password> dist\extracted\bin\*.exe
```

### macOS

Sign and notarize the `.app` and `.dylib`:

```
codesign --force --deep --sign "Developer ID Application: <team>" <artifact>
xcrun notarytool submit <artifact>.zip --keychain-profile <profile> --wait
xcrun stapler staple <artifact>.zip
```

### Linux / FreeBSD

Signing is usually unnecessary. If GPG signing is wanted:

```
gpg --detach-sign --armor dist/nim_duilib-<version>-<platform>-<arch>.tar.gz
```

Verify after publishing with `gpg --verify`.

## Platform differences

| Platform | Default format | Status |
| :--- | :--- | :--- |
| Windows | `zip` | Verified |
| Linux | `tgz` | Same script; artifacts not verified on this machine |
| macOS | `tgz` | Same script; artifacts not verified on this machine |
| FreeBSD | `tgz` | Same script; artifacts not verified on this machine |

The script itself is cross-platform (standard library only, no external
dependencies) and a single implementation serves all four platforms.
Windows `.lib` and Unix `.a` files are both collected by the directory
whitelist; the Unix `lib` prefix is normalized automatically (e.g.
`libduilib-zlib.a` → `duilib-zlib`). However, **only the Windows artifacts have
been verified so far**; the library list on the other platforms should still be
checked after the first package is produced.

## Related documentation

* [Build process and scripts](../build/README.en.md)
* [Quick Start](Getting-Started.en.md)
* [Example Programs](Examples.en.md)
* [CEF Control](CEF.en.md)
