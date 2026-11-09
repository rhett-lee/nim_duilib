# ColorTheme 示例：112 个内部子样式显式演示计划

## Context

`examples/ColorTheme` 的主题预览页此前只直接引用了部分 Class；`global.xml` 中还有 112 个"内部子样式"（由控件在运行时自动应用的子控件样式），页面中没有显式出现。用户要求补充这 112 个内部子样式的显式演示，同时保证 Debug 版本运行不再触发断言。

上一阶段已修复 `MainForm.cpp` 中 Alias 色块误用 Class 名作为颜色导致的断言，并移除了 `Control.cpp` 中的临时诊断代码。本阶段只做两件事：
1. 在 `bin/resources/themes/default/color_theme/color_theme.xml` 新增 Section 32，显式实例化全部 112 个内部子样式（另含 2 个基类 `btn_global_base`/`btn_wnd_base`，合计 114 个）。
2. 通过 Debug 构建 + 实际滚动预览验证无 CRT 断言。

## 关键事实（已核实）

- 内部子样式必须宿主到其原生控件类型，否则 Debug 会因未知属性触发 `ASSERT(!"Control::SetAttribute failed: unknown attribute name!")`。
- `Line`、`Mask`、`Switch`、`Calendar`、`Pagination`、`SearchBox`、`TagInput`、`AutoComplete`、`AddressBar`、`Popover`、`Popconfirm`、`DateTime`、`RichEdit`、`ListBoxItem`、`VListBox`、`TreeView`、`TreeNode` 均为合法 XML 标签。
- ListCtrl / TreeGrid / TreeView / TabCtrl / AddressBar / Pagination / Combo / ScrollBox / RichEdit 均支持 `*_class` 属性（见下）。
- `calendar_day*`/`calendar_weekday` 仅在 `Calendar.h` 中文档化，`Calendar.cpp` 未实际应用；用 `Label`（及背景 `Box`）显式展示其文字/背景样式。
- `popover_*`/`popconfirm_*` 子样式由 Popover/Popconfirm 动态生成；用等价静态结构显式展示。
- `auto_complete_*`/`list_ctrl_combo_*` 浮层子样式同样用等价静态结构展示。

## 修改文件

### 1. `bin/resources/themes/default/color_theme/color_theme.xml`

在 Section 31（约 L1250-1253）之后、`<Control height="20"/>`（L1255）之前插入 **Section 32**，结构为：

```
<!-- Section 32: Internal Sub-Styles - Explicit Demo (112 classes) -->
<GroupVBox class="group_box_round" text="Internal Sub-Styles - Explicit Demo (112)" height="auto"
           margin="0,4,0,0" padding="10,24,6,10" control_padding="false">
    ... 各子分组 ...
</GroupVBox>
```

每个子分组由一个说明 `Label`（列出该组 Class 名）+ 演示控件组成，统一使用 `VBox width="stretch" height="auto" child_margin="6"`。

子分组与宿主映射（共 114 个，含 2 个基类）：

1. **Base / ToolBar（5）**
   - `<Button class="btn_global_base" width="120" height="28" text="..."/>`
   - `<Button class="btn_wnd_base" .../>`
   - `<Button class="toolbar_button" text="toolbar_button"/>`
   - `<CheckBox class="toolbar_check" text="toolbar_check" selected="true"/>`
   - `<Line class="toolbar_separator"/>`

2. **Combo 基类（2）**：两个 `<Combo class="combo_basic|combo_base" combo_type="drop_list" dropbox_size="0,120" width="180" height="30">`，内含 `<TreeNode class="tree_node" text="..."/>`。

3. **AddressBar（11）**
   - 独立按钮：`address_bar_reset_button`、`address_bar_history_button`、`address_bar_path_separator`、`address_bar_sub_path_collapse`、`address_bar_sub_path_button`、`address_bar_sub_path_button_current`、`address_bar_sub_path_root`、`address_bar_sub_path_drive`、`address_bar_sub_path_computer`（全部 `<Button>`）。
   - `address_bar_sub_path_hbox` → `<HBox>` 内放一个 Label。
   - `address_bar_edit` → `<RichEdit class="address_bar_edit" .../>`。
   - 另加一个真实 `<AddressBar class="address_bar" address_path="C:\Users\Public\Documents" .../>`，显式写出 `rich_edit_class/reset_button_class/sub_path_hbox_class/sub_path_button_class/sub_path_root_class/sub_path_drive_class/sub_path_current_class/collapse_button_class/path_separator_class`。

4. **AutoComplete（6）**
   - `<AutoComplete class="auto_complete" ... items="Alpha;Beta;Gamma"/>`
   - `<RichEdit class="auto_complete_edit" .../>`
   - `<VBox class="auto_complete_flyout"><VListBox class="auto_complete_list"><ListBoxItem class="auto_complete_item"><Label class="auto_complete_item_match"/><Label class="auto_complete_item_text"/></ListBoxItem></VListBox></VBox>`

5. **Calendar（7）**：`calendar_weekday`/`calendar_day`/`calendar_day_muted`/`calendar_day_today`/`calendar_day_selected`/`calendar_day_range`/`calendar_day_hovered` 用 `<Label>` 展示；selected/range/hovered 外套带背景色的 `<Box>`；另加一个 `<Calendar class="calendar" width="280" height="240"/>`。

6. **CascadingSelect（1）**：`<CascadingSelect class="cascader" dropdown="true" popup_class="cascading_select_popup" items="A/1|A/2|B/3" .../>`。

7. **ListCtrl（20）**：三个声明式 `<ListCtrl>`（report / icon / list），分别显式写出全部 `*_class` 属性：
   - report：`header_class`、`header_item_class`、`header_split_box_class`、`header_split_control_class`、`check_box_class`、`data_item_class`、`data_sub_item_class`、`report_view_class`、`list_ctrl_richedit_class`、`list_ctrl_datetime_class`；子项用 `<ListCtrlHeaderItem>`、`<ListCtrlItem>`、`<ListCtrlSubItem item_index column_index>`。
   - icon：`icon_view_class`、`icon_view_item_class`、`icon_view_item_image_class`、`icon_view_item_label_class`。
   - list：`list_view_class`、`list_view_item_class`、`list_view_item_image_class`、`list_view_item_label_class`。
   - 浮层三件套静态结构：`<VBox class="list_ctrl_combo_flyout"><VListBox class="list_ctrl_combo_list"><ListBoxItem class="list_ctrl_combo_item"/></...>`。
   - 另加 `<RichEdit class="list_ctrl_richedit"/>`、`<DateTime class="list_ctrl_datetime"/>` 直接展示。

8. **Mask（1）**：`<Box width="240" height="80" bkcolor="bg_window_card"><Label text="Under Mask"/><Mask class="mask"/></Box>`。

9. **Pagination（3）**：`<Pagination total_count="95" page_size="10" show_page_size="true" show_first_last="true" page_size_class="pagination_page_size" button_class="pagination_button" page_size_options="10,20,50"/>`（`pagination_nav_button` 由上一页/下一页/首末页按钮自动应用）。

10. **Popover / Popconfirm（19）**
    - `<Popover class="popover_trigger" trigger="hover" placement="top" text="popover_trigger"/>`
    - `<VBox class="popover_card"><Label class="popover_title"/><Label class="popover_content"/></VBox>`
    - `<Control class="popover_arrow_up|down|left|right"/>`
    - `<Button class="popconfirm_btn|popconfirm_cancel|popconfirm_ok_default|popconfirm_ok_primary|popconfirm_ok_danger"/>`
    - `<Control class="popconfirm_icon_warning|info|success|error"/>`
    - `<Label class="popconfirm_title"/>`、`<Label class="popconfirm_description"/>`

11. **RichEdit（8）**：`<RichEdit class="richedit_base"/>`、`rich_edit_spin`、`rich_edit_spin2`、`rich_edit_clear_btn`、`rich_edit_show_password_btn`（后四者会分别带出 `rich_edit_spin_box`、`rich_edit_spin_btn_up/down`、`rich_edit_spin_btn_up2/down2`、`rich_edit_clear_btn_class`、`rich_edit_show_password_btn_class`）。

12. **ScrollBar（5）**
    - `<VScrollBox vscrollbar="true" vscrollbar_class="vscrollbar|vscrollbar2|scrollbar_base">`（各配高内容）
    - `<HScrollBox hscrollbar="true" hscrollbar_class="hscrollbar|hscrollbar2">`（各配宽内容）
    - `scrollbar_base` 注明为基类（无图片资源，需由 vscrollbar/hscrollbar 派生显示）。

13. **SearchBox（3）**：`<SearchBox class="search_box" text="search"/>`，另直接展示 `<Control class="search_box_icon"/>`、`<RichEdit class="search_box_edit"/>`、`<Button class="search_box_clear_btn"/>`。

14. **Switch（2）**：`<Switch class="switch_ios" selected="false|true"/>`、`<Switch class="switch_small" selected="false|true"/>`。

15. **TabCtrl（10）**
    - 指示条风格：`<TabCtrl class="tab_ctrl tab_ctrl_indicator"><TabCtrlItem class="tab_ctrl_item tab_ctrl_item_indicator" title="..." width="220"/></TabCtrl>`（带出 `tab_ctrl_indicator`、`tab_ctrl_item_indicator`、`tab_ctrl_item_indicator_title`）。
    - 普通风格 TabCtrl，TabCtrlItem 显式写 `icon_class="tab_ctrl_item_icon" title_class="tab_ctrl_item_title" close_button_class="tab_ctrl_item_close" line_class="tab_ctrl_item_line" badge_class="tab_ctrl_item_badge" badge_count="5"`；另两个 Item 分别用 `badge_class="tab_ctrl_item_badge_dot" badge_dot="true"` 与 `badge_class="tab_ctrl_item_badge_float" badge_float="true" badge_count="99"`。

16. **TagInput（4）**：`<HBox class="tag_input_tag"><Label class="tag_input_tag_text"/><Button class="tag_input_tag_close"/></HBox>`、`<RichEdit class="tag_input_edit"/>`。

17. **TreeView / TreeGrid（7）**
    - `<TreeView class="tree_view" check_box_class="tree_node_checkbox" expand_image_class="tree_node_expand">` + 嵌套 `<TreeNode>`（覆盖 2 个）。
    - 修改 **Section 25**（约 L1151、L1157）两个 `<TreeGrid>` 标签，显式补充 `expand_image_class="tree_grid_expand" check_image_class="tree_grid_check" header_cell_class="tree_grid_header_cell" cell_class="tree_grid_cell" row_class="tree_grid_row"`（这 5 个已由 MainForm 填充数据，无需新增代码）。

所有新增控件命名统一加 `_styles` 后缀，避免与已有 name 重复（重复 name 会触发断言）。

### 2. `examples/ColorTheme/MainForm.cpp`（可选，仅滚动条常显）

若新增 ScrollBox 的滚动条因 `auto_hide_scroll` 默认隐藏影响展示，在 `InitPreviewControls()` 中对新增的 `preview_vscroll_styles_*` / `preview_hscroll_styles_*` 复用现有逻辑：
`GetVScrollBar()->SetAttribute("auto_hide_scroll", "false")` 并 `SetVisible(true)`。不改变其他行为。

## 验证

1. Debug 构建（链接前先结束进程）：
   ```
   Stop-Process -Name ColorTheme -Force -ErrorAction SilentlyContinue
   & 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -SkipAutomaticLocation
   cmake --build 'e:\develop\nim_duilib\out\x64\Debug' --target ColorTheme --config Debug
   ```
2. 运行 `bin\ColorTheme.exe`，激活窗口后连续发送 `{PGDN}` 滚动到页面底部（覆盖 Section 32 全部内容）。
3. 断言判定：窗口标题不得变为 "Microsoft Visual C++ Runtime Library"；若出现断言，按 File/Line/Expression 修正对应 Class 的宿主控件类型或去掉不支持的属性。
4. 人工检查：Section 32 中每个 Class 均有可见样例（`popover_trigger` 需悬停、`cascading_select_popup` 需点击展开、`auto_complete_*` 浮层为静态等价展示）。
5. 完成后可选执行 Release 构建回归。

## 约束

- 不创建额外文档/README；不提交 git。
- 不删除或改动 `Control.cpp` 中已恢复的断言逻辑。
