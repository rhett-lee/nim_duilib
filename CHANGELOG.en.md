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
  tree was selected and `gn gen` failed; `.exe` files are now used on Windows only and
  Unix uses native tools.
- `build.py`: `vcvarsall` output (`Path`) and the inherited `PATH` coexisted, so tool
  lookups saw the stale value.

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
