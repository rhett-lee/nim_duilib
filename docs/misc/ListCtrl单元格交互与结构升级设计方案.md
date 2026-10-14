# ListCtrl 单元格交互与结构升级设计方案

> 本文档给出 `ui::ListCtrl` 一系列"结构相关"规划项的**统一设计方案**：单元格选择 / 单元格编辑 / 单元格复制 / 单元格合并 / 多级表头 / 内嵌树 / 行分组折叠 / 行拖拽换序。
> 跟踪清单见 [ListCtrl-Roadmap.md](../ListCtrl-Roadmap.md)（§二 L1~L5、§三 A2、§四功能对照、§五第 12 项）；存储层方案见 [ListCtrl存储重构设计方案.md](ListCtrl存储重构设计方案.md)。
> 本文档只做**设计**，不含实现；文中所有"现有代码锚点"均已在源码中核实。
>
> 下文出现的以下事件名均为**本方案新提议**、源码中尚不存在（落地时才按"三件套"约定新增）：
> <!-- verify:allow-missing kEventCurrentCellChanged kEventCellSelectionChanged kEventCellsCopied kEventCellEditBegin kEventCellEditCommit kEventCellEditCancel kEventCellsMerged kEventCellsUnmerged kEventItemExpandedChanged kEventGroupCollapsedChanged kEventRowMoved -->

---

## 0. 背景：为什么这些项要一起设计

现状的两个基本假设，决定了这批规划项**互相耦合、无法各自独立实现**：

**假设一：选择粒度 = 行**。选择状态寄存在数据层"行"上：
- 数据层接口 [`VirtualListBoxElement`](../../duilib/Box/VirtualListBox.h#L39-L63) 定义 `SetElementSelected` / `IsElementSelected` / `GetSelectedElements` / `IsMultiSelect` / `SetMultiSelect`；
- ListCtrl 的行选中实现在 [`ListCtrlData::SetElementSelected / IsElementSelected / GetSelectedElements`](../../duilib/Control/ListCtrlData.cpp#L101-L160)，单选时用 `m_nSelectedRowId`、多选时用行属性 `bSelected`；
- 鼠标按下统一落到"选中整行"：[`ListCtrlView::SelectItem`](../../duilib/Control/ListCtrlView.cpp#L81-L85) 跳过基类逻辑直接调 `ListCtrlSelectItem`；
- 对外 API 也是行粒度的：[`ListCtrl::SetMultiSelect / SetSelectedDataItems / GetSelectedDataItems`](../../duilib/Control/ListCtrl.cpp#L2246-L2279)。

**假设二：UI 元素模型 = "行容器 → N 个等高单元格子控件"**。Report 视图填充数据时按列遍历、把每个单元格宽度写死到子控件上：
- 列宽来源 = 表头项的列宽，[`ListCtrlReportView.cpp`](../../duilib/Control/ListCtrlReportView.cpp#L1063-L1071) 取 `pHeaderCtrl->GetColumnWidth(nColumnIndex)`；
- [`FillDataItem`](../../duilib/Control/ListCtrlReportView.cpp#L1113-L1187) 创建/复用 `ListCtrlSubItem` 并 `SetFixedWidth(elementData.nColumnWidth)`。

由这两个假设直接引出的结构性障碍：

| 规划项 | 被哪个假设卡住 |
|--------|---------------|
| L1 F2 单元格编辑 | 假设一（无"当前单元格"概念）、假设二（定位靠 `GetTextRect` 这一绘制副产物） |
| L2 Ctrl+C 单元格复制 | 假设一（只有行选择，无区域概念） |
| A2 单元格合并 | 假设二（列宽线性排布 + 一行 N 个等高格，合并不成立） |
| A2 多级表头 | 表头结构：`ListCtrlHeaderItem` 与 `SplitBox` **成对交替**、偶数下标为表头项、列数 = `ItemCount / 2`（[`ListCtrlHeader::GetColumnCount`](../../duilib/Control/ListCtrlHeader.cpp#L186-L221)），是硬编码的"平铺单行"结构 |
| A2 内嵌树 / 行分组折叠 | 假设二（行是扁平的、等高、无缩进层级），且需要"显示顺序 ≠ 数据顺序" |
| 行拖拽换序 | 假设一/二（行的显示位置不可被拖改，且与排序/分组/树要协同） |

**核心结论**：需要引入一层**单元格坐标系（Cell Coordinate Model）**，让"绘制 / 命中 / 网格线 / 滚动 / 编辑框定位 / 拖放落点 / 选择高亮"全部建立在同一套 cell 几何之上。这不是新路线——**A1（冻结列统一坐标变换层）就是它的最小特例**，本方案是把 A1 的 `FrozenTransform` 推广成通用的 `ListCtrlCellGeometry`。

---

## 1. 设计目标与原则

1. **单一坐标来源**：任何"第 N 列的 X / 某一格的矩形"都只能由几何层推导，禁止在绘制、命中、网格线、编辑、拖放各处自行累加列宽（A1 的教训：同一"冻结区右界"被三处用不同口径重算）。
2. **向后兼容、默认关闭**：默认仍是**行选择**（`selection_mode="row"`），所有新能力 opt-in；表头单级、无合并、无树时行为与现状完全一致。
3. **状态入模型、不入控件**：Report 视图是虚表、单元格控件滚动时复用，per-cell 状态（当前格 / 选择 / 编辑 / 展开）必须存数据层——沿用"单行/多行换行属性存进数据模型"的既有教训。
4. **优先复用既有基础设施**：行选择的 `VirtualListBoxElement` 接口、剪贴板 [`Clipboard_Windows`](../../duilib/Utils/Clipboard_Windows.cpp#L8-L79)（`GetClipboardText` / `SetClipboardText`）、拖拽 [`ControlDragableT`](../../duilib/Core/ControlDragable.h#L320-L368)、单元格文本矩形 [`ListCtrlLabel::GetTextRect`](../../duilib/Control/ListCtrlLabel.cpp#L146-L160)。
5. **分阶段、可单独验收与回退**：每阶段独立提交，行为等价性可测。

---

## 2. 目标架构：两层几何

```
                    ┌─────────────────────────────────────────┐
                    │  ListCtrlColumnLayout（列布局表）         │
   表头树 / 列宽 /   │  columnId → { 列序, x 偏移, 宽度, 可见 }   │  列宽/列序/显隐/多级表头变化时重算
   列序 / 显隐  ───▶ │  （由表头叶子节点推导）                    │
                    └──────────────┬──────────────────────────┘
                                   │
   行高 / 置顶 /     ┌──────────────▼──────────────────────────┐
   树展开 / 分组 ──▶ │  ListCtrlCellGeometry（单元格几何层）      │  ← A1 的 FrozenTransform 推广而来
   合并 span /       │  · GetCellRect(CellId) → 合并后矩形        │
   冻结列 / 滚动     │  · GetCellAt(Point)  → CellId（合并取锚点）│
                    │  · 行边界 / 列边界（供网格线、落点线）      │
                    │  · 内容坐标 ↔ 视口坐标（含冻结区处理）      │
                    └──────────────┬──────────────────────────┘
                                   │
     消费方（全部只读几何层，不再自己算）：
     绘制 PaintChild │ 网格线 PaintGridLines │ 命中 FindControl │ tooltip
     编辑器定位 │ 选择高亮 │ 拖放落点线 │ 横向滚动 extent
```

- **CellId** = `{ nRowIndex（显示序）, nColumnId }`。用**列 ID**而非列下标，与既有"列宽 / 列序 / 显隐 / 排序"全部按 `columnId` 组织保持一致（列序可变、ID 稳定）。
- 合并格：`CellId` 指向**锚点格**；被覆盖格通过 `IsCovered(CellId)` / `GetMergeAnchor(CellId)` 归一。
- **缓存与脏标记**：列布局表在"列宽 / 列序 / 列显隐 / 表头层级"变化时重算；行布局在"行高 / 置顶集 / 树展开 / 分组折叠"变化时重算。绘制期不重算（沿用 P4 的 `NeedReArrange()` 守卫思路，避免每帧重排）。

---

## 3. 分阶段方案

### C0　单元格坐标层（基础，**无用户可见行为变化**）

**做什么**
1. 把 `ListCtrlReportView` 内私有的 `FrozenTransform`（A1 产物）上提/推广为 `ListCtrlCellGeometry`，保留冻结列语义（冻结区右界、命中归零、两趟裁剪、网格线回位），并新增"列 X 区间 / 行 Y 区间 / cell 矩形"能力。
2. 新增 `ListCtrlColumnLayout`：由表头叶子推导 `columnId → {列序, x 偏移, 宽度, 可见}`，替代当前"在 `FillDataItem` 里按列宽逐个累加"的隐式排布。
3. 把绘制 `PaintChild`、网格线 `PaintGridLines`、命中 `FindControl`、tooltip 判定，全部改为消费几何层。

**为什么先做**：它是其余所有项的公共地基，且**不改变任何可见行为**——相当于把 A1 的手法从"冻结列"推广到"整张表"，可先单独验证无回归。

**验收**：冻结列开/关、列显隐、列宽调整、列拖拽换序、排序、置顶行等全部现状行为不变（回归性验证）。

---

### C1　单元格选择模型（**解锁 L1 / L2 的关键**）

**状态（存数据层）**
- 在 `ListCtrlData` 增加单元格级状态：`m_currentCell`（当前单元格，作锚点）+ `m_focusCell`（区域焦点）；规范化区域 `{rowBegin,rowEnd,colBegin,colEnd}` 由二者推导。
- 行选择状态保留不动（`bSelected` / `m_nSelectedRowId` 继续服务 `kRow` 模式）。
- 新增枚举 `ListCtrlSelectionMode { kRow（默认）, kCell }`，XML 属性 `selection_mode="row|cell"`。

**对外 API（`ui::ListCtrl` 一级，失败返回 `false` / 无效值，风格与既有接口一致）**
| 接口 | 说明 |
|------|------|
| `SetSelectionMode / GetSelectionMode` | 切换行/单元格选择模式 |
| `SetCurrentCell(rowIndex, columnId)` / `GetCurrentCell` | 当前单元格（可跨列移动） |
| `SetCellSelection(anchorCell, focusCell, bClearExisting)` / `ClearCellSelection` | 设置/清除单元格区域 |
| `GetCellSelection(anchorCell, focusCell)` | 取回区域（规范化） |
| `GetSelectedCells(std::vector<CellId>&)` | 展开为单元格列表（跳过被合并覆盖格） |
| `IsCellSelected(rowIndex, columnId)` / `IsCellCurrent(rowIndex, columnId)` | 查询（供应用自定义绘制/编辑判定） |

> 说明：单元格选择**不塞进** `VirtualListBoxElement`（那是通用列表的元素级接口，不应被表格污染）；它是 ListCtrl 自己的模型。

**事件（按项目"三件套"约定落地）**
- `kEventCurrentCellChanged`（当前单元格变化；wParam=行索引，lParam=列ID）
- `kEventCellSelectionChanged`（区域变化；pEventData 指向规范化区域）
- XML 内联事件名同步写入 `docs/XmlEvents.md`（`on_current_CellChanged` 等，命名随三件套表生成）。

**交互**
- 鼠标：`kCell` 模式下 `ButtonDown` 先 `GetCellAt(point)` 命中单元格 → 设锚点/当前格；`Shift+Click` 扩展到焦点格；`Ctrl+Click` 追加/切换；`MouseMove` 拖动做矩形扩展，到视口边缘触发自动滚动。
- 键盘：方向键移动当前格（横向沿**可见列序**）、`Shift+方向` 扩展区域、`Ctrl+A` 全选单元格、`Home/End/PageUp/PageDown` 与既有行级导航共存（由模式决定语义）。
- 绘制：选中区域半透明填充；当前格画焦点框；**冻结列单元格同样参与选择**（几何层已统一处理冻结区）。

**验收**：`kRow` 模式下所有既有行选择/键盘/事件行为**完全不变**；`kCell` 模式下可选中任意单元格与矩形区域。

---

### C2　单元格复制 / 导出（L2）

- 依赖 C1。`Ctrl+C`：
  - `kRow` 模式：复制选中行（多行 → TSV 逐行）；
  - `kCell` 模式：复制选中区域 → TSV（合并格按其展开区域填同一文本）。
- 落地直接调用 [`Clipboard::SetClipboardText`](../../duilib/Utils/Clipboard_Windows.cpp#L8-L79)（框架已具备，ListCtrl 目前未调用——正是 L2 的缺口）。
- **CSV 导出（已拍板：纳入 L2 范围）**：`Ctrl+Shift+C` 或接口 `ExportCellsToCsv`，可导出当前选中区域 / 整表。
- 可选：`Ctrl+V` 粘贴 TSV 到区域（写入走 `SetSubItemData`，见 [`ListCtrl::SetSubItemData`](../../duilib/Control/ListCtrl.cpp#L1782-L1804)，写入后 `RefreshSubItemData` 单格刷新）。
- 与编辑态互斥（编辑中先提交再复制）。
- 事件（可选）：`kEventCellsCopied` + XML。

**验收**：复制结果粘贴到 Excel 行列对齐正确；含合并格时不串位。

---

### C3　单元格编辑（L1 + L5）

- 依赖 C1。**进入编辑**的触发：`F2`（当前格）/ 双击 / 直接键入（可配置）。
- 现有基础：`ListCtrl` 已有基于 `RichEdit` 的就地编辑器路径，定位读取 `pSubItem->GetTextRect()`（[`ListCtrl.cpp`](../../duilib/Control/ListCtrl.cpp#L2838-L2953)，P5 已记录其"未绘制时为空"的边界）。本阶段把定位改为由 C0 的 `GetCellRect` 推导（**合并格按合并矩形**），并补 P5 建议的兜底（矩形为空则回退 `GetRect()`）。
- **L5 编辑器类型**：按列声明 `SetColumnEditorType(ById, kEditText|kEditNumber|kEditDate|kEditCombo|kEditCheck)` / `GetColumnEditorType(ById)`，编辑时创建对应输入控件；数值/日期带解析与校验（可复用日期排序已实现的"六段解析"思路）。
- 提交/取消语义：`Enter` 提交并下移当前格、`Esc` 取消、失焦提交（可配置）。
- 事件（三件套 + XML）：`kEventCellEditBegin` / `kEventCellEditCommit` / `kEventCellEditCancel`。

**验收**：编辑 → 提交 → 写回数据层 → 单格刷新（复用 `kEventReportViewSubItemFilled` 通路）；数值/日期编辑器正确解析。

---

### C4　单元格合并（A2-a）

**数据模型**：在 `ListCtrlSubItemData2` 增加**稀疏** span 字段（`nRowSpan` / `nColumnSpan`，默认 1），**仅锚点格存储**，被覆盖格只置一个"覆盖"标记位。必须适配 P1 的分页连续存储（Phase 1/2/3 已完成），避免再次膨胀内存（可复用现有稀疏属性位的做法）。

**几何**：`ListCtrlCellGeometry` 计算合并矩形（宽 = 跨越列宽之和，高 = 跨越高行高之和）；被覆盖格不绘制、不参与选择、不画网格线。

**网格线**：只画 cell 的**外边界**，合并内部的分隔线跳过（由几何层提供"边"信息，网格线只消费之）。

**命中与选择**：任意点落在合并格内 → 归一为锚点格；区域选择与合并格有交集即包含整格。

**API**
- `SetSubItemSpan(ById, nRowSpan, nColumnSpan)` / `GetSubItemSpan(ById, ...)`
- `MergeSelectedCells()` / `UnmergeSelectedCells()`（作用于 C1 的选中区域）
- XML：`sub_item_row_span` / `sub_item_column_span`

**约束与冲突策略（已拍板）**
- 合并**不得跨越冻结列边界**（冻结分界处强制断开）。
- **自动解除合并**：排序 / 筛选 / 树折叠 / 行拖拽等会破坏"合并前提"的操作，触发时**自动解除受影响的合并**并派发通知（而非静默错误绘制）。

**验收**：合并格绘制无重复边框、命中/复制/编辑定位均正确、解合并可完整还原。

---

### C5　行分组折叠 / 内嵌树（A2-b / A2-c）

**数据模型（已拍板：内嵌树与行分组共用同一套层级模型）**：在行属性池（`ListCtrlItemData`）增加 `nParentRowId`（可选）与 `bExpanded`；**分组折叠** = 一种特殊"分组行"（`bIsGroupRow`，标题跨全列显示），与树节点共用同一套父子结构。

**显示顺序**：由"树展开扁平化"生成可见行列表。P1 Phase 1 已把**身份（RowId）与显示位置分离**（`m_displayOrder` 只随增删改/排序变动），这里正好复用：`m_displayOrder` 承载"扁平化后的可见顺序"。

**绘制**：列 0 增加缩进 + 展开/折叠 chevron；缩进宽度走 `ListCtrlSubItem` 的**左侧额外占位**通路（P5 已修正 `EstimateText` 只累加左侧占位宽度，正是为此类需求预留的点）。

**API**
- `SetItemParent(ById)` / `GetItemParent(ById)`
- `SetItemExpanded(ById, bool)` / `IsItemExpanded(ById)` / `GetItemLevel(ById)`
- `ExpandAll()` / `CollapseAll()`
- 分组：`InsertGroupRow(...)` / `IsGroupRow(...)` / `SetGroupCollapsed(...)`

**事件（三件套 + XML）**：`kEventItemExpandedChanged` / `kEventGroupCollapsedChanged`。

**协同**：排序在树/分组模式下**仅在兄弟节点内排序**；与合并互斥（见 C4 策略）。

**验收**：展开/折叠后可见行数、滚动 extent、键盘导航、选择、复制（自动跳过折叠行）全部正确。

---

### C6　多级表头（A2-d）

**结构升级（改动最大的一项）**：现有"偶数下标=表头项、奇数=分割条、列数=ItemCount/2"是硬编码假设，需改为**表头树**：
- `ListCtrlHeaderItem` 增加 `children`（子表头项）、`bIsLeaf`、叶子携带 `columnId`；
- 父节点宽度 = 子节点宽度之和（自动随叶子变化重算）；
- `SplitBox` 只挂**叶子**右侧（列宽拖动语义不变）；
- 保持 `GetColumnCount()` / `GetColumn()` 的**外部语义不变**（`columnIndex` 仍指叶子列序），内部改为遍历叶子。

**布局与对齐**：表头高度 = 层数 × 单层高度，`ListCtrlHeader` 的 Arrange 支持多层；行内单元格与**叶子列**对齐（由 C0 的列布局表保证），父表头跨列显示。

**交互**：点击父表头 = 选中其下所有叶子列（或作为 C1 的区域锚）；排序仅叶子可点。

**列拖拽换序（已拍板：允许跨父拖动）**：叶子可在父内 / **跨父**移动（沿用既有 `enable_header_drag_order` 与 `SplitBox` 拖动，[`ListCtrl.cpp`](../../duilib/Control/ListCtrl.cpp#L126-L158)）；跨父后该列的父节点归属按拖入位置重算，`columnId` 保持不变（列 ID 稳定，仅层级归属变化）。

**兼容**：单级表头 = "只有一层叶子"的退化情形，行为与现状一致。

**验收**：单级行为不变；二级表头的列宽调整、列显隐、横向滚动、冻结列均正确。

---

### C7　行拖拽换序（§四"行拖拽换序"）

- 复用 [`ControlDragableT`](../../duilib/Core/ControlDragable.h#L320-L368)（拖拽状态机 / 位置调整 / 目标项判断，[L374-431](../../duilib/Core/ControlDragable.h#L374-L431)、[L495-635](../../duilib/Core/ControlDragable.h#L495-L635)），与"列头拖拽换序"同源。
- 语义：拖动行到目标位置 → 修改 `m_displayOrder`（树模式下改兄弟顺序 / 改 parent）；绘制**落点指示线**（由几何层的行边界推导）。
- 与排序/筛选/树/分组协同：排序激活时禁止（或清除排序）；拖到某父行内 = 改变父级；拖入分组 = 加入该组。
- API：`SetRowDragEnabled(bool)`、`MoveDataItem(fromRowIndex, toRowIndex)`；事件 `kEventRowMoved`（三件套 + XML）。

**验收**：拖动后显示顺序与数据顺序一致，滚动/选择/复制均正确。

---

## 4. 依赖关系与建议落地顺序

```
C0 单元格坐标层 ──┬──▶ C1 单元格选择 ──┬──▶ C2 复制/导出
（无行为变化）    │                    └──▶ C3 单元格编辑 (+L5 编辑器类型)
                  │
                  ├──▶ C6 多级表头        （布局层 / 表头树）
                  ├──▶ C5 内嵌树+分组折叠  （布局层 / 行树，复用 m_displayOrder）
                  ├──▶ C4 单元格合并       （依赖 C1 的区域语义 + C0 几何）
                  └──▶ C7 行拖拽换序       （独立，需 C0 提供落点几何）
```

**建议顺序**：`C0 → C1 → C2 → C3 → C6/C5 → C4 → C7`。
理由：C0/C1 收益最大（直接解锁 L1/L2 两个"低-中成本"待办），且默认关闭、风险可控；C4/C6 改动面最大（数据模型 + 表头结构），放在坐标层与选择层稳定之后。

---

## 5. 风险与回退

| 风险 | 说明与对策 |
|------|-----------|
| 兼容风险 | 表头结构、行选择都是既有公共行为。**全部改动默认关闭**（`selection_mode="row"`、无合并、单级表头），未启用时逐字节等价 |
| 性能风险 | C0 引入几何层，须避免"每帧重排几何"。对策：列布局/行布局**带脏标记缓存**，只在列宽/列序/显隐/行高/展开变化时重算；绘制期只读（沿用 P4 已验证的 `NeedReArrange()` 守卫思路） |
| 语义冲突 | 合并 ↔ 排序/筛选/树折叠/行拖拽相互冲突。**已拍板：自动解除合并**——触发这些操作时自动解除受影响的合并并派发通知 |
| 存储层适配 | C4 的 span、C5 的 parent/expanded 必须以**稀疏字段**落在 P1 的分页连续存储里，避免重新引入内存膨胀 |
| 编辑器定位 | 复用 `GetTextRect()` 时补 P5 记录的兜底（矩形为空回退 `GetRect()`）；合并格按合并矩形定位 |
| 回退 | 每阶段独立提交、可单独 `git revert`；C0 本身不产生行为变化，回退最安全 |

---

## 6. 已拍板事项（实施依据）

> 以下 5 项已于设计评审阶段拍板，作为实施依据；实施过程中如需调整再更新本节。

1. **选择模式默认值**：默认 `kRow`（完全向后兼容）；**允许运行时动态切换**行选择与单元格选择（`SetSelectionMode` 可随时切换，切换时清空另一模式的选择状态）。
2. **合并与排序/筛选冲突策略**：**自动解除合并**——触发排序 / 筛选 / 树折叠 / 行拖拽时，自动解除受影响的合并并派发通知。
3. **内嵌树与行分组层级模型**：**共用同一套**（分组行 = 特殊父行）。
4. **多级表头跨父拖动列**：**允许**跨父拖动；跨父后该列的父节点归属按拖入位置重算，`columnId` 保持不变。
5. **复制格式**：默认 **TSV**；**CSV 导出作为 L2 的一部分一并实现**。

---

## 7. 文档同步清单（落地时按项目约定执行）

- `docs/ListCtrl-Roadmap.md` / `.en.md`：L1/L2/A2/§四对应条目状态与实现要点回填；A2 拆为 C4/C5/C6 三项。
- `docs/Control.md` / `.en.md`：`ui::ListCtrl` 新增接口、XML 属性（`selection_mode` / `sub_item_row_span` / `sub_item_column_span`）与示例。
- `docs/Events.md` / `.en.md`、`docs/XmlEvents.md` / `.en.md`：新增事件参数表与 XML 内联事件名。
- 三处技能参考副本（`.trae` / `.workbuddy` / `.claude`）与 `README.md` / `.en.md` 功能计数。
- 每项落地后运行 `python .trae/skills/nim-duilib/scripts/verify_docs.py` 校验无漂移。

---

## 8. 术语表

| 术语 | 含义 |
|------|------|
| 行（Row） | 数据层的一条记录，内部以 `RowId` 标识（P1 已引入） |
| 显示序（Display Index） | 行在当前可见顺序中的位置（受排序 / 树展开 / 分组折叠影响） |
| 单元格（Cell） | `{显示序, 列ID}` 唯一确定的一个格子 |
| 锚点格 / 焦点格 | 区域选择的起止单元格；规范化后即选中矩形 |
| 合并锚点 | 合并区域的左上格，唯一持有 span 信息 |
| 列布局表 | `columnId → {列序, x 偏移, 宽度, 可见}`，由表头叶子推导 |
| 几何层 | `ListCtrlCellGeometry`，唯一推导 cell 矩形 / 命中 / 网格线 / 落点 / 冻结区的地方 |
</content>
</invoke>
