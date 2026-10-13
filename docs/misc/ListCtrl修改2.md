以下是对当前 ListCtrl 代码的盘点与改进建议（均基于代码核查，未能确认的会明确标注）。

## 一、现状能力（已具备）
- 列 ：固定/自动/拉伸列宽、列宽拖拽、列头拖拽换序、列宽双击自适应、列显隐（在`ListCtrlHeaderItem` 上）、每列排序标志（ ListCtrl.h ）
- 数据 ：按列存储，行级增删/插删、行属性、行可见/置顶、行首图标、行高统一设置
- 单元格 ：文本/颜色/背景/文本格式/图标/CheckBox/可编辑/进度条/富文本（ ListCtrl.h ）
- 交互 ：单选/多选/键盘无关的框选、双击、右键、子项进入/离开编辑
- 视图 ：Report / List / Icon 三视图；Report 为虚表（只创建可见行）
- 刷新 ：`Refresh()` 全量 +`RefreshDataItems(vector)` 按行
- 事件 ：约 20 个`kEvent*` （Select、SelChanged、Click、DoubleClick、RClick、Item/SubItemMouseEnter/Leave、KeyDown/Up、Enter/LeaveEdit、DataItemCountChanged、ReportView/ListView/IconView ItemFilled、ViewTypeChanged、ViewPos/SizeChanged 等）
## 二、明显的不足与可优化点（附证据）
### 1. 单元格存储开销大（最值得优化）
ListCtrlData.h ：

C++

1
2

`typedef std::shared_ptr<Storage> StoragePtr;                 // Storage = ListCtrlSubItemData2(96B)
typedef std::unordered_map<size_t, StoragePtrList> StorageMap;`

每个单元格一次`make_shared` 堆分配 ，额外承担 16B 智能指针 + 控制块/分配器开销（x64 上单格实际 ~130B+）。100 万单元格 ≈ 100 万次小分配，既是内存也是碎片与局部性问题。改为 按列连续存储 （`vector<ListCtrlSubItemData2>` ，或对进度/富文本等稀疏字段做 SoA 拆分）可省 30%+ 内存并消除海量小分配。

### 2. 列优先存储 vs 行优先访问
数据按列放在`m_dataMap` ，而绘制/填充是行优先（`GetSubItemStorageList(itemIndex)` 每次要跨 N 个列 vector 收集并构造临时容器）。可考虑按行缓存或改为行优先 + 列索引。

### 3. 缺"单格刷新"接口
只有整行级的`RefreshDataItems` 。进度条这种高频更新的单元格会带着整行重建。建议加`RefreshSubItem(rowIndex, columnIndex)` ，只重绘一个单元格。

### 4. 绘制期做重排
ListCtrlReportView.cpp 的`PaintChild` 每次绘制都调用`ReArrangeChild(false)` ；加了冻结列后每个控件还要画两趟。建议在尺寸/数据未变时跳过重排。

### 5. 冻结列的实现方式偏"外挂"（需验证两处）
- 命中测试靠"落在冻结区就把横向偏移置 0"的补偿，绘制靠两趟裁剪，与常规布局坐标混用，后续改动容易再踩坑（这次`frozen=0` 的绘制异常就是同源问题）。
- 需确认 ：横向滚动条的范围/最大值是否已扣除冻结列宽度。若未扣除，冻结后最右侧列可能滚不到底或出现空白带。
### 6. 排序语义靠"单元格推断"
我把"进度列按数值排序"做成了 按单元格`nProgress` 推断 （ ListCtrlData.cpp ）。能用，但更干净的做法是 列级比较模式 （文本 / 数值 / 进度 / 自定义），由`nSortFlag` 或列配置声明。另外目前只支持 单列排序 ，且是整表排序 + 全量刷新。

### 7. 脆弱写法（建议全文审查同类代码）
刚修的`EstimateText` 是典型：绝对坐标与宽度混用，只在"控件尚未布局"时才碰巧正确（ ListCtrlSubItem.cpp ）。这类"依赖调用时序 / 依赖 GetRect() 有效"的写法建议统一排查。

### 8. 缺少列布局的对外事件
`OnColumnWidthChanged` /`OnColumnSorted` /`OnHeaderColumnOrderChanged` /`OnHeaderColumnVisibleChanged` 都是 ListCtrl 的 私有槽 ，没有对应的`kEvent*` 对外通知 → 应用 无法保存/恢复用户的列宽、列序、列显隐 。

### 9. 列显隐缺一级 API
只有`ListCtrlHeaderItem::SetColumnVisible` ，`ListCtrl` 上没有对应公开接口，应用必须绕过封装去操作表头子项。

### 10. 键盘支持几乎为零
整个 ListCtrl 只处理了`kVK_ESCAPE` （退出编辑）。方向键、Home/End、PageUp/PageDown、F2 编辑、Ctrl+C 等都没有。

## 三、相对 QTableView / WPF DataGrid 缺失的功能
功能 状态 多级表头 / 表头单元格合并 缺失（表头是平铺单行） 行拖拽换序 缺失（列头拖拽有） 单元格合并 缺失 表格内嵌树（父行展开子行） 缺失（TreeView 是独立控件） 行分组折叠显示 缺失（只有排序分组`nSortGroup` ） 行筛选 / 表头筛选器 缺失 汇总行 / 页脚行 缺失 复制到剪贴板 / 导出 缺失 键盘导航 缺失（仅 ESC） 单元格 tooltip 无专门 API（只能靠通用`tooltip_text` ） 自定义单元格渲染器（嵌入任意控件） 缺失（仅内置文本/图标/CheckBox/进度条/富文本） 可变行高 / 单元格自动换行 未确认（行高统一，未见 wrap 入口） 列宽/列序/列显隐持久化 缺事件，无法可靠实现 单元格级编辑器（下拉/日期/数值） 缺失（只有一种 RichEdit 编辑器）

## 四、优先级建议
低成本高收益

1. 单格刷新接口`RefreshSubItem` （进度/状态类列立竿见影）
2. 列宽/列序/列显隐/排序的对外`kEvent*` 事件（应用可持久化）
3. `ListCtrl` 级`SetColumnVisible/IsColumnVisible`
4. 键盘导航（方向键 + Home/End/PageUp/PageDown + F2/Ctrl+C）
5. 单元格 tooltip（内容超长时自动提示）
中等成本 6. 列级比较模式（数值/日期/进度/自定义），替代按单元格推断
7. 可变行高 + 单元格自动换行 + 省略号
8. 行筛选（按条件过滤 + 表头筛选器）
9. 复制到剪贴板 / 导出 CSV

架构性改动 10. 单元格存储改为连续内存（去掉每格`shared_ptr` ），顺带改善行访问局部性
11. 冻结列从"两趟绘制 + 命中补偿"收敛为 统一的坐标变换层 （绘制/命中/网格线/滚动条范围都由它推导），避免同类 bug 复发
12. 多级表头 / 行分组折叠 / 表格内嵌树 / 单元格合并（需要表头与行的布局模型升级）

----
## 改动内容(1)

1. 单格刷新`RefreshSubItem`

- ListCtrl.h ：新增`RefreshSubItem(dataItemIndex, columnIndex)` /`RefreshSubItemById(dataItemIndex, columnId)` 。
- ListCtrlReportView.cpp ：把`FillDataItem` 里逐格填充数据的逻辑抽取为共用的`FillSubItemData()` ，新增`RefreshSubItemData(nElementIndex, nColumnId)` ——按`GetDisplayItemIndex` 定位当前显示行，按列ID找到对应`ListCtrlSubItem` ，只重填这一格后`Invalidate()` ，不重建控件、不改宽度/事件绑定。
- 仅 Report 视图有效；单元格不在可见范围时返回`false` （数据已更新，滚入后自然显示）。

2. 列布局对外事件

- duilib_defs.h / EventArgs.cpp ：新增`kEventColumnWidthChanged` /`kEventColumnOrderChanged` /`kEventColumnVisibleChanged` /`kEventColumnSorted` 。
- ListCtrl.h ：新增`AttachColumnWidthChanged/AttachColumnOrderChanged/AttachColumnVisibleChanged/AttachColumnSorted` 。
- ListCtrl.cpp ：在`OnColumnWidthChanged` 、`SortDataItemsById` 、`OnColumnSorted` 、`OnHeaderColumnOrderChanged` 、`OnHeaderColumnVisibleChanged` 中触发。参数：列宽 = 列ID + 关联列ID；列显隐 = 列ID + 新状态；排序 = 列ID + 升降序。

3. ListCtrl 级列显隐

- ListCtrl.cpp ：`SetColumnVisible(ById)` /`IsColumnVisible(ById)` ，转发到表头项（ ListCtrlHeaderItem.cpp 原有显隐逻辑），自动同步数据列与刷新。
- 为此把`OnHeaderColumnVisibleChanged()` 改为携带列ID（`ListCtrlHeader.h/.cpp` 、`ListCtrlHeaderItem.cpp` 同步），使显隐事件能带上具体列。
    
  其他 ：`ListCtrlData` 增加`friend class ListCtrlReportView;` （单格刷新需读取单元格存储）；文档已同步 Control.md / Control.en.md 、路线图 #18 与三处技能参考副本。代码未提交，留给你手动提交。
