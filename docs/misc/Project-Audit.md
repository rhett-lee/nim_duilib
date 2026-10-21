# 项目盘点与改进建议（源码 / 文档 / 跨平台）

> 本文档是一次全项目盘点结论，用于后续排期与跟踪。**状态截至 2026-10-21**：第一章的 ColorPicker 回环与静默赋值已完成，B1–B5 / B7 / B9 已完成，第三章文档漂移已回写，第四章 C3 / C4 / C6 已完成；**C1 / C2 / C5 / C7 / C8 及 C9 剩余项已明确不做、不再跟进**。剩余项见各表「状态」列、第五章顺序与文末附录。

## 扫描范围与方法

| 项 | 说明 |
| :--- | :--- |
| 源码 | `duilib/Control`（83 个头文件）、`duilib/Core`、`duilib/Utils`、`duilib/RenderSkia`、`duilib/Box`、`duilib/Layout`、`duilib/CEFControl`、`duilib/Image` |
| 文档 | `docs/`（55 份，含中英双语对） + `README.md` + `docs/misc/` 专项文档 |
| 平台维度 | Windows / macOS / Linux / FreeBSD，按"平台专属源文件矩阵"逐项核对 |
| 方法 | 类名与 `DUI_CTR_*` 宏检索确认控件是否存在；`_Windows` / `_SDL` / `_MacOS` / `_Linux` / `_X11` / `_Wayland` 文件配对确认平台覆盖；与既有规划文档（[Control-Roadmap.md](Control-Roadmap.md)、[跨平台开发相关事项.md](misc/跨平台开发相关事项.md)、[ListCtrl-Roadmap.md](misc/ListCtrl-Roadmap.md)）交叉比对，避免重复已记录项 |

### 与既有规划文档的关系

* [Control-Roadmap.md](Control-Roadmap.md) 的控件新增清单（第 1–18 项）**已全部清空**；其"平台整合能力"（第 18–23 项）与"开发者体验与工程化"（第 24–26 项）中，原生系统通知已完成，平台适配矩阵文档已完成；无障碍、安装/签名/公证剩余项及第 20–22、25 项明确不做（详见该文档第五、六节状态列）；
* 本文档补充的是 Roadmap 中**尚未识别**的三类问题：① 健壮性类缺陷（控件值同步回环）；② 规划清单之外的控件空白（壳层/工具条类）；③ 跨平台矩阵的实际状态与文档漂移。

---

## 一、健壮性：控件值互相同步导致的无限递归（最高优先级）

### 1.1 已发生的故障：ColorPicker 启动即卡死

`ColorPicker_d.exe` 启动首帧即失去响应。堆栈显示为一条自我循环的事件递归链：

```
Window::OnPreparePaint → ArrangeRoot → ColorPickerCustom::SetPos → InitPicker
  → OnColorChanged → UpdateRGB / UpdateHSV / UpdateHSL
  → ColorSlider::SetValue → kEventValueChanged → 各分量滑块回调
  → OnRGBChanged / OnHSVChanged / OnHSLChanged → OnColorChanged  ← 回到起点
```

* 根因：[ColorPickerCustom.cpp](../duilib/Control/ColorPickerCustom.cpp#L212-L374) 用 `ColorSlider::SetValue` 把颜色扇出写入 RGB / HSV / HSL 三组滑块，滑块同步派发 `kEventValueChanged`，回调再次进入 `OnColorChanged`；由于颜色量化后数值持续微变（`Progress::SetValue` 仅在值变化时派发事件），该环不收敛，首帧绘制永不返回。
* 实测：修复前进程 5 秒内消耗 CPU 时间 9.25s、`Responding=False`；修复后 0.078s、`Responding=True`，界面数值一致（RGB `255/255/255`、HSV `H0 S0 V100`、HSL `H0 S0 L100`）。
* 已落地修复：`ColorPickerCustom` 增加 `m_bUpdatingColorUI` 重入守卫（[ColorPickerCustom.h](../duilib/Control/ColorPickerCustom.h#L133-L136)、[ColorPickerCustom.cpp](../duilib/Control/ColorPickerCustom.cpp#L150-L157)、[ColorPickerCustom.cpp](../duilib/Control/ColorPickerCustom.cpp#L209)）。

### 1.2 框架级缺口（部分实施：A1/A3 已完成，A2 未实施）

| # | 缺口 | 证据 | 影响 |
| :--- | :--- | :--- | :--- |
| A1 | **缺"静默赋值"能力**：控件以代码方式设置值时无法抑制事件派发 | 检索 `duilib/Control` 下 `SetValueNoEvent` / `SetValueSilent` 无命中；现有静默机制 `SetTextNoEvent` 仅覆盖文本类控件 | 任何"一个事件源驱动多个控件同步"的场景都依赖调用方自行打补丁，容易漏 |
| A2 | 缺统一的重入守卫约定 | 各控件靠 `needUpdate` / `reason` 参数自行判断，无通用机制；`ColorPickerCustom` 是全库唯一做分量同步的控件 | 同类问题在新增控件时容易重新引入 |
| A3 | 未做全库同类回环排查 | 本轮仅按 `AttachValueChanged + SetValue + On*Changed` 模式做过一轮定位，尚无系统性排查记录 | 可能仍有未暴露的同类缺陷 |

### 1.3 待核实清单（同类风险候选）

以下联动对与 ColorPicker 模式相同（事件回调中写另一个控件、被写控件又派发事件），需要逐一核实是否存在环、是否已有防护：颜色类控件（`ColorSlider` / `ColorControl` / `ColorPickerRegular` / `ColorPickerStandardGray`）、`ComboBox` 与 `ListBox`（选中联动）、`DateTime` 与 `CalendarFlyout`、`SpinBox` 与 `RichEdit`、`TabBox` / `TabCtrl` 与 `NavigationView`（选中与页面切换联动）、`PropertyGrid`、`TreeView` 勾选级联、`ListCtrl` 与 `ListCtrlHeader`（排序 / 列显隐联动）。

**排查结论（2026-10-21）**：上述候选已逐一核对，除 `ColorPickerCustom` 外未发现新的实际回环；相关联动均已具备重入保护或采用单向事件。新增控件统一使用 `Progress::SetValueNoEvent`（见 1.4 第 1 条），框架级重入守卫仍不引入（1.4 第 2 条）。

### 1.4 建议动作

1. 在 `Progress` / `Slider` 层增加 `SetValueNoEvent()`（对标既有 `SetTextNoEvent` 语义），并把控件内部所有"同步其它视图"的赋值统一切换到该接口；**（✅ 已完成：`Progress::SetValueNoEvent`，`Slider`/`ColorSlider` 继承；`ColorPickerCustom` 的 7 处同步赋值已切换）**
2. 提供通用重入守卫（基类成员或宏），并在 `_DEBUG` 下增加"事件派发重入深度"断言，使同类缺陷在开发期暴露；**（⏳ 未实施：经 A3 排查确认同类回环均已具备防护，暂不引入框架级守卫）**
3. 按 1.3 清单做一次专项排查，结论回写到本文档。**（✅ 已完成：见 1.3 结论）**

---

## 二、控件层缺口

### 2.1 确认缺失的控件（类名与 `DUI_CTR_*` 宏均无命中）

| # | 控件 | 用途 / 缺失影响 | 优先级 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| B1 | StatusBar 状态栏 | 光标位置、字数、连接状态、后台进度等状态提示；编辑器 / IDE / 通信类软件标配 | 高 | ✅ 已完成（`duilib/Control/StatusBar.h/.cpp`，节点 `StatusBar`；示例 `examples/controls/StatusBarForm`） |
| B2 | ToolBar 工具栏容器 | 目前只有 `MenuBar`，工具条需手写布局（按钮 + 分隔 + 溢出菜单）；属于"应用壳层"刚需 | 高 | ✅ 已完成（`duilib/Control/ToolBar.h/.cpp`，节点 `ToolBar`；宽度不足时自动收纳尾部子控件并在右端显示"更多"按钮，点击弹出浮层并通过 `AttachOverflowItemActivated` 回调通知应用层；示例 `examples/controls/ToolBarForm`） |
| B3 | RangeSlider 双滑块范围选择 | 价格 / 时间区间筛选；现有 `Slider` 只有单值 | 中 | ✅ 已完成（`duilib/Control/RangeSlider.h/.cpp`，节点 `RangeSlider`；一条轨道上放置下限/上限两个滑块并高亮选中区间，支持 `min_range` 最小间距、`range_color`/`range_radius` 高亮样式、点击轨道跳值；范围变化通过 `AttachRangeChanged` 回调，`pEventData` 指向 `RangeValue` 携带 low/high；示例 `examples/controls/RangeSliderForm`） |
| B4 | Wizard / Step 向导步骤条 | 安装、注册、配置向导；`Timeline` 只能模拟，语义不符 | 中 | ✅ 已完成（`duilib/Control/Stepper.h/.cpp`，节点 `Stepper`；水平/垂直排列，节点显示序号或对勾，状态由"当前索引 + 禁用集合"派生为待开始/进行中/已完成/禁用，连接线按完成度着色，支持 `Next`/`Back`/`GoTo` 与点击跳转、多语言 `step_title_id`/`step_desc_id`；步骤变化通过 `AttachStepChanged` 回调，`pEventData` 携带 currentIndex/oldIndex；示例 `examples/controls/StepperForm`） |
| B5 | Rating 评分（星级） | 评价类界面高频控件 | 中 | ✅ 已完成（`duilib/Control/Rating.h/.cpp`，节点 `Rating`；星形完全由 SVG 图片驱动（不自绘），四类星图 实心/半星/空心/悬停，颜色经 `svg_replace_colors` 映射主题语义色随主题切换；支持只读展示、半星（`allow_half`，步长 0.5）、悬停预览、点击/拖拽打分，数值经 `value`/`max` 钳位并按步长对齐；评分变化通过 `AttachRatingChanged` 回调，`pEventData` 指向 `RatingValue` 携带 value/oldValue；默认皮肤 `rating` 类内置星图，XML 中 `class="rating"` 即可引用；示例 `examples/controls/RatingForm`） |
| B6 | FontDialog / PrintDialog / PrintPreview | 编辑器与报表类必需；当前系统对话框只有 `FileDialog` | 中 | ❌ 不开发（无使用场景） |
| B7 | Carousel / 分页器 / 无限滚动 | 图片轮播、卡片流 | 中 | ✅ 已完成（`duilib/Control/Carousel.h/.cpp`，节点 `Carousel`，图片轮播：`item_image` 幻灯片列表 + 左右导航按钮 + 底部页码指示点，外观全由 SVG 图片驱动、颜色经 `svg_replace_colors` 映射主题语义色；支持 `loop` 无限循环 / `auto_play`+`hover_pause` / `transition_ms` 水平滑动动画 / 鼠标拖拽翻页；索引变化 `AttachCarouselChanged` → `kEventValueChanged`，`pEventData` 指向 `CarouselValue`。`duilib/Control/Pagination.h/.cpp`，节点 `Pagination`，继承 `HBox` 的分页器：动态创建页码按钮、超出 `max_visible_count` 以 `…` 收纳并保留首尾、当前页选中高亮；页码变化 `AttachPageChanged` → `kEventValueChanged`，`pEventData` 指向 `PaginationValue`。示例 `examples/controls/CarouselForm`） |
| B8 | 富内容 Tooltip | `Tooltip` 仅纯文本；Windows 平台使用系统原生 Tooltip 实现，其他平台使用自绘 Tooltip；`Flyout` 地基已就绪，但未封装为富提示控件 | 中 | ❌ 不处理 |
| B9 | 树表格 TreeGrid / 列筛选 | ListCtrl 规划中已明确取消（含内嵌树、分组折叠、L3 筛选），若出现企业报表需求需重新评估 | 低（按需） | ✅ 已完成（改为**新增独立控件**落地，不改动 `ListCtrl`；`duilib/Control/TreeGrid.h/.cpp`，节点 `TreeGrid`：继承 `VBox`，内部由"固定不滚动的表头 + 自带纵向滚动条的列表体（VListBox）"组成，表头与数据行复用同一列宽数组天然逐列对齐；`column_title`/`column_width`/`column_id`/`column_align` 配置多列，`tree_column`+`indent` 控制层级缩进（缩进用树列单元格内嵌缩进占位，其它列不错位）；展开/收起切换子孙行可见性，右键 `Left`/`Right` 收起/展开；`multi_select` 行多选；`check_mode`+`check_cascade` 行首复选框父子三态级联（全选/部分选/未选）；数据节点 `TreeGridNode` 提供增删查与 `SetCellText`/`SetUserData`；勾选变化 `AttachCheckChanged` → `kEventTreeGridCheckChanged`（`pEventData` 指向 `TreeGridCheckValue`），其余事件 `kEventExpand`/`kEventCollapse`/`kEventClick`/`kEventSelect`/`kEventUnSelect`/`kEventSelChanged`（`pEventData` 为 `TreeGridNode*`）；主题 `tree_grid` 及子类走语义色，展开箭头与三态复选框复用 `${PATH_TREE}`/`${PATH_CHECKBOX}` 既有 SVG；示例 `examples/controls/TreeGridForm`。**L3 列筛选不做**） |
| B10 | 打印 / 导出通道 | 除 ListCtrl 的 CSV 导出外，无统一打印与导出（PDF / XLSX）通路 | 低 | ❌ 不开发（无使用场景） |

### 2.2 既有控件的补强点

本节列出的既有控件补强点**均不处理**。

| 控件 | 补强建议 |
| :--- | :--- |
| `Tooltip` | 复用 `Flyout` 实现自绘富内容提示，减少对 Windows 系统原生实现的依赖 |
| `Slider` | 增加双滑块（范围）形态，与 B3 合并实现 |
| `DateTime` | 非 Windows 平台能力补齐（见第三章），并把自绘 `Calendar` 作为默认编辑方式 |
| `RichEdit`（非 Windows） | 补齐 RichText 能力，缩小与 Windows 版的能力差 |
| `Chart` / `Timeline` | 已有能力较完整，建议补"大数据量下的绘制性能"与"交互无障碍"两项（后者依赖无障碍改造） |

---

## 三、跨平台支持矩阵与缺口

### 3.1 已具备跨平台实现的核心能力（按文件配对确认）

| 模块 | Windows | macOS | Linux | FreeBSD | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 窗口创建 / 消息循环 | `NativeWindow_Windows` | `NativeWindow_SDL` | `NativeWindow_SDL` | `NativeWindow_SDL` | FreeBSD 复用 Linux/X11 分支（待实测） |
| 渲染（Skia CPU 光栅） | `SkRasterWindowContext_Windows` | SDL 版 | SDL 版 | SDL 版 | 已完成 |
| 键盘 / 光标 | `Keyboard_Windows`、`CursorManager_Windows` | SDL 版 | SDL 版 | SDL 版 | 已完成 |
| DPI 感知 | `DpiAwareness_Windows` | `DpiAwareness_SDL` | `DpiAwareness_SDL` | 同左 | 非 Windows 平台"暂不支持"缩放感知 |
| 剪贴板 | `Clipboard_Windows` | `Clipboard_SDL` | `Clipboard_SDL` | 同左 | 已完成 |
| 文件对话框 | `FileDialog_Windows` | `FileDialog_SDL` | `FileDialog_SDL` | 同左 | SDL 多选返回值有上游 Bug |
| 托盘图标 | `TrayIcon_Windows` | `TrayIcon_SDL` | `TrayIcon_SDL` | 同左 | 已完成 |
| 屏幕截图 / 取色 | `ScreenCapture_Windows` | `ScreenCapture_MacOS.mm` | `ScreenCapture_X11/Wayland/Linux` | 复用 X11 | **已有非 Windows 实现，旧文档描述过期** |
| 目录树 | `DirectoryTreeImpl_Windows` | `DirectoryTreeImpl_MacOS` | `DirectoryTreeImpl_Linux` | 复用 Linux | 已完成 |
| 系统提示框 | `ToolTip_Windows` | `ToolTip_SDL` | `ToolTip_SDL` | 同左 | 已完成 |
| 文件拖放（窗口级） | `WindowDropTarget_Windows` | `WindowDropTarget_SDL` | 同左 | 同左 | 已完成 |
| 线程间消息 | `ThreadMessage_Windows` | `ThreadMessage_SDL` | 同左 | 同左 | 已完成 |
| RichEdit | `RichEdit_Windows` | `RichEdit_SDL` | 同左 | 同左 | 缺 RichText 能力 |

### 3.2 仅 Windows 有实现的缺口

| 能力 | 证据（仅 Windows 侧文件） | 影响 |
| :--- | :--- | :--- |
| OpenGL / GPU 渲染后端 | `RenderSkia/SkGLWindowContext_Windows.*`（SDL 侧"暂不支持，待调研"） | 非 Windows 只能 CPU 光栅，高分辨率 / 高刷新场景性能受限 |
| Web 嵌入（CEF / WebView2） | `CEFControl/CefManager_Windows.*`、`CefWindowUtils_Windows.*` | 非 Windows 无任何 Web 内容方案 |
| 单实例 | `Utils/ProcessSingleton_Windows.h` | 非 Windows 无法做单实例互斥 |
| 点击穿透 | `Core/ClickThrough_Windows.cpp` | 非 Windows 不支持穿透型浮层 |
| 不规则窗口区域 / 圆角裁剪 | `RenderSkia/WindowRgn_Windows.*` | 非 Windows 依赖 SDL 能力，自定义形状窗口受限 |
| 动态库 / 插件加载 | `Utils/DllManager_Windows.*` | 非 Windows 需补 `dlopen` 实现 |
| 磁盘 / 系统信息 | `Utils/DiskUtils_Windows.*` | 非 Windows 缺失 |
| 内联 Hook | `Utils/InlineHook_Windows.*` | 平台专属，非缺口但需在矩阵中标明 |
| 控件级拖拽源 | `Control/RichEditDragSource_Windows.*` | 待核实非 Windows 是否具备控件级拖拽 |

### 3.3 Linux 已知问题（记录于[跨平台开发相关事项.md](misc/跨平台开发相关事项.md#L138-L156)）

1. 自定义（半透明 + 阴影）标题栏：双击标题栏无法最大化 / 还原（需 SDL 底层支持）；
2. 调整窗口大小时黑屏（消息过多导致绘制不及时，多数发行版存在，需 SDL 底层支持）；
3. 窗口最大化还原后位置不正确（原因待定位）；
4. `ShadowWnd_SDL` 处于开发中：SDL 不支持修改窗口 Z-Order，阴影 + 半透明标题栏达不到预期效果；
5. Wayland 未测试；强制 Wayland 时无 DPI 缩放、高 DPI 界面模糊；部分发行版（DDE、OpenSUSE）透明窗口不可用、阴影显示为黑色。

### 3.4 macOS 与 FreeBSD

* 代码层面已有 SDL 实现（含 `ScreenCapture_MacOS.mm`、`DirectoryTreeImpl_MacOS`），但**没有任何实测记录**：平台测试表仅覆盖 Linux 发行版；
* FreeBSD 无专属实现文件，复用 Linux / X11 分支，同样无构建与运行验证记录；
* 建议：把 macOS / FreeBSD 纳入构建 + 冒烟测试矩阵后再对外声明支持。

### 3.5 文档漂移（跨平台部分）

[跨平台开发相关事项.md](misc/跨平台开发相关事项.md) 中以下描述与源码现状不符，需要更新（该文档是第三章矩阵的直接来源，建议以本文档 3.1/3.2 为准重建）。**2026-10-21 已按下表回写该文档；仅 HotKey 仍待核实。**

| 文档描述 | 实际情况 |
| :--- | :--- |
| `ScreenCapture`：`不予支持`，其他平台不支持屏幕取色 | 已存在 `ScreenCapture_X11 / Wayland / Linux / MacOS` 实现 |
| `ToolTip`：其他平台暂不支持 | 已存在 `ToolTip_SDL` |
| 日期控件：暂不支持，需要调研替代方案 | 自绘 `Calendar` + `CalendarFlyout` 已落地，`DateTime` 的 `date_calendar` 已切换 |
| `DragDrop`：暂不支持 | `WindowDropTarget_SDL`、`ControlDropTargetImpl_SDL` 均已存在 |
| `HotKey`：暂不支持 | 待核实（`HotKey.cpp` 中未见平台分支） |
| `RichEdit`：待开发 | `RichEdit_SDL` 已实现基础文本编辑能力 |

---

## 四、工程化与文档

| # | 项 | 现状 | 建议 | 优先级 | 状态（2026-10-21） |
| :--- | :--- | :--- | :--- | :--- | :--- |
| C1 | CI 构建 | `.github` 下只有 ISSUE / PR 模板，**无任何 workflow** | 至少增加 Windows + Linux 的编译矩阵，阻止跨平台回归 | 高 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| C2 | 单元 / 回归测试 | 无 `tests/` 目录，未接入测试框架 | Skia CPU 后端支持**离屏无窗口渲染**，建议做"控件截图回归测试"（无头加载 XML → 比对 PNG），另对控件状态机（日历、表格、树）加逻辑单测 | 高 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| C3 | 平台适配矩阵文档 | 规划中（Roadmap 第 26 项），且现有跨平台文档已过期 | 直接以本文档第三章为基线，替换过期描述 | 高 | ✅ 已完成（本文档第三章 + `跨平台开发相关事项.md` 已回写，2026-10-21；HotKey / macOS / FreeBSD 实测仍待补） |
| C4 | 文档漂移 | 全库存在约 50 处示例 / 占位名与源码不符（集中在中英双语的 Box / Global / Menu / Theme / ThemeColor 章节） | 跑 `verify_docs.py` 清理，不要用豁免注释掩盖 | 中 | ✅ 已完成（`verify_docs.py --extra docs` 零漂移，2026-10-21） |
| C5 | 无障碍（Accessibility） | **完全缺失**：全库无 `IAccessible` / UI Automation / AT-SPI / NSAccessibility 相关代码 | 先做"控件名 / 值 / 角色的可访问属性标注 + Windows UI Automation 适配"，其余平台逐步补齐；企业 / 政务合规刚需 | 高 | ❌ 明确不做（2026-10-21 决策，不再跟进；对应 Development-Plan F-2） |
| C6 | 原生系统通知 | Toast 为应用内自绘 | 接入 Windows 桌面通知、macOS 通知中心、Linux D-Bus 通知，形成双通道 | 高 | ✅ 已完成（2026-10-21：`duilib/Utils/SystemNotification.h`，SDL 构建走 SDL 系统通知，非 SDL 的 Windows 构建走 `Shell_NotifyIcon` 并兼容 Win7+；controls 示例已加入口，Toast 未改动） |
| C7 | 应用内更新框架 | **缺失** | Windows 自研或 WinSparkle、macOS Sparkle、Linux 包管理器 / AppImage | 中高 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| C8 | 文件关联 / URL Scheme | **缺失** | 注册协议 → 唤起应用 → 解析参数 | 中 | ❌ 明确不做（2026-10-21 决策，不再跟进） |
| C9 | 安装 / 打包 / 签名 / 公证 | **缺失** | 各平台安装器（NSIS / MSIX、DMG、AppImage / deb / rpm）、代码签名、macOS 公证；以文档化标准流程落地即可 | 高（工程化） | ❌ 明确不做（2026-10-21 决策，不再跟进；B-3 已交付 `build/pack.py` + `VERSION` + `docs/Release.md`，安装器与签名/公证交由使用方按目标平台处理） |
| C10 | 可视化布局 / 主题编辑器 | 无，XML 手写 | 工程量极大，建议独立长期立项，不进控件规划主线 | 远期 | ❌ 未启动 |

---

## 五、建议落地顺序（2026-10-21 更新）

**已完成（不再排期）**：

- 第一章健壮性：ColorPicker 回环修复、`Progress::SetValueNoEvent`、同类回环专项排查；
- 控件层：B1 StatusBar、B2 ToolBar、B3 RangeSlider、B4 Stepper、B5 Rating、B7 Carousel / Pagination、B9 TreeGrid（B6 / B8 / B10 明确不做）；
- 第三章平台矩阵与文档回写（C3）、第四章文档漂移清理（C4）。

**明确不做（2026-10-21 决策，不再跟进）**：

- 工程化：CI 构建（C1）、单元 / 回归测试（C2）、应用内更新（C7）、文件关联 / URL Scheme（C8）；
- 发布：安装器 / 签名 / 公证剩余项（C9，B-3 的打包脚本已交付）；
- 系统能力：无障碍（C5）；
- 控件层：B6 FontDialog / PrintDialog / PrintPreview、B8 富内容 Tooltip、B10 打印 / 导出通道；ListCtrl 列汇总行 / 分组行（Development-Plan E-6①）；
- 既有控件补强（2.2 节全部）。

**剩余优先级**：

1. **跨平台实测**：macOS / FreeBSD 构建与冒烟；Linux 已知问题（标题栏双击、调整大小黑屏、最大化位置、Wayland）；核实 `HotKey` 与控件级拖拽源；
2. **平台差异与生态**：GPU 渲染后端、Web 嵌入的非 Windows 替代方案，按实际需求评估；可视化编辑器（C10，远期）。

## 附录：待核实项清单（2026-10-21 更新）

1. ~~1.3 节同类回环候选~~：✅ 已完成（2026-10-21，除 `ColorPickerCustom` 外未发现新增回环）；
2. `HotKey`、控件级拖拽源（`RichEditDragSource`）在非 Windows 平台的实现状态：仍待核实；
3. FreeBSD 复用 Linux / X11 分支的实际可运行性：仍待实测；
4. macOS 各项能力的实测结论：仍无记录；原生通知还需在 `.app` 包内验证；
5. `Control-Roadmap.md` 已取消项（ListCtrl 内嵌树 / 分组折叠 / L3 列筛选）：已决定不做，TreeGrid 独立控件已落地；如出现企业报表需求再重新评估。
