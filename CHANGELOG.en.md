# Changelog

All notable changes to this project are documented in this file. The format is
based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

Version numbers are maintained in `duilib/duilib_version.h` in
`MAJOR.MINOR.PATCH.BUILD` form; `BUILD` uses `MMDD` without a leading zero.

## [Unreleased]

### Added

- Root CMake project and unified entry `python build/build.py` (or `build\build.bat`):
  `build` (default), `deps`, `test`, `pack`, `doctor`, `clean`; build trees live in
  `out/<arch>/<Config>`.
- Automatic dependency preparation driven by `cmake/third_party_versions.json`
  (Skia, SDL3, CEF), with download caches, sha256 checks, resume and a process lock;
  existing artifacts are adopted through stamp files.
- Bilingual build reference `build/build-reference.md` / `.en.md`; `build/README.md`
  and `README.en.md` now focus on prerequisites and packaging.
- `build/test_build.py`: standard-library unit tests for the build entry and i18n.
- Compliance: `THIRD-PARTY-NOTICES.md`; SDK license directory renamed to `LICENSES/`.

### Changed

- One invocation builds one `--config` (Debug/Release); invoke twice for both.
  `build/autotest.py` now builds through `build.py`.
- CEF runtime files are placed in `bin/libcef_win[/_109]/<arch>` or `bin/libcef_linux`;
  compilation uses the tracked headers and import libraries.

### Removed

- `build/build.md`; its content moved into `build/build-reference.md`.
- Legacy per-platform build scripts are archived under `build/legacy/` (not part of
  the build; removed after full-platform verification).

### Fixed

- `build.py`: options placed before a subcommand (e.g. `--config Debug build`) were
  overwritten by the subcommand defaults; subcommand parsers now suppress defaults.
- `build.py`: reusing the same `out/<arch>/<Config>` after switching compilers
  (MSVC ↔ MinGW) kept the stale CMake cache and failed to configure; the build tree
  is now cleared automatically when the compiler changes.
- Dependency preparation: on Unix the Windows `gn.exe`/`ninja.exe` inside the Skia
  tree was selected, and the PATH fallback was skipped because the result variable
  had already been set to an empty string (so `gn` was reported missing). `.exe`
  files are now used on Windows only; Unix searches PATH through a separate
  temporary variable.
- `build.py`: `vcvarsall` output (`Path`) and the inherited `PATH` coexisted, so tool
  lookups saw the stale value.
- `build.py doctor`: the preflight ran with `DUILIB_DEPS_MODE=off`, which skipped the
  dependency tool checks entirely, so `gn` was missing from the environment summary.
  `gn` is now always checked; `doctor` uses `prepare` and fails when it is missing,
  while `build --no-deps` (`off`) only warns so builds reusing prepared dependencies
  still work.
- Preflight: non-Windows platforms now require a matching `gcc/g++` or `clang/clang++`
  pair; `CC`/`CXX` are honoured when set, otherwise gcc is probed before clang.
- Dependency preparation: the Skia build scripts invoke `python3` and do not accept
  `python` / `py -3`, while the preflight accepted either, so the failure only showed
  up while compiling Skia. The preflight now warns when `python3` is missing, and the
  Skia build step aborts with an error before compiling.
- Docs and command examples: macOS/Linux/FreeBSD usually provide only `python3`, so the
  documented `python build/build.py` failed with "command not found"; examples now use
  `python3 build/build.py`, and Windows can keep using `build\build.bat` directly.
- Dependency preparation (macOS): `SDL3 build failed` was reported even after a successful
  SDL3 build and install; the readiness check only accepted `libSDL3.so`, while macOS
  installs `libSDL3.dylib`. macOS is now checked for the dylib.
- Skia's toolchain is now decoupled from the main build: non-Windows platforms prefer
  `clang/clang++` (GCC-built Skia performs noticeably worse) and only fall back to
  `gcc/g++` when clang is missing; artifacts land in `out/llvm.<arch>.release` or
  `out/gcc.<arch>.release` accordingly.
- Fixed `SDL3 build failed` after a successful first SDL3 build: the `.duilib-sdl`
  completion marker was written after the readiness check, so the very first install
  always failed; it is now written before the check.
- Unix examples now use relative rpaths (`$ORIGIN` / `@loader_path`, including
  `../lib` and `libcef_linux`) so packaged binaries load the bundled SDL3/CEF
  libraries instead of the builder's absolute paths; the packer also detects Unix
  examples by the executable bit (previously `.exe` only).
- When packaging without CEF (`--with-cef` omitted), the `cef` and `CefBrowser`
  examples (`cef.exe`/`CefBrowser.exe` on Windows) are skipped automatically so the
  package never contains binaries that cannot start; the bundled README says so.
- Packaging supports the macOS CEF example App Bundles (`bin/cef.app`,
  `bin/CefBrowser.app`): with `--with-examples`, `--with-cef` controls whether they
  ship, exactly like other platforms — omitted means skipped with a warning, specified
  means the whole directory (CEF framework, Helpers and symlinks under
  `Contents/Frameworks`) is collected with its original layout. On macOS `--with-cef`
  only controls examples; it collects no CEF development files or runtime (bundles
  carry the framework themselves). The previous "error out" behaviour is removed.
- Only the `cef` and `CefBrowser` examples link CEF: with `DUILIB_ENABLE_CEF` enabled
  in the root project, every example used to link `libcef` (`ldd` on Linux showed
  `libcef.so` for `basic` and friends). This is now controlled per example by
  `DUILIB_EXAMPLE_CEF`; `DUILIB_ENABLE_CEF` only decides whether the library compiles
  the CEF control sources.

## [1.0.2026.1023] - 2026-10-23

### Added

- Release packaging script `build/pack.py`: collects already-built artifacts
  under the SDK contract and produces zip/tgz archives with `MANIFEST.txt`,
  `SHA256SUMS` and an in-package `VERSION`.
- Release orchestration script `build/release.py` with `check`, `package`,
  `finalize` and `clean`; it never performs git write operations.
- Integration layers inside the package: VS `msvc/nim_duilib.props`, `.targets`
  and x64/x86 manifests; CMake `lib/cmake/nim_duilib/*.cmake` providing
  `nim_duilib::duilib`.
- Top-level user documentation ships with the SDK, with Markdown links rewritten
  for the package layout.
- `--with-cef` now also packages the CEF runtime, keeping the
  `bin/libcef_win[_109]/<arch>/` layout (`libcef.dll`, `*.pak`, `icudtl.dat`,
  `locales/`, ...) on Windows.
- `MANIFEST.txt` and the release documentation state that WebView2 depends on
  the Microsoft Edge WebView2 Evergreen Runtime installed on the system and
  that the SDK only ships the loader; dynamic/static loader usage and
  troubleshooting are documented.
- The release documentation and `MANIFEST.txt` add Windows package verification
  commands (PowerShell `Get-FileHash`, `certutil -hashfile`) that do not require
  `sha256sum`.
- License compliance: added `licenses/expat.LICENSE.txt` (expat bundled by Skia,
  MIT) and `licenses/RobotoMono.LICENSE.txt` (bundled font, Apache-2.0), plus
  `docs/misc/License-Compliance.md` mapping shipped components to their
  licenses and package files.
- Release documentation: `docs/Release.md`, `docs/misc/Release-Checklist.md` and
  the packaging work record.

### Changed

- Version numbers are unified in `duilib/duilib_version.h`; `msvc/dllRes/version.h`
  is now an RC adapter.
- Resources moved to `bin/resources/`, next to the example programs.
- Package names gain feature suffixes: `-sdl`, `-cef` / `-cef109`, `-examples`,
  `-debug`, so packages of the same version can coexist under `dist/`.
- `docs/` ships only top-level user documentation; subdirectories such as
  `misc/` and `specs/` are not packaged.
- Archive semantics: tar entries use the source file permissions (executable and
  setuid bits preserved) and zip stores symbolic links as `S_IFLNK` entries, so
  Unix packages keep the `.so` link chain and executable permissions after
  extraction; symbolic links are no longer listed in `SHA256SUMS`.
- Reproducible builds: `SOURCE_DATE_EPOCH` normalizes archive entry times, the
  gzip header and the build time in `MANIFEST.txt`; without it, entries use
  source/artifact mtimes and are sorted, so consecutive packings of unchanged
  input are byte-identical.

### Fixed

- Fixed the macOS build failure caused by the root `VERSION` file clashing with
  the standard `<version>` header.
- The SDK integration layers (VS property sheet and CMake config) now add
  `/NODEFAULTLIB:"libcmt.lib"` for Debug builds, removing the downstream
  LNK4098 warning.

### Removed

- Removed the root `VERSION.txt` to avoid a second source of version data.
