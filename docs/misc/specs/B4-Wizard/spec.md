# Spec: B4 Stepper 向导步骤条控件

## 1. 目标

为 nim_duilib 新增向导步骤条控件 `Stepper`（节点名 `Stepper`），用于安装 / 注册 / 配置向导、审批流程、订单进度等"流程状态机"场景。现有 `Timeline` 只能模拟（时间轴语义 ≠ 流程步骤状态机），语义不符；现有导航类控件（`MenuBar` / `NavigationView`）也不承载"步骤"语义。

完成后在 [Project-Audit.md](../../Project-Audit.md) 的 B4 行标注 ✅ 已完成。

用户约束：
- 代码提交由用户手动执行，助手不执行 `git commit`。
- 只需保证 **Debug|x64** 编译通过（不编译 Release）。

## 2. 需求要点（来自盘点表 B4）

- 步骤模型：每步含标题（title）、描述（description）、节点标识（number / checkmark）、状态（pending / active / completed / disabled）。
- 方向：水平（horizontal，默认）/ 垂直（vertical）两种排列。
- 当前步骤可由程序驱动（`Next` / `Back` / `GoTo`），也支持鼠标点击步骤跳转（`click_to_jump`）。
- 值（当前步骤索引）变化时回调，事件复用 `kEventValueChanged`，`pEventData` 携带新旧索引。
- 提供 XML 属性与 C++ API；多语言标题/描述通过 `step_title_id` / `step_desc_id` 绑定，语言切换自动刷新。
- 避免值同步自递归（沿用 A1/A3 结论：提供 `SetCurrentIndexNoEvent` / `GoToNoEvent` 等静默接口）。

## 3. 设计决策

### 3.1 继承与数据模型

- `class DUILIB_API Stepper : public Control`，`typedef Control BaseClass;`（纯自绘，参考 [Timeline.h](file:///c:/develop/nim_duilib/duilib/Control/Timeline.h) 的数据驱动范式）。
- 步骤状态枚举：
  ```cpp
  enum class StepStatus { kPending = 0, kActive, kCompleted, kDisabled };
  ```
- 步骤结构体 `Step { DString title; DString description; }`；**状态不由 Step 自身携带**，而由"当前索引 + 禁用集合"派生：
  - `index < currentIndex` → `kCompleted`
  - `index == currentIndex` → `kActive`
  - `index > currentIndex` → `kPending`
  - `index ∈ disabledSteps` → `kDisabled`（优先于上述三态）
- 当前索引 `m_nCurrentIndex`（默认 0），越界自动钳位到 `[0, stepCount-1]`。

### 3.2 事件编码（沿用 Calendar / RangeSlider 模式）

- 事件类型：`kEventValueChanged`。
- 编码：`wParam = 2`（步骤模式标记，与 RangeSlider 的 `1` 区分），`lParam = 0`，`pEventData = &StepValue{ currentIndex, oldIndex }`（栈上临时对象，仅本次同步派发有效）。
- 提供结构体：
  ```cpp
  struct StepValue { size_t currentIndex = 0; size_t oldIndex = 0; };
  ```
- 便捷接口：`void AttachStepChanged(const EventCallback&, EventCallbackID = 0) { AttachEvent(kEventValueChanged, ...); }`。

### 3.3 导航 API 与静默赋值

- `Next()` / `Back()`：`GoTo(current±1)` 的便捷封装；越界返回 false。
- `GoTo(size_t index)`：目标为禁用步骤时返回 false；索引未变化时不派发事件。
- `GoToNoEvent(size_t)` / `SetCurrentIndexNoEvent(size_t)` / `NextNoEvent()` / `BackNoEvent()`：抑制事件（守卫 `m_bMuteStepEvent`），供程序内部同步使用，避免与业务回调形成自递归。
- `SetCurrentIndex()` 内部先行钳位，再与旧值比较，仅在变化时派发一次事件。

### 3.4 布局与绘制

- 水平：每步占据等宽槽位 `slotW = rc.Width() / n`；节点圆心 `x = rc.left + slotW*(i+0.5)`、`y = rc.top + nodeSize/2 + 上边距`；连接线沿 `y = nodeCenterY` 从上一节点圆心画到下一节点圆心（绘制在节点下层）；标题/描述在节点下方居中。
- 垂直：节点列在左侧，节点圆心 `y = rc.top + slotH*(i+0.5)`（`slotH = rc.Height()/n`）、`x = rc.left + nodeSize/2 + 左边距`；连接线沿 `x = nodeCenterX` 纵向连接；标题/描述在节点右侧左对齐。
- 节点绘制：用状态色填充圆；未完成步骤在圆内绘制**序号**（1 起）；已完成步骤在圆内绘制**对勾**（两段直线）；禁用步骤整体降低透明度。
- 连接线颜色：位于当前索引之前的段（已完成）用 `connector_active_color`，其余用 `connector_color`。
- 标题/描述颜色按状态取色：active → `title_color_active`，completed → `title_color_completed`，pending → `title_color_pending`，disabled → `node_color_disabled`；描述统一用 `desc_color`。

### 3.5 交互

- `ButtonDown`：命中某步骤槽位（整槽命中，便于点击）→ 若 `click_to_jump` 且该步非禁用 → `GoTo(index)`。
- 命中检测使用与绘制一致的分槽公式，水平/垂直两分支。

## 4. 精确改动清单

### 4.1 新增控件源码

**`duilib/Control/Stepper.h`（新建）** — `class Stepper : public Control`：
- 覆写：`GetType`、`SetAttribute`、`PaintStateImages`、`OnLanguageChanged`、`ButtonDown`。
- 公开 API：
  - 数据：`size_t GetStepCount() const`、`const Step* GetStepAt(size_t) const`、`void AddStep(const Step&)`、`void AddStep(const DString& title, const DString& description = DString())`、`bool InsertStep(size_t, const Step&)`、`bool RemoveStepAt(size_t)`、`void ClearSteps()`、`void SetSteps(const std::vector<Step>&)`。
  - 当前索引：`size_t GetCurrentIndex() const`、`void SetCurrentIndex(size_t)`、`void SetCurrentIndexNoEvent(size_t)`。
  - 导航：`bool Next()`、`bool NextNoEvent()`、`bool Back()`、`bool BackNoEvent()`、`bool GoTo(size_t index)`、`bool GoToNoEvent(size_t index)`。
  - 禁用：`void SetStepDisabled(size_t index, bool bDisabled)`、`bool IsStepDisabled(size_t index) const`、`void SetDisabledSteps(const std::vector<size_t>&)`、`const std::vector<size_t>& GetDisabledSteps() const`。
  - 外观：`void SetHorizontal(bool)` / `bool IsHorizontal() const`；`void SetNodeSize(int32_t)` / `int32_t GetNodeSize() const`；`void SetClickToJump(bool)` / `bool IsClickToJump() const`；`void SetStateColor(StepStatus, const DString&)` / `const DString& GetStateColor(StepStatus) const`；`void SetConnectorColor(const DString&)` / `GetConnectorColor()` 与 `SetConnectorActiveColor` / `GetConnectorActiveColor()`；`void SetTitleColor(StepStatus, const DString&)` / `GetTitleColor(StepStatus)`；`void SetDescColor(const DString&)` / `GetDescColor()`；`void SetTitleFont(const DString&)` / `GetTitleFont()`；`void SetDescFont(const DString&)` / `GetDescFont()`。
  - 事件：`struct StepValue { size_t currentIndex; size_t oldIndex; };`、`void AttachStepChanged(const EventCallback&, EventCallbackID = 0);`。
  - 派生状态：`StepStatus GetStepStatus(size_t index) const`。
- 私有成员：`m_steps`、`m_disabledSteps`、`m_nCurrentIndex`、`m_bHorizontal`、`m_nNodeSize`、`m_bClickToJump`、`m_bMuteStepEvent`、`m_stateColors[4]`、`m_titleColors[4]`、`m_strConnectorColor`、`m_strConnectorActiveColor`、`m_strDescColor`、`m_strTitleFont`、`m_strDescFont`、`m_strItemTitleIds`、`m_strItemDescIds`。
- 私有辅助：`ParseItemsField(strValue, nField, bTranslate)`、`EmitStepChanged(oldIndex)`、`HitTestStep(rc, count, pt) const`、`CalcStepSlotRect(rc, count, index) const`，以及水平/垂直两套 `PaintHorizontal` / `PaintVertical`。

**`duilib/Control/Stepper.cpp`（新建）**：
- 构造：`m_bHorizontal(true)`、`m_nNodeSize(24)`、`m_bClickToJump(true)`、`m_bMuteStepEvent(false)`；默认色：pending=`text_muted`、active=`color_accent`、completed=`color_success`、disabled=`border_control_normal`；标题色 active=`text_default`、completed=`color_success`、pending=`text_muted`、disabled=`text_muted`；`m_strConnectorColor=_T("border_control_normal")`、`m_strConnectorActiveColor=_T("color_success")`、`m_strDescColor=_T("text_muted")`、字体 `system_bold_14` / `system_regular_12`。
- `GetType() { return DUI_CTR_STEPPER; }`
- `SetAttribute`：`step_title` / `step_title_id` / `step_desc` / `step_desc_id`（`|` 分隔，`_id` 走多语言）；`current` / `current_index`；`direction`（horizontal/vertical，别名 `orientation`）；`click_to_jump`；`node_size`；`disabled_steps`（`|` 或 `,` 分隔的索引）；`node_color_*`（pending/active/completed/disabled）；`connector_color`、`connector_active_color`；`title_color_*`；`desc_color`；`title_font`、`desc_font`；其余回退 `BaseClass::SetAttribute`。
- `OnLanguageChanged`：重解析 `step_title_id` / `step_desc_id`。
- 绘制与命中公式见 §3.4；文本用 `DrawStringParam` + `GetIFontById` 绘制（参考 Timeline 的 `DrawTimelineText`）。
- `GetStepStatus` 判定顺序：禁用 > 当前索引比较。

### 4.2 控件注册（4 处）

1. `duilib/duilib_defs.h`：在 `DUI_CTR_TIMELINE` 附近新增 `#define DUI_CTR_STEPPER (_T("Stepper"))`。
2. `duilib/Core/WindowBuilder.cpp`：`#include "duilib/Control/Stepper.h"`；`CreateControlMap()` 新增 `{DUI_CTR_STEPPER, [](Window* pWindow) { return new Stepper(pWindow); }}`。
3. `duilib/duilib.vcxproj`：`<ClCompile Include="Control\Stepper.cpp" />` 与 `<ClInclude Include="Control\Stepper.h" />`。
4. `duilib/duilib.vcxproj.filters`：上述两项 `<Filter>Control</Filter>`。
5. `duilib/duilib.h`：新增 `#include "Control/Stepper.h"`。

### 4.3 示例

- **新建 `examples/controls/StepperForm.h/.cpp`**：`class StepperForm : public ui::WindowImplBase`（自带 `typedef ui::WindowImplBase BaseClass;`）；`GetSkinFolder()="controls"`、`GetSkinFile()="stepper.xml"`；`OnInitWindow()` 绑定：
  - `stepper_demo` → `AttachStepChanged` → 更新 `step_event` 标签（前缀 + `第 N 步`）；另绑定下方"上一步 / 下一步 / 跳到第3步 / 重置"四个按钮驱动 `Back()` / `Next()` / `GoTo(2)` / `SetCurrentIndex(0)`。
- **新建 `bin/resources/themes/default/controls/stepper.xml`**：标准标题栏 + 提示 Label + 内容卡片；含：
  - 水平 `Stepper name="stepper_demo"`（4 步 `step_title_id`，`current="1"`）；
  - 垂直 `Stepper name="stepper_vertical" direction="vertical"`（3 步示例）；
  - `step_event` 标签与操作按钮。
  颜色名用现有语义色（`bg_window_main/bg_window_card/bg_titlebar/border_window/text_default/text_muted`）。
- `bin/resources/themes/default/controls/test.xml`：`Stepper Demo` 分组内新增 `<Button class="btn_global" name="stepper_btn" text_id="STRID_CONTROLS_STEPPER_TEST" .../>`。
- **语言资源**：`bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt` 各新增 `STRID_CONTROLS_STEPPER_TEST`、`STRID_STEPPER_WINDOW_TITLE`、`STRID_STEPPER_HINT`、步骤标题/描述 STRID（水平 4 步 + 垂直 3 步）、`STRID_STEPPER_PREV`、`STRID_STEPPER_NEXT`、`STRID_STEPPER_GOTO`、`STRID_STEPPER_RESET`、`STRID_STEPPER_EVENT_NONE`、`STRID_STEPPER_EVENT_PREFIX`。
- `examples/controls/controls.vcxproj` + `.filters`：新增 `StepperForm.h`（ClInclude）与 `StepperForm.cpp`（ClCompile）。
- `examples/controls/TestForm.h`：新增 `void ShowStepperWindow();`。
- `examples/controls/TestForm.cpp`：`#include "StepperForm.h"`；`OnInitWindow` 绑定 `stepper_btn` → `ShowStepperWindow()`；实现该方法（`kWS_POPUP + kWS_EX_LAYERED + m_bCenterWindow`，`ShowModalFake()`）。

### 4.4 文档同步

- `docs/Control.md` / `docs/Control.en.md`：新增 Stepper 条目（属性表 + XML 示例 + C++ 接口 + 事件）。
- `docs/Summary.md` / `docs/Summary.en.md`：控件清单补充 Stepper 行。
- `README.md` / `README.en.md`：控件计数同步（按现状 +1）。
- `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：API 参考补充。
- `.claude/docs/nim-duilib-llm-reference.md`：补充。
- `docs/misc/Project-Audit.md`：B4 行改为 ✅ 已完成（列出 `duilib/Control/Stepper.h/.cpp`、节点 `Stepper`、示例 `examples/controls/StepperForm`）。
- `docs/misc/Development-Plan.md`：D-6 勾选为 `- [x]` 并填完成日期。

## 5. 验证步骤

1. **编译**：`MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly` 直至无错误。
2. **运行时核对**（DPI-aware 辅助脚本，如已删除则重建）：
   - 启动 demo → 打开「控件测试」→ 点击「Stepper Demo」→ 截图确认：水平 4 步 + 垂直 3 步，节点序号/对勾、连接线、标题/描述显示正确。
   - 点击"下一步 / 上一步 / 跳到第3步 / 重置"→ 截图确认节点状态（已完成/进行中/待开始）与 `step_event` 标签同步更新。
   - 直接点击某个步骤节点 → 确认跳转生效。
3. **文档漂移**：`python3 ".trae/skills/nim-duilib/scripts/verify_docs.py" --repo "c:/develop/nim_duilib" --extra <本次改动的 md 文件>` → 期望「全部命中，未发现文档漂移」。
4. **盘点表 / 计划**：确认 B4 行已标注 ✅、D-6 已勾选。

## 6. 验收标准

- Debug|x64 编译无错误。
- `Stepper` 节点可在 XML 中创建并被 `FindControl` 找到。
- 水平/垂直两种方向、4 种状态（pending/active/completed/disabled）绘制正确。
- `Next` / `Back` / `GoTo` 可驱动当前步骤；`AttachStepChanged` 回调携带 `currentIndex` / `oldIndex`。
- `SetCurrentIndexNoEvent` / `GoToNoEvent` 不派发事件（无自递归）。
- `click_to_jump` 开启时点击步骤槽位可跳转，禁用步骤不可跳转。
- 多语言 `step_title_id` / `step_desc_id` 在语言切换后自动刷新。
- 文档同步完成且 `verify_docs.py` 无漂移；Project-Audit B4 / Development-Plan D-6 标记完成。

## 7. 待用户确认

请确认以上方案（尤其 §3.1 状态派生模型、§3.2 事件编码、§4 文件清单）后，再进入实现。

---

# 增量需求（2026-10-19）：节点 hover / 按下态 SVG 图标

## 8. 目标

为 `Stepper` 的步骤节点补充 **hover（悬停）** 与 **pressed（点击按下）** 两种状态的 SVG 图标视觉反馈。节点默认以自绘圆 + 序号/对勾呈现，缺图标资源时保持既有行为不变。

## 9. 设计决策

- 复用仓库既有「状态图片」范式 `StateImage`，属性命名对齐 `RangeSlider` 习惯（[RangeSlider.cpp](file:///c:/develop/nim_duilib/duilib/Control/RangeSlider.cpp#L476-L487)）：
  - `node_hovered_image`（别名 `node_hot_image` / `nodehoveredimage`）→ `kControlStateHovered`
  - `node_pressed_image`（别名 `node_pushed_image` / `nodepressedimage`）→ `kControlStatePressed`
- 图标 = **节点外围的强调色光晕 + 描边环**（viewBox 48×48）；绘制在节点圆形**下层**（先画图标、后画节点），因此保留 4 态语义色与序号/对勾，仅在节点外圈形成反馈。
- 图标占位色 `#0078D4`，经 `svg_replace_colors='#0078D4|color_accent'` 映射到语义色 `color_accent`，随主题/强调色自适应（与仓库 switch/checkbox 用法一致，见 [global.xml](file:///c:/develop/nim_duilib/bin/resources/themes/default/global.xml#L292-L302)）。
- 仅对「可交互步骤」绘制：`IsEnabled() && IsMouseEnabled() && m_bClickToJump && 步骤非 disabled`；否则完全不绘制。
- 目标矩形：以节点圆心为中心、边长 `2*nRadius + Dpi().GetScaleInt(10)`；用 `PaintImage(pRender, pImage, "destscale='false' dest='x,y,x,y'")`，其中 dest 为**控件相对坐标**（先减 `GetRect()` 原点，参照 [RangeSlider.cpp](file:///c:/develop/nim_duilib/duilib/Control/RangeSlider.cpp#L571-L608) 的既有做法；`Control::PaintImage` 内部会把配置 dest 加上控件原点，见 [Control.cpp](file:///c:/develop/nim_duilib/duilib/Core/Control.cpp#L3389-L3394)）。
- 状态优先级：pressed > hovered；pressed 图片未配置时回退 hovered；两者均未配置则不做额外绘制（等价旧版）。

## 10. 精确改动清单

1. **新增 SVG 资源**（`bin/resources/themes/default/public/stepper/`）
   - `stepper_node_hovered.svg`：48×48；强调色半透明光晕（r≈22，opacity 0.10）+ 细环（r≈21，stroke 2.5，opacity 0.55），色值 `#0078D4`。
   - `stepper_node_pressed.svg`：48×48；更深光晕（r≈22.5，opacity 0.18）+ 粗环（r≈20.5，stroke 4，opacity 0.85），色值 `#0078D4`。
2. **`duilib/Control/Stepper.h`**
   - 新增 `#include "duilib/Image/StateImage.h"`。
   - 覆写声明：`bool MouseMove(const EventArgs&) override;`、`bool MouseLeave(const EventArgs&) override;`、`bool ButtonUp(const EventArgs&) override;`、`void ClearImageCache() override;`。
   - 公开 API：`void SetNodeStateImage(ControlStateType stateType, const DString& strImage);`、`DString GetNodeStateImage(ControlStateType stateType) const;`。
   - 私有辅助：`void PaintNodeStateIcon(IRender*, const UiPoint& ptCenter, int32_t nRadius, StepStatus status, size_t index);`。
   - 私有成员（置于成员列表末尾）：`StateImage m_nodeStateImage;`、`int32_t m_nHoverStep;`、`int32_t m_nPressedStep;`（`-1` 表示无）。
   - 顶部 XML 属性注释补充上述两属性。
3. **`duilib/Control/Stepper.cpp`**
   - 构造初始化 `m_nHoverStep(-1)`、`m_nPressedStep(-1)`；构造体 `m_nodeStateImage.SetControl(this)`。
   - `SetAttribute`：新增 `node_hovered_image` / `node_hot_image` / `nodehoveredimage` → `SetNodeStateImage(kControlStateHovered, ...)`；`node_pressed_image` / `node_pushed_image` / `nodepressedimage` → `SetNodeStateImage(kControlStatePressed, ...)`。
   - 实现 `SetNodeStateImage`（`m_nodeStateImage.SetImageString(stateType, strImage, Dpi())` + `Invalidate()`）、`GetNodeStateImage`、`ClearImageCache`（`BaseClass` + `m_nodeStateImage.ClearImageCache()`）。
   - `MouseMove`：`msg.eventType == kEventMouseMove` 时按 `HitTestStep` 取悬停索引（命中禁用步 → -1）；变化则 `Invalidate()`；末尾 `BaseClass::MouseMove(msg)`。
   - `MouseLeave`：清空 `m_nHoverStep` / `m_nPressedStep`，变化则 `Invalidate()`；末尾 `BaseClass::MouseLeave(msg)`。
   - `ButtonDown`：命中且 `m_bClickToJump` 且步骤非禁用 → `m_nPressedStep = nHit`、`SetMouseFocused(true)`、`Invalidate()`，再 `GoTo(nHit)`；其余回退 `BaseClass::ButtonDown(msg)`（保留原逻辑）。
   - `ButtonUp`：清 `m_nPressedStep` → `Invalidate()` → `BaseClass::ButtonUp(msg)`。
   - `PaintHorizontal` / `PaintVertical`：每个节点 `PaintNode(...)` **之前**调用 `PaintNodeStateIcon(...)`。
   - `PaintNodeStateIcon`：按 §9 判定与回退，计算控件相对 dest 后 `PaintImage(pRender, pImage, strModify)`。
4. **`bin/resources/themes/default/global.xml`**
   - 紧随 `PATH_SLIDER` 新增 `<Var name="PATH_STEPPER" value="public/stepper"/>`。
   - 新增 `<Class name="stepper_node" node_hovered_image="file='${PATH_STEPPER}/stepper_node_hovered.svg' width='48' height='48' svg_replace_colors='#0078D4|color_accent'" node_pressed_image="file='${PATH_STEPPER}/stepper_node_pressed.svg' width='48' height='48' svg_replace_colors='#0078D4|color_accent'"/>`。
5. **`bin/resources/themes/default/controls/stepper.xml`**
   - 两个 `<Stepper>` 元素新增 `class="stepper_node"`。
6. **文档同步**
   - `docs/Control.md` / `docs/Control.en.md`：Stepper 属性表新增 `node_hovered_image` / `node_pressed_image`，说明补充 hover/按下态图标行为与默认皮肤。
   - `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：「Stepper 属性」小节新增两属性。
   - `.claude/docs/nim-duilib-llm-reference.md`：`Stepper` 速查行描述补充「节点 hover/按下态图标」。

## 11. 验证步骤

1. **编译**：`MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly` 无输出。
2. **运行时核对**（DPI-aware 截图脚本）：鼠标悬停某可交互节点 → 出现强调色光晕环；按下 → 环加深/加粗；移出 → 消失；禁用步骤与 `click_to_jump=false` 时不出现。
3. **文档漂移**：`verify_docs.py --repo "c:/develop/nim_duilib" --extra <本次改动 md>` → 「全部命中，未发现文档漂移」。

## 12. 验收标准

- Debug|x64 编译无错误。
- `node_hovered_image` / `node_pressed_image`（含别名）在 XML 中生效；悬停/按下态图标仅对可交互步骤显示。
- pressed 未配置时回退 hovered；均未配置时行为与旧版一致。
- `MouseMove` / `MouseLeave` / `ButtonUp` 覆写不破坏原有点击跳转与 `AttachStepChanged` 事件。
- `ClearImageCache()` 释放图标缓存。
- 文档同步完成且 `verify_docs.py` 无漂移。

## 13. 待用户确认

请确认 §9 视觉方案（**节点外围光晕环**，保留序号/对勾与 4 态语义色，不替换节点本体）与 §10 文件清单后，进入实现。
