English | [简体中文](Summary.md)

> Last synced: 2026-09-29

## Project Related Content Links

* [Project Introduction](../README.en.md)
* [Quick Start](Getting-Started.en.md)
* [Example Programs](Examples.en.md)
* [Global Styles: Fonts, Colors, Images, and Other Resources](Global.en.md)
* [Window Styles](Window.en.md)
* [Window Shadow Reference Document](WindowShadow.en.md)
* [Container Styles](Box.en.md)
* [Control Styles](Control.en.md)
* [Menu Styles](Menu.en.md)
* [Modal Message Box (MessageBoxWnd)](MessageBox.en.md)
* [Non-Modal Toast Notification (ToastWnd)](Toast.en.md)
* [Control Events / Messages](Events.en.md)
* [Node Names of Each Control in XML Files](XmlNode.en.md)
* [Directly Responding to Control Events in XML Files](XmlEvents.en.md)
* [Theme Reference Document](Theme.en.md)
* [Control Library Capability Inventory and New Control Roadmap](Control-Roadmap.en.md)
* [CEF Control](CEF.en.md)
* [Project Build Related Documents and Scripts](../build/build.md)

## Brief Table of Main Classes in the Project
For detailed interface descriptions of each class, please refer to the associated header file, which has detailed comments.    
* Basic Data Types

| Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- |
| UiSize | [duilib/Core/UiSize.h](../duilib/Core/UiSize.h) | 32-bit Size type wrapper |
| UiSize64 | [duilib/Core/UiSize64.h](../duilib/Core/UiSize64.h) | 64-bit Size type wrapper |
| UiPoint | [duilib/Core/UiPoint.h](../duilib/Core/UiPoint.h) | Integer Point type wrapper |
| UiPointF | [duilib/Core/UiPointF.h](../duilib/Core/UiPointF.h) | Floating-point Point type wrapper |
| UiFixedInt | [duilib/Core/UiFixedInt.h](../duilib/Core/UiFixedInt.h) | Wrapper for the numeric type for setting the length (or width) of a control |
| UiEstInt | [duilib/Core/UiEstInt.h](../duilib/Core/UiEstInt.h) | Wrapper for the estimated length (or width) numeric type of a control |
| UiFixedSize | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | The set size of a control |
| UiEstSize | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | Estimated size of a control (compared to UiFixedSize, no Auto type) |
| UiEstResult | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | The result of estimating a control's size |
| UiPadding | [duilib/Core/UiPadding.h](../duilib/Core/UiPadding.h) | Inner padding type wrapper |
| UiMargin | [duilib/Core/UiMargin.h](../duilib/Core/UiMargin.h) | Outer margin type wrapper |
| UiString | [duilib/Core/UiString.h](../duilib/Core/UiString.h) | The string used by controls, used to replace std::wstring to reduce the memory usage of controls; this class is suitable for storing strings in a lower memory space; in terms of performance, due to the excessive string copying, the performance is relatively weak |
| UiRect | [duilib/Core/UiRect.h](../duilib/Core/UiRect.h) | 32-bit Rect type wrapper |
| UiFont | [duilib/Core/UiFont.h](../duilib/Core/UiFont.h) | Font type wrapper |
| UiColor | [duilib/Core/UiColor.h](../duilib/Core/UiColor.h) | Color type wrapper |
| UiColors | [duilib/Core/UiColors.h](../duilib/Core/UiColors.h) | Common color value constants (ARGB format) |

* Window Related

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| Window | [duilib/Core/Window.h](../duilib/Core/Window.h) | duilib core window wrapper |
| WindowBuilder | [duilib/Core/WindowBuilder.h](../duilib/Core/WindowBuilder.h) | Parse XML files and create windows, container layouts, controls, etc. |
| Shadow | [duilib/Core/Shadow.h](../duilib/Core/Shadow.h) | Window shadow attributes |
| WindowImplBase | [duilib/Utils/WinImplBase.h](../duilib/Utils/WinImplBase.h) | The window base class provided to the application layer; implements a window with a title bar, with support for maximize, minimize, and restore buttons, and provides a window shadow with the WS_EX_LAYERED attribute |
| EventType | [duilib/duilib_defs.h](../duilib/duilib_defs.h) | Defines all message types |
| EventArgs | [duilib/Core/EventArgs.h](../duilib/Core/EventArgs.h) | Parameters for window messages and control event notifications |
| WindowDropTarget | [duilib/Core/WindowDropTarget.h](../duilib/Core/WindowDropTarget.h) | Drag-and-drop support for controls in a window |
| Windows Version Compatibility | [duilib/duilib_config.h](../duilib/duilib_config.h) | Windows version compatibility definition; the default configuration is compatible with Windows 7 and above operating systems |

* Base Classes of Controls and Containers

| Class Name | Base Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- | :--- |
| PlaceHolder | nbase::SupportWeakCallback | [duilib/Core/PlaceHolder.h](../duilib/Core/PlaceHolder.h) | The top-level base class of all controls and containers; encapsulates basic data such as the position, size, and appearance shape of a control |
| Control | PlaceHolder | [duilib/Core/Control.h](../duilib/Core/Control.h) | A basic control, and also the base class of all controls and containers; includes basic functions such as the position, size, state, color, image resources, animation, drawing, cursor, mouse, keyboard, focus, and shortcut key operations of a control |
| Box | Control | [duilib/Core/Box.h](../duilib/Core/Box.h) | The base class of all containers; encapsulates the position, size, appearance shape, and child control management (add, delete, modify, draw, operate, drag-and-drop, layout, etc.) of a container |

* Layout

| Class Name | Base Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- | :--- |
| Layout | | [duilib/Layout/Layout.h](../duilib/Layout/Layout.h) | Floating layout |
| HLayout | Layout | [duilib/Layout/HLayout.h](../duilib/Layout/HLayout.h) | Horizontal layout |
| VLayout | Layout | [duilib/Layout/VLayout.h](../duilib/Layout/VLayout.h) | Vertical layout |
| HFlowLayout | Layout | [duilib/Layout/HFlowLayout.h](../duilib/Layout/HFlowLayout.h) | Horizontal flow layout |
| VFlowLayout | Layout | [duilib/Layout/VFlowLayout.h](../duilib/Layout/VFlowLayout.h) | Vertical flow layout |
| HTileLayout | Layout | [duilib/Layout/HTileLayout.h](../duilib/Layout/HTileLayout.h) | Horizontal tile layout |
| VTileLayout | Layout | [duilib/Layout/VTileLayout.h](../duilib/Layout/VTileLayout.h) | Vertical tile layout |
| VirtualHLayout | HLayout | [duilib/Layout/VirtualHLayout.h](../duilib/Layout/VirtualHLayout.h) | Virtual-list horizontal layout |
| VirtualVLayout | VLayout | [duilib/Layout/VirtualVLayout.h](../duilib/Layout/VirtualVLayout.h) | Virtual-list vertical layout |
| VirtualHTileLayout | HTileLayout | [duilib/Layout/VirtualHTileLayout.h](../duilib/Layout/VirtualHTileLayout.h) | Virtual-list horizontal tile layout |
| VirtualVTileLayout | VTileLayout | [duilib/Layout/VirtualVTileLayout.h](../duilib/Layout/VirtualVTileLayout.h) | Virtual-list vertical tile layout |
| GridLayout | Layout | [duilib/Layout/GridLayout.h](../duilib/Layout/GridLayout.h) | Grid layout |

* Container

| Class Name | Base Class Name | Layout Type | Associated Header File | Name / Purpose |
| :--- | :--- | :--- | :--- | :--- |
| Box | Control | [Layout](../duilib/Layout/Layout.h) | [duilib/Core/Box.h](../duilib/Core/Box.h) | Free-layout container; the base class of all containers; encapsulates the position, size, appearance shape, and child control management (add, delete, modify, draw, operate, drag-and-drop, layout, etc.) of a container |
| VBox | Box | [VLayout](../duilib/Layout/VLayout.h) | [duilib/Box/VBox.h](../duilib/Box/VBox.h) | Vertical layout container |
| HBox | Box | [HLayout](../duilib/Layout/HLayout.h) | [duilib/Box/HBox.h](../duilib/Box/HBox.h) | Horizontal layout container |
| VFlowBox | Box | [VFlowLayout](../duilib/Layout/VFlowLayout.h) | [duilib/Box/VBox.h](../duilib/Box/VBox.h) | Vertical flow layout container |
| HFlowBox | Box | [HFlowLayout](../duilib/Layout/HFlowLayout.h) | [duilib/Box/HBox.h](../duilib/Box/HBox.h) | Horizontal flow layout container |
| TabBox | Box | [Layout](../duilib/Layout/Layout.h) | [duilib/Box/TabBox.h](../duilib/Box/TabBox.h) | Multi-tab layout container |
| VTileBox | Box | [VTileLayout](../duilib/Layout/VTileLayout.h) | [duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | Vertical tile layout container |
| HTileBox | Box | [HTileLayout](../duilib/Layout/HTileLayout.h) | [duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | Horizontal tile layout container |
| ScrollBox | Box | [Layout](../duilib/Layout/Layout.h) | [duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Free-layout scrollable container, with vertical or horizontal scroll bars |
| VScrollBox | ScrollBox | [VLayout](../duilib/Layout/VLayout.h) | [duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Vertical layout scrollable container, with vertical or horizontal scroll bars |
| HScrollBox | ScrollBox | [HLayout](../duilib/Layout/HLayout.h) | [duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Horizontal layout scrollable container, with vertical or horizontal scroll bars |
| VTileScrollBox | ScrollBox | [VTileLayout](../duilib/Layout/VTileLayout.h) | [duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Vertical tile layout scrollable container, with vertical or horizontal scroll bars |
| HTileScrollBox | ScrollBox | [HTileLayout](../duilib/Layout/HTileLayout.h) | [duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Horizontal tile layout scrollable container, with vertical or horizontal scroll bars |
| ListBox | ScrollBox | [Layout](../duilib/Layout/Layout.h) | [duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Free-layout list container |
| VListBox | ListBox | [VLayout](../duilib/Layout/VLayout.h) | [duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Vertical layout list container |
| HListBox | ListBox | [HLayout](../duilib/Layout/HLayout.h) | [duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Horizontal layout list container |
| VTileListBox | ListBox | [VTileLayout](../duilib/Layout/VTileLayout.h) | [duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Vertical tile layout list container |
| HTileListBox | ListBox | [HTileLayout](../duilib/Layout/HTileLayout.h) | [duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Horizontal layout list container |
| VirtualListBox | ListBox | [Layout](../duilib/Layout/Layout.h) | [duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Free-layout list container implemented by virtual list |
| VirtualVListBox | VirtualListBox | [VirtualVLayout](../duilib/Layout/VirtualVLayout.h) | [duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Virtual-list implemented vertical layout list container |
| VirtualHListBox | VirtualListBox | [VirtualHLayout](../duilib/Layout/VirtualHLayout.h) | [duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Virtual-list implemented horizontal layout list container |
| VirtualVTileListBox | VirtualListBox | [VirtualVTileLayout](../duilib/Layout/VirtualVTileLayout.h) | [duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Virtual-list implemented vertical tile layout list container |
| VirtualHTileListBox | VirtualListBox | [VirtualHTileLayout](../duilib/Layout/VirtualHTileLayout.h) | [duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Virtual-list implemented horizontal layout list container |
| GridBox | Box | [GridLayout](../duilib/Layout/GridLayout.h) | [duilib/Box/GridBox.h](../duilib/Box/GridBox.h) | Grid layout container |
| GridScrollBox | ScrollBox | [GridLayout](../duilib/Layout/GridLayout.h) | [duilib/Box/GridBox.h](../duilib/Box/GridBox.h) | Grid layout container (supports scroll bars) |
| Panel | Box | [Layout](../duilib/Layout/Layout.h) | [duilib/Box/Panel.h](../duilib/Box/Panel.h) | Titled panel container (PanelTemplate instance), supporting title bar styling, collapse/expand animation and accordion groups |
| PanelHBox | HBox | [HLayout](../duilib/Layout/HLayout.h) | [duilib/Box/Panel.h](../duilib/Box/Panel.h) | Titled panel container with horizontal layout |
| PanelVBox | VBox | [VLayout](../duilib/Layout/VLayout.h) | [duilib/Box/Panel.h](../duilib/Box/Panel.h) | Titled panel container with vertical layout |
| NavigationView | HBox | [HLayout](../duilib/Layout/HLayout.h) | [duilib/Control/NavigationView.h](../duilib/Control/NavigationView.h) | Sidebar navigation container (left nav pane + right content area); supports grouped items, a bottom settings item and pane collapse/expand |

* Image

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| Image | [duilib/Image/Image.h](../duilib/Image/Image.h) | Image-related wrapper; supported file formats: SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO/Lottie-JSON/PAG |
| ImageAttribute | [duilib/Image/ImageAttribute.h](../duilib/Image/ImageAttribute.h) | Image attributes |
| ImageLoadParam | [duilib/Image/ImageLoadParam.h](../duilib/Image/ImageLoadParam.h) | Image loading attributes, used to load an image |
| ImageInfo | [duilib/Image/ImageInfo.h](../duilib/Image/ImageInfo.h) | Image information |
| ImageDecoder | [duilib/Image/ImageDecoder.h](../duilib/Image/ImageDecoder.h) | Interface supporting multi-threaded decoding (used at the bottom layer for decoding, supports delayed decoding, can be decoded in multiple threads to avoid UI thread stalling when decoding images) |
| ImagePlayer | [duilib/Image/ImagePlayer.h](../duilib/Image/ImagePlayer.h) | Logical wrapper for control image animation playback (supports GIF/WebP/APNG/Lottie-JSON/PAG animations) |
| StateImage | [duilib/Image/StateImage.h](../duilib/Image/StateImage.h) | Mapping between control states and images |
| StateImageMap | [duilib/Image/StateImageMap.h](../duilib/Image/StateImageMap.h) | Mapping between control image types and state images |
| ImageManager | [duilib/Core/ImageManager.h](../duilib/Core/ImageManager.h) | Image resource manager |
| IconManager | [duilib/Core/IconManager.h](../duilib/Core/IconManager.h) | Icon resource manager (thread-safe, suitable for small icon-like image resources), supports the HICON handle on the Windows platform |
| ImageList | [duilib/Core/ImageList.h](../duilib/Core/ImageList.h) | Image list |

* Animation

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| AnimationManager | [duilib/Animation/AnimationManager.h](../duilib/Animation/AnimationManager.h) | Image animation manager |
| AnimationPlayer | [duilib/Animation/AnimationPlayer.h](../duilib/Animation/AnimationPlayer.h) | Image animation playback state management |

* Color

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| UiColor | [duilib/Core/UiColor.h](../duilib/Core/UiColor.h) | Color type wrapper |
| UiColors | [duilib/Core/UiColors.h](../duilib/Core/UiColors.h) | Common color value constants (ARGB format) |
| StateColorMap | [duilib/Core/StateColorMap.h](../duilib/Core/UiColors.h) | Mapping between control states and color values |

* Font

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| UiFont | [duilib/Core/UiFont.h](../duilib/Core/UiFont.h) | Font type wrapper |
| FontManager | [duilib/Core/FontManager.h](../duilib/Core/FontManager.h) | Font manager |

* Render Engine Interface

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| IRenderFactory | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Render factory interface, used to create render implementation objects such as Font, Pen, Brush, Path, Matrix, Bitmap, and Render |
| IFont | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Font interface |
| IBitmap | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Bitmap interface |
| IPen | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Pen interface |
| IBrush | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Brush interface |
| IPath | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Path interface |
| IMatrix | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Matrix interface |
| IRender | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | Render interface, used for drawing, drawing text, etc. |

* Skia Render Engine

| Class Name / Description | Associated Header File | Purpose |
| :--- | :--- | :--- |
| RenderFactory_Skia | [duilib/RenderSkia/RenderFactory_Skia.h](../duilib/RenderSkia/RenderFactory_Skia.h) | Implementation of the render factory interface |
| Font_Skia | [duilib/RenderSkia/Font_Skia.h](../duilib/RenderSkia/Font_Skia.h) | Implementation of the font interface |
| Bitmap_Skia | [duilib/RenderSkia/Bitmap_Skia.h](../duilib/RenderSkia/Bitmap_Skia.h) | Implementation of the bitmap interface |
| Pen_Skia | [duilib/RenderSkia/Pen_Skia.h](../duilib/RenderSkia/Pen_Skia.h) | Implementation of the pen interface |
| Brush_Skia | [duilib/RenderSkia/Brush_Skia.h](../duilib/RenderSkia/Brush_Skia.h) | Implementation of the brush interface |
| Path_Skia | [duilib/RenderSkia/Path_Skia.h](../duilib/RenderSkia/Path_Skia.h) | Implementation of the path interface |
| Matrix_Skia | [duilib/RenderSkia/Matrix_Skia.h](../duilib/RenderSkia/Matrix_Skia.h) | Implementation of the matrix interface |
| FontMgr_Skia | [duilib/RenderSkia/FontMgr_Skia.h](../duilib/RenderSkia/FontMgr_Skia.h) | Interface implementation of the font manager |
| Render_Skia | [duilib/RenderSkia/Render_Skia.h](../duilib/RenderSkia/Render_Skia.h) | Implementation of the render interface, used for drawing, drawing text, etc. |
| Render_Skia_Windows | [duilib/RenderSkia/Render_Skia_Windows.h](../duilib/RenderSkia/Render_Skia_Windows.h) | Implementation of the Windows-related functions of the render interface |
| Render_Skia_SDL | [duilib/RenderSkia/Render_Skia_SDL.h](../duilib/RenderSkia/Render_Skia_SDL.h) | Implementation of the SDL-related functions of the render interface, mainly used for Linux, also supported on Windows |

* Controls / Functional Components

| Class Name / Functional Component | Base Class | Associated Header File | Purpose |
| :--- | :--- | :--- | :--- |
| ScrollBar | Control | [duilib/Core/ScrollBar.h](../duilib/Core/ScrollBar.h) | Scroll bar control |
| Label | Control | [duilib/Control/Label.h](../duilib/Control/Label.h) | Label control (template), used to display text |
| LabelBox | Box | [duilib/Control/Label.h](../duilib/Control/Label.h) | Label container (template), used to display text |
| Button | Control | [duilib/Control/Button.h](../duilib/Control/Button.h) | Button control (template implementation) |
| ButtonBox | Box | [duilib/Control/Button.h](../duilib/Control/Button.h) | Button container control (template implementation) |
| CheckBox | Control | [duilib/Control/CheckBox.h](../duilib/Control/CheckBox.h) | Check box control (template implementation) |
| CheckBoxBox | Box | [duilib/Control/CheckBox.h](../duilib/Control/CheckBox.h) | Check box container (template implementation) |
| Option | Control | [duilib/Control/Option.h](../duilib/Control/Option.h) | Radio button control |
| OptionBox | Box | [duilib/Control/Option.h](../duilib/Control/Option.h) | Radio button container |
| GroupBox | Box | [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | Group container (template) |
| GroupVBox | VBox | [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | Vertical group container (template) |
| GroupHBox | HBox | [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | Horizontal group container (template) |
| Combo | Box | [duilib/Control/Combo.h](../duilib/Control/Combo.h) | Combo box |
| ComboButton | Box | [duilib/Control/ComboButton.h](../duilib/Control/ComboButton.h) | Button with a drop-down combo box |
| CheckCombo | Control | [duilib/Control/CheckCombo.h](../duilib/Control/CheckCombo.h) | Combo box with a check box |
| FilterCombo | Combo | [duilib/Control/FilterCombo.h](../duilib/Control/FilterCombo.h) | Combo box with filter function |
| DateTime | Label | [duilib/Control/DateTime.h](../duilib/Control/DateTime.h) | Date-time picker control |
| HotKey | HBox | [duilib/Control/HotKey.h](../duilib/Control/HotKey.h) | Hot key control |
| HyperLink | Label | [duilib/Control/HyperLink.h](../duilib/Control/HyperLink.h) | Text with a hyperlink; if the URL is empty, it can be used as a normal text button |
| IPAddress | HBox | [duilib/Control/IPAddress.h](../duilib/Control/IPAddress.h) | IP address control |
| Line | Control | [duilib/Control/Line.h](../duilib/Control/Line.h) | Line drawing control |
| NavigationViewItem | Control | [duilib/Control/NavigationView.h](../duilib/Control/NavigationView.h) | Navigation item of NavigationView; supports item/header/separator forms |
| Menu | WindowImplBase | [duilib/Control/Menu.h](../duilib/Control/Menu.h) | Menu, independent window |
| Progress | Label | [duilib/Control/Progress.h](../duilib/Control/Progress.h) | Progress bar control |
| Slider | Progress | [duilib/Control/Slider.h](../duilib/Control/Slider.h) | Slider control |
| CircleProgress | Control | [duilib/Control/CircleProgress.h](../duilib/Control/CircleProgress.h) | Circular progress bar |
| RichEdit | ScrollBox | [duilib/Control/RichEdit.h](../duilib/Control/RichEdit.h) | Rich text edit box control |
| RichEdit Implementation Class | | [duilib/Control/RichEditCtrl_Windows.h](../duilib/Control/RichEditCtrl_Windows.h) | Main function wrapper of the rich text edit box (Windows) |
| RichEdit Implementation Class | | [duilib/Control/RichEditHost_Windows.h](../duilib/Control/RichEditHost_Windows.h) | Main function implementation of the rich text edit box (Windows) |
| RichEdit Implementation Class | | [duilib/Control/RichEdit_SDL.h](../duilib/Control/RichEdit_SDL.h) | Main function wrapper of the rich text edit box (SDL) |
| RichText | Control | [duilib/Control/RichText.h](../duilib/Control/RichText.h) | Formatted text (HTML-like format) |
| Split | Control | [duilib/Control/Split.h](../duilib/Control/Split.h) | Splitter control |
| SplitBox | Box | [duilib/Control/Split.h](../duilib/Control/Split.h) | Splitter container |
| TabCtrl | ListBox | [duilib/Control/TabCtrl.h](../duilib/Control/TabCtrl.h) | Multi-tab control (similar to the multi-tabs of a browser) |
| TreeView | ListBox | [duilib/Control/TreeView.h](../duilib/Control/TreeView.h) | Tree control |
| TreeNode | ListBoxItem | [duilib/Control/TreeView.h](../duilib/Control/TreeView.h) | Node of the tree control |
| DirectoryTree | TreeView | [duilib/Control/DirectoryTree.h](../duilib/Control/DirectoryTree.h) | Directory tree control, used to display the directory structure of the file system |
| ListCtrl | VBox | [duilib/Control/ListCtrl.h](../duilib/Control/ListCtrl.h) | List control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlDefs.h](../duilib/Control/ListCtrlDefs.h) | Basic type definition of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlHeader.h](../duilib/Control/ListCtrlHeader.h) | Header of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlHeaderItem.h](../duilib/Control/ListCtrlHeaderItem.h) | Header sub-item of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlItem.h](../duilib/Control/ListCtrlItem.h) | Data item of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlSubItem.h](../duilib/Control/ListCtrlSubItem.h) | Sub-item of the data item of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlView.h](../duilib/Control/ListCtrlView.h) | View base class of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlReportView.h](../duilib/Control/ListCtrlReportView.h) | Report view of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlIconView.h](../duilib/Control/ListCtrlIconView.h) | Icon/List view of the list control |
| ListCtrl Implementation Class | | [duilib/Control/ListCtrlData.h](../duilib/Control/ListCtrlData.h) | Data manager of the list control |
| PropertyGrid | VBox | [duilib/Control/PropertyGrid.h](../duilib/Control/PropertyGrid.h) | Property grid control, supports text, number, check box, font, color, date, IP address, hot key, file path, folder, and other attributes |
| ColorPicker | WindowImplBase | [duilib/Control/ColorPicker.h](../duilib/Control/ColorPicker.h) | Color picker, independent window |
| ColorPicker Implementation Class | | [duilib/Control/ColorControl.h](../duilib/Control/ColorControl.h) | Implementation class of ColorPicker; custom color control |
| ColorPicker Implementation Class | | [duilib/Control/ColorConvert.h](../duilib/Control/ColorConvert.h) | Implementation class of ColorPicker; color type (RGB/HSV/HSL) conversion class |
| ColorPicker Implementation Class | | [duilib/Control/ColorPickerCustom.h](../duilib/Control/ColorPickerCustom.h) | Implementation class of ColorPicker; custom color |
| ColorPicker Implementation Class | | [duilib/Control/ColorPickerRegular.h](../duilib/Control/ColorPickerRegular.h) | Implementation class of ColorPicker; common colors |
| ColorPicker Implementation Class | | [duilib/Control/ColorPickerStandard.h](../duilib/Control/ColorPickerStandard.h) | Implementation class of ColorPicker; standard colors |
| ColorPicker Implementation Class | | [duilib/Control/ColorPickerStandardGray.h](../duilib/Control/ColorPickerStandardGray.h) | Implementation class of ColorPicker; standard colors, gray |
| ColorPicker Implementation Class | | [duilib/Control/ColorSlider.h](../duilib/Control/ColorSlider.h) | Implementation class of ColorPicker |
| MessageBoxWnd | WindowImplBase | [duilib/Utils/MessageBoxWnd.h](../duilib/Utils/MessageBoxWnd.h) | Owner-drawn skin modal message box, independent window; see [MessageBox.en.md](MessageBox.en.md) for details |
| ToastWnd | WindowImplBase | [duilib/Utils/ToastWnd.h](../duilib/Utils/ToastWnd.h) | Owner-drawn skin non-modal toast notification, independent window, auto-dismiss and stackable; see [Toast.en.md](Toast.en.md) for details |
| ControlDragable | Control | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | Supports adjusting the order of child controls within the same Box by dragging |
| BoxDragable | Box | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | Supports adjusting the order of child controls within the same Box by dragging |
| HBoxDragable | HBox | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | Supports adjusting the order of child controls within the same Box by dragging |
| VBoxDragable | VBoxDragable | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | Supports adjusting the order of child controls within the same Box by dragging |
| IconControl | Control | [duilib/Control/IconControl.h](../duilib/Control/IconControl.h) | Control used to display icons; if icon data is not set, it is compatible with all functions of the base class Control |
| AddressBar | HBox | [duilib/Control/AddressBar.h](../duilib/Control/AddressBar.h) | Address bar control, used to display the path of the local file system |

* Global Resources

| Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- |
| GlobalManager | [duilib/Core/GlobalManager.h](../duilib/Core/GlobalManager.h) | Global attribute management utility class, used to manage some global attributes, including global styles (global.xml) and language settings, etc. |
| IRenderFactory | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | A management class for the render interface; manages the render interface, used to create render implementation objects such as Font, Pen, Brush, Path, Matrix, Bitmap, and Render |
| FontManager | [duilib/Core/FontManager.h](../duilib/Core/FontManager.h) | Font management class |
| ImageManager | [duilib/Core/ImageManager.h](../duilib/Core/ImageManager.h) | Image management class |
| IconManager | [duilib/Core/IconManager.h](../duilib/Core/IconManager.h) | Icon resource manager (thread-safe, suitable for small icon-like image resources), supports the HICON handle on the Windows platform |
| ZipManager | [duilib/Core/ZipManager.h](../duilib/Core/ZipManager.h) | ZIP archive manager |
| DpiManager | [duilib/Core/DpiManager.h](../duilib/Core/DpiManager.h) | DPI manager, used to support DPI adaptation and other functions |
| TimerManager | [duilib/Core/TimerManager.h](../duilib/Core/TimerManager.h) | Timer manager |
| LangManager | [duilib/Core/LangManager.h](../duilib/Core/LangManager.h) | Multi-language support manager |
| CursorManager | [duilib/Core/CursorManager.h](../duilib/Core/CursorManager.h) | Cursor manager |
| ThreadManager | [duilib/Core/ThreadManager.h](../duilib/Core/ThreadManager.h) | Thread manager |
| ColorManager | [duilib/Core/ColorManager.h](../duilib/Core/ColorManager.h) | Color management class |
| ThemeManager | [duilib/Core/ThemeManager.h](../duilib/Core/ThemeManager.h) | Theme management class |
| WindowManager | [duilib/Core/WindowManager.h](../duilib/Core/WindowManager.h) | Window management class |
| ImageDecoderFactory | [duilib/Image/ImageDecoderFactory.h](../duilib/Image/ImageDecoderFactory.h) | Image decoder management |

* libcef Control Encapsulation Related

| Class Name | Associated Header File | Purpose |
| :--- | :--- | :--- |
| CefManager | [duilib/CEFControl/CefManager.h](../duilib/CEFControl/CefManager.h) | CEF control manager, responsible for the initialization and deinitialization of the CEF module |
| CefControl | [duilib/CEFControl/CefControl.h](../duilib/CEFControl/CefControl.h) | CEF control interface, providing basic web browsing functions and event reception |
| CefControlEvent | [duilib/CEFControl/CefControlEvent.h](../duilib/CEFControl/CefControlEvent.h) | Event reception interface for web browsing related to the CEF control |
| CefControlNative | [duilib/CEFControl/CefControlNative.h](../duilib/CEFControl/CefControlNative.h) | Encapsulation of the window mode of the CEF control |
| CefControlOffScreen | [duilib/CEFControl/CefControlOffScreen.h](../duilib/CEFControl/CefControlOffScreen.h) | Encapsulation of the off-screen rendering mode of the CEF control |
