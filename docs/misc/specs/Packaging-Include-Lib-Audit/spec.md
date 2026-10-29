# SDK include/lib 完整性与冗余审计（四平台）

## 1. 背景与目标

`build/pack.py` 已完成过多轮头文件裁剪。本次针对 Windows、macOS、Linux、FreeBSD
四个平台重新核对：

1. `include/` 是否完整——从 `duilib.h` 出发的 `#include "duilib/..."` 闭包是否全部命中；
2. `lib/` 是否完整——CMake / VS 链接清单需要的库是否全部随包；
3. 是否存在冗余——平台无关的库、实现头、重复 loader、符号链接重复等。

结论：**闭包完整，库清单与 CMake/VS 一致；但仍有 17 个仅被 `.cpp` 使用的实现头
被随包，另有 3 处打包缺口需要修正。**

## 2. 审计证据

### 2.1 头文件闭包（当前实现）

用 `collect_headers()` 模拟各组合并扫描包内所有 `#include "duilib/..."`：

| 组合 | 头文件数 | third_party | 未解析包含 |
| --- | --- | --- | --- |
| Windows 默认 | 262 | 1（WebView2.h） | 1：`ControlDragable.h -> DragWindowFilter_SDL.h`（`DUILIB_BUILD_FOR_SDL` 未定义，不编译） |
| Windows + `--with-sdl` | 263 | 1 | 0 |
| Windows + `--with-cef` | 287 | 1 | 1（同上） |
| Unix（Linux/macOS/FreeBSD） | 255 | 0 | 0 |
| Unix + CEF（模拟） | 280 | 0 | 0 |

结论：宏组合下闭包完整。唯一的文本缺口是 Windows 非 SDL 包中的 SDL 条件包含，
该分支在默认宏下不参与编译，属预期行为（下游自行定义 `DUILIB_SDL=1` 时才会触发，
此时也缺少 SDL3 库，应在文档中提醒）。

### 2.2 库清单（当前实现）

| 平台 | CMake / VS 需要 | `pack.py` 实际收集 | 结论 |
| --- | --- | --- | --- |
| Windows | 5 个基础库 + Skia 6 直接库 + 4 传递库 + WebView2 loader + turbojpeg（VS 默认开启） | 5 + 10 + 2 + 1 | 完整 |
| Linux | 5 + Skia 10 + SDL3 动态库 + 系统库（X11/freetype/fontconfig） | 5 + 10 + SDL3 | 完整 |
| macOS | 5 + Skia 10 + SDL3 dylib | 5 + 10 + SDL3 | 完整 |
| FreeBSD | 5 + Skia 10 + SDL3 + 系统库（X11/freetype/fontconfig） | 5 + 10 + SDL3 | 完整 |
| Windows + CEF | `libcef.lib` + `libcef_dll_wrapper.lib` + CEF 头 | 已随包（`latest`/`109` 二选一） | 完整（运行库按文档外置） |

`LIB_BASENAMES` 与 `cmake/duilib_common.cmake:114` 的 `DUILIB_LIBS`、`msvc/PropertySheets/
CommonSettings.props:87` 的 `DuilibThirdLibs` 完全一致（`duilib-cximage/png/webp/zlib`）。
Skia 目录当前实际内容就是 6 个直接库 + 4 个传递依赖，无多余库。

### 2.3 发现的冗余

以下头文件在整个仓库中**只被 `.cpp` 包含，没有任何 `.h` 包含它们**，属于实现细节，
与已排除的 `Core/WindowDropTarget_*` 同类，但目前仍随包（Windows 全带，Unix 带对应平台项）：

```
Core/SDL_Linux.h
Core/MessageLoop_SDL.h
Core/MessageLoop_Windows.h
Utils/ProcessSingleton_Linux.h
Utils/ProcessSingleton_MacOS.h
Utils/ProcessSingleton_Windows.h
Utils/BitmapHelper_SDL.h
Utils/BitmapHelper_Windows.h
Utils/ApiWrapper_Windows.h
Utils/Macros_Windows.h
Utils/DiskUtils_Windows.h
Utils/DllManager_Windows.h
Utils/InlineHook_Windows.h
Control/RichEditHost_Windows.h
Control/RichEditDragSource_Windows.h
Control/RichEditDropTarget_Windows.h
Control/RichEditDropTarget_SDL.h
```

排除后预期头文件数（已用脚本预演，闭包仍为 0 缺失）：

| 组合 | 排除后 |
| --- | --- |
| Windows 默认 | 245 |
| Windows + `--with-sdl` | 246 |
| Windows + `--with-cef` | 270 |
| Unix | 238 |
| Unix + CEF（模拟） | 263 |

其他冗余（保留，不修改）：

- WebView2 同时随包 `WebView2LoaderStatic.lib` 与 `WebView2Loader.dll.lib`：
  VS 默认静态 loader，CMake 使用动态 loader，两种构建体系并存，二者都必要；
- `turbojpeg-static.lib`：仅 VS 构建启用（`JpegTurboEnabled=1`），CMake 不使用；
- Unix SDL3 的 `libSDL3.so` / `.so.0` / 实文件（或 macOS dylib）三者为链接名、
  SONAME、实文件的正常组合；tgz 保留符号链接，zip 会展开为多份副本（仅体积冗余）。

### 2.4 发现的缺口

1. **WebView2Loader.dll 未随 SDK 包**：`collect_webview2_loader()` 只收集两个 `.lib`，
   `WebView2Loader.dll` 仅在有 `--with-examples` 时经 `collect_examples()` 放入 `bin/`。
   SDK-only 包的 MANIFEST 却提示"从 bin/ 或 third_party 复制"，这两处在该包内不存在。
   CMake 构建体系（`DUILIB_USE_WEBVIEW2_LOADER_DLL=1`）运行期需要该 DLL。
2. **基础库缺失只告警**：`collect_libs()` 仅对 `duilib` 抛错，`duilib-cximage/png/webp/zlib`
   缺失只 `warn`，会产出链接必然失败的 SDK 包。
3. **非 Windows CEF 无法打包**：`collect_cef()` 对非 Windows 直接抛错，但 CMake
   （`duilib_common.cmake:143-159`）支持 Linux（`libcef.so + cef_dll_wrapper + X11`）
   与 macOS（`cef_dll_wrapper`），仓库也带了 `libcef_linux` / `libcef_macos`
   （仅 wrapper 源码与头，无 CEF 二进制）。本次**不处理**：需要额外约定 CEF
   二进制分发包路径与 `.a` 输出位置，超出本次审计范围。

### 2.5 已知但不修改的风险

- `duilib_config.h` 在 Windows / Linux / macOS 默认定义 `DUILIB_BUILD_FOR_CEF`，
  而 CEF 头默认不随包；下游若用该宏启用 CEF，需要 `--with-cef` 包（且目前仅 Windows）。
- Windows 未带 `--with-sdl` 的包不含 `DragWindowFilter_SDL.h`；下游自行定义
  `DUILIB_SDL=1` 会编译失败，需同时使用 `--with-sdl` 包。
- Windows arm64：`third_party` 只有 x64/x86 的 WebView2 loader 与 libjpeg-turbo，
  `pack.py` 会告警并跳过，arm64 SDK 不完整（VS 工程仅 x64/Win32，暂不影响）。

## 3. 变更方案（待批准后实施）

### 3.1 `build/pack.py`

1. `INTERNAL_HEADER_FILES`（约 111-119 行）追加 2.3 节列出的 17 项。
   `SDL_HEADER_FILES` / `IMAGE_DECODER_HEADER_FILES` 保持不变。
2. `collect_webview2_loader()`（约 582-599 行）：
   - 在收集两个 `.lib` 后，把 `third_party/.../native/<x64|x86>/WebView2Loader.dll`
     以 `bin/WebView2Loader.dll` 加入包；
   - 缺失时沿用现有 `warn`。
3. `collect_libs()`（约 449-465 行）：`LIB_BASENAMES` 中任意库缺失改为 `PackageError`，
   错误文案列出缺失库与库目录；CEF wrapper 的判断保持不变。
4. `write_manifest()`（约 834-845 行）：
   - 运行期说明改为"`bin/WebView2Loader.dll` 已随包"；
   - 检测 `bin/WebView2Loader.dll` 是否在 `collector.items` 中，决定是否输出该行。

### 3.2 文档同步

- `docs/Release.md` / `docs/Release.en.md`：内部实现头排除清单追加 2.3 节 17 项。
- `docs/misc/Release-Packaging-Notes.md` / `.en.md`：验证数字更新为
  245 / 246 / 238（Windows / Windows+SDL / Unix），并记录本次排除项。
- `build/README.md` / `build/README.en.md`：补充"WebView2Loader.dll 随包"与
  "基础库缺失即失败"。

## 4. 验收

1. `python3 -m py_compile build/pack.py` 通过。
2. 闭包脚本：Windows 245（唯一条件包含）、Windows+SDL 246（0 缺失）、
   Unix 238（0 缺失）、Windows+CEF 270、Unix+CEF 263。
3. `python3 build/pack.py --config Debug --list`：
   - 头文件 245；
   - 基础库 5、Skia 10、WebView2 3（2 个 `.lib` + 1 个 DLL）、JPEG 1；
   - `bin/WebView2Loader.dll` 出现在清单中；
   - 无新增告警（CEF 未启用时仍提示未打包 CEF 库）。
4. 实际打包一次（`--config Debug --out-dir <临时目录>`），解压后抽查：
   `include/duilib/Core/MessageLoop_Windows.h` 不存在，
   `bin/WebView2Loader.dll` 存在，MANIFEST 运行期文案正确。
5. 本次不改动 `duilib/` 源码，Debug 编译行为不变；安装包可正常执行。

## 5. 不在本次范围

- Linux / macOS CEF 打包（需 CEF 二进制分发路径约定）。
- Windows arm64 的 WebView2 loader / libjpeg-turbo 预编译库。
- SDL3 zip 符号链接展开导致的体积冗余。
