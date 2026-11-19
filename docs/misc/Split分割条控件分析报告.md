下面的分析基于 `duilib/Control/Split.h`（以及其依赖的 `Control`、`PlaceHolder`、`ListCtrlHeader`、`global.xml` 中的相关代码）。文中引用的行号是分析时（修复前）的版本。

## 处理状态

| 编号 | 内容 | 状态 |
| --- | --- | --- |
| 1 | 快速拖到边界时提前"卡住" | 已修复 |
| 2 | `ButtonDown` 未清理上一次的拖动状态 | 已修复 |
| 3 | 窗口失焦后拖动状态残留 | 已修复 |
| 4 | 另一侧最小尺寸占满时最大值约束被跳过 | 已修复 |
| 5 | `min > max` 冲突时结果可能小于最小值 | 已修复 |
| 6 | 不可拖动时仍显示调整光标 | 已修复 |
| 7 | `enable_split_single` 缺少父容器边界约束 | 已修复 |
| 8 | 每次鼠标移动触发两次完整布局 | 已修复 |
| 9 | 数值未变化也发送拖动事件 | 已修复 |
| 10 | 代码重复与命名问题 | 已修复 |
| 11 | 头注释与实现矛盾 | 已修复（代码注释 + `docs/Control.md` / `Control.en.md`） |
| 三 | 双击复位 | 已实现（属性 `double_click_reset`，默认 `false`） |
| 三 | 拖动开始/结束事件 | 已实现（`kEventSplitDragBegin` / `kEventSplitDragEnd` + `AttachSplitDragBegin` / `AttachSplitDragEnd`） |
| 三 | 拖动阈值 | 已实现（属性 `drag_threshold`，默认 `2` 个 96 DPI 逻辑像素，`0` 表示关闭） |
| 三 | 目标控件可指定 | 已实现（属性 `left_top_ctrl_name` / `right_bottom_ctrl_name`，支持跨容器调整） |
| 三 | 键盘操作 | 已实现（获得焦点后方向键微调，属性 `keyboard_step` 默认 `10` 个 96 DPI 逻辑像素，Shift 为 10 倍步长） |
| 三 | 拖动尺寸提示 | 已实现（属性 `drag_size_tip`，默认 `false`，拖动时在分割条附近显示当前尺寸） |
| 五 | 界面美观度改进 | 已实现（主题类/热区分离/拖动反馈/对比度/光标语义此前已完成；本次补齐双击复位过渡动画，属性 `reset_anim` 默认 `150` 毫秒） |

## 一、结论

`Split.h` 采用"模板 + 头文件实现"的方式，被 `Split`（继承 `Control`）和 `SplitBox`（继承 `Box`）复用；实现方式是：拖动分割条时修改相邻两个兄弟控件的 `fixed_width/fixed_height`。

整体思路清晰，DPI 处理正确（拖动值使用物理像素，`SetFixedWidth(..., false)` 不再做二次缩放），鼠标捕获期间的移动消息会被路由到按下的控件，因此拖动过程可以稳定工作。

存在的问题集中在四个方面：

1. 边界夹紧逻辑不一致（快速拖动时提前失效）；
2. 拖动状态的生命周期管理不完整（遗留旧指针/旧方向）；
3. 光标与可用性反馈失真；
4. 代码重复较多，且缺少现代分割条的常用交互（双击复位、拖动起止事件、拖动阈值等）。

## 二、错误与设计缺陷（按严重程度）

### 1. 快速拖到边界时提前"卡住"，无法到达真正的极限位置（功能性 Bug，高）

`Split.h#L428-L467`：两侧分别用 `nOffset` 和 `-nOffset` 独立计算新值，再要求 `nNewValue1 + nNewValue2 == nTotal`，否则整体放弃。

举例：左侧 200、右侧 300（合计 500），`max_width = 300`。一次性拖到 +120：

- `nNewValue1 = min(320, 300) = 300`
- `nNewValue2 = 300 - 120 = 180`
- `300 + 180 ≠ 500` → 直接 return，控件停在 +50 的位置。

正确结果应是 `300 / 200`。也就是说：鼠标移动很快（事件步长较大）时，拖动会停在离边界还有一段距离的位置，并且继续往同方向拖持续无效，必须反向拖回才能恢复。这是实际使用中最容易被感知到的问题。

根因是"先各自夹紧、再校验总和"。应改为：**先求出统一的有效偏移（同时满足两侧的 min/max），再由偏移推导两侧数值**，这样两侧之和必然等于 `nTotal`。

### 2. `ButtonDown` 未清理上一次的拖动状态（高）

`Split.h#L209-L291`：`m_pLeftTop / m_pRightBottom / m_bHLayout` 只在 `IsVLayout/IsHLayout` 分支里被覆盖。若控件在运行期被移动到非 H/V 布局的父容器（或父容器布局被替换），这些成员会保留旧值；而基类 `ButtonDown` 已经把 `IsMouseFocused` 置为 true，后续 `MouseMove` 会拿**旧的兄弟控件指针**去改尺寸（控件树重建后还可能悬空）。

建议：在 `ButtonDown` 开头先调用 `StopSplitDrag()`，再根据当前父容器重新判定方向与邻居。

### 3. 窗口失焦后拖动状态残留（中高）

`Split.h#L155-L160` 的 `StopSplitDrag()` 只在 `ButtonUp` 时调用。窗口失焦时 `Control.cpp#L3256-L3276` 只会把 `IsMouseFocused` 置为 false，Split 的邻居指针仍然保留，导致公共接口 `IsSplitDragValid()` 在"没在拖动"时仍返回 true。

建议：覆写 `OnKillFocus`，在其中调用 `StopSplitDrag()` 并 `Invalidate()`。

### 4. `CalculateControlPos` 在另一侧最小尺寸已占满时跳过约束（中高）

`Split.h#L347-L365`：

```cpp
if ((nNewMax > 0) && (nMax > nNewMax)) { nMax = nNewMax; }
```

当 `nTotal - pSecond->GetMinWidth() <= 0` 时约束被整个忽略，`nMax` 退化为 `pFirst->GetMaxWidth()`（默认 `INT32_MAX`），本侧可以被放大到超出可用空间。

建议：改为 `nNewMax = max(0, nTotal - secondMin)` 且始终参与取小；若 `nTotal <= secondMin`，说明没有可调整空间，直接返回当前值。

### 5. `min > max` 冲突时最终会小于最小值（中）

`Split.h#L366-L369`：先夹 min 再夹 max，若配置的 `max_width < min_width`（或第 4 点导致 max 变负），结果会落在 min 之下。

建议：最后统一执行 `nNewValue = std::clamp(nNewValue, nMin, std::max(nMin, nMax))`。

### 6. 不可拖动时仍显示"可调整"光标（中，交互误导）

`Split.h#L190-L206`：只根据父布局方向设置 `SizeWE/SizeNS`，没有判断：

- 控件是否 `IsEnabled()`；
- `enable_split_single = false` 且只有一个邻居；
- 两侧都是 stretch（或唯一邻居是 stretch，此时 `OnMouseMoveSingle` 会直接返回）。

这些情况下鼠标显示可拖动光标但实际拖不动，属于误导性反馈。另外父容器不是 H/V 布局时不会设置光标，但**也不会把它恢复为默认箭头**（例如布局方向在运行期变更后）。

建议：抽出 `UpdateCursor()`，在所有前置条件变化处（`MouseEnter`、`SetAttribute`、子项增删）调用。

### 7. `enable_split_single` 缺少父容器边界约束（中）

`Split.h#L495-L596`：单邻居模式下，只受自身 `max_width/max_height` 限制，不受父容器可用空间限制，可能把内容撑出父容器（出现滚动条或被相邻内容遮挡）。

### 8. 性能：每次鼠标移动触发两次完整布局（中）

`Split.h#L470-L487`：两侧各调用一次 `SetFixedWidth(..., true, ...)`，而它在 `PlaceHolder.cpp#L217-L237` 内部会执行 `ArrangeAncestor()`。因此每次 `MouseMove` 产生两次全量布局，复杂界面上会造成拖动卡顿。

建议：第一侧用 `bArrange = false`，第二侧用 `true`，保证一次布局。

### 9. 数值没变化也会发送拖动事件（中）

`Split.h#L489-L491`：`bAdjusted` 只表示"执行了赋值"，即使新旧值相同也会 `SendEvent(kEventSplitDraged)`。例如 `ListCtrlHeader` 的处理器会因此反复设置列宽。

建议：只在"旧值 != 新值"时发送事件。

### 10. 代码重复与命名问题（低）

- `ButtonDown` 的 V/H 两段（`Split.h#L247-L290`）与 `MouseMove` 的宽/高两段（`Split.h#L432-L487`）几乎完全对称，可用 `GetAxisSize()/SetAxisFixed()` 两个小 helper 消除约 60 行重复。
- `ButtonDown` 中 `IsVLayout/IsHLayout` 对同一父容器最多执行 3 次 `dynamic_cast` + `GetLayout()`，缓存一次即可。
- 公开 API 拼写：`kEventSplitDraged` / `AttachSplitDraged`（应为 `Dragged`）、成员 `m_nLeftUpFixedValue`（应为 `LeftTop`）、`SetAttribute` 的参数名 `strValue2`。前者改动会破坏兼容，建议新增 `AttachSplitDragged` 别名，并在注释中标注旧名废弃。
- `CalculateControlPos` 的注释写"nFirstFixedInt 是原始值"，而 `nFirstFixedInt.GetInt32() + nOffset` 的语义是"从按下时的值开始累计"，建议注释写清"鼠标当前相对按下点的累计偏移"。

### 11. 头注释与实现矛盾（低，文档）

`Split.h#L10-L14` 说"两个控件都设置为拉伸类型时分割条无法正常工作"，但 `Split.h#L295-L305` 在按下时会把左上控件转换为固定值，实际可以拖动。

建议：更新文档为"首次按下后自动把左上控件转换为固定尺寸"，并同步 `docs/Control.md#L3325-L3333`。

### 修复实现说明（第 1~11 点）

- **边界处理**：`MouseMove` 中两侧都是固定尺寸时，改为先计算第一个控件允许的"最小值/最大值"（同时扣除另一侧的最小/最大尺寸），再夹紧"按下时尺寸 + 累计偏移"，第二个控件取 `总尺寸 - 新值`，从而保证尺寸之和恒等于总尺寸；删除了"两侧独立计算后要求之和恰好相等"的放弃逻辑。`CalculateControlPos` 中最大值始终与 `总尺寸 - 另一侧最小值` 取小，并保证最大值不小于最小值。
- **拖动状态生命周期**：`ButtonDown` 开头调用 `StopSplitDrag()`；新增 `OnWindowKillFocus` 覆写，窗口失焦时结束拖动并重绘。
- **光标**：新增 `CanStartDrag()` 与 `UpdateCursor()`，只有控件可用、父容器为 H/V 布局且相邻控件满足条件时才显示 `size_we/size_ns`，否则恢复默认箭头；`MouseEnter` 与 `SetEnableSplitSingle` 都会刷新光标。
- **单控件模式的边界**：`OnMouseMoveSingle` 新增父容器可用空间约束（扣除分割条与同一容器中其它控件占用的空间，拉伸控件按最小尺寸计算），并复用统一的 min/max 夹紧。
- **性能与事件**：只有尺寸变化时才设置新值；两侧都需要调整时，第一侧以 `bArrange=false` 只改值，由第二侧触发布局，每次鼠标移动最多一次布局；尺寸未变化时不再派发 `kEventSplitDraged`。
- **代码质量**：新增 `GetAxisSize` / `GetAxisFixedSize` / `SetAxisFixedSize` / `GetAxisMinSize` / `GetAxisMaxSize` / `GetParentAvailableSize` / `FindPrevNextControl` 等辅助函数，消除横向/纵向的重复分支；`m_nLeftUpFixedValue` 更名为 `m_nLeftTopFixedValue`；新增拼写正确的 `AttachSplitDragged`（`AttachSplitDraged` 保留兼容）；`IsSplitDragValid()` 改为 `const`。
- **文档**：更新 `Split.h` 头注释，并同步 `docs/Control.md`、`docs/Control.en.md` 中 Split 的说明（双拉伸行为、光标行为、拖动事件）。

### 回归验证与相关修复

在 Debug 版 `TreeView` 示例中验证（非 DLL 模式）：

- 拖动主界面的 `split_vertical`：左面板宽度由 280 增大到 480、再缩小到最小宽度 60，尺寸变化正确，光标在可拖动时显示 `size_we`。
- 缩小到最小宽度时，出现 `Image.cpp(329) : Assertion failed: !rcDest.IsEmpty() && !rcSource.IsEmpty()`。

该断言与分割条逻辑无关，属于绘制路径的健壮性问题：`TreeNode` 的子项（图标）在树控件被压窄后，其绘制目标区域会被裁剪为 0 宽/0 高（`rcDest` 为空），而 `Control::PaintImage` 仍然继续调用 `Image::GetCurrentBitmap`，触发了该断言（Debug 下会中断为未处理异常，Release 下则直接不绘制）。

修复：在 `Control::PaintImage` 计算完 `rcImageDect` 后，如果目标区域为空则直接返回 `false`（同时仍通过 `pPaintedRect` 返回该空区域），不再进入图片解码/绘制流程。该修改只影响"完全不可见"的图片绘制，不改变可见状态下任何绘制结果。

修复后重新验证：分割条左右拖动到最小宽度时不再触发断言，程序保持正常运行。

### 双击复位实现说明

- **属性**：新增 `double_click_reset`（默认 `false`），对应 `SetDoubleClickReset()` / `IsDoubleClickReset()`。默认为关闭，避免影响自行处理双击的场景（如 `ListCtrlHeader` 在 `SplitBox` 上挂的双击）。
- **记录时机**：在 `ButtonDown` 中，首次真正开始拖动前记录两侧控件的 `UiFixedInt` 原始值（保留固定值、`stretch`、`auto` 三种类型）；双拉伸场景下，记录发生在"左上控件转固定值"之前，因此复位后可以恢复为双拉伸。
- **复位行为**：覆写 `ButtonDoubleClick()`，先调用基类保持既有双击事件派发；当属性开启、已记录初始值且当前满足拖动条件时，用 `ResetAxisFixedSize()` 按原类型/原值恢复两侧控件，两者都改完后统一调用 `ArrangeAncestor()` 触发布局（避免单侧值未变化时 `SetFixedWidth` 不重排的问题）。
- **交互结果**：复位后仍可继续拖动；再次双击仍恢复到"首次拖动前"的尺寸（初始值只记录一次，不随拖动更新）。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中给主分割条加 `double_click_reset="true"`，拖动使左面板由 280 增大到约 400，双击后恢复原宽度（截图测量：分割线物理像素 x 519 → 727 → 517），界面正常、无断言。

### 拖动开始/结束事件实现说明

- **事件定义**：在 `duilib_defs.h` 中新增 `kEventSplitDragBegin`、`kEventSplitDragEnd`（与 `kEventSplitDraged` 相邻），并在 `EventArgs.cpp` 中注册事件名称（`SplitDragBegin`/`split_drag_begin`、`SplitDragEnd`/`split_drag_end`），支持代码和 XML 两种方式绑定。
- **注册函数**：`Split` 新增 `AttachSplitDragBegin()` / `AttachSplitDragEnd()`，参数格式与 `AttachSplitDragged()` 一致（`wParam` 为左侧/上侧控件，`lParam` 为右侧/下侧控件，均可能为 `nullptr`）。
- **开始事件**：在 `ButtonDown` 中，当相邻控件解析成功、拖动条件满足时，先置"拖动中"标记，再在 `Invalidate()` 之后发送 `kEventSplitDragBegin`（避免回调中删除控件导致后续访问失效）。
- **结束事件**：`StopSplitDrag()` 中先取出控件指针、清理拖动状态并复位标记，然后发送 `kEventSplitDragEnd`；鼠标抬起（`ButtonUp`）和窗口失去焦点（`OnWindowKillFocus`）都会结束拖动，事件成对出现。即使按下后没有移动，也会收到一次开始和一次结束事件。
- **重入保护**：事件发送前已完成状态清理，回调中再次调用 `StopSplitDrag()` 不会重复触发结束事件；`OnWindowKillFocus` 中的重绘调整到结束拖动之前，避免回调中删除控件后继续访问。
- **验证**：Debug 版 `TreeView`（非 DLL 模式）下执行拖动 + 双击操作，拖动和双击复位行为正常（分割线物理像素 x 519 → 727 → 517），进程无断言、无崩溃。

### 拖动阈值实现说明

- **属性**：新增 `drag_threshold`（默认 `2`），对应 `SetDragThreshold()` / `GetDragThreshold()`；单位是 96 DPI 的逻辑像素，设置为 `0` 表示不启用阈值，负数按 `0` 处理。
- **DPI 缩放**：比较时用 `Dpi().GetScaleInt(m_nDragThreshold)` 换算为物理像素，保证不同缩放下手感一致（例如 175% 缩放时默认阈值约 4 个物理像素）。
- **状态**：新增成员 `m_nDragThreshold`、`m_bDragThresholdExceeded`；每次 `ButtonDown` 进入拖动时把 `m_bDragThresholdExceeded` 复位为 `false`。
- **判定时机**：`MouseMove` 与 `OnMouseMoveSingle` 在算出布局方向的偏移量（`nOffset`）后，先调用 `IsDragThresholdExceeded(nOffset)`；未超过阈值时直接返回，不修改尺寸、不派发 `kEventSplitDraged`，但`kEventSplitDragBegin` 仍会在按下时触发（一次拖动只触发一对起止事件）。
- **超过阈值后的行为**：一旦超过阈值，本次拖动按**完整的鼠标偏移量**调整尺寸（不做偏移补偿），并在剩余的拖动过程中不再重复判定，直到鼠标抬起。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中，对主分割条先做 2 个物理像素的轻微抖动（小于阈值），分割线位置不变（物理像素 x 519 → 519）；随后做 210 物理像素的正常拖动，分割线移动到 x 727，尺寸调整正常，进程无断言。

### 目标控件可指定实现说明

- **属性与接口**：新增属性 `left_top_ctrl_name` / `right_bottom_ctrl_name`，对应 `SetLeftTopCtrlName()`/`GetLeftTopCtrlName()` 与 `SetRightBottomCtrlName()`/`GetRightBottomCtrlName()`。
- **解析规则**：`FindPrevNextControl()` 先按原有逻辑求出同一父容器中的前后相邻控件；某一侧配置了名称时，改用 `FindTargetControl()` 在**同一个窗口内**按名称查找并覆盖该侧结果。未配置名称的一侧保持"相邻控件"行为，因此可以只指定一侧。
- **有效性校验**：按名称找到的控件需要非空、不是分割条自身且当前可见，否则该侧视为没有目标控件（配合 `enable_split_single` 决定能否拖动）。
- **跨容器支持**：`GetParentAvailableSize()` 改为以目标控件所在父容器计算可用空间；仅当目标控件与分割条在同一父容器时才扣除分割条自身占用的空间，因此跨容器调整时边界约束仍然正确。
- **拖动方向**：仍由分割条所在父容器的布局决定（`HLayout` 左右、`VLayout` 上下），目标控件沿同一方向调整固定宽度/高度。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中给主分割条显式配置 `left_top_ctrl_name="tree_panel" right_bottom_ctrl_name="main_view_tab_box"`，拖动行为与默认（自动取相邻控件）一致：2 物理像素抖动不改变尺寸（x 519 → 519），210 物理像素拖动后分割线移动到 x 727，进程无断言。

### 键盘操作实现说明

- **获取焦点**：鼠标按下并进入可拖动状态时调用 `SetFocus()`，使分割条获得焦点（不需要额外的 XML 配置）。
- **属性与接口**：新增属性 `keyboard_step`（默认 `10`，单位 96 DPI 逻辑像素，负数按 `0` 处理），对应 `SetKeyboardStep()` / `GetKeyboardStep()`；`0` 表示不启用键盘操作。
- **按键映射**：`OnKeyDown()` 中按布局方向处理——`HLayout` 用 `←` / `→`，`VLayout` 用 `↑` / `↓`，其余按键不处理并返回给父控件；步长按当前 DPI 缩放（175% 缩放时默认步长约 18 个物理像素），按住 `Shift` 时为 10 倍步长。
- **复用拖动逻辑**：把原 `MouseMove` / `OnMouseMoveSingle` 的尺寸计算与边界夹紧提取为 `ApplyDragOffset()` / `ApplyDragOffsetSingle()`，鼠标拖动与键盘操作共用同一套计算；键盘操作前调用 `PrepareDragTargets()` 重新获取目标控件与当前尺寸，因此可以连续按键逐次累加。
- **事件**：键盘调整尺寸时派发 `kEventSplitDraged`（两侧都有目标控件时 `wParam`/`lParam` 为两侧控件，单侧时为该控件 + `nullptr`），不派发拖动开始/结束事件。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中单击主分割条后连续按 `→`，分割线物理像素 x 依次为 519 → 535 → 553 → 571（每步约 18 像素）；`Shift + →` 单次移动约 180 像素（10 倍步长），进程无断言。

### 拖动尺寸提示实现说明

- **属性与接口**：新增属性 `drag_size_tip`（默认 `false`），对应 `SetDragSizeTip()` / `IsDragSizeTip()`；关闭属性时会立即隐藏已创建的提示框。
- **为什么不能自绘**：`Control::PaintControl` 会把每个控件的绘制区域裁剪到自身的 `GetRect()`（`AutoClip`），分割条自身只有几像素宽，无法在它旁边绘制提示框，因此改为"动态创建浮动子控件"的方案。
- **提示控件**：首次显示时用 `new Label(GetWindow())` 创建一个 `Label`，加入分割条的父容器（`Box::AddItem`）；设置为 float（不参与父容器布局）、`SetMouseEnabled(false)`（不拦截鼠标）、深色圆角背景（`#E6323232`）、白字、单行居中，宽度/高度使用**固定值**（`SetFixedWidth/SetFixedHeight(..., false, false)`，值为 76×22 逻辑像素按 DPI 缩放）。固定宽高是必要的：`Label` 默认宽高为 auto，`SetText()` 会触发 `ArrangeAncestor()`，父容器重新布局时会把 float 子控件重置到默认对齐位置（左上角）——这正是最初"气泡跑到内容区左上角"的原因。
- **位置保持**：提示控件还要调用 `SetKeepFloatPos(true)`，这样后续父容器布局（拖动改变尺寸时会持续触发布局）不会重置已设置的位置。
- **定位规则**：提示框纵向居中于分割条，横向布局放在分割条右侧、纵向布局放在下方；空间不足时改到另一侧，最后夹紧到父容器 `GetRect()` 范围内（超出部分会被父容器裁剪）。注意 `SetPos()` 使用窗口坐标（与 `GetPos()`/`GetRect()` 同一坐标系），不能用父容器宽高代替父容器右/下边界。
- **显示数值**：显示 `Dpi().GetUnscaleInt(nSize)`，即 96 DPI 逻辑像素（与 XML 中的尺寸单位一致，避免 175% 缩放下显示"630 px"这类物理像素值造成困惑）。
- **显示与隐藏时机**：只在鼠标拖动路径上调用（`MouseMove` / `OnMouseMoveSingle`，且已超过拖动阈值）；`StopSplitDrag()` 中隐藏（鼠标抬起、窗口失去焦点都会走到），键盘调整尺寸时不显示。提示控件由父容器管理生命周期，代码中通过 `ControlPtr` 弱引用访问，父容器销毁后自动失效。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中临时给主分割条配置 `drag_size_tip="true"`，拖动过程中截图可见分割条中部右侧显示深色圆角气泡"360 px"（逻辑像素），位置紧随分割条；拖动前与松开鼠标后气泡均不显示（截图对比 `1_before` / `2_dragging` / `3_after`），分割线物理像素 x 519 → 667，进程无断言。

## 三、功能不足（建议补齐）

| 能力 | 现状 | 建议 |
| --- | --- | --- |
| 双击复位 | 控件本身不支持（`ListCtrlHeader` 自己在 SplitBox 上挂双击） | 已实现：内建保存首次拖动前的 `fixed` 值（含固定/拉伸/自动类型），双击恢复；属性 `double_click_reset` 控制，默认 `false` |
| 拖动开始/结束事件 | 只有 `kEventSplitDraged` | 已实现：新增 `kEventSplitDragBegin/kEventSplitDragEnd`（`AttachSplitDragBegin/AttachSplitDragEnd`），便于应用层暂停刷新、持久化布局 |
| 拖动阈值 | 无，轻微移动即改尺寸并发事件 | 已实现：新增属性 `drag_threshold`（默认 `2` 个 96 DPI 逻辑像素，按当前 DPI 缩放，`0` 表示关闭），鼠标在布局方向上移动超过该距离后才开始调整尺寸 |
| 目标控件可指定 | 只能取"同父容器内、可见、非 float 的前后兄弟" | 已实现：新增属性 `left_top_ctrl_name` / `right_bottom_ctrl_name`（同一窗口内按名称查找），未指定的一侧仍按相邻控件处理，支持跨容器分割 |
| 键盘操作 | 无 | 已实现：新增属性 `keyboard_step`（默认 `10` 个 96 DPI 逻辑像素，`0` 表示关闭），获得焦点后按布局方向的方向键微调，Shift + 方向键为 10 倍步长 |
| 拖动尺寸提示 | 无 | 已实现：新增属性 `drag_size_tip`（默认 `false`），鼠标拖动时在分割条中部附近显示"320 px"样式的浮动提示（类似 Qt Designer），显示 96 DPI 逻辑像素值，提升可预期性 |

## 四、核心修复示例（边界夹紧）

以下是修复思路（已在代码中实现，成员已更名为 `m_nLeftTopFixedValue`）：

```cpp
// 两侧都是固定值时的正确做法：先把偏移夹紧到"两侧都合法"的区间，再推导数值
int32_t nOrig1 = m_nLeftTopFixedValue.GetInt32();
int32_t nOrig2 = nTotal - nOrig1;

int32_t nFirstMin  = m_bHLayout ? m_pLeftTop->GetMinWidth()    : m_pLeftTop->GetMinHeight();
int32_t nFirstMax  = m_bHLayout ? m_pLeftTop->GetMaxWidth()    : m_pLeftTop->GetMaxHeight();
int32_t nSecondMin = m_bHLayout ? m_pRightBottom->GetMinWidth() : m_pRightBottom->GetMinHeight();

nFirstMax = std::min(nFirstMax, nTotal - nSecondMin);   // 给另一侧留出最小空间
nFirstMax = std::max(nFirstMax, nFirstMin);             // 处理 min > max 的冲突配置

int32_t nMinOffset = nFirstMin - nOrig1;
int32_t nMaxOffset = nFirstMax - nOrig1;
nOffset = std::clamp(nOffset, nMinOffset, nMaxOffset);

int32_t nNew1 = nOrig1 + nOffset;
int32_t nNew2 = nTotal - nNew1;   // 恒等，不再需要 sum != nTotal 的放弃分支
```

这样能同时修掉第 1、4、5 点，并可删除 `nNewValue1 + nNewValue2 != nTotal` 的拒绝路径。

## 五、界面美观度改进建议

原报告中"默认主题只给列头分割条定义了 3px 热区 + 1px 细线"的现象已经不存在：主题中已提供通用的分割条样式类（`bin/resources/themes/default/global.xml`）：

```xml
<Class name="split_vertical" width="5" state_color_min_width="1"
       normal_color_margin="2,0,2,0" normal_color="bg_split_normal"
       hovered_color_margin="1,0,1,0" hovered_color="bg_split_hovered"
       pressed_color_margin="1,0,1,0" pressed_color="bg_split_pressed"
       disabled_color_margin="2,0,2,0" disabled_color="bg_split_disabled" />
<Class name="split_horizontal" height="5" state_color_min_height="1" ...（与上面一致，方向相反）... />
```

各项建议的处理情况：

1. **通用主题类**：已实现。`split_vertical` / `split_horizontal` 统一了所有示例中分割条的观感，使用基类的状态颜色驱动 `normal/hovered/pressed/disabled`。
2. **热区与视觉分离**：已实现。热区为 5 个 96 DPI 逻辑像素（按 DPI 缩放），配合状态颜色的外边距，常态只画 1px 细线；悬停/按下时线条变粗（3px），更容易抓取。
3. **拖动反馈**：已实现。按下时 `pressed_color` 自动生效，松开后恢复；颜色过渡（约 150ms）为可选项，当前未启用（状态颜色切换本身足够轻快，且可避免额外的动画定时器开销）。
4. **双击复位 + 轻微动画**：本次已实现。属性 `double_click_reset` 打开后，双击复位默认带 150ms 的缓出尺寸过渡；动画时长由新增属性 `reset_anim` 控制（毫秒，`0` 表示不使用动画）。
5. **深浅色对比度**：已实现。浅色/深色主题分别定义了 `bg_split_normal`（#D9D9D9 / #4D4D4D）、`bg_split_hovered`（#C2C2C2 / #666666）、`bg_split_pressed`（#A6A6A6 / #808080）、`bg_split_disabled`（#E6E6E6 / #3A3A3A），与两侧内容背景均有明显区分。
6. **光标语义**：已实现。`UpdateCursor()` 只在真正可拖动（横向/纵向布局且存在有效目标控件）时显示 `size_we` / `size_ns`，否则恢复默认箭头；拖动过程中保持调整光标。

### 双击复位动画实现说明

- **属性与接口**：新增属性 `reset_anim`（默认 `150` 毫秒），对应 `SetResetAnimMillSeconds()` / `GetResetAnimMillSeconds()`；`0` 表示不使用动画，直接复位（与原行为一致）。
- **目标尺寸计算**（`CalcResetTargetSize()`）：动画值代表左侧（上侧）控件的尺寸，按首次拖动前记录的类型计算目标值：
  - 该侧恢复为固定尺寸：直接使用记录值；
  - 该侧恢复为拉伸、另一侧为固定尺寸：用两侧当前尺寸之和减去另一侧的固定值；
  - 两侧都恢复为拉伸：按两侧尺寸之和均分；
  - 只有一侧控件且恢复为拉伸：最终尺寸由父容器决定、无法预估，跳过动画直接复位。
  目标值还会夹紧到控件的 `min_width/max_width`（纵向为高度）。
- **动画过程**：使用现有的 `AnimationPlayer`（`AnimationType::kAnimationNone` + `EaseOutCubic`，16ms 帧间隔）。动画期间把两侧控件临时转换为固定尺寸，每帧设置左侧为插值、右侧为“总尺寸 - 插值”（只有一侧控件时只调整该控件），保证动画期间总占用空间不变、分割线平滑移动。
- **结束处理**：动画结束时按首次拖动前记录的原始类型和值恢复两侧控件（固定/拉伸/自动），统一触发布局并重绘；尺寸没有变化或动画时长为 0 时走立即复位的分支。
- **交互冲突**：动画期间再次按下鼠标（开始新的拖动）会在 `ButtonDown()` 中立即停止动画，由拖动接管；按下方向键时会在 `OnKeyDown()` 中先停止动画，避免键盘操作与动画同时修改控件尺寸；再次双击也会先停止上一次动画。
- **验证**：Debug 版 `TreeView` 示例（非 DLL 模式）中给主分割条配置 `double_click_reset="true" reset_anim="1200"`（放大动画时长便于观察），先把分割线从物理像素 x=519 拖到 667，再双击分割条：动画中（约 350ms）分割线位于 x=566，动画结束后回到 x=517（与拖动前基本一致，误差为像素测量精度），进程无断言；恢复默认 150ms 后双击同样能正常复位。

## 六、建议实施顺序

1. 修边界夹紧（第 1、4、5 点）—— 影响正确性；
2. 拖动状态生命周期（第 2、3 点）+ 光标可用性（第 6 点）；
3. 性能与事件去抖（第 8、9 点）；
4. 代码去重/重命名、文档同步（第 10、11 点）；
5. 新交互：双击复位、拖动开始/结束事件、拖动阈值、目标控件可指定、键盘操作、尺寸提示；
6. 主题类与示例，统一各处的分割条观感。

以上各项均已完成并通过 Debug 版 `TreeView` 示例（非 DLL 模式）的界面验证。
