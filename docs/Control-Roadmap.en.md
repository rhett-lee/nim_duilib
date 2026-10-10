English | [简体中文](Control-Roadmap.md)

> Last synced: 2026-10-10 (roadmap merged: Tier-3 supplement controls, platform integration, engineering; Chart completed)

## Control Library Capability Inventory and New Control Roadmap

This document is based on a full scan of the existing code (`duilib/Control`, `duilib/Box`, `duilib/Layout`, `duilib/Utils`, and the control type definitions in `duilib/duilib_defs.h`), compared against the common control lists of mainstream desktop UI frameworks (Qt, WPF, WinUI, Flutter Desktop). It summarizes current capabilities, identifies gaps, and proposes a tiered plan for new controls, with the goal of enabling this UI library to serve desktop software development across platforms well.

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

The heavyweight controls (lists, tables, trees, virtual lists, property grid, color picker, web embedding) are all in place with complete coverage. Compared with mainstream frameworks, the gaps concentrate in **modern interaction controls** and the **flyout/popup system**.

### 2. Proposed New Controls

Grouped into three tiers by value and implementation cost; implement in tier order.

#### Tier 1: Form Essentials, Low Cost

| # | Control | Current Status | Direction |
|---|------|------|------|
| 1 | RichEdit placeholder | **Already supported**: `prompt_mode` / `prompt_text` / `prompt_text_id` / `prompt_color` ([RichEdit_Windows.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit_Windows.h), [RichEdit2.h](file:///c:/develop/nim_duilib/duilib/Control/RichEdit2.h); cross-platform RichEdit2 also supports it). Demo: rich_edit.exe | **Completed**: docs for placeholder attributes and examples written into the RichEdit docs |
| 2 | Switch/Toggle | **Already exists but static**: `checkbox_toggle_1/2` in [global.xml](file:///c:/develop/nim_duilib/bin/resources/themes/default/global.xml#L263-L268) uses CheckBox + two SVGs (off/on) with hard switching, no thumb animation | **Completed**: added the `Switch` control class (self-drawn track + thumb slide animation + color transition, semantic colors adapted to light/dark themes) |
| 3 | SpinBox | **Already supported**: RichEdit's `spin_class` attribute (`rich_edit_spin` / `rich_edit_spin_box/btn_up/btn_down`) plus `min_number/max_number/number_only/limit_text`. Demo: rich_edit.exe | Completed: extracted into the SpinBox control class (supports `step`) |
| 4 | SearchBox | **Missing**: no dedicated control built on prompt_text + clear-button combination | Completed: added the composite SearchBox control (left icon + edit box + clear button, supports Enter event) |
| 5 | Badge | Unread counts / red dots on TabCtrl tabs and buttons. Essential for IM and mail apps | **Completed**: added the `Badge` control class (count badge/dot mode, hidden automatically when count<=0, semantic colors adapted to light/dark themes) |

#### Tier 2: Flyout Foundation, Medium Cost

| # | Control | Description | Reference | Status |
|---|------|------|------|------|
| 6 | Flyout/Popup container (highest priority) | Today Menu can only be a menu and Tooltip is plain text only (system-native on Windows). A generic "floating card with arbitrary content" base is missing: anchor positioning, no focus stealing, auto-dismiss, themed drawing. It is the shared foundation for calendar popups, search suggestions, rich tooltips, and value bubbles | Qt Popup, WinUI Flyout | **Completed**: added the Flyout popup window class (8-way anchor placement with auto-flip, no focus stealing, click-outside/Esc auto-dismiss, semantic themed colors, single active flyout); integrated into the controls and ColorTheme demos |
| 7 | Calendar panel | The current DateTime relies on the system control (superclassed DTP, spinner-style); there is no custom-drawn month panel. Needed by schedule/report software; uses item 6 as its popup host | Qt QCalendarWidget, WPF DatePicker | **Completed**: added the Calendar custom-drawn month panel (single/range selection, month/year/decade navigation, today highlight, semantic themed colors) + CalendarFlyout popup wrapper; DateTime `date_calendar` edit format now uses CalendarFlyout; integrated into the controls and ColorTheme demos |
| 8 | NavigationView (sidebar) | Hamburger button + grouped nav items + header + content area. A staple of modern settings pages and desktop clients; TreeView+TabBox can emulate it but with poor cohesion | WinUI NavigationView | **Completed**: added NavigationView + NavigationViewItem controls (pane with hamburger button, pane title, scrollable nav item list and a bottom settings item; content area with page header + TabBox pages; items support item/header/separator forms, custom-drawn icon+text, selected gray pill with accent text, hover rounded highlight; hamburger toggles expanded/compact instantly, compact mode shows icons only with tooltips; non-item direct children are routed to the content TabBox as pages, switching by name updates the page header; the event API includes kEventNavigationItemClick (item invoked; also fires when clicking the already-selected item), kEventNavigationSelectionChanged (selection changed; carries new/old indices), kEventNavigationPaneToggling (before pane collapse/expand; return false to cancel), kEventNavigationPaneToggled (after pane collapse/expand completes); semantic colors adapt to light/dark themes, text_id supports localization). The controls example adds a navigation.xml demo window (11 nav items in 3 groups + header/separator/settings item + explicit TabBox pages, with dynamic add-item and programmatic selection demos) |
| 9 | Toast action buttons | The current toast only closes when clicked as a whole; add an action-button area ("Undo" / "View details") to match Win32 notification capability. Small enhancement | Windows notification ActionButton | **Completed**: added the `ToastAction` struct and a `Show` overload taking `std::vector<ToastAction>`; renders a row of rounded action buttons at the bottom of the toast. Clicking a button runs its callback then closes the toast; the buttons are `mouse_enabled=true` so they do not bubble to the root container (whole-bar click-to-close applies only to non-interactive areas). Layout and behavior are unchanged when no actions are supplied. The controls demo has a sample button |

#### Tier 3: Large Items / Standalone Modules, Scheduled as Needed

| # | Control | Description | Reference | Status |
|---|------|------|------|------|
| 10 | Lightweight Chart | Line/bar/pie charts, custom-drawn with data binding. Charts currently require CEF embedding or a third-party library; the largest workload but the highest value for a "full toolkit", best evolved as a standalone module | Qt Charts (slimmed) | **Completed**: added the Chart control (derives from Control, fully self-drawn, supports line/bar/pie forms, data/data_labels binding, axis/grid/legend/value labels, multi-series, negative bars, stacking, curve/area, donut pie; interaction includes hover tooltip/click-select/zoom-pan; visuals include rounded corners/gradient/glow/data animation; semantic colors adapt to light/dark themes). See [Control.md](Control.md#chart-attributes) |
| 11 | Timeline | Vertical timeline (nodes + content) for logs, task flows, approval flows. Relatively cheap to draw | Common across platforms | |
| 12 | Skeleton / loading placeholder | ControlLoading (loading state) already exists; add list/card skeleton placeholder modes to improve async-loading UX | Common across platforms | |

#### Tier 3 (supplement): Form and Table Enhancements, Low to Medium Cost

The items above are "large" ones. The following are smaller items that strengthen existing form/table capabilities and can proceed in parallel with Tier 3:

| # | Control | Description | Reference |
|---|------|------|------|
| 13 | TagInput | A "type → Enter to create tag → delete tag" composite control for recipients, tag selection, keyword entry. Essential for IM/collaboration/admin apps; reuses RichEdit + self-drawn tags | Element UI el-tag, Ant Design Tag |
| 14 | AutoComplete | A text box that shows suggestions as you type (search history, address completion), distinct from FilterCombo's "dropdown filtering" — the former is edit-box-centric with a popup following the caret. Can reuse Flyout as the popup host | Qt QCompleter, WPF AutoCompleteBox |
| 15 | Unified context-menu API | Menu already exists, but a "bind a right-click menu to any control in one line" convenience wrapper is missing (currently requires manual creation + popup + coordinate conversion) | Common across platforms |
| 16 | Mask / Overlay layer | Modal-dialog background dimming, onboarding masks, loading overlays. The last missing piece of the popup system (Flyout/Modal) | Common across platforms |
| 17 | ListCtrl table enhancements | Column width drag-resize, column freezing (fixed first column), header drag reorder, rich text / progress bar / icons in cells — frequent needs in report-style software | Qt QTableView, WPF DataGrid |

### 3. Explicitly Not Planned

- **Docking layout**: extremely costly (floating windows, drop indicators, layout persistence), hard to keep consistent across platforms, and unnecessary for most desktop apps.
- **MDI**: largely abandoned by modern desktop applications.
- **Heavy chart engines**: beyond the scope of a control library; the lightweight Chart in item 10 covers common scenarios.

### 4. Progress and Ordering

#### Completed (Tier 1–2 items 1–9, plus Tier 3 Chart item 10)

Tier 1 (form essentials 1–5), Tier 2 (flyout system 6–9), and the Tier 3 Chart (item 10) are all delivered: Switch, SpinBox, SearchBox, Badge, plus Flyout, Calendar, NavigationView, Toast action buttons, and Chart are implemented and integrated into the controls / ColorTheme demos. The shared flyout foundation (Flyout) is ready, so subsequent popup-style controls (AutoComplete, rich tooltips, etc.) can build directly on it.

#### Subsequent Ordering

1. **Finish Tier 3 supplement items 13–17** (low to medium cost, mostly composites/enhancements of existing controls): prioritize TagInput, AutoComplete (reusing Flyout), and ListCtrl table enhancements — the most direct gaps for form/report-style software.
2. **Schedule the remaining Tier 3 large items by product needs**: Timeline and skeleton screens can be inserted as needs arise.
3. **Platform integration and engineering (sections 5 and 6)**: run in parallel with the control roadmap; accessibility and packaging/signing are hard requirements for "cross-platform desktop development" and should start early.

### 5. Platform Integration Roadmap (Application Skeleton Layer)

Beyond the control layer, desktop software depends on a set of "system integration" capabilities. The library currently has only a few of these (ProcessSingleton, TrayIcon, Clipboard, FileDialog, ScreenCapture). The following are missing and are the key gaps for "reducing cross-platform desktop development cost" — proposed as a second evolution line parallel to the control layer:

| # | Capability | Current State | Description | Priority |
|---|-----------|---------------|-------------|----------|
| 18 | Accessibility / screen reader | **Missing** | Windows needs UI Automation, macOS needs NSAccessibility, Linux needs AT-SPI; a compliance requirement for enterprise/government software and a hard threshold for any cross-platform desktop library. Start with "accessible name/value/role annotation + Windows UI Automation adaptation", then cover other platforms gradually | High |
| 19 | Native system notifications | Toast is in-app and self-drawn | Integrate macOS Notification Center, Windows desktop notifications, and Linux (D-Bus/Freedesktop) native notifications, forming a dual channel (in-app + system-level) with the existing Toast | High |
| 20 | In-app update framework | **Missing** | Cross-platform self-update: Windows (custom or WinSparkle), macOS (Sparkle), Linux (package manager / AppImage) | Medium-High |
| 21 | File association / URL scheme / deep links | **Missing** | Register protocol → launch app → parse parameters; standard for "open with" and external invocation of desktop software | Medium |
| 22 | Auto-start + tray menu integration | TrayIcon exists | Add a tray right-click menu API and auto-start registration (login item / startup entry) | Medium |
| 23 | Install / package / sign / notarize | **Missing** | Platform installers (NSIS/MSIX, DMG, AppImage/deb/rpm), code signing, macOS notarization. This is the bulk of "desktop development cost"; a documented standard process is enough — no need to reinvent wheels | High (engineering) |

### 6. Developer Experience and Engineering

| # | Item | Description | Priority |
|---|------|------|----------|
| 24 | Visual layout / theme editor | XML is currently hand-written with no designer. This is the most direct lever for "reducing UI development cost", but the engineering effort is huge; propose it as a standalone long-term project outside the main control roadmap | Long-term strategy |
| 25 | Control unit-test infrastructure | No C++ test framework is currently integrated; add basic tests for control state machines (calendar, table, tree) to lower regression risk | Medium |
| 26 | Platform adaptation matrix doc | Support differences of each control across Windows/Linux/macOS/FreeBSD are scattered across docs; add a summary table clarifying "which controls are degraded/unimplemented on which platform" | Medium |

> Note: this is a planning document. Concrete designs (interfaces, XML attributes, skin resources) will be produced when each item starts.
