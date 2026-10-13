# ListCtrl 数据存储重构设计方案

> 目标控件：`ui::ListCtrl` / `ui::ListCtrlData`（Report / List / Icon 三视图共用同一数据模型）。
> 面向三个硬约束：**绘制快（最高优先）**、**中间增删改不降级（拒绝 O(N) 大规模搬移）**、**改动易测试、可回退**。

---

## 0. 结论速览

一句话：**把"行身份"与"行物理位置"解耦**。

- 引入单调递增、删除不复用的 **`RowId`** 作为行身份；
- 新增 **`m_displayOrder : std::vector<RowId>`** 表达"显示顺序"，是**唯一**随 增/删/改/排序 变动的结构（元素仅 4B）；
- 每列的单元格数据、每行的属性数据**按 `RowId` 索引存放，永不因增删而搬移**；
- 增/删/改只改 `m_displayOrder`（4B 搬移）与目标行自身（O(1)）；
- 排序只重排 `m_displayOrder`，**彻底去掉现在"整列深拷贝"的写回**。

这套改动**分两个阶段**落地：Phase 1 先只做"身份/位置分离"（解决增删改性能，不动单元格存储形态）；Phase 2 再把单元格由"每格 `shared_ptr` 堆对象"换成"连续分页存储"（解决内存与绘制局部性）。每阶段独立可测、可回退。

---

## 1. 现状与瓶颈（带量化）

现状模型（见 [ListCtrlData.h](file:///c:/develop/nim_duilib/duilib/Control/ListCtrlData.h#L19-L24)）：

| 结构 | 定义 | 含义 |
|------|------|------|
| 单元格 | `m_dataMap: columnId → std::vector<std::shared_ptr<Storage>>` | **列优先**，`Storage = ListCtrlSubItemData2`(~96B) |
| 行属性 | `m_rowDataList: std::vector<ListCtrlItemData>` | 行属性 |
| 行身份 | 无独立身份 | **行索引即物理下标** |

瓶颈点：

1. `InsertDataItem` / `DeleteDataItem` 对**每一列**做 `vector::insert/erase`（见 [ListCtrlData.cpp](file:///c:/develop/nim_duilib/duilib/Control/ListCtrlData.cpp#L503-L514) 与 [L534-L539](file:///c:/develop/nim_duilib/duilib/Control/ListCtrlData.cpp#L534-L539)）——搬移量随"列数 × 行数"放大。
2. `SortDataItems` 写回时对**每列整列深拷贝**（见 [ListCtrlData.cpp](file:///c:/develop/nim_duilib/duilib/Control/ListCtrlData.cpp#L1578-L1592)）——O(N×C)。
3. 每格 `shared_ptr` 独立堆分配：内存开销大（~120~136B/格）。

**量化对比（N=10 万行、C=10 列，删除中间 1 行）：**

| 模型 | 单次删除搬移量 | 删 500 行 |
|------|--------------|-----------|
| 现状 | ≈ C×N×8B(=8MB) + N×sizeof(ListCtrlItemData)(≈3MB) ≈ **11 MB** | ≈ **5.5 GB**（数百 ms ~ 秒级） |
| 本方案(Phase 1) | ≈ N×4B(=400KB) + O(C) 原地标记 | ≈ **200 MB**（数十 ms）；配批量删除接口后 ≈ **400 KB** |

> 用户反馈的"删几百行要等秒级"正是第 1 条瓶颈所致。Phase 1 即可消除。

---

## 2. 目标架构：身份与位置分离

```
                  显示序间接表（唯一变动项，4B/元素）
显示第 i 行  ──►  m_displayOrder[i] == RowId ──►  列数据池[col][RowId]
                                             ──►  行属性池[RowId]
（排序 / 插入 / 删除只重排 m_displayOrder 与标记，数据本体不动）

列数据池（每列一份，按 RowId 索引）
  col#1: [ Slot(RowId=0) ][ Slot(1) ][ Slot(2) ] ...
  col#2: [ Slot(0) ][ Slot(1) ][ Slot(2) ] ...
```

### 2.1 行身份 `RowId`

- 类型：`typedef uint32_t RowId;`（`kInvalidRowId = 0xFFFFFFFF` 作哨兵）。
- 生成：`m_nextRowId` 单调自增，**删除后不复用**（保证已发出的 RowId 语义稳定）。
- 生命周期：一次"重编号压缩"（见 §5 Phase 3）可整体重排 RowId，属显式维护操作。

### 2.2 显示序间接表 `m_displayOrder`

- 类型：`std::vector<RowId> m_displayOrder;`
- 语义：`m_displayOrder[i]` 即"当前屏幕上第 i 行"的行身份；`GetDataItemCount() == m_displayOrder.size()`。
- **一切"位置相关"操作只作用于它：**
  - 中间插入：`m_displayOrder.insert(begin()+pos, newRowId)` → O(N) 搬 4B；
  - 删除：`m_displayOrder.erase(...)` 或批量标记后单趟压缩 → O(N) 搬 4B；
  - 排序：对 `m_displayOrder` 排序（比较时按 RowId 读排序列），O(N log N) 比较 + O(N) 4B 交换。

### 2.3 列数据池（Phase 1 与 Phase 2 两种形态）

- Phase 1（`std::vector<StoragePtr>` 按 RowId 索引，保留 `shared_ptr`）：
  - 增删不再触碰列 vector 的位置搬移；行删除使 `pool[rowId] = nullptr`（墓碑）。
  - 只拿到"增删改性能"，内存不变。
- Phase 2（**分页连续存储**，替换 `shared_ptr`）：
  ```cpp
  static constexpr size_t kPageCells = 64;             // 每页 64 个单元格（可配）
  struct StoragePage { std::array<Storage, kPageCells> cells;
                       uint8_t usedMask[kPageCells]; };  // 是否已写入
  // 每列： std::vector<std::unique_ptr<StoragePage>> m_dataMap;
  // 索引： page = rowId / kPageCells;  slot = rowId % kPageCells;
  ```
  - **懒分配**：某页第一次写入时才分配 → 稀疏/纯虚表场景内存不膨胀（保住 `SetDataItemCount` 的轻量语义）。
  - **连续**：页内数组连续，绘制/排序扫描缓存友好；无 `shared_ptr` 指针追逐。
  - **O(1) 随机访问**：`page = rowId / K` + `slot = rowId % K`。
  - **回收**：整页全墓碑时释放该页（见 Phase 3）。

### 2.4 行属性池

- Phase 1：`std::vector<ListCtrlItemData> m_rowDataList`（按 RowId 索引，append-only，删除留墓碑）。
- 选中状态（多选）本就在 `ListCtrlItemData.bSelected` 中 → 按 RowId 存放后，**排序/换序时选择状态自动跟随行**，与现状一致。

### 2.5 必须维持的不变量（供 `_DEBUG` 断言）

1. `m_displayOrder` 是"存活 RowId 集合"的一个排列（无重复、无失效 RowId）。
2. 每个存活 RowId 在各列池中的槽位可达（页存在、或被允许为"无数据"）。
3. `m_nSelectedRowId`（单选）若有效，则必须存在于 `m_displayOrder` 中。
4. `m_hideRowCount / m_heightRowCount / m_atTopRowCount` 与按 RowId 扫描结果一致。
5. 所有页的 `usedMask` 与实际写入状态一致；全墓碑页可被回收。

---

## 3. 复杂度对比（目标形态）

| 操作 | 现状 | 本方案 | 备注 |
|------|------|--------|------|
| 追加行 `AddDataItem` | O(1)均摊 + C 次堆分配 | O(1)均摊（Phase2 无分配） | 更快 |
| 中间插入 `InsertDataItem` | O(C×N) 搬 8B | **O(N) 搬 4B** | 关键提升 |
| 中间删除 `DeleteDataItem` | O(C×N) 搬 8B | **O(N) 搬 4B** + O(C) 标记 | 关键提升 |
| 批量删除 K 行 | O(K×C×N) | **O(N) 搬 4B**（单趟压缩） | Phase 3 |
| 单元格修改 `SetSubItem*` | O(1) + 可能分配 | **O(1) 原地写** | 更快 |
| 排序 `SortDataItems` | O(NlogN) + **O(N×C) 写回** | O(NlogN) 比较 + **O(N) 搬 4B** | 去掉整列深拷贝 |
| 按列取 `GetSubItemStorage(col,row)` | O(1)（map+下标） | O(1)（map+页下标） | 持平 |
| 按行填充 `GetSubItemStorageList(i)` | O(C) + **每次 new 临时 vector** | O(C)（可复用缓冲） | 去掉每次分配 |

---

## 4. 现有接口 → 新内部模型 映射

对外 API/行为**保持不变**（`itemIndex` 仍表示"显示索引"），仅内部实现改写：

| 现有接口 | 新内部实现 |
|---------|-----------|
| `AddColumn(cid)` | 新建该列的（分页）池；必要时按 `m_nextRowId` 预留 |
| `RemoveColumn(cid)` | 丢弃该列池；列空则清空 `m_displayOrder`/行属性池/生成器 |
| `AddDataItem` | `rid = m_nextRowId++`；各列池追加槽位、目标列写入；行属性追加；`m_displayOrder.push_back(rid)` |
| `InsertDataItem(pos)` | 同上分配 `rid`；`m_displayOrder.insert(begin()+pos, rid)`；**不动任何列池** |
| `DeleteDataItem(pos)` | `rid = m_displayOrder[pos]`；各列标记墓碑 O(C)；`m_displayOrder.erase`；计数修正 |
| `DeleteAllDataItems` | 清空所有池与 `m_displayOrder`；`m_nextRowId` 归零 |
| `SetDataItemCount(n)` | 追加/收缩 `m_displayOrder` 与行属性池；**列池懒分配**（Phase2 不预分配 96B×n） |
| `GetSubItemStorage(col,row)` | `rid = m_displayOrder[row]` → 页/槽读取（内部 `StoragePtr`→`const Storage*`） |
| `GetSubItemStorageList(i)` | `rid = m_displayOrder[i]` → 逐列读取（对外复用传入缓冲，避免每次 new） |
| 各 `Set/GetSubItem*` | 先 `row→rid`，再写/读目标列单元格 |
| 各 `Set/GetDataItem*`（行级） | 先 `row→rid`，再写/读 `行属性池[rid]` |
| `SortDataItems` | 仅对 `m_displayOrder` 排序（比较函数按列读 `pool[col][rid]`） |
| 单选 `m_nSelectedIndex` | 改为 `m_nSelectedRowId`；需要显示索引时在 `m_displayOrder` 中定位 |
| 多选 | 仍存于 `ListCtrlItemData.bSelected`，按 RowId 存 → 换序自动跟随 |
| `IsNormalMode` / 三个计数 | 按 RowId 扫描存活行统计 |

> 对外接口签名尽量不动；内部把 `StoragePtr`（`shared_ptr`）改为"按 RowId 的访问器 + 原始指针/引用"。改动会波及 `ListCtrlReportView::FillSubItemData` 等使用点，但都在 duilib 内部，需整体重编 `duilib` 库。

---

## 5. 分阶段实施（每阶段独立验收、可回退）

### Phase 0 — 基线与护栏（不改架构）
- 在 `examples/ListCtrl` 增加"压力测试"面板：**批量插入 10 万行 / 中间插入 / 删除 500 行 / 排序 / 滚动绘制**，用 `QueryPerformanceCounter` 打印耗时。
- 采集**现状基线**（记录到本文件），作为后续对比依据。
- 增加 `_DEBUG` 断言函数 `AssertDataConsistency()`（可先对旧模型做等价校验，作为回归网）。
- **验收**：基线数据留档；断言函数可运行。

### Phase 1 — 身份/位置分离（**解决增删改性能**，不动单元格形态）
- 引入 `RowId`、`m_nextRowId`、`m_displayOrder`；列池与行属性池改为**按 RowId 索引**（仍用 `shared_ptr`）。
- 改写 §4 表中所有接口的内部实现；`m_nSelectedIndex → m_nSelectedRowId`。
- **不动内存形态**，因此风险聚焦在"索引换算"上，便于定位。
- **验收**：
  - 现有功能与示例行为完全一致（回归）；
  - 中间插入/删除 1 行在 10 万×10 下 < 5 ms；
  - 删 500 行 < 100 ms（现状为秒级）；
  - 排序写回不再深拷贝整列；
  - `AssertDataConsistency()` 全绿。

### Phase 2 — 值存储 + 分页连续（**解决内存与绘制局部性**）
- 列池由 `vector<shared_ptr<Storage>>` 改为 `vector<unique_ptr<StoragePage>>`（页大小 64，懒分配，页内连续）。
- `StoragePtr` 使用点改为按 `(RowId → const Storage*)` 访问；`ListCtrlReportView::FillSubItemData` 同步适配。
- 行属性池可保持 `vector<ListCtrlItemData>`（体积小）。
- **验收**：
  - 内存：填满 10 万×10 单元格，峰值内存较现状下降 ≥20%（目标 ~30%）；
  - 绘制：滚动帧耗时 ≤ 现状（不得变差）；
  - 稀疏场景（`SetDataItemCount` 大、实际写入少）内存不膨胀；
  - `AssertDataConsistency()` 全绿。

### Phase 3 — 批量删除 + 存储压缩（**补齐批量与回收**）
- 新增批量接口 `DeleteDataItems(const std::vector<size_t>& displayIndexs)`：内部降序标记 + **单趟压缩** `m_displayOrder`，O(N) 而非 O(K×N)。
- 页回收：全墓碑页释放；提供 `CompactStorage()` 做"重编号 + 整体压缩"（显式维护操作，用于长时间高频增删后的内存回收）。
- **验收**：删 500 行（乱序索引）< 30 ms；多次增删后内存回落到接近存活规模。

### （可选）Phase 4 — 绘制专项
- 若基准显示"按行填充"仍是瓶颈，可评估**可见行行优先缓存**；否则维持列优先。
- 与既有 P4（`PaintChild` 每帧 `ReArrangeChild`）合并评估，二者是**互补**的绘制优化（数据层 + 布局层）。

---

## 6. 测试与验证策略

| 层次 | 手段 |
|------|------|
| 正确性不变量 | `AssertDataConsistency()`（`_DEBUG` 下每次结构性操作后调用）：排列性、可达性、计数一致、页掩码一致 |
| 行为回归 | 现有 `examples/ListCtrl` 全部功能逐项手测；重点覆盖：增删改、排序、单/多选、冻结列、单格刷新、换行 |
| 边界 | 空表、单行、末行插入/删除、删至空、`SetDataItemCount(0)`、首/末列、0 列 |
| 性能基准 | Phase 0 的压力面板，输出 P50/P95 耗时；对照 Phase 0 基线 |
| 内存 | 任务管理器/`GetProcessMemoryInfo` 采样：稠密填充 vs 稀疏填充 |
| 差分验证（可选） | 开发期临时保留"新旧双跑"开关：同一操作序列，比对新旧模型 `GetSubItemStorageList` 结果一致 |

**建议验收门槛（10 万行 × 10 列）**：中间插/删单行 < 5 ms；删 500 行 < 100 ms（Phase 3 < 30 ms）；排序 < 300 ms；滚动帧耗时不高于现状；稠密内存下降 ≥20%。

---

## 7. 风险与回退

| 风险 | 说明 | 对策 |
|------|------|------|
| 索引换算错误 | `row↔rid` 是最易错点 | 统一封装 `RowId DisplayIndexToRowId(i)` / 定位函数，只在此换算；`_DEBUG` 断言兜底 |
| 单选状态迁移 | `m_nSelectedIndex`→`m_nSelectedRowId` 语义变化 | 单独子任务 + 回归用例（单选、排序后单选、删除选中行） |
| 绘制取数生命周期 | Phase 2 `shared_ptr`→原始指针 | 取数仅在"填充/绘制"期间进行且期间不修改存储；断言校验 |
| 页回收与坐标 | `page=rowId/K` 必须始终成立 | 页槽位不重编号（Phase 3 的"重编号"是显式整表操作，非隐式） |
| ABI/ODR | 改动 `ListCtrlData` 布局 | 改后 **Rebuild `duilib` 库**再编 examples（与既有约定一致） |
| 回退 | 数据层改动面大 | 分阶段提交，Phase 1 / Phase 2 各自可独立回退（Phase 1 已能解决性能诉求） |

---

## 8. 兼容性与构建

- **对外 API 不变**：`ui::ListCtrl` 的公开接口、XML 属性、事件、示例用法全部保持不变（纯内部重构）。
- 内部受影响类：`ListCtrlData`、`ListCtrlReportView`（`FillSubItemData` / 单格刷新取数）、`ListCtrl`（`SortDataItems` 调用点）。
- 构建：改动 `ListCtrlData.h` 后 **Rebuild `duilib` target**，再编 `examples.sln`（Debug|x64）。
- 文档：完成后按约定同步 `docs/Control.md/.en.md`、`docs/ListCtrl-Roadmap.md/.en.md`（P1/P2 由"待评估"更新为"已完成/实施中"）及三处技能参考副本。

---

## 9. 待确认事项（实施前需拍板）

1. Phase 1 是否**单独交付并验收**（推荐先只做 Phase 1 解决性能），Phase 2 是否本轮做。
2. 分页页大小 `kPageCells` 取值（建议 64；需要时再调）。
3. 批量删除接口是否作为公开 API 暴露（`DeleteDataItems(vector<size_t>)`），还是仅内部优化。
4. 是否需要 Phase 4 的行优先绘制缓存（取决于 Phase 1/2 后的实测）。
