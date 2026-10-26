# The build Directory

This directory contains the unified build entry point, packaging/release tools,
unit tests and Visual Studio solutions. For full commands, options, dependencies
and troubleshooting, see [build-reference.en.md](build-reference.en.md)
(Chinese: [build-reference.md](build-reference.md)).

## Files

| File | Purpose |
| :--- | :--- |
| `build.py` | **the single command-line entry**: deps, build, test, pack, clean |
| `build.bat` | thin Windows wrapper that locates Python 3 and calls `build.py` |
| `build-reference.md` / `.en.md` | build reference manual (commands, deps, outputs, troubleshooting) |
| `pack.py` | produces SDK packages (collects existing artifacts, never builds) |
| `release.py` | release orchestration: checks, packaging, verification (no commit/tag/push) |
| `autotest.py` | local automation: environment check → build → package → verify → assertions |
| `test_build.py` | unit tests for the build entry |
| `test_pack.py` / `test_autotest.py` | packaging and automation tests |
| `duilib.sln` / `duilib_no_cef.sln` | Visual Studio solutions (library only) |
| `examples.sln` / `examples_no_cef.sln` | Visual Studio solutions (library + examples) |
| `README.md` / `README.en.md` | this document |

The legacy per-platform scripts are archived under `legacy/` (not part of the
build, to be deleted after full-platform verification; Windows, Linux and FreeBSD
are verified, macOS pending); use `python3 build/build.py`.

## Prerequisites

- CMake 3.24+, Python 3 (required; the command is `python3` on Unix), Git, Ninja.
- Windows: Visual Studio 2022+ with the C++ workload, or MSYS2 / MinGW-w64; you can run
  `build\build.bat` directly (same arguments, no `python3` command needed).
- Linux/macOS/FreeBSD: gcc 10+/clang 12+, plus font/GL/X11 development packages; these
  systems usually provide only `python3`, not `python`.
- Run `python3 build/build.py deps` once to prepare Skia/SDL3/CEF.
- On Windows, replace `python3` with `python` in the commands below; the `build.py`
  entry point can also be run as `build\build.bat`.

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
`SHA256SUMS`. `LICENSES/` and `THIRD-PARTY-NOTICES.md` must be kept when redistributing.

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
arm64 prebuilt library, but nothing has been validated on real arm64 hardware; CEF has
no Windows arm64 prebuilt package, so arm64 packages exclude CEF.

### Downstream integration

Packages ship a downstream integration layer, so consumers do not assemble include
paths, library lists and macros by hand:

- `msvc/` (Windows + MSVC only): `nim_duilib.props`, `nim_duilib.targets`,
  `nim_duilib.sdk.props` and `manifest/`. Import `nim_duilib.targets` once at the end of
  a `.vcxproj` to get compilation, linking, manifests and runtime DLL copying.
- `lib/cmake/nim_duilib/` (all platforms): `find_package(nim_duilib CONFIG REQUIRED)`
  provides `nim_duilib::duilib` and `nim_duilib_copy_runtime(<target> [RESOURCES])`.
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
- In DLL mode (`msvc/PropertySheets/duilib_dll.flag`) `bin/duilib*.dll` ships with
  matching import libraries and the package name gets `-dll`. DLL mode requires `/MD`,
  so Skia must use the same CRT.
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
