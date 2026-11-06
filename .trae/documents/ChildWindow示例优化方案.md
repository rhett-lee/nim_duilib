# ChildWindow 示例优化方案

## Context

`examples/ChildWindow` 是 nim_duilib 中演示「系统原生子窗口嵌入 + 应用层自行绘制」的示例。当前存在两类问题：

1. **界面观感**：窗口尺寸/阴影与其它示例不一致；内容区直接用 `bg_container` 铺满，没有卡片层次；每个子窗口顶部信息行字号、颜色、间距混乱；子窗口内部的演示画面是纯黑/纯蓝底 + 随机 RGB 图形，噪声感强。
2. **代码健壮性**：子窗口创建失败时仍保存事件对象；调度查找「未找到目标窗口」时跳过第 0 项；关闭后仍保留 `m_pChildWindow` 裸指针；全屏退出回调未解绑；FPS 暂停/恢复不清零；Win 端 GDI 位图重创建时先删后选导致资源泄漏；SDL 端圆形填充实际画的是三角形描边。

目标：在不改变示例教学目的（展示嵌入子窗口与刷帧）的前提下，统一视觉规范、把演示画面重做为深色主题的流动波形动画，并修复上述确定性问题。

## 视觉设计

- 深蓝黑渐变背景（`#0D121A → #161E2A`），叠加极淡网格（`#1A2432`）。
- 4 条正弦波形，使用主题强调蓝邻近色（`#0078D4` / `#409CFF` / `#00B7C3` / `#8B7CFF`），每像素 2px 主色 + 外层暗色描边模拟辉光。
- 每条波的频率、振幅、相位不同；相位由 `ChildWindowPaint` 构造时传入的子窗口序号错开，避免 9 个面板同步摆动。
- 动画进度使用 `std::chrono::steady_clock` 计算，与绘制帧率解耦；暂停绘制时固定显示相位 0 的静止帧（不再显示纯蓝底）。
- 两种后端（Win GDI / SDL3）绘制同一套视觉；波形使用一次 `Polyline` / `SDL_RenderLines` 批量提交，避免逐段调用。

## 实施步骤

### 1. C++ 逻辑（examples/ChildWindow）

- [MainForm.cpp](e:\develop\nim_duilib\examples\ChildWindow\MainForm.cpp)
  - `CreateChildWindows()`：检查 `CreateChildWindow()` 返回值，失败时 `delete` 事件对象并跳过；已创建过则直接返回，避免重复创建。
  - `CloseChildWindows()`：结束后置空 `m_pChildWindow`；删除无意义的 `pChildWindowEvents = nullptr;`。
  - `DoPaintNextChildWindow()`：用 `bFound` 标记修正「目标不在列表中」时的起始下标（应回绕到索引 0，而不是从 1 开始）；删除重复的空指针判断。
- [MyChildWindowEvents.h](e:\develop\nim_duilib\examples\ChildWindow\MyChildWindowEvents.h) / [MyChildWindowEvents.cpp](e:\develop\nim_duilib\examples\ChildWindow\MyChildWindowEvents.cpp)
  - 构造函数对 `pChildWindow == nullptr` 直接返回；`AttachWindowExitFullscreenMsg` 传入以 `this` 为值的 `EventCallbackID`，析构时调用 `Window::DetachWindowEventCallbackByID()` 解绑。
  - `FPSCounter` 增加 `Reset()`，在「绘制」勾选时重置计数器与 `m_lastPaintTime`，避免恢复后长时间显示 0。
  - 构造 `ChildWindowPaint` 时传入子窗口序号，用于波形相位错开。
- [ChildWindowPaint.h](e:\develop\nim_duilib\examples\ChildWindow\ChildWindowPaint.h)
  - 构造函数增加 `size_t nChildWindowIndex` 参数并保存。
- [ChildWindowPaint_Win.cpp](e:\develop\nim_duilib\examples\ChildWindow\ChildWindowPaint_Win.cpp)
  - 删除随机图形/位图缓存实现（`CacheBitmap`、`CreateCacheBitmap`、`DrawFromCache`、`MAX_CACHE_BITMAP_COUNT`、`ShapeType`），改为 `PaintScene()` 程序化绘制。
  - 修复 `GetPaintDC()`：位图尺寸变化时先把原对象选回 DC、置空 `m_hOldObj`，再 `DeleteObject`，避免删除仍被选中的位图。
  - 静态状态改为绘制相位 0 的静止帧，移除纯蓝 `FillRect`。
  - 普通窗口与分层窗口都复用同一 `PaintScene()`。
- [ChildWindowPaint_SDL.cpp](e:\develop\nim_duilib\examples\ChildWindow\ChildWindowPaint_SDL.cpp)
  - 删除随机图形与错误的 `RenderCircle`，改为与 Win 端一致的 `PaintScene()`；开启 `SDL_BLENDMODE_BLEND`，用 `SDL_RenderLine` 批量绘制波形，静态状态画相位 0 帧。

### 2. 界面皮肤（bin/resources）

- [child_window.xml](e:\develop\nim_duilib\bin\resources\themes\default\child_window\child_window.xml)
  - Window：`size="80%,85%"`、`min_size="720,480"`、`use_system_caption="false"`、`shadow_type="default"`、`shadow_attached="true"`，标题栏结构保持现有框架约定。
  - 本地新增 `panel_card` Class（`bg_window_card` + 1px `border_split_level1` + `border_round="10,10"`）。
  - 内容区改为 `padding="16,12,16,16"`、`child_margin="12"`，背景 `bg_window_main`；增加一行紧凑说明卡片（Logo + `system_bold_14` 标题 + `system_regular_12` 灰色说明）。
  - GridBox 改为 `bkcolor="bg_window_main"`、`child_margin="12"`，去掉无效的 `valign="center"`。
- [child_window_item.xml](e:\develop\nim_duilib\bin\resources\themes\default\child_window\child_window_item.xml)
  - 子窗口根节点加 `bkcolor="bg_window_card" border_size="1" border_color="border_split_level1" border_round="10,10"`，形成卡片外框。
  - 顶部信息区改为两行、总高 64（`child_window_margin="12,64,12,12"`）：
    - 第一行：`system_bold_14` 子窗口名 + 右侧 `text_muted` 的 FPS 文案 + `color_accent`、右对齐的 FPS 数值（宽度 42）。
    - 第二行：两个 `checkbox_1` 复选框，间距 12，避免窄单元格下横向溢出。
  - 所有说明文字 `mouse_enabled="false"`，字体/颜色全部使用语义 token。

### 3. 语言键

- [zh_CN-examples.txt](e:\develop\nim_duilib\bin\resources\lang\zh_CN-examples.txt) / [en_US-examples.txt](e:\develop\nim_duilib\bin\resources\lang\en_US-examples.txt) 成对新增：
  - `STRID_CHILDWINDOW_HEADLINE`：子窗口绘制演示 / Child Window Rendering Demo
  - `STRID_CHILDWINDOW_DESC`：说明示例用途与勾选「绘制」查看刷帧性能 / 对应英文

## 验证

1. XML：用 PowerShell `[xml]` 载入两个皮肤文件校验格式；全局检查新增属性均为框架支持项（`bkcolor`/`border_size`/`border_color`/`border_round`/`child_window_margin`）。
2. 资源引用：确认两个新语言键在中英文文件中都存在且无重复；`text_id` 全部可解析。
3. 编译：若存在已配置的 CMake 构建目录，执行 `cmake --build <build> --target ChildWindow`；至少对 `ChildWindowPaint_Win.cpp`、`ChildWindowPaint_SDL.cpp`、`MyChildWindowEvents.cpp`、`MainForm.cpp` 做单文件编译检查。
4. 运行：启动程序，确认 3×3 卡片显示、波形流动、勾选/取消「绘制」时动画暂停并显示静止帧、FPS 数值在恢复后正常刷新、全屏切换/关闭窗口无异常。
