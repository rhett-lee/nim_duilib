下面的分析基于 `duilib/Render/IRender.h`，并对照 Skia 实现（`duilib/RenderSkia/Render_Skia.cpp`、`Bitmap_Skia.h`、`Path_Skia.h`、`DrawSkiaImage.cpp`）核实。

> **文档状态（2026-11-14 更新）**：本报告已随"字体回退性能优化"（见 [字体回退性能优化分析.md](字体回退性能优化分析.md)）重新校准；2026-11-14 完成了第四节"接口一致性问题"的全部 8 条（第 1、2、5、6、7、8 条：`Clone` 家族、只读 const、测量返回值、魔法值、文本间距命名、取值风格；第 3、4 条：裁剪接口改名、`MeasureRichText` 重载合并），完成第五节第 2 条（富文本接口去掉 `IRenderFactory*` 参数）、第 3 条（`rcPaint` 后移为可选参数并内部兜底）与第 5 条（三个绘图接口合并为两个入口 + `ImageDrawParam`），并补齐第六节 P0/P1/P2 的全部缺失能力（`FillArc`、`DrawEllipse`/`FillEllipse`、`DrawPolygon`/`FillPolygon`、`ClipPath`、`ImageSamplingQuality`；`IFont` 度量/宽度/信息、`IBitmap` 只读查询、`MeasureString` 轻量重载、局部快照、`SaveLayer`/`RestoreLayer`；`IMatrix`/`IPen`/`IBrush` 扩展、`BlendMode`、`SetTransform`、`CreateBitmap` 重载）。文中代码行号以当前代码为准；每条问题均标注当前处理状态（已修复 / 部分修复 / 未处理 / 暂不处理）；"零、进度总览"与"九、字体回退相关接口变更记录"便于后续跟进。

## 零、进度总览（2026-11-14）

字体回退优化以渲染内部实现为主，但同时调整了 `IFont` / `IFontMgr` / `IFallbackFontMgr` 三个公共接口（见第九节），并顺带修复了本报告的部分"必须改"项；2026-11-14 已集中处理完第四节的 8 条接口一致性问题。当前状态：

| 编号 | 问题 | 位置（当前行号） | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| 二.1 | `RopMode` 只实现 `kSrcCopy` | `IRender.h#L526-L542` | **已修复** | 四种模式均已映射；`kSrcAnd` 为近似实现，已在枚举注释中写明 |
| 二.2 | `WritePixels` 源数据应为只读 | `IRender.h#L1372-L1385` | **已修复** | 两个重载均已改为 `const void*` |
| 二.3 | `uFade` 覆盖颜色 Alpha | `IRender.h#L844` | **已修复** | 统一为"与颜色 Alpha 相乘"，绘制类接口全部生效 |
| 二.4 | 注释与参数名错误 | 见第二节第 4 条 | **已修复** | `ClearClip` / `MeasureRichText` / `HasFontName` / `CreateBrush` 均已修正 |
| 三.1 | `BitmapAlphaType` 采用 Skia 命名 | `IRender.h#L136-L160` | **已修复** | 改为中性 `AlphaType::{kUnknown,kOpaque,kPremul,kUnpremul}` |
| 三.2 | 字形 ID 泄漏到公共接口 | `IRender.h#L46-L74` | **已修复** | 公共接口不再出现 `SkGlyphID`（详见第九节） |
| 三.3 | `RenderType` 是"假抽象" | — | **已修复** | `RenderType` 与 `GetRenderType()` 已删除 |
| 三.4 | `RichTextData::m_textView` 编码不一致 | `IRender.h#L575-L577` | **暂不处理** | 与内部实现耦合，收益不明显 |
| 三.5 | Windows 窗口语义混入通用接口 | `IRender.h#L1338-L1347`、`#L1680-L1701` | **已修复** | RGN 三个接口已加 `DUILIB_BUILD_FOR_WIN`；非 Windows 平台不再暴露，`NativeWindow_SDL` 侧同步做条件编译 |
| 三.6 | 注释中直接写引擎名 | 见第三节第 6 条 | **已修复** | `IRender.h` 的 `RopMode`、`SystemNotification.h` 已改为行为/平台描述，公共头文件普查完成 |
| 四.1 | `Clone` 家族所有权与 const 不统一 | `IRender.h#L185`、`#L280`、`#L289`、`#L424`、`#L1367` | **已修复** | 五个 `Clone` 统一为 `std::unique_ptr<T> Clone() const` |
| 四.2 | 只读接口缺 const | 见第四节第 2 条 | **已修复** | 只读接口/只读指针参数全部补 const（含 `IsUnicodeCharSupported`、`BitBlt`） |
| 四.3 | 两套裁剪状态机制并存 | `IRender.h#L900-L921` | **已修复** | 保留 `SaveClip/RestoreClip`；`SetClip→ClipRect`、`SetRoundClip→ClipRoundRect`；删除 `ClearClip` |
| 四.4 | `MeasureRichText*` 编号式重载膨胀 | `IRender.h#L1176` | **已修复** | 合并为单个 `MeasureRichText`，可选输出参数默认 `nullptr` |
| 四.5 | 返回值约定不一致 | `IRender.h#L1176` | **已修复** | `MeasureRichText` 返回 `UiRect`（宽度=内容最大行宽，高度=内容总高度） |
| 四.6 | "未设置"用魔法值表达 | `IRender.h#L826-L829` | **已修复** | `rectSize` 拆分为 `limitWidth` / `limitHeight`，`-1` 表示不限制 |
| 四.7 | 文本间距模型分裂 | `IRender.h#L835`、`#L851`、`#L604` | **已修复** | `Measure/DrawStringParam::fLetterSpacing` 与 `RichTextData::m_fLetterSpacing` 命名统一 |
| 四.8 | 取值风格不一致 | `IRender.h#L24`、`#L90` | **已修复** | `IFontMgr::GetFontName` 改为按值返回 `DString` |
| 五.1 | `IRender` 职责过载 | 全文 | **已修复** | 拆分为 `IRender`（绘制）与 `IRenderWindow`（窗口刷新/平台DC/窗口形状）；新增离屏渲染 `CreateOffscreenRender`（Raster + GL） |
| 五.2 | 富文本接口每次传 `IRenderFactory*` | `IRender.h#L1170-L1199`、`#L1373` | **已修复** | Render 创建时注入 factory（`SetRenderFactory`），三个富文本接口不再传该参数 |
| 五.3 | `rcPaint` 脏区参数 | `IRender.h#L859-L1019`、`Render_Skia.cpp#L2168` | **已修复** | `rcPaint` 后移为可选参数（默认 `UiRect()` = 不限制），内部用当前 clip 兜底 |
| 五.4 | 测量依赖画布尺寸 | `Render_Skia.cpp#L2021-L2039` | **已修复** | `MeasureString` 不再检查 `GetWidth()/GetHeight()`，无 Surface 时也可测量 |
| 五.5 | 三个绘图接口职责重叠、参数易错 | `IRender.h#L859-L1019` | **已修复** | 收敛为两个 `DrawImage` 重载 + `ImageDrawParam`；删除 `DrawImageRect` |
| 六 | 缺失接口（P0/P1/P2） | 见第六节 | **已全部修复** | P0/P1/P2 的缺失接口均已补齐 |

> 补充（2026-11-14 第三批）：`ClearClip` 已删除，其实现（`restoreToCount` + `m_saveCount` 同步）并入 `RestoreClip`，全部调用方改用 `RestoreClip`，行为与删除前一致。

> 补充（2026-11-14 第四批）：`MeasureRichText` / `DrawRichText` / `CreateDrawRichTextCache` 的 `IRenderFactory*` 参数已删除，改由 `IRenderFactory::CreateRender` 通过 `IRender::SetRenderFactory` 注入（`Clone` 同步复制），行为不变。

> 补充（2026-11-14 第五批）：`DrawImage` / `DrawImageRect` 的 `rcPaint` 已后移为参数列表末尾的可选参数（默认 `UiRect()`），传空表示不限制，由 `Render_Skia::GetEffectivePaintRect` 用当前裁剪区域兜底；新增 `image_draw` 单元测试覆盖该语义。

> 补充（2026-11-14 第六批）：`DrawImage` 的三个重载已合并为两个入口：`DrawImage(bitmap, rcDest)`（整图）与 `DrawImage(bitmap, rcDest, rcSource, const ImageDrawParam&)`；九宫格边距、平铺、窗口阴影、变换矩阵、脏区全部收敛到 `ImageDrawParam`，并删除 `DrawImageRect`。

> 补充（2026-11-14 第七批）：第六节 P0 的 8 组缺失接口已全部补齐——`FillArc`、`DrawEllipse`（2 个重载）、`FillEllipse`、`DrawPolygon`（2 个重载）、`FillPolygon`、`ClipPath`、`ImageSamplingQuality` + `Set/GetImageSamplingQuality`；新增 `render_shape` 单元测试覆盖。

> 补充（2026-11-14 第八批）：第六节 P1、P2 已全部补齐——`IFont` 的 `GetFontMetrics`/`MeasureTextWidth`/`GetFontInfo`，`IBitmap` 的 `IsValid`/`GetRowBytes`/`GetAlphaType`/`GetImageSizeScale`/只读 `LockPixelBits`，`IRender` 的 `MeasureString` 轻量重载、`MakeImageSnapshot(rc)`、`SaveLayer`/`RestoreLayer`、`SetBlendMode`/`GetBlendMode`、`SetTransform`/`ConcatMatrix`，`IMatrix` 的 `Reset`/`Concat`/`Invert`/`IsIdentity`/`MapPoint`，`IPen` 的 `SetAntiAlias`/`SetMiterLimit`/`SetDashPattern`/`HasDashPattern`，`IBrush` 的线性/径向渐变，`IRenderFactory::CreateBitmap(w,h,alphaType)`；`DrawArc`/`FillArc` 的矩形参数统一为 `UiRectF`，并按"Draw 后紧跟 Fill"的分组规则调整了 `FillArc` 的位置；新增 `render_ext` 单元测试覆盖。

> 补充（2026-11-14 第九批）：完成第三节第 6 条的公共头文件注释普查——`IRender.h` 的 `RopMode` 枚举改为纯"行为描述与限制"，`SystemNotification.h` 的 SDL 构建说明改为"跨平台构建"；仅保留 `SharePtr.h` 的来源署名与后端专用头文件中的引擎命名。

> 补充（2026-11-14 第十批）：完成第三节第 5 条与第五节第 4 条——RGN 三个接口（`SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn`）在 `IRender` 中加 `#ifdef DUILIB_BUILD_FOR_WIN`，`Render_Skia_SDL` 与 `NativeWindow_SDL` 同步做条件编译，非 Windows 平台不再暴露窗口形状语义；`Render_Skia::MeasureString` 去掉对 `GetWidth()/GetHeight()` 的依赖，并允许画布为 `nullptr`（`VerticalDrawText`/`HorizontalDrawText` 的测量路径同步放宽），无 Surface 的纯测量场景可正常返回结果；`render_ext` 新增 7 项检查。

> 补充（2026-11-14 第十一批）：完成第五节第 1 条——新增 `IRenderWindow : public virtual IRender`，`PaintAndSwapBuffers` / `GetRenderDC` / `ReleaseRenderDC` / `SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn` 从 `IRender` 迁出；`Render_Skia_Windows/SDL` 改为继承 `IRenderWindow`，`Window::m_render` 改为 `std::unique_ptr<IRenderWindow>`，`INativeWindow::OnNativeGetRender()` 返回 `IRenderWindow*`（`Window::GetRender()` 仍返回 `IRender*`，绘制代码零改动）。同时新增离屏渲染：`IRenderFactory::IsOffscreenBackendSupported` 与 `CreateOffscreenRender`——`kRaster` 全平台可用；`kNativeGL` 条件支持（复用同线程窗口 GL 上下文，或惰性创建后台隐藏窗口上下文；Windows ARM64 不支持）；离屏对象不实现 `IRenderWindow`，支持 `Resize` / `Clone` / 快照回读。`IRenderFactory::CreateRender` 已从接口移除，窗口与离屏分别使用 `CreateWindowRender` / `CreateOffscreenRender`，examples/CEF/单元测试等调用点全部同步迁移。新增 `offscreen_render` 单元测试，ctest 12/12 通过。

统计：已修复 24 项、部分修复 0 项、暂不处理 1 项、未处理 0 项、缺失接口 P0/P1/P2 已全部补齐。

## 一、结论

接口覆盖了 UI 框架的常见需求（图元、文本、富文本缓存、位图、裁剪、窗口刷新），富文本的三级缓存（`CreateDrawRichTextCache` / `IsValidDrawRichTextCache` / `UpdateDrawRichTextCache` / `DrawRichTextCacheData`）设计得比较好。

主要问题集中在四类，按严重程度排列（当前处理状态见上一节）：

1. **误导性与正确性问题**：`RopMode` 声明了未实现的模式；`uFade` 与颜色 Alpha 语义冲突；`WritePixels` 源数据不是只读；若干注释与拼写错误。
2. **抽象层泄漏**：`BitmapAlphaType` 直接使用 Skia 命名；字形 ID（`SkGlyphID`）进入公共接口；`RenderType` 只有一个取值；Windows 窗口语义混入通用接口；`RichTextData` 强制 UTF-16。
3. **接口一致性**：`Clone` 家族所有权与 const 不统一；多个只读接口缺 const；两套裁剪状态机制并存；`MeasureRichText` 用编号式重载膨胀。
4. **职责过载与能力缺失**：`IRender` 同时承担绘制、窗口、Surface 三类职责；缺少椭圆/多边形/路径裁剪/图层/图像采样质量等常用能力。

## 二、必须改（正确性 / 误导性）

### 1. `RopMode` 只实现了 `kSrcCopy`（高）— **已修复**

- 位置：`duilib/Render/IRender.h#L526-L542`；实现：`duilib/RenderSkia/Render_Skia.cpp#L318-L343`。
- 历史问题：枚举声明了四种光栅操作，但 Skia 实现只映射 `kSrcCopy`，其余分支执行 `ASSERT` 后静默不绘制。
- 当前实现：四种模式均已映射——`kSrcCopy`→`kSrc`、`kDstInvert`→`kDifference`（先把源图转为纯白）、`kSrcInvert`→`kXor`、`kSrcAnd`→`kSrcIn`；枚举头的注释已逐条写明映射关系与限制。
- 遗留：`kSrcAnd` 与 GDI 的位与并非完全等价（注释已标注"近似实现"）。如需严格等价，需在 Blitter 层自定义混合，优先级低。

### 2. `WritePixels` 的源数据应为只读（中）— **已修复**

- 位置：`IRender.h#L1372-L1385`；实现：`Render_Skia.cpp` 中两个重载（`WritePixels(..., rc)` / `WritePixels(..., rc, rcPaint)`）。
- 当前签名均为 `bool WritePixels(const void* srcPixels, size_t srcPixelsLen, ...)`，与 `DrawPath(const IPath*)` 等只读参数保持一致。

### 3. `uFade` 与颜色 Alpha 语义冲突（中）— **已修复**

- 位置：`DrawStringParam::uFade`（`IRender.h#L844`）；实现：`Render_Skia.cpp#L1608-L1614`（`DrawString`）。
- 当前语义统一为"**与颜色值自身的 Alpha 相乘**"：实现先计算 `colorAlpha * uFade / 255` 再设置，不再用 `skPaint.setAlpha(uFade)` 覆盖。
- 同类修正：`FillRect` / `FillRoundRect` / `FillCircle` / `DrawImage` / `DrawImageRect` / 富文本绘制等所有带 `uFade` 的接口均已改为相乘语义，`IRender.h` 中对应注释也已更新。

### 4. 注释与参数名错误（低，但会误导调用方）— **已修复**

- `ClearClip` 的注释曾修正为 `@param [in] nState`；该接口已于 2026-11-14 删除（并入 `RestoreClip`，见第四节第 3 条）。
- `MeasureRichText` 的 `richTextData` 已改为 `@param [in]`，并补充 `@param [out] pRichTextRects`（`IRender.h#L1165-L1176`）。
- `HasFontName` 的 `@param` 已改为 `[in]`（`IRender.h#L93-L97`）。
- `CreateBrush(UiColor color)` 拼写已修正（`IRender.h#L1450`）。

## 三、抽象层泄漏（Skia 细节进入公共接口）

### 1. `BitmapAlphaType` 直接采用 Skia 命名 — **已修复**

- 位置：`IRender.h#L136-L160`。
- 当前类型为中性 `enum class AlphaType { kUnknown, kOpaque, kPremul, kUnpremul }`，`IBitmap::Init(..., AlphaType alphaType = AlphaType::kPremul)`；注释中"只有 Skia 引擎需要此参数"的表述已删除，映射在 `Bitmap_Skia` 内部完成。

### 2. 字形 ID 泄漏 — **已修复（字体回退优化中调整）**

- 位置：`IFont::IsUnicodeCharSupported(uint32_t)`（`IRender.h#L46-L50`）、`IFallbackFontMgr::CreateFallbackFont(const IFont*, uint32_t)`（`IRender.h#L63-L74`）。
- 当前公共接口**不再出现 `SkGlyphID`**：`IsUnicodeCharSupported` 只返回 `bool`；回退接口只返回 `IFont*`，不输出字形索引。字形索引完全收敛到渲染层内部（`DrawSkiaText` / `Font_Skia`）。
- 新增的配套接口：`IFont::GetFontMgr()`（`#L52-L55`）、`IFont::GetFontKey()`（`#L57-L60`）、`IFontMgr::SetFallbackFontMgr` / `GetFallbackFontMgr`（`#L125-L133`），详见第九节。
- `IsUnicodeCharSupported` 已于 2026-11-14 补 `const`（`Font_Skia::GetFontHandle()` 同步改为 const + `m_skFont` 声明为 `mutable`），与 `GetFontMgr() const` 风格一致（见第四节第 2 条）。

### 3. `RenderType` 是"假抽象" — **已修复**

- `RenderType` 枚举与 `IRender::GetRenderType()` 已从 `IRender.h` 删除，调用方不再需要写"永远只有一个分支"的 `switch`。

### 4. `RichTextData::m_textView` 跨平台类型不一致 — **暂不处理**

- 位置：`IRender.h#L575-L577`，类型仍是 `std::wstring_view`。
- **处理结论：暂不处理，保持现状。** 原因：富文本功能的内部实现统一使用 `std::wstring`，改为其他类型会波及内部实现，收益不明显。

### 5. Windows 窗口语义混入通用接口 — **已修复**

- 位置：`GetRenderDC(HWND)` / `ReleaseRenderDC`（`IRender.h#L1592-L1601`）、`SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn`（`IRender.h#L1680-L1701`）。
- 现状：`GetRenderDC` / `ReleaseRenderDC` 与三个 RGN 接口均已用 `DUILIB_BUILD_FOR_WIN` 保护，非 Windows 平台不再在通用渲染接口中暴露窗口形状语义。
- 同步调整：`Render_Skia_SDL.h/.cpp` 的三个 RGN 重写也放入 `DUILIB_BUILD_FOR_WIN`（SDL 在 Windows 上仍通过 `HWND` 实现，非 Windows 平台直接不编译）；`NativeWindow_SDL.cpp` 的调用点做同样的条件编译，非 Windows 平台返回 `false`/空实现，窗口层（`WindowBase`、`NativeWindow_SDL`）对外接口保持不变。

### 6. 注释中直接写引擎名 — **已修复**

- 已清理 `IBitmap::Init` 的"只有 Skia 引擎需要此参数"、`DrawArc` 的"仅 Skia 引擎使用"等表述（`IRender.h#L1095-L1106` 现为行为描述）。
- 已完成公共头文件普查：`duilib/Render/IRender.h` 的 `RopMode` 枚举由"Skia 后端映射关系"改为"行为描述与限制"（不再写 `SkBlendMode`/`GDI`/`SRCCOPY`）；`duilib/Utils/SystemNotification.h` 的"使用 SDL 构建"改为"跨平台构建"，参数说明改为"仅跨平台通知通道实现生效"。
- 保留项（不属于后端耦合）：`duilib/Core/SharePtr.h` 中"基于 Skia 源码 SkRefCnt.h 做的修改"是来源署名，予以保留；`RenderSkia/`、`NativeWindow_SDL`、`ControlDropTarget_SDL` 等后端/平台专用头文件本身即以后端命名，其注释不受此约束。

## 四、接口一致性问题

> 状态：本组 8 条问题已于 2026-11-14 全部修复（第 1、2、5、6、7、8 条为第一批，第 3、4 条为第二批）。下列行号已按当前代码校准。

### 1. `Clone` 家族所有权与 const 不统一 — **已修复**

| 接口 | 位置 | 现状 |
| --- | --- | --- |
| `IRender::Clone` | `IRender.h#L1367` | `std::unique_ptr<IRender> Clone() const` |
| `IBitmap::Clone` | `IRender.h#L185` | `std::unique_ptr<IBitmap> Clone() const` |
| `IPath::Clone` | `IRender.h#L424` | `std::unique_ptr<IPath> Clone() const` |
| `IPen::Clone` | `IRender.h#L280` | `std::unique_ptr<IPen> Clone() const` |
| `IBrush::Clone` | `IRender.h#L289` | `std::unique_ptr<IBrush> Clone() const` |
| `IFont` / `IMatrix` | — | 无 Clone，维持原状 |

- 五个 `Clone` 已统一为 `std::unique_ptr<T> Clone() const`：所有权明确（调用方独占）、不再需要手工 `delete`、`const` 修饰一致。
- 实现要点：`Bitmap_Skia::Clone` 在 const 方法内通过 `SkPixmap::addr()` 读像素（`Init` 入参为 `const void*`）；`Path_Skia::Clone` / `Transform` 使用新增的 `Matrix_Skia::GetMatrix() const` 重载。

### 2. 只读接口缺 const — **已修复**

- 只读成员函数：`IPath::GetFillType()`（`IRender.h#L326`）、`IPath::GetBounds(const IPen*)`（`#L412`）、`IRender::GetClipInfo(std::vector<UiRect>&) const`（`#L1397`）、`IFont::IsUnicodeCharSupported(uint32_t) const`（`#L50`）均已加 `const`。
- 只读指针参数：`IPath::Transform(const IMatrix*)`（`#L407`）、`DrawArc(..., const UiColor* gradientColor, const UiRect* gradientRect)`（`#L1105`）、`DrawImage(..., const IBitmap* pBitmap, ...)`（`#L987`、`#L995`）、`DrawImageRect(..., const IBitmap*, ..., const IMatrix* pMatrix)`（`#L1009`）、`BitBlt/StretchBlt(..., const IRender* pSrcRender, ...)`（`#L938`、`#L955`）均已加 `const`。
- 连带改动：`Font_Skia::GetFontHandle()` 改为 `const`（字体句柄为懒加载，`m_skFont` 声明为 `mutable`），使 `IsUnicodeCharSupported` 可以保持 const；Skia 侧的 `dynamic_cast` 目标同步改为 `const Render_Skia*` / `const Bitmap_Skia*` / `const Matrix_Skia*`。

### 3. 两套裁剪状态机制并存 — **已修复**

- 保留底层状态栈：`SaveClip(int32_t&)` / `RestoreClip(int32_t)`（`IRender.h#L900`、`#L905`）语义不变（显式保存/恢复画布状态，配合 `OffsetWindowOrg` 等使用）。
- 裁剪族按"动作"重命名，语义自解释，不再与状态栈混淆：
  - `SetClip(rc, bIntersect)` → **`ClipRect(rc, bIntersect)`**（`IRender.h#L912`）；
  - `SetRoundClip(rc, rx, ry, bIntersect)` → **`ClipRoundRect(rc, rx, ry, bIntersect)`**（`#L921`）；
  - 原 `ClearClip(nState)` 已删除，裁剪清理统一由 `RestoreClip(nState)` 承担（`#L905`），其内部即 `restoreToCount(nState)` + 同步 `m_saveCount`，与删除前的行为完全一致。
- 调用方已同步：`AutoClip`、`Panel`、`GroupBox`、`ListCtrlReportView`、`RichEdit2` 均改用 `RestoreClip`；`SaveClip` 语义不变（当前无调用方）。

### 4. `MeasureRichText` 编号式重载膨胀 — **已修复**

- 原 `MeasureRichText` / `MeasureRichText2` / `MeasureRichText3` 只差输出参数（是否输出行信息、是否创建绘制缓存），已合并为单个接口（`IRender.h#L1176`）：
  ```cpp
  virtual UiRect MeasureRichText(const UiRect& textRect,
                                 const UiSize& szScrollOffset,
                                 IRenderFactory* pRenderFactory,
                                 const std::vector<RichTextData>& richTextData,
                                 RichTextLineInfoParam* pLineInfoParam = nullptr,
                                 std::shared_ptr<DrawRichTextCache>* spDrawRichTextCache = nullptr,
                                 std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;
  ```
- 只测量时直接调用 5 参数形式；需要行信息/绘制缓存时按需追加实参。`Render_Skia` 只保留一份实现，`RichEditData`、`RichTextImpl`、基准测试与单元测试均已按新签名调用。

### 5. 返回值约定不一致 — **已修复**

- 测量族统一返回尺寸：`MeasureString` 原本就返回 `UiRect`；`MeasureRichText`（合并后位于 `IRender.h#L1176`）返回 `UiRect`，不再依赖出参。
- 返回值约定：以 `(0,0)` 为起点，**宽度为内容的最大行宽，高度为内容的总高度**（与入参矩形的坐标无关，可直接作为布局尺寸使用）。实现上由 `DrawRichText::InternalDrawRichText` 在布局过程中同步累计最大行宽与内容底端坐标得到。
- 绘制族保持统一 `void`（内部 ASSERT + 静默忽略失败），`CreateDrawRichTextCache` / `WritePixels` / `ReadPixels` 等"创建/IO"族保持 `bool`；两类职责清晰，不再出现同一族接口两种风格。

### 6. "未设置"用魔法值表达 — **已修复**

- `MeasureStringParam::rectSize`（原 `= DUI_NOSET_VALUE`）已拆分为两个语义明确的字段（`IRender.h#L826-L828`）：
  ```cpp
  int32_t limitWidth = -1;   //横向文本的限制宽度（像素），-1表示不限制
  int32_t limitHeight = -1;  //纵向文本的限制高度（像素），-1表示不限制
  ```
- 调用方按文本方向二选一赋值：`TextDrawer` 用 `limitWidth`；`LabelImpl` 按 `IsVerticalText()` 赋值 `limitWidth` 或 `limitHeight`；`ChatMessageList` / `Stepper` / `Timeline` 的横向测量统一用 `limitWidth`。
- `-1` 表示不限制，`0` 仍是合法的限制值（空宽度/高度），与原 `rectSize` 的语义边界保持一致。

### 7. 文本间距模型分裂 — **已修复**

- `DrawStringParam` / `MeasureStringParam` 的 `fWordSpacing` 已改名为 `fLetterSpacing`（`IRender.h#L835`、`#L851`），与 `RichTextData::m_fLetterSpacing`（`#L604`）命名一致，注释中注明"与 `RichTextData::m_fLetterSpacing` 语义一致"，单位统一为像素。
- 内部实现同步改名：`HorizontalDrawText` / `VerticalDrawText` 的局部变量与辅助函数入参（原 `fWordHorizontalSpacing` / `fWordVerticalSpacing`）、`TextDrawer::UpdateTextDrawProps` 的入参均统一为 `fLetterSpacing`。
- 说明：控件层 `Label::SetWordSpacing` 及其 XML 属性 `word_spacing` 属于对外 UI 属性，名称保持不变，内部赋值到 `fLetterSpacing`，避免破坏现有资源文件。

### 8. 取值风格不一致 — **已修复**

- `IFontMgr::GetFontName` 由出参 `bool GetFontName(uint32_t, DString&) const` 改为按值返回 `DString GetFontName(uint32_t) const`（`IRender.h#L90`），与 `IFont::FontName()`（`#L24`）风格一致；下标无效或获取失败时返回空字符串，调用方以 `empty()` 判断。
- 调用方 `FontManager::GetFontNameList` 已同步改为返回值判断。

## 五、职责过载与结构性问题

### 1. `IRender` 承担了过多职责 — **已修复**

历史情况（拆分前）`IRender` 同时包含：

1. 图元绘制与路径；
2. 文本测量与绘制、富文本缓存；
3. 裁剪与视区原点；
4. 位图读写与快照（`ReadPixels` / `WritePixels` / `MakeImageSnapshot`）；
5. 窗口刷新（`PaintAndSwapBuffers`）；
6. 窗口形状（`SetWindowRoundRectRgn` 等）；
7. DPI 与平台 DC（`SetRenderDpi` / `GetRenderDC`）。

当前实现（2026-11-14 第十一批）：

- `IRender`：绘制 + 裁剪 + 文本 + 位图快照 + `GetWidth/GetHeight/Resize`；
- `IRenderWindow : public virtual IRender`：`PaintAndSwapBuffers`、`SetWindow*Rgn`、`GetRenderDC/ReleaseRenderDC`（窗口形状与平台 DC 仍限定 Windows）；
- `Render_Skia` 虚继承 `IRender`，`Render_Skia_Windows/SDL` 再同时继承 `IRenderWindow`；新增的 `Render_Skia_Offscreen` 只继承 `IRender`，编译期即无法调用窗口接口；
- 工厂新增 `IsOffscreenBackendSupported` / `CreateOffscreenRender`：`kRaster` 必选，`kNativeGL` 条件支持（复用同线程窗口 GL 上下文或创建后台隐藏窗口上下文；Windows ARM64 不支持），显式请求不支持的后端返回 `nullptr`，不做静默回退；
- 现有隐式离屏调用点（`Control::CreateTempRender`、`ControlDragable`、`BitmapHepler_Windows/SDL`、`CefControlOffScreen`、examples、单元测试）已迁移到 `CreateOffscreenRender`；`CreateRender` 接口已移除。

收益：离屏渲染（导出图片、控件离屏缓存）不再被迫依赖窗口相关接口；跨平台实现也不需要为窗口接口写空实现。

### 2. 富文本调用每次都传 `IRenderFactory*` — **已修复**

- 位置：`MeasureRichText` / `DrawRichText` / `CreateDrawRichTextCache`（原 `IRender.h#L1168-L1290`）。
- 历史问题：`IRender` 本身就是 `IRenderFactory::CreateRender` 创建的，字体工厂却要在每次富文本调用时由调用方再传一遍。
- 当前实现（2026-11-14 第四批）：
  - `IRender` 新增 `SetRenderFactory(IRenderFactory*)`（`IRender.h#L1373`），由 `RenderFactory_Skia::CreateRender` 在创建 Render 时注入，`Clone` 时同步复制；
  - `Render_Skia` 保存 `m_pRenderFactory`，三个富文本接口删除 `IRenderFactory*` 参数，改为使用注入的工厂创建字体；
  - 调用方（`RichTextImpl`、`RichEditData`、`RichEdit2`、单元测试、基准程序）删除对应实参。

### 3. `rcPaint` 脏区参数把优化责任推给调用方 — **已修复**

- 位置：`DrawImage` 系列（`IRender.h#L859-L1019`）。
- 历史问题：每次绘制图片都要传"当前全部可绘制区域"，漏传或传错会导致多画/少画；脏区判断本可以由 Render 内部的 clip 统一处理。
- 本质：`rcPaint` 在实现中只用于"与目标区域不相交时提前返回"，真正的裁剪一直由 canvas clip 承担（`Window::Paint` 已将根裁剪设为 `rcPaint`），因此它只是性能优化参数，不影响可见结果。
- 当前实现（2026-11-14 第五批）：
  - `rcPaint` 移到参数列表末尾并默认 `UiRect()`，传空表示"不限制"，新调用方可直接省略；
  - `Render_Skia::GetEffectivePaintRect`（`Render_Skia.cpp#L2168`）在 `rcPaint` 为空时取 `getDeviceClipBounds()` 并减去 `GetWindowOrg()` 换算到绘制坐标空间兜底；刻意不复用 `GetClipInfo`（其向内缩 1 px，会导致边缘漏画）；
  - `WritePixels` 的 `rcPaint` 是真实复制边界（写像素不受 clip 约束），保持"空 = 不复制"语义不变；
  - 新增 `tests/test_image_draw.cpp`（`image_draw` 用例）覆盖三种情形。

### 4. 测量依赖画布尺寸 — **已修复**

- 位置：`Render_Skia::MeasureString`（`Render_Skia.cpp#L2021-L2039`）。
- 历史问题：在 `GetWidth() <= 0 || GetHeight() <= 0` 时直接返回空，影响无 Surface 的纯测量场景（如提前估算布局）。
- 当前实现（2026-11-14 第十批）：
  - 删除 `GetWidth()/GetHeight()` 判断：文本测量只依赖字体与绘制属性，与画布尺寸无关；
  - 删除普通路径上的 `GetSkCanvas() == nullptr` 提前返回，测量过程不再获取画布；
  - 纵向与两端对齐路径需要构造 `VerticalDrawText`/`HorizontalDrawText`，仅在画布存在时传入画布，否则传 `nullptr`；这两个类的 `MeasureString` 同步放宽为不要求画布非空（其测量过程也不使用画布）；
  - 新增 `render_ext` 用例 `TestMeasureWithoutCanvas`：在未 `Resize()`（尺寸为 0）的离屏 Render 上验证单行与多行测量均可返回有效结果。

### 5. 三个绘图接口职责重叠、参数易错 — **已修复**

- 位置：`IRender.h#L859-L1019`（原 2 个 `DrawImage` + 1 个 `DrawImageRect`）、`Render_Skia.cpp#L550-L915`。
- 历史问题：
  - 九宫格版 `DrawImage` 有 9 个参数、4 个默认值，且 `rcDestCorners` / `rcSourceCorners` 两个相邻的 `UiRect` 实际语义是"四条边的边距"，框架内二者恒等（`ImageInfo::ScaleImageSourceRect` 中直接 `rcDestCorners = rcSourceCorners`），调用方必须重复传两遍，传反或漏传时编译期无法发现；
  - 简化版 `DrawImage` 转发时传入空 corners，`Render_Skia` 内部会因为 `rcImageBorder.IsEmpty()` 强制关闭 `bWindowShadowMode`，该参数实际是死参数；
  - `DrawImageRect` 的名字无法体现"带变换矩阵"，能力与 `DrawImage` 互斥又重叠（多数调用点传 `pMatrix == nullptr`），调用方难以判断该用哪个；
  - 缺少"整图绘制"入口，调用方反复手写 `UiRect(0,0,w,h)`。
- 当前实现（2026-11-14 第六批）：
  - `DrawImage(bitmap, rcDest)`：整图绘制，源区域自动取整图；
  - `DrawImage(bitmap, rcDest, rcSource, const ImageDrawParam&)`：其余全部能力，可选参数按名称赋值，顺序无关；
  - `ImageDrawParam` 字段：`uFade`、`rcImageBorder`（`UiMargin`，目标与源共用）、`pTiledDrawParam`、`bWindowShadowMode`、`pMatrix`、`rcPaint`；
  - 删除 `DrawImageRect`，矩阵绘制改为 `ImageDrawParam::pMatrix`，实现内用 `save()/restore()` 替代 `resetMatrix()`，不再遗留画布变换副作用；
  - 矩阵与九宫格/平铺/窗口阴影的互斥关系由 `Render_Skia::DrawImage` 内部断言保证，`Control::PaintImage` 不再需要外层 `if/else` 分支；
  - 删除 `ImageInfo::ScaleImageSourceRect` 的 `rcDestCorners` 出参，消除冗余的第二个 corners 值。
- 影响：框架内 24 处、示例与测试 7 处调用点同步调整；`bWindowShadowMode` 现在在简化入口不再存在，只保留在 `ImageDrawParam` 中且仅在边距非空时生效。

## 六、常用但缺失的接口（建议补充）

> 状态：P0/P1/P2 均已于 2026-11-14 **全部补齐**（见下方每项标注）。

### P0：高频、Skia 实现成本低 — **已全部补齐**

```cpp
// 图元：补齐 DrawArc 的填充对称项、椭圆、多边形的填充/描边
//（DrawArc/FillArc 的矩形参数与其余图元接口一致，使用 UiRectF）
virtual void FillArc(const UiRectF& rc, float startAngle, float sweepAngle, bool useCenter,
                     UiColor dwColor, uint8_t uFade = 255) = 0;              // 新增
virtual void DrawEllipse(const UiRectF& rc, UiColor penColor, float fWidth) = 0;   // 新增
virtual void DrawEllipse(const UiRectF& rc, const IPen* pen) = 0;                  // 新增
virtual void FillEllipse(const UiRectF& rc, UiColor dwColor, uint8_t uFade = 255) = 0; // 新增
virtual void DrawPolygon(const UiPointF* points, int32_t count, UiColor penColor, float fWidth) = 0; // 新增
virtual void DrawPolygon(const UiPointF* points, int32_t count, const IPen* pen) = 0;                // 新增
virtual void FillPolygon(const UiPointF* points, int32_t count, UiColor dwColor, uint8_t uFade = 255) = 0; // 新增

// 非矩形裁剪：圆形头像、异形卡片（与 ClipRect / ClipRoundRect 同一套 save/restore 语义）
virtual int32_t ClipPath(const IPath* path, bool bIntersect = true) = 0;      // 新增

// 图像缩放质量：默认 kNearest 保持既有渲染效果，可按需切换
enum class ImageSamplingQuality { kNearest = 0, kLinear = 1, kCubic = 2 };     // 新增
virtual void SetImageSamplingQuality(ImageSamplingQuality quality) = 0;        // 新增
virtual ImageSamplingQuality GetImageSamplingQuality() const = 0;             // 新增
```

已补齐项的实现说明：

| 接口 | 实现位置 | 说明 |
| --- | --- | --- |
| `FillArc` | `Render_Skia.cpp` | 与 `DrawArc` 共用 `drawArc`，仅把画笔换成填充画刷；`useCenter=true` 为扇形，`false` 为弦与弧围成的区域；`uFade` 与颜色 Alpha 相乘 |
| `DrawEllipse` ×2 | `Render_Skia.cpp` | 由矩形构造 `SkRect` 后 `drawOval`，描边由 `SetPaintByPen` 设置；带 `IPen*` 的重载支持虚线/线帽等线形 |
| `FillEllipse` | `Render_Skia.cpp` | `drawOval` + 填充画刷，支持 `uFade` |
| `DrawPolygon` ×2 | `Render_Skia.cpp` | 顶点数组内部构造闭合 `SkPath`（`moveTo`/`lineTo`/`close`），再 `drawPath`；`count < 3` 直接返回 |
| `FillPolygon` | `Render_Skia.cpp` | 同上路径 + 填充画刷；任意顶点数 ≥ 3，支持凹多边形（Skia 默认非零环绕规则） |
| `ClipPath` | `Render_Skia.cpp` | 路径按 `m_pSkPointOrg` 做 `makeOffset` 后 `clipPath`，与 `ClipRect`/`ClipRoundRect` 一致地同步 `m_saveCount` 并返回还原句柄；`bIntersect=false` 为反向裁剪 |
| `ImageSamplingQuality` | `Render_Skia.cpp` | 内部 `ToSkSamplingOptions` 映射为 `kNearest` / `kLinear` / `kCubic(Mitchell)`；`DrawImage`、`BitBlt`、`StretchBlt`、`AlphaBlend` 全部接入；`Clone` 同步复制该设置 |

> 默认值保持 `kNearest`，是为了不改变现有 UI 的渲染效果；DPI 缩放、缩略图等场景建议调用方显式设置为 `kLinear` 或 `kCubic`。
>
> 未纳入 P0 的相关能力：`DrawArc`/`FillArc` 目前只接受 `UiColor` 填充，弧形渐变仍走 `DrawArc` 的 `gradientColor` 参数；`DrawPolygon` 的 `IPen*` 重载使用 `const IPen*`（比既有的 `DrawCircle`/`DrawRoundRect` 的 `IPen*` 更严格，实参兼容）。

### P1：文本与位图能力 — **已全部补齐**

```cpp
// IFont
struct FontMetrics { float fAscent; float fDescent; float fLeading; float fLineHeight; };
virtual bool  GetFontMetrics(FontMetrics& metrics) const;         // 新增
virtual float MeasureTextWidth(const DString& text) const;        // 新增（不依赖Render/Canvas，支持字体回退）
virtual bool  GetFontInfo(UiFont& fontInfo) const;                // 新增（回读创建参数）

// IRender
virtual UiRect MeasureString(const DString& text, IFont* pFont, uint32_t uFormat = 0); // 新增轻量重载
virtual IBitmap* MakeImageSnapshot(const UiRect& rc);                                  // 新增局部快照
virtual void SaveLayer(const UiRect& rc, uint8_t alpha = 255);                          // 新增分组透明度
virtual void RestoreLayer();                                                            // 新增

// IBitmap
virtual bool IsValid() const; virtual uint32_t GetRowBytes() const; virtual AlphaType GetAlphaType() const;
virtual float GetImageSizeScale() const; virtual const void* LockPixelBits() const;     // 均新增
```

| 接口 | 实现位置 | 说明 |
| --- | --- | --- |
| `IFont::GetFontMetrics` | `Font_Skia.cpp` | `SkFont::getMetrics`，换算为 ascent/descent/leading/行高；字体无效时返回 `false` |
| `IFont::MeasureTextWidth` | `Font_Skia.cpp` | 复用 `DrawSkiaText::MeasureText`，与绘制使用同一字体回退链；空串返回 0 |
| `IFont::GetFontInfo` | `Font_Skia.cpp` | 回读 `InitFont` 时的 `UiFont`（字体名/字号/样式），便于缓存键比较 |
| `IRender::MeasureString`（轻量重载） | `Render_Skia.cpp` | 内部组装 `MeasureStringParam` 后复用原实现，调用方无需再拼参数结构体 |
| `IRender::MakeImageSnapshot(rc)` | `Render_Skia.cpp` | 区域先裁剪到画布范围，再逐行 `memcpy` 到新位图；区域无效返回 `nullptr` |
| `IRender::SaveLayer/RestoreLayer` | `Render_Skia.cpp` | `SkCanvas::saveLayer` + 图层 Alpha；同步维护 `m_saveCount`，与 `RestoreClip` 的断言保持一致 |
| `IBitmap` 只读查询 | `Bitmap_Skia.cpp` | `IsValid` 用 `readyToDraw()`；`GetRowBytes` 取 `SkPixmap::rowBytes()`；`GetAlphaType` 复用既有映射；`GetImageSizeScale` 记录 `Init` 入参；新增 `const` 版 `LockPixelBits` 返回 `SkPixmap::addr()` |

> 未新增 `IBitmap::GetPixelFormat`：框架内位图恒为 ARGB8888（N32），新增单值枚举与第三节已删除的 `RenderType` 属同类"假抽象"，故不引入。若后续支持多像素格式，再随格式一起增加。

### P2：抽象能力完善 — **已全部补齐**

```cpp
// IMatrix：新增读取与组合能力
virtual void Reset();  virtual void Concat(const IMatrix* pMatrix);
virtual bool Invert(); virtual bool IsIdentity() const; virtual UiPointF MapPoint(const UiPointF& pt) const;

// IPen：抗锯齿 / 斜接限制 / 自定义虚线
virtual void SetAntiAlias(bool bAntiAlias); virtual bool IsAntiAlias() const;
virtual void SetMiterLimit(float fMiterLimit); virtual float GetMiterLimit() const;
virtual void SetDashPattern(const float* intervals, int32_t count); virtual bool HasDashPattern() const;

// IBrush：纯色 / 线性渐变 / 径向渐变
enum class BrushType { kSolid, kLinearGradient, kRadialGradient };
virtual BrushType GetBrushType() const;
virtual void SetSolid(UiColor color);
virtual void SetLinearGradient(UiColor endColor, const UiPointF& startPt, const UiPointF& endPt);
virtual void SetRadialGradient(UiColor endColor, const UiPointF& center, float radius);

// IRender / IRenderFactory
enum class BlendMode { kSrcOver, kClear, kMultiply, kScreen, kXor, kPlus };
virtual void SetBlendMode(BlendMode blendMode); virtual BlendMode GetBlendMode() const;
virtual void SetTransform(const IMatrix* pMatrix); virtual void ConcatMatrix(const IMatrix* pMatrix);
virtual IBitmap* CreateBitmap(uint32_t nWidth, uint32_t nHeight, AlphaType alphaType = kPremul);
```

| 接口 | 实现位置 | 说明 |
| --- | --- | --- |
| `IMatrix` 扩展 | `Matrix_Skia.cpp` | `Concat` 用 `SkMatrix::postConcat`（先当前变换、后入参变换）；`Invert` 失败时矩阵保持不变；`MapPoint` 用 `mapPoint` |
| `IPen` 扩展 | `Pen_Skia.cpp` + `Render_Skia::SetPaintByPen` | 抗锯齿与斜接限制直接映射到 `SkPaint`；自定义虚线用 `SkDashPathEffect`，长度按 DPI 缩放；调用 `SetDashStyle` 会清除自定义样式（以最后一次设置为准） |
| `IBrush` 渐变 | `Brush_Skia.cpp` + `Render_Skia::FillPath` | 线性/径向渐变生成 `SkShaders` 着色器；渐变坐标加上视图原点偏移；纯色时行为不变 |
| `IRender::SetBlendMode` | `Render_Skia.cpp` | 映射为 `SkBlendMode` 并写入 `m_pSkPaint`，对后续绘制生效；`GetBlendMode` 回读 |
| `IRender::SetTransform/ConcatMatrix` | `Render_Skia.cpp` | 对应 `SkCanvas::setMatrix` / `concat`；`SetTransform(nullptr)` 重置为单位矩阵 |
| `IRenderFactory::CreateBitmap` | `RenderFactory_Skia.cpp` | 分配并初始化指定位图，失败返回 `nullptr` |

> `FillPath` 目前是唯一接受 `IBrush` 的填充接口；`FillRect`/`FillRoundRect` 仍使用"颜色 + 可选渐变参数"的既有形式，保持不变以兼容调用方。

## 七、建议的落地顺序

1. **第一批（低风险、不改调用方）— 已完成约一半**
   - ✅ 注释与拼写修正：`CreateBrush` 的 `corlor`、`ClearClip` 的 `@return`、`MeasureRichText` 的参数说明、`HasFontName` 的 `@param`；
   - ✅ `RopMode` 收敛（补齐四种模式的映射，并在注释中标注限制）；
   - ✅ `uFade` 语义明确为"与颜色 Alpha 相乘"，绘制类接口全部生效；
   - ✅ `WritePixels` 源数据改为 `const void*`；
   - ✅ **const 修正（2026-11-14 完成）**：`GetFillType`、`GetBounds`、`GetClipInfo`、`Transform(IMatrix*)`、`DrawArc` 的 `gradientColor/gradientRect`、`DrawImage(IBitmap*)`、`DrawImageRect(IBitmap*/IMatrix*)`、`BitBlt/StretchBlt` 的源 `IRender*`，以及 `IFont::IsUnicodeCharSupported`；
   - ✅ **一致性修正（2026-11-14 完成）**：`Clone` 家族统一为 `unique_ptr<T> Clone() const`；`MeasureRichText*` 返回 `UiRect`；`rectSize` 拆分为 `limitWidth`/`limitHeight`；`fWordSpacing` 统一为 `fLetterSpacing`；`IFontMgr::GetFontName` 改为按值返回（详见第十节）。
   - ✅ **裁剪与重载整理（2026-11-14 完成）**：保留 `SaveClip/RestoreClip`，`SetClip→ClipRect`、`SetRoundClip→ClipRoundRect`；`MeasureRichText2/3` 合并进 `MeasureRichText`（详见第十节）。

2. **第二批（只增不改，补常用能力）— 已完成**
   - ✅ `FillArc`；
   - ✅ `DrawEllipse` / `FillEllipse`；
   - ✅ `DrawPolygon` / `FillPolygon`；
   - ✅ `ClipPath`（原计划的 `SetPathClip`，按裁剪接口命名规则改为 `ClipPath`）；
   - ✅ `SetImageSamplingQuality` / `GetImageSamplingQuality`；
   - ✅ `IFont` 的度量接口（`GetFontMetrics` / `MeasureTextWidth` / `GetFontInfo`）。

3. **第三批（结构性调整，需评估影响面）— 部分完成**
   - ✅ `BitmapAlphaType` 去 Skia 化（已改为中性 `AlphaType`）；
   - ⬜ 拆分 `IRender` 的窗口/Surface 职责；
   - ✅ 合并 `MeasureRichText*` 编号重载（已合并为单个 `MeasureRichText`）；
   - ✅ `IBrush` 支持渐变（线性/径向，已在 `FillPath` 生效）；
   - ⬜ `RichTextData::m_textView` 统一跨平台编码（当前结论为暂不处理）。

## 八、附：值得保留的设计

- 富文本三级缓存接口（创建 / 校验 / 增量更新 / 绘制）职责清晰，`IsValidDrawRichTextCache` 的 `bCompareTextColor` 和 `DrawRichTextCacheData` 的 `pTextColors` 为"仅变色不重排"提供了正确路径，应在新增接口时保持这一模式。
- `TEXT_PATH_ELLIPSIS`（路径中间省略）是 UI 框架里少见但实用的能力。
- `IRenderPaint` 的回调式绘制（`DoPaint` + `GetUpdateRect`）把局部绘制决策交给窗口层，边界划分是合理的。
- 字体回退的接口分层值得保留：`IFontMgr` 持有 `IFallbackFontMgr`，`IFont` 可反查 `IFontMgr`，渲染层通过后者按需创建回退字体；公共接口只暴露"字符是否支持 / 回退字体对象"，字形索引完全留在渲染层内部。这条链路既支持 Emoji/扩展汉字，也避免了热路径上的 `dynamic_cast` 与字符串比较。

## 九、字体回退相关接口变更记录（2026-11-13）

本轮字体回退优化（详见 [字体回退性能优化分析.md](字体回退性能优化分析.md)）涉及以下**公共接口的签名/语义变更**，二次开发中自定义 `IFont` / `IFontMgr` / `IFallbackFontMgr` 实现或直接调用这些接口的代码需要同步修改。仓库内实现（`Font_Skia`、`FontMgr_Skia` 等）与示例工程已更新。

| 接口 | 变更前 | 变更后 | 目的 / 影响 |
| --- | --- | --- | --- |
| `IFont::IsUnicodeCharSupported` | `bool IsUnicodeCharSupported(uint32_t unicodeChar, uint16_t* glyphId)` | `bool IsUnicodeCharSupported(uint32_t unicodeChar)` | 公共接口不再输出 `SkGlyphID`；调用方删除 glyphId 实参，自定义实现同步改签名 |
| `IFont::GetFontMgr` | 无 | `IFontMgr* GetFontMgr() const`（默认返回 `nullptr`） | 渲染层由字体反查字体管理器，消除 `GetFontHandle` / `GetFallbackFontMgr` 等热路径上的 `dynamic_cast`；自定义实现可按需覆盖 |
| `IFont::GetFontKey` | 无 | `uint64_t GetFontKey() const`（默认返回 0，表示不支持） | 提供"字体名 + 字号 + 粗/斜/下划线/删除线"的稳定签名，用于回退字体缓存键，解决原先以 `IFont*` 为键导致的悬垂键、缓存失效与错命中问题；返回 0 时框架用 `CalcFontKey` 兜底 |
| `IFallbackFontMgr::CreateFallbackFont` | `IFont* CreateFallbackFont(const IFont* pFont, uint32_t unicodeChar, uint16_t* glyphId)` | `IFont* CreateFallbackFont(const IFont* pFont, uint32_t unicodeChar)` | 回退管理器只负责创建/复用回退字体对象，字形索引由渲染层调用 `unicharToGlyph` 解析 |
| `IFontMgr::SetFallbackFontMgr` / `GetFallbackFontMgr` | 无 | 新增（`IRender.h#L125-L133`） | 把回退字体管理器挂到字体管理器上，生命周期由设置者管理；配合 `global.xml` 的 `FallbackFontFamilyNames` 使用 |
| `BitmapAlphaType` | `BitmapAlphaType::{kUnknown_SkAlphaType, ...}` | `AlphaType::{kUnknown, kOpaque, kPremul, kUnpremul}` | 去 Skia 化，`Bitmap_Skia` 内部做双向映射；`IBitmap::Init` 的默认参数同步更新 |
| `RenderType` / `IRender::GetRenderType` | `RenderType::kRenderType_Skia` | 已删除 | 使用 `GetRenderBackendType()` 区分后端；原调用方删除对应 `switch` 分支 |

**本轮（字体回退，2026-11-13）未变化的接口**：`DrawString` / `MeasureString` / `DrawRichText` / `CreateDrawRichTextCache` / `DrawRichTextCacheData`、`RichTextData`，以及 `IBitmap` 除 `AlphaType` 命名外的其它方法。

> 注意：`MeasureRichText*` 的返回类型、`DrawStringParam` / `MeasureStringParam` 的字段（`rectSize`、`fWordSpacing`）已在 2026-11-14 的接口一致性调整中变更，见第十节。

**渲染层内部类型（不属于公共接口，可自由调整）**：`DrawSkiaText` / `SkTextBox` / `TextGlyphRunList` / `TextLineLayout` 均在 `duilib/RenderSkia` 目录内，`SkGlyphID`、`SkFont`、run 批量绘制等细节只出现在这些头文件中；`IRender.h` 中已不再出现 Skia 类型。

**测试覆盖**：`tests/test_text_draw.cpp` 通过真实的 `FontMgr_Skia` + 主字体 + 回退字体链，覆盖"主字体不支持 → 回退字体命中 → glyph 0 时绘制方框占位"的完整链路，以及测量、断行、run 构建与横排/竖排绘制；`ctest` 已注册 `text_draw` 用例。

## 十、接口一致性调整记录（2026-11-14）

本轮只做接口一致性修正，不改变绘制/测量的视觉效果。变更已同步 `Render_Skia` 实现、控件层调用方与单元测试，`ctest` 11/11 通过（`text_draw` 202 项、`image_draw` 22 项、`render_shape` 36 项、`render_ext` 87 项检查，0 失败）。分十批完成：第一批为第 1、2、5、6、7、8 条，第二批为第 3、4 条（`MeasureRichText` 合并行与末尾两行裁剪接口），第三批删除 `ClearClip`，第四批为第五节第 2 条（富文本接口去掉 `IRenderFactory*` 参数，改为 Render 创建时注入），第五批为第五节第 3 条（`rcPaint` 后移为可选参数并内部兜底），第六批为第五节第 5 条（三个绘图接口合并为两个入口 + `ImageDrawParam`），第七批为第六节 P0 缺失接口补齐，第八批为第六节 P1/P2 缺失接口补齐，第九批为第三节第 6 条（公共头文件注释去掉后端名），第十批为第三节第 5 条与第五节第 4 条（RGN 接口条件编译、测量不依赖画布尺寸）。

| 接口 / 字段 | 变更前 | 变更后 | 影响 |
| --- | --- | --- | --- |
| `IBitmap` / `IPath` / `IPen` / `IBrush` / `IRender::Clone` | 裸指针、const 不统一 | `std::unique_ptr<T> Clone() const` | 调用方改用 `unique_ptr` 持有并删除手工 `delete`；仓库内暂无其它调用方 |
| `IPath::GetFillType` / `GetBounds` | 非 const | `const` | 可对 const 路径对象调用 |
| `IPath::Transform(IMatrix*)` | 非 const 指针 | `const IMatrix*` | 实参不变，仅收紧 |
| `IRender::GetClipInfo` | 非 const | `const` | 可对 const Render 调用 |
| `DrawArc` 的渐变参数 | `UiColor*` / `UiRect*` | `const UiColor*` / `const UiRect*` | 实参不变 |
| `DrawImage` / `DrawImageRect` | `IBitmap*` / `IMatrix*` | `const IBitmap*` / `const IMatrix*` | 实参不变 |
| `BitBlt` / `StretchBlt` 的源 | `IRender*` | `const IRender*` | 实参不变 |
| `IFont::IsUnicodeCharSupported` | 非 const | `const` | 自定义 `IFont` 实现需同步改签名 |
| `MeasureRichText` / `MeasureRichText2` / `MeasureRichText3` | 三个编号重载，返回 `void` | 合并为单个 `MeasureRichText`，返回 `UiRect`（宽=内容最大行宽，高=内容总高度，原点为 `(0,0)`）；`pLineInfoParam` / `spDrawRichTextCache` / `pRichTextRects` 改为默认 `nullptr` 的可选出参 | 可忽略返回值；`MeasureRichText2/3` 的调用方改用 `MeasureRichText` 并追加实参 |
| `MeasureStringParam::rectSize` | `int32_t rectSize = DUI_NOSET_VALUE` | `int32_t limitWidth` / `int32_t limitHeight`（默认 `-1` 表示不限制） | 调用方按文本方向改用对应字段 |
| `DrawStringParam` / `MeasureStringParam::fWordSpacing` | `fWordSpacing` | `fLetterSpacing` | 调用方改用新字段名；控件层 `SetWordSpacing` 与 XML 属性 `word_spacing` 保持不变 |
| `IFontMgr::GetFontName` | `bool GetFontName(uint32_t, DString&) const` | `DString GetFontName(uint32_t) const` | 自定义 `IFontMgr` 实现需同步；调用方改为返回值判断空串 |
| `IRender::SetClip` | `SetClip(rc, bIntersect)` | `ClipRect(rc, bIntersect)` | 调用方改名即可（实参、返回值、语义均不变） |
| `IRender::SetRoundClip` | `SetRoundClip(rc, rx, ry, bIntersect)` | `ClipRoundRect(rc, rx, ry, bIntersect)` | 调用方改名即可（实参、返回值、语义均不变） |
| `IRender::ClearClip` | `ClearClip(nState)` | **已删除**，统一使用 `RestoreClip(nState)` | `RestoreClip` 内部已包含 `restoreToCount` 与 `m_saveCount` 同步；7 处调用方与 `AutoClip` 已切换，行为不变 |
| `IRender::MeasureRichText` / `DrawRichText` / `CreateDrawRichTextCache` | 每次调用传 `IRenderFactory* pRenderFactory` | 删除该参数，改用 Render 创建时注入的工厂 | 调用方（`RichTextImpl`、`RichEditData`、`RichEdit2`、测试与基准程序）删除实参；自定义 `IRender` 实现需从注入的工厂取字体 |
| `IRender::SetRenderFactory` | 无 | 新增 `virtual void SetRenderFactory(IRenderFactory* pRenderFactory) = 0`（`IRender.h#L1373`） | 由 `IRenderFactory::CreateRender` 创建后注入，`Clone` 时同步复制；自定义 `IRender` 实现需实现该方法，直接 `new` 渲染对象时须手动注入 |
| `IRender::DrawImage` / `DrawImageRect` 的 `rcPaint` | 首个参数，必传 | 后移到参数列表末尾，默认 `UiRect()`（空 = 不限制） | 控件层、工具类与示例的调用点调整参数位置；新调用方可省略，由当前裁剪区域兜底 |
| `IRender::DrawImage`（九宫格版） | 9 个参数，`rcDestCorners` / `rcSourceCorners` 两个 `UiRect`（实为边距且恒等） | 合并为 `DrawImage(bitmap, rcDest, rcSource, const ImageDrawParam&)`，边距改为单个 `UiMargin rcImageBorder` | 调用方改用 `ImageDrawParam` 按名赋值；边距合并后行为不变 |
| `IRender::DrawImage`（简化版） | 7 个参数，含实际无效的 `bWindowShadowMode` | 改为 `DrawImage(bitmap, rcDest)` 整图入口；`bWindowShadowMode` 仅保留在 `ImageDrawParam` | 去掉死参数；需要 `uFade` / 平铺 / 脏区的调用方改用 `ImageDrawParam` |
| `IRender::DrawImageRect` | 独立函数，名字不体现"矩阵"，能力与 `DrawImage` 重叠 | **已删除**，矩阵绘制改为 `ImageDrawParam::pMatrix` | 调用方合并到 `DrawImage`；实现改用 `save()/restore()`，不再 `resetMatrix()` |
| `ImageInfo::ScaleImageSourceRect` | 4 个参数，`rcDestCorners` 出参与 `rcSourceCorners` 恒等 | 3 个参数，删除 `rcDestCorners` 出参 | 仅 `Control::PaintImage` 一处调用方 |
| `Render_Skia::GetEffectivePaintRect` | 无 | 新增内部辅助函数（`Render_Skia.cpp#L2168`） | `rcPaint` 为空时用 `getDeviceClipBounds()` 减去 `GetWindowOrg()` 兜底；不复用 `GetClipInfo`（其内缩 1 px 会漏画）；`WritePixels` 的 `rcPaint` 语义保持不变 |
| `IRender::FillArc` | 无 | 新增 `FillArc(rc, startAngle, sweepAngle, useCenter, UiColor, uFade = 255)` | 与 `DrawArc` 对称；`useCenter=true` 填充扇形，`false` 填充弦与弧围成的区域 |
| `IRender::DrawEllipse` / `FillEllipse` | 无（只能手工构造 `IPath`） | 新增 `DrawEllipse(rc, UiColor, fWidth)`、`DrawEllipse(rc, const IPen*)`、`FillEllipse(rc, UiColor, uFade = 255)` | 椭圆绘制/填充不再需要手工拼路径；`IPen*` 重载支持虚线等线形 |
| `IRender::DrawPolygon` / `FillPolygon` | 无（`IPath::AddPolygon` 存在但无快捷方法） | 新增 `DrawPolygon(points, count, UiColor, fWidth)`、`DrawPolygon(points, count, const IPen*)`、`FillPolygon(points, count, UiColor, uFade = 255)` | `count < 3` 直接返回；内部构造闭合路径，支持凹多边形 |
| `IRender::ClipPath` | 无（只有矩形/圆角矩形裁剪） | 新增 `int32_t ClipPath(const IPath* path, bool bIntersect = true)` | 与 `ClipRect`/`ClipRoundRect` 同一套 `SaveClip`/`RestoreClip` 语义；路径按视图原点偏移后裁剪 |
| `ImageSamplingQuality` / `IRender::Set/GetImageSamplingQuality` | 无（`DrawImage`、`BitBlt`/`StretchBlt`/`AlphaBlend` 固定最近邻采样） | 新增枚举 `{kNearest, kLinear, kCubic}` + 设置/读取接口，默认 `kNearest` | 默认值不变，既有渲染效果不受影响；`Clone` 同步复制该设置；DPI 缩放/缩略图场景可切换为 `kLinear`/`kCubic` |
| `IRender::DrawArc` / `FillArc` 的矩形参数 | `UiRect` / `const UiRect*` | `UiRectF` / `const UiRectF*`（与其它图元接口一致） | 调用方（`CircleProgress`）改用 `UiRectF::MakeFromRect`；浮点坐标不再被截断 |
| `IRender::FillArc` 的声明位置 | 位于 `FillCircle` 之后 | 移到 `DrawArc` 之后（Draw/Fill 成对分组） | 仅头文件顺序调整，无语义变化 |
| `IFont` 文本能力 | 无度量/宽度/信息读取接口 | 新增 `FontMetrics` 结构 + `GetFontMetrics`、`MeasureTextWidth`、`GetFontInfo`（均有默认实现返回 `false`/`0`） | 自定义 `IFont` 实现可按需覆盖；`MeasureTextWidth` 不依赖 Render/Canvas |
| `IBitmap` 只读查询 | 只能 `GetSize` 与非常量 `LockPixelBits` | 新增 `IsValid`、`GetRowBytes`、`GetAlphaType`、`GetImageSizeScale`、`const void* LockPixelBits() const`（纯虚） | 自定义 `IBitmap` 实现需补齐这 5 个方法；未新增 `GetPixelFormat`（恒为 ARGB8888，避免假抽象） |
| `IRender::MeasureString` | 只有 `MeasureStringParam` 版本 | 新增轻量重载 `MeasureString(text, IFont*, uFormat = 0)`（纯虚） | 自定义 `IRender` 实现需补齐；内部复用原测量逻辑 |
| `IRender::MakeImageSnapshot` | 只有整画布版本 | 新增 `MakeImageSnapshot(const UiRect& rc)`（纯虚） | 局部快照，区域无效返回 `nullptr` |
| `IRender::SaveLayer` / `RestoreLayer` | 无（只能逐元素乘 Alpha） | 新增 `SaveLayer(rc, alpha = 255)` / `RestoreLayer()`（纯虚） | 用于整组淡出/离屏合成；与 `SaveClip`/`RestoreClip` 共用 `m_saveCount` 同步机制 |
| `IMatrix` | 只有变换接口 | 新增 `Reset`、`Concat`、`Invert`、`IsIdentity`、`MapPoint`（纯虚） | `Concat` 等价 `postConcat`（先当前变换、后入参变换）；`Invert` 失败时矩阵不变 |
| `IPen` | 抗锯齿/斜接/虚线不可控 | 新增 `SetAntiAlias`/`IsAntiAlias`、`SetMiterLimit`/`GetMiterLimit`、`SetDashPattern`/`HasDashPattern`（纯虚） | 自定义 `IPen` 实现需补齐；`SetDashStyle` 会清除自定义虚线样式 |
| `IBrush` | 只有纯色 | 新增 `BrushType`、`GetBrushType`、`SetSolid`、`SetLinearGradient`、`SetRadialGradient`（纯虚） | 自定义 `IBrush` 实现需补齐；渐变色刷在 `FillPath` 生效，`FillRect`/`FillRoundRect` 的既有渐变参数保持不变 |
| `BlendMode` / `IRender::Set/GetBlendMode` | 无（固定 SrcOver） | 新增枚举 `{kSrcOver, kClear, kMultiply, kScreen, kXor, kPlus}` + 设置/读取接口（纯虚） | 写入 `m_pSkPaint`，对后续绘制生效；`BitBlt` 的 `RopMode` 语义不受影响 |
| `IRender::SetTransform` / `ConcatMatrix` | 无（只能通过 `DrawImage` 的矩阵） | 新增 `SetTransform(const IMatrix*)`、`ConcatMatrix(const IMatrix*)`（纯虚） | 对应 `SkCanvas::setMatrix`/`concat`；`SetTransform(nullptr)` 恢复单位矩阵 |
| `IRenderFactory::CreateBitmap` | 只有空位图 + 手动 `Init` | 新增 `CreateBitmap(w, h, alphaType = kPremul)`（纯虚） | 一步创建并初始化；失败返回 `nullptr` |
| `RopMode` 枚举注释 | 逐个映射 `SkBlendMode`，并对比 `GDI` 位运算 | 改为"行为描述与限制"，说明各模式的语义与近似程度 | 仅注释变更，不影响任何实现或调用 |
| `SystemNotification` 注释 | "使用 SDL 构建时…"、"仅 SDL 实现生效" | 改为"跨平台构建"、"仅跨平台通知通道实现生效" | 仅注释变更；平台差异信息保留 |
| `IRender::SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn` | 通用接口中无条件暴露（仅 Windows 有实际实现） | 三个接口包入 `#ifdef DUILIB_BUILD_FOR_WIN` | 非 Windows 平台不再暴露窗口形状语义；`Render_Skia_SDL` 与 `NativeWindow_SDL` 的调用点同步条件编译 |
| `Render_Skia::MeasureString` | 尺寸为 0 或画布为空时返回空矩形 | 去掉画布尺寸/画布非空检查，无 Surface 时仍可测量 | 纯测量场景（提前估算布局）可正常返回；`VerticalDrawText`/`HorizontalDrawText` 的测量路径同步放宽 |

**测试补充**：`tests/test_text_draw.cpp` 新增 `MeasureRichText` 返回值契约校验——原点为 `(0,0)`、宽度不超过限制宽度、限制宽度越小（换行越多）内容总高度越大（共 7 项检查）。

**测试补充**：新增 `tests/test_image_draw.cpp`（`image_draw` 用例）——覆盖 `rcPaint` 传 `UiRect()` 正常绘制、与目标不相交时跳过绘制、部分相交时按裁剪区域可见、整图绘制入口、矩阵绘制时 `rcPaint` 不参与可见性判断（共 22 项检查）。

**测试补充**：新增 `tests/test_render_shape.cpp`（`render_shape` 用例）——覆盖 `FillEllipse`/`DrawEllipse`（中心填充、四角留白、描边位置）、`FillArc`（扇形内外象限）、`FillPolygon`/`DrawPolygon`（内部填充、外部留白、边框位置）、`ClipPath`（三角形内填充、外部留白）、`Set/GetImageSamplingQuality`（默认值与来回切换），共 36 项检查。

**测试补充**：新增 `tests/test_render_ext.cpp`（`render_ext` 用例）——覆盖 `IFont` 度量/文本宽度/字体信息、`IBitmap` 只读查询与新位图创建、`MeasureString` 轻量重载、`MakeImageSnapshot(rc)`（尺寸与像素校验）、`SaveLayer/RestoreLayer` 的 50% 图层混合、`Set/GetBlendMode`、`SetTransform`、`IMatrix` 的 `Reset/Concat/Invert/IsIdentity/MapPoint`、`IPen` 的抗锯齿/斜接限制/自定义虚线、`IBrush` 线性渐变填充（左右两端像素校验）、无画布时的文本测量（`TestMeasureWithoutCanvas`），共 87 项检查。
