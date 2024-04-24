## Control（基础控件）的属性

`Control` 控件为所有可用控件的基类，包含了控件常用的属性，如宽度、高度、边距等属性。一般情况下不作为具体可用控件使用，但常用于一些占位符，请参考示例。
`Control` 是所有控件的基类，包含了所有控件的可用通用属性。

```xml
<!-- 一个根据父容器宽度自动拉伸的 Control 控件，一般用于作为占位符使用 -->
<Control width="stretch"/>
```

| 属性名称 | 默认值 | 参数类型 |关联[Control.h](../duilib/Core/Control.h)中的函数| 用途 |
| :--- | :--- | :--- | :---| :--- |
| class |  | string | SetClass|控件样式,用样式里面的属性值来设置本控件的属性,如(btn_default),可以同时指定多个样式,各个样式之间用空格分隔,需要写在第一个属性位置 |
| name |  | string | SetName|控件名字,同一窗口内必须唯一,如(testbtn) |
| width | stretch | int / string | SetFixedWidth|可以设置int或string类型的值.当值为int是则设置控件的宽度值,如(100);当值为string时,stretch代表由父容器计算控件宽度,auto代表根据内容情况自动计算宽度,如果为百分比值"50%"，代表该控件的宽度期望值为父控件宽度的50% |
| height | stretch | int / string | SetFixedHeight|可以设置int或string类型的值.当值为int是则设置控件的高度值,如(100);当值为string时,stretch代表由父容器计算控件高度,auto代表根据内容情况自动计算高度,如果为百分比值"30%"，代表该控件的宽度期望值为父控件高度的30%  |
| min_width | -1 | int | SetMinWidth|控件的最小宽度,如(30) |
| min_height | -1 | int | SetMinHeight|控件的最小高度,如(30) |
| max_width | INT32_MAX | int / string | SetMaxWidth|控件的最大宽度,描述同width |
| max_height | INT32_MAX | int / string | SetMaxHeight|控件的最大高度,描述同height |
| margin | 0,0,0,0 | rect | SetMargin|外边距,如(2,2,2,2) |
| padding | 0,0,0,0 | rect | SetPadding|内边距,如(2,2,2,2) |
| control_padding | true | bool | SetEnableControlPadding|是否允许控件自身运用内边距 |
| halign | left | string | SetHorAlignType|控件的横向位置,如(center),支持left、center、right三种位置 |
| valign | top | string | SetVerAlignType|控件的纵向位置,如(center),支持top、center、bottom三种位置 |
| float | false | bool | SetFloat|是否使用绝对定位,如(true) |
| bkcolor |  | string | SetBkColor|背景颜色字符串常量,如(white) |
| normal_color |  | string | SetStateColor|普通状态颜色,如(white) |
| hot_color |  | string | SetStateColor|悬浮状态颜色,如(white) |
| pushed_color |  | string | SetStateColor|按下状态颜色,如(white) |
| disabled_color |  | string | SetStateColor|禁用状态颜色,如(white) |
| border_color |  | string | SetBorderColor|设置所有状态下的边框颜色,如(blue) |
| normal_border_color |  | string | SetBorderColor|正常状态的边框颜色,如(blue) |
| hot_border_color |  | string | SetBorderColor|悬浮状态的边框颜色,如(blue) |
| pushed_border_color |  | string | SetBorderColor|按下状态的边框颜色,如(blue) |
| disabled_border_color |  | string | SetBorderColor|禁止状态的边框颜色,如(blue) |
| focus_border_color |  | string | SetFocusBorderColor|焦点状态的边框颜色,如(blue) |
| border_size | 0 | int / rect | SetBorderSize|可以设置int或rect类型的值。当值为int时则左、上、右、下都用该值作为宽。值为rect类型时则分别设置左、上、右、下的边框 |
| left_border_size | 0 | int | SetLeftBorderSize|左边边框大小,如(1),设置该值大于0,则将忽略border_size属性的设置 |
| top_border_size | 0 | int | SetTopBorderSize|顶部边框大小,如(1),设置该值大于0,则将忽略border_size属性的设置 |
| right_border_size | 0 | int | SetRightBorderSize|右边边框大小,如(1),设置该值大于0,则将忽略border_size属性的设置 |
| bottom_border_size | 0 | int | SetBottomBorderSize|底部边框大小,如(1),设置该值大于0,则将忽略border_size属性的设置 |
| border_round | 0,0 | size | SetBorderRound|边框圆角半径,如(2,2) |
| bkimage |  | string | SetBkImage|背景图片,如(bk.bmp或file='aaa.jpg' res='' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' fade='255' xtiled='false' ytiled='false') |
| normal_image |  | string | SetStateImage|普通状态图片 |
| hot_image |  | string | SetStateImage|鼠标悬浮的状态图片 |
| pushed_image |  | string | SetStateImage|鼠标按下的状态图片 |
| disabled_image |  | string | SetStateImage|禁用的状态图片 |
| fore_normal_image |  | string | SetForeStateImage|普通状态前景图片 |
| fore_hot_image |  | string | SetForeStateImage |鼠标悬浮的状态前景图片 |
| fore_pushed_image |  | string | SetForeStateImage|鼠标按下的状态前景图片 |
| fore_disabled_image |  | string | SetForeStateImage|禁用的状态前景图片 |
| tooltip_text |  | string | SetToolTipText|鼠标悬浮提示,如(请在这里输入你的密码) |
| tooltip_textid |  | string | SetToolTipTextId|鼠标悬浮提示,指定多语言模块的ID,当tooltiptext为空时则显示此属性,如(TOOL_TIP_ID) |
| dataid |  | string | SetDataID|自定义字符串数据,辅助函数，供用户使用 |
| user_dataid |  | size_t | SetUserDataID|自定义整型数据, 供用户使用 |
| enabled | true | bool | SetEnabled|是否可以响应用户操作,如(true) |
| mouse_enabled | true | bool | SetMouseEnabled|本控件是否可以响应鼠标操作,如(true) |
| keyboard_enabled | true | bool | SetKeyboardEnabled|非CButtonUI类忽略该值,为false时不支持TAB_STOP,且该对象不处理键盘信息 |
| receivepointer | true | bool | |是否响应触控消息,如(true) |
| visible | true | bool | SetVisible|是否可见,如(true) |
| fade_visible | true | bool | SetFadeVisible|是否可见,如(true),此属性会触发控件动画效果 |
| menu | false | bool | |是否需要右键菜单,如(true) |
| nofocus | false | bool | SetNoFocus|是否可以获取焦点,如(true) |
| tab_stop | true | bool | SetTabStop| 是否允许通过按TAB键切换到此控件 |
| show_focus_rect | false| bool | SetLoadingBkColor| 是否显示焦点状态(一个虚线构成的矩形) |
| focus_rect_color | | string | SetFocusRectColor| 焦点状态矩形的颜色 |
| alpha | 255 | int | SetAlpha|控件的整体透明度,如(128)，有效值为 0-255 |
| state | normal | string | SetState|控件的当前状态: 支持normal、hot、pushed、disabled状态 |
| cursortype | arrow | string | SetCursorType|鼠标移动到控件上时的鼠标光标: arrow(箭头)、hand(手型)、ibeam(“I”形状)、sizewe(左右拖动)、sizens(上下拖动)|
| render_offset | 0,0 | size | SetRenderOffset|控件绘制时的偏移量,如(10,10),一般用于绘制动画 |
| fade_alpha | false | bool | GetAnimationManager(). SetFadeAlpha|是否启用控件透明渐变动画,如(true) |
| fade_hot | false | bool |GetAnimationManager(). SetFadeHot |是否启用控件悬浮状态下 的透明渐变动画,如(true) |
| fade_width | false | bool | GetAnimationManager(). SetFadeWidth|是否启用控件宽度渐变动画,如(true) |
| fade_height | false | bool | GetAnimationManager(). SetFadeHeight|是否启用控件高度渐变动画,如(true) |
| fade_in_out_x_from_left | false | bool | GetAnimationManager(). SetFadeInOutX|是否启用控件从左到右的动画,如(true) |
| fade_in_out_x_from_right | false | bool | GetAnimationManager(). SetFadeInOutX|是否启用控件从右到左的动画,如(true) | 
| fade_in_out_y_from_top | false | bool | GetAnimationManager().  SetFadeInOutY|是否启用控件从上到下的动画,如(true) | 
| fade_in_out_y_from_bottom | false | bool | GetAnimationManager().  SetFadeInOutY|是否启用控件从下到上的动画,如(true) |
| loading_image | | string | SetLoadingImage| 加载中状态的图片 |
| loading_bkcolor | | string | SetLoadingBkColor| 加载中状态的背景色 |
| paint_order | | string | SetPaintOrder| 设置绘制顺序：0 表示常规绘制，非0表示指定绘制顺序，值越大表示越晚绘制 |
| start_gif_play | | int | StartGifPlay| 播放动画，参数表示播放循环次数 |
| stop_gif_play | | int | StopGifPlay| 停止动画，参数表示停止在哪一帧 |
| box_shadow | | string | SetBoxShadow|设置控件的阴影属性，举例：boxshadow="color='red' offset='0,0' blurradius='8' spreadradius='8' |
| cache | false | bool |SetUseCache |是否启用控件绘制缓存,如(true) |