# TabCtrl 功能完善（C1–C10）实施方案

## Context

`TabCtrl`（duilib/Control/TabCtrl.h/.cpp，继承 `ListBox`）目前具备标签绘制、固定标签、角标、关闭按钮、溢出滚动等能力，但功能完善度仍存在 C1–C10 十项缺口。本次在不破坏现有行为的前提下补齐这些能力，并按用户要求：

- C1：`TabBox ↔ TabCtrl` 双向同步，必须防止互相触发造成死循环。
- C2：点击关闭按钮**仍然不删除标签**，只提供“关闭请求”事件和“可关闭”开关，删除由业务层完成。
- C5：中键关闭默认开启，并提供关闭该功能的选项。

所有新增 XML 属性与公开 C++ 接口需同步更新 `docs/Control.md`、`docs/Control.en.md`、`docs/schema/nim_duilib.xsd`。不自动提交 git。

## 关键现状（已确认）

- 事件：`EventType` 定义在 [duilib_defs.h](file:///e:/develop/nim_duilib/duilib/duilib_defs.h#L425-L465)，字符串映射表在 [EventArgs.cpp](file:///e:/develop/nim_duilib/duilib/Core/EventArgs.cpp#L111-L130)。
- `Control` 提供 `AttachEvent` / `DetachEvent`（`EventCallbackID`），中键消息会分发到虚函数 `MButtonDown` / `MButtonUp`；键盘用 `IsKeyDown(msg, ModifierKey::kControl)`，键值常量在 `duilib/Core/Keycode.h`。
- `ListBox` 已实现方向键/PageUp/PageDown/Home/End 选择逻辑（`OnListBoxKeyDown`，virtual）与 `OnListBoxMouseWheel`（virtual），并有受保护的 `SelectEnsureVisible(index, bTakeFocus)`。
- `ScrollBox::SetPos` 为 virtual，可重写做等宽布局后处理；`GetScrollRange()` 可判断是否溢出。
- 布局层已支持 `min_width` / `max_width`（`Layout.cpp` 中 `std::clamp`），因此 C7 的最小/最大宽度只需文档说明并在等宽模式中复用。
- `global.xml` 中 `tab_ctrl_item_line` 为 `float="true" halign="right" valign="center" width="1" height="22"`（水平模式的右侧竖线）。

## 实施内容

### 1. 事件与开关（C2、C5、C8）

`duilib/duilib_defs.h` + `duilib/Core/EventArgs.cpp`：
- 在 `EventType` 末尾新增：
  - `kEventTabClose`：TabCtrl 请求关闭标签，WPARAM = 标签索引。
  - `kEventTabNew`：点击“新建标签”按钮，WPARAM = 0。
- 同步补充事件名映射（`kEventTabClose`/`TabClose`/`tab_close`，`kEventTabNew`/`TabNew`/`tab_new`）。

### 2. TabCtrl.h / TabCtrl.cpp

新增公开 API / 属性：

| 功能 | 接口 | XML 属性 | 默认值 |
|---|---|---|---|
| C1 | `SetTabBox/GetTabBox` 保持不变，内部挂接事件 | 已有 `tab_box_name` | - |
| C2 | `AttachTabClose` | - | - |
| C3 | 无新接口（选择变化时自动调用 `SelectEnsureVisible`） | - | 开启 |
| C4 | `SelectNextTab/SelectPrevTab` | - | - |
| C5 | `SetEnableWheelSwitch/IsEnableWheelSwitch` | `wheel_switch` | false（默认关闭，滚轮用于滚动标签栏） |
| C5 | `SetEnableMiddleClickClose/IsEnableMiddleClickClose` | `middle_click_close` | true |
| C5 | `SetDragInsertColor/GetDragInsertColor` | `drag_insert_color` | `#FF2D7FF9` |
| C6 | `SetEnableOverflowButtons/IsEnableOverflowButtons` | `overflow_buttons` | true |
| C7 | `SetTabItemJustify/IsTabItemJustify` | `tab_item_justify` | false |
| C8 | `SetEnableNewTabButton/IsEnableNewTabButton`、`AttachTabNew` | `new_tab_button` | false |
| C9 | `GetSelectedTabItem/SetSelectedTabItem/GetTabItemAt/AddTabItemToEnd` | - | - |

实现要点：

- **C1 双向同步（防死循环）**
  - 新增成员 `bool m_bSyncingTabBox`。
  - `SetTabBox`：先对旧 TabBox `DetachEvent(kEventTabSelect, (EventCallbackID)this)`，再挂接新 TabBox 的 `kEventTabSelect` 回调（回调内用 `ControlPtrT<TabCtrl>` 保护）。
  - `TabBox → TabCtrl`：回调取 `msg.wParam` 新索引，先用 `FindTabItemByTabBoxIndex()` 映射到标签项（优先 `tab_box_item_index`，未配置时回退为同索引），置 `m_bSyncingTabBox = true` 后 `SelectItem(index, false, true)`，结束后复位。
  - `TabCtrl → TabBox`：`SyncTabBoxSelection()` 开头 `if (m_bSyncingTabBox) return;`，调用 `m_pTabBox->SelectItem()` 前后设置/复位该标志。
  - 避免在未初始化/未绑定/索引无效时同步。

- **C2 关闭请求**
  - 不修改“点击关闭不删标签”的现有行为。
  - `TabCtrlItem` 新增 `SetClosable/IsClosable`（XML `closable`，默认 true）；不可关闭或固定标签时隐藏关闭按钮（复用 `UpdateCloseButtonVisible`）。
  - `TabCtrlItem::HandleEvent` 中判断 `msg.GetSender() == m_pCloseBtn && msg.eventType == kEventMouseClick`，调用新增的 `TabCtrl::NotifyTabCloseRequest(TabCtrlItem*)`，由 TabCtrl 计算索引并 `SendEvent(kEventTabClose, index)`。

- **C3 选中项自动可见**
  - `TabCtrl::HandleEvent` 处理完 `kEventSelect` 后调用 `SelectEnsureVisible(GetCurSel(), false)`（仅单选、索引有效时）。

- **C4 键盘导航**
  - 重写 `bool OnListBoxKeyDown(const EventArgs&)`：
    - `Ctrl+Tab` → `SelectNextTab()`（循环）；`Ctrl+Shift+Tab` → `SelectPrevTab()`（循环）。
    - `Ctrl+1..9` → 选择对应索引（存在时），返回 true。
    - 其余按键回落 `BaseClass::OnListBoxKeyDown`（方向键/翻页/Home/End 已由 ListBox 按当前布局方向处理）。

- **C5 鼠标交互**
  - 滚轮：重写 `OnListBoxMouseWheel`；`wheel_switch` 开启时按滚轮方向 `SelectPrevTab/SelectNextTab`，否则回落基类滚动。
  - 中键关闭：`TabCtrlItem` 重写 `MButtonDown`，当父 TabCtrl 的 `middle_click_close` 开启、标签可关闭且非固定时，调用 `NotifyTabCloseRequest`。
  - 拖动插入提示：`PaintForeColor` 中查找 `IsInDraggingOrder()` 的标签项，用 `drag_insert_color` 在其前缘绘制 2px 插入线（水平画竖线，垂直画横线）。

- **C6 溢出滚动按钮（选择“左右箭头按钮”方案）**
  - 当 `GetScrollRange()` 在对应方向大于 0 时，`PaintForeColor` 在标签栏两端绘制半透明箭头按钮（三角/折线用 `IRender::DrawLine`）。
  - 命中处理：`TabCtrl::ButtonDown` 处理空白区域点击；`TabCtrlItem::ButtonDown` 在调用基类前把落在箭头区域的点击转发给 TabCtrl（`HandleOverflowButtonClick`），执行 `PageLeft/PageRight`（垂直方向 `PageUp/PageDown`；`LineXxx` 的默认步长为 0，不能产生滚动）。
  - 箭头矩形由 DPI 缩放的固定尺寸计算，供绘制与命中测试共用。
  - 与浮动滚动条互斥显示：箭头开启时隐藏滚动条（`SetVisible(false)` + `SetMouseEnabled(false)`），但滚动条对象必须保留，否则 `SetScrollPos` 失效；箭头关闭后恢复鼠标响应，悬浮时重新淡入。

- **C7 自适应宽度**
  - 最小/最大宽度：直接复用 Control 的 `min_width` / `max_width`，在文档中说明 TabCtrlItem 支持。
  - 等宽填充：`SetTabItemJustify(true)` 后重写 `TabCtrl::SetPos`，先 `BaseClass::SetPos`，再 `LayoutJustify()`：取可见非浮动标签项，按可用宽度（或高度）平分，`std::clamp` 到 min/max 后依次 `SetPos`；该模式下关闭滚动条并复位滚动位置，避免内容尺寸与实际不符。

- **C8 新建标签按钮**
  - `new_tab_button=true` 时，在最后一个标签之后（或标签栏右端）绘制“+”按钮；溢出时隐藏（与箭头按钮错开）。
  - 命中转发同 C6；触发 `SendEvent(kEventTabNew)`，不自动创建标签（由业务层处理）。

- **C9 便捷 API**
  - `TabCtrlItem* GetSelectedTabItem() const;`
  - `bool SetSelectedTabItem(TabCtrlItem* pItem, bool bTriggerEvent = true);`
  - `TabCtrlItem* GetTabItemAt(size_t) const;`
  - `bool AddTabItemToEnd(Control* pItem);`（固定标签仍进入固定块）
  - `bool SelectNextTab(bool bTakeFocus = true, bool bTriggerEvent = true);` / `SelectPrevTab(...)`（循环）。

- **C10 垂直模式细节**
  - `TabCtrl` 新增 `UpdateItemsLayoutForOrientation()`：垂直时把标签项宽度设为 `stretch`（铺满标签栏），水平时恢复 `auto`；在 `OnInit`、`SetTabCtrlOrientation`、`AddItemAt` 后调用。
  - `TabCtrlItem` 新增 `UpdateLineOrientation(bool bVertical)`：垂直时 `m_pLine` 用 `SetClass(m_lineClass)` 复位后改为 `width=stretch`、`height=1`、`valign=bottom`、`margin="4,0,4,0"`（底部横线），水平时复位为类配置（右侧竖线）；保留 `m_bLineVisible` 的 Alpha 状态。
  - 垂直模式的底部边线（`PaintStateColors`）当前绘制“右边竖线”，改为绘制“右侧竖线”保持不变（这是标签栏与内容区的分隔线，语义正确）；仅修正标签项内部分隔线。

### 3. 文档同步

- `docs/Control.md` / `docs/Control.en.md`：在 TabCtrl 与 TabCtrlItem 属性表补充上述新增属性（属性/默认值/类型/说明），并补充新接口说明。
- `docs/schema/nim_duilib.xsd`：在 TabCtrl、TabCtrlItem 对应 `attributeGroup` 中新增 `xs:attribute` + `xs:documentation`。
- 若项目存在事件文档（如 `docs/Event*`），补充 `kEventTabClose` / `kEventTabNew`。

## 涉及文件

- `duilib/duilib_defs.h`（事件枚举）
- `duilib/Core/EventArgs.cpp`（事件名映射）
- `duilib/Control/TabCtrl.h` / `duilib/Control/TabCtrl.cpp`（主要实现）
- `docs/Control.md`、`docs/Control.en.md`、`docs/schema/nim_duilib.xsd`
- 可能微调 `examples/controls/TabCtrlForm.cpp`（仅在需要演示/验证新特性时）

## 验证

1. 编译（PowerShell，显式设置 MSVC/SDK 环境变量，与之前一致）：
   - `cmake --build --preset x64-debug --target duilib`
   - `cmake --build --preset x64-release --target duilib`
2. 运行 `bin/controls.exe` 的 TabCtrl 示例：
   - 水平/垂直切换：垂直模式标签铺满宽度、分隔线为底部横线。
   - 溢出时出现左右箭头，点击可滚动；不溢出时不显示。
   - 滚轮切换标签；中键点击触发关闭事件（不删标签）；`middle_click_close="false"` 时不触发。
   - `Ctrl+Tab` / `Ctrl+Shift+Tab` / `Ctrl+1..9` 切换正确；选中项自动滚入视野。
   - `tab_box_name` 绑定后，TabBox 侧选择变化能同步回 TabCtrl，且无死循环/递归。
   - `tab_item_justify="true"` 时标签等宽铺满；`new_tab_button="true"` 时出现“+”并触发事件。
3. 文档与 XSD 自查：新增属性均能在文档表中检索到、XSD 可被示例 XML 校验通过。
