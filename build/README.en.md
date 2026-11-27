# The build Directory

This directory contains the unified build entry point, packaging/release tools,
unit tests and Visual Studio solutions. For full commands, options, dependencies
and troubleshooting, see [docs/build-reference.en.md](docs/build-reference.en.md).

## I want to… → read this

| I want to… | Read this |
| :--- | :--- |
| Build the project | `python3 build/build.py build`; see [docs/build-reference.en.md](docs/build-reference.en.md) for every option |
| Package and release | `python3 build/pack.py`, `python3 build/release.py check`; downstream integration in [docs/Release.en.md](../docs/Release.en.md) |
| Look up platform dependencies | Linux: [docs/linux-dependencies.en.md](docs/linux-dependencies.en.md); macOS App Bundle: [docs/macos-app-bundle.en.md](docs/macos-app-bundle.en.md) |
| Look up all options | `python3 build/build.py --help`, or [docs/build-reference.en.md](docs/build-reference.en.md) |
| Read test reports and plans | [docs/test-matrix.md](docs/test-matrix.md), [docs/sdk-package-test.md](docs/sdk-package-test.md), [docs/arm64-test-plan.md](docs/arm64-test-plan.md) |
| Self-test before committing | `python3 build/build.py test` (cases live in [testing/](testing/)) |

## Files you do not need to read

| Path | Purpose (for reference only) |
| :--- | :--- |
| `testing/` | unit tests and automation scripts (`test_*.py`, `autotest.py`, `sdk_package_test.py`), driven by `build.py test` or `testing/autotest.py` |
| `testing/tests/` | CMake rule fixture `skia_variants_check.cmake`, run only by `testing/test_cmake_scripts.py` through `cmake -P` |
| `*.sln` | Visual Studio solutions for opening in the IDE; use `build.py` for command-line builds |
| `sdk_test/` | SDK consumer test projects, used only by `testing/sdk_package_test.py` |
| `skia_patch/` | Skia patch archive and manifest, verified and used automatically by `build.py deps` |
| `i18n.py` | shared language resolution and message catalog for the build scripts; application code never calls it directly |

## Files

```text
build/
├── build.py                  # single command-line entry (deps / build / test / pack / doctor / clean)
├── pack.py / release.py      # SDK packaging and release orchestration
├── i18n.py                   # shared language resolution and message catalog
├── README.md / README.en.md  # this document (navigation)
├── docs/                     # documentation, Chinese/English pairs
│   ├── build-reference.md / build-reference.en.md
│   ├── linux-dependencies.md / linux-dependencies.en.md
│   ├── macos-app-bundle.md / macos-app-bundle.en.md
│   ├── test-matrix.md
│   ├── sdk-package-test.md
│   └── arm64-test-plan.md
├── testing/                  # unit tests, automation and SDK package tests
│   ├── test_*.py             # unit tests (discovered by build.py test)
│   ├── autotest.py           # local packaging automation
│   ├── sdk_package_test.py   # SDK package layout and artifact tests
│   └── tests/                # CMake rule fixture skia_variants_check.cmake
├── sdk_test/                 # SDK consumer test projects
├── skia_patch/               # Skia patch archive
└── *.sln                     # 4 Visual Studio solutions (paths unchanged)
```

Entries in detail:

| File | Purpose |
| :--- | :--- |
| `build.py` | **the single command-line entry**: deps, build, test, pack, clean |
| `i18n.py` | shared language resolution and message catalog for the build entry points (single source: `cmake/i18n/messages.json`, same order as `cmake/duilib_i18n.cmake`) |
| `pack.py` | produces SDK packages (collects existing artifacts, never builds) |
| `release.py` | release orchestration: checks, packaging, verification (no commit/tag/push) |
| `docs/` | build and test references: build manual, Linux dependencies, macOS App Bundle, test matrix, SDK package test, arm64 test plan |
| `testing/` | unit tests, automation and SDK package test scripts, plus the CMake fixture in `testing/tests/` |
| `sdk_test/` | SDK consumer test projects (depend only on the unpacked SDK) |
| `duilib.sln` / `duilib_no_cef.sln` | Visual Studio solutions (library only) |
| `examples.sln` / `examples_no_cef.sln` | Visual Studio solutions (library + examples) |
| `README.md` / `README.en.md` | this document |

The legacy per-platform scripts were deleted on 2026-10-29 (all four platforms are
verified with the new entry point); use `python3 build/build.py`.

## Prerequisites

- CMake 3.24+, Python 3.8+ (required; commands use `python3` throughout), Git, Ninja.
- Windows: Visual Studio 2022+ with the C++ workload, or MSYS2 / MinGW-w64.
- Linux/macOS/FreeBSD: gcc 10+/clang 12+, plus font/GL/X11 development packages; these
  systems usually provide only `python3`.
- Run `python3 build/build.py deps` once to prepare Skia/SDL3/CEF. The Skia build
  scripts invoke `python3`, so it must be on PATH.
- Commands below use `python3 build/build.py` on Windows and Unix alike; for the
  Windows fallback when no `python3` command exists, see the build-entry section of the
  root [README.md](../README.md) (in Chinese).

### Compiler and CMake version requirements

| Purpose | Minimum | Notes |
| :--- | :--- | :--- |
| Main project (Linux/macOS/FreeBSD) | gcc 10+ or clang 12+ | Checked by the preflight at configure time |
| Building Skia (Linux/macOS/FreeBSD) | clang 12+ or gcc 11+ | Skia's `ml3`/`ml4` targets always use `-march=x86-64-v3` / `-march=x86-64-v4`, which only clang 12+ / gcc 11+ understand; clang is preferred and falls back to gcc when older than 12, and configuration fails early when both are too old |
| CMake | 3.24+ (CMakePresets is version 6, so 3.25+ in practice) | The root project is configured via `cmake --preset` from `build.py` |

Domestic distributions (NFSChina/中科方德 and UnionTech UOS): **the compilers shipped with both systems cannot build Skia** (NFSChina ships clang 11.0.1 and gcc 10.2.1; UOS ships gcc older than 11), so users must install **clang 12+ (recommended) or gcc 11+** themselves. In addition, **the CMake shipped with both systems is older than 3.24** (NFSChina ships 3.18.4, and UOS also misses the 3.24+ requirement), so you **must install a newer CMake manually (3.25 or newer is recommended); otherwise configuration fails immediately with `CMake 3.24 or higher is required` and the build cannot proceed**. Put the new compiler's and CMake's `bin` directories first in `PATH`; `build.py doctor` prints the CMake version and the Skia toolchain actually in use.

Per-platform package lists and version requirements are in
[docs/build-reference.en.md](docs/build-reference.en.md), sections 1 and 8.

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

Package name: `nim_duilib-<version>-<system>-<arch>[-mingw-gcc|-mingw-llvm][-sdl][-cef|-cef109][-examples][-dll][-debug]`.
It contains `include/duilib/`, `lib/` (flat layout; architecture and configuration
are distinguished by the package name), `bin/` (runtime DLLs and `bin/resources/`),
`msvc/` (Windows MSVC integration layer), `lib/cmake/nim_duilib/`, `LICENSES/`,
`docs/`, `THIRD-PARTY-NOTICES*.md`, `VERSION`, `MANIFEST.txt` and `SHA256SUMS`.
`LICENSES/`, `THIRD-PARTY-NOTICES.md` and `THIRD-PARTY-NOTICES.en.md` must be kept
when redistributing.

### Local packaging automation

`testing/autotest.py` chains environment checks → (optional) build → package → verification →
structural assertions → (optional) smoke tests. It is **not wired into CI**, does not
write the repository's `bin/`, `lib/` or `dist/`, and stores output under
`<system temp>/nim_duilib_autotest/<timestamp>/` (override with `--out-dir`).

```bash
python3 build/testing/autotest.py --preset unit
python3 build/testing/autotest.py --preset pack-msvc-x64-release
python3 build/testing/autotest.py --preset arm64-synthetic
python3 build/testing/autotest.py --preset all-ready
python3 build/testing/autotest.py --preset pack-msvc-x64-release --build
python3 build/testing/autotest.py --preset arm64-synthetic --dry-run
python3 build/testing/autotest.py --preset pack-msvc-x64-release --with-examples --smoke
```

Exit codes: 0 some preset passed and none failed; 1 a failure; 2 all skipped.
`--list-presets` lists presets and preconditions; results go to `report.json` and
`autotest.log`. The `--build` timeout is controlled by `--build-timeout`
(default 3600 seconds), `--with-sdl` / `--with-examples` are passed through to
`pack.py`, and `--smoke` requires examples inside the package and runs only for
Release presets (it unpacks and launches `bin/basic.exe` for 8 seconds, skipping
when the example architecture differs from the host).

`build.py`, `pack.py`, `release.py`, `testing/autotest.py` and
`testing/sdk_package_test.py` all accept
`--lang auto|zh_CN|en` (default follows the system, also reads `DUILIB_LANG`, and a
repeated option uses the last value), which switches both console messages and
`--help`; `testing/autotest.py` passes the language to the `build.py` / `pack.py` it spawns,
so logs do not mix languages.

arm64 scope: integration files and package layout are ready and libjpeg-turbo ships an
arm64 prebuilt library. All three platforms can ship CEF on arm64 (`windows/arm64` prebuilt
archives exist for both CEF 156 and 109, with the import library taken from `lib/arm64` and
the runtime from `bin/libcef_win[_109]/arm64`; Linux/macOS use the `linux/arm64` and
`macos/arm64` archives). **This round validates Windows only** (macOS / Linux / FreeBSD have
no real hardware and are out of scope; Windows on ARM run verification has no device and is
untested, not followed up). Windows arm64 has passing Release and Debug builds, DLL mode,
and four packages that were packed and package-tested (Release without CEF, Release with
CEF, Release with CEF 109, Debug with CEF; all artifacts being PE `0xAA64`; packing was
measured on 2026-10-28 when the default CEF version was 142, and arm64 packing has not been
re-run after the 156 upgrade); steps and results are in the
[arm64 test plan](docs/arm64-test-plan.md) (Chinese).

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
| `gn/ninja not found` | use VS's bundled tools on Windows, or run `python3 build/build.py deps` first to extract the bundled `bin/gn.exe` / `bin/ninja.exe` from the patch archive; install `ninja`/`gn` elsewhere (see section 10 of [docs/build-reference.en.md](docs/build-reference.en.md)) |
| CEF example does not start | ensure `bin/libcef_win/<arch>` or `bin/libcef_linux` has the runtime |
| No text on Linux | install fontconfig and CJK fonts; check `bin/resources/fonts` |
| Broken build tree | `python3 build/build.py clean` and retry |

## Maintenance notes

### Adding an example

1. Create `examples/<name>/CMakeLists.txt` and sources.
2. Register the name in the example list of `cmake/duilib_root_options.cmake`.
3. Examples that need CEF follow the default CEF switch.

### Skia/SDL3 variants and runtime

The Skia variant directory is derived from architecture, toolchain and runtime
(e.g. `llvm.x64.release`, `llvm.x64.debug`, `mingw64-gcc.x64.release`). Switching
`--md` or the toolchain makes `deps` produce the matching variant; `pack.py` detects
it from the directory contents. MinGW Skia only has release variants, so Debug and
Release share `<prefix><gcc|llvm>.<arch>.release`.

On Windows, SDL3 is installed per architecture × configuration × runtime into
`<deps-root>/SDL3/<variant>` (`msvc.<arch>.<debug|release>[.mdd|.md]`; MinGW reuses
the Skia release variant name). The rule lives in
`cmake/duilib_sdl_variants.cmake`; a missing variant falls back to the single prefix
with a warning.

### Supported compilers

MSVC 19.30+ (VS 2022), gcc 10+, clang 12+. MSVC 2017/2019 are supported only on the
`develop-cpp17` branch, and VS 2017 does not support CEF.
Building Skia additionally needs clang 12+ or gcc 11+ (see the table above).
