简体中文 | [English](Differences.en.md)

# 与上游 duilib / NIM_Duilib_Framework 的差异

nim_duilib 基于 [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/) 继续开发，而 NIM_Duilib_Framework 又源自经典的 [duilib](https://github.com/duilib/duilib)。在长期维护中，本项目对代码结构、渲染引擎、布局体系、控件集合与示例程序做了系统性的调整与扩充。

本文完整记录这些差异，主要用于三类场景：评估从上游迁移到本项目的改动量；判断某个已有能力来自上游还是本项目新增；在遇到与上游文档不一致的行为时，确认是否属于有意调整。按类别分为整体改进、功能完善、新增控件/容器、性能优化、示例程序、文档与授权六部分，其中"新增控件/容器"有 50+ 项，逐项给出用途说明。

## 1. 整体改进（十余项）

这一部分是架构层面的调整，决定了本项目的代码组织方式与依赖关系，也是与上游差异最大、迁移时最需要关注的部分。

| 序号 | 改进内容 |
| :--- | :--- |
| 1 | 调整代码的组织结构，按功能模块划分；大文件按类拆分为多个小文件，便于理解整个库的体系结构 |
| 2 | 梳理接口文件，补充各接口的注释和功能说明，便于阅读和理解代码 |
| 3 | 优化配置 XML 文件：调整属性命名规则，控件宽度和高度支持按百分比设置；图片属性增加部分扩展，并优化图片加载流程 |
| 4 | 扩展图片资源支持：新增 APNG/WEBP 动画、Lottie JSON 动画、PAG 动画，新增 ICO 支持，并优化图片加载引擎及代码实现逻辑 |
| 5 | 重新实现各布局的代码，并新增常见界面布局方案，总体归类为：浮动布局、水平布局、垂直布局、水平流式布局、垂直流式布局、网格布局、水平瓦片布局、垂直瓦片布局、虚表水平布局、虚表垂直布局、虚表水平瓦片布局、虚表垂直瓦片布局，使布局概念更易理解、易于扩展（见 [Box.md](Box.md)） |
| 6 | XML 文件解析引擎替换为 pugixml parser，性能更优 |
| 7 | 移除对 base 库的依赖，消息循环和线程通信相关功能改为自行实现 |
| 8 | 集成 Skia 引擎，并作为默认的渲染引擎 |
| 9 | 支持 SDL3，实现跨平台（已适配 Windows、Linux、macOS、FreeBSD） |
| 10 | CEF 组件并入 duilib 工程，并升级 CEF 版本：支持 libcef 109（兼容 Windows 7）与 libcef 156（Windows 10 及以上） |
| 11 | 重新设计图片管理接口和加载流程（Image 目录），支持多线程加载图片，便于扩展更多图片格式 |
| 12 | 重新设计全局资源管理器的 XML 结构（global.xml），改进字体管理，支持字体回退以显示 Emoji；支持浅色/深色主题与强调色 |

## 2. 功能完善（二十余项）

这一部分是对上游已有窗口、容器和控件的可用性改进，多数改动会改变同名控件的行为或属性，迁移时需要按项确认。

| 序号 | 完善内容 |
| :--- | :--- |
| 1 | 完善窗口类（Window）的 DPI 自适应与窗口消息处理，提高代码容错，并调整代码结构 |
| 2 | 窗口类（Window）新增属性：`use_system_caption`、`snap_layout_menu`、`sys_menu`、`sys_menu_rect`、`icon`；既可使用操作系统默认标题栏，也可自绘标题栏并保持与系统标题栏相近的行为 |
| 3 | 窗口（Window）引入 `WindowDropTarget` 辅助类，提供基于窗口的拖放功能支持 |
| 4 | 梳理资源管理相关代码，使字体、颜色、图片等资源的管理更易理解 |
| 5 | 优化 ListBox 控件，按"排列方向 × 布局方式"细分为 ListBox（浮动布局）、HListBox/VListBox（水平/垂直布局）、HTileListBox/VTileListBox（水平/垂直瓦片布局），以及虚表实现的 VirtualListBox、VirtualHListBox、VirtualVListBox、VirtualHTileListBox、VirtualVTileListBox：普通版本为全部子项创建控件，虚表版本只创建并维护可见项，支持大数据量与滚动条；XML 中换用对应节点即可切换布局，虚表实现的可用性有较大提升（见 [Box.md](Box.md)） |
| 6 | 优化组合框（Combo）及关联控件：Combo 用 `combo_type` 选择不可编辑的下拉列表（`drop_list`）或可编辑的下拉框（`drop_down`），`dropbox_size`/`popup_top` 控制下拉尺寸与弹出方向、`shadow_type` 设置下拉窗口阴影，编辑框/按钮/图标/下拉树等子控件均可用各自的 Class（`combo_edit_class`/`combo_button_class`/`combo_icon_class`/`combo_tree_view_class`/`combo_tree_node_class`）定制；CheckCombo 的下拉项带勾选并支持多选（`GetSelectedText` 返回全部已选项，`dropbox`/`dropbox_item_class`/`selected_item_class` 配置下拉与已选项外观）；FilterCombo 支持按输入文本实时过滤下拉项（不支持 `combo_type` 属性） |
| 7 | 完善日期时间（DateTime）控件的编辑功能：`format` 定义显示格式，`edit_format` 提供 9 种编辑方式——日历浮层类（`date_calendar`/`date_time_calendar`/`time_calendar`）与上下箭头类（`date_up_down`/`date_time_up_down`/`date_minute_up_down`/`time_up_down`/`minute_up_down`）；支持日期约束（`min_date`/`max_date`/`disabled_dates`）、每周起始日（`first_day_of_week`）、秒列开关（`show_seconds`）与 `current_time` 自动初始化为当前时间；日历浮层支持方向键 / PageUp / PageDown / Home / End 导航，时间输入框支持上下键按步长调整 |
| 8 | 优化菜单（CMenuWnd）类：按新结构重新实现菜单，菜单内的控件完全兼容现有容器/控件体系，易于理解和维护 |
| 9 | 优化和扩展文本编辑控件（RichEdit）：Windows 平台为基于系统 ITextServices 的实现，RichEdit2 为本项目自研实现（非 Windows 平台上 RichEdit 即 RichEdit2 的别名）；支持单行/多行、只读、密码模式（`password`/`show_password`/`password_char`/`flash_password_char` 与显示密码按钮）、数字模式（`number_only` + `max_number`/`min_number`）、字符集与长度限制（`limit_chars`/`limit_text`）、URL 自动识别（`auto_detect_url`）、缩放（`zoom`/`wheel_zoom`）、行距（`row_spacing_mul`/`row_spacing_add`）、选择与当前行背景色、提示文字（`prompt_mode`/`prompt_text`）、焦点底边框（`focused_bottom_border_size`/`focused_bottom_border_color`），并内置步进/清除/显示密码按钮（`spin_class`/`clear_btn_class`/`show_password_btn_class`）以及文本与文件拖放（`enable_drag_drop`/`enable_drop_file`/`drop_file_types`）和默认右键菜单（`default_context_menu`） |
| 10 | 优化和扩展树控件（TreeView）：在 ListBox 基础上增加层级缩进（`indent`）、多选（`multi_select`）、三态勾选（`check_box_class`）、展开/收起图标（`expand_image_class`）与节点图标（`show_icon`）等能力；TreeNode 可分别配置"展开/收起 × 常态/悬停/按下/禁用"共 8 张状态图片，并用 `expand_image_right_space`/`check_box_image_right_space`/`icon_image_right_space` 调整图标与文字间距，配合示例程序 [examples/TreeView](../examples/TreeView) 使用 |
| 11 | 优化全局资源（GlobalManager）接口，所有资源统一通过该接口管理，易于理解和维护 |
| 12 | 同一窗口内的不同容器之间，容器内的控件可通过属性设置支持拖出和拖入操作 |
| 13 | 控件背景色支持渐变色，并新增前景色功能 |
| 14 | 完善多国语言功能，更好地支持多语言动态切换，并提供示例程序 [examples/MultiLang](../examples/MultiLang) |
| 15 | 完善 DPI 感知功能：支持 Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2 四种模式，支持独立设置 DPI 与高 DPI 适配，并提供示例程序 [examples/DpiAware](../examples/DpiAware) |
| 16 | 移除 ui_components 工程，CEF 组件代码重新梳理后并入 duilib 工程，其余内容删除 |
| 17 | 优化窗口阴影功能：阴影使用 SVG 图片，新增阴影类型属性（`shadow_type`），支持自绘阴影与系统阴影（见 [Window.md](Window.md)） |
| 18 | 新增 APNG/SVG/WEBP/ICO/LOTTIE/PAG 图片格式支持 |
| 19 | 重新设计控件的 loading 功能：使用 Box 容器展示 loading，通过 XML 配置 loading 界面（含动画图片），并支持与动画图片交互 |
| 20 | 加强 Label 文本显示控件：`text_align` 新增"两端对齐"（`hjustify`/`vjustify`）；新增竖排文本（`vertical_text`，从上到下、从右到左，配套 `use_font_height` 让所有字等高、`ascii_rotate_90` 把字母数字顺时针旋转 90 度）；支持行（列）间距倍数与附加量（`spacing_mul`/`spacing_add`）与字间距（`word_spacing`）；另有 `end_ellipsis`/`path_ellipsis` 省略号策略、`auto_tooltip`（仅在文本被省略时显示 ToolTip）、`rich_text` 富文本模式与 `text_id` 多语言切换 |
| 21 | Control 控件支持全屏显示（通过新增的 `Window::SetFullscreenControl` 实现），CEF 控件和 WebView2 控件支持 F11 切换页面全屏 |
| 22 | 完善控件动画细节并引入缓动函数，支持配置缓动函数类型、动画总时长与播放间隔等属性 |
| 23 | 统一上下文菜单 API：`Control::AttachPopupMenu` 一行代码即可把右键菜单绑定到任意控件（自动创建菜单、坐标换算、菜单项回调，支持菜单弹出后动态调整），配套 `DetachPopupMenu` 解除绑定 |

## 3. 新增控件/容器（50+ 项）

以下 50+ 项在 NIM_Duilib_Framework 中不存在，是迁移时最直接的"能力增量"。属性、事件与用法以 [Control.md](Control.md) 为准。

| 序号 | 名称 | 说明 |
| :--- | :--- | :--- |
| 1 | GroupBox | 分组框容器（GroupBox/<wbr/>GroupVBox/<wbr/>GroupHBox 为同一模板实现，分别继承 Label/<wbr/>VBox/<wbr/>HBox）：绘制圆角边框并在左上角显示分组标题，`corner_size`/<wbr/>`line_width`/<wbr/>`line_color` 控制边框圆角、线宽与颜色，`text`/<wbr/>`text_id` 为分组标题；适合把一组相关控件在视觉上归为一块（如设置面板、条件筛选区） |
| 2 | HotKey | 热键输入控件（继承 HBox）：获得焦点后按下组合键即记录热键，`SetHotKey`/<wbr/>`GetHotKey` 以虚拟键码 + 修饰键（或 `MAKEWORD` 组合值）读写，`GetHotKeyName`/<wbr/>`SetHotKeyName`/<wbr/>`GetKeyName` 负责键值与 `Ctrl + C` 形式显示名的互转，未设置时显示 `default_text`/<wbr/>`default_text_id`；用于快捷键设置界面 |
| 3 | HyperLink | 超链接文字（继承 Label）：`url` 指定链接地址（`SetUrl`/<wbr/>`GetUrl`），`show_url_tooltip` 控制是否把 URL 显示为 ToolTip；点击时派发 `kEventLinkClick`（`AttachLinkClick`），URL 为空时可当作普通文字按钮使用 |
| 4 | IPAddress | IP 地址输入控件（继承 HBox）：由 4 段编辑框组成，按 `192.168.0.1` 形式编辑并在各段之间自动跳转焦点，`ip` 属性或 `SetIPAddress`/<wbr/>`GetIPAddress` 读写整串地址；用于网络配置类界面 |
| 5 | Line | 画线控件（继承 Control）：`vertical` 选择横线/<wbr/>竖线，`line_color`/<wbr/>`line_width` 设定颜色与宽度，`dash_style` 支持 `solid`、`dash`、`dot`、`dash_dot`、`dash_dot_dot` 五种线型；用于区域分隔或装饰线 |
| 6 | RichText | 富文本显示控件（继承 Control，RichTextBox/<wbr/>RichTextHBox/<wbr/>RichTextVBox 为同模板变体）：支持类 HTML 标签 `<a href>`、`<b>`、`<i>`、`<s>`、`<u>`、`<bgcolor>`、`<font face size color>`、`<br/>`；`text`/<wbr/>`text_id` 中可用 `{` `}` 代替尖括号（`replace_brace`），并可配置对齐、内边距、行距倍数/<wbr/>附加量、自动换行、链接三态颜色与下划线、空格裁剪策略 |
| 7 | Split | 分割条控件/<wbr/>容器（继承 Control，SplitBox 继承 Box）：放在 HBox 中可左右拖动、放在 VBox 中可上下拖动，拖动时改变相邻两个控件的宽度/<wbr/>高度；`enable_split_single` 控制只有一侧有控件时是否仍可拖动。两侧控件都设为 stretch 时分割条无效 |
| 8 | TabCtrl | 多标签控件（继承 ListBox，TabCtrlItem 为标签项）：浏览器式标签，支持图标、关闭按钮、固定标签（`fixed`：不可拖动、不可关闭、始终排在最前）与内置角标（`badge_class`/<wbr/>`badge_count`/<wbr/>`badge_max_count`/<wbr/>`badge_dot`）；`tab_box_name` 与 TabBox 联动切换页面，`drag_order`/<wbr/>`drag_out_id`/<wbr/>`drop_in_id` 支持标签拖动排序与跨容器拖动，`tab_ctrl_orientation` 支持水平/<wbr/>垂直标签栏，`overflow_scroll` 在标签超出可视区时显示悬浮滚动条 |
| 9 | ListCtrl | 列表/<wbr/>报表控件（继承 VBox，Report/<wbr/>Icon/<wbr/>List 三种视图，视图皮肤在 global.xml 中定义）：Report 视图面向报表场景，支持表头拖动换序（`enable_header_drag_order`）、拖动或双击分割条调整列宽、冻结前 N 列（`frozen_column_count`）、行/<wbr/>单元格两种选择模式（`selection_mode`）、表头与行首复选框（`show_header_checkbox`/<wbr/>`show_data_item_checkbox`/<wbr/>`auto_check_select`）、单元格图标/<wbr/>进度条/<wbr/>富文本/<wbr/>单行或自动换行、按列编辑器类型（文本/<wbr/>数值/<wbr/>日期/<wbr/>下拉候选）与列级排序比较模式（文本/<wbr/>数值/<wbr/>日期/<wbr/>进度）；另支持单格刷新、列显隐、列宽/<wbr/>列序/<wbr/>显隐/<wbr/>排序事件、TSV 复制（`Ctrl+C`）与 CSV 导出（`Ctrl+Shift+C` 或 `ExportToCsvFile`）、批量删除行与 `CompactStorage()` 存储压缩 |
| 10 | PropertyGrid | 属性表控件（继承 VBox）：按分组展示"属性名 /<wbr/> 属性值"，值编辑器覆盖文本、数字、下拉框、字体名称、字号、颜色、日期时间、IP 地址、热键、文件路径、文件夹，属性名支持多语言；`property_font_normal`/<wbr/>`property_font_modified` 可区分未修改/<wbr/>已修改的值，布局由 `property_grid_xml` 指定的皮肤文件驱动 |
| 11 | ColorPicker | 拾色器（独立窗口，`WindowImplBase` 派生）：包含标准色、灰色标准色、自定义颜色与屏幕取色（吸管）等子控件，`SetSelectedColor`/<wbr/>`GetSelectedColor` 读写颜色，可订阅选择颜色与窗口关闭事件，`GetContrastTextColor` 按背景色求对比度最佳的黑/<wbr/>白前景色；子控件 ColorPickerRegular/<wbr/>Standard/<wbr/>StandardGray/<wbr/>Custom 也可单独嵌入使用 |
| 12 | ComboButton | 带下拉组合框的按钮（继承 Box）：左侧为主按钮（可上下两行文字，分别由 `left_button_top_label_*`/<wbr/>`left_button_bottom_label_*` 配置文字、颜色与 Class），右侧按钮点击弹出下拉组合框；`dropbox_size`/<wbr/>`popup_top` 控制下拉尺寸与弹出方向，`shadow_type` 设置下拉窗口阴影 |
| 13 | DirectoryTree | 目录树控件（继承 TreeView）：展示文件系统目录、磁盘与虚拟目录，目录枚举在子线程执行避免界面卡死；支持 `show_hiden_files`/<wbr/>`show_system_files`、`small_icon_size`/<wbr/>`large_icon_size`、按路径选中并逐级展开（`SelectPath`，异步）、与文件系统同步刷新（异步）及刷新完成回调，以及节点关联数据的自定义回调 |
| 14 | AddressBar | 地址栏控件（继承 HBox）：把本地路径按分隔符拆成可点击的层级按钮（根路径、子路径、分隔符各有独立 Class），编辑框支持回车/<wbr/>Esc/<wbr/>失焦自动更新显示（`return_update_ui`/<wbr/>`esc_update_ui`/<wbr/>`kill_focus_update_ui`）；`address_path` 设置当前路径，`path_tooltip` 控制路径 ToolTip，并提供 `AttachPathChanged`/<wbr/>`AttachPathClick`/<wbr/>`AttachReturn`/<wbr/>`AttachEsc` 事件 |
| 15 | WebView2Control | WebView2 浏览器控件（继承 Control）：`url` 指定初始地址（可为本地文件，相对路径以程序目录为根），支持 F11 页面全屏、F12 与 `devtools_enabled` 开发者工具、网站 Favicon 下载；封装了导航/<wbr/>后退/<wbr/>前进/<wbr/>刷新/<wbr/>停止、`ExecuteScript`、`PostWebMessageAsJson`/<wbr/>`PostWebMessageAsString`、User-Agent 与缩放因子，以及导航状态/<wbr/>标题/<wbr/>URL/<wbr/>新窗口/<wbr/>历史/<wbr/>缩放/<wbr/>图标等回调（接口见 `duilib/WebView2/WebView2Control.h`） |
| 16 | GridBox/<wbr/>GridScrollBox | 网格布局容器（继承 Box /<wbr/> ScrollBox）：`rows`/<wbr/>`columns`/<wbr/>`grid_width`/<wbr/>`grid_height` 定义网格（0 表示自动计算），子控件可用 `row_span`/<wbr/>`col_span` 跨行跨列合并单元格；`scale_down` 控制内容超出单元格时是否按比例缩小；GridScrollBox 额外带滚动条 |
| 17 | HFlowBox/<wbr/>VFlowBox/<br/>HFlowScrollBox/<br/>VFlowScrollBox | 流式布局容器：HFlowBox/<wbr/>VFlowBox 让子控件沿水平/<wbr/>垂直方向依次排列，超出宽度/<wbr/>高度时自动换行（换列）；HFlowScrollBox/<wbr/>VFlowScrollBox 在此之上带滚动条。支持 `child_halign`/<wbr/>`child_valign` 与 `justify`（`start`/<wbr/>`center`/<wbr/>`end`/<wbr/>`space-between`/<wbr/>`space-around`/<wbr/>`space-evenly`）控制行内/<wbr/>列内剩余空间分布，适合标签云、按钮墙等自适应排版（见 [Box.md](Box.md)） |
| 18 | MenuBar | 菜单栏控件（继承 HBox）：用 `AddTopMenu`/<wbr/>`InsertTopMenu`/<wbr/>`RemoveTopMenu`/<wbr/>`GetTopMenu` 管理顶级菜单，每个菜单对应一个 Menu 窗口的 XML；菜单文本支持多语言 ID，并可自定义按钮 Class 与附加属性；`SetActiveTopMenuIndex` 控制展开项，`AttachMenuBarItemActivated` 回调提供 menuItemId、menuName、menuLevel、itemName、itemIndex |
| 19 | IconControl/<wbr/>BitmapControl | 内存位图显示控件：IconControl（继承 Control）用 `SetIconData` 直接显示 32 位 BGRA/<wbr/>RGBA 像素数据（适合小图标，仅限 UI 线程使用），`ClearIconData`/<wbr/>`HasIconData` 管理数据；BitmapControl（继承 Box）支持目标/<wbr/>源矩形、外边距、水平/<wbr/>垂直对齐、透明度、等比适应（`bitmap_adaptive_dest_rect`）或拉伸（`bitmap_stretch`）以及多线程位图操作 |
| 20 | ChildWindow | 子窗口控件（继承 Box）；Windows 平台为系统原生子窗口（`WS_CHILD`），其他平台为 SDL 弹出式窗口（SDL 不支持原生子窗口）；`child_window_margin` 设置子窗口外边距，外边距区域可用于放置其他控件 |
| 21 | ControlDragableT | 模板类，包含 ControlDragable/<wbr/>BoxDragable/<wbr/>HBoxDragable/<wbr/>VBoxDragable 四个标准控件：支持在相同 Box 内拖动调整子控件顺序，也支持跨 Box 拖动改变控件所属容器；`drag_alpha` 控制拖动时的透明度，`drag_out` 控制是否允许跨容器拖出 |
| 22 | ControlMovableT | 模板类，包含 ControlMovable/<wbr/>BoxMovable/<wbr/>HBoxMovable/<wbr/>VBoxMovable 四个标准控件：支持鼠标拖动调整控件位置，也支持拖动父容器位置 |
| 23 | ControlResizableT | 模板类，包含 ControlResizable/<wbr/>BoxResizable/<wbr/>HBoxResizable/<wbr/>VBoxResizable 四个标准控件：支持鼠标拖动调整控件大小，效果与调整窗口大小类似 |
| 24 | XmlBox | XML 预览容器（继承 Box）：`xml_file_path` 指定要加载的界面 XML，`res_path` 指定该 XML 使用的图片资源根目录；加载后在容器内直接渲染 XML 定义的控件，用于界面预览（示例 [examples/XmlPreview](../examples/XmlPreview)） |
| 25 | Pane/<wbr/>PanelHBox/<wbr/>PanelVBox | 带标题栏的面板容器（同一模板 `PanelTemplate` 的三个实例，分别继承 Box/<wbr/>HBox/<wbr/>VBox）：标题栏支持 `title`/<wbr/>`title_id`（多语言，切换语言自动刷新）、高度/<wbr/>背景色/<wbr/>文字色/<wbr/>字体/<wbr/>对齐，以及自定义箭头图片；`collapsible` 开启折叠，`collapse_trigger` 选择点击标题栏或仅箭头触发，`collapse_anim` 控制动画时长、`collapsed` 设初始状态、`group` 支持手风琴分组（同组同时只展开一个）、`title_slot` 可在标题栏放置自定义控件；折叠/<wbr/>展开派发 `kEventCollapse`/<wbr/>`kEventExpand`，动作前派发 `kEventPanelCollapsing`/<wbr/>`kEventPanelExpanding`（回调返回 false 可取消） |
| 26 | MessageBoxWnd | 自绘皮肤的模态消息框（`duilib/Utils/MessageBoxWnd.h`）：静态 `Show()` 弹出，按钮支持确定/<wbr/>取消/<wbr/>是/<wbr/>否/<wbr/>重试按位组合（常用组合 `kButtonsOK`/<wbr/>`kButtonsOKCancel`/<wbr/>`kButtonsYesNo`/<wbr/>`kButtonsYesNoCancel`/<wbr/>`kButtonsRetryCancel`），图标支持无/<wbr/>信息/<wbr/>警告/<wbr/>错误/<wbr/>疑问，可自定义按钮文字与多语言文本；`DoModal` 返回值与 Win32 的 `IDOK`/<wbr/>`IDCANCEL`/<wbr/>`IDRETRY`/<wbr/>`IDYES`/<wbr/>`IDNO` 保持一致 |
| 27 | ToastWnd | 自绘皮肤的非模态通知窗口（`duilib/Utils/ToastWnd.h`）：静态 `Show()` 即发即忘，支持信息/<wbr/>成功/<wbr/>警告/<wbr/>错误四种图标与顶部/<wbr/>居中/<wbr/>底部/<wbr/>右上/<wbr/>右下五种位置，默认 3000ms 自动关闭（传 0 表示不自动关闭），带淡入淡出与上移动画；多条通知从锚点垂直堆叠、关闭后平滑补齐，鼠标悬停暂停倒计时、移开继续、点击立即关闭；文本支持多语言 ID，可带"撤销/<wbr/>查看详情"等操作按钮 |
| 28 | SearchBox | 搜索框组合控件（继承 HBox）：左侧搜索图标 + RichEdit 编辑框 + 右侧清除按钮；`text`/<wbr/>`text_id` 为初始文本，`prompt_text`/<wbr/>`prompt_text_id`/<wbr/>`prompt_color` 为空态提示，`debounce_interval` 让 `kEventTextChanged` 只在输入停顿后触发；回车派发 `kEventReturn`（用于执行搜索），并内置候选项自动完成（`SetCandidates`/<wbr/>`SetFilterEnabled`/<wbr/>`SetMatchMode`/<wbr/>`SetMinChars`/<wbr/>`SetIgnoreCase`，选中派发 `kEventAutoCompleteSelect`）与搜索历史（`SetHistoryEnabled`/<wbr/>`SetMaxHistory`） |
| 29 | SpinBox | 数字输入框（继承 RichEdit，默认开启 `number_only`）：支持步长（`step`）、范围（`min_number`/<wbr/>`max_number`）、小数位（`decimals`）、前后缀与千分位分组（`prefix`/<wbr/>`suffix`/<wbr/>`group_separator`）；设置 `spin_class` 后显示步进按钮，否则可用上下方向键调整；失焦时按当前格式重排文本，无法解析的输入恢复为最近一次有效值，值变化派发 `kEventTextChanged` |
| 30 | Switch | 滑块开关（继承 CheckBox）：轨道与滑块既可用语义色自绘，也可配置 SVG 图片皮肤（按动画进度交叉淡入淡出、滑块沿轨道平移），`switch_animation_ms` 控制切换动画；支持轨道内文字/<wbr/>图标（`switch_text_on`/<wbr/>`switch_text_off`、`switch_image_on`/<wbr/>`switch_image_off`）、加载中态（`switch_loading`，滑块内绘制旋转指示器并屏蔽点击）与禁用态配色；选中状态变化派发 `kEventSelect`/<wbr/>`kEventUnSelect` |
| 31 | Badge | 角标控件（继承 Label）：数字角标（圆角胶囊 + 数字，超过 `max_count` 显示"99+"，`count<=0` 自动隐藏）与红点角标（`dot`，显隐由 `visible` 控制）两种形态；背景走语义色 `bg_badge` 并自动适配深浅色主题，宽度随数字自适应，可用 `float="true"` + `margin` 悬浮在宿主控件角上；不响应鼠标，不挡住宿主控件的点击 |
| 32 | Flyout | 通用浮层窗口（`WindowImplBase` 派生）：在锚点控件周围浮出任意 Box 内容（操作面板、确认卡片、富内容提示等），也是 CalendarFlyout、AutoComplete 等浮层的公共基座；支持 12 个方位（下/<wbr/>上/<wbr/>左/<wbr/>右 × 左对齐/<wbr/>右对齐/<wbr/>居中），空间不足自动翻转并夹持在显示器工作区；默认不抢焦点、点击外部或 Esc 关闭、同一时刻只保留一个活动浮层，`SetFollowAnchor` 可跟随滚动容器内的锚点并在锚点不可见时暂时隐藏；提供 `ShowAt`/<wbr/>`Dismiss`/<wbr/>`IsOpen`/<wbr/>`GetVisualRect`/<wbr/>`AttachOpened`/<wbr/>`AttachClosed`，关闭原因区分 `kManual`/<wbr/>`kClickOutside`/<wbr/>`kEscape`/<wbr/>`kAnchorLost` |
| 33 | Calendar | 日历控件（继承 Control）：支持单选（`calendar_mode="single"`）与范围选择（`range`）、每周起始日（`first_day_of_week`）、`min_date`/<wbr/>`max_date` 以及业务禁用日期（`disabled_dates`，支持区间与动态 API），提供月/<wbr/>年/<wbr/>十年三级视图；另有 `CalendarFlyout` 日期选择浮层（可带时间选择），并支持完整的键盘导航与视图模式/<wbr/>显示周期/<wbr/>值变化事件 |
| 34 | NavigationView | 侧边栏导航容器（继承 HBox）：左侧导航窗格 + 右侧内容区，窗格可在展开宽度（`pane_width`）与图标条收起宽度（`compact_pane_width`）之间切换；支持导航项分组（header）/<wbr/>分隔线（separator）、底部内置"设置"项、页头随选中项同步、多语言标题与图标，收起时悬停显示 tooltip；导航项通过 `page` 与 TabBox 内容页联动，可用 `AddNavItem`/<wbr/>`AddPage`/<wbr/>`RemoveNavItem` 等接口动态管理，并派发 `AttachNavItemClick`/<wbr/>`AttachNavSelectionChanged`/<wbr/>`AttachNavPaneToggling`/<wbr/>`AttachNavPaneToggled` 事件 |
| 35 | Chart | 轻量图表控件（继承 Control，纯自绘、不依赖第三方图表库）：支持折线（`line`，含平滑曲线、面积填充、发光、滚轮缩放与拖拽平移）、柱状（`bar`，分组/<wbr/>堆叠、负值零基线、圆角与渐变）与饼图（`pie`，环形、百分比、扇区分离高亮）三种形态；数据可由 XML `data`/<wbr/>`data_labels` 或 C++ `SetSeriesData`/<wbr/>`AddSeries` 多系列绑定，支持增量更新（`AddDataPoint` 等）与滚动窗口 `SetMaxDataPoints`、坐标轴标签格式化回调、悬停 Tooltip、点击选中、图例/<wbr/>网格/<wbr/>数值标签开关、标题与多语言 ID，颜色走语义色自动适配深浅色主题 |
| 36 | Marquee | 文本跑马灯（继承 Label，复用全部文本属性）：文本超出显示区域时按 `scroll_direction`（left/<wbr/>right/<wbr/>up/<wbr/>down）无缝循环滚动，未超出时按 `text_align` 静态显示且不占用定时器；`scroll_speed`（像素/<wbr/>秒）与 `scroll_interval`（帧间隔）控制滚动，`hover_pause` 悬停暂停，`Pause`/<wbr/>`Resume`/<wbr/>`SetPaused` 可手动暂停；`text_id` 随语言切换生效，更新文本后自动重置滚动位置 |
| 37 | Timeline | 垂直时间轴控件（继承 Control，纯自绘、不创建子控件）：左侧为节点与连接线，右侧为时间/<wbr/>标题/<wbr/>正文，完全由数据驱动渲染；XML 中 `item_time`/<wbr/>`item_title`/<wbr/>`item_content`/<wbr/>`item_status` 用 `\|` 分隔多条数据（`item_time_id`/<wbr/>`item_title_id`/<wbr/>`item_content_id` 为多语言版本，语言切换自动刷新），也可用 `AddItem`/<wbr/>`InsertItem`/<wbr/>`SetItems` 动态驱动；节点四态（pending/<wbr/>inprogress/<wbr/>completed/<wbr/>failed）颜色由 `node_color_pending`/<wbr/>`node_color_inprogress`/<wbr/>`node_color_completed`/<wbr/>`node_color_failed` 配置，`line_style` 选实线/<wbr/>虚线、`reverse` 倒序、`node_size`/<wbr/>`item_spacing` 调节节点直径与条目间距；内容超出高度时滚轮滚动并显示右侧滚动条（`auto_hide_scrollbar` 控制是否仅悬停时显示），`empty_text`/<wbr/>`loading` 提供空数据提示与加载态；点击节点派发 `kEventTimelineItemClick`（`wParam` 为条目序号），`SetItemDrawCallback` 可完全重写单条条目的绘制；适合日志记录、任务流、审批流等场景 |
| 38 | TagInput | 标签输入框组合控件（继承 Box）：内部用水平流式布局把"标签（文本 + 关闭按钮）"与一个 RichEdit 编辑框组合在一起，输入后按回车（或 `delimiter` 指定的字符）生成标签，点标签右侧关闭按钮或在空编辑框按 Backspace 删除标签，标签过多自动换行、控件高度自动增长；`tags` 以 `;` 分隔设置初始标签，`prompt_text`/<wbr/>`prompt_text_id`/<wbr/>`prompt_color` 为空态提示，`allow_duplicate` 控制是否允许重复文本，`max_count` 限制标签个数（0 为不限）；接口 `AddTag`/<wbr/>`RemoveTagAt`/<wbr/>`RemoveAllTags`/<wbr/>`GetTagCount`/<wbr/>`GetTagText`/<wbr/>`GetEditControl`/<wbr/>`SetFocus`；事件 `kEventTagAdded`/<wbr/>`kEventTagRemoved`/<wbr/>`kEventTagClick`（`wParam` 为标签序号）与内部编辑框转发的 `kEventTextChanged`；标签底色走语义色 `bg_tag`/<wbr/>`bg_tag_hover`；适合收件人、标签选择、关键词录入等场景 |
| 39 | AutoComplete | 自动完成输入框组合控件（继承 Box）：内部为一个占满控件的单行编辑框（RichEdit），输入达到 `min_chars` 个字符后在编辑框下方弹出不抢焦点的候选浮层（`AutoCompleteFlyout`，复用 Flyout，宽度与编辑框一致、不占布局空间、高度随候选项数量自适应）；`items` 以 `;` 分隔提供初始候选，或由 `SetCandidates`/<wbr/>`AddCandidate`/<wbr/>`ClearCandidates` 动态维护，`match_mode` 选 `prefix`（前缀）/<wbr/>`contains`（包含）匹配、`ignore_case` 控制是否忽略大小写、`max_visible` 限制可见行数（超出显示滚动条）；支持上下方向键移动、回车确认、Esc 关闭、鼠标悬停高亮与点击确认，确认后回填编辑框且程序化回填不再触发联想；选中派发 `kEventAutoCompleteSelect`（`wParam` 为候选序号），文本变化转发 `kEventTextChanged`；与 FilterCombo 的区别是"以编辑框为主体、边输入边过滤、浮层跟随编辑框"，适合搜索历史、城市/<wbr/>地址补全等场景 |
| 40 | Mask | 遮罩/<wbr/>蒙版层容器（继承 Box）：铺在其它控件之上形成半透明蒙层，覆盖模态对话框背景、新手引导蒙层、加载遮罩三类场景；`mask_color` 指定蒙层颜色（为空则不绘制，默认走语义色 `bg_overlay`），`highlight_control` 按控件名在整个窗口内"镂空"高亮目标控件（`highlight_padding`/<wbr/>`highlight_corner` 控制镂空区域内边距与圆角），`click_through` 让镂空区域内的点击穿透到目标控件；继承的 `mouse_enabled` 决定是否拦截鼠标（默认拦截＝模态遮罩，设为 false 则仅视觉遮挡）；本身是容器，可在蒙层上再放加载动画、提示文案、按钮等任意子控件，作为浮层使用时建议 `float` + 满宽满高并置于父容器最后一个子节点；静态接口 `Mask::ShowOverWindow` 可一键在整个窗口上覆盖遮罩，`Remove()` 移除遮罩 |
| 41 | StatusBar | 状态栏控件（继承 HBox）：每个子控件就是一个状态面板（通常为 Label/<wbr/>IconControl/<wbr/>Progress），沿用 HBox 的固定宽度/<wbr/>拉伸等布局能力，相邻面板之间自动绘制竖直分隔线（`separator_width` 设 0 可关闭，`separator_color` 调色），右下角可选尺寸手柄（`size_grip` 开关、`size_grip_resize` 控制手柄是否参与窗口拖拽缩放、`size_grip_color` 配色）；面板支持按索引访问且只统计可见面板（`GetPanelCount`/<wbr/>`GetPanel`/<wbr/>`GetPanelIndex`/<wbr/>`FindPanel`/<wbr/>`SetPanelText`/<wbr/>`GetPanelText`），业务侧无需 `FindControl` + `dynamic_cast`；适合编辑器/<wbr/>IDE/<wbr/>通信类软件显示光标位置、字数、连接状态、后台进度等提示 |
| 42 | ToolBar | 工具条控件（继承 HBox）：承载一排工具按钮与分隔线（分隔线可直接用 `Line` 控件），可用宽度不足时自动隐藏尾部子控件并在右端显示"更多"按钮（`overflow` 开关，`overflow_button_class`/<wbr/>`overflow_button_width`/<wbr/>`overflow_button_text` 定制外观），点击弹出浮层列出被收纳项、点击项回调 `AttachOverflowItemActivated`，窗口宽度变化时自动重新计算、宽度足够后被收纳项恢复显示；提供工具项管理接口（`AddToolButton`/<wbr/>`AddToolCheckBox`/<wbr/>`AddToolSeparator`/<wbr/>`AddTool`/<wbr/>`InsertTool`/<wbr/>`RemoveTool`/<wbr/>`FindTool`/<wbr/>`GetToolAt`/<wbr/>`GetToolCount`/<wbr/>`SetToolChecked`/<wbr/>`SetToolVisible`），默认 Class 由 `tool_class`/<wbr/>`tool_checkbox_class`/<wbr/>`tool_separator_class` 指定，勾选类工具项用 CheckBox 的选中态表示；工具项点击与勾选变化分别回调 `AttachToolItemClick`/<wbr/>`AttachToolItemCheckedChanged`（参数为工具项 `name`） |
| 43 | RangeSlider | 双滑块范围选择控件（继承 Progress，复用 `min`/<wbr/>`max`）：在一条轨道上放置下限与上限两个滑块并高亮选中区间，用于价格/<wbr/>时间等区间筛选；下限为 `low_value`（`GetLowValue`/<wbr/>`SetLowValue`），上限复用基类进度值（`high_value`/<wbr/>`SetHighValue`），`min_range` 约束两滑块最小间距，`included` 可改为高亮滑块外侧的两段区间，`click_to_jump` 控制点击轨道是否按下即跳值，`range_color`/<wbr/>`range_radius` 设置高亮区间颜色与圆角；`marks` 支持刻度与标签（`value=label@color` 语法）并可用 `snap_to_marks` 吸附，`thumb_size` 与各状态滑块图片（`thumb_normal_image`/<wbr/>`high_thumb_normal_image` 等）控制滑块外观，`tooltip_open`/<wbr/>`tooltip_format` 控制值气泡常显与格式；范围变化派发 `AttachRangeChanged`，`pEventData` 携带 `RangeValue` 的 `lowValue`/<wbr/>`highValue` |
| 44 | Stepper | 向导步骤条控件（继承 Control）：节点显示序号或对勾，状态由当前索引与禁用集合派生（待开始/<wbr/>进行中/<wbr/>已完成/<wbr/>禁用），已完成段连接线额外着色；`step_title`/<wbr/>`step_desc` 用 `\|` 分隔各步骤标题与描述（`step_title_id`/<wbr/>`step_desc_id` 支持多语言），`current` 指定当前步骤，`direction` 可选 `horizontal`/<wbr/>`vertical`，`click_to_jump` 控制点击跳转，`disabled_steps` 指定禁用步骤，`node_size` 调节节点直径，`node_color_pending`/<wbr/>`node_color_active`/<wbr/>`node_color_completed`/<wbr/>`node_color_disabled`、`connector_color`/<wbr/>`connector_active_color`、`title_color_active`/<wbr/>`title_color_completed`/<wbr/>`title_color_pending`、`desc_color` 控制各状态配色，`node_hovered_image`/<wbr/>`node_pressed_image` 提供悬停/<wbr/>按下光晕；接口 `AddStep`/<wbr/>`SetSteps`/<wbr/>`SetCurrentIndex`/<wbr/>`Next`/<wbr/>`Back`/<wbr/>`GoTo`/<wbr/>`SetStepDisabled`，步骤变化派发 `AttachStepChanged`（`pEventData` 携带 `StepValue` 的 `currentIndex`/<wbr/>`oldIndex`）；用于安装/<wbr/>注册/<wbr/>配置向导与审批流程 |
| 45 | Rating | 星级评分控件（继承 Control）：支持只读展示（`read_only`）、半星（`allow_half`）、鼠标悬停预览与点击/<wbr/>拖拽打分；`value` 钳位到 `[0, max]` 并对齐步长，`max`（别名 `star_count`）为星星数量，`star_size`/<wbr/>`star_gap` 控制单星直径与间距；星形外观完全由 SVG 图片驱动（`star_filled_image`/<wbr/>`star_half_image`/<wbr/>`star_empty_image`，可选 `star_hovered_image`/<wbr/>`star_pressed_image` 在星形下层绘制外圈光晕），颜色经 `svg_replace_colors` 映射语义色（实心/<wbr/>半星走 `color_accent`、空心走 `border_control_normal`），切换主题自动跟随；接口 `SetValue`/<wbr/>`GetValue`/<wbr/>`SetMax`/<wbr/>`SetStarSize`/<wbr/>`SetStarImage`，值变化派发 `AttachRatingChanged`（`pEventData` 携带 `RatingValue` 的 `value`/<wbr/>`oldValue`）；用于评价/<wbr/>打分/<wbr/>满意度调查 |
| 46 | Carousel | 图片轮播控件（继承 Control）：同一时刻显示一张幻灯片，用于首页横幅、广告位、图片集；`item_image` 用 `\|` 分隔多张幻灯片图片（也可用 `AddItem`/<wbr/>`InsertItem`/<wbr/>`SetItems` 动态管理），`current_index` 为当前索引，`loop` 无限循环、`auto_play`+`interval`+`hover_pause` 控制自动播放与悬停暂停、`transition_ms` 设置切换动画时长；`show_indicator`/<wbr/>`indicator_size`/<wbr/>`indicator_gap`/<wbr/>`indicator_bottom` 与 `show_nav_button`/<wbr/>`btn_size`/<wbr/>`btn_margin` 控制底部页码指示点与左右切换按钮（图片均经 `svg_replace_colors` 跟随主题语义色），并支持按住鼠标左右拖拽翻页；接口 `Next`/<wbr/>`Prev`/<wbr/>`GoTo`/<wbr/>`GetItemCount`，索引变化派发 `AttachCarouselChanged`（`pEventData` 携带 `CarouselValue` 的 `index`/<wbr/>`oldIndex`） |
| 47 | Pagination | 分页器控件（继承 HBox，页码按钮由控件内部按数据动态创建）：上一页/<wbr/>下一页（可选首页/<wbr/>末页）+ 页码按钮，当前页呈选中态，页码超出 `max_visible_count` 时以"…"收纳并始终保留首尾与当前页附近；`page_count`（别名 `total_page`）/<wbr/>`current_page`（别名 `page`）读写总页数与当前页，也可用 `total_count`+`page_size` 推导页数（`SetTotalCount`），`show_prev_next`/<wbr/>`show_first_last` 控制按钮显隐，`button_class`/<wbr/>`button_width`/<wbr/>`button_height` 控制页码按钮外观（走语义色）；可选的"每页条数"选择器由 `show_page_size` 开启（默认关闭），`page_size_options`/<wbr/>`page_size_format`/<wbr/>`page_size_class`/<wbr/>`page_size_width` 配置候选项与文本，切换后按 `total_count` 重算页数并重置到第 1 页；页码或每页条数变化派发 `AttachPageChanged`（`pEventData` 携带 `PaginationValue` 的 `page`/<wbr/>`oldPage`/<wbr/>`pageSize`/<wbr/>`oldPageSize`）；用于报表/<wbr/>表格/<wbr/>列表的页码导航 |
| 48 | TreeGrid | 树表格控件（继承 VBox）：在同一控件内呈现"可展开/<wbr/>收起的层级行 + 多列表格"，表头固定不滚动、列表体自带纵向滚动条，表头与数据行复用同一列宽数组而天然逐列对齐（缩进通过树列单元格内嵌占位实现，其它列不会随层级错位）；`column_title`/<wbr/>`column_width`/<wbr/>`column_id`/<wbr/>`column_align` 用 `\|` 分隔配置列标题（支持多语言 ID）、列宽、列 ID 与对齐，`tree_column`/<wbr/>`indent`/<wbr/>`header_height`/<wbr/>`row_height`/<wbr/>`show_header` 调整树列与尺寸，`multi_select` 多选、`check_mode` 行首复选框配合 `check_cascade` 实现父子三态级联；接口 `AddNode`/<wbr/>`SetCellText`/<wbr/>`SetNodeExpand`/<wbr/>`EnsureNodeVisible`/<wbr/>`SelectNode`/<wbr/>`SetNodeChecked`/<wbr/>`GetCheckedNodes`，事件 `kEventExpand`/<wbr/>`kEventCollapse`/<wbr/>`kEventClick`/<wbr/>`kEventSelect`/<wbr/>`kEventUnSelect`/<wbr/>`kEventSelChanged`/<wbr/>`kEventTreeGridCheckChanged`（前六个的 `pEventData` 为 `TreeGridNode*`，勾选变化为 `TreeGridCheckValue`）；用于文件树、报表、组织架构 |
| 49 | Dialog | 通用模态对话框（`duilib/Utils/Dialog.h`）：窗口布局由类在运行时组装（标题栏 + 内容区 + 按钮区），不依赖额外的 XML 文件；`SetContentXml` 指定内容区的任意 XML 片段（容器与控件组合），`AddButton` 按调用顺序从左到右添加底部按钮并标记默认按钮（默认按钮获得初始焦点、按 Enter 即触发），`SetTitle`/<wbr/>`SetSize`/<wbr/>`SetTitleBarVisible`/<wbr/>`SetSkinFolder` 配置标题、尺寸、标题栏与资源目录，`SetTextIdMode` 让标题与按钮文字按多语言 ID 解析；`AttachInit` 用于打开后向内容区填充数据，`AttachButtonClick` 在关闭前回调（返回 false 可阻止关闭，便于读取内容区数据或校验输入）；`DoModal` 返回被点击按钮的索引（`GetClickedButtonId` 取按钮标识），ESC 或标题栏关闭返回 `kResultCancel`，创建失败返回 `kResultFailed` |
| 50 | SystemNotification | 系统原生通知（`duilib/Utils/SystemNotification.h`，与 ToastWnd 的应用内提示相互独立）：使用 SDL3 构建时通过 SDL 系统通知 API 发送到系统通知中心（Windows 10+/<wbr/>macOS/<wbr/>Linux/<wbr/>FreeBSD，macOS 需运行在 app 包内），不使用 SDL 的 Windows 构建通过 `Shell_NotifyIcon` 托盘气泡实现（兼容 Windows 7 及以上，显示期间临时创建托盘图标）；提供 `IsSupported`（平台是否支持）、`RequestPermission`（请求通知权限，部分平台首次显示前需要）、`Show`（返回通知 ID，失败返回 0，可指定 `SystemNotificationPriority` 的 `kLow`/<wbr/>`kNormal`/<wbr/>`kHigh`/<wbr/>`kCritical` 优先级与 Windows 原生实现的显示时长）、`Remove`（按通知 ID 移除）接口；最终显示样式与时长由系统策略决定，不提供通知点击回调（见 [SystemNotification.md](SystemNotification.md)） |
| 51 | Breadcrumb | 面包屑导航控件（继承 Control，纯自绘、不创建子控件，`duilib/Control/Breadcrumb.h`）：一行显示可点击的祖先路径 + 当前项，点击祖先项切换当前项并派发 `AttachCurrentChanged`（`pEventData` 携带 `BreadcrumbValue` 的 `currentIndex`/<wbr/>`oldIndex`）；`item_text`/<wbr/>`item_text_id` 用 `\|` 分隔条目（多语言切换自动刷新），`current` 指定当前项（-1 表示跟随最后一项），`separator`/<wbr/>`spacing`/<wbr/>`item_padding`/<wbr/>`item_round` 控制分隔符、间距、点击区与胶囊背景圆角，`click_to_jump`/<wbr/>`disabled_items` 控制可跳转与禁用项，`item_color`/<wbr/>`current_color`/<wbr/>`hover_bg_color`/<wbr/>`pressed_bg_color`/<wbr/>`font`/<wbr/>`current_font` 控制配色与字体（默认祖先项为链接蓝 `color_blue`、当前项加粗、分隔符为灰色）；宽度不足时右对齐裁剪以保证当前项可见；用于文件/<wbr/>目录路径、站点层级、设置页层级 |
| 52 | CascadingSelect | 级联选择器（继承 Control，纯自绘、不创建子控件，`duilib/Control/CascadingSelect.h`）：支持"内嵌多列"与"下拉列表框（`dropdown`）"两种形态，点击某列某行后子列立即过滤为该节点子集并重置深层旧选择，支持任意层级；`items`/<wbr/>`items_id` 用 `\|` 分隔多条路径（路径内层级由 `path_separator` 分隔，相同前缀自动合并，`items_id` 每段为语言 ID），`disabled_paths` 禁用节点及其整棵子树，`column_width`/<wbr/>`row_height`/<wbr/>`text_padding` 控制尺寸，`item_color`/<wbr/>`selected_color`/<wbr/>`hover_bg_color`/<wbr/>`selected_bg_color` 控制配色；下拉形态额外支持 `placeholder`/<wbr/>`placeholder_id` 提示文本、`allow_clear` 清空按钮、`expand_on_hover` 移入展开、`popup_width`/<wbr/>`popup_max_rows` 控制浮层尺寸；接口 `SetSelection`/<wbr/>`GetSelection`/<wbr/>`GetSelectionText`/<wbr/>`GetSelectionPath`/<wbr/>`SetDisabledPaths`/<wbr/>`AttachDisplayText`，选择变化派发 `AttachSelectionChanged`（`pEventData` 携带 `CascadingSelectValue` 的 `indexes`/<wbr/>`text`/<wbr/>`path`）；用于省市区等层级数据选择 |
| 53 | Popover | 气泡卡片（`duilib/Control/Popover.h`，对齐 Ant Design 的 Popover 语义）：**控件自身即锚点**，按 `click`/<wbr/>`hover`/<wbr/>`focus`/<wbr/>`manual` 四种触发方式在锚点周围弹出带标题与正文的卡片，正文也可用 `content_xml` 插入任意 XML 富内容；12 种弹出方位（上下左右 + 四角偏移）越界时自动翻转，卡片朝向锚点的一侧带尖角箭头与轻量自定义阴影；支持卡片内控件访问（`GetCardWindow`/<wbr/>`FindCardControl`）、点击外部/<wbr/>Esc 自动关闭与受控模式（`Show`/<wbr/>`Hide`/<wbr/>`Toggle`/<wbr/>`IsOpen` + 打开状态事件 `kEventPopoverOpenChanged`）、多语言标题/<wbr/>正文；锚点位于滚动容器内时卡片跟随锚点滚动并在锚点滚出可视区域时暂时隐藏、滚回后自动恢复（复用 Flyout 的跟随锚点能力） |
| 54 | Popconfirm | 确认气泡卡片（`duilib/Control/Popconfirm.h`，继承 Popover，对齐 Ant Design 的 Popconfirm 语义）：点击锚点弹出"图标 + 标题 + 描述 + 取消/<wbr/>确定"按钮行的卡片浮层；图标支持 `warning`/<wbr/>`info`/<wbr/>`success`/<wbr/>`error`/<wbr/>`none` 五种语义，"确定"按钮支持 `primary`/<wbr/>`default`/<wbr/>`danger` 三种外观，可隐藏取消按钮（`show_cancel`）、可保持打开（`close_on_ok="false"`），按钮文案默认取框架语言键 `STRID_PUBLIC_STRING_OK`/<wbr/>`STRID_PUBLIC_STRING_CANCEL`；点击确定/<wbr/>取消派发 `kEventPopconfirmConfirm`/<wbr/>`kEventPopconfirmCancel`，关闭原因扩展 `PopoverCloseReason::kConfirm`（4）/<wbr/>`kCancel`（5），因此 `kEventPopoverOpenChanged` 能区分"卡片按钮触发的关闭"；复用 Popover 的全部方位/<wbr/>箭头/<wbr/>滚动跟随能力，卡片内容由 `BuildCardBodyXml()` 钩子生成，需要富内容时可派生重写 |
| 55 | Avatar | 头像控件（`duilib/Control/Avatar.h`，对齐 Ant Design 的 Avatar 语义）：用圆形/<wbr/>圆角方形展示"图片 /<wbr/> 图标 /<wbr/> 文字"三类头像内容；`size` 支持 `small`(24)/<wbr/>`default`(32)/<wbr/>`large`(40) 预设与数值并随 DPI 缩放，`shape` 取 `circle`（默认，非正方形控件矩形时为正椭圆）或 `square`，`corner` 控制方形圆角；内容优先级 `image`（等比 cover 填充并按形状裁剪）> `icon`（等比 contain 居中，`icon_scale` 控制占比）> `text`/<wbr/>`text_id`，文字字号按控件短边的 `text_scale` 换算，超宽时先缩小字号再省略；配色优先级 `color` > `bkcolor` > 自动调色板（文字头像按文本哈希稳定取 `bg_avatar_1..6`）> `bg_avatar`，深浅色主题自动适配；`text_id` 随语言切换自动刷新，`ClearImageCache` 支持主题切换后重载图片与图标 |

## 4. 性能优化（3 项）

| 序号 | 优化内容 |
| :--- | :--- |
| 1 | 优化 Control 及子控件的内存占用，界面元素较多时内存占用有大幅降低 |
| 2 | 优化动画绘制流程，合并定时器的触发事件，避免播放控件动画或动画图片时界面卡顿 |
| 3 | 基于虚表的 ListBox 控件及关联控件：通过优化实现机制，使可用性和性能有较大改善 |

## 5. 示例程序（16 项）

示例程序既是使用说明，也是回归测试的入口；下列示例可直接对照本文其他章节查阅对应控件的用法。

| 序号 | 示例 | 说明 |
| :--- | :--- | :--- |
| 1 | [examples/ColorPicker](../examples/ColorPicker) | 新增拾色器示例程序 |
| 2 | [examples/ListCtrl](../examples/ListCtrl) | 新增列表示例程序，演示列表的各项功能 |
| 3 | [examples/render](../examples/render) | 新增渲染引擎示例程序，演示大部分容器、控件与资源管理功能 |
| 4 | [examples/TreeView](../examples/TreeView) | 新增树控件示例程序，演示树控件的各种功能 |
| 5 | [examples/RichEdit](../examples/RichEdit) | 新增富文本编辑控件示例程序，演示其各种功能 |
| 6 | [examples/MultiLang](../examples/MultiLang) | 多语言动态切换功能演示 |
| 7 | [examples/DpiAware](../examples/DpiAware) | DPI 感知功能演示 |
| 8 | [examples/threads](../examples/threads) | 多线程功能演示 |
| 9 | [examples/WebView2](../examples/WebView2) | WebView2 控件功能演示 |
| 10 | [examples/WebView2Browser](../examples/WebView2Browser) | WebView2 控件功能演示（多标签） |
| 11 | [examples/layout](../examples/layout) | 所有布局和容器的功能演示 |
| 12 | [examples/ChildWindow](../examples/ChildWindow) | 子窗口控件功能演示 |
| 13 | [examples/XmlPreview](../examples/XmlPreview) | XML 文件的界面效果预览功能测试（测试 XmlBox 容器） |
| 14 | [examples/ColorTheme](../examples/ColorTheme) | 颜色主题的功能预览与测试程序 |
| 15 | [examples/panel](../examples/panel) | 带标题栏的面板容器（Pane/PanelHBox/PanelVBox）功能演示，含折叠/展开动画、标题栏槽位按钮、折叠前取消（veto）与动态修改标题 |
| 16 | 其他示例程序 | 大部分进行了代码兼容性修改和优化，使示例程序也可以当作测试程序使用 |

## 6. 文档与授权（4 项）

| 序号 | 内容 |
| :--- | :--- |
| 1 | README.md 和 docs 子目录的文档重新梳理，便于读者理解界面库的功能与用法、更快上手 |
| 2 | 各控件的接口没有单独整理成文档，可直接阅读接口文件中的注释（目前各接口的注释较为完善） |
| 3 | 补充各平台的编译文档与依赖的编译脚本 |
| 4 | 重新整理主体项目与依赖第三方源码的授权文件，统一放在 `licenses` 目录中管理 |

## 7. 相关链接

1. 本项目在 [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/) 基础上继续开发
2. NIM_Duilib_Framework 基于 [duilib](https://github.com/duilib/duilib) 开发

## 8. 延伸阅读

- 完整变更记录：[CHANGELOG.md](../CHANGELOG.md)
- 控件属性、事件与用法：[Control.md](Control.md)
- 布局与容器：[Box.md](Box.md)
- 示例程序索引：[Examples.md](Examples.md)
