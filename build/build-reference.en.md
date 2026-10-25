# Build Reference

This is the single build reference for nim_duilib: command-line builds, dependency
preparation, packaging and troubleshooting. The legacy per-platform scripts
(`build_duilib_all_in_one.*`, `msvc_build.bat`, `linux_build.sh`, ...) are archived
under `build/legacy/` and no longer part of the build (deleted after full-platform
verification); use `python build/build.py` instead.

Chinese version: [build-reference.md](build-reference.md)

## 1. Requirements

| Item | Requirement |
| :--- | :--- |
| CMake | 3.24 or newer (the root project uses CMakePresets) |
| Python | **Python 3 is required**; the Skia build flow needs `python3` |
| Git | to fetch dependency sources |
| Ninja | bundled with Visual Studio on Windows; install via package manager elsewhere |
| Windows | Visual Studio 2022+ with the C++ workload, or MSYS2 / MinGW-w64 |
| Linux | gcc 10+ or clang 12+, plus X11/font development packages |
| macOS | Xcode command line tools 13.5+, cmake/ninja/gn via Homebrew |
| FreeBSD | clang, cmake, ninja, gn, fontconfig, freetype2 |

Without Python 3, `build.bat` / `build.py` fail immediately and print installation
hints; there is no Python-free build path.

## 2. Quick start

```bash
python build/build.py                  # Release: prepare deps + build library and examples
python build/build.py --config Debug   # Debug (separate build tree)
python build/build.py doctor           # environment check (no side effects)
python build/build.py pack --with-cef  # build an SDK package
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
| `build.py doctor` | environment check: toolchain, version manifest, dependency dirs, preflight |
| `build.py clean` | remove the build tree; `--deps` also removes dependency artifacts |

One invocation builds one architecture and one configuration. Build Debug and Release
by invoking the command twice:

```bash
python build/build.py --config Debug
python build/build.py --config Release
```

## 4. Common options

| Option | Description |
| :--- | :--- |
| `--config Release\|Debug` | configuration, default `Release`; no `--debug` alias |
| `--arch auto\|x86\|x64\|arm64` | target architecture, default host; required for Windows+Ninja arm64 |
| `--lang auto\|zh_CN\|en` | UI language, defaults to the system; also reads `DUILIB_LANG` |
| `--build-dir <dir>` | override the build directory (may be outside the repo) |
| `--deps-root <dir>` | dependency source/install root, defaults to the repo's parent |
| `--jobs N` | parallel build jobs |
| `--offline` | use cached dependency sources/archives only |
| `--dry-run` | print commands without executing them |
| `--cmake-arg <arg>` / `--build-arg <arg>` | forwarded to configure/build; use `--cmake-arg=-DFOO=1` when the value starts with `-` |
| `--vs-version N` | pin the Visual Studio major version (for example `17`) |
| `--cef-version 109\|142` | CEF version, default from the version manifest |
| `--skia-ref` / `--sdl-ref` | select a ref from the version manifest |
| `--sdl` / `--no-sdl` | force SDL on/off; off by default on Windows, forced on elsewhere |
| `--md` | use the MSVC dynamic runtime (/MD, /MDd) |
| `--cef-archive <file>` | use a local CEF archive and skip downloading |
| `--no-deps` (build) | skip dependency preparation |
| `--target <name>` (build) | build only the given target, e.g. `duilib` |
| `--no-examples` (build) | do not build examples |
| `--force` (deps) | rebuild dependencies |
| `--with-cef` / `--with-sdl` / `--with-examples` (pack) | include the given content |
| `--out-dir` (pack) | package output directory |

## 5. Dependencies

Dependency directories live next to the repository by default (`--deps-root` overrides):

```
<deps-root>/
  skia/            # Skia source (patched by skia_compile)
  skia_compile/    # Skia patch archives (e.g. skia.2026-09-16.src.zip)
  SDL/             # SDL3 source
  SDL3/            # SDL3 install prefix
  SDL3_build/      # SDL3 build directory
  cef_binary/      # CEF download/extract cache (full distribution on macOS)
```

Versions are pinned in [third_party_versions.json](../cmake/third_party_versions.json):

- **Skia**: fixed commit (`skia.default`) plus a patch zip; Visual Studio's bundled
  clang/ninja are used on Windows, system LLVM on Unix. Debug reuses the release build
  mode and only changes the runtime (`/MTd` or `/MDd`), so each configuration needs its
  own Skia variant directory.
- **SDL3**: fixed tag (`sdl.default`). Disabled on Windows by default; when enabled,
  `deps` builds Debug and Release static libraries. Unix builds shared libraries and
  writes rpath entries.
- **CEF**: fixed version branch (`cef.default`; `109` is available for Windows 7).
  Only `Release/*` and `Resources/*` are downloaded to:
  - Windows: `bin/libcef_win/<arch>` (142) or `bin/libcef_win_109/<arch>` (109)
  - Linux: `bin/libcef_linux`
  - macOS: `<deps-root>/cef_binary`
  Compilation uses the tracked headers and import libraries; the downloaded package's
  `CEF_VERSION` must match the tracked headers. Debug uses the Release runtime. MinGW
  does not support CEF.

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
for usage.

## 7. Packaging

SDKs are produced only by `pack.py`; the root CMake project has no `install()` target.

```bash
python build/build.py pack                    # Release SDK
python build/build.py pack --config Debug     # Debug SDK (name has -debug)
python build/build.py pack --with-cef         # include the CEF runtime
python build/build.py pack --with-examples    # include examples
python build/build.py pack --out-dir dist     # custom output directory
```

Packages contain `include/`, `lib/<arch>/<Config>/`, `bin/`, `lib/cmake/nim_duilib/`,
`LICENSES/` and `THIRD-PARTY-NOTICES.md`. Both compliance files must be kept when
redistributing the SDK.

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
| `Skia not found` | run `python build/build.py deps` first, or check `--deps-root` |
| `gn/ninja not found` | use VS's bundled tools on Windows; on Unix install `ninja` and `gn`. Unix ignores `gn.exe` inside the Skia tree (it cannot run there); install a native `gn` when only `gn.exe` exists |
| `CMake version is too old` | upgrade to CMake 3.24+ |
| CEF example does not start | ensure `bin/libcef_win/<arch>` (or `bin/libcef_linux`) contains the runtime; `pack --with-cef` includes it |
| CEF download fails | pass `--cef-archive <file>` or use `--offline` with a cache |
| No text on Linux | install fontconfig and CJK fonts; check `bin/resources/fonts` |
| Corrupted build tree | `python build/build.py clean` and retry |
| Configure fails after switching compilers (e.g. `cl.exe` cannot find `kernel32.lib`) | no manual cleanup needed: `build.py` detects the compiler change in the `out/<arch>/<Config>` cache and clears that directory |

Legacy troubleshooting notes from the script era remain in [build/README.en.md](README.en.md).

## 11. Differences from the legacy scripts

| Item | Legacy | Now |
| :--- | :--- | :--- |
| Entry point | 11 .bat/.sh scripts | `python build/build.py` |
| Configuration | Debug + Release in one run | one `--config` per run, invoke twice |
| Build tree | `build/build_temp` | `out/<arch>/<Config>` |
| Dependencies | handled inside all-in-one scripts | `deps` subcommand driven by the version manifest |
| SDK | assembled manually | `build.py pack` (with `LICENSES/`, `THIRD-PARTY-NOTICES.md`) |

The Visual Studio solutions (`build/duilib.sln`, `examples.sln`, ...) are kept; run
`python build/build.py deps` before opening them.
