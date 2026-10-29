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
| 10 | CEF 组件并入 duilib 工程，并升级 CEF 版本：支持 libcef 109（兼容 Windows 7）与 libcef 142（Windows 10 及以上） |
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
| 5 | 优化 ListBox 控件，细分为 ListBox、VListBox、HListBox、VTileListBox、HTileListBox、VirtualListBox、VirtualVListBox、VirtualHListBox、VirtualVTileListBox、VirtualHTileListBox，功能更完善，虚表实现可用性提升 |
| 6 | 优化组合框（Combo）关联控件（CheckCombo、FilterCombo），提升可用性 |
| 7 | 完善日期时间（DateTime）控件的编辑功能 |
| 8 | 优化菜单（CMenuWnd）类：按新结构重新实现菜单，菜单内的控件完全兼容现有容器/控件体系，易于理解和维护 |
| 9 | 优化和扩展文本编辑控件（RichEdit），补充较多常用功能 |
| 10 | 优化和扩展树控件（TreeView），补充较多常用功能，提升可用性 |
| 11 | 优化全局资源（GlobalManager）接口，所有资源统一通过该接口管理，易于理解和维护 |
| 12 | 同一窗口内的不同容器之间，容器内的控件可通过属性设置支持拖出和拖入操作 |
| 13 | 控件背景色支持渐变色，并新增前景色功能 |
| 14 | 完善多国语言功能，更好地支持多语言动态切换，并提供示例程序 [examples/MultiLang](../examples/MultiLang) |
| 15 | 完善 DPI 感知功能：支持 Unaware、SystemAware、PerMonitorAware、PerMonitorAware_V2 四种模式，支持独立设置 DPI 与高 DPI 适配，并提供示例程序 [examples/DpiAware](../examples/DpiAware) |
| 16 | 移除 ui_components 工程，CEF 组件代码重新梳理后并入 duilib 工程，其余内容删除 |
| 17 | 优化窗口阴影功能：阴影使用 SVG 图片，新增阴影类型属性（`shadow_type`），支持自绘阴影与系统阴影（见 [Window.md](Window.md)） |
| 18 | 新增 APNG/SVG/WEBP/ICO/LOTTIE/PAG 图片格式支持 |
| 19 | 重新设计控件的 loading 功能：使用 Box 容器展示 loading，通过 XML 配置 loading 界面（含动画图片），并支持与动画图片交互 |
| 20 | 加强 Label 文本显示控件：文本对齐新增"两端对齐"，新增竖排文本支持（从上到下、从右到左），并支持设置行间距与字间距 |
| 21 | Control 控件支持全屏显示（通过新增的 `Window::SetFullscreenControl` 实现），CEF 控件和 WebView2 控件支持 F11 切换页面全屏 |
| 22 | 完善控件动画细节并引入缓动函数，支持配置缓动函数类型、动画总时长与播放间隔等属性 |
| 23 | 统一上下文菜单 API：`Control::AttachPopupMenu` 一行代码即可把右键菜单绑定到任意控件（自动创建菜单、坐标换算、菜单项回调，支持菜单弹出后动态调整），配套 `DetachPopupMenu` 解除绑定 |

## 3. 新增控件/容器（50+ 项）

以下 50+ 项在 NIM_Duilib_Framework 中不存在，是迁移时最直接的"能力增量"。属性、事件与用法以 [Control.md](Control.md) 为准。

| 序号 | 名称 | 说明 |
| :--- | :--- | :--- |
| 1 | GroupBox | 分组容器 |
| 2 | HotKey | 热键控件 |
| 3 | HyperLink | 带有超级链接的文字 |
| 4 | IPAddress | IP 地址控件 |
| 5 | Line | 画线控件 |
| 6 | RichText | 格式化文本（类 HTML 格式） |
| 7 | Split | 分割条控件/容器 |
| 8 | TabCtrl | 多标签控件（类似浏览器的多标签） |
| 9 | ListCtrl | 列表控件（Report/Icon/List 三个视图） |
| 10 | PropertyGrid | 属性表控件，支持文本、数字、复选框、字体、颜色、日期、IP 地址、热键、文件路径、文件夹等属性 |
| 11 | ColorPicker | 拾色器，独立窗口；其中的子控件也可以单独作为颜色控件使用 |
| 12 | ComboButton | 带有下拉组合框的按钮 |
| 13 | DirectoryTree | 目录树控件，用于显示文件系统中的目录 |
| 14 | AddressBar | 地址栏控件，用于显示本地文件系统的路径 |
| 15 | WebView2Control | 封装 WebView2 控件的基本功能 |
| 16 | GridBox/GridScrollBox | 基于网格布局的控件 |
| 17 | HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox | 基于水平流式布局和垂直流式布局的控件 |
| 18 | MenuBar | 菜单栏控件 |
| 19 | IconControl/BitmapControl | 用于显示基于内存的小图标和位图数据 |
| 20 | ChildWindow | 子窗口控件；Windows 平台为系统原生子窗口（`WS_CHILD`），其他平台为 SDL 弹出式窗口（SDL 不支持原生子窗口） |
| 21 | ControlDragableT | 模板类，包含 ControlDragable/BoxDragable/HBoxDragable/VBoxDragable 四个标准控件：支持在相同 Box 内拖动调整子控件顺序，也支持跨 Box 拖动改变控件所属容器 |
| 22 | ControlMovableT | 模板类，包含 ControlMovable/BoxMovable/HBoxMovable/VBoxMovable 四个标准控件：支持鼠标拖动调整控件位置，也支持拖动父容器位置 |
| 23 | ControlResizableT | 模板类，包含 ControlResizable/BoxResizable/HBoxResizable/VBoxResizable 四个标准控件：支持鼠标拖动调整控件大小，效果与调整窗口大小类似 |
| 24 | XmlBox | 支持加载并预览界面库 XML 文件的容器，可用于预览 XML 中定义的控件显示效果 |
| 25 | Pane/PanelHBox/PanelVBox | 带标题栏的面板容器（PanelTemplate<Box>），支持标题栏样式、折叠/展开、手风琴分组 |
| 26 | MessageBoxWnd | 框架自带的自绘皮肤模态消息框 |
| 27 | ToastWnd | 框架自带的自绘皮肤非模态通知框，自动消失、多条堆叠、悬停暂停、点击关闭 |
| 28 | SearchBox | 搜索框组合控件，左侧图标 + 编辑框 + 清除按钮 |
| 29 | SpinBox | 数字输入框，继承 RichEdit，支持步长/范围/步进按钮 |
| 30 | Switch | 滑块开关控件，继承 CheckBox，自带滑动动画与颜色过渡 |
| 31 | Badge | 角标控件，用于未读数/小红点；数量超限显示 "99+"，`count<=0` 时自动隐藏 |
| 32 | Flyout | 通用浮层（弹出卡片）窗口，用于在锚点控件周围浮出任意 Box 内容（操作面板、确认卡片、富内容提示等） |
| 33 | Calendar | 日历控件 |
| 34 | NavigationView | 侧边栏导航容器（左侧导航窗格 + 右侧内容区），支持导航项分组、底部设置项、窗格收起/展开 |
| 35 | Chart | 轻量图表控件，支持折线图（line）、柱状图（bar）、饼图（pie）等形态 |
| 36 | Marquee | 文本跑马灯控件，用于横向/纵向循环滚动显示文本 |
| 37 | Timeline | 垂直时间轴控件（节点 + 连接线 + 内容），数据驱动、状态配色、正序/倒序、多语言绑定、滚动条 |
| 38 | TagInput | 标签输入框组合控件：回车（或指定分隔符）生成标签，支持关闭按钮/Backspace 删除、去重、最大数量限制，标签过多自动换行 |
| 39 | AutoComplete | 自动完成输入框组合控件：边输入边在编辑框下方弹出候选浮层（复用 Flyout、不抢焦点、宽度跟随编辑框），支持前缀/包含匹配、忽略大小写、键盘上下键/回车/Esc 与鼠标选择 |
| 40 | Mask | 遮罩/蒙版层控件（继承 Box），用于模态对话框背景遮罩、新手引导蒙层、加载遮罩；支持镂空高亮指定控件、镂空区域内点击穿透到目标控件，并适配深浅色语义色 |
| 41 | StatusBar | 状态栏控件（继承 HBox），用于显示光标位置、字数、连接状态、后台进度等状态提示；子控件即状态面板，相邻面板之间自动绘制分隔线（宽度与颜色可配置），可选右下角尺寸手柄 |
| 42 | ToolBar | 工具条控件（继承 HBox），用于承载一排工具按钮与分隔线；可用宽度不足时自动隐藏尾部子控件并在右端显示"更多"按钮，点击弹出浮层列出被收纳项并通过回调通知应用层，窗口宽度变化时自动重新计算 |
| 43 | RangeSlider | 双滑块范围选择控件（继承 Progress），在一条轨道上放置下限/上限两个滑块并高亮选中区间；支持最小间距、区间颜色/圆角、值气泡 Tooltip、marks 刻度与标签、included 外侧高亮、刻度吸附与范围变化回调（`pEventData` 携带 low/high） |
| 44 | Stepper | 向导步骤条控件，用于安装/注册/配置向导与审批流程；支持水平/垂直排列，节点显示序号或对勾，状态由当前索引与禁用集合派生（待开始/进行中/已完成/禁用），连接线按完成度着色，支持 Next/Back/GoTo 与点击跳转、多语言标题/描述，步骤变化回调的 `pEventData` 携带 currentIndex/oldIndex |
| 45 | Rating | 星级评分控件（继承 Control），用于评价/打分/满意度调查；支持只读展示、半星（`allow_half`）、鼠标悬停预览与点击/拖拽打分，实心星/半星/空心星及悬停/按下态外观全部由 SVG 图片驱动并映射语义色，可配置星星数量（`max`/`star_count`）、尺寸与间距，值变化回调的 `pEventData` 携带 value/oldValue |
| 46 | Carousel | 图片轮播控件（继承 Control），用于首页横幅/广告位/图片集；幻灯片、左右切换按钮与底部可点击页码指示点均由 SVG 图片承载并映射语义色，支持自动播放 + 悬停暂停、无限循环（`loop`）、切换动画（`transition_ms`）与鼠标按住左右拖拽翻页，索引变化回调的 `pEventData` 携带 index/oldIndex |
| 47 | Pagination | 分页器控件（继承 HBox），用于报表/表格/列表的页码导航；上一页/下一页（可选首页/末页）+ 按数据动态生成的页码按钮，当前页呈选中态，页码超出显示上限时以 "…" 收纳并始终保留首尾与当前页附近，按钮配色由 `button_class` 皮肤类（走语义色）驱动；可选"每页条数"选择器（`show_page_size`，默认关闭；`page_size_options` 配置候选值，切换后按 `total_count` 重算页数并重置到第 1 页），页码/每页条数变化回调的 `pEventData` 携带 page/oldPage/pageSize/oldPageSize |
| 48 | TreeGrid | 树表格控件（继承 VBox），在同一控件内呈现"可展开/收起的层级行 + 多列表格"，用于文件树/报表/组织架构；表头固定不滚动、列表体自带纵向滚动条，表头与数据行复用同一列宽数组而逐列对齐（缩进由树列单元格内嵌占位实现，其他列不会错位）；列标题/列宽/对齐可配置，支持多选（`multi_select`）与行首复选框父子三态级联（`check_mode` + `check_cascade`），勾选变化事件的 `pEventData` 携带 TreeGridCheckValue |
| 49 | Dialog | 通用模态对话框（`duilib/Utils/Dialog.h`）：内容区支持任意 XML 片段，底部按钮的数量/文字/默认按钮可自定义，不依赖额外的 XML 文件；`DoModal` 返回被点击按钮的索引，支持 ESC 与标题栏关闭取消、Enter 触发当前焦点按钮、多语言文本 ID，以及关闭前执行的按钮点击回调（返回 false 可阻止关闭，便于读取内容区数据或校验输入） |
| 50 | SystemNotification | 系统原生通知（`duilib/Utils/SystemNotification.h`）：使用 SDL3 构建时通过 SDL 系统通知 API 发送到系统通知中心（Windows/macOS/Linux/FreeBSD），不使用 SDL 的 Windows 构建通过 `Shell_NotifyIcon` 托盘气泡实现（兼容 Windows 7 及以上）；提供 `IsSupported`/`RequestPermission`/`Show`/`Remove` 接口，支持通知优先级，通知的显示与销毁由系统负责（见 [SystemNotification.md](SystemNotification.md)） |
| 51 | Breadcrumb | 面包屑导航控件（`duilib/Control/Breadcrumb.h`）：纯自绘，支持 `\|` 分隔条目、当前项高亮、点击祖先项跳转、禁用项、自定义分隔符/间距/字体与语义色，多语言 `item_text_id`，当前项变化事件 `AttachCurrentChanged`；宽度不足时右对齐裁剪以保证当前项可见 |
| 52 | CascadingSelect | 级联选择器（`duilib/Control/CascadingSelect.h`）：支持内嵌多列与下拉列表框两种形态，父项变更自动过滤子列并重置深层旧选择，支持任意层级、`items`/`items_id` 路径数据（相同前缀自动合并）、禁用路径及子树、固定/等分列宽、列内滚轮滚动、多语言与语义色；下拉形态支持提示文本、清空按钮、移入展开、自定义显示文本与浮层尺寸控制，选择变化事件 `AttachSelectionChanged` |

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
