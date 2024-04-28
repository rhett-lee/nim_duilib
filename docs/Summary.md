## 项目关联内容链接

* [项目简介](../README.md)
* [快速上手](Getting-Started.md)
* [示例程序](Examples.md)
* [全局样式：字体、颜色、图片等资源](Global.md)
* [窗口的样式](Window.md)
* [容器的样式](Box.md)
* [控件的样式](Control.md)
* [菜单的样式](Menu.md)
* [XML文件中各控件的节点名称](XmlNode.md)

## 项目中主要类的简表
各个类的详细接口说明请参考关联头文件，有较详细的注释。    
* 基本数据类型
| 类名称 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| UiSize | [duilib/Core/UiSize.h](../duilib/Core/UiSize.h) | 32位Size类型的封装|
| UiSize64 | [duilib/Core/UiSize64.h](../duilib/Core/UiSize64.h) | 64位Size类型的封装|
| UiPoint | [duilib/Core/UiPoint.h](../duilib/Core/UiPoint.h) | 整型Point类型的封装|
| UiPointF | [duilib/Core/UiPointF.h](../duilib/Core/UiPointF.h) | 浮点型Point类型的封装|
| UiFixedInt | [duilib/Core/UiFixedInt.h](../duilib/Core/UiFixedInt.h) | 控件设置长度（或宽度）数值类型的封装|
| UiEstInt | [duilib/Core/UiEstInt.h](../duilib/Core/UiEstInt.h) | 控件估算长度（或宽度）数值类型的封装|
| UiFixedSize | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | 设置的控件大小|
| UiEstSize | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | 估算的控件大小（相比UiFixedSize，没有Auto类型）|
| UiEstResult | [duilib/Core/UiTypes.h](../duilib/Core/UiTypes.h) | 估算控件大小的结果|
| UiPadding | [duilib/Core/UiPadding.h](../duilib/Core/UiPadding.h) | 内边距类型的封装|
| UiMargin | [duilib/Core/UiMargin.h](../duilib/Core/UiMargin.h) | 外边距类型的封装|
| UiString | [duilib/Core/UiString.h](../duilib/Core/UiString.h) | 控件使用的字符串，用于替代std::wstring，以减少控件的内存占用，该类适合用于较低的内存空间来存储字符串，性能方面由于字符串复制偏多，性能偏弱|
| UiRect | [duilib/Core/UiRect.h](../duilib/Core/UiRect.h) | 32位Rect类型的封装|
| UiFont | [duilib/Core/UiFont.h](../duilib/Core/UiFont.h) | 字体类型的封装|
| UiColor | [duilib/Core/UiColor.h](../duilib/Core/UiColor.h) | 颜色类型的封装|
| UiColors | [duilib/Core/UiColors.h](../duilib/Core/UiColors.h) | 常见颜色值常量(ARGB格式)|

* 窗口相关
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| Window | [duilib/Core/Window.h](../duilib/Core/Window.h) | duilib核心窗口的封装|
| WindowBuilder | [duilib/Core/WindowBuilder.h](../duilib/Core/WindowBuilder.h) | 解析XML文件，并创建窗口、容器布局和控件等|
| Shadow | [duilib/Core/Shadow.h](../duilib/Core/Shadow.h) | 窗口阴影属性|
| WindowImplBase | [duilib/Utils/WinImplBase.h](../duilib/Utils/WinImplBase.h) | 提供给应用层的窗口基类，实现一个带有标题栏的窗体，带有最大化、最小化、还原按钮的支持，提供带有 WS_EX_LAYERED 属性窗口阴影|
| ShadowWnd | [duilib/Utils/ShadowWnd.h](../duilib/Utils/ShadowWnd.h) | 是WinImplBase的子类，使用附加阴影窗口实现的基类，实现了创建窗口并附加的功能，提供没有 WS_EX_LAYERED 属性的窗口阴影|
| EventType | [duilib/duilib_defs.h](../duilib/duilib_defs.h) | 定义所有消息类型|
| EventArgs | [duilib/Core/EventArgs.h](../duilib/Core/EventArgs.h) | 窗口消息与控件事件通知的参数|
| WindowDropTarget | [duilib/Core/WindowDropTarget.h](../duilib/Core/WindowDropTarget.h) | 窗口中控件的拖放支持|
| Windows版本兼容性 | [duilib/duilib_config.h](../duilib/duilib_config.h) | Windows版本兼容性定义，默认配置兼容Win7以上操作系统|

* 控件与容器的基类
| 类名称 | 基类名称 |关联头文件| 用途 |
| :--- | :--- |:--- | :--- |
| PlaceHolder |nbase::SupportWeakCallback| [duilib/Core/PlaceHolder.h](../duilib/Core/PlaceHolder.h) | 所有控件和容器的顶层基类，封装了控件的位置、大小、外观形状等基本数据|
| Control | PlaceHolder | [duilib/Core/Control.h](../duilib/Core/Control.h) | 一个基本的控件，也是所有控件和容器的基类，包含了控件的位置、大小、状态、颜色、图片资源、动画、绘制、光标、鼠标、键盘、焦点、快捷键操作等基本功能|
| Box | Control| [duilib/Core/Box.h](../duilib/Core/Box.h) | 所有容器的基类，封装了容器的位置、大小、外观形状、子控件管理（添加、删除、修改、绘制、操作、拖放、布局等）|

* 布局
| 类名称 | 基类名称 |关联头文件| 用途 |
| :--- | :--- |:--- | :--- |
| Layout | |[duilib/Box/Layout.h](../duilib/Box/Layout.h) | 自由布局（浮动布局）|
| HLayout |Layout |[duilib/Box/HLayout.h](../duilib/Box/HLayout.h) | 水平布局|
| VLayout |Layout |[duilib/Box/VLayout.h](../duilib/Box/VLayout.h) | 垂直布局|
| HTileLayout |Layout| [duilib/Box/HTileLayout.h](../duilib/Box/HTileLayout.h) | 水平瓦片布局|
| VTileLayout |Layout| [duilib/Box/VTileLayout.h](../duilib/Box/VTileLayout.h) | 垂直瓦片布局|
| VirtualHLayout |HLayout| [duilib/Box/VirtualHLayout.h](../duilib/Box/VirtualHLayout.h) | 虚表水平布局|
| VirtualVLayout |VLayout| [duilib/Box/VirtualVLayout.h](../duilib/Box/VirtualVLayout.h) | 虚表垂直布局|
| VirtualHTileLayout |HTileLayout| [duilib/Box/VirtualHTileLayout.h](../duilib/Box/VirtualHTileLayout.h) | 虚表水平瓦片布局|
| VirtualVTileLayout |VTileLayout| [duilib/Box/VirtualVTileLayout.h](../duilib/Box/VirtualVTileLayout.h) | 虚表垂直瓦片布局|

* 容器
| 类名称 | 基类名称 |布局类型|关联头文件| 名称/用途 |
| :--- | :--- |:--- |:--- | :--- |
| Box | Control|[Layout](../duilib/Box/Layout.h)|[duilib/Core/Box.h](../duilib/Core/Box.h) | 自由布局容器，所有容器的基类，封装了容器的位置、大小、外观形状、子控件管理（添加、删除、修改、绘制、操作、拖放、布局等）|
| VBox | Box|[VLayout](../duilib/Box/VLayout.h)| [duilib/Box/VBox.h](../duilib/Box/VBox.h) | 垂直布局容器 |
| HBox | Box|[HLayout](../duilib/Box/HLayout.h)|[duilib/Box/HBox.h](../duilib/Box/HBox.h) | 水平布局容器 |
| TabBox | Box|[Layout](../duilib/Box/Layout.h)|[duilib/Box/TabBox.h](../duilib/Box/TabBox.h) | 多标签布局容器 |
| VTileBox | Box|[VTileLayout](../duilib/Box/VTileLayout.h)|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | 垂直瓦片布局容器|
| HTileBox | Box|[HTileLayout](../duilib/Box/HTileLayout.h)|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | 水平瓦片布局容器|
| ScrollBox | Box|[Layout](../duilib/Box/Layout.h)|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 自由布局可滚动容器，带有垂直或水平滚动条|
| VScrollBox | ScrollBox|[VLayout](../duilib/Box/VLayout.h)|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 垂直布局可滚动容器，带有垂直或水平滚动条|
| HScrollBox | ScrollBox|[HLayout](../duilib/Box/HLayout.h)|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 水平布局可滚动容器，带有垂直或水平滚动条|
| VTileScrollBox |ScrollBox|[VTileLayout](../duilib/Box/VTileLayout.h)|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 垂直瓦片布局可滚动容器，带有垂直或水平滚动条|
| HTileScrollBox |ScrollBox|[HTileLayout](../duilib/Box/HTileLayout.h)|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 水平瓦片布局可滚动容器，带有垂直或水平滚动条|
| ListBox | ScrollBox|[Layout](../duilib/Box/Layout.h)|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 自由布局的列表容器|
| VListBox | ListBox|[VLayout](../duilib/Box/VLayout.h)|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 垂直布局的列表容器|
| HListBox | ListBox|[HLayout](../duilib/Box/HLayout.h)|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 水平布局的列表容器|
| VTileListBox | ListBox|[VTileLayout](../duilib/Box/VTileLayout.h)|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 垂直瓦片布局的列表容器|
| HTileListBox | ListBox|[HTileLayout](../duilib/Box/HTileLayout.h)|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 水平布局的列表容器|
| VirtualListBox | ListBox|[Layout](../duilib/Box/Layout.h)|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 虚表实现的自由布局的列表容器|
| VirtualVListBox | VirtualListBox|[VirtualVLayout](../duilib/Box/VirtualVLayout.h)|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 虚表实现的垂直布局的列表容器|
| VirtualHListBox | VirtualListBox|[VirtualHLayout](../duilib/Box/VirtualHLayout.h)|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 虚表实现的水平布局的列表容器|
| VirtualVTileListBox | VirtualListBox|[VirtualVTileLayout](../duilib/Box/VirtualVTileLayout.h)|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 虚表实现的垂直瓦片布局的列表容器|
| VirtualHTileListBox | VirtualListBox|[VirtualHTileLayout](../duilib/Box/VirtualHTileLayout.h)|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 虚表实现的水平布局的列表容器|

* 图片
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| Image | [duilib/Image/Image.h](../duilib/Image/Image.h) | 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO|
| ImageAttribute | [duilib/Image/ImageAttribute.h](../duilib/Image/ImageAttribute.h) | 图片属性|
| ImageLoadAttribute | [duilib/Image/ImageLoadAttribute.h](../duilib/Image/ImageLoadAttribute.h) | 图片加载属性，用于加载一个图片|
| ImageDecoder | [duilib/Image/ImageDecoder.h](../duilib/Image/ImageDecoder.h) | 图片格式解码类，负责图片的加载，图片大小调整等|
| ImageInfo | [duilib/Image/ImageInfo.h](../duilib/Image/ImageInfo.h) | 图片信息|
| StateImage | [duilib/Image/StateImage.h](../duilib/Image/StateImage.h) | 控件状态与图片的映射|
| StateImageMap | [duilib/Image/StateImageMap.h](../duilib/Image/StateImageMap.h) | 控件图片类型与状态图片的映射|
| ImageGif | [duilib/Image/ImageGif.h](../duilib/Image/ImageGif.h) | 控件背景动态播放动画的逻辑封装（支持GIF/WebP/APNG动画）|
| ImageList | [duilib/Core/ImageList.h](../duilib/Core/ImageList.h) | 图片列表|
| ImageManager | [duilib/Core/ImageManager.h](../duilib/Core/ImageManager.h) | 图片资源管理器|
| IconManager | [duilib/Core/IconManager.h](../duilib/Core/IconManager.h) | HICON句柄管理器|

* 动画
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| AnimationManager | [duilib/Animation/AnimationManager.h](../duilib/Animation/AnimationManager.h) | 图片动画管理器|
| AnimationPlayer | [duilib/Animation/AnimationPlayer.h](../duilib/Animation/AnimationPlayer.h) | 图片动画播放状态管理|

* 颜色
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| UiColor | [duilib/Core/UiColor.h](../duilib/Core/UiColor.h) | 颜色类型的封装|
| UiColors | [duilib/Core/UiColors.h](../duilib/Core/UiColors.h) | 常见颜色值常量(ARGB格式)|
| StateColorMap | [duilib/Core/StateColorMap.h](../duilib/Core/UiColors.h) | 控件状态与颜色值的映射|

* 字体
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| UiFont | [duilib/Core/UiFont.h](../duilib/Core/UiFont.h) | 字体类型的封装|
| FontManager | [duilib/Core/FontManager.h](../duilib/Core/FontManager.h) | 字体管理器|

* 渲染引擎接口
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| 渲染引擎设置 | [duilib/Render/RenderConfig.h](../duilib/Render/RenderConfig.h) | 可设置使用哪个引擎（Skia引擎，或者GdiPlus引擎）|
| IRenderFactory | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 渲染工厂接口，用于创建Font、Pen、Brush、Path、Matrix、Bitmap、Render等渲染实现对象 |
| IFont | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 字体接口 |
| IBitmap | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 位图接口 |
| IPen | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 画笔接口 |
| IBrush | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 画刷接口 |
| IPath | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 路径接口 |
| IMatrix | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 矩阵接口 |
| IRender | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 渲染接口，用于画图、绘制文字等 |

* GdiPlus渲染引擎(略，不推荐使用。仅旧功能支持此引擎，新功能均不再支持该引擎，后续不再维护）

* Skia渲染引擎
| 类名称/说明 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| RenderFactory_Skia | [duilib/RenderSkia/RenderFactory_Skia.h](../duilib/RenderSkia/RenderFactory_Skia.h) | 渲染工厂接口的实现 |
| Font_Skia | [duilib/RenderSkia/Font_Skia.h](../duilib/RenderSkia/Font_Skia.h) | 字体接口的实现 |
| Bitmap_Skia | [duilib/RenderSkia/Bitmap_Skia.h](../duilib/RenderSkia/Bitmap_Skia.h) | 位图接口的实现 |
| Pen_Skia | [duilib/RenderSkia/Pen_Skia.h](../duilib/RenderSkia/Pen_Skia.h) | 画笔接口的实现 |
| Brush_Skia | [duilib/RenderSkia/Brush_Skia.h](../duilib/RenderSkia/Brush_Skia.h) | 画刷接口的实现 |
| Path_Skia | [duilib/RenderSkia/Path_Skia.h](../duilib/RenderSkia/Path_Skia.h) | 路径接口的实现 |
| Matrix_Skia | [duilib/RenderSkia/Matrix_Skia.h](../duilib/RenderSkia/Matrix_Skia.h) | 矩阵接口的实现 |
| Render_Skia | [duilib/RenderSkia/Render_Skia.h](../duilib/RenderSkia/Render_Skia.h) | 渲染接口的实现，用于画图、绘制文字等 |

* 控件/功能组件
| 类名称/功能组件 | 基类 | 关联头文件| 用途 |
| :--- | :--- | :---| :--- |
| ScrollBar | Control| [duilib/Core/ScrollBar.h](../duilib/Core/ScrollBar.h) | 滚动条控件 |
| Label | Control| [duilib/Control/Label.h](../duilib/Control/Label.h) | 标签控件（模板），用于显示文本 |
| LabelBox | Box| [duilib/Control/Label.h](../duilib/Control/Label.h) | 标签容器（模板），用于显示文本 |
| Button | Control| [duilib/Control/Button.h](../duilib/Control/Button.h) | 按钮控件（模板实现） |
| ButtonBox | Box| [duilib/Control/Button.h](../duilib/Control/Button.h) | 按钮容器控件（模板实现） |
| CheckBox | Control| [duilib/Control/CheckBox.h](../duilib/Control/CheckBox.h) | 复选框控件（模板实现） |
| CheckBoxBox | Box| [duilib/Control/CheckBox.h](../duilib/Control/CheckBox.h) | 复选框容器（模板实现） |
| Option | Control| [duilib/Control/Option.h](../duilib/Control/Option.h) | 单选按钮控件|
| OptionBox | Box| [duilib/Control/Option.h](../duilib/Control/Option.h) | 单选按钮容器|
| GroupBox | Box| [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | 分组容器（模板） |
| GroupVBox | VBox| [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | 垂直分组容器（模板） |
| GroupHBox | HBox| [duilib/Control/GroupBox.h](../duilib/Control/GroupBox.h) | 水平分组容器（模板） |
| Combo | Box| [duilib/Control/Combo.h](../duilib/Control/Combo.h) | 组合框 |
| ComboButton | Box| [duilib/Control/ComboButton.h](../duilib/Control/ComboButton.h) | 带有下拉组合框的按钮 |
| CheckCombo | Control| [duilib/Control/CheckCombo.h](../duilib/Control/CheckCombo.h) | 带复选框的组合框 |
| FilterCombo | Combo| [duilib/Control/FilterCombo.h](../duilib/Control/FilterCombo.h) | 带有过滤功能的组合框 |
| DateTime | Label| [duilib/Control/DateTime.h](../duilib/Control/DateTime.h) | 日期时间选择控件 |
| HotKey | HBox| [duilib/Control/HotKey.h](../duilib/Control/HotKey.h) | 热键控件 |
| HyperLink | Label| [duilib/Control/HyperLink.h](../duilib/Control/HyperLink.h) | 带有超级链接的文字, 如果URL为空的话，可以当作普通的文字按钮使用 |
| IPAddress | HBox| [duilib/Control/IPAddress.h](../duilib/Control/IPAddress.h) | IP地址控件 |
| Line | Control| [duilib/Control/Line.h](../duilib/Control/Line.h) | 画线控件 |
| CMenuWnd | WindowImplBase| [duilib/Control/Menu.h](../duilib/Control/Menu.h) | 菜单，独立窗口 |
| Progress | Label| [duilib/Control/Progress.h](../duilib/Control/Progress.h) | 进度条控件 |
| Slider | Progress| [duilib/Control/Slider.h](../duilib/Control/Slider.h) | 滑块控件 |
| CircleProgress | Control| [duilib/Control/CircleProgress.h](../duilib/Control/CircleProgress.h) | 环形进度条 |
| RichEdit | ScrollBox| [duilib/Control/RichEdit.h](../duilib/Control/RichEdit.h) | 富文本编辑框控件 |
| RichEdit实现类 | | [duilib/Control/RichEditCtrl.h](../duilib/Control/RichEditCtrl.h) | 富文本编辑框的主要功能封装 |
| RichEdit实现类 | | [duilib/Control/RichEditHost.h](../duilib/Control/RichEditHost.h) | 富文本编辑框的主要功能实现 |
| RichText | Control| [duilib/Control/RichText.h](../duilib/Control/RichText.h) | 格式化文本（类HTML格式） |
| Split | Control| [duilib/Control/Split.h](../duilib/Control/Split.h) | 分割条控件 |
| SplitBox | Box| [duilib/Control/Split.h](../duilib/Control/Split.h) | 分割条容器 |
| TabCtrl | ListBox| [duilib/Control/TabCtrl.h](../duilib/Control/TabCtrl.h) | 多标签控件（类似浏览器的多标签） |
| TreeView | ListBox| [duilib/Control/TreeView.h](../duilib/Control/TreeView.h) | 树控件 |
| TreeNode | ListBoxItem| [duilib/Control/TreeView.h](../duilib/Control/TreeView.h) | 树控件的节点 |
| ListCtrl | VBox| [duilib/Control/ListCtrl.h](../duilib/Control/ListCtrl.h) | 列表控件 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlDefs.h](../duilib/Control/ListCtrlDefs.h) | 列表控件的基本类型定义 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlHeader.h](../duilib/Control/ListCtrlHeader.h) | 列表控件的表头 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlHeaderItem.h](../duilib/Control/ListCtrlHeaderItem.h) | 列表控件的表头子项 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlItem.h](../duilib/Control/ListCtrlItem.h) | 列表控件数据项 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlSubItem.h](../duilib/Control/ListCtrlSubItem.h) | 列表控件数据项的子项 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlView.h](../duilib/Control/ListCtrlView.h) | 列表控件视图基类 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlReportView.h](../duilib/Control/ListCtrlReportView.h) | 列表控件Report视图 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlIconView.h](../duilib/Control/ListCtrlIconView.h) | 列表控件Icon/List视图 |
| ListCtrl实现类 | | [duilib/Control/ListCtrlData.h](../duilib/Control/ListCtrlData.h) | 列表控件的数据管理器 |
| PropertyGrid | VBox| [duilib/Control/PropertyGrid.h](../duilib/Control/PropertyGrid.h) | 属性表控件，支持文本、数字、复选框、字体、颜色、日期、IP地址、热键、文件路径、文件夹等属性 |
| ColorPicker | WindowImplBase| [duilib/Control/ColorPicker.h](../duilib/Control/ColorPicker.h) | 拾色器，独立窗口 |
| 拾色器实现类 | | [duilib/Control/ColorControl.h](../duilib/Control/ColorControl.h) | ColorPicker的实现类，自定义颜色控件 |
| 拾色器实现类 | | [duilib/Control/ColorConvert.h](../duilib/Control/ColorConvert.h) | ColorPicker的实现类，颜色类型（RGB/HSV/HSL）转换类 |
| 拾色器实现类 | | [duilib/Control/ColorPickerCustom.h](../duilib/Control/ColorPickerCustom.h) | ColorPicker的实现类，自定义颜色 |
| 拾色器实现类 | | [duilib/Control/ColorPickerRegular.h](../duilib/Control/ColorPickerRegular.h) | ColorPicker的实现类，常用颜色 |
| 拾色器实现类 | | [duilib/Control/ColorPickerStatard.h](../duilib/Control/ColorPickerStatard.h) | ColorPicker的实现类，标准颜色 |
| 拾色器实现类 | | [duilib/Control/ColorPickerStatardGray.h](../duilib/Control/ColorPickerStatardGray.h) | ColorPicker的实现类，标准颜色，灰色 |
| 拾色器实现类 | | [duilib/Control/ColorSlider.h](../duilib/Control/ColorSlider.h) | ColorPicker的实现类 |
| ControlDragable | Control | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | 支持相同Box内的子控件通过拖动来调整顺序 |
| BoxDragable | Box | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | 支持相同Box内的子控件通过拖动来调整顺序 |
| HBoxDragable | HBox | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | 支持相同Box内的子控件通过拖动来调整顺序 |
| VBoxDragable | VBoxDragable | [duilib/Control/ControlDragable.h](../duilib/Control/ControlDragable.h) | 支持相同Box内的子控件通过拖动来调整顺序 |

* 全局资源
| 类名称 | 关联头文件| 用途 |
| :--- | :--- | :--- |
| GlobalManager | [duilib/Core/GlobalManager.h](../duilib/Core/GlobalManager.h) | 全局属性管理工具类，用于管理一些全局属性的工具类，包含全局样式（global.xml）和语言设置等 |
| IRenderFactory | [duilib/Render/IRender.h](../duilib/Render/IRender.h) | 渲染接口的管理类，渲染接口管理，用于创建Font、Pen、Brush、Path、Matrix、Bitmap、Render等渲染实现对象 |
| FontManager | [duilib/Core/FontManager.h](../duilib/Core/FontManager.h) | 字体的管理类 |
| ImageManager | [duilib/Core/ImageManager.h](../duilib/Core/ImageManager.h) | 图片的管理类 |
| IconManager | [duilib/Core/IconManager.h](../duilib/Core/IconManager.h) | HICON句柄管理器 |
| ZipManager | [duilib/Core/ZipManager.h](../duilib/Core/ZipManager.h) | ZIP压缩包管理器 |
| DpiManager | [duilib/Core/DpiManager.h](../duilib/Core/DpiManager.h) | DPI管理器，用于支持DPI自适应等功能 |
| TimerManager | [duilib/Core/TimerManager.h](../duilib/Core/TimerManager.h) | 定时器管理器 |
| LangManager | [duilib/Core/LangManager.h](../duilib/Core/LangManager.h) | 多语言支持管理器 |
