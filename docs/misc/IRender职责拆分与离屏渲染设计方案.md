# IRender 职责拆分与离屏渲染设计方案

> 状态：已实现（2026-11-14），三个阶段均已完成；Debug x64 构建通过、ctest 12/12 通过，离屏测试 108 项检查全部通过（GPU 后端能力查询结果为 true）。
> 关联文档：[IRender接口分析报告.md](IRender接口分析报告.md) 第五节第 1 条。
>
> 实现备注：
> - 新增文件：`duilib/RenderSkia/Render_Skia_Offscreen.h/.cpp`（Raster/GL 离屏渲染）、`duilib/RenderSkia/SkOffscreenGLContext.h/.cpp`（每线程共享 GL 上下文）、`tests/test_offscreen_render.cpp`；
> - `Render_Skia` 改为虚继承 `IRender`，`Render_Skia_Windows/SDL` 同时继承 `IRenderWindow`；
> - GL 离屏优先复用同线程窗口的 `GrDirectContext`（窗口上下文创建时注册到 `OffscreenGLContext`）；同线程无窗口上下文时，惰性创建隐藏窗口承载的后台 GL 上下文；
> - `Render_Skia` 的 `Clear` / `ClearRect` / `MakeImageSnapshot` 在无法访问像素指针（GPU 后端）时改用 Skia 接口（`SkCanvas::clear`、`SkSurface::readPixels` 等）；
> - 宽高约束：单边上限 16384；宽高必须同时为 0 或同时大于 0；
> - 测试用例 OS-21/OS-22（位图直绘）本期不实现；OS-32/OS-33（窗口控件迁移回归）需在集成环境验证，测试中标记为 SKIP。

## 一、背景与问题

当前 `duilib/Render/IRender.h` 中的 `IRender` 同时承担 7 类职责：

1. 图元绘制与路径；
2. 文本测量/绘制与富文本缓存；
3. 裁剪与视区原点；
4. 位图读写与快照（`ReadPixels` / `WritePixels` / `MakeImageSnapshot`）；
5. 窗口刷新（`PaintAndSwapBuffers`）；
6. 窗口形状（`SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn`）；
7. 平台 DC（`GetRenderDC` / `ReleaseRenderDC`）。

由此带来的实际约束：

- 离屏渲染被迫实现窗口接口，`Render_Skia_Windows/SDL` 即使没有窗口也要实现 `PaintAndSwapBuffers`、`ClearWindowRgn` 等空实现；
- 新增后端（含离屏后端）必须补齐窗口接口；
- 离屏能力没有显式入口，现有代码通过 `CreateRender(spDpi, nullptr)` + `Resize()` 隐式获得，语义不清；
- `Control::CreateTempRender`、`ControlDragable`、`BitmapHepler_Windows/SDL`、`CefControlOffScreen`、单元测试各自重复这套隐式用法。

## 二、目标与范围

目标：

1. `IRender` 只保留与窗口无关的绘制/测量能力；
2. 窗口相关能力拆分到 `IRenderWindow`，窗口层只依赖该接口；
3. 提供一级 API 的离屏渲染：指定尺寸创建 → 绘制 → 快照，或直接把快照绘制到窗口，全程不依赖任何窗口对象；
4. 离屏渲染支持多后端（Raster 必选，GPU 后端在可用时支持，并支持能力查询）；
5. 现有绘制调用方（`Control::Paint(IRender*)` 等）零改动。

不在本期范围：

- 图片编码导出（PNG/JPEG）：当前仓库只有解码器，没有编码器；
- Vulkan / Metal 等新后端接入（仅预留扩展点）。

## 三、接口分层设计

```
SupportWeakCallback
└── IRender                 绘制 / 文本 / 位图 / 裁剪 / 变换 / 尺寸 / DPI（与窗口无关）
    └── IRenderWindow       窗口刷新 + Windows 平台 DC + 窗口形状
```

### 1. `IRender`（通用绘制接口，保留）

- `GetRenderBackendType`、`GetWidth` / `GetHeight` / `Resize`；
- `Clear`、`ClearRect`、`ClearAlpha`、`RestoreAlpha`；
- 裁剪、图层、视区原点、矩阵、混合模式；
- 全部图元与图片绘制；
- 文本测量/绘制、富文本缓存；
- `ReadPixels`、`WritePixels`、`MakeImageSnapshot`；
- `SetRenderDpi`、`SetRenderFactory`、`Set/GetImageSamplingQuality`；
- `Clone`。

移除（迁移到 `IRenderWindow`）：`PaintAndSwapBuffers`、`GetRenderDC`、`ReleaseRenderDC`、`SetWindowRoundRectRgn`、`SetWindowRectRgn`、`ClearWindowRgn`。

> 与报告原文的差异：报告建议把 `Resize` 也划到窗口接口；但离屏渲染同样需要 `Resize` / `GetWidth` / `GetHeight`，因此这三项保留在 `IRender`，不额外引入 `IRenderSurface` 层。

### 2. `IRenderWindow`（窗口渲染接口，新增）

```cpp
class DUILIB_API IRenderWindow : public virtual IRender
{
public:
    /** 将当前绘制内容刷新到窗口（仅窗口渲染实现） */
    virtual bool PaintAndSwapBuffers(IRenderPaint* pRenderPaint) = 0;

#ifdef DUILIB_BUILD_FOR_WIN
    virtual HDC GetRenderDC(HWND hWnd) = 0;
    virtual void ReleaseRenderDC(HDC hdc) = 0;
    virtual bool SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw) = 0;
    virtual bool SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw) = 0;
    virtual void ClearWindowRgn(bool bRedraw) = 0;
#endif
};
```

窗口层调整：

- `Window::m_render` 改为 `std::unique_ptr<IRenderWindow>`；
- `Window::GetRender()`、`Control::Paint(IRender*)` 仍以 `IRender*` 对外，绘制代码零改动；
- `INativeWindow::OnNativeGetRender()` 返回 `IRenderWindow*`（仅 NativeWindow 内部用于窗口刷新与 RGN）。

## 四、离屏渲染设计

### 1. 类结构

```
Render_Skia                             绘制实现（纯虚 GetSkSurface/GetSkCanvas）
├── Render_Skia_Windows : IRenderWindow Windows 窗口渲染
├── Render_Skia_SDL     : IRenderWindow SDL 窗口渲染
└── Render_Skia_Offscreen : IRender     离屏渲染（Raster / GPU 两种模式）
```

`Render_Skia_Offscreen` 为新增文件（`duilib/RenderSkia/Render_Skia_Offscreen.h/.cpp`）：

- 持有 `sk_sp<SkSurface> m_skSurface`；
- `Resize(w, h)` 重建 Surface；
- `GetWidth/GetHeight` 取自 Surface；
- `GetSkSurface/GetSkCanvas` 直接返回；
- `GetRenderBackendType()` 返回实际使用的后端（Raster 或 GL）；
- `Clone()` 创建同尺寸、同后端的离屏对象，`BitBlt` 复制内容并同步 DPI / 采样质量；
- 拆分后天然不承担任何窗口接口。

### 2. 后端支持确认（重要）

**结论：离屏渲染不只支持 Raster；设计上支持 Raster + GPU 两类后端，采用"Raster 必选 + GPU 可选 + 显式能力查询"的策略。**

| 后端 | 离屏支持 | 条件与说明 |
| --- | --- | --- |
| `kRaster_BackendType` | 所有平台 | 默认后端；`SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h))`，无额外上下文，结果确定 |
| `kNativeGL_BackendType` | 条件支持 | 需要可用的后台 GL 上下文：Windows 用隐藏窗口 + WGL；SDL/Linux 用隐藏窗口 + GL；Linux 还可用 EGL surfaceless。使用 `SkSurfaces::RenderTarget(GrDirectContext*, ...)` |
| `kNativeGL_BackendType`（Windows ARM64） | 不支持 | Skia 上游不提供 ARM64 的 WGL 实现（`MakeGLForWin` 直接返回 `nullptr`），只能回退 Raster |
| Vulkan / Metal | 预留 | 不在本期实现；通过后端上下文工厂扩展，API 不变 |

GL 离屏的关键约束（写入文档与接口注释）：

1. **线程绑定**：GL 上下文是线程私有的；GPU 离屏 Render 必须在创建它的线程上使用和析构。
2. **每线程一个 GL 上下文**：与现有窗口 GL 渲染的约束一致（见 `SkGLWindowContext_Windows::getBackbufferSurface` 的断言）。因此：
   - 同线程已有 GL 窗口渲染时，**复用**该窗口的 `GrDirectContext`（共享上下文）；
   - 同线程没有 GL 窗口渲染时，惰性创建一份**每线程共享的后台 GL 上下文**（隐藏窗口 + WGL/GL，1×1，不显示）；
   - 若无法创建，只能使用 Raster。
3. **快照必须回读**：GPU 离屏在 `MakeImageSnapshot()` / `ReadPixels()` 时通过 `SkSurface::readPixels` 回读到 CPU 的 `IBitmap`，保证快照可跨线程/脱离上下文存活；GPU 离屏不暴露原始像素指针。
4. **性能取向**：GPU 离屏适合大画布、滤镜、模糊等合成；`MakeImageSnapshot` 的回读有额外开销，频繁快照场景优先 Raster。

实现上需要一处小重构：`Render_Skia` 中 `GetPixelBits()`（`peekPixels`）只适用于 Raster。改为统一通过 `SkSurface` 的 `readPixels` / `writePixels` 实现 `ReadPixels` / `WritePixels` / `MakeImageSnapshot`，Raster 与 GPU 共用同一路径；`Clear` 改用 `SkCanvas::clear`，避免依赖像素指针。

### 3. 工厂 API

```cpp
class DUILIB_API IRenderFactory
{
public:
    /** 查询指定后端是否支持离屏渲染 */
    virtual bool IsOffscreenBackendSupported(RenderBackendType backendType) const = 0;

    /** 创建离屏渲染对象（宽高可为 0，表示创建后再 Resize）
    * @param [in] nWidth 画布宽度（物理像素）
    * @param [in] nHeight 画布高度（物理像素）
    * @param [in] spRenderDpi DPI 转换接口（用于线宽、字体等缩放）
    * @param [in] backendType 离屏渲染后端；不支持时返回 nullptr，不做静默回退
    * @param [in] alphaType 位图 Alpha 类型
    * @return 成功返回离屏渲染对象；后端不可用、尺寸非法或内存不足时返回 nullptr
    */
    virtual std::unique_ptr<IRender> CreateOffscreenRender(
        uint32_t nWidth, uint32_t nHeight,
        const IRenderDpiPtr& spRenderDpi = nullptr,
        RenderBackendType backendType = RenderBackendType::kRaster_BackendType,
        AlphaType alphaType = AlphaType::kPremul) = 0;

    /** 创建窗口渲染对象（Window 使用） */
    virtual IRenderWindow* CreateWindowRender(
        const IRenderDpiPtr& spRenderDpi,
        void* platformData,
        RenderBackendType backendType = RenderBackendType::kRaster_BackendType) = 0;

    /** 兼容入口：platformData 非空 → 窗口渲染；为空 → Raster 离屏渲染（0×0，随后 Resize） */
    virtual IRender* CreateRender(
        const IRenderDpiPtr& spRenderDpi,
        void* platformData = nullptr,
        RenderBackendType backendType = RenderBackendType::kRaster_BackendType) = 0;
};
```

兼容策略：保留 `CreateRender`，内部按 `platformData` 分派，避免改动 examples、`BitmapHepler`、CEF 等 20 余处调用点；窗口层内部改用 `CreateWindowRender`；离屏场景逐步迁移到 `CreateOffscreenRender`。

显式请求 GPU 时，如果上下文不可用则返回 `nullptr`，由调用方先 `IsOffscreenBackendSupported()` 判断或降级到 Raster，避免隐藏的能力差异。

### 4. 使用示例

```cpp
// 1) 默认 Raster 离屏：绘制 → 快照 → 绘制到窗口
auto pOffscreen = pRenderFactory->CreateOffscreenRender(200, 100, GetRenderDpi());
if (pOffscreen) {
    pOffscreen->Clear(UiColor(0, 0, 0, 0));
    pOffscreen->FillRoundRect(UiRectF(0, 0, 200, 100), 8, 8, UiColor(0xFF3B82F6));
    std::unique_ptr<IBitmap> pImage(pOffscreen->MakeImageSnapshot());
    pWindowRender->DrawImage(pImage.get(), UiRect(10, 10, 210, 110));
}

// 2) 显式选择 GPU 后端（不可用时自行降级）
RenderBackendType backend = RenderBackendType::kNativeGL_BackendType;
if (!pRenderFactory->IsOffscreenBackendSupported(backend)) {
    backend = RenderBackendType::kRaster_BackendType;
}
auto pGpuOffscreen = pRenderFactory->CreateOffscreenRender(1024, 1024, GetRenderDpi(), backend);
```

### 5. 边界与约束

| 项目 | 约定 |
| --- | --- |
| 像素格式 | ARGB8888 预乘（`AlphaType::kPremul`），与现有 `IBitmap` 一致 |
| 尺寸语义 | 宽高为物理像素；DPI 只影响线宽、字体等缩放，不自动放大画布 |
| 尺寸保护 | `w/h > 0` 且 `w*h*4` 不溢出，单边上限 16384；非法/超大返回 `nullptr` |
| 线程 | Raster：实例可跨线程创建/使用，但同一实例仍建议单线程；GPU：实例绑定创建线程 |
| 生命周期 | 由 `unique_ptr` 管理；GPU 实例必须在创建线程析构；快照 `IBitmap` 可跨线程 |
| 后端 | 默认 Raster；GL 后端需通过能力查询，Windows ARM64 不支持 GL |
| 与窗口关系 | 离屏对象不实现 `IRenderWindow`，编译期即无法调用窗口接口 |

## 五、单元测试用例（离屏渲染）

新增 `tests/test_offscreen_render.cpp`，注册到 CMake / ctest（测试总数 11 → 12）。沿用现有测试骨架：`GlobalManager::Startup(LocalFilesResParam(...))` + `RenderFactory_Skia` + `Check(condition, message)` 计数；编译期宏 `DUILIB_TEST_RESOURCE_DIR`。

用例编号前缀：`OS-` = Offscreen。

### A. 基础创建与尺寸（Raster）

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-01 | 正常创建 | `CreateOffscreenRender(200, 100)` | 非空；`GetWidth()==200`、`GetHeight()==100`；`GetRenderBackendType()==kRaster` |
| OS-02 | 0×0 懒创建 | `CreateOffscreenRender(0, 0)` | 非空；宽高为 0；`Resize(64, 64)` 返回 true 且宽高更新 |
| OS-03 | 重新 Resize | 创建 32×32 → 绘制 → `Resize(64, 48)` | 返回 true；宽高更新；旧内容不保留（新 Surface） |
| OS-04 | 非法尺寸 | `CreateOffscreenRender(0, 100)` 后 `Resize(-1, 10)`、`Resize(10, 0)` | `Resize` 返回 false，宽高保持原值 |
| OS-05 | 超大尺寸 | `CreateOffscreenRender(20000, 20000)` | 返回 nullptr（或按配置上限拒绝），不崩溃、不 OOM |
| OS-06 | 未初始化就绘制 | 0×0 对象调用 `Clear` / `FillRect` / `MakeImageSnapshot` | 不崩溃；快照返回 nullptr |

### B. 绘制与像素正确性（Raster）

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-07 | Clear | `Clear(红)` 后 `ReadPixels` | 四角与中心像素均为红（ARGB 精确匹配） |
| OS-08 | 图元位置 | 先 `Clear(白)`，再 `FillRect((10,10)-(20,20), 蓝)` | 矩形内为蓝；矩形外仍为白 |
| OS-09 | 坐标原点 | 在 `(0,0)` 画 1×1 像素 | `ReadPixels` 首像素为该颜色，验证左上原点 |
| OS-10 | 裁剪 | `ClipRect((0,0)-(50,50))` 后绘制整屏红 | 裁剪区内红、区外保持背景色 |
| OS-11 | 图层合成 | 白底 + `SaveLayer` 内画不透明蓝 + `alpha=128` | 合成后 R/G 落在 0x70–0x90 区间，B==0xFF |
| OS-12 | 文本 | 绘制 "Hello" 并用 `MeasureString` 对比 | 绘制区域宽度与测量宽度误差 ≤ 2 px（或不作像素断言，仅验证不崩溃） |

### C. 快照

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-13 | 整图快照 | 绘制后 `MakeImageSnapshot()` | 非空；宽高一致；抽样像素与源一致 |
| OS-14 | 局部快照 | `MakeImageSnapshot(UiRect(10, 10, 30, 30))` | 返回 20×20；首像素对应源 `(10,10)` |
| OS-15 | 越界局部快照 | 请求超出画布的区域 | 自动裁剪到画布内；完全越界返回 nullptr |
| OS-16 | 快照独立性 | 快照后再把源画布改为绿色 | 已生成的 `IBitmap` 仍为原内容（深拷贝） |
| OS-17 | 快照用于绘制 | 快照 `DrawImage` 到另一个离屏 Render | 目标区域像素与源一致（位置偏移正确） |

### D. Clone

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-18 | Clone 基本 | 原对象绘制后 `Clone()` | 尺寸、后端、抽样像素一致 |
| OS-19 | Clone 独立性 | Clone 后修改原对象 | Clone 内容不变 |
| OS-20 | Clone 属性 | 设置 DPI、`ImageSamplingQuality=kLinear` 后 Clone | Clone 的 DPI/采样质量与原对象一致 |

### E. 位图直绘（阶段 3，可选）

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-21 | 直绘到 IBitmap | `CreateBitmapRender(pBitmap)` 后绘制 | `pBitmap->LockPixelBits()` 立即可见绘制结果（零拷贝） |
| OS-22 | 位图尺寸不匹配 | 传入未初始化的 `IBitmap` | 返回 nullptr，不崩溃 |

### F. 多后端

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-23 | 能力查询 | `IsOffscreenBackendSupported(kRaster)` | 恒为 true |
| OS-24 | GPU 能力查询 | `IsOffscreenBackendSupported(kNativeGL)` | 返回平台实际能力；Windows ARM64 为 false |
| OS-25 | GPU 离屏冒烟 | 能力为 true 时创建 GL 离屏，执行 OS-07/OS-08/OS-13 | 结果与 Raster 一致（像素允许 1/255 误差） |
| OS-26 | GPU 不可用 | 能力为 false 时 `CreateOffscreenRender(..., kNativeGL)` | 返回 nullptr，不静默回退 |
| OS-27 | GPU 快照回读 | GL 离屏 `MakeImageSnapshot()` | 返回 CPU 位图；销毁 Render 后位图仍可读 |
| OS-28 | GPU 线程约束 | 在创建线程析构 GL 离屏 | 无断言/崩溃；跨线程只传递快照位图 |

无 GPU 或 CI 无显示环境时，OS-24～OS-28 打印 `[SKIP]` 并计入跳过数，不影响退出码。

### G. 线程与生命周期

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-29 | 子线程 Raster | 在 `std::thread` 内创建/绘制/快照/析构 Raster 离屏 | 全程无断言、无崩溃，快照数据正确 |
| OS-30 | 重复创建销毁 | 循环 200 次创建 16×16 → 绘制 → 快照 → 析构 | 无泄漏（配合 CRT 泄漏检测）、无崩溃 |
| OS-31 | 快照跨线程 | 主线程创建快照，子线程读取 `LockPixelBits()` | 数据可读且内容正确 |

### H. 迁移回归（阶段 2 完成后）

| 编号 | 场景 | 步骤 | 预期 |
| --- | --- | --- | --- |
| OS-32 | Control 缓存 | 迁移 `Control::CreateTempRender` 后，绘制带 Alpha 的控件 | 输出像素与迁移前一致（基准图对比） |
| OS-33 | 拖拽图 | 迁移 `ControlDragable` 后创建拖拽图 | 尺寸与内容与迁移前一致 |
| OS-34 | 兼容入口 | `CreateRender(spDpi, nullptr)` | 返回非空 Raster 离屏对象，宽高为 0，`Resize` 后可用 |

### 测试文件与构建

```text
tests/test_offscreen_render.cpp     新增用例
tests/CMakeLists.txt                新增 test_offscreen_render 目标 + add_test(NAME offscreen_render ...)
```

结构建议：

```cpp
int main()
{
    // 初始化 GlobalManager / RenderFactory
    TestCreateAndResize();      // OS-01 ~ OS-06
    TestRasterDraw();           // OS-07 ~ OS-12
    TestSnapshot();             // OS-13 ~ OS-17
    TestClone();                // OS-18 ~ OS-20
    TestBackends();             // OS-23 ~ OS-28（含能力跳过）
    TestThreadAndLifetime();    // OS-29 ~ OS-31
    TestCompatEntry();          // OS-34
    // 输出检查总数/失败数，返回失败数 ? 1 : 0
}
```

## 六、兼容与迁移步骤

**阶段 1：接口拆分（无行为变化）**

1. 新增 `IRenderWindow`，从 `IRender` 迁出 6 个窗口方法；
2. `Render_Skia_Windows` / `Render_Skia_SDL` 增加 `IRenderWindow` 基类；
3. `Window`、`INativeWindow::OnNativeGetRender` 调整类型；
4. 工厂新增 `CreateWindowRender`，`CreateRender` 按 `platformData` 分派；
5. 编译并全量回归现有 11 个测试。

**阶段 2：离屏渲染（Raster）**

6. 新增 `Render_Skia_Offscreen` + `CreateOffscreenRender`（Raster）；
7. `Render_Skia` 的像素访问改为 `SkSurface::readPixels/writePixels`，兼容 Raster 与 GPU；
8. 迁移 `Control::CreateTempRender`、`ControlDragable`、`BitmapHepler_Windows/SDL`、`CefControlOffScreen`、测试；
9. 新增 `tests/test_offscreen_render.cpp`，覆盖 A～D、G、H 组用例。

**阶段 3：GPU 后端（可选但已规划）**

10. 每线程后台 GL 上下文（Windows 隐藏窗口 + WGL；SDL 隐藏窗口 + GL；Linux EGL surfaceless）；
11. 同线程已有 GL 窗口渲染时复用其 `GrDirectContext`；
12. `IsOffscreenBackendSupported` + GL 离屏用例（F 组）；
13. 明确 ARM64 Windows 不支持并回退 Raster。

**阶段 4（可选）**

14. `CreateBitmapRender(IBitmap*)` 零拷贝直绘；
15. `Control::CreateSnapshot()` / `Window::Snapshot()` 高层封装；
16. 图片编码导出（需新增编码器）。

## 七、受影响文件清单（预估）

- `duilib/Render/IRender.h`：新增 `IRenderWindow`；`IRenderFactory` 新增 3 个方法；
- `duilib/RenderSkia/RenderFactory_Skia.h/.cpp`：实现新工厂方法；
- `duilib/RenderSkia/Render_Skia_Offscreen.h/.cpp`：新增；
- `duilib/RenderSkia/Render_Skia.h/.cpp`：像素读写改用 `SkSurface`，清理窗口相关声明；
- `duilib/RenderSkia/Render_Skia_Windows.h/.cpp`、`Render_Skia_SDL.h/.cpp`：改为实现 `IRenderWindow`；
- `duilib/Core/Window.h/.cpp`、`WindowBase.h/.cpp`、`INativeWindow.h`、`NativeWindow_Windows/SDL.cpp`：接口类型调整；
- `duilib/Core/Control.cpp`、`ControlDragable.h`：迁移到 `CreateOffscreenRender`；
- `duilib/Utils/BitmapHepler_Windows/SDL.cpp`、`duilib/CEFControl/CefControlOffScreen.cpp`：迁移；
- `tests/test_offscreen_render.cpp`（新增）、`tests/CMakeLists.txt`；
- `docs/misc/IRender接口分析报告.md`：同步第五节第 1 条状态。

## 八、待确认决策

1. GPU 离屏的上下文策略：**复用同线程窗口 GL 上下文 + 无窗口时惰性创建每线程后台上下文**（推荐） vs 只允许复用、无窗口时不支持 GPU。
2. `CreateOffscreenRender` 显式请求 GPU 失败时：**返回 nullptr**（推荐，能力透明） vs 静默回退 Raster。
3. 位图直绘（`CreateBitmapRender`）是否纳入本期（阶段 3/4）。
4. 是否同期提供 `Control::CreateSnapshot()` / `Window::Snapshot()` 高层 API。
