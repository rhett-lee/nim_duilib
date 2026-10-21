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
  bin/                  Present only with --with-examples
```

### SDK usage contract

To let downstream builds **compile, link and run**, `MANIFEST.txt` records:

1. **Include path**: `include/` (`#include "duilib/duilib.h"`). When CEF is
   included, also add `include/duilib/third_party/libcef/<libcef_win|libcef_win_109>`.
2. **Library path**: `lib/` (flat).
3. **Link list**:
   - Core: `duilib`, `duilib-cximage`, `duilib-webp`, `duilib-png`, `duilib-zlib`
   - Skia: `skia`, `svg`, `skshaper`, `skottie`, `sksg`, `jsonreader`
     (`bentleyottmann`, `expat`, `skcms`, `skresources` are shipped; add them
     if unresolved symbols appear)
   - WebView2: `WebView2LoaderStatic` (VS) or `WebView2Loader.dll` (CMake,
     requires the DLL at runtime)
   - JPEG (Windows): `turbojpeg-static`
   - SDL (optional): `SDL3-static` (Windows) / `SDL3` (Unix)
   - CEF (optional): `libcef` + `libcef_dll_wrapper[_109]`
4. **Platform system libraries**: with SDL on Windows add
   `Version/Winmm/Imm32/Setupapi`; on Unix `X11`/`freetype`/`fontconfig` come
   from the system and are not packaged.
5. **Runtime**: `resources/` must sit next to the executable; the CEF runtime
   is external; on Unix `libSDL3.so*` must be findable (`LD_LIBRARY_PATH`
   etc.).
6. **Configuration consistency**: a Debug package serves Debug downstream
   builds (`/MTd`) only, and a Release package serves Release (`/MT`) only.
   Never mix the two.

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

### third_party headers are forwarded too

Five public headers include headers from `third_party` directly, so those
must be forwarded with the package or downstream builds fail:

| Public header | Depends on |
| :--- | :--- |
| `Core/ZipStreamIO.h` | `zlib` |
| `Image/APngDecoder.h` | `zlib`, `libpng` |
| `Image/FrameSequence_gif.h` | `giflib` |
| `WebView2/WebView2Control.h` | `Microsoft.Web.WebView2` |
| `WebView2/WebView2EnvironmentOptions.h` | `Microsoft.Web.WebView2` |

`zlib` and `libpng` have internal transitive dependencies (for example
`pngpriv.h` includes `pnglibconf.h`), so those two are forwarded as whole
directory trees rather than filtered file by file.

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
| `--with-examples` | Include example programs and all per-example resources |
| `--format zip\|tgz\|tarbz2` | Archive format; defaults to `zip` on Windows, `tgz` elsewhere |
| `--out-dir <path>` | Output directory; defaults to `dist/` |
| `--list` | Print the file list that would be collected without packaging |

## Debug packages

Configuration is determined solely by the directory (`lib/<arch>/Debug` or
`lib/Debug`); the `_d` suffix is no longer inspected.

- With `--config Debug` the package name gains `-debug` and `MANIFEST.txt`
  records `Configuration : Debug`.
- Debug libraries carry the debug CRT and `/MTd` and may be used only by Debug
  downstream builds — **never mix them into a Release SDK**.
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
