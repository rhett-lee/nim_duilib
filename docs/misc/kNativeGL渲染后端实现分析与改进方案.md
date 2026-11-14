# kNativeGL 渲染后端实现分析与改进方案

> 状态：分析报告（2026-11-14）；阶段 1（正确性）、阶段 2（性能）已实施（2026-11-14）
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
| 阶段 3（体验） | 持久 FBO 局部刷新、离屏共享上下文、vsync/msaa 配置 | 4K 窗口小动画 CPU 占用明显下降；离屏 GL 在多窗口下可用 |
| 阶段 4（工程化） | GL 多窗口冒烟测试、文档补充 | 回归测试可自动执行；文档说明 GL 后端限制与降级策略 |

完成阶段 1、2 后，GL 后端才具备"多窗口、可缩放、不透明窗口"的日常可用性；阶段 3 才解决"大面积重绘带来的 CPU 占用"。

## 八、关键代码位置索引

| 文件 | 说明 |
| --- | --- |
| `duilib/RenderSkia/SkGLWindowContext_Windows.h/.cpp` | WGL 上下文创建、绘制、交换、销毁 |
| `duilib/RenderSkia/Render_Skia_Windows.h/.cpp` | 窗口 Render 实现、GL/CPU 选择与回退、Clone |
| `duilib/RenderSkia/SkOffscreenGLContext.h/.cpp` | 每线程 GL 上下文复用与后台隐藏窗口上下文 |
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


