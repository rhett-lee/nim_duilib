English | [简体中文](Control-Roadmap.md)

> Last synced: 2026-09-29

## Control Library Capability Inventory and New Control Roadmap

This document is based on a full scan of the existing code (`duilib/Control`, `duilib/Box`,
`duilib/Layout`, `duilib/Utils`, and the control type definitions in `duilib/duilib_defs.h`),
compared against the common control lists of mainstream desktop UI frameworks
(Qt, WPF, WinUI, Flutter Desktop). It summarizes current capabilities, identifies gaps,
and proposes a tiered plan for new controls, with the goal of enabling this UI library
to serve desktop software development across platforms well.

### 1. Current Capability Inventory

| Category | Existing Controls |
|------|---------|
| Text | Label, RichText (HTML-like formatted text), HyperLink |
| Buttons / Selection | Button, Option (radio), CheckBox |
| Input | RichEdit (single/multi-line rich text editing), IPAddress, HotKey, DateTime (date-time picker) |
| Dropdowns | Combo, ComboButton, FilterCombo (filterable), CheckCombo (multi-select) |
| Progress | Progress, CircleProgress, Slider |
| List / Table | ListBox (horizontal/vertical/tile + Virtual series), ListCtrl (Report/ListView/IconView views, header, check box column, sorting, grouping, cell editing) |
| Trees | TreeView, DirectoryTree |
| Tab Controls | TabBox (container-style), TabCtrl (browser-style multi-tab) |
| Menus | Menu/MenuItem/SubMenu/MenuBar |
| Colors | Full ColorPicker family (Regular/Standard/StandardGray/Custom) plus ColorSlider, ColorControl |
| Containers / Layout | HBox/VBox/ScrollBox/TileBox/GridBox/FlowBox/Panel (titled collapsible panel with accordion grouping), SplitBox + 13 layout engines (including virtual layouts) |
| Misc Controls | Split, GroupBox, Line, ScrollBar, PropertyGrid, AddressBar, ChildWindow, IconControl, BitmapControl |
| Drag Enhancements | ControlDragable/HBoxDragable/VBoxDragable (drag to reorder children), BoxMovable, BoxResizable |
| Web Embedding | CefControl, WebView2Control |
| Window Utilities | ToastWnd (non-modal toast), MessageBoxWnd (modal message box), Tooltip, FileDialog, TrayIcon, Clipboard, ScreenCapture |
| Infrastructure | SVG (nanosvg), GIF/WebP/APNG/Lottie/PAG animations, light/dark themes, multi-language, DPI scaling, file drag-and-drop, animation system, SDL cross-platform backend (Linux/MacOS/FreeBSD) |

The heavyweight controls (lists, tables, trees, virtual lists, property grid, color picker,
web embedding) are all in place with complete coverage. Compared with mainstream frameworks,
the gaps concentrate in **modern interaction controls** and the **flyout/popup system**.

### 2. Proposed New Controls

Grouped into three tiers by value and implementation cost; implement in tier order.

#### Tier 1: Form Essentials, Low Cost

| # | Control | Current Status | Direction |
|---|------|------|------|
| 1 | RichEdit placeholder | **Already supported**: `prompt_mode` / `prompt_text` / `prompt_text_id` / `prompt_color` ([RichEdit_Windows.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit_Windows.h), [RichEdit2.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit2.h); cross-platform RichEdit2 also supports it). Demo: rich_edit.exe | No new code needed; docs only |
| 2 | Switch/Toggle | **Completed**: Added `Switch` control class (self-drawn track + thumb slide animation + color transition, semantic colors adapted to light/dark themes) | — |
| 3 | SpinBox | **Already supported**: RichEdit's `spin_class` attribute (`rich_edit_spin` / `rich_edit_spin_box/btn_up/btn_down`) plus `min_number/max_number/number_only/limit_text`. Demo: rich_edit.exe | Completed: extracted into SpinBox control class (supports `step`) |
| 4 | SearchBox | **Missing**: no dedicated control built on prompt_text + clear-button combination | Completed: added composite SearchBox control (left icon + edit box + clear button, supports Enter event) |
| 5 | Badge | Unread counts / red dots on TabCtrl tabs and buttons. Essential for IM and mail apps | Common across platforms |

#### Tier 2: Flyout Foundation, Medium Cost

| # | Control | Description | Reference |
|---|------|------|------|
| 6 | Flyout/Popup container (highest priority) | Today Menu can only be a menu and Tooltip is plain text only (system-native on Windows). A generic "floating card with arbitrary content" base is missing: anchor positioning, no focus stealing, auto-dismiss, themed drawing. It is the shared foundation for calendar popups, search suggestions, rich tooltips, and value bubbles | Qt Popup, WinUI Flyout |
| 7 | Calendar panel | The current DateTime relies on the system control (superclassed DTP, spinner-style); there is no custom-drawn month panel. Needed by schedule/report software; uses item 6 as its popup host | Qt QCalendarWidget, WPF DatePicker |
| 8 | NavigationView (sidebar) | Hamburger button + grouped nav items + header + content area. A staple of modern settings pages and desktop clients; TreeView+TabBox can emulate it but with poor cohesion | WinUI NavigationView |
| 9 | Toast action buttons | The current toast only closes when clicked as a whole; add an action-button area ("Undo" / "View details") to match Win32 notification capability. Small enhancement | Windows notification ActionButton |

#### Tier 3: Large Items / Standalone Modules, Scheduled as Needed

| # | Control | Description | Reference |
|---|------|------|------|
| 10 | Lightweight Chart | Line/bar/pie charts, custom-drawn with data binding. Charts currently require CEF embedding or a third-party library; the largest workload but the highest value for a "full toolkit", best evolved as a standalone module | Qt Charts (slimmed) |
| 11 | Timeline | Vertical timeline (nodes + content) for logs, task flows, approval flows. Relatively cheap to draw | Common across platforms |
| 12 | Skeleton / loading placeholder | ControlLoading (loading state) already exists; add list/card skeleton placeholder modes to improve async-loading UX | Common across platforms |

### 3. Explicitly Not Planned

* **Docking layout**: extremely costly (floating windows, drop indicators, layout persistence),
  hard to keep consistent across platforms, and unnecessary for most desktop apps.
* **MDI**: largely abandoned by modern desktop applications.
* **Heavy chart engines**: beyond the scope of a control library; the lightweight Chart in
  item 10 covers common scenarios.

### 4. Suggested Implementation Order

1. **Start with item 6, the Flyout container**: it is the shared dependency of several
   Tier 2/3 controls, and allows replacing the system-native Tooltip on Windows with a
   self-drawn one so popup styling stays consistent in dark theme.
2. Then finish Tier 1 items 1–5 (form essentials); item 4 depends on item 1.
3. Item 7 (Calendar) builds on Flyout; items 8 and 9 can proceed in parallel.
4. Schedule Tier 3 by product needs; it does not block the first two tiers.

> Note: this is a planning document. Concrete designs (interfaces, XML attributes, skin
> resources) will be produced when each item starts.
