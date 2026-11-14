# kNativeGL 渲染后端实现分析与改进方案

> 状态：分析报告（2026-11-14）；阶段 1（正确性）、阶段 2（性能）、阶段 3（体验）、阶段 4（工程化）已实施（2026-11-14）
> 适用范围：Windows 平台（`DUILIB_BUILD_FOR_WIN`，不含 `_M_ARM64`），Skia Ganesh + WGL
> 关联文档：[IRender职责拆分与离屏渲染设计方案.md](IRender职责拆分与离屏渲染设计方案.md)、[IRender接口分析报告.md](IRender接口分析报告.md)

## 一、结论

`kNativeGL_BackendType` 目前还不能满足日常使用。代码能够跑通"单窗口、不缩放、不透明、创建后不再变尺寸"这一条最窄的路径，这也是此前仅做局部测试就能通过的原因；一旦进入真实使用场景（多窗口、拖动缩放、窗口透明度、最小化恢复、多线程窗口），就会出现黑屏、渲染错乱、断言失败甚至上下文泄漏。

核心原因不是 Skia 集成本身，而是 **WGL 上下文的生命周期与"当前上下文"（current context）状态没有被显式管理**，现有实现把"GL 上下文创建后一直留在当前线程上"当成了前提。

主要阻塞点：

1. 同一线程只允许一个 GL 窗口存活，第二个窗口创建后第一个窗口必然渲染错乱；
2. 窗口每次尺寸变化都会销毁并重建整个 WGL 上下文，拖动缩放时卡顿、闪烁；
3. Vsync 在 `wglMakeCurrent` 之前设置，实际从未生效；
4. 上下文初始化失败仍返回可用对象，窗口全黑且不回退 CPU；
5. GL 与分层窗口（`opacity < 255`）冲突时没有降级策略；
6. 每帧 `GrSyncCpu::kYes` 全同步，CPU 与 GPU 相互等待。

## 二、当前实现流程

### 2.1 创建阶段

```
WindowBuilder 解析 XML（PreInitWindow 阶段，原生窗口已创建）
  └─ Window::SetRenderBackendType(kNativeGL)          duilib/Core/Window.cpp#L136-L164
       └─ RenderFactory_Skia::CreateWindowRender
            └─ new Render_Skia_Windows(hWnd, GL)      duilib/RenderSkia/Render_Skia_Windows.cpp#L64-L82
                 └─ MakeGLForWin(hWnd, DisplayParams)
                      └─ SkGLWindowContext_Windows     duilib/RenderSkia/SkGLWindowContext_Windows.cpp#L23-L36
                           ├─ SkCreateWGLContext（SetPixelFormat + wglCreateContext）
                           ├─ swapInterval(1)          ← 此时无 current context（L103-L106）
                           ├─ wglMakeCurrent(dc, rc)   ← L109 / L124，成功后一直保持 current
                           └─ GetClientRect → fWidth/fHeight + glViewport
```

`Render_Skia_Windows` 在 GL 上下文创建失败时会回退到 CPU 上下文（`MakeRasterForWin`），并把 `m_backendType` 改为 `kRaster`，这一点设计是正确的（见 `duilib/RenderSkia/Render_Skia_Windows.cpp#L64-L82`）。

### 2.2 绘制阶段

```
WM_PAINT
  └─ NativeWindow_Windows::OnPaintMsg               duilib/Core/NativeWindow_Windows.cpp#L1622-L1641
       └─ IRenderWindow::PaintAndSwapBuffers
            └─ SkGLWindowContext_Windows::PaintAndSwapBuffers   duilib/RenderSkia/SkGLWindowContext_Windows.cpp#L171-L221
                 ├─ 取整个客户区（GL 不支持局部绘制）
                 ├─ getBackbufferSurface()（要求本上下文是 current）
                 ├─ BeginPaint / EndPaint → DoPaint(全窗口)
                 ├─ ValidateRect
                 ├─ flushAndSubmit(..., GrSyncCpu::kYes)   ← 每帧 CPU 等待 GPU
                 └─ swapBuffers()
```

`PaintAndSwapBuffers` 返回 `false` 时，`NativeWindow_Windows` 会用 `BeginPaint/EndPaint` 兜底校验区域（`#L1637-L1641`），不会造成 WM_PAINT 死循环。

### 2.3 尺寸变化

- `Window::ResizeRenderToClientSize()`（`duilib/Core/Window.cpp#L1197` 附近）在 `GetRender()`、`PostInitWindow`、WM_SIZE 等路径被调用；
- `Render_Skia_Windows::Resize()`（`duilib/RenderSkia/Render_Skia_Windows.cpp#L95-L114`）调用 `m_pWindowContext->resize()`；
- `SkGLWindowContext_Windows::resize()`（`#L44-L58`）直接转调基类 `GLWindowContext::resize()`，而基类实现是 **`destroyContext()` + `initializeContext()`**，即销毁并重建 WGL 上下文与 `GrDirectContext`。

### 2.4 销毁

`SkGLWindowContext_Windows::~SkGLWindowContext_Windows()`（`#L38-L42`）先注销离屏上下文，再 `destroyContext()`；`onDestroyContext()`（`#L161-L169`）断言当前 RC 等于自身 RC，然后 `wglMakeCurrent(nullptr)` + `wglDeleteContext()`。

### 2.5 离屏 GL 上下文复用

`duilib/RenderSkia/SkOffscreenGLContext.cpp#L114-L139`：

- `thread_local g_pWindowContext` 只保存 **一个** 窗口 GL 上下文（最后注册的那个）；
- `GetDirectContext()` 直接返回该上下文的 `GrDirectContext`，不校验/不切换 current context；
- 无窗口上下文时惰性创建隐藏窗口承载的后台 GL 上下文；
- `IsSupported()` 会触发 `GetDirectContext()`，即"能力查询"存在创建上下文的副作用。

## 三、问题清单

| 级别 | 问题 | 位置 | 现象 |
| --- | --- | --- | --- |
| P0 | 同线程多窗口 GL 上下文互相抢占 | `SkGLWindowContext_Windows.cpp#L67-L77` | 第二个 GL 窗口创建后，第一个窗口绘制到错误上下文；Debug 断言、Release 花屏/黑屏 |
| P0 | 窗口尺寸变化重建整个 WGL 上下文 | `SkGLWindowContext_Windows.cpp#L44-L58` | 拖动缩放时频繁销毁/重建上下文，卡顿、闪烁，析构断言失败 |
| P0 | Vsync 设置在 `wglMakeCurrent` 之前 | `SkGLWindowContext_Windows.cpp#L103-L109` | `wglSwapIntervalEXT` 无当前上下文而失败，Vsync 从未生效，GPU 空转/撕裂 |
| P0 | 上下文初始化失败仍返回可用渲染对象 | `SkGLWindowContext_Windows.cpp#L124-L158` | `wglMakeCurrent` 失败时 `fWidth/fHeight` 仍为 0，窗口全黑且不回退 CPU |
| P0 | GL 与分层窗口（`opacity < 255`）冲突 | `WindowBuilder.cpp#L1054-L1064` | 显式设置 opacity 时仍保留 layered window，GL 窗口黑屏/不可见 |
| P0 | 析构/销毁依赖"上下文恰好是 current" | `SkGLWindowContext_Windows.cpp#L161-L169` | 关闭非当前 GL 窗口时断言失败，`wglDeleteContext` 可能失败导致泄漏 |
| P1 | 每帧 `GrSyncCpu::kYes` | `SkGLWindowContext_Windows.cpp#L213-L219` | CPU 与 GPU 全同步，帧延迟高、占用高 |
| P1 | GL 窗口的 `Clone()` 在同一个 HWND 上再建上下文 | `Render_Skia_Windows.cpp#L132-L141` | 第二次 `SetPixelFormat` 失败，`Clone` 静默退化为 CPU 后端 |
| P1 | 启动阶段创建两次 GL 上下文 | `Window.cpp#L369-L396` | 先按初始/0 尺寸建上下文，再 Resize 销毁重建，启动变慢且多余 |
| P1 | RenderDoc 分支删除当前上下文失败 | `SkGLWindowContext_Windows.cpp#L113-L121` | `wglDeleteContext` 对 current 上下文无效，泄漏一个 RC |
| P1 | GL 离屏依赖"某窗口上下文恰好是 current" | `SkOffscreenGLContext.cpp#L114-L139` | 多窗口时离屏绘制使用错误上下文；能力查询有创建上下文副作用 |
| P2 | 全窗口重绘（GL 无局部刷新） | `SkGLWindowContext_Windows.cpp#L183-L208` | 4K 窗口任何小动画都整窗重绘，CPU 光栅化开销大 |
| P2 | GL 下富文本依赖 `GetRenderDC() == nullptr` 的隐式约定 | `Render_Skia_Windows.cpp#L201-L220` | 可维护性差，新控件容易踩坑 |
| P2 | 缺少多窗口/缩放/透明度的自动化验证 | `tests/` | 回归只能靠人工，问题不可控 |

## 四、P0 问题的详细分析与修复

### 4.1 多窗口同线程上下文抢占（最致命）

`getBackbufferSurface()` 在 Debug 下断言 `wglGetCurrentContext() == m_fHGLRC`，说明代码作者已经知道该限制，但 Release 下没有任何保护：

1. 创建窗口 A → A 的 RC 成为 current；
2. 创建窗口 B → B 的 RC 成为 current；
3. 再刷新窗口 A → `getBackbufferSurface()` 在 B 的上下文上读取 `FRAMEBUFFER_BINDING`，却用 A 的 `GrDirectContext` 绘制，结果写入错误的 FBO，A 黑屏/B 花屏；
4. 关闭 A 时 `onDestroyContext` 的 `ASSERT(m_fHGLRC == wglGetCurrentContext())` 失败。

**修复**：引入显式的 `MakeCurrent()`，并在所有触碰 GL 的入口调用：

```cpp
// SkGLWindowContext_Windows
bool MakeCurrent()
{
    if (m_hDC == nullptr) {
        m_hDC = ::GetDC(m_hWnd);   // 创建上下文时已 SetPixelFormat，复用同一个 DC
    }
    return ::wglMakeCurrent(m_hDC, m_fHGLRC) != FALSE;
}

// 入口：resize / getBackbufferSurface / PaintAndSwapBuffers / onDestroyContext
if (!MakeCurrent()) {
    return nullptr;
}
```

`MakeCurrent()` 只是把 `wglMakeCurrent` 指向本上下文，开销在微秒级。若担心每帧多次调用，可在 `PaintAndSwapBuffers()` 开始时调用一次，绘制过程内部不再重复调用。

### 4.2 窗口缩放不应重建 GL 上下文

拖动窗口时 WM_SIZE 高频触发，当前实现每次都销毁并重建 WGL 上下文与 `GrDirectContext`，代价过高，且旧帧缓冲被销毁会造成闪烁。

**修复**：不再调用基类 `resize()`，只重建 SkSurface：

```cpp
void SkGLWindowContext_Windows::resize(int w, int h)
{
    if ((w == fWidth) && (h == fHeight)) {
        return;
    }
    if (!MakeCurrent()) {
        return;
    }
    fWidth = w;
    fHeight = h;
    fSurface.reset();          // 下次 getBackbufferSurface() 时按新尺寸重建
    glViewport(0, 0, w, h);
}
```

这样缩放只重建一个 render target，WGL 上下文与 `GrDirectContext` 保持稳定。

### 4.3 Vsync 顺序错误

`extensions.swapInterval()` 必须在该 RC 为 current 时调用，否则调用失败。应将 Vsync 设置移入 `wglMakeCurrent` 成功的分支：

```cpp
if (wglMakeCurrent(dc, m_fHGLRC)) {
    SkWGLExtensions ext;
    if (ext.hasExtension(dc, "WGL_EXT_swap_control")) {
        ext.swapInterval(pDisplayParams->disableVsync() ? 0 : 1);
    }
    // ... 初始化 stencil / sampleCount / viewport
}
```

同时建议把 `DisplayParams` 中的 `disableVsync`、`msaaSampleCount` 暴露为窗口配置项（见 6.4）。

### 4.4 初始化失败必须返回失败并回退 CPU

当前 `L124` 的 `if (wglMakeCurrent(...))` 失败时仍然 `return GrGLMakeNativeInterface()`，导致 `isValid()` 为 true 而 `fWidth/fHeight` 为 0。修复：

```cpp
if (!wglMakeCurrent(dc, m_fHGLRC)) {
    ::wglDeleteContext(m_fHGLRC);
    m_fHGLRC = nullptr;
    ::ReleaseDC(m_hWnd, dc);
    return nullptr;    // → MakeGLForWin 返回 nullptr → Render_Skia_Windows 走 CPU 回退
}
```

同时把 `Render_Skia_Windows` 构造中的 `ASSERT(m_pWindowContext != nullptr)`（`Render_Skia_Windows.cpp#L64-L82`）改为"记录日志 + 回退"，因为无 OpenGL 的机器（远程桌面、虚拟机、部分 ARM 设备）是正常场景，不应断言。

### 4.5 GL 与分层窗口

`WindowBuilder.cpp#L1054-L1064` 仅在"未设置 opacity 或 opacity == 255"时禁用 layered。应改为：

- GL 后端一律 `SetLayeredWindow(false, false)`；
- 若 XML 要求 `opacity < 255`，该窗口降级为 `kRaster_BackendType` 并输出告警，而不是继续用 GL。

同时在运行时接口增加保护：

```cpp
// Window::SetLayeredWindow / SetLayeredWindowAlpha
if (GetRenderBackendType() == RenderBackendType::kNativeGL_BackendType) {
    // GL 不支持分层窗口，忽略并记录日志（或断言）
    return false;
}
```

### 4.6 析构顺序

```cpp
void SkGLWindowContext_Windows::onDestroyContext()
{
    if (m_fHGLRC == nullptr) {
        return;
    }
    MakeCurrent();                                  // 尽量切回本上下文，不再依赖断言
    ::wglMakeCurrent(nullptr, nullptr);
    ::wglDeleteContext(m_fHGLRC);
    m_fHGLRC = nullptr;
    if (m_hDC != nullptr) {
        ::ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }
}
```

RenderDoc 分支在 `wglDeleteContext` 前也必须先 `wglMakeCurrent(nullptr, nullptr)`（当前实现直接删除 current 上下文，删除会失败）。

## 五、P1 问题与修复

### 5.1 去掉每帧 CPU 同步

```cpp
if (auto dContext = this->directContext()) {
    dContext->flushAndSubmit(backbuffer.get(), GrSyncCpu::kNo);
}
swapBuffers();
```

仅在 resize、销毁、`MakeImageSnapshot()`、`ReadPixels()` 等需要读回结果的场景使用 `kYes`。

### 5.2 启动期只创建一次 GL 上下文

`Window::PostInitWindow()`（`Window.cpp#L369-L396`）先创建 Render，再应用 XML 初始尺寸并调用 `ResizeRenderToClientSize()`，导致 GL 上下文被创建两次。建议：

- 先应用初始尺寸，再创建 Render；
- 或让 GL 上下文延迟到首次 `GetSkSurface()` 且客户区非 0 时初始化（惰性初始化），同时把"Render 已创建但上下文未初始化"作为合法状态处理。

### 5.3 `Clone()` 对 GL 返回独立离屏副本

当前 `Clone()` 在同一个 HWND 上再建一个 `Render_Skia_Windows`，第二次 `SetPixelFormat` 会失败，`Clone` 静默退化成 CPU 后端。建议：

- GL 窗口的 `Clone()` 返回 `nullptr`（明确不支持）；
- 或返回 `CreateOffscreenRender()` 创建的离屏副本并用 `BitBlt` 复制内容，保证后端语义一致。

### 5.4 GL 离屏显式管理 current context

- `OffscreenGLContext` 保存 `SkGLWindowContext_Windows*` 列表（线程内可多个窗口），而不是单个指针；
- 返回 `GrDirectContext` 前先对选中的上下文 `MakeCurrent()`；
- 窗口上下文与离屏上下文共享同一个 `GrDirectContext`，统一纹理缓存、减少显存占用；
- `IsOffscreenBackendSupported()` 改为纯查询，避免触发隐藏窗口与上下文创建。

## 六、P2 与体验改进

### 6.1 局部刷新（收益最大）

GL 默认帧缓冲在 `SwapBuffers` 后内容未定义，因此当前只能整窗重绘。可选方案：维护一个窗口尺寸的持久离屏 FBO：

1. 按 `rcPaint` 只绘制脏区到持久 FBO；
2. 每帧把整张纹理贴到默认帧缓冲；
3. 交换。

这样既保留控件树的脏区优化，也消除缩放闪烁，但需要额外的显存（宽 x 高 x 4 字节）。

### 6.2 帧率与动画

- Vsync 修复后，动画帧率受 WM_TIMER/刷新节奏约束；
- 大窗口 + 高频动画场景建议配合 6.1 的局部刷新，避免整窗重绘。

### 6.3 可维护性

- `GetRenderDC()` 对 GL 返回 `nullptr` 的约定建议写入 `IRenderWindow` 注释，并在新控件开发文档中说明；
- `PaintAndSwapBuffers` 中"GL 必须整窗绘制"的假设应显式注释，避免后续误加局部裁剪逻辑。

### 6.4 可配置项

当前 GL 参数写死（`msaaSampleCount = 0`、Vsync 默认开）。建议在窗口 XML 增加透传配置，例如：

```xml
<Window render_backend_type="GL" vsync="true" msaa="0" />
```

### 6.5 测试补充（日常可用性验收）

- 同一线程创建 2~3 个 GL 窗口，交替 `Invalidate`，验证内容正确、无断言；
- 拖动缩放、最小化/恢复、DPI 切换、关闭其中一个窗口；
- 显式 `opacity` 窗口验证自动降级为 CPU；
- 用 PresentMon 验证 Vsync 生效（帧间隔约 16.7ms），GPU 占用回到正常；
- 长稳 30~60 分钟，检查 GDI/USER 句柄与显存无持续增长。

## 七、改造路线与验收标准

| 阶段 | 内容 | 验收标准 |
| --- | --- | --- |
| 阶段 1（正确性）**已实施** | MakeCurrent 管理、初始化失败回退、Vsync 顺序、destroy 顺序、layered 保护 | 同线程多 GL 窗口交替绘制正常；关闭任意窗口无断言；无 OpenGL 环境自动回退 CPU；Vsync 生效 |
| 阶段 2（性能）**已实施** | resize 不重建上下文、`kNo` 同步、启动单次初始化、Clone 修正 | 拖动缩放无闪烁、帧率稳定；启动只创建一次上下文；CPU 占用明显下降 |
| 阶段 3（体验）**已实施** | 持久 FBO 局部刷新、离屏共享上下文、vsync/msaa 配置 | 4K 窗口小动画 CPU 占用明显下降；离屏 GL 在多窗口下可用 |
| 阶段 4（工程化）**已实施** | GL 多窗口冒烟测试、文档补充 | 回归测试可自动执行；文档说明 GL 后端限制与降级策略 |

完成阶段 1、2 后，GL 后端才具备"多窗口、可缩放、不透明窗口"的日常可用性；阶段 3 才解决"大面积重绘带来的 CPU 占用"；阶段 4 把上述能力固化为可自动执行的回归测试，并明确限制与降级策略。

## 八、关键代码位置索引

| 文件 | 说明 |
| --- | --- |
| `duilib/RenderSkia/SkGLWindowContext_Windows.h/.cpp` | WGL 上下文创建、绘制、交换、销毁 |
| `duilib/RenderSkia/Render_Skia_Windows.h/.cpp` | 窗口 Render 实现、GL/CPU 选择与回退、Clone |
| `duilib/RenderSkia/SkOffscreenGLContext.h/.cpp` | 每线程 GL 上下文复用与后台隐藏窗口上下文 |
| `duilib/RenderSkia/Render_Skia_Offscreen.h/.cpp` | 离屏渲染（GPU 时共享窗口 `GrDirectContext`） |
| `tests/test_gl_window_smoke.cpp` | GL 多窗口冒烟测试（GS-01 ~ GS-16） |
| `duilib/RenderSkia/RenderFactory_Skia.cpp` | 窗口/离屏渲染创建入口 |
| `duilib/Core/Window.cpp` | `PostInitWindow`、`SetRenderBackendType`、Render 尺寸同步 |
| `duilib/Core/WindowBuilder.cpp` | `render_backend_type` 解析、layered 与 GL 的兼容处理 |
| `duilib/Core/NativeWindow_Windows.cpp` | WM_PAINT 分发与兜底 |
| Skia `tools/window/GLWindowContext.cpp` | 基类 `resize`（销毁重建上下文）与 `getBackbufferSurface` |

> 说明：Skia 属于外部依赖，不在本仓库内；行号以当前工作区代码为准，后续修改后需同步更新本文档。

## 九、阶段 1 实施记录（2026-11-14）

### 9.1 修改内容

| 目标 | 修改位置 | 具体做法 |
| --- | --- | --- |
| MakeCurrent 管理 | `SkGLWindowContext_Windows.h/.cpp` | 新增 `MakeCurrent()`（含 `wglGetCurrentContext` 快路径）与 `ReleaseWindowDC()`；`getBackbufferSurface()`、`PaintAndSwapBuffers()`、`onDestroyContext()` 均先切换为本窗口上下文 |
| DC 复用 | `SkGLWindowContext_Windows` | 新增成员 `m_hDC`，与 GL 上下文同生命周期；`onSwapBuffers()` 直接使用 `m_hDC` |
| 初始化失败回退 | `SkGLWindowContext_Windows::onInitializeContext()` | `wglMakeCurrent` 失败时删除已创建的 RC、释放 DC 并返回 `nullptr`；`SkCreateWGLContext` 失败同样清理 |
| 初始化失败回退 | `Render_Skia_Windows` 构造函数 | 移除 GL 失败时的 `ASSERT`，改为输出日志后回退 CPU 绘制 |
| Vsync 顺序 | `SkGLWindowContext_Windows::onInitializeContext()` | `swapInterval()` 移到 `wglMakeCurrent` 成功之后；RenderDoc 重建上下文后再次设置 |
| destroy 顺序 | `SkGLWindowContext_Windows::onDestroyContext()` | 不再断言"上下文恰好是当前上下文"，先 `MakeCurrent()` 再 `wglMakeCurrent(nullptr,nullptr)`、`wglDeleteContext`，最后释放 DC |
| RenderDoc 分支 | `SkGLWindowContext_Windows::onInitializeContext()` | 删除当前上下文前先 `wglMakeCurrent(nullptr, nullptr)`，避免删除失败泄漏 RC |
| layered 保护（配置） | `WindowBuilder::ParseWindowCreateAttributes()` | GL 且未设置 `opacity` 时一律关闭层窗口；删除矛盾的 `ASSERT(!m_bIsLayeredWindow)` |
| layered 保护（降级） | `WindowBuilder::ParseWindowAttributes()` | XML 同时要求 `render_backend_type="GL"` 与 `opacity < 255` 时，自动降级为 `kRaster_BackendType` 并输出日志 |
| layered 保护（配置） | `WindowBuilder::ParseWindowAttributes()` | GL 窗口一律 `SetLayeredWindow(false, false)`，避免阴影逻辑再次开启层窗口 |
| layered 保护（运行时） | `WindowBase::SetLayeredWindow()` / `SetLayeredWindowOpacity()` | GL 后端请求层窗口或 `opacity < 255` 时直接返回 `false` |
| layered 保护（运行时） | `Window::UpdateLayeredWindowStyleEx()` | GL 后端不再因阴影自动开启层窗口，已开启的会关闭 |

### 9.2 验证结果

- Debug x64 构建通过（`python build/build.py build --config Debug --arch x64 --jobs 4`）；
- `ctest --test-dir out/x64/Debug`：12/12 通过（现有自动化用例均基于 CPU 离屏渲染，GL 窗口行为需按 6.5 的清单人工验证）；
- 阶段 1 未改变 `resize` 策略（仍销毁重建上下文），拖动缩放卡顿/闪烁问题留待阶段 2 处理（阶段 2 已处理）。

## 十、阶段 2 实施记录（2026-11-14）

### 10.1 修改内容

| 目标 | 修改位置 | 具体做法 |
| --- | --- | --- |
| resize 不重建上下文 | `SkGLWindowContext_Windows::resize()` | 不再调用基类 `resize`（它会销毁并重建 WGL 上下文）；只 `MakeCurrent()` 后更新 `fWidth/fHeight`、`fSurface.reset()`、`glViewport()`，下次 `getBackbufferSurface()` 按新尺寸重建绘制表面 |
| 去掉每帧 CPU 同步 | `SkGLWindowContext_Windows::PaintAndSwapBuffers()` | `flushAndSubmit(backbuffer, GrSyncCpu::kNo)`，由 `SwapBuffers` 与显示同步；读回像素的场景由 `SkSurface::readPixels` 内部保证 |
| 启动单次初始化 | `SkGLWindowContext_Windows::resize()` | 由"resize 不重建上下文"直接解决：`Window::PostInitWindow()` 创建 Render 后再应用初始尺寸时，不再产生第二个 WGL 上下文，无需调整初始化顺序 |
| Clone 修正 | `Render_Skia_Windows::Clone()` | GL 窗口不再在同一个 HWND 上创建第二个 GL 上下文，改为 `CreateOffscreenRender()` 创建离屏副本并用 `BitBlt` 拷贝内容；创建失败返回 `nullptr` |

### 10.2 说明

- `resize` 期间 `GrDirectContext` 保持存活，已录制的绘制命令由 Skia 管理引用计数，重置 `fSurface` 不会造成悬空引用；
- `fStencilBits/fSampleCount` 只在上下文创建时读取，本次修改不再重建上下文，因此无需重新读取；
- GL 窗口的 `Clone()` 返回值是离屏（CPU）副本，仅保证像素内容一致，不保证后端类型一致。

### 10.3 验证结果

- Debug x64 构建通过；
- `ctest --test-dir out/x64/Debug`：12/12 通过；
- 拖动缩放、帧率与 CPU 占用的量化验证需在带 OpenGL 的桌面环境按 6.5 清单人工执行。

## 十一、阶段 3 实施记录（2026-11-14）

### 11.1 修改内容

| 目标 | 修改位置 | 具体做法 |
| --- | --- | --- |
| 持久 FBO | `SkGLWindowContext_Windows.h/.cpp` | 新增 `m_persistentSurface` 与 `m_bNeedFullRedraw`；`getBackbufferSurface()` 优先返回持久化的 GPU 渲染目标（`SkSurfaces::RenderTarget`，`kTopLeft` 原点、`kRGBA_8888`、采样数与窗口一致），创建后清空并标记完整重绘；`resize()` 中一并释放该表面 |
| 局部刷新 | `SkGLWindowContext_Windows::PaintAndSwapBuffers()` | 使用 `GetUpdateRect()` 的脏区作为 `DoPaint()` 的绘制区域（`m_bNeedFullRedraw` 或持久表面不可用时使用整客户区）；绘制完成后用 `drawImage(..., kSrc)` 把整幅持久表面贴到默认帧缓冲，再 `flushAndSubmit(kNo)` + `SwapBuffers()` |
| 多窗口离屏共享 | `SkOffscreenGLContext.h/.cpp` | 线程内的单个窗口上下文指针改为上下文列表（堆对象故意不析构，规避线程退出时的析构顺序问题）；`GetDirectContext()` 逆序遍历，对 `SkGLWindowContext_Windows` 先 `MakeCurrent()`，成功者优先复用；注册/注销改为增删列表 |
| MakeCurrent 可见性 | `SkGLWindowContext_Windows.h` | `MakeCurrent()` 由 `private` 调整为 `public`，供离屏上下文复用窗口上下文时调用 |
| vsync/msaa 透传 | `IRender.h`、`RenderFactory_Skia`、`Render_Skia_Windows` | `IRenderFactory::CreateWindowRender()` 增加 `nMsaaSampleCount`、`bDisableVsync`（默认 0/false，保持原有行为）；Windows 实现用 `DisplayParamsBuilder` 设置 MSAA 与垂直同步 |
| 窗口属性 | `Window.h/.cpp`、`WindowBuilder.cpp` | `Window::SetRenderBackendType()` 增加两个参数并保存，两处 `CreateWindowRender()` 调用透传；XML 新增 `vsync`、`msaa` 属性解析并纳入已知属性 |

### 11.2 XML 配置说明

```xml
<Window render_backend_type="GL" vsync="true" msaa="4" ...>
```

- `vsync`：`true`/`false`，默认 `true`；`false` 等价于禁用垂直同步（`swapInterval(0)`）。
- `msaa`：MSAA 采样数，默认 `0`（使用 1 倍采样，即不启用 MSAA）；非 2 的幂会被 Skia 向上取整，显卡不支持时 Skia 会自动折半重试。
- 两个属性仅在 `render_backend_type` 为 GL/GPU 时生效，CPU 绘制时被忽略。

### 11.3 说明与限制

- 持久 FBO 会额外占用一份显存（宽 x 高 x 4 字节，4K 下约 33MB）；
- 每帧仍需把整幅持久表面拷贝到默认帧缓冲：这是"GL 默认帧缓冲在 `SwapBuffers` 后内容未定义"的必要代价，但拷贝发生在 GPU 内，CPU 侧不再重绘整窗；
- 局部刷新依赖 WM_PAINT 的更新区域，控件树需按 `rcPaint` 正确裁剪（`Window::Paint()` 已用 `AutoClip` 处理）；
- 多窗口场景下，离屏 GPU 渲染与"最近注册且 `MakeCurrent()` 成功"的窗口共享 `GrDirectContext`；窗口销毁后自动回退到其它窗口上下文或后台隐藏窗口上下文；
- `Render_Skia_Windows::Clone()` 对 GL 仍返回 CPU 离屏副本，本次未改变。

### 11.4 验证结果

- Debug x64 构建通过（`python build/build.py build --config Debug --arch x64 --jobs 4`）；
- `ctest --test-dir out/x64/Debug`：12/12 通过（现有用例均基于 CPU 离屏渲染）；
- GL 局部刷新的 CPU 占用、多窗口离屏渲染行为需在带 OpenGL 的桌面环境按 6.5 清单人工验证。

### 11.5 运行过程中实时切换与全局开关

在上述配置透传的基础上，补充了运行过程中的动态控制能力：

| 能力 | 位置 | 说明 |
| --- | --- | --- |
| 窗口属性实时切换 | `Window::SetAttribute` | 支持 `render_backend_type`、`vsync`、`msaa`；`Window::SetRenderBackendType()` 发现生效配置变化时，销毁并重建 Render 对象，随后同步尺寸并重绘窗口 |
| 全局开关（属性） | `WindowBuilder::ParseGlobalAttributes` | 在 `global.xml` 的 `Global` 根节点设置 `render_backend_type`（可选 `vsync`、`msaa`）即启用全局开关，所有窗口统一使用该配置 |
| 全局开关（函数） | `GlobalManager` | `SetGlobalRenderBackendType()` / `ClearGlobalRenderBackendType()` / `ApplyGlobalRenderBackendTypeToAllWindows()` 及配套 getter |

注意事项：

- 全局开关优先级高于窗口自身配置，`Window::SetRenderBackendType()` 入口处统一做覆盖；
- 运行过程中切换到 GL 时，若窗口是层窗口则自动降级为 CPU 绘制（GL 不支持层窗口）；
- 全局开关关闭后，已创建的窗口保持当前渲染方式，新建窗口恢复使用自身配置。

## 十二、阶段 4 实施记录（2026-11-14）

### 12.1 代码 Review 与修复

对阶段 1~3 的全部改动做了一次完整审查，发现并修复以下问题：

| 问题 | 位置 | 影响 | 修复方式 |
| --- | --- | --- | --- |
| 窗口后台缓冲包装了错误的 FBO | `SkGLWindowContext_Windows` | 持久化 RenderTarget 创建/使用后，OpenGL 当前绑定的 FBO 已不是窗口默认帧缓冲；基类 `getBackbufferSurface()` 按"当前绑定"包装，会把持久表面自身当成窗口后台缓冲，导致整窗黑屏/画面冻结（缩放后必现） | 新增 `GetWindowSurface()`：用 `GrBackendRenderTargets::MakeGL(fWidth, fHeight, fSampleCount, fStencilBits, {fFBOID = 0, GL_RGBA8})` + `SkSurfaces::WrapBackendRenderTarget()` 显式包装默认帧缓冲（FBO 0），缓存于 `m_windowSurface`，`resize()`/析构时释放；`PaintAndSwapBuffers()` 不再调用基类实现 |
| 共享上下文在窗口销毁后被继续使用 | `SkGLWindowContext_Windows` 析构、`Render_Skia_Offscreen` | 离屏 GPU 渲染共享窗口的 `GrDirectContext`；窗口销毁后离屏对象仍持有该上下文，后续访问为未定义行为 | 窗口析构时在销毁 GL 上下文前调用 `abandonContext()`（离屏对象后续调用变为安全空操作）；`Render_Skia_Offscreen::m_pGrContext` 由裸指针改为 `sk_sp<GrDirectContext>`（`sk_ref_sp` 增加引用计数），避免堆内存 UAF |
| 记录的渲染后端与实际不一致 | `Window::SetRenderBackendType()` | GL 创建失败时 `Render_Skia_Windows` 内部已回退 CPU，但 `m_renderBackendType` 仍是 GL；重复设置 `render_backend_type="GL"` 会反复销毁重建 Render | 创建成功后以 `m_render->GetRenderBackendType()` 为准同步 `m_renderBackendType`（回退 CPU 时同时清零 `msaa`/`vsync` 配置） |

### 12.2 自动化回归测试

新增 `tests/test_gl_window_smoke.cpp`（仅 Windows 构建，已在 `tests/CMakeLists.txt` 注册为 `gl_window_smoke`），在真实窗口上覆盖：

| 编号 | 覆盖点 |
| --- | --- |
| GS-01 ~ GS-02 | 同一线程创建两个窗口的 GL 渲染对象，校验后端类型为 `kNativeGL` |
| GS-03 ~ GS-05 | 两窗口交替绘制 3 轮（验证每帧绘制前 `MakeCurrent()` 切换上下文），绘制次数与 `rcPaint` 正确 |
| GS-06 ~ GS-08 | 缩小/放大窗口后 `Resize()` 重建持久表面与窗口后台缓冲，继续绘制；另一窗口不受影响 |
| GS-09 ~ GS-12 | 运行过程中切换后端：GL → CPU → GL（模拟 `Window::SetRenderBackendType()` 的销毁重建流程） |
| GS-13 ~ GS-16 | 两个 GL 窗口存活时创建离屏 GPU 渲染，绘制并读回像素校验颜色，随后窗口继续绘制 |

测试特性：

- 窗口创建在屏幕外并显示（`SW_SHOWNOACTIVATE`）——隐藏窗口不产生更新区域，无法触发 GL 的局部重绘路径；
- 运行环境不支持 OpenGL 时（远程桌面、无驱动、ARM64 等），工厂会把 GL 降级为 CPU，此时测试打印 `[SKIP]` 并返回成功，保证在 CI/无人值守环境可自动执行；
- 断言输出重定向到标准错误，避免无界面环境下弹出断言对话框卡住进程。

### 12.3 GL 后端限制与降级策略

| 限制/场景 | 现象或原因 | 策略 |
| --- | --- | --- |
| 分层窗口（`WS_EX_LAYERED`、`opacity < 255`、透明窗口） | GL 渲染到窗口默认帧缓冲后由 DWM 合成，无法实现逐像素 Alpha | `Window::SetRenderBackendType()` 检测到层窗口请求 GL 时记录日志并降级为 CPU 绘制 |
| Windows ARM64 | Skia 未提供 ARM64 的 WGL 实现（`SkWGL_win.cpp` 被排除，`GrGLMakeNativeInterface()` 返回 `nullptr`） | 编译期排除 GL 上下文实现，始终使用 CPU 绘制 |
| 非 Windows 平台 / SDL 后端 | 无 WGL | `SetRenderBackendType()` 内部强制 CPU |
| 无 OpenGL 环境（远程桌面、虚拟机、缺少驱动、像素格式不支持、`wglMakeCurrent` 失败） | `SkCreateWGLContext()` 或初始化失败 | `Render_Skia_Windows` 记录日志并自动改用 CPU 渲染，`GetRenderBackendType()` 返回 `kRaster`，调用方无需特殊处理 |
| 运行过程中切换后端 | 需要销毁并重建 Render 对象 | `SetRenderBackendType()` 重建后同步尺寸并整窗重绘；切换瞬间 GPU 资源（含离屏快照）失效，调用方不能缓存旧的绘制结果 |
| 同一线程多个 GL 窗口 | 每个窗口有独立的 WGL 上下文与 `GrDirectContext` | 绘制、读取后台缓冲前显式 `MakeCurrent()`；`Render` 对象不能跨线程使用 |
| 离屏 GPU 渲染共享窗口上下文 | 共享"最近注册且 `MakeCurrent()` 成功"的窗口 `GrDirectContext` | 离屏 Render 的生命周期必须短于（或等于）共享窗口；窗口销毁时上下文被 `abandonContext()`，离屏对象退化为空操作而非崩溃 |
| 持久化 FBO 的显存占用 | 额外一份 `宽 × 高 × 4` 字节（4K 约 33MB），MSAA 时更高 | 仅在需要时开启 GL 与 MSAA；4K + 高倍 MSAA 需评估显存 |
| 每帧整幅拷贝 | `SwapBuffers` 后默认帧缓冲内容未定义，无法增量保留 | 每帧用 `drawImage(..., kSrc)` 把持久表面贴到默认帧缓冲（GPU 内拷贝，CPU 侧只重绘脏区）；因此不能对默认帧缓冲做混合绘制 |
| MSAA / Vsync | 依赖显卡与驱动扩展（`WGL_ARB_multisample`、`WGL_EXT_swap_control`） | MSAA 不支持时 Skia 自动折半重试；无 `WGL_EXT_swap_control` 时 Vsync 设置被忽略 |
| 像素直接访问 | GL 表面不支持 `peekPixels` | `Clear()`/`ClearRect()` 走 Skia 绘制接口；`GetRenderDC()` 对 GL 返回 `nullptr`（RichEdit 等控件自动走其它绘制路径） |

### 12.4 验证结果

- Debug x64 构建通过（`python build/build.py build --config Debug --arch x64 --jobs 4`）；
- `ctest --test-dir out/x64/Debug`：13/13 通过（新增 `gl_window_smoke` 在带 OpenGL 的桌面环境实际执行，33 项检查全部通过；无 OpenGL 环境自动 SKIP）。

### 12.5 缺陷修复：开启 GL 后窗口内容全黑（2026-11-14）

问题现象：在 `controls.exe` 的 Window 节点配置 `render_backend_type="GL" vsync="true" msaa="0"` 后，窗口边框正常但客户区（含标题栏）全部为黑色。

排查过程：

1. 临时在 `SkGLWindowContext_Windows.cpp` 中埋点记录构造、`resize()`、`PaintAndSwapBuffers()` 各阶段状态，确认 GL 上下文创建成功（`samples=1`、`stencil=8`）、绘制流程正常执行且 `glGetError()` 为 0；
2. 在持久化表面与窗口后台缓冲（FBO 0）上分别 `readPixels` 取点，两者像素值均正确（如 `FFFAFAFA`），说明绘制与拷贝逻辑本身没有问题，黑屏发生在"命令提交/呈现"阶段；
3. 对比发现：加入 `readPixels` 探针后画面即恢复正常——`readPixels` 会隐式触发 Skia 刷新；去掉探针后再次全黑。

根因：`PaintAndSwapBuffers()` 末尾调用的是 `dContext->flushAndSubmit(nullptr, GrSyncCpu::kNo)`。该重载签名为 `flushAndSubmit(SkSurface* surface, GrSyncCpu)`，第一个参数是"要刷新的表面"；Skia 实现中 `GrDirectContext::flush(SkSurface*)` 对 `surface == nullptr` 直接返回（`return {false, GrSemaphoresSubmitted::kNo}`），因此 `flushAndSubmit(nullptr, ...)` 只执行了 `submit()`，**没有 flush 任何绘制命令**。所有绘制（含把持久化表面贴到 FBO 0 的 `drawImage`）都停留在 Skia 的延迟命令缓冲中，`SwapBuffers()` 交换的是内容未更新的后台缓冲，窗口因此显示为黑色。

修复：改用刷新整个上下文的重载 `dContext->flushAndSubmit(GrSyncCpu::kNo)`（等价于 `flush(GrFlushInfo())` + `submit(sync)`），并在代码中补充注释说明不能传入 `nullptr`。

验证：

- Debug x64 构建通过，`ctest` 13/13 通过（含 `gl_window_smoke`）；
- 带 GL 配置启动 `controls.exe` 截图确认界面完整显示，标题栏、卡片、按钮等渲染正常；
- 验证完成后已还原 `controls.xml` 中的临时 GL 配置。

补充说明：该缺陷仅在"把绘制结果提交到窗口"这一环节，`gl_window_smoke` 只校验了 `PaintAndSwapBuffers()` 的返回值与离屏读回像素，无法覆盖"窗口实际呈现内容"，因此原有自动化测试未发现该问题；日常验收仍建议开启 GL 后目视确认一次画面。



