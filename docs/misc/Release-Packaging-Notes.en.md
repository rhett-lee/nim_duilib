# Release Packaging Work Record (B-3)

This document is the **work record** for B-3 "Release and packaging process": it captures the delivered results, the technical decisions and their rationale, the pitfalls hit along the way, and the open items needed for follow-up.

The user-facing operation guide lives in [Release and Packaging](../Release.en.md). This document does not repeat that content; it records **why things were done this way** and **what is still not finished**.

> Status: the feature is implemented and tested on Windows, but the work is **currently uncommitted** (on 2026-10-20 it was decided to hold off committing because the scope turned out to be broader than expected; to be picked up in follow-up). The "Current working tree state" section below lists the complete set of files to commit.

## 1. Current working tree state

All changes are **still in the working tree**; the last commit is `5c836967` ("FreeBSD platform, explicitly does not support CEF").

### Files that should be committed

| File | Type | Content |
| :--- | :--- | :--- |
| `VERSION` | new | Single source of truth for the version number, four-segment `1.0.2026.0416` |
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
| `third_party` forwarding | zlib / libpng / giflib / Microsoft.Web.WebView2 | 5 public headers include them directly; without forwarding, downstream builds fail |
| zlib / libpng | Forward whole directory trees of headers, no per-file filtering | Conditional transitive dependencies; a whitelist would miss them (see section 4) |
| Library directory | Only one layout (Windows `lib/<arch>/<Config>`, Unix `lib/<Config>`) | CM-5 D-2 option B; configuration comes from the directory, no `_d` detection or directory merging |
| Debug package | With `--config Debug` the name gains `-debug` and the manifest records the config | Debug libraries carry the debug CRT and `/MTd` and only serve Debug downstream builds; never mix the two |
| Skia libraries | Whole directory shipped, plus a link list in the manifest | The `duilib` static library contains Skia symbols; linking fails without them |
| WebView2 / JPEG | Windows ships loader libraries and `turbojpeg-static` by default | VS enables WebView2 and libjpeg-turbo by default; `duilib.lib` contains those symbols |
| SDL3 | Always shipped on Unix; on Windows only with `--with-sdl` | SDL is off by default on Windows; on Unix it is enabled and built as a shared library |
| CEF development set | Off by default, needs `--with-cef` (optionally `--cef-version 109`) | Each wrapper is 80–100 MB; the runtime still has to be supplied separately |
| `bin/resources.zip` | Always excluded | An empty shell file of only 748 bytes |
| Version number | Single source of truth is the root `VERSION` file | Package name, in-package `VERSION` and `MANIFEST.txt` are all derived from it |

### Resource scope difference with `--with-examples`

This is the easiest thing to get wrong when packaging. `resources/themes/default/` holds both public resources and each example's private theme directories:

| Mode | `resources/` scope |
| :--- | :--- |
| Default (SDK) | Only `themes/default/public/` and `themes/default/global.xml` |
| `--with-examples` | The entire `resources/`, no exclusions |

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

Artifacts go to `dist/`, named `nim_duilib-<version>-<system>-<arch>[-debug].zip`,
containing `VERSION`, `MANIFEST.txt`, `SHA256SUMS`, `LICENSE`,
`include/duilib/`, `lib/` (including Skia and third-party link libraries),
`resources/` and `licenses/`.

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

**Step one — who references it.** There are 313 public headers under `duilib/` (excluding `third_party/` and `RenderSkia/`), as measured on 2026-10-20. Scanning every `#include`, only 5 reference `duilib/third_party/...`, via 9 include directives in total:

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

**Step three — why zlib / libpng must be forwarded as whole trees.** The headers of these two libraries are chained by **same-directory relative includes**, and the chains are guarded by conditions:

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

38 extra headers, all a few KB of text each. The volume is negligible; in exchange, nothing is missed because of a macro combination or an upstream patch-level upgrade.

### 4. Debug library detection: suffix plus directory name

Debug artifacts come from **two** places, and checking only one misses the other:

- Filename carries the `_d` suffix — the CMake path's `duilib_d.lib`
- The containing directory is named `Debug` — the MSBuild path's `lib/Debug/duilib.lib` (**no suffix**, distinguished by directory name only; see `msvc/PropertySheets/DuilibSettings.props`)

The first version recognized only `_d`, and consequently packaged a 232 MB Debug library as if it were Release, inflating the compressed archive from 4.8 MB to 45.8 MB. Detection now uses both conditions.

### 5. Version number: single source of truth and the known gap

The single source of truth for the version is the root `VERSION` file, formatted as `major.minor.patch.build`, e.g. `1.0.2026.0416`. `pack.py` reads it and derives the package name, the in-package `VERSION` and `MANIFEST.txt`; on a malformed value (not four numeric segments) the script errors out rather than producing a half-finished package.

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
| `pngpriv.h` dependency chain incomplete | The doc said "`pngpriv.h` → `pnglibconf.h`", but there is also `pngdebug.h` | Only the first few includes were checked while writing | See the closure in section 3; all 13 files verified one by one |

**There is also an ordering dependency worth remembering**: `libpng/pngstruct.h:28-29` includes `"zlib.h"` only `#ifndef ZLIB_H`, and there is no `zlib.h` in the `libpng/` directory. It compiles only because `APngDecoder.h:5` includes `zlib/zlib.h` first, defining `ZLIB_H`. The in-package `include/duilib/third_party/libpng/pngstruct.h` preserves that order, which is what makes the closure self-consistent.

## 5. Known gaps and follow-up

### Gaps

1. **The version number is maintained in two places.** The `DUILIB_VER_*` macros in `msvc/dllRes/version.h` and the `VERSION` file are currently updated independently, and those macros are used only by the `.vcxproj` — the CMake side cannot read them. A version bump requires changing both. **The follow-up should make `version.h` derive from `VERSION`.**

2. **Only Windows was tested.** The script itself is cross-platform (pure standard library, one codebase for all four platforms). Unix `.a` collection and `lib` prefix normalization are implemented, but the resulting packages have not yet been verified on Linux/macOS/FreeBSD. `docs/Release.en.md` already marks these as "not verified on this machine".

3. **Signing and notarization are not automated.** The project produces pure static libraries (`add_library` does not specify `SHARED`, and `msvc/dllRes/dllmain.cpp` only compiles in the `.vcxproj`), so the SDK package itself needs no signature; the documentation only gives the manual procedure. Automated signing is equivalent to B-2's CI — it needs certificate management, which is a long-term maintenance burden.

4. **Two constant names in `pack.py` are misleading.** `THIRD_PARTY_INCLUDE_DIRS` ("specified subdirectory") and `THIRD_PARTY_INCLUDE_TREE_ROOTS` ("whole tree") are **exactly equivalent in implementation** — both blocks at `pack.py:260-282` recursively walk `iter_files(root)`; the only difference is how the path is assembled. `giflib` sits in DIRS but has its **entire directory** forwarded (3 headers), not "only the one specified file". What the two constants really encode is "**single-component shortcut path** vs **multi-component path tuple**". `('giflib',)` could be merged into TREE_ROOTS with zero behavioral change. **The comments and constant wording should be adjusted** (not a bug, a wording inaccuracy).

5. **Deliberately dropped sub-items.** NSIS/WiX (installers) and DEB/dmg (system packages) are not done — zip / tgz is enough for a static-library SDK, and building an installer is over-engineering at this stage.

### How to re-verify

When the packaging feature is picked up again, verify in this order:

```
# 1. Use list mode to inspect the collection scope first
python build/pack.py --list

# 2. Package
python build/pack.py

# 3. Integrity check
cd dist && unzip -q nim_duilib-<version>-<system>-<arch>.zip -d /tmp/check
cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS

# 4. Downstream compile verification (the key acceptance test)
```

Step 4 is **the most effective acceptance check**: unzip the artifact, write a `probe.cpp` **outside** the package, add **only `/Iinclude`**, and then:

```cpp
#include "duilib/duilib.h"
#include "duilib/Core/ZipStreamIO.h"
#include "duilib/Image/APngDecoder.h"
#include "duilib/Image/FrameSequence_gif.h"
#include "duilib/WebView2/WebView2Control.h"
```

Expected result: **EXITCODE=0**. This proves "a standalone project outside the package can compile", which is far more convincing than counting files.

## 6. Acceptance results already obtained

Measured on Windows on 2026-10-20:

- **Downstream compile verification**: `duilib/duilib.h` and the 4 third_party-dependent headers (`ZipStreamIO.h` / `APngDecoder.h` / `FrameSequence_gif.h` / `WebView2Control.h`) all returned **EXITCODE=0**.
- **Resource exclusion precision**: with examples off, only `global.xml` and `public/` remain under `themes/default/` in the package; with examples on, all 389 files match the repository exactly.
- **Checksums**: `SHA256SUMS` independently recomputed, 587/587 all matching.
- **Package structure**: both zip and tgz formats are correct.
- **Four-platform differences**: the script is isomorphic, but **only Windows artifact correctness has been tested** (see gap 2 in section 5).

## 7. Related documents

* [Release and Packaging](../Release.en.md) — the complete user-facing guide
* [Build Process and Scripts](../../build/README.en.md)
* [Development Plan](Development-Plan.md) — the B-3 entry and the overall roadmap
* [Project Audit Report](Project-Audit.md) — the original audit conclusion behind B-3
