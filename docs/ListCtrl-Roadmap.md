简体中文 | [English](ListCtrl-Roadmap.en.md)

## ListCtrl 优化路线图

本文档专门用于跟踪 `ListCtrl` 控件的优化与增强项，便于逐项跟进与验收。控件级整体规划见 [Control-Roadmap.md](Control-Roadmap.md)，ListCtrl 的完整属性 / 接口 / 示例见 [Control.md](Control.md#listctrl-控件继承-vbox-属性)。

覆盖范围：`ui::ListCtrl` 及其配套类 `ui::ListCtrlReportView`、`ui::ListCtrlHeader`、`ui::ListCtrlHeaderItem`、`ui::ListCtrlItem`、`ui::ListCtrlSubItem`、`ui::ListCtrlData`（含 Report / List / Icon 三种视图）。

状态说明：**已完成** ｜ **待办**（已确认缺失，等待实现）｜ **待评估**（候选优化，需先确认成本收益）｜ **待核实**（当前结论存疑，需先验证再定）。

### 一、已完成优化项

#### 1. 表格增强（对应 Control-Roadmap 第 17 项）

| 能力 | 接口 / XML | 说明 |
|------|-----------|------|
| 列冻结 | `SetFrozenColumnCount` / `GetFrozenColumnCount`，XML `frozen_column_count` | 固定左侧前 N 列，横向滚动时保持不动；Report 视图采用双趟绘制，表头、网格线、命中测试均已适配 |
| 单元格进度条 | `SetSubItemProgress(ById)` / `GetSubItemProgress(ById)` | 进度值 `[0,100]`，`-1` 表示关闭；`progress_color` / `progress_bkcolor` 控制前景与轨道配色，`progress_text_pos` 控制百分比文字位置（`center` / `left` / `right` / `outside`） |
| 单元格富文本 | `SetSubItemRichText(ById)` / `IsSubItemRichText(ById)` | 复用 Label 内建富文本渲染，支持 `<b>` / `<font>` / `<a>` 等标签 |

#### 2. 交互增强（对应 Control-Roadmap 第 18 项）

| 能力 | 接口 | 说明 |
|------|------|------|
| 单格刷新 | `RefreshSubItem` / `RefreshSubItemById` | 仅刷新指定单元格，避免整行刷新；仅 Report 视图有效，单元格不在可见范围时返回 `false`（数据已更新，滚动到可见后显示最新数据）；刷新时派发 `kEventReportViewSubItemFilled` 回调（`pEventData` 为该单元格 `ListCtrlSubItem*`），应用可在此动态设置单元格显示内容 |
| 列显示 / 隐藏 | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl 一级的列显隐接口，内部自动同步数据列 |
| 列布局事件 | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | 分别派发 `kEventColumnWidthChanged` / `kEventColumnOrderChanged` / `kEventColumnVisibleChanged` / `kEventColumnSorted`，便于应用持久化列宽、列序、列显隐与排序状态 |

配套演示：`examples/ListCtrl`（默认冻结首列、第 2 列为进度条、第 3 列为富文本；含单格刷新动态进度演示，界面可操作的列显隐测试项，以及"单元格换行"单行/多行切换测试项）。

#### 3. 单元格自动换行（Report 视图）

| 能力 | 接口 | 说明 |
|------|------|------|
| 单元格单行 / 多行（自动换行） | `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` | 控制 Report 单元格文本单行（`true`，默认，超出按省略号裁剪）或多行（`false`，按单元格宽度自动换行）；行高需由调用方通过 `SetDataItemHeight` 调够，否则超出部分会被裁剪 |

实现要点：Report 视图为虚表，单元格控件在滚动时会被复用，因此单行/多行属性存储于数据模型（`nTextFormat` 的 `TEXT_SINGLELINE` 位），由框架在**填充单元格数据时**应用到复用的控件上（复用既有的 per-cell 文本格式通路，未新增结构体字段）。List / Icon 视图的换行由其自身的 `SetTextSingleLine` 控制，本接口仅 Report 视图有效（非 Report 视图 `Set*` 返回 `false`，`Is*` 返回 `true`）。

#### 4. 列级比较模式（排序）

| 能力 | 接口 / 标志位 | 说明 |
|------|-------------|------|
| 列级比较模式 | `SetColumnSortFlag(ById)` / `GetColumnSortFlag(ById)`，标志位 `ListCtrlSubItemSortFlag` | 按列声明排序的比较方式，鼠标点击该列表头排序时自动采用；由"列的语义"决定比较方式，替代早期"按单元格 `nProgress` 推断"的行为 |

标志位（`enum ListCtrlSubItemSortFlag`，按位组合）：`kSortByGroup`（先按 `nSortGroup` 分组）、`kSortNoCase`（字符串忽略大小写）、`kSortByText`（默认，按文本字典序）、`kSortByTextAsNumber`（文本按数值比较，`"10" > "9"`）、`kSortByUserDataN` / `kSortByUserDataS`（按用户数据）、`kSortByProgress`（按进度值 `nProgress`）。

实现要点：列级 flag 存于 `ListCtrl::m_columnSortFlagMap`（列ID → `uint8_t`）；表头点击排序（`OnColumnSorted`）与主动排序（`SortDataItemsById`）都会读取/写入该列 flag，应用声明一次即长期生效。原 `SortDataCompareFunc` 中"任一单元格 `nProgress >= 0` 即按进度排序"的推断分支已移除，改为由 `kSortByProgress` 显式声明。

配套演示：`examples/ListCtrl` 中第 2 列（进度条列）声明 `kSortByProgress`，第 4 列为纯数字文本并声明 `kSortByTextAsNumber`，点击对应列表头即可看到按进度 / 按数值排序。

### 二、已具备能力（非本次新增，曾一度被误判为缺失）

| 能力 | 实现位置 | 说明 |
|------|---------|------|
| 键盘导航 | `VirtualListBox::OnListCtrlKeyDown`（多选）/ `ListBox::OnListBoxKeyDown`（单选） | 方向键、`Home` / `End`、`PageUp` / `PageDown` 移动选择；`Shift` + 方向键范围选择；`Ctrl+A` 全选。当无选中项时，方向键退化为滚动（预期设计，非缺陷） |
| 单元格 tooltip（超长自动提示） | 主题 `global.xml` 的 `list_ctrl_sub_item` Class 带 `auto_tooltip="true"`；逻辑在 `LabelImpl::CheckShowToolTip` / `LabelImpl::GetAutoToolTipText` | 文本按单元格可用宽度放不下时自动弹出完整内容。注意 XML 属性名是 `auto_tooltip`（不是 `auto_show_tooltip`） |
| 可变行高 | `ListCtrl::SetDataItemHeight`（默认行高）/ `SetDataItemHeight(itemIndex, ...)`（逐行行高） | 支持统一行高与逐行不同行高，现有测试程序已含演示 |
| 文本省略号 | Label 默认文本样式含 `TEXT_END_ELLIPSIS` | 文本超出单元格可用宽度时自动以省略号结尾，现有测试程序已含演示 |

### 三、待办清单（已确认缺失）

| # | 项 | 现状 | 实现要点 |
|---|----|------|---------|
| L1 | F2 进入单元格编辑 | 缺失：`ListCtrl` 现仅处理 `kVK_ESCAPE`（退出编辑），无 `kVK_F2` | 在 `ListCtrl` 的按键处理分支中响应 `kVK_F2`，进入当前单元格编辑态；需与既有 `m_pRichEdit` 编辑态互斥 |
| L2 | Ctrl+C 复制选中内容到剪贴板 | 缺失：框架已提供 `Clipboard::SetClipboardText`，但 `ListCtrl` 未调用 | 复制当前选中单元格的文本；可扩展为整行 / 多行按 TSV 格式复制（便于粘贴到 Excel）；需与编辑态互斥 |

### 四、候选优化项（待评估 / 待核实）

| # | 项 | 状态 | 说明 |
|---|----|------|------|
| P1 | 单元格存储的堆分配开销 | 待评估 | 数据模型 `ListCtrlData` 中单元格存储为 `std::shared_ptr`（`StoragePtr` / `StoragePtrList` / `StorageMap`），每个单元格独立堆分配，大数据量下内存占用与分配开销偏大；可评估紧凑存储或对象池化方案 |
| P2 | 列存储与行优先访问的遍历开销 | 待评估 | 数据按列组织（列 ID → 行列表），而绘制 / 填充按行优先访问，需要跨列收集数据，存在额外遍历；可评估行优先缓存 |
| P3 | 冻结列下的横向滚动最大范围 | 待核实 | 冻结列固定在左侧，横向滚动条的最大范围是否已扣除冻结列宽度（避免滚动到末端时最后一列被冻结列遮挡），需先验证再决定是否调整 |

### 五、跟进状态总览

| 编号 | 项 | 状态 |
|------|----|------|
| — | 列冻结 / 单元格进度条 / 单元格富文本 | 已完成 |
| — | 单格刷新 / 列显隐 / 列布局事件 | 已完成 |
| — | 列级比较模式（排序） | 已完成 |
| — | 键盘导航（方向键 / Home / End / PageUp / PageDown） | 已具备 |
| — | 单元格 tooltip（超长自动提示） | 已具备 |
| — | 可变行高 / 文本省略号 | 已具备 |
| L1 | F2 进入单元格编辑 | 待办 |
| L2 | Ctrl+C 复制到剪贴板 | 待办 |
| L3 | 单元格自动换行（Report 视图） | 已完成 |
| P1 | 单元格存储堆分配开销 | 待评估 |
| P2 | 列存储与行优先访问开销 | 待评估 |
| P3 | 冻结列横向滚动范围 | 待核实 |

> 注：本文档为 ListCtrl 专项跟进清单，条目完成后请同步更新本表状态；控件级的新增规划仍以 [Control-Roadmap.md](Control-Roadmap.md) 为准。
