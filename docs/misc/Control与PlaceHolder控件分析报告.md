# Control 与 PlaceHolder 控件分析报告

下面的分析基于 `duilib/Core/Control.h`、`duilib/Core/Control.cpp`、`duilib/Core/PlaceHolder.h`、`duilib/Core/PlaceHolder.cpp`，并对照了 `duilib/Layout/Layout.cpp`、`duilib/Box/ScrollBox.h`、`duilib/Box/ScrollBox.cpp` 等关联实现。文中的行号对应分析时的代码版本；修复进展请更新开头的“处理状态”表。

## 处理状态

本轮已完成 PH-1 ~ PH-9（PlaceHolder 部分全部完成），Control/美观度部分仍为“待处理”。

| 编号 | 类别 | 内容 | 状态 |
| --- | --- | --- | --- |
| PH-1 | 正确性 | 64 位滚动偏移与 32 位实现语义不一致 | 已修复 |
| PH-2 | 性能 | `SetMinWidth` / `SetMinHeight` 先分配数据再比较 | 已修复 |
| PH-3 | 性能 | `SetToolTipWidth(0)` 为无 ToolTip 的控件分配数据 | 已修复 |
| PH-4 | 交互 | 运行时修改 `rowSpan` / `columnSpan` 不触发布局 | 已修复 |
| PH-5 | 交互 | `SetEnableControlPadding` 不触发重新布局 | 已修复 |
| PH-6 | 健壮性 | `maxWidth` / `maxHeight` 允许小于 `min`，非法值被静默吞掉 | 已修复 |
| PH-7 | 代码质量 | `SetMargin` 与 `SetPadding` 的负值处理不一致、存在冗余钳制 | 已修复 |
| PH-8 | 文档 | `InvalidateRect()` 的坐标空间未说明；大内容下仍使用 32 位偏移 | 已修复 |
| PH-9 | API | `GetAncestor()` 缺少 `const` 重载 | 已修复 |
| C-1 | 正确性 | `SetShowFocusedRect` / `SetFocusedRectColor` 修改后不重绘 | 已修复 |
| C-2 | 文档 | `HoveredAlpha` 注释写成“焦点状态透明度” | 已修复 |
| C-3 | 文档 | 焦点框注释仍写“虚线构成的矩形”，与可重写的圆角描边不符 | 已修复 |
| C-4 | 文档 | `OnApplyAttributeList` 注释为“待补充” | 已修复 |
| C-5 | 文档 | `SetStateColor*` 注释参数与实际重载签名不匹配 | 已修复 |
| C-6 | API | `SetCursor()` / `SetCursorType()` 语义不同但命名近似 | 已修复 |
| C-7 | API | `IsMouseFocused()` 实际含义是“鼠标是否在控件上” | 已修复 |
| C-8 | 健壮性 | `TEventMapData` 裸指针 + 自定义析构，未禁止拷贝（rule of five） | 已修复 |
| C-9 | 代码质量 | 构造函数成员初始化顺序与声明顺序不一致 | 已修复 |
| C-10 | 健壮性 | ToolTip 默认宽度 500 未按 DPI 初始化 | 已修复 |
| C-11 | 结构 | `Control.h` 职责过重、`AttachXXX` 样板代码过多 | 不处理 |
| U-1 | 美观度 | 通用焦点框为内缩 3px 的方形点线，观感较差 | 已修复 |
| U-2 | 交互 | 焦点框只应在键盘操作后出现（MenuBar 方案已通用化到 Window） | 已修复 |
| U-3 | 交互 | 键盘焦点复用鼠标悬浮状态，导致悬浮状态被误清除 | 待处理 |
| U-4 | 美观度 | ToolTip 最大宽度在高 DPI 下偏窄 | 待处理 |
| U-5 | 美观度 | 高 DPI 下边框/焦点框细线可能时隐时现 | 待处理 |

> PH-1 修复说明：在 `ScrollBox` 新增虚函数 `GetScrollOffset64ForChild(const PlaceHolder*)`（默认返回 `GetScrollOffset64()`），
> `PlaceHolder::GetScrollOffsetInScrollBox64()` 改为调用该接口；`ListCtrlReportView` 同步重写，
> 使冻结列区域的横向偏移在 64 位路径下同样按 0 计，与 32 位 `GetScrollOffsetForChild` 的口径一致。

## 一、总体结论

`PlaceHolder` 是所有控件的基类，负责矩形区域、内外边距、固定尺寸与最小/最大尺寸、对齐、浮动定位、估算尺寸缓存、单元格合并与滚动偏移换算；`Control` 在此基础上叠加状态、颜色、图片、字体、动画、事件、拖动、ToolTip、右键菜单等能力。

整体设计是合理的：

- 大量可选数据采用 `std::unique_ptr` 懒分配（`TPlaceHolderData`、`TColorData`、`TEventMapData` 等），空控件开销小；
- `UiFixedInt` 同时表达“固定值 / Auto / Stretch”，布局语义清晰；
- 关键入口普遍采用“断言 + 运行时容错”双保险；
- DPI 缩放集中在 `ChangeDpiScale()` 与各 Setter 的 `bNeedDpiScale` 参数上。

主要问题集中在四类：

1. 少量正确性/一致性问题（64 位滚动偏移、运行时修改属性不触发重排）；
2. DPI 切换与默认值设置路径上的无谓内存分配；
3. 注释与实现脱节、命名易混淆等可维护性问题；
4. 焦点视觉过于简陋，且键盘焦点与鼠标悬浮状态耦合。

## 二、PlaceHolder 的问题

### 2.1 正确性与健壮性

**PH-1 64 位滚动偏移与 32 位实现不一致（已修复）**

- 位置：`duilib/Core/PlaceHolder.cpp` 中的 `GetScrollOffsetInScrollBox()`（约 733-758 行）与 `GetScrollOffsetInScrollBox64()`（约 760-784 行）。
- 现象：32 位版本对每个祖先 `ScrollBox` 调用 `GetScrollOffsetForChild(this)`（虚函数，子类可重写，用于冻结列等特殊偏移）；64 位版本直接累加 `GetScrollOffset64()`，绕过了子类重写。
- 影响：同一控件在两条代码路径下得到的滚动偏移不同；虚拟滚动、冻结列、超大内容场景会出现点击坐标与绘制位置偏移或重绘错位。
- 建议：在 `ScrollBox` 中补充 `GetScrollOffset64ForChild(const PlaceHolder*)`（默认返回 `GetScrollOffset64()`，供子类重写），64 位版本改为调用该接口；或在文档中明确 64 位路径不支持冻结列并加断言。
- 修复：已按上述第一种方案实现。`ScrollBox` 新增虚函数 `GetScrollOffset64ForChild()`，`PlaceHolder::GetScrollOffsetInScrollBox64()` 改为调用它；`ListCtrlReportView` 同步重写，对冻结列子控件返回横向偏移 0，与 32 位实现一致。

**PH-2 / PH-3 DPI 切换时的无谓分配（已修复）**

- 位置：`duilib/Core/Control.cpp` 的 `ChangeDpiScale()`（约 732-760 行）、`duilib/Core/PlaceHolder.cpp` 的 `SetMinWidth()`（约 301-322 行）/ `SetMinHeight()`（约 367-387 行）、`duilib/Core/Control.cpp` 的 `SetToolTipWidth()`（约 2022-2037 行）。
- 现象：
  - `GetMinWidth()` / `GetMinHeight()` 在无数据时返回 0，`ChangeDpiScale()` 用 `>= 0` 判断后调用 Setter，而 Setter 内部先 `CheckPlaceHolderData()` 再比较值，导致几乎所有控件在每次 DPI 切换时都会分配 `TPlaceHolderData`；
  - `GetToolTipWidth()` 在无 ToolTip 数据时返回 0，`ChangeDpiScale()` 同样会调用 `SetToolTipWidth(0)`，而该函数会创建 `TOtherData` 与 `TTooltipData`。
- 影响：DPI 切换（多显示器拖动、系统缩放变化）时全窗口控件批量分配，增加内存与缓存压力，且产生一次无意义的重排调用。
- 建议：
  - Setter 先比较当前值再决定是否 `CheckPlaceHolderData()`；
  - `GetToolTipWidth()` 无法区分“无数据”和“宽度为 0”，建议增加 `HasToolTip()` 判断，或让 `ChangeDpiScale()` 仅在 `GetToolTipWidth() > 0` 时缩放；
  - `SetToolTipWidth()` 在 `nWidth == 0 && 无 ToolTip 文本` 时直接返回。
- 修复：
  - `SetMinWidth` / `SetMinHeight` / `SetMaxWidth` / `SetMaxHeight` 改为先与 `GetMinWidth()` / `GetMaxHeight()` 等 Getter 比较，值未变化时直接返回，再按需 `CheckPlaceHolderData()`；
  - `SetToolTipWidth()` 增加两处提前返回：宽度为 0 且无 ToolTip 数据时不创建数据块；值未变化时不重复赋值；
  - `ChangeDpiScale()` 的 ToolTip 宽度缩放条件由 `>= 0` 改为 `> 0`。

**PH-4 运行时修改单元格合并不重排（已修复）**

- 位置：`duilib/Core/PlaceHolder.cpp` 的 `SetRowSpan()` / `SetColumnSpan()`（约 620-652 行）。
- 现象：只更新数据，不调用 `ArrangeAncestor()` / `RelayoutOrRedraw()`。
- 影响：`GridLayout` 页面在运行时动态合并/拆分单元格后，界面要等到窗口尺寸变化才刷新。
- 建议：值发生变化时调用 `RelayoutOrRedraw()`（该方法已根据 Auto 尺寸选择重排或重绘）。
- 修复：`SetRowSpan()` / `SetColumnSpan()` 在值变化后按浮动状态调用 `ArrangeAncestor()` 或 `Arrange()`；同时对相同值提前返回，避免无意义的数据块创建与重排。

**PH-5 `SetEnableControlPadding` 不刷新布局（已修复）**

- 位置：`duilib/Core/PlaceHolder.cpp`（约 520-528 行）。
- 影响：该属性参与 `GetControlPadding()`，切换后子控件可用区域不更新。
- 建议：值变化时调用 `ArrangeAncestor()`（或 `RelayoutOrRedraw()`）。
- 修复：值变化后按浮动状态调用 `ArrangeAncestor()` 或 `Arrange()`；值未变化时直接返回。

**PH-6 最大尺寸允许小于最小尺寸（已修复）**

- 位置：`SetMaxWidth()`（约 336-357 行）、`SetMaxHeight()`（约 401-422 行）；`GetMaxWidth()`（约 324-334 行）、`GetMaxHeight()`（约 389-399 行）。
- 现象：Setter 只断言 `>= 0`，不校验与 min 的关系；Getter 检测到 `max < min` 时静默返回 `INT32_MAX`。
- 影响：配置错误被掩盖，布局结果与配置预期不一致。`duilib/Layout/Layout.cpp` 使用 `std::clamp(v, min, max)`，目前依赖 Getter 的补偿才避免违规，属于隐式契约。
- 时序问题（分析补充）：只在 max 的 Setter 里做 `max = max(min, max)`（或只在 min 的 Setter 里钳制），会导致“先设置的一方胜出、后设置的值被吞掉”。例如原区间 `[100, 200]` 要改成 `[40, 90]` 时，`SetMaxWidth(90)` 会先被抬到 100，再调用 `SetMinWidth(40)` 后结果变成 `[40, 100]`，与调用顺序相关。
- 修复（采用方案 A：Setter 保持不变量 `min <= max`，双向收敛）：
  - `SetMinWidth` / `SetMinHeight`：若设置值大于当前最大尺寸，同步把最大尺寸调整为该值；
  - `SetMaxWidth` / `SetMaxHeight`：若设置值小于当前最小尺寸，同步把最小尺寸调整为该值；
  - 因此任意调用顺序的最终结果一致，上例两种顺序都得到 `[40, 90]`；
  - Getter 的异常兜底由 `INT32_MAX` 改为返回 min（`max < min` 时以 min 为准），保证下游 `std::clamp` 不会出现未定义行为；
  - 提前返回条件同步扩展为“值未变化且不破坏不变量”，避免无意义的数据分配与重排。

**PH-7 负值处理不一致（已修复）**

- 位置：`SetMargin()`（约 491-506 行）与 `SetPadding()`（约 513-532 行）。
- 说明：`SetMargin` 在 `Validate()` 之后又手工把四个方向钳到 `>= 0`；`SetPadding` 依赖 `Validate()` + `TruncateToUInt16(int32_t)`（`duilib/Core/UiTypes.h` 约 232-238 行）把负值钳到 0。
- 结论：**不会出现负值转成 65531 的问题**，因为 `TruncateToUInt16` 已做下限钳制；此处仅属于重复代码与风格不一致。
- 修复：删除 `SetMargin` 中 `Validate()` 之后的 4 段重复负值钳制，两个 Setter 统一为“`ASSERT` → `Validate()` → 按需 DPI 缩放 → 比较 → `TruncateToUInt16` 存储”，行为不变。

**PH-8 重绘接口的坐标契约与 64 位支持（已修复）**

- 位置：`Invalidate()` / `InvalidateRect()`（约 724-760 行）。
- 说明：`InvalidateRect(rc)` 会先与控件绘制外展矩形取交集，再减去滚动偏移；因此 `rc` 必须是“未减滚动偏移的绝对坐标”，但接口注释未说明，容易误用。
- 另外：重绘始终使用 32 位 `GetScrollOffsetInScrollBox()`；内容滚动超过 `int32` 范围时（虚拟列表极端数据量）偏移会失真。窗口重绘区域本身是 32 位限制，可接受，但建议注释说明该限制。
- 修复：在 `PlaceHolder.h` 的 `Invalidate()` / `InvalidateRect()` 注释中明确三点：重绘区域使用绝对坐标（窗口客户区坐标）、不包含父容器滚动偏移、内部会取交集并减去滚动偏移；同时注明最终受 32 位坐标限制及触发场景。未新增接口（“本地坐标”重载暂不需要）。

**PH-9 缺少 `const` 重载（已修复）**

- 便于在 `const` 上下文中查询祖先容器，新增 `const Box* GetAncestor(const DString& strName) const`（`PlaceHolder.h` / `PlaceHolder.cpp`），与非 `const` 版本行为一致；非 `const` 对象仍优先匹配原重载，不影响既有调用。

### 2.2 其他代码质量项（已补充注释）

- `SetRowSpan()` 使用 `TruncateToInt16` 存储（约 625 行），超出范围时静默饱和，建议在注释中说明上限（或改用 `int32`）。
- `ArrangeAncestor()`（约 550-575 行）逻辑正确，但“window 为空且无父控件”的分支会调用 `ArrangeSelf()`，而 `ArrangeSelf()` 在不可见时直接返回，建议注释说明该行为，避免调用方误以为已重排。
- `SetMargin`/`SetPadding`/`SetMinWidth` 等 Setter 的“值未变化则不重排”逻辑重复，可提取模板/辅助函数，减少遗漏。

> 处理说明（按要求仅补充注释，不修改代码）：
> - `SetRowSpan()` / `SetColumnSpan()`：头文件注释已明确有效范围 `[1, 32767]`，`<= 0` 忽略（Debug 断言）、`> 32767` 按 32767 存储（Debug 断言），并注明值变化后触发重排。
> - `ArrangeAncestor()` / `ArrangeSelf()`：头文件注释已说明本函数只发起重排请求、实际布局由窗口统一执行，且 `ArrangeSelf()` 在控件不可见时直接返回。
> - “值未变化则不重排”的约定：已记录在 `ArrangeAncestor()` 的注释中，作为各布局相关 Setter 的统一约定，后续新增 Setter 应遵循。

## 三、Control 的问题

### 3.1 行为与状态

**C-1 焦点框属性修改后不重绘（已修复）**

- 位置：`duilib/Core/Control.cpp` 的 `SetShowFocusedRect()`（约 2303-2313 行）与 `SetFocusedRectColor()`。
- 现象：只更新数据，不调用 `Invalidate()`。
- 影响：运行时开启/关闭焦点框、修改颜色后，必须等下一次其他原因触发重绘才生效。MenuBar 的键盘焦点功能因此额外调用了 `Invalidate()` 规避。
- 修复：`SetShowFocusedRect()` 增加值变化判断，值变化且控件持有焦点时立即 `Invalidate()`；`SetFocusedRectColor()` 原本已在值变化后调用 `Invalidate()`。

**C-2 至 C-5 注释与实现脱节（已修复）**

- `SetHoveredAlpha` / `GetHoveredAlpha` 注释原写“焦点状态透明度”，实际是鼠标悬浮淡入淡出进度（0=正常，255=完全悬浮）。
- `SetShowFocusedRect` / `SetFocusedRectColor` 注释原写“虚线构成的矩形”，但 `DoPaintFocusRect()` 已为虚函数，派生类可重写为圆角描边等样式。
- `OnApplyAttributeList()` 注释原为 `@brief 待补充`。
- `SetStateColor*` 上方的注释原描述了合并后的 5 个参数，与三个重载各自的签名不一致。
- 修复：以上注释均已按实际语义与签名重写；`OnApplyAttributeList()` 补充了 `strReceiver` 的四类取值（空/`#window#`/`./`或`.\\`/窗口内名称）、属性列表的花括号写法、返回值与调用来源。

**C-6 / C-7 命名易混淆（已修复：补充注释，未改名以保持兼容）**

- `SetCursorType()`（约 361 行）只设置属性，等待 `OnSetCursor` 时生效；`SetCursor()`（约 1645-1648 行）会立即应用到系统光标。建议将后者改名（如 `ApplyCursor()`）或补充“立即生效”的说明。
- `IsMouseFocused()`（约 518-521 行）实际表示“鼠标是否位于控件上”，与键盘焦点无关；建议改名 `IsMouseHovered()` 或补充注释。
- 修复：两处均补充了明确的注释说明（未改名，避免破坏既有代码）；注释中指明键盘焦点应使用 `IsFocused()`。

**C-8 `TEventMapData` 的 rule-of-five 问题（已修复）**

- 位置：`Control.h` 约 1770-1812 行。
- 现象：字段包含 `EventMap` 值对象与三个裸指针，析构中手工 `delete`，但未禁用拷贝构造/赋值。
- 风险：若被按值拷贝会 double free。当前通过 `unique_ptr` 持有、实际未发生拷贝，但属于潜在隐患。
- 修复：显式声明默认构造 + `= delete` 的拷贝构造/拷贝赋值（数据块由 `std::unique_ptr` 独占持有），从编译期阻止浅拷贝；析构保持原有释放逻辑。

**C-9 成员初始化顺序（已修复）**

- `Control` 构造函数（`Control.cpp` 约 30-48 行）的初始化列表顺序与声明顺序不一致（如 `m_bBoxShadowPainted`、`m_uUserDataID` 在列表中较晚，但声明在前）。
- 影响：MSVC C5038 / GCC `-Wreorder` 可能告警；实际初始化按声明顺序执行，阅读时易误判依赖关系。
- 修复：按成员声明顺序重排初始化列表（`m_uUserDataID` → `m_bBoxShadowPainted` → `m_controlState` → `m_nAlpha` → ... → `m_bMouseEnter`），初始化值不变，行为不受影响。

**C-10 ToolTip 默认宽度未按 DPI 初始化（已修复）**

- 位置：`TTooltipData::m_nTooltipWidth = 500`（`Control.h` 约 1814-1830 行），`GetToolTipWidth()` 返回 0 表示“无数据”。
- 现象：高 DPI 下首次创建 ToolTip 时得到 500 物理像素，逻辑宽度反而变窄；无数据时返回 0 又与结构体默认值不一致。
- 修复：`TTooltipData` 改为必须显式传入宽度，并新增 `kDefaultWidth = 500`（逻辑像素）；创建 ToolTip 数据时统一使用 `Dpi().GetScaleInt(kDefaultWidth)` 初始化，高 DPI 下默认宽度保持 500 逻辑像素；显式调用 `SetToolTipWidth` 时仍以传入值为准。

### 3.2 结构性优化

**C-11 `Control.h` 职责过重（不处理）**

- 现状：单头文件同时承载状态、颜色、图片、字体、动画、事件、拖动、ToolTip、右键菜单、快照、绘制顺序等能力；`AttachXXX` 系列内联包装超过 60 个。
- 影响：头文件依赖广、下游工程重编译成本高；新增事件类型需要修改超大类。
- 建议（长期项）：
  1. 把事件相关接口下沉到独立基类（如 `EventSource`），`Control` 继承之；
  2. 不常用能力（拖动、动画、快照）使用 pimpl 或 mixin，减少头文件暴露；
  3. 事件包装可考虑模板化生成，降低样板代码量。
- 结论：属于结构性重构，影响面大、收益偏长期，经确认**暂不处理**，保留现状。

## 四、界面美观度与交互改进

**U-1 通用焦点框观感较差（已修复）**

- 原状：`Control::DoPaintFocusRect()` 绘制内缩 3px、线宽 1.5px（DPI 缩放）的**方形点线**，不跟随控件圆角。
- 问题：小尺寸按钮上点线过密、方形描边与圆角按钮不协调，MenuBar 的实际效果被用户评价为“太难看”。
- 修复（对齐 Windows 11 原生焦点指示器）：
  1. 改为**强调色圆角描边**，内缩 1px、线宽约 2px（DPI 缩放且保证至少 1px）、圆角约 3px；
  2. 颜色优先 `focused_rect_color`，其次 `color_accent`，再回退 `border_focus_ring`，最后 `Gray`；
  3. 移除 `MenuBarButton::DoPaintFocusRect` 的私有定制，MenuBar 复用统一默认实现，避免两套视觉；
  4. 线宽使用 `Dpi().GetScaleFloat(2.0f)` 并下限钳制到 1px，避免高 DPI 下过细。
- 说明：主题新增 `focused_rect_width` / `focused_rect_round` 配置项暂未实现；如需不同主题差异化，后续可再扩展。

**U-2 焦点框只应响应键盘操作（已修复）**

- 现状：MenuBar 通过控件内 `m_bKeyboardFocus` 实现“仅键盘焦点显示”，其他控件未通用化；鼠标点击也会让控件获得键盘焦点并显示焦点框。
- 修复：把“最近一次输入是键盘还是鼠标”上移到 `Window`：
  1. `Window` 新增 `m_bKeyboardCueVisible`（默认 false，与 Windows 初始隐藏焦点提示一致）与只读接口 `IsKeyboardCueVisible()`；
  2. `Window::OnKeyDownMsg()`（含 `WM_SYSKEYDOWN`）置为 true；`OnMouseLButtonDownMsg()` / `OnMouseRButtonDownMsg()` / `OnMouseMButtonDownMsg()` 置为 false；
  3. 状态变化时调用 `InvalidateAll()`，使焦点框立即显示或隐藏；
  4. `Control::ShouldPaintFocusRect()` 增加该条件判断；`TabCtrl::PaintTabFocusRect()` 同步判断；
  5. 移除 `MenuBarButton` 的 `m_bKeyboardFocus` / `SetKeyboardFocused()` / `ShouldPaintFocusRect()` 重写，统一由窗口状态驱动。
- 效果：键盘操作（Tab、方向键、F10/Alt、回车等）后显示焦点框；鼠标按下后立即隐藏，与 Windows 原生行为一致。
- 配套调整（默认开启策略）：`Control` 仍保持默认 `false`，改由交互控件在构造函数中默认开启，避免给容器/装饰控件绘制大焦点框：
  - 已开启：`Button`（含 `ButtonBox`/`ButtonHBox`/`ButtonVBox`，以及派生的 `CheckBox`/`Option`/`Switch`）、`Combo`、`Slider`、`RichEdit`（Windows 与 SDL 两个实现）、`IPAddress`、`DateTime`。
  - 未开启：`ListBox`/`TreeView` 等容器类——焦点实际落在条目上，且 `Menu`、`Combo` 的下拉列表继承自 `ListBox`，整体开启会让菜单/下拉层出现边框；`TabCtrl` 保持既有实现（焦点框绘制在选中标签上）。

**U-3 键盘焦点复用鼠标悬浮状态**

- 现状：`Control::OnSetFocus()` 在 Normal 状态下把控件置为 `kControlStateHovered`（`Control.cpp` 约 3236-3254 行）；`OnKillFocus()` 再从 Hovered 恢复 Normal（约 3256-3276 行）。
- 问题：
  1. 键盘焦点与鼠标悬浮视觉完全相同，用户难以区分两者；
  2. 鼠标停在控件上时用 Tab 把焦点移走，`OnKillFocus` 会把状态改成 Normal，悬浮高亮错误消失，直到鼠标移动才恢复。
- 建议：新增 `kControlStateFocused`（并提供 `focused_bk_color` / `focused_border_color` 等主题属性）；或在 `OnKillFocus` 中检测鼠标是否仍在控件内，仅在鼠标不在时恢复 Normal。更彻底的做法是焦点视觉只由焦点框表达，不再借用 Hovered 状态。

**U-4 ToolTip 最大宽度 DPI 化**

- 与 C-10 同源。高 DPI 下 500 物理像素的提示宽度过早换行，影响可读性。建议按逻辑像素定义并缩放，0 明确表示“不限制”。

**U-5 高 DPI 细线一致性**

- 已有 `SetStateColorMinWidth()` / `SetStateColorMinHeight()` 解决状态色矩形在缩放后线条消失的问题（`Control.h` 约 147-152 行）。
- 建议把同样的“最小可见宽度”思路扩展到 `border_size` 与焦点框线宽，避免 125%/150%/175% 缩放下边框时隐时现。

## 五、建议实施顺序

1. **PH-1**：统一 32/64 位滚动偏移语义（正确性，影响大内容与冻结列）。
2. **PH-2 / PH-3**：调整 Setter 的“先比较后分配”，减少 DPI 切换时的全量内存分配。
3. **PH-4 / PH-5 / PH-6 / C-1**：补重排与重绘触发、校验 min/max 关系（改动小、收益直接）。
4. **U-1 / U-2 / U-3**：焦点框视觉统一、仅键盘显示、焦点状态与悬浮状态解耦（体验提升）。
5. **C-2 至 C-7、C-9**：文档与命名清理（无行为风险）。
6. **C-8**：事件数据 RAII 化（维护性）。
7. **C-10 / U-4 / U-5**：DPI 相关视觉细节。
8. **C-11**：头文件职责拆分（长期项，需评估下游兼容性）。

## 六、核对说明

以下结论已对照实现核对，避免误报：

- `SetPadding()` 传入负值不会产生 65531：`TruncateToUInt16(int32_t)` 内部已把负值钳到 0（见 `duilib/Core/UiTypes.h` 约 232-238 行），仅 Debug 下有断言。因此 PH-7 定为代码质量项而非功能缺陷。
- `GetMaxWidth()` / `GetMaxHeight()` 在 `max < min` 时的兜底已由 `INT32_MAX` 改为返回 `min`（PH-6 修复后），保证 `Layout.cpp` 中的 `std::clamp` 始终满足 `min <= max`，不会触发未定义行为。
- `Control::OnSetFocus()` 关闭输入法的行为与焦点视觉无关，属于既有设计，本报告不将其列为问题。
- U-1 已把通用焦点框改为强调色圆角描边，MenuBar 的私有重写（`MenuBarButton::DoPaintFocusRect`）已移除，统一复用默认实现。
