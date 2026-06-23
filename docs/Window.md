## 窗口的属性
| 属性名称          | 属性分类| 默认值  |参数类型| 关联[Window.h](../duilib/Core/Window.h)中的函数| 用途 |
| :---              | :---    | :---    | :---   |:---                     |:--- |
| caption           | 标题栏  | "0,0,0,0" | rect   | SetCaptionRect          |窗口可拖动的标题栏大小的边距, 最后一个参数是指离上边框的距离,如"0,0,0,36" |
| use_system_caption| 标题栏  | false   | bool   | SetUseSystemCaption     |设置是否使用系统标题栏 |
| snap_layout_menu  | 标题栏  | true    | bool   | SetEnableSnapLayoutMenu |是否支持显示贴靠布局菜单（Windows 11新功能：通过将鼠标悬停在窗口的最大化按钮上或按 Win + Z，可以轻松访问对齐布局。），该属性仅当在窗口为分层窗口或者使用系统阴影时默认开启 |
| sys_menu          | 标题栏  | true    | bool   | SetEnableSysMenu        |在右键点击标题栏时，是否显示系统的窗口菜单（可进行调整窗口状态，关闭窗口等操作）|
| sys_menu_rect     | 标题栏  | "0,0,0,0" | rect   | SetSysMenuRect          | 窗口菜单区域，在窗口标题栏的左上角（双击该区域可关闭窗口，点击显示系统的窗口菜单），该功能若XML中无设置，默认是关闭的 |
| icon              | 标题栏  |         | string | SetWindowIcon           |设置窗口的图标文件路径，支持ico格式 |
| text              | 标题栏  |         | string | SetText                 |窗体标题字符串|
| textid            | 标题栏  |         | string | SetTextId               |窗体标题字符串的ID, ID在多语言文件中指定, 如"STRID_MIANWINDOW_TITLE" |
| drag_drop         | 拖放    | true    | bool   | SetEnableDragDrop       |设置窗口内控件是否支持拖放操作（拖入文件/文本）：true=支持，false=不支持 |
| shadow_attached   | 窗口阴影| true    | bool   | SetShadowAttached       |窗口是否附加阴影效果，如"true"表示开启窗口阴影功能，"false"表示关闭窗口阴影功能<br>如果设置了shadow_type属性，相当于隐式将shadow_attached设置为"true"，<br>所以一般设置了shadow_type属性以后，不需要再单独设置shadow_attached属性|
| shadow_type       | 窗口阴影|         | string | SetShadowType           |设置窗口的阴影类型：<br> "default"：默认自绘阴影 <br> "big"：有自绘阴影（大），直角，有边框（适合普通窗口）<br> "big_round"：有自绘阴影（大），圆角，有边框（适合普通窗口）<br> "small"：有自绘阴影（小），直角，有边框（适合普通窗口）<br> "small_round"：有自绘阴影（小），圆角，有边框（适合普通窗口）<br> "menu"：有自绘阴影（小），直角，有边框（适合弹出式窗口，比如菜单等）<br> "menu_round"：有自绘阴影（小），圆角，有边框（适合弹出式窗口，比如菜单等）<br> "none"：无阴影，直角，有边框<br> "none_round"：无阴影，圆角，有边框（自绘）<br> "custom": 自定义自绘阴影，该类型需要同时设置`shadow_image`、`shadow_corner`、`shadow_border_round属性` <br> "system_default"：使用系统阴影，跟随操作系统默认窗口阴影属性<br> "system_not_round"：使用系统阴影，直角<br> "system_round"：使用系统阴影，圆角<br> "system_small_round"：使用系统阴影，小圆角<br>兼容性说明：<br>（1）对于Win7和Win10系统：系统阴影仅支持"system_default"这一个，其他三个不支持<br>（2）对于Win11系统，四个系统阴影属性均支持，系统默认阴影为圆角<br>（3）对于macOS系统，四个系统阴影属性均支持，系统默认阴影为圆角<br>（4）对于Linux系统和FreeBSD系统，不支持系统阴影<br>（5）若设置为不支持的系统阴影，会自动切换为相应支持的系统阴影或者自绘阴影|
| shadow_image      | 窗口阴影|         | string | SetShadowImage          |使用自定义的阴影图片去代替默认的阴影效果，该属性一般仅当shadow_type="custom"时设置<br>设置的路径要注意相对路径以及九宫格属性，<br>如(file='public/shadow/shadow_big.svg' corner='64,64,68,70') |
| shadow_corner     | 窗口阴影| "0,0,0,0" | rect   | SetShadowCorner       |设置了shadow_image属性后，设置此属性来指定阴影素材的九宫格描述，该属性一般仅当shadow_type="custom"时设置 |
| shadow_border_round| 窗口阴影| "0,0"    | size  | SetShadowBorderRound|设置了shadow_image属性后，设置此属性来指定阴影的圆角属性，该属性一般仅当shadow_type="custom"时设置 |
| shadow_border_color| 窗口阴影|          |string | SetShadowBorderColor  |设置窗口阴影的边框颜色 |
| shadow_border_size | 窗口阴影|2         |int    | SetShadowBorderSize   |设置窗口阴影的边框像素大小，实际视觉显示的边框宽度为此值的一半（例如设置为2，实际显示1像素） |
| shadow_snap        | 窗口阴影| true     | bool  | SetEnableShadowSnap   |设置阴影是否支持窗口贴边操作，如果为true，则在窗口贴近屏幕边缘时，这一侧的阴影自动隐藏，以增大视图内的有效空间<br>该属性仅当使用自绘阴影时有效，当使用系统阴影时无效|
| size              | 窗口大小| "0,0"     | size   | SetInitSize             |窗口的初始化大小, 支持的格式：size="1200,800", 或者size="50%,50%", 或者size="1200,50%", size="50%,800"，百分比是指屏幕宽度或者高度的百分比 |
| size_contain_shadow| 窗口大小| false  | bool   |    |窗口的初始化大小(size属性)，是否包含窗口的阴影，默认不包含，窗口的实际大小是配置的size值 + 阴影大小 |
| min_size          | 窗口大小| "0,0"     | size   | SetWindowMinimumSize    |窗口最小大小, 如"320,240" |
| max_size          | 窗口大小| "0,0"     | size   | SetWindowMaximumSize    |窗口最大大小, 如"1600,1200" |
| size_box          | 窗口大小| "0,0,0,0" | rect   | SetSizeBox              |窗口可拖动改变窗口大小的边距, 如"4,4,4,4" |
| round_corner      | 窗口形状| "0,0"     | size   | SetRoundCorner          |窗口圆角大小, 如"4,4"，该属性在使用窗口阴影（即设置了shadow_type属性）时，一般不需要设置 |
| layered_window    | 窗口绘制| false   | bool   | SetLayeredWindow        |设置是否为层窗口，使用注意事项：<br>（1）该属性在使用窗口阴影（即设置了shadow_type属性）时不需要设置，窗口阴影功能会根据需要来管理该属性<br>（2）当使用系统标题栏时（即use_system_caption="true"），不应设置该属性 |
| alpha             | 窗口绘制| 255     | int    | SetLayeredWindowAlpha   |设置透明度数值[0, 255]，当 alpha 为 0 时，窗口是完全透明的。 当 alpha 为 255 时，窗口是不透明的。<br>仅当layered_window="true"时有效，<br>该参数在UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)|
| opacity           | 窗口绘制| 255     | int    | SetLayeredWindowOpacity |设置不透明度数值[0, 255]，当 opacity 为 0 时，窗口是完全透明的。 当 opacity 为 255 时，窗口是不透明的。<br> 仅当layered_window="true"时有效，所以如果当前不是分层窗口，内部会自动设置为分层窗口 <br>该参数在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)|
| render_backend_type|窗口绘制| "CPU"   | string |SetRenderBackendType     | "CPU": CPU绘制 <br> "GL": 使用OpenGL绘制 <br> 注意事项: <br> （1）一个线程内，只允许有一个窗口使用OpenGL绘制，否则会出现导致程序崩溃的问题 <br> （2）OpenGL绘制的窗口，不能是分层窗口（即带有WS_EX_LAYERED属性的窗口），因此使用GL渲染时，`layered_window`、`alpha`、`opacity` 属性均无效 <br> （3）使用OpenGL的窗口，每次绘制都是绘制整个窗口，不支持局部绘制，所以不一定比使用CPU绘制的情况下性能更好|

备注：窗口阴影相关的内容，请参考文档：[WindowShadow.md](WindowShadow.md)    
备注：窗口属性的解析函数参见：[WindowBuilder::ParseWindowAttributes函数](../duilib/Core/WindowBuilder.cpp)    
备注：窗口在XML中的标签名称是："Window"     
使用示例：    
```xml
<Window size="75%,90%" min_size="80,50" size_box="4,4,4,4"
        caption="0,0,0,36" shadow_type="system_default">
</Window>