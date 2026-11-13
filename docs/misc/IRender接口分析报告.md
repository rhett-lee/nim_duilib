下面的分析基于 `duilib/Render/IRender.h`，并对照 Skia 实现（`duilib/RenderSkia/Render_Skia.cpp`、`Bitmap_Skia.h`、`Path_Skia.h`、`DrawSkiaImage.cpp`）核实。

> **文档状态（2026-11-13 更新）**：本报告已随"字体回退性能优化"（见 [字体回退性能优化分析.md](字体回退性能优化分析.md)）重新校准。文中代码行号以当前代码为准；每条问题均标注当前处理状态（已修复 / 部分修复 / 未处理 / 暂不处理）；新增"零、进度总览"与"九、字体回退相关接口变更记录"，便于后续跟进。

## 零、进度总览（2026-11-13）

字体回退优化以渲染内部实现为主，但同时调整了 `IFont` / `IFontMgr` / `IFallbackFontMgr` 三个公共接口（见第九节），并顺带修复了本报告的部分"必须改"项。当前状态：

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
| 三.5 | Windows 窗口语义混入通用接口 | `IRender.h#L1338-L1347`、`#L1412-L1429` | **部分修复** | `GetRenderDC` 已有条件编译；RGN 三个接口仍未加 `DUILIB_BUILD_FOR_WIN` |
| 三.6 | 注释中直接写引擎名 | 见第三节第 6 条 | **部分修复** | 已清理 `AlphaType`、`DrawArc` 等处的引擎名，建议后续普查 |
| 四.1 | `Clone` 家族所有权与 const 不统一 | `IRender.h#L186` 等 | 未处理 | 需统一为 `unique_ptr<T> Clone() const` |
| 四.2 | 只读接口缺 const | 见第四节第 2 条 | 未处理 | 建议连同 `IFont::IsUnicodeCharSupported` 一并加 const |
| 四.3 | 两套裁剪状态机制并存 | `IRender.h#L897-L925` | 未处理 | 建议保留状态栈，`SetClip` 内部复用 |
| 四.4 | `MeasureRichText*` 编号式重载膨胀 | `IRender.h#L1165-L1208` | 未处理 | 建议合并为可选出参的单接口 |
| 四.5 | 返回值约定不一致 | 全文 | 未处理 | 测量/绘制的返回风格需统一 |
| 四.6 | "未设置"用魔法值表达 | `IRender.h#L827` | 未处理 | 建议改为显式 `limitWidth/limitHeight` |
| 四.7 | 文本间距模型分裂 | `IRender.h#L834`、`#L605` | 未处理 | `fWordSpacing` 与 `m_fLetterSpacing` 命名/单位需统一 |
| 四.8 | 取值风格不一致 | `IRender.h#L24`、`#L91` | 未处理 | `FontName()` 按值返回、`GetFontName` 用出参 |
| 五.1 | `IRender` 职责过载 | 全文 | 未处理 | 需拆分绘制 / 窗口 Surface 职责 |
| 五.2 | 富文本接口每次传 `IRenderFactory*` | `IRender.h#L1165-L1228` | 未处理 | 建议 Render 创建时注入 factory |
| 五.3 | `rcPaint` 脏区参数 | `IRender.h#L986-L1010` | 未处理 | 建议在注释中明确空矩形语义 |
| 五.4 | 测量依赖画布尺寸 | `Render_Skia.cpp#L1692-L1695` | 未处理 | 影响无 Surface 的纯测量场景 |
| 六 | 缺失接口（P0/P1/P2） | 见第六节 | 未处理 | 字体回退优化未新增任何图元/裁剪/采样接口 |

统计：已修复 7 项、部分修复 2 项、暂不处理 1 项、未处理 12 项、缺失接口 3 组待排期。

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

- `ClearClip` 的注释已改为 `@param [in] nState`（`IRender.h#L922-L925`）。
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
- 遗留：`IsUnicodeCharSupported` 仍未加 `const`，与 `GetFontMgr() const` 风格不一致（见第四节第 2 条）。

### 3. `RenderType` 是"假抽象" — **已修复**

- `RenderType` 枚举与 `IRender::GetRenderType()` 已从 `IRender.h` 删除，调用方不再需要写"永远只有一个分支"的 `switch`。

### 4. `RichTextData::m_textView` 跨平台类型不一致 — **暂不处理**

- 位置：`IRender.h#L575-L577`，类型仍是 `std::wstring_view`。
- **处理结论：暂不处理，保持现状。** 原因：富文本功能的内部实现统一使用 `std::wstring`，改为其他类型会波及内部实现，收益不明显。

### 5. Windows 窗口语义混入通用接口 — **部分修复**

- 位置：`GetRenderDC(HWND)` / `ReleaseRenderDC`（`IRender.h#L1338-L1347`）、`SetWindowRoundRectRgn` / `SetWindowRectRgn` / `ClearWindowRgn`（`IRender.h#L1412-L1429`）。
- 现状：`GetRenderDC` / `ReleaseRenderDC` 已用 `DUILIB_BUILD_FOR_WIN` 保护；三个 RGN 接口仍未加平台条件编译，其他平台实现只能空实现。
- 后续：给 RGN 接口补 `DUILIB_BUILD_FOR_WIN`，或随第五节第 1 条一起移到窗口层接口。

### 6. 注释中直接写引擎名 — **部分修复**

- 已清理 `IBitmap::Init` 的"只有 Skia 引擎需要此参数"、`DrawArc` 的"仅 Skia 引擎使用"等表述（`IRender.h#L1095-L1106` 现为行为描述）。
- 建议后续普查全部公共头文件，注释统一描述"行为与限制"，不写具体后端名。

## 四、接口一致性问题

> 状态：本组问题截至 2026-11-13 **均未处理**（字体回退优化未触及这些接口），下列行号已按当前代码校准，可直接作为后续修改的定位依据。

### 1. `Clone` 家族所有权与 const 不统一

| 接口 | 位置 | 返回 | const |
| --- | --- | --- | --- |
| `IRender::Clone` | `IRender.h#L1363` | `std::unique_ptr<IRender>` | 否 |
| `IBitmap::Clone` | `IRender.h#L186` | `IBitmap*`（裸指针） | 否 |
| `IPath::Clone` | `IRender.h#L425` | `IPath*`（裸指针） | 否 |
| `IPen::Clone` | `IRender.h#L281` | `IPen*`（裸指针） | **是** |
| `IBrush::Clone` | `IRender.h#L290` | `IBrush*`（裸指针） | 否 |
| `IFont` / `IMatrix` | — | 无 Clone | — |

- 建议：统一为 `std::unique_ptr<T> Clone() const`。至少要统一 const 修饰，避免"有的能克隆 const 对象、有的不能"。

### 2. 只读接口缺 const

- `IPath::GetFillType()`（`IRender.h#L327`）、`IPath::GetBounds()`（`IRender.h#L413`）、`IRender::GetClipInfo()`（`IRender.h#L1393`）都只读取状态，应为 const。
- 只读指针参数也应加 const：`IPath::Transform(IMatrix*)`（`IRender.h#L408`）、`DrawArc` 的 `UiColor* gradientColor`（`IRender.h#L1106`，实现中只读）、`DrawImage(IBitmap*)`（`IRender.h#L986`、`#L994`）、`DrawImageRect(IMatrix*)`（`IRender.h#L1008`）。
- 字体回退优化后新增/保留的 `IFont::IsUnicodeCharSupported(uint32_t)`（`IRender.h#L50`）同样只读，建议一并加 `const`，与 `GetFontMgr() const` 保持一致。

### 3. 两套裁剪状态机制并存

- `SaveClip(int32_t&)` / `RestoreClip(int32_t)`（`IRender.h#L899`、`#L904`）与 `SetClip(...)` / `SetRoundClip(...)` / `ClearClip(int32_t)`（`IRender.h#L911`、`#L920`、`#L925`）功能重叠，状态码的来源与含义还不一致：前者通过出参返回，后者通过返回值返回。
- 建议：只保留一套（保留 `SaveClip/RestoreClip` 作为底层状态栈，`SetClip` 内部复用），或让 `SetClip` 明确为 `SaveClip` + `clipRect` 的组合语义。

### 4. `MeasureRichText` 编号式重载膨胀

- 位置：`IRender.h#L1172-L1208`（`MeasureRichText` / `MeasureRichText2` / `MeasureRichText3`）。
- 三个接口只差输出参数：是否输出行信息、是否创建绘制缓存。
- 建议：合并为一个接口，用默认 `nullptr` 的可选输出参数控制：
  ```cpp
  virtual void MeasureRichText(const UiRect& textRect,
                               const UiSize& szScrollOffset,
                               IRenderFactory* pRenderFactory,
                               const std::vector<RichTextData>& richTextData,
                               RichTextLineInfoParam* pLineInfoParam = nullptr,
                               std::shared_ptr<DrawRichTextCache>* spDrawRichTextCache = nullptr,
                               std::vector<std::vector<UiRect>>* pRichTextRects = nullptr) = 0;
  ```

### 5. 返回值约定不一致

- `MeasureString` 返回 `UiRect`，`MeasureRichText*` 返回 `void`（结果只能从出参拿）。
- 绝大多数绘制接口返回 `void`，`CreateDrawRichTextCache` / `WritePixels` / `ReadPixels` 返回 `bool`。
- 建议：测量类统一返回尺寸；绘制类要么统一 `void`（并明确"内部 ASSERT + 静默忽略"），要么统一 `bool`。不要同一族接口两种风格混用。

### 6. "未设置"用魔法值表达

- 位置：`MeasureStringParam::rectSize = DUI_NOSET_VALUE`（`IRender.h#L827`）。
- 同一字段在横向文本表示"限制宽度"、纵向文本表示"限制高度"，语义随 `uFormat` 变化，且用魔法值表示"不设置"。
- 建议：改为 `int32_t limitWidth = -1; int32_t limitHeight = -1;`，或使用 `std::optional<int32_t>`。

### 7. 文本间距模型分裂

- `DrawStringParam` / `MeasureStringParam` 只有 `fWordSpacing`（`IRender.h#L834`、`#L850`），`RichTextData` 有 `m_fLetterSpacing`（`IRender.h#L605`）。
- 两者都描述"字与字之间的间距"，命名和单位却不统一，容易误用。字体回退优化后两个字段仍并存（内部实现分别处理），建议在接口注释中互相引用，或统一命名（如 `letterSpacing`）和语义。

### 8. 取值风格不一致

- `IFont::FontName()` 按值返回 `DString`（`IRender.h#L24`），`IFontMgr::GetFontName(uint32_t, DString&)` 用出参（`IRender.h#L91`）。建议统一（优先按值返回，或返回 `const DString&`）。

## 五、职责过载与结构性问题

### 1. `IRender` 承担了过多职责

当前 `IRender` 同时包含：

1. 图元绘制与路径；
2. 文本测量与绘制、富文本缓存；
3. 裁剪与视区原点；
4. 位图读写与快照（`ReadPixels` / `WritePixels` / `MakeImageSnapshot`）；
5. 窗口刷新（`PaintAndSwapBuffers`）；
6. 窗口形状（`SetWindowRoundRectRgn` 等）；
7. DPI 与平台 DC（`SetRenderDpi` / `GetRenderDC`）。

建议拆分为：

- `IRender`：绘制 + 裁剪 + 文本 + 位图快照；
- `IRenderWindow`（或 `IRenderSurface`）：`Resize`、`PaintAndSwapBuffers`、`SetWindow*Rgn`、`GetRenderDC/ReleaseRenderDC`。

收益：离屏渲染（导出图片、控件离屏缓存）不再被迫依赖窗口相关接口；跨平台实现也不需要为窗口接口写空实现。

### 2. 富文本调用每次都传 `IRenderFactory*`

- 位置：`MeasureRichText*` / `DrawRichText` / `CreateDrawRichTextCache` 等（`IRender.h#L1165-L1300`）。
- `IRender` 本身就是 `IRenderFactory::CreateRender` 创建的，字体工厂应由 Render 创建时注入。
- 建议：`CreateRender` 时注入并保存 factory，新增 `IRenderFactory* GetRenderFactory() const`；富文本接口不再要求调用方传 factory（可保留重载以兼容）。

### 3. `rcPaint` 脏区参数把优化责任推给调用方

- 位置：`DrawImage` 系列（`IRender.h#L986-L1010`）。
- 每次绘制图片都要传"当前全部可绘制区域"，漏传或传错会导致多画/少画；脏区判断本可以由 Render 内部的 clip 统一处理。
- 建议：保留该参数（现有调用方较多），但在注释中明确"传 `UiRect()` 表示不限制"，或内部用当前 clip 兜底。

### 4. 测量依赖画布尺寸

- 位置：`Render_Skia::MeasureString` 在 `GetWidth() <= 0 || GetHeight() <= 0` 时直接返回空（`Render_Skia.cpp#L1692-L1695`）。
- 影响：无 Surface 的纯测量场景（如提前估算布局）拿不到结果。
- 建议：让测量不依赖画布尺寸；或在 `IFont` 上提供 `MeasureTextWidth` 之类的轻量度量接口（见第六节）。

## 六、常用但缺失的接口（建议补充）

> 状态：截至 2026-11-13 **均未新增**（字体回退优化只调整了字体相关接口，未触及图元、裁剪、采样与位图读取能力）。以下接口描述可直接作为需求条目排期。

### P0：高频、Skia 实现成本低

```cpp
// 图元：当前画椭圆只能手工构造 Path
virtual void DrawEllipse(const UiRectF& rc, UiColor penColor, float fWidth) = 0;
virtual void DrawEllipse(const UiRectF& rc, IPen* pen) = 0;
virtual void FillEllipse(const UiRectF& rc, UiColor color, uint8_t uFade = 255) = 0;

// 多边形：IPath::AddPolygon 已存在，但 IRender 没有对应快捷方法
//（图表、箭头、异形按钮会用到）
virtual void DrawPolygon(const UiPointF* pts, int32_t count, IPen* pen) = 0;
virtual void FillPolygon(const UiPointF* pts, int32_t count, IBrush* brush) = 0;

// 非矩形裁剪：圆形头像、异形卡片（目前只有矩形/圆角矩形裁剪）
virtual int32_t SetPathClip(const IPath* path, bool bIntersect = true) = 0;

// 图像缩放质量：当前全部使用 SkSamplingOptions()（即最近邻）
// 见 DrawSkiaImage.cpp#L195-L208、Render_Skia.cpp#L376 等处
virtual void SetImageSamplingQuality(ImageSamplingQuality quality) = 0;
```

`SetImageSamplingQuality` 是最值得优先补的一项：DPI 缩放和缩略图场景下，最近邻采样会造成明显锯齿。

### P1：文本与位图能力

- `IFont`：
  - `bool GetFontMetrics(FontMetrics& metrics) const`（ascent / descent / lineGap / 行高）；
  - `float MeasureTextWidth(const DString& text) const`（不依赖 Render/Canvas 的快速测量）；
  - `bool GetFontInfo(UiFont& fontInfo) const`（回读创建参数，便于缓存键比较）。
- `IRender`：
  - `UiRect MeasureString(const DString& text, IFont* font, uint32_t uFormat)` 轻量重载，避免每次拼装 `MeasureStringParam`；
  - `IBitmap* MakeImageSnapshot(const UiRect& rc)`：当前只能整画布快照，做局部缓存不方便。
- `IBitmap`：
  - `bool IsValid() const`、`uint32_t GetRowBytes() const`、`GetAlphaType()`、`GetPixelFormat()`；
  - `float GetImageSizeScale() const`（`Init` 有该入参，却没有读取接口）；
  - `const void* LockPixelBits() const`（只读访问，避免调用方误改）。
- 分组透明度：
  ```cpp
  virtual void SaveLayer(const UiRect& rc, uint8_t alpha = 255) = 0;
  virtual void RestoreLayer() = 0;
  ```
  用于整组淡出/离屏合成。当前只能逐元素乘 Alpha，重叠区域会出现叠加误差。

### P2：抽象能力完善

- `IMatrix`（`IRender.h#L430`）只有变换、没有读取：补 `Reset()`、`Concat(const IMatrix&)`、`Invert()`、`MapPoint`、`IsIdentity()`。
- `IPen`（`IRender.h#L191`）：补 `SetAntiAlias`、`SetMiterLimit`、`SetDashPattern(const float*, int32_t)`（现在虚线只能用 5 种预设样式）。
- `IBrush`（`IRender.h#L287`）目前只支持纯色：建议支持线性/径向渐变。现在 `FillRect`、`FillRoundRect`、`FillPath` 各有一套"两色 + 4 方向"的渐变参数，重复且表达能力有限。
- `IRender`：补 `SetBlendMode`（Multiply / Screen / Xor / Clear）与 `SetTransform` / `ConcatMatrix`（整组旋转缩放）。
- `IRenderFactory`：补 `CreateBitmap(width, height, alphaType)` 便捷重载（`GetRenderType` 已随第三节第 3 条删除，不再新增）。

## 七、建议的落地顺序

1. **第一批（低风险、不改调用方）— 已完成约一半**
   - ✅ 注释与拼写修正：`CreateBrush` 的 `corlor`、`ClearClip` 的 `@return`、`MeasureRichText` 的参数说明、`HasFontName` 的 `@param`；
   - ✅ `RopMode` 收敛（补齐四种模式的映射，并在注释中标注限制）；
   - ✅ `uFade` 语义明确为"与颜色 Alpha 相乘"，绘制类接口全部生效；
   - ✅ `WritePixels` 源数据改为 `const void*`；
   - ⬜ **未完成：const 修正**——`GetFillType`、`GetBounds`、`GetClipInfo`、`Transform(IMatrix*)`、`DrawArc` 的 `gradientColor`、`DrawImage(IBitmap*)`、`DrawImageRect(IMatrix*)`，以及 `IFont::IsUnicodeCharSupported`。

2. **第二批（只增不改，补常用能力）— 未开始**
   - `DrawEllipse` / `FillEllipse`；
   - `DrawPolygon` / `FillPolygon`；
   - `SetPathClip`；
   - `SetImageSamplingQuality`；
   - `IFont` 的度量接口。

3. **第三批（结构性调整，需评估影响面）— 部分完成**
   - ✅ `BitmapAlphaType` 去 Skia 化（已改为中性 `AlphaType`）；
   - ⬜ 拆分 `IRender` 的窗口/Surface 职责；
   - ⬜ 合并 `MeasureRichText*` 编号重载；
   - ⬜ `IBrush` 支持渐变；
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

**未变化的接口**（调用方无需改动）：`DrawString` / `MeasureString` / `DrawRichText` / `MeasureRichText*` / `CreateDrawRichTextCache` / `DrawRichTextCacheData`、`RichTextData`、`DrawStringParam`、`MeasureStringParam`，以及 `IBitmap` 除 `AlphaType` 命名外的其它方法。

**渲染层内部类型（不属于公共接口，可自由调整）**：`DrawSkiaText` / `SkTextBox` / `TextGlyphRunList` / `TextLineLayout` 均在 `duilib/RenderSkia` 目录内，`SkGlyphID`、`SkFont`、run 批量绘制等细节只出现在这些头文件中；`IRender.h` 中已不再出现 Skia 类型。

**测试覆盖**：`tests/test_text_draw.cpp` 通过真实的 `FontMgr_Skia` + 主字体 + 回退字体链，覆盖"主字体不支持 → 回退字体命中 → glyph 0 时绘制方框占位"的完整链路，以及测量、断行、run 构建与横排/竖排绘制；`ctest` 已注册 `text_draw` 用例。
