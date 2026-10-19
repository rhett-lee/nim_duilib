# Rating 评分（星级）控件 Spec

## Why
`docs/misc/Project-Audit.md` 的 **B5** 登记「Rating 评分（星级）」为待办（`Development-Plan.md` 的 **D-11** 亦登记；全仓无 Rating 命中）。评价类界面（打分、满意度调查、商品/服务评价）高频使用，需要一个可主题化、支持半星与只读展示、可被鼠标与事件驱动的评分控件。

## What Changes
- 新增 `Rating` 控件（继承 `Control`），**星形外观完全由 SVG 图片驱动**（不使用 `IPath` 自绘星形）：
  - **显示**：按 `value` 与 `max` 逐颗绘制实心 / 半星 / 空心图片。
  - **主题自适应**：图片经 `svg_replace_colors`（如 `#0078D4|color_accent`）把占位色映射到主题语义色，切换主题时颜色由皮肤层自动接管，与 `Stepper` / `Slider` 既有范式一致。
  - **交互**：非只读时鼠标悬停预览、点击 / 拖拽打分；只读时纯展示；控件禁用时置灰且不可交互。
  - **属性**：`value` / `max` / `allow_half` / `read_only` / `star_size` / `star_gap` / 四张星图（实心 / 半星 / 空心 / 悬停）。
  - **事件**：值变化派发 `kEventValueChanged`（`AttachRatingChanged`），参数为 `RatingValue{value, oldValue}`。
- 新增 3–4 张星形 SVG 资源（实心 / 半星 / 空心 / 可选悬停），并在默认皮肤注册 `rating` 类。
- 端到端接线：类型常量、工厂注册、构建工程、演示窗口、示例入口、多语言文案。
- 文档同步：`docs/Control.md`、`docs/Control.en.md`、两份 `api-reference.md`、`nim-duilib-llm-reference.md`。
- **非 BREAKING**：全部为新增，不改动既有控件行为。

## Impact
- Affected specs：`Project-Audit.md` B5；`Development-Plan.md` D-11（Rating 部分）。
- Affected code：见下方「改动文件清单」。

## ADDED Requirements

### Requirement: Rating 星级评分控件
系统 SHALL 提供可主题化、支持半星与只读模式、可被鼠标与事件驱动的星级评分控件 `<Rating>`，其星形外观 SHALL 由 SVG 图片（而非代码自绘）呈现，并 SHALL 通过 `svg_replace_colors` 跟随主题语义色。

#### Scenario: 只读展示
- **WHEN** `read_only="true"` 且 `value="4.5"`、`allow_half="true"`
- **THEN** 渲染 4 个实心星 + 1 个半星，鼠标点击 / 悬停均不改变数值。

#### Scenario: 交互打分
- **WHEN** 非只读，用户点击第 3 颗星（整数模式）
- **THEN** `value` 变为 3，并派发 `kEventValueChanged`（携带新值与旧值）。

#### Scenario: 半星模式
- **WHEN** `allow_half="true"`，用户点击第 3 颗星的右半边
- **THEN** `value` 变为 3.5。

#### Scenario: 悬停预览
- **WHEN** 非只读，鼠标悬停在第 4 颗星上
- **THEN** 前 4 颗星以预览样式（配置了 `star_hovered_image` 则用之，否则回退实心星）显示；鼠标移出后恢复为 `value` 对应的显示。

#### Scenario: 主题切换
- **WHEN** 运行时切换主题
- **THEN** 实心 / 半星 / 空心星颜色随 `svg_replace_colors` 映射的语义色（`color_accent` / `border_control_normal` 等）自动变化，无需改动 C++ 代码。

#### Scenario: 值钳位
- **WHEN** 设置 `value` 超出 `[0, max]`，或 `max` 被调小到小于当前 `value`
- **THEN** `value` 自动钳位到有效范围，并按 `allow_half` 对齐到步长。

## 设计与约定

### XML 属性
| 属性名称 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| value | 0 | double | 当前评分值，钳位到 `[0, max]` |
| max（别名 star_count） | 5 | int | 星星数量 |
| allow_half | false | bool | 是否允许半星（步长 0.5，否则 1） |
| read_only（别名 readonly） | false | bool | 只读展示模式，禁止鼠标交互 |
| star_size | 20 | int | 单颗星绘制直径（像素，自动 DPI 缩放；通过 `PaintImage` 的 `dest` 覆盖图片固有尺寸） |
| star_gap | 4 | int | 星星间距（像素，自动 DPI 缩放） |
| star_filled_image（别名 star_full_image） |  | 图片属性串 | 实心星图片（必配），如 `file='${PATH_RATING}/star_filled.svg' width='20' height='20' svg_replace_colors='#0078D4\|color_accent'` |
| star_half_image |  | 图片属性串 | 半星图片（可选）；未配置时对实心星做左半裁剪绘制 |
| star_empty_image |  | 图片属性串 | 空心星图片，如映射到 `border_control_normal` |
| star_hovered_image（别名 star_hover_image） |  | 图片属性串 | 悬停预览星图片（可选）；未配置时回退使用实心星 |

`Rating` 继承 `Control` 的全部属性；默认皮肤的 `rating` 类已内置上述三张星图与语义色映射，可直接 `class="rating"` 引用。

### C++ 接口
```cpp
enum class StarImageType { kFilled = 0, kHalf = 1, kEmpty = 2, kHovered = 3 };

struct RatingValue { double value = 0; double oldValue = 0; };

double  GetValue() const;
void    SetValue(double value);       // 钳位 + 对齐步长，变化时派发事件
void    SetValueNoEvent(double value);
int32_t GetMax() const;               // 别名 GetStarCount
void    SetMax(int32_t nMax);         // 调小 max 时同步钳位 value
bool    IsAllowHalf() const; void SetAllowHalf(bool);
bool    IsReadOnly() const;  void SetReadOnly(bool);
int32_t GetStarSize() const; void SetStarSize(int32_t);
int32_t GetStarGap() const;  void SetStarGap(int32_t);
void    SetStarImage(StarImageType type, const DString& strImage);  // 设置某类星图属性串
DString GetStarImage(StarImageType type) const;
void    SetFilledImage(const DString&);  void SetHalfImage(const DString&);
void    SetEmptyImage(const DString&);   void SetHoveredImage(const DString&);
void    AttachRatingChanged(const EventCallback& callback, EventCallbackID id = 0); // kEventValueChanged
void    ClearImageCache() override;   // 释放星图缓存
```

### 绘制约定（图片驱动，颜色走 SVG 主题映射）
- 星图载体使用 `StateImage`（每类星图各一个实例，图片存入 `kControlStateNormal`），绘制用 `Control::PaintImage(IRender*, Image*, strModify, ...)`；`strModify` 携带 `destscale='false' dest='x,y,x,y'`（控件相对坐标），把星图缩放到 `star_size` 对应的目标位置。
- 逐颗计算：`ratio = clamp(value - i, 0, 1)`（`i` 为星索引）。`ratio >= 1` → 实心图；`ratio == 0` → 空心图；`0 < ratio < 1`（半星模式）→ 有 `star_half_image` 则用半星图，否则对实心图 `SaveClip / SetClip(左半矩形) / PaintImage / ClearClip`（**图片裁剪，非自绘**）。
- 悬停预览：非只读且有悬停时，预览范围内的星优先用 `star_hovered_image`，未配置则回退实心图。
- 布局：从左到右排列 `max` 颗星，控件矩形内垂直居中；`star_size` / `star_gap` 均经 `Dpi().GetScaleInt()` 缩放。
- 颜色全部由图片的 `svg_replace_colors` 决定（映射到语义色名），C++ 侧不持有任何颜色值。

### 交互约定
- `ButtonDown`：非只读 → 命中星索引与左右半边 → `SetValue`（派发事件），进入拖动状态。
- `MouseMove`：按下时拖拽连续更新；未按下时更新悬停预览并 `Invalidate`。
- `ButtonUp` / `MouseLeave` / 窗口失焦：结束拖动 / 清除悬停预览。
- `IsEnabled()==false` 时整体降低透明度且不响应交互。

## 改动文件清单
1. `duilib/duilib_defs.h`：新增 `#define DUI_CTR_RATING _T("Rating")`（参照 `DUI_CTR_STEPPER`）。
2. `duilib/Control/Rating.h`（新增）：类声明、`StarImageType` / `RatingValue`、XML 属性与 C++ 接口注释。
3. `duilib/Control/Rating.cpp`（新增）：属性解析、取值/钳位、星图缓存、图片驱动绘制、交互与事件。
4. `duilib/Core/WindowBuilder.cpp`：`#include "duilib/Control/Rating.h"`；在 `createControlMap` 注册 `{DUI_CTR_RATING, [](Window* pWindow){ return new Rating(pWindow); }}`。
5. `duilib/duilib.vcxproj` 与 `duilib.vcxproj.filters`：新增 `Control\Rating.cpp` / `Control\Rating.h` 条目。
6. `bin/resources/themes/default/public/rating/`（新增目录）：`star_filled.svg`、`star_half.svg`、`star_empty.svg`（可选 `star_hovered.svg`），均含 `#0078D4` 占位色以便 `svg_replace_colors` 映射。
7. `bin/resources/themes/default/global.xml`：新增 `<Var name="PATH_RATING" ...>` 与 `<Class name="rating">`（三/四张星图 + 语义色映射）。
8. `bin/resources/themes/default/controls/rating.xml`（新增）：演示窗口（只读平均分、可交互 5 星、半星、事件回显与操作按钮）。
9. `bin/resources/themes/default/controls/test.xml`：在 Stepper 按钮同排新增 `rating_btn`。
10. `examples/controls/RatingForm.h` / `.cpp`（新增）：演示窗口逻辑（事件回显、清零 / 设置 4.5）。
11. `examples/controls/TestForm.h` / `.cpp`：新增 `ShowRatingWindow()` 与 `rating_btn` 点击处理。
12. `examples/controls/controls.vcxproj` 与 `controls.vcxproj.filters`：新增 `RatingForm.h` / `.cpp` 条目。
13. `bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt`：新增 `STRID_RATING_*` 文案。
14. `docs/Control.md`、`docs/Control.en.md`：新增「Rating 控件（星级评分）」小节（属性表 + 说明 + XML 示例 + C++ 接口）。
15. `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：同步 Rating 条目。
16. `.claude/docs/nim-duilib-llm-reference.md`：速查行补充 Rating。

## 验证方式
- **编译**：`build/examples_no_cef.sln` → Configuration `Debug` / Platform `x64` 编译通过（无错误）。
- **资源**：三张 SVG 存在且 `class="rating"` 正确引用，`svg_replace_colors` 映射到语义色。
- **运行时**（DPI 感知脚本 + 截图）：打开 Rating 演示窗口，逐项核对——
  1. 只读行按 `4.5` 显示 4 实心 + 1 半星；
  2. 悬停第 4 星出现预览样式，移出恢复；
  3. 点击第 3 星（整数模式）→ 事件回显 `3`；
  4. 半星行点击某星右半边 → 出现半星且回显 `x.5`；
  5. 未配置 `star_hovered_image` 时预览回退实心星；
  6. 有主题切换入口时，切换主题后星色跟随变化（无则核对 `svg_replace_colors` 配置正确）。
- **文档**：`verify_docs.py` 全量扫描无漂移。
- **回归**：既有控件与示例行为不变。

## 非目标（Out of Scope）
- 键盘（方向键 / Home / End）调分：本期不做，留待后续（框架焦点模型需另行确认）。
