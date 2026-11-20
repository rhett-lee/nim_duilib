# Box 目录代码审查报告

## 一、审查范围

- 审查目录：`duilib/Box`
- 审查文件：
  - `Box` 相关的容器与派生：`Panel.h`、`TabBox.h` / `TabBox.cpp`
  - `ListBox.h` / `ListBox.cpp`、`ListBoxItem.h`、`ListBoxHelper.h` / `ListBoxHelper.cpp`
  - `ScrollBox.h` / `ScrollBox.cpp`、`VirtualListBox.h` / `VirtualListBox.cpp`
  - 简单派生类：`HBox.h`、`VBox.h`、`GridBox.h`、`TileBox.h`
- 审查重点：索引维护、生命周期/重入安全、DPI 与折叠布局状态一致性、边界条件。
- 审查原则：不做大的修改，只修复确认的必要缺陷；需要较大范围调整或改变既有行为的问题
  记录在"五、建议后续处理（本轮未修改）"。

## 二、总体结论

整体实现质量较好：

1. 容器 `Box` 的子项管理（`AddItemAt` / `RemoveItemAt` / `SetItemIndex`）中，索引越界检查
   与 `Box::InvalidIndex` 的判定使用一致，未发现越界访问；
2. `ListBox` 的单选/多选状态同步（`SelectItemSingle` / `SelectItemMulti` /
   `OnSwitchToSingleSelect`）逻辑完整，多选切换单选时能正确收敛到一个选择项；
3. `ScrollBox` 的滚动位置更新有递归深度保护，`EnsureVisible` 的位置钳位正确；
4. `VirtualListBox` 的数据索引与控件索引映射、`ListBoxHelper` 的框选坐标换算与定时器
   重入保护均正确；
5. `Panel` 的折叠/展开动画回调使用 `ControlPtrT` 弱引用，动画中销毁控件不会产生悬空访问。

本轮确认并修复 4 处必要缺陷（其中 3 处属于索引/状态一致性，1 处属于事件后的悬空访问），
并补充了覆盖这些修复点的单元测试。

## 三、已修复的问题

### 1. TabBox 无选择项时删除子项，选择索引下溢为"有效索引"

- 位置：`TabBox.cpp`，`TabBox::RemoveItem`
- 问题：
  - 删除非当前选择项时执行 `else if (m_nCurSel > index) { m_nCurSel -= 1; }`；
  - 当所有页签都不可见时，当前选择为 `Box::InvalidIndex`（即 `size_t(-1)`），
    该条件恒成立，减一后变成 `size_t(-2)`；
  - `Box::IsValidItemIndex` 只判断"不等于 `InvalidIndex`"，因此 `size_t(-2)` 会被当成
    有效索引，后续按该索引访问页签会越界或选中错误的页面。
- 修复：条件改为 `Box::IsValidItemIndex(m_nCurSel) && (m_nCurSel > index)`，
  无选择项时保持 `InvalidIndex` 不变。
- 影响：仅影响"当前无选择页"这一边界状态，正常选择场景行为不变。

### 2. TabBox 页签切换事件回调中销毁控件后继续访问成员（悬空访问）

- 位置：`TabBox.cpp`，`TabBox::DoSelectItem`
- 问题：
  - 代码已为 `m_tabSelectListeners` 建立 `ControlPtrT` 临时副本，避免事件回调删除
    `TabCtrl` 后的悬空指针；
  - 但 `SendEvent(kEventTabSelect, m_nCurSel, ...)` 之后仍两次读取成员 `m_nCurSel`
    （事件参数与 `OnBoundTabBoxSelectChanged` 的入参）；
  - 若用户在 `kEventTabSelect` 回调中销毁了 `TabBox` 本身，后续对 `m_nCurSel` 的读取
    属于对已析构对象的访问。
- 修复：发送事件前保存局部变量 `const size_t nNewSelIndex = m_nCurSel;`，
  事件参数与反向同步均使用局部变量。
- 影响：无行为变化，仅消除悬空访问风险。

### 3. Panel 在展开动画中途遇到 DPI 变化，最小高度被永久放宽

- 位置：`Panel.h`，`PanelTemplate::ChangeDpiScale`
- 问题：
  - 折叠动画开始时会调用 `BeginCollapseLayout()`，把用户配置的 `min_height` 保存到
    `m_nSavedMinHeight`，并临时把 `min_height` 放宽到标题栏高度；
  - 展开动画结束（`OnCollapseAnimFinished`）时才通过 `EndCollapseLayout()` 恢复；
  - 若在**展开动画进行中**（`m_bCollapsed == false`，但 `m_nSavedMinHeight >= 0`）
    发生 DPI 变化，原实现停止动画后仍按"折叠布局生效中"重新放宽 `min_height`，
    并保留 `m_nSavedMinHeight`，导致面板的最小高度被永久放宽，直到下一次完整的
    折叠+展开动画才恢复。
- 修复：在 `m_nSavedMinHeight >= 0` 分支中区分折叠状态：
  - `m_bCollapsed == true`：保持原有逻辑，按新标题栏高度重新放宽；
  - `m_bCollapsed == false`：按新 DPI 缩放保存值后恢复用户的最小高度，并清空
    `m_nSavedMinHeight`。
- 影响：仅影响"展开动画中被 DPI 变化打断"的低频场景，布局结果恢复为用户配置值。

### 4. ListBox 的 Shift 多选起点未随子项插入/删除/排序调整

- 位置：`ListBox.cpp`，`ListBox::AddItemAt` / `RemoveItemAt` / `RemoveAllItems` / `SortItems`
- 问题：
  - `m_nLastNoShiftItem` 是不按 Shift 时最后一次选中项的索引，作为 Shift 范围选择的起点；
  - `AddItemAt` 会同步 `m_iCurSel` 与各子项的 `ListBoxIndex`，`RemoveItemAt` 也会同步，
    但都没有调整 `m_nLastNoShiftItem`；
  - 插入/删除后该值可能指向错误的项，导致下一次 Shift+点击的选择区间起点错位。
    使用处虽有 `nLastNoShiftItem >= nItemCount` 的兜底（不会崩溃），但区间会不符合预期。
- 修复：
  - `AddItemAt`：`iIndex <= m_nLastNoShiftItem` 时起点加一；
  - `RemoveItemAt`：`m_nLastNoShiftItem >= iIndex` 且大于 0 时起点减一；
  - `RemoveAllItems` / `SortItems`：起点复位为 0（排序后原选择已清空）。
- 影响：仅影响 Shift 范围选择的起点，普通点击选择不受影响。

## 四、已核实、无需修改的部分

以下位置在审查中被重点核对，未发现缺陷，本轮未修改：

1. `ListBox::SelectItemSingle` / `SelectItemMulti` / `UnSelectItem`：
   选择事件在状态更新之后、函数返回之前发送，符合事件约定；
2. `ListBox::OnSwitchToSingleSelect` / `UpdateCurSelItemSelectStatus`：
   多选切单选时以"第一个已选择项"为准，索引无效时会重新扫描；
3. `ListBox::SetItemIndex`：
   重排后会同步受影响区间的 `ListBoxIndex`，并按 `GetListBoxIndex` 反推当前选择索引；
4. `ListBox::RemoveItemAt` 的 `SelectNextWhenActiveRemoved` 分支：
   `FindSelectable(m_iCurSel, false)` 在删除之后计算，命中"原位置/向前"的可选项，
   不会索引越界；
5. `TabBox::AddItemAt` / `OnInit` / `OnSetVisible`：
   插入位置与当前选择比较时已正确使用 `Box::IsValidItemIndex`；
6. `Panel` 的 `SetCollapsible` / `ApplyCollapsed` / `StartCollapseAnim`：
   折叠布局与动画的状态切换一致，动画回调均有弱引用保护；
7. `ScrollBox::DoSetPos` 的递归深度保护、`SetScrollPos` 的钳位；
   `VirtualListBox::FillElementData` / `EnsureVisible` / `PaintChild` 的索引映射；
8. `ListBoxHelper::OnCheckScrollView`：
   坐标换算（相对窗口客户区 + 滚动偏移）正确，定时器重入有
   `m_scrollViewFlag.Cancel()` 保护。

## 五、建议后续处理（本轮未修改）

1. `ListBox::RemoveItemAt` 中 `SelectNextWhenActiveRemoved` 的注释写"选择下一个可选择的项"，
   实际行为是优先选择"原位置（即被删除项的下一个）"，向前找不到时才回退；建议后续
   统一注释与实现描述。
2. `Box::FindSubControl` 直接调用 `GetWindow()->FindSubControlByName(...)`，在无窗口
   （如单元测试直接实例化控件）时不可用；这是既有设计约定，未修改。
3. `ListBox::EnsureVisible` / `VirtualListBox::EnsureVisible` 对非法索引使用 `ASSERT`
   依赖调用方保证参数合法；如需更强的健壮性，可改为返回失败而不是断言。
4. `ListBoxHelper::OnCheckScrollView` 在发送鼠标事件后仍会访问 `m_pListBox` 的部分状态，
   若事件回调销毁了 ListBox 存在理论风险；当前实现中该访问是最后一步，风险很低，
   未做改动。

## 六、单元测试

新增 `tests/test_box.cpp`，并在 `tests/CMakeLists.txt` 注册为 `box`：

- `Box` 基类：子项增加/插入/删除/清空、索引查询、`SetItemIndex` 重排、边界返回值；
- `FindSelectable`：跳过不可见/不可用/不可选择的子项，前向与后向查找；
- `HBox` / `VBox` / `GridBox` / `TileBox`：布局类型与 `GridBox` 行列属性；
- `ListBox`：单选、插入/删除时的当前选择索引与 Shift 锚点维护、删除当前项后自动选择、
  多选与切换回单选的同步、框选配置项与取值范围；
- `TabBox`：页签选择、插入/删除时的选择索引维护，以及"无选择项时删除子项"的
  `InvalidIndex` 下溢回归用例；
- `Panel`：标题、可折叠状态、折叠时 `min_height` 的临时放宽与展开后的恢复、属性读写。

测试不创建窗口、不启动消息循环，并在测试入口关闭全局动画，避免启动定时器。

验证命令与结果：

```text
python build\build.py build --config Debug --arch x64 --offline --no-deps --no-examples --target test_box
ctest --test-dir out\x64\Debug -C Debug -R "^box$" --output-on-failure

1/1 Test #33: box ..............................   Passed    0.09 sec
100% tests passed, 0 tests failed out of 1
```
