# layout 示例优化方案

## Context

`layout` 示例用于演示各种布局容器（Box/HBox/VBox/FlowBox/TileBox/ScrollBox/ListBox/GridBox 等），当前界面与 `TreeView` 示例存在明显风格差异：

- 内容区没有任何外边距，TabBox 的 `padding="3,3,3,3"` 实际不生效（TabBox 不会把 padding 传递给子页面），页面内容紧贴边框。
- TabBox 圆角只有 3px、边框色用 `border_window`，与 TreeView 的「8px 圆角卡片 + `border_split_level1` 描边 + `bg_window_card` 底色」不一致。
- `TabCtrl` 外层套了一个 `bg_tab_ctrl_item_selected` 色块的 VBox，形成一条突兀的横条，且标签栏没有圆角卡片感。
- 17 个标签在窗口默认宽度下被压缩，文字截断为 `HFlo…`、`Scro…`，无法区分 HFlowBox 与 HFlowScrollBox。

用户已确认：**保留页面内的彩色演示色块**（不改成主题中性色），只统一外壳风格；**标签改为按文字自适应宽度**。

## 一、代码检查结论（不改动 C++）

`MainForm.cpp / MainThread.cpp / TestApplication.cpp` 均为标准示例模板，未发现功能性问题（`CreateWnd` 未判空是各示例的统一写法，保持不动）。

XML 侧发现的问题：

1. `layout.xml`：TabBox `padding` 不生效（见上）。
2. `page_gridbox.xml`：
   - 第 4 行 `text_align="center,hcenter"`，`hcenter` 是水平对齐值，这里想要「水平+垂直居中」，应改为 `hcenter,vcenter`。
   - 第 75 行 `text_align="center,right"`，解析时 `center` 优先级高于 `right`，实际是居中而非右对齐，应改为 `right,vcenter`。
3. `page_hflow_scroll_box.xml` 第 39 行文案 `text="2 enter"` 为笔误，应为 `2 center`。
4. `page_tile_scroll_box.xml`：VTileScrollBox 固定 `height="400"`、HTileScrollBox 固定 `height="380"`，在默认窗口高度下第二个容器会被裁切；改为 `45%` / `stretch`。
5. `page_hbox.xml` 第 61 行：四个 HBox 高度 60%+15%+10%+15% = 100%，再加上各自的 4px margin，会溢出 12px；最后一个改为 `height="stretch"`。

以下仅为记录、不修改：`page_list_box.xml` 列表项用 `system_regular_14`（其余页面为 12）；`page_tabbox.xml` 用 `normal_text_color="black"`；若干页面标签编号重复（如两个 "12 center"）。

## 二、外壳统一（`bin/resources/themes/default/layout/layout.xml`）

参考 `tree_view.xml` 的卡片样式（`bg_window_card` + `border_split_level1` + 8px 圆角 + 10px 外边距 + 8px 间隔）。

1. 窗口尺寸 `75%,75%` → `80%,80%`（与 TreeView 一致，也给标签栏更多宽度）。
2. 在 `<Window>` 下新增局部样式类：
   - `layout_card`：`bkcolor="bg_window_card" border_size="1" border_color="border_split_level1" border_round="8,8"`（供 TabBox 使用）。
   - `layout_tab_item`：继承 `tab_ctrl_item`，`padding="8,0,0,0"`、`title_class="layout_tab_item_title"`。
   - `layout_tab_item_title`：继承 `tab_ctrl_item_title`，`width="auto"`、`margin="2,0,8,0"`（去掉 `stretch`，让标签宽度随文字自适应）。
3. 把原来的 `<VBox bkcolor="bg_tab_ctrl_item_selected">` 换成 `<VBox bkcolor="bg_window_main" padding="10,10,10,10">`，作为整个内容区。
4. TabCtrl：保留 `class="tab_ctrl"`，显式设置 `height="40"`、`border_round="8,8"`、`margin="0,0,0,8"`。
5. 17 个 `TabCtrlItem` 的 `class` 改为 `layout_tab_item`，其余属性不变。
6. TabBox：`class="layout_card"`、`bkcolor="bg_window_card"`、`selected_id="0"`，删除 `margin="4,4,4,4"`、`padding="3,3,3,3"`、`border_size/border_color/border_round`（由类提供），保留淡入淡出切换属性。
7. 17 个页面容器 `<VBox name="main_view_page_XX">` 增加 `padding="10,10,10,10"`，让页面内容与卡片边框保持 10px 间距。

## 三、验证

1. 构建：`build\build.bat build --config Debug --arch x64 --sdl --no-deps --target layout`。
2. 运行 `bin\layout.exe`（工作目录 `bin`），截图检查：
   - 内容区四周 10px 留白、TabBox 8px 圆角描边、标签栏与内容卡片间距 8px；
   - 标签显示完整名称（必要时出现横向滚动条），不再出现 `HFlo…`；
   - 切换 `ListBox`、`GridBox`、`TileScrollBox` 页，确认彩色演示块保持原样、无裁切；
   - 点击标题栏主题按钮切到深色主题，确认外壳（卡片、标签栏、留白）跟随主题变化。
3. 清理临时截图与后台进程。
