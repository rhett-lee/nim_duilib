# Spec: B3 RangeSlider 双滑块范围控件

## 1. 目标

为 nim_duilib 新增双滑块范围选择控件 `RangeSlider`（节点名 `RangeSlider`），用于价格 / 时间区间筛选等场景；现有 `Slider` 只有单值。完成后在 [Project-Audit.md](../../Project-Audit.md) 的 B3 行标注 ✅ 已完成。

用户约束：
- 代码提交由用户手动执行，助手不执行 `git commit`。
- 只需保证 **Debug|x64** 编译通过（不编译 Release）。

## 2. 需求要点（来自盘点表 B3）

- 双滑块（下限 / 上限两个 thumb）。
- 可高亮显示被选中的区间（range track）。
- 提供 XML 属性与 C++ API，含值变化回调。
- 避免值同步自递归（沿用 A1/A3 结论：提供 `SetXXXNoEvent`）。

## 3. 设计决策

### 3.1 继承与取值模型

- `class DUILIB_API RangeSlider : public Progress`，`typedef Progress BaseClass;`
- 复用 `Progress` 的 `min/max`（允许区间边界）与内部当前值 `m_fCurrentValue` 作为**上限值**（`GetHighValue() == GetValue()`）。
- 新增成员 `m_fLowValue` 作为**下限值**；下限默认等于 `GetMinValue()`（初值 0；当 `min` 上抬时通过钳位自然跟随）。
- `SetHighValue` 内部调用 `Progress::SetValueNoEvent`（抑制基类事件），由 `RangeSlider` 自身统一派发一次范围事件；避免双事件与自递归。
- 新增噪声抑制守卫 `m_bMuteRangeEvent`，供 `SetLowValueNoEvent / SetHighValueNoEvent / SetRangeNoEvent` 使用。

### 3.2 事件编码（沿用 Calendar 模式）

- 事件类型：`kEventValueChanged`。
- 编码：`wParam = 1`（范围模式标记），`lParam = 0`，`pEventData = &RangeValue{ lowValue, highValue }`（栈上临时对象，仅本次同步派发有效）。
- 提供结构体 `struct RangeValue { double lowValue; double highValue; };`。
- 便捷接口：`void AttachRangeChanged(const EventCallback&, EventCallbackID = 0) { AttachEvent(kEventValueChanged, ...); }`。

### 3.3 交互

- 鼠标按下：命中某 thumb → 拖动该 thumb；未命中且 `click_to_jump` 开启 → 移动「离点击值更近」的 thumb 并进入拖动。
- 鼠标移动：更新 hover 目标 thumb；拖动中按鼠标位置设值（带钳位 `low <= high - min_range`）。
- 鼠标松开：结束拖动（`click_to_jump=false` 时在松开处才跳值）。
- 键盘：本次不实现（非 B3 必需，避免过度设计）。

### 3.4 绘制

- 轨道 = 控件自身 `bkcolor/bkimage`（由基类 Control 常规绘制），不引入 `progress_bar_padding`。
- 高亮区间：以 `ratio=(v-min)/(max-min)` 计算 `[low, high]` 区间矩形；水平取 `left+rcLow.right .. left+rcHigh.right`，垂直取 `top+rcHigh.top .. top+rcLow.top`；颜色取 `range_color`，未设则回退基类 `progress_color`；`range_radius>0` 用 `FillRoundRect`，否则 `FillRect`。
- 两个 thumb：复用 `StateImage`（下限 `m_thumbStateImage`、上限 `m_highThumbStateImage`，上限为空时回退下限图片）；状态优先级：disabled → pressed(拖动中) → hovered(hover 该 thumb) → normal。

## 4. 精确改动清单

### 4.1 新增控件源码

**`duilib/Control/RangeSlider.h`（新建）** — `class RangeSlider : public Progress`：
- 覆写：`GetType`、`SetAttribute`、`HandleEvent`、`PaintStateImages`、`ClearImageCache`、`ChangeDpiScale`。
- 公开 API：
  - `double GetLowValue() const; void SetLowValue(double); void SetLowValueNoEvent(double);`
  - `double GetHighValue() const; void SetHighValue(double); void SetHighValueNoEvent(double);`
  - `void SetRange(double low, double high); void SetRangeNoEvent(double low, double high);`
  - `double GetMinRange() const; void SetMinRange(double);`
  - `UiSize GetThumbSize() const; void SetThumbSize(UiSize, bool bNeedDpiScale);`
  - `UiRect GetLowThumbRect() const; UiRect GetHighThumbRect() const;`
  - `DString GetThumbStateImage(ControlStateType) const; void SetThumbStateImage(ControlStateType, const DString&);`（下限）
  - `DString GetHighThumbStateImage(ControlStateType) const; void SetHighThumbStateImage(ControlStateType, const DString&);`（上限）
  - `void SetRangeColor(const DString&); const DString& GetRangeColor() const;`
  - `void SetRangeRadius(int32_t); int32_t GetRangeRadius() const;`
  - `void SetClickToJump(bool); bool IsClickToJump() const;`
  - `struct RangeValue { double lowValue; double highValue; };`
  - `void AttachRangeChanged(const EventCallback&, EventCallbackID = 0);`
- 私有成员：`m_fLowValue`、`m_fMinRange`、`m_szThumb`、`m_thumbStateImage`、`m_highThumbStateImage`、`m_strRangeColor`、`m_nRangeRadius`、`m_bClickToJump`、`m_bMuteRangeEvent`、`m_dragThumb`(枚举 None/Low/High)、`m_hoverThumb`(枚举)、`m_sImageModify`。
- 私有辅助：`ClampLowValue()`、`ThumbRectFromValue(double) const`、`CalcRangeRect(double) const`、`ValueFromMousePt(const UiPoint&) const`、`EmitRangeChanged(double oldLow, double oldHigh)`。

**`duilib/Control/RangeSlider.cpp`（新建）**：
- 构造：`m_fLowValue(0)`、`m_fMinRange(0)`、`m_szThumb(0,0)`、`m_nRangeRadius(0)`、`m_bClickToJump(true)`、`m_bMuteRangeEvent(false)`、`m_dragThumb(kNone)`、`m_hoverThumb(kNone)`；`SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER, false)`（对齐 Slider）。
- `GetType() { return DUI_CTR_RANGE_SLIDER; }`
- 取值/设值按 §3.1 钳位；仅当值实际变化才派发事件；`*NoEvent` 期间置 `m_bMuteRangeEvent=true`。
- `SetAttribute`：处理 `low_value`、`high_value`、`value`(→高值)、`min_range`、`range_color`、`range_radius`、`thumb_size`、`thumb_*`(下限 4 态)、`high_thumb_*`(上限 4 态)、`click_to_jump`；`min`/`max` 先调 `BaseClass::SetAttribute` 再 `ClampLowValue()`；其余回退 `BaseClass::SetAttribute`。
- 绘制/命中公式复用 [Slider.cpp](file:///c:/develop/nim_duilib/duilib/Control/Slider.cpp) 的 `GetThumbRect` / `ValueFromMousePt`（水平 + 垂直两分支）。
- `ChangeDpiScale`：缩放 `m_szThumb` 后调用 `BaseClass::ChangeDpiScale`。
- `ClearImageCache`：`BaseClass` + 两个 `StateImage`。

### 4.2 控件注册（4 处）

1. `duilib/duilib_defs.h`：在 `DUI_CTR_SLIDER` 附近新增 `#define DUI_CTR_RANGE_SLIDER (_T("RangeSlider"))`。
2. `duilib/Core/WindowBuilder.cpp`：`#include "duilib/Control/RangeSlider.h"`；`CreateControlMap()` 新增 `{DUI_CTR_RANGE_SLIDER, [](Window* pWindow) { return new RangeSlider(pWindow); }}`。
3. `duilib/duilib.vcxproj`：`<ClCompile Include="Control\RangeSlider.cpp" />` 与 `<ClInclude Include="Control\RangeSlider.h" />`。
4. `duilib/duilib.vcxproj.filters`：上述两项 `<Filter>Control</Filter>`。
5. `duilib/duilib.h`：新增 `#include "Control/RangeSlider.h"`。

### 4.3 示例

- **新建 `examples/controls/RangeSliderForm.h/.cpp`**：`class RangeSliderForm : public ui::WindowImplBase`（自带 `typedef ui::WindowImplBase BaseClass;`）；`GetSkinFolder()="controls"`、`GetSkinFile()="rangeslider.xml"`；`OnInitWindow()` 绑定：
  - `price_range` → `AttachRangeChanged` → 更新 `rs_event` 标签（前缀 + `%.0f ~ %.0f`）。
  - `btn_range_reset` → `SetRange(默认)`；`btn_range_full` → `SetRange(min,max)`。
- **新建 `bin/resources/themes/default/controls/rangeslider.xml`**：标准标题栏 + 提示 Label + 内容卡片；含水平 `RangeSlider name="price_range" min="0" max="1000" low_value="200" value="800" min_range="50" range_color="bg_progress"`、垂直示例 `Level_range`、`rs_event` 标签与两个按钮；颜色名用现有语义色（`bg_window_main/bg_window_card/bg_titlebar/border_window/text_default/text_muted/bg_progress`）。
- `bin/resources/themes/default/controls/controls.xml`：第 2 列 `VBox` 追加 `<Button class="btn_global" name="rangeslider_btn" text_id="STRID_CONTROLS_RANGESLIDER_TEST" width="110" height="30" border_round="3,3" margin="0,2,0,0"/>`；首排 `HBox height` 由 `288` 调至 `320`。
- **语言资源**：`bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt` 各新增 `STRID_CONTROLS_RANGESLIDER_TEST`、`STRID_RANGESLIDER_WINDOW_TITLE`、`STRID_RANGESLIDER_HINT`、`STRID_RANGESLIDER_PRICE`、`STRID_RANGESLIDER_LEVEL`、`STRID_RANGESLIDER_RESET`、`STRID_RANGESLIDER_FULL`、`STRID_RANGESLIDER_EVENT_NONE`、`STRID_RANGESLIDER_EVENT_PREFIX`。
- `examples/controls/controls.vcxproj` + `.filters`：新增 `RangeSliderForm.h`（ClInclude）与 `RangeSliderForm.cpp`（ClCompile）。
- `examples/controls/ControlForm.h`：新增 `void ShowRangeSliderWindow();`。
- `examples/controls/ControlForm.cpp`：`#include "RangeSliderForm.h"`；`OnInitWindow` 绑定 `rangeslider_btn` → `ShowRangeSliderWindow()`；实现该方法（`kWS_POPUP + kWS_EX_LAYERED + m_bCenterWindow`，`ShowModalFake()`）。

### 4.4 文档同步

- `docs/Control.md` / `docs/Control.en.md`：新增 RangeSlider 条目。
- `docs/Summary.md` / `docs/Summary.en.md`：控件清单补充。
- `README.md` / `README.en.md`：控件计数同步（按现状 +1）。
- `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：API 参考补充。
- `.claude/docs/nim-duilib-llm-reference.md`：补充。
- `docs/misc/Project-Audit.md`：B3 行改为 ✅ 已完成（列出 `duilib/Control/RangeSlider.h/.cpp`、节点 `RangeSlider`、示例 `examples/controls/RangeSliderForm`）。

## 5. 验证步骤

1. **编译**：`MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly` 直至无错误。
2. **运行时核对**（DPI-aware 辅助脚本，如已删除则重建）：
   - 启动 demo → 打开「范围滑块测试」窗口 → 截图确认：双 thumb + 高亮区间 + 垂直示例。
   - 真实鼠标拖动下限/上限 thumb → 截图确认高亮区间随之变化，`rs_event` 标签更新为 `low ~ high`。
   - 点击「重置区间 / 全选区间」按钮 → 标签回显对应范围。
3. **文档漂移**：`python ".trae/skills/nim-duilib/scripts/verify_docs.py" --repo "c:/develop/nim_duilib" --extra <本次改动的 md 文件>` → 期望「全部命中，未发现文档漂移」。
4. **盘点表**：确认 B3 行已标注 ✅。

## 6. 验收标准

- Debug|x64 编译无错误。
- `RangeSlider` 节点可在 XML 中创建并被 `FindControl` 找到。
- 双 thumb 可拖动、区间高亮正确、`AttachRangeChanged` 回调携带 `low/high`。
- `SetLowValueNoEvent / SetHighValueNoEvent / SetRangeNoEvent` 不派发事件（无自递归）。
- 文档同步完成且 `verify_docs.py` 无漂移；Project-Audit B3 标记完成。

## 7. 待用户确认

请确认以上方案（尤其 §3 取值模型与 §3.2 事件编码、§4 文件清单）后，再进入实现。
