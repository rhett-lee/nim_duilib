简体中文 | [English](Control-Roadmap.en.md)

## 控件库能力盘点与新增规划

本文档基于对现有代码的全面扫描（`duilib/Control`、`duilib/Box`、`duilib/Layout`、`duilib/Utils` 及 `duilib/duilib_defs.h` 中的控件类型定义），对照主流桌面 UI 框架（Qt、WPF、WinUI、Flutter Desktop）的常用控件清单，梳理现有能力、找出缺口，并给出建议新增控件的分层规划，目标是让本界面库能够较好地满足各类平台桌面软件的开发需求。

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
| 其他控件 | Split（分割条）、GroupBox、Line、ScrollBar、PropertyGrid（属性表）、AddressBar（地址栏）、ChildWindow、IconControl、BitmapControl、StatusBar（状态栏）、ToolBar（工具栏）、RangeSlider（范围滑块）、Stepper（步骤条）、Rating（评分）、Carousel（轮播）、Pagination（分页器）、TreeGrid（树表格） |
| 拖拽增强 | ControlDragable/HBoxDragable/VBoxDragable（拖动调整子控件顺序）、BoxMovable、BoxResizable |
| Web 嵌入 | CefControl、WebView2Control |
| 窗口工具 | ToastWnd（非模态通知框）、MessageBoxWnd（模态消息框）、Dialog（通用模态对话框）、Tooltip（悬停提示）、FileDialog、TrayIcon（托盘）、Clipboard、ScreenCapture |
| 基础设施 | SVG（nanosvg）、GIF/WebP/APNG/Lottie/PAG 动图、深浅色主题、多语言、DPI 自适应、文件拖放、动画系统、SDL 跨平台后端（Linux/MacOS/FreeBSD） |

大件控件（列表、表格、树、虚表、属性表、拾色器、Web 嵌入）均已具备，覆盖面完整。对照主流框架，缺口集中在**现代交互控件**与**浮层体系**两块。

### 二、建议新增控件

按价值与实现成本分为三个梯队，建议按梯队顺序实施。

#### 第一梯队：表单刚需，实现成本低

| # | 控件 | 现状 | 优化方向 |
|---|------|------|------|
| 1 | RichEdit 占位提示 | **已有**：`prompt_mode` / `prompt_text` / `prompt_text_id` / `prompt_color`（[RichEdit_Windows.h](../../duilib/Control/RichEdit_Windows.h)、[RichEdit2.h](../../duilib/Control/RichEdit2.h)，跨平台 RichEdit2 亦支持），示例见 rich_edit.exe | 已完成：占位提示属性与示例的文档补充已写入 RichEdit 相关文档 |
| 2 | Switch/Toggle 开关 | **已有但样式静态**：[global.xml](../../bin/resources/themes/default/global.xml#L263-L268) 的 `checkbox_toggle_1/2` 用 CheckBox + 两张 SVG（off/on）硬切换，无滑块动画 | 已完成：新增 Switch 控件类（自绘轨道+滑块动画+颜色过渡，语义色适配深浅色） |
| 3 | SpinBox 数字输入框 | **已有**：RichEdit 的 `spin_class` 属性（`rich_edit_spin` / `rich_edit_spin_box/btn_up/btn_down`），配合 `min_number/max_number/number_only/limit_text`，示例见 rich_edit.exe | 已完成：独立为 SpinBox 控件类（支持 step 步长） |
| 4 | SearchBox 搜索框 | **缺失**：未基于 prompt_text + clear 组合，无专门控件 | 已完成：新增 SearchBox 组合控件（左图标+编辑框+清除按钮，支持回车事件） |
| 5 | Badge 角标 | TabCtrl 标签、按钮上的未读数/小红点。IM、邮件类应用必备 | 已完成：新增 Badge 控件类（数字角标/红点模式，count<=0 自动隐藏，语义色适配深浅色） |

#### 第二梯队：补齐浮层地基，实现成本中等

| # | 控件 | 说明 | 参考 | 状态 |
|---|------|------|------|------|
| 6 | Flyout/Popup 浮层容器（建议最优先） | 目前 Menu 只能做菜单形态、Tooltip 仅纯文本（Windows 实现为系统原生）。缺一个"任意内容浮出卡片"基座：定位锚点、不抢焦点、自动关闭、按主题自绘。它是日历弹层、搜索建议、富内容提示、数值气泡等控件的公共地基 | Qt Popup、WinUI Flyout | 已完成：新增 Flyout 浮层窗口类（8 方位锚点定位+自动翻转、不抢焦点、点击外部/Esc 自动关闭、语义色按主题自绘、全局单活），controls 与 ColorTheme 示例已接入 |
| 7 | Calendar 日历面板 | 现有 DateTime 依赖系统控件（超类化 DTP，数字滚动式），无自绘月历面板。日程、报表类软件需要，依赖第 6 项作为弹层载体 | Qt QCalendarWidget、WPF DatePicker | 已完成：新增 Calendar 自绘月历控件（单选/范围选择、月/年/十年三级导航、今天高亮、语义色适配深浅色）+ CalendarFlyout 弹层封装；DateTime 的 `date_calendar` 编辑格式已改用 CalendarFlyout，controls 与 ColorTheme 示例已接入 |
| 8 | NavigationView 侧边栏导航 | 汉堡按钮 + 分组导航项 + 页头 + 内容区联动。现代设置页、桌面客户端标配；可用 TreeView+TabBox 组合模拟，但交互内聚性差 | WinUI NavigationView | 已完成：新增 NavigationView + NavigationViewItem 两个控件（窗格含汉堡按钮/窗格标题/可滚动导航项列表/底部设置项，内容区含页头+TabBox 页面；导航项支持 item/header/separator 三种形态、图标+文字自绘、选中灰底强调色、hover 圆角热区；汉堡按钮即时展开/收起，收起态仅显示图标并带 tooltip；非导航项直接子节点自动路由为内容页，按 name 联动切换并同步页头；事件接口含 kEventNavigationItemClick（点击导航项，重复点击已选中项也触发）、kEventNavigationSelectionChanged（选中变化，携带新/旧序号）、kEventNavigationPaneToggling（窗格收起/展开前，回调返回 false 可取消）、kEventNavigationPaneToggled（窗格收起/展开完成）；语义色适配深浅色，text_id 支持多语言），controls 示例新增 navigation.xml 演示窗口（11 个导航项+3 分组+header/separator/设置项+显式 TabBox 多页面，并演示动态添加项与程序化切换） |
| 9 | Toast 操作按钮 | 现有 Toast 仅整条可点关闭；增加"撤销/查看详情"类操作按钮区，对齐 Win32 通知能力，小增强 | Windows 通知 ActionButton | 已完成：新增 `ToastAction` 结构与 `Show` 重载（携带 `std::vector<ToastAction>`），在通知底部渲染一组圆角操作按钮；点击按钮先执行回调再关闭通知，按钮 `mouse_enabled=true` 不冒泡到根容器（整条点击关闭仅对非交互区生效）；无操作按钮时布局与行为完全不变。controls 示例已加演示按钮 |

#### 第三梯队：大件/独立模块，按需排期

| # | 控件 | 说明 | 参考 | 状态 |
|---|------|------|------|------|
| 10 | 轻量 Chart 图表 | 折线/柱状/饼图自绘 + 数据绑定。目前图表只能嵌 CEF 或引第三方库；工作量最大，但对"桌面开发全家桶"价值最高，建议作为独立模块演进 | Qt Charts（精简） | 已完成：新增 Chart 控件类（继承 Control，纯自绘，支持 line/bar/pie 三形态，data/data_labels 数据绑定、坐标轴/网格/图例/数值标签、多系列、负值柱状、堆叠、曲线/面积、环形饼图；交互含悬停 Tooltip/点击选中/缩放平移；视觉含圆角/渐变/发光/数据动画；语义色适配深浅色），详见 [Control.md](Control.md#chart的属性) |
| 11 | Timeline 时间轴 | 垂直时间轴（节点 + 内容），用于日志、任务流、审批流展示。自绘成本较低 | 各平台通用 | 已完成：新增 Timeline 控件类（继承 Control，纯自绘，节点+状态连接线+时间/标题/正文内容；数据驱动 `item_time/item_title/item_content/item_status` 及多语言 `item_*_id` 绑定、4 态节点配色、实线/虚线连接线、正序/倒序、节点点击事件、空数据/加载状态、滚动条（hover 自动显隐、颜色可配、可拖动）、自定义渲染回调），详见 [Control.md](Control.md#timeline的属性) |
| 12 | 骨架屏/加载占位 | 已有 ControlLoading（控件加载中状态），补充列表/卡片骨架屏占位模式，改善异步数据加载体验 | 各平台通用 | 已完成：新增 `loading_skeleton_list.xml`（12 行列表骨架，含工具栏占位）与 `loading_skeleton_card.xml`（3 列 × 2 行卡片骨架，带底色圆角）两套骨架屏皮肤，走 `SetLoadingAttribute` + `StartLoading` 既有机制，**未新增控件类**。配套骨架屏四要点：① 根容器固定像素或 stretch + attribute `width='0' height='0'` 即可铺满目标控件（含表头）；② 根容器必须有**不透明** `bkcolor`，因 loading 层是叠加绘制不是遮罩，不画底色下方内容会透出；③ 骨架屏是静态占位无"动画播完"时刻，故**不设** `animation_control`/`auto_stop`，必须由业务侧判 `ControlLoadingStatus::m_nElapsedTimeMs` 超时兜底（`nMaxCount` 只是回调次数上限）；④ 呼吸动画**不能给根容器设 `SetAlpha`**（会连底板一起半透明导致遮罩失效），须根容器固定 255、只遍历 `GetItemCount()`/`GetItemAt(i)` 给子控件设 alpha。ListCtrl 示例已接入三个演示按钮（列表/卡片/停止），详见 [Control.md](Control.md#controlloading-与骨架屏) |

#### 第三梯队（补充）：表单与表格增强，成本低到中

上表是"大件"项，下列为补强现有表单/表格能力的较小件，可与第三梯队并行推进：

| # | 控件 | 说明 | 参考 |
|---|------|------|------|
| 13 | **TagInput 标签输入框（已完成）** | "输入→回车成标签→删除标签"的组合控件，用于收件人、标签选择、关键词录入。IM/协作/后台管理类刚需，复用 RichEdit + 自绘标签。已实现：继承 Box + 水平流式布局，回车/可选分隔符成标签、X 按钮与空框 Backspace 删除、去重与最大数量限制、标签过多自动换行增高，事件 `kEventTagAdded/kEventTagRemoved/kEventTagClick`，语义色适配深浅色，详见 [Control.md](Control.md#taginput的属性) | Element UI el-tag、Ant Design Tag |
| 14 | **AutoComplete 自动完成（已完成）** | 编辑框边输入边联想下拉（搜索历史、地址补全），与 FilterCombo 的"下拉过滤"不同——前者以编辑框为主、浮层跟随光标。已实现：继承 Box + 内部单行 RichEdit，输入达到 `min_chars` 后在下方弹出复用 Flyout 的不抢焦点候选浮层（宽度跟随编辑框、高度随候选项数量自适应、超出 `max_visible` 滚动），支持 prefix/contains 两种匹配与忽略大小写，键盘上下键/回车/Esc + 鼠标悬停/点击选择，回填不重弹，事件 `kEventAutoCompleteSelect`（XML 内联事件名 `select`），语义色适配深浅色，详见 [Control.md](Control.md#autocomplete的属性) | Qt QCompleter、WPF AutoCompleteBox |
| 15 | **上下文菜单统一 API（已完成）** | Menu 已具备，本次补齐"一行代码把右键菜单绑定到任意控件"的便捷封装（此前需手动创建 Menu + 弹窗 + 坐标换算）。已实现：`Control` 新增 `AttachPopupMenu(menuXml, actionCallback, creatingCallback)`——右键点击控件时自动创建 `Menu`、完成屏幕坐标换算（含 `Shift+F10` 场景）并弹出菜单，菜单项被激活（鼠标点击/回车）后通过 `ContextMenuActionEvent` 回调；`creatingCallback`（`ContextMenuCreatingEvent`）用于菜单弹出后动态增删菜单项、设置勾选/禁用状态；配套 `DetachPopupMenu()` 解除绑定；重复绑定直接覆盖。跨平台通用（Windows/Linux/macOS），详见 [Menu.md](Menu.md#6-便捷绑定右键菜单一行代码) | 各平台通用 |
| 16 | **遮罩/蒙版层（Mask/Overlay）（已完成）** | 模态对话框背景遮罩、新手引导蒙层、加载遮罩。弹层体系（Flyout/Modal）缺的最后一块。已实现：新增 `Mask` 控件类（继承 Box，XML 节点 `Mask`）——半透明蒙层填充、`highlight_control` 镂空高亮指定控件（偶奇路径挖洞，支持 `highlight_padding` 内边距与 `highlight_corner` 圆角）、`click_through` 使镂空区域内点击穿透到目标控件（新手引导）、非镂空区域拦截鼠标（默认 `mouse_enabled=true`）实现模态；提供 `Mask::ShowOverWindow(parentWnd)` 便捷接口覆盖整窗、`Remove()` 卸载；复用语义色 `bg_overlay`，详见 [Control.md](Control.md#mask的属性) | 各平台通用 |
| 17 | **ListCtrl 表格增强（已完成）** | 列宽拖拽调整、列冻结（固定首列）、列头拖拽换序、单元格富文本/进度条/图标，报表类软件高频诉求。已实现：列宽拖拽调整、列头拖拽换序、单元格图标为既有能力；本次新增 **列冻结**（`SetFrozenColumnCount/GetFrozenColumnCount`，XML `frozen_column_count`，冻结列表左侧前 N 列，横向滚动时固定不动，表头同步冻结、网格线与命中测试均已适配）、**单元格进度条**（`SetSubItemProgress(ById)` / `GetSubItemProgress(ById)`，进度值 `[0,100]`、`-1` 关闭，语义色 `progress_color`/`progress_bkcolor` 可配，可叠加百分比文字）、**单元格富文本**（`SetSubItemRichText(ById)` / `IsSubItemRichText(ById)`，复用 Label 内建富文本渲染，支持 `<b>/<font>/<a>` 等标签），ListCtrl 示例已接入（默认冻结首列、第2列进度条、第3列富文本），详见 [Control.md](Control.md#listctrl-表格增强) | Qt QTableView、WPF DataGrid |

### 三、明确不建议做的

- **Docking 可停靠布局**：成本极高（浮动窗、落泊指示器、布局持久化），跨平台一致性难保证，且多数桌面软件用不到。
- **MDI 多文档界面**：现代桌面应用已基本弃用。
- **重型图表引擎**：超出控件库范畴，有第 10 项的轻量 Chart 覆盖常见场景即可。

### 四、实施进度与顺序

#### 已完成（第一、二梯队 1–9 项 + 第三梯队 Chart 第 10 项、Timeline 第 11 项、骨架屏第 12 项 + 补充项 TagInput 第 13 项、AutoComplete 第 14 项、上下文菜单统一 API 第 15 项、遮罩/蒙版层第 16 项、ListCtrl 表格增强第 17 项、ListCtrl 交互增强第 18 项）

第一梯队（表单刚需 1–5）、第二梯队（浮层体系 6–9）与第三梯队的 Chart（第 10 项）、Timeline（第 11 项）、骨架屏/加载占位（第 12 项），以及第三梯队补充项 TagInput（第 13 项）、AutoComplete（第 14 项）、上下文菜单统一 API（第 15 项）、遮罩/蒙版层（第 16 项）、ListCtrl 表格增强（第 17 项）均已落地：Switch、SpinBox、SearchBox、Badge 及 Flyout、Calendar、NavigationView、Toast 操作按钮、Chart、Timeline、骨架屏、TagInput、AutoComplete、Mask 均已实现并接入 controls / ColorTheme / ListCtrl 示例；上下文菜单统一 API 以 `Control::AttachPopupMenu/DetachPopupMenu` 形式提供便捷封装，已在 controls 示例的 `test.xml` 内置演示控件 `context_menu_btn`。浮层公共地基 Flyout 已就绪，后续弹层类控件（富内容提示等）可直接复用。遮罩/蒙版层以 `Mask` 控件形式补齐了弹层体系的最后一块（模态遮罩/新手引导/加载遮罩），controls 示例新增三个演示按钮。骨架屏复用既有 `ControlLoading` 机制，未新增控件类，后续如需通用列表虚拟化骨架或与 ListCtrl item 模板自动联动，可作为增强项再排期。ListCtrl 表格增强复用了已有的列宽拖拽/表头换序/单元格图标能力，新增列冻结、单元格进度条与单元格富文本三项，ListCtrl 示例已接入演示。ListCtrl 交互增强（第 18 项）在其基础上补充了单格刷新接口（`RefreshSubItem/RefreshSubItemById`）、ListCtrl 一级列显隐接口（`SetColumnVisible/IsColumnVisible`）以及列布局对外事件（`kEventColumnWidthChanged/OrderChanged/VisibleChanged/Sorted`），使应用可对进度/状态类列做局部刷新，并可持久化列宽、列序、列显隐与排序状态。在其之上还完成了 **ListCtrl 单元格交互与编辑**：单元格选择模型（`selection_mode="cell"`，选择状态按行身份 `RowId` + 列ID 的逐列位图存储，支持 `Ctrl`/`Shift` 扩展与鼠标框选，事件 `kEventCellSelectionChanged`）与单元格就地编辑（`ListCtrlEditorType`：文本 / 数值 / 日期 / 下拉候选；`F2` 或单击进入编辑，提交按类型校验，复用 `kEventEnterEdit` / `kEventLeaveEdit`）。

#### 后续顺序建议

1. **第三梯队补充项已全部完成**：TagInput（第 13 项）、AutoComplete（第 14 项）、上下文菜单统一 API（第 15 项）、遮罩/蒙版层（第 16 项）、ListCtrl 表格增强（第 17 项）、ListCtrl 交互增强（第 18 项）均已落地。至此控制层规划清单已清空，后续新需求按实际需要再排期。
2. **平台整合能力与工程化（见第五、六部分）**：与控件规划并行，其中无障碍与打包签名是"覆盖各平台桌面开发"的硬门槛，建议尽早启动。
3. **第 16 项遮罩/蒙版层与骨架屏的关系（已解决）**：蒙层已由 `Mask` 控件统一提供（模态遮罩/新手引导/加载遮罩）；骨架屏的 loading 层仍是"叠加绘制"语义，二者定位不同——需要拦截交互或镂空高亮时用 `Mask`，仅需视觉占位时用骨架屏。
4. **审计新增控件已全部落地**：Project-Audit 的 B1 StatusBar、B2 ToolBar、B3 RangeSlider、B4 Stepper、B5 Rating、B7 Carousel/Pagination、B9 TreeGrid 均已完成；B6 FontDialog/PrintDialog、B8 富内容 Tooltip、B10 打印/导出通道明确不做。

### 五、平台整合能力规划（应用骨架层）

控件层之外，桌面软件还依赖一组"系统整合"能力。当前库只有 ProcessSingleton（单实例）、TrayIcon（托盘）、Clipboard、FileDialog、ScreenCapture 等少数几个，以下能力缺失，是"减少各平台桌面开发成本"的关键空白，建议作为与控件层并列的第二条演进线：

| # | 能力 | 现状 | 说明 | 优先级 | 状态（2026-10-21） |
|---|------|------|------|------|------|
| 18 | 无障碍（Accessibility/屏幕阅读器） | **缺失** | Windows 需接 UI Automation、macOS 接 NSAccessibility、Linux 接 AT-SPI；企业/政务软件合规刚需，也是跨平台桌面库的硬门槛。可先做"控件名/值/角色的可访问属性标注 + Windows UI Automation 适配"，其余平台逐步补齐 | 高 | ❌ 未启动（跟踪 Development-Plan F-2） |
| 19 | 原生系统通知 | Toast 为应用内自绘 | 接入 macOS 通知中心、Windows 桌面通知、Linux（D-Bus/Freedesktop）的原生通知，与现有 Toast 形成"应用内 + 系统级"双通道 | 高 | 🟡 部分完成（托盘气泡已通过 SDL 系统通知在非 Windows 落地，2026-10-21；Toast 仍为应用内自绘，通用通知 API 未封装） |
| 20 | 应用内更新框架 | **缺失** | 跨平台自更新：Windows 自研或 WinSparkle、macOS Sparkle、Linux 走包管理器/AppImage | 中高 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| 21 | 文件关联 / URL Scheme / 深链 | **缺失** | 注册协议→唤起应用→解析参数，桌面软件"打开方式"与"外部唤起"标配 | 中 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| 22 | 开机自启 + 托盘菜单联动 | TrayIcon 已有 | 补齐托盘右键菜单 API、开机自启注册（登录项/启动项） | 中 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| 23 | 安装/打包/签名/公证 | **缺失** | 各平台安装器（NSIS/MSIX、DMG、AppImage/deb/rpm）、代码签名、macOS 公证（notarization）。这是"桌面开发成本"的大头，提供文档化标准流程即可，不必造轮子 | 高（工程化） | 🟡 部分完成（B-3 已完成 `build/pack.py` + `docs/Release.md`，2026-10-20；安装器不做，签名/公证仍为手工流程） |

### 六、开发者体验与工程化

| # | 项 | 说明 | 优先级 | 状态（2026-10-21） |
|---|------|------|------|------|
| 24 | 可视化布局/主题编辑器 | 目前 XML 手写无设计器。这是"减少界面开发成本"最直接的杠杆，但工程量极大，建议作为独立长期项目单独立项，不进控件规划主流程 | 远期战略 | ❌ 未启动 |
| 25 | 控件单元测试基建 | 当前无 C++ 测试框架接入；为控件状态机（日历、表格、树）补基础测试，降低回归风险 | 中 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| 26 | 平台适配矩阵文档 | 各控件在 Windows/Linux/macOS/FreeBSD 的支持差异散落各文档，补一张总表，明确"哪些控件在哪些平台降级/未实现" | 中 | ✅ 已完成（Project-Audit 第 3 章 + `跨平台开发相关事项.md` 已回写，2026-10-21；HotKey / macOS / FreeBSD 实测仍待补） |

> 注：本文档为规划性质，具体实现方案（接口、XML 属性、皮肤资源）待各项启动时另行设计。
