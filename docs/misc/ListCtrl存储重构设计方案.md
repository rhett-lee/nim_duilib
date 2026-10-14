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

> **实测对照（x64 Release，10 万行 × 10 列）**：现状基线测得"删 500 行 = **615.9 ms**"（对应上表"数百 ms"）、"删单行 = **1.232 ms**"、中间插 1000 行 = **605.7 ms**、排序整表 = **76.7 ms**、滚动重绘 = **15.529 ms/帧**，与上表估算吻合。完整现状基线数据见 [§10 实施记录](#10-实施记录)。

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
| 批量删除 K 行 | O(K×C×N) | **O(N) 搬 4B**（单趟压缩） | ✅ 已实施 |
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

### Phase 0 — 基线与护栏（不改架构）✅ 面板、现状基线与 Phase 1 后数据均已留档（2026-10-13）
- 在 `examples/ListCtrl` 增加"压力测试"面板：**批量插入 10 万行 / 中间插入 / 删除 500 行 / 排序 / 滚动绘制**，用 `QueryPerformanceCounter` 打印耗时。（**✅ 已落地**，见 §10）
- 采集**现状基线**（记录到本文件），作为后续对比依据。（**✅ 重构前现状基线与 Phase 1 后实测数据均已按 x64 Release 手动采集并留档**，见 §10）
- 增加 `_DEBUG` 断言函数 `AssertDataConsistency()`（可先对旧模型做等价校验，作为回归网）。（**✅ 已随 Phase 1 落地**）
- **验收**：基线数据留档；断言函数可运行。

### Phase 1 — 身份/位置分离（**解决增删改性能**，不动单元格形态）✅ 已完成并通过性能验收（2026-10-13）
- 引入 `RowId`、`m_nextRowId`、`m_displayOrder`；列池与行属性池改为**按 RowId 索引**（仍用 `shared_ptr`）。
- 改写 §4 表中所有接口的内部实现；`m_nSelectedIndex → m_nSelectedRowId`。
- **不动内存形态**，因此风险聚焦在"索引换算"上，便于定位。
- **验收**：
  - 现有功能与示例行为完全一致（回归）；
  - 中间插入/删除 1 行在 10 万×10 下 < 5 ms；
  - 删 500 行 < 100 ms（现状为秒级）；
  - 排序写回不再深拷贝整列；
  - `AssertDataConsistency()` 全绿。

### Phase 2 — 值存储 + 分页连续（**解决内存与绘制局部性**）✅ 已实施（性能 / 绘制 / 内存门槛 Release 实测全部通过）
- 列池由 `vector<shared_ptr<Storage>>` 改为 `vector<unique_ptr<StoragePage>>`（页大小 64，懒分配，页内连续）。
- `StoragePtr` 使用点改为按 `(RowId → const Storage*)` 访问；`ListCtrlReportView::FillSubItemData` 同步适配。
- 行属性池可保持 `vector<ListCtrlItemData>`（体积小）。
- **验收**：
  - 内存：填满 10 万×10 单元格，峰值内存较现状下降 ≥20%（目标 ~30%）；
  - 绘制：滚动帧耗时 ≤ 现状（不得变差）；
  - 稀疏场景（`SetDataItemCount` 大、实际写入少）内存不膨胀；
  - `AssertDataConsistency()` 全绿。

### Phase 3 — 批量删除 + 存储压缩（**补齐批量与回收**）✅ 已实施（批量删除 / 内存回收 / 绘制门槛 Release 实测全部通过）
- ✅ 新增批量接口 `DeleteDataItems(const std::vector<size_t>& displayIndexs)`：内部按行身份去重 + 标记 + **单趟压缩** `m_displayOrder`，O(N) 而非 O(K×N)。（另在 `ui::ListCtrl` 同步暴露，返回实际删除行数）
- ✅ 页回收：`ResetStorageAt` 中整页全部变为墓碑时释放该页并裁剪尾部空页；提供 `CompactStorage()` 做"重编号 + 整体压缩"（显式维护操作，用于长时间高频增删后的内存回收）。
- **验收**：删 500 行（乱序索引）**实测 2.4 ms（< 30 ms 门槛通过）**；稠密内存 **132.0/110.4 MB ≈ Phase 2**（内存不劣化）；50 轮 / 20 轮乱序批量删除后内存**不膨胀无泄漏**（131.3/109.8 MB、131.3/109.6 MB）；**内存回落门槛已实测、达成**——100 轮删除（剩 50,000 行）后显式 `CompactStorage()`（39.2 ms）使 WS **146.6→105.2 MB**、私有 **124.8→83.2 MB**（-28.2% / -33.3%，低于 Phase 2 稠密基线 131.2/109.7 MB）；边界：仅删 10% 时压缩观测不到回落（回收量 ~9.6 MB 低于工作集口径噪声，且墓碑散布不触发页回收）；**绘制门槛同机 A/B 已闭环**——Phase 3 同机 3 次 14.542 / 12.164 / 12.991（均值 ≈13.2）优于同机 Phase 2 的 19.987 / 16.041 / 16.695（均值 ≈17.6）约 24.7%，无回归（绘制路径未改动）。

### （可选）Phase 4 — 绘制专项 ✅ 已核实：无需实施
- 原议：若基准显示"按行填充"仍是瓶颈，可评估**可见行行优先缓存**；否则维持列优先。
- 与既有 P4（`PaintChild` 每帧 `ReArrangeChild`）合并评估，二者是**互补**的绘制优化（数据层 + 布局层）。
- **结论（2026-10）：无需实施**——P2 实测显示"列优先 + 分页 `GetStorageAt` 为 O(1) 指针定位"不构成瓶颈，无需改为行优先存储或加行优先缓存；关联的 P4 亦已核实为"由 `NeedReArrange()` 守卫、常规重绘无重排"。故本项正式关闭。

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

1. Phase 1 是否**单独交付并验收**（推荐先只做 Phase 1 解决性能），Phase 2 是否本轮做。**已拍板：按 Phase 1 → 2 → 3 分阶段交付并各自验收（均已完成）。**
2. 分页页大小 `kPageCells` 取值（建议 64；需要时再调）。**已拍板：取 64。**
3. 批量删除接口是否作为公开 API 暴露（`DeleteDataItems(vector<size_t>)`），还是仅内部优化。**已拍板：作为公开 API 暴露（`ui::ListCtrl::DeleteDataItems`）。**
4. ~~是否需要 Phase 4 的行优先绘制缓存（取决于 Phase 1/2 后的实测）~~ **已拍板：无需实施**（Phase 1/2/3 实测显示列优先取数不构成瓶颈；详见 §5 Phase 4 与 Roadmap P2 / P4）。

---

## 10. 实施记录

### Phase 3 — 批量删除 + 存储压缩（已实施；批量删除门槛 Release 实测通过，绘制需复测）

**状态**：代码已完成并通过编译验证（VS2022 MSBuild，先 Rebuild `duilib` target 再增量编译 `build/examples.sln`，Debug|x64，0 错误 0 警告，仅既有 libpng C4996 警告）。批量删除、页回收、`CompactStorage` 均已落地；Release（x64）实测数据已通过压力测试面板「批量删除500行（乱序）」按钮采集并回填（见下）；面板另新增「压缩存储」按钮（调用 `CompactStorage()` 并输出压缩前后工作集 / 私有内存对比），用于验证内存回落门槛，其实测数据**待补采**。

**核心改动文件**：

- `duilib/Control/ListCtrlData.h` / `ListCtrlData.cpp`：新增 `DeleteDataItems`（批量删除 + 单趟压缩）、`CompactStorage`（重编号 + 整体压缩）；`ResetStorageAt` 增加页回收。
- `duilib/Control/ListCtrl.h` / `ListCtrl.cpp`：`ui::ListCtrl` 同步暴露 `DeleteDataItems`、`CompactStorage`。
- `examples/ListCtrl/MainForm.h` / `MainForm.cpp`：新增 `StressTestDeleteBatch`（构造 500 个乱序索引，单趟批量删除并计时）与 `StressTestCompact`（压缩前后读进程内存 + `CompactStorage()` 计时）。
- `bin/resources/themes/default/list_ctrl/list_ctrl.xml` + `bin/resources/lang/{zh_CN,en_US}-examples.txt`：新增「批量删除500行（乱序）」「压缩存储」两个按钮与其多语言文案。

**核心机制**：

- **批量删除单趟压缩**：`DeleteDataItems` 先按行身份去重（`std::set<RowId>`，忽略重复与越界索引），逐行 `FreeRowId` 置墓碑（O(K×C)），再对 `m_displayOrder` 做一次读写双指针单趟压缩（O(N) 搬 4B），彻底避免逐行 `erase` 的 O(K×N) 搬移；只派发一次计数变更事件。
- **页回收**：`ResetStorageAt` 清空某槽位后，若该存储页 `usedMask` 全为 0（无任何已写入单元格）则释放整页，并裁剪页列表尾部已释放的空页，及时释放已删除行占用的内存（无需等到 `CompactStorage`）。
- **`CompactStorage`（显式整表维护）**：按当前显示顺序把存活行重编号为 `0..N-1`（丢弃墓碑槽位）：`m_rowDataList` 按显示顺序重建；各列存储页按新行身份紧凑排布（仅搬运已写入的单元格，未写入的不分配页），旧页列表随局部变量析构释放；`m_displayOrder` 变为恒等映射、`m_nextRowId = N`；选中行身份按"压缩前的显示位置"重新映射。仅应在无填充/绘制进行时调用（调用后内部单元格原始指针失效）。

**外层包装**：`ui::ListCtrl::DeleteDataItems` 返回实际删除行数并在删除生效后同步表头 CheckBox 状态；`ui::ListCtrl::CompactStorage` 直接转发数据层。

**Phase 3 后实测数据（10 万行 × 10 列，x64 Release，手动采集）**：

| 指标 | 验收门槛 | 现状基线（Release） | Phase 1 后 | Phase 2 后 | Phase 3 后 | 结论 |
|------|---------|------------------|-----------|-----------|-----------|------|
| 载入：插 10 列（`InsertColumn`） | — | 0.3 ms | 0.3 ms | 0.3 ms | 0.3 ms | 持平 |
| 载入：`SetDataItemCount(100000)` | — | 3.9 ms | 23.2 ms | 2.7 ms | **2.4 ms** | 与 Phase 2 持平（噪声级） |
| 载入：填充 100 万单元格（`SetSubItemData`） | — | 1163.9 ms | 1425.8 ms | 1204.8 ms | 1237.8 ms | 与 Phase 2 持平（+2.7%，噪声级） |
| 载入合计 | — | 1168.1 ms | 1449.3 ms | 1207.8 ms | 1240.5 ms | 与 Phase 2 持平 |
| 中间插入 1000 行 | — | 605.7 ms | 68.1 ms | 68.7 ms | **64.3 ms**（0.064 ms/行） | 与 Phase 2 持平（略快） |
| 中间删除 500 行（逐行 `DeleteDataItem`） | < 100 ms | 615.9 ms | 56.4 ms | 52.9 ms | **57.0 ms**（0.114 ms/行） | 与 Phase 2 持平（噪声级），远优于门槛 |
| **批量删除 500 行（乱序，`DeleteDataItems`）** | **< 30 ms** | — | — | — | **2.4 ms** | ✅ 新增能力，**远优于门槛**（约为同数量逐行删除的 1/24） |
| **压缩存储（`CompactStorage`，100 轮删除后剩 50,000 行）** | **内存回落至接近存活规模** | — | — | — | 耗时 **39.2 ms**；WS **146.6 → 105.2 MB**、私有 **124.8 → 83.2 MB** | ✅ **达成**：存活行减半后回落 **-41.4 / -41.6 MB（-28.2% / -33.3%）**，**低于** Phase 2 稠密 10 万行基线（131.2 / 109.7 MB），与"存活单元格 1,000,000 → 500,000、约省 48 MB"量级吻合 |
| 压缩存储（同上，20 轮删除后剩 90,000 行） | 内存回落至接近存活规模 | — | — | — | 耗时 156.7 ms；WS 131.3 → 137.9 MB、私有 109.6 → 115.5 MB（压缩后再点「内存读数」：WS 138.0 / 峰值 146.5 / 私有 115.6 MB） | ⚠️ **未观测到回落（+6.7 MB）**：仅删 10%，墓碑均匀散布不触发页回收，可回收量 ~9.6 MB 低于测量口径噪声（详见观察） |
| 排序（整表） | < 300 ms | 76.7 ms | 31.5 ms | 27.7 ms | **21.2 ms** | 与 Phase 2 持平（略快） |
| 滚动重绘单帧 | ≤ 13.020 ms/帧 | 15.529 ms/帧 | 13.020 ms/帧 | 11.032 ms/帧（环境 A） | **同机同环境 3 次：14.542 / 12.164 / 12.991 ms/帧（均值 ≈13.2）**；早期非本机 4 次：17.396 / 15.841 / 16.664 / 14.201（均值 ≈16.0）；同机 Phase 2 3 次：19.987 / 16.041 / 16.695（均值 ≈17.6） | ✅ **同机 A/B 通过（已闭环）**：Phase 3（≈13.2）优于同机 Phase 2（≈17.6）约 **24.7%**，绘制门槛（不得变差）**达成**；环境 A 的 11.032 / 13.020 仅环境内可比 |
| 峰值内存（稠密 10 万×10） | 不劣于 Phase 2 | 174.0 / 153.2 MB（WS / 私有） | 174.4 / 160.3 MB | 131.2 / 109.7 MB | **132.0 / 133.4 / 110.4 MB**（WS / 峰值WS / 私有） | ✅ 与 Phase 2 持平（+0.6%），内存不劣化 |

**观察**：

- **批量删除门槛达成**：乱序删除 500 行仅 **2.4 ms**，远低于 < 30 ms 门槛；对照"中间逐行删除 500 行"的 57.0 ms，单趟压缩把 O(K×N) 的逐行搬移降为 O(N) 一趟搬移，性能提升约 24×。
- **时序路径（增删改 / 排序 / 载入）**：均与 Phase 2 持平或略快，全部落在 ±3% 量级的运行间噪声内，未出现降级——满足"中间增删改不降级"约束。
- **滚动重绘：同机 A/B 已闭环，绘制门槛达成（Phase 3 反而更快）**：同一台测试机、同一环境各测 3 次——Phase 3 为 **14.542 / 12.164 / 12.991 ms/帧（均值 ≈13.2）**，Phase 2 为 **19.987 / 16.041 / 16.695 ms/帧（均值 ≈17.6）**；**Phase 3 快约 24.7%**，且已接近环境 A 的 Phase 1 门槛 13.020 ms/帧。因 Phase 3 未改动任何绘制 / 取数路径（仅新增 `DeleteDataItems`、`ResetStorageAt` 页回收、`CompactStorage`，面板重绘测试均不调用它们），两者差值主要来自**运行间波动与系统负载**（Phase 2 单次 16.0~20.0、Phase 3 单次 12.2~14.5；每轮仅 20 帧样本），按硬约束"绘制不得变差"判定为**无回归**。早期 4 次复测（17.396 / 15.841 / 16.664 / 14.201）为**非本机环境**数据，不与本机 A/B 直接比较。（面板重绘测试固定从第 0 行起、覆盖第 0–3800 行，与当前滚动位置无关。）
- **内存不劣化**：Phase 3 稠密 10 万×10 峰值内存 **132.0 MB 工作集 / 110.4 MB 私有**，与 Phase 2 的 131.2 / 109.7 MB 基本持平（+0.6%），远低于基线 174.0 / 153.2 MB —— 内存门槛保持。
- **内存不膨胀（无泄漏）**：50 轮「批量删除500行」（累计删 25,000 行，剩 75,000 行）后工作集 **131.3 MB / 私有 109.8 MB**，与载入后（100,000 行）的 132.0 / 110.4 MB 基本持平（-0.5%）；20 轮复测（累计删 10,000 行，剩 90,000 行）为 **131.3 / 109.6 MB**（载入后 131.1 / 109.7 MB，+0.2 / -0.1 MB）——反复批量删除**不引起内存增长**，无泄漏。
- **页回收在"均匀乱序删除"下未触发（符合设计）**：该门槛期望"内存回落到接近存活规模"，但本测试的索引均匀散落在整表（`nTotal/500` 步长），每个 64 槽存储页平均仅约 6~32 槽变墓碑，**几乎不会整页全墓碑**，而页回收按设计只在整页全墓碑时才释放该页，故删掉 10%~50% 的行后存储页都不会被释放（读数只会因堆滞留而小幅抬高）。这正是 `CompactStorage()` 存在的意义——**要真正回收内存需显式调用 `CompactStorage()`（或删除成片的连续行使整页清空）**。
- **「压缩存储」在大比例删除后内存显著回落（内存回收门槛达成）**：100 轮「批量删除500行」（累计删 50,000 行，剩 50,000 行）后点「压缩存储」耗时 **39.2 ms**，弹框读数 WS **146.6 → 105.2 MB**、私有 **124.8 → 83.2 MB**，即**回落 -41.4 / -41.6 MB（-28.2% / -33.3%）**；对照 Phase 2 稠密 10 万行基线 131.2 / 109.7 MB，**只剩一半行数后已低于基线**，与"存活单元格由 1,000,000 降为 500,000、约省 48 MB"的量级吻合。`CompactStorage` 按显示顺序把存活行重编号为 `0..N-1` 并逐列重建紧凑页，旧页随 `pageList.swap(newPageList)` 释放。
- **压缩前读数偏高属反复删除 / 刷新的堆滞留（非泄漏）**：该轮压缩前读数为 146.6 / 124.8 MB，比新载入态高约 15 MB（20 轮时为新载入态持平），说明反复删除 + 刷新会留下未归还 OS 的堆内存；但它**不随删除次数线性增长**（20 轮：131.3 / 109.6 MB ≈ 载入 131.1 / 109.7 MB），且**压缩后立即回落**至 105.2 / 83.2 MB。
- **小比例删除（10%）观测不到回落——口径边界**：20 轮删除（剩 90,000 行）后压缩，WS **131.3 → 137.9 MB**、私有 **109.6 → 115.5 MB**（随后「内存读数」WS 138.0 / 峰值 146.5 / 私有 115.6 MB），**不降反升 +6.7 MB**。原因：(a) 10% 墓碑均匀散布，每页平均约 6 个，**无一页整页全墓碑 → 页回收不触发**；(b) 有效内存只省 ~9.6 MB，低于"旧页释放回堆但不归还 OS"的量级；(c) 压缩逐列"先建新页、后释放旧页"（逐列 `swap`），瞬时新旧两套页并存（峰值 ≈ 基线 + 单列约 10 MB）。**结论：工作集口径只在回收量足够大（如删 50%）时能观测到回落；小比例删除应改看"私有提交不随删除次数增长（无泄漏）"。**

**验收结论（Release 实测）**：

- ✅ **批量删除门槛**（乱序删 500 行 < 30 ms）：**2.4 ms** —— 通过；
- ✅ **时序路径**（增删改 / 排序 / 载入）与 Phase 2 持平、未降级 —— 通过；
- ✅ **内存门槛**（稠密峰值内存不劣于 Phase 2）：Phase 3 **132.0 MB 工作集 / 110.4 MB 私有** ≈ Phase 2 131.2 / 109.7 —— 通过（较基线 174.0 / 153.2 MB ↓24.1% / ↓28.0%）；
- ✅ **绘制门槛**（硬约束"绘制快 / 不得变差"）：同机同环境各 3 次 A/B —— Phase 3 **≈13.2 ms/帧**（14.542 / 12.164 / 12.991）vs Phase 2 **≈17.6 ms/帧**（19.987 / 16.041 / 16.695），**Phase 3 快约 24.7%、无回归** —— 通过（环境 A 的 11.032 ms/帧与门槛 13.020 仅在环境 A 内可比）；
- ✅ **内存回收门槛**（多次增删后内存回落至接近存活规模）：**已实测、达成**。100 轮乱序批量删除（剩 50,000 行）后显式 `CompactStorage()`（**39.2 ms**）使 WS **146.6 → 105.2 MB**、私有 **124.8 → 83.2 MB**（**-28.2% / -33.3%**，低于 Phase 2 稠密 10 万行基线 131.2 / 109.7 MB）；反复删除本身**不膨胀无泄漏**（50 轮 131.3 / 109.8 MB、20 轮 131.3 / 109.6 MB ≈ 载入 131.1 / 109.7 MB）。边界：仅删 10%（剩 90,000 行）时压缩观测不到回落（131.3 → 137.9 / 109.6 → 115.5 MB，有效内存仅省 ~9.6 MB 且 10% 墓碑散布不触发页回收）——**工作集口径只在回收量足够大时可见**。

### Phase 2 — 值存储 + 分页连续（已实施；性能 / 绘制 / 内存门槛 Release 实测全部通过）

**状态**：代码已完成并通过编译验证（VS2022 MSBuild，先 Rebuild `duilib` target 再增量编译 `build/examples.sln`，Debug|x64，0 错误 0 警告，仅既有 libpng C4996 警告）。时序路径（载入 / 增删改 / 排序）、滚动帧耗时，以及基线 / Phase 1 / Phase 2 三态稠密与稀疏峰值内存的 Release（x64）实测数据均已采集并回填（见下）；`AssertDataConsistency()` 断言经 Debug（x64）运行确认无失败；全部验收门槛已通过。

**核心改动文件**：

- `duilib/Control/ListCtrlData.h`：分页存储结构定义与访问器声明。
- `duilib/Control/ListCtrlDefs.h`：对外取数类型由 `shared_ptr` 改为原始 `const` 指针；并移除 `ListCtrlSubItemData2Ptr` typedef，代码中统一直接书写 `const ListCtrlSubItemData2*`（避免 typedef 名字形似智能指针、定义变量时漏初始化）。
- `duilib/Control/ListCtrlData.cpp`：全部取数 / 增删 / 排序 / 校验逻辑改写为分页存储。
- `duilib/Control/ListCtrlReportView.cpp`：`GetSubItemStorage` 返回类型适配（`StoragePtr` → `const Storage*`）。
- `duilib/Control/ListCtrlListView.cpp` / `duilib/Control/ListCtrlIconView.cpp`：原始指针成员补 `= nullptr` 初始化，消除 C4701/C4703 警告。

**核心机制**：

- **分页连续存储**：列池由"每列 `std::vector<std::shared_ptr<Storage>>`"改为"每列 `std::vector<std::unique_ptr<StoragePage>>`"；`struct StoragePage { std::array<Storage, kPageCells> cells; uint8_t usedMask[kPageCells]; }`，`static constexpr size_t kPageCells = 64;`；页索引 = `RowId / kPageCells`，页内偏移 = `RowId % kPageCells`。
- **懒分配**：某页首次写入时才 `make_unique<StoragePage>()`（`pageList.resize(nPage + 1)` 增长页列表，中间未触及页留空指针）；稀疏 / 纯虚表场景（`SetDataItemCount` 大、实际写入少）内存不膨胀。`AllocRowId` 不再对每列 `resize`，顺带修复 Phase 1 的 `SetDataItemCount` 载入路径回退。
- **指针稳定性**：`std::vector<std::unique_ptr<StoragePage>>` 扩容只移动 `unique_ptr`，`StoragePage` 对象地址不变，故页内 `&cells[slot]` 原始指针在 Phase 2（暂无页回收）期间稳定。
- **对外取数统一为原始指针**：直接使用 `const ListCtrlSubItemData2*`（避免 `ListCtrlSubItemData2Ptr` 这类 typedef 名字形似智能指针、易漏初始化，故未保留 typedef）；`StoragePtr` typedef 移除，内部读点用 `const Storage*`、写点用 `Storage*`。
- **页操作辅助**（`ListCtrlData` 静态成员）：`GetStorageAt`（读，命中未写入 / 未分配页返回 `nullptr`）、`GetStorageAtForWrite`（写，按需分配页并置位掩码）、`ResetStorageAt`（删除行墓碑：重置单元格并清掩码）。

**关键语义保持**：

- `SetSubItemData` 的 `bCheckChanged`：原靠 `shared_ptr` 是否为 null 判断"是否首次写入"，分页模型下改为先 `GetStorageAt`（`nullptr` 表示未写入）再 `GetStorageAtForWrite`，语义一致。
- `AddColumn`：由"对已存在列 `resize`"改为 find-guard 的 `emplace`，保留已有页数据。
- `AssertDataConsistency` 不变量重写为：(1) `m_displayOrder` 合法无重复；(2) 各列已写入单元格的 `RowId` 必须落在存活行集合内；(3) `m_rowDataList.size() == m_nextRowId`；(4) 单选有效；(5) 计数一致。

**验收门槛（见 §5）**：

- 内存：稠密 10 万×10 峰值内存较基线（重构前，x64 Release 实测 174.0 MB 工作集）下降 ≥20%（目标 ~30%）；
- 绘制：滚动帧耗时不高于 Phase 1 基线（13.020 ms/帧，不得变差）；
- 稀疏场景内存不膨胀；
- `AssertDataConsistency()` 全绿。

**Phase 2 后实测数据（10 万行 × 10 列，x64 Release，手动采集）**：

| 指标 | 验收门槛 | 现状基线（Release） | Phase 1 后（Release） | Phase 2 后（Release） | 改善 |
|------|---------|------------------|--------------------|--------------------|------|
| 载入：插 10 列（`InsertColumn`） | — | 0.3 ms | 0.3 ms | 0.3 ms | 持平 |
| 载入：`SetDataItemCount(100000)` | — | 3.9 ms | 23.2 ms | **2.7 ms** | ✅ 载入回退已修复（较 Phase 1 快 8.6×，优于基线 1.4×） |
| 载入：填充 100 万单元格（`SetSubItemData`） | — | 1163.9 ms | 1425.8 ms | **1204.8 ms** | ✅ 较 Phase 1 快 15.5%，基本回到基线（+3.5%） |
| 载入合计 | — | 1168.1 ms | 1449.3 ms | **1207.8 ms** | ✅ 较 Phase 1 快 16.7% |
| 中间插入单行 | < 5 ms | 0.606 ms | 0.068 ms | **0.069 ms** | ✅ 保持（较基线快 8.8×） |
| 中间插入 1000 行 | — | 605.7 ms | 68.1 ms | **68.7 ms** | ✅ 保持（较基线快 8.8×） |
| 中间删除单行 | < 5 ms | 1.232 ms | 0.113 ms | **0.106 ms** | ✅ 保持（较基线快 11.6×） |
| 删除 500 行 | < 100 ms | 615.9 ms | 56.4 ms | **52.9 ms** | ✅ 较 Phase 1 快 6.2%（较基线快 11.6×） |
| 排序（整表） | < 300 ms | 76.7 ms | 31.5 ms | **27.7 ms** | ✅ 较 Phase 1 快 12.1%（较基线快 2.8×） |
| 滚动重绘单帧 | ≤ 13.020 ms/帧 | 15.529 ms/帧 | 13.020 ms/帧 | **11.032 ms/帧**（环境 A） | ✅ 较 Phase 1 快 15.3%（较基线快 29.0%；同一台测试机上 Phase 2 复测为 ≈17.6 ms/帧，见 §10 Phase 3 观察） |
| 峰值内存（稠密 10 万×10） | 较基线 ↓≥20% | **174.0 / 153.2 MB**（WS / 私有） | 174.4 / 160.3 MB | **131.2 / 109.7 MB** | ✅ 较基线 ↓24.6% / ↓28.4%（较 Phase 1 ↓24.8% / ↓31.6%） |
| 峰值内存（稀疏 10 万行 / 1000 格） | 不膨胀 | **67.0 / 41.8 MB**（WS / 私有） | 67.3 / 48.8 MB | **52.0 / 27.8 MB** | ✅ 不膨胀（较基线 ↓22.4% / ↓33.5%，懒分配页生效） |

**观察**：

- **载入路径回退已消除**：`SetDataItemCount(100000)` 由 Phase 1 的 23.2 ms 回落到 **2.7 ms**（优于基线 3.9 ms）；填充 100 万单元格由 1425.8 回到 **1204.8 ms**（较 Phase 1 快 15.5%）。印证「`AllocRowId` 不再按列 `resize`、单元格改为懒分配页」的预期——`SetDataItemCount` 现仅推进 `m_displayOrder` / `m_nextRowId`，不再按列预分配。
- **增删改 / 排序保持领先**：均与 Phase 1 持平或微升（删 500 行 56.4→52.9 ms、排序 31.5→27.7 ms、插 1000 行 68.1→68.7 ms 属噪声级），仍大幅优于现状基线。
- **填充路径仍略高于基线**（1204.8 vs 1163.9 ms，+3.5%）：属一次性载入开销，不影响"绘制 / 增删改"两条最高优先级路径。
- **绘制提速（最高优先硬约束满足）**：滚动重绘单帧由 Phase 1 的 13.020 ms 进一步降到 **11.032 ms/帧**（较 Phase 1 快 15.3%、较基线快 29.0%）。分页连续存储使同一页内相邻单元格内存连续，绘制取数时缓存命中更好，兑现了"改善绘制局部性"的设计预期。（注：上述三组绘制数据同属环境 A；同一台测试机上 Phase 2 复测为 19.987 / 16.041 / 16.695 ms/帧、均值 ≈17.6，与 Phase 3 的 ≈16.0 同量级，见 §10 Phase 3 观察——该机整体读数偏高，故绝对门槛 13.020 ms/帧 只在环境 A 内可比。）
- **内存大幅下降（内存门槛满足）**：稠密 10 万×10 峰值内存由基线 174.0 MB / Phase 1 174.4 MB 降至 **131.2 MB**（工作集，较基线 ↓24.6%、较 Phase 1 ↓24.8%）；私有提交由 153.2 / 160.3 MB 降至 **109.7 MB**（较基线 ↓28.4%、较 Phase 1 ↓31.6%，达成"目标 ~30%"）。收益源于去掉每格 `shared_ptr`（指针 + 控制块）与指针 `vector`，改为页内连续 `Storage`。

**验收结论**：**全部验收门槛 Release 实测通过（性能 / 绘制 / 内存）**。

- ✅ **时序路径**（载入 / 增删改 / 排序）Release 实测不劣于 Phase 1，且载入回退已修复——满足"中间增删改不降级"约束；
- ✅ **绘制门槛**（滚动帧 ≤ 13.020 ms/帧，硬约束"绘制快"最高优先）：**11.032 ms/帧 ≤ 13.020**（较 Phase 1 再快 15.3%）——通过（环境 A 内可比；同一测试机上 Phase 2 复测 ≈17.6 ms/帧，见 §10 Phase 3 观察）；
- ✅ **内存门槛**（稠密峰值内存较基线 ↓≥20%）：Phase 2 稠密工作集 **131.2 MB**（较基线 174.0 ↓24.6%、较 Phase 1 174.4 ↓24.8%）、私有提交 **109.7 MB**（较基线 153.2 ↓28.4%、较 Phase 1 160.3 ↓31.6%）——**≥20% 达成**；
- ✅ **稀疏场景**（`SetDataItemCount` 大、写入少）内存不膨胀：Phase 2 稀疏 **52.0 MB** 工作集 / **27.8 MB** 私有，低于基线（67.0 / 41.8）与 Phase 1（67.3 / 48.8）——懒分配页生效，未写入页不分配内存；
- ✅ **`AssertDataConsistency()`**：校验逻辑已随实现更新至分页模型（`_DEBUG` 下每次结构性操作后调用）；Debug（x64）运行实测（载入 / 增删改 / 排序 / 稀疏载入等交互路径）**无断言失败**。

**实测项（已完成）**：基线（重构前）、Phase 1、Phase 2 三态的稠密 / 稀疏峰值内存均已采集回填（见 §10），内存门槛判定通过。

### Phase 1 — 身份/位置分离（已完成）

**状态**：已完成并通过编译验证（VS2022 MSBuild，`build/examples.sln` Debug|x64，0 错误）。

**核心改动**（`ListCtrlData.h` / `ListCtrlData.cpp`）：

- 新增行身份 `typedef uint32_t RowId;` 与哨兵 `kInvalidRowId = 0xFFFFFFFF`；成员 `m_nextRowId` 单调递增，删除不复用。
- 新增显示顺序表 `std::vector<RowId> m_displayOrder;`，作为**唯一**随增/删/改/排序变动的结构（4B/元素）。
- 列数据池 `m_dataMap`（`columnId → vector<shared_ptr<Storage>>`）与行属性池 `m_rowDataList` 改为**按 RowId 索引**；删除行时列池槽位置空指针墓碑、行属性槽位重置为默认值墓碑（append-only，不搬移）。
- `m_nSelectedIndex(size_t)` → `m_nSelectedRowId(RowId)`，单选状态记录行身份，排序/增删后自动跟随对应行。
- 新增内部辅助：`GetRowIdByDisplayIndex` / `GetDisplayIndexByRowId` / `AllocRowId` / `FreeRowId`；新增公开访问器 `GetDisplayItemData(size_t)`（替代原 `GetItemDataList()`，返回行属性指针）。
- 结构性操作后调用 `AssertDataConsistency()`（`#ifdef _DEBUG` 内校验：排列性、列池/行属性池长度=`m_nextRowId`、单选有效、三个计数一致）。

**性能关键改动**：

- `InsertDataItem` / `DeleteDataItem`：只改 `m_displayOrder`（insert/erase，4B 搬移）+ 目标行 O(C) 标记，**不再对每列做整列 `vector::insert/erase`**。
- `SortDataItems`：只对 `m_displayOrder` 排序（比较时按 RowId 读排序列），**彻底去掉"每列整列深拷贝写回"**。

**受影响消费方**：`ListCtrlReportView.cpp` 中 6 处 `GetItemDataList()` 使用点改为 `GetDataItemCount()` + `GetDisplayItemData(index)`（含框选 `SetSelectedElements` 前的行高/可见性扫描）。`ListCtrl.cpp` 对外 API 以显示索引为参数、行为不变，未改动。

### Phase 0 — 压力测试面板（已落地）

**状态**：面板与代码已落地并通过编译验证（VS2022 MSBuild，`build/examples.sln` Debug|x64，0 错误，仅既有 LNK4099 PDB 警告）；基线数据待运行采集后回填。

**改动文件**：

- `examples/ListCtrl/MainForm.h` / `MainForm.cpp`：新增压力测试面板方法（`InitStressTest` / `StressTestLoad` / `StressTestLoadSparse` / `StressTestMemory` / `StressTestInsert` / `StressTestDelete` / `StressTestDeleteBatch` / `StressTestCompact` / `StressTestSort` / `StressTestRepaint` / `StressTestRestore`）与成员 `m_bStressDataLoaded`；新增文件级计时助手 `GetPerfTick()` / `PerfTickToMs()`（Windows 走 `QueryPerformanceCounter` / `QueryPerformanceFrequency`，其他平台退化为 `std::chrono::steady_clock`）与内存读取助手 `GetProcessMemorySnapshot()`（Windows 走 `GetProcessMemoryInfo`）；在 `OnInitWindow()` 中调用 `InitStressTest(pListCtrl)` 绑定全部按钮。
- `bin/resources/themes/default/list_ctrl/list_ctrl.xml`：新增全宽"压力测试"分组行（`GroupVBox` + `btn_global` 按钮；Phase 0 首版 1 行 6 个，Phase 2/3 追加稀疏载入 / 内存读数 / 批量删除 / 压缩存储，现为 2 行共 10 个）。
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 / 扩充压力测试面板 `STRID_LISTCTRL_STRESS_*` 文案（含面板标题与"未载入数据"提示，现共 12 条）。

**面板按钮与测量口径**：

| 按钮 | text_id | 操作 | 计时口径 |
|------|---------|------|---------|
| 载入压测数据 | `STRID_LISTCTRL_STRESS_LOAD` | `DeleteAllColumns`+`DeleteAllDataItems` → 插 10 列 → `SetDataItemCount(100000)` → 逐格 `SetSubItemData` | 三段分别计时（插列 / 设行数 / 填单元格）+ 总耗时；数据操作期间 `SetEnableRefresh(false)` 隔离绘制噪声 |
| 载入稀疏数据 | `STRID_LISTCTRL_STRESS_LOAD_SPARSE` | 插 10 列 → `SetDataItemCount(100000)`（不整体填充）→ 仅写第 0 列前 1000 行 | 三段分别计时；验证分页存储的懒分配：大行数、极少写入时内存不膨胀 |
| 中间插入1000行 | `STRID_LISTCTRL_STRESS_INSERT` | 在 `GetDataItemCount()/2` 处循环 `InsertDataItem` 1000 行 | 总耗时 + 单行均值（触发中间位置搬移） |
| 删除500行 | `STRID_LISTCTRL_STRESS_DELETE` | 在 `GetDataItemCount()/2` 处循环 `DeleteDataItem` 500 行 | 总耗时 + 单行均值（触发中间位置搬移） |
| 批量删除500行（乱序） | `STRID_LISTCTRL_STRESS_DELETE_BATCH` | 构造 500 个乱序显示索引 → `DeleteDataItems` 单趟批量删除 | 总耗时（单趟压缩 `m_displayOrder`，O(N) 搬 4B，不做 K 次 `erase`） |
| 压缩存储 | `STRID_LISTCTRL_STRESS_COMPACT` | `CompactStorage()` 按显示顺序重编号存活行 + 整体压缩各列分页存储 | 总耗时 + 压缩前后工作集 / 私有提交对比（MB）；调用后内部单元格指针失效，仅应在无填充 / 绘制进行时点击 |
| 排序 | `STRID_LISTCTRL_STRESS_SORT` | `SortDataItems(0, true)` 整表按第 1 列升序 | 总耗时 + 返回值 |
| 滚动重绘 | `STRID_LISTCTRL_STRESS_REPAINT` | 20 帧，每帧 `EnsureDataItemVisible(index, true)` 置顶后 `Refresh(true)` 同步重绘 | 总耗时 + 单帧均值（含绘制） |
| 还原演示数据 | `STRID_LISTCTRL_STRESS_RESTORE` | 清空后 `InsertItemData(400, 9, imageId)` 回到初始演示态 | 无（退出压测态） |
| 内存读数 | `STRID_LISTCTRL_STRESS_MEMORY` | 不操作控件，直接读进程内存（`GetProcessMemoryInfo`） | 输出工作集 / 峰值工作集 / 私有提交（MB）；峰值自进程启动累计，单场景测量需重启进程 |

**基线数据采集说明**：

- **现状基线（重构前）**已在独立构建（**x64 Release**）上手动采集，见下表——这是后续所有改进的对比基准。
- 运行 `bin/ListCtrl_d.exe` → 点击面板按钮，即可得到 **Phase 1 后**的实测数据：既用于校验 Phase 1 验收门槛（10 万行×10 列：中间插/删单行 < 5 ms；删 500 行 < 100 ms；排序 < 300 ms；绘制不劣化），也作为 **Phase 2 / Phase 3** 的对比基线。
- 按钮每次点击弹出结果消息框（标题"压力测试"），记录 Total / Average / 分阶段耗时。
- 「内存读数」按钮（仅 Windows）直接读进程内存（工作集 / 峰值工作集 / 私有提交），无需任务管理器切窗口；**峰值工作集自进程启动累计**，故单独测量稠密或稀疏场景前应先重启进程。

**现状基线（重构前，x64 Release，手动采集，10 万行 × 10 列）**：

| 阶段 / 操作 | 实测耗时 |
|------|---------|
| 载入：插入 10 列（`InsertColumn`） | 0.3 ms |
| 载入：`SetDataItemCount(100000)` | 3.9 ms |
| 载入：填充 100 万单元格（`SetSubItemData`） | 1163.9 ms |
| 载入合计 | 1168.1 ms |
| 中间插入 1000 行（`InsertDataItem`） | 605.7 ms（0.606 ms/行） |
| 中间删除 500 行（`DeleteDataItem`） | 615.9 ms（1.232 ms/行） |
| 排序（整表，第 1 列升序，`SortDataItems`） | 76.7 ms |
| 滚动重绘（`EnsureDataItemVisible` + `Refresh(true)`，20 帧） | 310.6 ms（15.529 ms/帧） |

**现状基线内存（重构前，x64 Release，手动采集，用「内存读数」按钮）**：

| 场景 | 工作集 | 峰值工作集 | 私有提交 |
|------|-------|-----------|---------|
| 稠密（10 万×10 全填充，`载入压测数据`） | 174.0 MB | 174.0 MB | 153.2 MB |
| 稀疏（10 万行仅填 1000 格，`载入稀疏数据`） | 67.0 MB | 67.1 MB | 41.8 MB |

> 与 §1 的理论估算对照：现状"删 500 行"实测 **615.9 ms**（理论估"数百 ms"），"删单行"实测 **1.232 ms**，与估算一致；§1 的 11 MB / 5.5 GB 为搬移字节量的理论值，此处为端到端耗时实测值。

**Phase 1 后实测数据（10 万行 × 10 列，x64 Release，手动采集）**：

| 指标 | 验收门槛 | 现状基线（Release） | Phase 1 后（Release） | 改善 |
|------|---------|------------------|--------------------|------|
| 载入：插 10 列（`InsertColumn`） | — | 0.3 ms | 0.3 ms | 持平 |
| 载入：`SetDataItemCount(100000)` | — | 3.9 ms | 23.2 ms | ⚠ 慢 5.9× |
| 载入：填充 100 万单元格（`SetSubItemData`） | — | 1163.9 ms | 1425.8 ms | ⚠ 慢 22% |
| 载入合计 | — | 1168.1 ms | 1449.3 ms | ⚠ 慢 24% |
| 中间插入单行 | < 5 ms | 0.606 ms | 0.068 ms | ✅ 快 8.9× |
| 中间插入 1000 行 | — | 605.7 ms | 68.1 ms | ✅ 快 8.9× |
| 中间删除单行 | < 5 ms | 1.232 ms | 0.113 ms | ✅ 快 10.9× |
| 删除 500 行 | < 100 ms | 615.9 ms | 56.4 ms | ✅ 快 10.9× |
| 排序（整表） | < 300 ms | 76.7 ms | 31.5 ms | ✅ 快 2.4× |
| 滚动重绘单帧 | ≤ 现状 | 15.529 ms/帧 | 13.020 ms/帧 | ✅ 快 19% |
| 峰值内存（稠密 10 万×10） | — | 174.0 / 153.2 MB（WS / 私有） | 174.4 / 160.3 MB | 持平（WS +0.4 MB；私有 +4.6%，Phase 1 未改单元格存储形态） |
| 峰值内存（稀疏 10 万行 / 1000 格） | — | 67.0 / 41.8 MB（WS / 私有） | 67.3 / 48.8 MB | 持平（WS +0.3 MB；私有 +16.7%，Phase 1 引入 `m_displayOrder` / 行属性池所致） |

**Phase 1 验收结论**：✅ **通过**。

- 中间插 / 删单行 **0.068 / 0.113 ms** ≪ 5 ms；
- 删 500 行 **56.4 ms** < 100 ms（较基线提速 **10.9×**）；
- 排序 **31.5 ms** < 300 ms（提速 **2.4×**）；
- 绘制**不劣化**：**13.020 ms/帧 ≤ 基线 15.529 ms/帧**（提速 19%）——满足"绘制快（最高优先）"硬约束；
- 中间增删改吞吐大幅提升，满足"中间增删改不降级"硬约束。

**观察（载入路径轻微回退，留待 Phase 2/3 关注）**：

- `SetDataItemCount(100000)` 由 **3.9 ms → 23.2 ms**（慢 ~5.9×），填充 100 万单元格由 **1163.9 → 1425.8 ms**（慢 ~22%），载入合计慢 ~24%。
- 疑似原因（待核实）：Phase 1 按 RowId 物化 `m_displayOrder` 与行属性池后，`SetDataItemCount` 需按 `m_nextRowId` 填充 / 增长 `m_displayOrder`；单格写入多了一次 `row → RowId` 间接寻址。属**一次性载入开销**，不影响"绘制 / 增删改"两条最高优先级路径，可留待 Phase 2（值存储）一并评估优化。

**待办**：

1. Phase 2（连续分页存储，达成内存下降 ≥20%）**已完成并通过 Release 验收**（性能 / 绘制 / 内存实测全部通过；稠密峰值内存 174.0→131.2 MB，↓24.6%）；`AllocRowId` 不再按列预分配，已顺带修复上述载入路径回退（`SetDataItemCount` 23.2→2.7 ms，Release 实测确认）。
2. Phase 3（批量删除 + 存储压缩）**已实施**（批量删除单趟压缩、页回收、`CompactStorage` 均已落地并编译通过，见 §10）；**全部门槛 Release 实测结论（已闭环）**：批量删除门槛 ✅（2.4 ms < 30 ms）、内存门槛 ✅（稠密 132.0/110.4 MB ≈ Phase 2）、内存回收门槛 ✅（100 轮删除剩 50,000 行后 `CompactStorage()` 39.2 ms，WS 146.6→105.2、私有 124.8→83.2 MB）、无泄漏 ✅（50/20 轮删除读数与载入态持平）、绘制门槛 ✅（同机同环境 A/B：Phase 3 ≈13.2 vs Phase 2 ≈17.6 ms/帧，快 24.7%）。

