[简体中文](ListCtrl-Roadmap.md) | English

## ListCtrl Optimization Roadmap

This document tracks the optimization and enhancement items for the `ListCtrl` control, so each item can be followed up and verified individually. For the control-level plan, see [Control-Roadmap.en.md](Control-Roadmap.en.md); for the full properties / APIs / examples of ListCtrl, see [Control.en.md](Control.en.md#listctrl-控件继承-vbox-属性).

Scope: `ui::ListCtrl` and its companion classes `ui::ListCtrlReportView`, `ui::ListCtrlHeader`, `ui::ListCtrlHeaderItem`, `ui::ListCtrlItem`, `ui::ListCtrlSubItem`, `ui::ListCtrlData` (covering the Report / List / Icon views).

Status legend: **Done** | **Todo** (confirmed missing, awaiting implementation) | **To evaluate** (candidate optimization, cost/benefit to be confirmed) | **To verify** (current conclusion is uncertain, needs validation first).

### 1. Completed optimization items

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
| Column show/hide | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl-level column visibility API; the data column is synchronized internally |
| Column layout events | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | Dispatch `kEventColumnWidthChanged` / `kEventColumnOrderChanged` / `kEventColumnVisibleChanged` / `kEventColumnSorted` respectively, letting the app persist column width, order, visibility and sort state |

Demo: `examples/ListCtrl` (first column frozen by default, column 2 as a progress bar, column 3 as rich text; includes a live single-cell refresh progress demo, an interactive column show/hide test item, and a single-line / word-wrap toggle test item).

#### 1.3 Cell word wrap (Report view)

| Capability | API | Description |
|-----------|-----|-------------|
| Cell single/multi-line (word wrap) | `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` | Controls whether a Report cell's text is single-line (`true`, default, ellipsis on overflow) or multi-line (`false`, word wrap by the cell's width); the row height must be enlarged by the caller via `SetDataItemHeight`, otherwise the overflow is clipped |

Implementation note: the Report view is virtual, so its cell controls are reused while scrolling; therefore the single/multi-line property is stored in the data model (the `TEXT_SINGLELINE` bit of `nTextFormat`) and applied by the framework **when filling the cell data** onto the reused control (reusing the existing per-cell text-format path, with no new struct field). Wrapping in the List / Icon views is controlled by their own `SetTextSingleLine`; this API is Report-only (in non-Report views `Set*` returns `false` and `Is*` returns `true`).

### 2. Existing capabilities (not added this round; briefly misjudged as missing)

| Capability | Implementation | Description |
|-----------|----------------|-------------|
| Keyboard navigation | `VirtualListBox::OnListCtrlKeyDown` (multi-select) / `ListBox::OnListBoxKeyDown` (single-select) | Arrow keys, `Home` / `End`, `PageUp` / `PageDown` move the selection; `Shift` + arrow keys select a range; `Ctrl+A` selects all. With no item selected, arrow keys fall back to scrolling (intended, not a defect) |
| Cell tooltip (auto-show on overflow) | The `list_ctrl_sub_item` Class in the theme `global.xml` sets `auto_tooltip="true"`; logic in `LabelImpl::CheckShowToolTip` / `LabelImpl::GetAutoToolTipText` | When the text does not fit the cell's available width, the full content is shown automatically. Note the XML attribute is `auto_tooltip` (not `auto_show_tooltip`) |
| Variable row height | `ListCtrl::SetDataItemHeight` (default row height) / `SetDataItemHeight(itemIndex, ...)` (per-row height) | Supports a uniform row height and per-row heights; the existing demo already showcases it |
| Text ellipsis | The default Label text style includes `TEXT_END_ELLIPSIS` | When the text exceeds the cell's available width it is automatically truncated with an ellipsis; the existing demo already showcases it |

### 3. Todo list (confirmed missing)

| # | Item | Status quo | Implementation notes |
|---|------|-----------|----------------------|
| L1 | F2 to enter cell editing | Missing: `ListCtrl` currently handles only `kVK_ESCAPE` (leave editing), no `kVK_F2` | Handle `kVK_F2` in the `ListCtrl` key branch to enter editing for the current cell; must be mutually exclusive with the existing `m_pRichEdit` editing state |
| L2 | Ctrl+C to copy the selected content to the clipboard | Missing: the framework already provides `Clipboard::SetClipboardText`, but `ListCtrl` does not call it | Copy the current selected cell's text; can be extended to copy a whole row / multiple rows in TSV format (for pasting into Excel); must be mutually exclusive with the editing state |

### 4. Candidate optimizations (to evaluate / to verify)

| # | Item | Status | Description |
|---|------|--------|-------------|
| P1 | Heap-allocation overhead of cell storage | To evaluate | In the data model `ListCtrlData`, cell storage uses `std::shared_ptr` (`StoragePtr` / `StoragePtrList` / `StorageMap`), i.e. each cell is a separate heap allocation; at large scale this leads to noticeable memory use and allocation overhead. Consider a compact or pooled storage approach |
| P2 | Traversal overhead of column storage vs. row-major access | To evaluate | Data is organized by column (column ID to a row list) while painting / filling accesses it row-major, requiring cross-column gathering and extra traversal. Consider a row-major cache |
| P3 | Maximum horizontal scroll range with frozen columns | To verify | Whether the maximum horizontal scroll range already subtracts the frozen-column width (to avoid the last column being covered by the frozen columns at the end of the scroll); verify first before deciding whether to adjust |

### 5. Tracking overview

| ID | Item | Status |
|----|------|--------|
| — | Frozen columns / cell progress bar / cell rich text | Done |
| — | Single-cell refresh / column show-hide / column layout events | Done |
| — | Keyboard navigation (arrows / Home / End / PageUp / PageDown) | Existing |
| — | Cell tooltip (auto-show on overflow) | Existing |
| — | Variable row height / text ellipsis | Existing |
| L1 | F2 to enter cell editing | Todo |
| L2 | Ctrl+C to copy to clipboard | Todo |
| L3 | Cell word wrap (Report view) | Done |
| P1 | Heap-allocation overhead of cell storage | To evaluate |
| P2 | Column storage vs. row-major access overhead | To evaluate |
| P3 | Horizontal scroll range with frozen columns | To verify |

> Note: this document is the dedicated follow-up list for ListCtrl. Please update the status table as items are completed; control-level new plans still follow [Control-Roadmap.en.md](Control-Roadmap.en.md).
