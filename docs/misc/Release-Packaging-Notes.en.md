# Release Packaging Work Record (B-3)

This document is the **work record** for B-3 "Release and packaging process": it captures the delivered results, the technical decisions and their rationale, the pitfalls hit along the way, and the open items needed for follow-up.

The user-facing operation guide lives in [Release and Packaging](../Release.en.md). This document does not repeat that content; it records **why things were done this way** and **what is still not finished**.

> Status: the feature is implemented and tested on Windows, and the changes are **committed** (B-3 in `e4064ba8`; the CM-5 P4 follow-up in `25d34686`). The "Current working tree state" section below is kept as a record of what was committed.

## 1. Current working tree state

The changes are **committed**: B-3 in `e4064ba8`, CM-5 P1~P5 in `7f8f1500` ~ `6e2a2f0f`. The list below is kept as a historical record of the committed content.

### Files that should be committed

| File | Type | Content |
| :--- | :--- | :--- |
| `duilib/duilib_version.h` | new | Single source of truth for the version number, four-segment (replaced the earlier `VERSION.txt` approach on 2026-10-23) |
| `build/pack.py` | new | Packaging script, pure standard library, cross-platform, does not participate in compilation |
| `docs/Release.md` / `docs/Release.en.md` | new | User-facing packaging and release guide |
| `docs/misc/Release-Packaging-Notes.md` / `.en.md` | new | This document |
| `docs/Summary.md` / `Summary.en.md` | modified | Index registration |
| `build/README.md` / `README.en.md` | modified | New "Release packaging" section |
| `docs/misc/Development-Plan.md` | modified | B-3 entry marked done, pointing to this document |
| `.gitignore` | modified | Added `/dist` |

### Process scripts that should not be committed

This batch under `build/` is scratch tooling and, per project convention, **must stay out of the commit**:

```
add_dist_ignore.py          apply_a1_a2.py              apply_a4.py
build_a1a2_check.bat        build_a4_check.bat          build_chart_debug.bat
build_tabctrl_debug.bat     dedup_summary.py            fix_b3_spacing.py
fix_cef_comment.py          fix_spacing.py              fix_spacing2.py
mark_a1_a2_done.py          mark_a3_deferred.py         mark_a4_done.py
mark_b3_done.py             mark_b_deferred.py          register_release_docs.py
smoke_controls.bat          update_dev_plan.py          verify_cef_guard.py
verify_cef_macro.py         verify_overflow.py          verify_tabctrl_fixed.py
write_release_docs.py
```

`build/pack.py` is the **only** new file under `build/` to be committed (all other `.py` / `.bat` files there are process scripts).

## 2. Deliverables

### Artifact contract

The rules enforced by `pack.py`, each with an explicit rationale:

| Item | Rule | Rationale |
| :--- | :--- | :--- |
| Header path | Must be `include/duilib/xxx.h`, **never flattened** | Downstream code writes `#include "duilib/duilib.h"` (`examples/basic/MainForm.h:5`); flattening breaks every downstream build |
| `RenderSkia/` | Not exposed | `duilib.h` does not reference it; it is a rendering-backend implementation detail |
| `Core/ZipStreamIO.h` | Not exposed | Only `ZipManager.cpp` includes it; neither `duilib.h` nor `ZipManager.h` references it. Once excluded, zlib headers are no longer needed either |
| `Core/ControlDropTarget*`, `Core/WindowDropTarget_*`, `Core/SDL_MacOS.h`, plus platform implementation headers such as `MessageLoop_*` / `ProcessSingleton_*` / `RichEdit*_Windows/_SDL` | Not exposed | Only `.cpp` / `.mm` files include them; no public header references them |
| `Core/DragWindowFilter_SDL.h` | Always on non-Windows; on Windows only with `--with-sdl` | `ControlDragable.h` (on the `duilib.h` chain) includes it under `DUILIB_BUILD_FOR_SDL`; that macro is always defined on non-Windows |
| `third_party` forwarding | Microsoft.Web.WebView2 only (and Windows only) | Public headers depend on this one place only; image decoder implementation headers and `Core/ZipStreamIO.h` are internal, so libpng / giflib / zlib headers are no longer needed |
| Library directory | Only one layout (Windows `lib/<arch>/<Config>`, Unix `lib/<Config>`) | CM-5 D-2 option B; configuration comes from the directory, no `_d` detection or directory merging |
| Windows toolchain | Detected from the library directory: `duilib.lib` → MSVC, `libduilib*.a` → MinGW; MinGW collects `.a` / `.dll.a` and probes `mingw64-gcc.*` / `mingw64-llvm.*` for Skia | `build/gcc-mingw-w64_build.bat` and `llvm-mingw-w64_build.bat` produce exactly those names and Skia directories; the two artifacts must not be mixed |
| DLL-mode runtime | On Windows, `bin/duilib.dll` / `duilib_d.dll` / `duilib32.dll` / `duilib32_d.dll` are shipped when present; `lib/` then holds the import library; `bin/turbojpeg.dll` is shipped when JPEG is available | `build.py --dll` or `EnableDuilibDll` (`duilib_dll.flag`) writes the DLL to `bin/` and the import library to `lib/<arch>/<Config>/duilib.lib`; the DLL links `turbojpeg.lib`, so the static library is no longer shipped; downstream fails at runtime without the DLL |
| Debug package | With `--config Debug` the name gains `-debug` and the manifest records the config | Debug libraries carry the debug CRT (`/MTd`, or `/MDd` in DLL mode) and only serve Debug downstream builds; never mix the two |
| VS integration layer | Windows + MSVC packages generate `msvc/nim_duilib.props`, `.targets`, `nim_duilib.sdk.props` and `msvc/manifest/` (both x64 and x86) | Importing `.targets` once gives include paths, defines, link list, manifest and post-build DLL copying; both manifests ship so x86 programs can be built on x64 machines; metadata comes from the actual collected files |
| CMake integration layer | All platforms generate `lib/cmake/nim_duilib/*.cmake` with `nim_duilib::duilib` and `nim_duilib_copy_runtime()` | `find_package(nim_duilib CONFIG REQUIRED)`; arch/config/toolchain/runtime mismatches fail with `FATAL_ERROR` by default |
| MSVC runtime metadata | `--msvc-runtime auto` infers `MD` for DLL mode, otherwise `MT`, and writes it into package metadata | Mixing `/MT` and `/MD` triggers LNK2038, so the runtime is aligned for consumers by default, with an opt-out switch |
| Skia libraries | Whole directory shipped, plus a link list in the manifest | The `duilib` static library contains Skia symbols; linking fails without them |
| WebView2 / JPEG | Windows ships loader libraries and `bin/WebView2Loader.dll` by default; JPEG ships `turbojpeg-static` in static mode and `bin/turbojpeg.dll` in DLL mode | VS enables WebView2 and libjpeg-turbo by default; `duilib.lib` contains those symbols; CMake uses the dynamic loader, which needs the DLL at runtime; in DLL mode the duilib DLL links `turbojpeg.lib` |
| SDL3 | Always shipped on Unix; on Windows only with `--with-sdl`; collects the static or import library actually built, plus `bin/SDL3.dll` for dynamic builds | SDL is off by default on Windows; on Unix it is enabled and built as a shared library; MinGW CMake builds link the shared SDL3 (`libSDL3.dll.a`) |
| CEF development set | Off by default, needs `--with-cef` (optionally `--cef-version 109`) | Each wrapper is 80–100 MB; the runtime still has to be supplied separately |
| `bin/resources.zip` | Always excluded | An empty shell file of only 748 bytes |
| `docs/` | Only top-level user docs are shipped; subdirectories such as `misc/` and `specs/` are never packaged | User docs ship with the SDK; `Release.*` (release process), `Getting-Started.*` / `Examples.*` (library-internal docs) and the credits list target maintainers, not consumers; `Summary.*` ships after its links to outside directories were removed |
| Version number | Single source of truth is the four numeric macros in `duilib/duilib_version.h` | Package name, in-package `VERSION` and `MANIFEST.txt` are all derived from it; `pack.py` ignores comments and unrelated macros, and fails on missing/duplicated/out-of-range values |

### Resource scope difference with `--with-examples`

This is the easiest thing to get wrong when packaging. `bin/resources/themes/default/` holds both public resources and each example's private theme directories:

| Mode | `bin/resources/` scope |
| :--- | :--- |
| Default (SDK) | Only `themes/default/public/` and `themes/default/global.xml` |
| `--with-examples` | The entire `bin/resources/`, no exclusions |

`global.xml` **must be kept** in SDK mode. It is a startup requirement of the framework — `GlobalManager.cpp:440` returns `false` outright when it cannot be found in local-file mode, and initialization fails.

The 22 example-private theme directories excluded in SDK mode are:
`MultiLang`, `basic`, `cef`, `cef_browser`, `chat`, `child_window`,
`color_theme`, `controls`, `dpi_aware`, `layout`, `list_box`, `list_ctrl`,
`move_control`, `panel`, `render`, `rich_edit`, `threads`, `tree_view`,
`virtual_list_box`, `webview2`, `webview2_browser`, `xml_preview`.

### Command line interface

```
python build/pack.py                      # Release SDK package (no examples)
python build/pack.py --config Debug       # Debug SDK package (name gains -debug)
python build/pack.py --with-examples      # SDK plus example programs
python build/pack.py --with-cef           # additionally include the CEF development set
python build/pack.py --with-sdl           # additionally include SDL3 on Windows
python build/pack.py --list               # print the file list that would be collected, do not package
python build/pack.py --format tgz         # force tgz (Windows defaults to zip)
python build/pack.py --lib-dir lib/x86/Debug  # explicit library directory
python build/pack.py --out-dir dist       # specify the output directory
```

Running `pack.py` directly writes to `dist/`; `release.py package` always writes
to `dist/<version>/`. Package names are
`nim_duilib-<version>-<system>-<arch>[-sdl][-cef|-cef109][-examples][-debug]`,
containing `VERSION`, `MANIFEST.txt`, `SHA256SUMS`, `LICENSE`,
`include/duilib/`, `lib/` (including Skia and third-party link libraries),
`bin/` (DLLs and `bin/resources/`), `licenses/` and `docs/` (top-level user
documentation).

## 3. Key technical decisions and their rationale

### 1. Why not CPack / `install()`

The original audit conclusion (zero CPack, zero `install()` rules, zero NSIS/WiX/DEB/DMG) **still holds**, but the **overall approach was replaced**. The reason is architectural mismatch, not omission:

- **Adding `install()` rules — nowhere to add them.** This repository has **no root `CMakeLists.txt`**. `duilib/`, `examples/*/` and `duilib/third_party/*/` are each independent `project()`s, configured and built one by one by scripts such as `msvc_build.bat` and `linux_build.sh`. `install()` can only hang off a single project, and no single CMake project can cover all artifacts.
- **CPack — needs a top-level project to attach to**, same reason. More importantly, Windows has two build paths (`build/*.sln` with 36 `.vcxproj` files via MSBuild, and the CMake path via `msvc_build.bat`), whose artifacts land in the same `lib/` and `bin/`. A CPack-produced package would be empty for MSBuild users.
- Hence the switch to an **independent packaging script**: decoupled from the build system, collecting artifacts by filename whitelist, so it picks up both build paths and requires no new top-level project.

### 2. Header hierarchy must not be flattened

In-package headers keep the `include/duilib/xxx.h` hierarchy, matching their repo-root-relative path. Downstream code writes `#include "duilib/duilib.h"`, so there must be a `duilib/` level under `include/`.

The first version of the packaging script flattened headers to `include/duilib.h`, and every downstream build failed immediately.

### 3. How the `third_party` forwarding scope was determined

This is the most thoroughly derived part of the work. The chain of reasoning:

**Step one — who references it.** There are 313 public headers under `duilib/` (excluding `third_party/` and `RenderSkia/`), as measured on 2026-10-20. Scanning every `#include`, only 5 referenced `duilib/third_party/...` at that time, via 9 include directives in total:

> Updated 2026-10-23: `Image/APngDecoder.h`, `Image/FrameSequence_gif.h` and `Core/ZipStreamIO.h` in the table below are no longer packaged. The only `third_party` headers shipped now are WebView2 (Windows only); zlib, libpng and giflib are no longer forwarded. See section 3.1.

| Public header | Third-party headers referenced |
| :--- | :--- |
| `Core/ZipStreamIO.h:5` | `zlib/contrib/minizip/ioapi.h` |
| `Image/APngDecoder.h:5-9` | `zlib/zlib.h` + `libpng/{pngpriv,pngstruct,pnginfo,png}.h` |
| `Image/FrameSequence_gif.h:20` | `giflib/gif_lib.h` |
| `WebView2/WebView2Control.h:10` | `Microsoft.Web.WebView2/build/native/include/WebView2.h` |
| `WebView2/WebView2EnvironmentOptions.h:9` | same as above |

The key point is that these includes spell out the **full `duilib/third_party/...` path** (not a bare name like `zlib.h`), so the package must place them at `include/duilib/third_party/<original path>`. This is the same constraint as "headers must not be flattened", seen from the other side.

**Step two — the required file set.** Starting from those 5 direct references and recursively resolving same-directory / relative includes, the minimal closure is 13 files:

```
zlib      4 required: zlib.h → zconf.h; contrib/minizip/ioapi.h → ../../zlib.h, ints.h
libpng    7 required: png.h → pnglibconf.h, pngconf.h
                  pngpriv.h → pnglibconf.h, png.h, pngstruct.h, pnginfo.h, pngdebug.h
                  pngstruct.h, pnginfo.h
giflib    1 required: gif_lib.h (depends only on <stdbool.h> / <stddef.h>)
WebView2  1 required: WebView2.h
```

**Step three — why zlib / libpng had to be forwarded as whole trees (historical decision of 2026-10-20; no longer forwarded since 2026-10-23).** The headers of these two libraries are chained by **same-directory relative includes**, and the chains are guarded by conditions:

- `pngpriv.h:85` `#include "pngprefix.h"` — only when `PNG_PREFIX`
- `pngpriv.h:89` `#include "pngusr.h"` — only when `PNG_USER_CONFIG`
- `png.h:328` `#include "pnglibconf.h"` — only when `!PNGLCONF_H`
- `png.h:337` `#include "pngconf.h"` — only when `!PNG_VERSION_INFO_ONLY`
- `ioapi.h:74` `#include "mz64conf.h"` — only when `HAVE_MINIZIP64_CONF_H` (this file does not exist in the repository at all)

Forwarding by file whitelist would require enumerating every macro combination; missing one means a downstream build breaks as soon as it changes a compile switch. Forwarding whole directory trees covers all conditional branches by construction — **that is the real reason a whitelist is "not practical", not the file count**.

**Step four — why giflib and WebView2 are special.** `gif_lib.h` includes only `<stdbool.h>` / `<stddef.h>` and has no third-party relative dependencies. `WebView2.h`'s five extra dependencies (`rpc.h`, `rpcndr.h`, `objidl.h`, `oaidl.h`, `EventToken.h`) all come from the **Windows SDK** (`shared/`, `um/`, `winrt/`), not from third_party, so nothing extra needs forwarding. The WebView2 header lives at `third_party/Microsoft.Web.WebView2/build/native/include/`, a deep path, which is why it needs a multi-component path tuple.

**Cost accounting** (measured 2026-10-20):

| Directory | Headers forwarded as a tree | Required by the closure |
| :--- | :--- | :--- |
| zlib | 31 | 4 |
| libpng | 18 | 7 |
| giflib | 3 | 1 |
| Microsoft.Web.WebView2 | 1 | 1 |

38 extra headers, all a few KB of text each. The volume is negligible; in exchange, nothing is missed because of a macro combination or an upstream patch-level upgrade. (The table reflects the whole-tree forwarding policy as of 2026-10-20; since 2026-10-23 libpng / giflib / zlib are no longer forwarded — only the single WebView2 header is shipped. See section 3.1.)

### 3.1 Image decoder and internal implementation headers excluded from the package (2026-10-23)

**Conclusion.** Format-specific decoder implementation headers are used only by `.cpp` files inside the library. The `duilib.h` include chain never references them, so they are no longer shipped in the SDK.

Image entry points in `duilib.h` and their transitive dependencies:

```
Image.h               -> ImageInfo.h / ImageAttribute.h / ImageLoadParam.h
                      -> StateImageMap.h -> StateImage.h
ImageInfo.h           -> ImageDecoder.h
ImageDecoderFactory.h -> ImageDecoder.h / ImageUtil.h
```

The packaged `Image/` directory therefore keeps only 10 base headers: `Image.h`,
`ImageAttribute.h`, `ImageInfo.h`, `ImageLoadParam.h`, `ImageDecoder.h`,
`ImageDecoderFactory.h`, `ImageUtil.h`, `StateImage.h`, `StateImageMap.h`,
`ImagePlayer.h`.

**Exclusion list (`IMAGE_DECODER_HEADER_FILES` in `pack.py`, 23 files):**

```
Image/APngDecoder.h              Image/FrameSequence_gif.h
Image/Image_Animation.h          Image/Image_Bitmap.h
Image/Image_GIF.h                Image/Image_ICO.h
Image/Image_JPEG.h               Image/Image_LOTTIE.h
Image/Image_PAG.h                Image/Image_PNG.h
Image/Image_Svg.h                Image/Image_WEBP.h
Image/ImageDecoderUtil.h         Image/ImageDecoder_Common.h
Image/ImageDecoder_GIF.h         Image/ImageDecoder_ICO.h
Image/ImageDecoder_Icon.h        Image/ImageDecoder_JPEG.h
Image/ImageDecoder_LOTTIE.h      Image/ImageDecoder_PAG.h
Image/ImageDecoder_PNG.h         Image/ImageDecoder_SVG.h
Image/ImageDecoder_WEBP.h
```

**Resulting `third_party` scope:**

- `THIRD_PARTY_INCLUDE_TREE_ROOTS` removed: `libpng` was referenced only by `Image/APngDecoder.h`, and `zlib` only by `Core/ZipStreamIO.h`.
- `THIRD_PARTY_INCLUDE_DIRS` removed: `giflib` was referenced only by `Image/FrameSequence_gif.h`.
- New `INTERNAL_HEADER_FILES`: besides `Core/ZipStreamIO.h`, it includes 6 platform implementation headers — `Core/ControlDropTargetImpl_SDL.h`, `Core/ControlDropTargetImpl_Windows.h`, `Core/ControlDropTargetUtils.h`, `Core/SDL_MacOS.h`, `Core/WindowDropTarget_SDL.h`, `Core/WindowDropTarget_Windows.h`. They are included only by `.cpp` / `.mm` files.
- 2026-10-23 follow-up: 17 more implementation-only headers were added to `INTERNAL_HEADER_FILES` — `Core/SDL_Linux.h`, `Core/MessageLoop_SDL.h`, `Core/MessageLoop_Windows.h`, `Utils/ProcessSingleton_{Linux,MacOS,Windows}.h`, `Utils/BitmapHelper_{SDL,Windows}.h`, `Utils/ApiWrapper_Windows.h`, `Utils/Macros_Windows.h`, `Utils/DiskUtils_Windows.h`, `Utils/DllManager_Windows.h`, `Utils/InlineHook_Windows.h`, `Control/RichEditHost_Windows.h`, `Control/RichEditDragSource_Windows.h`, `Control/RichEditDropTarget_Windows.h`, `Control/RichEditDropTarget_SDL.h`.
- New `SDL_HEADER_FILES = ('Core/DragWindowFilter_SDL.h',)`: `Core/ControlDragable.h` includes it under `DUILIB_BUILD_FOR_SDL`. That macro is always defined on non-Windows, so the header must stay there; on Windows it is kept only with `--with-sdl`.
- `collect_jpeg_turbo` no longer forwards `turbojpeg.h` and friends; static packages still ship `turbojpeg-static.lib`, while DLL packages ship `bin/turbojpeg.dll` instead (the DLL links the `turbojpeg.lib` import library).
- Only `Microsoft.Web.WebView2` (Windows only) is forwarded.

**Verification (2026-10-23, Windows x64 Debug `--list`, EXIT=0):**

- 245 headers in total (without `--with-sdl`): 244 under `duilib/`, 1 from WebView2.
- `Image/` contains only the 10 base headers; no decoder implementation header leaks. `Core/ZipStreamIO.h`, the 6 platform implementation headers and the 17 new internal headers are not in the package.
- Scanning every `#include "duilib/..."` across the 245 packaged headers: apart from the conditional
  `#ifdef DUILIB_BUILD_FOR_SDL` include in `ControlDragable.h`, **0 unresolved**. That branch is not compiled
  when SDL is off on Windows; with `--with-sdl`, `DragWindowFilter_SDL.h` is added automatically (246 headers).
- Unix packages: `DragWindowFilter_SDL.h` is kept; excluding the WebView2 headers leaves 238 headers with a complete closure.
- Macro combos: Windows + `--with-cef` gives 270 headers; Unix + CEF gives 263, both with a complete closure.

### 3.2 Downstream integration layer (2026-10-23)

Previously the package only contained headers and libraries, leaving consumers
to assemble include paths, link lists, defines, manifests and runtime DLL
copying themselves. `pack.py` now generates two integration mechanisms from
the **actually collected files**:

- **VS (Windows + MSVC)**: `msvc/nim_duilib.targets` imports
  `nim_duilib.props` and `nim_duilib.sdk.props` automatically and handles
  include paths, `DUILIB_*` defines, `stdcpp20`, libraries/system libraries,
  MSVC runtime alignment, the manifest, and post-build `bin/*.dll` copying;
  `NimDuilibCopyResources=1` also copies `bin/resources/` to `resources/` in
  the output directory (next to the executable).
- **CMake (all platforms)**: `lib/cmake/nim_duilib/nim_duilibConfig.cmake`
  provides `nim_duilib::duilib`; it validates platform/toolchain/arch/config/
  runtime and provides `nim_duilib_copy_runtime(<target> [RESOURCES]
  [FRAMEWORKS] [DESTINATION <dir>])`; `FRAMEWORKS` (macOS) copies
  `lib/libSDL3*.dylib` into the App Bundle's `Contents/Frameworks` and sets the
  executable's rpath to `@executable_path/../Frameworks`
  (`BUILD_WITH_INSTALL_RPATH`, so the bundle's copies win); for a `MACOSX_BUNDLE` target
  without `DESTINATION`, `RESOURCES` copies into
  `Contents/Resources/duilib/resources/` (isomorphic to a plain program's
  `resources/`, i.e. `GetResourceRootPath(true)`).
- Metadata fields: version, system, arch, config, toolchain, DLL mode, SDL
  kind, WebView2/JPEG/CEF, MSVC runtime, runtime DLLs. MinGW packages do not
  generate `msvc/`; WebView2/JPEG/CEF are only declared available for MSVC.
- `--msvc-runtime auto|mt|md` controls the runtime written into metadata;
  `auto` means `MD` for DLL mode and `MT` otherwise.

**Verification (2026-10-23):**

- `py_compile` passes; Windows x64 Debug `--list` reports 538 files / 315.0 MB,
  including 9 integration files (2 manifests + 7 generated), with the original
  497 files unchanged.
- Windows packages include both `duilib.x64.manifest` and
  `duilib.x86.manifest`; the project's Platform selects the right one, so x86
  programs can be built on an x64 machine.
- All three props/targets files parse as XML; props carry a UTF-8 BOM and the
  virtual files' hashes in `SHA256SUMS` match the bytes in the archive.
- A temporary CMake consumer configures successfully with
  `find_package(nim_duilib CONFIG REQUIRED)` and gets the expected include/
  define/link properties; an arch mismatch (Win32 consumer, x64 package) and
  an MSVC runtime mismatch (MD consumer, MT package) both fail with the
  expected `FATAL_ERROR`.
- A mocked MinGW library directory produces no `msvc/` and only the 4 CMake
  integration files; `--with-sdl` / `--with-cef` `--list` variants succeed.

### 3.3 docs packaging (2026-10-23)

Top-level user documentation under `docs/` is shipped inside the package as
`docs/`, using first-level files only; subdirectories such as `misc/` and
`specs/` are never packaged. The following are the library's own maintenance
documents and are excluded:

- `Release.md` / `Release.en.md`: release and packaging process (versioning,
  signing, notarization, build scripts);
- `Getting-Started.md` / `Getting-Started.en.md` and `Examples.md` /
  `Examples.en.md`: library-internal quick-start and example descriptions;
- `感谢名单.txt`: project credits.

`Summary.md` / `Summary.en.md` is the in-package documentation index. Links to
other directories (`../README.md`, `../build/build-reference.md`, `build/README.md`, `misc/`, etc.) and
entries for documents not shipped in the package were removed.

The docs contain many links into the source tree, so they are rewritten at pack
time based on the actual collected files (`docs/` entries are virtual files and
are not written to disk):

| Original link | Handling inside the package |
| :--- | :--- |
| `../duilib/xxx.h` | Rewritten to `../include/duilib/xxx.h` when that header ships |
| `../bin/resources/...` | Unchanged (resources also live under `bin/` in the package) |
| `../../bin/resources/...` | Rewritten to `../bin/resources/...` |
| `../examples/`, `../build/`, `*.cpp`, internal headers not shipped | Link removed, display text kept |

No link is left broken inside the package, while the repository docs stay
unchanged and still navigate the source tree. The conversion is done by
`collect_docs()` in `pack.py`; docs are written into zip/tar as virtual files.
The "文档" (docs) section of `MANIFEST.txt` lists every packaged doc, and
`SHA256SUMS` covers all of them.

The exclusion list lives in `DOC_EXCLUDE_FILES` in `pack.py`; new top-level
documents ship by default. Windows x64 Debug verification: 32 docs (16
Chinese/English pairs); all relative links in the packaged docs resolve inside
the package (0 broken links), and SHA256SUMS matches. Adding `Deployment.md` /
`Deployment.en.md` on 2026-10-29 brings it to 34 docs (17 pairs); the link
rewriting rules are unchanged.

### 4. Debug library detection: suffix plus directory name

Debug artifacts come from **two** places, and checking only one misses the other:

- Filename carries the `_d` suffix — the CMake path's `duilib_d.lib`
- The containing directory is named `Debug` — the MSBuild path's `lib/Debug/duilib.lib` (**no suffix**, distinguished by directory name only; see `msvc/PropertySheets/DuilibSettings.props`)

The first version recognized only `_d`, and consequently packaged a 232 MB Debug library as if it were Release, inflating the compressed archive from 4.8 MB to 45.8 MB. Detection now uses both conditions.

### 5. Version number: single source of truth and the known gap

The single source of truth for the version is the four numeric macros in `duilib/duilib_version.h`, e.g. `1.0.2026.1023`. `pack.py` parses that header line by line (ignoring comments, `#undef` and unrelated macros, and failing on missing/duplicated/out-of-range values) and derives the package name, the in-package `VERSION` and `MANIFEST.txt`. `msvc/dllRes/version.h` is now a Windows resource-compile adapter and no longer stores version numbers; `VERSION.txt` has been removed.

git tags are recommended in the form `v<version>`. The repository's existing tags (`2026-06-08(main)` and similar) use a historical format that contains parentheses and cannot be used directly as package names or CPack version numbers.

## 4. Pitfalls hit along the way

| Pitfall | Symptom | Root cause | Fix |
| :--- | :--- | :--- | :--- |
| Headers flattened | Downstream `#include "duilib/duilib.h"` not found | First script version emitted `include/duilib.h` | Keep the `include/duilib/xxx.h` hierarchy |
| Debug library mixed in | Package grew 4.8 MB → 45.8 MB | Only `_d` recognized; MSBuild's directory-name criterion missed | Suffix + directory name, both required |
| `resources.zip` packaged | A 748-byte empty shell in the archive | Not explicitly excluded | Added to `EXCLUDED_RESOURCE_FILES` |
| `global.xml` dropped in SDK mode | Downstream startup fails outright | The resource exclusion also removed `global.xml` | Keep `global.xml` and `public/` explicitly |
| Duplicate entry in `Summary.en.md` | "Release and Packaging" listed twice in the index | The registration script had no existence guard | Fixed; check for duplicates before registering |
| README section hierarchy broken | "Maintenance Notes" became an empty heading, and its 5 `###` subsections got re-parented under "Release packaging" | The new section was inserted between a `##` heading and its subsections, and consumed the blank line after the heading | Fixed; the whole section was moved before its parent heading |
| `pngpriv.h` dependency chain incomplete | The doc said "`pngpriv.h` → `pnglibconf.h`", but there is also `pngdebug.h` | Only the first few includes were checked while writing | See the closure in section 3; all 13 files verified one by one (libpng is no longer packaged as of 2026-10-23) |

**There is also an ordering dependency worth remembering**: `libpng/pngstruct.h:28-29` includes `"zlib.h"` only `#ifndef ZLIB_H`, and there is no `zlib.h` in the `libpng/` directory. It compiles only because `APngDecoder.h:5` includes `zlib/zlib.h` first, defining `ZLIB_H`. This dependency exists only inside the source tree; since 2026-10-23 neither `libpng` nor `APngDecoder.h` is packaged, so it no longer affects the released artifact.

## 5. Known gaps and follow-up

### Gaps

1. **The version number was maintained in two places (resolved 2026-10-23).** The version now lives only in `duilib/duilib_version.h`; `msvc/dllRes/version.h` is an adapter and `VERSION.txt` has been removed. `DUILIB_VER_BUILD` uses MMDD without a leading zero to avoid C/RC octal issues.

2. **All four platforms are now tested (2026-10-28/29).** The script itself is cross-platform (pure standard library, one codebase for all four platforms). Unix `.a` collection and `lib` prefix normalization are implemented. Debug/Release packages were verified for every Windows toolchain combination, Linux x64 (default SDK, `--with-cef`, `--with-examples`; 2026-10-28), macOS x64 (same combinations; 2026-10-28) and FreeBSD x64 (default SDK, `--with-examples`; no CEF; 2026-10-29), and all three Unix platforms also cover post-unpack example startup runs.

3. **Signing and notarization are not automated.** The project produces pure static libraries (`add_library` does not specify `SHARED`, and `msvc/dllRes/dllmain.cpp` only compiles in the `.vcxproj`), so the SDK package itself needs no signature; the documentation only gives the manual procedure. Automated signing is equivalent to B-2's CI — it needs certificate management, which is a long-term maintenance burden.

4. **Two constant names in `pack.py` are misleading (no longer applicable).** `THIRD_PARTY_INCLUDE_DIRS` ("specified subdirectory") and `THIRD_PARTY_INCLUDE_TREE_ROOTS` ("whole tree") are **exactly equivalent in implementation**; only the path assembly differs. Since 2026-10-23 `giflib` was removed along with the image decoder headers, `THIRD_PARTY_INCLUDE_DIRS` is an empty tuple and only `WEBVIEW2_INCLUDE_DIR` is appended per platform, so this wording inaccuracy has no practical effect anymore.

5. **Deliberately dropped sub-items.** NSIS/WiX (installers) and DEB/dmg (system packages) are not done — zip / tgz is enough for a static-library SDK, and building an installer is over-engineering at this stage.

### How to re-verify

When the packaging feature is picked up again, verify in this order:

```
# 1. Use list mode to inspect the collection scope first
python build/pack.py --list

# 2. Package
python build/pack.py

# 3. Integrity check
cd dist/<version> && unzip -q nim_duilib-<version>-<system>-<arch>[-feature].zip -d /tmp/check
cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS

# 4. Downstream compile verification (the key acceptance test)
```

Step 4 is **the most effective acceptance check**: unzip the artifact, write a `probe.cpp` **outside** the package, add **only `/Iinclude`**, and then:

```cpp
#include "duilib/duilib.h"
#include "duilib/WebView2/WebView2Control.h"
```

Expected result: **EXITCODE=0**. This proves "a standalone project outside the package can compile", which is far more convincing than counting files.

> Since 2026-10-23 the image decoder implementation headers (`Image/APngDecoder.h`, `Image/FrameSequence_gif.h`, etc.) and the internal `Core/ZipStreamIO.h` are no longer packaged; do not reference them in the probe.

## 6. Acceptance results already obtained

Measured on Windows on 2026-10-20:

- **Downstream compile verification**: `duilib/duilib.h` and the third_party-dependent headers (`ZipStreamIO.h` / `WebView2Control.h`) all returned **EXITCODE=0**. Since 2026-10-23 `APngDecoder.h` / `FrameSequence_gif.h` / `ZipStreamIO.h` are no longer packaged, so the current probe only needs `duilib.h` + `WebView2Control.h`.
- **Header closure (added 2026-10-23)**: the Windows x64 Debug package contains 245 headers (244 under `duilib/` + 1 from WebView2); apart from the conditional `#ifdef DUILIB_BUILD_FOR_SDL` include in `ControlDragable.h`, **0 unresolved**. The Unix package has 238 headers with a complete closure.
- **Library collection (added 2026-10-23)**: Windows accepts both `.lib` and `.a`; the toolchain is detected from the library directory (MSVC `llvm.*`, MinGW `mingw64-gcc|llvm.*` Skia directories); DLL-mode `bin/duilib*.dll` and dynamic SDL3 `bin/SDL3.dll` are shipped; the manifest writes `-lxxx` for MinGW. Verified with a mocked MinGW tree; the MSVC x64 Debug `--list` is unchanged at 497 files / 314.8 MB.
- **Resource exclusion precision**: with examples off, only `global.xml` and `public/` remain under `themes/default/` in the package; with examples on, all 389 files match the repository exactly.
- **Checksums**: `SHA256SUMS` independently recomputed, 587/587 all matching.
- **Package structure**: both zip and tgz formats are correct.
- **Four-platform differences**: the script is isomorphic, and artifact correctness has been tested for Debug/Release on every Windows toolchain combination, Linux x64 (default SDK, `--with-cef`, `--with-examples`; 2026-10-28), macOS x64 (same combinations; 2026-10-28) and FreeBSD x64 (default SDK, `--with-examples`; no CEF; 2026-10-29) — see gap 2 in section 5.

## 7. Related documents

* [Release and Packaging](../Release.en.md) — the complete user-facing guide
* [Build Process and Scripts](../../build/README.en.md)
* [Development Plan](Development-Plan.md) (Chinese) — the B-3 entry and the overall roadmap
* [Project Audit Report](Project-Audit.md) (Chinese) — the original audit conclusion behind B-3
