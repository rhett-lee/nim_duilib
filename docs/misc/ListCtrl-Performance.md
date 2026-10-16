# ListCtrl 性能优化方案

> 适用范围：nim_duilib 的 ListCtrl（Report 视图）滚动绘制性能
> 基线环境：Debug|x64，400 行 × 9 列，冻结首列，窗口 1400×900，`mouse_event(MOUSEEVENTF_WHEEL)` 驱动滚动
> 状态：**P0-1「消除整表重填」已实施并实测验收（已完成）**；P0-2 已实测证伪、不实施；P0-3 与滚动复用方案待拍板

---

## 1. 已确认的事实（决定缓存做在哪一层）

| 事实 | 证据 |
|---|---|
| ListCtrl 单元格**不走 RichText 路径** | 单元格类 `list_ctrl_sub_item`（`bin/resources/themes/default/global.xml` L725）**未设置** `rich_text`，故 `LabelImpl::m_bRichText` 保持默认 `false` |
| 单元格文本走**纯文本**分支，**全程无整形缓存** | `duilib/Control/TextDrawer.cpp`（L98-140）的 `else` 分支直接 `pRender->DrawString(...)`；`duilib/RenderSkia/Render_Skia.cpp`（L1542-1684）每次从零构建 `SkTextBox` 并 `Draw` |
| 既有 `DrawRichTextCache` **完全用不上** | `duilib/RenderSkia/DrawRichText.cpp`（L719 `IsValidDrawRichTextCache`）的缓存只在 RichText 分支生效 |
| 冻结列使**整行子树被绘制两趟** | `duilib/Control/ListCtrlReportView.cpp`（L832-944 `PaintChild`）：`bHasFrozen` 时对每个行容器 `paintControl(false)` + `paintControl(true)` |
| 单元格类为 `text_align="vcenter,left"` → **每帧额外一次全量整行扫描** | `duilib/RenderSkia/SkTextBox.cpp`（L122-253）：`spacingAlign != kStart` 时先调 `CountLines`（对整段文本做一遍完整 `Linebreak`），再逐行 `Linebreak`；垂直居中正是这条路径 |

**结论**：整形缓存必须建在 `Render_Skia`（`SkTextBox` 之上）或 `TextDrawer` 薄封装层，**不能复用** RichText 的缓存。

---

## 2. 单元格文本整形缓存：完整失效因子清单

先把「失效」拆成两个层次，避免把不该失效的因素也拉进来（这是当前设计最容易做错的地方）：

- **Tier A｜整形缓存**：文本 → 每行的字节区间 + 字形（glyphID / advance / 回退字体）。只与「怎么把字符变成字形并计算宽度」有关。
- **Tier B｜排版/绘制缓存**：在 Tier A 之上，计算每行 X/Y 与行数（对外可直接「提交绘制」）。与「放在哪里、画几行、什么颜色」有关。

### 2.1 必须使 Tier A（整形）失效的因素

| # | 因素 | 说明 |
|---|---|---|
| 1 | **文本内容**（字节序列/字符数） | 含语言切换（`text_id` 解析结果变化）、`trim_policy` 去空白后的结果 |
| 2 | **字体**（家族名、字号、粗体、斜体） | 字号需按**物理像素**比较；DPI 换算后不同即失效 |
| 3 | **字体回退**（回退字体管理器 / 系统字体集变化） | 某字符命中哪个回退字体，直接决定 glyph 与宽度 |
| 4 | **可用宽度**（换行边界 = 文本矩形宽 − padding − checkbox/icon 占位） | 列宽变化、控件复用时都要重算 |
| 5 | **换行模式** `kOneLine / kWordBreak / kCharBreak` | 由 `single_line` / `word_wrap` 决定；模式变化会完全改变断行点 |
| 6 | **省略号模式**（`TEXT_END_ELLIPSIS` / `TEXT_PATH_ELLIPSIS`）+ 省略目标宽度 | 省略号会**替换/截断实际绘制的字符**，属于整形结果变化 |
| 7 | **字间距 `fWordSpacing` / 两端对齐 `TEXT_HJUSTIFY`** | 走的是另一套 `HorizontalDrawText` 实现，必须整体失效 |
| 8 | **纵向文本 `TEXT_VERTICAL`** | 走 `VerticalDrawText`，另一套实现，必须整体失效 |
| 9 | **显示缩放因子 / DPI 变化** | 通过 #2 生效；但需像 `RichTextImpl::CheckParseText`（`duilib/Control/RichTextImpl.cpp` L451）那样把 `DisplayScaleFactor` **显式列入失效条件**，否则缩放后字形陈旧 |

### 2.2 只使 Tier B（排版/绘制）失效、**不应**让 Tier A 失效的因素

| # | 因素 | 说明 |
|---|---|---|
| 10 | **行间距 `spacingMul/spacingAdd`** | 只改行基线 Y，不改字形 |
| 11 | **对齐方式**（水平左/中/右、垂直上/中/下） | 只改 X/Y 偏移；注意 `vcenter/bottom` 会额外触发一次 `CountLines` 全量扫描（可一并纳入 Tier B 缓存省掉） |
| 12 | **可用高度（框高）** | 决定能画几行、最后一行判定、垂直对齐位移 |
| 13 | **文本颜色 / 背景色** | 仅影响 `SkPaint`，与字形无关 |
| 14 | **透明度 fade / alpha** | 同上 |
| 15 | **控件位置（left/top，含滚动偏移）** | **只平移**，绝不能使缓存失效（对齐 `DrawRichText.cpp` 的做法：只比矩形**尺寸**，不比位置） |
| 16 | **是否裁剪 `TEXT_NOCLIP` / 裁剪框** | 仅绘制期参数 |
| 17 | **下划线 / 删除线**（字体装饰） | 绘制期画线，不改字形宽度 |

### 2.3 小结

「文本、字体、可用宽度」之外，**换行模式、省略号模式（含目标宽）、字间距/两端对齐、纵向/横向文本、DPI 缩放因子、字体回退** 都会影响整形结果；而 **行间距、对齐、可用高度、颜色/透明度、位置、裁剪、下划线** 只影响排版与绘制，**不应**触发重排。**位置变化绝不能让缓存失效**——这是最容易犯的错。

---

## 3. 优化方案（分阶段）

### Phase 0｜基线（已完成）

Debug|x64 实测稳态滚动：`frameMs` 13.4–20.9ms，其中 `itemPaintMs` ≈98%、`textPaintMs` ≈70%（of item）、`drawStrMs` ≈50%（of item）；`arrangeMs/fillMs` 稳定滚动恒为 0。

结论：

- 稳态滚动的瓶颈在**逐单元格文本绘制**，填充/排布不是滚动慢的原因。
- 但**整表重填单帧可达 3.4s**（偶发整表重排帧：`arrangeMs` 3789ms / 22 次、`fillMs` 3426ms / 555 行、`fillSubMs` 501ms / 4995 格；启动帧 `fillMs` 366.7ms / 46 行、`arrangeMs` 389.6ms），属**卡顿级**问题，列为**重点优化项**（见 P0-1）。

> **对「3.4s」的正确解读**（已由 P0-1 实施阶段复核）：该 3.4s **不是单次 `FillElementData` 的耗时**，而是**同一帧内连续多轮冗余排布的累计**——`arrangeMs 3789ms / 22 次`、`fillMs 3426ms / 555 行`（≈ 22 轮 × 25 行可见行），即每轮 ~155–172ms，一帧内叠了 ~22 轮。P0-1 消除的正是这 22 轮中「元素索引未变」的冗余重填。

### Phase 1｜P0（收益最大，先做）

**P0-1 消除「整表重填」（重点项）—— 已实施并验收**

实施结果（`duilib/Control/ListCtrlReportView.cpp` / `.h`，Debug|x64）：
- **落地内容（方案第 1 项「按需填充」）**：`ListCtrlReportLayout::LazyArrangeChild` 与 `LazyArrangeChildNormal` 中，在填充前先用 `IListBoxItem::GetElementIndex()` 与目标 `nElementIndex` 比较，**相同则跳过** `VirtualListBox::FillElementData` 及 `refreshDataList` / `OnRefreshElements` 记录（仅元素索引变化才填充）。
- **绑定失效机制（配套，保证正确性）**：新增 `ListCtrlReportView::InvalidateRowElementIndexes()`（把所有数据行的元素索引置 `Box::InvalidIndex`），在 `Refresh(bool)`（结构性刷新：排序 / 增删行 / 行高 / 置顶行 / 隐藏行等）与 `OnThemeChanged()` 中调用，强制下次布局对全量行重填；`ListCtrl::ChangeDpiScale` 末尾走 `Refresh()`，故 DPI 变化已覆盖。
- **实测收益（400 行 × 9 列，冻结首列，1400×900）**：
  - 启动阶段冗余排布：修复前 6 次排布 ≈ **1035ms**（其中 5 次为 `fill=0 same=23~24` 的纯冗余，各 ~145–165ms）；修复后同场景 ≈ **493ms**，冗余排布单次从 ~150ms 降到 **5–16ms**。
  - 一帧内叠 ~22 轮冗余重填的「卡顿级」帧（原 ~3.4s 量级）→ 冗余轮次成本归零，仅剩余「真正变化行」的填充。
  - 滚动热路径无回归：`fill=18~21 / same=1`（滚动时 1 行重叠复用被跳过）；**滚动期间 0 次触达失效机制**（`Invalidate` 仅出现在启动与增行阶段），证明失效机制不在滚动热路径生效。
  - 正确性：启动帧与修复前基线像素比对，**表格区域完全一致**（差异仅落在底部信息条，与本次改动无关）；排序 / 加行等结构性刷新后可见行按新顺序全量重填、无陈旧数据。
- **未纳入本次（方案第 2/3/4 项，仍待办）**：`defaultSubItem` 模板复用与类解析缓存、`SetStateTextColor` 属性写入去重、子项控件复用。滚动「翻页」时内容确实每 23~25 行全变（`same=1`），故滚动热路径的框架侧成本未被本次直接削减；实测单次排布 ~110–135ms 中约 **62% 为测试程序在填充事件里逐格拼字符串**（`sendEvt`），属应用侧成本。
- 回滚判据（未触发）：出现「行内容与数据不一致 / 复用后残留旧单元格属性 / 选择状态错乱」→ 回退。

- 现象（实测）：偶发整表重排单帧 `arrangeMs` 3789ms / 22 次、`fillMs` 3426ms / 555 行、`fillSubMs` 501ms / 4995 格；启动帧 `fillMs` 366.7ms / 46 行、`arrangeMs` 389.6ms。属卡顿级问题。
- 根因：
  1. `ListCtrlReportLayout::LazyArrangeChild`（`duilib/Control/ListCtrlReportView.cpp` L2596 处 `TODO: 优化代码，避免每次刷新都Fill`）在每次排布时对范围内的**每个**行控件**无条件**调用 `VirtualListBox::FillElementData(pControl, nElementIndex)`，即使该控件当前已绑定同一个元素索引 → 整表重填。
  2. `ListCtrlReportView::FillDataItem`（L1521-1522、L1526-1600）**每行**新建 `ListCtrlSubItem defaultSubItem` + `SetClass`（走主题类解析）；逐格写入 `SetText / SetTextStyle / SetTextPadding / SetBkColor / SetImageId / SetProgress`，且对 6 个 `ControlState` 逐一 `SetStateTextColor`（4995 格 × 6 ≈ 3 万次调用）；新列控件还需 `new` + `AddItem` + 挂 3 组事件回调。
- 改哪里：`ListCtrlReportLayout::LazyArrangeChild` + `ListCtrlReportView::FillDataItem` / `FillSubItemData`。
- 怎么改（按收益排序）：
  1. **按需填充（首要做）—— ✅ 已实施**：`LazyArrangeChild` 中先用 `IListBoxItem::GetElementIndex()` 与目标 `nElementIndex` 比较，**相同则跳过** `FillElementData` 与 `diplayItemIndexList` / refresh 记录（仅当索引变化才填充）。这能在「内容未变的重排」（排序 / 列宽 / 主题 / DPI / 滚动回边）下直接归零重填成本。（配套的失效机制见上方「实施结果」）
  2. **`defaultSubItem` 模板复用 + 类解析缓存**：把每列默认属性一次性解析为普通结构体（text style / padding / 各状态色 / bkcolor），不再每行构造临时控件 + `SetClass`。
  3. **属性写入去重**：`FillSubItemData` 对 6 个 `ControlState` 的 `SetStateTextColor` 改为「写入前比较、相同则跳过」或提供整组 setter。
  4. **子项控件复用**：行控件已存在且列数未变时，复用现有 `ListCtrlSubItem` 只更新数据，不 `new` / `AddItem` / 挂事件（列数变化才增删）。
- 预期（已达成）：整表重填单帧从 ~3.4s 量级降到「仅变化行」重填；稳态滚动 `fillMs` 保持 0。
- 验证（已完成）：用 QPC 插桩统计 `arrangeMs / fillMs / fillSubMs / defSubMs` 与 `FillElementData` 调用次数、命中跳过次数（日志 `fill=/same=/top=/last=` 序列），重跑启动 / 滚动 / 排序场景；插桩已在验收后整体撤销，Debug|x64 重编译无警告无错误。

**P0-2 消除冻结列双趟整行重绘 —— 已实测证伪，不实施**

- 前置测量（Debug|x64，1400×900，冻结 1 列，共 9 列，滚动 26 帧）：`visRows` 23–24、`visCells` 207–216、`expFrozen` 23–24、`expScroll` 184–192；实测 `ptFrozen` = 23–24 **恰等于** `expFrozen`，`ptScroll` = 92–96（= 4 列 × 行数，说明 9 列中另 4 列内容为空）。
- 结论：`GetPaintPass` 在每趟内已限制 `rcPaint`，叠加 `Control::AlphaPaint` 的矩形相交提前返回（`duilib/Core/Control.cpp` L3660 `if (!UiRect::Intersect(rcTemp, rcPaint, ...)) return;`），**每个单元格每帧只被绘制一次**，冻结趟只画冻结列 → 不存在重复整形 / 重复文本绘制。
- 第二趟的净开销仅为：每行对 8 个非冻结子项各做一次「提前返回」的 `AlphaPaint` 调用（≈24×8=192 次/帧，量级 ~10–20µs），约占 13ms 帧的 **~0.1%**。
- 处置：**不实施**。原「文本绘制调用数与整形次数约减半」的判断为误判；保留此记录，避免重复评估。

**P0-3 单元格文本整形缓存**

- 落点：`Render_Skia` / `SkTextBox` 层（纯文本路径），按 **Tier A / Tier B** 两级键设计，键集合严格按第 2 节表格。
- 关键收益点：① 同帧内冻结双趟复用；② 静止帧重复 `WM_PAINT` 复用；③ 稳定滚动时内容不变（`fillMs=0`）→ 可跨帧命中。
- **必须先验证命中率**：插桩统计滚动期间 cache hit/miss。若命中率低（虚拟列表复用控件导致内容每帧变化），则此项收益退化，需改用「按内容哈希的共享 LRU 缓存」而非「挂在控件上」。
- 附带：把 `vcenter/bottom` 触发的 `CountLines` 全量扫描结果一并缓存进 Tier B。

### Phase 2｜P1

- **P1-1** 热路径去开销：`PaintChild` 内 `std::vector<Control*> items = m_items;` 拷贝与 `std::find` 线性查找改为原地遍历（`ListCtrlReportView.cpp`，同 P0-2 处）。
- **P1-2** 进度条绘制（≈1.2ms/帧）。
- **P1-3** kCell 模式逐格选择/hover 判定按 RowId+列缓存（默认 kRow 下无收益，优先级最低）。

### Phase 3｜P2

- 绘制热路径 `dynamic_cast<ListCtrlSubItem*>`（每格一次）、`ASSERT`、STL 调试迭代器开销；该部分仅对 Debug 有明显收益，Release 视 P0 验证结果决定是否做。

### 统一验证方式

沿用已建立的 QPC 插桩（仅日志，不改业务逻辑）重跑同一场景，对比 `frameMs / itemPaintMs / textPaintMs / drawStrMs` 与调用次数；每阶段结束都**撤销插桩**并保证 Debug|x64 无警告无错误。

---

## 4. 待拍板事项

1. **方案范围**：P0-1 已完成；P0-2 已实测证伪、不实施；建议后续范围为 **P0-3（文本整形缓存）**，是否加 P1？（P2 建议先不做。）
2. **执行顺序**：P0-1 已完成，建议下一步做 **P0-3**；P0-3 是稳态滚动约 50% 帧开销的直接来源（实测每帧仅约 115 个单元格文本绘制即占 ~6ms），收益最确定。
3. **是否进一步优化「翻页滚动」热路径（需拍板，可能为破坏性变更）**：P0-1 只消除了「索引未变」的冗余重填；而滚动是「翻页」粒度（一次滚轮翻 ~23 行，`top` 步进 ~2~23，滚动时 `same=1`、其余 18~24 行内容全变），故滚动仍需重填全部可见行。若要继续压缩滚动成本，唯一途径是改为「控件↔元素复用 / 环形池」——滚动时仅重填**新进入视口**的行，而非全部可见行。**风险**：这会破坏「`m_items` 顺序 == 显示顺序」的不变量，并改变事件 `wParam`（`GetListBoxIndex()`）语义（应用把它当行索引使用），属破坏性变更，**需单独确认后再实施**。
