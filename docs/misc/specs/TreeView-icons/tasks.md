# TreeView 图标优化 Tasks

## 实施任务

- [x] T1 重绘界面图标（`bin/resources/themes/default/tree_view/`）
  - [x] `arrow-up.svg`
  - [x] `arrow-left.svg`
  - [x] `arrow-right.svg`
  - [x] `refresh.svg`
  - [x] `chevron-bottom.svg`
  - [x] `view-sort.svg`
  - [x] `view-list-symbolic.svg`
- [x] T2 重绘私有菜单图标（`bin/resources/themes/default/tree_view/menu/`）
  - [x] `data_icons_display-symbolic.svg`
  - [x] `view-list-symbolic.svg`（与 T1 同名文件内容一致）
  - [x] `view-list-compact-symbolic.svg`
  - [x] `view-list-images-symbolic.svg`
  - [x] `ui-item-symbolic.svg`
- [x] T3 `menu/list_type_menu.xml`：8 处图标 `bkimage` 增加 `file/width/height/svg_replace_colors`，适配深色主题
- [x] T4 `examples/TreeView/MainForm.cpp`：新增 `GetMenuCheckImage()`，替换 3 处 `SetBkImage`
- [x] T5 构建 `duilib` + `TreeView`，确认编译通过
- [x] T6 运行程序做视觉核对（浅色/深色主题、菜单、禁用态、DPI）
- [x] T7 功能回归（目录树、右键菜单、地址栏、搜索、状态栏）

- [x] T8 统一重绘 `public/filesystem` 13 个图标（96px 文件夹/文件 9 个 + 16px 设备 4 个），彩色扁平风格，不改文件名
- [x] T9 预览与实机核对：13 个图标在 48/32/20/16px 浅色与深色背景下可辨识；地址栏“此电脑”图标正常显示
- [x] T10 盘点 `public/address_bar`：6 个 SVG 中 5 个在用（`folder-root`/`reset`/`history`/`more-horizontal`/`chevron-right`），`filename-slash.svg` 全仓库无引用
- [x] T11 重绘 5 个在用图标为 16px 网格统一描边风格，文件名与 XML 引用不变；`filename-slash.svg` 与 `address_bar_menu.xml` 保持原样

## 验收标准

1. 12 个 SVG 全部为统一 16px 网格、1.5~1.7px 描边、圆角端点风格。
2. 图标文件名、XML/代码引用路径完全不变。
3. 深色主题下菜单图标与工具栏图标可见、颜色协调。
4. 菜单“当前选中”显示主题强调色勾选。
5. 构建成功，原有交互行为无变化。

## 验证记录

- T5：`cmake --build out\x64\Release --target duilib TreeView` 成功。
- T6：在 175% 缩放屏幕（2880×1800，窗口 2304×1384）实机截图核对，工具栏 7 个图标渲染清晰、视重一致，禁用态淡出正常；12 个图标另用浏览器 16px/32px 浅色/深色渲染预览确认形状与语义。
- T6：`color_dark/global.xml` 中 `border_svg_image=#D9F9F9F9`、`color_accent=#FF0078D4`，菜单图标与勾选在深色主题下可正常适配。
- T7：程序启动稳定运行，目录树、文件列表、地址栏、搜索框、状态栏均正常显示。
- T11：5 个图标用 headless Edge 在 16/14/12px 与 4× 放大下核对形状（浅色 `#5d656b` / 深色 `#D9F9F9F9`）；实机 175% 缩放下地址栏面包屑“本地磁盘 (C:) > Users > lee”的磁盘图标、chevron 分隔符、历史时钟按钮渲染正常，新建资源无需重新构建。
