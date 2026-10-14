[简体中文](ListCtrl-Roadmap.md) | English

## ListCtrl Optimization Roadmap

This document tracks the optimization and enhancement items for the `ListCtrl` control, so each item can be followed up and verified individually. For the control-level plan, see [Control-Roadmap.en.md](Control-Roadmap.en.md); for the full properties / APIs / examples of ListCtrl, see [Control.en.md](Control.en.md#listctrl-控件继承-vbox-属性).

Scope: `ui::ListCtrl` and its companion classes `ui::ListCtrlReportView`, `ui::ListCtrlHeader`, `ui::ListCtrlHeaderItem`, `ui::ListCtrlItem`, `ui::ListCtrlSubItem`, `ui::ListCtrlData` (covering the Report / List / Icon views).

Status legend: **Done** | **Existing** (framework capability, not added this round; once briefly misjudged as missing) | **Todo** (confirmed missing, awaiting implementation) | **To evaluate** (candidate optimization, cost/benefit to be confirmed) | **To verify** (current conclusion is uncertain, needs validation first) | **Missing** (no counterpart yet compared with QTableView / WPF DataGrid).

> Note: besides the original items, this table adds the shortcomings raised by an external review, the features missing relative to QTableView / WPF DataGrid, and a tiered priority suggestion, as a per-item acceptance checklist.

### 1. Done / existing capabilities

#### 1.1 Table enhancements (Control-Roadmap item 17)

| Capability | API / XML | Description |
|-----------|-----------|-------------|
| Frozen columns | `SetFrozenColumnCount` / `GetFrozenColumnCount`, XML `frozen_column_count` | Pins the first N columns on the left; they stay fixed during horizontal scrolling. Implemented with a two-pass paint in the Report view; header, grid lines, and hit-testing are all adapted |
| Cell progress bar | `SetSubItemProgress(ById)` / `GetSubItemProgress(ById)` | Progress value in `[0,100]`, `-1` disables it; `progress_color` / `progress_bkcolor` set the bar and track colors, `progress_text_pos` controls the percentage text position (`center` / `left` / `right` / `outside`) |
| Cell rich text | `SetSubItemRichText(ById)` / `IsSubItemRichText(ById)` | Reuses the built-in Label rich-text rendering, supporting `<b>` / `<font>` / `<a>` tags |

#### 1.2 Interaction enhancements (Control-Roadmap item 18)

| Capability | API | Description |
|-----------|-----|-------------|
| Single-cell refresh | `RefreshSubItem` / `RefreshSubItemById` | Refreshes only the specified cell to avoid a full-row refresh. Report view only; returns `false` when the cell is outside the visible range (the data is already updated and will be shown once scrolled into view). Dispatches `kEventReportViewSubItemFilled` (with `pEventData` being that cell's `ListCtrlSubItem*`) so the app can set the cell content dynamically |
| Column show/hide | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl-level column visibility API; the data column is synchronized internally, so apps no longer need to bypass the wrapper to touch header sub-items |
| Column layout events | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | Dispatch `kEventColumnWidthChanged` / `kEventColumnOrderChanged` / `kEventColumnVisibleChanged` / `kEventColumnSorted` respectively, letting the app persist column width, order, visibility and sort state |

Demo: `examples/ListCtrl` (first column frozen by default, column 2 as a progress bar, column 3 as rich text; includes a live single-cell refresh progress demo, an interactive column show/hide test item, and a single-line / word-wrap toggle test item).

#### 1.3 Cell word wrap (Report view)

| Capability | API | Description |
|-----------|-----|-------------|
| Cell single/multi-line (word wrap) | `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` | Controls whether a Report cell's text is single-line (`true`, default, ellipsis on overflow) or multi-line (`false`, word wrap by the cell's width); the row height must be enlarged by the caller via `SetDataItemHeight`, otherwise the overflow is clipped |

Implementation note: the Report view is virtual, so its cell controls are reused while scrolling; therefore the single/multi-line property is stored in the data model (the `TEXT_SINGLELINE` bit of `nTextFormat`) and applied by the framework **when filling the cell data** onto the reused control (reusing the existing per-cell text-format path, with no new struct field). Wrapping in the List / Icon views is controlled by their own `SetTextSingleLine`; this API is Report-only (in non-Report views `Set*` returns `false` and `Is*` returns `true`).

#### 1.4 Column-level compare mode (sorting)

| Capability | API / flags | Description |
|-----------|-------------|-------------|
| Column-level compare mode | `SetColumnSortFlag(ById)` / `GetColumnSortFlag(ById)`, flags in `ListCtrlSubItemSortFlag` | Declares the comparison mode per column; clicking that column's header sorts by it automatically. Determined by the column's semantics, replacing the earlier "infer from the cell's `nProgress`" behavior |

Flags (`enum ListCtrlSubItemSortFlag`, bitwise-combinable): `kSortByGroup` (group by `nSortGroup` first), `kSortNoCase` (case-insensitive strings), `kSortByText` (default, lexicographic text), `kSortByTextAsNumber` (numeric text, `"10" > "9"`), `kSortByUserDataN` / `kSortByUserDataS` (by user data), `kSortByProgress` (by progress value `nProgress`).

Implementation note: the per-column flag is stored in `ListCtrl::m_columnSortFlagMap` (column ID -> `uint8_t`); both header-click sorting (`OnColumnSorted`) and programmatic sorting (`SortDataItemsById`) read/write it, so the app only declares it once. The `SortDataCompareFunc` inference branch ("sort by progress when any cell has `nProgress >= 0`") has been removed and replaced by the explicit `kSortByProgress` declaration.

Demo: in `examples/ListCtrl`, column 2 (the progress column) declares `kSortByProgress`, and column 4 holds pure numeric text and declares `kSortByTextAsNumber`; click the respective header to see progress / numeric sorting.

Todo (phase 2): the date mode `kSortByTextAsDate` is not yet implemented (it can reuse `duilib/Control/DateTime.h`).

> Note: only single-column sorting is supported, and it is a full-table sort with a full refresh (incremental / partial sorting is to be evaluated).

<!-- verify:allow-missing ListCtrlSubItemSortFlag -->

#### 1.5 Keyboard navigation (existing)

| Capability | Implementation | Description |
|-----------|----------------|-------------|
| Arrow keys / Home / End / PageUp / PageDown | `VirtualListBox::OnListCtrlKeyDown` (multi-select) / `ListBox::OnListBoxKeyDown` (single-select) | Arrow keys, `Home` / `End`, `PageUp` / `PageDown` move the selection; `Shift` + arrow keys select a range; `Ctrl+A` selects all. With no item selected, arrow keys fall back to scrolling (intended, not a defect) |

#### 1.6 Cell tooltip (existing)

| Capability | Implementation | Description |
|-----------|----------------|-------------|
| Cell tooltip (auto-show on overflow) | The `list_ctrl_sub_item` Class in the theme `global.xml` sets `auto_tooltip="true"`; logic in `LabelImpl::CheckShowToolTip` / `LabelImpl::GetAutoToolTipText` | When the text does not fit the cell's available width, the full content is shown automatically. Note the XML attribute is `auto_tooltip` (not `auto_show_tooltip`) |

#### 1.7 Variable row height / text ellipsis (existing)

| Capability | Implementation | Description |
|-----------|----------------|-------------|
| Variable row height | `ListCtrl::SetDataItemHeight` (default row height) / `SetDataItemHeight(itemIndex, ...)` (per-row height) | Supports a uniform row height and per-row heights; the existing demo already showcases it |
| Text ellipsis | The default Label text style includes `TEXT_END_ELLIPSIS` | When the text exceeds the cell's available width it is automatically truncated with an ellipsis; the existing demo already showcases it |

### 2. Todo list (confirmed missing)

| # | Item | Status quo | Implementation notes |
|---|------|-----------|----------------------|
| L1 | F2 to enter cell editing | Missing: `ListCtrl` currently handles only `kVK_ESCAPE` (leave editing), no `kVK_F2` | Handle `kVK_F2` in the `ListCtrl` key branch to enter editing for the current cell; must be mutually exclusive with the existing `m_pRichEdit` editing state |
| L2 | Ctrl+C to copy the selected content to the clipboard | Missing: the framework already provides `Clipboard::SetClipboardText`, but `ListCtrl` does not call it | Copy the current selected cell's text; can be extended to copy a whole row / multiple rows in TSV format (for pasting into Excel); must be mutually exclusive with the editing state. Can be further extended to "export CSV" |
| L3 | Row filtering / header filters | Missing (only the sort group `nSortGroup` exists; no condition-based row filtering) | Provide APIs to filter rows by condition + a header filter dropdown UI |
| L4 | Custom cell renderer | Missing (only the built-in text / icon / CheckBox / progress bar / rich text renderers) | Allow embedding an arbitrary child control (custom drawing) inside a cell |
| L5 | Cell-level editors (dropdown / date / numeric) | Missing (only one RichEdit editor) | Declare the editor type per column and create the matching input control on editing |

### 3. Candidate optimizations (to evaluate / to verify / architectural)

| # | Item | Status | Description |
|---|------|--------|-------------|
| P1 | Heap-allocation overhead of cell storage | In progress (Phase 1 and Phase 2 done and accepted in Release plus the Debug assertion verified; Phase 3 pending) | In the data model `ListCtrlData`, cell storage uses `std::shared_ptr` (`StoragePtr` / `StoragePtrList` / `StorageMap`), i.e. each cell is a separate heap allocation, actually ~130B+ per cell on x64; at large scale this leads to noticeable memory use, allocation overhead and fragmentation. Consider contiguous per-column storage (`std::vector<ListCtrlSubItemData2>`) or splitting sparse fields (progress / rich text) into an SoA layout, expected to save 30%+ of memory. The storage refactor has started (design: [ListCtrl存储重构设计方案.md](misc/ListCtrl存储重构设计方案.md)): **Phase 1 "identity/position split" is done** (introducing `RowId` + `m_displayOrder`, with the column/row-attribute pools indexed by RowId), removing the O(N×C) shifting on middle insert/delete and the full-column deep copy on sort; **Phase 2 "contiguous paged storage" done and accepted** (the column pool changed from `std::vector<std::shared_ptr<Storage>>` to `std::vector<std::unique_ptr<StoragePage>>` contiguous paged storage, page size 64, lazily allocated, and `Storage` usage points uniformly access by `(RowId → const Storage*)`; the code is complete and compiles, and all Release measurements passed and the Debug `AssertDataConsistency()` assertion reported no failure: `SetDataItemCount` regression fixed 23.2→2.7 ms, cell fill 1425.8→1204.8 ms, insert/delete/sort flat or slightly better, scroll repaint 13.020→11.032 ms/frame, dense peak memory 174.0→131.2 MB (↓24.6%), sparse 67.0→52.0 MB not inflated), delivering the memory reduction target above; **Phase 0 "stress test panel" delivered** (`examples/ListCtrl`, a 100k-row benchmark panel with "Load Sparse Data" and "Memory Readout" buttons, see design doc §10) to collect performance baselines and verify each phase's acceptance gates; **Phase 1 performance acceptance passed** (100k×10, x64 Release measured: delete 500 rows 615.9→56.4 ms, insert 1000 rows 605.7→68.1 ms, sort 76.7→31.5 ms, scroll repaint 15.529→13.020 ms/frame, painting not degraded; the load path regressed slightly ~24%, fixed by Phase 2) |
| P2 | Column-major storage vs. row-major access traversal overhead | To evaluate | Data is organized by column (column ID to a row list) while painting / filling accesses it row-major; `GetSubItemStorageList(itemIndex)` gathers across N columns and builds a temporary container, causing extra traversal. Consider a row-major cache or a row-major layout with a column index |
| P3 | Maximum horizontal scroll range with frozen columns | To verify | Whether the maximum horizontal scroll range already subtracts the frozen-column width (to avoid the last column being covered by the frozen columns / a blank strip at the end of the scroll). The horizontal extent calculation in `ListCtrlReportView` (`EstimateLayoutSize` / `CalcEstimateSize`) shows no explicit frozen-width deduction, so it needs hands-on verification |
| P4 | Re-arrange on every paint | To evaluate | `ListCtrlReportView::PaintChild` calls `ReArrangeChild(false)` on every paint, and in the frozen-column case each control is painted twice; re-arranging can be skipped when the size / data has not changed |
| P5 | Fragile patterns that depend on call order / valid `GetRect()` | Todo | `ListCtrlSubItem::EstimateText` once mixed absolute coordinates with widths, which was only correct by luck when "the control was not yet laid out" (fixed); it is advisable to review similar patterns that depend on call order / valid `GetRect()` |
| A1 | Converge frozen columns into a unified coordinate transform | To evaluate (architectural) | Hit-testing currently compensates by "zeroing the horizontal offset when inside the frozen region" and painting uses two-pass clipping, mixed with normal layout coordinates, making later changes error-prone (the `frozen=0` paint bug came from the same source). Suggest deriving painting / hit-testing / grid lines / scroll range from one coordinate-transform layer |
| A2 | Layout model upgrade (multi-level headers / cell merge / in-table tree / row group collapse) | Missing (architectural) | Requires an upgrade of the header and row layout model; see the feature comparison below |

### 4. Feature comparison with QTableView / WPF DataGrid

| Feature | Status | Description |
|---------|--------|-------------|
| Multi-level header / header cell merge | Missing | The header is a flat single row |
| Row drag reorder | Missing | Column-header drag reorder exists; row drag reorder does not |
| Cell merge | Missing | Spanning rows / columns |
| In-table tree (parent row expands child rows) | Missing | `TreeView` is a separate control, not merged with the table |
| Row group collapse | Missing | Only the sort group `nSortGroup` exists (rows still flat after grouping, no collapse) |
| Row filtering / header filters | Missing | See L3 |
| Summary row / footer row | Missing | No fixed summary / footer row |
| Copy to clipboard / export | Missing | See L2 |
| Keyboard navigation | Existing | Arrow keys / Home / End / PageUp / PageDown / Shift range select / Ctrl+A |
| Cell tooltip | Existing | `auto_tooltip="true"` by default, auto-show on overflow |
| Custom cell renderer (embed any control) | Missing | See L4; only built-in text / icon / CheckBox / progress bar / rich text for now |
| Variable row height / cell word wrap | Done | `SetDataItemHeight` + `SetSubItemSingleLine`; text ellipsis comes from the default Label style |
| Column width / order / visibility persistence | Done | App is notified via the `kEventColumn*` events and can persist and restore them |
| Cell-level editors (dropdown / date / numeric) | Missing | See L5; only one RichEdit editor for now |

### 5. Priority suggestion

Low / medium cost items (most done):

| # | Item | Status |
|---|------|--------|
| 1 | Single-cell refresh API `RefreshSubItem` | Done |
| 2 | External `kEvent*` events for column width / order / visibility / sort | Done |
| 3 | `ListCtrl`-level `SetColumnVisible` / `IsColumnVisible` | Done |
| 4 | Keyboard navigation (arrows + Home/End/PageUp/PageDown + F2/Ctrl+C) | Navigation done; F2 (L1) and Ctrl+C (L2) todo |
| 5 | Cell tooltip (auto-show on overflow) | Existing |
| 6 | Column-level compare mode (numeric / date / progress / custom) | Numeric / progress done; date (`kSortByTextAsDate`) in phase 2 |
| 7 | Variable row height + cell word wrap + ellipsis | Done |
| 8 | Row filtering (condition filter + header filters) | Todo (L3) |
| 9 | Copy to clipboard / export CSV | Todo (L2) |

Architectural items:

| # | Item | Status |
|---|------|--------|
| 10 | Cell storage to contiguous memory (drop the per-cell `shared_ptr`), also improving row-access locality | In progress (identity/position split Phase 1 done; contiguous paged storage Phase 2 done and accepted in Release plus the Debug assertion verified; Phase 3 pending) |
| 11 | Converge frozen columns from "two-pass paint + hit-test compensation" into a unified coordinate transform to avoid recurring bugs | To evaluate (A1; scroll-range scope see P3) |
| 12 | Multi-level header / row group collapse / in-table tree / cell merge (requires header and row layout model upgrade) | Missing (A2, architectural) |

### 6. Tracking overview

| ID | Item | Status |
|----|------|--------|
| — | Frozen columns / cell progress bar / cell rich text | Done |
| — | Single-cell refresh / column show-hide / column layout events | Done |
| — | Cell word wrap (Report view) | Done |
| — | Column-level compare mode (sorting, numeric / progress) | Done (date in phase 2) |
| — | Keyboard navigation (arrows / Home / End / PageUp / PageDown, Shift, Ctrl+A) | Existing |
| — | Cell tooltip (auto-show on overflow) | Existing |
| — | Variable row height / text ellipsis | Existing |
| — | Column width / order / visibility persistence (via events) | Done |
| — | Fixed / auto / stretch column width, width drag, header drag reorder, double-click width auto-fit | Existing |
| — | Report / List / Icon views (Report is virtual) | Existing |
| — | Full refresh `Refresh()` + per-row refresh `RefreshDataItems(vector)` | Existing |
| L1 | F2 to enter cell editing | Todo |
| L2 | Ctrl+C to copy to clipboard (multi-row TSV / export CSV) | Todo |
| L3 | Row filtering / header filters | Todo |
| L4 | Custom cell renderer | Todo |
| L5 | Cell-level editors (dropdown / date / numeric) | Todo |
| P1 | Cell storage heap-allocation overhead (contiguous memory / SoA) | In progress (Phase 1 and Phase 2 done and accepted in Release plus the Debug assertion verified; Phase 3 pending) |
| P2 | Column storage vs. row-major access overhead | To evaluate |
| P3 | Horizontal scroll range with frozen columns (frozen width deducted?) | To verify |
| P4 | Re-arrange on every paint (`ReArrangeChild`) | To evaluate |
| P5 | Fragile pattern review (call order / `GetRect()`) | Todo |
| A1 | Converge frozen columns into a unified coordinate transform | To evaluate (architectural) |
| A2 | Multi-level header / cell merge / in-table tree / row group collapse | Missing (architectural) |

> Note: this document is the dedicated follow-up list for ListCtrl. Please update the status table as items are completed; control-level new plans still follow [Control-Roadmap.en.md](Control-Roadmap.en.md).
