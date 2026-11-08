# Combo 控件界面美化与交互优化

## Context

用户要求 Review `Combo` 控件（含下拉列表弹窗）的代码，重点改善界面美观度。当前实现存在以下观感问题：

- 控件本体为直角边框（无 `border_round`），内部编辑区使用 `bg_richedit` 独立底色，与控件底色割裂；右侧下拉按钮用整块矩形悬停底色 + 左侧分隔线，观感偏"老式 Win32 组合框"。
- 下拉弹窗使用方形阴影（`kShadowMenu`），列表项无内边距（文字贴边）、悬停/选中高亮为满宽矩形并直贴窗口边缘，弹窗与列表均无圆角、无描边，与主题其余控件（卡片、按钮均 4~10px 圆角）风格不一致。
- 弹窗位置计算未对显示器工作区做水平方向钳制，控件靠近屏幕右边缘时下拉框可能溢出屏幕。

目标：在**不改动控件结构与 API**的前提下，通过主题（global.xml）与少量弹窗定位代码，统一为 4~6px 圆角、留白合理、悬停柔和的现代下拉框风格，并保持 Windows / SDL 双后端一致。

## 关键现状（已确认）

- 控件外观全部来自主题类：`bin/resources/themes/default/global.xml` 中 `combo_basic` / `combo_base` / `combo`（约 343-364 行）。
- 下拉弹窗由 `CComboWnd` 承载（`duilib/Control/Combo.cpp`），根 `Box` 无背景，实际铺满窗口的是 `TreeView`；其样式由 `combo_tree_view_class` 属性列表决定，因此**只改主题即可改变弹窗底色/内边距/描边/圆角**，无需改窗口代码。
- 阴影类型支持 `shadow_type="menu_round"`（`Shadow::GetShadowType`），且 Combo 主题类可直接声明 `shadow_type` 属性覆盖默认的 `kShadowMenu`（`Combo::SetAttribute` 已实现）。
- 颜色语义已存在：`bg_menu`、`border_combo_normal`、`bg_tree_view_node_hovered/selected`、`bg_combo` 等，light/dark 两套主题均有定义，**无需新增颜色**。
- `GetComboWndRect()`（Combo.cpp 约 105-180 行）已取 `GetMonitorWorkRect` 做上/下翻转判断，但未做左右边界钳制。

## 实施方案

### 1. 主题样式（bin/resources/themes/default/global.xml）

**`combo_basic`**：增加 `border_round="4,4"`，保留 1px 描边与现有配色，使控件本体呈圆角。

**`combo_base`**：
- `combo_tree_view_class` 改为：`class='tree_view' padding='4,4,4,4' border_size='1' border_color='border_combo_normal' border_round='6,6' bkcolor='bg_menu' indent='20'`。弹窗整体为 6px 圆角、1px 描边、4px 内边距，底色与菜单/控件统一。
- `combo_tree_node_class` 改为：`class='tree_node' text_padding='8,0,8,0' text_align='vcenter,left' hovered_color_round='4,4' selected_normal_color_round='4,4'`。列表项文字留白，悬停/选中高亮呈 4px 圆角胶囊（若 TreeNode 不支持 round 属性则自动退化为原矩形，无副作用）。
- `combo_edit_class`：`bkcolor` 改为空（透出控件自身 `bg_combo`），`text_padding` 改为 `8,0,4,0`，消除内外底色割裂。

**`combo`**（在 `combo_base` 之上）：
- 增加 `shadow_type="menu_round"`，弹窗圆角与内部 6px 圆角对齐。
- `combo_button_class` 调整为：去掉左侧分隔线（`border_size='0,0,0,0'`）、`margin='2,2,2,2'`、`padding='0,0,0,0'`、`hovered_color_round='3,3'` / `pressed_color_round='3,3'`，箭头图片保持现有 SVG。悬停时呈内缩的小圆角块，不再顶满圆角容器。
- 属性列表采用主题中已有的单引号内嵌写法，注意与 `pagination_page_size`（继承 `combo`，已自行覆盖大部分属性）不冲突。

### 2. 弹窗定位代码（duilib/Control/Combo.cpp）

`CComboWnd::GetComboWndRect()` 末尾（`ClientToScreen` 之后、返回之前）增加水平钳制：

```cpp
// 将下拉窗口限制在显示器工作区内，避免靠近屏幕边缘时溢出
UiRect rcWork;
pOwner->GetWindow()->GetMonitorWorkRect(rcWork);
if (rc.right > rcWork.right) {
    rc.Offset(-(rc.right - rcWork.right), 0);
}
if (rc.left < rcWork.left) {
    rc.Offset(rcWork.left - rc.left, 0);
}
```

（复用函数开头已调用的 `GetMonitorWorkRect`，不新增接口。）

同时修复一个一致性小问题：`Combo::SetComboType(kCombo_DropDown)` 分支补 `SetNoCaretReadonly(false)`，避免从 `drop_list` 切换为 `drop_down` 后仍保留"只读不显示光标"的设置。

### 3. 不改动的部分

- 不新增/删除控件属性与 API；不改 `CheckCombo`、`ComboButton`（如需要可后续统一）。
- 不新增图片资源，沿用 `public/combo/arrow_normal.svg` 与 `arrow_hovered.svg`。

## 验证

1. 构建 Debug x64（SDL）：`build\build.bat --config Debug --arch x64 --sdl --offline --jobs 2 build`。
2. 运行 `bin\RichEdit.exe`：
   - 工具栏字体/字号/缩放三个 Combo：确认控件为圆角描边、文字留白、右箭头按钮悬停为内缩圆角块。
   - 展开字号 Combo：确认弹窗为 6px 圆角 + 1px 描边 + 4px 内边距、列表项悬停为圆角高亮、滚动条正常、选中项显示正常。
   - 将窗口拖到屏幕右边缘后展开较宽的下拉框，确认弹窗被钳制在工作区内。
   - 切换深色主题复核配色（`bg_menu` / `border_combo_normal` 均已存在）。
3. 回归：`controls.xml` 示例页的 Combo / FilterCombo / CheckCombo 外观与交互正常；SDL 下下拉项点击仍有效（避免回归上一轮修复）。
4. 清理验证用临时截图与脚本（不留在工作区）；不执行 git commit。
