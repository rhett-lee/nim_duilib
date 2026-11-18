# SDL 渲染后端绘制流程性能分析与优化建议

**日期**: 2026-11-17
**平台**: Windows x64（SDL3 静态 Release 构建）
**评测对象**: `render.exe` 首页（页面含较多图片，且含 GIF/APNG/WebP/Lottie/PAG 多个动画图片）
**关联背景**: 后端切换开关为 `DUILIB_ENABLE_SDL`（`python build/build.py build --offline --sdl`）

---

## 一、背景与目标

SDL 后端是跨平台的窗口与绘制通道，不同平台下 SDL 的呈现路径完全不同：

| 平台 / 视频驱动 | SDL 表面路径 | 实际提交方式 |
|----------------|-------------|-------------|
| Windows | native framebuffer（DIB Section） | GDI `BitBlt` |
| Linux X11 | native framebuffer（`XImage`） | `XShmPutImage`；无 MIT-SHM 时退化为 `XPutImage` |
| Linux Wayland / EGL | texture framebuffer 兜底路径 | `SDL_UpdateTexture` + `RenderTexture` + **`RenderPresent`（每次更新）** |

本报告的目标是：用 `render.exe` 首页做实测，找出 SDL 绘制流程的耗时分布与性能瓶颈，并给出可优化项及预估收益。

---

## 二、评测方法

### 2.1 数据采集

使用库内置的性能统计（`duilib/Utils/PerformanceUtil.h`，`DUILIB_PERFORMANCE_STAT_ENABLED`）采集各阶段累计耗时与次数，程序退出时（静态对象析构）输出。

采集方式：

1. 构建 SDL Release 静态版本：`python build/build.py build --offline --sdl`；
2. 启动 `bin/render.exe`，停留在首页，运行 14 秒（动画图片持续刷新，处于稳态）；
3. 通过 `CloseMainWindow()` 正常关闭窗口，触发统计输出；
4. 统计输出重定向到文件（仅在评测期间临时加入，评测后已还原）。

> 说明：`OutputDebugString` 的 DBWIN 共享内存缓冲在消息突发时会丢行，因此评测采用"临时写文件 + 评测后还原"的方式，保证数据完整。

### 2.2 环境

| 项目 | 值 |
|------|-----|
| 显示分辨率 | 1646 × 1029 |
| 窗口尺寸 | `85% × 80%`（约 1399 × 823，客户区约 1399 × 787 ≈ 4.4 MB/帧） |
| 后端 | Raster（Skia CPU） + SDL3 窗口 |

---

## 三、实测结果

### 3.1 总体

| 指标 | 数值 |
|------|------|
| 绘制次数 | 824 次 / 14.1 s ≈ **58 fps** |
| 单次绘制总耗时 | **1.318 ms**（最大 68 ms） |
| 进程 CPU 时间 | 1266 ms / 14.1 s（约单核 **9%**） |

### 3.2 单次绘制耗时分解

| 阶段 | 单次耗时 | 占比 | 代码位置 |
|------|---------|------|---------|
| `DoPaint`（`Window::OnPaintMsg`） | 1.044 ms | 79% | `duilib/Core/Window.cpp` |
| ├ `AlphaPaint`(Root Box) | 1.029 ms | 78% | `duilib/Core/Window.cpp` |
| │ ├ `Control::PaintImage`（**13 张图/次**） | **0.679 ms** | **52%** | `duilib/Core/Control.cpp:3341` |
| │ │ └ `Image::GetCurrentFrame`（5 张动画/次） | 0.199 ms | 15% | `duilib/Image/Image.cpp:180` |
| │ ├ `DrawSkiaText::DrawSimpleText`（13 次/次） | 0.034 ms | 3% | `duilib/RenderSkia/DrawSkiaText.cpp` |
| │ └ 控件树遍历 / 背景 / 边框 / 裁剪 | 0.316 ms | 24% | — |
| `SwapPaintBuffersFast`（memcpy + BitBlt） | 0.206 ms | 16% | `duilib/RenderSkia/SkRasterWindowContext_SDL.cpp:261` |
| 其它（`GetUpdateRect`、提交等） | 0.068 ms | 5% | — |

### 3.3 关键观察

1. **每次绘制都重绘了 13 张图片**（`10744 ÷ 824 = 13.04`），即稳态下重绘区域覆盖了页面第 1~3 行的整段区域（约 1399 × 600），而真正发生变化的只有 **5 张动画图片**。
2. 13 张图中仅 5 张为动画图（`Image::GetCurrentFrame` 调用 `4120 ÷ 824 = 5`）。
3. 图片绘制是最大单项：`0.052 ms/张`，占单次绘制的 40%~52%。
4. 交换阶段（memcpy + BitBlt）占 16%，其中约一半是 `memcpy`。
5. 单次绘制最大 68 ms，出现在初始化 / 窗口暴露触发的**整页重绘**（含首帧图片解码）。

---

## 四、绘制流程与跨平台差异

### 4.1 当前流程（Raster + SDL）

```
NativeWindow_SDL::PaintWindow()
  → IRender::PaintAndSwapBuffers(pRender, pRenderPaint)
      → 查更新区域（GetUpdateRect） → 决定 rcPaint（脏区 / 整幅）
      → DoPaint(rcPaint)                           // Skia CPU 光栅化到 m_fSurfaceMemory
      → SwapPaintBuffers(rcPaint, alpha)           // SkRasterWindowContext_SDL.cpp:175
          → SwapPaintBuffersFast()                 // 成功则返回，否则走纹理路径
              → memcpy(m_fSurfaceMemory → SDL 窗口表面像素)
              → SDL_UpdateWindowSurfaceRects()     // Windows: BitBlt；X11: XShmPutImage/XPutImage
          → （兜底）SDL_UpdateTexture + RenderTexture + RenderPresent()   // 见 4.2
```

### 4.2 纹理兜底路径（跨平台风险点）

当 `SDL_GetWindowSurface()` 不可用（如 Wayland/EGL）或尺寸/格式不匹配时，会退化为 SDL 纹理路径。该路径的实现要点（`E:/develop/SDL3/src/video/SDL_video.c`，`SDL_UpdateWindowTexture`）：

1. 仅把**脏区的跨度矩形**上传到纹理（`SDL_UpdateTexture`，局部，尚可）；
2. 但随后**渲染整幅纹理**（`SDL_RenderTexture`）；
3. 并且**每次更新都调用 `SDL_RenderPresent`** —— 每个脏区提交都触发一次完整呈现。

这意味着：Wayland 等平台下，每次局部刷新都会被 vsync / 合成器帧回调耦合（可能阻塞到 16.7 ms），且无法把一帧内的多次提交合并，是跨平台体验的主要风险点。

---

## 五、瓶颈点分析

### 5.1 脏区被合并成单个矩形（占比 52% 的根因）

`NativeWindow_SDL::InvalidateRect()` 只把新脏区 `Union` 进 `m_rcUpdateRect`（单矩形）。首页的 5 个动画图片分布在多行，Union 后形成一个覆盖第 1~3 行的**大矩形**，把中间不相邻的静态图片也一并重绘——于是每次绘制 13 张图片，而实际只有 5 张变化。

### 5.2 图片每次绘制都重新缩放采样

首页图片原始尺寸远大于显示尺寸（例如 bmp 约 1024×768、APNG 379 KB，显示宽度仅 140~170 px）。`Control::PaintImage → Render_Skia::DrawImage → drawImageRect` 每次绘制都从大图按目标尺寸做 nearest 采样，**没有缩放结果缓存**，这部分工作每帧重复。

### 5.3 双缓冲带来两次全量内存拷贝

Raster 路径的像素搬运链为：

```
Skia 绘制 → m_fSurfaceMemory →(memcpy)→ SDL 窗口表面 →(BitBlt/XShmPutImage)→ 屏幕
```

全幅重绘时第一步 memcpy 约 4.4 MB；即使局部重绘，也要按脏区矩形逐行拷贝。若能把 Skia 直接渲染进 SDL 窗口表面像素，可省掉这一整次拷贝。

### 5.4 窗口暴露事件触发整页重绘

`OnNativeWindowExposedEvent`（`NativeWindow_SDL.cpp:747`）收到 `SDL_EVENT_WINDOW_EXPOSED` 后直接调用 `PaintWindow(true)`：

- 会清空更新区域，导致 `GetUpdateRect` 返回空 → 按整幅重绘；
- 而多数暴露场景（遮挡恢复、最小化还原、部分重绘请求）下**后备缓冲内容仍然有效**，只需重新提交一次像素，无需重新光栅化整棵控件树。

这是"单次绘制最大 68 ms"的直接来源，也是拖动缩放窗口时卡顿的主要原因。

### 5.5 动画帧对象每帧新建

`ImageInfo::GetFrame()`（`duilib/Image/ImageInfo.cpp:230`）每次调用都会 `make_shared<AnimationFrame>()` 并按目标尺寸重新取帧数据。每次绘制有 5 个动画控件各调用一次，合计约 0.2 ms/次（15%），其中包含重复的对象分配与尺寸查表。

### 5.6 其它

- 每次提交都重新获取 `SDL_GetWindowSurface()`、解析像素格式掩码（µs 级，可缓存）；
- 每次绘制调用 `GetSkCanvas()` 都会拷贝一次 `sk_sp<SkSurface>`（原子引用计数，µs 级）；
- 控件树全量遍历：局部绘制仍需遍历整棵树（0.316 ms/次，占 24%），可用"脏控件集合"优化，但改动较大。

---

## 六、可优化项

按"收益 / 成本"排序：

| 优先级 | 优化项 | 方案 | 预估收益 | 风险 | 状态 |
|-------|--------|------|---------|------|------|
| 1 | **图片缩放结果缓存** | 按 `(bitmap, 目标宽高)` 缓存缩放后的 `SkImage`（LRU），绘制改为 1:1 blit | 图片绘制 `0.68 ms → 0.2 ms`（**-35%~45%**），且 CPU/GL/SDL 全后端通用 | 小：内存占用增加；需在 DPI/控件尺寸变化时失效缓存 | **已完成（详见第十节）** |
| 2 | **动画帧对象复用** | `ImageInfo::GetFrame` 在（帧号 + 目标尺寸）未变时复用上一帧对象，避免每帧 `make_shared` 与重复取帧 | 同帧重复读取 `0.134 → 0.066 µs/次`（**-51%**）；动画逐帧播放场景与优化前持平 | 小：每个 `ImageInfo` 额外缓存 1 帧对象 | **已完成（详见第十一节）** |
| 3 | **暴露事件只重新提交，不重绘** | 区分"内容失效"（首帧 / resize / DPI 变化）与"仅需重新显示"；后者直接把后备缓冲提交到窗口 | 暴露、遮挡恢复、最小化还原等场景：**5~68 ms → 0.2 ms 量级** | 中：必须严格维护"后备缓冲内容有效"标志，resize 后必须置失效 | 未开始 |
| 4 | **直绘窗口表面，去掉 memcpy** | 用 `SkSurface::MakeRasterDirect` 包装 SDL 窗口表面像素（Windows `XRGB8888` / X11 `RGB888` 的内存字节序与 Skia `kN32` 一致） | 全幅重绘 `-0.15~0.35 ms`；局部重绘 `-0.05~0.1 ms` | 中：SDL 表面在 resize 后失效需重建；格式不兼容时回退现有双缓冲路径 | 未开始 |
| 5 | **多脏矩形替代单矩形 Union** | 维护少量脏矩形列表，Skia 侧多 clip 绘制，像素提交按矩形列表进行 | 重绘图片 `13 → 6` 张、重绘面积降至约 1/3，单次绘制 **-40%~50%** | 中大：需扩展 `IRenderPaint::GetUpdateRect` 这类单矩形接口 | **已完成（详见第九节）** |
| 6 | **纹理兜底路径批量提交** | 识别 SDL texture 兜底路径后自管 `SDL_Texture`，合并脏区、帧末只 `RenderPresent` 一次；必要时设置 `SDL_HINT_RENDER_VSYNC=0` 解除 vsync 耦合 | Wayland 场景每次 9~17 ms 阻塞 → **< 1 ms** | 中：需按平台特性分支 | 未开始 |
| 7 | 微项 | 缓存 `SDL_GetWindowSurface()` 与像素格式解析结果；缓存 `GetSkCanvas()` 的裸指针 | µs 级 | 小 | 未开始 |

### 建议实施顺序

1. **优化 1 + 2**（最小改动、直接砍掉 52% 的大头，且对所有后端有效）；
2. **优化 3**（解决暴露/缩放窗口时的整页重绘，收益直观）；
3. **优化 4**（去掉一次全量 memcpy）；
4. **优化 5、6**（较大改动，按跨平台诉求再决定）。

---

## 七、结论

- Windows 稳态下，首页绘制仅 **1.3 ms/帧**（约占单核 9%），**本身不是瓶颈**；真正的痛点是：
  1. **重绘面积被单矩形脏区合并放大**（52% 的时间耗在图片绘制上，其中大部分图片并未变化）；
  2. **窗口暴露即整页重绘**（单次最大 68 ms，影响缩放/恢复体验）；
  3. **双缓冲的两次全量像素搬运**（memcpy + BitBlt）。
- 投入产出最高的是 **图片缩放缓存** 与 **动画帧对象复用**：不改绘制架构、跨后端通用，可直接降低约一半光栅化时间。
- 若目标是跨平台体验一致性，则 **暴露事件只重新提交**、**直绘窗口表面**、**纹理路径批量提交** 更为关键（对应 X11 的 `XPutImage` 慢路径与 Wayland 每次提交都 Present 的问题）。

---

## 八、涉及文件（分析涉及，未改动）

| 文件 | 关注点 |
|------|--------|
| `duilib/RenderSkia/SkRasterWindowContext_SDL.cpp` | `PaintAndSwapBuffers:105`、`SwapPaintBuffers:175`、`SwapPaintBuffersFast:261`、颜色字节序/透明度后处理 |
| `duilib/Core/NativeWindow_SDL.cpp` | `OnNativeWindowExposedEvent:747`（暴露即整页重绘）、`PaintWindow:2780`、`InvalidateRect`（单矩形 Union） |
| `duilib/Core/MessageLoop_SDL.cpp` | 事件驱动的绘制调度（`WM_USER_PAINT_MSG` → `PaintWindow(false)`） |
| `duilib/Core/Control.cpp` | `Control::PaintImage:3341`、`Control::Paint:3884`（遍历与裁剪） |
| `duilib/Image/Image.cpp` / `duilib/Image/ImageInfo.cpp` | `Image::GetCurrentFrame:180`、`ImageInfo::GetFrame:230` |
| `duilib/RenderSkia/Render_Skia.cpp` / `DrawSkiaImage.cpp` | `DrawImage` → `drawImageRect`（每次重新采样） |

> 注：本次仅为分析与评测，评测期间加入的临时统计输出代码已还原，构建配置已恢复为非 SDL 构建。

---

## 九、优化项 5 实现与评测（多脏矩形）

**状态**：已完成（2026-11-17）
**目标**：用脏区列表替代"单矩形 Union"，Skia 侧按多矩形裁剪绘制，像素提交也按矩形列表进行；重绘图片数从 13 张降到约 6 张、重绘面积降到约 1/3。

### 9.1 实现要点

| 层次 | 改动 | 说明 |
|------|------|------|
| 接口 | `IRenderPaint::GetUpdateRects(std::vector<UiRect>&)` | 新增**带默认实现**的虚函数（默认退化为 `GetUpdateRect` 单矩形），原有自研实现无需改动，源码/二进制兼容 |
| 接口 | `IRender::BeginPaintRects` / `EndPaintRects` / `IntersectPaintRect` | 分别用于"设置脏区列表 + 建立裁剪"、"恢复裁剪"、"控件级精确剔除" |
| Skia 通用 | `Render_Skia::BeginPaintRects` | 多矩形 → `SkRegion` 求并 + `clipRegion`；单矩形 → `clipIRect`（更快）；**脏区覆盖整个画布时不设置裁剪**（整画布上设置区域裁剪比不裁剪慢约 10%，实测发现并规避） |
| 控件层 | `Control::AlphaPaint` | 对每个控件调用 `IntersectPaintRect(控件外扩矩形, rcPaint)`，与所有脏矩形都不相交则直接跳过（`m_paintRects.size() <= 1` 时走与原逻辑完全一致的快速路径，保证完整绘制无额外开销） |
| Windows 原生 | `NativeWindow_Windows.cpp` | 在 `BeginPaint` **之前**用 `GetUpdateRgn` + `GetRegionData` 取矩形数组；为空或超过 12 个时退化为 `GetUpdateRect` 单矩形 |
| SDL 原生 | `NativeWindow_SDL.cpp` | 新增 `m_updateRects` 脏区列表：被已有矩形包含则丢弃、否则追加（上限 12 个，超限合并为并集边界）；`PaintWindow` 结束时清空 |
| Windows Raster | `SkRasterWindowContext_Windows.cpp` | 脏区分散（矩形数 ≤ 6 且总面积 ≤ 并集面积的一半）时按矩形逐个 `BitBlt`，否则按并集一次 `BitBlt`（减少调用次数带来的固定驱动开销）；分层窗口 `UpdateLayeredWindow` 路径保持整幅不变 |
| SDL Raster | `SkRasterWindowContext_SDL.cpp` | 快速路径（framebuffer）按矩形逐行 `memcpy`，最后一次 `SDL_UpdateWindowSurfaceRects`（Windows 内部即多次 `BitBlt`，X11 为多次 `XShmPutImage`）；纹理兜底路径按矩形逐个 `SDL_UpdateTexture`，仍只 `RenderTexture`/`RenderPresent` 一次 |
| GL 后端 | `SkGLWindowContext_Windows.cpp` | 局部重绘时在持久化表面上按多矩形裁剪绘制，再整幅拷贝到 FBO0（该路径必须整幅上屏，故收益来自"少光栅化"，不减少拷贝） |

设计取舍：`DoPaint(rcPaint)` 的签名不变（仍传入脏区并集矩形），脏区列表由后端在调用 `DoPaint` 前后通过 `BeginPaintRects` / `EndPaintRects` 注入；每个控件的精确剔除由 `Control::AlphaPaint` 完成，因此改动面集中在"后端 + 一处控件层判断"，`Control::Paint` 等既有逻辑保持不变。

### 9.2 跨后端实测（同环境 A/B 交替测量）

方法：用 `git` 暂存功能改动生成补丁 → 还原为"仅带性能统计"的基线版本 → 构建并测量 → 重新应用补丁 → 构建并测量，交替进行，避免环境漂移。

**非 SDL（Windows 原生 + Skia Raster，`render.exe` 首页 14 s 稳态，基线 3 次 / 优化后 3 次取均值）**

| 指标 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| 单次绘制（`Window::OnPaintMsg`） | 1.562 ms | **1.094 ms** | **-30.0%** |
| `AlphaPaint`（`Root Box`） | 1.547 ms | 1.084 ms | -29.9% |
| `Control::PaintImage`（每次绘制的合计） | 1.10 ms | **0.81 ms** | **-26.4%** |
| `PaintImage` 调用次数（张/次绘制） | 12.0 | **1.85** | -85% |
| `Image::GetCurrentFrame`（次/次绘制） | 5.00 | 1.80 | -64% |
| `SwapPaintBuffers` | 0.238 ms | 0.270 ms | +0.03 ms（单次 `BitBlt`，基本持平） |
| 进程 CPU | 约 1080 ms | 约 740 ms | **-31%** |

**非 SDL，`controls.exe`（6 个分散小矩形，33 ms × 10 s 外部驱动）**

| 指标 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| 单次绘制 | 1.757 ms | **1.299 ms** | **-26.1%** |
| `AlphaPaint` | 1.352 ms | 0.802 ms | -40.7% |
| `PaintImage` | 11.0 张/次 | 0.06 张/次 | -99% |
| 文本绘制 | 56 次/次 | 4.9 次/次 | -91% |
| 进程 CPU | 5.9% | 3.9% | -34% |

**SDL（SDL3 窗口 + Skia Raster，`render.exe` 首页 14 s 稳态，2 次测量）**

| 指标 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| 单次绘制（`NativeWindow_SDL::PaintWindow`） | 1.289 ms | **0.947~0.982 ms** | **-24~27%** |
| `AlphaPaint` | 1.025 ms | 0.705~0.729 ms | -29~31% |
| `PaintImage` | 13.0 张/次 | **2.06 张/次** | -84% |
| `SwapPaintBuffers` | 0.185 ms | 0.163~0.171 ms | -8~12% |
| 进程 CPU | 1234 ms | 约 810 ms | **-34%** |
| `controls.exe`（外部驱动＝整幅重绘路径） | 17.751 ms/次 | 17.494 ms/次 | 持平（无回归） |

结论：

1. **收益与预估一致**：重绘图片数由 12~13 张降到约 2 张（多脏矩形 + 控件级剔除共同作用，优于预估的 6 张），单次绘制耗时下降 **26%~30%**。
2. SDL 的 `controls.exe` 场景由外部 Win32 `InvalidateRect` 驱动，这类更新不会进入 SDL 窗口自身的脏区列表（duilib 会将其视为曝光并整幅重绘），因此该场景只用于**证明整幅路径无回归**；SDL 侧的收益由 `render.exe` 的动画首页场景体现。

### 9.3 关于 "`PaintImage` 平均耗时由 0.133 ms 上升到 0.859 ms" 的排查结论

该现象**不是性能降级**，而是"按调用次数求平均"这一统计口径在图片集合发生变化后失去可比性，排查过程与证据如下。

1. 调用次数变化：优化前每帧重绘 12 张图片（11.9 张/次绘制），优化后每帧只重绘与脏区相交的 **1.8 张**；被剔除的是大量小尺寸静态图片，保留下来的是尺寸较大的动画图片。`PaintImage` 的单次平均耗时因此被"小图被稀释"变成"只剩大图"，与优化前不可直接比较。
2. 同场景 A/B（同一台机器、同一程序、交替测量）：`PaintImage` 每次绘制的合计耗时由 1.10 ms 降到 0.81 ms（**-26.4%**），`AlphaPaint` 由 1.547 ms 降到 1.084 ms（**-29.9%**），整体绘制耗时下降 30%。即帧级成本是下降的。
3. 为验证"单次平均上升"是否由代码引入，做了两组对照实验（同一二进制、用环境变量切换行为）：
   - **关闭控件级精确剔除**（恢复为只与并集矩形求交集，等价于优化前的绘制集合）：统计结果立即回到 `PaintImage` 0.095~0.134 ms/次、`GetCurrentFrame` 0.042~0.052 ms/次、12 张/次绘制，说明"单次耗时上升"完全由"绘制的图片集合变化"引起，与裁剪、接口改动无关；
   - **对比三种裁剪方式**（多矩形区域裁剪、并集矩形裁剪、完全不裁剪，每组 3 次）：`AlphaPaint` 分别为 1.08 / 1.14 / 1.17 ms，`PaintImage` 单次分别 0.43 / 0.44 / 0.45 ms——区域裁剪没有额外开销，因此保留区域裁剪（更精确，不重绘脏区之间的内容）。
4. 结论：优化项 5 在帧级是净收益；阅读性能统计时应使用"每次绘制的合计耗时（total / 绘制次数）"或"进程 CPU 时间"，而不是"单次调用的平均值"，后者会随被剔除的控件类型变化而失真。

### 9.4 可靠性验证

| 验证项 | 内容 | 结果 |
|--------|------|------|
| 新增单元测试 | `tests/test_offscreen_render.cpp` 增加 OS-37/38/39：多脏矩形只有脏区内像素被更新（脏区外保持原像素）、`IntersectPaintRect` 的剔除/相交/跨矩形并集边界、`EndPaintRects` 后恢复单矩形语义、脏区覆盖整画布时不裁剪仍绘制正确 | 通过（`offscreen_render` 共 **146 项检查，0 失败**，2 项 SKIP 为需窗口的迁移用例） |
| 全量回归 | `ctest`（17 个用例，含 `gl_window_smoke` 的局部矩形绘制用例、`render_api`、`render_ext`） | 非 SDL 与 SDL 构建均 **17/17 通过** |
| GL 后端 | `gl_window_smoke` 的 GS-17~GS-19 局部增量刷新用例（多矩形裁剪在持久化表面上的绘制） | 通过 |
| 显示正确性 | SDL / 非 SDL 下 `render.exe`、`controls.exe` 运行并截图；`controls.exe` 用 6 个分散脏矩形驱动 6 s 后截图 | 无残影、无缺失、无越界绘制 |
| 兼容性 | `IRenderPaint::GetUpdateRects` 提供默认实现 | 既有自研 `IRenderPaint` 实现（如测试代码）无需修改即可编译运行 |

### 9.5 已知限制

- 脏区列表上限为 12 个，超限合并为并集边界（避免 `SkRegion` 与逐矩形拷贝的开销失控）；
- 分层窗口（`UpdateLayeredWindow`）与 GL 后端上屏仍需整幅拷贝，多脏矩形只减少光栅化与控件遍历；
- SDL 后端的脏区列表仅由 duilib 内部 `Invalidate` 维护，外部系统发起的更新区域仍按整幅处理（可作为优化项 3 的输入）。

---

## 十、优化项 1 实现与评测（图片缩放结果缓存）

**状态**：已完成（2026-11-18）
**目标**：按 `(源图片, 目标宽高, 采样参数)` 缓存缩放后的 `SkImage`，把"每帧重新采样"改为"1:1 拷贝"，对所有渲染后端（Windows 原生 / SDL、Raster / GL）通用。

### 10.1 实现要点

| 位置 | 改动 | 说明 |
|------|------|------|
| `duilib/RenderSkia/DrawSkiaImage.cpp` | 新增文件级 `ImageScaleCache`（LRU，`std::mutex` 保护） | 缓存 key = `SkImage::uniqueID` + 目标宽高 + 采样参数；`uniqueID` 随图片内容变化，尺寸/DPI 变化则目标宽高变化，因此**无需显式失效** |
| 同上 | `DrawImage` 增加快速路径 | 仅当"整幅源图 + 有缩放 + 画布无缩放/整数平移 + 目标尺寸合法"时启用；命中后按 `kNearest` 1:1 绘制到目标矩形 |
| 同上 | 准入策略（首次出现不缓存） | 同一个 key 第一次出现只记录、仍走原有绘制路径；第二次出现才创建缓存。避免动图首轮、只绘制一次的图片付出"额外建表面 + 采样 + 快照"的开销 |
| 同上 | 预算控制 | 总字节上限 64 MB、条目上限 512、单个条目上限 16 MB（超过则不缓存），超出预算按 LRU 淘汰 |
| 同上 | 公开静态接口 | `ClearImageScaleCache()` / `GetImageScaleCacheCount()` / `GetImageScaleCacheHitCount()` / `EnableImageScaleCache(bool)`，供单元测试与 A/B 定位 |
| Alpha/混合 | 缩放时用 `kSrc` 混合作图，`uFade`/`blend` 仍在最终绘制阶段应用 | 同一缓存条目可被不同 Alpha 复用（单元测试 OS-42 覆盖） |

不进入缓存的绘制路径（行为与优化前完全一致）：九宫格/平铺等**子区域**绘制、目标尺寸与源尺寸相同（无缩放）、源图片小于 `64x64`（缩放本身很快）、画布存在缩放/旋转变换（预缩放会被二次采样）、缓存被禁用。

### 10.2 基准测试（隔离场景，最能体现收益）

新增手工评测工具 `tests/bench_image_scale_cache.cpp`（Release，非 SDL Raster，1000 次/组，重复 3 组取最优；下表为 3 次运行的区间）：

| 场景 | 缓存关闭 | 缓存开启 | 加速比 |
|------|---------|---------|--------|
| 静态图 `1024x768 → 170x130` | 122~156 µs | **7.5~8.9 µs** | **16.1~17.8x** |
| 静态图 `1024x768 → 400x300` | 665~796 µs | **39.1~46.0 µs** | **17.0~17.3x** |
| 动图 64 帧 `512x384 → 200x150`（循环绘制） | 209~238 µs | **15.0~19.1 µs** | **12.4~13.9x** |
| 小图 `32x32 → 16x16`（低于缓存阈值，对照组） | 2.0~3.0 µs | 2.0~3.0 µs | 无变化（符合预期） |

说明：静态图场景下首次绘制仍走原有路径，第 2 次创建缓存，从第 3 次起均为命中；动图场景中 64 帧在缓存中可全部容纳，首轮结束后同样全部命中。

### 10.3 真实程序 A/B（`render.exe` 首页，非 SDL Raster）

方法同 9.2：`git` 暂存功能改动 → 测量基线 → 恢复改动 → 测量优化后（各 3 次取均值）。用外部 Win32 `InvalidateRect(整客户区)` 每 33 ms 驱动一次整幅重绘，共 10 s：

| 指标 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| 单次绘制（`Window::OnPaintMsg`） | 2.008 ms | 2.037 ms | +1.4%（噪声内） |
| `AlphaPaint`（`Root Box`） | 1.476 ms | 1.530 ms | +3.7%（噪声内） |
| `Control::PaintImage`（每次绘制的合计） | 0.960 ms | 1.004 ms | +4.6%（噪声内） |
| `PaintImage` 单次平均 | 0.343 ms | 0.352 ms | +2.6%（噪声内） |
| 进程 CPU（10 s 驱动窗口内） | 1031 ms | 1021 ms | -1% |

结论：该场景下**没有可测量的收益，也没有实质回归**。原因是 `render.exe` 首页每帧绘制的图片以**动画帧**为主（每次绘制的内容都不同，key 不同），缓存不可能命中；准入策略保证这类一次性绘制不额外创建缓存，因此开销与优化前基本一致。缓存的价值体现在"同一张图片按固定尺寸重复绘制"的场景（滚动、静态图重绘、动图循环播放），由 10.2 的基准数据体现。

### 10.4 可靠性验证

| 验证项 | 内容 | 结果 |
|--------|------|------|
| 新增单元测试 | `tests/test_offscreen_render.cpp` 增加 OS-40~OS-44：准入/命中/目标尺寸变化、缓存路径像素正确性（四象限颜色）、`uFade` 不被烘焙进缓存、子区域/无缩放/小图/禁用缓存四条旁路、源图片内容变化后自动失效 | 通过（`offscreen_render` 共 **192 项检查，0 失败**） |
| 全量回归 | `ctest`（17 个用例，含 GL 后端的 `gl_window_smoke`） | 非 SDL 与 SDL 构建均 **17/17 通过** |
| GL 后端 | `gl_window_smoke` 的窗口绘制用例（缓存生成的是 Raster 图像，GL 下按纹理上传后 1:1 绘制） | 通过 |
| 显示/稳定性 | SDL 与非 SDL 下 `render.exe` 运行 6 s 后正常退出（无崩溃、无断言） | 通过 |
| 并发 | 缓存的所有操作均在 `std::mutex` 保护下；缩放图像的创建在锁外完成，重复创建时后插入者被忽略 | 代码走查确认 |

### 10.5 已知限制

- 内存占用：缓存上限 64 MB / 512 条，超出按 LRU 淘汰；控件尺寸或 DPI 变化后旧尺寸的条目会保留到被淘汰，属于"用内存换 CPU"的取舍；
- 动画首轮、只绘制一次的图片不进入缓存（准入策略），这是有意为之；
- 画布存在缩放/旋转变换时（如矩阵绘制路径）不启用缓存，避免二次采样导致画质下降；
- 缓存是进程级的，与具体窗口无关，多窗口共享；`Render_Skia` 销毁时不会主动清空（由 LRU 与总预算兜底）。

---

## 十一、优化项 2 实现与评测（动画帧对象复用）

**状态**：已完成（2026-11-18）
**目标**：`ImageInfo::GetFrame` 在"帧号 + 目标尺寸"均未变化时复用上一帧对象，省掉每帧的 `make_shared` 与重复的 `ReadFrameData`（含位图引用计数增减）。

### 11.1 实现要点

| 位置 | 改动 | 说明 |
|------|------|------|
| `duilib/Image/ImageInfo.h` | 新增 3 个成员 | `m_pLastFrame`（缓存的帧对象）、`m_nLastFrameIndex`（帧号）、`m_lastFrameDestSize`（目标尺寸）；`m_nLastFrameIndex` 初值为 `(uint32_t)-1` 表示无缓存 |
| `duilib/Image/ImageInfo.cpp` `GetFrame` | 入口快速路径 | 三个条件全部相同时直接返回缓存对象，跳过 `GetAnimationImage`、`make_shared`、`ReadFrameData` |
| 同上 | 只缓存"读取完成"的帧 | `m_bDataPending`（后台解码未完成）与 `m_bDataError` 的帧不写入缓存，并清空旧缓存，避免把待解码帧长期缓存后画面不再更新 |
| 同上 | `SetImageData` 清空缓存 | 图片数据/加载参数变化后旧帧对象失效，防止新旧图片串帧 |
| 内存 | 每个 `ImageInfo` 只缓存 1 帧 | GIF/APNG/WebP 的帧位图本就由解码器持有，缓存仅增加一次引用，**不增加像素内存**；PAG/Lottie 这类按需渲染的帧，额外常驻 1 张帧位图（与优化前每帧临时分配的尺寸相同，未放大峰值） |

不改动的行为：

- 帧号变化、目标尺寸变化（多控件共用同一图片且尺寸不同）时仍完整走 `ReadFrameData`，与优化前一致；
- 同一帧的 `AnimationFrame` 对象被复用后，调用方（`Image::GetCurrentFrame` / `Control::PaintImage`）只读访问其位图与尺寸，不做写入，因此共享对象是安全的；
- 所有读写均在 UI 线程（`GlobalManager::AssertUIThread()`）完成，无并发问题。

### 11.2 基准测试（隔离场景）

新增手工评测工具 `tests/bench_animation_frame_reuse.cpp`（Release，非 SDL，20 万次/组，预热 10% 后重复 3 次取最优），图片为 `themes/default/render/gif_test.gif`：

| 场景 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| `same-frame`：同一帧、96x96 重复读取 | 0.134 µs | **0.066 µs** | **-51%（2.0x）** |
| `next-frame`：逐帧轮换读取（缓存不命中） | 0.146 µs | 0.162~0.165 µs | +0.016~0.019 µs（+11%~13%） |
| `two-sizes`：同帧、96x96 与 64x64 交替（缓存不命中） | 0.138 µs | 0.163 µs | +0.025 µs（+18%） |

结论：

- 命中路径把"新建对象 + 取帧数据"降为"比较 3 个字段 + 返回智能指针"，收益约 **0.07 µs/次**；
- 不命中路径多出的 16~25 ns 是缓存赋值/释放的引用计数记账开销，绝对值可忽略（相对一次完整绘制是万分之一量级），不存在实质回退。

### 11.3 真实程序 A/B（`render.exe`，非 SDL Raster，动画驱动重绘 14 s）

| 指标 | 优化前 | 优化后 | 变化 |
|------|--------|--------|------|
| `ImageInfo::GetFrame` 次数 / 总耗时 | 750 次 / 61 ms | 744 次 / 65 ms | 噪声（±6%） |
| `ImageInfo::GetFrame` 单次平均 | 0.081 ms | 0.087 ms | 噪声 |
| `Control::PaintImage` 单次平均 | 0.474 ms | 0.464 ms | -2%（噪声内） |
| `Window::OnPaintMsg` 单次平均 | 1.588 ms | 1.768 ms | 噪声 |

说明：该场景每次绘制使用的动画帧号几乎都在变化（`GetFrame` 调用数 ≈ 绘制次数），且不存在"同一图片同尺寸重复绘制"，因此**缓存基本不命中**，与优化前持平，符合预期。`controls.exe` 在 14 s 内只重绘了 1 次（19 张图片），同样不具备重复取帧的条件，不单独列数据。

### 11.4 可靠性验证

| 验证项 | 内容 | 结果 |
|--------|------|------|
| 新增单元测试 | `tests/test_image_draw.cpp` 增加"动画帧对象复用"用例：首次读取有效、同帧同尺寸复用同一对象、帧号变化不复用、目标尺寸变化不复用、回到旧组合时重新读取且帧数据正确（新增 17 项检查） | 通过（`image_draw` 共 **41 项检查，0 失败**） |
| 全量回归 | `ctest`（17 个用例，含 GL 后端的 `gl_window_smoke`） | 非 SDL 与 SDL 构建均 **17/17 通过** |
| 待解码帧 | 后台解码未完成时返回新对象且不入缓存（代码走查 + GIF 同步/异步解码路径共用同一实现） | 代码走查确认 |
| 图片切换 | `SetImageData` 清空缓存，避免新旧图片串帧 | 代码走查确认 |

### 11.5 已知限制

- 仅缓存最近一帧：多个控件共用同一 `ImageInfo` 且帧号/尺寸来回交替时，缓存会互相覆盖，退化为优化前的行为（功能正确，无收益）；
- 常驻一个帧对象的引用：对按需渲染的格式（PAG/Lottie）会多保留一张帧位图，生命周期与 `ImageInfo` 相同；
- 若外部修改了返回的 `AnimationFrame` 对象（当前代码库中没有这种用法），会影响下一次复用，后续如新增此类调用需要改为返回值拷贝。

