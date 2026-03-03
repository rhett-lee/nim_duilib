## 窗口的属性
| 属性名称          | 属性分类| 默认值  |参数类型| 关联[Window.h](../duilib/Core/Window.h)中的函数| 用途 |
| :---              | :---    | :---    | :---   |:---                     |:--- |
| caption           | 标题栏  | 0,0,0,0 | rect   | SetCaptionRect          |窗口可拖动的标题栏大小的边距, 最后一个参数是指离上边框的距离,如(0,0,0,36) |
| use_system_caption| 标题栏  | false   | bool   | SetUseSystemCaption     |设置是否使用系统标题栏 |
| snap_layout_menu  | 标题栏  | true    | bool   | SetEnableSnapLayoutMenu |是否支持显示贴靠布局菜单（Windows 11新功能：通过将鼠标悬停在窗口的最大化按钮上或按 Win + Z，可以轻松访问对齐布局。） |
| sys_menu          | 标题栏  | true    | bool   | SetEnableSysMenu        |在右键点击标题栏时，是否显示系统的窗口菜单（可进行调整窗口状态，关闭窗口等操作）|
| sys_menu_rect     | 标题栏  | 0,0,0,0 | rect   | SetSysMenuRect          | 窗口菜单区域，在窗口标题栏的左上角（双击该区域可退出窗口，点击显示系统的窗口菜单），该功能若XML中无设置，默认是关闭的 |
| icon              | 标题栏  |         | string | SetWindowIcon           |设置窗口的图标文件路径，支持ico格式 |
| text              | 标题栏  |         | string | SetText                 |窗体标题字符串|
| textid            | 标题栏  |         | string | SetTextId               |窗体标题字符串的ID, ID在多语言文件中指定, 如(STRID_MIANWINDOW_TITLE) |
| drag_drop         | 拖放    | true    | bool   | SetEnableDragDrop       |设置是否允许拖放操作（拖入文件和拖入文本）<br>当设置drag_drop="true"时，该窗口内的控件支持拖入文件和拖入文本<br>如果设置drag_drop="false"时，该窗口内的控件不支持拖入文件和拖入文本|
| shadow_attached   | 窗口阴影| true    | bool   | SetShadowAttached       |窗口是否附加阴影效果,如(true) |
| shadow_type       | 窗口阴影|         | string | SetShadowType           |设置窗口的阴影类型：<br> "default"：默认阴影 <br> "big"：有阴影（大），直角，有边框（适合普通窗口）<br> "big_round"：有阴影（大），圆角，有边框（适合普通窗口）<br> "small"：有阴影（小），直角，有边框（适合普通窗口）<br> "small_round"：有阴影（小），圆角，有边框（适合普通窗口）<br> "menu"：有阴影（小），直角，有边框（适合弹出式窗口，比如菜单等）<br> "menu_round"：有阴影（小），圆角，有边框（适合弹出式窗口，比如菜单等）<br> "none"：无阴影，直角，有边框<br> "none_round"：无阴影，圆角，有边框|
| shadow_image      | 窗口阴影|         | string | SetShadowImage          |使用自定义的阴影图片去代替默认的阴影效果，设置的路径要注意相对路径以及九宫格属性，如(file='../public/shadow/shadow_big.svg' corner='64,64,68,70') |
| shadow_corner     | 窗口阴影| 0,0,0,0 | rect   | SetShadowCorner         |设置了shadow_image属性后，设置此属性来指定阴影素材的九宫格描述 |
| shadow_border_round| 窗口阴影| 0,0       | size  | SetShadowBorderRound  |设置了shadow_image属性后，设置此属性来指定阴影的圆角属性 |
| shadow_border_color| 窗口阴影|"#FFA3A3A3"|string | SetShadowBorderColor  |设置窗口阴影的边框颜色 |
| shadow_border_size | 窗口阴影|2          |int    | SetShadowBorderSize   |设置窗口阴影的边框大小，实际显示的边框大小为此值的一半 |
| shadow_snap        | 窗口阴影| true      | bool  | SetEnableShadowSnap   |设置阴影是否支持窗口贴边操作，如果为true，则在窗口贴近屏幕边缘时，这一侧的阴影自动隐藏，以增大视图内的有效空间 |
| size              | 窗口大小| 0,0     | size   | SetInitSize             |窗口的初始化大小, 支持的格式：size="1200,800", 或者size="50%,50%", 或者size="1200,50%", size="50%,800"，百分比是指屏幕宽度或者高度的百分比 |
| size_contain_shadow| 窗口大小| false  | size   |                         |窗口的初始化大小(size属性)，是否包含窗口的阴影，默认不包含，窗口的实际大小是配置的size值 + 阴影大小 |
| min_size          | 窗口大小| 0,0     | size   | SetWindowMinimumSize    |窗口最小大小, 如(320,240) |
| max_size          | 窗口大小| 0,0     | size   | SetWindowMaximumSize    |窗口最大大小, 如(1600,1200) |
| size_box          | 窗口大小| 0,0,0,0 | rect   | SetSizeBox              |窗口可拖动改变窗口大小的边距, 如(4,4,6,6) |
| round_corner      | 窗口形状| 0,0     | size   | SetRoundCorner          |窗口圆角大小, 如(4,4) |
| alpha_fix_corner  | 窗口形状|14,14,14,14| rect | SetAlphaFixCorner       |窗口圆角的透明通道修补范围 |
| layered_window    | 窗口绘制| false   | bool   | SetLayeredWindow        |设置是否为层窗口 |
| alpha             | 窗口绘制| 255     | int    | SetLayeredWindowAlpha   |设置透明度数值[0, 255]，当 alpha 为 0 时，窗口是完全透明的。 当 alpha 为 255 时，窗口是不透明的。<br>仅当layered_window="true"时有效，<br>该参数在UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)|
| opacity           | 窗口绘制| 255     | int    | SetLayeredWindowOpacity |设置透不明度数值[0, 255]，当 opacity 为 0 时，窗口是完全透明的。 当 opacity 为 255 时，窗口是不透明的。<br> 仅当IsLayeredWindow()为true的时候有效，所以如果当前不是分层窗口，内部会自动设置为分层窗口 <br>该参数在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)|
| render_backend_type|窗口绘制| "CPU"   | string |SetRenderBackendType     | "CPU": CPU绘制 <br> "GL": 使用OpenGL绘制 <br> 注意事项: <br> （1）一个线程内，只允许有一个窗口使用OpenGL绘制，否则会出现导致程序崩溃的问题 <br> （2）OpenGL绘制的窗口，不能是分层窗口（即带有WS_EX_LAYERED属性的窗口）<br> （3）使用OpenGL的窗口，每次绘制都是绘制整个窗口，不支持局部绘制，所以不一定比使用CPU绘制的情况下性能更好|

备注：窗口属性的解析函数参见：[WindowBuilder::ParseWindowAttributes函数](../duilib/Core/WindowBuilder.cpp)    
备注：窗口在XML中的标签名称是："Window"     
使用示例：    
```xml
<Window size="75%,90%" min_size="80,50" use_system_caption="false" caption="0,0,0,36"
        shadow_attached="true" layered_window="true" alpha="255" size_box="4,4,4,4">
</Window>
```