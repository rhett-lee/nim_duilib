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
- When the Skia patch archive is missing, `build.py deps` / `build` clones the whole
  [skia_compile](https://github.com/rhett-lee/skia_compile) repository into
  `skia_compile/`; an existing checkout is updated with `git pull`, falling back to the
  local copy when the update fails (matching the legacy script); `--offline` never
  touches the network. Building directly with CMake (`DUILIB_DEPS_MODE=auto` or the
  `duilib_deps` target) performs the same clone automatically.
- Unified network retries: `git clone/fetch` and CEF downloads share the `retry` block in
  the version manifest (`maxAttempts=0` = unlimited, `delaySeconds=20`).
- Bilingual build reference `build/build-reference.md` / `.en.md`; `build/README.md`
  and `README.en.md` now focus on prerequisites and packaging.
- `build/test_build.py`: standard-library unit tests for the build entry and i18n.
- Compliance: `THIRD-PARTY-NOTICES.md`; SDK license directory renamed to `LICENSES/`.
- SDK integration layer (macOS App Bundles): `nim_duilib_copy_runtime` gained a
  `FRAMEWORKS` option (the target must be a `MACOSX_BUNDLE`) that copies the
  package's `lib/libSDL3*.dylib` into `<target>.app/Contents/Frameworks` and
  sets the executable's rpath to `@executable_path/../Frameworks`
  (`BUILD_WITH_INSTALL_RPATH`, so the bundled libraries win over the SDK's
  build-tree absolute path).
  `nim_duilibConfig.cmake` records the SDL3 dylib names, and the bundled README
  and integration docs describe the option.
- SDK integration layer (bundle resources): on a `MACOSX_BUNDLE` target without
  `DESTINATION`, `nim_duilib_copy_runtime`'s `RESOURCES` now copies into
  `Contents/Resources/duilib/`, matching
  `GlobalManager::GetResourceRootPath(true)`.
- Package test suite: new C11 macOS App Bundle consumer scenario. A
  `nim_duilib_sdk_bundle` target in `build/sdk_test` (with `bundle_main.cpp`)
  builds `sdk_bundle.app` from the SDK package and verifies
  `Contents/Frameworks/libSDL3*.dylib`, `@rpath`/`LC_RPATH`,
  `Contents/Resources/duilib`, then runs a copy of the app outside the build tree.

### Changed

- One invocation builds one `--config` (Debug/Release); invoke twice for both.
  `build/autotest.py` now builds through `build.py`.
- CEF runtime files are placed in `bin/libcef_win[/_109]/<arch>` or `bin/libcef_linux`
  using the `_minimal` distributions on every platform (about half the size of the
  standard package, still containing `include/`, `Release/` and `Resources/`);
  compilation uses the tracked headers and import libraries.
- Downloaded CEF archives are verified against the official `<download-url>.sha1`
  published by CEF. The hash file is cached next to the archive and reused offline;
  when it cannot be fetched, `deps` falls back to the manifest's `sha256` and only
  warns when neither is available.
- `doctor` reports dependency state in more detail: Skia source and patch archive,
  SDL source and prefix, and the CEF runtime are listed separately, noting that
  `deps` fetches them while `--offline` cannot; the interpreter is shown under its
  actual command name (`python3` on Linux).

### Removed

- `build/build.md`; its content moved into `build/build-reference.md`.
- Legacy per-platform build scripts are archived under `build/legacy/` (not part of
  the build; removed after full-platform verification).

### Fixed

- Build (Windows link dependencies): Skia and SDL3 static libraries were passed to
  `target_link_libraries` by name (with `link_directories`), so Ninja did not
  track them as link dependencies; after rebuilding a dependency the example
  binaries were not relinked and packages could ship binaries built against the
  old libraries. Static libraries are now linked by absolute path on all
  platforms (MSVC `SDL3-static[d].lib`, MinGW `libSDL3.a`, Skia `.a`/`.lib`);
  shared libraries (Unix SDL3 `.so`/`.dylib`) keep the plain names resolved by
  `-l` so build-machine paths are not baked into dylib `install_name`/rpath.
- Build (Windows examples): Debug enables incremental linking by default, and
  linking many examples in parallel occasionally failed with
  `LNK1000: Internal error during IncrBuildImage` (plus stale `.ilk` files).
  Examples now pass `/INCREMENTAL:NO`; the extra link time is negligible.
- Packaging (same-name Debug/Release examples): a POST_BUILD step now records the
  configuration in `bin/<target>.build-config`; `pack.py` skips examples built
  for the other configuration and only warns when the marker is missing, instead
  of a blanket "may be mixed" warning.
- Dependency preparation (SDL3): Debug and Release produce the same
  `SDL3-static.lib` name and install into one prefix; both configurations were
  built and Release was installed last, so Debug builds actually linked the
  Release static library (`/MTd` vs `/MT`, `/MDd` vs `/MD`). Single-config
  generators (Ninja, the `build.py` default) now build and install only the
  current configuration, and the stamp records the configuration so switching
  between Debug/Release rebuilds it; multi-config generators (Visual Studio)
  still build both and print a notice that Debug links the Release library.
- Build (CMake example list): `DUILIB_EXAMPLES` wrote its derived value into the
  cache, so after turning `DUILIB_ENABLE_CEF` off the old list still contained
  `cef`/`CefBrowser` and linking failed with unresolved `CefManager::GetInstance`
  symbols. The platform default list is now re-derived on every configure (a
  cached value is treated as a user override only when it differs from the
  defaults and from the previously derived list), and the root project also
  skips CEF examples with a notice when `DUILIB_ENABLE_CEF=OFF`.
- `build.py`: `DUILIB_ENABLE_CEF` is a stateful cache entry too; passing
  `--cmake-arg=-DDUILIB_ENABLE_CEF=OFF` once made later builds keep it off. It is
  now passed explicitly on every configure (ON for Windows MSVC, Linux and
  macOS; OFF for MinGW/MSYS2 and FreeBSD), and `--cmake-arg` still wins.
- Packaging (SDK integration): `NimDuilibPackageRuntimeDlls` used to list the CEF
  runtime DLLs from `bin/libcef_win[_109]/<arch>/` as flattened names, so
  downstream `nim_duilib_copy_runtime` tried to copy a non-existent `bin/<name>`
  and the build failed. The metadata now lists only DLLs directly under `bin/`,
  and `nim_duilibConfig.cmake` also filters by existence so older packages keep
  working. The CEF runtime is still deployed by copying that subdirectory, as
  documented.
- Build (macOS CEF examples): `cef.app`/`CefBrowser.app` did not ship the SDL3
  dylib in `Contents/Frameworks` and had no relative rpath pointing there, so the
  apps only ran through the builder's absolute SDL3 path. The post-build step in
  `duilib_copy_sdl_bundle.cmake` now copies `libSDL3*.dylib` (preserving the
  version links) and sets the main executable's rpath to
  `@executable_path/../Frameworks` (Helpers use `@executable_path/../../..`);
  a user-provided static SDL3 is detected and the copy is skipped.
- Build (macOS): CEF binary distributions usually ship only the Release framework,
  so a Debug build failed while copying
  `Debug/Chromium Embedded Framework.framework`. Configuration now checks both
  `Debug/` and `Release/`, falls back to Release with a status message when the
  Debug framework is missing, and fails early with a `CEF_ROOT` hint when neither
  exists.
- Build tools: `autotest.py` and `sdk_package_test.py` now default their output root to
  the system temp directory (`tempfile.gettempdir()`) instead of hard-coded
  `C:\develop\temp` and `/tmp`, so no code changes are needed on another machine. The
  examples in `README`, `README.en.md` and `sdk-package-test.md` were updated to match.
- Dependency preparation (Windows): the LLVM root used by Skia is no longer hard-coded
  to `C:/LLVM`; it is detected from `-DDUILIB_LLVM_ROOT`, the `LLVM_ROOT` environment
  variable, `C:/LLVM`, `Program Files/LLVM`, and the Visual Studio bundled LLVM, in
  that order.
- Build (macOS): linking example programs made ld64 report "ignoring duplicate
  libraries" (`-lpthread`, `-lSDL3` and the Skia libraries were listed more than once;
  the linker ignores duplicates anyway). The warning is suppressed by adding
  `-Wl,-no_warn_duplicate_libraries` when the linker supports it.
- Examples (controls): `TabCtrlForm.h`/`TabCtrlForm.cpp` lacked a trailing newline and
  `TimelineForm.cpp` had an incremented but unread `nIndex`, triggering
  `-Wnewline-eof` and `-Wunused-but-set-variable`; the newlines were added and the
  unused variable removed.
- Build (Windows/CEF): the main project uses C++20 while `libcef_dll_wrapper` is pinned
  to C++17 upstream, so the compile line contained both `/std:c++20` and `/std:c++17`
  and produced warning D9025. The wrapper targets (including the 109 variant) now set
  `CXX_STANDARD 17` locally, so the global standard is no longer overridden.
- Dependency preparation (UOS): like the legacy scripts, UnionTech UOS always uses
  `gcc/g++` (for both Skia and the main build) to avoid clang++ link errors there.
- Dependency preparation: the Skia patch archive now records a content fingerprint and
  an "already applied" check (`skia/.duilib-patch`); unchanged patches are not extracted
  again, so source mtimes are preserved and Skia is not rebuilt from scratch.
- Dependency preparation (CEF): a cached archive with a mismatching sha256 is removed
  and downloaded again; archives passed through `DUILIB_CEF_ARCHIVE` still fail without
  being deleted, so user-provided files are never destroyed.
- Dependency preparation: curl downloads now use `--speed-limit 100 --speed-time 120`,
  aborting a stalled transfer and retrying with the shared policy instead of hanging
  forever under unlimited retries.
- `build.py` (MSYS2): compilers must target mingw-w64 (clang first, then gcc) or a clear
  error is shown, preventing MSYS-targeted compilers from producing non-Windows binaries.
- Dependency preparation: the Skia source repository now matches the legacy scripts
  (`https://github.com/google/skia.git`); the previous `skia.googlesource.com` URL was
  unreachable on some networks. Skia/SDL clone URLs are read from the version manifest
  instead of being hard-coded in the script.
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
- CEF examples (macOS): the Helper sub-targets did not receive duilib's include/link
  directories, so compilation failed with `fatal error: 'duilib/duilib.h' file not
  found`; Helpers now use the same directory settings and link dependencies as the
  main target.
- Root CMake project (macOS): the CEF wrapper (`libcef_dll_wrapper`) was not built even
  though the CEF examples link against it; macOS now builds the wrapper as well, with
  output in `lib/<Config>`.
- CEF examples (macOS/Linux): the wrapper's link name `cef_dll_wrapper` differs from its
  target name `libcef_dll_wrapper`, so `target_link_libraries` created no build
  dependency and Helpers could link before the wrapper was built, failing with
  `ld: library 'cef_dll_wrapper' not found`. The wrapper target now also provides an
  ALIAS named `cef_dll_wrapper`, so linkers referencing that name get the absolute path
  and the build dependency (the link name is unchanged and packaging is unaffected).
- Root CMake project: the libpng/zlib targets are renamed from `png_static`/`zlibstatic`
  to `duilib-png`/`duilib-zlib` so they match the library file names. `DUILIB_LIBS`
  previously linked by file name, so no build dependency was created and parallel builds
  could link before the libraries existed (`library 'duilib-png' not found`); with the
  rename CMake uses absolute paths and tracks the dependencies automatically. libpng is
  built embedded with `SKIP_INSTALL_ALL` to avoid export errors from linking the
  non-exported `duilib-zlib` target.
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
- Packaging (Linux): the CEF wrapper's Unix artifact drops the `lib` prefix
  (upstream CMake sets `PREFIX ""`), producing `cef_dll_wrapper.a`, which was not
  in `pack.py`'s collection whitelist — `--with-cef` failed with
  "missing libcef_dll_wrapper". Name normalization now maps the prefixless name
  back to the whitelist name; the packaged file keeps its original name (both
  spellings are accepted by `nim_duilibConfig.cmake`).
- Dependency prep (Unix): the SDL3 completion stamp included the build
  configuration, but on Unix SDL3 is always built as a Release shared library
  independent of duilib's Debug/Release, so switching configurations needlessly
  rebuilt SDL3; the configuration component is now kept on Windows only.
- Dependency prep (CEF): `_duilib_cef_tracked_dir` ignored the platform and
  always returned `libcef_win[_109]`, so Linux/macOS compared versions against
  the Windows tracking header; it now returns `libcef_linux` / `libcef_macos` /
  `libcef_win[_109]` per platform.
- Packaging supports the macOS CEF example App Bundles (`bin/cef.app`,
  `bin/CefBrowser.app`): with `--with-examples`, `--with-cef` controls whether they
  ship, exactly like other platforms — omitted means skipped with a warning, specified
  means the whole directory (CEF framework, Helpers and symlinks under
  `Contents/Frameworks`) is collected with its original layout. On macOS `--with-cef`
  only controls examples; it collects no CEF development files or runtime (bundles
  carry the framework themselves). The previous "error out" behaviour is removed.
- CEF example App Bundles (macOS): the programs link dynamic SDL3 (`SDL_SHARED=ON`
  at build time), but `duilib_cef_macos.cmake` neither copied `libSDL3*.dylib`
  into `Contents/Frameworks` nor set an rpath for it. They only ran because the
  rpath still contained the builder's absolute SDL3 path, so moving the bundle or
  using another machine failed to load the dylib. The build now copies the dylib
  into the bundle, sets the main executable's rpath to
  `@executable_path/../Frameworks` and each Helper's rpath to
  `@executable_path/../../..` (a Helper executable lives in
  `Contents/Frameworks/<Helper>.app/Contents/MacOS`); packaging collects it with
  the bundle.
- Only the `cef` and `CefBrowser` examples link CEF: with `DUILIB_ENABLE_CEF` enabled
  in the root project, every example used to link `libcef` (`ldd` on Linux showed
  `libcef.so` for `basic` and friends). This is now controlled per example by
  `DUILIB_EXAMPLE_CEF`; `DUILIB_ENABLE_CEF` only decides whether the library compiles
  the CEF control sources.

- CEF 109 (Windows CMake): the root project always added `libcef_win` (latest) for
  the wrapper, and the 109 wrapper target was still named `libcef_dll_wrapper`
  instead of the `libcef_dll_wrapper_109` the link side expects (link failure or
  the latest sources compiled against 109 headers). The root project now picks the
  directory from `DUILIB_CEF_VERSION`, and the 109 wrapper target is renamed to
  `libcef_dll_wrapper_109`.
- CEF 109 (Windows CMake, Debug): with `USE_SANDBOX=ON` the wrapper defines
  `_HAS_ITERATOR_DEBUGGING=0`, conflicting with the Debug project's default
  `_ITERATOR_DEBUG_LEVEL=2` (LNK2038). The project does not link
  `cef_sandbox.lib`, so 109 configurations now force `USE_SANDBOX=OFF`.
- `build.py pack --sdl`: `--sdl` is a build switch only; the package does not
  include SDL3 (that needs `--with-sdl`). A hint is now printed so the package
  contents are not misread.
- `build.py --dry-run`: the `pack` and `test` subcommands used to ignore it and
  run for real; they now print the command only, like the other subcommands.
- `pack.py`: when one `lib/<arch>/<Config>` holds both MSVC `.lib` and MinGW
  `.a` files (switching toolchains does not delete old libraries), toolchain
  detection could pick the wrong one; packaging now fails with a request to
  clean that directory instead of silently producing a wrong package.
- MinGW example linking: a single Debug example can exceed 200 MB, so linking
  several in parallel makes `ld` consume a lot of memory (`out of memory` on
  8 GB machines). GNU ld now gets `-Wl,--no-keep-memory` to lower the peak (use
  `--jobs 2` if memory is still tight); lld (clang/CLANG64) does not accept the
  flag and is left unchanged.
- Argument validation: an unknown `--skia-ref` / `--sdl-ref` key now names the
  key and the fix instead of a generic "third_party_versions.json is invalid";
  a missing `--cef-archive` file is reported immediately instead of failing
  later during the CMake configure step.
- `pack.py --with-cef`: when the wrapper for the selected version is missing
  (typically a build made with `--cef-version 109` packed without that flag),
  the error names the CEF version actually present in the lib directory and
  the matching `--cef-version` to pass.
- `pack.py --with-examples` (Windows): `bin/` is shared by all architectures,
  so packing an x86 SDK used to collect leftover x64 examples. The PE COFF
  Machine field is now checked; mismatching exe/dll files are skipped and
  packaging is blocked (clean `bin/` or use `--allow-missing`).
- `build.py`: stateful options such as `DUILIB_MD`, `DUILIB_BUILD_EXAMPLES`,
  `DUILIB_ENABLE_SDL`, `DUILIB_CEF_VERSION` and `DUILIB_SKIA_REF` were only set
  when passed, so reusing an `out/<arch>/<Config>` directory kept stale values
  in the CMake cache (e.g. a plain build after `--no-examples` no longer built
  examples). Every invocation now passes the resolved values explicitly, so the
  result depends only on the current command line.
- Dependency preparation: the SDL3 build directories are keyed by
  architecture/config only, independent of the toolchain. After switching
  toolchains or runtimes (MSVC <-> MinGW, /MT <-> /MD) the old CMake cache kept
  the previous compiler, so e.g. `--sdl` under MinGW still invoked `cl.exe` and
  failed without the MSVC environment. The `.duilib-sdl` stamp is now compared
  first; on a CRT mismatch the SDL3 build directory is cleared and reconfigured.
- `build.py --dry-run`: `build` and `clean` still printed "build finished" /
  "removed" although they only printed commands; the completion messages are now
  suppressed in dry-run mode, matching `pack`/`test`.
- SDK test consumer project (`build/sdk_test`): `NIM_DUILIB_SDK_DIR` may now be omitted
  or given as a relative path. After unpacking, placing `sdk_test` inside the SDK root
  and running `cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./` resolved the
  relative path against the build directory, so `find_package(nim_duilib CONFIG)`
  could not find `lib/cmake/nim_duilib/nim_duilibConfig.cmake`. Relative paths are now
  resolved against `sdk_test` itself, its parent/grandparent, and the build directory
  (and its parents), taking the first hit, so
  `-DNIM_DUILIB_SDK_DIR=./nim_duilib-<version>-<platform>` also works when `sdk_test`
  sits next to the unpacked directory. Omitting the variable enables automatic layout
  detection; on failure the error lists every directory that was tried.
- Unit tests (macOS): `test_build.SkiaPatchTests.test_existing_repo_is_pulled` compared
  against the raw temp directory path, while the deps root in `build.py` is `resolve()`d
  to the real path; on macOS `/var` is a symlink to `/private/var`, so the strings
  differed and the test failed. The assertion now compares `Path.resolve()`d paths;
  Windows/Linux results are unchanged.

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
