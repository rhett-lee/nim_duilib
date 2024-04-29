## 窗口的属性
| 属性名称 | 默认值 | 参数类型 | 关联[Window.h](../duilib/Core/Window.h)中的函数| 用途 |
| :--- | :--- | :--- | :--- |:--- |
| size | 0,0 | size | SetInitSize|窗口的初始化大小, 支持的格式：size="1200,800", 或者size="50%,50%", 或者size="1200,50%", size="50%,800"，百分比是指屏幕宽度或者高度的百分比 |
| mininfo | 0,0 | size | SetMinInfo|窗口最小大小, 如(320,240) |
| maxinfo | 0,0 | size | SetMaxInfo|窗口最大大小, 如(1600,1200) |
| sizebox | 0,0,0,0 | rect | SetSizeBox|窗口可拖动改变窗口大小的边距, 如(4,4,6,6) |
| caption | 0,0,0,0 | rect | SetCaptionRect|窗口可拖动的标题栏大小的边距, 最后一个参数是指离上边框的距离,如(0,0,0,28) |
| use_system_caption | false | bool | SetUseSystemCaption|设置是否使用系统标题栏 |
| text |  | string | SetText|窗体标题字符串|
| textid |  | string | SetTextId|窗体标题字符串的ID, ID在多语言文件中指定, 如(STRID_MIANWINDOW_TITLE) |
| round_corner | 0,0 | size | SetRoundCorner|窗口圆角大小, 如(4,4) |
| shadow_attached | true | bool | SetShadowAttached|窗口是否附加阴影效果,如(true) |
| shadow_image |  | string | SetShadowImage|使用自定义的阴影素材去代替默认的阴影效果，设置的路径要注意相对路径以及九宫格属性，如(file='../public/bk/bk_shadow.png' corner='30,30,30,30') |
| shadow_corner | 0,0,0,0 | rect | SetShadowCorner|设置了shadowimage属性后，设置此属性来指定阴影素材的九宫格描述，这个属性一定要写在size属性前面 |
| alpha_fix_corner | 14,14,14,14 | rect | SetAlphaFixCorner|透明通道修补范围的的九宫格描述 |
| render_transparent | false | bool | SetRenderTransparent|如果为true，表示设置渲染为透明图层 |
| layered_window | false | bool | SetLayeredWindow|设置是否为层窗口 |
| alpha | 255 | int | SetWindowAlpha|设置窗口的透明度（0 - 255），仅当使用层窗口时有效 |

备注：窗口属性的解析函数参见：[WindowBuilder::Create函数](../duilib/Core/WindowBuilder.cpp)    
备注：窗口在XML中的标签名称是："Window"     
使用示例：    
```xml
<Window size="75%,90%" mininfo="80,50" use_system_caption="false" caption="0,0,0,36"
        shadowattached="true" layered_window="true" alpha="255" sizebox="4,4,4,4">
</Window>
```