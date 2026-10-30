English | [简体中文](Deployment.md)

# SDK Guide: Installation, Upgrade, Migration and FAQ

This document is for **consumers of the release package (SDK)**: how to integrate
`nim_duilib-<version>-<system>-<arch>...zip/tar.gz` into a project, deploy the
runtime files, upgrade to a newer package, and migrate from the old layout.

> Every package ships a `MANIFEST.txt` that describes its actual contents, link
> list, preprocessor definitions and runtime requirements. This document is the
> general guide; when it disagrees with the package, `MANIFEST.txt` wins.

## 1. Package layout

| Directory / file | Description |
| :--- | :--- |
| `include/duilib/**.h` | Public headers, keeping repository-relative paths: `#include "duilib/duilib.h"` |
| `include/duilib/third_party/libcef/<libcef_win\|libcef_win_109>/include/*.h` | CEF headers of a `--with-cef` package; the integration layer adds the parent directory to the include path, so use `#include "include/cef_xxx.h"` |
| `lib/` | Static or import libraries (`.lib` / `.a`, flat) |
| `bin/*.dll` | Runtime DLLs (`duilib.dll`, `turbojpeg.dll`, ... for DLL packages) |
| `bin/resources/` | Themes, languages, fonts; **must sit next to the executable** |
| `bin/libcef_win[_109]/<arch>/` | CEF runtime of a `--with-cef` package (`libcef.dll`, `*.pak`, `icudtl.dat`, `locales/`) |
| `bin/libcef_linux/` | CEF runtime of a Linux package (`libcef.so` and friends) |
| `bin/*.exe`, `bin/*.app` | Example programs of an `-examples` package (`.app` on macOS) |
| `msvc/` | Windows + MSVC integration: `nim_duilib.props`, `nim_duilib.targets`, manifests |
| `lib/cmake/nim_duilib/` | Cross-platform CMake integration: `find_package(nim_duilib CONFIG)` |
| `docs/` | Bundled documentation (relative links rewritten for the package layout) |
| `LICENSES/`, `THIRD-PARTY-NOTICES.md` / `.en.md` | Third-party licenses and notices (Chinese / English) |
| `VERSION`, `MANIFEST.txt`, `SHA256SUMS` | Version, manifest, per-file checksums |

Package name segments: `-mingw-gcc` / `-mingw-llvm` (the two MinGW C++ runtime
variants), `-sdl`, `-cef` / `-cef109`, `-examples`, `-dll`, `-debug`.
**A package is single-architecture and single-configuration**: Debug and Release,
x64/x86/arm64 each use their own package and must not be mixed.
Platform difference: Windows/Linux/macOS packages may include CEF; **FreeBSD
packages never do** (no upstream Chromium build, so `pack.py --with-cef` fails
fast). Everything else matches the Linux package layout.

## 2. Installation

### 2.1 Unpack and verify

```powershell
# Windows (PowerShell)
Expand-Archive nim_duilib-<version>-win-x64-examples.zip -DestinationPath .
```

```bash
# Linux / macOS / FreeBSD
tar -xzf nim_duilib-<version>-linux-x64-examples.tar.gz
cd nim_duilib-<version>-linux-x64
sha256sum -c SHA256SUMS          # on Windows use the PowerShell snippet in MANIFEST.txt
```

The SDK requires no installation: unpack it anywhere; no registry entries and no
`PATH` changes. Keeping the package next to the project (for example
`third_party/nim_duilib-sdk`) or in a shared directory both work.

### 2.2 Windows + Visual Studio (MSVC)

Import the integration file at the end of the `.vcxproj` (it imports
`nim_duilib.props` automatically):

```xml
<Import Project="<SDK root>\msvc\nim_duilib.targets" />
```

The integration file:

1. adds `include/` (plus the CEF header root for CEF packages) and `lib/`;
2. defines the capability macros from the package metadata: `DUILIB_SDL`,
   `DUILIB_CEF`, `DUILIB_WEBVIEW2`, `DUILIB_JPEG_TURBO`, `DUILIB_LIB_PAG=0`;
   DLL packages also add `DUILIB_DLL`;
3. adds the link list (Skia, `duilib-*` and optional SDL/JPEG/WebView2/CEF for
   static packages; DLL packages link only the import library plus system libraries)
   and delay-loads `libcef.dll`;
4. aligns `RuntimeLibrary` with the package (`/MT` or `/MD`;
   set `NimDuilibForceRuntimeLibrary=0` to keep your own setting);
5. embeds the bundled Common Controls manifest matching `Platform`;
6. copies `bin/*.dll` to the output directory after building
   (`NimDuilibCopyRuntime=0` disables it), and copies `bin/resources/` as well
   when `NimDuilibCopyResources=1`.

Architecture, configuration, toolchain or artifact mismatches fail early in
`PrepareForBuild` instead of surfacing as `LNK2019` / `LNK2038` at link time.
Import only `msvc\nim_duilib.props` when you want the compile/link settings
without the runtime copy step.

### 2.3 CMake (all platforms)

```cmake
find_package(nim_duilib CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)

# copy runtime files after building (bin/*.dll on Windows)
nim_duilib_copy_runtime(my_app)
# when the application does not ship its own theme resources
nim_duilib_copy_runtime(my_app RESOURCES)
```

If `find_package` cannot locate the package, pass
`-DCMAKE_PREFIX_PATH=<SDK root>` or
`-Dnim_duilib_DIR=<SDK root>/lib/cmake/nim_duilib`. Optional switches (set before
`find_package`):

| Switch | Effect |
| :--- | :--- |
| `NIM_DUILIB_SET_MSVC_RUNTIME=OFF` | Do not force the MSVC runtime from the package metadata |
| `NIM_DUILIB_ALLOW_ARCH_MISMATCH=ON` | Warn instead of failing on architecture mismatch (linking will most likely fail) |
| `NIM_DUILIB_ALLOW_CONFIG_MISMATCH=ON` | Warn instead of failing on configuration mismatch |

On macOS a `.app` target can use `FRAMEWORKS` to copy `lib/libSDL3*.dylib` into
`Contents/Frameworks` and set the `@executable_path/../Frameworks` rpath;
`RESOURCES` then copies into `Contents/Resources/duilib/resources/`
(the standard directory located by `GlobalManager::GetResourceRootPath(true)`).

## 3. Runtime deployment

| File / directory | Required | Notes |
| :--- | :--- | :--- |
| `<exe dir>/resources/` | **Yes** (unless the app ships its own) | The framework reads `global.xml` and themes at startup; copy `bin/resources/` together with programs taken from `bin/` |
| `WebView2Loader.dll` | Only for dynamic-loader packages | A static loader (`WebView2LoaderStatic.lib`) needs no DLL; WebView2 also requires the Edge WebView2 Evergreen Runtime |
| `duilib*.dll`, `turbojpeg.dll`, `SDL3.dll` | DLL / dynamic-SDL packages only | Must sit next to the executable; the same names in `lib/` are import libraries |
| `bin/libcef_win[_109]/<arch>/` | `--with-cef` packages using CEF | Copy the whole directory next to the executable; **do not flatten it**: the framework looks for `libcef.dll` in `<exe dir>/<that subdirectory>` |
| MSVC runtime | `/MD` builds | The target machine needs the matching VC++ Redistributable; `/MT` packages need nothing |
| MinGW runtime | When the consumer does not link with `-static` | gcc: `libstdc++-6.dll`, `libgcc_s_*.dll`, `libwinpthread-1.dll`; llvm-mingw: `libc++.dll` etc. |
| `libSDL3.so*` / `libSDL3*.dylib` | Unix / macOS | Shipped in `lib/`; examples carry a relative rpath, downstream programs need their own rpath or `LD_LIBRARY_PATH` / `DYLD_LIBRARY_PATH` |

## 4. Upgrading

1. **Check the matrix**: confirm that system, architecture, configuration, toolchain
   and features (SDL/CEF/CEF109/DLL) of the new package match your project. A
   different package name means a different package; do not swap it in place.
2. **Replace the directory**: unpack the new package to a new directory and point
   `NimDuilibDir` / `CMAKE_PREFIX_PATH` at it (keeping per-version directories
   makes rollback easy).
3. **Clean stale artifacts**: delete previously copied `bin/*.dll` and old
   `resources/` from the output directory, and reconfigure (the CMake cache
   stores the old `nim_duilib_DIR`) so headers, libraries, macros and runtime all
   come from the same package.
4. **Rebuild and relink**: replacing DLLs alone is not an upgrade.
5. **Regression check**: the program starts (resources in place), images,
   WebView2/CEF pages work, and Debug and Release each use the matching package.
6. If a release changes the layout or library names, the "layout / link list /
   runtime" sections of `MANIFEST.txt` spell out the differences.

## 5. Migrating from older setups

| Scenario | What to do |
| :--- | :--- |
| Using the source tree → consuming an SDK package | Point the include path at `<SDK root>/include` and take the link list from `MANIFEST.txt`; let `nim_duilib.targets` or the CMake config define `DUILIB_SDL`/`DUILIB_CEF`/... instead of hand-writing them |
| Static package → DLL package (`-dll`) | Switch to the `-dll` package and define `DUILIB_DLL` (the integration layer does this for you); ship `bin/duilib*.dll` and `bin/turbojpeg.dll` next to the program; `/MT` and `/MD` must not be mixed |
| CEF 156 package → CEF 109 package | Replace the whole package with the `-cef109` one; the wrapper becomes `libcef_dll_wrapper_109` and the two wrappers must not be mixed; the runtime directory is `bin/libcef_win_109/<arch>` |
| Building from source → consuming the package | No longer need to build Skia/SDL/CEF yourself; enable the macros of the third-party features (WebView2, CEF) that the package provides |
| Single-configuration project → Debug and Release | Prepare two packages (`-debug` and the default Release) and import each per `Configuration`; with CMake, configure each prefix directory separately |
| x64 → arm64 (Windows) | Use the arm64 package; it ships without SDL and libpag (no upstream binaries); CEF 156 and 109 packages are both available |

## 6. FAQ

| Symptom | Cause | Fix |
| :--- | :--- | :--- |
| The program crashes at startup and cannot read `global.xml` / themes | `resources/` is not next to the executable | Copy `bin/resources/` to `<exe dir>/resources/`, or use `nim_duilib_copy_runtime(... RESOURCES)` / `NimDuilibCopyResources=1` |
| `0xc000007b` (application failed to start) | Executable and DLL architecture differ (x86/x64/arm64 mixed) | Use one package and toolchain architecture; remove stale DLLs from the output directory |
| `LNK2038` (`RuntimeLibrary` / `_ITERATOR_DEBUG_LEVEL` mismatch) | The project and the SDK use different runtimes (`/MT` vs `/MD`) or configurations (Debug vs Release) | Use the matching package; keep the integration layer's forced alignment or align your project settings |
| `LNK4098` (default libraries conflict) | A Debug project linked Release third-party libraries, or runtimes are mixed | Use the matching Debug package; the integration layer already adds `/NODEFAULTLIB:"libcmt.lib"` for static Debug |
| `LNK2019` for many `duilib-*` / Skia symbols | Only `duilib.lib` was linked; Skia and the third-party libraries are missing | Follow the link list in `MANIFEST.txt`, or use `nim_duilib.targets` / `nim_duilib::duilib` |
| WebView2 creation fails (`0x80070002` and similar) | The Edge WebView2 Evergreen Runtime is missing, or the dynamic loader is used without `WebView2Loader.dll` | Install the WebView2 Runtime; copy `bin/WebView2Loader.dll` next to the executable |
| CEF page stays blank / `CefManager` initialization fails | The CEF runtime directory is missing, flattened, or 156/109 files are mixed | Copy `bin/libcef_win[_109]/<arch>/` as a whole next to the executable; keep a single CEF version per process |
| Unix examples cannot find `libSDL3.so.0` | The rpath was rewritten or no library search path is set | Use the bundled relative rpath (`$ORIGIN/../lib`) or set `LD_LIBRARY_PATH` (macOS: `DYLD_LIBRARY_PATH`) |
| "The SDK is a MinGW build and cannot be used with VS" | A MinGW package is used with an MSVC project | Switch to the MSVC package, or use CMake with a MinGW toolchain |
| `sdk_package_test.py` reports C01 as SKIP | An arm64 package is validated on an x64 host, so the artifact cannot run here | Expected; run-time verification needs an environment of the matching architecture |

## 7. Related documentation

* [Bundled documentation index (Summary.en.md)](Summary.en.md)
* [CEF control guide (CEF.en.md)](CEF.en.md)
* `MANIFEST.txt` inside the package: layout, link list, runtime requirements, verification commands
