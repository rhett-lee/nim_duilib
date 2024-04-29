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
| cursor_type | arrow | string | SetCursorType|鼠标移动到控件上时的鼠标光标: arrow(箭头)、hand(手型)、ibeam(“I”形状)、sizewe(左右拖动)、sizens(上下拖动)|
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

## ScrollBar的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| button1_normal_image |  | string | 左或上按钮普通状态图片 |
| button1_hot_image |  | string | 左或上按钮鼠标悬浮状态图片 |
| button1_pushed_image |  | string | 左或上按钮鼠标按下状态图片 |
| button1_disabled_image |  | string | 左或上按钮禁用状态图片 |
| button2_normal_image |  | string | 右或下按钮普通状态图片 |
| button2_hot_image |  | string | 右或下按钮鼠标悬浮状态图片 |
| button2_pushed_image |  | string | 右或下按钮鼠标按下状态图片 |
| button2_disabled_image |  | string | 右或下按钮禁用状态图片 |
| thumb_normal_image |  | string | 滑块普通状态图片 |
| thumb_hot_image |  | string | 滑块鼠标悬浮状态图片 |
| thumb_pushed_image |  | string | 滑块鼠标按下状态图片 |
| thumb_disabled_image |  | string | 滑块禁用状态图片 |
| rail_normal_image |  | string | 滑块中间标识普通状态图片 |
| rail_hot_image |  | string | 滑块中间标识鼠标悬浮状态图片 |
| rail_pushed_image |  | string | 滑块中间标识鼠标按下状态图片 |
| rail_disabled_image |  | string | 滑块中间标识禁用状态图片 |
| bk_normal_image |  | string | 背景普通状态图片 |
| bk_hot_image |  | string | 背景鼠标悬浮状态图片 |
| bk_pushed_image |  | string | 背景鼠标按下状态图片 |
| bk_disabled_image |  | string | 背景禁用状态图片 |
| horizontal | false | bool | 水平或垂直,如(true) |
| line_size | 8 | int | 滚动一行的大小,如(8) |
| thumb_min_length | 30 | int | 滑块的最小长度 |
| range | 100 | int | 滚动范围,如(100) |
| value | 0 | int | 滚动位置,如(0) |
| show_button1 | true | bool | 是否显示左或上按钮,如(true) |
| show_button2 | true | bool | 是否显示右或下按钮,如(true) |
| auto_hide_scroll | true | bool | 是否自动隐藏滚动条,如(true) |

ScrollBar 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## Label的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text |  | string | 显示文本 |
| textid |  | string | 多语言功能的文本ID |
| text_align | left | string | 文本的对其方式, 支持以下取值：left、hcenter、right、top、vcenter、bottom |
| text_padding | 0,0,0,0 | rect | 文字显示的边距, 如(2,2,2,2) |
| font | | string | 字体ID，该字体ID必须在 global.xml 中存在 |
| end_ellipsis | false | bool | 句末显示不完整是否使用...代替 |
| path_ellipsis | false | bool | 对于路径，显示不完整时是否使用...代替中间路径 |
| normal_text_color |  | string | 普通字体颜色,不指定则使用默认颜色,如(blue) |
| hot_text_color |  | string | 鼠标悬浮字体颜色,不指定则使用默认颜色,如(blue) |
| pushed_text_color |  | string | 鼠标按下字体颜色,不指定则使用默认颜色,如(blue) |
| disabled_text_color |  | string | disabled字体颜色,不指定则使用默认颜色,如(blue) |
| single_line | true | bool | 是否单行输出文字 |
| multi_line | false | bool | 是否多行输出文字，与single_line属性互斥 |
| auto_tooltip | false | bool | 鼠标悬浮到控件显示的提示文本是否省略号出现时才显示|

Label 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## LabelBox的属性
LabelBox与Label是基于相同模板的类，请参考 `Label`的属性    
LabelBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## Button的属性
Button 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## ButtonBox的属性
ButtonBox与Button是基于相同模板的类，请参考 `Button`的属性    
ButtonBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## CheckBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| selected | false | bool | 是否选中 |
| selected_normal_image |  | string | 选择状态时，普通状态图片 |
| selected_hot_image |  | string | 选择状态时，鼠标悬浮的状态图片 |
| selected_pushed_image |  | string | 选择状态时，鼠标按下的状态图片 |
| selected_disabled_image |  | string | 选择状态时，禁用的状态图片 |
| selected_fore_normal_image |  | string | 选择状态时，前景图片 |
| selected_fore_hot_image |  | string | 选择状态时，鼠标悬浮状态的图片 |
| selected_fore_pushed_image |  | string | 选择状态时，鼠标按下状态的前景图片 |
| selected_fore_disabled_image |  | string | 选择状态时，禁用状态的前景图片 |
| part_selected_normal_image |  | string | 部分选择时，普通状态图片 |
| part_selected_hot_image |  | string | 部分选择时，鼠标悬浮的状态图片 |
| part_selected_pushed_image |  | string | 部分选择时，鼠标按下的状态图片 |
| part_selected_disabled_image |  | string | 部分选择时，禁用的状态图片 |
| part_selected_fore_normal_image |  | string | 部分选择时，前景图片 |
| part_selected_fore_hot_image |  | string | 部分选择时，鼠标悬浮状态的图片 |
| part_selected_fore_pushed_image |  | string | 部分选择时，鼠标按下状态的前景图片 |
| part_selected_fore_disabled_image |  | string | 部分选择时，禁用状态的前景图片 |
| selected_text_color |  | string | 选择状态的字体颜色,不指定则使用默认颜色,如(blue) |
| selected_normal_text_color |  | string | 选择状态的普通状态字体颜色,不指定则使用默认颜色,如(blue) |
| selected_hot_text_color |  | string | 选择状态的鼠标悬浮状态字体颜色,不指定则使用默认颜色,如(blue) |
| selected_pushed_text_color |  | string | 选择状态的鼠标按下状态字体颜色,不指定则使用默认颜色,如(blue) |
| selected_disabled_text_color |  | string | 选择状态的禁用状态字体颜色,不指定则使用默认颜色,如(blue) |
| normal_first | false | bool | 控件在选择状态下，没有设置背景色或背景图时，用非选择状态的对应属性来绘制 |

CheckBox 控件继承了 `Button` 属性，更多可用属性请参考`Button`的属性

## CheckBoxBox的属性
CheckBoxBox与CheckBox是基于相同模板的类，请参考 `CheckBox`的属性    
CheckBoxBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## Option的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| group |  | string | 所属组的名称，在相同的组名称下，保持单选 |

Option 控件继承了 `CheckBox` 属性，更多可用属性请参考`CheckBox`的属性

## OptionBox的属性
OptionBoxBox与OptionBox是基于相同模板的类，请参考 `OptionBox`的属性    
OptionBoxBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## GroupBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| corner_size | "0,0" | size | 圆角大小 |
| line_width | 0 | int | 线条宽度 |
| line_color | | string | 线条颜色 |
| text | | string | 文本内容 |

GroupBox 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## GroupVBox的属性
GroupVBox 与 GroupBox 是相同模板实现，可用属性请参考`GroupBox`的属性    
GroupVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## GroupHBox的属性
GroupHBox 与 GroupBox 是相同模板实现，可用属性请参考`GroupBox`的属性    
GroupHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## Combo的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| combo_type | "drop_down" | string | 组合框的类型："drop_list" 表示为不可编辑列表，"drop_down" 表示为可编辑列表|
| dropbox_size | | string | 下拉列表的大小（宽度和高度）|
| popup_top | false | bool | 下拉列表是否向上弹出 |
| combo_tree_view_class | | string | 下拉表TreeView的Class属性，定义方法请参考`global.xml` 中的对应内容|
| combo_tree_node_class | | string | 下拉表TreeView的节点的Class属性，定义方法请参考`global.xml` 中的对应内容|
| combo_icon_class | | string | 显示图标的Class属性，定义方法请参考`global.xml` 中的对应内容|
| combo_edit_class | | string | 编辑控件的Class属性，定义方法请参考`global.xml` 中的对应内容|
| combo_button_class | | string | 按钮控件的Class属性，定义方法请参考`global.xml` 中的对应内容|

Combo 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## FilterCombo的属性
FilterCombo 控件不支持"combo_type"属性    
FilterCombo 控件继承了 `Combo` 属性，更多可用属性请参考`Combo`的属性

## ComboButton的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| dropbox_size | | string | 下拉列表的大小（宽度和高度）|
| popup_top | false | bool | 下拉列表是否向上弹出 |
| combo_box_class | | string | 下拉表组合框的Class属性，定义方法请参考`global.xml` 中的对应内容|
| left_button_class | | string | 左侧按钮控件的Class属性，定义方法请参考`global.xml` 中的对应内容|
| left_button_top_label_class | | string | 左侧按钮上侧的Label控件的Class属性，定义方法请参考`global.xml` 中的对应内容|
| left_button_bottom_label_class | | string | 左侧按钮下侧的Label控件的Class属性，定义方法请参考`global.xml` 中的对应内容|
| left_button_top_label_text | | string | 左侧按钮上侧的Label控件的文本|
| left_button_bottom_label_text | | string | 左侧按钮下侧的Label控件的文本|
| left_button_top_label_bkcolor | | string | 左侧按钮上侧的Label控件的背景色|
| left_button_bottom_label_bkcolor | | string | 左侧按钮下侧的Label控件的背景色|
| right_button_class | | string | 右侧按钮控件的Class属性，定义方法请参考`global.xml` 中的对应内容|

ComboButton 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## CheckCombo的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| dropbox | | string | 下拉框的属性信息，具体设置方法可参照示例程序|
| dropbox_size | | string | 下拉列表的大小（宽度和高度）|
| popup_top | false | bool | 下拉列表是否向上弹出 |
| dropbox_item_class | | string | 下拉列表中每一个列表项的属性，具体设置方法可参照示例程序|
| selected_item_class | | string | 选择项中每一个子项的属性，具体设置方法可参照示例程序|

CheckCombo 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## DateTime的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| format | | string | 日期的格式，具体可参考：`DateTime.h`中函数的说明 |
| edit_format | | string | 编辑状态时，日期的编辑格式，可选值："date_calendar"：年-月-日，通过下拉框展示月日历的方式来修改日期；"date_up_down"： 编辑时显示：年-月-日，通过控件的右侧放置一个向上-向下的控件以修改日期；"date_time_up_down"：编辑时显示：年-月-日 时:分:秒；"date_minute_up_down"：编辑时显示：年-月-日 时:分；"time_up_down"：编辑时显示：时:分:秒；"minute_up_down"：编辑时显示：时:分|

DateTime 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## HotKey的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| default_text | | string | 默认显示的文字 |

HotKey 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## HyperLink的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| url | | string | URL |

HyperLink 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## IPAddress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| ip | | string | IP地址，比如："192.168.0.0" |

HotKey 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## Line的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vertical | false | bool | 是否为垂直的线 |
| line_color | | string | 线的颜色 |
| line_width | | int | 线的宽度 |
| dash_style | | string | 线的形状，可选值："solid"：实线；"dash"：短划线构成的虚线；"dot"：点构成的虚线；"dash_dot"：交替短划线点线；"dash_dot_dot"：交替短划线点点线|

Line 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## CMenuWnd的属性
CMenuWnd是一个窗口，具体用法请参考示例程序中的菜单    
CMenuWnd 控件继承了 `Window` 属性，更多可用属性请参考`Window`的属性

## Progress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| horizontal | true | bool | 是否水平的，true表示水平，false表示垂直 |
| min | 0 | int | 进度最小值,如(0) |
| max | 100 | int | 进度最大值,如(100) |
| value | 0 | int | 进度值,如(50) |
| progress_image |  | string | 进度条前景图片 |
| stretch_fore_image | true | bool | 指定进度条前景图片是否缩放显示 |
| progress_color |  | string | 进度条前景颜色,不指定则使用默认颜色,如(blue) |
| marquee | true | bool | 是否滚动显示 |
| marquee_width | | int | 滚动的宽度 |
| marquee_step | | int | 滚动的步长 |
| reverse | false | bool | 进度值是否倒数（进度从100 到 0） |

Progress 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## Slider的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| thumb_normal_image |  | string | 拖动滑块普通状态图片 |
| thumb_hot_image |  | string | 拖动滑块鼠标悬浮状态图片 |
| thumb_pushed_image |  | string | 拖动滑块鼠标按下状态图片 |
| thumb_disabled_image |  | string | 拖动滑块鼠标禁用状态图片 |
| thumb_size | 10,10 | size | 拖动滑块大小,如(10,10) |
| step | 1 | int | 进度步长,如(1) |
| progress_bar_padding | 0,0,0,0 | rect | 滑动条绘制时缩小的内边距 |

Slider 控件继承了 `Progress` 属性，更多可用属性请参考`Progress`的属性

## CircleProgress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| circular | true | bool | 功能开关：是否为环形进度条 |
| circle_width | 1 | int | 环形进度条的宽度，如(10) |
| indicator |  | string | 设置进度指示移动图标 |
| clockwise | true | bool |设置递增方向 |
| bgcolor |  | string | 设置进度条背景颜色 |
| fgcolor |  | string | 设置进度条背前景色 |
| gradient_color |  | string | 设置进度条前景渐变颜色，与 fgcolor 同时使用，可以不设置则无渐变效果 |

CircleProgress 控件继承了 `Progress` 属性，更多可用属性请参考`Progress`的属性

## RichEdit的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vscrollbar | false | bool | 是否使用竖向滚动条,如(true) |
| hscrollbar | false | bool | 是否使用横向滚动条,如(true) |
| auto_vscroll | false | bool | 是否随输入竖向滚动,如(true) |
| auto_hscroll | false | bool | 是否随输入横向滚动,如(true) |
| want_tab | true | bool | 是否接受tab按键消息,如(true) |
| want_return_msg | true | bool | 是否接受return按键消息,如(true) |
| return_msg_want_ctrl | true | bool | 是否接受ctrl+return按键消息,如(true) |
| rich_text | true | bool | 是否使用富格式,如(true) |
| single_line | true | bool | 是否使用单行,如(true) |
| multi_line | true | bool | 是否使用多行,该属性与single_line互斥,如(true) |
| readonly | false | bool | 是否只读,如(false) |
| password | false | bool | 是否为密码模式,如(true) |
| show_password | false | bool | 是否显示密码符,如(true) |
| password_char | | string | 设置密码字符，默认为 " * " 字符，可用通过这个属性改变|
| flash_password_char | false | bool | 先显示字符，然后再显示密码字符|
| number_only | false | bool | 是否只允许输入数字,如(false) |
| max_number | INT_MAX | int | 允许的最大数字(仅当number_only为true的时候有效) |
| min_number | INT_MIN | int | 允许的最小数字(仅当number_only为true的时候有效 |
| text_align | left,top | string | 文字对齐方式, 取值: left、right、hcenter、top、vcenter、bottom, 如(right,bottom) |
| text_padding |  | rect | 文本内边距，如："2,2,2,2" |
| text |  | string | 显示文本,如(测试文本) |
| textid |  | string | 显示文本的多语言功能ID |
| font | | string | 字体ID |
| normal_text_color |  | string | 普通状态文字颜色,不指定则使用默认颜色,如(blue) |
| disabled_text_color |  | string | 禁用状态文字颜色,不指定则使用默认颜色,如(blue) |
| caret_color |  | string | 光标的颜色 |
| prompt_mode | false | bool | 是否显示提示文字,如(true) |
| prompt_text |  | string | 文本框内提示文字,当文本框text为空时显示 |
| prompt_textid |  | string | 多语言功能的ID,如(TEXT_OUT) |
| prompt_color |  | string | 文本框内提示文字的颜色 |
| focused_image |  | string | 焦点状态下的图片 |
| auto_detect_url | false | bool | 是否自动检测URL，如果是URL则显示为超链接 |
| limit_text | | int | 限制最多字符数 |
| limit_chars | | string | 限制允许输入哪些字符，比如"abc"表示只允许输入a、b、c字符，不允许输入其他字符 |
| allow_beep | | bool | 是否允许发出Beep声音 |
| word_wrap | | bool | 是否自动换行 |
| no_caret_readonly | | bool | 只读模式，不显示光标 |
| save_selection | | bool | 如果 为 true，则当控件处于非活动状态时，应保存所选内容的边界 |
| hide_selection | | bool | 是否隐藏选择内容 |
| zoom | | size | 设置缩放比例：设 wParam：缩放比例的分子，lParam：缩放比例的分母。"wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50% |
| wheel_zoom | | bool | 是否允许Ctrl + 滚轮来调整缩放比例 |
| default_context_menu | | bool | 是否使用默认的右键菜单 |
| enable_drag_drop | | bool | 是否允许拖放操作 |
| spin_class | | string | 设置Spin功能的Class名称，如果不为空则显示Spin按钮，详细用法参见示例程序|
| clear_btn_class | | string | 设置清除按钮功能的Class名称，如果不为空则显示清楚按钮，详细用法参见示例程序 |
| show_passowrd_btn_class | | string |设置显示密码按钮功能的Class名称，如果不为空则显示显示密码按钮 ，详细用法参见示例程序 |

RichEdit 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## RichText的属性
RichText是带有格式的文本，其格式类似于HTML标签，格式文本以`<RichText>`开头，以`</RichText>`结尾。    
举例：`<RichText>格式文本演示：<a href="URL">文本</a></RichText>`    
支持的标签列表：    
```cpp
   // 支持的标签列表(兼容HTML的标签):
   // 超级链接：   <a href="URL">文本</a>
   // 粗体字:      <b> </b>
   // 斜体字:      <i> </i>
   // 删除字:      <s> </s> 或 <del> </del> 或者 <strike> </strike>
   // 下划线字:    <u> </u>
   // 设置背景色:  <bgcolor color="#000000"> </bgcolor>
   // 设置字体:    <font face="宋体" size="12" color="#000000">
   // 换行标签：   <br/>
```
具体用法也可用参考示例程序。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text_align | left,top | string | 文字对齐方式, 取值: left、right、hcenter、top、vcenter、bottom, 如(right,bottom) |
| text_padding |  | rect | 文本内边距，如："2,2,2,2" |
| font | | string | 字体ID |
| text_color | | string | 默认文本颜色 |
| default_link_font_color | | string | 超级链接：常规文本颜色值 |
| hover_link_font_color | | string | 超级链接：Hover状态文本颜色值 |
| mouse_down_link_font_color | | string | 超级链接：鼠标按下状态文本颜色值 |
| link_font_underline | | string | 超级链接：是否使用带下划线的字体 |
| row_spacing_mul | | float | 行间距倍数, 比如1.5代表1.5倍行间距 |

RichText 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## Split的属性
分割条控件，可以通过拖动分割条改变左右或者上下两个控件的宽度或者高度，应用方法:     
如果放在横向布局（HLayout）中，则左右拖动    
如果放在纵向布局（VLayout）中，则上下拖动    
注意事项：如果两个控件都设置为拉伸类型的，则分割条无法正常工作。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| enable_split_single | false | bool | 当只有一个控件的时候，是否允许调整其宽度 |

Split 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## SplitBox的属性
SplitBox 与 Split 是相同模板实现，可用属性请参考`Split`的属性    
SplitBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## TabCtrl的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| selected_id | | int | 默认选择的子项 |
| tab_box_name | | string | 绑定的TabBox控件名称，绑定后TabCtrl的选择项变化时，TabBox的选择项会跟随变化 |

TabCtrl 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## TreeView的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| indent | | int | 树节点的缩进（每层节点缩进一个indent单位） |
| multi_select | false | bool | 是否支持多选 |
| check_box_class | | string | 显示CheckBox的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_image_class | | string | 显示展开/收起图标的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| show_icon | | string | 显示图标的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|

TreeView 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## TreeNode的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| expand_normal_image | | string | 展开时，正常状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_hot_image | | string | 展开时，悬停状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_pushed_image | | string | 展开时，按下状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_disabled_image | | string | 展开时，禁止状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_normal_image | | string | 收起时，正常状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_hot_image | | string | 收起时，悬停状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_pushed_image | | string | 收起时，按下状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_disabled_image | | string | 收起时，禁止状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_image_right_space | | int | 展开图片右侧的空隙 |
| check_box_image_right_space | | int | CheckBox图片右侧的空隙 |
| icon_image_right_space | | int | 图标右侧的空隙 |

TreeNode 控件继承了 `ListBoxItem` 属性，更多可用属性请参考`ListBoxItem`的属性

## ListCtrl的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| type | "report" | string | 类型，可选值："report"、"icon"、"list" |
| header_class | | string | ListCtrlHeader的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| header_item_class | | string | ListCtrlHeaderItem的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| header_split_box_class | | string | ListCtrlHeader/SplitBox的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| header_split_control_class | | string | ListCtrlHeader/SplitBox/Control的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| enable_header_drag_order | true | bool | 是否支持列表头拖动改变列的顺序|
| check_box_class | | string | CheckBox的Class属性(应用于Header和ListCtrl数据)，定义方法请参考`global.xml` 中的对应内容和示例程序|
| data_item_class | | string | ListCtrlItem的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| data_sub_item_class | | string | ListCtrlItem/ListCtrlSubItem的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| row_grid_line_width | | int | 横向网格线的宽度|
| row_grid_line_color | | int | 横向网格线的颜色|
| column_grid_line_width | | int | 纵向网格线的宽度|
| column_grid_line_color | | int | 纵向网格线的颜色|
| report_view_class | | string | 数据Report视图中的ListBox的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| header_height | | int | 表头控件的高度|
| data_item_height | | int | 数据项的默认高度(行高)|
| show_header | true | bool | 是否显示表头控件|
| multi_select | true | bool | 是否支持多选|
| enable_column_width_auto | true | bool | 是否支持双击Header的分割条自动调整列宽|
| auto_check_select | false | bool | 是否自动勾选选择的数据项(作用于Header与每行)|
| show_header_checkbox | false | bool | 是否在表头最左侧显示CheckBox|
| show_data_item_checkbox | false | bool | 是否在每行行首显示CheckBox|
| icon_view_class | | string | 数据Icon视图中的ListBox的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| icon_view_item_image_class | | string | 数据Icon视图中的ListBox的子项中图片的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| icon_view_item_label_class | | string | 数据Icon视图中的ListBox的子项中Label的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| list_view_class | | string | 数据List视图中的ListBox的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| list_view_item_class | | string | 数据List视图中的ListBox的子项Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| list_view_item_image_class | | string | 数据List视图中的ListBox的子项的图片的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| list_view_item_label_class | | string | 数据List视图中的ListBox的子项的Label的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| enable_item_edit | true | bool | 是否支持子项编辑|
| list_ctrl_richedit_class | | string | 编辑框的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|

ListCtrl 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## PropertyGrid的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| property_grid_xml | | string | 配置文件XML，如果为空，默认为："public/property_grid/property_grid.xml" |
| row_grid_line_width | | int | 横向网格线的宽度|
| row_grid_line_color | | int | 横向网格线的颜色|
| column_grid_line_width | | int | 纵向网格线的宽度|
| column_grid_line_color | | int | 纵向网格线的颜色|
| header_class | | string | 表头的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| group_class | | string | 分组的ClassClass属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| propterty_class | | string | 属性的ClassClass属性，定义方法请参考`global.xml` 中的对应内容和示例程序|
| left_column_width | | int | 左侧一列的宽度|

PropertyGrid 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## ColorPicker的属性
ColorPicker是一个窗口，具体用法请参考示例程序中的菜单    
ColorPicker 控件继承了 `Window` 属性，更多可用属性请参考`Window`的属性

## ControlDragable的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| drag_order | true | bool | 是否支持拖动调整顺序（在同一个容器内） |
| drag_alpha | 216 | int | 设置拖动顺序时，控件的透明度（0 - 255） |
| drag_out | true | bool | 是否支持拖出操纵（在相同窗口的不同容器内） |

ControlDragable 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性
