# 发布打包工作记录（B-3）

本文档是 B-3「发布与打包流程」的**工作记录**：记录已产出的成果、做过的技术决策与依据、踩过的坑，以及后续跟进时需要知道的待办。

面向使用者的操作说明在 [发布与打包](../Release.md)。本文不重复那份内容，只记录**为什么这么做**和**还没做完什么**。

> 状态：功能已实现并通过 Windows 实测，**改动已提交**（B-3 阶段 `e4064ba8`；CM-5 P4 的补齐见 `25d34686`）。下文「当前工作区状态」一节保留为提交时的清单记录。

## 一、当前工作区状态

改动**已提交**：B-3 阶段为 `e4064ba8`，CM-5 P1~P5 为 `7f8f1500` ~ `6e2a2f0f`。以下清单保留为提交内容的历史记录。

### 应当提交的文件

| 文件 | 类型 | 内容 |
| :--- | :--- | :--- |
| `duilib/duilib_version.h` | 新增 | 版本号单一事实源，四段式（2026-10-23 取代原 `VERSION.txt` 方案） |
| `build/pack.py` | 新增 | 打包脚本，纯标准库、跨平台、不参与编译 |
| `docs/Release.md` / `docs/Release.en.md` | 新增 | 面向使用者的打包与发布说明 |
| `docs/misc/Release-Packaging-Notes.md` / `.en.md` | 新增 | 本文档 |
| `docs/Summary.md` / `Summary.en.md` | 修改 | 索引登记 |
| `build/README.md` / `README.en.md` | 修改 | 新增「发布打包」章节 |
| `docs/misc/Development-Plan.md` | 修改 | B-3 条目改为已完成并指向本文 |
| `.gitignore` | 修改 | 新增 `/dist` |

### 不应提交的临时脚本

`build/` 目录下这一批是过程性脚本，**按项目纪律不纳入提交**：

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

`build/pack.py` 是**唯一**要提交的 `build/` 下新增文件（其余 `.py` / `.bat` 均为过程脚本）。

## 二、成果清单

### 产物契约

`pack.py` 强制的规则，全部有明确依据：

| 项 | 规则 | 依据 |
| :--- | :--- | :--- |
| 头文件路径 | 必须是 `include/duilib/xxx.h`，**不可压平** | 下游写法是 `#include "duilib/duilib.h"`（`examples/basic/MainForm.h:5`），压平则下游全部编译失败 |
| `RenderSkia/` | 不对外暴露 | `duilib.h` 不引用它，是渲染后端实现细节 |
| `Core/ZipStreamIO.h` | 不对外暴露 | 只被 `ZipManager.cpp` 使用，`duilib.h` / `ZipManager.h` 都不引用；不转发后 zlib 头文件也无需随包 |
| `Core/ControlDropTarget*`、`Core/WindowDropTarget_*`、`Core/SDL_MacOS.h` 及 `MessageLoop_*` / `ProcessSingleton_*` / `RichEdit*_Windows/_SDL` 等平台实现头 | 不对外暴露 | 只被 `.cpp` / `.mm` 包含，任何对外头文件都不引用 |
| `Core/DragWindowFilter_SDL.h` | 非 Windows 必带；Windows 仅 `--with-sdl` | `ControlDragable.h`（在 `duilib.h` 链上）于 `DUILIB_BUILD_FOR_SDL` 下包含它；非 Windows 该宏恒为开 |
| `third_party` 转发 | 仅 Microsoft.Web.WebView2（且仅 Windows） | 对外头文件只有这一处依赖；图片解码实现头文件与 `Core/ZipStreamIO.h` 均只在库内部使用，libpng / giflib / zlib 头文件随之不再需要 |
| 库目录 | 只认一套布局（Windows `lib/<arch>/<Config>`、Unix `lib/<Config>`） | CM-5 D-2 方案 B；配置由目录表达，删除 `_d` 判定与多目录合并 |
| Windows 工具链 | 按库目录内容判定：`duilib.lib` → MSVC，`libduilib*.a` → MinGW；MinGW 收集 `.a` / `.dll.a`，Skia 探测 `mingw64-gcc.*` / `mingw64-llvm.*` | `build/gcc-mingw-w64_build.bat` 与 `llvm-mingw-w64_build.bat` 分别产出上述库名与 Skia 目录；两类产物不可混用 |
| DLL 模式运行库 | Windows 下若存在 `bin/duilib.dll` / `duilib_d.dll` / `duilib32.dll` / `duilib32_d.dll` 则随包，`lib/` 中为导入库；JPEG 随包时还收集 `bin/turbojpeg.dll` | `build.py --dll` 或 `EnableDuilibDll`（`duilib_dll.flag`）开启时输出 DLL 到 `bin/`，导入库仍写 `lib/<arch>/<Config>/duilib.lib`；DLL 内部已链接 `turbojpeg.lib`，不再随包静态库；缺 DLL 下游运行失败 |
| Debug 包 | `--config Debug` 时包名加 `-debug`、MANIFEST 标注 | Debug 库带调试 CRT（静态 `/MTd`，DLL 模式 `/MDd`），只能给 Debug 下游；两套不可混用 |
| VS 接入层 | Windows + MSVC 包内生成 `msvc/nim_duilib.props`、`.targets`、`nim_duilib.sdk.props` 与 `msvc/manifest/`（x64、x86 两份） | 下游导入一次 `.targets` 即可完成 include/宏/链接/manifest/DLL 复制；x64 机器上也能开发 x86 程序，故两种 manifest 都随包；元数据来自实际收集结果 |
| CMake 接入层 | 所有平台生成 `lib/cmake/nim_duilib/*.cmake`，提供 `nim_duilib::duilib` 与 `nim_duilib_copy_runtime()` | `find_package(nim_duilib CONFIG REQUIRED)`；架构/配置/工具链/运行库不匹配默认 FATAL |
| MSVC 运行库元数据 | `--msvc-runtime auto` 时按模式推断：DLL 模式 `MD`，否则 `MT`；写入包元数据供下游对齐 | 混用 `/MT` 与 `/MD` 会触发 LNK2038，因此默认强制对齐，可用开关退出 |
| Skia 库 | 整目录随包，并写入链接列表 | `duilib` 静态库含 Skia 符号，缺 Skia 无法链接 |
| WebView2 / JPEG | Windows 默认随包 loader 库、`bin/WebView2Loader.dll` 与 `turbojpeg-static`；DLL 模式改为 `bin/turbojpeg.dll` | VS 默认启用 WebView2 与 libjpeg-turbo，`duilib.lib` 有对应符号；CMake 使用动态 loader，运行期需要该 DLL；DLL 模式的 duilib.dll 已链接动态 turbojpeg |
| SDL3 | Unix 始终随包；Windows 仅 `--with-sdl`；按产物收集静态库或导入库，动态时连同 `bin/SDL3.dll` | Windows 默认关闭 SDL；Unix 默认启用且为动态库；MinGW 的 CMake 构建链接动态 SDL3（`libSDL3.dll.a`） |
| CEF 开发包 | 默认不打，需 `--with-cef`（可选 `--cef-version 109`） | wrapper 单个 80–100 MB；运行库仍需外置 |
| `bin/resources.zip` | 一律排除 | 仅 748 字节的空壳文件 |
| `docs/` | 只打包顶层使用文档；`misc/`、`specs/` 等子目录一律不打包 | 使用文档随 SDK 提供；`Release.*`（发布流程）、`Getting-Started.*` / `Examples.*`（库内部文档）与 `感谢名单.txt`（致谢）面向维护者，不适合分发；`Summary.*` 已去掉指向包外目录的链接，随包 |
| 版本号 | 单一事实源为 `duilib/duilib_version.h` 的四个数字宏 | 包名、包内 `VERSION`、`MANIFEST.txt` 全部由它派生；`pack.py` 忽略注释与无关宏，缺失/重复/越界时报错 |

### `--with-examples` 的资源范围差异

这是打包时最容易搞错的一点。`bin/resources/themes/default/` 下同时放着公共资源和各示例的私有主题目录：

| 模式 | `bin/resources/` 收录范围 |
| :--- | :--- |
| 默认（SDK） | 仅 `themes/default/public/` 与 `themes/default/global.xml` |
| `--with-examples` | 整个 `bin/resources/`，无排除项 |

`global.xml` 在 SDK 模式下**必须保留**。它是框架启动的必需文件，`GlobalManager.cpp:440` 在本地文件模式下找不到它会直接 `return false`，初始化直接失败。

SDK 模式下被排除的是 22 个示例私有主题目录：
`MultiLang`、`basic`、`cef`、`cef_browser`、`chat`、`child_window`、
`color_theme`、`controls`、`dpi_aware`、`layout`、`list_box`、`list_ctrl`、
`move_control`、`panel`、`render`、`rich_edit`、`threads`、`tree_view`、
`virtual_list_box`、`webview2`、`webview2_browser`、`xml_preview`。

### 命令行接口

```
python build/pack.py                      # Release SDK 包（不含示例）
python build/pack.py --config Debug       # Debug SDK 包（包名带 -debug）
python build/pack.py --with-examples      # SDK + 示例程序
python build/pack.py --with-cef           # 额外带上 CEF 开发包
python build/pack.py --with-sdl           # Windows 额外带上 SDL3
python build/pack.py --list               # 只打印将收集的文件，不打包
python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
python build/pack.py --lib-dir lib/x86/Debug  # 显式指定库目录
python build/pack.py --out-dir dist       # 指定输出目录
```

直接运行 `pack.py` 时产物输出到 `dist/`；`release.py package` 固定输出到
`dist/<版本>/`。包名格式
`nim_duilib-<版本>-<系统>-<架构>[-sdl][-cef|-cef109][-examples][-debug]`，
内含 `VERSION`、`MANIFEST.txt`、`SHA256SUMS`、`LICENSE`、`include/duilib/`、
`lib/`（含 Skia 与第三方链接库）、`bin/`（DLL 与 `bin/resources/`）、
`licenses/`、`docs/`
（顶层使用文档）。

## 三、关键技术决策与依据

### 1. 为什么不走 CPack / `install()`

原审计结论（零 CPack、零 `install()` 规则、零 NSIS/WiX/DEB/DMG）**仍然成立**，但**方案整体更换**。原因是架构不匹配，不是遗漏：

- **补 `install()` 规则 —— 无处可补**。本仓库**没有根 `CMakeLists.txt`**，`duilib/`、`examples/*/`、`duilib/third_party/*/` 各自独立 `project()`，由 `msvc_build.bat`、`linux_build.sh` 等脚本逐个 configure + build。`install()` 只能挂在单个项目里，没有任何单一 CMake 项目能覆盖全部产物。
- **CPack —— 需要顶层项目才能挂载**，同上。更关键的是 Windows 有两条构建路径（`build/*.sln` 共 36 个 `.vcxproj` 的 MSBuild 路径、`msvc_build.bat` 的 CMake 路径），产物混在同一 `lib/`、`bin/` 下，CPack 产物对 MSBuild 用户是空的。
- 因此改为「**独立打包脚本**」路线：与构建体系解耦，按文件名白名单收集，两条构建路径的产物都能收，也不需要新增顶层工程。

### 2. 头文件层级不可压平

包内头文件保持 `include/duilib/xxx.h` 的层级，与仓库根的相对路径一致。下游的引用写法是 `#include "duilib/duilib.h"`，因此 `include/` 下面必须还有一层 `duilib/`。

首版打包脚本把头文件压平为 `include/duilib.h`，下游立刻全部编译失败。

### 3. `third_party` 转发范围的判定链

这是本次工作中推导最细的一块，判定链路如下：

**第一步 —— 谁在引用。** 对外头文件（`duilib/` 下排除 `third_party/` 与 `RenderSkia/`）共 313 个（2026-10-20 实测）。逐个扫描 `#include`，当时只有 5 个引用了 `duilib/third_party/...`，共 9 条引用：

> 2026-10-23 更新：下表中 `Image/APngDecoder.h`、`Image/FrameSequence_gif.h`、`Core/ZipStreamIO.h` 已改为不随包；当前随包的 third_party 头文件只剩 WebView2（且仅 Windows），zlib / libpng / giflib 均不再转发，详见 3.1 节。

| 对外头文件 | 引用的第三方头文件 |
| :--- | :--- |
| `Core/ZipStreamIO.h:5` | `zlib/contrib/minizip/ioapi.h` |
| `Image/APngDecoder.h:5-9` | `zlib/zlib.h` + `libpng/{pngpriv,pngstruct,pnginfo,png}.h` |
| `Image/FrameSequence_gif.h:20` | `giflib/gif_lib.h` |
| `WebView2/WebView2Control.h:10` | `Microsoft.Web.WebView2/build/native/include/WebView2.h` |
| `WebView2/WebView2EnvironmentOptions.h:9` | 同上 |

关键在于这些 include 写的是 **`duilib/third_party/...` 全路径**（不是 `zlib.h` 这种裸名），所以包内必须原样落在 `include/duilib/third_party/<原路径>`。这与「头文件不可压平」是同一个约束的两面。

**第二步 —— 必需文件集。** 从那 5 个直接引用出发递归解析同目录 / 相对 include，得到最小闭包共 13 个文件：

```
zlib      必需 4：zlib.h → zconf.h；contrib/minizip/ioapi.h → ../../zlib.h, ints.h
libpng    必需 7：png.h → pnglibconf.h, pngconf.h
                  pngpriv.h → pnglibconf.h, png.h, pngstruct.h, pnginfo.h, pngdebug.h
                  pngstruct.h, pnginfo.h
giflib    必需 1：gif_lib.h（只依赖 <stdbool.h> / <stddef.h>）
WebView2  必需 1：WebView2.h
```

**第三步 —— 为什么 zlib / libpng 必须整树转发（2026-10-20 的历史决策，2026-10-23 起已不再转发）。** 这两个库的头文件之间是**同目录相对 include 链**，且带条件分支：

- `pngpriv.h:85` `#include "pngprefix.h"` —— 仅当 `PNG_PREFIX`
- `pngpriv.h:89` `#include "pngusr.h"` —— 仅当 `PNG_USER_CONFIG`
- `png.h:328` `#include "pnglibconf.h"` —— 仅当 `!PNGLCONF_H`
- `png.h:337` `#include "pngconf.h"` —— 仅当 `!PNG_VERSION_INFO_ONLY`
- `ioapi.h:74` `#include "mz64conf.h"` —— 仅当 `HAVE_MINIZIP64_CONF_H`（该文件本仓库根本没有）

按文件白名单转发，就得穷举所有宏组合；漏一个，下游换个编译开关就编译失败。整目录保形转发天然覆盖全部条件分支 —— **这才是「不现实，做整目录转发」的真正原因，不是文件数量问题**。

**第四步 —— giflib 与 WebView2 的特殊性。** `gif_lib.h` 只 include `<stdbool.h>` / `<stddef.h>`，无第三方相对依赖；`WebView2.h` 额外的 5 个依赖（`rpc.h` `rpcndr.h` `objidl.h` `oaidl.h` `EventToken.h`）**全部来自 Windows SDK**（`shared/` `um/` `winrt/`），不在 third_party 内，无需转发。WebView2 头文件位于 `third_party/Microsoft.Web.WebView2/build/native/include/`，路径深，所以需要多级路径分量元组。

**成本核算**（2026-10-20 实测）：

| 目录 | 整树转发的 .h 数 | 闭包中必需数 |
| :--- | :--- | :--- |
| zlib | 31 | 4 |
| libpng | 18 | 7 |
| giflib | 3 | 1 |
| Microsoft.Web.WebView2 | 1 | 1 |

多带 38 个头文件，全是几 KB 的文本文件，体积可忽略，换来不因宏组合或上游小版本升级而漏文件。（该表是 2026-10-20 的整树转发口径；2026-10-23 起 libpng / giflib / zlib 均已不再转发，实际只转发 WebView2 的 1 个头文件，见 3.1 节。）

### 3.1 图片解码头与内部实现头文件已排除打包（2026-10-23）

**结论。** 具体图片解码实现头文件只在库内部被 `.cpp` 使用，`duilib.h` 的包含链完全不引用，因此不再随 SDK 转发。

`duilib.h` 的图片入口及传递依赖：

```
Image.h               -> ImageInfo.h / ImageAttribute.h / ImageLoadParam.h
                      -> StateImageMap.h -> StateImage.h
ImageInfo.h           -> ImageDecoder.h
ImageDecoderFactory.h -> ImageDecoder.h / ImageUtil.h
```

包内 `Image/` 因此只保留 10 个基础头文件：`Image.h`、`ImageAttribute.h`、
`ImageInfo.h`、`ImageLoadParam.h`、`ImageDecoder.h`、`ImageDecoderFactory.h`、
`ImageUtil.h`、`StateImage.h`、`StateImageMap.h`、`ImagePlayer.h`。

**排除清单（`pack.py` 的 `IMAGE_DECODER_HEADER_FILES`，共 23 个）：**

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

**third_party 随之收敛：**

- `THIRD_PARTY_INCLUDE_TREE_ROOTS` 删除：`libpng` 只被 `Image/APngDecoder.h` 引用，而 `zlib` 只被 `Core/ZipStreamIO.h` 引用。
- `THIRD_PARTY_INCLUDE_DIRS` 删除：`giflib` 只被 `Image/FrameSequence_gif.h` 引用。
- 新增 `INTERNAL_HEADER_FILES`：除 `Core/ZipStreamIO.h` 外，还包含 6 个平台实现头文件
  `Core/ControlDropTargetImpl_SDL.h`、`Core/ControlDropTargetImpl_Windows.h`、
  `Core/ControlDropTargetUtils.h`、`Core/SDL_MacOS.h`、`Core/WindowDropTarget_SDL.h`、
  `Core/WindowDropTarget_Windows.h`。它们只被 `.cpp` / `.mm` 包含。
- 2026-10-23 补充：`INTERNAL_HEADER_FILES` 再纳入 17 个仅被 `.cpp` 包含的实现头：
  `Core/SDL_Linux.h`、`Core/MessageLoop_SDL.h`、`Core/MessageLoop_Windows.h`、
  `Utils/ProcessSingleton_{Linux,MacOS,Windows}.h`、`Utils/BitmapHelper_{SDL,Windows}.h`、
  `Utils/ApiWrapper_Windows.h`、`Utils/Macros_Windows.h`、`Utils/DiskUtils_Windows.h`、
  `Utils/DllManager_Windows.h`、`Utils/InlineHook_Windows.h`、
  `Control/RichEditHost_Windows.h`、`Control/RichEditDragSource_Windows.h`、
  `Control/RichEditDropTarget_Windows.h`、`Control/RichEditDropTarget_SDL.h`。
- 新增 `SDL_HEADER_FILES = ('Core/DragWindowFilter_SDL.h',)`：`Core/ControlDragable.h`
  在 `DUILIB_BUILD_FOR_SDL` 下包含它。非 Windows 平台该宏恒为开，必须保留；
  Windows 仅在 `--with-sdl` 时保留。
- `collect_jpeg_turbo` 不再转发 `turbojpeg.h` 等头文件；静态包仍随
  `turbojpeg-static.lib`，DLL 包改为随 `bin/turbojpeg.dll`（duilib.dll 已链接
  `turbojpeg.lib` 导入库）。
- 最终只转发 `Microsoft.Web.WebView2`（仅 Windows）。

**验证（2026-10-23，Windows x64 Debug `--list`，EXIT=0）：**

- 头文件共 245 个（未启用 `--with-sdl`）：`duilib/` 244 个 + WebView2 1 个。
- `Image/` 仅 10 个基础头文件，无任何解码实现头文件泄漏；`Core/ZipStreamIO.h`、6 个平台实现头与新增的 17 个内部实现头均不在包内。
- 对 245 个包内头文件扫描 `#include "duilib/..."`：除 `ControlDragable.h` 中
  `#ifdef DUILIB_BUILD_FOR_SDL` 的条件包含外，**0 处缺失**；Windows 未启用 SDL 时该分支不编译，
  启用 `--with-sdl` 时会自动把 `DragWindowFilter_SDL.h` 打进包（246 个头文件）。
- Unix 包：保留 `DragWindowFilter_SDL.h`，排除 WebView2 头文件后共 238 个头文件，闭包完整。
- 宏组合预演：Windows + `--with-cef` 270 个、Unix + CEF 263 个，闭包同样完整。

### 3.2 库收集：MinGW 与 DLL 模式（2026-10-23）

此前 `pack.py` 在 Windows 只认 `.lib`：MinGW 构建的 SDK 会因找不到基础库直接失败，
Skia 也只会探测 `llvm.<arch>.<config>`；DLL 模式的 `bin/duilib*.dll` 不随包，
下游能链接但运行期缺 DLL。现已修正：

- `_scan_lib_dir` / `collect_skia_libs` 在 Windows 同时接受 `.lib` 与 `.a`；
  `libduilib-zlib.a` → `duilib-zlib`，导入库 `libduilib.dll.a` → `duilib`。
- `detect_windows_toolchain` 按库目录内容判定：`duilib.lib` 为 MSVC，
  `libduilib*.a` 为 MinGW；`resolve_skia_lib_dir` 据此只探测对应的 Skia 目录
  （MSVC `llvm.<arch>.<release|debug>`，MinGW `mingw64-gcc|llvm.<arch>.release`）。
- DLL 模式收集 `bin/duilib[_d].dll` / `duilib32[_d].dll`（存在才收集）；清单
  运行期注明需与下游可执行文件同级。
- `--with-sdl` 按产物收集：`SDL3-static.lib`（MSVC 静态）、`SDL3.lib` /
  `libSDL3.dll.a` + `bin/SDL3.dll`（动态）、`libSDL3.a`（MinGW 静态）。
- MANIFEST 工具链为 MinGW 时链接列表输出 `-lxxx`，系统库也改写为
  `-lversion -lwinmm -limm32 -lsetupapi`。

**验证：** MSVC x64 Debug `--list` 仍为 497 个文件 / 314.8 MB，与改动前一致；
用临时目录模拟 MinGW 产物，得到 5 个 `.a`、`mingw64-gcc.x64.release` 的 Skia 目录、
`libSDL3.dll.a` + `bin/SDL3.dll` 与 `bin/duilib.dll`，清单按 `-lxxx` 输出。

### 3.3 下游接入层（2026-10-23）

发布包此前只有头文件与库，下游要自己拼 include、库列表、宏、manifest 和
运行期 DLL 复制。现在由 `pack.py` 按**实际收集结果**生成两套接入机制：

- **VS（Windows + MSVC）**：`msvc/nim_duilib.targets` 自动导入
  `nim_duilib.props` 与 `nim_duilib.sdk.props`，完成 include、`DUILIB_*` 宏、
  `stdcpp20`、库列表/系统库、MSVC 运行库对齐、manifest 嵌入，并在构建后
  复制 `bin/*.dll`；`NimDuilibCopyResources=1` 时再复制 `bin/resources/` 到
  输出目录的 `resources/`（与可执行文件同级）。
- **CMake（全平台）**：`lib/cmake/nim_duilib/nim_duilibConfig.cmake` 提供
  `nim_duilib::duilib`；校验平台/工具链/架构/配置/运行库，并提供
  `nim_duilib_copy_runtime(<target> [RESOURCES] [FRAMEWORKS] [DESTINATION <dir>])`；
  `FRAMEWORKS`（macOS）把 `lib/libSDL3*.dylib` 复制到 App Bundle 的
  `Contents/Frameworks`，并把可执行文件的 rpath 设为
  `@executable_path/../Frameworks`（`BUILD_WITH_INSTALL_RPATH`，bundle 内的库优先）；
  `MACOSX_BUNDLE` 目标未指定 `DESTINATION` 时，`RESOURCES` 复制到
  `Contents/Resources/duilib/resources/`。
- 元数据字段：版本、系统、架构、配置、工具链、DLL 模式、SDL 形态、
  WebView2/JPEG/CEF、MSVC 运行库、运行期 DLL。MinGW 包不生成 `msvc/`；
  WebView2/JPEG/CEF 只在 MSVC 下声明为可用。
- `--msvc-runtime auto|mt|md` 控制写入元数据的运行库；`auto` 为 DLL 模式
  `MD`、否则 `MT`。

**验证（2026-10-23）：**

- `py_compile` 通过；Windows x64 Debug `--list` 共 538 个文件 / 315.0 MB，
  其中接入文件 9 个（manifest 2 + 生成 7），原有 497 个文件未减少。
- Windows 包同时带 `duilib.x64.manifest` 与 `duilib.x86.manifest`，
  下游按工程 Platform 选择（x64 机器上也能开发 x86 程序）。
- 三份 props/targets 均通过 XML 解析；props 带 UTF-8 BOM，虚拟文件在
  `SHA256SUMS` 中的摘要与包内字节一致。
- 临时 CMake 消费工程 `find_package(nim_duilib CONFIG REQUIRED)` 配置成功，
  target 的 include/宏/库列表正确；架构不匹配（Win32 工程消费 x64 包）与
  MSVC 运行库不一致（MD 工程消费 MT 包）均按预期 `FATAL_ERROR`。
- 伪造 MinGW 库目录：不生成 `msvc/`，只输出 4 个 CMake 接入文件；
  `--with-sdl` / `--with-cef` 变体 `--list` 均正常。

### 3.4 docs 文档打包（2026-10-23）

`docs/` 顶层的使用文档随 SDK 打进包内 `docs/`，只取一级文件，
`misc/`、`specs/` 等子目录一律不打包。以下属于库自身的维护文档，不打包：

- `Release.md` / `Release.en.md`：发布与打包流程（版本、签名、公证、构建脚本）；
- `Getting-Started.md` / `Getting-Started.en.md`、`Examples.md` / `Examples.en.md`：
  库内部的快速上手与示例程序说明；
- `感谢名单.txt`：项目致谢。

`Summary.md` / `Summary.en.md` 是包内文档索引，已移除指向其他目录的链接
（`../README.md`、`../build/build-reference.md`、`build/README.md`、`misc/` 等），以及未随包文档的条目。

文档里有大量指向源码树的链接，打包时按实际收集结果重写（`docs/` 是虚拟文件，
不落盘）：

| 原链接 | 包内处理 |
| :--- | :--- |
| `../duilib/xxx.h` | 该头文件随包时改为 `../include/duilib/xxx.h` |
| `../bin/resources/...` | 保持不变（包内资源同样位于 `bin/` 下） |
| `../../bin/resources/...` | 改为 `../bin/resources/...` |
| `../examples/`、`../build/`、`*.cpp`、未随包的内部头文件 | 去掉链接，只保留显示文字 |

这样包内文档不存在断链，仓库内文档保持原样、仍可在源码树中跳转。
`pack.py` 的 `collect_docs()` 负责该转换，文档作为虚拟文件写入 zip/tar；
`MANIFEST.txt` 的“文档”章节逐条列出随包文档，`SHA256SUMS` 覆盖全部文档。

排除名单集中在 `pack.py` 的 `DOC_EXCLUDE_FILES`，新增顶层文档默认随包。
Windows x64 Debug 验证：文档 32 个（16 对中英文）；包内 32 篇文档的相对链接
全部指向包内文件（0 断链），SHA256SUMS 校验一致。2026-10-29 新增
`Deployment.md` / `Deployment.en.md` 后为 34 篇（17 对），链接转换规则不变。

### 4. Debug 库判定：后缀 + 目录名双条件

Debug 来源有**两处**，只看一个都会漏：

- 文件名带 `_d` 后缀 —— CMake 路径的 `duilib_d.lib`
- 所在目录名为 `Debug` —— MSBuild 路径的 `lib/Debug/duilib.lib`（**无后缀**，只靠目录名区分，见 `msvc/PropertySheets/DuilibSettings.props`）

首版只认 `_d`，结果把 232 MB 的 Debug 库当成 Release 打进了包，压缩后体积从 4.8 MB 涨到 45.8 MB。现按双条件判定。

### 5. 版本号：单一事实源与已知缺口

版本号的单一事实源是 `duilib/duilib_version.h` 中的四个数字宏，例如 `1.0.2026.1023`。`pack.py` 逐行解析该头文件（忽略注释、`#undef` 与其它宏，缺失/重复/越界时报错），用于包名、包内 `VERSION` 与 `MANIFEST.txt`。`msvc/dllRes/version.h` 已改为 Windows 资源编译适配层，不再保存版本数字；`VERSION.txt` 已删除。

git tag 建议用 `v<版本号>` 格式。仓库现有 tag（`2026-06-08(main)` 等）是历史格式，含括号，不能直接用作包名或 CPack 版本号。

## 四、踩过的坑

| 坑 | 现象 | 根因 | 修法 |
| :--- | :--- | :--- | :--- |
| 头文件被压平 | 下游 `#include "duilib/duilib.h"` 全部找不到 | 首版脚本输出 `include/duilib.h` | 保持 `include/duilib/xxx.h` 层级 |
| Debug 库混入 | 包体积 4.8 MB → 45.8 MB | 只认 `_d` 后缀，漏了 MSBuild 的目录名判据 | 后缀 + 目录名双条件 |
| `resources.zip` 被打进 | 包里多一个 748 字节空壳 | 未显式排除 | 加入 `EXCLUDED_RESOURCE_FILES` |
| SDK 模式丢了 `global.xml` | 下游启动直接失败 | 资源排除时把 `global.xml` 一起排掉了 | 单独保留 `global.xml` 与 `public/` |
| `Summary.en.md` 重复登记 | 索引里「Release and Packaging」出现两次 | 登记脚本无存在性守卫 | 已修，登记前先查重 |
| README 章节层级断裂 | 「维护注意事项」变成空标题，5 个 `###` 子节被挂到「发布打包」下 | 新章节被插在 `##` 标题与其子节之间，且吃掉了标题后的空行 | 已修，章节整段移到父标题之前 |
| `pngpriv.h` 依赖链漏项 | 文档写「`pngpriv.h` → `pnglibconf.h`」，实际还有 `pngdebug.h` | 写文档时只看了前几处 include | 见第三节闭包，13 个文件已逐个核实（2026-10-23 起 libpng 不再随包） |

**另有一条顺序依赖**值得记住：`libpng/pngstruct.h:28-29` 是 `#ifndef ZLIB_H` 才 `#include "zlib.h"`，而 `libpng/` 目录下并没有 `zlib.h`。它能编过完全靠 `APngDecoder.h:5` 先包含了 `zlib/zlib.h` 把 `ZLIB_H` 定义掉。该依赖只存在于源码树内部；2026-10-23 起 `libpng` 与 `APngDecoder.h` 都不再随包，因此不影响发布产物。

## 五、已知缺口与后续跟进

逐项待办与验收标准已整理到 [Packaging-Follow-Up-Items.md](Packaging-Follow-Up-Items.md)，后续以该清单为准推进；本节保留已完成工作的缺口记录。

### 缺口

1. **版本号两处独立维护（已解决，2026-10-23）。** 版本已统一到 `duilib/duilib_version.h`；`msvc/dllRes/version.h` 改为适配层，`VERSION.txt` 已删除。`DUILIB_VER_BUILD` 按 MMDD 书写且不补零，避免 C/RC 的八进制问题。

2. **已实测 Windows、Linux 与 FreeBSD。** 脚本本身跨平台（纯标准库，四平台共用同一份代码），Unix 的 `.a` 收集与 `lib` 前缀归一化已实现。Windows 各工具链组合、Linux x64 Release（默认 SDK、`--with-cef`、`--with-examples`）与 FreeBSD x64 Release（默认 SDK、`--with-examples`，CEF 不支持）出包均已实测（2026-10-25）；macOS 的产物尚未在本机实测，`docs/Release.md` 中仍标注「未在本机验证」。Linux/FreeBSD 的 Debug 配置与解包后示例启动运行也未覆盖。

3. **签名与公证未自动化。** 本项目产出纯静态库（`add_library` 未指定 `SHARED`，`msvc/dllRes/dllmain.cpp` 只在 `.vcxproj` 中编译），SDK 包本身无需签名，文档只给手工流程。自动化签名等同 B-2 的 CI —— 需要证书管理，是长期维护负担。

4. **`pack.py` 两处常量命名有误导（已不适用）。** `THIRD_PARTY_INCLUDE_DIRS`（"指定子目录"）与 `THIRD_PARTY_INCLUDE_TREE_ROOTS`（"整树"）在实现上**完全等价**，区别仅在路径拼接层数。2026-10-23 起 `giflib` 已随图片解码头文件一同移除，`THIRD_PARTY_INCLUDE_DIRS` 改为空元组，仅由 `WEBVIEW2_INCLUDE_DIR` 按平台追加，这条表述偏差不再有实际影响。

5. **已放弃的子项。** NSIS/WiX（安装程序）与 DEB/dmg（系统包）不做 —— 静态库 SDK 用 zip / tgz 足够，制作安装程序对当前阶段是过度设计。

### 重新验收的方法

打包功能重新启用时，按这个顺序验收：

```
# 1. 列表模式先看收集范围
python build/pack.py --list

# 2. 打包
python build/pack.py

# 3. 完整性校验
cd dist/<版本> && unzip -q nim_duilib-<版本>-<系统>-<架构>[-特性].zip -d /tmp/check
cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS

# 4. 下游编译验证（关键验收）
```

第 4 步是**最有效的验收手段**：解压产物后，在包外写一个 `probe.cpp`，**只加 `/Iinclude`**，然后：

```cpp
#include "duilib/duilib.h"
#include "duilib/WebView2/WebView2Control.h"
```

期望 **EXITCODE=0**。这项证明「包外独立工程可编译」，比检查文件数量有效得多。

> 2026-10-23 起，图片解码实现头文件（`Image/APngDecoder.h`、`Image/FrameSequence_gif.h` 等）与内部实现头文件 `Core/ZipStreamIO.h` 都不再随包，探针里不要再引用它们。

## 六、当前已有的验收结果

2026-10-20 在 Windows 上的实测结果：

- **下游编译验证**：`duilib/duilib.h` 与依赖 third_party 的头文件（`ZipStreamIO.h` / `WebView2Control.h`）**全部 EXITCODE=0**；2026-10-23 起 `APngDecoder.h` / `FrameSequence_gif.h` / `ZipStreamIO.h` 已改为不随包，当前探针只需 `duilib.h` + `WebView2Control.h`。
- **头文件闭包（2026-10-23 补充）**：Windows x64 Debug 包共 245 个头文件（`duilib/` 244 + WebView2 1），除 `ControlDragable.h` 的 `#ifdef DUILIB_BUILD_FOR_SDL` 条件包含外 **0 处缺失**；Unix 包 238 个，闭包完整。
- **资源排除精确性**：关闭示例时包内 `themes/default/` 下只有 `global.xml` 与 `public/`；打开时 389 个文件与仓库完全一致。
- **校验和**：`SHA256SUMS` 独立复算 587/587 全部一致。
- **包结构**：zip 与 tgz 两种格式均正确。
- **四平台差异**：脚本同构；产物正确性已在 Windows 各工具链组合、Linux x64 Release（默认 SDK、`--with-cef`、`--with-examples`）与 FreeBSD x64 Release（默认 SDK、`--with-examples`，不支持 CEF）上实测（2026-10-25），macOS 仍待实测（见第五节缺口 2）。

## 七、相关文档

* [发布与打包](../Release.md) —— 面向使用者的完整操作说明
* [编译流程与脚本说明](../../build/README.md)
* [开发计划](Development-Plan.md) —— B-3 条目及整体路线图
* [项目审计报告](Project-Audit.md) —— B-3 的原始审计结论
