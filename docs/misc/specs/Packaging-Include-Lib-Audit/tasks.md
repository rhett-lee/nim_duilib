# 任务清单：SDK include/lib 审计修订

依赖：[spec.md](spec.md)。用户批准后按顺序执行。

## T1 扩展内部实现头排除清单

- [x] `build/pack.py` 的 `INTERNAL_HEADER_FILES` 追加 17 项：
  - `Core/SDL_Linux.h`、`Core/MessageLoop_SDL.h`、`Core/MessageLoop_Windows.h`
  - `Utils/ProcessSingleton_Linux.h`、`Utils/ProcessSingleton_MacOS.h`、`Utils/ProcessSingleton_Windows.h`
  - `Utils/BitmapHelper_SDL.h`、`Utils/BitmapHelper_Windows.h`
  - `Utils/ApiWrapper_Windows.h`、`Utils/Macros_Windows.h`
  - `Utils/DiskUtils_Windows.h`、`Utils/DllManager_Windows.h`、`Utils/InlineHook_Windows.h`
  - `Control/RichEditHost_Windows.h`、`Control/RichEditDragSource_Windows.h`
  - `Control/RichEditDropTarget_Windows.h`、`Control/RichEditDropTarget_SDL.h`
- [x] 更新文件头设计约束注释，说明这些头只被 `.cpp` 使用。

## T2 WebView2Loader.dll 随 SDK 包

- [x] `collect_webview2_loader()` 追加 `bin/WebView2Loader.dll`（来源
      `third_party/Microsoft.Web.WebView2/build/native/<x64|x86>/`）。
- [x] `write_manifest()` 运行期说明改为指向包内 `bin/`。

## T3 基础库缺失改为失败

- [x] `collect_libs()` 对 `LIB_BASENAMES` 任意缺失抛 `PackageError`，文案列缺失库与目录。

## T4 文档同步

- [x] `docs/Release.md` / `docs/Release.en.md` 排除清单追加 17 项。
- [x] `docs/misc/Release-Packaging-Notes.md` / `.en.md` 数字改为 245 / 246 / 238。
- [x] `build/README.md` / `build/README.en.md` 补充 loader DLL 与失败策略。

## T5 验证

- [x] `python3 -m py_compile build/pack.py`
- [x] 闭包预演：Windows 245、Windows+SDL 246、Windows+CEF 270、Unix 238、Unix+CEF 263；
      除 Windows 非 SDL 的 SDL 条件包含外 0 缺失。
- [x] `python3 build/pack.py --config Debug --list` 核对头文件数、库数、`bin/WebView2Loader.dll`。
- [x] 实际打包到临时目录并解压抽查文件存在性与 MANIFEST 文案。
