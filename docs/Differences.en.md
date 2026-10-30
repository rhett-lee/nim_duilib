English | [简体中文](Differences.md)

# Differences from Upstream duilib / NIM_Duilib_Framework

nim_duilib is developed on top of [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/), which in turn derives from the classic [duilib](https://github.com/duilib/duilib). Over time this project has systematically reworked the code structure, the rendering engine, the layout system, the control set and the example programs.

This document records those differences in full. It is intended for three purposes: estimating the amount of work needed to migrate from upstream; deciding whether a given capability comes from upstream or was added here; and confirming whether behaviour that differs from upstream documentation is an intentional change. The differences are grouped into overall improvements, feature enhancements, new controls/containers, performance optimizations, example programs, and documentation and licensing; the new controls/containers section lists 50+ items with a description for each.

## 1. Overall improvements (a dozen-plus)

These are architecture-level changes. They define how the code is organized and which dependencies exist, and they matter most when migrating.

| No. | Change |
| :--- | :--- |
| 1 | Reorganized the code by functional module; large files were split by class into smaller files, which makes the overall architecture easier to understand |
| 2 | Reviewed the interface headers and added comments for interfaces and functionality, which makes the code easier to read and understand |
| 3 | Optimized the configuration XML files: attribute naming rules were adjusted, control width and height can be set as percentages, image attributes gained some extensions, and the image loading flow was optimized |
| 4 | Extended image support: added APNG/WEBP animation, Lottie JSON animation and PAG animation, added ICO support, and optimized the image loading engine and its implementation |
| 5 | Reimplemented every layout and added common layout schemes, grouped into floating, horizontal, vertical, horizontal flow, vertical flow, grid, horizontal tile, vertical tile, and the virtual-list variants of horizontal, vertical, horizontal tile and vertical tile layouts; this makes the layout concept easier to understand and extend (see [Box.md](Box.en.md)) |
| 6 | Replaced the XML parser with the pugixml parser, which performs better |
| 7 | Removed the dependency on the base library; the message loop and thread communication are implemented in this project |
| 8 | Integrated Skia and made it the default rendering engine |
| 9 | Added SDL3 support for cross-platform windowing (Windows, Linux, macOS and FreeBSD are adapted) |
| 10 | Moved the CEF component into the duilib project and upgraded CEF: libcef 109 (Windows 7 compatible) and libcef 156 (Windows 10 and later) |
| 11 | Redesigned the image management interfaces and loading flow (Image directory) to support multi-threaded image loading and make it easier to add more image formats |
| 12 | Redesigned the XML structure of the global resource manager (global.xml), improved font management with font fallback for emoji, and added light/dark themes plus accent colors |

## 2. Feature enhancements (twenty-plus)

These improve the usability of existing windows, containers and controls. Many of them change the behaviour or attributes of controls that also exist upstream, so they should be checked one by one when migrating.

| No. | Enhancement |
| :--- | :--- |
| 1 | Improved DPI awareness and window message handling in the `Window` class, increased code robustness, and adjusted the code structure |
| 2 | Added `Window` attributes: `use_system_caption`, `snap_layout_menu`, `sys_menu`, `sys_menu_rect` and `icon`; the window can use the operating system title bar or a custom-drawn title bar that behaves like the system one |
| 3 | Added the `WindowDropTarget` helper class to `Window`, providing window-based drag-and-drop support |
| 4 | Reviewed resource management so that fonts, colors and images are easier to understand |
| 5 | Optimized `ListBox` and split it into ListBox, VListBox, HListBox, VTileListBox, HTileListBox, VirtualListBox, VirtualVListBox, VirtualHListBox, VirtualVTileListBox and VirtualHTileListBox; the virtual-list variants became considerably more usable |
| 6 | Optimized the controls associated with `Combo` (CheckCombo, FilterCombo) for better usability |
| 7 | Improved editing in the `DateTime` control |
| 8 | Reworked the menu class (`CMenuWnd`): the menu is reimplemented on the new structure and the controls inside it are fully compatible with the existing container/control system |
| 9 | Optimized and extended the `RichEdit` control with many commonly used features |
| 10 | Optimized and extended the `TreeView` control with many commonly used features and better usability |
| 11 | Optimized the `GlobalManager` interfaces so that all resources are managed through one place |
| 12 | Controls inside different containers of the same window can be dragged out of and into containers through attributes |
| 13 | Control background color supports gradients, and a foreground color feature was added |
| 14 | Improved multi-language support, including dynamic language switching, with the [examples/MultiLang](../examples/MultiLang) example |
| 15 | Improved DPI awareness: Unaware, SystemAware, PerMonitorAware and PerMonitorAware_V2 modes, independent DPI settings and high-DPI adaptation, with the [examples/DpiAware](../examples/DpiAware) example |
| 16 | Removed the ui_components project; the CEF component code was reworked and merged into the duilib project, and the rest was deleted |
| 17 | Improved window shadows: shadows use SVG images, a `shadow_type` attribute was added, and both custom-drawn and system shadows are supported (see [Window.md](Window.en.md)) |
| 18 | Added support for APNG/SVG/WEBP/ICO/LOTTIE/PAG image formats |
| 19 | Redesigned the control loading feature: loading is shown by a Box container configured through XML (including animation images), and interaction with those images is supported |
| 20 | Strengthened the `Label` control: added "justify" text alignment, vertical text (top to bottom, right to left), and configurable line spacing and letter spacing |
| 21 | `Control` supports full-screen display (through the new `Window::SetFullscreenControl`); CEF and WebView2 controls support F11 page full-screen toggling |
| 22 | Improved control animation details and introduced easing functions; easing type, total duration and playback interval are configurable |
| 23 | Unified context-menu API: `Control::AttachPopupMenu` binds a context menu to any control in one line (menu creation, coordinate conversion, item callbacks, and dynamic adjustment while the menu is open); `DetachPopupMenu` unbinds it |

## 3. New controls/containers (50+ items)

None of the following 50+ items exist in NIM_Duilib_Framework, so they are the most direct capability gain when migrating. Attributes, events and usage are documented in [Control.md](Control.en.md).

| No. | Name | Description |
| :--- | :--- | :--- |
| 1 | GroupBox | Group container |
| 2 | HotKey | Hotkey control |
| 3 | HyperLink | Text with a hyperlink |
| 4 | IPAddress | IP address control |
| 5 | Line | Line-drawing control |
| 6 | RichText | Formatted text (HTML-like) |
| 7 | Split | Splitter control/container |
| 8 | TabCtrl | Multi-tab control (browser-like tabs) |
| 9 | ListCtrl | List control with Report/Icon/List views |
| 10 | PropertyGrid | Property grid supporting text, number, checkbox, font, color, date, IP address, hotkey, file path and folder properties |
| 11 | ColorPicker | Color picker in a separate window; its sub-controls can also be used standalone as color controls |
| 12 | ComboButton | Button with a drop-down combo box |
| 13 | DirectoryTree | Directory tree that shows directories of the file system |
| 14 | AddressBar | Address bar that shows a local file-system path |
| 15 | WebView2Control | Wraps the basic functionality of the WebView2 control |
| 16 | GridBox/GridScrollBox | Controls based on the grid layout |
| 17 | HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox | Controls based on horizontal and vertical flow layouts |
| 18 | MenuBar | Menu bar control |
| 19 | IconControl/BitmapControl | Display small in-memory icons and bitmap data |
| 20 | ChildWindow | Child-window control; on Windows it is a native child window (`WS_CHILD`), on other platforms it is an SDL popup window (SDL has no native child windows) |
| 21 | ControlDragableT | Template class with four standard controls (ControlDragable/BoxDragable/HBoxDragable/VBoxDragable): drag to reorder children inside the same Box, or drag across Boxes to change a control's container |
| 22 | ControlMovableT | Template class with four standard controls (ControlMovable/BoxMovable/HBoxMovable/VBoxMovable): drag with the mouse to move a control or its parent container |
| 23 | ControlResizableT | Template class with four standard controls (ControlResizable/BoxResizable/HBoxResizable/VBoxResizable): drag with the mouse to resize a control, similar to resizing a window |
| 24 | XmlBox | Container that loads and previews a duilib XML file, useful for previewing the appearance defined in XML |
| 25 | Pane/PanelHBox/PanelVBox | Panel containers with a title bar (`PanelTemplate<Box>`), supporting title-bar styles, collapse/expand and accordion grouping |
| 26 | MessageBoxWnd | Skinable modal message box shipped with the framework |
| 27 | ToastWnd | Skinable modeless notification window shipped with the framework: auto-dismiss, stacking, pause on hover, click to close |
| 28 | SearchBox | Search-box composite control: leading icon + edit box + clear button |
| 29 | SpinBox | Numeric input box derived from RichEdit, supporting step, range and step buttons |
| 30 | Switch | Sliding switch derived from CheckBox with its own slide animation and color transition |
| 31 | Badge | Badge control for unread counts and red dots; counts above the limit show "99+" and `count<=0` hides it |
| 32 | Flyout | Generic flyout (popup card) window that shows arbitrary Box content around an anchor control (action panels, confirmation cards, rich tooltips, etc.) |
| 33 | Calendar | Calendar control |
| 34 | NavigationView | Sidebar navigation container (navigation pane on the left, content area on the right) with item groups, a bottom settings item and a collapsible pane |
| 35 | Chart | Lightweight chart control supporting line, bar and pie charts |
| 36 | Marquee | Text marquee that scrolls text horizontally or vertically in a loop |
| 37 | Timeline | Vertical timeline control (nodes + connecting lines + content) with data binding, state colors, ascending/descending order, localized text and a scroll bar |
| 38 | TagInput | Tag-input composite control: Enter (or a configured separator) creates a tag; close buttons/Backspace delete tags; de-duplication, a maximum count and automatic wrapping are supported |
| 39 | AutoComplete | Auto-complete composite control: a suggestion popup follows the edit box while typing (reusing Flyout, without stealing focus, width following the edit box); prefix/contains matching, case-insensitive matching, arrow keys/Enter/Esc and mouse selection are supported |
| 40 | Mask | Mask/overlay control (derived from Box) for modal dialog backdrops, onboarding overlays and loading covers; it can highlight a target control through a cutout, let clicks inside the cutout pass through to the target, and adapt to light/dark semantic colors |
| 41 | StatusBar | Status bar control (derived from HBox) for cursor position, character count, connection state, background progress and similar hints; children are the status panels, separators between adjacent panels are drawn automatically (width and color configurable), and an optional size grip is available |
| 42 | ToolBar | Toolbar control (derived from HBox) that hosts a row of tool buttons and separators; when width is insufficient, trailing children are hidden and a "more" button appears at the right, listing the hidden items in a flyout and notifying the application through a callback; layout is recalculated when the window width changes |
| 43 | RangeSlider | Two-thumb range slider (derived from Progress) with lower/upper thumbs on one track and the selected range highlighted; minimum gap, range color/corner radius, value-bubble tooltip, marks and labels, `included` highlighting outside the range, snapping to marks and a range-changed callback (`pEventData` carries low/high) |
| 44 | Stepper | Wizard step control for setup/registration/configuration wizards and approval flows; horizontal or vertical, nodes show numbers or check marks, state is derived from the current index plus a disabled set (pending/in progress/completed/disabled), connector lines are colored by progress, Next/Back/GoTo and click navigation are supported, titles/descriptions are localized, and the step-changed callback's `pEventData` carries currentIndex/oldIndex |
| 45 | Rating | Star rating control (derived from Control) for reviews, scores and satisfaction surveys; read-only display, half stars (`allow_half`), hover preview and click/drag rating are supported; filled/half/empty stars and hover/pressed states are all driven by SVG images mapped to semantic colors; star count (`max`/`star_count`), size and spacing are configurable, and the value-changed callback's `pEventData` carries value/oldValue |
| 46 | Carousel | Image carousel (derived from Control) for hero banners, ad slots and image galleries; slides, previous/next buttons and clickable page dots are SVG images mapped to semantic colors; autoplay with pause on hover, infinite looping (`loop`), transition animation (`transition_ms`) and press-and-drag paging are supported, and the index-changed callback's `pEventData` carries index/oldIndex |
| 47 | Pagination | Pagination control (derived from HBox) for page navigation in reports, tables and lists; previous/next (optionally first/last) plus page buttons generated from the data, with the current page selected; when there are more pages than the display limit they are collapsed into "…" while the first, last and pages around the current one stay visible; button colors come from the `button_class` skin class (semantic colors); an optional page-size selector (`show_page_size`, off by default; `page_size_options` configures the candidates, and changing it recomputes the page count from `total_count` and resets to page 1) is available; page and page-size change callbacks carry page/oldPage/pageSize/oldPageSize in `pEventData` |
| 48 | TreeGrid | Tree-table control (derived from VBox) that shows "expandable/collapsible hierarchy rows + a multi-column table" in one control, for file trees, reports and org charts; the header stays fixed while the body scrolls vertically, and the header and data rows share the same column-width array so columns stay aligned (indentation is done with an inline placeholder in the tree column, so other columns do not shift); column title, width and alignment are configurable, multi-select (`multi_select`) is supported, and header check boxes cascade in three states between parents and children (`check_mode` + `check_cascade`); the check-changed event `pEventData` carries a TreeGridCheckValue |
| 49 | Dialog | Generic modal dialog (`duilib/Utils/Dialog.h`): the content area accepts any XML fragment, and the buttons at the bottom (count, text, default button) are configurable without an extra XML file; `DoModal` returns the index of the clicked button; ESC and title-bar close cancel, Enter triggers the focused button, text IDs are localized, and a per-button callback runs before closing (returning false prevents closing, which allows reading content data or validating input) |
| 50 | SystemNotification | Native system notification (`duilib/Utils/SystemNotification.h`): builds that use SDL3 send notifications through the SDL system notification API (Windows/macOS/Linux/FreeBSD), while Windows builds without SDL use `Shell_NotifyIcon` tray balloons (Windows 7 and later); `IsSupported`/`RequestPermission`/`Show`/`Remove` are provided, notification priority is supported, and display and dismissal are handled by the system (see [SystemNotification.md](SystemNotification.en.md)) |
| 51 | Breadcrumb | Breadcrumb navigation control (`duilib/Control/Breadcrumb.h`): fully custom-drawn, `\|`-separated items, current item highlighted, clicking an ancestor navigates, disabled items, configurable separator/spacing/font and semantic colors, localized `item_text_id`, and a current-changed event `AttachCurrentChanged`; when width is insufficient, items are trimmed from the left so the current item stays visible |
| 52 | CascadingSelect | Cascading select control (`duilib/Control/CascadingSelect.h`): supports an inline multi-column form and a drop-down list form; changing a parent filters the child columns and resets deeper selections, with any number of levels, `items`/`items_id` path data (common prefixes merged), disabled paths and subtrees, fixed/equal column widths, per-column wheel scrolling, localization and semantic colors; the drop-down form supports placeholder text, a clear button, expand on hover, custom display text and flyout sizing; the selection-changed event is `AttachSelectionChanged` |

## 4. Performance optimizations (3 items)

| No. | Optimization |
| :--- | :--- |
| 1 | Reduced the memory footprint of Control and its subclasses; memory usage drops significantly on windows with many elements |
| 2 | Optimized the animation drawing path and merged timer events, avoiding UI lag while control animations or animated images play |
| 3 | Improved the virtual-list based ListBox controls and their related controls through a reworked implementation, improving both usability and performance |

## 5. Example programs (16 items)

The examples serve both as usage documentation and as regression-test entry points; the items below can be used to look up how the corresponding controls are used.

| No. | Example | Description |
| :--- | :--- | :--- |
| 1 | [examples/ColorPicker](../examples/ColorPicker) | New color picker example |
| 2 | [examples/ListCtrl](../examples/ListCtrl) | New list example demonstrating the list features |
| 3 | [examples/render](../examples/render) | New rendering example demonstrating most containers, controls and resource management |
| 4 | [examples/TreeView](../examples/TreeView) | New tree control example demonstrating its features |
| 5 | [examples/RichEdit](../examples/RichEdit) | New rich edit example demonstrating its features |
| 6 | [examples/MultiLang](../examples/MultiLang) | Demonstrates dynamic language switching |
| 7 | [examples/DpiAware](../examples/DpiAware) | Demonstrates DPI awareness |
| 8 | [examples/threads](../examples/threads) | Demonstrates multi-threading |
| 9 | [examples/WebView2](../examples/WebView2) | Demonstrates the WebView2 control |
| 10 | [examples/WebView2Browser](../examples/WebView2Browser) | Demonstrates the WebView2 control with multiple tabs |
| 11 | [examples/layout](../examples/layout) | Demonstrates all layouts and containers |
| 12 | [examples/ChildWindow](../examples/ChildWindow) | Demonstrates the child-window control |
| 13 | [examples/XmlPreview](../examples/XmlPreview) | Tests XML preview (the XmlBox container) |
| 14 | [examples/ColorTheme](../examples/ColorTheme) | Previews and tests color themes |
| 15 | [examples/panel](../examples/panel) | Demonstrates panel containers with a title bar (Pane/PanelHBox/PanelVBox), including collapse/expand animation, title-bar slot buttons, cancel-before-collapse (veto) and dynamic title changes |
| 16 | Other examples | Most received compatibility fixes and optimizations so that the examples can also be used as test programs |

## 6. Documentation and licensing (4 items)

| No. | Item |
| :--- | :--- |
| 1 | Reworked README.md and the documents under docs so that readers can understand the library's features and usage and get started faster |
| 2 | Control interfaces are not documented separately; the comments in the interface headers serve that purpose and are fairly complete at present |
| 3 | Added build documentation and dependency build scripts for each platform |
| 4 | Reorganized the license files of the main project and of the bundled third-party sources, all managed in the `licenses` directory |

## 7. Related links

1. This project continues development on top of [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)
2. NIM_Duilib_Framework is based on [duilib](https://github.com/duilib/duilib)

## 8. Further reading

- Full change history: [CHANGELOG.en.md](../CHANGELOG.en.md)
- Control attributes, events and usage: [Control.en.md](Control.en.md)
- Layouts and containers: [Box.en.md](Box.en.md)
- Example index: [Examples.en.md](Examples.en.md)
