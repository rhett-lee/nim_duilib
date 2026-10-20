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
`msvc_build.bat` (CMake) — whose artifacts land in the same `lib/` and `bin/`
directories. No single build system can cover both. `pack.py` collects files
by whitelist, so artifacts from either path are picked up.

## Quick start

You must complete a Release build before packaging; the script does not
trigger compilation:

```
# Windows (MSBuild path)
build\duilib.sln      # Configuration: Release

# Windows (CMake path)
build\msvc_build.bat

# Linux / macOS / FreeBSD
./build/linux_build.sh
./build/macos_build.sh
./build/freebsd_build.sh
```

Then package:

```
# SDK only (recommended)
python build/pack.py

# SDK plus example programs
python build/pack.py --with-examples
```

Output goes to `dist/`, with package names of the form
`nim_duilib-<version>-<system>-<arch>.zip`:

```
dist/nim_duilib-1.0.2026.0416-win-x64.zip
```

## Package layout

```
nim_duilib-<version>-<system>-<arch>/
  VERSION               Version number, identical to the root VERSION file
  MANIFEST.txt          Package contents and caveats
  SHA256SUMS            SHA256 of every file, verifiable with sha256sum -c
  LICENSE               Main license
  include/duilib/       Public headers
  lib/                  Static libraries
  resources/            Theme, language and font resources
  licenses/             Third-party licenses
  bin/                  Present only with --with-examples
```

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
| `--with-examples` | Include example programs and all per-example resources |
| `--with-cef-libs` | Include CEF wrapper static libraries (80–100 MB each, off by default) |
| `--allow-debug-libs` | Allow packaging libraries for which only a Debug build exists (excluded by default, see below) |
| `--format zip\|tgz\|tarbz2` | Archive format; defaults to `zip` on Windows, `tgz` elsewhere |
| `--lib-dir <path>` | Library directory; auto-detected by default |
| `--out-dir <path>` | Output directory; defaults to `dist/` |
| `--list` | Print the file list that would be collected without packaging |

## Debug libraries are excluded by default

The script distinguishes Release from Debug libraries and **packages Release
only** by default. Debug libraries carry the debug CRT and the `/MTd` runtime;
mixing them into a Release SDK breaks downstream linking.

Debug detection has two sources, and both are required:

- A `_d` suffix on the file name (the CMake path, e.g. `duilib_d.lib`)
- The containing directory being named `Debug` (the MSBuild path, e.g.
  `lib/Debug/duilib.lib`)

Looking only at file names misses the second case and packages a 200 MB-plus
Debug library as if it were Release. Use `--allow-debug-libs` when you really
need it.

If no candidate directory contains the main `duilib` library, the script
warns explicitly — such a package has no linkable duilib and is only useful
as a reference for headers and resources.

## CEF limitations

The CEF runtime (`libcef.dll`, `*.pak`, `icudtl.dat`) is **not part of this
repository**; users must obtain it from the official libcef release.
Consequently:

- The CEF-based examples (`cef`, `CefBrowser`) cannot run straight from the
  package
- `MANIFEST.txt` lists those examples and notes the limitation
- CEF wrapper static libraries are excluded by default; pass
  `--with-cef-libs` to include them

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
However, **only the Windows artifacts have been verified**. Differences in
library file name suffixes and static library formats (`.a` vs `.lib`) on the
other platforms may require adjusting `LIB_BASENAMES` and the format
detection once real build results are available.

## Related documentation

* [Build process and scripts](../build/README.en.md)
* [Quick Start](Getting-Started.en.md)
* [Example Programs](Examples.en.md)
* [CEF Control](CEF.en.md)
