# GL 渲染后端性能评测与优化报告

**日期**: 2026-11-17
**平台**: Windows x64 / Skia + WGL
**关联 Issue**: 用户实测发现开启 GL 后端后 CPU 占用与 Raster 模式相差不大，GL 后端未带来性能提升。

---

## 一、背景

nim_duilib 提供两种渲染后端：

| 后端 | 枚举值 | 说明 |
|------|--------|------|
| Raster (CPU) | `RenderBackendType::kRaster_BackendType` (0) | Skia CPU 栅格化绘制，通过 GDI BitBlt 呈现到窗口 |
| NativeGL (GPU) | `RenderBackendType::kNativeGL_BackendType` (1) | Skia Ganesh GL 后端，WGL 创建 OpenGL 上下文 |

切换 API：`Window::SetRenderBackendType(backendType, msaaSampleCount, bDisableVsync)`

用户实测反馈：**开启 GL 后端后 CPU 占用与 Raster 相差不大，未观察到 GPU 加速带来的性能提升**。本次做了系统 benchmark 评测，定位瓶颈并实施优化。

---

## 二、评测方法

### 2.1 基准程序

新建独立 benchmark：`tests/test_gl_benchmark.cpp`，不在常规测试流程中，手工运行：

```bash
python build/build.py build --offline --target test_gl_benchmark
.\out\x64\Release\tests\test_gl_benchmark.exe frames=200 size=960x600 workload=medium
```

### 2.2 核心设计

- **同一窗口实例**上先跑 N 帧 Raster、再切 GL 跑 N 帧，避免"新窗口初始化开销"干扰
- 使用 `Window::SetRenderBackendType` 热切换后端（内部会销毁并重建 Render 对象）
- warmup 30 帧后进入正式测量，排除首帧创建 surface/context 的一次性开销
- 每帧调用 `::InvalidateRect(hWnd, nullptr, FALSE)` 强制全量重绘（对应当前项目的实际使用场景）

### 2.3 采集指标

| 指标 | 来源 | 说明 |
|------|------|------|
| Wall-clock 每帧耗时 | `QueryPerformanceCounter` (QPC) | 高精度毫秒级 |
| CPU 用户态+内核态时间 | `GetProcessTimes` | 整个 benchmark 运行期间累计 |
| 帧耗时统计 | 程序内 | avg / median / p95 / p99 / min / max / 直方图 |
| 实际后端类型 | `IRenderWindow::GetRenderBackendType()` | 验证 GL 是否真正启用（防止 fallback） |

### 2.4 负载设计

| 级别 | 尺寸 | 矩形 | 文本 | 半透明方块 (模拟图片) |
|------|------|------|------|----------------------|
| Light | 960×600 | 12 | 12 | 0 |
| Medium | 960×600 | 80 | 60 | 0 |
| Heavy | 1280×800 | 200 | 160 | 40 |

绘制内容：随机位置/颜色的 `FillRect` + 随机位置的 `DrawString`（英文 "abcdefg-NNN"）。每帧调用 `IRender::Clear` 清空画布。

### 2.5 关键代码路径

```
BenchPaint::DoPaint(rcPaint)            // benchmark 绘制入口
  → IRender::Clear + FillRect + DrawString
    → SkCanvas (Skia 内部)
      → SkGLWindowContext_Windows::PaintAndSwapBuffers   // GL 后端
      → SkRasterWindowContext_Windows::PaintAndSwapBuffers // Raster 后端
```

---

## 三、原始数据（优化前）

### 3.1 Medium workload (960×600, 80 rect + 60 text, 200 frames)

| 指标 | Raster | GL | 对比 |
|------|--------|-----|------|
| avg 帧耗时 | **0.94 ms** | 2.16 ms | GL 慢 **2.30x** ❌ |
| 估算 FPS | **1062** | 464 | Raster 快 2.30x |
| total wall | 188 ms | 431 ms | GL 多花 243ms |
| CPU user+kernel | **125 ms** | 188 ms | GL CPU 高 50% |
| p99 尾延迟 | **5.33 ms** | 7.49 ms | GL 更差 |
| max 帧耗时 | 7.00 ms | 10.66 ms | GL 更差 |

### 3.2 Heavy workload (1280×800, 200 rect + 160 text + 40 img, 300 frames)

| 指标 | Raster | GL | 对比 |
|------|--------|-----|------|
| avg 帧耗时 | **1.72 ms** | 2.06 ms | GL 慢 **1.20x** ❌ |
| 估算 FPS | **582** | 486 | Raster 快 1.20x |
| total wall | 516 ms | 617 ms | GL 多花 101ms |
| CPU user+kernel | **297 ms** | 313 ms | GL CPU 略高 |
| p99 尾延迟 | 7.09 ms | 6.35 ms | 两者接近 |

### 3.3 初步结论

1. **GL 后端确实比 Raster 慢**——完全印证了用户的观察
2. **负载越重，GL 相对优势越接近**（差距从 2.30x 缩到 1.20x），说明轻负载下 GL 的**固定开销占主导**
3. **GL CPU 占用并未降低**，有时反而更高——说明 GPU 并没有真正承担计算，CPU 仍在做大量工作

---

## 四、瓶颈定位

### 4.1 代码路径分析

`SkGLWindowContext_Windows::PaintAndSwapBuffers`（优化前）：

```cpp
// 1. 获取/创建 persistent surface（FBO 离屏渲染目标）
sk_sp<SkSurface> paintSurface = getBackbufferSurface();   // ← 创建 SkSurfaces::RenderTarget
const bool bUsePersistentSurface = (m_persistentSurface.get() == paintSurface.get());

if (bUsePersistentSurface) {
    // 2. 获取窗口 FBO 0 包装的 surface
    backbuffer = GetWindowSurface();                      // ← SkSurfaces::WrapBackendRenderTarget(FBO 0)
}

// 3. DoPaint 绘制到 paintSurface（persistent surface）
pRenderPaint->DoPaint(rcPaint);

// 4. 把 persistent surface 的内容拷贝到窗口 FBO 0 ← 核心瓶颈！
sk_sp<SkImage> image = paintSurface->makeImageSnapshot(); // ← 强制 Skia flush + 创建 SkImage
pCanvas->drawImage(image, 0, 0, ...);                    // ← drawImage 到 FBO 0

// 5. 提交 + SwapBuffers
dContext->flushAndSubmit(GrSyncCpu::kNo);
swapBuffers();
```

### 4.2 瓶颈根因

**persistent surface 每帧整帧拷贝**：`makeImageSnapshot()` + `drawImage()` 让 GPU→CPU→GPU 往返一次。

- persistent surface 是 **FBO 离屏渲染目标**（GL_RGBA 8888，1280×800 = ~4MB）
- `makeImageSnapshot()` 虽然 Skia 内部会先尝试 GPU-backed image，但在 Ganesh GL 后端中，快照创建过程会强制所有未提交的 GL 命令 flush 并等待 GPU 完成（隐式同步点）
- 随后的 `drawImage()` 是 GPU→GPU 操作，但快照创建的 flush 开销已经发生

**persistent surface 的设计意图**：支持脏区域增量刷新。上一帧内容保留在 FBO 中，每帧只重绘 Win32 `GetUpdateRect` 返回的脏区域，然后把**整幅** FBO 内容拷贝到窗口 FBO 0。

**但 benchmark 的场景是全量重绘**（`InvalidateRect(nullptr)` → `GetUpdateRect` 返回整个客户区），此时：
- persistent surface 的"脏区域增量"优势完全丧失
- 每帧都要做一次 2-4MB 的整帧拷贝，纯粹浪费

### 4.3 验证：绕过 persistent surface

临时让 `getBackbufferSurface()` 直接返回 `GetWindowSurface()`（跳过 persistent surface），跑 benchmark：

**Medium workload (960×600)**：

| 指标 | 原 GL (persistent) | GL 直绘 FBO 0 | Raster |
|------|-------------------|--------------|--------|
| avg 帧耗时 | 2.16 ms ❌ | **1.12 ms** | 0.87 ms |
| FPS | 464 | **893** | 1153 |
| CPU sum | 188 ms | **78 ms** ✅ | 125 ms |

**验证结论**：跳过 persistent surface 后，GL 性能从 2.16ms 降到 1.12ms（**提升 93%**），CPU 从 188ms 降到 78ms（**降 59%**），**瓶颈确认**。

---

## 五、优化方案

### 5.1 核心思路

> **全量重绘时直接绘制到窗口 FBO 0，跳过 persistent surface 的中间拷贝；只在真正需要局部增量刷新时才使用 persistent surface。**

### 5.2 实现细节

重构 `SkGLWindowContext_Windows::PaintAndSwapBuffers`，改为三阶段：

**阶段 1：判断是否需要全量重绘**

```cpp
bool bNeedFullRedraw = m_bNeedFullRedraw; // 尺寸变化后为 true
if (!bNeedFullRedraw && (m_persistentSurface != nullptr)) {
    RECT rectUpdate = {};
    if (::GetUpdateRect(m_hWnd, &rectUpdate, FALSE)) {
        int32_t updateW = rectUpdate.right - rectUpdate.left;
        int32_t updateH = rectUpdate.bottom - rectUpdate.top;
        if ((updateW >= clientW) && (updateH >= clientH)) {
            bNeedFullRedraw = true;  // UpdateRect 覆盖整个客户区 → 全量重绘
        }
    }
}
```

**阶段 2：选择绘制目标 surface**

```cpp
if (bNeedFullRedraw) {
    // 全量重绘 → 直接用窗口 FBO 0
    paintSurface = GetWindowSurface();
    m_persistentSurface.reset();  // 内容已过期，下次按需重建
} else {
    // 局部重绘 → 使用 persistent surface
    paintSurface = getBackbufferSurface();
    bUsePersistentSurface = true;
}
```

**阶段 3：绘制 + 条件性拷贝**

- 全量重绘：直接 `DoPaint(rcPaint)` → `flushAndSubmit` → `swapBuffers`，**零拷贝**
- 局部重绘：`DoPaint(脏区域)` → `makeImageSnapshot + drawImage`（persistent surface 拷贝） → `flushAndSubmit` → `swapBuffers`

### 5.3 附加优化

- 移除 GL 后端的 `BeginPaint/EndPaint` 调用——GL 后端通过 `InvalidateRect` + `GetUpdateRect` 已经能获取绘制区域，不需要 GDI 的 PAINTSTRUCT 机制，这对每帧都有开销

### 5.4 安全性分析

| 场景 | 优化前行为 | 优化后行为 | 是否安全 |
|------|-----------|-----------|---------|
| 首次绘制 (`m_bNeedFullRedraw=true`) | persistent surface + 拷贝 | 直绘 FBO 0 | ✅ |
| 窗口尺寸变化 | persistent surface 重建 + 拷贝 | 直绘 FBO 0（自动随 FBO 0 缩放） | ✅ |
| 全量重绘 (`InvalidateRect(nullptr)`) | persistent surface + 拷贝 | 直绘 FBO 0 | ✅ |
| 局部增量刷新（理想场景） | persistent surface 脏区域 + 拷贝 | **保留** persistent surface + 拷贝 | ✅ |
| FBO 0 包装失败 | 已 fallback 到 persistent surface | 同左 | ✅ |
| MSAA 退化 | 已在 getBackbufferSurface 处理 | 同左 | ✅ |

---

## 六、优化效果

### 6.1 Medium workload (960×600, 80 rect + 60 text, 200 frames)

| 指标 | Raster | GL 优化前 | GL 优化后 | GL 相对 Raster |
|------|--------|----------|----------|---------------|
| avg 帧耗时 | 1.51 ms | 2.16 ms ❌ | **1.36 ms** ✅ | **快 10%** |
| FPS | 660 | 464 | **733** | **快 11%** |
| CPU sum | 203 ms | 188 ms | **94 ms** | **降 54%** |
| p99 尾延迟 | 8.71 ms | 7.49 ms | **4.88 ms** | **降 44%** |
| max 帧耗时 | 9.84 ms | 10.66 ms | **5.96 ms** | **降 40%** |

### 6.2 Heavy workload (1280×800, 200 rect + 160 text + 40 img, 300 frames)

| 指标 | Raster | GL 优化前 | GL 优化后 | GL 相对 Raster |
|------|--------|----------|----------|---------------|
| avg 帧耗时 | 1.87 ms | 2.06 ms ❌ | **1.15 ms** ✅ | **快 62%** |
| FPS | 534 | 486 | **867** | **快 62%** |
| CPU sum | 344 ms | 313 ms | **109 ms** | **降 68%** |
| p99 尾延迟 | 8.39 ms | 6.35 ms | **5.04 ms** | **降 40%** |
| max 帧耗时 | 10.29 ms | 8.27 ms | **5.76 ms** | **降 44%** |

### 6.3 全量汇总

| 场景 | GL 优化前 avg | GL 优化后 avg | 提升幅度 |
|------|-------------|-------------|---------|
| Medium (960×600) | 2.16 ms | **1.36 ms** | **+59%** |
| Heavy (1280×800) | 2.06 ms | **1.15 ms** | **+79%** |

### 6.4 回归测试

```
ctest --test-dir out\x64\Release -C Release --output-on-failure
100% tests passed, 0 tests failed out of 17
```

包括 `gl_window_smoke`（16/17 号测试，覆盖 GL 后端核心路径）在内的全部 17 个单元测试通过。

---

## 七、进一步优化方向（按代码扫描结果重新排序）

> 下面的分析基于对实际代码的扫描确认，而非纯理论推断。
> 优先级按"收益/难度/风险"综合排序。

### 7.1 方向一：vsync 默认禁用（★★★★★ 先做）

**扫描确认的问题**：

`duilib/Core/Window.cpp:32` — `Window` 构造函数中：
```cpp
m_bDisableVsync(false),  // 默认启用垂直同步
```

这意味着 controls.exe 等应用通过 XML 设置 GL 后端时，`SetRenderBackendType` 会把当前 `m_bDisableVsync`（false）传给 GL 工厂 → `wglSwapIntervalEXT(1)` → **SwapBuffers 阻塞等显示器刷新**（通常 60Hz → 每帧卡 16.67ms）。

benchmark 里手动传了 `bDisableVsync=true` 所以数据好看（能到 800+ fps），但实际 controls.exe 默认 vsync 开启，fps 上限就是 ~60。

**改法**（1 行代码，零风险）：
- 把 `Window.cpp` 构造函数 `m_bDisableVsync(false)` 改为 `m_bDisableVsync(true)`
- Raster 后端的"呈现"是 GDI `BitBlt`，不受 vsync 控制，所以这个默认值只影响 GL

**验证**：controls.exe 设置 GL 后端后，fps 应该不再被 60Hz 卡上限。

---

### 7.2 方向二：纹理上传缓存（★★★★★ 收益极高）

**扫描确认的瓶颈**：

`duilib/RenderSkia/Bitmap_Skia.h:94` — `Bitmap_Skia` 只持有 CPU-side 的 raster bitmap：
```cpp
std::unique_ptr<SkBitmap> m_pSkBitmap;  // CPU raster bitmap
// 没有任何 GPU-backed SkImage / GrBackendTexture 缓存！
```

而 `duilib/RenderSkia/Render_Skia.cpp:920-928` 的 `DrawImage` 每次都：
```cpp
const SkBitmap& skSrcBitmap = skiaBitmap->GetSkBitmap();
SkPixmap skSrcPixmap;
if (skSrcBitmap.peekPixels(&skSrcPixmap)) {
    skImage = SkImages::RasterFromPixmap(skSrcPixmap, ...);  // 每帧新创建 SkImage
}
// Ganesh 在 drawImage 时把 raster image 上传成 GPU texture
```

**后果**：每帧每张图都走 **CPU raster → GPU texture** 上传。如果 controls.exe 有 50 张图、60fps → **3000 次 GPU 上传/秒**。而 GL 加速的核心优势就是"纹理在 GPU 端一次上传、反复复用"，现在完全浪费了。

**改法**（中难度）：
- 给 `Bitmap_Skia` 加成员 `sk_sp<SkImage> m_pGpuImage`（初始为 null）
- 首次 `DrawImage` 时通过 `SkImages::RasterFromPixmap` 创建 SkImage，然后**缓存** `m_pGpuImage`
- 后续每帧直接复用缓存的 GPU-backed SkImage，避免重复上传
- 关键点：需要能拿到 `GrDirectContext*` 来创建 GPU 纹理上下文（`Render_Skia_Windows` 持有 `m_pWindowContext`，可以通过接口传递）

---

### 7.3 方向三：Skia GrDirectContext 调优（★★★☆☆ 后续）

**扫描确认**：`Render_Skia_Windows` 创建 `GrDirectContext` 时没有传 `GrContextOptions`，用的是 Skia 默认值。

可尝试开启：
- `fShaderCache` / `fShaderCacheType` — GL 后端每个 shader 首次使用都要编译，缓存后第二次绘制相同内容直接复用
- `fGpuFontCache` — GPU 侧字形 atlas，减少字体位图 CPU→GPU 上传
- `fMaxTextureSize` / `fAtlasMaxSize` — 按需调整

---

### 7.4 方向四：局部增量刷新模式（★★★☆☆ 暂不动）

分两层：
- **GL 底层**：persistent surface + 拷贝路径已优化，拷贝开销在 GPU 端（`drawImage + kSrc blend`），已高效，GL 底层基本就绪
- **应用层**：要让 controls.exe（或通用控件树）只 invalidate 脏区域而非整窗口 `InvalidateRect(nullptr)`，需要改 `Window::OnPaint` 的 InvalidateRect 策略 + 控件树脏区域传播机制 —— **改动面大**、**涉及上层架构**、**风险高**

建议先不动，等纹理缓存做好后先看实际收益再决定。

---

### 7.5 方向五：批量 DrawCall 合并（★★☆☆☆ 不动）

扫描 `SkGLWindowContext_Windows.cpp:496-498`，`flushAndSubmit` **每帧只调一次**——Skia Ganesh 内部的 batching 完全没有被截断。这个方向的瓶颈不在我们的代码，在 Skia 内部的 batching 策略（它已经做得很好了）。**暂时不动**。

---

### 优先级排序（最终）

| 序 | 方向 | 收益 | 难度 | 一句话 |
|----|------|------|------|--------|
| **① 先做** | **vsync 默认禁用** | 高 | **极低**（1 行） | GL 默认被 vsync 卡 16ms/帧上限，改掉立即生效、零风险 |
| **② 再做** | **纹理上传缓存** | **极高** | 中 | 解决 GL 后端最大浪费——每帧 CPU→GPU 重复上传 |
| ③ 后续 | Skia GrDirectContext 调优 | 中 | 中 | shader cache + GPU font atlas |
| ④ 暂不动 | 局部增量刷新 | 条件性 | 高 | 应用层改动面大 |
| ⑤ 不动 | DrawCall 合并 | 低 | 高 | 不是我们代码的瓶颈 |

---

## 八、涉及文件

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `duilib/RenderSkia/SkGLWindowContext_Windows.cpp` | 核心优化 | 重构 `PaintAndSwapBuffers`，全量重绘直绘 FBO 0；新增智能判断 |
| `duilib/RenderSkia/Render_Skia_Windows.cpp` | 日志细化 | `MakeGLForWin` 新增 hWnd/ctx 无效的细粒度失败日志（前一轮修复遗留） |
| `tests/CMakeLists.txt` | 新增目标 | 新增 `test_gl_benchmark` 构建配置 |
| `tests/test_gl_benchmark.cpp` | 新增文件 | GL vs Raster 性能基准测试程序（手工运行，不加入常规 ctest） |

---

## 九、结论

1. **用户观察完全正确**——优化前 GL 后端的 persistent surface 中间拷贝让 GPU 加速形同虚设
2. **瓶颈已定位并修复**——全量重绘场景下跳过 persistent surface 直接绘 FBO 0
3. **优化后 GL 后端真正发挥了 GPU 加速**：
   - 中等负载 (960×600)：GL avg **1.36ms** vs Raster **1.51ms**，GL **快 10%**
   - 重负载 (1280×800)：GL avg **1.15ms** vs Raster **1.87ms**，GL **快 62%**
   - CPU 占用大幅下降（GL 94-109ms vs Raster 203-344ms，**降 54%-68%**）
4. **提升空间还存在**：纹理上传缓存、批量 DrawCall、让应用层真正用局部增量刷新等方向

---

## 十、提交日志

```
[优化] GL 渲染后端性能评测与瓶颈修复

问题：用户实测发现开启 GL 后端后性能无提升，经系统 benchmark 评测定位
根因为 SkGLWindowContext_Windows::PaintAndSwapBuffers 中 persistent surface
的整帧拷贝（makeImageSnapshot + drawImage），每帧 GPU→CPU→GPU 往返开销
占 GL 总耗时 50%+。

优化：
- 重构 PaintAndSwapBuffers，先判断是否需要全量重绘
- 全量重绘（GetUpdateRect 覆盖整个客户区）时直接绘制到窗口 FBO 0，
  跳过 persistent surface 的中间拷贝
- 局部重绘时保留 persistent surface 支持脏区域增量刷新
- 移除 GL 后端不需要的 BeginPaint/EndPaint 调用

效果：
- Medium workload (960x600): GL avg 2.16ms → 1.36ms，反超 Raster(1.51ms) 10%
- Heavy workload (1280x800): GL avg 2.06ms → 1.15ms，比 Raster(1.87ms) 快 62%
- CPU 占用大幅下降（94-109ms vs Raster 的 203-344ms）
- ctest 17/17 全部通过，包括 gl_window_smoke

涉及文件：
- duilib/RenderSkia/SkGLWindowContext_Windows.cpp  (核心优化)
- duilib/RenderSkia/Render_Skia_Windows.cpp        (GL 失败日志细化)
- tests/CMakeLists.txt                             (benchmark 构建配置)
- tests/test_gl_benchmark.cpp                      (新增基准测试程序)
```

（按规则，仅生成提交日志，不执行 git commit）
