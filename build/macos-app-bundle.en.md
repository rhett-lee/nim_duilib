# Building an Example as a macOS App Bundle

This document explains how to build a normal macOS example as an `.app` bundle
without changing any C++ source. Windows, Linux and FreeBSD builds are unaffected.

`examples/basic` is already opted in, so a normal build produces `bin/basic.app`:

```bash
python3 build/build.py --config Release
open bin/basic.app
```

For any other example, add one line to `examples/<name>/CMakeLists.txt`:

```cmake
set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)
```

The line must appear before `include(.../duilib_example.cmake)`. No example source
or resource-initialization code needs to change.

On macOS, `bin/basic` is now `bin/basic.app`; the executable lives at
`bin/basic.app/Contents/MacOS/basic`. Scripts that call `bin/basic` directly should
use that path or run `open bin/basic.app`.

## 1. Two ways to enable it

### Option A: one line in the example (recommended)

```cmake
# examples/chat/CMakeLists.txt
cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
project(chat CXX)

set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)   # macOS only

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
```

The setting travels with the example and wins over the global target list.

### Option B: a target list from the command line

The new CMake cache variable accepts CMake target names separated by semicolons or
commas; `all` or `*` selects every example:

```bash
# Only chat
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=chat

# Several targets
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=chat,panel

# Every example
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=all
```

With plain CMake:

```bash
cmake -S . -B out/x64/Release -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DDUILIB_MACOS_BUNDLE_EXAMPLES="chat;panel"
cmake --build out/x64/Release
```

To turn a bundle off, remove the per-example line or set it to `OFF`. If it was
enabled by the global list, pass an empty list:

```bash
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=
```

> CEF examples (`cef`, `CefBrowser`) are already bundles handled by
> `cmake/duilib_cef_macos.cmake`; this switch does not affect them.

## 2. Output layout

For `basic`:

```text
bin/basic.app/
  Contents/
    Info.plist
    MacOS/
      basic                       # executable
    Resources/
      duilib/                     # standard bundle resource directory
        themes/
        lang/
        fonts/
      basic.icns                  # only when an icon is configured
    Frameworks/
      libSDL3*.dylib              # absent when SDL3 is static
```

Resources use the standard macOS layout `Contents/Resources/duilib/`. On macOS,
`GlobalManager::GetResourceRootPath()` no longer depends on its argument; it
detects the resource directory from the runtime environment:

1. it prefers `<App>.app/Contents/Resources/duilib/` when that directory exists;
2. otherwise it falls back to "executable directory + `resources/`", which keeps
   the legacy layout and non-bundle executables working.

As a result, example code works unchanged whether it calls
`GetResourceRootPath(false)` or `GetResourceRootPath(true)`. SDK consumers using
`nim_duilib_copy_runtime(... RESOURCES)` follow the same standard layout.

## 3. What the build does

`cmake/duilib_macos_bundle.cmake` runs after linking and:

1. copies the whole `bin/resources` into `Contents/Resources/duilib` (standard
   layout, about 4 MB; unused theme directories can be trimmed after packaging);
2. copies `libSDL3*.dylib` and its version symlink chain into `Contents/Frameworks`
   when SDL3 is enabled (always on macOS);
3. sets the executable rpath to `@executable_path/../Frameworks` so SDL3 is not
   loaded from a build-machine absolute path;
4. removes a stale plain `bin/<target>`; switching back to a plain executable
   removes `bin/<target>.app`;
5. reads `duilib/duilib_version.h` and fills `CFBundleVersion` /
   `CFBundleShortVersionString`.

Bundles are only produced on macOS; other platforms are unchanged.

## 4. Custom bundle metadata

Set these in the example's `CMakeLists.txt` before
`include(duilib_example.cmake)`:

```cmake
# Default: com.nimduilib.<target>
set(DUILIB_MACOS_BUNDLE_IDENTIFIER "com.example.chat")

# Finder display name; default is the target name
set(DUILIB_MACOS_BUNDLE_NAME "Chat Demo")

# .icns icon: absolute, or relative to the example source directory
set(DUILIB_MACOS_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/chat.icns")
```

The icon must be `.icns`; a Windows `.ico` is not accepted. Include 16, 32, 128,
256, 512 and matching `@2x` sizes. A typical conversion:

```bash
mkdir -p Chat.iconset
sips -z 16 16     chat.png --out Chat.iconset/icon_16x16.png
sips -z 32 32     chat.png --out Chat.iconset/icon_16x16@2x.png
sips -z 128 128   chat.png --out Chat.iconset/icon_128x128.png
sips -z 256 256   chat.png --out Chat.iconset/icon_128x128@2x.png
sips -z 512 512   chat.png --out Chat.iconset/icon_512x512.png
sips -z 1024 1024 chat.png --out Chat.iconset/icon_512x512@2x.png
iconutil -c icns Chat.iconset -o chat.icns
```

The template is `cmake/macos/Info.plist.in`; it includes
`NSHighResolutionCapable`, `NSPrincipalClass` and `LSMinimumSystemVersion`.
To add entitlements, URL schemes and similar keys, point
`MACOSX_BUNDLE_INFO_PLIST` at a custom template.

## 5. Verification

```bash
plutil -lint bin/basic.app/Contents/Info.plist
plutil -p bin/basic.app/Contents/Info.plist | grep -E 'CFBundle(Identifier|Version|ShortVersionString)'

otool -L bin/basic.app/Contents/MacOS/basic | grep SDL3
# expect @rpath/libSDL3*.dylib

otool -l bin/basic.app/Contents/MacOS/basic | grep -A2 LC_RPATH
# expect path @executable_path/../Frameworks

otool -D bin/basic.app/Contents/Frameworks/libSDL3.0.dylib
# expect @rpath/libSDL3.0.dylib

test -f bin/basic.app/Contents/Resources/duilib/themes/default/global.xml && echo resources-ok
DYLD_PRINT_LIBRARIES=1 bin/basic.app/Contents/MacOS/basic
open bin/basic.app
```

If themes or fonts are missing at startup, check that
`themes/default/public`, `themes/default/basic`, `themes/default/global.xml`,
`themes/color_light`, `themes/color_dark`, `lang/` and `fonts/` all exist under
`bin/basic.app/Contents/Resources/duilib/`. The build copies the whole
`bin/resources` directory.

## 6. Signing and notarization

Ad-hoc signing is enough for local testing. Distribution requires a Developer ID
signature and notarization. Sign after resources and dylibs are final; re-sign
whenever their contents change.

```bash
# Local test
codesign --force --deep --sign - bin/basic.app
codesign --verify --deep --strict --verbose=2 bin/basic.app

# Distribution (use your own certificate and profile)
codesign --force --deep --options runtime \
         --sign "Developer ID Application: <Team Name>" bin/basic.app
ditto -c -k --keepParent bin/basic.app basic.zip
xcrun notarytool submit basic.zip --keychain-profile <profile> --wait
xcrun stapler staple bin/basic.app
spctl -a -vv bin/basic.app
```

`--deep` is convenient for smoke tests; for release builds sign from the inside out
(`Contents/Frameworks/libSDL3*.dylib` → main executable → `.app`) so no nested
binary is missed. See the macOS section of
[docs/Release.en.md](../docs/Release.en.md#signing-and-notarization) for CEF
Helpers and Frameworks.

## 7. Packaging and package tests

`build/pack.py` collects every macOS `.app` directory under `bin/`; no extra flag
is required:

```bash
python3 build/build.py --config Release
python3 build/pack.py --config Release --with-examples
```

The package contains `bin/basic.app/...`. C10 in
`build/sdk_package_test.py` already accepts
`bin/basic.app/Contents/MacOS/basic`:

```bash
python3 build/sdk_package_test.py \
    --package dist/nim_duilib-<version>-macos-x64-examples.tar.gz
```

Debug and Release bundles share the same name and are still distinguished by
`bin/.build-config/<target>`. `build.py` removes the other configuration's
artifacts when switching configurations.

## 8. Troubleshooting

| Symptom | Cause | Fix |
| :--- | :--- | :--- |
| Still `bin/basic`, no `.app` | Switch not applied | Put `set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)` before the include; it only applies on macOS |
| No `Resources/duilib/` inside the bundle | Copy skipped or source missing | Ensure `bin/resources` exists and rebuild; look for `Copying duilib resources` in the build log |
| Themes/fonts not found at startup | Resources not in the standard directory | Check `Contents/Resources/duilib/themes/default/global.xml`; `GetResourceRootPath()` detects this directory automatically |
| `Library not loaded: @rpath/libSDL3*.dylib` | SDL3 missing or rpath changed | Check `Contents/Frameworks` and `LC_RPATH`; rebuild and re-sign |
| “Developer cannot be verified” | Not signed/notarized | Ad-hoc sign for local tests; follow section 6 for distribution |
| Both `basic` and `basic.app` exist | Old artifact from a previous mode | Relink the target (the build removes the opposite artifact) or delete the old one manually |
| Packaging picks up an old `.app` | Bundle was disabled without relinking | `rm -rf bin/<target>.app` and rebuild |

## 9. Related files

| File | Purpose |
| :--- | :--- |
| `examples/basic/CMakeLists.txt` | Example already opted in with one line |
| `cmake/duilib_example.cmake` | Resolves `DUILIB_EXAMPLE_MACOS_BUNDLE` and the target list |
| `cmake/duilib_bin_macos.cmake` | Chooses between `MACOSX_BUNDLE` and a plain executable |
| `cmake/duilib_macos_bundle.cmake` | Bundle properties, resources, icon, SDL3, rpath |
| `cmake/macos/Info.plist.in` | Info.plist template |
| `cmake/duilib_copy_sdl_bundle.cmake` | Copies SDL3 dylibs into `Contents/Frameworks` |
