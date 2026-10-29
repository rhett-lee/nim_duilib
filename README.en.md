English | [简体中文](README.md)

> Last synced: 2026-09-29

# nim_duilib

[nim_duilib](https://github.com/rhett-lee/nim_duilib) is a cross-platform GUI library developed in C++, derived from the classic duilib GUI library with deep optimization and feature extensions. It supports the Windows/Linux/macOS/FreeBSD platforms. The supported Linux distributions include OpenEuler, OpenKylin, UbuntuKylin, UnionTech UOS, NeoKylin, Ubuntu, Fedora, Debian, and more, focusing on simplifying efficient desktop application development. Its design incorporates the DirectUI philosophy, describing the UI layout through XML to achieve separation of visuals and logic, significantly improving development flexibility and maintainability.

![GitHub](https://img.shields.io/badge/license-MIT-green.svg)

## Core Technical Features

 - XML layout driven: XML files are used to define the UI structure, fully separating the UI layout from the business logic. Developers can quickly adjust control positions, sizes, and styles by modifying XML without changing the C++ core code, greatly improving development and iteration efficiency.
 - Rich control system: Built-in basic controls such as buttons, text boxes, list views, virtual list controls, tree controls, sliders, progress bars, menus, color pickers, property pages, and tab pages, while also supporting custom control development to meet diverse UI design needs.
 - Efficient rendering mechanism: Uses Skia as the rendering engine to achieve windowless drawing of controls, reducing system resource usage and improving UI refresh speed. Supports hardware-accelerated rendering (the backend drawing supports CPU or GPU rendering), ensuring smooth operation of complex interfaces.
 - Event-driven: Event handling based on the message mechanism makes the UI interaction logic clear, and supports configuring event response code in XML files.
 - Multiple image formats: Supports SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO image formats.
 - Animation format support: Supports GIF, APNG, WEBP, Lottie JSON, and PAG animation file formats.
 - Multilingual and internationalization: Supports dynamic switching between multiple languages, facilitating the development of globalized applications.
 - Dynamic skinning support: Defines the skin structure through XML files, making it easy to change the UI style, with support for dynamic skinning.
 - Window shadow support: Supports rounded-corner shadows and right-angle shadows for windows, with selectable shadow sizes that can be updated in real time; on Windows and macOS systems, system shadows are supported.
 - DPI awareness support: Provides four modes—Unaware, SystemAware, PerMonitorAware, and PerMonitorAware_V2—supports setting DPI independently and adapting to high-DPI displays (Windows platform only).
 - CEF control support: Supports libcef version 109 for compatibility with Windows 7; supports libcef version 142 for Windows 10 and above, as well as the Linux and macOS platforms.
 - WebView2 control support: Supports using the WebView2 control to display web pages; its interface is simply wrapped and easier to use (Windows platform only).
 - SDL3 support: SDL3 can be used as the provider of basic functions such as window management and input/output, thereby enabling cross-platform support (currently adapted for Windows/Linux/macOS/FreeBSD platforms).
 - Theme switching support: Light and dark themes are supported by default, accent color settings are supported, and auxiliary tools for designing themes are provided.

## Directory Structure
| Directory | Description |
| :--- | :--- |
| duilib        | Source code of the project |
| docs          | Documentation of the project, including feature descriptions and attribute lists for each control |
| bin           | Output directory for each example program, containing preset skins, language files, and CEF dependencies |
| licenses      | License files corresponding to other referenced open-source code |
| cmake         | Common settings depended on during CMake builds |
| build         | Build scripts and build projects for each platform (including VC build projects) |
| msvc          | Application manifest files and common VC project configuration for the Windows platform |
| examples      | Source code of the project's example programs, covering basic usage examples of all controls (example programs, see [docs/Examples.en.md](docs/Examples.en.md)) |
| duilib/third_party| Third-party libraries that the project code depends on; details are in subsequent documentation |

## Major Modifications Based on the NIM_Duilib_Framework Source Code
<table>
    <tr>
        <th>Category</th>
        <th>Modification</th>
    </tr>
    <tr>
        <td rowspan="12">Overall Improvements</td>
        <td align="left">1. Reorganized the code structure by functional modules, splitting large files into multiple smaller files by class, which helps understand the overall architecture of the library</td>
    </tr>
    <tr><td align="left">2. Reviewed the code's interface files and added comments and functional annotations for each interface, facilitating code reading and understanding</td></tr>
    <tr><td align="left">3. Optimized the configuration XML files, adjusted the attribute naming rules, allowed control width and height to be set as percentages, added some extensions to image attributes, and optimized the image loading process</td></tr>
    <tr><td align="left">4. Extended image resource support: added APNG/WEBP animation/Lottie JSON animation/PAG animation, added ICO support, and optimized the image loading engine and code implementation logic</td></tr>
    <tr><td align="left">5. Reimplemented the code for each layout and added common UI layout schemes, generally categorized as: float layout, horizontal layout, vertical layout, horizontal flow layout, vertical flow layout, grid layout, horizontal tile layout, vertical tile layout, virtual horizontal layout, virtual vertical layout, virtual horizontal tile layout, and virtual vertical tile layout, making the layout concept easier to understand and extend. See the documentation (docs/Box.en.md) for details</td></tr>
    <tr><td align="left">6. Replaced the XML file parsing engine with the pugixml parser, which offers better performance</td></tr>
    <tr><td align="left">7. Removed the dependency on the base library; message loop and thread communication related functions were reimplemented</td></tr>
    <tr><td align="left">8. Integrated the Skia engine as the default rendering engine</td></tr>
    <tr><td align="left">9. Added SDL3 support for cross-platform compatibility (already adapted for Windows, Linux, macOS, and FreeBSD platforms)</td></tr>
    <tr><td align="left">10. Moved the CEF component into the duilib project and upgraded the CEF version (supports libcef version 109 for compatibility with Windows 7; supports libcef version 142 for Windows 10 and above)</td></tr>
    <tr><td align="left">11. Redesigned the image management interface and loading process (Image directory), supporting multi-threaded image loading for better extensibility to other image formats</td></tr>
    <tr><td align="left">12. Redesigned the XML file structure of the global resource manager (global.xml), improved font management, and added font fallback support, thereby enabling Emoji font display; supports light and dark themes and accent colors</td></tr>
    <tr>
        <td rowspan="23">Feature Improvements</td>
        <td align="left">1. Added new attributes to the Window class: improved its functions, enhanced code fault tolerance for DPI adaptation and window messages, and adjusted the code structure</td>
    </tr>
    <tr><td align="left">2. Added new attributes to the Window class: use_system_caption, snap_layout_menu, sys_menu, sys_menu_rect, and icon, providing the option to use the operating system's default title bar; the custom-drawn title bar functions similarly to the system title bar</td></tr>
    <tr><td align="left">3. Introduced the WindowDropTarget helper class for the Window class, providing support for window-based drag-and-drop functionality</td></tr>
    <tr><td align="left">4. Reviewed the resource management related parts, making the management of fonts, colors, images, and other resources easier to understand</td></tr>
    <tr><td align="left">5. Optimized the ListBox control: subdivided into ListBox, VListBox, HListBox, VTileListBox, HTileListBox, VirtualListBox, VirtualVListBox, VirtualHListBox, VirtualVTileListBox, and VirtualHTileListBox variants, with more complete functionality and improved usability of the virtual-list-based ListBox</td></tr>
    <tr><td align="left">6. Optimized the controls associated with the Combo box (CheckCombo, FilterCombo), improving usability</td></tr>
    <tr><td align="left">7. Improved the editing functionality of the DateTime control</td></tr>
    <tr><td align="left">8. Optimized the functional code of the CMenuWnd class and reimplemented the menu according to the new structure, making the controls inside the menu fully compatible with the existing container/control system, easy to understand and maintain</td></tr>
    <tr><td align="left">9. Optimized and extended the functionality of the RichEdit control, adding many commonly used features</td></tr>
    <tr><td align="left">10. Optimized and extended the functionality of the TreeView control, adding many commonly used features and improving usability</td></tr>
    <tr><td align="left">11. Optimized the GlobalManager interface so that all resources are managed through this interface, easy to understand and maintain</td></tr>
    <tr><td align="left">12. Controls within containers in the same window can be configured via attributes to support drag-out and drag-in operations between different containers</td></tr>
    <tr><td align="left">13. Control background colors now support gradients, and a foreground color feature was added</td></tr>
    <tr><td align="left">14. Improved the multilingual feature to better support dynamic language switching, and provided the examples/MultiLang example program</td></tr>
    <tr><td align="left">15. Improved the DPI awareness feature, supporting the four modes Unaware, SystemAware, PerMonitorAware, and PerMonitorAware_V2, supporting independent DPI settings and high-DPI adaptation, and provided the examples/DpiAware example program</td></tr>
    <tr><td align="left">16. Removed the ui_components project; the CEF component code was reorganized and merged into the duilib project, and other contents were deleted</td></tr>
    <tr><td align="left">17. Optimized the window shadow feature; the window shadow uses an SVG image, added the shadow type attribute (shadow_type), and supports custom-drawn shadows and system shadows. See the `docs/Window.en.md` documentation for details</td></tr>
    <tr><td align="left">18. Added support for APNG/SVG/WEBP/ICO/LOTTIE/PAG image formats</td></tr>
    <tr><td align="left">19. Redesigned the control loading feature, using a Box container to display the loading function, configuring the loading interface (including animation images) through an XML file, and supporting interaction with animation images</td></tr>
    <tr><td align="left">20. Enhanced the Label text display control: added "justify" text alignment, added support for vertical text (text drawing direction from top to bottom, right to left), and added support for setting line spacing and character spacing</td></tr>
    <tr><td align="left">21. The Control control supports full-screen display (implemented by calling the newly added Window::SetFullscreenControl function); the CEF control and WebView2 control support switching the page to full screen via F11</td></tr>
    <tr><td align="left">22. Improved the details of control animation functions and introduced easing functions, supporting the configuration of control animation properties, such as setting the easing function type, total animation duration, and playback interval</td></tr>
    <tr><td align="left">23. Unified context-menu API: Control::AttachPopupMenu binds a right-click menu to any control in one line (auto-creates the menu, resolves coordinates, invokes the item callback, and supports adjusting the menu after it pops up); the paired DetachPopupMenu unbinds it.</td></tr>
    <tr>
        <td rowspan="52">New Controls / New Containers</td>
        <td align="left">1. GroupBox: grouping container</td>
    </tr>
    <tr><td align="left">2. HotKey: hotkey control</td></tr>
    <tr><td align="left">3. HyperLink: text with hyperlinks</td></tr>
    <tr><td align="left">4. IPAddress: IP address control</td></tr>
    <tr><td align="left">5. Line: line drawing control</td></tr>
    <tr><td align="left">6. RichText: formatted text (HTML-like format)</td></tr>
    <tr><td align="left">7. Split: splitter control/container</td></tr>
    <tr><td align="left">8. TabCtrl: multi-tab control (similar to browser tabs)</td></tr>
    <tr><td align="left">9. ListCtrl: list control (Report/Icon/List three views)</td></tr>
    <tr><td align="left">10. PropertyGrid: property grid control supporting text, number, checkbox, font, color, date, IP address, hotkey, file path, folder, and other properties</td></tr>
    <tr><td align="left">11. ColorPicker: color picker, a standalone window whose child controls can be used individually as color controls</td></tr>
    <tr><td align="left">12. ComboButton: button with a dropdown combo box</td></tr>
    <tr><td align="left">13. DirectoryTree: directory tree control for displaying directories in the file system</td></tr>
    <tr><td align="left">14. AddressBar: address bar control for displaying the path of the local file system</td></tr>
    <tr><td align="left">15. WebView2Control: encapsulates the basic functions of the WebView2 control</td></tr>
    <tr><td align="left">16. GridBox/GridScrollBox: controls based on grid layout</td></tr>
    <tr><td align="left">17. HFlowBox/VFlowBox/HFlowScrollBox/VFlowScrollBox: controls based on horizontal flow layout and vertical flow layout</td></tr>
    <tr><td align="left">18. MenuBar: menu bar control</td></tr>
    <tr><td align="left">19. IconControl/BitmapControl: used to display small icons and bitmap data based on memory</td></tr>
    <tr><td align="left">20. ChildWindow: child window control. On the Windows platform it is implemented as a native child window (with the WS_CHILD attribute); on other platforms it is an SDL popup window, not a native child window, since SDL does not support native child windows</td></tr>
    <tr><td align="left">21. ControlDragableT (template class, including the following four standard controls: ControlDragable/BoxDragable/HBoxDragable/VBoxDragable): supports reordering child controls within the same Box by dragging, and supports adjusting the container a control belongs to by dragging across different Boxes</td></tr>
    <tr><td align="left">22. ControlMovableT (template class, including the following four standard controls: ControlMovable/BoxMovable/HBoxMovable/VBoxMovable): supports adjusting the position of a control by mouse dragging, and also supports adjusting the position of the parent container by mouse dragging</td></tr>
    <tr><td align="left">23. ControlResizableT (template class, including the following four standard controls: ControlResizable/BoxResizable/HBoxResizable/VBoxResizable): supports adjusting the size of a control by mouse dragging, with functionality similar to resizing a window</td></tr>
    <tr><td align="left">24. XmlBox: a container that supports loading and previewing the XML files of the GUI library, which can be used to preview the display effects of controls defined in XML files</td></tr>
    <tr><td align="left">25. Pane/PanelHBox/PanelVBox: Titled panel container (PanelTemplate<Box>), supporting title bar styling, collapse/expand and accordion groups</td></tr>
    <tr><td align="left">26. MessageBoxWnd: a built-in owner-drawn skin modal message box.</td></tr>
    <tr><td align="left">27. ToastWnd: a built-in owner-drawn skin non-modal toast notification, with auto-dismiss, multi-toast stacking, hover-to-pause and click-to-dismiss.</td></tr>
    <tr><td align="left">28. SearchBox: Composite search box with left icon, edit box and clear button.</td></tr>
    <tr><td align="left">29. SpinBox: Numeric input box, derived from RichEdit, supports step/range/spin buttons.</td></tr>
    <tr><td align="left">30. Switch: Toggle switch control derived from CheckBox with sliding animation and color transition</td></tr>
    <tr><td align="left">31. Switch: Badge control for unread counts/red dots; shows "99+" when exceeding the limit, hidden automatically when count<=0</td></tr>
    <tr><td align="left">32. Flyout: a general-purpose popup (flying card) windowd. It is used to pop up arbitrary Box content around an anchor control (action panels, confirmation cards, rich-content tips, etc.).</td></tr>
    <tr><td align="left">33. Calendar: Calendar control.</td></tr>
    <tr><td align="left">34. NavigationView: Sidebar navigation container (left nav pane + right content area); supports grouped items, a bottom settings item and pane collapse/expand.</td></tr>
    <tr><td align="left">35. Chart: a lightweight chart control. It supports line, bar and pie.</td></tr>
    <tr><td align="left">36. Marquee: a text marquee (scrolling text) control, It scrolls text horizontally/vertically in a loop.</td></tr>
    <tr><td align="left">37. Timeline: Vertical timeline control (nodes + connector lines + content, data-driven, status colors, forward/reverse, multi-language binding, scrollbar).</td></tr>
    <tr><td align="left">38. TagInput: Composite tag input — press Enter (or a configured delimiter) to create tags; supports close-button/Backspace removal, duplicate rejection and a max tag count, with automatic line wrapping.</td></tr>
    <tr><td align="left">39. AutoComplete: Composite auto-complete input — shows a candidate popup below the edit box as you type (reusing Flyout; non-focus-stealing, width follows the edit box); supports prefix/contains matching, case-insensitivity, Up/Down-key/Enter/Esc and mouse selection.</td></tr>
    <tr><td align="left">40. Mask: Mask/overlay layer control (derived from Box) for modal-dialog background dimming, onboarding masks and loading overlays; supports punching out a highlight over a target control and passing clicks inside the highlight through to it, with semantic colors adapted to light/dark themes.</td></tr>
    <tr><td align="left">41. StatusBar: Status bar control (derived from HBox) for status hints such as cursor position, character count, connection state and background progress; each child control is a status pane, separators (with configurable width and color) are drawn automatically between adjacent panes, and an optional size grip can be shown at the bottom-right corner. See docs/Control.en.md.</td></tr>
    <tr><td align="left">42. ToolBar: Toolbar control (derived from HBox) holding a row of tool buttons and separators; when the available width is insufficient it automatically hides trailing children and shows a "more" button at the right end, which pops up a flyout listing the collapsed items and notifies the application layer through a callback; the layout is recalculated when the window width changes. See docs/Control.en.md.</td></tr>
    <tr><td align="left">43. RangeSlider: Range slider control with two thumbs (derived from Progress) that places a low/high thumb on a single track and highlights the selected range; supports a minimum range, range color/radius, value tooltips, marks with labels, `included` outer highlighting, mark snapping and a range-change callback (low/high carried in pEventData). See docs/Control.en.md.</td></tr>
    <tr><td align="left">44. Stepper: Wizard step indicator for install/register/config wizards and approval flows; horizontal/vertical layout, nodes show a number or check mark, state derived from the current index + disabled set (pending/active/completed/disabled), connectors colored by completion, supports Next/Back/GoTo and click-to-jump, multi-language titles/descriptions, and a step-change callback carrying currentIndex/oldIndex in pEventData. See docs/Control.en.md.</td></tr>
    <tr><td align="left">45. Rating: Star rating control (derived from Control) for reviews, scoring and satisfaction surveys; supports read-only display, half stars (allow_half), hover preview and click/drag rating, with filled/half/empty stars plus hovered/pressed states driven entirely by SVGs mapped to semantic colors, configurable star count (max/star_count), size and gap, and a value-change callback carrying value/oldValue in pEventData. See docs/Control.en.md.</td></tr>
    <tr><td align="left">46. Carousel: Image carousel control (derived from Control) for hero banners, ad slots and image galleries; slides, prev/next buttons and clickable bottom indicator dots are all carried by SVGs mapped to semantic colors, with auto-play + hover pause, infinite loop (loop), transition animation (transition_ms) and press-and-drag paging, plus an index-change callback carrying index/oldIndex in pEventData. See docs/Control.en.md.</td></tr>
    <tr><td align="left">47. Pagination: Pager control (derived from HBox) for page navigation in reports, tables and lists; prev/next (optionally first/last) plus page buttons generated dynamically from the data, with the current page shown selected, pages beyond the display limit collapsed into an ellipsis while always keeping the first/last and the pages around the current one, button colors driven by the button_class skin class (semantic colors), an optional "records per page" selector (show_page_size, off by default; page_size_options configures the candidates, and switching re-derives the page count from total_count and resets to page 1), and a page/page-size change callback carrying page/oldPage/pageSize/oldPageSize in pEventData. See docs/Control.en.md.</td></tr>
    <tr><td align="left">48. TreeGrid: Tree grid control (derived from VBox) presenting "expandable/collapsible hierarchical rows + a multi-column table" in a single control, for file trees, reports and org charts; the header is fixed and does not scroll while the list body has its own vertical scrollbar, and the header and data rows share the same column-width array so they align column by column by construction (indentation is an indent placeholder nested inside the tree-column cell, so other columns never shift); column titles/widths/alignment are configurable, with multi-row selection (multi_select) and parent/child tri-state checkbox cascade at the start of each row (check_mode + check_cascade), and a check-change event carrying TreeGridCheckValue in pEventData. See docs/Control.en.md.</td></tr>
    <tr><td align="left">49. Dialog: Generic modal dialog (duilib/Utils/Dialog.h) whose content can be any XML fragment, with fully customizable footer buttons (count, text and default button) and no extra XML file required; DoModal returns the index of the clicked button, and it supports ESC/title-bar close as cancel, Enter activating the focused button, text IDs for multi-language support, and a button-click callback executed before closing (returning false keeps the dialog open, making it possible to read content data or validate input).</td></tr>
    <tr><td align="left">50. SystemNotification: Native system notifications (duilib/Utils/SystemNotification.h). SDL builds send notifications to the system notification center through the SDL notification API (Windows/macOS/Linux/FreeBSD); Windows builds without SDL use a Shell_NotifyIcon tray balloon (Windows 7 and later). Provides IsSupported/RequestPermission/Show/Remove, supports notification priority, and leaves display and dismissal to the system. See docs/SystemNotification.en.md.</td></tr>
    <tr><td align="left">51. Breadcrumb: Breadcrumb navigation control (duilib/Control/Breadcrumb.h), fully self-drawn. Supports `|`-separated items, a highlighted current item, click-to-jump on ancestor items, disabled items, configurable separator/spacing/font and semantic colors, multi-language `item_text_id`, and the `AttachCurrentChanged` event. When the width is insufficient it right-aligns and clips so the current item stays visible.</td></tr>
    <tr><td align="left">52. CascadingSelect: Cascading selector (duilib/Control/CascadingSelect.h) with inline-column and dropdown trigger-box forms. Changing a parent filters child columns and resets deeper old selections automatically. Supports unlimited depth, `items`/`items_id` path data (common prefixes merged), disabled paths and subtrees, fixed or equal column widths, per-column wheel scrolling, localization and semantic colors. The dropdown form adds placeholder text, a clear button, expand-on-hover, custom display text and flyout sizing, plus the `AttachSelectionChanged` event.</td></tr>
    <tr>
        <td rowspan="3">Performance Optimization</td>
        <td align="left">1. Optimized the memory usage of Control and its child controls, greatly reducing memory footprint when there are many UI elements</td>
    </tr>
    <tr><td align="left">2. Optimized the animation drawing process and merged timer trigger events to avoid UI lag when playing control animations or animation images</td></tr>
    <tr><td align="left">3. The virtual-list-based ListBox control and its associated controls: by optimizing the implementation mechanism, both usability and performance were significantly improved</td></tr>
    <tr>
        <td rowspan="16">Example Program Improvements</td>
        <td align="left">1. examples/ColorPicker: added a color picker example program</td>
    </tr>
    <tr><td align="left">2. examples/ListCtrl: added a list example program demonstrating the list's distinctive features</td></tr>
    <tr><td align="left">3. examples/render: added a rendering engine example program demonstrating most container, control, and resource management functions</td></tr>
    <tr><td align="left">4. examples/TreeView: added a tree control example program demonstrating various tree control functions</td></tr>
    <tr><td align="left">5. examples/RichEdit: added a rich text edit control example program demonstrating various rich text edit control functions</td></tr>
    <tr><td align="left">6. examples/MultiLang: demonstrates dynamic switching between multiple languages</td></tr>
    <tr><td align="left">7. examples/DpiAware: demonstrates the DPI awareness feature</td></tr>
    <tr><td align="left">8. examples/threads: demonstrates multi-threading functionality</td></tr>
    <tr><td align="left">9. examples/WebView2: demonstrates the WebView2 control</td></tr>
    <tr><td align="left">10. examples/WebView2Browser: demonstrates the WebView2 control (multi-tab)</td></tr>
    <tr><td align="left">11. examples/layout: demonstrates all layouts and containers</td></tr>
    <tr><td align="left">12. examples/ChildWindow: demonstrates the child window control</td></tr>
    <tr><td align="left">13. examples/XmlPreview: tests the XML file UI preview feature (testing the XmlBox container)</td></tr>
    <tr><td align="left">14. examples/ColorTheme: a preview and test program for color themes</td></tr>
    <tr><td align="left">15. examples/panel: demonstrates the titled panel container (Pane/PanelHBox/PanelVBox), including collapse/expand animation, title-bar slot buttons, pre-collapse veto and dynamically modifying the title</td></tr>
    <tr><td align="left">16. Other example programs: most have undergone code compatibility modifications and optimizations, allowing the example programs to also be used as test programs</td></tr>
    <tr>
        <td rowspan="8">Documentation Improvements</td>
        <td align="left">1. Reorganized the README.md and the documentation in the docs subdirectory, making it easier for readers to understand the GUI library's functions and usage, and easier to get started</td>
    </tr>
    <tr><td align="left">2. The interfaces of each control are not organized into separate documents, because the purpose can be achieved by directly reading the comments in the interface files; currently the comments for each interface are fairly complete</td></tr>
    <tr><td align="left">3. Build documentation for each platform and the dependent build scripts</td></tr>
    <tr><td align="left">4. Reorganized the license files for the main project and dependent third-party source code, unified and managed in the licenses directory</td></tr>
</table>

## Description of Third-Party Libraries Used
| Name | Code Subdirectory | Purpose | License File | License Classification |
| :--- | :--- |:--- |:--- |:--- |
|apng |duilib/third_party/libpng | Supports APNG image format |zlib/libpng License|zlib/libpng License, a permissive open-source license|
|libpng |duilib/third_party/libpng | Supports PNG image format |[libpng.LICENSE.txt](licenses/libpng.LICENSE.txt)|Custom BSD-style permissive license|
|zlib |duilib/third_party/zlib | Supports PNG/APNG image format<br>Zip file decompression |[zlib.LICENSE.txt](licenses/zlib.LICENSE.txt)|zlib license, a permissive open-source license|
|cximage |duilib/third_party/cximage | Supports ICO image format |[cximage.LICENSE.txt](licenses/cximage.LICENSE.txt)|MIT-style license (non-standard MIT license)|
|giflib |duilib/third_party/giflib | Supports GIF image format |[giflib.LICENSE.txt](licenses/giflib.LICENSE.txt)|MIT License|
|libwebp |duilib/third_party/libwebp | Supports WebP image format |[libWebP.LICENSE.txt](licenses/libwebp.LICENSE.txt)|BSD 3-Clause License|
|stb_image |duilib/third_party/stb_image| Supports BMP image format<br>Image resizing |[stb_image.LICENSE.txt](licenses/stb_image.LICENSE.txt)|MIT License / Public Domain License|
|libjpeg-turbo |duilib/third_party/libjpeg-turbo| Supports JPEG image format |[libjpeg-turbo.LICENSE.md](licenses/libjpeg-turbo.LICENSE.md)|IJG License and modified BSD 3-Clause License|
|nanosvg |duilib/third_party/svg | Supports SVG image format |[nanosvg.LICENSE.txt](licenses/nanosvg.LICENSE.txt)|zlib License|
|pugixml |duilib/third_party/xml | Supports parsing of resource description XML |[pugixml.LICENSE.txt](licenses/pugixml.LICENSE.txt)|MIT License|
|ConvertUTF |duilib/third_party/convert_utf| Used for UTF-8/UTF-16 encoding conversion |[llvm.LICENSE.txt](licenses/llvm.LICENSE.txt)|Primarily Apache License Version 2.0,<br>supplemented by LLVM exception clauses,<br>legacy protocol for historical versions|
|skia |Project does not include skia source code | GUI library rendering engine<br>Supports SVG image format<br>Supports Lottie JSON animation|[skia.LICENSE.txt](licenses/skia.LICENSE.txt)|BSD 3-Clause License|
|expat |Skia source tree third_party/expat | Used by Skia for SVG parsing |[expat.LICENSE.txt](licenses/expat.LICENSE.txt)|MIT License|
|Roboto Mono |bin/resources/fonts | UI font (example assets) |[RobotoMono.LICENSE.txt](licenses/RobotoMono.LICENSE.txt)|Apache License 2.0|
|SDL |Project does not include SDL source code | Cross-platform window management |[SDL.LICENSE.txt](licenses/SDL.LICENSE.txt)|zlib License|
|duilib | | NIM_Duilib_Framework<br>is developed based on duilib |[duilib.LICENSE.txt](licenses/duilib.LICENSE.txt)|BSD 2-Clause License|
|NIM_Duilib<br>Framework| | This project is developed based on<br>NIM_Duilib_Framework |[NIM_Duilib_Framework.LICENSE.txt](licenses/NIM_Duilib_Framework.LICENSE.txt)|MIT License|
|libcef |duilib/third_party/libcef | Used to load the CEF module|[libcef.LICENSE.txt](licenses/libcef.LICENSE.txt)|BSD 3-Clause License|
|udis86 |duilib/third_party/libudis86| Disassembly to calculate the minimum length of complete instructions |[udis86.LICENSE.txt](licenses/udis86.LICENSE.txt)|BSD 2-Clause License|
|WebView2 |duilib/third_party/<br>Microsoft.Web.WebView2| Supports the WebView2 control |[Microsoft.Web.WebView2.LICENSE.txt](licenses/Microsoft.Web.WebView2.LICENSE.txt)|BSD 3-Clause License|
|libpag |duilib/third_party/libpag | Supports PAG animation files<br>(this feature is disabled by default; see subsequent documentation) |[libpag.LICENSE.txt](licenses/libpag.LICENSE.txt)|Apache License Version 2.0 (main body)<br>The third-party components that libpag depends on have<br>many different licenses; see the directory:<br>`duilib/third_party/libpag/licenses`<br>for the files. If you mind libpag's license<br>(including the main body license / third-party component licenses),<br>you may leave libpag disabled.|

## UI Preview
The example programs written using this GUI library, demonstrating the display effects of each control, can be found in: [docs/Examples.en.md](docs/Examples.en.md) 

## About the PAG Animation File Format
* Currently the PAG animation file format is only supported on the Windows platform; other platforms are not yet supported
* The PAG animation file format support is disabled by default (because you need to compile libpag.lib and libpag.dll yourself and place them in the project for normal compilation and execution)
* How to enable the PAG animation format:    
(1) Open the [`msvc/PropertySheets/LibPagSettings.props`](msvc/PropertySheets/LibPagSettings.props) file with a text editor and change the `LibPagEnabled` variable's value to `1`    
(2) Compile the libpag library by referring to the following documentation: [`duilib/third_party/libpag/windows/libpag-build.md`](duilib/third_party/libpag/windows/libpag-build.md)     
* When building nim_duilib, you must use `build/duilib.sln` or `build/examples.sln` to build; other build methods are not supported.
* The main license of the libpag library is Apache License Version 2.0, and its dependent third-party components have many different licenses,<br>see the files in the directory: `duilib/third_party/libpag/licenses`.<br>If you mind libpag's license (including the main body license / third-party component licenses), you may leave libpag disabled.

## Programming Languages
- C/C++: The compiler needs to support C++20 (`main` branch)
- C/C++: The compiler needs to support C++17 (`develop-cpp17` branch; this branch is only for supporting the VS2017/VS2019 compilers on Windows; for other environments please use the main branch)

## Supported Operating Systems
- Windows: 7/10/11 and above
- Linux: OpenEuler, OpenKylin (openKylin), UbuntuKylin (Ubuntu Kylin), NeoKylin, UnionTech UOS, Ubuntu, Debian, Fedora, OpenSuse, etc.
- macOS: 12+
- FreeBSD

## Supported Compilers
- Visual Studio 2022/2026 (Windows)
- Visual Studio 2017/2019 (Windows; the compilers of these two versions are only supported by the `develop-cpp17` branch code, not by other branches; when using VS2017, the CEF module is not supported)
- LLVM (Windows)
- MinGW-W64: gcc/g++, clang/clang++ (Windows)
- gcc/g++ (Linux)
- clang/clang++ (Linux)
- clang/clang++ (macOS)
- clang/clang++ (FreeBSD)

## Build

Requirements: CMake 3.24+, Python 3, Git, Ninja; on Windows also Visual Studio 2022+
(or MSYS2 / MinGW-w64); on Linux/macOS/FreeBSD use gcc/clang. macOS, Linux and FreeBSD
usually only provide the `python3` command, so all examples below use `python3`; on
Windows you can run `build\build.bat` instead (same arguments).

### Quick Start

```bash
python3 build/build.py                 # Release: prepare Skia/CEF on first run (SDL is off by default on Windows; pass --sdl if needed), build lib + examples
python3 build/build.py --config Debug  # Debug (separate build tree)
python3 build/build.py doctor          # environment check
python3 build/build.py pack            # build an SDK package
```

The build tree is `out/<arch>/<Config>`; libraries go to `lib/<arch>/<Config>`, examples
and runtime assets go to `bin/`. One invocation builds one configuration; run it twice
for Debug + Release.

### Which Way Should I Use?

| Scenario | Command |
| :--- | :--- |
| Just get it running | `python3 build/build.py` |
| Use Visual Studio | `python3 build/build.py deps`, then open `build/examples.sln` |
| Library only | `python3 build/build.py build --target duilib` |
| Build a release package | `python3 build/build.py pack --with-cef` |
| Debug build | `python3 build/build.py --config Debug` |
| Full options | [build/build-reference.en.md](build/build-reference.en.md) |

On Windows, prefer `build\build.bat` (same arguments, no `python3` command needed). The legacy
per-platform scripts (`build_duilib_all_in_one.*`, `msvc_build.bat`, `linux_build.sh`, ...)
were deleted on 2026-10-29 (the new entry point is validated on Windows, Linux, macOS and
FreeBSD); use `build\build.bat` or `build.py`.

### Dependencies and Platform Differences

- **Skia**: required; built by `deps` from the pinned manifest
  (`cmake/third_party_versions.json`). Windows uses Visual Studio's bundled clang/ninja;
  Linux/macOS/FreeBSD use the system LLVM.
- **SDL3**: off by default on Windows (enable with `--sdl`); forced on elsewhere.
- **CEF**: on by default on Windows/Linux/macOS; not supported on FreeBSD or MinGW.
  Runtime files go to `bin/libcef_win/<arch>`, `bin/libcef_win_109/<arch>` or
  `bin/libcef_linux`.
- **WebView2**: Windows only.
- Per-distribution Linux packages (fonts, GL, X11, ibus, ...) are listed in
  [build-reference.en.md](build/build-reference.en.md); macOS needs
  `brew install cmake ninja` plus `gn` built from source; FreeBSD uses
  `sudo pkg install git unzip python3 cmake ninja gn llvm fontconfig freetype2`.

The static runtime (/MT, /MTd) is the default; add `--md` for the dynamic runtime
(/MD, /MDd) and build Skia with the same runtime. SDKs are produced only by `pack.py`
and include `LICENSES/`, `THIRD-PARTY-NOTICES.md` and `THIRD-PARTY-NOTICES.en.md`.

## Development Plan
 - Continue to enrich the controls of the GUI library and improve its functionality
 - Continuous testing and improvement of the cross-platform (Windows/Linux/macOS/FreeBSD) window engine (based on [SDL3.0](https://www.libsdl.org/)) (currently relatively stable under X11/XWayland desktop environments, but has more issues under pure Wayland desktop environments)
 - Test the GUI library, discover and fix defects, and continuously improve the code

## AI-Assisted Development (Claude Code Integration)

nim_duilib provides AI-friendly documentation and skills that can be used with [Claude Code](https://docs.anthropic.com/en/docs/claude-code) to enable AI-assisted UI development.

### Feature Description
After registration, Claude Code can use the following nim_duilib-specific skills in **any project**:

| Command / Skill | Description |
| :--- | :--- |
| `/nim-init` | Initialize the nim_duilib development environment for the current project (copy the LLM reference documentation, update CLAUDE.md) |
| `/nim-duilib-create-window` | Create a new window (automatically generate C++ class + XML layout file) |
| `/nim-duilib-xml-layout` | Design XML UI layouts (templates such as forms, split panels, toolbars, card grids) |
| `/nim-duilib-add-control` | Add controls (XML snippets and C++ event binding code for 15+ controls) |
| `/nim-duilib-event-handler` | Event handling (XML inline events and C++ Attach binding) |
| `/nim-duilib-theme` | Theme customization (predefined colors, fonts, quick reference for 100+ common style classes) |
| `/nim-duilib-resource-pack` | Resource packaging and deployment (ZIP packaging, embedding into a single-file EXE release) |

### Quick Start

**Prerequisites:** [Claude Code](https://docs.anthropic.com/en/docs/claude-code) is installed

**Step 1: Register (One-Time Only)**

Run the following in the nim_duilib root directory:
```bash
# Windows (CMD / PowerShell)
.claude\register.bat

# Linux / macOS
bash .claude/register.sh
```
The registration script installs all skills to `~/.claude/skills/`, taking effect globally.

**Step 2: Use in Your Application Project**

Open Claude Code in any project that needs to use nim_duilib, and type:
```
/nim-init
```
The AI will automatically configure the LLM reference documentation and CLAUDE.md for the project. After that, you can directly instruct Claude to complete UI development using natural language, for example:
- "Create a settings window with a username input box and a save button"
- "Design a layout with a left navigation bar and a right content area"
- "Add a click event to this button"
- "Package the resources into a single EXE"

### Updating Skills
When the nim_duilib AI skill files (`.claude/skills/`) are updated, simply re-run the registration script:
```bash
cd nim_duilib
.claude\register.bat   # Windows
# bash .claude/register.sh  # Linux / macOS
```

### Unregister
To remove all global skills:
```bash
bash nim_duilib/.claude/unregister.sh
```

### File Structure
```
nim_duilib/.claude/
├── register.bat / register.ps1 / register.sh   # Global registration script
├── unregister.sh                                # Unregister script
├── docs/
│   └── nim-duilib-llm-reference.md              # Complete LLM reference manual
└── skills/                                      # AI skill definitions
    ├── nim-duilib-create-window.md
    ├── nim-duilib-xml-layout.md
    ├── nim-duilib-add-control.md
    ├── nim-duilib-event-handler.md
    ├── nim-duilib-theme.md
    └── nim-duilib-resource-pack.md
```

## Reference Documentation

 - [Quick Start](docs/Getting-Started.en.md)
 - [Example Programs](docs/Examples.en.md)
 - [Global Styles: Fonts, Colors, Images, and Other Resources](docs/Global.en.md)
 - [Window Styles](docs/Window.en.md)
 - [Container Styles](docs/Box.en.md)
 - [Control Styles](docs/Control.en.md)
 - [Menu Styles](docs/Menu.en.md)
 - [Modal Message Box (MessageBoxWnd)](docs/MessageBox.en.md)
 - [Non-Modal Toast Notification (ToastWnd)](docs/Toast.en.md)
 - [Native System Notification (SystemNotification)](docs/SystemNotification.en.md)
 - [Control Events/Messages](docs/Events.en.md)
 - [Node Names of Each Control in XML Files](docs/XmlNode.en.md)
 - [Responding to Control Events Directly in XML Files](docs/XmlEvents.en.md)
 - [CEF Control](docs/CEF.en.md)
 - [Build Reference](build/build-reference.en.md)
 - [Building an Example as a macOS App Bundle](build/macos-app-bundle.en.md)
 - [build directory: prerequisites and packaging](build/README.en.md)
 - [Reference Documentation](docs/Summary.en.md)

## Related Links
1. The Skia build documentation repository, click to visit: [skia_compile](https://github.com/rhett-lee/skia_compile):    
2. This project is developed directly on top of the NIM_Duilib_Framework project. Project address: [NIM_Duilib_Framework](https://github.com/netease-im/NIM_Duilib_Framework/)
3. The NIM_Duilib_Framework project is developed based on duilib. Project address: [duilib](https://github.com/duilib/duilib)
