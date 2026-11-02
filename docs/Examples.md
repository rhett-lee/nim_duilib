简体中文 | [English](Examples.en.md)

## 示例程序
examples 目录中包含了一些示例程序，可以参考这些示例程序来完善自己的应用程序。

## 1. examples/basic
一个基础的窗口示例程序，带有最小化、最大化、还原和关闭按钮。    
macOS 下默认编译为 `bin/basic.app`，C++ 代码无需修改；配置方法见
[build/macos-app-bundle.md](../build/macos-app-bundle.md)。    
代码入口：[MainThread.cpp](../examples/basic/MainThread.cpp)、[MainForm.cpp](../examples/basic/MainForm.cpp)；界面资源：`bin/resources/themes/default/basic/basic.xml`；相关文档：[Getting-Started.md](Getting-Started.md)、[Window.md](Window.md)。    
<img width="700" src="./Images/basic.png"/>

## 2. examples/ColorPicker
演示如何使用拾色器控件的程序。    
代码入口：[MainThread.cpp](../examples/ColorPicker/MainThread.cpp)；界面资源：`bin/resources/themes/default/public/color/color_picker.xml`（拾色器是独立窗口，皮肤目录为 `public`）；相关文档：[Control.md](Control.md) 中的 ColorPicker 章节。    
<img width="700" src="./Images/ColorPicker.png"/>

## 3. examples/controls
一些常用控件的样式使用方法示例。    
代码入口：[MainThread.cpp](../examples/controls/MainThread.cpp)、[ControlForm.cpp](../examples/controls/ControlForm.cpp)；界面资源：`bin/resources/themes/default/controls/controls.xml`（同目录下还有各控件的示例 XML，如 `message_box_test.xml`、`toast_test.xml`、`tree_grid.xml` 等）；相关文档：[Control.md](Control.md)、[MessageBox.md](MessageBox.md)、[Toast.md](Toast.md)。    
<img width="900" src="./Images/controls.png"/>

## 4. examples/chat
包含一个登录窗口和一个模拟 PC 聊天布局的窗口示例。    
代码入口：[MainThread.cpp](../examples/chat/MainThread.cpp)、[ChatForm.cpp](../examples/chat/ChatForm.cpp)；界面资源：`bin/resources/themes/default/chat/wechat.xml`、`bin/resources/themes/default/chat/login.xml`；相关文档：[Box.md](Box.md)、[Control.md](Control.md)。    
<img width="800" src="./Images/chat.png"/>

## 5. examples/ListBox
演示如何使用ListBox容器的程序。    
代码入口：[MainThread.cpp](../examples/ListBox/MainThread.cpp)、[ListBoxForm.cpp](../examples/ListBox/ListBoxForm.cpp)；界面资源：`bin/resources/themes/default/list_box/list_box.xml`、`list_box/item.xml`；相关文档：[Control.md](Control.md)。    
<img width="600" src="./Images/ListBox.png"/>
    
## 6. examples/ListCtrl
演示如何使用ListCtrl控件。    
代码入口：[MainThread.cpp](../examples/ListCtrl/MainThread.cpp)、[MainForm.cpp](../examples/ListCtrl/MainForm.cpp)；界面资源：`bin/resources/themes/default/list_ctrl/list_ctrl.xml`（同目录下还有 `loading*.xml` 等加载状态示例）；相关文档：[Control.md](Control.md)。    
<img width="1000" src="./Images/ListCtrl.png"/>

## 7. examples/MoveControl
演示如何使用Box容器的"drag_out_id"和"drop_in_id"属性来实现控件在不同容器之间的移动效果。    
代码入口：[MainThread.cpp](../examples/MoveControl/MainThread.cpp)、[MoveControlForm.cpp](../examples/MoveControl/MoveControlForm.cpp)；界面资源：`bin/resources/themes/default/move_control/main.xml`、`move_control/app_item.xml`；相关文档：[Control.md](Control.md)、[XmlNode.md](XmlNode.md)。    
<img width="600" src="./Images/MoveControl.png"/>
   
## 8. examples/render
演示如何使用渲染引擎的程序，同时也包含了各种控件/容器的使用示例。    
代码入口：[MainThread.cpp](../examples/render/MainThread.cpp)、[RenderForm.cpp](../examples/render/RenderForm.cpp)；界面资源：`bin/resources/themes/default/render/render.xml`（同目录下还有 `page_*.xml` 各页面示例）；相关文档：[Control.md](Control.md)、[WindowShadow.md](WindowShadow.md)。    
<img width="900" src="./Images/render01.png"/>
<img width="900" src="./Images/render02.png"/>
<img width="900" src="./Images/render03.png"/>
<img width="900" src="./Images/render04.png"/>

## 9. examples/RichEdit
演示如何使用RichEdit控件。    
代码入口：[MainThread.cpp](../examples/RichEdit/MainThread.cpp)、[MainForm.cpp](../examples/RichEdit/MainForm.cpp)、[FindForm.cpp](../examples/RichEdit/FindForm.cpp)、[ReplaceForm.cpp](../examples/RichEdit/ReplaceForm.cpp)；界面资源：`bin/resources/themes/default/rich_edit/rich_edit.xml`、`rich_edit/find.xml`、`rich_edit/replace.xml`；相关文档：[Control.md](Control.md)。    
<img width="900" src="./Images/RichEdit.png"/>

## 10. examples/TreeView
演示如何使用TreeView控件。    
代码入口：[MainThread.cpp](../examples/TreeView/MainThread.cpp)、[MainForm.cpp](../examples/TreeView/MainForm.cpp)；界面资源：`bin/resources/themes/default/tree_view/tree_view.xml`、`tree_view/tree_node.xml`；相关文档：[Control.md](Control.md)。    
<img width="900" src="./Images/TreeView.png"/>
    
## 11. examples/VirtualListBox
演示如何使用基于虚表实现的ListBox容器。    
代码入口：[MainThread.cpp](../examples/VirtualListBox/MainThread.cpp)、[MainForm.cpp](../examples/VirtualListBox/MainForm.cpp)；界面资源：`bin/resources/themes/default/virtual_list_box/main.xml`、`virtual_list_box/item.xml`；相关文档：[Control.md](Control.md)。    
<img width="900" src="./Images/VirtualListBox.png"/>
    
## 12. examples/MultiLang
演示如何使用多语言功能，支持多语言的动态切换。    
代码入口：[MainThread.cpp](../examples/MultiLang/MainThread.cpp)、[MainForm.cpp](../examples/MultiLang/MainForm.cpp)；界面资源：`bin/resources/themes/default/MultiLang/MultiLang.xml`；相关文档：[Global.md](Global.md)。    
<img width="432" src="./Images/MultiLang.png"/>

## 13. examples/DpiAware
演示如何使用DPI感知功能，如何动态设置窗口的DPI。    
代码入口：[MainThread.cpp](../examples/DpiAware/MainThread.cpp)、[MainForm.cpp](../examples/DpiAware/MainForm.cpp)；界面资源：`bin/resources/themes/default/dpi_aware/DpiAware.xml`；相关文档：[Window.md](Window.md) 中的窗口 C++ 接口（DPI 相关）。    
<img width="700" src="./Images/DpiAware.png"/>

## 14. examples/threads
演示如何使用多线程功能，包含多线程通信等。    
代码入口：[MainThread.cpp](../examples/threads/MainThread.cpp)、[MainForm.cpp](../examples/threads/MainForm.cpp)、[WorkerThread.cpp](../examples/threads/WorkerThread.cpp)；界面资源：`bin/resources/themes/default/threads/threads.xml`；相关文档：[Getting-Started.md](Getting-Started.md) 中的“引入线程库”。    
<img width="800" src="./Images/threads.png"/>

## 15. examples/layout
演示容器的各种布局。    
代码入口：[MainThread.cpp](../examples/layout/MainThread.cpp)、[MainForm.cpp](../examples/layout/MainForm.cpp)；界面资源：`bin/resources/themes/default/layout/layout.xml`（同目录下的 `page_*.xml` 分别演示各种布局容器）；相关文档：[Box.md](Box.md)。    
<img width="1000" src="./Images/layout.png"/>

## 16. examples/cef
演示如何在使用 CEF 控件，包含 C++ 与 JavaScript 交互。    
代码入口：[MainThread.cpp](../examples/cef/MainThread.cpp)、[CefForm.cpp](../examples/cef/CefForm.cpp)；界面资源：`bin/resources/themes/default/cef/cef.xml`；相关文档：[CEF.md](CEF.md)。    
<img width="700" src="./Images/cef.png"/>

## 17. examples/CefBrowser
基于 CEF 的自定义多标签可拖曳浏览器示例。    
代码入口：[MainThread.cpp](../examples/CefBrowser/MainThread.cpp)、[BrowserForm.cpp](../examples/CefBrowser/browser/BrowserForm.cpp)、[DragForm.cpp](../examples/CefBrowser/browser/DragForm.cpp)；界面资源：`bin/resources/themes/default/cef_browser/cef_browser.xml`、`cef_browser/drag_form.xml`、`cef_browser/browser_box.xml`、`cef_browser/tab_item.xml`；相关文档：[CEF.md](CEF.md)。    
<img width="800" src="./Images/CefBrowser.png"/>
    
## 18. examples/WebView2
演示如何在使用 WebView2 控件。    
代码入口：[MainThread.cpp](../examples/WebView2/MainThread.cpp)、[MainForm.cpp](../examples/WebView2/MainForm.cpp)；界面资源：`bin/resources/themes/default/webview2/webview2.xml`；相关文档：[Control.md](Control.md) 中的 WebView2Control 章节。    
<img width="800" src="./Images/WebView2.png"/>

## 19. examples/WebView2Browser
基于 WebView2 的自定义多标签可拖曳浏览器示例。    
代码入口：[MainThread.cpp](../examples/WebView2Browser/MainThread.cpp)、[BrowserForm.cpp](../examples/WebView2Browser/BrowserForm.cpp)、[DragForm.cpp](../examples/WebView2Browser/DragForm.cpp)；界面资源：`bin/resources/themes/default/webview2_browser/webview2_browser.xml`、`webview2_browser/drag_form.xml`、`webview2_browser/browser_box.xml`、`webview2_browser/tab_item.xml`；相关文档：[Control.md](Control.md) 中的 WebView2Control 章节。    
<img width="800" src="./Images/WebView2Browser.png"/>
    
## 20. examples/ChildWindow    
子窗口控件的功能演示示例。    
代码入口：[MainThread.cpp](../examples/ChildWindow/MainThread.cpp)、[MainForm.cpp](../examples/ChildWindow/MainForm.cpp)；界面资源：`bin/resources/themes/default/child_window/child_window.xml`、`child_window/child_window_item.xml`；相关文档：[Control.md](Control.md) 中的 ChildWindow 章节。    
<img width="800" src="./Images/ChildWindow.png"/>
    
## 21. examples/XmlPreview    
XML文件的界面效果预览功能演示示例。    
代码入口：[MainThread.cpp](../examples/XmlPreview/MainThread.cpp)、[MainForm.cpp](../examples/XmlPreview/MainForm.cpp)；界面资源：`bin/resources/themes/default/xml_preview/xml_preview.xml`；相关文档：[XmlNode.md](XmlNode.md)、[XmlEvents.md](XmlEvents.md)。    
<img width="800" src="./Images/XmlPreview.png"/>
    
## 22. examples/ColorTheme    
主题颜色预览与生成功能演示示例。    
代码入口：[MainThread.cpp](../examples/ColorTheme/MainThread.cpp)、[MainForm.cpp](../examples/ColorTheme/MainForm.cpp)；界面资源：`bin/resources/themes/default/color_theme/color_theme.xml`、`color_theme/flyout_demo.xml`；相关文档：[Theme.md](Theme.md)、[ThemeColor.md](ThemeColor.md)。    
<img width="800" src="./Images/ColorTheme.png"/>

## 23. examples/panel
演示 Panel/PanelHBox/PanelVBox 带标题栏面板容器的用法，包含：标题栏样式、点击标题栏折叠/展开（支持动画）、手风琴分组、折叠触发热区、自定义箭头状态图片、箭头左置与标题对齐、标题栏槽位、折叠前取消事件，以及通过 C++ 代码控制面板状态。    
代码入口：[MainThread.cpp](../examples/panel/MainThread.cpp)、[PanelForm.cpp](../examples/panel/PanelForm.cpp)；界面资源：`bin/resources/themes/default/panel/panel.xml`；相关文档：[Box.md](Box.md) 中的 Panel/PanelHBox/PanelVBox 章节。    
<img width="900" src="./Images/panel.png"/>
