# The build Directory

This directory contains the unified build entry point, packaging/release tools,
unit tests and Visual Studio solutions. For full commands, options, dependencies
and troubleshooting, see [build-reference.en.md](build-reference.en.md).

## Files

| File | Purpose |
| :--- | :--- |
| `build.py` | **the single command-line entry**: deps, build, test, pack, clean |
| `build-reference.md` / `.en.md` | build reference manual (commands, deps, outputs, troubleshooting) |
| `linux-dependencies.md` / `.en.md` | system dependency commands for Linux distributions and how to verify them |
| `macos-app-bundle.md` / `.en.md` | reference for building examples as macOS `.app` bundles |
| `pack.py` | produces SDK packages (collects existing artifacts, never builds) |
| `release.py` | release orchestration: checks, packaging, verification (no commit/tag/push) |
| `autotest.py` | local automation: environment check → build → package → verify → assertions |
| `test_build.py` | unit tests for the build entry point |
| `test_pack.py` / `test_autotest.py` | packaging and automation tests |
| `test_sdk_package.py` | unit tests for the pure functions of the SDK package test orchestrator |
| `sdk_package_test.py` | released-SDK package test orchestrator (structure checks + consumer build/run + negative cases) |
| `sdk_test/` | SDK consumer test projects (depend only on the unpacked SDK) |
| `sdk-package-test.md` | SDK package test methods and purpose (Chinese) |
| `arm64-test-plan.md` | arm64 test plan (Windows matrix, dependency checks, result backfill) (Chinese) |
| `duilib.sln` / `duilib_no_cef.sln` | Visual Studio solutions (library only) |
| `examples.sln` / `examples_no_cef.sln` | Visual Studio solutions (library + examples) |
| `README.md` / `README.en.md` | this document |

The legacy per-platform scripts were deleted on 2026-10-29 (all four platforms are
verified with the new entry point); use `python3 build/build.py`.

## Prerequisites

- CMake 3.24+, Python 3 (required; commands use `python3` throughout), Git, Ninja.
- Windows: Visual Studio 2022+ with the C++ workload, or MSYS2 / MinGW-w64.
- Linux/macOS/FreeBSD: gcc 10+/clang 12+, plus font/GL/X11 development packages; these
  systems usually provide only `python3`.
- Run `python3 build/build.py deps` once to prepare Skia/SDL3/CEF. The Skia build
  scripts invoke `python3`, so it must be on PATH.
- Commands below use `python3 build/build.py` on Windows and Unix alike; for the
  Windows fallback when no `python3` command exists, see the build-entry section of the
  root [README.md](../README.md) (in Chinese).

Per-platform package lists and version requirements are in
[build-reference.en.md](build-reference.en.md), sections 1 and 8.

## Visual Studio solutions

Solutions do not prepare dependencies automatically; run this first:

```bash
python3 build/build.py deps
```

- `duilib.sln` / `examples.sln`: CEF enabled.
- `duilib_no_cef.sln` / `examples_no_cef.sln`: CEF disabled, faster builds.
- Solutions keep using `build/build_temp` and `build/.vs`; the new entry uses
  `out/<arch>/<Config>` and the two do not interfere.
- All solutions and projects now contain `Debug|ARM64` and `Release|ARM64`
  configurations. Pick `ARM64` in the VS configuration manager, or build from
  the command line:

  ```bat
  msbuild build\duilib.sln /p:Configuration=Release /p:Platform=ARM64 /m
  msbuild build\examples.sln /p:Configuration=Release /p:Platform=ARM64 /m:2
  ```

  Libraries are written to `lib/arm64/<Configuration>`, the same directory the
  CMake entry uses. arm64 Skia / CEF / WebView2 / libjpeg-turbo are required;
  SDL3 and libpag have no arm64 binaries and are disabled for ARM64 (libpag on
  ARM64 is untested and not followed up). Use
  `/m:2` for the examples solution to avoid C1060 during LTCG linking.
  All four solutions have been built and verified for both `Release|ARM64` and
  `Debug|ARM64` (23 examples with CEF, 19 examples for the `_no_cef` variants);
  the output is PE `AA64`.

## DLL mode (Windows MSVC)

```bash
# Build duilib as bin/duilib*.dll; prepares Skia .md/.mdd (SDL is off by default; add --sdl if needed)
python3 build/build.py build --dll --config Release

# Debug DLL (bin/duilib_d.dll, Skia uses .mdd)
python3 build/build.py build --dll --config Debug

# Library only, no examples
python3 build/build.py build --dll --target duilib
```

`--dll` implies `--md` and synchronizes `msvc/PropertySheets/duilib_dll.flag`;
running without `--dll` removes the flag and returns to static mode. On a mode
switch the build tree is cleaned automatically, so the CEF wrapper and all
targets rebuild with the new CRT; no manual `build_temp/` cleanup is needed.
SDL is off by default on Windows and DLL mode does not turn it on either (SDL
behaves noticeably worse than the native backends there); while it is off,
`deps` neither downloads the SDL sources nor builds SDL3. Pass `--sdl`
explicitly if you really need it.
DLL packages collect `bin/duilib*.dll`; JPEG support adds `bin/turbojpeg.dll`
only when the DLL really imports it, and the package name gets `-dll`.

## Outputs and cleanup

| Path | Content | Safe to delete |
| :--- | :--- | :--- |
| `out/<arch>/<Config>/` | CMake build tree of the new entry | yes |
| `lib/<arch>/<Config>/` | compiled libraries | rebuilt on demand |
| `bin/` | examples, `resources/`, CEF runtime | required at runtime |
| `build/build_temp/` | intermediate dir of VS solutions | yes |
| `build/.vs/` | Visual Studio cache | yes |
| `cef_cache/`, `webview2_cache/` | CEF / WebView2 network caches | yes |

```bash
python3 build/build.py clean          # remove the whole out/
python3 build/build.py clean --deps   # also remove dependency artifacts
```

## Packaging

SDKs are produced only by `pack.py`; the root CMake project has no `install()` target.
`release.py` orchestrates official releases: it calls `pack.py` and verifies versions
and artifacts, but never commits, tags or pushes.

```bash
python3 build/pack.py                    # Release SDK
python3 build/pack.py --config Debug     # Debug SDK (name has -debug)
python3 build/pack.py --with-examples    # include examples
python3 build/pack.py --list             # only list collected files
python3 build/pack.py --msvc-runtime md  # MSVC dynamic runtime (auto: md for DLL builds, otherwise mt)
python3 build/pack.py --allow-missing    # debug only: keep going when key artifacts are missing

python3 build/release.py check
python3 build/release.py package --config Release --arch x64 --dry-run
python3 build/release.py finalize
python3 build/release.py clean --list
```

Or use the unified entry:

```bash
python3 build/build.py pack --with-cef
python3 build/build.py pack --config Debug --out-dir dist
```

Package name: `nim_duilib-<version>-<system>-<arch>[-sdl][-cef|-cef109][-examples][-debug]`.
It contains `include/duilib/`, `lib/<arch>/<Config>/`, `bin/` (runtime DLLs and
`bin/resources/`), `lib/cmake/nim_duilib/`, `LICENSES/`, `docs/`, `MANIFEST.txt` and
`SHA256SUMS`. `LICENSES/`, `THIRD-PARTY-NOTICES.md` and `THIRD-PARTY-NOTICES.en.md`
must be kept when redistributing.

### Local packaging automation

`autotest.py` chains environment checks → (optional) build → package → verification →
structural assertions → (optional) smoke tests. It is **not wired into CI**, does not
write the repository's `bin/`, `lib/` or `dist/`, and stores output under
`<system temp>/nim_duilib_autotest/<timestamp>/` (override with `--out-dir`).

```bash
python3 build/autotest.py --preset unit
python3 build/autotest.py --preset pack-msvc-x64-release
python3 build/autotest.py --preset arm64-synthetic
python3 build/autotest.py --preset all-ready
python3 build/autotest.py --preset pack-msvc-x64-release --build
python3 build/autotest.py --preset arm64-synthetic --dry-run
```

Exit codes: 0 some preset passed and none failed; 1 a failure; 2 all skipped.
`--list-presets` lists presets and preconditions; results go to `report.json` and
`autotest.log`.

arm64 scope: integration files and package layout are ready and libjpeg-turbo ships an
arm64 prebuilt library. All three platforms can ship CEF on arm64 (`windows/arm64` prebuilt
archives exist for both CEF 142 and 109, with the import library taken from `lib/arm64` and
the runtime from `bin/libcef_win[_109]/arm64`; Linux/macOS use the `linux/arm64` and
`macos/arm64` archives). **This round validates Windows only** (macOS / Linux / FreeBSD have
no real hardware and are out of scope; Windows on ARM run verification has no device and is
untested, not followed up). Windows arm64 has passing Release and Debug builds, DLL mode,
and four packages that were packed and package-tested (Release without CEF, Release with
CEF 142, Release with CEF 109, Debug with CEF 142), all artifacts being PE `0xAA64`;
steps and results are in the [arm64 test plan](arm64-test-plan.md) (Chinese).

### Downstream integration

Packages ship a downstream integration layer, so consumers do not assemble include
paths, library lists and macros by hand:

- `msvc/` (Windows + MSVC only): `nim_duilib.props`, `nim_duilib.targets`,
  `nim_duilib.sdk.props` and `manifest/`. Import `nim_duilib.targets` once at the end of
  a `.vcxproj` to get compilation, linking, manifests and runtime DLL copying.
- `lib/cmake/nim_duilib/` (all platforms): `find_package(nim_duilib CONFIG REQUIRED)`
  provides `nim_duilib::duilib` and
  `nim_duilib_copy_runtime(<target> [RESOURCES] [FRAMEWORKS])`
  (`FRAMEWORKS` is for macOS App Bundles only; on a bundle target `RESOURCES`
  copies into `Contents/Resources/duilib/resources/`). See `sdk_test/bundle_main.cpp`
  and `sdk_test/CMakeLists.txt` for a runnable bundle integration sample.
- Both mechanisms validate architecture/configuration/toolchain. Packages are single
  architecture and single configuration; Debug and Release need their own packages.
  The MSVC runtime is enforced from package metadata by default (can be disabled).

See [docs/Release.en.md](../docs/Release.en.md) for examples and property details.
Common pitfalls:

- Keep the `include/duilib/xxx.h` layout; consumers use `#include "duilib/duilib.h"`.
- Image decoder implementation headers and internal headers are not packaged;
  `third_party` headers are forwarded only for WebView2. `zlib` / `libpng` / `giflib`
  / `libjpeg-turbo` headers are no longer forwarded (their static libraries still ship).
- `Core/DragWindowFilter_SDL.h` is mandatory on non-Windows and included on Windows
  only with `--with-sdl`.
- Windows packages include `bin/WebView2Loader.dll`. Packaging fails if any of
  `duilib` / `duilib-cximage` / `duilib-png` / `duilib-webp` / `duilib-zlib` is missing.
- MinGW libraries are `libduilib*.a` (static) or `libduilib*.dll.a` (import). The packer
  detects the toolchain from library contents; manifest link lines use `-lxxx`.
- In DLL mode (`bin/` contains `duilib*.dll`, from `build.py --dll` or
  `duilib_dll.flag`) `bin/duilib*.dll` ships with the import library and the
  package name gets `-dll`. DLL mode requires `/MD`, so Skia must use the same
  CRT. If the duilib DLL was built with SDL, packaging must pass `--with-sdl`
  (`pack.py` probes the DLL export table and fails otherwise); `turbojpeg.dll`
  is shipped only when that DLL actually imports it (MSBuild builds do, the
  CMake build currently does not enable libjpeg-turbo).
- `--with-sdl` collects actual SDL3 artifacts; Windows does not package SDL3 by default.
- CEF requires `--with-cef`: on Windows runtime files follow
  `bin/libcef_win[/_109]/<arch>/`; on Linux they are flat under `bin/libcef_linux/`.
  `--cef-version 109` is Windows-only.
- Archive permissions/symlinks: tar preserves source permissions; zip stores symlinks
  as `S_IFLNK` entries, which are excluded from `SHA256SUMS`.
- Reproducible builds: `SOURCE_DATE_EPOCH` fixes entry times, gzip headers and the
  MANIFEST timestamp.

See [docs/Release.en.md](../docs/Release.en.md) for the complete guide.

## Troubleshooting index

| Symptom | Action |
| :--- | :--- |
| `Visual Studio not found` | install VS 2022+ with the C++ workload, or pass `--vs-version` |
| `Skia not found` | run `python3 build/build.py deps` first |
| `gn/ninja not found` | use VS's bundled tools on Windows; install `ninja`/`gn` elsewhere |
| CEF example does not start | ensure `bin/libcef_win/<arch>` or `bin/libcef_linux` has the runtime |
| No text on Linux | install fontconfig and CJK fonts; check `bin/resources/fonts` |
| Broken build tree | `python3 build/build.py clean` and retry |

## Maintenance notes

### Adding an example

1. Create `examples/<name>/CMakeLists.txt` and sources.
2. Register the name in the example list of `cmake/duilib_root_options.cmake`.
3. Examples that need CEF follow the default CEF switch.

### Skia variants and runtime

The Skia variant directory is derived from architecture, toolchain and runtime
(e.g. `llvm.x64.release`, `llvm.x64.debug`, `mingw64-gcc.x64.release`). Switching
`--md` or the toolchain makes `deps` produce the matching variant; `pack.py` detects
it from the directory contents.

### Supported compilers

MSVC 19.30+ (VS 2022), gcc 10+, clang 12+. MSVC 2017/2019 are supported only on the
`develop-cpp17` branch, and VS 2017 does not support CEF.
