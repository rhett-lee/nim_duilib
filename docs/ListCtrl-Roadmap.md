简体中文 | [English](ListCtrl-Roadmap.en.md)

## ListCtrl 优化路线图

本文档专门用于跟踪 `ListCtrl` 控件的优化与增强项，便于逐项跟进与验收。控件级整体规划见 [Control-Roadmap.md](Control-Roadmap.md)，ListCtrl 的完整属性 / 接口 / 示例见 [Control.md](Control.md#listctrl-控件继承-vbox-属性)。

覆盖范围：`ui::ListCtrl` 及其配套类 `ui::ListCtrlReportView`、`ui::ListCtrlHeader`、`ui::ListCtrlHeaderItem`、`ui::ListCtrlItem`、`ui::ListCtrlSubItem`、`ui::ListCtrlData`（含 Report / List / Icon 三种视图）。

状态说明：**已完成** ｜ **已具备**（框架既有能力，非本次新增，曾一度被误判为缺失）｜ **待办**（已确认缺失，等待实现）｜ **待评估**（候选优化，需先确认成本收益）｜ **待核实**（当前结论存疑，需先验证再定）｜ **缺失**（相对 QTableView / WPF DataGrid 尚无对应实现）。

> 说明：本表在原有条目基础上，补充了外部评审提出的不足点、相对 QTableView / WPF DataGrid 的缺失功能，以及分档优先级建议，作为逐项验收清单。

### 一、已完成 / 已具备能力

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
| 列显示 / 隐藏 | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl 一级的列显隐接口，内部自动同步数据列，应用无需再绕过封装操作表头子项 |
| 列布局事件 | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | 分别派发 `kEventColumnWidthChanged` / `kEventColumnOrderChanged` / `kEventColumnVisibleChanged` / `kEventColumnSorted`，应用可据此持久化列宽、列序、列显隐与排序状态 |

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

待办（二期）：日期模式 `kSortByTextAsDate` 尚未实现（可复用 `duilib/Control/DateTime.h` 解析）。

> 注：目前仅支持单列排序，且为整表排序 + 全量刷新（增量/局部排序待评估）。

<!-- verify:allow-missing ListCtrlSubItemSortFlag -->

#### 5. 键盘导航（已具备）

| 能力 | 实现位置 | 说明 |
|------|---------|------|
| 方向键 / Home / End / PageUp / PageDown | `VirtualListBox::OnListCtrlKeyDown`（多选）/ `ListBox::OnListBoxKeyDown`（单选） | 方向键、`Home` / `End`、`PageUp` / `PageDown` 移动选择；`Shift` + 方向键范围选择；`Ctrl+A` 全选。当无选中项时，方向键退化为滚动（预期设计，非缺陷） |

#### 6. 单元格 tooltip（已具备）

| 能力 | 实现位置 | 说明 |
|------|---------|------|
| 单元格 tooltip（超长自动提示） | 主题 `global.xml` 的 `list_ctrl_sub_item` Class 带 `auto_tooltip="true"`；逻辑在 `LabelImpl::CheckShowToolTip` / `LabelImpl::GetAutoToolTipText` | 文本按单元格可用宽度放不下时自动弹出完整内容。注意 XML 属性名是 `auto_tooltip`（不是 `auto_show_tooltip`） |

#### 7. 可变行高 / 文本省略号（已具备）

| 能力 | 实现位置 | 说明 |
|------|---------|------|
| 可变行高 | `ListCtrl::SetDataItemHeight`（默认行高）/ `SetDataItemHeight(itemIndex, ...)`（逐行行高） | 支持统一行高与逐行不同行高，现有测试程序已含演示 |
| 文本省略号 | Label 默认文本样式含 `TEXT_END_ELLIPSIS` | 文本超出单元格可用宽度时自动以省略号结尾，现有测试程序已含演示 |

### 二、待办清单（已确认缺失）

| # | 项 | 现状 | 实现要点 |
|---|----|------|---------|
| L1 | F2 进入单元格编辑 | 缺失：`ListCtrl` 现仅处理 `kVK_ESCAPE`（退出编辑），无 `kVK_F2` | 在 `ListCtrl` 的按键处理分支中响应 `kVK_F2`，进入当前单元格编辑态；需与既有 `m_pRichEdit` 编辑态互斥 |
| L2 | Ctrl+C 复制选中内容到剪贴板 | 缺失：框架已提供 `Clipboard::SetClipboardText`，但 `ListCtrl` 未调用 | 复制当前选中单元格的文本；可扩展为整行 / 多行按 TSV 格式复制（便于粘贴到 Excel）；需与编辑态互斥。可进一步扩展"导出 CSV" |
| L3 | 行筛选 / 表头筛选器 | 缺失（现仅有排序分组 `nSortGroup`，无按条件过滤） | 提供按条件过滤行的接口 + 表头筛选器下拉 UI |
| L4 | 自定义单元格渲染器 | 缺失（仅内置文本 / 图标 / CheckBox / 进度条 / 富文本几种渲染） | 允许在单元格内嵌入任意子控件（自定义绘制） |
| L5 | 单元格级编辑器（下拉 / 日期 / 数值） | 缺失（仅一种 RichEdit 编辑器） | 按列声明编辑器类型，编辑时创建对应输入控件 |

### 三、候选优化项（待评估 / 待核实 / 架构性）

| # | 项 | 状态 | 说明 |
|---|----|------|------|
| P1 | 单元格存储的堆分配开销 | 实施中（Phase 1 已完成，性能验收通过） | 数据模型 `ListCtrlData` 中单元格存储为 `std::shared_ptr`（`StoragePtr` / `StoragePtrList` / `StorageMap`），每个单元格独立堆分配，x64 下单格实际约 130B+；大数据量下内存占用、分配开销与内存碎片偏大。可评估按列连续存储（`std::vector<ListCtrlSubItemData2>`）或对进度 / 富文本等稀疏字段做 SoA 拆分，预计可省 30%+ 内存。已启动"数据存储重构"（方案见 [ListCtrl存储重构设计方案.md](misc/ListCtrl存储重构设计方案.md)）：**Phase 1「身份/位置分离」已完成**（引入 `RowId` + `m_displayOrder`，列池 / 行属性池按 RowId 索引），解决中间增删改 O(N×C) 搬移的性能问题，并去掉排序时的整列深拷贝；**Phase 2「连续分页存储」待实施**，落地后达成上表的内存下降目标；**Phase 0「压力测试面板」已落地**（`examples/ListCtrl` 新增 10 万行基准面板，见设计方案 §10），用于采集性能基线并校验各阶段验收门槛；**Phase 1 性能验收已通过**（10万×10，x64 Release 实测：删 500 行 615.9→56.4 ms、插 1000 行 605.7→68.1 ms、排序 76.7→31.5 ms、滚动重绘 15.529→13.020 ms/帧，绘制未劣化；载入路径轻微回退 ~24%，留待 Phase 2 评估） |
| P2 | 列优先存储 vs 行优先访问的遍历开销 | 待评估 | 数据按列组织（列 ID → 行列表），而绘制 / 填充按行优先访问，`GetSubItemStorageList(itemIndex)` 需跨 N 个列收集并构造临时容器，存在额外遍历；可评估行优先缓存或改为行优先 + 列索引 |
| P3 | 冻结列下的横向滚动最大范围 | 待核实 | 冻结列固定在左侧，横向滚动条的最大范围是否已扣除冻结列宽度（避免滚动到末端时最后一列被冻结列遮挡 / 出现空白带）。当前 `ListCtrlReportView` 的横向 extent 计算（`EstimateLayoutSize` / `CalcEstimateSize`）未发现显式的冻结宽度扣除，需先实测确认再决定是否调整 |
| P4 | 绘制期每帧重排 | 待评估 | `ListCtrlReportView::PaintChild` 每次绘制都调用 `ReArrangeChild(false)`，且在冻结列场景下每个控件还要画两趟；可在尺寸 / 数据未变时跳过重排 |
| P5 | 依赖调用时序 / `GetRect()` 有效的脆弱写法排查 | 待办 | `ListCtrlSubItem::EstimateText` 曾将绝对坐标与宽度混用，仅在"控件尚未布局"时碰巧正确（已修复）；建议全文审查同类"依赖调用时序 / 依赖 `GetRect()` 有效"的写法 |
| A1 | 冻结列实现收敛为统一坐标变换层 | 待评估（架构性） | 当前命中测试靠"落在冻结区就把横向偏移置 0"的补偿、绘制靠两趟裁剪，与常规布局坐标混用，后续改动容易再踩坑（`frozen=0` 绘制异常即同源问题）；建议将绘制 / 命中 / 网格线 / 滚动条范围统一由一层坐标变换推导 |
| A2 | 布局模型升级（多级表头 / 单元格合并 / 内嵌树 / 行分组折叠） | 缺失（架构性） | 需要表头与行的布局模型升级，详见下节功能对照 |

### 四、相对 QTableView / WPF DataGrid 功能对照

| 功能 | 状态 | 说明 |
|------|------|------|
| 多级表头 / 表头单元格合并 | 缺失 | 当前表头为平铺单行 |
| 行拖拽换序 | 缺失 | 列头拖拽换序已有，行拖拽换序无 |
| 单元格合并 | 缺失 | 支持跨行 / 跨列合并 |
| 表格内嵌树（父行展开子行） | 缺失 | `TreeView` 为独立控件，未与表格融合 |
| 行分组折叠显示 | 缺失 | 现仅有排序分组 `nSortGroup`（分组后仍是平铺行，无折叠） |
| 行筛选 / 表头筛选器 | 缺失 | 见 L3 |
| 汇总行 / 页脚行 | 缺失 | 无固定汇总 / 页脚行 |
| 复制到剪贴板 / 导出 | 缺失 | 见 L2 |
| 键盘导航 | 已具备 | 方向键 / Home / End / PageUp / PageDown / Shift 范围选择 / Ctrl+A |
| 单元格 tooltip | 已具备 | `auto_tooltip="true"` 默认开启，超长自动提示 |
| 自定义单元格渲染器（嵌入任意控件） | 缺失 | 见 L4，现仅内置文本 / 图标 / CheckBox / 进度条 / 富文本 |
| 可变行高 / 单元格自动换行 | 已完成 | `SetDataItemHeight` + `SetSubItemSingleLine`，文本省略号由 Label 默认样式提供 |
| 列宽 / 列序 / 列显隐持久化 | 已完成 | 通过 `kEventColumn*` 系列事件通知应用，应用可持久化与恢复 |
| 单元格级编辑器（下拉 / 日期 / 数值） | 缺失 | 见 L5，现仅一种 RichEdit 编辑器 |

### 五、优先级建议

低 / 中等成本项（多数已完成）：

| # | 项 | 状态 |
|---|----|------|
| 1 | 单格刷新接口 `RefreshSubItem` | 已完成 |
| 2 | 列宽 / 列序 / 列显隐 / 排序的对外 `kEvent*` 事件 | 已完成 |
| 3 | `ListCtrl` 级 `SetColumnVisible` / `IsColumnVisible` | 已完成 |
| 4 | 键盘导航（方向键 + Home/End/PageUp/PageDown + F2/Ctrl+C） | 导航已完成；F2（L1）、Ctrl+C（L2）待办 |
| 5 | 单元格 tooltip（内容超长时自动提示） | 已具备 |
| 6 | 列级比较模式（数值 / 日期 / 进度 / 自定义） | 数值 / 进度已完成；日期（`kSortByTextAsDate`）二期 |
| 7 | 可变行高 + 单元格自动换行 + 省略号 | 已完成 |
| 8 | 行筛选（按条件过滤 + 表头筛选器） | 待办（L3） |
| 9 | 复制到剪贴板 / 导出 CSV | 待办（L2） |

架构性改动项：

| # | 项 | 状态 |
|---|----|------|
| 10 | 单元格存储改为连续内存（去掉每格 `shared_ptr`），顺带改善行访问局部性 | 实施中（身份/位置分离 Phase 1 已完成；连续存储 Phase 2 待实施） |
| 11 | 冻结列从"两趟绘制 + 命中补偿"收敛为统一坐标变换层，避免同类 bug 复发 | 待评估（A1；滚动范围口径见 P3） |
| 12 | 多级表头 / 行分组折叠 / 表格内嵌树 / 单元格合并（需表头与行的布局模型升级） | 缺失（A2，架构性） |

### 六、跟进状态总览

| 编号 | 项 | 状态 |
|------|----|------|
| — | 列冻结 / 单元格进度条 / 单元格富文本 | 已完成 |
| — | 单格刷新 / 列显隐 / 列布局事件 | 已完成 |
| — | 单元格自动换行（Report 视图） | 已完成 |
| — | 列级比较模式（排序，数值 / 进度） | 已完成（日期二期） |
| — | 键盘导航（方向键 / Home / End / PageUp / PageDown、Shift、Ctrl+A） | 已具备 |
| — | 单元格 tooltip（超长自动提示） | 已具备 |
| — | 可变行高 / 文本省略号 | 已具备 |
| — | 列宽 / 列序 / 列显隐持久化（靠事件） | 已完成 |
| — | 固定 / 自动 / 拉伸列宽、列宽拖拽、列头拖拽换序、双击列宽自适应 | 已具备 |
| — | Report / List / Icon 三视图（Report 为虚表） | 已具备 |
| — | 全量刷新 `Refresh()` + 按行刷新 `RefreshDataItems(vector)` | 已具备 |
| L1 | F2 进入单元格编辑 | 待办 |
| L2 | Ctrl+C 复制到剪贴板（多行 TSV / 导出 CSV） | 待办 |
| L3 | 行筛选 / 表头筛选器 | 待办 |
| L4 | 自定义单元格渲染器 | 待办 |
| L5 | 单元格级编辑器（下拉 / 日期 / 数值） | 待办 |
| P1 | 单元格存储堆分配开销（连续内存 / SoA） | 实施中（Phase 1 已完成，性能验收通过） |
| P2 | 列存储与行优先访问开销 | 待评估 |
| P3 | 冻结列横向滚动范围（是否扣除冻结宽度） | 待核实 |
| P4 | 绘制期每帧重排（`ReArrangeChild`） | 待评估 |
| P5 | 脆弱写法排查（依赖时序 / `GetRect()`） | 待办 |
| A1 | 冻结列收敛为统一坐标变换层 | 待评估（架构性） |
| A2 | 多级表头 / 单元格合并 / 内嵌树 / 行分组折叠 | 缺失（架构性） |

> 注：本文档为 ListCtrl 专项跟进清单，条目完成后请同步更新本表状态；控件级的新增规划仍以 [Control-Roadmap.md](Control-Roadmap.md) 为准。
