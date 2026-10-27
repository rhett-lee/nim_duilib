# Build Reference

This is the single build reference for nim_duilib: command-line builds, dependency
preparation, packaging and troubleshooting. The legacy per-platform scripts
(`build_duilib_all_in_one.*`, `msvc_build.bat`, `linux_build.sh`, ...) are archived
under `build/legacy/` and no longer part of the build (deleted after full-platform
verification); use `python3 build/build.py` instead.

Chinese version: [build-reference.md](build-reference.md)

## 1. Requirements

| Item | Requirement |
| :--- | :--- |
| CMake | 3.24 or newer (the root project uses CMakePresets) |
| Python | **Python 3 is required**; the Skia build scripts invoke `python3`, so `python` / `py -3` are not a substitute |
| Git | to fetch dependency sources |
| Ninja | bundled with Visual Studio on Windows; install via package manager elsewhere |
| Windows | Visual Studio 2022+ with the C++ workload, or MSYS2 / MinGW-w64 |
| Linux | gcc 10+ or clang 12+, plus X11/font development packages |
| macOS | Xcode command line tools 13.5+, cmake/ninja/gn via Homebrew |
| FreeBSD | clang, cmake, ninja, gn, fontconfig, freetype2 |

Without Python 3, `build.bat` / `build.py` fail immediately and print installation
hints; there is no Python-free build path. If only `python` / `py -3` is available
and `python3` is missing from PATH, the preflight prints a warning and the Skia
build aborts with an error before compiling.

Commands in this document use `python3`: macOS, Linux and FreeBSD usually have no
`python` command. On Windows, run `build\build.bat` instead (same arguments, no
`python3` needed), or replace `python3` with `python`.

## 2. Quick start

```bash
python3 build/build.py                  # Release: prepare deps + build library and examples
python3 build/build.py --config Debug   # Debug (separate build tree)
python3 build/build.py doctor           # environment check (no side effects)
python3 build/build.py pack --with-cef  # build an SDK package
```

On Windows you can also run:

```bat
build\build.bat --config Debug
```

The build tree is `out/<arch>/<Config>` (for example `out/x64/Debug`), a sibling of
`build/`; `build/build_temp` is never written by the new entry. Executables and runtime
assets always go to `bin/`; libraries go to `lib/<arch>/<Config>/` (`lib/<Config>/` on Unix).

## 3. Commands

| Command | Purpose |
| :--- | :--- |
| `build.py` (default) | same as `build.py build`: prepare deps, then build one configuration |
| `build.py deps` | prepare Skia / SDL3 / CEF for the current configuration |
| `build.py build` | build one configuration; prepares deps unless `--no-deps` |
| `build.py test` | run the standard-library unittests under `build/` |
| `build.py pack` | package an SDK via `pack.py` |
| `build.py doctor` | environment check: toolchain, version manifest, dependency dirs, preflight (including compiler, `gn`) |
| `build.py clean` | remove the build tree; `--deps` also removes dependency artifacts |

One invocation builds one architecture and one configuration. Build Debug and Release
by invoking the command twice:

```bash
python3 build/build.py --config Debug
python3 build/build.py --config Release
```

Debug and Release examples share the same output names and both write into `bin/`,
while their object files live in separate per-configuration trees. When switching
configurations, `build.py` first removes the other configuration's example outputs and
their `bin/.build-config/<target>` stamps so the requested configuration relinks;
otherwise Ninja sees `bin/` files newer than its own objects and skips the link steps
(`ninja: no work to do`).

## 4. Common options

| Option | Description |
| :--- | :--- |
| `--config Release\|Debug` | configuration, default `Release`; no `--debug` alias |
| `--arch auto\|x86\|x64\|arm64` | target architecture, default host; required for Windows+Ninja arm64 |
| `--lang auto\|zh_CN\|en` | UI language, defaults to the system; also reads `DUILIB_LANG` |
| `--build-dir <dir>` | override the build directory (may be outside the repo) |
| `--deps-root <dir>` | dependency source/install root, defaults to the repo's parent (next to `nim_duilib`); Skia, SDL3 and CEF are installed here |
| `--jobs N` | parallel build jobs, defaulting to the CPU logical core count |
| `--offline` | use cached dependency sources/archives only |
| `--dry-run` | print commands without executing them |
| `--cmake-arg <arg>` / `--build-arg <arg>` | forwarded to configure/build (repeatable); use `--cmake-arg=-DFOO=1` when the value starts with `-` |
| `--vs-version N` | pin the Visual Studio major version (for example `17`); Windows only |
| `--cef-version 109\|latest` | CEF version: `109` is the legacy build, `latest` follows the manifest default (currently 142 and tracking manifest upgrades) |
| `--skia-ref` / `--sdl-ref` | select a ref from the version manifest, e.g. `chrome/m142`, `main` or `release-3.4.18` |
| `--sdl` / `--no-sdl` | Windows only; off by default on Windows, forced on elsewhere |
| `--md` | use the MSVC dynamic runtime (/MD, /MDd); Windows only |
| `--no-absolute-rpath` | Unix only; write only origin-relative rpath entries |
| `--cef-archive <file>` | use a local CEF archive and skip downloading (still verified against the official `.sha1`) |
| `--no-deps` (build) | skip dependency preparation and reuse the ready Skia/SDL3/CEF |
| `--target <name>` (build) | build only the given CMake target, e.g. `duilib` |
| `--no-examples` (build) | do not build examples (the project is still configured) |
| `--force` (deps) | ignore completion stamps and rebuild dependencies |
| `--with-cef` / `--with-sdl` / `--with-examples` (pack) | include the given content (Unix always packages SDL3, so `--with-sdl` is unnecessary) |
| `--out-dir` (pack) | package output directory (default `dist/`) |

`--sdl` is a build switch only; a package includes SDL3 only when `--with-sdl`
is also given (using `--sdl` alone now prints a hint and leaves package contents
unchanged).

`out/<arch>/<Config>` reuses the CMake cache, and `DUILIB_MD`,
`DUILIB_BUILD_EXAMPLES`, `DUILIB_ENABLE_SDL`, `DUILIB_ENABLE_CEF`,
`DUILIB_CEF_VERSION`, `DUILIB_SKIA_REF` and friends are stateful options.
`build.py` always passes the resolved values explicitly, so running
`--no-examples --md` first and then a plain command does not inherit the old
options; `DUILIB_ENABLE_CEF` is also re-evaluated per platform (ON for Windows
MSVC, Linux and macOS; OFF for MinGW/MSYS2 and FreeBSD). Only when configuring
the same directory directly with `cmake` do you need to clear the cache (delete
the build directory and reconfigure). When CEF is turned off through
`--cmake-arg`, stale `cef`/`CefBrowser` entries left in `DUILIB_EXAMPLES` are
dropped or skipped instead of failing to link. `DUILIB_MACOS_BUNDLE_EXAMPLES`
(the macOS App Bundle target list) is set only through `--cmake-arg` and persists
in the cache; pass an empty value to disable it. See
[macos-app-bundle.en.md](macos-app-bundle.en.md).

On Unix, examples carry origin-relative rpath entries (`$ORIGIN/../lib`,
`@loader_path/../lib`, etc.) plus the SDL3 prefix as a build-tree fallback. To
produce artifacts with no build-machine absolute paths, pass
`--no-absolute-rpath` (equivalent to
`--cmake-arg=-DDUILIB_RPATH_ABSOLUTE=OFF`); examples in the build tree then need
`LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`, while packaged programs still find the
bundled SDL3 through the relative entries.

## 5. Dependencies

Dependency directories live next to the repository by default (`--deps-root` overrides):

```
<deps-root>/
  skia/            # Skia source (patched by skia_compile)
  skia_compile/    # skia_compile repository (cloned by deps when the patch is missing)
  SDL/             # SDL3 source
  SDL3/            # SDL3 install prefix
  SDL3_build/      # SDL3 build directory
  cef_binary/      # CEF download/extract cache (full distribution on macOS)
```

The Skia patch archive is obtained automatically by `build.py deps` (and the default
`build`): when missing, the whole
[skia_compile](https://github.com/rhett-lee/skia_compile) repository is cloned into
`<deps-root>/skia_compile`; an existing checkout is checked against the current commit
first (the same name may be committed more than once, e.g.
`skia.2026-09-16.src.zip`), so a local copy that was replaced with an older version (or
deleted) is restored from the checkout before `git pull` fetches the latest commit; a
changed patch makes the matching Skia artifacts rebuild. A failed update keeps the local
copy (matching the legacy script). Building directly with CMake
(`DUILIB_DEPS_MODE=auto` or the `duilib_deps` target) performs the same logic through
the dependency script. `--offline` never touches the network and only restores the local
copy, so the archive must be prepared in advance; `doctor` reports whether it exists and
whether it differs from the local checkout.

All network operations (git clone/fetch, CEF downloads) share one retry policy kept in
the `retry` block of
[third_party_versions.json](../cmake/third_party_versions.json):

```json
"retry": { "maxAttempts": 0, "delaySeconds": 20 }
```

`maxAttempts=0` means retry forever (the default) and `delaySeconds` is the fixed delay
between attempts. `git pull` is the exception: it is tried once and a failure only warns
before continuing with the local copy (matching the legacy script). Switching a
dependency checkout to the manifest ref fetches only when the target commit is missing
locally; an already fetched commit is checked out directly, which also works offline.

Versions are pinned in [third_party_versions.json](../cmake/third_party_versions.json):

- **Skia**: fixed commit (`skia.default`) plus a patch zip; Visual Studio's bundled
  clang/ninja are used on Windows. On non-Windows platforms Skia is built with
  clang/clang++ whenever available (GCC-built Skia performs noticeably worse) and only
  falls back to gcc/g++ when clang is missing; the artifact directory is
  `out/llvm.<arch>.release` or `out/gcc.<arch>.release` accordingly. Debug reuses the
  release build mode and only changes the runtime (`/MTd` or `/MDd`), so each
  configuration needs its own Skia variant directory. If the local `skia/` checkout is
  not at the manifest commit (or the tag/branch selected by `--skia-ref`), `deps`
  switches it to the manifest revision; a branch ref only requires the checkout to be
  on that branch and does not chase remote tip on every run.
- **SDL3**: the default ref is the `main` branch (`sdl.default`; the
  `release-3.4.18` tag remains in `refs` but does not compile, so do not select it
  unless you need it via `--sdl-ref`). Disabled on Windows by default; when enabled,
  `deps` builds Debug and Release static libraries. Unix builds shared libraries and
  writes rpath entries (relative entries first; the absolute fallback is controlled
  by `DUILIB_RPATH_ABSOLUTE`). If the local `SDL/` checkout is on another
  branch/tag, `deps` switches to the manifest ref before building (a branch ref
  only requires being on that branch, without chasing the remote tip); the SDL3
  install stamp records the source commit, so artifacts built from the wrong
  revision are rebuilt automatically (once when upgrading to this version).
- **CEF**: fixed version branch (`cef.default`; `109` is available for Windows 7).
  The `_minimal` distributions are used everywhere (about half the size of the standard
  package; they contain `include/`, `Release/` and `Resources/`). Only
  `Release/*` and `Resources/*` are installed to:
  - Windows: `bin/libcef_win/<arch>` (142) or `bin/libcef_win_109/<arch>` (109)
  - Linux: `bin/libcef_linux`
  - macOS: `<deps-root>/cef_binary`
  Compilation uses the tracked headers and import libraries; the downloaded package's
  `CEF_VERSION` must match the tracked headers. Debug uses the Release runtime and
  framework (CEF distributions usually only ship
  `Release/Chromium Embedded Framework.framework`; CMake falls back automatically).
  MinGW does not support CEF.

Downloaded CEF archives are verified against the official `<download-url>.sha1`
published by CEF. The hash file is cached next to the archive and reused offline. If
it cannot be fetched, `deps` falls back to the manifest's `sha256`; when both are
unavailable it only warns and skips the check.

When dependency artifacts exist without a stamp file, `deps` validates and adopts them
(Skia: source HEAD; CEF: tracked header major version) instead of rebuilding or
re-downloading.

`DUILIB_DEPS_MODE` controls dependency handling: `off` (check only), `prepare` (prepare
from the build target), `auto` (prepare at configure time). `build.py` uses `prepare`;
`clean` keeps dependency caches unless `--deps` is given.

## 6. Outputs

| Path | Content |
| :--- | :--- |
| `out/<arch>/<Config>/` | CMake build tree (safe to delete) |
| `lib/<arch>/<Config>/` | static libraries on Windows; `lib/<Config>/` on Unix |
| `bin/` | example executables, `resources/`, CEF runtime |
| `bin/resources/` | XML skins, images, `fonts/`, `lang/`, `themes/` |

Trimming `bin/resources` for release: delete `fonts/` when no custom fonts are used and
`lang/` when multilingual text is unused; keep only `public/` and `global.xml` under
`themes/default/`. To embed resources in the executable, zip the trimmed `resources/`
into `resources.zip`, replace `bin/resources.zip`, and rebuild; see `examples/basic`.

Ship the CEF runtime directory with your application; see [docs/CEF.en.md](../docs/CEF.en.md)
for usage. Only the `cef` and `CefBrowser` examples link CEF (`DUILIB_EXAMPLE_CEF`);
other example executables do not import `libcef`. `DUILIB_ENABLE_CEF` only controls
whether the library compiles the CEF control sources.

### macOS App Bundles

On macOS, `basic` is built as `bin/basic.app` by default. Any other example can add
`set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)` to its `CMakeLists.txt`, or be selected with
`-DDUILIB_MACOS_BUNDLE_EXAMPLES=<target list>`; no C++ changes are needed. Resources
are copied into `Contents/Resources/duilib/resources` (isomorphic to the plain
`<exe directory>/resources/`), and SDL3 dylibs into `Contents/Frameworks` with a
relative rpath. On macOS, `GlobalManager::GetResourceRootPath(true)` (the default
argument) returns that directory; with `false` it falls back to `resources/` next
to the executable.

See [macos-app-bundle.en.md](macos-app-bundle.en.md) for the switch, layout, icons,
verification, signing and packaging details.

## 7. Packaging

SDKs are produced only by `pack.py`; the root CMake project has no `install()` target.

```bash
python3 build/build.py pack                    # Release SDK
python3 build/build.py pack --config Debug     # Debug SDK (name has -debug)
python3 build/build.py pack --with-cef         # include the CEF runtime
python3 build/build.py pack --with-examples    # include examples
python3 build/build.py pack --out-dir dist     # custom output directory
```

Packages contain `include/`, `lib/<arch>/<Config>/`, `bin/`, `lib/cmake/nim_duilib/`,
`LICENSES/` and `THIRD-PARTY-NOTICES.md`. Both compliance files must be kept when
redistributing the SDK.

With `--with-examples` but without `--with-cef`, the `cef` and `CefBrowser`
binaries (`cef.exe`/`CefBrowser.exe` on Windows) are skipped automatically so the
package never ships examples that cannot start without the libcef runtime; add
`--with-cef` for runnable CEF examples.

macOS is the exception: CEF examples are App Bundles (`bin/cef.app`,
`bin/CefBrowser.app` directories), and ordinary examples can be built as
`bin/<example>.app` as described in [macos-app-bundle.en.md](macos-app-bundle.en.md).
`.app` directories are collected as a whole (executable, resources and Frameworks under
`Contents/`). With `--with-examples`, `--with-cef` decides whether the CEF examples
ship: omitted means they are skipped with a warning, specified means the whole bundle
(CEF framework and Helpers under `Contents/Frameworks`) is packaged with its original
layout. Bundles carry their own runtime, so on macOS `--with-cef` only controls the
examples and never collects CEF development files or a runtime directory (the SDK has
no macOS CEF development files; build from the source tree as described in
[Release.en.md](../docs/Release.en.md)).

## 8. Platform × compiler × dependencies × optional modules

| Platform | Compiler | Skia | SDL3 | CEF | WebView2 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Windows | MSVC 2022+ | required | off by default, enable with `--sdl` | on by default (109/142) | supported |
| Windows | MSYS2 / MinGW-w64 gcc, clang | required | off by default, may enable | not supported | supported |
| Linux | gcc 10+ / clang 12+ | required | forced on | on by default | not supported |
| macOS | clang (Xcode) | required | forced on | on by default | not supported |
| FreeBSD | clang | required | forced on | not supported | not supported |

Per-distribution system packages (fonts, GL, X11, ibus, ...) and Skia build guides are in
[skia_compile](https://github.com/rhett-lee/skia_compile). macOS needs
`brew install cmake ninja` and `gn` built from source; FreeBSD uses
`sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2`.

### Forcing a toolchain

CMake picks the compiler by default: `CC`/`CXX` environment variables win,
otherwise `cc`/`c++` are used (usually gcc/g++ on Linux). To pin a toolchain:

```bash
# Option 1: environment variables (recommended; applies to doctor/deps/build)
CC=clang CXX=clang++ python3 build/build.py
CC=gcc   CXX=g++     python3 build/build.py

# Option 2: pass it to the CMake configure step
python3 build/build.py --cmake-arg -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
python3 build/build.py --cmake-arg -DCMAKE_C_COMPILER=gcc   -DCMAKE_CXX_COMPILER=g++
```

- On non-Windows platforms `build.py doctor` reports the compiler selected from `CC`/`CXX`.
- No manual cleanup is needed after switching compilers: `build.py` detects the
  changed compiler in the `out/<arch>/<Config>` cache and clears that build directory.
- Windows/MSVC ignores `CC`/`CXX` (the toolchain comes from vcvars); in MSYS2 use
  `CC=gcc CXX=g++` or `CC=clang CXX=clang++`.
- On non-Windows platforms Skia's toolchain is decoupled from the main build: Skia
  always prefers clang/clang++ (GCC-built Skia performs noticeably worse), so a
  gcc-based main build does not change Skia's compiler.

Each MSYS2 environment uses the toolchain in its own directory (inside an MSYS2
terminal `MSYSTEM` and `PATH` are already set):

| Environment | `MSYSTEM` | toolchain directory | `CC`/`CXX` | Skia directory candidates (`skia/out/`, first match wins) | Package suffix |
| --- | --- | --- | --- | --- | --- |
| MINGW64 (msvcrt) | `MINGW64` | `E:\msys64\mingw64\bin` | `gcc` / `g++` | `gcc.msvcrt.<arch>.release` → `mingw64-gcc.<arch>.release` → `gcc.<arch>.release` | `-mingw-gcc` |
| UCRT64 | `UCRT64` | `E:\msys64\ucrt64\bin` | `gcc` / `g++` | `mingw64-gcc.<arch>.release` → `gcc.<arch>.release` → `gcc.msvcrt.<arch>.release` | `-mingw-gcc` |
| CLANG64 | `CLANG64` | `E:\msys64\clang64\bin` | `clang` / `clang++` | `mingw64-llvm.<arch>.release` → `llvm.<arch>.release` | `-mingw-llvm` |

MINGW64 (msvcrt) and UCRT64 (ucrt) gcc static libraries use different C runtimes, so
Skia must come from a matching directory; MINGW64 therefore prefers the `.msvcrt`
variant. Both use the `-mingw-gcc` suffix, so their packages **must not be archived
into the same directory**. A standalone llvm-mingw uses
`mingw64-llvm.<arch>.release` (libc++), which is also not the same artifact as MSYS2
CLANG64's `llvm.<arch>.release`; `pack.py` tells them apart by library symbols
(`libstdc++` / `libc++`). Use `--skia-lib-dir <dir>` to force a specific Skia
directory (the build equivalent is `-DDUILIB_SKIA_LIB_SUBPATH=<dir>`).

When invoking from PowerShell, export `MSYSTEM` yourself and prepend the matching
`bin` directory to `PATH`; `build.py` skips vcvars whenever `MSYSTEM` is set and
uses the `CC`/`CXX` compilers. A standalone llvm-mingw works the same way: set
`MSYSTEM`, point `CC`/`CXX` at its absolute `clang.exe` / `clang++.exe`, and make
sure the Skia libraries are the `mingw64-llvm.<arch>` variant.

## 9. Runtime selection

- Static runtime (/MT, /MTd) is the default.
- `--md` switches to the dynamic runtime (/MD, /MDd); `pack.py` uses
  `--msvc-runtime md`.
- Skia and nim_duilib must use the same runtime; `deps` builds the matching Skia variant.
- If nim_duilib is built as a DLL, the dynamic runtime is mandatory.

## 10. Troubleshooting

| Symptom | Action |
| :--- | :--- |
| `Visual Studio not found` | install VS 2022+ with the C++ workload, or pass `--vs-version` |
| `Skia not found` | run `python3 build/build.py deps` first, or check `--deps-root` |
| `gn/ninja not found` | use VS's bundled tools on Windows; on Unix install `ninja` and `gn`. Unix ignores `gn.exe` inside the Skia tree (it cannot run there); install a native `gn` when only `gn.exe` exists |
| `python: command not found` (macOS/Linux) | these systems usually provide only `python3`: use `python3 build/build.py ...`; on Windows run `build\build.bat` instead |
| `python3 not found` (error while building Skia) | the Skia build scripts invoke `python3` and do not accept `python` / `py -3`; install Python 3 and make sure `python3 --version` works (on Windows add the Python install directory to PATH) |
| `CMake version is too old` | upgrade to CMake 3.24+ |
| CEF example does not start | ensure `bin/libcef_win/<arch>` (or `bin/libcef_linux`) contains the runtime; `pack --with-cef` includes it |
| macOS Debug build fails to copy `Chromium Embedded Framework.framework` | CEF distributions usually ship only the Release framework; CMake now detects this and falls back to `Release/`, printing `CEF Debug framework not found; using Release framework`. If neither exists, configuration fails early with a `CEF_ROOT` hint |
| Packaged examples load the builder's `.so` | examples carry a relative rpath (`$ORIGIN/../lib`, `$ORIGIN/libcef_linux`) so bundled libraries win; use `pack --with-cef` when examples depend on libcef |
| Build-machine absolute paths remain in the binary (`otool`/`readelf`) | They come from the `DUILIB_RPATH_ABSOLUTE=ON` SDL3 fallback; build packages with `--cmake-arg=-DDUILIB_RPATH_ABSOLUTE=OFF`, or disable rpath entirely with `DUILIB_SDL_RPATH=OFF` |
| CEF download fails | pass `--cef-archive <file>` or use `--offline` with a cache |
| No text on Linux | install fontconfig and CJK fonts; check `bin/resources/fonts` |
| Corrupted build tree | `python3 build/build.py clean` and retry |
| Configure fails after switching compilers (e.g. `cl.exe` cannot find `kernel32.lib`) | no manual cleanup needed: `build.py` detects the compiler change in the `out/<arch>/<Config>` cache and clears that directory |
| Building Debug and then Release finishes immediately (`ninja: no work to do`) and `bin/` still holds Debug examples | both configurations write examples into the shared `bin/`; `build.py` removes the other configuration's outputs and `bin/.build-config` stamps before relinking. When invoking CMake directly, remove the same-named examples from `bin/` yourself |
| `--cef-version 109` cannot find `libcef_dll_wrapper_109` | fixed: the wrapper source directory and target name now follow `DUILIB_CEF_VERSION`; run `build.py clean` and reconfigure an old build tree |
| `--cef-version 109` Debug link fails with `_ITERATOR_DEBUG_LEVEL` 0 vs 2 (LNK2038) | fixed: 109 configurations force `USE_SANDBOX=OFF`, so the wrapper's `_HAS_ITERATOR_DEBUGGING=0` no longer conflicts with a Debug project |
| `pack` reports that `lib/<arch>/<Config>` mixes MSVC `.lib` and MinGW `.a` | old libraries survive a toolchain switch; delete that directory (or run `build.py clean`) and rebuild with a single toolchain |
| `pack --with-cef` reports the wrapper for the selected CEF version is missing | the build and pack CEF versions must match (e.g. `build --cef-version 109` with `pack --with-cef --cef-version 109`); the error names the version actually present in the lib directory |
| `pack --with-examples` reports that `bin` does not match the target x86/x64 architecture | on Windows `bin/` is shared by all architectures; remove the other architecture's exe/dll files (or keep only the target build's artifacts) and repack |
| MinGW build or link (especially Debug) fails with `out of memory allocating ...` | reduce parallelism with `--jobs 2`; GNU ld already gets `-Wl,--no-keep-memory` (lld on clang/CLANG64 does not need it) |
| MinGW examples fail to start after disabling static linking (`DUILIB_MINGW_STATIC=OFF`) | that mode depends on compiler runtime DLLs; copy `libstdc++-6.dll`, `libgcc_s_seh-1.dll` (x86: `libgcc_s_dw2-1.dll`) and `libwinpthread-1.dll` next to the exe; the default static build does not need them |
| MinGW Debug link fails with `No space left on device` | a Debug example is roughly 200–280 MB and all examples take about 10 GB; clean `bin/` and retry, or use the Release configuration |
| `deps` fails building SDL3 with `cl.exe`/`LNK1104`, or `--sdl` fails after switching MSVC/MinGW | the SDL3 build directory was used by another toolchain or runtime; dependency preparation clears it automatically when the CRT does not match, so simply retry |

Legacy troubleshooting notes from the script era remain in [build/README.en.md](README.en.md).

## 11. Differences from the legacy scripts

| Item | Legacy | Now |
| :--- | :--- | :--- |
| Entry point | 11 .bat/.sh scripts | `python3 build/build.py` |
| Configuration | Debug + Release in one run | one `--config` per run, invoke twice |
| Build tree | `build/build_temp` | `out/<arch>/<Config>` |
| Dependencies | handled inside all-in-one scripts | `deps` subcommand driven by the version manifest |
| SDK | assembled manually | `build.py pack` (with `LICENSES/`, `THIRD-PARTY-NOTICES.md`) |

The Visual Studio solutions (`build/duilib.sln`, `examples.sln`, ...) are kept; run
`python3 build/build.py deps` before opening them.

The full build/pack regression matrix, per-platform status and suggested order are in
[test-matrix.md](test-matrix.md) (Chinese).
