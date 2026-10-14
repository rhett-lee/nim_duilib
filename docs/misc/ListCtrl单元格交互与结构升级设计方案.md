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
   列序 / 显隐  ───▶ │  （由可见行几何推导，回退表头）            │
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

### C0　单元格坐标层（基础，**无用户可见行为变化**）　【已完成】

**做什么**
1. 把 `ListCtrlReportView` 内私有的 `FrozenTransform`（A1 产物）上提/推广为 `ListCtrlCellGeometry`，保留冻结列语义（冻结区右界、命中归零、两趟裁剪、网格线回位），并新增"列 X 区间 / 行 Y 区间 / cell 矩形"能力。
2. 新增 `ListCtrlColumnLayout`：由**首个可见数据行的子项控件几何**推导 `columnId → {列序, X 区间, 宽度}`（无可见数据行时回退到**表头几何**），作为"某一列 X 区间"的唯一来源，替代此前在 `GetFrozenColumnsWidth` 中扫描子项控件逐个累加的隐式排布。
3. 把绘制 `PaintChild`、网格线 `PaintGridLines`、命中 `FindControl`、tooltip 判定，全部改为消费几何层。

> **实施说明（与上述设计的偏差，已落地）**：列布局表的 X 区间**不由表头列宽累加**得到，而是**读取"已排布后的子项控件几何"**。原因是列内容坐标还受"行首 CheckBox / 内边距 / 子项间距"等占位影响，仅凭表头列宽无法复现；宽度数值的最终来源仍是 `FillDataItem`（表头列宽 + 第 1 列的对齐修正），故 C0 与既有行为**逐字节等价**。`ListCtrlCellGeometry` 已替换 `FrozenTransform`（`BuildFrozenTransform()` → `BuildCellGeometry()`；冻结区宽度改由列布局表 `GetColumnsWidth()` 推导，保留"冻结列数超出实际列数时按实际列数截断"的历史语义）。

**为什么先做**：它是其余所有项的公共地基，且**不改变任何可见行为**——相当于把 A1 的手法从"冻结列"推广到"整张表"，可先单独验证无回归。

**验收**：冻结列开/关、列显隐、列宽调整、列拖拽换序、排序、置顶行等全部现状行为不变（回归性验证）。

---

### C1　单元格选择模型（**解锁 L1 / L2 的关键**）　【已完成】

**状态（存数据层）**
- 在 `ListCtrlData` 增加单元格级状态：锚点 `m_anchorCell` + 当前格 `m_currentCell`（均以**行身份 RowId + 列ID** 存储，与显示顺序解耦）+ 选中列ID范围 `m_cellColumnIdBegin/End` + **选中行集合** `m_cellRowSelected`（按 RowId 索引的位图）。
- 区域选择在设置时把显示区间内的行展开为行身份集合存入 `m_cellRowSelected`：**排序/增删后选择状态跟随原数据行**，不再因显示顺序变化而错位到其他行；行成员判定 O(1)。
- 行选择状态保留不动（`bSelected` / `m_nSelectedRowId` 继续服务 `kRow` 模式）。
- **演进（见 C1b）**：该"单矩形"模型无法表达**不连续**选择，且 `kCell` 模式下的行选择入口未全部受模式约束；C1b 将其升级为"**逐单元格选中状态** + 行/单元格统一动作语义"。
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
- `kEventCellSelectionChanged`（区域变化；pEventData 指向选中区域列表 `std::vector<ListCtrlCellRange>`，见 C1b）
- XML 内联事件名同步写入 `docs/XmlEvents.md`（`on_current_CellChanged` 等，命名随三件套表生成）。

**交互**
- 鼠标：`kCell` 模式下 `ButtonDown` 先 `GetCellAt(point)` 命中单元格 → 设锚点/当前格；`Shift+Click` 扩展到焦点格；`Ctrl+Click` 切换；`MouseMove` 拖动做矩形扩展，到视口边缘触发自动滚动。
- 键盘：方向键移动当前格（横向沿**可见列序**）、`Shift+方向` 扩展区域、`Ctrl+A` 全选单元格、`Home/End/PageUp/PageDown` 与既有行级导航共存（由模式决定语义）。
- 绘制：选中区域半透明填充；当前格画焦点框；**冻结列单元格同样参与选择**（几何层已统一处理冻结区）。
- **说明**：`Ctrl+Click` 切换等在 C1 阶段未落地；行/单元格动作语义与逐单元格状态由 **C1b** 统一定义，C1 阶段的"单矩形"选择语义由 C1b 取代。

**验收**：`kRow` 模式下所有既有行选择/键盘/事件行为**完全不变**；`kCell` 模式下可选中任意单元格与矩形区域。

---

### C1b　统一选择模型（行选择 / 单元格选择）　【已完成】

**问题（C1 落地后暴露）**
- **行选择入口未全部受模式约束**——`kCell` 模式下，鼠标点击行 / 行 Hover 高亮、键盘 `Ctrl+A` 与方向键（`ListBox::OnListCtrlKeyDown` 分支）、以及 `SetSelectedDataItems` / `SetSelectAll` / `SetSelectNone` 等 API 仍会改动**行**选择状态，导致"行选择状态错误"。
- **单元格选择被建模为单个矩形**（锚点 + 焦点 + 行位图 + 列范围），**无法表达不连续选择**，且 `Ctrl+Click` 切换未实现。

**统一原则（对齐 Windows 资源管理器）**
1. **选择粒度由模式唯一决定**：`kRow` → 行、`kCell` → 单元格；切换模式清空另一模式（已拍板事项 1）。
2. **两种模式共享同一套动作语义**：鼠标 / 键盘 / API 一一对应，仅"作用单元"由行变为单元格。
3. **状态一律存数据层、按身份（RowId）索引**，与显示序解耦：行 → `ListCtrlItemData::bSelected`（现状）；单元格 → 底层新增"每单元格选中状态"（本项新增）。
4. **模式隔离（强制，只隔离"视觉"不隔离"状态"）**：`kCell` 模式下界面不得显示出任何"行被选中 / 行可选"的视觉表现（含行选中背景色、行 Hover 高亮、行级框选蒙层、右键触发的行选中）；但底层行选择**状态**本身不作屏蔽（见评审结论第 3 条）。反之 `kRow` 模式下不显示单元格选择。

**评审结论（已拍板）**
1. `Ctrl+拖动` **要做**：向已有选择**追加**一个矩形区域（不清除）；拖动过程需绘制**半透明矩形蒙层选择框**（行选择模式已支持，复用同一绘制通路）。
2. `Ctrl+A` / `SetSelectAll` / `SetSelectNone` 在 `kCell` 模式**按模式分派**（全选 / 清空**单元格**）。
3. 行专用 API（`SetSelectedDataItems` 等）在 `kCell` 模式**保留原有行为**（仍正常读写底层行选择状态，不做 no-op）；隔离改为在**填充/绘制层**屏蔽行选中视觉表现，使界面看不出行可选状态；另提供单元格等价 API（`SetSelectedCells` / `GetSelectedCells`）。
4. `kEventCellSelectionChanged` payload 改为**选中区域列表** `std::vector<ListCtrlCellRange>`。
5. 存储用**独立位图**（不放进 `ListCtrlSubItemData2`，理由见下）。
6. C1 阶段的"单矩形"实现**按本设计重新实现**（C1 的对外 API 名与语义尽量保留，内部模型替换）。

**底层数据（`ListCtrlData` 新增）**
- 每列一张"单元格选中位图"：`std::unordered_map<size_t /*columnId*/, std::vector<uint8_t> /*按 RowId 索引*/> m_cellSelected`；`AllocRowId` 时各列位图同步扩容，`FreeRowId` 时清位；`CompactStorage` 重编号 RowId 后需按 `m_displayOrder` 重映射位图（锚点/当前格同步重映射）。
- 保留锚点格 `m_anchorCell` / 当前格 `m_currentCell`（均 `RowId + 列ID`），**仅用于 Shift 区域扩展与焦点框绘制**；选中集合不再由锚点/焦点推导，避免排序后漂移。
- 新增接口：`IsCellSelected(RowId, columnId)`、`SetCellSelected(RowId, columnId, bool)`、`ClearAllCells()`、`SelectAllCells()`。
- **存储位置决策（已拍板）：独立位图，不放进 `ListCtrlSubItemData2`。** 原因：
  1. `ListCtrlSubItemData2` 是**分页懒分配**存储（`GetStorageAtForWrite` 首次写入才 `make_unique<StoragePage>`，`usedMask` 标记已写入格）。若把选中位放进去，"全选单元格"（`Ctrl+A`）或对**未实化行**的选中会强制为整表分配单元格实体——内存从"每格 1 bit"（10 列 × 100 万行 ≈ 1.25MB）暴涨到"每格一个结构体"（含 `UiString` 等），直接破坏 Phase 2 建立的稀疏内存优势。
  2. 选中是**瞬态 UI 状态**，业务数据写入通路（`FillElement` / `SetSubItemData` / 虚表滚动复用）不应顺带改写它；分离存储避免两者耦合被误改。
  3. 与行选择的对称性由"**同样按身份(RowId)索引**"保证，而非"放在同一个结构体里"——行的 `bSelected` 稠密（每行一条），单元格是"行 × 列"，量级不同，独立位图更经济。

**动作语义对照表**

| 动作 | `kRow` 模式（保持现状） | `kCell` 模式（本项） |
|---|---|---|
| 单击 | 选中该行（清其它） | 选中该格（清其它），更新锚点/当前格 |
| `Ctrl+单击` | 切换该行选中，不清其它 | **切换该格选中**（再次点击取消），不清其它 |
| `Shift+单击` | 锚点行 → 该行（替换） | 锚点格 → 该格矩形（替换） |
| 拖动 | 左/右键行区域框选（替换，`ListBoxHelper` 半透明蒙层） | 左/右键拖动：由框选矩形（同款 `ListBoxHelper` 半透明蒙层）命中的单元格决定选择（替换） |
| `Ctrl+拖动` | 追加框选区域 | 追加矩形区域（不清其它，只增不减） |
| 空白单击 | 清空行选择 | 清空单元格选择 |
| 方向键 | 移动当前行（单选替换） | 移动当前格（单选替换该格） |
| `Shift+方向` | 扩展行区域 | 扩展矩形区域 |
| `Ctrl+方向` | 只移焦点/滚动，不改选择 | 只移当前格焦点，不改选择 |
| `Home/End` | 首/末行 | 当前行首/末列；`Ctrl+Home/End` = 表格首/末格 |
| `PageUp/PageDown` | 翻页移动当前行 | 翻页移动当前格 |
| `Ctrl+A` | 全选行 | **全选单元格** |
| `Ctrl+Space` | 切换当前行选中 | 切换当前格选中（可选） |
| `SelectAll/SelectNone` API | 行 | 单元格（**按模式分派**） |
| `SetSelectedDataItems` API | 行 | 保留原有行为（仍读写底层行选择状态）；界面侧由**绘制层**屏蔽行选中视觉（`ListCtrlReportView::CanPaintSelectedColors` 返回 false），另提供单元格等价 API `SetSelectedCells` |

**交互层改造点（入口清单，全部受模式约束）**
- 鼠标：`ListCtrlItem::ButtonDown/ButtonUp`（已有守卫，需补齐）、`ListBoxItemTemplate::HandleEvent` 的 Hover / Pushed 状态、`ListCtrlReportView::OnListBoxItemMouseEvent`（`kCell` 模式下拦截 Hover 与左/右键按下、抬起、移动，**先把事件转发给基类喂给 `ListBoxHelper`（获得与行模式一致的框选状态与蒙层绘制），再按单元格语义处理，最后消费事件**，从而既不触发行选择、又能显示框选蒙层；因消费导致 Item 不执行 `Control::ButtonDown`，需显式对 `msg.GetSender()` 调 `SetMouseFocused(true/false)` 以满足 helper 的 `pSender->IsMouseFocused()` 守卫）。**注意：`kEventMouseEnter` / `kEventMouseLeave` 不消费**，必须继续向父控件冒泡（否则视图收不到 `MouseEnter` / `MouseLeave`，自动隐藏滚动条无法淡入淡出）；行级 Hover / Pressed 视觉由 `ListCtrlItem` 重写的 `MouseEnter` / `MouseLeave`（`kCell` 下不进入 Hovered 状态、不启动悬停渐隐动画）屏蔽，绘制层的 `ListCtrlItem::PaintStateColors` 作为兜底。`ListBoxHelper` 框选在 `kCell` 下**不做额外开关**：沿用其既有守卫（`IsEnableFrameSelection() && IsMultiSelect()`，框架层未改动），框选矩形交给 `OnFrameSelection` 的 `kCell` 分支按矩形命中单元格（`Ctrl+拖动` 追加，故单元格框选需 `multi_select == true`）。
- 键盘：`ListCtrlReportView::HandleEvent(kEventKeyDown)` → `OnCellKeyDown` 必须拦截**全部**选择相关键（含 `Ctrl+A`、方向键、`Home/End/Page*`），确保不会落到 `ListBox::OnListCtrlKeyDown` 的行选择分支。
- API：`ListCtrl::SetSelectAll/SetSelectNone` 按模式分派；`SetSelectedDataItems` 等**保留原有行为**（不再加模式守卫）；行选择视觉的屏蔽统一收敛到**绘制层**：`ListCtrlReportView::CanPaintSelectedColors(bHasStateImages)` 在 `kCell` 模式下返回 false，使任何入口（鼠标 / 键盘 / API）改动的行选择状态都不显示到界面（唯一视觉输出口，避免逐个入口堵截）。

**事件**：`kEventCellSelectionChanged` 的 payload 由"单矩形"改为**选中区域列表** `std::vector<ListCtrlCellRange>`（可能多项；空表示无选择）；详细单元格列表仍可通过 `GetSelectedCells()` 查询。

**绘制**：逐格 `m_pData->IsCellSelected(RowId, columnId)`（O(1)）；拖动过程的**半透明矩形蒙层选择框**完全复用行选择模式的框选通路（`ListBoxHelper::PaintFrameSelection`，颜色/透明度/DPI 缩放边框/顶部裁剪到表头下沿全部一致；`ListCtrlReportView::PaintCellSelection` 不再自绘蒙层，只画单元格选中填充 / 悬停填充 / 当前格焦点框）；`kCell` 模式下不绘制行背景 / 行 Hover。

**置顶行处理**：表头与置顶行（不可选行）固定显示在顶部，会遮挡其下方滚动的普通行，因此不参与单元格选择与高亮绘制：
- 鼠标入口处用 `ListCtrlReportView::IsPointOnAtTopItem` 判断落点是否在置顶行内，是则跳过单元格的选择与悬停处理（并清除悬停状态），避免"穿透"选中被遮挡的下方单元格，也不误清空已有的单元格选择；
- 绘制高亮（`PaintCellSelection`）与框选命中（`OnFrameSelectionForCells`）前，用 `GetTopFixedItemsRect` 取表头与置顶行所占区域，把被其遮挡的部分从单元格矩形中裁剪掉，避免高亮覆盖置顶行内容。

**框选坐标口径**：`ListBoxHelper` 传给 `OnFrameSelection` 的矩形是"内容坐标"（叠加了 ListBox 自身的滚动量），而单元格子控件的矩形是"控件坐标"：
- 纵向：Report 视图采用虚拟滚动布局（控件被复用后重新排列在可视位置），控件矩形的位置就是屏幕位置、不随纵向滚动移动，故 `OnFrameSelectionForCells` 需先扣除 `GetScrollPos().cy` 再做命中判定；否则滚动后框选会与鼠标实际框出的位置相差一个滚动量；
- 横向：是真实滚动条偏移（绘制时按 `-偏移量` 移动、命中时按 `+偏移量` 还原），两者口径已一致，无需换算。

**验收**：`kRow` 行为完全不变；`kCell` 下支持任意不连续单元格选择（`Ctrl+Click` 切换）、`Ctrl+拖动` 追加、`Shift` 矩形扩展；`kCell` 下行选择状态与行 Hover 均不再出现；模式切换清空对侧状态。

**交互缺陷修复补充（实测反馈）**
- **焦点机制（CheckBox 勾选 / 键盘导航的前提）**：`kCell` 模式下鼠标事件在视图层被消费、行选择逻辑不执行，导致 `ListBoxItem` 从未成为窗口焦点控件。`ListCtrlReportView::OnListBoxItemMouseEvent` 现在在左/右键按下时，对 `HitTestCell` 命中的 `ListCtrlItem` 显式调用 `SetFocus()`（与行模式 `ListBox::SelectItemSingle` 的焦点设置一致）：这样 (1) 单元格内的 `ListCtrlLabel`（`CheckBoxTemplate<HBox>` 子类）满足其勾选前置条件 `pWindow->GetFocusControl() == m_pListBoxItem`，CheckBox 可正常勾选/取消勾选；(2) 键盘事件经焦点控件冒泡到视图被 `OnCellKeyDown` 处理，方向键、`Ctrl+A` 稳定生效。为此 `HitTestCell` 增加了可选出参 `ListCtrlItem** ppItem` 输出命中的行项。
- **行级悬停/按下视觉的屏蔽（状态层 + 绘制层）**：`list_ctrl_item` 类定义了 `hovered_color="bg_list_ctrl_item_hovered"`（以及 `pressed_color`）；同时框架在设置 `hovered_color` 时会自动 `SetFadeHovered(true)` 创建悬停动画播放器。若行项进入 Hovered 状态，行选择模式下会渐显整行背景；而在 `kCell` 模式下行项**根本不应进入 Hovered**，否则移出时 `Control::MouseLeave` 触发的 `ReverseContinue()` 渐隐动画会在状态已回 Normal 时经 `StateColorMap::PaintStateColor` 的动画分支继续绘制 hovered 色，表现为"移出单元格时该行闪现一下悬停色"的闪烁。故 `kCell` 模式下：
  - 状态层：`ListCtrlItem` 重写 `MouseEnter` / `MouseLeave`，`kCell` 时**不调用基类逻辑**（不置 Hovered、不启动/反转悬停动画），但仍 `return false` 让事件继续冒泡给视图（滚动条淡入淡出依赖之）；`MouseLeave` 仅清除可能由 `SetFocus()`（`Control::OnSetFocus` 会置 Hovered）残留的行级悬停状态。`kRow` 行为完全不变。
  - 绘制层（兜底）：`ListCtrlItem::PaintStateColors` 在状态为 `kControlStateHovered` / `kControlStatePressed` 时**直接跳过基类绘制**，覆盖 `SetFocus()` 造成的 Hovered 副作用。
- **置顶行（不可选行）的 CheckBox 焦点**：`ListCtrlLabel::OnMouseEvent` 对"非 Enter/Leave/Move/Hover"的鼠标事件有前置条件 `pWindow->GetFocusControl() == m_pListBoxItem`，否则直接返回、不执行 CheckBox 的勾选逻辑。普通行在按下时由 `HitTestCell` 命中的行项 `SetFocus()` 满足该条件；而置顶行既被 `HitTestCell` 跳过（不可选），又会走 `IsPointOnAtTopItem` 的提前 `return true`，导致其 CheckBox 永远拿不到焦点、无法勾选（行模式由 `ListBox::SelectItemSingle` 的焦点设置而正常）。故 `kCell` 模式下在置顶行分支内对左/右键按下补充焦点：新增 `ListCtrlReportView::HitTestTopItem` 命中鼠标点所在的置顶行列表项并 `SetFocus()`（`IsPointOnAtTopItem` 的 `return true` 消费逻辑与 Move/Hover 清悬停逻辑保持不变）。
- **右键语义**：`kCell` 模式下右键点击「已选中」的单元格时，`BeginCellDrag` 保持现有选择集合不变，仅同步锚点/当前格（供弹出右键菜单对已选项执行操作），与行模式 `ListBox::ListCtrlSelectItem`（`if (bRbuttonDown && IsItemSelected(iIndex))`）语义一致；右键点击未选中单元格仍为替换选择。
- **Enter/Leave 不消费（滚动条可见性）**：item 级的 `kEventMouseEnter` / `kEventMouseLeave` 不再被消费，必须继续冒泡到父控件（视图 `ScrollBox`）——否则视图永远进不了 Hovered 状态，`ScrollBox::MouseEnter` 就不会对自动隐藏的滚动条调 `SetFadeVisible(true)`，表现为"鼠标在视图内但不显示滚动条"（行选择模式因基类 `OnListBoxItemMouseEvent` 返回 false 而正常）。
- **悬停清除**：item 级 `kEventMouseLeave`（不再消费）仍会调用 `ListCtrl::ClearHoverCell()`，避免鼠标移出单元格到视图空白区域后悬停高亮残留在最后划过的单元格上。

**示例程序（examples/ListCtrl）**：新增「选择方式」开关（`selection_mode='row'|'cell'` 内联事件 + `STRID_LISTCTRL_SELECTION_*` 语言串）；「支持多选」复选框（`multi_select={true|false}`）即单/多选切换开关（既有）。

**C1 复查结论**（复查范围：C0 / C1 / C1b 全部已落地代码）

**遗留未实现 / 状态更正**
1. **冻结列与单元格选择的命中口径适配：未完成。** `ListCtrlReportView::HitTestCell` 与 `OnFrameSelectionForCells` 均未做"冻结列"判定（仅 `PaintCellSelection` 分冻结趟绘制）。横向存在滚动偏移且同时启用冻结列时，点击 / 框选的命中可能与绘制结果错位。**待适配并实测验证。**
2. **视口边缘拖拽自动滚动：已具备（更正为此前的"遗留"记载）。** 由两种选择模式共用的 `ListBoxHelper::OnCheckScrollView`（定时器驱动）实现，`kCell` 下框选拖动到视口边缘即触发。
3. **C1 阶段的"单矩形"选择语义与 `Ctrl+单击` 切换：已由 C1b 取代并落地，闭环。**
4. **属后续阶段、尚未实施**：`Ctrl+C` 复制 / CSV 导出（C2 ＝ L2）、`F2` 进入单元格编辑（C3 ＝ L1）。

**已确认缺陷（已修复，Debug|x64 编译验证通过）**
1. 【中·已修复】**右键按在"已选中"单元格上继续拖动会清除选择。** 原因：`ListCtrl::BeginCellDrag` 的"右键命中已选格"分支未 `return`，函数末尾仍置 `m_bCellDragSelecting = true`，随后 `ListBoxHelper` 的右键框选按"替换"语义执行 → 清除可见区已选单元格。修复：该分支末尾直接 `return`，不再进入拖动框选状态（选择集合保持不变，供右键菜单使用）。
2. 【中·已修复】**单元格全选 / 批量设置 / 查询未过滤不可选行。** 修复：`ListCtrlData::SetCellSelected`、`SetCellSelectionRect`（`SelectAllCells` 与 Shift 区域扩展的公共通路）增加 `IsSelectableElement`（`bVisible && nAlwaysAtTop < 0`）过滤；`ListCtrl::SetSelectedCells`、`GetSelectedCells`、`GetSelectedCellRanges` 同步跳过置顶行 / 不可见行。
3. 【中低·已修复】`kEventCellSelectionChanged` 重复派发。修复：`ListCtrlData::SetCellSelected` 改为比对 `IsCellSelectedByRowId` 旧值，仅当选中状态真正变化时返回 true（返回语义由"是否设置成功"改为"是否发生变化"），拖动期间鼠标移动不再重复派发事件。
4. 【低·已修复】`m_hoverCell` 以显示索引存储。修复：改为私有 `HoverCellId`（`RowId + 列ID`）存储，`UpdateHoverCell` / `IsCellHovered` 经 `GetRowIdByDisplayIndex` 换算（为此在 `ListCtrlData` 增加 `friend class ListCtrl;`），排序 / 删行后悬停高亮不再漂移到其它数据行。
5. 【低·文档·已修复】`duilib_defs.h:490-491` 对 `kEventCellSelectionChanged` 的注释更正为"`wParam` ＝ 选中区域个数，`lParam` ＝ 0，`pEventData` 指向 `std::vector<ui::ListCtrlCellRange>`"，与实现一致。

**已核实无误（排除的疑点）**
- 选中集合、锚点、当前格均以 `RowId` 存储，排序 / 增删后跟随原数据行；列删除同步丢弃对应列位图；模式切换双向清空对侧状态。
- `OnFrameSelectionForCells` 纵向扣除 `GetScrollPos().cy`、横向不换算，与 `HitTestCell`（经 `GetSubItem` 叠加 `GetScrollOffsetInScrollBox`）口径一致，非缺陷。
- `SelectAll / SelectNone / SetSelectedDataItems` 的模式分派与绘制层视觉屏蔽符合"只隔离视觉、不隔离状态"的既定决策。

**C1c 单选语义对齐（已实施，Debug|x64 编译验证通过）**

问题：`multi_select="false"`（单选）在行模式下严格执行"至多一个选择单元"，但 `kCell` 模式的各入口均未读取该开关，导致单选下仍可用 `Shift/Ctrl+单击`、`Shift+方向键`、`Ctrl+A`、批量 API 选中多个单元格（而框选因 `ListBoxHelper::OnMouseMove` 要求 `IsMultiSelect()` 本来就不生效），同一开关下两种模式行为割裂。

决策（已拍板）：单一选择开关对两种模式语义一致 —— 单选下至多一个单元格；批量 API 取"第一个有效格"；"当前格焦点框"保留，仅把选中集合限制为 1。

实施要点：
- `ListCtrl::BeginCellDrag`：`!IsMultiSelect()` 时忽略 Shift/Ctrl，按普通单击处理（`SetCurrentCell` 替换选中该格），并**不进入**拖动框选状态（避免拖动时焦点框跟随鼠标而选中集合不动造成视觉错位）。
- `ListCtrl::OnCellKeyDown`：`Ctrl+A` 仅在多选下生效；`Shift+方向键` 的区域扩展分支增加 `IsMultiSelect()` 条件，单选下退化为普通移动当前格（选中集合随之保持 1 个）；`Ctrl+Space` 维持"至多一个"的切换语义。
- `ListCtrl::SetSelectAll()`：函数入口统一加 `!IsMultiSelect()` 早退（两种模式"全选"均无意义；同时避免行模式下继续走到 `ListCtrlData::SelectAll` 内的 `IsMultiSelect` 断言）。
- `ListCtrl::SetCellSelection()`：单选下忽略追加语义，仅选中焦点格。
- `ListCtrl::SetSelectedCells()`：单选下取列表中第一个有效单元格后即结束（`break`），且无论 `bClearOthers` 为何值都先清空（保证结果至多一个单元格）。
- `ListCtrl::SetMultiSelect(false)`：由多选切到单选时，把已选中的多个单元格裁剪为保留首格（`GetSelectedCells` + `SetSelectedCells({首格}, true)`），与行模式 `ListBox::OnSwitchToSingleSelect` 对称。
- 未改动项：框选（橡皮筋）本就被 `IsMultiSelect()` 把守；`GetSelectedCells` / `GetSelectedCellRanges` 为查询接口，单选下至多回报一个单元格，无需额外裁剪。

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
