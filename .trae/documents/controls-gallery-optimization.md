# controls 示例程序优化方案（控件画廊）

## 一、背景与目标

`examples/controls` 当前是"大杂烩"式页面：主窗口 `controls.xml` 把按钮、输入框、列表、进度条、RichEdit 源码等全部平铺；点"Control Test"后再进入 `test.xml` 这层按钮列表，再到具体测试页，层级混乱，首屏也不够美观。

本次改造目标：

1. 主窗口只展示**最常用控件**及其**基本外观**，让用户一眼看清框架有哪些常用控件。
2. 主窗口采用彩色 Hero + 分类卡片画廊，提升美观度，并保留明暗主题适配。
3. 主窗口是**唯一入口**：每张卡片直接打开对应控件的**专用测试页**（二级页面），页面内可完整手工测试。
4. 标题栏、卡片、字号、语义色与 `basic`、`color_theme` 等示例保持统一。
5. 允许大幅调整：修复现有 XML/代码问题，移除冗余页面层级，补齐核心控件测试页。

已确认的用户选择：

- 主界面形态：**单页滚动画廊**
- 测试页范围：**补齐核心控件**（Combo、ListBox/TreeView、RichEdit、DateTime 等）
- 原 RichEdit 源码大区域：**移除**，改为卡片入口

## 二、现状梳理（关键事实）

- 主窗口：[ControlForm.cpp](file:///e:/develop/nim_duilib/examples/controls/ControlForm.cpp) → `controls.xml`
- 二级入口页：[TestForm.cpp](file:///e:/develop/nim_duilib/examples/controls/TestForm.cpp) → `test.xml`（本质是第三级页面的按钮导航）
- 现有专用测试页：switch、button_checkbox、progress、tabctrl、rangeslider、stepper、rating、carousel、tree_grid、breadcrumb、cascading_select、popover、popconfirm、avatar、chart、marquee、timeline、statusbar、toolbar、navigation、animation、message_box_test、toast_test 等。
- 主题资源：`bin/resources/themes/default/controls/*.xml`，本地文件方式加载，**改 XML 无需重新编译**。
- 颜色/样式：`global.xml` 提供 `btn_global`、`btn_primary_global`、`btn_global_color_*`、`panel_card` 参考写法、`bg_window_card`、`border_window`、`color_accent/success/warning/error` 等。
- 渐变背景：`bkcolor2` + `bkcolor2_direction`（1 横 / 2 纵 / 3 左上→右下 / 4 右上→左下），见 `render/page_property_grid.xml`。
- CMake 使用 `aux_source_directory` 自动收集 `.cpp`（`cmake/duilib_bin.cmake:61`），新增源文件需重新执行 CMake 配置；VS 工程 `controls.vcxproj(.filters)` 需同步登记。
- 多语言：XML 用 `text_id`，词条在 `bin/resources/lang/zh_CN-examples.txt`、`en_US-examples.txt`。

## 三、主界面改造（controls.xml 重写）

### 3.1 窗口与标题栏

- `size="1200,820"`、`min_size="960,640"`，保留 `caption`、`shadow_type`、`size_box`、`icon`。
- 标题栏沿用 `basic.xml` 写法：logo + "Controls Gallery" + 主题/语言/全屏/最小化/最大化/关闭按钮。
- 工作区整体放入 `VScrollBox`，保证小窗口不被裁切。

### 3.2 首屏 Hero 区

- 渐变卡片：`bkcolor="color_accent"` + `bkcolor2="color_blue_light"` + `bkcolor2_direction="3"`，圆角 12。
- 内容：logo、标题（白色 `system_bold_20`）、副标题（白色 12 号）、右侧 3 个"特性胶囊"（常用控件 / 明暗主题 / 手工测试）。
- 下方一行 4 个统计小卡片：基础控件、输入控件、数据展示、反馈浮层，使用不同语义色竖条区分。

### 3.3 分类卡片画廊（5 个分区）

每个分区：左侧 4×18 彩色竖条 + 粗体分区标题 + 灰色说明 + 分隔；下方卡片按行排布（每行 2~3 张）。

卡片统一结构（VBox，`bg_window_card` + 1px `border_window` + 圆角 8 + padding 14）：

```
彩色小色块/图标 + 控件名（bold 15） + ui::Xxx（muted 12）
------------ 实时预览区 ------------
说明文字（muted 12）
[打开测试页]（btn_global 小号）
```

分区与卡片（约 20 张，全部直接跳二级页，不再经过 test.xml 中转）：

1. **基础控件**（accent 色条）
   - Button 按钮（实时：主/次/危险按钮）→ ButtonCheckBoxForm
   - CheckBox / Option（实时）→ ButtonCheckBoxForm
   - Switch 开关（实时 3 个）→ SwitchForm
   - Badge 角标（实时）→ ButtonCheckBoxForm（该页新增 Badge 演示区）
   - Label / HyperLink / Line / RichText（实时小样例，不设专属页）
2. **输入控件**（success 色条）
   - RichEdit 文本输入（实时单行 + Spin）→ 新增 RichEditForm
   - Combo 下拉（实时 3 个）→ 新增 ComboForm
   - DateTime 日期时间（实时 3 个）→ 新增 DateTimeForm
   - SearchBox / TagInput / AutoComplete（实时）→ TestForm（改造为交互控件页）
   - IPAddress / HotKey（实时）→ TestForm
3. **数据展示**（blue 色条）
   - Progress / Slider / CircleProgress（进度条保留现有定时器动画）→ ProgressForm
   - ListBox / TreeView（实时）→ 新增 ListViewForm
   - TreeGrid 树表格（实时小表）→ TreeGridForm
   - Chart 图表（实时小图）→ ChartForm
   - Rating / Stepper（实时）→ RatingForm / StepperForm
4. **反馈与浮层**（warning 色条）
   - MessageBox（按钮）→ MessageBoxForm
   - Toast / SystemNotification（按钮）→ ToastForm / 直接调用
   - Popover / Popconfirm（按钮）→ 各自页面
   - Flyout / Calendar（4 向按钮 + 单选/范围）→ TestForm
   - Mask 遮罩 / 右键菜单（按钮）→ TestForm
5. **导航与布局**（error 色条）
   - TabCtrl 标签栏 → TabCtrlForm
   - Navigation / Breadcrumb → 各自页面
   - CascadingSelect → CascadingSelectForm
   - Avatar / Carousel → 各自页面
   - RangeSlider / Timeline / Marquee / Animation / StatusBar / ToolBar → 各自页面（紧凑按钮组）

> 说明：主界面不放大段 RichEdit；原 `LoadRichEditData`、`OnResourceFileLoaded` 及相关线程任务删除。

### 3.4 主界面交互（C++）

改造 `ControlForm::OnInitWindow()`：

- 删除已不存在控件的绑定（list/tree/combo/filtercombo/checkcombo/edit/edit2/circleprogress/rich_text/hyper_link/hotkey/colorpicker 等）。
- 保留：进度条定时刷新、托盘图标开关、设置菜单、主题/语言按钮（框架自带）。
- 新增按钮绑定：`rich_edit_btn`、`combo_btn`、`list_view_btn`、`datetime_btn`、`badge` 相关、MessageBox/Toast/Popover/Popconfirm/Flyout/Calendar/Mask 等直达入口。
- 抽取 `ShowChildForm(ui::WindowImplBase* pForm, const DString& title)` 私有辅助函数，统一 `WindowCreateParam` + `CreateWnd` + `ShowModalFake`，消除重复代码。

## 四、二级页面改造与新增

### 4.1 统一规范

所有 `controls/*.xml` 二级页统一：

- 自定义标题栏：logo + 标题（`text_id`）+ `btn_switch_theme` + `btn_switch_lang` + `btn_window_close`；移除 `use_system_caption="true"`（test.xml 当前例外）。
- 顶部一行灰色提示（`text_muted`，`auto_tooltip="true"`）。
- 主体 `VScrollBox` + 分区卡片（`bg_window_card` / `border_window` / 圆角 8）。
- 分区标题使用 `system_bold_15`，说明使用 `system_regular_12`。
- 补齐 `avatar.xml`、`carousel.xml`、`tree_grid.xml` 等缺少卡片化和主题按钮的页面。

### 4.2 test.xml 重新定位

- 从"三级导航按钮列表"改为 **交互控件专用测试页**：
  - TagInput（含 delimiter 变体、添加标签）
  - AutoComplete（前缀/包含匹配、代码数据源）
  - SearchBox（建议 + 历史 + 防抖）
  - Flyout（上/下/左/右）
  - Calendar（单选 + 范围）
  - 右键菜单（AttachPopupMenu）
  - Mask（模态 / 新手引导 / 加载遮罩）
  - SystemNotification
  - IPAddress / HotKey / RichText / HyperLink / Line 基础元素
- `TestForm` 删除已迁出的 `Show*Window()`，仅保留上述交互逻辑。

### 4.3 新增页面（4 个）

| 页面 | XML | 新增类 | 内容要点 |
|---|---|---|---|
| Combo | `combo.xml` | `ComboForm` | drop_list / drop_down / 可编辑；FilterCombo 过滤；CheckCombo 多选；禁用态；事件回显（代码填充数据） |
| List | `list_view.xml` | `ListViewForm` | ListBox、VListBox、HListBox、TreeView 多级节点；选中/双击事件回显；滚动条样式 |
| RichEdit | `rich_edit.xml` | `RichEditForm` | 多行 RichEdit/RichEdit2、单行 prompt、数字 Spin、全屏、拖放文件、右键菜单 |
| DateTime | `datetime.xml` | `DateTimeForm` | date_up_down / date_time_up_down / minute_up_down / date_time_calendar / time_calendar；show_seconds、min/max、disabled_dates、first_day_of_week；值变化回显 |

新增类统一继承 `ui::WindowImplBase`，`GetSkinFolder()` 返回 `"controls"`，`GetSkinFile()` 返回对应 XML；事件绑定后通过 `ToastWnd` 回显。

> Combo 选项必须由代码注入（`Combo::GetTreeView()->AddChildNode` / `AddTextItem`），不能在 XML 里直接写子节点，见 `ControlForm.cpp:91-140`。

### 4.4 Badge 演示补充

在 `button_checkbox.xml` 末尾增加 "Badge 角标" 分区：count=5、count=120（>99 显示 99+）、红点、浮动定位在按钮右上角。

## 五、多语言

- 新增卡片标题/说明/按钮文案使用 `text_id`，在 `zh_CN-examples.txt` 与 `en_US-examples.txt` 同步增加 `STRID_GALLERY_*` 词条（约 40 条）。
- 控件专有名词（Button、Switch、TreeGrid 等）直接使用 `text`，不翻译。
- 四个新页面的分区标题、说明、事件回显文案同样补齐中英文词条。

## 六、错误检查与修复清单

1. 清理 `controls.xml` 中已废弃的 `name` 与 C++ 绑定，避免悬挂逻辑。
2. `test.xml` 的 `use_system_caption="true"` → 统一自定义标题栏。
3. 统一窗口尺寸与 `min_size`，避免卡片内容被裁切。
4. 检查新增 XML 的 Class/属性是否在 `global.xml`/`Control.cpp` 中合法，防止 Debug ASSERT。
5. 检查所有 `name` 唯一、`text_id` 在语言文件中存在。
6. `avatar.xml`/`carousel.xml`/`tree_grid.xml` 补齐 `VScrollBox` 与卡片容器。
7. 更新 `controls.vcxproj`、`controls.vcxproj.filters`，登记新增源文件/头文件。

## 七、涉及文件

**修改**
- `examples/controls/ControlForm.h/.cpp`
- `examples/controls/TestForm.h/.cpp`
- `examples/controls/controls.vcxproj`、`controls.vcxproj.filters`
- `bin/resources/themes/default/controls/controls.xml`（重写）
- `bin/resources/themes/default/controls/test.xml`（重写）
- 各二级页标题栏/卡片统一：`switch.xml`、`button_checkbox.xml`、`avatar.xml`、`carousel.xml`、`tree_grid.xml`、`progress.xml`、`tabctrl.xml` 等
- `bin/resources/lang/zh_CN-examples.txt`、`en_US-examples.txt`

**新增**
- `examples/controls/ComboForm.h/.cpp`
- `examples/controls/ListViewForm.h/.cpp`
- `examples/controls/RichEditForm.h/.cpp`
- `examples/controls/DateTimeForm.h/.cpp`
- `bin/resources/themes/default/controls/combo.xml`
- `bin/resources/themes/default/controls/list_view.xml`
- `bin/resources/themes/default/controls/rich_edit.xml`
- `bin/resources/themes/default/controls/datetime.xml`

## 八、验证方式

1. 重新配置 CMake 后编译 Debug：
   - `& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -SkipAutomaticLocation`
   - `cmake --build 'e:\develop\nim_duilib\out\x64\Debug' --target controls --config Debug`
   - 若链接失败，先结束同名 `controls.exe` / `controls_d.exe` 进程。
2. 运行 `bin/controls_d.exe`，检查：
   - 启动无 ASSERT / 无空白卡片；
   - 主界面滚动流畅，缩到最小尺寸不裁切；
   - 逐个点击卡片，二级页正常打开且关闭正常；
   - 切换浅色/深色主题，卡片、Hero、文字对比度正常；
   - 切换中英文，新增文案完整；
   - 新页面（Combo/List/RichEdit/DateTime）交互与 Toast 回显正常；
   - 进度条动画、托盘图标、遮罩、Flyout、Calendar 均可正常操作。
3. 用浏览器/截图方式抽查主界面与 2~3 个二级页面的视觉效果。
4. 最后给出中文 conventional commit 日志（不执行 git commit）。
