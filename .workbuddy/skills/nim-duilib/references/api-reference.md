# nim_duilib 完整 API 速查

仅在需要查具体 API / 属性 / 容器定义时载入本文。日常任务优先读对应的专题文档。

---

## 一、XML 布局结构

### 窗口模板

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="800,600" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="../public/caption/logo.ico">
  <VBox bkcolor="bg_window_main">
    <!-- 标题栏（name 必须是 window_title_bar；旧名 window_caption_bar 仅为兼容 fallback） -->
    <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
      <Control />
      <Button class="btn_wnd_min_11" name="btn_window_min" height="32" width="40" margin="0,2,0,2"/>
      <Box height="stretch" width="40" margin="0,2,0,2">
        <Button class="btn_wnd_max_11" name="btn_window_max" height="32" width="stretch"/>
        <Button class="btn_wnd_restore_11" name="btn_window_restore" height="32" width="stretch" visible="false"/>
      </Box>
      <Button class="btn_wnd_close_11" name="btn_window_close" height="stretch" width="40"/>
    </HBox>
    <!-- 内容区域 -->
    <Box>
      <!-- 放置内容 -->
    </Box>
  </VBox>
</Window>
```

### Window 属性速查

| 属性 | 类型 | 说明 |
|------|------|------|
| size | size | 窗口初始大小，支持百分比 "75%,75%" |
| min_size / max_size | size | 最小/最大尺寸 |
| caption | rect | 标题栏可拖动区域 "0,0,0,36" |
| size_box | rect | 可拖动调整大小的边距 "4,4,4,4" |
| shadow_type | string | 阴影类型: default/big/big_round/small/small_round/menu/menu_round/none/none_round/custom/system_default/system_not_round/system_round/system_small_round |
| shadow_attached | bool | 是否附加阴影 |
| layered_window | bool | 是否为层窗口 |
| alpha | int | 透明度 0-255 |
| round_corner | size | 窗口圆角 "4,4" |
| icon | string | 窗口图标路径(ico) |
| use_system_caption | bool | 使用系统标题栏 |
| text / textid | string | 窗口标题/多语言ID |

## 二、容器类型速查

| XML节点 | 布局方式 | 说明 |
|---------|---------|------|
| Box | 浮动(Layout) | 自由定位，子控件绝对或相对布局 |
| VBox | 垂直(VLayout) | 子控件从上到下依次排列 |
| HBox | 水平(HLayout) | 子控件从左到右依次排列 |
| StatusBar | 水平(HLayout) | 状态栏：子控件即状态面板，相邻面板之间自动绘制分隔线（可配置宽度/颜色）；支持按索引读写面板（GetPanelCount/GetPanel/SetPanelText）；右下角尺寸手柄可真正拖拽调整窗口大小（size_grip_resize，默认 true）（继承 HBox） |
| ToolBar | 水平(HLayout) | 工具条：承载工具按钮与分隔线（分隔线用 Line，或用 AddToolSeparator）；提供工具项管理 API（AddToolButton/AddToolCheckBox/AddToolSeparator/RemoveTool/SetToolChecked/FindTool 等）与 ToolItemClick/ToolItemCheckedChanged 回调；宽度不足时自动收纳尾部子控件并在右端显示"更多"按钮，浮层激活同样走工具项回调（继承 HBox） |
| VFlowBox | 垂直流式 | 垂直排列，自动换列 |
| HFlowBox | 水平流式 | 水平排列，自动换行 |
| Panel | 浮动(Layout) | 带标题栏的面板容器（`PanelTemplate<Box>`），可折叠/手风琴 |
| PanelHBox | 水平(HLayout) | 带标题栏的水平面板（`PanelTemplate<HBox>`） |
| PanelVBox | 垂直(VLayout) | 带标题栏的垂直面板（`PanelTemplate<VBox>`） |
| NavigationView | 水平(HLayout) | 侧边栏导航容器（左导航窗格+右内容区），窗格可收起为图标条 |
| VTileBox | 垂直瓦片 | 网格式垂直排列，columns属性 |
| HTileBox | 水平瓦片 | 网格式水平排列，rows属性 |
| GridBox | 网格(GridLayout) | 网格布局，支持单元格合并 |
| TabBox | 浮动 | 多页签切换，仅显示当前页 |
| ScrollBox | 浮动+滚动条 | 可滚动的Box |
| VScrollBox | 垂直+滚动条 | 可滚动的VBox |
| HScrollBox | 水平+滚动条 | 可滚动的HBox |
| VListBox | 垂直列表 | 可选择的垂直列表 |
| HListBox | 水平列表 | 可选择的水平列表 |
| VirtualVListBox | 虚拟垂直列表 | 大数据量虚拟列表(垂直) |
| VirtualHListBox | 虚拟水平列表 | 大数据量虚拟列表(水平) |

### 容器属性

| 属性 | 类型 | 说明 |
|------|------|------|
| child_margin | int | 子控件间距(XY相同) |
| child_margin_x | int | 子控件水平间距 |
| child_margin_y | int | 子控件垂直间距 |
| child_halign | string | 子控件水平对齐: left/center/right |
| child_valign | string | 子控件垂直对齐: top/center/bottom |
| mouse_child | bool | 子控件是否响应鼠标 |
| padding | rect | 内边距 "L,T,R,B" |

### Panel 面板容器属性（Panel / PanelHBox / PanelVBox）

三种节点是同一模板 `PanelTemplate` 的实例（基类分别为 Box/HBox/VBox），在普通容器之外
增加标题栏与折叠/展开能力。头文件 `duilib/Box/Panel.h`，完整示例见 `examples/panel`。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| title | | string | 标题文字 |
| title_id | | string | 标题文字的多语言资源 ID（别名 titleid）；按当前语言解析标题，切换语言自动刷新，找不到 ID 保留原标题；用法同 Label 的 text_id |
| title_height | 28 | int | 标题栏高度（像素，DPI 自适应） |
| title_bk_color | | string | 标题栏背景语义色名，不设置则不绘制 |
| title_text_color | text_default | string | 标题文字语义色名 |
| title_text_align | left | string | 标题水平对齐：left/hcenter/right |
| title_font | system_bold_14 | string | 标题字体 ID |
| collapsible | false | bool | 是否可点击标题栏折叠/展开 |
| collapsed | false | bool | 初始折叠状态（不播动画、不触发事件） |
| collapse_trigger | title | string | 折叠热区：title（整个标题栏）/ arrow（仅箭头） |
| collapse_anim | 0 | int | 折叠动画毫秒数，0 立即切换（建议 150~300） |
| arrow_align | right | string | 箭头位置：right/left |
| group | | string | 手风琴分组名；同窗口同组同时只展开一个（可全折叠） |
| title_slot | | string | 标题栏槽位子控件 name（子控件建议 float="true"），折叠后仍可点击 |
| arrow_expanded_normal_image | | string | 展开态（▼）箭头普通图 |
| arrow_expanded_hovered_image | | string | 展开态箭头悬停图（别名 arrow_expanded_hot_image） |
| arrow_expanded_pushed_image | | string | 展开态箭头按下图（别名 arrow_expanded_pressed_image） |
| arrow_expanded_disabled_image | | string | 展开态箭头禁用图 |
| arrow_collapsed_normal_image | | string | 折叠态（▶）箭头普通图 |
| arrow_collapsed_hovered_image | | string | 折叠态箭头悬停图（别名 arrow_collapsed_hot_image） |
| arrow_collapsed_pushed_image | | string | 折叠态箭头按下图（别名 arrow_collapsed_pressed_image） |
| arrow_collapsed_disabled_image | | string | 折叠态箭头禁用图 |

要点：

- `padding` 仍是内容区内边距，标题栏空间由控件在顶部自动额外预留。
- 未配箭头图时用矢量三角（折叠▶/展开▼），悬停标题栏时箭头变 `color_accent`；某状态缺图回退普通图。
- 折叠只屏蔽内容区的测量/绘制/命中测试，不改子控件 `visible`，fixed/auto/stretch 高度均可折叠。
- 多语言：`title_id` 标题在切换语言时自动刷新（C++ 接口 `SetTitleId()`/`GetTitleId()`）；
  若用 `SetTitle()` 设置了带参数的动态文案，需自行在窗口 `OnLanguageChanged()` 中重设。
- C++：`SetTitle()`/`GetTitle()`、`SetTitleId()`/`GetTitleId()`、
  `SetCollapsed(bool bCollapsed, bool bFireEvent=true, bool bPlayAnim=true)`、`IsCollapsed()`、
  `SetCollapseAnimMillSeconds()`、`SetGroup()`、`SetArrowAlign()`、`SetTitleTextHAlign()`、
  `SetArrowStateImage()`、`SetTitleSlotName()`。
- 事件：完成后触发 `kEventCollapse` / `kEventExpand`；动作前触发
  `kEventPanelCollapsing` / `kEventPanelExpanding`，回调返回 false 可取消
  （XML 初始 `collapsed` 与手风琴内部联动不触发取消事件）。

### NavigationView 侧边栏导航属性（NavigationView / NavigationViewItem）

左侧导航窗格 + 右侧内容区的组合导航容器，头文件 `duilib/Control/NavigationView.h`。
节点 `<NavigationView>` / `<NavigationViewItem>` 已在 `WindowBuilder::CreateControlByClass` 的
`createControlMap` 中注册（`WindowBuilder.cpp`）——自定义 XML 标签必须走这条路径，仅靠
`GlobalManager::AddCreateControlCallback` 会因"回调在控件自身构造里才注册"而首个控件永远创建不了。
完整示例见 `examples/controls`（NavigationForm + `themes/default/controls/navigation.xml`）。

NavigationView（继承 HBox）属性：

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| pane_width | 220 | int | 展开态窗格宽度（DPI 自适应） |
| compact_pane_width | 48 | int | 收起态窗格宽度（仅显示图标） |
| pane_title | | string | 窗格顶部标题文字 |
| pane_title_id | | string | 窗格标题多语言资源 ID，切换语言自动刷新 |
| toggle_icon | | string | 汉堡按钮图标图片属性串 |
| collapsed | false | bool | 初始是否为收起（紧凑）态 |
| show_header | true | bool | 内容区是否显示页头（随选中项同步文字） |
| settings_item | false | bool | 是否在窗格底部生成内置"设置"导航项 |
| settings_page | | string | 设置项关联的内容页 name |
| settings_text_id | | string | 设置项文字多语言 ID |
| settings_icon | | string | 设置项图标图片属性串 |
| selected_id | | string | 初始选中项关联的 page name，缺省为第一个可选项 |

NavigationViewItem（继承 Control）属性：

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 项文字（header 形态显示为分组标题） |
| text_id | | string | 项文字多语言资源 ID（优先于 text） |
| icon | | string | 图标图片属性串（separator 形态忽略） |
| page | | string | 关联内容页的控件 name，点击后切换页面并同步页头 |
| item_type | item | string | 形态：item（可选中导航项）/ header（分组标题）/ separator（分隔线） |
| icon_size | 16 | int | 图标尺寸（DIP） |
| icon_left | 16 | int | 展开态图标左边距（DIP） |
| text_left | 44 | int | 展开态文字左边距（DIP） |
| text_right | 8 | int | 展开态文字右边距（DIP） |
| pill_radius | 6 | int | 选中/悬停热区圆角半径（DIP） |
| pill_inset_x | 6 | int | 热区水平内缩（DIP） |
| pill_inset_y | 3 | int | 热区垂直内缩（DIP） |
| selected_bg_color | bg_list_item_selected | string | 选中背景色名 |
| hovered_bg_color | bg_list_item_hovered | string | 悬停背景色名 |
| selected_text_color | color_accent | string | 选中文字色名 |
| normal_text_color | text_default | string | 普通文字色名 |
| disabled_text_color | text_disabled | string | 禁用文字色名 |
| header_text_color | text_muted | string | 分组标题文字色名 |
| separator_color | border_window | string | 分隔线颜色名 |

子节点路由规则（AddItem 分流）：

- `NavigationViewItem` 子节点 → 加入窗格导航项列表（header/separator 同样入列，仅渲染形态不同）；
- `TabBox` 子节点 → 仅允许一个，被接管为内容区宿主（推荐显式提供，页面结构完全自控）；
- 其他子控件 → 自动加入惰性创建的内容区 TabBox 作为页面，按 `page`/`name` 联动切换。

要点：

- 收起态仅显示图标，鼠标悬停导航项显示文字 tooltip；设置项固定在窗格底部，不参与默认选中与序号统计。
- 序号仅统计可选中导航项（item 形态），header / separator 与设置项均不计入。
- C++：`SelectItem(NavigationViewItem*, bool bFireEvent=true)`、`SelectItem(const DString& pageName)`、
  `SelectItemByIndex(size_t)`、`GetSelectedItem()`、`GetSelectedIndex()`、`GetSelectedPage()`、
  `SetCollapsed(bool)`、`IsCollapsed()`、
  `AddNavItem(strTextId, strIcon, strPageName, pPage=nullptr)`（传入 pPage 时一步完成"建内容页+加导航项+按 name 联动"，不传则仅加导航项）、
  `AddPage(Control*)`（单独把页面加入内容区）、
  `RemoveNavItem(NavigationViewItem*, bool bRemovePage=false)`（bRemovePage=true 时一并移除关联内容页）、
  `RemoveAllNavItems()`、`SetItemEnabled(NavigationViewItem*, bool)`、
  `MoveFocusByKey(NavigationViewItem*, bool bForward)`。
- 键盘：item 形态 tab 可聚焦，上/下方向键在可选项间移动焦点，回车/空格选中；聚焦时绘制强调色焦点环。
- 禁用态：`SetItemEnabled(pItem,false)` 或 XML `enabled="false"`，禁用项不响应点击/键盘，文字走 `disabled_text_color`。
- 事件由 NavigationView 自身触发，与内部 TabBox 的 `kEventTabSelect` 互不干扰（详见"XML 事件系统"一节）：
  `AttachNavItemClick`（重复点击已选中项也触发，wParam 可选项序号）、
  `AttachNavSelectionChanged`（wParam 新序号、lParam 旧序号，无选中为 `Box::InvalidIndex`）、
  `AttachNavPaneToggling`（wParam 目标状态 1=收起/0=展开，回调返回 false 取消切换）、
  `AttachNavPaneToggled`（wParam 当前状态）。

## 三、控件类型速查

| XML节点 | 基类 | 说明 |
|---------|------|------|
| Control | - | 基础控件/占位符 |
| Label | Control | 文本标签 |
| Marquee | Label | 文本跑马灯（横向/纵向循环滚动，速度/方向/暂停/悬停暂停，`duilib/Control/Marquee.h`），XML 节点名 `Marquee` |
| Button | Label | 按钮 |
| CheckBox | Button | 复选框 |
| Option | CheckBox | 单选按钮(group属性分组) |
| Combo | Box | 下拉组合框 |
| FilterCombo | Combo | 可过滤的下拉框 |
| CheckCombo | Box | 多选下拉框 |
| ComboButton | Box | 带下拉的按钮 |
| RichEdit | ScrollBox | 文本编辑框(单行/多行/密码) |
| RichText | Control | 富文本显示(HTML子集) |
| Progress | Label | 进度条（水平/垂直、浮点值、圆角、缓冲 buffer、百分比、值动画、marquee，`duilib/Control/Progress.h`），XML 节点名 `Progress` |
| Slider | Progress | 滑块（可拖动、步长、值变化事件，`duilib/Control/Slider.h`），XML 节点名 `Slider` |
| RangeSlider | Progress | 双滑块范围选择控件（下限/上限两个滑块 + 高亮选中区间；最小间距/区间颜色/圆角；节点名 `RangeSlider`），`duilib/Control/RangeSlider.h` |
| Stepper | Control | 向导步骤条（水平/垂直；节点显示序号或对勾，状态由当前索引+禁用集合派生：待开始/进行中/已完成/禁用；连接线按完成度着色；Next/Back/GoTo 与点击跳转；多语言 `step_title_id`/`step_desc_id`；节点名 `Stepper`），`duilib/Control/Stepper.h` |
| Rating | Control | 星级评分（value/max/allow_half/read_only/star_size/star_gap；星形内容图：实心/半星/空心 + 悬停/按下态图标，SVG 图片驱动、svg_replace_colors 主题色自适应；悬停预览 + 点击/拖拽打分；节点名 `Rating`），`duilib/Control/Rating.h` |
| Carousel | Control | 图片轮播（`item_image` 幻灯片列表；左右导航按钮 + 底部页码指示点，均 SVG 图片驱动、svg_replace_colors 主题色自适应；loop 无限循环 / auto_play 自动播放 + hover_pause / transition_ms 滑动动画 / 鼠标拖拽翻页；节点名 `Carousel`），`duilib/Control/Carousel.h` |
| Pagination | HBox | 分页器（`page_count`/`current_page` 或 `total_count`+`page_size`；动态创建页码按钮，超出 max_visible_count 以 `…` 收纳保留首尾；当前页按钮选中高亮；可选每页条数选择器 `show_page_size` 默认关闭；`button_class` 皮肤类走语义色；节点名 `Pagination`），`duilib/Control/Pagination.h` |
| Breadcrumb | Control | 面包屑导航（`item_text`/`item_text_id` 以 `\|` 分隔条目；`current` 缺省跟随末项；点击祖先项切换当前项；`disabled_items`；自定义 `separator`（默认 ›）/`spacing`/`item_padding`/`item_round`/`click_to_jump`；祖先项链接蓝 + 悬停/按下胶囊背景、当前项加粗 `current_font`；节点名 `Breadcrumb`），`duilib/Control/Breadcrumb.h` |
| CascadingSelect | Control | 级联选择器（内嵌多列 / 下拉列表框两种形态，父项变更自动过滤子列、深层旧选择自动重置；`items`/`items_id` 路径数据（`\|` 分路径、`path_separator` 分层级，相同前缀自动合并）、任意层级、`disabled_paths` 禁用子树、`column_width`/`row_height`/`text_padding`、列内滚轮滚动 + 比例滚动条；下拉形态支持 `dropdown`/`placeholder(_id)`/`allow_clear`/`expand_on_hover`/`popup_width`/`popup_max_rows`/`popup_class` 与自定义显示 `AttachDisplayText`；事件 `AttachSelectionChanged`（`wParam=4`，pEventData 为 CascadingSelectValue）；节点名 `CascadingSelect`），`duilib/Control/CascadingSelect.h` |
| TreeGrid | VBox | 树表格（同一控件内呈现"可展开/收起的层级行 + 多列表格"；表头固定不滚动、列表体自带纵向滚动条，表头与行复用同一列宽天然对齐；`column_title`/`column_width`/`column_align`/`tree_column`/`indent`；`multi_select` 多选、`check_mode`+`check_cascade` 行首复选框父子三态级联；节点名 `TreeGrid`），`duilib/Control/TreeGrid.h` |
| CircleProgress | Progress | 环形进度条（渐变/指示图标/逆时针/indeterminate 转圈，`duilib/Control/CircleProgress.h`），XML 节点名 `CircleProgress` |
| Chart | Control | 轻量图表（折线/柱状/饼图自绘；多系列/坐标轴/负值/堆叠/曲线/面积/环形；悬停Tooltip/点击选中/缩放平移；圆角/渐变/发光/动画，`duilib/Control/Chart.h`），XML 节点名 `Chart` |
| Timeline | Control | 垂直时间轴（节点+连接线+内容，数据驱动、4态配色、正序/倒序、多语言 `item_*_id` 绑定、节点点击事件、空/加载状态、滚动条 hover 显隐可拖动，`duilib/Control/Timeline.h`），XML 节点名 `Timeline` |
| DateTime | HBox | 日期时间选择器（`LabelTemplate<HBox>`） |
| TreeView | ListBox | 树形控件 |
| TreeNode | ListBoxItem | 树节点 |
| ListCtrl | VBox | 列表控件(Report/List/Icon视图) |
| PropertyGrid | VBox | 属性网格 |
| HyperLink | Label | 超级链接 |
| Line | Control | 画线控件 |
| NavigationViewItem | Control | NavigationView 导航项（item/header/separator 三种形态） |
| Split / SplitBox | Control/Box | 分隔条（`SplitTemplate<Control>` / `SplitTemplate<Box>`） |
| ScrollBar | Control | 滚动条 |
| TabCtrl | ListBox | 标签页控件 |
| IPAddress | HBox | IP地址输入 |
| HotKey | HBox | 热键输入 |
| GroupBox/GroupVBox/GroupHBox | Box | 分组容器 |
| ColorControl/ColorPicker* | - | 颜色选择器组件 |
| DirectoryTree | TreeView | 目录树 |

### Control 通用属性(所有控件继承)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| name | | string | 控件名称(窗口内建议唯一) |
| class | | string | 引用global.xml中的通用样式，**必须写在最前面** |
| width | stretch | int/string | 宽度: 数值/stretch/auto/"50%" |
| height | stretch | int/string | 高度: 数值/stretch/auto/"50%" |
| min_width / min_height | -1 | int | 最小宽度/高度 |
| max_width / max_height | MAX | int/string | 最大宽度/高度 |
| margin | 0,0,0,0 | rect | 外边距 "L,T,R,B" |
| padding | 0,0,0,0 | rect | 内边距 "L,T,R,B" |
| halign | left | string | 水平对齐: left/center/right |
| valign | top | string | 垂直对齐: top/center/bottom |
| visible | true | bool | 是否可见 |
| enabled | true | bool | 是否可用 |
| float | false | bool | 是否绝对定位 |
| bkcolor | | string | 背景色(颜色名/ARGB) |
| bkimage | | string | 背景图片 |
| normal_image / hot_image / pushed_image / disabled_image | | string | 各状态图片 |
| normal_color / hot_color / pushed_color / disabled_color | | string | 各状态颜色 |
| border_size | 0 | int/rect | 边框大小 |
| border_color | | string | 边框颜色 |
| border_round | 0,0 | size | 边框圆角 |
| tooltip_text | | string | 鼠标悬浮提示 |
| alpha | 255 | int | 透明度 0-255 |
| cursor_type | arrow | string | 光标: arrow/hand/ibeam/wait/cross/size_we/size_ns 等 |
| no_focus | false | bool | 是否不可获取焦点 |
| tab_stop | true | bool | 是否允许TAB切换 |
| show_focused_rect | false | bool | 键盘TAB聚焦时是否绘制焦点虚线框（别名 show_focus_rect）；C++ 接口 SetShowFocusedRect()/IsShowFocusedRect()，需要"初始不显示、TAB后才显示"时在 OnKeyDownMsg 中按需开启 |
| fade_visible | true | bool | 可见性变化时是否有动画 |
| box_shadow | | string | 阴影 "color='red' offset='0,0' blurradius='8' spreadradius='8'" |

### Label 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 显示文本 |
| text_id | | string | 多语言ID |
| text_align | "left,top" | string | 对齐: left/hcenter/right/hjustify + top/vcenter/bottom |
| text_padding | 0,0,0,0 | rect | 文字内边距 |
| font | | string | 字体ID(定义在global.xml) |
| normal_text_color | | string | 普通文字颜色 |
| hot_text_color | | string | 悬浮文字颜色 |
| single_line | true | bool | 单行显示 |
| multi_line | false | bool | 多行显示 |
| end_ellipsis | false | bool | 省略号截断 |
| rich_text | false | bool | 支持HTML子集富文本 |

### CheckBox 额外属性(继承 Button)

| 属性 | 类型 | 说明 |
|------|------|------|
| selected | bool | 是否选中 |
| selected_normal_image / selected_hot_image / ... | string | 选中状态各状态图片 |

### Option 额外属性(继承 CheckBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| group | string | 分组名称，同组内互斥 |

### RichEdit 属性(继承 ScrollBox)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 文本内容 |
| font | | string | 字体ID |
| text_align | "left,top" | string | 对齐方式 |
| text_padding | | rect | 文字内边距 |
| single_line | true | bool | 单行模式 |
| multi_line | false | bool | 多行模式 |
| password | false | bool | 密码模式 |
| readonly | false | bool | 只读 |
| number_only | false | bool | 仅数字 |
| max_number / min_number | | int64 | 数字范围 |
| limit_text | | int | 最大字符数 |
| prompt_mode | false | bool | 是否显示占位提示文字 |
| prompt_text | | string | 占位提示文字（text 为空时显示） |
| prompt_text_id | | string | 占位提示文字的多语言 ID |
| prompt_color | | string | 提示文字颜色 |
| word_wrap | false | bool | 自动换行 |
| vscrollbar / hscrollbar | false | bool | 滚动条 |
| want_return | false | bool | 接受回车 |
| want_tab | false | bool | 接受Tab |
| caret_color | | string | 光标颜色 |
| normal_text_color | | string | 文字颜色 |

### SpinBox 属性(继承 RichEdit，数字输入框)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| decimals | 0 | int(0~10) | 小数位数；0 为普通整数模式 |
| step | 1 | double | 步长（必须 > 0，支持小数） |
| value | 0 | double | 初始值（超范围时钳位，并按 decimals 四舍五入） |
| min_number / max_number | 无限制 | double | 数值范围（别名 `min_value` / `max_value`） |
| prefix / suffix | "" | string | 显示前缀 / 后缀（如 `"$ "`、`" kg"`） |
| group_separator | "" | string | 千分位分隔符（别名 `thousand_separator`） |
| spin_class | | string | 步进按钮样式（同 RichEdit，如 `rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down`） |

C++ 接口：`SetStep/GetStep/GetStepDouble`、`SetValue/GetValue/GetValueDouble`、`SetRange`、`SetMinValue/GetMinValue`、`SetMaxValue/GetMaxValue`、`SetDecimals`、`SetPrefix/SetSuffix`、`SetGroupSeparator`。数值变化触发 `kEventTextChanged`；失焦时按格式重写文本，非法输入回退到最近有效值。

注意：需设置 `spin_class` 属性才会显示步进按钮，否则只有上下方向键能调整数值；纯整数模式保持 RichEdit 的 `number_only` 输入过滤，设置小数/前后缀/千分位后改为 `limit_chars` 过滤。

### SearchBox 属性(继承 HBox，搜索框组合控件)

左图标 + 内部 RichEdit + 清除按钮的组合控件（`duilib/Control/SearchBox.h`），XML 节点名 `SearchBox`。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 初始文本 |
| text_id | | string | 初始文本的多语言字符串 ID（与 text 同时设置时，后设置的属性生效） |
| prompt_text | | string | 空内容时的占位提示文本 |
| prompt_text_id | | string | 占位提示文本的多语言字符串 ID |
| prompt_color | | string | 占位提示文本颜色（语义色名） |

C++ 接口：`SetSearchText/GetSearchText`、`GetEditControl()`（取内部 RichEdit）。
事件：文本变化触发 `kEventTextChanged`，按回车触发 `kEventReturn`（发起搜索）。
清除按钮复用 RichEdit 内置 `clear_btn_class`：聚焦且有文本时显示，点击清空，失焦隐藏。
默认皮肤类：`search_box` / `search_box_icon` / `search_box_edit` / `search_box_clear_btn`。

### TagInput 属性(继承 Box，标签输入框组合控件)

流式标签 + 内部 RichEdit 的组合控件（`duilib/Control/TagInput.h`），XML 节点名 `TagInput`。回车成标签、X 按钮或空框 Backspace 删标签，标签过多自动换行增高。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| prompt_text | | string | 空内容时的占位提示文本 |
| prompt_text_id | | string | 占位提示文本的多语言字符串 ID |
| prompt_color | | string | 占位提示文本颜色（语义色名） |
| allow_duplicate | false | bool | 是否允许重复标签 |
| max_count | 0 | int | 最多标签个数，0 不限 |
| delimiter | | string | 输入时直接成标签的分隔符（如 `,`），空表示仅回车成标签 |
| tags | | string | 初始标签，多个以分号 `;` 分隔 |

C++ 接口：`AddTag`（空/重复/超限返回 false）、`RemoveTagAt`、`RemoveAllTags`、`GetTagCount`、`GetTagText`、`SetAllowDuplicate/IsAllowDuplicate`、`SetMaxCount/GetMaxCount`、`GetEditControl()`、`SetFocus`。
事件：`kEventTagAdded`（WPARAM=新标签序号）、`kEventTagRemoved`（WPARAM=被删序号，回调中仍可 GetTagText）、`kEventTagClick`（WPARAM=序号，点关闭按钮不触发）、转发 `kEventTextChanged`。
默认皮肤类：`tag_input` / `tag_input_tag` / `tag_input_tag_text` / `tag_input_tag_close` / `tag_input_edit`，底色语义色 `bg_tag` / `bg_tag_hover`；关闭按钮悬浮/按下为灰圆底白叉，语义色 `bg_tag_close_hover` / `bg_tag_close_pressed`。

### AutoComplete 属性(继承 Box，自动完成输入框组合控件)

内部单行 RichEdit + 下方候选浮层（`AutoCompleteFlyout` 复用 Flyout，不抢焦点、宽度跟随编辑框、高度随候选项自适应、超出滚动）的组合控件（`duilib/Control/AutoComplete.h`），XML 节点名 `AutoComplete`。输入达 min_chars 后实时过滤弹层；上下键移动、回车确认、Esc 关闭，鼠标悬停高亮/点击确认；确认后回填文本且不重弹。与 FilterCombo 区别：以编辑框为主体，浮层不占布局空间。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| items | | string | 初始候选数据源，多个以分号 `;` 分隔（也可代码 `SetCandidates`） |
| prompt_text / prompt_text_id / prompt_color | | string | 占位提示文本 / 多语言 ID / 颜色（语义色名） |
| min_chars | 1 | int | 输入多少字符后开始联想 |
| max_visible | 8 | int | 浮层最多可见行数，超出滚动 |
| match_mode | prefix | string | `prefix` 前缀匹配 / `contains` 包含匹配 |
| ignore_case | true | bool | 是否忽略大小写 |

C++ 接口：`SetCandidates/GetCandidates`、`AddCandidate`（去重）、`ClearCandidates`、`SetText/GetText`（程序化 SetText 不弹浮层）、`SetMinChars/GetMinChars`、`SetMaxVisible/GetMaxVisible`、`SetMatchMode/GetMatchMode`（`AutoCompleteMatchMode::kPrefix/kContains`）、`SetIgnoreCase/IsIgnoreCase`、`GetEditControl()`、`SetFocus`。
事件：`kEventAutoCompleteSelect`（WPARAM=选中项在当前候选列表中的序号；XML 内联事件名 `auto_complete_select`）、转发 `kEventTextChanged`（回填不触发）。
默认皮肤类：控件 `auto_complete` / `auto_complete_edit`；浮层 `auto_complete_flyout` / `auto_complete_list` / `auto_complete_item`，候选项复用列表语义色 `bg_list_item_hovered` / `bg_list_item_selected`，卡片 `bg_window_card` / `border_window`。

### Switch 属性(继承 CheckBox，滑块开关)

轨道 + 圆形滑块 + 滑动/淡入淡出过渡动画的开关控件（`duilib/Control/Switch.h`），XML 节点名 `Switch`。
轨道与滑块均支持 SVG 图片皮肤（off/on 成对配置，按动画进度交叉淡入，滑块图同时平移）；未配置图片时用语义色自绘。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| switch_animation_ms / animation_ms | 160 | int | 动画时长（毫秒），0 表示无动画 |
| track_off_color / trackoncolor | `bg_switch_track_off` | string | 轨道未选中颜色（语义色名，无轨道图时生效） |
| track_on_color / trackoncolor | `bg_switch_track_on` | string | 轨道选中颜色 |
| thumb_off_color / thumboffcolor | `bg_switch_thumb_off` | string | 滑块未选中颜色（无滑块图时生效） |
| thumb_on_color / thumboncolor | `bg_switch_thumb_on` | string | 滑块选中颜色 |
| track_off_image / trackoffimage | | string | 轨道未选中图片（图片属性串，须与 track_on_image 成对） |
| track_on_image / trackonimage | | string | 轨道选中图片（拉伸铺满轨道矩形） |
| thumb_off_image / thumboffimage | | string | 滑块未选中图片（方形，须与 thumb_on_image 成对） |
| thumb_on_image / thumbonimage | | string | 滑块选中图片（方形框边长=轨道高，阴影留白画在图内） |
| thumb_padding | 0 | int | 滑块方形框距轨道边缘距离（自动 DPI 缩放） |
| switch_text_on / switchtexton | | string | 轨道内文字（选中态，对应 Ant 的 checkedChildren） |
| switch_text_off / switchtextoff | | string | 轨道内文字（未选中态，对应 Ant 的 unCheckedChildren） |
| switch_text_on_color / switchtextoncolor | `bg_switch_thumb_on` | string | 轨道内文字颜色（选中态） |
| switch_text_off_color / switchtextoffcolor | `text_default` | string | 轨道内文字颜色（未选中态） |
| switch_image_on / switchimageon | | string | 轨道内图标（选中态，图片属性串，可与文字组合） |
| switch_image_off / switchimageoff | | string | 轨道内图标（未选中态，图片属性串，可与文字组合） |
| switch_loading / switchloading | false | bool | 加载中：滑块内 8 点旋转指示器，且屏蔽用户点击切换 |
| switch_loading_color / switchloadingcolor | `color_accent` | string | 加载指示器颜色 |
| track_disabled_color / trackdisabledcolor | `bg_switch_track_off` | string | 禁用态轨道颜色（无轨道图时生效） |
| thumb_disabled_color / thumbdisabledcolor | `bg_switch_thumb_on` | string | 禁用态滑块颜色（无滑块图时生效） |

注意：滑块/轨道图片属性串中不要带 width/height/valign/halign（否则绘制位置固定不平移）；
图片内固定色用 `svg_replace_colors='#XXXXXX|语义色名'` 替换适配深浅色（多组分号分隔）。
轨道内文字/图标按动画进度交叉淡入淡出；同时配置时按「图标 + 文字」水平排列并居中；禁用时文字用 `text_disabled`。

C++ 接口：`SetAnimationDuration`、`SetTrackOffColor/GetTrackOffColor`、`SetTrackOnColor/GetTrackOnColor`、`SetThumbOffColor/GetThumbOffColor`、`SetThumbOnColor/GetThumbOnColor`、`SetTrackOffImage/GetTrackOffImage`、`SetTrackOnImage/GetTrackOnImage`、`SetThumbOffImage/GetThumbOffImage`、`SetThumbOnImage/GetThumbOnImage`、`SetThumbPadding/GetThumbPadding`、`SetTextOn/GetTextOn`、`SetTextOff/GetTextOff`、`SetTextOnColor/GetTextOnColor`、`SetTextOffColor/GetTextOffColor`、`SetImageOn/GetImageOn`、`SetImageOff/GetImageOff`、`SetLoading/IsLoading`、`SetLoadingColor/GetLoadingColor`、`SetTrackDisabledColor/GetTrackDisabledColor`、`SetThumbDisabledColor/GetThumbDisabledColor`。
事件：状态变化触发 `kEventSelect` / `kEventUnSelect`（继承 CheckBox）。
四套默认皮肤类（SVG 在 `public/switch/`）：
- `switch`（别名 `switch_ios`）：iOS 风格 51x31，白色大滑块带柔和投影，轨道色随 `color_accent`；
- `switch_fluent`：Win11 Fluent 风格 48x24（padding 2），描边小轨道 + 14px 小滑块；
- `switch_material`：Material 3 风格 52x32，未选中描边轨道+16px 小滑块，选中主题色轨道+24px 带对勾大滑块；
- `switch_small`：小号 40x20，字体 `system_regular_12`，其余沿用 iOS 皮肤。

### Badge 属性(继承 Label，角标)

TabCtrl 标签、按钮、图标上的未读数/小红点控件（`duilib/Control/Badge.h`），XML 节点名 `Badge`。
数字角标：圆角胶囊背景+数字，超过上限显示"99+"；红点角标（dot 模式）：纯小圆点。继承 Label 的文本属性（font/normal_text_color/text_padding 等）。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| count / badge_count | 0 | int64 | 角标数量：>0 显示，<=0 自动隐藏 |
| max_count / maxcount | 99 | int64 | 数量上限，超过显示"上限+"（如"99+"） |
| dot | false | bool | 红点模式：true 为纯小圆点不显示数字（显隐由 visible 控制，不受 count 影响） |
| badge_color / badgecolor | `bg_badge` | string | 角标背景颜色（语义色名或颜色值） |

用法：作为子控件放在任意 Box 内；悬浮宿主控件角上用 `float="true"` + `margin` 定位（子控件超出父容器边界的部分会被裁剪，父容器需留出角标外露宽度）。
默认皮肤类：`badge`（数字角标，高 18，白字红底，宽度自适应） / `badge_dot`（8x8 红点）。
语义色：`bg_badge`（派生自 `color_error`）、`text_badge`（白色，light/dark 同值）。
C++ 接口：`SetCount/GetCount`、`SetMaxCount/GetMaxCount`、`SetDotMode/IsDotMode`、`SetBadgeColor/GetBadgeColor`。
示例：`<Badge class="badge" count="5"/>`；按钮右上角 `<Box width="66" height="30"><Button .../><Badge class="badge" count="6" float="true" margin="42,0,0,0"/></Box>`。

### TabCtrlItem 内置角标（继承 ControlDragableT）

TabCtrl 标签页内建未读数/小红点支持（`duilib/Control/TabCtrl.h`），角标为懒创建的 Badge 子控件（排在标题之后）。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| badge_count | 0 | int64 | 角标数量：>0 显示，<=0 自动隐藏；未设置 badge_class 时仅保存数值 |
| badge_max_count | 99 | int64 | 数量上限，超过显示"上限+"（如"99+"） |
| badge_dot | false | bool | 红点模式：true 为纯小圆点不显示数字 |
| badge_class | | string | 角标控件 Class，为空时不创建角标子控件 |

默认皮肤类：`tab_ctrl_item_badge`（数字角标，高 16，挂在 `tab_ctrl_item` 的 badge_class 上） / `tab_ctrl_item_badge_dot`（8x8 红点，dot 模式需指定此类，否则 auto 宽度下红点不可见）。
C++ 接口：`SetBadgeCount/GetBadgeCount`、`SetBadgeMaxCount/GetBadgeMaxCount`、`SetBadgeDot/IsBadgeDot`、`SetBadgeClass/GetBadgeClass`、`GetBadgeControl`。
示例：`<TabCtrlItem class="tab_ctrl_item" title="消息" badge_count="5"/>`、`<TabCtrlItem class="tab_ctrl_item" title="动态" badge_class="tab_ctrl_item_badge_dot" badge_dot="true"/>`。

### Flyout 浮层窗口(继承 WindowImplBase)

通用浮层（弹出卡片）窗口（`duilib/Control/Flyout.h`，已在 duilib.h 中 include）。在锚点控件周围浮出任意 Box 内容，非模态、不抢焦点、点击外部/Esc 自动关闭、全局单活，窗口关闭后框架自动 delete。
窗口样式由 C++ 内部固定（`WS_POPUP|WS_EX_LAYERED|WS_EX_TOPMOST`，noFocus 时加 `WS_EX_NOACTIVATE`），调用方只提供**内容 XML**：普通 `<Window shadow_type="default" shadow_attached="true" layered_window="true" caption="0,0,0,0" use_system_caption="false">` + 固定 `size="宽,高"`（DIP 逻辑像素，高度须容纳全部内容，超出会画到阴影区；框架自动在外层包阴影）。

C++ 接口（无 XML 控件属性）：
- `Flyout(Window* pParentWindow)`：构造，锚点控件必须属于父窗口；
- `SetSkinFolder(path)`：内容 XML 资源文件夹，默认用父窗口 GetResourcePath()；
- `bool ShowAt(Control* anchor, const DString& xmlFile, Placement = Bottom)`：弹出；**xmlFile 只传文件名不带子目录前缀**（SkinFolder 已含子目录）；失败时内部已 `delete this`，返回 false 后不可再访问；
- `Dismiss()` 主动关闭；`SetAutoDismiss/IsAutoDismiss`（默认 true）、`SetNoFocus/IsNoFocus`（默认 true）、`SetGap/GetGap`（默认 6 DIP）、`SetAllowFlip/IsAllowFlip`（默认 true）；
- `GetAnchor()`、`GetPlacement()`、`IsOpen()`；`AttachOpened(cb)` / `AttachClosed(cb)`（`typedef std::function<void(CloseReason)> FlyoutEvent`）；
- 静态 `GetActiveFlyout()` / `DismissActive()`。

`enum class Placement {Bottom,BottomEnd,Top,TopEnd,Right,RightEnd,Left,LeftEnd}`（8 方位，空间不足自动翻转对侧并夹持工作区）；
`enum class CloseReason {kManual,kClickOutside,kEscape,kAnchorLost}`（父窗口销毁/隐藏/最小化、锚点失效、跨屏 DPI 变化→kAnchorLost）。

内容 XML 默认皮肤类（global.xml）：`flyout`（240 宽卡片，`bg_window_card`/`border_window`/圆角 8/padding 12）、`flyout_title`（system_bold_14 + text_default）、`flyout_desc`（system_regular_12 + text_muted，多行）。

```cpp
ui::Flyout* p = new ui::Flyout(this);
p->SetSkinFolder(GetResourcePath().ToString());
if (!p->ShowAt(pBtn, _T("my_flyout.xml"), ui::Flyout::Placement::Bottom)) return;
ui::Button* pOk = (ui::Button*)p->FindControl(_T("ok"));   // ShowAt 成功后再绑事件
pOk->AttachClick([p](const ui::EventArgs&){ p->Dismiss(); return true; });
```

注意：noFocus 模式外部点击检测仅 Windows（全局鼠标按键 50ms 轮询，非 Win 平台用 `SetNoFocus(false)` 焦点模式）；浮层内控件可正常点击但键盘焦点不进入，需要输入文字时 SetNoFocus(false)；同锚点再次点击切换关闭需调用方自行比较 `GetActiveFlyout()->GetAnchor()`；锚点在滚动容器内时自动扣除累计滚动偏移；父窗口通过 `OpenColorTheme` 使用私有颜色主题时浮层自动继承该主题（新增 `Window::GetColorThemeXmlData()` 暴露私有主题 XML 数据）。

### Mask 属性(继承 Box，遮罩/蒙版层)

半透明蒙层容器（`duilib/Control/Mask.h`），XML 节点名 `Mask`。用于模态对话框背景遮罩、新手引导蒙层、加载遮罩；可镂空高亮指定控件，镂空区域内点击可穿透到目标控件。跨平台通用。
作为浮动覆盖层使用时配置 `float="true" width="stretch" height="stretch"`（内置 `mask` Class 已含该配置），并置于父容器最后一个子节点（或配合 `paint_order`），以保证置顶绘制与优先命中。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| mask_color / maskcolor | `bg_overlay` | string | 蒙层颜色（语义色名或颜色值）；为空不绘制底色 |
| highlight_control / highlightcontrol | | string | 需镂空(高亮)的控件名称（在整个窗口内按名称查找，可为 Mask 之外的控件）；为空则不镂空，整块区域均为蒙层 |
| highlight_padding / highlightpadding | 4 | int | 镂空区域内边距（自动 DPI 缩放） |
| highlight_corner / highlightcorner | 0 | int | 镂空区域圆角半径（自动 DPI 缩放） |
| click_through / clickthrough | false | bool | 镂空区域内点击是否穿透到下层控件（新手引导用） |

`mouse_enabled`（继承自容器基类）默认 true 即拦截鼠标（模态）；设为 false 为仅视觉遮挡、不拦截鼠标。
蒙层内可继续放置任意子控件（加载动画、提示文案、模态卡片按钮等）；模态卡片在 Mask 内用 `float="true" halign="center" valign="center"` 居中。
C++ 接口：`SetMaskColor/GetMaskColor`、`SetHighlightControl/GetHighlightControl`、`SetHighlightPadding/GetHighlightPadding`、`SetHighlightCorner/GetHighlightCorner`、`SetClickThrough/IsClickThrough`；静态 `Mask::ShowOverWindow(Window*)`（整窗覆盖，返回指针或 nullptr）、`Remove()`（从父容器移除；父容器自动销毁子控件时调用后对象即销毁）。
默认皮肤类：`mask`（`float="true" width="stretch" height="stretch" mask_color="bg_overlay"`）。
示例：`<Mask class="mask" highlight_control="btn_guide" highlight_padding="6" highlight_corner="6" click_through="true"/>`；加载遮罩内放 `<Progress class="progress_horizontal" marquee="true" marquee_width="60"/>`。

### ListCtrl 表格增强(继承 VBox，Report 视图)

面向报表场景（类似 Qt QTableView / WPF DataGrid）。列宽拖拽调整、列头拖拽换序（`enable_header_drag_order`）、单元格图标（`SetSubItemImageId`）为既有能力；表格增强新增：

| 能力 | 属性/接口 | 说明 |
|------|-----------|------|
| 列冻结 | `frozen_column_count`(XML) / `SetFrozenColumnCount` / `GetFrozenColumnCount` | 冻结列表左侧前 N 列，横向滚动时固定不动；表头同步冻结，网格线与命中测试已适配 |
| 单元格进度条 | `SetSubItemProgress(ById)` / `GetSubItemProgress(ById)` | 进度值 `[0,100]`，`-1` 关闭（按普通文本显示）；可叠加文本显示百分比 |
| 单元格富文本 | `SetSubItemRichText(ById)` / `IsSubItemRichText(ById)` | 文本按富文本渲染（支持 `<b>/<i>/<u>/<font>/<a>` 等标签） |
| 单元格自动换行 | `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` | 控制 Report 单元格文本单行(true，默认，超出按省略号裁剪)/多行(false，按单元格宽度自动换行)；行高需由调用方通过 `SetDataItemHeight` 调够，否则超出部分被裁剪；仅 Report 视图有效（非 Report 视图 `Set*` 返回 false）。虚表下属性存于数据模型，填充单元格时应用 |
| 单格刷新 | `RefreshSubItem` / `RefreshSubItemById` | 只刷新指定单元格（进度/状态类列局部更新），仅 Report 视图；单元格不在可见范围时返回 false（数据已更新） |
| 列显示/隐藏 | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl 一级列显隐接口，内部自动同步数据列 |
| 列布局事件 | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | 列宽/列序/列显隐/排序变化时回调，便于应用持久化列布局；参数见 `kEventColumn*`（wParam 为列ID，lParam 为新状态或关联列ID） |
| 列级比较模式 | `SetColumnSortFlag(ById)` / `GetColumnSortFlag(ById)`，标志位 `ListCtrlSubItemSortFlag` | 按列声明排序比较方式（点击该列表头自动采用）；`kSortByProgress` 按进度值排序，`kSortByTextAsNumber` 文本按数值比较（`"10" > "9"`），`kSortByTextAsDate` 文本按日期时间先后比较（`"2024-3-7" < "2024-10-1"`），替代按单元格 `nProgress` 推断 |
| 批量删除行 | `DeleteDataItems(const std::vector<size_t>&)` | 一次删除多行（索引可乱序，越界/重复项被忽略）；内部按行身份去重 + 对显示顺序表做单趟压缩，避免逐行删除的 O(K×N) 搬移，返回实际删除行数 |
| 存储压缩 | `CompactStorage()` | 显式整表维护操作：行身份重编号 + 存储页全量压缩，回收多次增删后遗留的墓碑槽位与空页内存；仅在无填充/绘制进行时调用（调用后内部已发出的单元格原始指针失效）。单行/批量删除时，整页全部变为墓碑的存储页会被自动回收 |
| 单元格选择模式 | `selection_mode`(XML，`row`/`cell`) / `SetSelectionMode` / `GetSelectionMode` | 选择模式：`row`(行选择，默认，既有行为不变) / `cell`(单元格选择)；切到 `cell` 会清空行选择，切回 `row` 会清空单元格选择 |
| 单元格选择 | `SetCurrentCell` / `GetCurrentCell` / `SetCellSelection` / `SetSelectedCells` / `ClearCellSelection` / `GetCellSelection` / `GetSelectedCells` / `IsCellSelected` / `IsCellCurrent` | 单元格选择模式下的当前格与区域选择；选择状态按行身份（`RowId`）+ 列ID 的逐列位图存储，排序后不漂移，支持不连续选择。单击命中格设为当前格/锚点；`Shift+单击`矩形扩展；`Ctrl+单击`切换单元格；按住**左键或右键**拖动：出现与行选择模式**完全一致**的浅蓝半透明矩形框选蒙层（复用 `ListBoxHelper`），松手后矩形内的单元格被选中（替换），`Ctrl+拖动`为追加（只增不减）、拖到视口边缘自动滚动。`SetCellSelection(anchor, focus, bAdditive)` 第三参 `bAdditive=true` 表示追加；`SetSelectedCells(cellList, bClearOthers)` 按单元格列表设置；`GetSelectedCells` 展开为单元格列表（单元格框选复用 ListBox 框选能力，需要 `multi_select == true`）；`ClearCellSelection` 仅在确实存在选择状态（有选中单元格或当前格）时才刷新并派发 `kEventCellSelectionChanged`，无变化时不派发 |
| 行/单元格模式分派 | `SetSelectAll` / `SetSelectNone` / `SetSelectedDataItems` / `GetSelectedDataItems` | `SetSelectAll`/`SetSelectNone` 按当前模式分派：`row` 全选/清空行、`cell` 全选/清空单元格；行专用 API（`SetSelectedDataItems`/`GetSelectedDataItems` 等）**保留原有行为**（仍读写底层行选择状态），`SetSelectedCells` 作为单元格等价 API。`cell` 模式下由**绘制层**屏蔽行选中视觉（`ListCtrlReportView::CanPaintSelectedColors` 返回 false），故无论行选择由哪个入口（含鼠标右键）改动，界面都看不出行被选中 |
| 单元格选择事件 | `AttachCurrentCellChanged` / `AttachCellSelectionChanged` | `kEventCurrentCellChanged`(当前单元格变化，wParam=行显示索引，lParam=列ID) / `kEventCellSelectionChanged`(选择变化，wParam=选中区域个数，pEventData 指向选中区域列表 `std::vector<ui::ListCtrlCellRange>*`，可能多项，无选择时为 nullptr)；XML 内联事件名 `on_current_cell_changed` / `on_cell_selection_changed` |
| 单元格编辑(C3) | `SetColumnEditorType(ById)` / `GetColumnEditorType(ById)` / `EnterCellEdit` / `EnterCurrentCellEdit` / `IsCellEditing` / `GetEditingCell`，类型 `ListCtrlEditorType` | 单元格就地编辑（Report 视图；`kEditCombo` 在 `kCell` 模式与行选择模式均支持，其余编辑器仅 `kCell` 模式）：列级声明编辑器类型（`kEditText`=0 文本默认 / `kEditNumber`=1 十进制数值 / `kEditDate`=2 日期 `yyyy-mm-dd`、`yyyy/mm/dd` / `kEditCombo`=3 下拉候选，枚举补全为 `{ kEditText, kEditNumber, kEditDate, kEditCombo }`）。`kEditCombo` 候选取自按列候选列表：`SetColumnCandidates(columnIndex,candidates)` / `SetColumnCandidatesById(columnId,candidates)` / `GetColumnCandidates(columnIndex)` / `GetColumnCandidatesById(columnId)`（空列表清空候选；未配置候选的列不进入编辑）；候选浮层复用 `Flyout`+`VListBox`（主题类 `list_ctrl_combo_flyout`/`list_ctrl_combo_list`/`list_ctrl_combo_item`），鼠标点选或 `Enter` 提交、`↑`/`↓` 移动高亮、`Esc` 或点击外部取消不写回），类型按列ID保存（列序变化不受影响，未设置返回 `kEditText`）；`F2`/鼠标左键单击进入编辑，`Enter` 提交并当前格下移一行（仅 `kCell` 模式；`kRow` 行模式提交候选不移动选中行、不设置当前格、不滚动视图）、`Esc` 取消不写回、失焦提交不移动；提交按类型校验（空串合法=清空单元格，非法输入丢弃且不派发 `kEventLeaveEdit`）。`kEditNumber` 编辑期即限制输入字符（数字/正负号/小数点，RichEdit 字符白名单 `SetLimitChars`，键盘与粘贴均生效）；`kEditDate` 使用独立 `DateTime` 控件编辑（编辑格式 `date_calendar`、主题类 `list_ctrl_datetime`、ListCtrl 属性 `list_ctrl_datetime_class`；进入编辑自动弹出月日历，选中日期即提交，`Esc` 关闭日历不写回；复用该控件前需调用 `DateTime::ResetCalendarFlyoutState()` 清除日历浮层的"抑制重开"标志，退出编辑态前调用 `DateTime::CloseCalendarFlyout()` 关闭仍打开的浮层）。复用既有 `kEventEnterEdit`/`kEventLeaveEdit`，未新增事件 |

- 进度条样式：`ListCtrlSubItem` 的 `progress_color`(前景，默认 `bg_progress`) 与 `progress_bkcolor`(背景轨道，默认 `bg_progress_track`)，默认值配置在 `global.xml` 的 `list_ctrl_sub_item` Class。
- 也可在填充数据时通过 `ui::ListCtrlSubItemData` 的 `nProgress`(-1 关闭)/`bRichText` 一次性设置。
- 示例：`<ListCtrl class="list_ctrl" name="list_ctrl" frozen_column_count="1"/>`。
- 单元格编辑（C3）：`ListCtrlColumn::nEditorType`（默认 `ListCtrlEditorType::kEditText`）按列保存编辑器类型；`EnterCellEdit(nRowIndex, nColumnId)` 进入指定单元格编辑（等同对当前格按 F2，行不可视时先滚动到可视区），`EnterCurrentCellEdit()` 对当前格进入编辑（F2 实现入口），`GetEditingCell(ListCtrlCellId&)` 取编辑中单元格（无编辑态返回无效值）。触发进入编辑仅 `F2`（立即进入，不走单击链路的 600ms 定时器）与鼠标左键单击（沿用原有链路，未做双击/直接键入）。提交校验：空串合法（清空单元格）、`kEditNumber` 为十进制数值、`kEditDate` 为 `yyyy-mm-dd`/`yyyy/mm/dd`（复用 `Calendar::ParseDateString`），非法输入丢弃（不写回、不派发 `kEventLeaveEdit`）。

### Progress 属性(继承 Label)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| min | 0 | int | 最小值 |
| max | 100 | int | 最大值 |
| value | 0 | double | 当前值（支持浮点） |
| horizontal | true | bool | 水平方向 |
| progress_color | | string | 进度条前景颜色（语义色名或颜色值） |
| progress_image | | string | 进度条前景图片 |
| progress_radius | 0 | int | 纯色前景圆角半径（0 为直角） |
| buffer_value | -1 | double | 次级进度值（缓冲/buffer，≤min 不绘制） |
| buffer_color | | string | 次级进度颜色（空则用前景色浅色版） |
| show_percent | false | bool | 自动显示百分比文本 |
| animation_enabled | false | bool | 值变化动画 |
| animation_duration | 300 | int | 动画时长（毫秒） |
| animation_easing | EaseOutQuad | string | 动画缓动曲线，取值与 `EasingFunctionType` 枚举名完全一致（EaseLinear / EaseIn|Out|InOut{Sine,Quad,Cubic,Quart,Quint,Expo,Circ,Back,Elastic,Bounce}，共 31 条；首字母大小写均可，无法识别时按 EaseLinear 处理） |
| marquee | false | bool | 滚动显示（不确定进度） |
| marquee_width / marquee_step | | int | 滚动宽度 / 步长 |
| reverse | false | bool | 反向填充：false 为 value=min 空 / value=max 满；true 则相反。min≠0 时也按比例正确换算 |

**健壮性约定（已实现，勿重复实现）**：
- `SetMinValue`/`SetMaxValue` 会自动调用 `ClampCurrentValue()`，把 `m_fCurrentValue`、动画中间值
  `m_fDisplayValue` 与 `m_fBufferValue` 一并拉回新区间。因此 `GetValue()` 始终在 `[min,max]` 内，
  不会出现"绘制时才钳位、成员值越界"的不一致。区间非法（min>max）时不改动。
- `progress_radius` 同时作用于**纯色主进度、buffer 次级进度与 marquee 滚动块**（三者共用
  `DrawSolidProgressRect`），因此配了圆角后 marquee 不再是直角。
- `ClampCurrentValue()` **不钳位未设置的 buffer**（值<= min 时保持原状），否则会把"未设置"污染成 min。
  值被钳位时会**停止正在播放的动画**——动画终点是旧值，不停会把显示值重新推回越界区间。
- `reverse` 必须**先归一化成比例再取反**（`ratio = (v-min)/(max-min)`，reverse 时 `1-ratio`），
  **不要**用 `(max-min)-fValue` 翻转绝对值：min≠0 时会算出越界/负比例（Progress 与 CircleProgress 同此坑，已修）。
- marquee 回绕周期 = 轨道长度 + 块宽，取模并处理负值；轨道长度 <= 0 时pos 复位为 0。

C++ 接口：`SetValue/GetValue/SetValueNoEvent`、`SetMinValue/SetMaxValue`、`ClampCurrentValue`、`SetHorizontal`、`SetProgressColor`、`SetProgressRadius`、`SetBufferValue/SetBufferColor`、`SetShowPercent`、`SetAnimationEnabled/SetAnimationDuration`、`SetAnimationEasing(EasingFunctionType)`、`SetAnimationEasingName/GetAnimationEasingName`、`SetMarquee`、`SetReverse`、`SetTextFormatCallback`（自定义进度文本格式）。
事件：`AttachValueChanged`（值变化）、`AttachProgressCompleted`（到达完成端点：正向 max / 倒数 min）。

### Slider 属性(继承 Progress)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| thumb_normal_image / thumb_hovered_image / thumb_pressed_image / thumb_disabled_image | | string | 滑块各状态图片 |
| thumb_size | 10,10 | size | 滑块大小 |
| step | 1 | int | 步长 |
| page_step | 0 | int | PageUp/PageDown 及 Shift+方向键的大步长，0=自动取 (max-min)/10 |
| click_to_jump | true | bool | 点击轨道按下即跳值（false=松开才跳值） |
| progress_bar_padding | 0,0,0,0 | rect | 滑块绘制内边距 |
| show_ticks | false | bool | 是否绘制刻度线（标尺） |
| tick_count | 5 | int | 刻度数量（含首尾，至少 2） |
| tick_length | 4 | int | 刻度线长度（自动 DPI 缩放） |
| tick_color | text_muted | string | 刻度线颜色 |
| tick_pos | bottom | string | 刻度位置：bottom/top（水平）、right/left（垂直） |
| show_tick_labels | false | bool | 是否绘制刻度值文本 |
| tick_label_interval | 1 | int | 每隔几个刻度显示一次标签 |
| tick_font | system_regular_12 | string | 刻度值字体ID |
| tick_label_color | text_muted | string | 刻度值颜色 |

键盘操作（无障碍，Tab 可聚焦，处理 `kEventKeyDown`）：←/→（垂直为 ↑/↓）按 `step` 步进；Shift+方向键与 PageUp/PageDown 按 `page_step` 大步进；Home/End 跳到 min/max；滚轮按 `step` 步进。均自动钳位到 [min,max] 并触发 `kEventValueChanged`。
刻度按滑块可移动区间定位，与滑块中心对齐；整数刻度去掉小数。带标签的刻度为长主刻度、其余为短次刻度（标尺层级），标签半宽按刻度间距自适应收窄（从设计上避免重叠），首尾标签改为左/右靠边对齐（垂直为顶/底），文字向内展开，不裁剪也不侵占相邻标签空间。自定义刻度文本用 `SetTickLabelCallback(std::function<DString(double value, int32_t index)>)`。
C++ 接口：`SetPageStep/GetPageStep`、`SetClickToJump/IsClickToJump`、`SetShowTicks`、`SetTickCount`、`SetTickLength`、`SetTickColor`、`SetTickPos(SliderTickPos)`、`SetShowTickLabels`、`SetTickLabelInterval`、`SetTickFontId`、`SetTickLabelColor`、`SetTickLabelCallback`。

### RangeSlider 属性(继承 Progress)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| low_value | 0 | double | 下限值（浮点） |
| high_value | max | double | 上限值（与基类 value 等价） |
| min_range | 0 | double | 下限与上限之间的最小间距 |
| range_color |  | string | 高亮选中区间颜色（为空回退 progress_color） |
| range_radius | 0 | int | 高亮区间圆角半径（DPI 自适应） |
| click_to_jump | true | bool | 点击轨道是否按下即跳值（false=松开才跳值） |
| thumb_size | 10,10 | size | 两个滑块的统一大小 |
| thumb_normal_image / thumb_hovered_image / thumb_pressed_image / thumb_disabled_image |  | string | 下限滑块各状态图片 |
| high_thumb_normal_image / high_thumb_hovered_image / high_thumb_pressed_image / high_thumb_disabled_image |  | string | 上限滑块各状态图片（为空回退下限滑块图片） |
| tooltip_open | false | bool | 是否始终显示两个滑块的值气泡（false=拖动/悬停时显示，松开后延迟 1 秒消失） |
| tooltip_format / tooltip_format_id |  | string | 值气泡文本格式（如 `%.0f`）/ 其多语言 ID（优先，语言切换自动刷新） |
| tooltip_bkcolor（别名 tooltip_bg_color） | bg_tooltip | string | 值气泡背景色 |
| tooltip_text_color | text_tooltip | string | 值气泡文本颜色 |
| marks |  | string | 刻度列表，语法 `value=label@color\|value=label\|value`，多个用 `\|` 分隔 |
| mark_color（别名 mark_dot_color） | text_disabled | string | 刻度点颜色 |
| mark_label_color | text_muted | string | 刻度标签默认颜色（`marks` 中 `@color` 可覆盖单个标签） |
| mark_dot_size | 6 | int | 刻度点直径（DPI 自适应） |
| mark_label_gap | 6 | int | 刻度标签与轨道之间的距离（DPI 自适应） |
| snap_to_marks | false | bool | 是否把取值吸附到最近的刻度（拖动/点击/SetXXX 均生效） |
| included | true | bool | true 高亮两个滑块之间；false 高亮滑块外侧两段 |

双滑块范围控件：一条轨道上放置下限、上限两个滑块并高亮选中区间，常用于价格/时间区间筛选。上限复用基类值（value），下限独立维护（low_value）；下限钳位到 `[min, high - min_range]`、上限钳位到 `[low + min_range, max]`。
C++ 接口：`GetLowValue/SetLowValue/SetLowValueNoEvent`、`GetHighValue/SetHighValue/SetHighValueNoEvent`、`SetRange/SetRangeNoEvent`、`GetMinRange/SetMinRange`、`SetThumbSize/GetThumbSize`、`GetLowThumbRect/GetHighThumbRect`、`SetThumbStateImage/GetThumbStateImage`、`SetHighThumbStateImage/GetHighThumbStateImage`、`SetRangeColor/GetRangeColor`、`SetRangeRadius/GetRangeRadius`、`SetClickToJump/IsClickToJump`、`SetTooltipOpen/IsTooltipOpen`、`SetTooltipFormat/GetTooltipFormat`、`SetTooltipFormatId`、`SetTooltipBkColor/SetTooltipTextColor`、`SetMarks（vector 或字符串）/GetMarks`、`SetMarkColor/SetMarkLabelColor`、`SetMarkDotSize/SetMarkLabelGap`、`SetSnapToMarks/IsSnapToMarks`、`SetIncluded/IsIncluded`。
事件：`AttachRangeChanged`——范围值变化时触发，事件编码 `kEventValueChanged`（wParam=1），low/high 由 `EventArgs::pEventData`（指向 `RangeSlider::RangeValue`）携带，仅在回调派发期间有效。

### Stepper 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| step_title / step_title_id |  | string | 各步骤标题（多条用 `|` 分隔；`_id` 为多语言 ID，语言切换自动刷新） |
| step_desc / step_desc_id |  | string | 各步骤描述（多条用 `|` 分隔；`_id` 为多语言 ID） |
| current / current_index | 0 | int | 当前步骤索引（0 起） |
| direction / orientation | horizontal | string | 排列方向：horizontal（默认）/ vertical |
| click_to_jump | true | bool | 鼠标点击步骤是否跳转 |
| node_size | 24 | int | 节点直径（像素，DPI 自适应） |
| disabled_steps |  | string | 禁用步骤索引列表（`|` 或 `,` 分隔，如 "2\|3"） |
| node_color_pending / node_color_active / node_color_completed / node_color_disabled | text_muted / color_accent / color_success / border_control_normal | string | 各状态节点颜色 |
| connector_color / connector_active_color | border_control_normal / color_success | string | 未完成段 / 已完成段连接线颜色 |
| title_color_active / title_color_completed / title_color_pending | text_default / color_success / text_muted | string | 各状态标题颜色 |
| desc_color | text_muted | string | 描述文本颜色 |
| title_font / desc_font | system_bold_14 / system_regular_12 | string | 标题 / 描述字体 ID |
| node_hovered_image（别名 node_hot_image） |  | string | 节点悬停态图片，可选；悬停于可点击步骤上时显示（绘制在节点下层） |
| node_pressed_image（别名 node_pushed_image） |  | string | 节点按下态图片，可选；未配置时回退悬停态图片 |

向导步骤条（安装/注册/配置向导、审批流程）：状态由"当前索引 + 禁用集合"派生——索引小于当前者为已完成（节点显示对勾），等于者为进行中，大于者为待开始，被显式禁用者优先显示禁用态；连接线按完成度着色。
悬停/按下态图片仅对可交互步骤（控件启用、click_to_jump 开启且该步非禁用）显示；默认皮肤 `stepper_node` 类内置光晕环图标（颜色经 svg_replace_colors 跟随主题强调色），XML 中 `class="stepper_node"` 即可引用。
C++ 接口：`GetStepCount/GetStepAt/AddStep/InsertStep/RemoveStepAt/ClearSteps/SetSteps`、`GetCurrentIndex/SetCurrentIndex/SetCurrentIndexNoEvent`、`Next/NextNoEvent/Back/BackNoEvent/GoTo/GoToNoEvent`、`SetStepDisabled/IsStepDisabled/SetDisabledSteps/GetStepStatus`、`SetHorizontal/IsHorizontal`、`SetNodeSize/GetNodeSize`、`SetClickToJump/IsClickToJump`、`SetStateColor/SetTitleColor/SetConnectorColor/SetConnectorActiveColor/SetDescColor`、`SetNodeStateImage/GetNodeStateImage`。
事件：`AttachStepChanged`——当前步骤变化时触发，事件编码 `kEventValueChanged`（wParam=2），`currentIndex`/`oldIndex` 由 `EventArgs::pEventData`（指向 `Stepper::StepValue`）携带，仅在回调派发期间有效。

### Rating 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| value | 0 | double | 当前评分值，钳位到 [0, max]，按 allow_half 对齐步长 |
| max（别名 star_count） | 5 | int | 星星数量 |
| allow_half | false | bool | 是否允许半星（步长 0.5，否则 1） |
| read_only（别名 readonly） | false | bool | 只读展示模式，禁止鼠标交互 |
| star_size | 20 | int | 单颗星绘制直径（像素，DPI 自适应） |
| star_gap | 4 | int | 星星间距（像素，DPI 自适应） |
| star_filled_image（别名 star_full_image） |  | string | 实心星图片 |
| star_half_image |  | string | 半星图片（可选，未配置时对实心星做左半裁剪绘制） |
| star_empty_image |  | string | 空心星图片 |
| star_hovered_image（别名 star_hover_image） |  | string | 星悬停态图标，可选（绘制在星形下层，形成外圈光晕 / 描边） |
| star_pressed_image（别名 star_pushed_image） |  | string | 星按下态图标，可选；未配置时回退悬停态图标 |

星级评分（评价/打分/满意度调查）：星形外观完全由 SVG 图片驱动（不做星形自绘），颜色经 svg_replace_colors 映射主题语义色随主题切换；支持只读展示、半星、鼠标悬停预览、点击/拖拽打分。
C++ 接口：`GetValue/SetValue/SetValueNoEvent`、`GetMax/GetStarCount/SetMax`、`IsAllowHalf/SetAllowHalf`、`IsReadOnly/SetReadOnly`、`GetStarSize/SetStarSize`、`GetStarGap/SetStarGap`、`SetStarImage/GetStarImage`、`SetFilledImage/SetHalfImage/SetEmptyImage`、`SetStarStateImage/GetStarStateImage`、`SetHoveredImage/SetPressedImage`、`ClearImageCache`；星图类型枚举 `StarImageType`（kFilled/kHalf/kEmpty）。
事件：`AttachRatingChanged`——评分值变化时触发，事件编码 `kEventValueChanged`，`value`/`oldValue` 由 `EventArgs::pEventData`（指向 `Rating::RatingValue`）携带，仅在回调派发期间有效。
默认皮肤 `rating` 类内置五张图（实心/半星/悬停态/按下态映射 color_accent，空心星映射 border_control_normal），XML 中 `class="rating"` 即可引用。

### Carousel 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| item_image | | string | 幻灯片图片列表，`\|` 分隔的图片属性串（每项如 `file='.../slide1.svg'`） |
| current_index | 0 | int | 当前幻灯片索引（0 起），钳位到 [0, count-1] |
| loop | true | bool | 是否循环（首尾相接，无限滚动） |
| auto_play（别名 autoplay） | false | bool | 是否自动播放 |
| interval | 3000 | int | 自动播放间隔（毫秒） |
| hover_pause | true | bool | 悬停时暂停自动播放 |
| transition_ms | 300 | int | 切换动画时长（毫秒，0 = 立即切换） |
| show_indicator | true | bool | 是否显示页码指示点 |
| indicator_normal_image / indicator_active_image | | string | 指示点常态 / 当前页图片 |
| indicator_size / indicator_gap / indicator_bottom | 8 / 8 / 10 | int | 指示点尺寸 / 间距 / 距内容区底边距离（像素，DPI 自适应） |
| show_nav_button | true | bool | 是否显示左右切换按钮 |
| btn_prev_image / btn_next_image | | string | 上一张 / 下一张按钮图 |
| btn_prev_hovered_image / btn_next_hovered_image | | string | 按钮悬停态图片（可选） |
| btn_prev_pressed_image / btn_next_pressed_image | | string | 按钮按下态图片（可选，回退悬停 / 常态） |
| btn_size / btn_margin | 32 / 12 | int | 切换按钮尺寸 / 距内容区边缘距离（像素，DPI 自适应） |

图片轮播（首页横幅/广告位/图片集）：外观完全由 SVG 图片驱动（幻灯片 + 导航按钮 + 指示点），颜色经 svg_replace_colors 映射主题语义色随主题切换；支持左右切换、可点击页码指示点（当前页高亮）、loop 无限循环、auto_play 自动播放 + hover_pause 悬停暂停、transition_ms 水平滑动动画、鼠标拖拽翻页。
C++ 接口：`GetCurrentIndex/SetCurrentIndex/SetCurrentIndexNoEvent`、`GetItemCount/AddItem/InsertItem/RemoveItem/ClearItems/SetItems/GetItem`、`Next/Prev/GoTo`（含 `NoEvent` 变体）、`IsLoop/SetLoop`、`IsAutoPlay/SetAutoPlay`、`GetInterval/SetInterval`、`IsHoverPause/SetHoverPause`、`GetTransitionMs/SetTransitionMs`。
事件：`AttachCarouselChanged`——当前索引变化时触发，事件编码 `kEventValueChanged`，`index`/`oldIndex` 由 `EventArgs::pEventData`（指向 `Carousel::CarouselValue`）携带，仅在回调派发期间有效。
默认皮肤 `carousel` 类内置导航按钮与指示点图片及语义色映射，XML 中 `class="carousel"` 即可引用。

### Pagination 属性(继承 HBox)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| page_count（别名 total_page） | 0 | int | 总页数 |
| current_page（别名 page） | 1 | int | 当前页码（1 起），钳位到 [1, page_count] |
| total_count | 0 | int | 总记录数（与 page_size 配合推导 page_count，可选） |
| page_size | 0 | int | 每页记录数（可选；total_count + page_size > 0 时优先推导） |
| show_page_size | false | bool | 是否显示“每页条数”选择器（可选功能，默认关闭） |
| page_size_options | 10,20,50,100 | string | 每页条数候选项，逗号或 `\|` 分隔 |
| page_size_format / page_size_format_id | %d | string | 候选项文本格式（含 `%d` 占位符，如 `%d 条/页`）；ID 优先并支持多语言 |
| page_size_class | pagination_page_size | string | 每页条数选择器（Combo）引用的皮肤 Class |
| page_size_width | 112 | int | 每页条数选择器宽度（像素，DPI 自适应；0 = 自适应） |
| max_visible_count | 7 | int | 序号区最多显示的页码按钮数（含首尾），超出用 `…` 收纳 |
| show_prev_next | true | bool | 是否显示上一页 / 下一页 |
| show_first_last | false | bool | 是否显示首页 / 末页按钮 |
| prev_text / next_text | ‹ / › | string | 上一页 / 下一页文本 |
| first_text / last_text | « / » | string | 首页 / 末页文本 |
| ellipsis_text | … | string | 省略号文本 |
| button_class | pagination_button | string | 页码按钮引用的皮肤 Class（含常态/悬停/选中态） |
| button_width / button_height | 32 / 32 | int | 页码按钮尺寸（像素，DPI 自适应；0 = 自适应） |

分页器（报表/表格/列表页码导航）：继承 HBox 全部布局属性；页面按钮由控件内部按数据动态创建（延迟定时器重建，规避子控件自删的 use-after-free）；当前页按钮选中高亮，页码超出上限以 `…` 收纳并保留首尾与当前页附近；颜色走 button_class 引用的皮肤语义色，C++ 无写死色值。
C++ 接口：`GetPageCount/SetPageCount`、`GetCurrentPage/SetCurrentPage/SetCurrentPageNoEvent`、`SetTotalCount`、`Next/Prev/GoTo`、`GetMaxVisibleCount/SetMaxVisibleCount`、`SetButtonClass/GetButtonClass`。
每页条数选择器（可选，默认关闭）：`IsShowPageSize/SetShowPageSize`、`GetPageSize/SetPageSize`（切换后按 total_count 重算页数、重置到第 1 页并派发事件）、`GetPageSizeOptions/SetPageSizeOptions`（vector 或逗号/`|` 分隔字符串）、`SetPageSizeFormat/SetPageSizeFormatId`、`SetPageSizeClass/SetPageSizeWidth`；`page_size` 不在候选项时自动补到首位。
事件：`AttachPageChanged`——当前页码或每页条数变化时触发，事件编码 `kEventValueChanged`，`page`/`oldPage`/`pageSize`/`oldPageSize` 由 `EventArgs::pEventData`（指向 `Pagination::PaginationValue`）携带，仅在回调派发期间有效。

### Breadcrumb 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| item_text | 空 | string | 条目文本，`\|` 分隔多条 |
| item_text_id | 空 | string | 条目文本的多语言 ID，`\|` 分隔多条；语言切换自动刷新 |
| current（别名 current_index） | -1 | int | 当前项索引（0 起）；-1 = 自动跟随最后一项 |
| separator（别名 separator_text） | › | string | 条目之间的分隔符 |
| spacing | 4 | int | 分隔符两侧间距（像素，DPI 自适应） |
| item_padding | 6 | int | 条目点击区域左右内边距（像素，DPI 自适应） |
| item_round | 4 | int | 悬停/按下胶囊背景圆角（像素，DPI 自适应） |
| click_to_jump | true | bool | 是否允许点击祖先项切换当前项 |
| disabled_items | 空 | string | 禁用项索引列表，`\|` 或 `,` 分隔 |
| item_color / current_color / hovered_color / pressed_color / disabled_color | color_blue / text_default / color_blue_light / color_blue / text_disabled | string | 各状态文本颜色（语义色名） |
| separator_color | color_gray | string | 分隔符颜色 |
| hover_bg_color / pressed_bg_color | bg_btn_hovered / bg_btn_pressed | string | 悬停/按下胶囊背景色（留空则不绘制） |
| font / current_font | system_regular_14 / system_bold_14 | string | 普通条目字体 / 当前项加粗字体 |

面包屑导航（文件/目录路径、站点层级、设置页层级）：纯自绘，一行显示可点击的祖先路径 + 当前项；祖先项默认链接蓝、当前项加粗，悬停/按下绘制圆角胶囊背景（`hover_bg_color`/`pressed_bg_color`，可留空关闭）；状态颜色优先级为 禁用 > 按下 > 悬停 > 当前 > 普通；宽度不足时右对齐裁剪，保证末项/当前项可见。
C++ 接口：`AddItem/InsertItem/RemoveItemAt/ClearItems/SetItems/GetItemCount/GetItemAt/GetItems`、`GetCurrentIndex/SetCurrentIndex/SetCurrentIndexNoEvent/GoTo/GoToNoEvent`、`SetItemEnabled/IsItemEnabled/SetDisabledItems/GetDisabledItems`、`SetSeparator/GetSeparator`、`SetSpacing/GetSpacing`、`SetItemPadding/GetItemPadding`、`SetItemRound/GetItemRound`、`SetClickToJump/IsClickToJump`、各颜色 Set/Get、`SetHoveredBgColor/SetPressedBgColor`、`SetFont/GetFont`、`SetCurrentFont/GetCurrentFont`。
事件：`AttachCurrentChanged`——当前项变化时触发，事件编码 `kEventValueChanged`（`wParam=3`），`currentIndex`/`oldIndex` 由 `EventArgs::pEventData`（指向 `ui::BreadcrumbValue`）携带，仅在回调派发期间有效。
默认皮肤 `breadcrumb` 类只定义颜色与字体（祖先项链接蓝、当前项加粗、分隔符灰色、悬停/按下胶囊背景），无图片资源。

### CascadingSelect 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| items | 空 | string | 字面路径列表，`\|` 分隔多条路径，路径内用 `path_separator` 分隔层级 |
| items_id | 空 | string | 多语言路径列表，格式同 `items`，每段为语言 ID；语言切换自动刷新 |
| path_separator | / | string | 路径层级分隔符 |
| value_separator | " / " | string | `GetSelectionText()` 使用的连接符 |
| column_width | 0 | int | 固定列宽（像素，DPI 自适应）；0 = 按控件宽度等分 |
| row_height | 28 | int | 行高（像素，DPI 自适应） |
| text_padding | 10 | int | 行内文本左右内边距（像素，DPI 自适应） |
| disabled_paths | 空 | string | 禁用路径列表，`\|` 分隔；命中节点文本路径 / ID 路径或其前缀时禁用整棵子树 |
| item_color / selected_color / disabled_color | text_default / color_accent / text_disabled | string | 普通 / 选中 / 禁用行文本颜色（语义色名） |
| hover_bg_color / selected_bg_color | bg_list_item_hovered / bg_list_item_selected | string | 悬停 / 选中行背景色（留空则不绘制） |
| divider_color / chevron_color / scrollbar_color | border_split_level1 / color_gray / color_gray | string | 列分隔线 / 父项箭头 / 滚动条颜色 |
| font | system_regular_14 | string | 文本字体 ID |
| dropdown | false | bool | true = 下拉列表框形态（选择框 + Flyout 浮层）；false = 内嵌多列 |
| placeholder / placeholder_id | 空 | string | 未选择时的提示文本（字面 / 多语言 ID，ID 优先） |
| allow_clear | true | bool | 悬停且已有选择时是否显示清空按钮（×） |
| expand_on_hover | false | bool | 悬停父项时自动展开子列（只展开，不提交） |
| popup_width | 0 | int | 浮层宽度（像素）；0 = 按列数 × 列宽自动计算，显式值不足时自动放宽到可容纳全部列 |
| popup_max_rows | 8 | int | 浮层最多可见行数，超出时列内滚动 |
| popup_class | cascading_select_popup | string | 浮层内级联列表使用的样式 Class |
| text_color / placeholder_color / disabled_text_color | text_default / text_muted / text_disabled | string | 选择框已选 / 提示 / 禁用文本颜色 |
| bg_color / border_color / hover_border_color | bg_window_card / border_control_normal / border_btn_hovered | string | 选择框背景、边框、悬停（或展开）边框颜色 |
| clear_color / clear_hover_color | color_gray / text_default | string | 清空按钮颜色 / 悬停颜色 |
| border_round / border_size | 4 / 1 | int | 选择框圆角与边框宽度（像素） |

级联选择（地区、分类、组织层级等）：支持内嵌多列与下拉列表框两种形态。内嵌形态纯自绘，列数 = 数据最大深度；点击某列后子列内容立即过滤为该节点子集，深层旧选择立即重置。未选满前缀的列留空；有子项的节点行右侧绘制矢量箭头。禁用优先于选中/悬停；列溢出时在该列滚轮可滚动（每格 3 行），右缘绘制比例滚动条。下拉形态点击选择框用 `Flyout` 弹出同一套列表，支持提示文本、悬停清空、移入展开、自定义显示文本与浮层尺寸估算；暂不支持键盘导航。
C++ 接口：`SetItems/SetItemIds/AddPath/ClearItems/GetDepth/GetItemCount/GetItemText`、`GetSelection/SetSelection/SetSelectionByText/ClearSelection/GetSelectionText/GetSelectionPath`、`SetDisabledPaths/GetDisabledPaths/IsItemDisabled`、`SetPathSeparator/GetPathSeparator`、`SetValueSeparator/GetValueSeparator`、`SetColumnWidth/GetColumnWidth`、`SetRowHeight/GetRowHeight`、`SetTextPadding/GetTextPadding`、各颜色 Set/Get、`SetFont/GetFont`；下拉形态另有 `SetDropdown/IsDropdown`、`SetPlaceholder/SetPlaceholderId/GetPlaceholder`、`SetExpandOnHover/IsExpandOnHover`、`SetAllowClear/IsAllowClear`、`SetPopupWidth/GetPopupWidth`、`SetPopupMaxRows/GetPopupMaxRows`、`SetPopupClass/GetPopupClass`、`AttachDisplayText/GetDisplayText` 及选择框样式 Set/Get。
事件：`AttachSelectionChanged`——选择实际变化时触发一次，事件编码 `kEventValueChanged`（`wParam=4`），`indexes`/`text`/`path` 由 `EventArgs::pEventData`（指向 `ui::CascadingSelectValue`）携带，仅在回调派发期间有效；`SetSelection(..., false)` / `ClearSelection(false)` 不派发。
默认皮肤 `cascading_select`（内嵌）/ `cascader`（下拉）/ `cascading_select_popup`（浮层列表）类只定义颜色、字体与尺寸，无图片资源。

### TreeGrid 属性(继承 VBox)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| column_title | | string | 列标题列表，`\|` 分隔（每项可为多语言 ID 或字面文本） |
| column_width | | string | 列宽列表，`\|` 分隔（原始像素值） |
| column_id | | string | 列 ID 列表，`\|` 分隔（缺省按列索引从 0 起） |
| column_align | | string | 列对齐列表，`\|` 分隔：left / center / right |
| default_column_width | 120 | int | 未显式指定列宽时使用的默认列宽（原始像素） |
| tree_column | 0 | int | 承载缩进与展开开关的树列索引 |
| indent | 20 | int | 每一层级的缩进量（原始像素） |
| header_height | 30 | int | 表头高度（原始像素） |
| row_height | 28 | int | 数据行高（原始像素） |
| show_header | true | bool | 是否显示表头 |
| multi_select | false | bool | 是否允许选择多行 |
| check_mode | false | bool | 是否在行首显示复选框 |
| check_cascade | true | bool | 父子复选框是否级联（关闭时仅改自身） |
| expand_image_class | tree_grid_expand | string | 展开 / 收起开关引用的皮肤 Class（两态图片） |
| check_image_class | tree_grid_check | string | 行复选框引用的皮肤 Class（三态图片） |
| header_cell_class | tree_grid_header_cell | string | 表头标题单元格引用的皮肤 Class |
| cell_class | tree_grid_cell | string | 数据单元格引用的皮肤 Class |
| row_class | tree_grid_row | string | 数据行引用的皮肤 Class |

树表格（文件树 / 报表 / 组织架构）：在同一控件内呈现"可展开/收起的层级行 + 多列表格"。控件继承 VBox，内部由固定不滚动的表头 HBox 与自带纵向滚动条的列表体 VListBox 组成；表头与行复用同一列宽数组，天然逐列对齐；展开 / 收起通过切换子孙行的可见性实现（隐藏行不占空间）。缩进用"树列单元格内嵌缩进占位"而非整行 padding，故各列不会错位。列标题支持多语言 ID（优先按 LanguageManager 解析，未命中则用字面文本）。
C++ 接口（列）：`AddColumn/GetColumnCount/GetColumn/SetColumnWidthById/GetColumnWidthById/SetColumnTitleById/ClearColumns`。
C++ 接口（节点与数据）：`GetRootNode/AddNode/RemoveNode/RemoveAllNodes`、`SetCellText/GetCellText`、`SetNodeRowHeight`；`TreeGridNode` 提供 `AddChildNode/RemoveChildNode/RemoveAllChildNodes/GetChildNodeCount/GetChildNode/GetChildNodeIndex/GetDescendantNodeCount/IsChildOf`、`GetParentNode/GetDepth`、`IsExpand/SetExpand`、`IsChecked/SetChecked/GetCheckState`、`IsSelected/SetSelected`、`SetCellText/GetCellText`、`SetUserData/GetUserData`。
C++ 接口（展开）：`SetNodeExpand/IsNodeExpand/ExpandNode/EnsureNodeVisible`。
C++ 接口（选择）：`SetMultiSelect/IsMultiSelect/GetSelectNodeCount/GetSelectNodeAt/GetSelectNodes/IsNodeSelected/SelectNode/SelectAllNodes/ClearSelectNodes`。
C++ 接口（复选）：`IsCheckMode/SetCheckMode/IsCheckCascade/SetCheckCascade/IsNodeChecked/SetNodeChecked/GetNodeCheckState/SetAllNodesChecked/GetCheckedNodes`。
交互：点击展开开关切换展开并派发展开 / 收起事件；点击行复选框做级联勾选（不改变行选择）；点击行选择该行并派发点击事件；键盘 Left / Right 收起 / 展开当前选中节点（沿用 ListBox 方向键导航）。
事件：`AttachNodeExpanded`/`AttachNodeCollapsed`（事件编码 `kEventExpand`/`kEventCollapse`）、`AttachNodeClick`（`kEventClick`）、`AttachNodeSelect`/`AttachNodeUnSelect`/`AttachSelectChanged`（`kEventSelect`/`kEventUnSelect`/`kEventSelChanged`）——`EventArgs::pEventData` 均为 `TreeGridNode*`；`AttachCheckChanged`——事件编码 `kEventTreeGridCheckChanged`，`EventArgs::pEventData` 指向 `TreeGridCheckValue`（`pNode` + `bChecked`），仅在回调派发期间有效。
默认皮肤 `tree_grid` 与 `tree_grid_header_cell`/`tree_grid_cell`/`tree_grid_row`/`tree_grid_expand`/`tree_grid_check` 类均走语义色，展开箭头与三态复选框复用 `${PATH_TREE}` / `${PATH_CHECKBOX}` 下的既有 SVG。

### CircleProgress 属性(继承 Progress)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| circular | true | bool | 是否环形 |
| circle_width | 1 | int | 圆环宽度 |
| indicator | | string | 进度指示图标（随进度旋转） |
| clockwise | true | bool | 顺时针 |
| bgcolor | | string | 背景（轨道）颜色 |
| fgcolor | | string | 前景（进度弧）颜色 |
| gradient_color | | string | 前景渐变颜色 |
| indeterminate | false | bool | 不确定进度（忽略 value，一段弧持续旋转） |
| start_angle | 270 | float | 圆弧起始角度（度），0 为 12 点方向，顺时针递增；360 归一化为 0 |
| span_angle | 360 | float | 圆弧总跨度（度），取值 (0, 360]；小于 360 即为缺口环/仪表盘 |
| indeterminate_arc | 90 | float | 不确定进度模式的旋转弧长（度） |
| indeterminate_speed | 6 | float | 不确定进度模式每次刷新前进的角度（度） |
| indeterminate_interval | 16 | int | 不确定进度模式刷新间隔（毫秒），默认约 60 帧/秒 |
| center_ratio | 100 | float | 中心内容区占控件百分比（0~100），XML 中按百分比填写（如 70 表示 70%） |
| center_padding | 0,0,0,0 | rect | 中心内容区内边距，如 "4,4,4,4" |

**仪表盘 / 缺口环**：前景弧按 `(value-min)/(max-min)*span_angle` 扫过，背景弧始终绘制完整 `span_angle`（缺口固定呈轨道色）；
非整圆时**圆心按弧的包围盒自动居中**（采样起止角+途经 0/90/180/270 象限极值），因此 180° 半圆表盘的上半弧能完整落在控件内不被裁剪；
`start_angle`/`span_angle` 对背景弧、前景弧、indicator 同时生效。

**中心自定义内容**：`SetCenterCallback([pCtrl](IRender*, const UiRect& rcCenter, double fValue) -> bool)` 接管中心绘制（指针/图标/组合数字）。
`rcCenter` 为控件内相对坐标矩形，由 `center_ratio` + `center_padding` 决定；不设回调时行为不变（中心文本仍由基类 `text`/`show_percent` 绘制，不会重复叠加）。
需要放真实子控件时叠加居中的子控件即可。

**indeterminate 参数化**：`indeterminate_arc`/`indeterminate_speed`/`indeterminate_interval` 运行期修改会自动重启定时器，无需重设 `indeterminate`。

C++ 接口：`SetStartAngle/GetStartAngle`、`SetSpanAngle/GetSpanAngle`、`SetIndeterminateArc/GetIndeterminateArc`、
`SetIndeterminateSpeed/GetIndeterminateSpeed`、`SetIndeterminateInterval/GetIndeterminateInterval`、
`SetCenterRatio/GetCenterRatio`、`SetCenterPadding/GetCenterPadding`、`SetCenterCallback`、`GetCenterRect()`。

### Combo 属性(继承 Box)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| combo_type | "drop_down" | string | "drop_down"(可编辑) / "drop_list"(不可编辑) |
| dropbox_size | | string | 下拉列表尺寸 |
| popup_top | false | bool | 向上弹出 |

### DateTime 属性(继承 Label，日期时间选择器)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| format | | string | 日期格式，具体可参考 `DateTime.h` 中 `SetStringFormat` 的说明（基于 `std::put_time`） |
| edit_format | "date_calendar" | string | 编辑格式：`date_calendar` / `date_time_calendar` / `time_calendar` / `date_up_down` / `date_time_up_down` / `date_minute_up_down` / `time_up_down` / `minute_up_down`。`date_time_calendar`=日历+时间弹层，`time_calendar`=仅时间弹层，两者点"确定"统一提交 |
| spin_class | | string | Spin 控件的 Class 属性（容器,上按钮,下按钮），仅 `*_up_down` 编辑格式生效 |
| min_date / max_date | | string | 可选日期范围（yyyy-mm-dd，空=不限），仅日历类格式生效 |
| disabled_dates | | string | 业务禁用日期，`,`/`;` 分隔；每项 `yyyy-mm-dd` 或 `yyyy-mm-dd~yyyy-mm-dd`；键盘焦点会跳过禁用日 |
| first_day_of_week | "monday" | string | 每周第一天：`monday` / `sunday` / 0~6（0=周日） |
| show_seconds | "true" | bool | 时间弹层是否显示秒；false 时格式为 `%H:%M`，仅 `date_time_calendar` / `time_calendar` 生效 |
| current_time | | bool | 初始化为当前本地时间（`"true"` 或 `"1"` 时生效），无需在代码中手动调用 `InitLocalTime()`；与 `text` 同时使用时 `current_time` 优先 |

C++ 接口：`InitLocalTime()`（设为当前时间）、`ClearTime()`、`SetDateTime/GetDateTime(tm)`、`SetDateTimeString/GetDateTimeString()`、`SetStringFormat/GetStringFormat()`、`SetEditFormat/GetEditFormat()`、`SetSpinClass/GetSpinClass()`、`SetDateLimit/GetMinDate/GetMaxDate()`、`SetDisabledDates/GetDisabledDates()`、`SetFirstDayOfWeek/GetFirstDayOfWeek()`、`SetShowSeconds/IsShowSeconds()`。值变化触发 `kEventValueChanged`。

## 四、全局资源 (global.xml)

### 字体定义

`<Font>` 命名规则为 `<字体类型>_<样式>_<字号>`，可选样式：regular / bold / underline / italic /
strikeout / fullstyle（四者全开），字号支持 12/14/16/18/20/22。`default="true"` 的字体为默认字体。

```xml
<DefaultFontFamilyNames windows="Microsoft YaHei, SimSun" macos="PingFang SC" linux="Noto Sans CJK SC"/>
<Font id="system_regular_14" name="system" size="14" default="true"/>   <!-- 默认字体 -->
<Font id="system_bold_14" name="system" size="14" bold="true"/>
<FontFile file="fonts/RobotoMono-Regular.ttf" desc="Roboto Mono常规"/>  <!-- 字体文件放在 resources/fonts/ -->
```

旧 ID（`system_12` ~ `system_22`，无样式段）仍保留作兼容别名，新代码请用 `system_<样式>_<字号>`。
**不存在 `arial_*` 系列。**

### 颜色定义

颜色**不要**在 `themes/default/global.xml` 里写死数值：该文件的 `<ThemeColor>` 已迁移到
`bin/resources/themes/color_light/global.xml`（浅色）与 `color_dark/global.xml`（深色），
由框架在运行时按当前主题加载。`themes/default/global.xml` 中只剩下**旧名→新名的 `<Alias>` 映射**。

```xml
<!-- 自定义颜色：写在 bin/resources/themes/color_light/global.xml 的 <Global> 内 -->
<ThemeColor name="my_brand_color" value="#FF1890FF" type="common"
            category="bg_color" role="neutral" comment_cn="品牌色"/>
```

颜色格式: "#AARRGGBB"(ARGB) 或 "#RRGGBB"(RGB) 或颜色名(Blue/Red/White...)

常用语义色（浅色主题实际取值，取自 `color_light/global.xml`）与兼容别名:

| global.xml 语义色名 | 浅色取值 | 兼容别名（旧名） | 用途 |
|------|------|------|------|
| bg_window_main | #FFF4F4F4 | bk_wnd_darkcolor | 窗口主背景 |
| bg_container | #FFF9F9F9 | bk_wnd_lightcolor | 容器背景 |
| bg_titlebar | #FFEAEAEA | - | 标题栏背景 |
| bg_list_item_hovered | #FFEAEAEA | bk_listitem_hovered | 列表项悬浮 |
| bg_list_item_selected | #FFE2E2E2 | bk_listitem_selected | 列表项选中 |
| bg_menu_item_hovered | #FFEBEBEB | bk_menuitem_hovered | 菜单项悬浮 |
| text_default | #FF1A1A1A | default_font_color | 主文本 |
| text_disabled | #B3343434 | disabled_font_color | 禁用文本 |
| border_split_level1 | #FFE7E7E7 | splitline_level1 | 分割线 |

完整的颜色清单与命名规范见仓库 `docs/ThemeColor.md`。深色主题取值不同，
**写业务 XML 时一律用语义色名，不要写死色值**。

### 通用样式(Class)

```xml
<Class name="btn_default" font="system_12" normal_text_color="white"
       normal_image="file='btn_normal.png'"
       hot_image="file='btn_hot.png'"
       pushed_image="file='btn_pushed.png'"/>
```

使用: `<Button class="btn_default" text="Click"/>`
**class属性必须写在所有属性最前面**

## 五、图片属性

```xml
<!-- 简单用法 -->
<Control bkimage="logo.png"/>

<!-- 完整属性 -->
<Control bkimage="file='icon.svg' width='24' height='24' valign='center' halign='center'"/>
```

| 属性 | 类型 | 说明 |
|------|------|------|
| file | string | 图片路径(相对于主题目录) |
| width / height | string | 图片尺寸(像素或百分比) |
| src | rect | 源区域裁剪 "L,T,R,B" |
| dest | rect | 目标绘制区域 |
| corner | rect | 九宫格参数 "L,T,R,B" |
| fade | int | 透明度 0-255 |
| halign / valign | string | 对齐方式 |
| xtiled / ytiled | bool | 平铺绘制 |
| auto_play | bool | 动画自动播放 |
| play_count | int | 动画播放次数(-1无限) |

支持格式: PNG, SVG, JPG, GIF, BMP, APNG, WEBP, ICO, Lottie-JSON, PAG

## 六、XML 事件系统

### XML内联事件

```xml
<Button name="my_btn" text="Click">
  <Event type="click" receiver="target_control_name" apply_attribute="visible='true'"/>
</Button>

<!-- 事件目标类型 -->
<!-- receiver="name"           按名称查找控件 -->
<!-- receiver="./name"         在当前容器内查找 -->
<!-- receiver=""               控件自身 -->
<!-- receiver="#window#"       窗口 -->
```

### 常用事件类型

| type值 | 说明 |
|--------|------|
| click | 点击 |
| rclick | 右键点击 |
| mouse_enter / mouse_leave | 鼠标进入/离开 |
| mouse_button_down / mouse_button_up | 鼠标按下/释放 |
| mouse_double_click | 双击 |
| select / unselect | 选中/取消选中(ListBox/Combo) |
| check / uncheck | 勾选/取消勾选(CheckBox) |
| tab_select | 标签页切换 |
| text_changed | 文本变化 |
| value_changed | 值变化 |
| expand / collapse | Panel 面板展开/折叠完成 |
| panel_expanding / panel_collapsing | Panel 即将展开/折叠（C++ 回调返回 false 可取消；XML 内联仅用于 apply_attribute） |
| navigation_item_click | NavigationView 导航项被点击（重复点击已选中项也触发） |
| navigation_selection_changed | NavigationView 选中项变化（wParam 新序号、lParam 旧序号） |
| navigation_pane_toggling / navigation_pane_toggled | NavigationView 窗格即将收起/展开 / 完成（前者回调返回 false 可取消） |
| key_down / key_up | 按键 |
| return | 回车 |
| visible_changed | 可见性变化 |
| window_close | 窗口关闭 |
| window_size | 窗口大小变化 |

## 七、C++ 代码模式

### 窗口类模板

```cpp
// MyForm.h
#ifndef MY_FORM_H_
#define MY_FORM_H_
#include "duilib/duilib.h"

// WindowImplBase 声明在 duilib/Utils/WinImplBase.h（文件名不带 Window 前缀）
class MyForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MyForm();
    virtual ~MyForm() override;
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
};
#endif

// MyForm.cpp
#include "MyForm.h"

MyForm::MyForm() {}
MyForm::~MyForm() {}

DString MyForm::GetSkinFolder() { return _T("my_skin"); }
DString MyForm::GetSkinFile() { return _T("my_form.xml"); }

void MyForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    // 在这里初始化控件和绑定事件
}
```

### 主线程模板

```cpp
// MainThread.h
#include "duilib/duilib.h"
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;
private:
    // 注意：FrameworkThread::OnInit() 返回 bool（FrameworkThread.h:122），不是 void
    virtual bool OnInit() override;
    virtual void OnCleanup() override;
};

// MainThread.cpp
#include "MainThread.h"
#include "MyForm.h"

MainThread::MainThread() : FrameworkThread(_T("MainThread"), ui::kThreadUI) {}
MainThread::~MainThread() {}

bool MainThread::OnInit()
{
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath();
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    MyForm* window = new MyForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MyApp"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}
```

### 入口函数(Windows)

```cpp
#include "MainThread.h"
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    MainThread thread;
    thread.RunMessageLoop();
    return 0;
}
```

### 控件操作

```cpp
// 查找控件
ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("my_button")));
ui::Label* label = dynamic_cast<ui::Label*>(FindControl(_T("my_label")));
ui::RichEdit* edit = dynamic_cast<ui::RichEdit*>(FindControl(_T("my_edit")));
ui::CheckBox* check = dynamic_cast<ui::CheckBox*>(FindControl(_T("my_check")));
ui::Combo* combo = dynamic_cast<ui::Combo*>(FindControl(_T("my_combo")));
ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
ui::Progress* progress = dynamic_cast<ui::Progress*>(FindControl(_T("my_progress")));

// 设置属性
label->SetText(_T("Hello"));
edit->SetText(_T("Input"));
DString text = edit->GetText();
check->SetSelected(true);
bool isChecked = check->IsSelected();
progress->SetValue(50);

// 可见性
btn->SetVisible(true);
btn->SetEnabled(false);
```

### 事件绑定

```cpp
void MyForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 按钮点击
    ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
    if (btn) {
        btn->AttachClick([this](const ui::EventArgs& args) {
            // 处理逻辑
            return true;
        });
    }

    // 复选框状态变化
    ui::CheckBox* check = dynamic_cast<ui::CheckBox*>(FindControl(_T("my_check")));
    if (check) {
        check->AttachSelect([this](const ui::EventArgs& args) {
            // 选中
            return true;
        });
        check->AttachUnSelect([this](const ui::EventArgs& args) {
            // 取消选中
            return true;
        });
    }

    // 文本变化
    ui::RichEdit* edit = dynamic_cast<ui::RichEdit*>(FindControl(_T("my_edit")));
    if (edit) {
        edit->AttachTextChanged([this](const ui::EventArgs& args) {
            // 文字改变
            return true;
        });
    }

    // 列表选择
    ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
    if (list) {
        list->AttachSelect([this](const ui::EventArgs& args) {
            // ListBox::AttachSelect：wParam 是新选中项索引，lParam 是 Box::InvalidIndex（占位符，不是旧索引）
            const size_t newIndex = static_cast<size_t>(args.wParam); // wParam 实际类型是 WPARAM
            // args.lParam 当前为 Box::InvalidIndex，请勿当旧索引使用
            return true;
        });
    }

    // 通用事件绑定
    btn->AttachEvent(ui::kEventMouseEnter, [](const ui::EventArgs&) {
        return true;
    });
}
```

### Panel 面板折叠/展开

```cpp
// 头文件 duilib/Box/Panel.h；节点 Panel / PanelHBox / PanelVBox 对应三个 C++ 类
ui::PanelVBox* panel = dynamic_cast<ui::PanelVBox*>(FindControl(_T("my_panel")));
if (panel != nullptr) {
    panel->SetCollapseAnimMillSeconds(220);

    // 切换折叠：bFireEvent 是否触发事件，bPlayAnim=false 立即切换不播动画
    panel->SetCollapsed(!panel->IsCollapsed(), true, true);

    // 完成事件（无动画立即触发，有动画在动画结束后触发）
    panel->AttachExpand([](const ui::EventArgs&) { return true; });
    panel->AttachCollapse([](const ui::EventArgs&) { return true; });

    // 即将折叠：返回 false 取消本次操作（展开不受限时同样可监听 AttachExpanding）
    panel->AttachCollapsing([](const ui::EventArgs&) {
        return false; // true 放行，false 取消
    });
}
```

手风琴：多个 Panel 设置相同 `group` 属性后，展开其中一个会自动折叠同窗口同组的其他面板；
初始展开哪个由 XML 的 `collapsed` 决定，允许全部折叠。

### NavigationView 侧边栏导航

```cpp
// 头文件 duilib/Control/NavigationView.h
ui::NavigationView* nav = dynamic_cast<ui::NavigationView*>(FindControl(_T("nav")));
if (nav != nullptr) {
    // 编程式切换页面（按导航项 page 属性对应的 name，或可选项序号）
    nav->SelectItem(_T("page_tools"));
    nav->SelectItemByIndex(0);

    // 动态追加导航项：两种方式——
    //  a) 内容页已存在于内容区 TabBox，仅加导航项：
    ui::NavigationViewItem* pItem = nav->AddNavItem(
        _T("STRID_NAV_NEW"),
        _T("file='nav/star.svg' width='16' height='16' svg_replace_colors='#333333|border_svg_image'"),
        _T("page_new"));
    //  b) 同时新建内容页（推荐）：第 4 参传页面控件，自动挂到内容区 TabBox 并按 name 联动，
    //     避免"只加了左侧节点、点击后右侧空白"
    ui::VBox* pPage = new ui::VBox(nav->GetWindow());
    pPage->SetBkColor(_T("bg_window_main"));
    pPage->AddItem(new ui::Label(nav->GetWindow()));   // 填充页内内容…
    ui::NavigationViewItem* pItem2 = nav->AddNavItem(
        _T("STRID_NAV_NEW2"), _T("file='nav/star.svg' width='16' height='16'"),
        _T("page_new2"), pPage);
    // 也可单独加页：nav->AddPage(pPage);

    // 窗格收起/展开（AttachNavPaneToggling 回调返回 false 可阻止）
    nav->SetCollapsed(!nav->IsCollapsed());

    // 项管理：禁用/启用、移除（第 2 参 true 一并移除关联内容页）、清空
    nav->SetItemEnabled(pItem, false);
    nav->RemoveNavItem(pItem);          // 仅移除导航项，保留内容页
    nav->RemoveNavItem(pItem2, true);   // 连内容页一起移除
    // nav->RemoveAllNavItems();

    // 选中变化：wParam 新序号、lParam 旧序号（无选中为 ui::Box::InvalidIndex）
    nav->AttachNavSelectionChanged([](const ui::EventArgs& args) {
        size_t nNew = (size_t)args.wParam;
        return true;
    });
    // 点击项（重复点击已选中项也触发）；另有 AttachNavPaneToggled 监听切换完成
    nav->AttachNavItemClick([](const ui::EventArgs&) { return true; });
}
```

内容区推荐显式提供一个 `<TabBox name="nav_content">` 子节点承载各页面，导航项的 `page`
属性与页面控件的 `name` 一一对应；完整示例见 `examples/controls/NavigationForm.cpp`。

### ListBox 动态添加项

```cpp
ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
for (int i = 0; i < 100; i++) {
    ui::ListBoxItem* item = new ui::ListBoxItem(this);
    item->SetText(ui::StringUtil::Printf(_T("Item %d"), i));
    item->SetClass(_T("list_item"));   // 真名 list_item；listitem 是 <Alias> 旧名
    item->SetFixedHeight(ui::UiFixedInt(20), true, true);
    list->AddItem(item);
}
```

### TreeView 动态添加节点

```cpp
ui::TreeView* tree = dynamic_cast<ui::TreeView*>(FindControl(_T("my_tree")));
ui::TreeNode* root = tree->GetRootNode();
ui::TreeNode* node = new ui::TreeNode(this);
node->SetClass(_T("tree_node"));
node->SetText(_T("New Node"));
root->AddChildNode(node);
```

### Combo 动态添加选项

```cpp
ui::Combo* combo = dynamic_cast<ui::Combo*>(FindControl(_T("my_combo")));
ui::TreeView* treeView = combo->GetTreeView();
ui::TreeNode* treeNode = treeView->GetRootNode();
for (int i = 0; i < 10; i++) {
    ui::TreeNode* node = new ui::TreeNode(this);
    node->SetClass(_T("tree_node"));
    node->SetText(ui::StringUtil::Printf(_T("Option %d"), i));
    treeNode->AddChildNode(node);
}
combo->SetCurSel(0); // 默认选中第一项
```

### 线程间通信

```cpp
// 投递任务到工作线程
ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadWorker,
    UiBind(&MyForm::DoBackgroundWork, this));

// 投递任务到UI线程
ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
    UiBind(&MyForm::UpdateUI, this));

// 定时重复任务
ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadWorker,
    ui::UiBind(this, [this]() {
        // 每次执行的逻辑
    }),
    200 // 毫秒间隔
);
```
### ControlLoading 与骨架屏（所有控件通用）

所有控件都继承 `Control`，都支持 `loading` 属性 + `StartLoading` / `StopLoading`，可用于列表、表格、卡片区域的加载占位。

**loading 属性的 9 个子属性**（写在 XML 里，注意 `InitAttribute` 末尾是 `else { ASSERT(0); }`，写错属性名 debug 版直接断言）：

```xml
loading="file='loading.xml' width='0' height='0' offset_x='-1' offset_y='-1'
         valign='center' halign='center' fade='255'
         animation_control='loading_animation' auto_stop='true'"
```

| 子属性 | 说明 |
| :--- | :--- |
| `file` | **必填**，loading 皮肤 XML 路径。为空则 `StartLoading` 返回 false |
| `width` / `height` | ≤0 时按皮肤的 fixed 值或 `EstimateSize()` 自适应 |
| `offset_x` / `offset_y` | 优先于 align；必须落在 `[0, 控件尺寸)` 内才生效，否则回退到 align |
| `halign` | `left` / `center` / `right`（默认 center） |
| `valign` | `top` / `center` / `bottom`（默认 center） |
| `fade` | 整体透明度 0-255 |
| `auto_stop` | 动画播完自动停止（**仅当 `animation_control` 指向的控件存在时才生效**） |
| `animation_control` | 动画控件名，用于取 `m_pAnimationControl` |

**骨架屏（skeleton）：灰白占位块，结构已知时提前呈现版式。** 项目已有两套皮肤可直接用：
- `bin/resources/themes/default/list_ctrl/loading_skeleton_list.xml`（12 行列表 + 工具栏占位）
- `bin/resources/themes/default/list_ctrl/loading_skeleton_card.xml`（3 列 × 2 行卡片）

**写骨架屏必须遵守的四条（都踩过坑）**：

1. **根容器必须有不透明 `bkcolor`。** loading 层是**叠加绘制不是遮罩** —— `CreateBox` 建的独立 Box
   不进控件树，靠 `Control::PaintLoading` → `AlphaPaint` 画在目标控件之上。不画底色，下方旧内容会透出来。
2. **尺寸**：`width='0' height='0'` + 根容器 `height="stretch" width="stretch"` 即可铺满目标控件（含表头）。
   机制是 `stretch` 既非 `IsAuto()` 也非 `IsInt32()` → 尺寸算成 0 → 被 clamp 补成控件全宽。
3. **必须业务侧超时兜底。** 骨架屏是静态占位，没有"动画播完"的时刻，所以**不要设** `animation_control`
   和 `auto_stop`，改为在 `kEventLoading` 里判 `pStatus->m_nElapsedTimeMs >= 5000` 后置 `m_bStopLoading = true`。
   注意 `StartLoading(nIntervalMs, nMaxCount)` 的 `nMaxCount` 是**回调次数上限，不是超时时间**。
4. **呼吸动画不能给根容器设 `SetAlpha`。** `SetAlpha` 作用于整个子树，根容器 `bkcolor` 是唯一遮罩，
   alpha 一降就失效。正确做法：根容器固定 255，只遍历 `GetItemCount()` / `GetItemAt(i)` 给子控件设 alpha
   （`Box` **没有** `GetChildCount()` / `GetChildAt()`）。

**XML 两条额外注意**：
- `valign` 只能填 `top`/`center`/`bottom`，`vcenter` 只在 `text_align` 里有效，填错 debug 版断言崩溃。
- 同层级兄弟要么统一 `height="stretch"`，要么都别混用 `auto`，否则留白不对称。行间距用 margin。

**最小可用代码**：

```cpp
pListCtrl->SetLoadingAttribute(_T("file='loading_skeleton_list.xml' width='0' height='0' "
    _T("offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255'")));
pListCtrl->AttachLoading([pListCtrl](const ui::EventArgs& args) {
    ui::ControlLoadingStatus* pStatus = (ui::ControlLoadingStatus*)args.wParam;
    if (pStatus == nullptr) { return false; }
    if (pStatus->m_nElapsedTimeMs >= 5000) {   // 超时兜底
        pStatus->m_bStopLoading = true;
        return true;
    }
    ui::ControlPtrT<ui::Box> pRootBox = pStatus->m_pLoadingUiRootBox;
    const uint8_t nAlpha = (uint8_t)(197.5 + 57.5 * std::sin(
        (double)pStatus->m_nElapsedTimeMs / 1000.0 * 3.1415926));
    for (size_t i = 0; i < pRootBox->GetItemCount(); ++i) {
        ui::Control* pChild = pRootBox->GetItemAt(i);
        if (pChild != nullptr) { pChild->SetAlpha(nAlpha); }
    }
    return true;
});
pListCtrl->StartLoading(33, -1);   // 33ms 心跳，-1 = 不靠次数停止
```

**其它必须知道的机制细节**：
- `StopLoading()` 会 `m_pLoadingBox.reset()` **销毁 Box**，下次 `StartLoading` 重新解析 XML
  → 皮肤 XML 必须每次能独立解析。
- `Control::StartLoading` 内部会 `SetEnabled(false)`，`StopLoading` 才恢复。loading 卡死会导致控件
  一直禁用、整片区域点不动，调试时务必留一个"无条件 `StopLoading()`"的兜底出口。
- `AttachLoading*` 回调的 `wParam` 是**栈上的 `ControlLoadingStatus*`，当场用完即亡，不能存**。
- **用 `animation_control` 做非循环动画**：`play_count='1' auto_play='false'` 的 json 当静态图集，
  在 `kEventLoading` 里 `SetImageAnimationFrame(percent * frameCount / 100)` 手动刷帧 ——
  `loading_progress1/2.xml` 的进度条就是这么做的，零框架改动。
- `kEventLoading` 回调里**不要用 `IsLoading()` 短路 return false**（该事件是定时器驱动的，
  结束后还会派发几次），只判 `args.wParam == nullptr`。

完整章节见 [docs/Control.md](../../../../docs/Control.md#controlloading-与骨架屏)，
演示见 `examples/ListCtrl` 的「骨架屏测试（列表）/（卡片）/ 停止Loading」三个按钮。

### 弱回调保护

```cpp
// 使用 UiBind 绑定成员函数(自动弱引用保护)
UiBind(&MyForm::OnButtonClick, this);

// lambda 中使用 ui::UiBind 包装
ui::UiBind(this, [this]() {
    // 控件销毁后不会执行
});
```

### 模态消息框 MessageBoxWnd

框架自带的自绘皮肤模态消息框，头文件 `duilib/Utils/MessageBoxWnd.h`，
皮肤 `bin/resources/themes/default/public/messagebox/messagebox.xml`，
完整演示见 `examples/controls`（MessageBoxForm）。只能通过静态接口 `Show` 使用，
内部以 `DoModal` 模态显示，阻塞父窗口，返回用户点击的按钮。

```cpp
#include "duilib/Utils/MessageBoxWnd.h"

//基本用法（按钮组合 + 图标）
int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("确定要删除该文件吗？"), _T("删除确认"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion);
if (nResult == ui::MessageBoxWnd::kResultYes) {
    //执行删除
}

//多语言：第 7 个参数传 true，text/title/按钮文字全部按多语言 ID 解析，切换语言实时生效
int32_t nResult2 = ui::MessageBoxWnd::Show(this,
    _T("STRID_DELETE_CONFIRM_TEXT"), _T("STRID_DELETE_CONFIRM_TITLE"),
    ui::MessageBoxWnd::kButtonsOKCancel,
    ui::MessageBoxWnd::kIconWarning, nullptr, true);

//自定义按钮文字（bTextId=true 时成员也填多语言 ID；某一项留空则用框架默认文字）
ui::MessageBoxWnd::ButtonText buttonText;
buttonText.yes    = _T("STRID_CUSTOM_SAVE");
buttonText.no     = _T("STRID_CUSTOM_DISCARD");
buttonText.cancel = _T("STRID_CUSTOM_CANCEL");
ui::MessageBoxWnd::Show(this, _T("..."), _T("..."),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion, &buttonText, true);
```

`Show(pParentWindow, text, title, buttonFlags, iconType, pButtonText=nullptr, bTextId=false)`：

| 参数 | 说明 |
|------|------|
| pParentWindow | 父窗口，模态期间不可操作，可为 nullptr |
| text / title | 消息内容（支持 `\n` 换行，长文本自动换行，窗口高度自适应）/ 窗口标题 |
| buttonFlags | 按钮组合，见下表 |
| iconType | `kIconNone` / `kIconInfo` / `kIconWarning` / `kIconError` / `kIconQuestion` |
| pButtonText | 自定义按钮文字，nullptr 用默认文字 |
| bTextId | 为 true 时 text/title/按钮文字均按多语言 ID 解析；任务栏原生标题也显示译文而非 ID |

| ButtonFlags 按钮组合 | 含有的按钮 |
|------|------|
| `kButtonsOK` | 确定 |
| `kButtonsOKCancel` | 确定、取消 |
| `kButtonsYesNo` | 是、否 |
| `kButtonsYesNoCancel` | 是、否、取消 |
| `kButtonsRetryCancel` | 重试、取消 |

返回值（与 Win32 `ID*` 一致；创建失败返回 -1）：

| Result | 值 | 触发方式 |
|------|----|------|
| `kResultOK` | 1 | 点击"确定"；或按 Enter 且组合含"确定" |
| `kResultCancel` | 2 | 点击"取消"、按 ESC、点标题栏关闭按钮 |
| `kResultRetry` | 4 | 点击"重试" |
| `kResultYes` | 6 | 点击"是"；Enter 默认按钮映射（组合无"确定"时） |
| `kResultNo` | 7 | 点击"否" |

要点：

- Enter 触发**当前焦点按钮**（默认焦点是组合中的第一个肯定性按钮：确定→是→重试）；
  按 TAB/Shift+TAB 可在功能按钮间循环切换焦点，标题栏关闭按钮不参与 TAB 循环。
- ESC 与标题栏 X 一律返回 `kResultCancel`，即使组合中没有"取消"按钮也不会映射成默认按钮
  （与 Win32 放弃选择的语义一致）。
- 焦点虚线框按需显示：窗口初始弹出时不显示焦点环，用户首次按 TAB 后才显示，
  由 `MessageBoxWnd::OnKeyDownMsg` 调 `SetShowFocusedRect(true)` 开启。
- DoModal 下 Enter 键能派发给焦点按钮，依赖 `NativeWindow_Windows::SetEnterKeyPassthrough`
  的注册机制（见 pitfalls.md「DoModal 模态对话框的键盘消息」）；自定义模态窗口若有同样需求需自行注册/注销。

### 通用模态对话框 Dialog

框架自绘的通用模态对话框，头文件 `duilib/Utils/Dialog.h`（已在 `duilib/duilib.h` 中 include）。
与 `MessageBoxWnd` 的区别：内容区可放任意 XML 片段，底部按钮的数量、文字、默认按钮由调用方自定义。
窗口 XML 由类在运行时按「标题栏 + 内容区 + 按钮区」组装，不依赖独立皮肤文件；默认资源目录 `public`。
演示见 `examples/controls`（MessageBoxForm 的「通用 Dialog」按钮）。

```cpp
#include "duilib/duilib.h"

ui::Dialog dlg;
dlg.SetTitle(_T("STRID_EDIT_TITLE"));      //SetTextIdMode(true) 时按多语言 ID 解析
dlg.SetTextIdMode(true);
dlg.SetSize(460, 300);                      //不含阴影，默认 480 x 360，最小 160 x 120
dlg.SetContentXml(_T("<VBox width=\"stretch\" height=\"stretch\">")
                  _T("<Label text_id=\"STRID_EDIT_TEXT\" multi_line=\"true\" width=\"stretch\" height=\"auto\"/>")
                  _T("<CheckBox name=\"chk_remember\" text=\"记住选择\" margin=\"0,16,0,0\"/>")
                  _T("</VBox>"));
dlg.AddButton(_T("ok"), _T("STRID_MSGBOX_OK"), true);   //默认按钮：蓝色样式 + 初始焦点
dlg.AddButton(_T("cancel"), _T("STRID_MSGBOX_CANCEL"));

//按钮点击回调在窗口销毁前执行，可读取/校验内容区控件；返回 false 阻止关闭
bool bRemember = false;
dlg.AttachButtonClick([&bRemember](ui::Dialog* pDialog, int32_t nIndex) {
    if ((nIndex >= 0) && (pDialog->GetClickedButtonId() == _T("ok"))) {
        ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(pDialog->FindControl(_T("chk_remember")));
        bRemember = (pCheckBox != nullptr) && pCheckBox->IsSelected();
    }
    return true;
});

int32_t nIndex = dlg.DoModal(this);         //返回按钮索引
```

| 接口 | 说明 |
|------|------|
| `SetTitle` / `SetTextIdMode` | 标题；多语言模式影响标题与按钮文字，任务栏原生标题也显示译文 |
| `SetSkinFolder` | 资源目录（相对资源根目录），默认 `public`；内容 XML 中的资源路径建议按资源根目录写 |
| `SetSize` | 窗口大小（不含阴影），默认 480 x 360，最小 160 x 120 |
| `SetTitleBarVisible` | 是否显示标题栏（默认 true）；隐藏后只能 ESC 或按钮关闭 |
| `SetContentXml` | 内容区 XML 片段（如 `<VBox>...</VBox>`），为空则内容区留空 |
| `AddButton(id, text, bDefault=false)` | 按添加顺序从左到右排列；`id` 必须非空且唯一，供 `GetClickedButtonId` 返回；id 为空或重复时添加失败并返回 false |
| `AttachInit(callback)` | 窗口创建完成后的回调，可 `FindControl` 填数据；在默认按钮获得焦点之后执行 |
| `AttachButtonClick(callback)` | 按钮点击/Enter 触发时、关闭前执行；回调返回 false 保持对话框打开 |
| `DoModal(parent)` | 返回被点击按钮的索引；ESC/标题栏关闭返回 `kResultCancel`(-1)；创建失败返回 `kResultFailed`(-2) |
| `GetClickedButtonIndex` / `GetClickedButtonId` | 被点击按钮的索引 / 业务标识；取消时分别为 -1 / 空 |

要点：

- 标题、内容 XML、按钮列表必须在 `DoModal` 之前配置：窗口 XML 在创建窗口时组装。
- 默认按钮初始获得焦点，Enter 触发当前焦点按钮；TAB 在按钮与内容区可聚焦控件间循环，标题栏关闭按钮不参与 TAB。
- ESC 与标题栏 X 一律返回 `kResultCancel`，不会映射成默认按钮。
- 未添加任何按钮时自动补一个"确定"按钮，避免对话框无法关闭。
- 内容区不提供滚动，超长内容请自行用 `ScrollBox` 包裹；内容区不做表单校验，校验逻辑放在 `AttachButtonClick` 中。
- 与 `MessageBoxWnd` 相同，Enter 能派发给焦点按钮依赖 `NativeWindow_Windows::SetEnterKeyPassthrough` 的注册/注销（Dialog 内部已处理）；SDL 平台的模态循环不做 Enter 关闭，避免吞掉输入控件的回车。

### 非模态通知框 ToastWnd

框架自带的自绘皮肤非模态通知框（Toast），头文件 `duilib/Utils/ToastWnd.h`
（已在 `duilib/duilib.h` 中 include），皮肤
`bin/resources/themes/default/public/toast/toast.xml`，
完整演示见 `examples/controls`（ToastForm）。非模态、不抢焦点，到时自动消失，
多条垂直堆叠，点击立即关闭，鼠标悬停暂停倒计时、移开按剩余时间继续。
fire-and-forget：只能通过静态 `Show` 使用，窗口对象关闭后由框架自动销毁。

```cpp
#include "duilib/duilib.h"

//基本用法：默认顶部居中、停留 3000ms
ui::ToastWnd::Show(this, _T("保存成功"), ui::ToastWnd::kTypeSuccess);

//错误通知，停留 5 秒
ui::ToastWnd::Show(this, _T("网络连接失败，请稍后重试。"),
                   ui::ToastWnd::kTypeError, 5000);

//右下角通知
ui::ToastWnd::Show(this, _T("文件已下载完成"),
                   ui::ToastWnd::kTypeInfo, 4000, ui::ToastWnd::kPosBottomRight);

//nDurationMs 传 0 = 不自动关闭，只能点击关闭
ui::ToastWnd::Show(this, _T("重要提示，需手动关闭"), ui::ToastWnd::kTypeWarning, 0);

//多语言 ID（切换语言后实时刷新）
ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_INFO"),
                   ui::ToastWnd::kTypeInfo, 3000, ui::ToastWnd::kPosTop, true);
```

`Show(pParentWindow, text, type=kTypeInfo, nDurationMs=3000, position=kPosTop, bTextId=false)`：

| 参数 | 说明 |
|------|------|
| pParentWindow | 父窗口，通知显示在其客户区附近，显示期间父窗口仍可操作；nullptr 时按显示器工作区定位 |
| text | 通知内容，支持 `\n` 换行，长文本自动换行，窗口高度自适应 |
| type | `kTypeInfo`（蓝 i）/ `kTypeSuccess`（绿对勾）/ `kTypeWarning`（黄 !）/ `kTypeError`（红 X） |
| nDurationMs | 停留毫秒数，默认 3000；**0 表示不自动关闭** |
| position | `kPosTop`(默认) / `kPosCenter` / `kPosBottom` / `kPosTopRight` / `kPosBottomRight` |
| bTextId | 为 true 时 text 按多语言 ID 解析 |

**带操作按钮（ActionButton）**：除整条点击关闭外，还可传入一组操作按钮，对齐 Win32 通知的
ActionButton 能力。调用 7 参数 `Show` 重载，额外传入 `std::vector<ToastAction>`：

```cpp
//带操作按钮的通知（如"知道了 / 查看详情"），对齐 Win32 通知的 ActionButton
std::vector<ui::ToastAction> actions;
ui::ToastAction gotIt;
gotIt.text = _T("STRID_TOAST_ACTION_GOTIT");   //按钮文本（bTextId=true 时按多语言 ID 解析）
gotIt.bTextId = true;
gotIt.callback = [&]() { /* 点击"知道了"后执行的逻辑 */ };  //回调为空则点击仅关闭通知
actions.push_back(gotIt);

ui::ToastAction detail;
detail.text = _T("STRID_TOAST_ACTION_DETAIL");
detail.bTextId = true;
detail.callback = [&]() { /* 点击"查看详情"后执行的逻辑 */ };
actions.push_back(detail);

ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_ACTION_TEXT"),
                   ui::ToastWnd::kTypeSuccess, actions, 6000, ui::ToastWnd::kPosTop, true);
```

`ToastAction` 结构：

| 字段 | 说明 |
|------|------|
| text | 按钮文本（bTextId=true 时按多语言 ID 解析） |
| callback | 点击回调（`StdClosure`，可为空）；执行后通知自动关闭 |
| bTextId | 为 true 时 text 按多语言 ID 解析 |

操作按钮行为要点：

- 操作按钮位于通知底部，默认隐藏；仅当传入非空 `actions` 时动态创建并显示，无操作按钮时布局与行为完全不变。
- 按钮 `mouse_enabled=true`，点击只触发自身回调（执行后关闭通知），**不会**冒泡到根容器的"整条点击关闭"；点击通知其它非交互区域仍整条关闭。
- 按钮样式与日历"今天/清除"按钮同款（圆角边框 + 悬停/按下背景填充），颜色跟随按钮状态。

要点：

- 堆叠规则：同一父窗口、同一 position 的通知垂直堆叠（间距 12 DIP，新的在后面），
  某条关闭后其余平滑上移补齐；同屏上限 5 条（`kMaxToastCount`），超出后最早的先退场。
- 倒计时用 `ThreadManager::PostDelayedTask` + `steady_clock` 时间戳实现，
  悬停暂停只暂停剩余时间，多次移入移出不会重置总时长。
- 动画用 `AnimationPlayer`（`AnimationType::kAnimationNone` 回调驱动）：
  入场 200ms 淡入+上移 12DIP（EaseOutCubic），退场 180ms 淡出（EaseInCubic），
  分层窗口透明度用 `SetLayeredWindowAlpha`。
- 窗口样式 `kWS_POPUP|kWS_EX_TOPMOST|kWS_EX_LAYERED|kWS_EX_NOACTIVATE`，
  无父窗口时再加 `kWS_EX_TOOLWINDOW`（不进任务栏/Alt+Tab）。
- **四个实现坑（自写类似交互窗口时务必注意，详见 pitfalls.md）**：
  ① 普通容器不派发 `kEventClick`，整条可点要用 `AttachButtonUp`（kEventMouseButtonUp）；
  ② 有阴影的窗口 `GetRoot()` 返回的 ShadowBox 已被 `SetMouseEnabled(false)`，
     鼠标事件必须绑在 `GetXmlRoot()`（XML 可见根容器）上；尺寸测量仍用 `GetRoot()`
     （EstimateSize 含阴影、GetPadding() 是阴影边距）；
  ③ 鼠标直接移出窗口边界时控件级 kEventMouseLeave 不派发，需重写窗口级
     `OnMouseLeaveMsg` 恢复倒计时；
  ④ 高度自适应窗口中的多行文本不要用 `width="stretch"`——估算高度时 stretch
     子项按容器可用宽（未扣除固定宽兄弟）换行，英文按词换行会少算行数导致末行
     被裁；Label 宽度写与实际布局相等的固定值（toast.xml 为 296）。

## 八、布局属性速查

### 瓦片布局 (HTileBox/VTileBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| item_size | size | 子项大小 "100,40" |
| rows (HTileBox) | int/"auto" | 行数 |
| columns (VTileBox) | int/"auto" | 列数 |
| scale_down | bool | 超出时缩小 |

### 网格布局 (GridBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| rows | int | 网格行数(0自动) |
| columns | int | 网格列数(0自动) |
| grid_width | int | 单元格宽度(0自动) |
| grid_height | int | 单元格高度(0自动) |

子控件使用 `row_span`/`col_span` 属性合并单元格。

### ScrollBox 额外属性

| 属性 | 类型 | 说明 |
|------|------|------|
| vscrollbar | bool | 垂直滚动条 |
| hscrollbar | bool | 水平滚动条 |
| vscrollbar_class | string | 垂直滚动条样式 |
| hscrollbar_class | string | 水平滚动条样式 |

## 九、文件路径约定

- 主题资源根目录: `bin/resources/themes/default/`
- 全局配置: `bin/resources/themes/default/global.xml`
- 窗口XML: `bin/resources/themes/default/<skin_folder>/<skin_file>.xml`
- 公共图片: `bin/resources/themes/default/public/`
- 字体文件: `bin/resources/fonts/`
- 语言文件: `bin/resources/lang/`（`zh_CN.txt` / `en_US.txt`，示例中另有 `zh_CN-public.txt`、`zh_CN-examples.txt` 等）
  **注意**：语言文件在 **`bin/resources/lang/`**，**不在** `themes/default/lang/`
- 主题色定义: `bin/resources/themes/color_light/global.xml`（浅色）/ `bin/resources/themes/color_dark/global.xml`（深色）
- 示例源码: `examples/<example_name>/`（注意目录名大小写，如 `examples/RichEdit`）
