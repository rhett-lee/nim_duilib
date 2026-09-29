English | [简体中文](Window.md)

> Last synced: 2026-09-29

## Window Attributes



| Attribute name          | Attribute category| Default value  |Parameter type| Function in [Window.h](../duilib/Core/Window.h) | Purpose |



| :---              | :---    | :---    | :---   |:---                     |:--- |



| caption           | Title bar  | "0,0,0,0" | rect   | SetCaptionRect          | Margin of the draggable title bar area of the window; the last parameter is the distance from the top border, e.g. "0,0,0,36" |



| use_system_caption| Title bar  | false   | bool   | SetUseSystemCaption     | Set whether to use the system title bar |



| snap_layout_menu  | Title bar  | true    | bool   | SetEnableSnapLayoutMenu | Whether to support displaying the snap layout menu (new Windows 11 feature: by hovering the mouse over the window's maximize button or pressing Win + Z, aligned layouts can be easily accessed). This attribute is enabled by default only when the window is a layered window or uses the system shadow |



| sys_menu          | Title bar  | true    | bool   | SetEnableSysMenu        | Whether to display the system window menu when right-clicking the title bar (can adjust window state, close window, etc.)|



| sys_menu_rect     | Title bar  | "0,0,0,0" | rect   | SetSysMenuRect          | Window menu area, at the top-left corner of the window title bar (double-clicking this area closes the window, clicking displays the system window menu). If not set in XML, this feature is disabled by default |



| icon              | Title bar  |         | string | SetWindowIcon           | Set the window icon file path, supports ico format |



| text              | Title bar  |         | string | SetText                 | Window title string|



| textid            | Title bar  |         | string | SetTextId               | ID of the window title string; the ID is specified in the multilingual file, e.g. "STRID_MIANWINDOW_TITLE" |



| drag_drop         | Drag and drop    | true    | bool   | SetEnableDragDrop       | Set whether the controls inside the window support drag-and-drop operations (drop in files/text): true = supported, false = not supported |



| shadow_attached   | Window shadow| true    | bool   | SetShadowAttached       | Whether the window has an attached shadow effect; "true" means enable the window shadow function, "false" means disable it<br>If the shadow_type attribute is set, it implicitly sets shadow_attached to "true"<br>So generally after setting shadow_type, there is no need to set shadow_attached separately|



| shadow_type       | Window shadow|         | string | SetShadowType           | Set the window shadow type:<br> "default": default self-drawn shadow <br> "big": self-drawn shadow (large), right angle, with border (suitable for normal windows)<br> "big_round": self-drawn shadow (large), rounded corner, with border (suitable for normal windows)<br> "small": self-drawn shadow (small), right angle, with border (suitable for normal windows)<br> "small_round": self-drawn shadow (small), rounded corner, with border (suitable for normal windows)<br> "menu": self-drawn shadow (small), right angle, with border (suitable for popup windows, such as menus)<br> "menu_round": self-drawn shadow (small), rounded corner, with border (suitable for popup windows, such as menus)<br> "none": no shadow, right angle, with border<br> "none_round": no shadow, rounded corner, with border (self-drawn)<br> "custom": custom self-drawn shadow; this type requires setting `shadow_image`, `shadow_corner`, `shadow_border_round` at the same time <br> "system_default": use the system shadow, following the OS default window shadow properties<br> "system_not_round": use the system shadow, right angle<br> "system_round": use the system shadow, rounded corner<br> "system_small_round": use the system shadow, small rounded corner<br>Compatibility notes:<br>(1) For Win7 and Win10 systems: only "system_default" is supported among the system shadows; the other three are not supported<br>(2) For Win11 systems, all four system shadow attributes are supported, and the default system shadow is rounded corner<br>(3) For macOS systems, all four system shadow attributes are supported, and the default system shadow is rounded corner<br>(4) For Linux and FreeBSD systems, system shadows are not supported<br>(5) If an unsupported system shadow is set, it will automatically switch to the corresponding supported system shadow or self-drawn shadow|



| shadow_image      | Window shadow|         | string | SetShadowImage          | Use a custom shadow image to replace the default shadow effect; this attribute is generally only set when shadow_type="custom"<br>Note the relative path and the nine-grid (9-slice) attributes for the set path<br>e.g. (file='public/shadow/shadow_big.svg' corner='64,64,68,70') |



| shadow_corner     | Window shadow| "0,0,0,0" | rect   | SetShadowCorner       | After setting the shadow_image attribute, set this attribute to specify the nine-grid description of the shadow material; this attribute is generally only set when shadow_type="custom" |



| shadow_border_round| Window shadow| "0,0"    | size  | SetShadowBorderRound| After setting the shadow_image attribute, set this attribute to specify the rounded-corner attribute of the shadow; this attribute is generally only set when shadow_type="custom" |



| shadow_border_color| Window shadow|          |string | SetShadowBorderColor  | Set the border color of the window shadow |



| shadow_border_size | Window shadow|2         |int    | SetShadowBorderSize   | Set the border pixel size of the window shadow; the actually displayed border width is half of this value (e.g. set to 2, actually displays 1 pixel) |



| shadow_snap        | Window shadow| true     | bool  | SetEnableShadowSnap   | Set whether the shadow supports the window snap-to-edge operation; if true, when the window is close to the screen edge, the shadow on that side is automatically hidden to increase the effective space within the view<br>This attribute is only effective when using self-drawn shadow, and is invalid when using system shadow|



| size              | Window size| "0,0"     | size   | SetInitSize             | Initial size of the window; supported formats: size="1200,800", or size="50%,50%", or size="1200,50%", size="50%,800"; the percentage refers to the percentage of the screen width or height |



| size_contain_shadow| Window size| false  | bool   |    | Whether the window's initial size (size attribute) includes the window shadow; by default it does not, and the actual window size is the configured size value + shadow size |



| min_size          | Window size| "0,0"     | size   | SetWindowMinimumSize    | Minimum window size, e.g. "320,240" |



| max_size          | Window size| "0,0"     | size   | SetWindowMaximumSize    | Maximum window size, e.g. "1600,1200" |



| size_box          | Window size| "0,0,0,0" | rect   | SetSizeBox              | Margin of the window area that can be dragged to resize, e.g. "4,4,4,4" |



| round_corner      | Window shape| "0,0"     | size   | SetRoundCorner          | Window corner radius, e.g. "4,4"; this attribute generally does not need to be set when using window shadow (i.e. when the shadow_type attribute is set) |



| layered_window    | Window rendering| false   | bool   | SetLayeredWindow        | Set whether it is a layered window. Usage notes:<br>(1) This attribute does not need to be set when using window shadow (i.e. when the shadow_type attribute is set); the window shadow function will manage this attribute as needed<br>(2) When using the system title bar (i.e. use_system_caption="true"), this attribute should not be set |



| alpha             | Window rendering| 255     | int    | SetLayeredWindowAlpha   | Set the transparency value [0, 255]; when alpha is 0, the window is completely transparent. When alpha is 255, the window is opaque.<br>Only effective when layered_window="true"<br>This parameter is used as an argument in the UpdateLayeredWindow function (BLENDFUNCTION.SourceConstantAlpha)|



| opacity           | Window rendering| 255     | int    | SetLayeredWindowOpacity | Set the opacity value [0, 255]; when opacity is 0, the window is completely transparent. When opacity is 255, the window is opaque.<br>Only effective when layered_window="true", so if it is not currently a layered window, it will be automatically set to a layered window internally<br>This parameter is used as an argument in the SetLayeredWindowAttributes function (bAlpha)|



| render_backend_type|Window rendering| "CPU"   | string |SetRenderBackendType     | "CPU": CPU rendering <br> "GL": use OpenGL rendering <br> Notes:<br>(1) Within one thread, only one window is allowed to use OpenGL rendering; otherwise it will cause the program to crash<br>(2) A window rendered with OpenGL cannot be a layered window (i.e. a window with the WS_EX_LAYERED attribute), so when using GL rendering, the `layered_window`, `alpha`, `opacity` attributes are all invalid<br>(3) A window using OpenGL is redrawn entirely each time, and does not support partial drawing, so its performance is not necessarily better than using CPU rendering |







Note: for content related to window shadows, please refer to the document: [WindowShadow.en.md](WindowShadow.en.md)    



Note: the parsing function for window attributes can be found in: [WindowBuilder::ParseWindowAttributes function](../duilib/Core/WindowBuilder.cpp)    



Note: the tag name of the window in XML is: "Window"    



Usage example:    



```xml



<Window size="75%,90%" min_size="80,50" size_box="4,4,4,4"



        caption="0,0,0,36" shadow_type="system_default">



</Window>



```



