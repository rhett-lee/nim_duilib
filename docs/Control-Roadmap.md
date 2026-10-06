简体中文 | [English](Control-Roadmap.en.md)

## 控件库能力盘点与新增规划

本文档基于对现有代码的全面扫描（`duilib/Control`、`duilib/Box`、`duilib/Layout`、
`duilib/Utils` 及 `duilib/duilib_defs.h` 中的控件类型定义），对照主流桌面 UI 框架
（Qt、WPF、WinUI、Flutter Desktop）的常用控件清单，梳理现有能力、找出缺口，
并给出建议新增控件的分层规划，目标是让本界面库能够较好地满足各类平台桌面软件的开发需求。

### 一、现有能力盘点

| 类别 | 已有控件 |
|------|---------|
| 文本 | Label、RichText（类 HTML 格式化文本）、HyperLink |
| 按钮/选择 | Button、Option（单选）、CheckBox（复选） |
| 输入 | RichEdit（单行/多行富文本编辑）、IPAddress、HotKey、DateTime（日期时间选择） |
| 下拉 | Combo、ComboButton、FilterCombo（可过滤）、CheckCombo（多选） |
| 进度 | Progress、CircleProgress（环形）、Slider（滑块） |
| 列表/表格 | ListBox（水平/垂直/瓦片 + Virtual 虚表系列）、ListCtrl（Report/ListView/IconView 三视图、表头、复选框列、排序、分组、单元格编辑） |
| 树 | TreeView、DirectoryTree（目录树） |
| 选项卡 | TabBox（容器式）、TabCtrl（浏览器式多标签） |
| 菜单 | Menu/MenuItem/SubMenu/MenuBar |
| 颜色 | ColorPicker 全家桶（Regular/Standard/StandardGray/Custom）及 ColorSlider、ColorControl |
| 容器/布局 | HBox/VBox/ScrollBox/TileBox/GridBox/FlowBox/Panel（带标题栏可折叠面板，支持手风琴分组）、SplitBox + 13 种布局引擎（含虚表布局） |
| 其他控件 | Split（分割条）、GroupBox、Line、ScrollBar、PropertyGrid（属性表）、AddressBar（地址栏）、ChildWindow、IconControl、BitmapControl |
| 拖拽增强 | ControlDragable/HBoxDragable/VBoxDragable（拖动调整子控件顺序）、BoxMovable、BoxResizable |
| Web 嵌入 | CefControl、WebView2Control |
| 窗口工具 | ToastWnd（非模态通知框）、MessageBoxWnd（模态消息框）、Tooltip（悬停提示）、FileDialog、TrayIcon（托盘）、Clipboard、ScreenCapture |
| 基础设施 | SVG（nanosvg）、GIF/WebP/APNG/Lottie/PAG 动图、深浅色主题、多语言、DPI 自适应、文件拖放、动画系统、SDL 跨平台后端（Linux/MacOS/FreeBSD） |

大件控件（列表、表格、树、虚表、属性表、拾色器、Web 嵌入）均已具备，覆盖面完整。
对照主流框架，缺口集中在**现代交互控件**与**浮层体系**两块。

### 二、建议新增控件

按价值与实现成本分为三个梯队，建议按梯队顺序实施。

#### 第一梯队：表单刚需，实现成本低

| # | 控件 | 现状 | 优化方向 |
|---|------|------|------|
| 1 | RichEdit 占位提示 | **已有**：`prompt_mode` / `prompt_text` / `prompt_text_id` / `prompt_color`（[RichEdit_Windows.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit_Windows.h)、[RichEdit2.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit2.h)，跨平台 RichEdit2 亦支持），示例见 rich_edit.exe | 无需新开发，文档补充即可 |
| 2 | Switch/Toggle 开关 | **已有但样式静态**：[global.xml](file:///c:/develop/nim_duilib/bin/resources/themes/default/global.xml#L263-L268) 的 `checkbox_toggle_1/2` 用 CheckBox + 两张 SVG（off/on）硬切换，无滑块动画 | 已完成：新增 Switch 控件类（自绘轨道+滑块动画+颜色过渡，语义色适配深浅色） |
| 3 | SpinBox 数字输入框 | **已有**：RichEdit 的 `spin_class` 属性（`rich_edit_spin` / `rich_edit_spin_box/btn_up/btn_down`），配合 `min_number/max_number/number_only/limit_text`，示例见 rich_edit.exe | 已完成：独立为 SpinBox 控件类（支持 step 步长） |
| 4 | SearchBox 搜索框 | **缺失**：未基于 prompt_text + clear 组合，无专门控件 | 已完成：新增 SearchBox 组合控件（左图标+编辑框+清除按钮，支持回车事件） |
| 5 | Badge 角标 | TabCtrl 标签、按钮上的未读数/小红点。IM、邮件类应用必备 | 已完成：新增 Badge 控件类（数字角标/红点模式，count<=0 自动隐藏，语义色适配深浅色） |

#### 第二梯队：补齐浮层地基，实现成本中等

| # | 控件 | 说明 | 参考 |
|---|------|------|------|
| 6 | Flyout/Popup 浮层容器（建议最优先） | 目前 Menu 只能做菜单形态、Tooltip 仅纯文本（Windows 实现为系统原生）。缺一个"任意内容浮出卡片"基座：定位锚点、不抢焦点、自动关闭、按主题自绘。它是日历弹层、搜索建议、富内容提示、数值气泡等控件的公共地基 | Qt Popup、WinUI Flyout |
| 7 | Calendar 日历面板 | 现有 DateTime 依赖系统控件（超类化 DTP，数字滚动式），无自绘月历面板。日程、报表类软件需要，依赖第 6 项作为弹层载体 | Qt QCalendarWidget、WPF DatePicker |
| 8 | NavigationView 侧边栏导航 | 汉堡按钮 + 分组导航项 + 页头 + 内容区联动。现代设置页、桌面客户端标配；可用 TreeView+TabBox 组合模拟，但交互内聚性差 | WinUI NavigationView |
| 9 | Toast 操作按钮 | 现有 Toast 仅整条可点关闭；增加"撤销/查看详情"类操作按钮区，对齐 Win32 通知能力，小增强 | Windows 通知 ActionButton |

#### 第三梯队：大件/独立模块，按需排期

| # | 控件 | 说明 | 参考 |
|---|------|------|------|
| 10 | 轻量 Chart 图表 | 折线/柱状/饼图自绘 + 数据绑定。目前图表只能嵌 CEF 或引第三方库；工作量最大，但对"桌面开发全家桶"价值最高，建议作为独立模块演进 | Qt Charts（精简） |
| 11 | Timeline 时间轴 | 垂直时间轴（节点 + 内容），用于日志、任务流、审批流展示。自绘成本较低 | 各平台通用 |
| 12 | 骨架屏/加载占位 | 已有 ControlLoading（控件加载中状态），补充列表/卡片骨架屏占位模式，改善异步数据加载体验 | 各平台通用 |

### 三、明确不建议做的

* **Docking 可停靠布局**：成本极高（浮动窗、落泊指示器、布局持久化），跨平台一致性难保证，且多数桌面软件用不到。
* **MDI 多文档界面**：现代桌面应用已基本弃用。
* **重型图表引擎**：超出控件库范畴，有第 10 项的轻量 Chart 覆盖常见场景即可。

### 四、实施顺序建议

1. **先做第 6 项 Flyout 浮层容器**：它是第二、三梯队多个控件的公共依赖，
   并可顺手将 Windows 平台的系统原生 Tooltip 替换为自绘实现，使弹层风格
   在深色主题下保持一致。
2. 随后完成第一梯队 1–5 项（表单刚需），其中第 4 项依赖第 1 项。
3. 第 7 项 Calendar 在 Flyout 之上实现；第 8、9 项可并行。
4. 第三梯队按产品需求排期，不阻塞前两梯队。

> 注：本文档为规划性质，具体实现方案（接口、XML 属性、皮肤资源）待各项启动时另行设计。
