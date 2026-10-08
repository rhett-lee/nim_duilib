English | [简体中文](Control.md)

> Last synced: 2026-09-29

## Attributes of `Control` (Basic Control)

`Control` is the base class of all available controls, containing commonly used control attributes such as width, height, and margin. Generally it is not used as a concrete usable control, but often used as a placeholder; please refer to the example.
`Control` is the base class of all controls, containing the common available attributes of all controls.

```xml
<!-- A Control that auto-stretches according to the parent container's width, generally used as a placeholder -->
<Control width="stretch"/>
```

| Attribute name | Default value | Parameter type | Function in [Control.h](../duilib/Core/Control.h) | Purpose |
| :--- | :--- | :--- | :---| :--- |
| class |  | string | SetClass|Control style; uses the attribute values defined in the style to set this control's attributes, e.g. "btn_default"; multiple styles can be specified at the same time, separated by spaces, and must be written in the first attribute position |
| enable_vars | true | bool | SetEnableVars| Whether variable expansion is supported in attribute values.<br> Example: suppose you add a variable definition line in globle.xml: `<Define name="SIZE_ICON_SMALL" value="16"/>` <br> The variable can be used in the control attribute string in XML like this: `width="${SIZE_ICON_SMALL}"` <br> In code you can also call the function to set the attribute like this: `Control::SetAttribute(_T("width"), _T("${SIZE_ICON_SMALL}"));`|
| name |  | string | SetName|Control name; it is recommended to keep it unique within the same window, otherwise it affects query efficiency and may fail to find the correct result |
| width | stretch | int / string | SetFixedWidth|Can be set to an int or string value. When the value is int, it sets the control's width value, e.g. "100"; when the value is string, "stretch" means the parent container calculates the control width, "auto" means the width is automatically calculated based on content, and if it is a percentage value like "50%", it means the expected width of the control is 50% of the parent control's width |
| height | stretch | int / string | SetFixedHeight|Can be set to an int or string value. When the value is int, it sets the control's height value, e.g. "100"; when the value is string, "stretch" means the parent container calculates the control height, "auto" means the height is automatically calculated based on content, and if it is a percentage value like "30%", it means the expected height of the control is 30% of the parent control's height  |
| min_width | -1 | int | SetMinWidth|Minimum width of the control, e.g. "30" |
| min_height | -1 | int | SetMinHeight|Minimum height of the control, e.g. "30" |
| max_width | INT32_MAX | int / string | SetMaxWidth|Maximum width of the control; description same as width |
| max_height | INT32_MAX | int / string | SetMaxHeight|Maximum height of the control; description same as height |
| margin | 0,0,0,0 | rect | SetMargin|Margin, e.g. "2,2,2,2" |
| padding | 0,0,0,0 | rect | SetPadding|Padding, e.g. "2,2,2,2" |
| control_padding | true | bool | SetEnableControlPadding|Whether the control itself is allowed to use padding |
| halign | left | string | SetHorAlignType|Horizontal alignment of the control, e.g. "center"; supports left, center, right positions |
| valign | top | string | SetVerAlignType|Vertical alignment of the control, e.g. "center"; supports top, center, bottom positions |
| align | left,top | string | SetHorAlignType<br>SetVerAlignType |Set both horizontal and vertical alignment of the control at the same time, same function as valign and halign.<br>Optional values: left, right, hcenter, top, vcenter, bottom, separated by commas, e.g. "hcenter,vcenter" |
| float | false | bool | SetFloat|Whether to use absolute positioning, e.g. "true"|
| keep_float_pos | false | bool | SetKeepFloatPos|Set whether to keep the floating control's position relative to the parent control unchanged when the parent control's position and size are adjusted, e.g. "true"|
| bkcolor |  | string | SetBkColor|Background color string constant, e.g. "white" |
| bkcolor2 |  | string | SetBkColor2|Second background color string constant; if the second background color is set, background color gradient is supported, e.g. "blue" |
| bkcolor2_direction | "1" | string | SetBkColor2Direction|Direction of the second background color, "1": left->right, "2": top->bottom, "3": top-left->bottom-right, "4": top-right->bottom-left|
| fore_color |  | string | SetForeColor|Foreground color string; the foreground color is generally set to a semi-transparent color, e.g. "#10006DD9" |
| normal_color |  | string | SetStateColor|Color in normal state, e.g. "white" |
| hovered_color |  | string | SetStateColor|Color in hovered state, e.g. "white" |
| pressed_color |  | string | SetStateColor|Color in pressed state, e.g. "white" |
| disabled_color |  | string | SetStateColor|Color in disabled state, e.g. "white" |
| normal_color_margin |  | UiMargin | SetStateColorMargin|Set the color margin of the control in normal state |
| hovered_color_margin |  | UiMargin | SetStateColorMargin|Set the color margin of the control in hovered state |
| pressed_color_margin |  | UiMargin | SetStateColorMargin|Set the color margin of the control in pressed state |
| disabled_color_margin |  | UiMargin | SetStateColorMargin|Set the color margin of the control in disabled state |
| state_color_min_width |  | float | SetStateColorMinWidth|Set the minimum width of the state color area (solves the problem of line width distortion caused by DPI scaling precision loss after setting Margin) |
| state_color_min_height |  | float | SetStateColorMinHeight|Set the minimum height of the state color area (solves the problem of line width distortion caused by DPI scaling precision loss after setting Margin) |
| normal_color_round |  | UiSize | SetStateColorRound|Set the color rounded-corner size of the control in normal state |
| hovered_color_round |  | UiSize | SetStateColorRound|Set the color rounded-corner size of the control in hovered state |
| pressed_color_round |  | UiSize | SetStateColorRound|Set the color rounded-corner size of the control in pressed state |
| disabled_color_round |  | UiSize | SetStateColorRound|Set the color rounded-corner size of the control in disabled state |
| border_color |  | string | SetBorderColor|Set the border color in all states, e.g. "blue" |
| normal_border_color |  | string | SetBorderColor|Border color in normal state, e.g. "blue" |
| hovered_border_color |  | string | SetBorderColor|Border color in hovered state, e.g. "blue" |
| pressed_border_color |  | string | SetBorderColor|Border color in pressed state, e.g. "blue" |
| disabled_border_color |  | string | SetBorderColor|Border color in disabled state, e.g. "blue" |
| focused_border_color |  | string | SetFocusedBorderColor|Border color in focused state, e.g. "blue" |
| border_size | 0 | int / rect | SetBorderSize|Can be set to an int or rect value. When the value is int, the left, top, right and bottom all use this value as the width. When the value is rect type, the left, top, right and bottom borders are set respectively |
| border_dash_style |"solid"| string | SetBorderDashStyle |Line shape, optional values:<br>"solid": solid line<br>"dash": dashed line composed of short dashes<br>"dot": dotted line composed of dots<br>"dash_dot": alternating dash-dot line<br>"dash_dot_dot": alternating dash-dot-dot line|
| borders_on_top | true | bool | SetBordersOnTop|Whether the border is on the top layer (i.e. child controls are drawn first, then the border, to avoid the border being covered by child controls)|
| left_border_size | 0 | int | SetLeftBorderSize|Left border size, e.g. "1"; if this value is set greater than 0, the border_size attribute setting will be ignored |
| top_border_size | 0 | int | SetTopBorderSize|Top border size, e.g. "1"; if this value is set greater than 0, the border_size attribute setting will be ignored |
| right_border_size | 0 | int | SetRightBorderSize|Right border size, e.g. "1"; if this value is set greater than 0, the border_size attribute setting will be ignored |
| bottom_border_size | 0 | int | SetBottomBorderSize|Bottom border size, e.g. "1"; if this value is set greater than 0, the border_size attribute setting will be ignored |
| border_round | 0,0 | size | SetBorderRound|Border corner radius, e.g. "2,2" |
| bkimage |  | string | SetBkImage|Background image, e.g. "bk.bmp or file='aaa.jpg' res='' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' fade='255' xtiled='false' ytiled='false'" |
| normal_image |  | string | SetStateImage|Normal state image |
| hovered_image |  | string | SetStateImage|State image in hovered state |
| pressed_image |  | string | SetStateImage|State image in pressed state |
| disabled_image |  | string | SetStateImage|State image in disabled state |
| fore_normal_image |  | string | SetForeStateImage |Normal state foreground image |
| fore_hovered_image |  | string | SetForeStateImage |State foreground image in hovered state |
| fore_pressed_image |  | string | SetForeStateImage|State foreground image in pressed state |
| fore_disabled_image |  | string | SetForeStateImage|State foreground image in disabled state |
| tooltip_text |  | string | SetToolTipText|Mouse hover tooltip, e.g. "Please enter your password here" |
| tooltip_text_id |  | string | SetToolTipTextId|Mouse hover tooltip, specifies the ID of the multilingual module; when tooltip_text is empty, this attribute is displayed, e.g. "TOOL_TIP_ID" |
| tooltip_width |  | int | SetToolTipWidth| Width occupied by the mouse hover tooltip |
| data_id |  | string | SetDataID|Custom string data, an auxiliary function for user use |
| user_data_id |  | size_t | SetUserDataID|Custom integer data, for user use |
| enabled | true | bool | SetEnabled|Whether it can respond to user operations, e.g. "true"|
| mouse_enabled | true | bool | SetMouseEnabled|Whether this control can respond to mouse operations, e.g. "true"|
| keyboard_enabled | true | bool | SetKeyboardEnabled|Non-CButtonUI classes ignore this value; when false, TAB_STOP is not supported, and the object does not process keyboard messages |
| visible | true | bool | SetVisible|Whether visible, e.g. "true"|
| fade_visible | true | bool | SetFadeVisible|Whether visible, e.g. "true"; this attribute triggers the control animation effect |
| menu | false | bool | |Whether a right-click menu is needed, e.g. "true"|
| no_focus | false | bool | SetNoFocus|Whether it can acquire focus, e.g. "true"|
| tab_stop | true | bool | SetTabStop| Whether it is allowed to switch to this control by pressing the TAB key |
| show_focused_rect | false| bool | SetShowFocusedRect| Whether to display the focus state (a rectangle composed of a dashed line) |
| focused_rect_color | | string | SetFocusedRectColor| Color of the focus state rectangle |
| alpha | 255 | int | SetAlpha|Overall opacity of the control, e.g. alpha="128"; valid values are 0-255 |
| state | normal | string | SetState|Current state of the control: supports normal, hovered, pressed, disabled states |
| cursor_type | arrow | string | SetCursorType|Mouse cursor when the mouse moves over the control: <br>"arrow": arrow<br>"hand": hand<br>"wait": busy<br>"cross": crosshair<br>"ibeam": I-beam cursor, text cursor<br>"size_we": horizontal resize<br>"size_ns": vertical resize<br>"size_nwse": diagonal resize, northwest-southeast<br>"size_nesw": diagonal resize, northeast-southwest<br>"size_all": move, four-way resize<br>"no": forbidden cursor<br>"progress": progress, application startup cursor|
| render_offset | 0,0 | size | SetRenderOffset|Offset when drawing the control, e.g. "10,10"; generally used for drawing animations |
| fade_hovered | false | bool |SetFadeHovered |Whether to enable the transparent gradient animation in the control's mouse hover state, e.g. "true"|
| fade_hovered_frame_interval_ms | 16 | int |SetFadeHoveredFrameIntervalMillSeconds |Set the timer interval (milliseconds) for playing the Hovered state animation|
| fade_hovered_total_ms | 180 | int |SetFadeHoveredTotalMillSeconds |Set the total playback time (milliseconds) of the Hovered state animation|
| fade_hovered_easing_function | EaseInOutCubic | string |SetFadeHoveredEasingFunctionType |Set the easing function type of the Hovered state animation; for the supported easing function types, see the EasingFunctions::GetEasingFunctionType implementation function|
| fade_alpha | false | bool/int | GetAnimationManager().SetFadeAlpha|Whether to enable the control's transparent gradient animation. Valid values are as follows:<br>fade_alpha="false": do not enable the control's transparent gradient animation <br>fade_alpha="true": enable the control's transparent gradient animation, and the final Alpha value of the control is set to 255 <br>fade_alpha="128": enable the control's transparent gradient animation, and the final Alpha value of the control is set to 128; the valid value in this case is 1-255.|
| fade_width | false | bool | GetAnimationManager().SetFadeWidth|Whether to enable the control's width gradient animation, e.g. "true". The control width cannot be of stretch type, and this attribute must be written after the width attribute|
| fade_height | false | bool | GetAnimationManager().SetFadeHeight|Whether to enable the control's height gradient animation, e.g. "true". The control height cannot be of stretch type, and this attribute must be written after the height attribute|
| fade_size | false | bool | GetAnimationManager().SetFadeSize|Whether to enable the control's size (height and width) gradient animation, e.g. "true". The control height and width cannot be of stretch type, and this attribute must be written after the height and width attributes|
| fade_in_out_x_from_left | false | bool | GetAnimationManager().SetFadeInOutX|Whether to enable the control's left-to-right animation, e.g. "true"|
| fade_in_out_x_from_right | false | bool | GetAnimationManager().SetFadeInOutX|Whether to enable the control's right-to-left animation, e.g. "true"| 
| fade_in_out_y_from_top | false | bool | GetAnimationManager().SetFadeInOutY|Whether to enable the control's top-to-bottom animation, e.g. "true"| 
| fade_in_out_y_from_bottom | false | bool | GetAnimationManager().SetFadeInOutY|Whether to enable the control's bottom-to-top animation, e.g. "true"|
| fade_frame_interval_ms | 16 | int |GetAnimationManager().SetFrameIntervalMillSeconds |Set the timer interval (milliseconds) for playing the animation|
| fade_total_ms | 180 | int |GetAnimationManager().SetTotalMillSeconds |Set the total playback time (milliseconds) of the animation|
| fade_easing_function | EaseInOutCubic | string |GetAnimationManager().SetEasingFunctionType |Set the easing function type of the animation; for the supported easing function types, see the EasingFunctions::GetEasingFunctionType implementation function|
| loading     | | string | SetLoadingAttribute| Set the UI display-related attributes for the control's loading state; the usage is similar to the Image attribute.<br>Usage example: loading="file='loading.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'" <br> The available attributes of loading are as follows:<br> "file": XML resource file name; based on this setting, the XML resource is loaded and finally displayed in a Box container<br>"width": display width of the loading control, in pixels<br>"height": display height of the loading control, in pixels <br> "offset_x": X-direction offset of the loading control's position, relative to the top-left corner of the associated control; valid value: >= 0 <br> "offset_y": Y-direction offset of the loading control's position, relative to the top-left corner of the associated control; valid value: >= 0  <br> "halign": horizontal alignment, optional values: "left" "center" "right", only takes effect when offset_x has no valid value <br>  "valign": vertical alignment, optional values: "top" "center" "bottom", only takes effect when offset_y has no valid value<br>"fade": transparency of the loading control, valid value: 0 - 255<br>"auto_stop": after the loading animation finishes playing, automatically stop the loading state (automatically call the StopLoading() function)<br>"animation_control": name of the animation control, used for the interaction between the Loading function and the animation control on the loading control<br><br>For a complete demonstration of the loading function, please refer to the `examples/ListCtrl` example program|
| paint_order | | string | SetPaintOrder| Set the drawing order: 0 means normal drawing, non-zero means a specified drawing order; the larger the value, the later the drawing |
| start_image_animation     | | string | StartImageAnimation   | Play animation, up to 3 parameters, each separated by ',', see the function's parameter list for details |
| stop_image_animation      | | string | StopImageAnimation    | Stop animation, up to 3 parameters, each separated by ',', see the function's parameter list for details |
| set_image_animation_frame | | string | SetImageAnimationFrame| Set the current frame of the animation, up to 2 parameters, each separated by ',', see the function's parameter list for details|
| box_shadow | | string | SetBoxShadow|Set the control's shadow attribute, example: boxshadow="color='red' offset='0,0' blurradius='8' spreadradius='8'" |
| enable_drag_drop |false| bool | SetEnableDragDrop | Whether to allow drag-and-drop operations, including dropping files and dropping text|
| enable_drop_file |false| bool | SetEnableDropFile | Whether to allow dropping files|
| drop_file_types  || string | SetDropFileTypes  | List of supported file extensions for drag-and-drop file operations, e.g. ".txt;.csv" means only txt and csv files are supported; if empty, all files are supported|
| row_span  | 1 | int | SetRowSpan  | Cell merging attribute, spanning how many rows (default 1 row), only takes effect in GridLayout |
| col_span  | 1 | int | SetColumnSpan  | Cell merging attribute, spanning how many columns (default 1 column), only takes effect in GridLayout |

## Attributes of ScrollBar
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| button1_normal_image |  | string | Normal state image of the left or top button |
| button1_hovered_image |  | string | State image of the left or top button in hovered state |
| button1_pressed_image |  | string | State image of the left or top button in pressed state |
| button1_disabled_image |  | string | Disabled state image of the left or top button |
| button2_normal_image |  | string | Normal state image of the right or bottom button |
| button2_hovered_image |  | string | State image of the right or bottom button in hovered state |
| button2_pressed_image |  | string | State image of the right or bottom button in pressed state |
| button2_disabled_image |  | string | Disabled state image of the right or bottom button |
| thumb_normal_image |  | string | Normal state image of the slider |
| thumb_hovered_image |  | string | State image of the slider in hovered state |
| thumb_pressed_image |  | string | State image of the slider in pressed state |
| thumb_disabled_image |  | string | Disabled state image of the slider |
| rail_normal_image |  | string | Normal state image of the slider's middle indicator |
| rail_hovered_image |  | string | State image of the slider's middle indicator in hovered state |
| rail_pressed_image |  | string | State image of the slider's middle indicator in pressed state |
| rail_disabled_image |  | string | Disabled state image of the slider's middle indicator |
| bk_normal_image |  | string | Normal state background image |
| bk_hovered_image |  | string | Background image in hovered state |
| bk_pressed_image |  | string | Background image in pressed state |
| bk_disabled_image |  | string | Disabled state background image |
| horizontal | false | bool | Horizontal or vertical, e.g. "true"|
| line_size | 8 | int | Size of scrolling one line, e.g. "8" |
| thumb_min_length | 30 | int | Minimum length of the slider |
| range | 100 | int | Scroll range, e.g. "100" |
| value | 0 | int | Scroll position, e.g. "0" |
| show_button1 | true | bool | Whether to display the left or top button, e.g. "true"|
| show_button2 | true | bool | Whether to display the right or bottom button, e.g. "true"|
| auto_hide_scroll | true | bool | Whether to automatically hide the scrollbar, e.g. "true"|

The ScrollBar control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of Label
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| text |  | string | Display text |
| text_id |  | string | Text ID for the multilingual function |
| rich_text | false | bool | Set whether the text content is RichText <br> Usage example: `<Label rich_text="true" text="A simple <b>window</b><br/>with a <u>title bar</u> and <u>normal buttons</u>, <b>bold, <font color='#FF0000'>red font</font></b>" />` <br> Note: in RichText mode, the following functions are not supported:<br> (1) Alignment does not support justified alignment<br> (2) The vertical_text attribute is not supported (nor are vertical-text-related attributes)<br>(3) The end_ellipsis attribute is not supported<br>(4) The path_ellipsis attribute is not supported<br>(5) The auto_tooltip attribute is not supported<br>(6) The word_spacing attribute is not supported|
| text_align | "left,top" | string | Set the horizontal and vertical alignment of the text, separated by a half-width comma, e.g. "hcenter,vcenter". <br>Horizontal alignment values: left (align left), hcenter (center align), right (align right), hjustify (justify) <br>Vertical alignment values: top (align top), vcenter (center align), bottom (align bottom), vjustify (justify)|
| text_padding | 0,0,0,0 | rect | Padding of the displayed text, in the format "left,top,right,bottom", representing the padding values set on the left, top, right and bottom of the target area, e.g. "2,2,2,2" |
| font | | string | Font ID; this font ID must exist in global.xml |
| end_ellipsis | false | bool | Whether to use ... to replace incomplete end-of-line display |
| path_ellipsis | false | bool | For paths, whether to use ... to replace the middle of the path when display is incomplete |
| text_color |  | string | Normal font color; if not specified, the default color is used, e.g. "blue" |
| normal_text_color |  | string | Normal font color; if not specified, the default color is used, e.g. "blue"; this attribute is the same as the `text_color` attribute |
| hovered_text_color |  | string | Font color in hovered state; if not specified, the default color is used, e.g. "blue" |
| pressed_text_color |  | string | Font color in pressed state; if not specified, the default color is used, e.g. "blue" |
| disabled_text_color |  | string | Disabled font color; if not specified, the default color is used, e.g. "blue" |
| single_line | true | bool | Whether to output text in a single line |
| multi_line | false | bool | Whether to output text in multiple lines; mutually exclusive with the single_line attribute |
| auto_tooltip | false | bool | Whether the tooltip text displayed when hovering over the control is shown only when an ellipsis appears|
| replace_newline | false | bool | Whether to replace newline characters in the text: replace the string "\\n" with the newline character "\n", so that the two characters (\n) in brackets can be used as newlines in XML to support multi-line text. Example: the original string is "first line\\nsecond line"; when true, the two characters "\\n" are replaced with the newline character "\n", and the final string becomes "first line\nsecond line" |
| spacing_mul | 1.0f | float | Multiple of the row (column) spacing, which is a proportion of the font size (default is usually 1.0, i.e. 100% font size), used to adjust the line spacing proportionally <br> After setting, the actual line spacing is: font size * spacing_mul + spacing_add |
| spacing_add | 0 | float | Additional amount of row (column) spacing: a fixed additional pixel value (default is usually 0), used to add a fixed offset (pixels) on top of proportional adjustment <br> After setting, the actual line spacing is: font size * spacing_mul + spacing_add |
| word_spacing | 0 | float | Set the spacing (in pixels) between two adjacent characters|
| vertical_text | false | bool | Set text direction: true for vertical text, false for horizontal text <br> Horizontal text drawing direction: from left to right, top to bottom <br> Vertical text drawing direction: from top to bottom, right to left|
| use_font_height | true | bool | When drawing text vertically, set the character spacing to use the default height of the font, rather than the actual height of each character (all characters are displayed at equal height) |
| ascii_rotate_90 | true | bool | When drawing text vertically, rotate letters, digits and other characters 90 degrees clockwise for display|

The Label control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of LabelBox
LabelBox and Label are classes based on the same template; please refer to the `Label` attributes.    
The LabelBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of LabelHBox
LabelHBox and Label are classes based on the same template; please refer to the `Label` attributes.    
The LabelHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of LabelVBox
LabelVBox and Label are classes based on the same template; please refer to the `Label` attributes.    
The LabelVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of Button
The Button control inherits the `Label` attributes. For more available attributes, please refer to the `Label` attributes.

## Attributes of ButtonBox
ButtonBox and Button are classes based on the same template; please refer to the `Button` attributes.    
The ButtonBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of ButtonHBox
ButtonHBox and Button are classes based on the same template; please refer to the `Button` attributes.    
The ButtonHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of ButtonVBox
ButtonVBox and Button are classes based on the same template; please refer to the `Button` attributes.    
The ButtonVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of CheckBox
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| selected | false | bool | Whether selected |
| selected_normal_image |  | string | Image in normal state when selected |
| selected_hovered_image |  | string | State image in hovered state when selected |
| selected_pressed_image |  | string | State image in pressed state when selected |
| selected_disabled_image |  | string | Disabled state image when selected |
| selected_fore_normal_image |  | string | Foreground image in normal state when selected |
| selected_fore_hovered_image |  | string | Foreground image in hovered state when selected |
| selected_fore_pressed_image |  | string | Foreground image in pressed state when selected |
| selected_fore_disabled_image |  | string | Foreground image in disabled state when selected |
| part_selected_normal_image |  | string | Normal state image when partially selected |
| part_selected_hovered_image |  | string | State image in hovered state when partially selected |
| part_selected_pressed_image |  | string | State image in pressed state when partially selected |
| part_selected_disabled_image |  | string | Disabled state image when partially selected |
| part_selected_fore_normal_image |  | string | Foreground image in normal state when partially selected |
| part_selected_fore_hovered_image |  | string | Foreground image in hovered state when partially selected |
| part_selected_fore_pressed_image |  | string | Foreground image in pressed state when partially selected |
| part_selected_fore_disabled_image |  | string | Foreground image in disabled state when partially selected |
| selected_text_color |  | string | Font color in selected state; if not specified, the default color is used, e.g. "blue" |
| selected_normal_text_color |  | string | Font color in normal state when selected; if not specified, the default color is used, e.g. "blue" |
| selected_hovered_text_color |  | string | Font color in hovered state when selected; if not specified, the default color is used, e.g. "blue" |
| selected_pressed_text_color |  | string | Font color in pressed state when selected; if not specified, the default color is used, e.g. "blue" |
| selected_disabled_text_color |  | string | Font color in disabled state when selected; if not specified, the default color is used, e.g. "blue" |
| normal_first | false | bool | When the control is in the selected state and no background color or background image is set, draw using the corresponding attribute of the non-selected state |

The CheckBox control inherits the `Button` attributes. For more available attributes, please refer to the `Button` attributes.

## Attributes of CheckBoxBox
CheckBoxBox and CheckBox are classes based on the same template; please refer to the `CheckBox` attributes.    
The CheckBoxBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of CheckBoxHBox
CheckBoxHBox and CheckBox are classes based on the same template; please refer to the `CheckBox` attributes.    
The CheckBoxHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of CheckBoxVBox
CheckBoxVBox and CheckBox are classes based on the same template; please refer to the `CheckBox` attributes.    
The CheckBoxVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of Option
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| group |  | string | Name of the group it belongs to; under the same group name, single selection is maintained |

The Option control inherits the `CheckBox` attributes. For more available attributes, please refer to the `CheckBox` attributes.

## Attributes of OptionBox
OptionBox and Option are classes based on the same template; please refer to the `Option` attributes.    
The OptionBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of OptionHBox
OptionHBox and Option are classes based on the same template; please refer to the `Option` attributes.    
The OptionHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of OptionVBox
OptionVBox and Option are classes based on the same template; please refer to the `Option` attributes.    
The OptionVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of GroupBox
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| corner_size | "0,0" | size | Corner radius |
| line_width | 1.0 | float | Line width |
| line_color | | string | Line color |
| text | | string | Text content |

The GroupBox control inherits the `Label` attributes. For more available attributes, please refer to the `Label` attributes.

## Attributes of GroupVBox
GroupVBox and GroupBox are implemented with the same template; for available attributes, please refer to the `GroupBox` attributes.    
The GroupVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of GroupHBox
GroupHBox and GroupBox are implemented with the same template; for available attributes, please refer to the `GroupBox` attributes.    
The GroupHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of Combo
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| combo_type | "drop_down" | string | Type of the combo box: "drop_list" means a non-editable list, "drop_down" means an editable list|
| dropbox_size | | string | Size of the drop-down list (width and height)|
| popup_top | false | bool | Whether the drop-down list pops up upward |
| combo_tree_view_class | | string | Class attribute of the drop-down TreeView; for the definition method, please refer to the corresponding content in `global.xml` |
| combo_tree_node_class | | string | Class attribute of the drop-down TreeView node; for the definition method, please refer to the corresponding content in `global.xml` |
| combo_icon_class | | string | Class attribute for displaying the icon; for the definition method, please refer to the corresponding content in `global.xml` |
| combo_edit_class | | string | Class attribute of the edit control; for the definition method, please refer to the corresponding content in `global.xml` |
| combo_button_class | | string | Class attribute of the button control; for the definition method, please refer to the corresponding content in `global.xml` |
| shadow_type        | "menu" | string | Set the shadow type of the drop-down window:<br> "default": default shadow <br> "big": large shadow, right angle (suitable for normal windows)<br> "big_round": large shadow, rounded corner (suitable for normal windows)<br> "small": small shadow, right angle (suitable for normal windows)<br> "small_round": small shadow, rounded corner (suitable for normal windows)<br> "menu": small shadow, right angle (suitable for popup windows, such as menus)<br> "menu_round": small shadow, rounded corner (suitable for popup windows, such as menus)<br> "none": no shadow|

The Combo control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of FilterCombo
The FilterCombo control does not support the "combo_type" attribute.    
The FilterCombo control inherits the `Combo` attributes. For more available attributes, please refer to the `Combo` attributes.

## Attributes of ComboButton
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| dropbox_size | | string | Size of the drop-down list (width and height)|
| popup_top | false | bool | Whether the drop-down list pops up upward |
| combo_box_class | | string | Class attribute of the drop-down combo box; for the definition method, please refer to the corresponding content in `global.xml` |
| left_button_class | | string | Class attribute of the left button control; for the definition method, please refer to the corresponding content in `global.xml` |
| left_button_top_label_class | | string | Class attribute of the Label control on the upper side of the left button; for the definition method, please refer to the corresponding content in `global.xml` |
| left_button_bottom_label_class | | string | Class attribute of the Label control on the lower side of the left button; for the definition method, please refer to the corresponding content in `global.xml` |
| left_button_top_label_text | | string | Text of the Label control on the upper side of the left button |
| left_button_bottom_label_text | | string | Text of the Label control on the lower side of the left button |
| left_button_top_label_bkcolor | | string | Background color of the Label control on the upper side of the left button |
| left_button_bottom_label_bkcolor | | string | Background color of the Label control on the lower side of the left button |
| right_button_class | | string | Class attribute of the right button control; for the definition method, please refer to the corresponding content in `global.xml` |
| shadow_type        | "menu" | string | Set the shadow type of the drop-down window:<br> "default": default shadow <br> "big": large shadow, right angle (suitable for normal windows)<br> "big_round": large shadow, rounded corner (suitable for normal windows)<br> "small": small shadow, right angle (suitable for normal windows)<br> "small_round": small shadow, rounded corner (suitable for normal windows)<br> "menu": small shadow, right angle (suitable for popup windows, such as menus)<br> "menu_round": small shadow, rounded corner (suitable for popup windows, such as menus)<br> "none": no shadow|

The ComboButton control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of CheckCombo
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| dropbox | | string | Attribute information of the drop-down box; for specific settings, please refer to the example program|
| dropbox_size | | string | Size of the drop-down list (width and height)|
| popup_top | false | bool | Whether the drop-down list pops up upward |
| dropbox_item_class | | string | Attribute of each list item in the drop-down list; for specific settings, please refer to the example program|
| selected_item_class | | string | Attribute of each sub-item in the selected item; for specific settings, please refer to the example program|
| shadow_type        | "menu" | string | Set the shadow type of the drop-down window:<br> "default": default shadow <br> "big": large shadow, right angle (suitable for normal windows)<br> "big_round": large shadow, rounded corner (suitable for normal windows)<br> "small": small shadow, right angle (suitable for normal windows)<br> "small_round": small shadow, rounded corner (suitable for normal windows)<br> "menu": small shadow, right angle (suitable for popup windows, such as menus)<br> "menu_round": small shadow, rounded corner (suitable for popup windows, such as menus)<br> "none": no shadow|

The CheckCombo control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of DateTime
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| format | | string | Date format; for details, please refer to the function descriptions in `DateTime.h` |
| edit_format | | string | Edit format of the date when editing, optional values: "date_calendar": year-month-day, modify the date by displaying a month calendar in a drop-down box; "date_up_down": when editing, display year-month-day, modify the date via an up-down control placed on the right side of the control; "date_time_up_down": when editing, display year-month-day hour:minute:second; "date_minute_up_down": when editing, display year-month-day hour:minute; "time_up_down": when editing, display hour:minute:second; "minute_up_down": when editing, display hour:minute|
| spin_class | | string | Class attribute of the Spin control in the date; only valid when using SDL; default value: "rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down" |
| current_time | | bool | Initialize to the current local time (effective when `"true"` or `"1"`), convenient to use without manually calling `InitLocalTime()` in code; when used together with the `text` attribute, `current_time` takes precedence |

The DateTime control inherits the `Label` attributes. For more available attributes, please refer to the `Label` attributes.

## Attributes of Calendar
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| calendar_mode | "single" | string | Selection mode: "single" for single date, "range" for date range selection |
| first_day_of_week | "monday" | string | First day of week: "monday" or "sunday" |
| min_date | | string | Minimum selectable date (yyyy-mm-dd), empty means no limit |
| max_date | | string | Maximum selectable date (yyyy-mm-dd), empty means no limit |

The Calendar control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

The Calendar control triggers the `kEventViewModeChanged` event when its view mode (month/year/decade) changes (wParam = new view mode 0/1/2, lParam = old view mode 0/1/2), which can be subscribed via `AttachViewModeChanged`; it triggers the `kEventDisplayDateChanged` event when the displayed period (the month in month-view, the year in year/decade-view) changes, which can be subscribed via `AttachDisplayDateChanged` (typically used to refresh the flyout title bar on page-turn or cross-month focus moves); it also triggers `kEventValueChanged` when the selected date or date range changes (consistent with DateTime).

## CalendarFlyout
CalendarFlyout is a date picker flyout based on Flyout hosting a Calendar control, corresponding to header `duilib/Control/CalendarFlyout.h`. It displays a calendar panel around an anchor control, supporting single/range selection and month/year/decade navigation.

Key features:
- Inherits Flyout: 8 placement directions, auto-flip on insufficient space, click-outside/Esc to close;
- Built-in XML layout: header navigation (prev/title/next) + Calendar + footer buttons (Today/Clear);
- Single mode: auto-close and trigger `AttachDateSelected` after date selection;
- Range mode: drag to select start/end dates, auto-close and trigger `AttachDateSelected`;
- Three-level navigation: click title to cycle month/year/decade views.

### C++ Interfaces

| Interface | Description |
| :--- | :--- |
| `CalendarFlyout(Window* pParentWindow)` | Construct flyout with parent window (anchor must belong to it) |
| `bool ShowAt(Control* anchor, const struct tm& initDate, Placement = Bottom)` | Show calendar flyout around anchor; returns false if creation failed (object auto-destroyed) |
| `SetMode(int32_t mode)` | Set selection mode: 0=single, 1=range |
| `GetMode()` | Get selection mode |
| `SetInitRange(start, end)` | Set initial range for range mode |
| `SetFirstDayOfWeek(dayOfWeek)` | Set first day of week: 0=Sunday, 1=Monday (default 1) |
| `SetDateLimit(minDate, maxDate)` | Set min/max date limit (yyyy-mm-dd) |
| `AttachDateSelected(callback)` | Register date selection callback with `(WPARAM wParam, LPARAM lParam)`; wParam=0 single, 1 range; lParam is time_t for single, low32=start high32=end for range |
| `AttachDateCleared(callback)` | Register clear callback (triggered by "Clear" button) |

### Usage Example

```cpp
//Single date selection
ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
pFlyout->SetMode(0);
struct tm today = ui::Calendar::GetToday();
pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom);
pFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
    if (wParam == 0) {
        time_t t = (time_t)lParam;
        struct tm date = ui::Calendar::TimeTToDate(t);
        DString text = ui::Calendar::FormatDateString(date);
        //Handle selected date
    }
});

//Range selection
ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
pFlyout->SetMode(1);
pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom);
pFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
    if (wParam == 1) {
        time_t tStart = (time_t)(lParam & 0xFFFFFFFF);
        time_t tEnd = (time_t)((lParam >> 32) & 0xFFFFFFFF);
        struct tm start = ui::Calendar::TimeTToDate(tStart);
        struct tm end = ui::Calendar::TimeTToDate(tEnd);
        //Handle selected range
    }
});
```

The DateTime control (`edit_format="date_calendar"`) already uses CalendarFlyout internally; no manual creation needed.

## Attributes of HotKey
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| default_text | | string | Default displayed text |
| default_text_id | | string | Default displayed text ID |

The HotKey control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of HyperLink
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| url | | string | URL |
| show_url_tooltip | true | bool | Whether to display the URL as a ToolTip |

The HyperLink control inherits the `Label` attributes. For more available attributes, please refer to the `Label` attributes.

## Attributes of IPAddress
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| ip | | string | IP address, e.g. "192.168.0.0" |

The IPAddress control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of Line
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| vertical | false | bool | Whether it is a vertical line |
| line_color | | string | Color of the line |
| line_width | 1.0 | float | Width of the line |
| dash_style | | string | Line shape, optional values: "solid": solid line; "dash": dashed line composed of short dashes; "dot": dotted line composed of dots; "dash_dot": alternating dash-dot line; "dash_dot_dot": alternating dash-dot-dot line|

The Line control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of Menu
Menu is a window; for specific usage, please refer to the menu in the example program.    
The Menu control inherits the `Window` attributes. For more available attributes, please refer to the `Window` attributes.

## Attributes of Progress
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| horizontal | true | bool | Whether horizontal; "true" means horizontal, "false" means vertical |
| min | 0 | int | Minimum progress value, e.g. "0" |
| max | 100 | int | Maximum progress value, e.g. "100" |
| value | 0 | int | Progress value, e.g. "50" |
| progress_image |  | string | Foreground image of the progress bar |
| stretch_fore_image | true | bool | Specify whether the progress bar foreground image is scaled for display |
| progress_color |  | string | Foreground color of the progress bar; if not specified, the default color is used, e.g. "blue" |
| marquee | true | bool | Whether to scroll/marquee display |
| marquee_width | | int | Marquee width |
| marquee_step | | int | Marquee step |
| reverse | false | bool | Whether the progress value counts down (progress from 100 to 0) |

The Progress control inherits the `Label` attributes. For more available attributes, please refer to the `Label` attributes.

## Attributes of Slider
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| thumb_normal_image |  | string | Normal state image of the drag slider |
| thumb_hovered_image |  | string | State image of the drag slider in hovered state |
| thumb_pressed_image |  | string | State image of the drag slider in pressed state |
| thumb_disabled_image |  | string | Disabled state image of the drag slider |
| thumb_size | 10,10 | size | Size of the drag slider, e.g. "10,10" |
| step | 1 | int | Progress step, e.g. "1" |
| progress_bar_padding | 0,0,0,0 | rect | Padding reduced when drawing the slider bar |

The Slider control inherits the `Progress` attributes. For more available attributes, please refer to the `Progress` attributes.

## Attributes of CircleProgress
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| circular | true | bool | Feature switch: whether it is a ring-shaped progress bar |
| circle_width | 1 | int | Width of the ring-shaped progress bar, e.g. "10" |
| indicator |  | string | Set the moving icon for progress indication |
| clockwise | true | bool |Set the increment direction |
| bgcolor |  | string | Set the progress bar background color |
| fgcolor |  | string | Set the progress bar foreground color |
| gradient_color |  | string | Set the progress bar foreground gradient color; used together with fgcolor; if not set, there is no gradient effect |

The CircleProgress control inherits the `Progress` attributes. For more available attributes, please refer to the `Progress` attributes.

## Attributes of RichEdit/RichEdit2
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| vscrollbar | false | bool | Whether to use a vertical scrollbar, e.g. "true"|
| hscrollbar | false | bool | Whether to use a horizontal scrollbar, e.g. "true"|
| auto_vscroll | false | bool | Whether to scroll vertically with input, e.g. "true" (invalid when using SDL implementation)|
| auto_hscroll | false | bool | Whether to scroll horizontally with input, e.g. "true" (invalid when using SDL implementation)|
| want_tab | false | bool | Whether to accept the TAB key message, e.g. "true" |
| want_return | false | bool | Whether to accept the Enter key message, e.g. "true" |
| want_ctrl_return | false | bool | Whether to accept the Ctrl+Return key message, e.g. "true"|
| rich_text | false | bool | Whether to use rich format, e.g. "true" (invalid when using SDL implementation)|
| single_line | true | bool | Whether to use a single line, e.g. "true"|
| multi_line | false | bool | Whether to use multiple lines; this attribute is mutually exclusive with single_line, e.g. "true"|
| readonly | false | bool | Whether read-only, e.g. "false" |
| password | false | bool | Whether in password mode, e.g. "true"|
| show_password | false | bool | Whether to display the password character, e.g. "true"|
| password_char || string | Set the password character; default is the " * " character, which can be changed via this attribute |
| flash_password_char | false | bool | Display the character first, then display the password character|
| number_only | false | bool | Whether only numbers are allowed to be entered, e.g. "false" |
| max_number | INT_MAX | int | Maximum allowed number (only valid when number_only is true) |
| min_number | INT_MIN | int | Minimum allowed number (only valid when number_only is true) |
| text_align | "left,top" | string | Horizontal and vertical alignment of the text, optional values: left, right, hcenter, top, vcenter, bottom, separated by commas, e.g. "hcenter,vcenter" |
| text_padding |  | rect | Text padding, e.g. "2,2,2,2" |
| text |  | string | Display text |
| text_id |  | string | Multilingual function ID of the displayed text |
| font | | string | Font ID |
| normal_text_color |  | string | Normal state text color; if not specified, the default color is used, e.g. "blue" |
| disabled_text_color |  | string | Disabled state text color; if not specified, the default color is used, e.g. "blue" |
| caret_color |  | string | Color of the caret |
| prompt_mode | false | bool | Whether to display prompt text, e.g. "true"|
| prompt_text |  | string | Prompt text inside the text box; displayed when the text box's text is empty |
| prompt_text_id |  | string | Multilingual function ID, e.g. "TEXT_OUT" |
| prompt_color |  | string | Color of the prompt text inside the text box |
| focused_image |  | string | Image in focused state |
| auto_detect_url | false | bool | Whether to automatically detect URLs; if it is a URL, display it as a hyperlink (invalid when using SDL implementation)|
| limit_text | | int | Limit the maximum number of characters |
| limit_chars | | string | Restrict which characters are allowed to be entered, e.g. "abc" means only a, b, c characters are allowed, other characters are not allowed |
| allow_beep | false | bool | Whether to allow the Beep sound (invalid when using SDL implementation)|
| word_wrap | false| bool | Whether to automatically wrap lines |
| no_caret_readonly |false| bool | Read-only mode, do not display the caret |
| save_selection |false| bool | If true, when the control is inactive, the boundaries of the selection should be saved (invalid when using SDL implementation)|
| hide_selection | true | bool | Whether to hide the selection |
| zoom | | size | Set the zoom ratio: wParam: numerator of the zoom ratio, lParam: denominator of the zoom ratio. "wParam,lParam" means zooming displayed according to numerator/denominator of the zoom ratio; value range: 1/64 < (wParam / lParam) < 64. Example: "0,0" means disable the zoom function, "2,1" means enlarge to 200%, "1,2" means shrink to 50% |
| wheel_zoom | | bool | Whether to allow Ctrl + wheel to adjust the zoom ratio |
| default_context_menu | false | bool | Whether to use the default right-click menu |
| spin_class | | string | Set the Class name of the Spin function; if not empty, the Spin button is displayed; for detailed usage, please refer to the example program|
| clear_btn_class | | string | Set the Class name of the clear button function; if not empty, the clear button is displayed; for detailed usage, please refer to the example program |
| show_password_btn_class | | string |Set the Class name of the show-password button function; if not empty, the show-password button is displayed; for detailed usage, please refer to the example program |
| selection_bkcolor | "CornflowerBlue" | string |Background color of the selected text (focused state); if empty, it is not displayed|
| inactive_selection_bkcolor | "DarkGray" | string | Background color of the selected text (unfocused state); if empty, it is not displayed |
| current_row_bkcolor | "" | string | Background color of the current row (focused state); if empty, the current row's background color is not displayed in the focused state|
| inactive_current_row_bkcolor | "" | string |Background color of the current row (unfocused state); if empty, the current row's background color is not displayed in the unfocused state  |
| select_all_on_focused |false| bool | Whether to select all when gaining focus |
| focused_bottom_border_size |0| int | Size of the bottom border in the focused state |
| focused_bottom_border_color || string | Color of the bottom border in the focused state |
| enable_drag_drop |false| bool   | Whether to allow drag-and-drop operations|
| enable_drop_file |false| bool   | Whether to allow drag-and-drop file operations|
| enable_drag_out  |true | bool   | Whether to allow text drag-out function (as a drag source); only supported on the Windows platform, not on other platforms; this option is only supported in RichEdit2|
| drop_file_types  |     | string | List of supported file extensions for drag-and-drop file operations, e.g. ".txt;.csv" means only txt and csv files are supported; if empty, all files are supported|
| row_spacing_mul  | 1.0 | float  | Line spacing multiple, e.g. 1.5 means 1.5x line spacing<br>Windows platform: only valid when the rich_text attribute is "true", because the Windows RichEdit control only supports setting line spacing in rich text mode;<br>When using SDL, it is always valid, i.e. on other platforms the line spacing attributes are all valid|
| row_spacing_add  |0    | float  | Line spacing additional amount: a fixed additional pixel value (default is usually 0), used to add a fixed offset (pixels) on top of proportional adjustment; only valid when using SDL|

The RichEdit control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.    
Functional description of the RichEdit2 class:    
(1) On the Windows platform, the RichEdit class is implemented using the Windows system's own ITextServices interface, while RichEdit2 is implemented by this project itself; the two have different implementation methods but basically the same functionality.    
(2) On non-Windows platforms, RichEdit is an alias of RichEdit2, and the two are identical.    

## Attributes of SpinBox
SpinBox is a numeric input control derived from RichEdit. It enables `number_only` mode by default and provides numeric input plus spin buttons and up/down arrow key adjustment.    
In addition to RichEdit's common attributes (`spin_class`, `min_number`, `max_number`, `limit_text`, etc.), it adds the following attributes:    

| Attribute | Default | Type | Purpose |
| :--- | :--- | :--- | :--- |
| step | 1 | int | Step value (positive integer); the amount added/subtracted by each spin button click or up/down arrow key press |
| value | 0 | int | Initial numeric value (clamped to the min/max range if out of bounds) |

Corresponding C++ interfaces: `SetStep/GetStep`, `SetValue/GetValue`, `SetRange(min, max)`.    
The `kEventTextChanged` event is fired when the value changes (same as RichEdit).    
Example (see the rich_edit example program):    

```xml
<SpinBox class="simple simple_border rich_edit_spin" min_number="0" max_number="100" step="5" value="50"/>
```

The `spin_class` styles (rich_edit_spin_box / rich_edit_spin_btn_up / rich_edit_spin_btn_down) are defined in the `rich_edit_spin` class of global.xml.    

## Attributes of SearchBox
SearchBox is a composite search input control derived from HBox. It combines a search icon on the left, a text edit box (RichEdit), and a clear button on the right, intended for keyword input and search scenarios. Header file: `duilib/Control/SearchBox.h`.

| Attribute | Default | Type | Purpose |
| :--- | :--- | :--- | :--- |
| text | "" | string | Initial text of the search box |
| prompt_text | "" | string | Placeholder text shown when the edit box is empty |
| prompt_text_id | "" | string | Multi-language string ID of the placeholder text |
| prompt_color | "" | string | Color of the placeholder text (semantic color name) |

Corresponding C++ interfaces: `SetSearchText/GetSearchText`, `GetEditControl()` (returns the inner RichEdit control).
The `kEventTextChanged` event is fired when the text changes; the `kEventReturn` event is fired when the Enter key is pressed (can be used to start a search).
The clear button reuses RichEdit's built-in `clear_btn_class` mechanism: it is shown when the edit box is focused and has text, clears the text when clicked, and is automatically hidden after losing focus.
Example (see the rich_edit example program):

```xml
<SearchBox class="search_box" prompt_text="Type to search"/>
```

The default skin defines four classes in global.xml: `search_box` (outer frame), `search_box_icon` (magnifier icon), `search_box_edit` (inner edit box), and `search_box_clear_btn` (clear button).

## Attributes of Switch
Switch is a toggle switch control derived from CheckBox, used for on/off state switching. Header file: `duilib/Control/Switch.h`.
Both the track and the thumb can be self-drawn with semantic colors or skinned with SVG images: when images are configured, the off/on images cross-fade with the animation progress while the thumb image slides along the track; when no images are configured, colors are self-drawn.

In addition to the common attributes of CheckBox, the following new attributes are added:

| Attribute | Default | Type | Purpose |
| :--- | :--- | :--- | :--- |
| switch_animation_ms | 160 | int | Transition animation duration in milliseconds (0 disables animation) |
| track_off_color | bg_switch_track_off | string | Track color in unselected state (semantic color name, effective without track images) |
| track_on_color | bg_switch_track_on | string | Track color in selected state (semantic color name, effective without track images) |
| thumb_off_color | bg_switch_thumb_off | string | Thumb color in unselected state (semantic color name, effective without thumb images) |
| thumb_on_color | bg_switch_thumb_on | string | Thumb color in selected state (semantic color name, effective without thumb images) |
| track_off_image | | string | Track image in unselected state (image attribute string, paired with track_on_image) |
| track_on_image | | string | Track image in selected state (image attribute string, paired with track_off_image) |
| thumb_off_image | | string | Thumb image in unselected state (square image, paired with thumb_on_image) |
| thumb_on_image | | string | Thumb image in selected state (square image, paired with thumb_off_image) |
| thumb_padding | 0 | int | Distance between the thumb image square box and the track edges (auto DPI scaled). The square box side equals the track height; shadow padding must be drawn inside the image itself |

Notes:
- Track images are stretched to fill the whole track rectangle; thumb images translate inside a square box as high as the track. Do not set width/height/valign/halign in the image attribute string, otherwise the draw position will be fixed.
- Fixed colors inside images can be replaced with semantic colors via `svg_replace_colors` (multiple replacements separated by semicolons) to adapt to light/dark themes.
- Track and thumb layers are independent: when only thumb images are configured, the track is still self-drawn with colors (this is how the iOS-style skin works).

Corresponding C++ interfaces: `SetAnimationDuration/GetAnimationDuration`, `SetTrackOffColor/GetTrackOffColor`, `SetTrackOnColor/GetTrackOnColor`, `SetThumbOffColor/GetThumbOffColor`, `SetThumbOnColor/GetThumbOnColor`, `SetTrackOffImage/GetTrackOffImage`, `SetTrackOnImage/GetTrackOnImage`, `SetThumbOffImage/GetThumbOffImage`, `SetThumbOnImage/GetThumbOnImage`, `SetThumbPadding/GetThumbPadding`.
Selection state changes trigger `kEventSelect`/`kEventUnSelect` events (same as CheckBox).

### XML Usage Example (see the controls and color_theme example programs)

```xml
<!-- Using default skins (classes defined in global.xml) -->
<Switch class="switch" selected="false"/>
<Switch class="switch_fluent" selected="true"/>
<Switch class="switch_material" selected="true"/>

<!-- Pure color self-drawing: no images configured, specify track/thumb colors (semantic color names or color values) -->
<Switch width="51" height="31" selected="true"
        track_off_color="bg_switch_track_off" track_on_color="bg_switch_track_on"
        thumb_off_color="bg_switch_thumb_off" thumb_on_color="bg_switch_thumb_on"/>

<!-- Custom image skin: off/on track images filling the track + off/on square thumb images (see switch_fluent) -->
<Switch width="48" height="24" padding="2,2,2,2" cursor_type="hand"
        track_off_image="file='public/switch/switch_fluent_track_off.svg' svg_replace_colors='#E9E9EA|bg_switch_track_off;#8A8886|bg_switch_thumb_off'"
        track_on_image="file='public/switch/switch_fluent_track_on.svg' svg_replace_colors='#0078D4|bg_switch_track_on'"
        thumb_off_image="file='public/switch/switch_fluent_thumb_off.svg' svg_replace_colors='#8A8886|bg_switch_thumb_off'"
        thumb_on_image="file='public/switch/switch_fluent_thumb_on.svg'"/>

<!-- XML events: link other controls on state change (type is select/unselect, same as CheckBox) -->
<Switch class="switch">
    <Event type="select" receiver="target_control" apply_attribute="visible='true'"/>
    <Event type="unselect" receiver="target_control" apply_attribute="visible='false'"/>
</Switch>
```

Three skin classes are defined in global.xml by default (SVG assets are in the `public/switch/` directory):
- `switch` (alias `switch_ios`): iOS style, 51x31, large white thumb with soft shadow, track color follows the theme accent color;
- `switch_fluent`: Windows 11 Fluent style, 48x24 (padding=2, track 44x20), outlined small track + 14px small thumb;
- `switch_material`: Material Design 3 style, 52x32; unselected shows an outlined track with a 16px small thumb, selected shows an accent-filled track with a 24px large thumb carrying a check mark.

Semantic colors are defined in the `color_light` and `color_dark` global.xml files as `bg_switch_track_on/off` and `bg_switch_thumb_on/off`; SVG colors in the Fluent/Material skins are mapped to these semantic colors or `color_accent` via `svg_replace_colors`.

### C++ Usage Example

```cpp
// Get the control pointer (name="switch_test" in XML)
ui::Switch* pSwitch = dynamic_cast<ui::Switch*>(pWindow->FindControl(_T("switch_test")));
if (pSwitch != nullptr) {
    // Listen for state change events (same as CheckBox)
    pSwitch->AttachSelect([](const ui::EventArgs& args) {
        // Switched to selected state
        return true;
        });
    pSwitch->AttachUnSelect([](const ui::EventArgs& args) {
        // Switched to unselected state
        return true;
        });

    // Read/toggle the selected state dynamically (plays the slide animation and fires select/unselect events)
    bool bSelected = pSwitch->IsSelected();
    pSwitch->Selected(!bSelected);

    // Set the appearance dynamically (color self-drawing; use SetTrackXxxImage/SetThumbXxxImage for image skins)
    pSwitch->SetTrackOnColor(_T("#FF0078D4"));
    pSwitch->SetAnimationDuration(200);
}
```

## Attributes of Badge
Badge is a badge control derived from Label, used to display unread counts/new message counts/red dots on TabCtrl tabs, buttons, icons and other controls. Header file: `duilib/Control/Badge.h`.
Two forms: count badge (rounded capsule background + number, shows "99+" when exceeding the limit) and dot badge (dot mode, a small round dot). The badge is automatically hidden when count <= 0 (no badge when there are no unread items); in dot mode, visibility is controlled by the visible attribute.
Badge inherits the text attributes of Label (font, normal_text_color, text_padding, etc.); the background is a rounded capsule self-drawn with semantic colors (corner radius = half the height), automatically adapting to light/dark themes.

| Attribute | Default | Type | Purpose |
| :--- | :--- | :--- | :--- |
| count | 0 | int64 | Badge count: shows the badge when greater than 0, automatically hidden when less than or equal to 0 |
| max_count | 99 | int64 | Count limit, shows "limit+" (e.g. "99+") when exceeded |
| dot | false | bool | Dot mode: true shows a pure small round dot without a number |
| badge_color | bg_badge | string | Badge background color (semantic color name or color value) |

Notes:
- Badge is used as a child control inside any Box container; to float it on the corner of a host control, use `float="true"` with the `margin` attribute.
- The count bubble width adapts to the number of digits (`width="auto"`); a single digit appears as a circle; the dot size is determined by the XML width/height (default skin 8x8).
- Child controls are clipped at the parent container boundary; ensure the parent container is large enough for a floating badge (see the controls example: parent Box width = button width + badge overhang).
- Badge does not respond to the mouse (`mouse_enabled="false"`), so it does not block clicks on the host control.

Corresponding C++ interfaces: `SetCount/GetCount`, `SetMaxCount/GetMaxCount`, `SetDotMode/IsDotMode`, `SetBadgeColor/GetBadgeColor`.
Usage example (see the controls example program):

```xml
<!-- Default skin: count badge (automatically hidden when count <= 0) -->
<Badge class="badge" count="5"/>
<Badge class="badge" count="120"/>   <!-- Shows "99+" -->

<!-- Dot badge -->
<Badge class="badge_dot"/>

<!-- Floating on the button's top-right corner: parent Box width = button width + badge overhang, positioned with float+margin -->
<Box width="66" height="30">
    <Button class="btn_global" text="Messages" width="48" height="30"/>
    <Badge class="badge" count="6" float="true" margin="42,0,0,0"/>
</Box>

<!-- Badge on a TabCtrl tab: TabCtrlItem has built-in badge_count/badge_dot/badge_class attributes, no manual nesting needed -->
<TabCtrlItem class="tab_ctrl_item" title="Messages" badge_count="3"/>
```

Updating the unread count dynamically in C++:

```cpp
ui::Badge* pBadge = dynamic_cast<ui::Badge*>(pWindow->FindControl(_T("badge_msg")));
if (pBadge != nullptr) {
    pBadge->SetCount(pBadge->GetCount() + 1);   // Unread +1, shows "99+" when exceeding 99
    pBadge->SetCount(0);                        // Reset to zero, the badge is hidden automatically
}
```

Two skin classes are defined in global.xml by default:
- `badge`: count badge, height 18, white text on red background (semantic colors `text_badge`/`bg_badge`), width adapts to the digits, text_padding="5,0,5,0";
- `badge_dot`: dot badge, 8x8, dot="true".

The semantic color `bg_badge` is defined in the color_light and color_dark global.xml files, derived from `color_error` (red); the count text color `text_badge` is white.

## Flyout Window
Flyout is a general-purpose popup (flying card) window derived from WindowImplBase. Header file: `duilib/Control/Flyout.h`. It is used to pop up arbitrary Box content around an anchor control (action panels, confirmation cards, rich-content tips, etc.), and serves as the common base for calendar popups, search suggestions and settings bubbles.

Key features:
- Anchor positioning: 8 placements (bottom/top/left/right x start/end aligned); when the preferred side has no room it automatically flips to the opposite side, and is finally clamped inside the monitor work area;
- Non-focus-stealing by default (`WS_EX_NOACTIVATE`): the parent window stays active while the flyout is shown and can keep receiving input;
- Automatically closed by clicking outside or pressing Esc by default; `Dismiss` closes it manually;
- Modeless window: the framework deletes the object automatically after the window closes, callers do not need to delete it;
- Only one active flyout at a time: showing a new flyout closes the previous one;
- The flyout is a layered popup window with shadow (`WS_POPUP | WS_EX_LAYERED`); the content XML is drawn with current-theme semantic colors, adapting to light/dark themes automatically.

### C++ Interfaces

| Interface | Description |
| :--- | :--- |
| `Flyout(Window* pParentWindow)` | Constructs a flyout with the parent window (the anchor control must belong to it) |
| `SetSkinFolder(path)` | Resource folder of the content XML; defaults to the parent window's resource path |
| `bool ShowAt(Control* anchor, const DString& xmlFile, Placement = Bottom)` | Shows the flyout around the anchor; on failure the object is already deleted and must not be accessed |
| `Dismiss()` | Closes the flyout manually (reason kManual) |
| `SetAutoDismiss(bool)` / `IsAutoDismiss()` | Whether click-outside/Esc closes automatically, default true |
| `SetNoFocus(bool)` / `IsNoFocus()` | Whether to avoid taking focus, default true; when false, focus loss is used to detect outside clicks |
| `SetGap(int)` / `GetGap()` | Gap between the flyout and the anchor (unscaled logical pixels), default 6 |
| `SetAllowFlip(bool)` / `IsAllowFlip()` | Whether flipping to the opposite side is allowed, default true |
| `GetAnchor()` | Returns the anchor control (may be nullptr after close or anchor destruction) |
| `GetPlacement()` | Returns the preferred placement |
| `IsOpen()` | Whether the flyout is open |
| `AttachOpened(callback)` / `AttachClosed(callback)` | Registers opened/about-to-close callbacks with a `CloseReason` argument |
| `static GetActiveFlyout()` | Returns the current active flyout, or nullptr |
| `static DismissActive()` | Dismisses the current active flyout (no-op if none) |

`Placement` enum: `Bottom` (below, start-aligned, default), `BottomEnd` (below, end-aligned), `Top`, `TopEnd`, `Right`, `RightEnd`, `Left`, `LeftEnd`.

`CloseReason` enum: `kManual` (manual close), `kClickOutside` (click outside / focus loss), `kEscape` (Esc key), `kAnchorLost` (anchor or parent window invalidated: destroyed/hidden/minimized/DPI changed).

### Writing the Content XML

The flyout window itself is created and styled in C++; the caller only provides a **content XML**. Its root is an ordinary `<Window>` with shadow and layered window enabled, and a fixed size fitting the content (the framework wraps a shadow around it):

```xml
<Window size="240,210" caption="0,0,0,0" use_system_caption="false"
        shadow_type="default" shadow_attached="true"
        layered_window="true" size_box="0,0,0,0">
    <VBox class="flyout" width="240" height="auto">
        <Label class="flyout_title" text="Flyout Title"/>
        <Label class="flyout_desc" text="Description text..." margin="0,6,0,10"/>
        <Button class="btn_primary_global" name="flyout_action" text="Primary Action"
                width="stretch" height="30" border_round="4,4"/>
    </VBox>
</Window>
```

Notes:
- The window size is in unscaled logical pixels; make the height large enough for all content (overflow would be painted over the shadow area);
- Name interactive controls with `name` and bind events via `FindControl` after `ShowAt` succeeds;
- The default skin defines 3 classes in global.xml: `flyout` (240-wide card, `bg_window_card` background, `border_window` border, corner radius 8), `flyout_title` (14pt bold title), `flyout_desc` (12pt `text_muted` multiline description).

### Usage Example

```cpp
//Called when the anchor button is clicked (clicking the same anchor again closes it)
void ShowFlyout(ui::Control* pAnchor)
{
    ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
    if (pActive != nullptr) {
        if (pActive->GetAnchor() == pAnchor) {
            pActive->Dismiss();      // Same anchor: toggle off
            return;
        }
        pActive->Dismiss();          // Different anchor: the old flyout closes
    }

    ui::Flyout* pFlyout = new ui::Flyout(this);
    pFlyout->SetSkinFolder(GetResourcePath().ToString());
    //The XML file name is resolved relative to SkinFolder, without a subfolder prefix
    if (!pFlyout->ShowAt(pAnchor, _T("my_flyout.xml"), ui::Flyout::Placement::Bottom)) {
        return;                      // On failure the object is already deleted
    }

    ui::Button* pAction = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_action")));
    if (pAction != nullptr) {
        pAction->AttachClick([pFlyout](const ui::EventArgs&) {
            //Do the action, then close the flyout
            pFlyout->Dismiss();
            return true;
            });
    }
}
```

Remarks:
- In no-focus mode, controls inside the flyout still receive clicks, but keyboard focus stays on the parent window; call `SetNoFocus(false)` when text input inside the flyout is required;
- Outside-click detection polls the global mouse button state (Windows only); on non-Windows platforms use the focus mode via `SetNoFocus(false)`;
- When the anchor is nested inside scrollable containers, the accumulated scroll offset is automatically removed, so the flyout always pops at the control's visual position after scrolling;
- When the parent window uses a private color theme via `OpenColorTheme` (e.g. the dark window in the ColorTheme demo), the flyout automatically inherits the same color set without extra setup;
- When the display DPI changes (e.g. moving across monitors), the flyout closes automatically with reason `kAnchorLost` to avoid wrong position/size.

See the controls demo (the "Flyout" group with 4 placement buttons) and the ColorTheme demo for complete examples.

## Attributes of RichText
RichText is formatted text whose format is similar to HTML tags; the formatted text starts with `<RichText>` and ends with `</RichText>`.    
Example: <RichText>RichText demo: <a href="URL">text</a></RichText>    
List of supported tags:    
```cpp
   // List of supported tags (compatible with HTML tags):
   // Hyperlink:   <a href="URL">text</a>
   // Bold:      <b> </b>
   // Italic:      <i> </i>
   // Strikethrough:      <s> </s>  or <del> </del>  or <strike> </strike>
   // Underline:    <u> </u>
   // Set background color:  <bgcolor color="#000000"> </bgcolor>
   // Set font:    <font face="SimSun" size="12" color="#000000">
   // Line break tag:   <br/>
```

| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| text_align | "left,top" | string | Horizontal and vertical alignment of the text, optional values: left, right, hcenter, top, vcenter, bottom, separated by commas, e.g. "hcenter,vcenter" |
| text_padding |  | rect | Text padding, e.g. "2,2,2,2" |
| font | | string | Font ID |
| text_color | | string | Default text color |
| replace_brace | true | bool | When setting the text attribute, whether to allow replacing '{' with '<' and '}' with '>'; this attribute must be placed before the text attribute to take effect, e.g. replace_brace="false" means replace is disabled |
| text | | string | Set the formatted text content, where '{' can be used instead of '<' and '}' instead of '>', thus avoiding escape characters and making it easier to read |
| text_id | | string | Set the formatted text content ID, where the corresponding content allows using '{' instead of '<' and '}' instead of '>', thus avoiding escape characters and making it easier to read |
| trim_policy | "all" | string | Set the text trim policy: "all" means remove all spaces; "none" means no need to remove spaces; "keep_one" means keep only one space |
| default_link_font_color | | string | Hyperlink: normal text color value |
| hovered_link_font_color | | string | Hyperlink: Hover state text color value |
| pressed_link_font_color | | string | Hyperlink: pressed state text color value |
| link_font_underline | true | bool | Hyperlink: whether to use underlined font |
| row_spacing_mul | 1.0 | float | Line spacing multiple, e.g. 1.5 means 1.5x line spacing |
| row_spacing_add  |0    | float  | Line spacing additional amount: a fixed additional pixel value (default is usually 0), used to add a fixed offset (pixels) on top of proportional adjustment|
| word_wrap | true| bool | Whether to automatically wrap; if false, line breaks only occur at the `<br/>` tag |

The RichText control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of RichTextBox
RichTextBox and RichText are classes based on the same template; please refer to the `RichText` attributes.    
The RichTextBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of RichTextHBox
RichTextHBox and RichText are classes based on the same template; please refer to the `RichText` attributes.    
The RichTextHBox control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of RichTextVBox
RichTextVBox and RichText are classes based on the same template; please refer to the `RichText` attributes.    
The RichTextVBox control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of Split
The splitter control can change the width or height of the left/right or top/bottom controls by dragging the splitter. Application method:     
If placed in a horizontal layout (HLayout), drag left and right.    
If placed in a vertical layout (VLayout), drag up and down.    
Note: if both controls are set to stretch type, the splitter will not work properly.

| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| enable_split_single | false | bool | Whether to allow adjusting its width when there is only one control |

The Split control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of SplitBox
SplitBox and Split are implemented with the same template; for available attributes, please refer to the `Split` attributes.    
The SplitBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of TabCtrl
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| selected_id | | int | Default selected child item |
| tab_box_name| | string | Name of the bound TabBox control; after binding, when the TabCtrl's selected item changes, the TabBox's selected item changes accordingly |
| drag_order  | true | bool | Whether to support drag to adjust order (within the same tab); enabled by default |
| drag_out_id | 0 | int | Whether to support drag-out of this container: if not 0, dragging out is supported; otherwise it is not (dragged out to a container whose drop_in_id == drag_out_id)|
| drop_in_id | 0 | int | Whether to support drag-and-drop into this container: if not 0, dragging in is supported; otherwise it is not (dragged in from a container whose drag_out_id == drop_in_id)|
| selected_tab_item_outline_width  | 0 | float  | Width of the outer outline of the selected tab item |
| selected_tab_item_outline_color  |   | string | Color of the outer outline of the selected tab item |
| tab_ctrl_bottom_line_height      | 0 | float  | Height of the bottom line of the tab bar (this line is not drawn in the selected tab area, but drawn in other areas) |
| tab_ctrl_bottom_line_color       |   | string | Set the color of the bottom line of the tab bar |

The TabCtrl control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.

## Attributes of TabCtrlItem
| Attribute name | Default value | Parameter type | Purpose |
| :---     | :---   | :---     | :--- |
| tab_box_item_index | | int | Index number of the bound TabBox child item (i.e. clicking this tab switches to the TabBox page with this index number) |
| title | | string | Title text of the tab page |
| title_id | | string | Title text ID of the tab page (for multilingual support) |
| title_class | | string | Class value of the title text resource attribute of the tab page |
| icon | | string | Icon resource string of the tab page |
| icon_class | | string | Class value of the icon resource attribute of the tab page |
| close_button_class | | string | Class value of the close button resource attribute of the tab page |
| line_class | | string | Class value of the divider resource attribute of the tab page |
| selected_round_corner | | size | Rounded corner size of the tab page in selected state|
| hovered_round_corner | | size | Rounded corner size of the tab page in hovered state|
| hovered_padding | | UiPadding | Padding of the background color of the tab page in hovered state|
| auto_hide_close_button | false | bool | Whether the close button is automatically hidden|
| badge_count | 0 | int64 | Badge count: shows the badge when greater than 0, automatically hidden when less than or equal to 0; the badge control is not created unless badge_class is set |
| badge_max_count | 99 | int64 | Badge count limit; when exceeded, "limit+" is displayed (e.g. "99+") |
| badge_dot | false | bool | Badge dot mode: true for a plain dot without digits; should be used together with badge_class or the badge attribute |
| badge_class | | string | Class value of the badge control (e.g. `tab_ctrl_item_badge`); the badge control is not created when empty |

**Notes:**
- TabCtrlItem has built-in badge support: specify a `Badge`-based Class via `badge_class`, then configure the display with `badge_count`/`badge_dot`/`badge_max_count`. Without `badge_class`, badge attributes only store values and no Badge child control is created.
- The default skin defines `tab_ctrl_item_badge` (count badge, height 16, white text on red background) and `tab_ctrl_item_badge_dot` (dot badge, 8x8) in global.xml, already attached to `badge_class` of `tab_ctrl_item`.
- The badge child control does not handle mouse events (`mouse_enabled="false"`), so it does not block tab clicks.

**Corresponding C++ interfaces:** `SetBadgeCount/GetBadgeCount`, `SetBadgeMaxCount/GetBadgeMaxCount`, `SetBadgeDot/IsBadgeDot`, `SetBadgeClass/GetBadgeClass`, `GetBadgeControl`.

**XML example:**

```xml
<TabCtrl class="tab_ctrl" width="stretch" height="36">
    <!-- Count badge (count=5) -->
    <TabCtrlItem class="tab_ctrl_item" title="Messages" badge_count="5"/>
    <!-- Overflow badge (shows "99+") -->
    <TabCtrlItem class="tab_ctrl_item" title="Notifications" badge_count="120"/>
    <!-- Dot badge (use the dedicated dot Class) -->
    <TabCtrlItem class="tab_ctrl_item" title="Feed" badge_class="tab_ctrl_item_badge_dot" badge_dot="true"/>
    <!-- No badge -->
    <TabCtrlItem class="tab_ctrl_item" title="Settings"/>
</TabCtrl>
```

Updating the badge dynamically in C++:

```cpp
ui::TabCtrlItem* pTabItem = dynamic_cast<ui::TabCtrlItem*>(pWindow->FindControl(_T("tab_msg")));
if (pTabItem != nullptr) {
    pTabItem->SetBadgeCount(pTabItem->GetBadgeCount() + 1);   // Unread +1
    pTabItem->SetBadgeCount(0);                               // Reset to zero, the badge is hidden automatically
}
```

The TabCtrlItem control inherits the `ControlDragableT` attributes. For more available attributes, please refer to the `ControlDragableT` attributes.

## Attributes of ControlDragableT (template class)
| Attribute name | Default value | Parameter type | Purpose |
| :---     | :---   | :---     | :--- |
| drag_order  | true | bool | Whether to support drag to adjust order (within the same container), enabled by default |
| drag_out    | true| bool | Whether to support drag-out operation (between different containers in the same window), enabled by default |
| drag_alpha  | 216 | uint8_t | Opacity of the control when dragging to reorder |

## Attributes of ControlDragable
The ControlDragable control inherits `ControlDragableT` and `Control` attributes. For more available attributes, please refer to the `ControlDragableT` and `Control` attributes.

## Attributes of BoxDragable
The BoxDragable control inherits `ControlDragableT` and `Box` attributes. For more available attributes, please refer to the `ControlDragableT` and `Box` attributes.

## Attributes of HBoxDragable
The HBoxDragable control inherits `ControlDragableT` and `HBox` attributes. For more available attributes, please refer to the `ControlDragableT` and `HBox` attributes.

## Attributes of VBoxDragable
The VBoxDragable control inherits `ControlDragableT` and `VBox` attributes. For more available attributes, please refer to the `ControlDragableT` and `VBox` attributes.

## Attributes of ControlMovableT (template class)
| Attribute name | Default value | Parameter type | Purpose |
| :---     | :---   | :---     | :--- |
| enable_move_pos  | true | bool    | Whether to support drag to adjust the control's position, enabled by default |
| move_pos_draggable_border |   | UiPadding | The border range of the control's movable rectangle (the surrounding area can be clicked to drag, but the center area cannot be dragged) |
| move_pos_non_draggable_margin |  | UiMargin | The margin of the control's movable rectangle (the surrounding area defined by the margin cannot be clicked to drag, only the center area can be dragged) |
| move_parent_pos  | false| bool    | When performing the drag-to-adjust-position operation, whether to adjust the parent container's position; "true" means adjust the parent container's position, "false" means adjust the control's own position |
| move_pos_alpha   | 216  | uint8_t | Opacity of the control when dragging to adjust position |
| move_pos_reserve_width   | 20  | int | When moving horizontally, the height reserved within the parent container to avoid the control completely overflowing the parent container (not DPI-scaled) |
| move_pos_reserve_height   | 20  | int | When moving vertically, the width reserved within the parent container to avoid the control completely overflowing the parent container (not DPI-scaled) |
| move_pos_keep_within_parent   | false  | bool | When moving the control, ensure the child control is within the parent container without overflow |

## Attributes of ControlMovable
The ControlMovable control inherits `ControlMovableT` and `Control` attributes. For more available attributes, please refer to the `ControlMovableT` and `Control` attributes.

## Attributes of BoxMovable
The BoxMovable control inherits `ControlMovableT` and `Box` attributes. For more available attributes, please refer to the `ControlMovableT` and `Box` attributes.

## Attributes of HBoxMovable
The HBoxMovable control inherits `ControlMovableT` and `HBox` attributes. For more available attributes, please refer to the `ControlMovableT` and `HBox` attributes.

## Attributes of VBoxMovable
The VBoxMovable control inherits `ControlMovableT` and `VBox` attributes. For more available attributes, please refer to the `ControlMovableT` and `VBox` attributes.

## Attributes of ControlResizableT (template class)
| Attribute name | Default value | Parameter type | Purpose |
| :---     | :---   | :---     | :--- |
| enable_resize   | true  | bool | Whether to support mouse dragging to change the control's size |
| enable_move_pos  | false | bool    | Whether to support drag to adjust the control's position, disabled by default; if enabled, related attributes can refer to `ControlMovableT` |
| resize_size_box   | | UiRect | Set the stretch range size when resizing the control on its four edges |
| resize_reserve_width   | 10| int | Set the minimum width reserved when resizing (not DPI-scaled) |
| resize_reserve_height   | 10| int | Set the minimum height reserved when resizing (not DPI-scaled) |
| resize_keep_within_parent| false | bool | Set whether to ensure the child control is within the parent container without overflow when resizing |

The ControlResizableT control inherits the `ControlMovableT` attributes. For more available attributes, please refer to the `ControlMovableT` attributes.

## Attributes of ControlResizable
The ControlResizable control inherits `ControlResizableT` and `Control` attributes. For more available attributes, please refer to the `ControlResizableT` and `Control` attributes.

## Attributes of BoxResizable
The BoxResizable control inherits `ControlResizableT` and `Box` attributes. For more available attributes, please refer to the `ControlResizableT` and `Box` attributes.

## Attributes of HBoxResizable
The HBoxResizable control inherits `ControlResizableT` and `HBox` attributes. For more available attributes, please refer to the `ControlResizableT` and `HBox` attributes.

## Attributes of VBoxResizable
The VBoxResizable control inherits `ControlResizableT` and `VBox` attributes. For more available attributes, please refer to the `ControlResizableT` and `VBox` attributes.

## Attributes of ListBoxItem
ListBoxItem is a concrete implementation of the template class ListBoxItemTemplate, defined in the file `duilib/Box/ListBoxItem.h`. There are three related type definitions:    
```
typedef ListBoxItemTemplate<Box> ListBoxItem;
typedef ListBoxItemTemplate<HBox> ListBoxItemH;
typedef ListBoxItemTemplate<VBox> ListBoxItemV;
```
As a child item in the ListBox container, ListBoxItem itself does not define any attributes.
The ListBoxItem inherits the `Option` attributes. For more available attributes, please refer to the `Option` attributes.

## Attributes of TreeView
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| indent | | int | Indentation of tree nodes (each level of node is indented by one indent unit) |
| multi_select | false | bool | Whether to support multi-selection |
| check_box_class | | string | Class attribute for displaying the CheckBox; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| expand_image_class | | string | Class attribute for displaying the expand/collapse icon; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| show_icon | | string | Class attribute for displaying the icon; for the definition method, please refer to the corresponding content in `global.xml` and the example program|

The TreeView control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.

## Attributes of TreeNode
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| expand_normal_image | | string | Image in normal state when expanded; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| expand_hovered_image | | string | Image in hovered state when expanded; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| expand_pressed_image | | string | Image in pressed state when expanded; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| expand_disabled_image | | string | Image in disabled state when expanded; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| collapse_normal_image | | string | Image in normal state when collapsed; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| collapse_hovered_image | | string | Image in hovered state when collapsed; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| collapse_pressed_image | | string | Image in pressed state when collapsed; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| collapse_disabled_image | | string | Image in disabled state when collapsed; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| expand_image_right_space | | int | Space on the right side of the expand image |
| check_box_image_right_space | | int | Space on the right side of the CheckBox image |
| icon_image_right_space | | int | Space on the right side of the icon |

The TreeNode control inherits the `ListBoxItem` attributes. For more available attributes, please refer to the `ListBoxItem` attributes.

## Attributes of DirectoryTree (inherits TreeView attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| small_icon_size | 16 | int | Icon size of tree nodes |
| large_icon_size | 32 | int | Large icon size, used to display the content inside the directory; the tree node itself does not use this attribute |
| show_hiden_files | false | bool | Whether to show hidden files |
| show_system_files | false | bool | Whether to show system files |

The DirectoryTree control inherits the `TreeView` attributes. For more available attributes, please refer to the `TreeView` attributes.

## Attributes of ListCtrl (inherits VBox attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| type | "report" | string | Type, optional values: "report", "icon", "list" |
| header_class | | string | Class attribute of ListCtrlHeader; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| header_item_class | | string | Class attribute of ListCtrlHeaderItem; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| header_split_box_class | | string | Class attribute of ListCtrlHeader/SplitBox; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| header_split_control_class | | string | Class attribute of ListCtrlHeader/SplitBox/Control; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| enable_header_drag_order | true | bool | Whether to support dragging the list header to change column order|
| check_box_class | | string | Class attribute of CheckBox (applied to Header and ListCtrl data); for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| data_item_class | | string | Class attribute of ListCtrlItem; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| data_sub_item_class | | string | Class attribute of ListCtrlItem/ListCtrlSubItem; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| row_grid_line_width | 1.0 | float | Width of horizontal grid lines|
| row_grid_line_color | | int | Color of horizontal grid lines|
| column_grid_line_width | 1.0 | float | Width of vertical grid lines|
| column_grid_line_color | | int | Color of vertical grid lines|
| report_view_class | | string | Class attribute of the ListBox in the data Report view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| header_height | | int | Height of the header control|
| data_item_height | | int | Default height of data items (row height)|
| show_header | true | bool | Whether to display the header control|
| multi_select | true | bool | Whether to support multi-selection|
| enable_column_width_auto | true | bool | Whether to support double-clicking the header's split bar to automatically adjust column width|
| auto_check_select | false | bool | Whether to automatically check the selected data items (applies to the Header and each row)|
| show_header_checkbox | false | bool | Whether to display a CheckBox at the far left of the header|
| show_data_item_checkbox | false | bool | Whether to display a CheckBox at the start of each row|
| icon_view_class | | string | Class attribute of the ListBox in the data Icon view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| icon_view_item_image_class | | string | Class attribute of the image in the ListBox's sub-items in the data Icon view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| icon_view_item_label_class | | string | Class attribute of the Label in the ListBox's sub-items in the data Icon view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| list_view_class | | string | Class attribute of the ListBox in the data List view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| list_view_item_class | | string | Class attribute of the ListBox's sub-item in the data List view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| list_view_item_image_class | | string | Class attribute of the image in the ListBox's sub-item in the data List view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| list_view_item_label_class | | string | Class attribute of the Label in the ListBox's sub-item in the data List view; for the definition method, please refer to the corresponding content in `global.xml` and the example program|
| enable_item_edit | true | bool | Whether to support sub-item editing|
| list_ctrl_richedit_class | | string | Class attribute of the edit box; for the definition method, please refer to the corresponding content in `global.xml` and the example program|

The ListCtrl control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.    
Each view of the ListCtrl control inherits the `ListBox` attributes; for more available attributes, please refer to the `ListBox` attribute settings: [Box.md](Box.en.md); the view attributes need to be set in `global.xml`.

## Attributes of PropertyGrid (inherits VBox attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| property_grid_xml |   | string | Configuration file XML; if empty, defaults to: "public/property_grid/property_grid.xml" |
| row_grid_line_width | 1.0 | float | Width of horizontal grid lines |
| row_grid_line_color |   | string | Color of horizontal grid lines |
| column_grid_line_width | 1.0 | float | Width of vertical grid lines |
| column_grid_line_color |   | string | Color of vertical grid lines |
| header_class |   | string | Class attribute of the header; for the definition method, please refer to the corresponding content in `global.xml` and the example program |
| group_class |   | string | Class attribute of the group; for the definition method, please refer to the corresponding content in `global.xml` and the example program |
| group_label_class |   | string | Class of the group's text control |
| property_class |   | string | Class attribute of the property; for the definition method, please refer to the corresponding content in `global.xml` and the example program |
| property_name_label_class |   | string | Class of the property name text control |
| property_value_label_class |   | string | Class of the property value text control |
| left_column_width |   | int | Width of the left column |
| property_font_normal |   | string | Set the font Id of the property value (normal state) |
| property_font_modified |   | string | Set the font Id of the property value (modified state) |

The PropertyGrid control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.

## Attributes of ColorPicker (inherits Window attributes)
ColorPicker is a window; for specific usage, please refer to the menu in the example program.    
The ColorPicker control inherits the `Window` attributes. For more available attributes, please refer to the `Window` attributes.

## Attributes of ControlDragable (inherits Control attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| drag_order | true | bool | Whether to support drag to adjust order (within the same container) |
| drag_alpha | 216 | int | Opacity of the control when dragging to reorder (0 - 255) |
| drag_out | true | bool | Whether to support drag-out operation (between different containers in the same window) |

The ControlDragable control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of CefControl (inherits Control attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| url |  | string | After the control is created successfully, navigate to this URL |
| url_is_local_file |  | string | Whether the URL specified by url is a local file; if it is a local file and a relative path is specified, the root directory is the directory where the executable program is located |
| F11 | true | bool | Whether to allow the F11 shortcut key (page fullscreen / exit page fullscreen) |
| F12 | true | bool | Whether to allow the F12 shortcut key (show/hide developer tools) |
| download_favicon_image | false | bool | Whether to download the website's FavIcon |

The CefControl control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of WebView2Control (inherits Control attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| url |  | string | After the control is created successfully, navigate to this URL |
| url_is_local_file |  | string | Whether the URL specified by url is a local file; if it is a local file and a relative path is specified, the root directory is the directory where the executable program is located |
| F11 | true | bool | Whether to allow the F11 shortcut key (page fullscreen / exit page fullscreen) |
| F12 | true | bool | Whether to allow the F12 shortcut key (show/hide developer tools) |
| devtools_enabled | true | bool | Whether to allow opening developer tools |

The WebView2Control control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of IconControl (inherits Control attributes)
The IconControl control inherits the `Control` attributes. For more available attributes, please refer to the `Control` attributes.

## Attributes of BitmapControl (inherits Box attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| bitmap_halign  | left | string | Horizontal alignment of the image, optional values: "left", "center", "right" |
| bitmap_valign  | top | string | Vertical alignment of the image, optional values: "top", "center", "bottom" |
| bitmap_alpha  | 255 | int | Opacity when drawing the image, optional values: 0 - 255 |
| bitmap_dest  |  | rect | Position and size of the image drawing destination area (relative to the control area)|
| bitmap_src  |  | rect | Position and size of the image drawing source area|
| bitmap_margin  |  | rect | Margin in the drawing destination area (if the dest value is specified, this value is invalid)|
| bitmap_adaptive_dest_rect  | false | bool | Whether to automatically adapt to the destination area when drawing (scale the image proportionally)|
| bitmap_stretch  | false | bool | Whether to stretch the image when drawing (mutually exclusive with IsAdaptiveDestRect(), and lower priority than IsAdaptiveDestRect())|
| bitmap_multi_thread  | true | bool | Whether to support multi-threaded operation of bitmap data (if not called, defaults to true, meaning multi-threaded bitmap data operation is supported by default)|

The BitmapControl control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.

## Attributes of AddressBar (inherits HBox attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| address_path |  | string | Set the path |
| path_tooltip | true | bool | Set whether to display the path tooltip |
| return_update_ui | true | bool | Set to automatically update the display control when Enter is pressed |
| esc_update_ui | true | bool | Set to automatically update the display control when ESC is pressed |
| kill_focus_update_ui | true | bool | Set to automatically update the display control when focus is lost |
| rich_edit_class | "address_bar_edit" | string | Set the Class of the edit box |
| rich_edit_clear_btn_class | "rich_edit_clear_btn" | string | Set the Class of the edit box's clear button |
| sub_path_hbox_class | "address_bar_sub_path_hbox"| string | Set the Class of the container (HBox) for the address bar path; each sub-path has one HBox container |
| sub_path_button_class | "address_bar_sub_path_button"| string | Set the Class of the address bar sub-path button |
| sub_path_root_class | "address_bar_sub_path_root" | string | Set the Class of the address bar root path ("/" path) |
| path_separator_class | "address_bar_path_separator" | string | Set the Class of the address bar path separator |

The AddressBar control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.

## Attributes of ChildWindow (inherits Box attributes)
| Attribute name | Default value | Parameter type | Purpose |
| :--- | :--- | :--- | :--- |
| child_window_margin |   | UiMargin | Set the margin of the child window; other controls can be placed in the margin space |

The ChildWindow control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.
