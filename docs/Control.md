简体中文 | [English](Control.en.md)

## `Control`（基础控件）的属性

`Control` 控件为所有可用控件的基类，包含了控件常用的属性，如宽度、高度、边距等属性。一般情况下不作为具体可用控件使用，但常用于一些占位符，请参考示例。
`Control` 是所有控件的基类，包含了所有控件的可用通用属性。

```xml
<!-- 一个根据父容器宽度自动拉伸的 Control 控件，一般用于作为占位符使用 -->
<Control width="stretch"/>
```

| 属性名称 | 默认值 | 参数类型 |关联[Control.h](../duilib/Core/Control.h)中的函数| 用途 |
| :--- | :--- | :--- | :---| :--- |
| class |  | string | SetClass|控件样式,用样式里面的属性值来设置本控件的属性,如"btn_default",可以同时指定多个样式,各个样式之间用空格分隔,需要写在第一个属性位置 |
| enable_vars | true | bool | SetEnableVars| 属性值中是否支持变量展开。<br> 用法举例：假设在globle.xml中增加一行变量定义：`<Define name="SIZE_ICON_SMALL" value="16"/>` <br> XML中的控件属性字符串中可以这样使用这个变量：`width="${SIZE_ICON_SMALL}"` <br> 代码中也可以这样调用函数设置属性代码：`Control::SetAttribute(_T("width"), _T("${SIZE_ICON_SMALL}"));`|
| name |  | string | SetName|控件名字,同一窗口建议保持唯一，否则影响查询效率，也可能查询不到正确的结果 |
| width | stretch | int / string | SetFixedWidth|可以设置int或string类型的值.当值为int是则设置控件的宽度值,如"100";当值为string时,stretch代表由父容器计算控件宽度,auto代表根据内容情况自动计算宽度,如果为百分比值"50%"，代表该控件的宽度期望值为父控件宽度的50% |
| height | stretch | int / string | SetFixedHeight|可以设置int或string类型的值.当值为int是则设置控件的高度值,如"100";当值为string时,stretch代表由父容器计算控件高度,auto代表根据内容情况自动计算高度,如果为百分比值"30%"，代表该控件的高度期望值为父控件高度的30%  |
| min_width | -1 | int | SetMinWidth|控件的最小宽度,如"30" |
| min_height | -1 | int | SetMinHeight|控件的最小高度,如"30" |
| max_width | INT32_MAX | int / string | SetMaxWidth|控件的最大宽度,描述同width |
| max_height | INT32_MAX | int / string | SetMaxHeight|控件的最大高度,描述同height |

> **注意**：布局阶段控件的实际尺寸会经过 `std::clamp(尺寸, GetMinWidth(), GetMaxWidth())` 钳制。
> 因此运行时若要把某个已设了 `min_width`/`max_width` 的控件改回 `stretch`，
> 必须同时把上下限放开（`SetMinWidth(0)` + `SetMaxWidth(INT32_MAX)`），否则宽度会被钳死在原值上；
> 若上下限被设为 0，控件宽度会变成 0 而完全不可见。
| margin | 0,0,0,0 | rect | SetMargin|外边距,如"2,2,2,2" |
| padding | 0,0,0,0 | rect | SetPadding|内边距,如"2,2,2,2" |
| control_padding | true | bool | SetEnableControlPadding|是否允许控件自身运用内边距 |
| halign | left | string | SetHorAlignType|控件的水平方向的对齐方式,如"center",支持left、center、right三种位置 |
| valign | top | string | SetVerAlignType|控件的垂直方向的对齐方式,如"center",支持top、center、bottom三种位置 |
| align | left,top | string | SetHorAlignType<br>SetVerAlignType |同时设置控件的水平方向和垂直方向的对齐方式，功能与valign和halign相同。<br>可取值: left、right、hcenter、top、vcenter、bottom，用逗号分割，如"hcenter,vcenter" |
| float | false | bool | SetFloat|是否使用绝对定位,如"true"|
| keep_float_pos | false | bool | SetKeepFloatPos|设置当父控件位置和大小调整时，是否保持浮动控件相对父控件的位置不变,如"true"|
| bkcolor |  | string | SetBkColor|背景颜色字符串常量,如"white" |
| bkcolor2 |  | string | SetBkColor2|第二背景颜色字符串常量,如果设置了第二背景色，则支持背景颜色渐变,如"blue" |
| bkcolor2_direction | "1" | string | SetBkColor2Direction|第二背景色方向，"1": 左->右，"2": 上->下，"3": 左上->右下，"4": 右上->左下|
| fore_color |  | string | SetForeColor|前景颜色字符串，前景色一般设置半透明的颜色，比如"#10006DD9" |
| normal_color |  | string | SetStateColor|普通状态颜色,如"white" |
| hovered_color |  | string | SetStateColor|悬浮状态颜色,如"white" |
| pressed_color |  | string | SetStateColor|按下状态颜色,如"white" |
| disabled_color |  | string | SetStateColor|禁用状态颜色,如"white" |
| normal_color_margin |  | UiMargin | SetStateColorMargin|设置普通状态下的控件颜色外边距 |
| hovered_color_margin |  | UiMargin | SetStateColorMargin|设置悬浮状态下的控件颜色外边距 |
| pressed_color_margin |  | UiMargin | SetStateColorMargin|设置按下状态下的控件颜色外边距 |
| disabled_color_margin |  | UiMargin | SetStateColorMargin|设置禁用状态下的控件颜色外边距 |
| state_color_min_width |  | float | SetStateColorMinWidth|设置状态颜色区域的最小宽度（解决设置Margin后，DPI缩放运算精度损失导致线条宽度失真问题） |
| state_color_min_height |  | float | SetStateColorMinHeight|设置状态颜色区域的最小高度（解决设置Margin后，DPI缩放运算精度损失导致线条宽度失真问题） |
| normal_color_round |  | UiSize | SetStateColorRound|设置普通状态下的控件颜色圆角大小 |
| hovered_color_round |  | UiSize | SetStateColorRound|设置悬浮状态下的控件颜色圆角大小 |
| pressed_color_round |  | UiSize | SetStateColorRound|设置按下状态下的控件颜色圆角大小 |
| disabled_color_round |  | UiSize | SetStateColorRound|设置禁用状态下的控件颜色圆角大小 |
| border_color |  | string | SetBorderColor|设置所有状态下的边框颜色,如"blue" |
| normal_border_color |  | string | SetBorderColor|正常状态的边框颜色,如"blue" |
| hovered_border_color |  | string | SetBorderColor|悬浮状态的边框颜色,如"blue" |
| pressed_border_color |  | string | SetBorderColor|按下状态的边框颜色,如"blue" |
| disabled_border_color |  | string | SetBorderColor|禁止状态的边框颜色,如"blue" |
| focused_border_color |  | string | SetFocusedBorderColor|焦点状态的边框颜色,如"blue" |
| border_size | 0 | int / rect | SetBorderSize|可以设置int或rect类型的值。当值为int时则左、上、右、下都用该值作为宽。值为rect类型时则分别设置左、上、右、下的边框 |
| border_dash_style |"solid"| string | SetBorderDashStyle |线的形状，可选值：<br>"solid"：实线<br>"dash"：短划线构成的虚线<br>"dot"：点构成的虚线<br>"dash_dot"：交替短划线点线<br>"dash_dot_dot"：交替短划线点点线|
| borders_on_top | true | bool | SetBordersOnTop|边框是否在顶层（即先绘制子控件，后绘制边框，避免边框被子控件覆盖）|
| left_border_size | 0 | int | SetLeftBorderSize|左边边框大小,如"1",设置该值大于0,则将忽略border_size属性的设置 |
| top_border_size | 0 | int | SetTopBorderSize|顶部边框大小,如"1",设置该值大于0,则将忽略border_size属性的设置 |
| right_border_size | 0 | int | SetRightBorderSize|右边边框大小,如"1",设置该值大于0,则将忽略border_size属性的设置 |
| bottom_border_size | 0 | int | SetBottomBorderSize|底部边框大小,如"1",设置该值大于0,则将忽略border_size属性的设置 |
| border_round | 0,0 | size | SetBorderRound|边框圆角半径,如"2,2" |
| bkimage |  | string | SetBkImage|背景图片,如"bk.bmp或file='aaa.jpg' res='' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' fade='255' xtiled='false' ytiled='false'" |
| normal_image |  | string | SetStateImage|普通状态图片 |
| hovered_image |  | string | SetStateImage|鼠标悬浮的状态图片 |
| pressed_image |  | string | SetStateImage|鼠标按下的状态图片 |
| disabled_image |  | string | SetStateImage|禁用的状态图片 |
| fore_normal_image |  | string | SetForeStateImage|普通状态前景图片 |
| fore_hovered_image |  | string | SetForeStateImage |鼠标悬浮的状态前景图片 |
| fore_pressed_image |  | string | SetForeStateImage|鼠标按下的状态前景图片 |
| fore_disabled_image |  | string | SetForeStateImage|禁用的状态前景图片 |
| tooltip_text |  | string | SetToolTipText|鼠标悬浮提示,如"请在这里输入你的密码" |
| tooltip_text_id |  | string | SetToolTipTextId|鼠标悬浮提示,指定多语言模块的ID,当tooltiptext为空时则显示此属性,如"TOOL_TIP_ID" |
| tooltip_width |  | int | SetToolTipWidth| 鼠标悬浮提示所占的宽度 |
| data_id |  | string | SetDataID|自定义字符串数据,辅助函数，供用户使用 |
| user_data_id |  | size_t | SetUserDataID|自定义整型数据, 供用户使用 |
| enabled | true | bool | SetEnabled|是否可以响应用户操作,如"true"|
| mouse_enabled | true | bool | SetMouseEnabled|本控件是否可以响应鼠标操作,如"true"|
| keyboard_enabled | true | bool | SetKeyboardEnabled|非CButtonUI类忽略该值,为false时不支持TAB_STOP,且该对象不处理键盘信息 |
| visible | true | bool | SetVisible|是否可见,如"true"|
| fade_visible | true | bool | SetFadeVisible|是否可见,如"true",此属性会触发控件动画效果 |
| menu | false | bool | |是否需要右键菜单,如"true"|
| no_focus | false | bool | SetNoFocus|是否可以获取焦点,如"true"|
| tab_stop | true | bool | SetTabStop| 是否允许通过按TAB键切换到此控件 |
| show_focused_rect | false| bool | SetShowFocusedRect| 是否显示焦点状态(一个虚线构成的矩形) |
| focused_rect_color | | string | SetFocusedRectColor| 焦点状态矩形的颜色 |
| alpha | 255 | int | SetAlpha|控件的整体透明度,如alpha="128"，有效值为 0-255 |
| state | normal | string | SetState|控件的当前状态: 支持normal、hovered、pressed、disabled状态 |
| cursor_type | arrow | string | SetCursorType|鼠标移动到控件上时的鼠标光标: <br>"arrow"：箭头<br>"hand"：手型<br>"wait"：忙碌<br>"cross"：十字线<br>"ibeam"：I型光标,文本光标<br>"size_we"：水平调整<br>"size_ns"：垂直调整<br>"size_nwse"：对角线调整，西北-东南调整<br>"size_nesw"：对角线调整，东北-西南调整<br>"size_all"：移动，四向调整<br>"no"：禁止光标<br>"progress"：进度，应用启动光标|
| render_offset | 0,0 | size | SetRenderOffset|控件绘制时的偏移量,如"10,10",一般用于绘制动画 |
| fade_hovered | false | bool |SetFadeHovered |是否启用控件鼠标悬停状态下的透明渐变动画,如"true"|
| fade_hovered_frame_interval_ms | 16 | int |SetFadeHoveredFrameIntervalMillSeconds |设置播放Hovered状态动画的定时器时间间隔（毫秒）|
| fade_hovered_total_ms | 180 | int |SetFadeHoveredTotalMillSeconds |设置Hovered状态动画总的播放时间（毫秒）|
| fade_hovered_easing_function | EaseInOutCubic | string |SetFadeHoveredEasingFunctionType |设置Hovered状态动画缓动函数类型,支持的缓动函数类型参见EasingFunctions::GetEasingFunctionType实现函数|
| fade_alpha | false | bool/int | GetAnimationManager().SetFadeAlpha|是否启用控件透明渐变动画。有效值如下：<br>fade_alpha="false"：不启用控件透明渐变动画 <br>fade_alpha="true"：启用控件透明渐变动画，最终控件的Alpha值设置为255 <br>fade_alpha="128"：启用控件透明渐变动画，最终控件的Alpha值设置为128，这种情况下的有效值为1-255。|
| fade_width | false | bool | GetAnimationManager().SetFadeWidth|是否启用控件宽度渐变动画,如"true"。控件宽度不能是拉伸类型，该属性必须写在width属性后面|
| fade_height | false | bool | GetAnimationManager().SetFadeHeight|是否启用控件高度渐变动画,如"true"。控件高度不能是拉伸类型，该属性必须写height属性后面|
| fade_size | false | bool | GetAnimationManager().SetFadeSize|是否启用控件大小（高度和宽度）的渐变动画,如"true"。控件高度和宽度不能是拉伸类型，该属性必须写height属性和width属性后面|
| fade_in_out_x_from_left | false | bool | GetAnimationManager().SetFadeInOutX|是否启用控件从左到右的动画,如"true"|
| fade_in_out_x_from_right | false | bool | GetAnimationManager().SetFadeInOutX|是否启用控件从右到左的动画,如"true"| 
| fade_in_out_y_from_top | false | bool | GetAnimationManager().SetFadeInOutY|是否启用控件从上到下的动画,如"true"| 
| fade_in_out_y_from_bottom | false | bool | GetAnimationManager().SetFadeInOutY|是否启用控件从下到上的动画,如"true"|
| fade_frame_interval_ms | 16 | int |GetAnimationManager().SetFrameIntervalMillSeconds |设置播放动画的定时器时间间隔（毫秒）|
| fade_total_ms | 180 | int |GetAnimationManager().SetTotalMillSeconds |设置动画总的播放时间（毫秒）|
| fade_easing_function | EaseInOutCubic | string |GetAnimationManager().SetEasingFunctionType |设置动画缓动函数类型,支持的缓动函数类型参见EasingFunctions::GetEasingFunctionType实现函数|
| loading     | | string | SetLoadingAttribute| 设置控件加载中状态的UI显示相关属性，使用方法与Image属性相似。<br>使用示例：loading="file='loading.xml' width='0' height='0' offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255' animation_control='loading_animation' auto_stop='true'" <br> loading的可用属性如下：<br> "file": XML资源文件名，根据此设置去加载XML资源，最终放在一个Box容器中显示<br>"width": longding控件的显示宽度，像素<br>"height": longding控件的显示高度，像素 <br> "offset_x": longding控件的位置X方向偏移，相对于关联控件的左上角，有效值: >= 0 <br> "offset_y": longding控件的位置Y方向偏移，相对于关联控件的左上角，有效值: >= 0  <br> "halign":水平方向对齐方式，可取值："left" "center" "right"，仅当offset_x不含有效值时生效 <br>  "valign":垂直方向对齐方式，可取值："top" "center" "bottom"，仅当offset_y不含有效值时生效<br>"fade":loading控件的透明度，有效值： 0 - 255<br>"auto_stop"：loading动画播放结束以后，自动停止loading状态（自动调用StopLoading()函数）<br>"animation_control":动画控件的名称，用于Loading功能与loading控件上的动画控件交互使用<br><br>loading功能的完整演示，可参考`examples/ListCtrl`示例程序|
| paint_order | | string | SetPaintOrder| 设置绘制顺序：0 表示常规绘制，非0表示指定绘制顺序，值越大表示越晚绘制 |
| start_image_animation     | | string | StartImageAnimation   | 播放动画，最多3个参数，每个参数用','分割，详见函数的参数列表 |
| stop_image_animation      | | string | StopImageAnimation    | 停止动画，最多3个参数，每个参数用','分割 ，详见函数的参数列表 |
| set_image_animation_frame | | string | SetImageAnimationFrame| 设置动画当前帧，最多2个参数，每个参数用','分割，详见函数的参数列表|
| box_shadow | | string | SetBoxShadow|设置控件的阴影属性，举例：boxshadow="color='red' offset='0,0' blurradius='8' spreadradius='8'" |
| enable_drag_drop |false| bool | SetEnableDragDrop | 是否允许拖放操作，包括拖入文件和拖入文本|
| enable_drop_file |false| bool | SetEnableDropFile | 是否允许拖入文件操作|
| drop_file_types  || string | SetDropFileTypes  | 拖放文件操作支持的后缀名列表，比如:".txt;.csv"，表示仅支持txt和csv文件；如果为空，表示支持所有文件|
| row_span  | 1 | int | SetRowSpan  | 单元格合并属性，占几行（默认占1行），仅在GridLayout布局中生效|
| col_span  | 1 | int | SetColumnSpan  | 单元格合并属性，占几列（默认占1列），仅在GridLayout布局中生效|

## `ASSERT` 的生效范围

`ASSERT(expr)` 由 `duilib/duilib_config.h` 定义，**四个平台的展开方式完全一致：Debug 生效，Release 不校验**。

| 平台 | Debug | Release |
| :--- | :--- | :--- |
| Windows | `_ASSERTE(expr)`，失败弹断言框 | `((void)(0))`，空操作 |
| Linux / macOS / FreeBSD | `assert(expr)` | `((void)(0))`，空操作 |

**对使用者的影响**：

- **Release 版遇到断言失败不会中断程序**，会带着非法状态继续执行，可能表现为样式不对、数据异常或后续崩溃，而不是明确的断言弹窗。因此断言覆盖的入参校验（例如 `halign` / `valign` 填了非法值）在 Release 下应视为「无保护」。
- **不要把 `ASSERT` 当运行期校验用**。它只用于「不应该发生」的前提检查，不校验来自外部的输入（XML 属性、用户输入、网络数据）。这些应由业务代码显式判断并处理。
- `ASSERT` 的表达式**在 Release 下不会被求值**，因此不要在里面写带副作用的语句。

**给「Release 也要生效的校验」的写法**：用普通的 `if` 判断 + 处理分支，不要用 `ASSERT`。需要同时在两种构建下都检查时，可写成一个普通函数在两条路径上都调用。

配套宏 `ASSERT_UNUSED_VARIABLE(expr)` 在 Release 下展开为 `UNUSED_VARIABLE(expr)`（即 `((void)(expr))`），会引用表达式而不执行检查，用于避免「仅在断言中使用的变量」触发未使用告警。

## ControlLoading 与骨架屏

所有控件都继承自 `Control`，因此都支持 `loading` 属性与 `StartLoading` / `StopLoading` 接口，可用于列表、表格、卡片区域的加载占位。

### 骨架屏与其它加载状态的分工

| 状态 | 适用场景 | 实现方式 |
| :--- | :--- | :--- |
| 转圈指示 | 布局结构未知，或加载只需几百毫秒 | `loading*.xml` + json 动画自动播放 |
| 进度条 loading | 能算出进度（下载、上传、批量任务） | 静态图集 json + 在 `kEventLoading` 里 `SetImageAnimationFrame` 手动刷帧 |
| **骨架屏** | 结构已知、希望提前呈现版式 | 灰白占位块（见下文），**必须业务侧超时兜底** |
| 空状态 | 加载完成但结果为空 | 真实控件，不是 loading |
| 错误态 | 加载失败 | 真实控件 + 重试入口 |

### 骨架屏的四条硬约束

这四条都踩过坑，写骨架屏 XML / 代码时务必遵守。

**① loading 层是"叠加绘制"而不是"遮罩"。**
`ControlLoading` 用 `GlobalManager::CreateBox` 建一个独立 Box，它挂在 Window 上、不进控件树，靠 `Control::PaintLoading` → `AlphaPaint` 画在目标控件之上。因此**根容器必须有不透明 `bkcolor`**，否则下方已有的内容会透出来。

**② 尺寸：用 `width='0' height='0'` + 根容器 `stretch`，即可铺满目标控件。**
`UpdateLoadingPos` 中 `stretch` 既非 `IsAuto()` 也非 `IsInt32()`，尺寸算出 0 后被 `if (nBoxWidth <= 0 || nBoxWidth > ctrlWidth) nBoxWidth = ctrlWidth` 补成控件全宽 —— 效果正是"铺满（含表头）"，无需 C++ 计算尺寸。想让尺寸自适应内容才用 `auto`（走 `EstimateSize()`）。

**③ 骨架屏没有"动画播完"的时刻，必须业务侧超时兜底。**
转圈/进度条可以靠 `auto_stop='true'` 自动结束，骨架屏是静态占位，所以**不要设 `animation_control` 和 `auto_stop`**，改为在 `kEventLoading` 里判断超时：

```cpp
const uint64_t nTimeoutMs = 5000;
if (pLoadingStatus->m_nElapsedTimeMs >= nTimeoutMs) {
    pLoadingStatus->m_bStopLoading = true;
}
```

注意 `StartLoading(nIntervalMs, nMaxCount)` 的 `nMaxCount` 是**回调次数上限，不是超时时间**；时间超时只能自己看 `ControlLoadingStatus::m_nElapsedTimeMs`。没有超时兜底的骨架屏比转圈更糟——用户会失去"还在加载"的确信感。

**④ 呼吸动画不能给根容器设 `SetAlpha`。**
`Control::SetAlpha` 作用于**整个子树**，根容器的 `bkcolor` 是唯一遮住下方内容的东西（见 ①），alpha 一降遮罩就失效、下方旧内容会透出来。正确做法是根容器固定 255、只给子控件设 alpha：

```cpp
// 根容器保持不透明，只让占位块呼吸
const uint8_t nAlpha = (uint8_t)(197.5 + 57.5 * std::sin(fSeconds * 3.1415926));
for (size_t i = 0; i < pLoadingUiRootBox->GetItemCount(); ++i) {
    ui::Control* pChild = pLoadingUiRootBox->GetItemAt(i);
    if (pChild != nullptr) {
        pChild->SetAlpha(nAlpha);
    }
}
```

`Box` 的子控件接口是 `GetItemCount()` / `GetItemAt(i)`，**没有** `GetChildCount()` / `GetChildAt()`。alpha 区间建议 140~255，保持占位块清晰可辨。子控件设 alpha 不触发布局重排。

### 骨架屏 XML 的两条额外注意

- **`valign` 只能填 `top` / `center` / `bottom`**，`vcenter` 只在 `text_align` 属性里有效。填错值时 `Control::SetAttribute` 末尾是 `else { ASSERT(0); }`，Debug 版弹断言框。`halign` 同理只能填 `left` / `center` / `right`。**Release 版不做校验**（见下节「`ASSERT` 的生效范围」），写错不会报错但也不会正确对齐。
- **同层级兄弟节点要么统一 `height="stretch"`，要么都别混用 `auto`**。混用会导致一个被拉伸、一个按内容高度，视觉上留白不对称。行间距用 margin 实现（上行 `margin="0,0,0,12"`、下行 `margin="0,12,0,0"`）。

### 示例代码

C++ 侧启动骨架屏：

```cpp
const DString strSkin = _T("loading_skeleton_list.xml");
const DString strLoading = _T("file='") + strSkin + _T("' width='0' height='0' "
    _T("offset_x='-1' offset_y='-1' valign='center' halign='center' fade='255'"));
pListCtrl->SetLoadingAttribute(strLoading);   // 注意：返回 bool，失败要检查

pListCtrl->AttachLoading([pListCtrl](const ui::EventArgs& args) {
    ui::ControlLoadingStatus* pStatus = (ui::ControlLoadingStatus*)args.wParam;
    if (pStatus == nullptr) {
        return false;
    }
    if (pStatus->m_nElapsedTimeMs >= 5000) {   // 超时兜底
        pStatus->m_bStopLoading = true;
        return true;
    }
    ui::ControlPtrT<ui::Box> pRootBox = pStatus->m_pLoadingUiRootBox;
    const uint8_t nAlpha = (uint8_t)(197.5 + 57.5 * std::sin(
        (double)pStatus->m_nElapsedTimeMs / 1000.0 * 3.1415926));
    for (size_t i = 0; i < pRootBox->GetItemCount(); ++i) {
        ui::Control* pChild = pRootBox->GetItemAt(i);
        if (pChild != nullptr) {
            pChild->SetAlpha(nAlpha);
        }
    }
    return true;
});

pListCtrl->StartLoading(33, -1);   // 33ms 一次心跳，-1 表示不靠次数停止
```

骨架屏 XML 骨架（完整示例见 `bin/resources/themes/default/list_ctrl/loading_skeleton_list.xml`）：

```xml
<Loading>
    <VBox height="stretch" width="stretch" bkcolor="bg_list" mousechild="false" padding="8,6,8,6">
        <HBox height="24" width="stretch" margin="0,0,0,6">
            <Control width="16" height="16" bkcolor="bg_progress_track" border_round="3,3" valign="center"/>
            <VBox width="stretch" height="auto" margin="12,0,0,0" valign="center">
                <Control width="180" height="11" bkcolor="bg_progress_track" border_round="3,3"/>
                <Control width="110" height="9" bkcolor="bg_progress_track" border_round="3,3" margin="0,5,0,0"/>
            </VBox>
        </HBox>
    </VBox>
</Loading>
```

### 已有骨架屏皮肤

`bin/resources/themes/default/list_ctrl/` 目录下：

- `loading_skeleton_list.xml` —— 列表骨架（顶部工具栏占位 + 12 行条目）
- `loading_skeleton_card.xml` —— 卡片骨架（3 列 × 2 行，每张 = 大图位 + 标题 + 副标题）

配套演示见 `examples/ListCtrl` 示例的「骨架屏测试（列表）」「骨架屏测试（卡片）」「停止Loading」三个按钮。

**注意**：`StopLoading()` 会销毁 loading Box，下次 `StartLoading` 重新解析 XML，因此皮肤 XML 必须每次能独立解析。另外 `Control::StartLoading` 内部会 `SetEnabled(false)`，若 loading 中途卡死会导致控件一直禁用、整片区域点不动，调试时建议额外提供一个"无条件 StopLoading"的兜底出口。

## ScrollBar的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| button1_normal_image |  | string | 左或上按钮普通状态图片 |
| button1_hovered_image |  | string | 左或上按钮鼠标悬浮状态图片 |
| button1_pressed_image |  | string | 左或上按钮鼠标按下状态图片 |
| button1_disabled_image |  | string | 左或上按钮禁用状态图片 |
| button2_normal_image |  | string | 右或下按钮普通状态图片 |
| button2_hovered_image |  | string | 右或下按钮鼠标悬浮状态图片 |
| button2_pressed_image |  | string | 右或下按钮鼠标按下状态图片 |
| button2_disabled_image |  | string | 右或下按钮禁用状态图片 |
| thumb_normal_image |  | string | 滑块普通状态图片 |
| thumb_hovered_image |  | string | 滑块鼠标悬浮状态图片 |
| thumb_pressed_image |  | string | 滑块鼠标按下状态图片 |
| thumb_disabled_image |  | string | 滑块禁用状态图片 |
| rail_normal_image |  | string | 滑块中间标识普通状态图片 |
| rail_hovered_image |  | string | 滑块中间标识鼠标悬浮状态图片 |
| rail_pressed_image |  | string | 滑块中间标识鼠标按下状态图片 |
| rail_disabled_image |  | string | 滑块中间标识禁用状态图片 |
| bk_normal_image |  | string | 背景普通状态图片 |
| bk_hovered_image |  | string | 背景鼠标悬浮状态图片 |
| bk_pressed_image |  | string | 背景鼠标按下状态图片 |
| bk_disabled_image |  | string | 背景禁用状态图片 |
| horizontal | false | bool | 水平或垂直,如"true"|
| line_size | 8 | int | 滚动一行的大小,如"8" |
| thumb_min_length | 30 | int | 滑块的最小长度 |
| range | 100 | int | 滚动范围,如"100" |
| value | 0 | int | 滚动位置,如"0" |
| show_button1 | true | bool | 是否显示左或上按钮,如"true"|
| show_button2 | true | bool | 是否显示右或下按钮,如"true"|
| auto_hide_scroll | true | bool | 是否自动隐藏滚动条,如"true"|

ScrollBar 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

说明：滚动条的键盘操作（方向键、PageUp / PageDown、Home / End）不由 ScrollBar 自身处理，
而由使用该滚动条的 ScrollBox 容器统一实现，详见 `Box.md` 文档中 ScrollBox 的"键盘导航"一节。

## Label的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text |  | string | 显示文本 |
| text_id |  | string | 多语言功能的文本ID |
| rich_text | false | bool | 设置文本内容是否为RichText <br> 使用示例如下：`<Label rich_text="true" text="一个简单<b>窗口</b><br/>带有<u>标题栏</u>和<u>常规按钮</u>，<b>粗体，<font color='#FF0000'>红色字体</font></b>" />` <br> 备注：在支持RichText的模式下，不支持如下功能：<br> （1）对齐方式不支持两端对齐<br> （2）不支持vertical_text属性（也不支持纵向文本相关属性）<br>（3）不支持end_ellipsis属性<br>（4）不支持path_ellipsis属性<br>（5）不支持auto_tooltip属性<br>（6）不支持word_spacing属性|
| text_align | "left,top" | string | 设置文字的水平与垂直对齐方式，用半角逗号分隔，如"hcenter,vcenter" 。<br>水平对齐方式可取值：left（靠左对齐）、hcenter（居中对齐）、right（靠右对齐）、hjustify（两端对齐） <br>垂直对齐方式可取值：top（靠上对齐）、vcenter（居中对齐）、bottom（靠下对齐）、vjustify（两端对齐）|
| text_padding | 0,0,0,0 | rect | 文字显示的内边距, 格式为"left,top,right,bottom"，分别代表在目标区域的左侧、上方、右侧、下方设置内边距值，如："2,2,2,2" |
| font | | string | 字体ID，该字体ID必须在 global.xml 中存在 |
| end_ellipsis | false | bool | 句末显示不完整是否使用 ... 代替 |
| path_ellipsis | false | bool | 对于路径，显示不完整时是否使用 ... 代替中间路径 |
| text_color |  | string | 普通字体颜色,不指定则使用默认颜色，如 "blue" |
| normal_text_color |  | string | 普通字体颜色,不指定则使用默认颜色，如 "blue"，该属性与`text_color`属性相同 |
| hovered_text_color |  | string | 鼠标悬浮字体颜色,不指定则使用默认颜色，如 "blue" |
| pressed_text_color |  | string | 鼠标按下字体颜色,不指定则使用默认颜色，如 "blue" |
| disabled_text_color |  | string | disabled字体颜色,不指定则使用默认颜色，如 "blue" |
| single_line | true | bool | 是否单行输出文字 |
| multi_line | false | bool | 是否多行输出文字，与single_line属性互斥 |
| auto_tooltip | false | bool | 鼠标悬浮到控件显示的提示文本是否省略号出现时才显示|
| replace_newline | false | bool | 是否替换文本中的换行符：将字符串"\\\\n"替换为换行符"\n"，这样可以在XML中使用括号中这两个字符(\n)来当作换行符，从而支持多行文本，举例：原始字符串为"第一行\\\\n第二行"，当为true时，"\\\\n"这两个字符会被替换为换行符"\n"，最终字符串变成"第一行\n第二行" |
| spacing_mul | 1.0f | float | 行（列）间距的倍数, 是字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距 <br> 设置后，实际的行间距为：字体大小 * spacing_mul + spacing_add |
| spacing_add | 0 | float | 行（列）间距附加量, 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移(像素) <br> 设置后，实际的行间距为：字体大小 * spacing_mul + spacing_add |
| word_spacing | 0 | float | 设置两个相邻的字符之间的间隔（像素）|
| vertical_text | false | bool | 设置文本方向：true为纵向文本，false为横向文本 <br> 横向文本绘制方向：从左到右，从上到下 <br> 纵向文本绘制方向：从上到下，从右到左|
| use_font_height | true | bool | 当纵向绘制文本时，设置字间距使用该字体的默认高度，而不是每个字的实际高度（显示时所有字体等高） |
| ascii_rotate_90 | true | bool | 当纵向绘制文本时，对于字母、数字等字符，顺时针旋转90度显示|

Label 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## Marquee的属性
Marquee是文本跑马灯控件，继承自Label（复用`text`/`text_id`/`font`/`text_color`/`vertical_text`/`text_padding`等全部文本属性），对应头文件`duilib/Control/Marquee.h`。用于在界面中横向/纵向循环滚动显示文本：文本内容超出控件显示区域时自动循环滚动，未超出时按`text_align`静态显示。

除Label的属性外，新增以下属性：

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| scroll_direction | left | string | 滚动方向："left"横向向左 / "right"横向向右 / "up"纵向向上 / "down"纵向向下。同时决定文本排列方向（left/right 为横向文本，up/down 为纵向文本） |
| scroll_speed | 50 | int | 滚动速度（像素/秒，自动DPI缩放），0 表示不滚动 |
| scroll_interval | 30 | int | 滚动定时器帧间隔（毫秒，5~1000），影响流畅度与CPU占用 |
| hover_pause | true | bool | 鼠标悬停到控件时是否暂停滚动 |

说明：
- **循环滚动**：文本超出显示区域时，采用"双份文本首尾相接"方式无缝循环滚动；文本未超出时静态显示（按`text_align`对齐），不占用定时器。
- **文本方向**：`scroll_direction`为`left`/`right`时横向文本滚动，为`up`/`down`时纵向文本滚动（内部自动设置`vertical_text`）。
- **动态更新**：`SetText`/`SetTextId`更新文本后自动重置滚动位置；语言切换（`text_id`）同样生效。
- **暂停/恢复**：`Pause()`/`Resume()`/`SetPaused()`控制手动暂停；鼠标悬停自动暂停由`hover_pause`控制（默认开启）。

对应C++接口：`SetScrollDirection/GetScrollDirection`、`SetScrollSpeed/GetScrollSpeed`、`SetScrollInterval/GetScrollInterval`、`SetHoverPause/IsHoverPause`、`Pause/Resume/SetPaused/IsPaused`、`IsScrolling`。

### XML使用示例

```xml
<!-- 横向向左滚动（文本超出控件宽度时循环滚动） -->
<Marquee width="200" height="28" text_id="STRID_NEWS_TICKER"
          scroll_direction="left" scroll_speed="60"
          text_color="text_default"/>

<!-- 纵向向上滚动（竖排文字） -->
<Marquee width="28" height="300" text="这是一条竖排滚动的公告文本"
          scroll_direction="up" scroll_speed="40" font="system_regular_14"/>

<!-- 横向向右滚动，悬停不暂停 -->
<Marquee width="300" height="24" text="向右滚动的文本"
          scroll_direction="right" hover_pause="false"/>
```

### C++使用示例

```cpp
ui::Marquee* pMarquee = dynamic_cast<ui::Marquee*>(pWindow->FindControl(_T("marquee")));
if (pMarquee != nullptr) {
    // 动态更新文本（自动重置滚动位置）
    pMarquee->SetText(_T("新的滚动公告内容"));

    // 调整速度与方向
    pMarquee->SetScrollSpeed(80);
    pMarquee->SetScrollDirection(ui::MarqueeDirection::kLeft);

    // 暂停 / 恢复
    pMarquee->Pause();
    pMarquee->Resume();
}
```

## LabelBox的属性
LabelBox与Label是基于相同模板的类，请参考 `Label`的属性    
LabelBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## LabelHBox的属性
LabelHBox与Label是基于相同模板的类，请参考 `Label`的属性    
LabelHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## LabelVBox的属性
LabelVBox与Label是基于相同模板的类，请参考 `Label`的属性    
LabelVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## Button的属性
Button 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## ButtonBox的属性
ButtonBox与Button是基于相同模板的类，请参考 `Button`的属性    
ButtonBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## ButtonHBox的属性
ButtonHBox与Button是基于相同模板的类，请参考 `Button`的属性    
ButtonHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## ButtonVBox的属性
ButtonVBox与Button是基于相同模板的类，请参考 `Button`的属性    
ButtonVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## CheckBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| selected | false | bool | 是否选中 |
| selected_normal_image |  | string | 选择状态时，普通状态图片 |
| selected_hovered_image |  | string | 选择状态时，鼠标悬浮的状态图片 |
| selected_pressed_image |  | string | 选择状态时，鼠标按下的状态图片 |
| selected_disabled_image |  | string | 选择状态时，禁用的状态图片 |
| selected_fore_normal_image |  | string | 选择状态时，前景图片 |
| selected_fore_hovered_image |  | string | 选择状态时，鼠标悬浮状态的图片 |
| selected_fore_pressed_image |  | string | 选择状态时，鼠标按下状态的前景图片 |
| selected_fore_disabled_image |  | string | 选择状态时，禁用状态的前景图片 |
| part_selected_normal_image |  | string | 部分选择时，普通状态图片 |
| part_selected_hovered_image |  | string | 部分选择时，鼠标悬浮的状态图片 |
| part_selected_pressed_image |  | string | 部分选择时，鼠标按下的状态图片 |
| part_selected_disabled_image |  | string | 部分选择时，禁用的状态图片 |
| part_selected_fore_normal_image |  | string | 部分选择时，前景图片 |
| part_selected_fore_hovered_image |  | string | 部分选择时，鼠标悬浮状态的图片 |
| part_selected_fore_pressed_image |  | string | 部分选择时，鼠标按下状态的前景图片 |
| part_selected_fore_disabled_image |  | string | 部分选择时，禁用状态的前景图片 |
| selected_text_color |  | string | 选择状态的字体颜色,不指定则使用默认颜色,如"blue" |
| selected_normal_text_color |  | string | 选择状态的普通状态字体颜色,不指定则使用默认颜色,如"blue" |
| selected_hovered_text_color |  | string | 选择状态的鼠标悬浮状态字体颜色,不指定则使用默认颜色,如"blue" |
| selected_pressed_text_color |  | string | 选择状态的鼠标按下状态字体颜色,不指定则使用默认颜色,如"blue" |
| selected_disabled_text_color |  | string | 选择状态的禁用状态字体颜色,不指定则使用默认颜色,如"blue" |
| normal_first | false | bool | 控件在选择状态下，没有设置背景色或背景图时，用非选择状态的对应属性来绘制 |

CheckBox 控件继承了 `Button` 属性，更多可用属性请参考`Button`的属性

## CheckBoxBox的属性
CheckBoxBox与CheckBox是基于相同模板的类，请参考 `CheckBox`的属性    
CheckBoxBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## CheckBoxHBox的属性
CheckBoxHBox与CheckBox是基于相同模板的类，请参考 `CheckBox`的属性    
CheckBoxHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## CheckBoxVBox的属性
CheckBoxVBox与CheckBox是基于相同模板的类，请参考 `CheckBox`的属性    
CheckBoxVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## Option的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| group |  | string | 所属组的名称，在相同的组名称下，保持单选 |

Option 控件继承了 `CheckBox` 属性，更多可用属性请参考`CheckBox`的属性

## OptionBox的属性
OptionBox与Option是基于相同模板的类，请参考 `Option`的属性    
OptionBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## OptionHBox的属性
OptionHBox与Option是基于相同模板的类，请参考 `Option`的属性    
OptionHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## OptionVBox的属性
OptionVBox与Option是基于相同模板的类，请参考 `Option`的属性    
OptionVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## GroupBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| corner_size | "0,0" | size | 圆角大小 |
| line_width | 1.0 | float | 线条宽度 |
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
| shadow_type        | "menu" | string | 设置下拉窗口的阴影类型：<br> "default", 默认阴影 <br> "big", 大阴影，直角（适合普通窗口）<br> "big_round", 大阴影，圆角（适合普通窗口）<br> "small", 小阴影，直角（适合普通窗口）<br> "small_round", 小阴影，圆角（适合普通窗口）<br> "menu", 小阴影，直角（适合弹出式窗口，比如菜单等）<br> "menu_round", 小阴影，圆角（适合弹出式窗口，比如菜单等）<br> "none", 无阴影|

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
| shadow_type        | "menu" | string | 设置下拉窗口的阴影类型：<br> "default", 默认阴影 <br> "big", 大阴影，直角（适合普通窗口）<br> "big_round", 大阴影，圆角（适合普通窗口）<br> "small", 小阴影，直角（适合普通窗口）<br> "small_round", 小阴影，圆角（适合普通窗口）<br> "menu", 小阴影，直角（适合弹出式窗口，比如菜单等）<br> "menu_round", 小阴影，圆角（适合弹出式窗口，比如菜单等）<br> "none", 无阴影|

ComboButton 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## CheckCombo的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| dropbox | | string | 下拉框的属性信息，具体设置方法可参照示例程序|
| dropbox_size | | string | 下拉列表的大小（宽度和高度）|
| popup_top | false | bool | 下拉列表是否向上弹出 |
| dropbox_item_class | | string | 下拉列表中每一个列表项的属性，具体设置方法可参照示例程序|
| selected_item_class | | string | 选择项中每一个子项的属性，具体设置方法可参照示例程序|
| shadow_type        | "menu" | string | 设置下拉窗口的阴影类型：<br> "default", 默认阴影 <br> "big", 大阴影，直角（适合普通窗口）<br> "big_round", 大阴影，圆角（适合普通窗口）<br> "small", 小阴影，直角（适合普通窗口）<br> "small_round", 小阴影，圆角（适合普通窗口）<br> "menu", 小阴影，直角（适合弹出式窗口，比如菜单等）<br> "menu_round", 小阴影，圆角（适合弹出式窗口，比如菜单等）<br> "none", 无阴影|

CheckCombo 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## DateTime的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| format | | string | 日期的格式，具体可参考：`DateTime.h`中函数的说明 |
| edit_format | | string | 编辑状态时，日期的编辑格式，可选值："date_calendar"：年-月-日，通过下拉框展示月日历的方式来修改日期；"date_time_calendar"：年-月-日 时:分:秒，日历 + 时间选择弹层，点"确定"提交；"time_calendar"：时:分:秒，仅时间选择弹层（无日历）；"date_up_down"： 编辑时显示：年-月-日，通过控件的右侧放置一个向上-向下的控件以修改日期；"date_time_up_down"：编辑时显示：年-月-日 时:分:秒；"date_minute_up_down"：编辑时显示：年-月-日 时:分；"time_up_down"：编辑时显示：时:分:秒；"minute_up_down"：编辑时显示：时:分|
| spin_class | | string | 日期中的Spin控件的Class属性，仅当使用SDL时有效，默认值为："rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down" |
| min_date | | string | 最小可选日期（yyyy-mm-dd，空表示不限制），仅日历类编辑格式生效 |
| max_date | | string | 最大可选日期（yyyy-mm-dd，空表示不限制），仅日历类编辑格式生效 |
| disabled_dates | | string | 业务禁用日期列表，仅日历类编辑格式生效；多个项用 `,` 或 `;` 分隔，每项为 `yyyy-mm-dd` 或 `yyyy-mm-dd~yyyy-mm-dd`（闭区间） |
| first_day_of_week | "monday" | string | 每周第一天："monday"周一，"sunday"周日，也接受 0~6（0=周日），仅日历类编辑格式生效 |
| show_seconds | "true" | bool | 时间弹层是否显示秒列；为 false 时格式为 `%H:%M`，仅 `date_time_calendar` / `time_calendar` 生效 |
| current_time | | bool | 初始化为当前本地时间（`"true"` 或 `"1"` 时生效），方便使用，无需在代码中手动调用 `InitLocalTime()`；与 `text` 属性同时使用时，`current_time` 优先 |

DateTime 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

> 复用提示：`DateTime` 控件在日历浮层关闭后会抑制"焦点回落导致的立即重开"。当该控件被复用为编辑器（例如 `ListCtrl` 的 `kEditDate` 单元格编辑器）时，每次进入编辑前应调用 `ResetCalendarFlyoutState()` 清除该状态，否则控件重新获得焦点时日历浮层不会自动弹出（需要再点一次鼠标）。另外，若要在日历浮层仍处于打开状态下隐藏控件（例如退出编辑态），应先调用 `CloseCalendarFlyout()` 关闭浮层，避免浮层因锚点控件不可见而遗留为悬空的置顶窗口。

### DateTime 日期约束与时间弹层接口
| 接口 | 说明 |
| :--- | :--- |
| `SetDateLimit(minDate, maxDate)` | 设置可选日期范围（yyyy-mm-dd，空表示不限制），浮层已打开时立即生效 |
| `GetMinDate()` / `GetMaxDate()` | 读取当前日期约束 |
| `SetDisabledDates(disabledDates)` | 设置业务禁用日期列表（`,` / `;` 分隔，单日或 `start~end`） |
| `GetDisabledDates()` | 读取禁用日期列表文本 |
| `SetFirstDayOfWeek(dayOfWeek)` | 设置每周第一天：0=周日，1=周一 |
| `GetFirstDayOfWeek()` | 读取每周第一天 |
| `SetShowSeconds(bShow)` | 时间弹层是否显示秒；未显式设置 `format` 时同步刷新显示文本 |
| `IsShowSeconds()` | 是否显示秒 |

`date_time_calendar` 与 `time_calendar` 的提交语义：点击日期只更新日历选中态，不关闭浮层；点击"确定"后一次性提交完整日期时间；"取消"、Esc、点击浮层外部均不提交。`date_calendar` 保持原有"点日期即提交并关闭"的行为。

键盘操作：日历获得焦点时，方向键 / PageUp / PageDown / Home / End 导航日期；时间输入框（时/分/秒）获得焦点时，Up / Down 按步长调整数值，Left / Right 移动光标。

## Calendar的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| calendar_mode | "single" | string | 选择模式："single"单选，"range"范围选择 |
| first_day_of_week | "monday" | string | 每周第一天："monday"周一，"sunday"周日 |
| min_date | | string | 最小可选日期（格式：yyyy-mm-dd），空表示不限制 |
| max_date | | string | 最大可选日期（格式：yyyy-mm-dd），空表示不限制 |
| disabled_dates | | string | 业务禁用日期列表；多个项用 `,` 或 `;` 分隔，每项为 `yyyy-mm-dd` 或 `yyyy-mm-dd~yyyy-mm-dd`（闭区间） |

Calendar 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

Calendar 也提供 `AddDisabledDate` / `AddDisabledDateRange` / `AddDisabledDates` / `ClearDisabledDates` / `IsDateDisabled` 接口用于动态维护业务禁用日期；禁用日期与 `min_date` / `max_date` 会合并为统一的不可选状态。

Calendar 控件在视图模式（月/年/十年）发生变化时触发 `kEventViewModeChanged` 事件（wParam=新视图模式，取值 0/1/2；lParam=旧视图模式，取值 0/1/2），可通过 `AttachViewModeChanged` 订阅；显示周期（月视图的月份、年/十年视图的年份）发生变化时触发 `kEventDisplayDateChanged` 事件，可通过 `AttachDisplayDateChanged` 订阅（常用于浮层标题栏随翻页/跨月移动刷新）；选中日期或日期范围变化时触发 `kEventValueChanged` 事件（与 DateTime 一致）。

日历浮层支持以下键盘交互（浮层打开并接管焦点后生效；Esc 关闭浮层）：

| 快捷键 | 月视图 | 年视图 | 十年视图 |
| :--- | :--- | :--- | :--- |
| ← / → | 焦点 ±1 天 | 焦点 ±1 月 | 焦点 ±1 年 |
| ↑ / ↓ | 焦点 ±1 周 | 焦点 ±3 月 | 焦点 ±3 年 |
| Ctrl + ← / → / ↑ / ↓ | 焦点 ±1 年 | 焦点 ±10 年（上一个/下一个十年） | 焦点 ±100 年（上一个/下一个世纪） |
| Home | 跳到当月首日 | 跳到当年首月（1 月） | 跳到当前十年首年 |
| End | 跳到当月末日 | 跳到当年末月（12 月） | 跳到当前十年末年 |
| PageUp / PageDown | 上一月 / 下一月 | 上一年 / 下一年 | 上一个十年 / 下一个十年 |
| Enter / Space | 选中焦点日期（单选直接确认；范围模式进入"待选结束日"或确认范围） | 下钻到月视图 | 下钻到年视图 |
| 范围模式下 Shift + ← / → / ↑ / ↓ | 以焦点为锚点扩展范围选择（实时高亮起止区间），再次 Enter/Space 确认 | — | — |

补充说明：
- 焦点移动与翻页都会收敛到 `min_date`/`max_date` 限制范围内，并跳过 `disabled_dates` 中的业务禁用日期，焦点环不会落在禁用日期上；翻到最早/最晚可选周期边界后不再继续前翻/后翻。
- 范围模式的键盘扩展与鼠标拖拽共用同一套起止逻辑与实时预览绘制。

## CalendarFlyout日历浮层
CalendarFlyout是基于Flyout承载Calendar控件的日期选择浮层，对应头文件`duilib/Control/CalendarFlyout.h`。用于在锚点控件周围弹出日历面板，支持单选/范围选择、月/年/十年三级导航，以及"日历 + 时间"或"仅时间"选择。

核心特性：
- 继承Flyout：自动获得8方位弹出、空间不足自动翻转、点击外部/Esc关闭等能力；
- 内嵌XML布局按模式动态生成：日期模式为头部导航（上一月/标题/下一月）+ Calendar + 底部按钮（今天/清除）；时间模式额外增加时:分[:秒]选择行与取消/确定按钮；仅时间模式不创建Calendar；
- 单选模式：点击日期后自动关闭并触发`AttachDateSelected`回调；
- 范围模式：拖拽选择起止日期后自动关闭并触发`AttachDateSelected`回调；
- 时间模式（`SetTimeMode`）：点击日期不关闭，只有点击"确定"才触发`AttachDateTimeSelected`并关闭；"取消"/Esc/点击外部不提交；
- 三级导航：点击标题在月/年/十年视图间循环切换。

### C++接口

| 接口 | 说明 |
| :--- | :--- |
| `CalendarFlyout(Window* pParentWindow)` | 构造浮层，传入父窗口（锚点控件必须属于该窗口） |
| `bool ShowAt(Control* anchor, const struct tm& initDate, Placement = Bottom)` | 在锚点周围显示日历浮层；返回false时对象已自动销毁，不可再访问 |
| `SetMode(int32_t mode)` | 设置选择模式：0=单选，1=范围 |
| `GetMode()` | 获取选择模式 |
| `SetInitRange(start, end)` | 设置范围选择的初始范围 |
| `SetFirstDayOfWeek(dayOfWeek)` | 设置每周第一天：0=周日，1=周一（默认1） |
| `SetDateLimit(minDate, maxDate)` | 设置可选日期范围限制（yyyy-mm-dd格式） |
| `SetTimeMode(bShowCalendar, bShowSeconds)` | 启用时间选择模式（必须在`ShowAt`之前调用）：`bShowCalendar=true`为日历+时间，`false`为仅时间；`bShowSeconds`控制秒列 |
| `SetDisabledDates(disabledDates)` | 设置业务禁用日期（`,` / `;` 分隔，单日或 `start~end`闭区间） |
| `ClearDisabledDates()` | 清空业务禁用日期 |
| `AttachDateTimeSelected(callback)` | 注册日期时间提交回调，参数为`(const struct tm& dateTime)`；时间模式下点击"确定"触发 |
| `AttachDateSelected(callback)` | 注册日期选择完成回调（两参数签名，仅适用于单选模式），参数为`(WPARAM wParam, LPARAM lParam)`；wParam=0单选、1范围；lParam单选时为time_t，范围时无意义 |
| `AttachDateSelectedEx(callback)` | 注册日期选择完成回调（三参数签名，推荐），参数为`(WPARAM wParam, LPARAM lParam, const Calendar::DateRange* pRange)`；范围模式从pRange读取完整64位start/end |
| `AttachDateCleared(callback)` | 注册清除日期回调（点击"清除"按钮时触发） |

### 使用示例

```cpp
//单选日期
ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
pFlyout->SetMode(0);
struct tm today = ui::Calendar::GetToday();
pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom);
pFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
    if (wParam == 0) {
        time_t t = (time_t)lParam;
        struct tm date = ui::Calendar::TimeTToDate(t);
        DString text = ui::Calendar::FormatDateString(date);
        //处理选中日期
    }
});

//范围选择
ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
pFlyout->SetMode(1);
pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom);
pFlyout->AttachDateSelectedEx([this](WPARAM wParam, LPARAM lParam, const ui::Calendar::DateRange* pRange) {
    if (wParam == 1) {
        //范围模式：起止值统一从 pRange 读取（完整 64 位）
        struct tm start = ui::Calendar::TimeTToDate(pRange->start);
        struct tm end = ui::Calendar::TimeTToDate(pRange->end);
        //处理选中范围
    }
});

//日历 + 时间（或仅时间）：bShowCalendar=false 时只有时间行
ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
pFlyout->SetTimeMode(true, false);      //显示日历，不显示秒
pFlyout->SetDateLimit(_T("2026-01-01"), _T("2026-12-31"));
pFlyout->SetDisabledDates(_T("2026-10-01~2026-10-07,2026-12-25"));
pFlyout->SetFirstDayOfWeek(1);
pFlyout->AttachDateTimeSelected([this](const struct tm& dateTime) {
    ui::DateTime* pDateTime = dynamic_cast<ui::DateTime*>(pAnchor);
    if (pDateTime != nullptr) {
        pDateTime->SetDateTime(dateTime);
    }
});
pFlyout->ShowAt(pAnchor, today, ui::Flyout::Placement::Bottom);
```

DateTime控件（`edit_format="date_calendar"` / `"date_time_calendar"` / `"time_calendar"`）已内置使用CalendarFlyout，无需手动创建。

## HotKey的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| default_text | | string | 默认显示的文字 |
| default_text_id | | string | 默认显示的文字ID |

HotKey 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## HyperLink的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| url | | string | URL |
| show_url_tooltip | true | bool | 是否将URL显示为ToolTip |

HyperLink 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## IPAddress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| ip | | string | IP地址，比如："192.168.0.0" |

IPAddress 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## Line的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vertical | false | bool | 是否为垂直的线 |
| line_color | | string | 线的颜色 |
| line_width | 1.0 | float | 线的宽度 |
| dash_style | | string | 线的形状，可选值："solid"：实线；"dash"：短划线构成的虚线；"dot"：点构成的虚线；"dash_dot"：交替短划线点线；"dash_dot_dot"：交替短划线点点线|

Line 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## Menu的属性
Menu是一个窗口，具体用法请参考示例程序中的菜单    
Menu 控件继承了 `Window` 属性，更多可用属性请参考`Window`的属性

## Progress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| horizontal | true | bool | 是否水平的，"true"表示水平，"false"表示垂直 |
| min | 0 | int | 进度最小值,如"0" |
| max | 100 | int | 进度最大值,如"100" |
| value | 0 | double | 进度值（支持浮点），如"50"或"0.5" |
| progress_image |  | string | 进度条前景图片 |
| stretch_fore_image | true | bool | 指定进度条前景图片是否缩放显示 |
| progress_color |  | string | 进度条前景颜色（语义色名或颜色值），如"bg_progress" |
| progress_radius | 0 | int | 纯色前景的圆角半径（像素，自动DPI缩放，0为直角） |
| buffer_value | -1 | double | 次级进度值（缓冲/buffer，通常比主进度略靠前，如视频已缓存位置；≤min 不绘制） |
| buffer_color |  | string | 次级进度颜色（语义色名或颜色值，为空则用前景色的浅色版） |
| show_percent | false | bool | 是否自动显示百分比文本（覆盖控件 text） |
| animation_enabled | false | bool | 是否启用值变化动画（默认关闭） |
| animation_duration | 300 | int | 值变化动画时长（毫秒） |
| animation_easing | EaseOutQuad | string | 值变化动画的缓动曲线，取值与 `EasingFunctionType` 枚举名完全一致，如"EaseLinear"、"EaseOutQuad"、"EaseOutElastic"、"EaseInOutBounce"（首字母大小写均可；无法识别时按 `EaseLinear` 处理） |
| marquee | false | bool | 是否滚动显示 |
| marquee_width | | int | 滚动的宽度 |
| marquee_step | | int | 滚动的步长 |
| reverse | false | bool | 进度是否反向填充：`false` 为 value=min 时空、value=max 时满格；`true` 则相反（value=min 时满格）。`min != 0` 时同样按比例正确换算 |

Progress 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

说明：
- `value` 支持浮点值（XML 按 double 解析，`SetValue(double)` 亦如此），适用于下载进度、音量等 0~1 区间场景。
- `progress_color` 与 `progress_image` 二选一作为前景；纯色前景可用 `progress_radius` 设圆角。轨道背景用 `bkcolor`（Control 通用属性）。
- `buffer_value` 设置次级进度（视频/下载的"已缓冲 + 已播放"双进度），绘制在主进度下方的浅色条；`buffer_color` 不指定时用前景色的浅色版（约 40% 不透明度）。
- `show_percent` 开启后控件文本自动更新为"xx%"（跟随值变化动画）；可用 `SetTextFormatCallback` 自定义格式（如 "5/10"、"3.2 GB"）。
- `animation_enabled` 开启后 `SetValue` 会从当前值缓动到新值（时长 `animation_duration`，缓动曲线由 `animation_easing` 决定，默认 `EaseOutQuad`）；Slider 拖动为即时响应，通常无需开启。
- `min`/`max` 在运行时修改会**自动把当前值钳位回新区间**（含`buffer_value` 与动画中间值），因此 `GetValue()` 始终在 `[min, max]` 内，不会出现"绘制被钳位但成员值越界"的不一致。可手动调用 `ClampCurrentValue()` 主动钳位。
- `progress_radius` 现在**同时作用于纯色主进度、buffer 次级进度与 marquee 滚动块**，三者风格一致。
- marquee 以「轨道长度 + 滚动块宽度」为周期循环回绕（取模并处理负值），轨道长度 <= 0 时复位为 0。
- 值变化事件：`AttachValueChanged`（`EventArgs::wParam` 为新值、`lParam` 为旧值，均按整数截断；精确浮点值从 `GetValue()` 读取）。
- 程序内部同步数值时请使用 `SetValueNoEvent()`：赋值但不派发 `kEventValueChanged` 事件（与文本控件的 `SetTextNoEvent` 语义一致）。用于"一个事件源驱动多个控件同步显示"的场景，避免值变化回调被再次触发形成循环调用；`Slider`、`ColorSlider` 自动继承该接口。
- 完成事件：`AttachProgressCompleted`——进度值到达完成端点（正向到 `max`、倒数到 `min`）时触发一次。
- 推荐语义色：前景 `bg_progress`、轨道 `bg_progress_track`（随深浅色主题自动切换）。

### 动画缓动曲线
`animation_easing` 直接复用框架已有的 `ui::EasingFunctionType` 枚举（见 `duilib/Animation/EasingFunctions.h`），共 31 条曲线：

```xml
<!-- 线性 -->
<Progress name="p1" min="0" max="100" value="70" animation_enabled="true" animation_duration="600" animation_easing="EaseLinear"/>
<!-- 缓出（二次，默认） -->
<Progress name="p2" min="0" max="100" value="70" animation_enabled="true" animation_duration="600" animation_easing="EaseOutQuad"/>
<!-- 弹性 -->
<Progress name="p3" min="0" max="100" value="70" animation_enabled="true" animation_duration="900" animation_easing="EaseOutElastic"/>
```

曲线分组：`EaseLinear`、`Ease{In|Out|InOut}{Sine|Quad|Cubic|Quart|Quint|Expo|Circ|Back|Elastic|Bounce}`。
运行时可随时修改，下一次动画即生效（不影响正在播放的动画）。

对应 C++ 接口：`SetAnimationEasing(EasingFunctionType)`、`SetAnimationEasingName(const DString&)`、`GetAnimationEasing()`、`GetAnimationEasingName()`。

### 反向填充（reverse）
`reverse="true"` 时填充方向反转：`value=min` 为满格、`value=max` 为空格。
内部实现是**先把 value 归一化成0~1 的比例再取反**，而不是对绝对值做 `(max-min)-value` 的翻转——
后者在 `min != 0`（如 `min="10" max="100"`）时会算出越界比例甚至负数。

`Progress`、`CircleProgress` 的反向换算语义一致。

### 值域运行时钳位
`SetMinValue`/`SetMaxValue` 会自动把当前值拉回新区间，避免成员值与绘制结果不一致：

```cpp
pProgress->SetValue(80.0);
pProgress->SetMaxValue(50);
//此时 GetValue() == 50，而不是 80
```

同时钳位 `m_fCurrentValue`、动画中间值 `m_fDisplayValue` 与 `buffer_value`（未设置 buffer 时保持原状）。
区间非法（`min > max`）时不改动，交由既有绘制逻辑兜底。
对应 C++ 接口：`ClampCurrentValue()`。

## Slider的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| thumb_normal_image |  | string | 拖动滑块普通状态图片 |
| thumb_hovered_image |  | string | 拖动滑块鼠标悬浮状态图片 |
| thumb_pressed_image |  | string | 拖动滑块鼠标按下状态图片 |
| thumb_disabled_image |  | string | 拖动滑块鼠标禁用状态图片 |
| thumb_size | 10,10 | size | 拖动滑块大小,如"10,10" |
| step | 1 | int | 进度步长,如"1" |
| page_step | 0 | int | PageUp/PageDown（及Shift+方向键）的大步长，0 表示自动取 (max-min)/10 |
| click_to_jump | true | bool | 点击轨道是否**按下即跳值**；false 则保持"松开鼠标才跳值"的旧行为 |
| progress_bar_padding | 0,0,0,0 | rect | 滑动条绘制时缩小的内边距 |
| show_ticks | false | bool | 是否显示刻度线（标尺） |
| tick_count | 5 | int | 刻度线数量（至少 2，含首尾两端） |
| tick_length | 4 | int | 刻度线长度（像素，内部按 DPI 缩放） |
| tick_color | text_muted | string | 刻度线颜色（语义色名或颜色值） |
| tick_pos | bottom | string | 刻度线位置：bottom/top（水平滑块）、right/left（垂直滑块） |
| show_tick_labels | false | bool | 是否显示刻度值文本（需 show_ticks 为 true） |
| tick_label_interval | 1 | int | 每隔多少个刻度显示一次刻度值文本 |
| tick_font | system_regular_12 | string | 刻度值文本的字体ID |
| tick_label_color | text_muted | string | 刻度值文本的颜色 |

Slider 控件继承了 `Progress` 属性，更多可用属性请参考`Progress`的属性

### 键盘操作（无障碍）
Slider 已接入 Tab 焦点循环（`GetControlFlags` 返回 `UIFLAG_TABSTOP`），并处理 `kEventKeyDown`：

| 按键 | 行为 |
| :--- | :--- |
| ←/→（水平）、↑/↓（垂直） | 按 `step` 步进/步退 |
| Shift + 方向键 | 按 `page_step` 大步进/大步退 |
| PageUp / PageDown | 按 `page_step` 大步进/大步退（与方向轴无关） |
| Home / End | 直接跳到 `min` / `max` |
| 鼠标滚轮 | 按 `step` 步进/步退 |

所有键盘操作都会自动钳位到 `[min, max]`，并触发 `kEventValueChanged`。
`page_step` 为 0（默认）时自动取 `(max-min)/10`，且不小于 `step`。

### 刻度与标尺
滑块支持类似 Chart 坐标刻度的刻度线与刻度值绘制，典型场景是音量条、时间轴拖动：

```xml
<Slider name="volume" min="0" max="100" value="30" height="42" width="stretch"
        show_ticks="true" tick_count="11" tick_length="5" tick_pos="bottom"
        tick_color="border_control_normal"
        show_tick_labels="true" tick_label_interval="2"
        tick_font="system_regular_12" tick_label_color="text_muted"/>
```

刻度位置按滑块**可移动区间**计算，与滑块中心严格对齐；水平滑块的刻度画在轨道上方或下方，垂直滑块画在轨道左侧或右侧。
刻度值文本默认对整数刻度去掉小数（显示 `20` 而非 `20.00`），非整数刻度保留两位小数。

视觉规则：
- **主次刻度分层**：带标签的刻度（`i % tick_label_interval == 0`）为长主刻度，其余为长度约一半的短次刻度，形成标尺的视觉层级
- **标签自适应宽度**：标签半宽按实际刻度间距自动收窄（不超过间距的一半），从设计上避免相邻标签重叠
- **首尾靠边对齐**：首个标签左对齐、末个标签右对齐（垂直滑块为顶/底对齐），文字向控件内侧展开。
  这样首尾标签不会因为平移出界而侵占相邻标签的空间，也保证文字完整不被裁剪

自定义刻度文本（如时间轴的 `03:20`）用 `SetTickLabelCallback`：

```cpp
pSlider->SetTickLabelCallback([](double value, int32_t /*index*/) {
    int nTotal = (int)value;
    return ui::StringUtil::Printf(_T("%02d:%02d"), nTotal / 60, nTotal % 60);
    });
```

### 点击轨道行为
`click_to_jump=true`（默认）为**按下即跳值**：在轨道任意位置按下鼠标左键，值立即跳到该位置，并进入拖动状态（可继续按住拖动）。这是桌面端更常见的交互。
`click_to_jump=false` 保留原有行为：仅在松开鼠标时跳到松开位置。

## RangeSlider 控件（继承 Progress 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| low_value | 0 | double | 下限值（支持浮点），如"200" |
| high_value | max | double | 上限值（支持浮点），与基类的 `value` 等价 |
| min_range | 0 | double | 下限与上限之间的最小间距（下限不得超过"上限 - min_range"）|
| range_color |  | string | 高亮选中区间的颜色（语义色名或颜色值，为空则回退使用 `progress_color`）|
| range_radius | 0 | int | 高亮选中区间的圆角半径（像素，自动DPI缩放，0为直角）|
| click_to_jump | true | bool | 点击轨道是否**按下即跳值**；false 则保持"松开鼠标才跳值"的旧行为 |
| thumb_size | 10,10 | size | 两个滑块的统一大小，如"10,10" |
| thumb_normal_image / thumb_hovered_image / thumb_pressed_image / thumb_disabled_image |  | string | 下限滑块各状态图片 |
| high_thumb_normal_image / high_thumb_hovered_image / high_thumb_pressed_image / high_thumb_disabled_image |  | string | 上限滑块各状态图片（为空时回退使用下限滑块的图片）|

RangeSlider 控件继承了 `Progress` 属性，更多可用属性请参考`Progress`的属性

双滑块范围控件在一条轨道上放置下限、上限两个滑块，被选中的区间以高亮显示，常用于价格 / 时间等区间筛选场景。
上限值直接复用基类的进度值（`value` / `GetValue`），下限值独立维护（`low_value` / `GetLowValue`）；边界 `min` / `max` 同样复用基类属性。
下限与上限都会自动钳位：下限落在 `[min, high - min_range]`、上限落在 `[low + min_range, max]`；拖动、点击轨道与 `SetXXX` 均遵循该规则。

XML 示例：
```xml
<RangeSlider class="range_slider_horizontal" name="price_range" min="0" max="1000"
             low_value="200" value="800" min_range="50" width="stretch"/>
```

对应的 C++ 接口：
```cpp
double GetLowValue() const;                          //下限值
void SetLowValue(double fValue);                     //设置下限（自动钳位，触发值变化事件）
void SetLowValueNoEvent(double fValue);              //设置下限（不触发事件）
double GetHighValue() const;                         //上限值（与 GetValue() 等价）
void SetHighValue(double fValue);                    //设置上限（自动钳位，触发事件）
void SetHighValueNoEvent(double fValue);             //设置上限（不触发事件）
void SetRange(double fLowValue, double fHighValue);  //同时设置下限与上限（触发一次事件）
void SetRangeNoEvent(double fLowValue, double fHighValue);
double GetMinRange() const;                          //最小间距
void SetMinRange(double fMinRange);
void SetThumbSize(UiSize szXY, bool bNeedDpiScale);  //滑块大小
UiSize GetThumbSize() const;
UiRect GetLowThumbRect() const;                      //下限滑块矩形
UiRect GetHighThumbRect() const;                     //上限滑块矩形
DString GetThumbStateImage(ControlStateType stateType) const;       //下限滑块图片
void SetThumbStateImage(ControlStateType stateType, const DString& strImage);
DString GetHighThumbStateImage(ControlStateType stateType) const;   //上限滑块图片
void SetHighThumbStateImage(ControlStateType stateType, const DString& strImage);
void SetRangeColor(const DString& strColor);         //高亮区间颜色
const DString& GetRangeColor() const;
void SetRangeRadius(int32_t nRadius);                //高亮区间圆角
int32_t GetRangeRadius() const;
void SetClickToJump(bool bEnable);                   //点击轨道是否按下即跳值
bool IsClickToJump() const;
void AttachRangeChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //范围值变化事件
```

值变化事件：`AttachRangeChanged`——范围值变化时触发，事件编码为 `kEventValueChanged`（`wParam` 为 `1`），下限与上限通过 `EventArgs::pEventData` 携带（指向 `RangeSlider::RangeValue`，含 `lowValue` / `highValue`，仅在本次回调派发期间有效，请勿保存该指针）：
```cpp
pRange->AttachRangeChanged([](const ui::EventArgs& msg) {
    if (msg.pEventData != nullptr) {
        auto* pRange = static_cast<const ui::RangeSlider::RangeValue*>(msg.pEventData);
        // 读取 pRange->lowValue / pRange->highValue
    }
    return true;
});
```

## Stepper 控件（向导步骤条）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| step_title |  | string | 各步骤标题，多条之间用 `|` 分隔，如"选择安装位置\|许可协议\|安装中\|完成" |
| step_title_id |  | string | 各步骤标题的多语言 ID（`|` 分隔），语言切换时自动刷新 |
| step_desc |  | string | 各步骤描述，多条之间用 `|` 分隔 |
| step_desc_id |  | string | 各步骤描述的多语言 ID（`|` 分隔），语言切换时自动刷新 |
| current / current_index | 0 | int | 当前步骤索引（0 起），位于其前的步骤显示为已完成 |
| direction / orientation | horizontal | string | 排列方向：`horizontal`（水平，默认）/ `vertical`（垂直） |
| click_to_jump | true | bool | 鼠标点击步骤是否跳转到该步骤 |
| node_size | 24 | int | 节点直径（像素，自动 DPI 缩放） |
| disabled_steps |  | string | 禁用步骤索引列表，用 `|` 或 `,` 分隔，如"2\|3"（禁用步骤不可跳转） |
| node_color_pending / node_color_active / node_color_completed / node_color_disabled | text_muted / color_accent / color_success / border_control_normal | string | 各状态节点颜色（语义色名或颜色值） |
| connector_color | border_control_normal | string | 未完成段连接线颜色 |
| connector_active_color | color_success | string | 已完成段连接线颜色 |
| title_color_active / title_color_completed / title_color_pending | text_default / color_success / text_muted | string | 各状态标题颜色 |
| desc_color | text_muted | string | 描述文本颜色 |
| title_font / desc_font | system_bold_14 / system_regular_12 | string | 标题 / 描述字体 ID |
| node_hovered_image（别名 node_hot_image） |  | string | 节点悬停态图片，可选；鼠标悬停于可点击步骤上时显示（绘制在节点圆形下层） |
| node_pressed_image（别名 node_pushed_image） |  | string | 节点按下态图片，可选；鼠标按下可点击步骤时显示，未配置时回退悬停态图片 |

Stepper 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性。

向导步骤条用于安装 / 注册 / 配置向导、审批流程等"流程状态机"场景（`Timeline` 是时间轴语义，不承载流程步骤）。
步骤状态由"当前索引 + 禁用集合"派生：位于当前索引之前的步骤为已完成（节点显示对勾），当前步骤为进行中，之后的为待开始；被显式禁用的步骤优先显示为禁用态。连接线按已完成段着色。

悬停/按下态图片仅对"可交互步骤"（控件启用、`click_to_jump` 开启且该步非禁用）在鼠标悬停/按下时显示，用于呈现强调色光晕/描边等反馈，不影响节点本色与序号/对勾。默认皮肤的 `stepper_node` 类已内置一组光晕环图标（颜色经 `svg_replace_colors` 跟随主题强调色），可直接 `class="stepper_node"` 引用。

XML 示例：
```xml
<Stepper class="stepper_node" name="stepper_demo" width="stretch" height="stretch" node_size="28" current="1"
         step_title_id="STRID_S1_TITLE|STRID_S2_TITLE|STRID_S3_TITLE|STRID_S4_TITLE"
         step_desc_id="STRID_S1_DESC|STRID_S2_DESC|STRID_S3_DESC|STRID_S4_DESC"/>
```

对应的 C++ 接口：
```cpp
size_t GetStepCount() const;                          //步骤数量
const Step* GetStepAt(size_t index) const;            //指定步骤数据
void AddStep(const Step& step);                       //追加步骤
void AddStep(const DString& title, const DString& description = DString());
bool InsertStep(size_t index, const Step& step);
bool RemoveStepAt(size_t index);
void ClearSteps();
void SetSteps(const std::vector<Step>& steps);
size_t GetCurrentIndex() const;                       //当前步骤索引（已钳位）
void SetCurrentIndex(size_t nIndex);                  //设置当前步骤（自动钳位，索引变化触发事件）
void SetCurrentIndexNoEvent(size_t nIndex);           //设置当前步骤（不触发事件）
bool Next();  bool NextNoEvent();                     //前进到下一步（跳过禁用步骤，失败返回 false）
bool Back();  bool BackNoEvent();                     //后退到上一步
bool GoTo(size_t nIndex);  bool GoToNoEvent(size_t nIndex); //跳转到指定步骤（禁用步骤返回 false）
void SetStepDisabled(size_t index, bool bDisabled);   //设置步骤是否禁用
bool IsStepDisabled(size_t index) const;
void SetDisabledSteps(const std::vector<size_t>& disabledSteps);
StepStatus GetStepStatus(size_t index) const;         //派生状态：kPending/kActive/kCompleted/kDisabled
void SetHorizontal(bool bHorizontal);                 //排列方向
bool IsHorizontal() const;
void SetNodeSize(int32_t nSize);                      //节点直径
int32_t GetNodeSize() const;
void SetClickToJump(bool bEnable);                    //点击步骤是否跳转
bool IsClickToJump() const;
void SetStateColor(StepStatus status, const DString& strColor);   //各状态节点颜色
void SetTitleColor(StepStatus status, const DString& strColor);   //各状态标题颜色
void SetConnectorColor(const DString& strColor);                  //未完成段连接线颜色
void SetConnectorActiveColor(const DString& strColor);            //已完成段连接线颜色
void SetDescColor(const DString& strColor);                       //描述文本颜色
void SetNodeStateImage(ControlStateType stateType, const DString& strImage); //节点悬停/按下态图片
DString GetNodeStateImage(ControlStateType stateType) const;
void AttachStepChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //当前步骤变化事件
```

值变化事件：`AttachStepChanged`——当前步骤变化时触发，事件编码为 `kEventValueChanged`（`wParam` 为 `2`），新/旧索引通过 `EventArgs::pEventData` 携带（指向 `Stepper::StepValue`，含 `currentIndex` / `oldIndex`，仅在本次回调派发期间有效，请勿保存该指针）：
```cpp
pStepper->AttachStepChanged([](const ui::EventArgs& msg) {
    if (msg.pEventData != nullptr) {
        auto* pValue = static_cast<const ui::Stepper::StepValue*>(msg.pEventData);
        // 读取 pValue->currentIndex / pValue->oldIndex
    }
    return true;
});
```

## Rating 控件（星级评分）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| value | 0 | double | 当前评分值，钳位到 `[0, max]`，按 `allow_half` 对齐步长 |
| max（别名 star_count） | 5 | int | 星星数量 |
| allow_half | false | bool | 是否允许半星（步长 0.5，否则 1） |
| read_only（别名 readonly） | false | bool | 只读展示模式，禁止鼠标交互 |
| star_size | 20 | int | 单颗星绘制直径（像素，自动 DPI 缩放） |
| star_gap | 4 | int | 星星间距（像素，自动 DPI 缩放） |
| star_filled_image（别名 star_full_image） |  | string | 实心星图片 |
| star_half_image |  | string | 半星图片（可选，未配置时对实心星做左半裁剪绘制） |
| star_empty_image |  | string | 空心星图片 |
| star_hovered_image（别名 star_hover_image） |  | string | 星悬停态图标，可选（绘制在星形下层，形成外圈光晕 / 描边） |
| star_pressed_image（别名 star_pushed_image） |  | string | 星按下态图标，可选；未配置时回退悬停态图标 |

Rating 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性。

星级评分用于评价 / 打分 / 满意度调查等场景，支持只读展示、半星、鼠标悬停预览与点击 / 拖拽打分。外观完全由 SVG 图片驱动（不做星形自绘）：

- 星形内容图（实心 / 半星 / 空心）经 `svg_replace_colors` 映射到主题语义色，切换主题时自动跟随；
- 悬停 / 按下态图标为可选的状态反馈图，绘制在星形下层（形成外圈光晕 / 描边效果），仅对"可交互的星星"（控件启用、非只读且鼠标启用）在鼠标悬停 / 按下时显示。

默认皮肤的 `rating` 类已内置五张图（实心 / 半星 / 悬停态 / 按下态映射 `color_accent`，空心星映射 `border_control_normal`），可直接 `class="rating"` 引用。

XML 示例：
```xml
<Rating class="rating" name="rating_demo" value="3" star_size="28" star_gap="6" width="170" height="40"/>
```

对应的 C++ 接口：
```cpp
double GetValue() const;
void SetValue(double fValue);                  //钳位 + 步长对齐，变化触发事件
void SetValueNoEvent(double fValue);           //设置值（不触发事件）
int32_t GetMax() const;                        //星星数量（别名 GetStarCount）
void SetMax(int32_t nMax);                     //调小 max 时同步钳位 value
bool IsAllowHalf() const;  void SetAllowHalf(bool bAllow);
bool IsReadOnly() const;   void SetReadOnly(bool bReadOnly);
int32_t GetStarSize() const;  void SetStarSize(int32_t nSize);
int32_t GetStarGap() const;   void SetStarGap(int32_t nGap);
void SetStarImage(StarImageType type, const DString& strImage);  //StarImageType: kFilled/kHalf/kEmpty
DString GetStarImage(StarImageType type) const;
void SetFilledImage(const DString& strImage);  //实心星图片
void SetHalfImage(const DString& strImage);    //半星图片
void SetEmptyImage(const DString& strImage);   //空心星图片
void SetStarStateImage(ControlStateType stateType, const DString& strImage); //星状态图标: kControlStateHovered/kControlStatePressed
DString GetStarStateImage(ControlStateType stateType) const;
void SetHoveredImage(const DString& strImage); //星悬停态图标
void SetPressedImage(const DString& strImage); //星按下态图标
void AttachRatingChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //评分值变化事件
```

值变化事件：`AttachRatingChanged`——评分值变化时触发，事件编码为 `kEventValueChanged`，新/旧值通过 `EventArgs::pEventData` 携带（指向 `RatingValue`，含 `value` / `oldValue`，仅在本次回调派发期间有效，请勿保存该指针）：
```cpp
pRating->AttachRatingChanged([](const ui::EventArgs& msg) {
    if (msg.pEventData != nullptr) {
        auto* pValue = static_cast<const ui::RatingValue*>(msg.pEventData);
        // 读取 pValue->value / pValue->oldValue
    }
    return true;
});
```

## Carousel 控件（图片轮播）

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_image |  | string | 幻灯片图片列表，`\|` 分隔的图片属性串，每项如 `file='.../slide1.png'` |
| current_index | 0 | int | 当前幻灯片索引（0 起），钳位到 `[0, count-1]` |
| loop | true | bool | 是否循环（首尾相接，无限滚动） |
| auto_play（别名 autoplay） | false | bool | 是否自动播放 |
| interval | 3000 | int | 自动播放间隔（毫秒） |
| hover_pause | true | bool | 鼠标悬停时暂停自动播放 |
| transition_ms | 300 | int | 切换动画时长（毫秒，0 = 立即切换） |
| show_indicator | true | bool | 是否显示页码指示点 |
| indicator_normal_image |  | string | 指示点常态图 |
| indicator_active_image |  | string | 指示点当前页图 |
| indicator_size | 8 | int | 指示点尺寸（像素，自动 DPI 缩放） |
| indicator_gap | 8 | int | 指示点间距（像素，自动 DPI 缩放） |
| indicator_bottom | 10 | int | 指示点距内容区底边距离（像素，自动 DPI 缩放） |
| show_nav_button | true | bool | 是否显示左右切换按钮 |
| btn_prev_image / btn_next_image |  | string | 上一张 / 下一张按钮图 |
| btn_prev_hovered_image / btn_next_hovered_image |  | string | 按钮悬停态图片（可选） |
| btn_prev_pressed_image / btn_next_pressed_image |  | string | 按钮按下态图片（可选，回退悬停 / 常态） |
| btn_size | 32 | int | 切换按钮尺寸（像素，自动 DPI 缩放） |
| btn_margin | 12 | int | 切换按钮距内容区边缘距离（像素，自动 DPI 缩放） |

Carousel 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性。

图片轮播用于首页横幅、广告位、图片集等场景，逐张展示幻灯片（同一时刻显示一张）。外观完全由 SVG 图片驱动（不做图形自绘）：

- 幻灯片、左右切换按钮、底部页码指示点均以图片承载，经 `svg_replace_colors` 把占位色映射到主题语义色，切换主题自动生效；
- 支持左右切换、底部可点击的页码指示点（当前页高亮）、`loop` 无限循环、`auto_play` 自动播放 + `hover_pause` 悬停暂停、`transition_ms` 水平滑动动画，以及按住鼠标左右拖拽翻页。

默认皮肤的 `carousel` 类已内置导航按钮与指示点图片及语义色映射，可直接 `class="carousel"` 引用。

XML 示例：
```xml
<Carousel class="carousel" name="carousel_demo" width="stretch" height="150"
          item_image="file='.../slide1.svg'|file='.../slide2.svg'|file='.../slide3.svg'"
          auto_play="true" interval="2500" loop="true" transition_ms="350"/>
```

对应的 C++ 接口：
```cpp
int32_t GetCurrentIndex() const;                 //当前索引，无幻灯片为 -1
void SetCurrentIndex(int32_t nIndex);            //钳位 + 变化时派发事件
void SetCurrentIndexNoEvent(int32_t nIndex);
int32_t GetItemCount() const;
void AddItem(const DString& strImage);           //追加一张（图片属性串）
void InsertItem(int32_t nIndex, const DString& strImage);
bool RemoveItem(int32_t nIndex);
void ClearItems();
void SetItems(const std::vector<DString>& items);
DString GetItem(int32_t nIndex) const;
bool Next();  bool Prev();  bool GoTo(int32_t nIndex);       //loop 越界首尾相接；非 loop 越界返回 false
bool NextNoEvent();  bool PrevNoEvent();  bool GoToNoEvent(int32_t nIndex);
bool IsLoop() const;       void SetLoop(bool bLoop);
bool IsAutoPlay() const;   void SetAutoPlay(bool bAutoPlay);
int32_t GetInterval() const; void SetInterval(int32_t nIntervalMs);
bool IsHoverPause() const; void SetHoverPause(bool bHoverPause);
int32_t GetTransitionMs() const; void SetTransitionMs(int32_t nMs);
void AttachCarouselChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //索引变化事件
```

索引变化事件：`AttachCarouselChanged`——当前索引变化时触发，事件编码为 `kEventValueChanged`，新/旧索引通过 `EventArgs::pEventData` 携带（指向 `CarouselValue`，含 `index` / `oldIndex`，仅在本次回调派发期间有效，请勿保存该指针）：
```cpp
pCarousel->AttachCarouselChanged([](const ui::EventArgs& msg) {
    if (msg.pEventData != nullptr) {
        auto* pValue = static_cast<const ui::CarouselValue*>(msg.pEventData);
        // 读取 pValue->index / pValue->oldIndex
    }
    return true;
});
```

## Pagination 控件（分页器）

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| page_count（别名 total_page） | 0 | int | 总页数 |
| current_page（别名 page） | 1 | int | 当前页码（1 起），钳位到 `[1, page_count]` |
| total_count | 0 | int | 总记录数（与 `page_size` 配合推导 `page_count`，可选） |
| page_size | 0 | int | 每页记录数（可选；`total_count` + `page_size` > 0 时优先推导） |
| max_visible_count | 7 | int | 序号区最多显示的页码按钮数（含首尾），超出用 `…` 收纳 |
| show_prev_next | true | bool | 是否显示上一页 / 下一页 |
| show_first_last | false | bool | 是否显示首页 / 末页按钮 |
| prev_text / next_text | ‹ / › | string | 上一页 / 下一页文本 |
| first_text / last_text | « / » | string | 首页 / 末页文本 |
| ellipsis_text | … | string | 省略号文本 |
| button_class | pagination_button | string | 页码按钮引用的皮肤 Class（含常态 / 悬停 / 选中态） |
| button_width / button_height | 32 / 32 | int | 页码按钮尺寸（像素，自动 DPI 缩放；0 = 自适应） |

Pagination 控件继承了 `HBox` 的全部布局属性（如 `child_margin` / `padding` / `halign`）；页面按钮由控件内部按数据动态创建。

分页器用于报表 / 表格 / 列表等场景的页码导航：上一页 / 下一页 + 页码按钮，当前页按钮呈选中态；页码超出显示上限时以 `…` 收纳，始终保留首页 / 末页与当前页附近。颜色由 `button_class` 引用的皮肤样式决定（走语义色），C++ 侧不写死色值。

XML 示例：
```xml
<Pagination name="pagination_demo" width="stretch" height="40" child_margin="2"
            page_count="10" current_page="3" show_prev_next="true" max_visible_count="7"/>
```

对应的 C++ 接口：
```cpp
int32_t GetPageCount() const;    void SetPageCount(int32_t nPageCount);
int32_t GetCurrentPage() const;  void SetCurrentPage(int32_t nPage);     //钳位 + 变化时派发事件
void SetCurrentPageNoEvent(int32_t nPage);
void SetTotalCount(int32_t nTotalCount, int32_t nPageSize);             //由总数推导 page_count
bool Next();  bool Prev();  bool GoTo(int32_t nPage);
int32_t GetMaxVisibleCount() const; void SetMaxVisibleCount(int32_t nCount);
void SetButtonClass(const DString& strClass);  DString GetButtonClass() const;
void AttachPageChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //页码变化事件
```

页码变化事件：`AttachPageChanged`——当前页码变化时触发，事件编码为 `kEventValueChanged`，新/旧页码通过 `EventArgs::pEventData` 携带（指向 `PaginationValue`，含 `page` / `oldPage`，仅在本次回调派发期间有效，请勿保存该指针）。

## Breadcrumb 控件（面包屑导航）

Breadcrumb 是面包屑导航控件，继承自 `Control`，纯自绘实现（不创建子控件），对应头文件 `duilib/Control/Breadcrumb.h`。用于文件 / 目录路径、站点层级、设置页层级等场景：一行显示可点击的祖先路径 + 当前项，点击祖先项可切换当前项并触发事件。与 `AddressBar`（浏览器地址栏）、`Stepper`（流程状态机）语义不同，故独立实现。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_text | 空 | string | 条目文本，`\|` 分隔多条，如 `首页\|产品\|文档` |
| item_text_id | 空 | string | 条目文本的多语言 ID，`\|` 分隔多条；语言切换时自动刷新 |
| current（别名 current_index） | -1 | int | 当前项索引（0 起）；-1 表示自动跟随最后一项 |
| separator（别名 separator_text） | › | string | 条目之间的分隔符 |
| spacing | 4 | int | 分隔符两侧的间距（像素，自动 DPI 缩放） |
| item_padding | 6 | int | 条目点击区域左右内边距（像素，自动 DPI 缩放） |
| item_round | 4 | int | 悬停 / 按下胶囊背景的圆角半径（像素，自动 DPI 缩放） |
| click_to_jump | true | bool | 是否允许点击祖先项切换当前项 |
| disabled_items | 空 | string | 禁用项索引列表，`\|` 或 `,` 分隔，如 `1\|2` |
| item_color | color_blue | string | 可点击祖先项颜色（语义色名或颜色值） |
| current_color | text_default | string | 当前项颜色 |
| hovered_color | color_blue_light | string | 祖先项悬停颜色 |
| pressed_color | color_blue | string | 祖先项按下颜色 |
| disabled_color | text_disabled | string | 禁用项颜色 |
| separator_color | color_gray | string | 分隔符颜色 |
| hover_bg_color | bg_btn_hovered | string | 悬停状态胶囊背景色（留空则不绘制） |
| pressed_bg_color | bg_btn_pressed | string | 按下状态胶囊背景色（留空则不绘制） |
| font | system_regular_14 | string | 普通条目与分隔符的字体 ID |
| current_font | system_bold_14 | string | 当前项字体 ID（加粗强调） |

XML 示例（`class` 属性必须写在所有属性最前面）：

```xml
<!-- 基础用法：当前项默认跟随最后一项 -->
<Breadcrumb class="breadcrumb" name="breadcrumb_demo" width="stretch" height="32"
            item_text="首页|产品中心|文档|控件库"/>

<!-- 多语言 + 自定义分隔符 + 显式当前项 + 禁用第 3 项 -->
<Breadcrumb class="breadcrumb" name="breadcrumb_custom" separator="&gt;" current="1" disabled_items="2"
            item_text_id="STRID_HOME|STRID_NETWORK|STRID_PROXY|STRID_DETAIL"/>
```

对应的 C++ 接口：

```cpp
void AddItem(const DString& text);                 bool InsertItem(size_t index, const DString& text);
bool RemoveItemAt(size_t index);                   void ClearItems();
void SetItems(const std::vector<Item>& items);     size_t GetItemCount() const;
const Item* GetItemAt(size_t index) const;
size_t GetCurrentIndex() const;                    void SetCurrentIndex(size_t index);      //钳位 + 变化时派发事件
void SetCurrentIndexNoEvent(size_t index);         bool GoTo(size_t index);                //禁用/越界返回 false
bool GoToNoEvent(size_t index);
void SetItemEnabled(size_t index, bool bEnabled);  bool IsItemEnabled(size_t index) const;
void SetDisabledItems(const std::vector<size_t>& items);
void SetSeparator(const DString& strSeparator);    void SetSpacing(int32_t nSpacing);
void SetItemPadding(int32_t nPadding);             void SetClickToJump(bool bEnable);
void SetItemRound(int32_t nRound);                 void SetFont(const DString& strFontId);
void SetCurrentFont(const DString& strFontId);
//颜色：SetItemColor / SetCurrentColor / SetHoveredColor / SetPressedColor / SetDisabledColor / SetSeparatorColor
//背景：SetHoveredBgColor / SetPressedBgColor（留空则不绘制胶囊背景）
void AttachCurrentChanged(const EventCallback& callback, EventCallbackID callbackID = 0);
```

当前项变化事件：`AttachCurrentChanged`——当前项变化时触发，事件编码为 `kEventValueChanged`，`wParam` 为 3（面包屑模式标记），新旧索引通过 `EventArgs::pEventData` 携带（指向 `BreadcrumbValue`，含 `currentIndex` / `oldIndex`，仅在本次回调派发期间有效，请勿保存该指针）。`SetCurrentIndexNoEvent` / `GoToNoEvent` 不派发事件。

行为说明：
- 当前项缺省（`current="-1"`）时自动跟随最后一项；`GetCurrentIndex()` 返回钳位后的有效索引。
- 只有"控件启用 + 鼠标启用 + `click_to_jump` 开启 + 条目未禁用 + 非当前项"的祖先项可点击，并显示悬停/按下态与手型光标。
- 祖先项默认为链接蓝（`color_blue`），当前项为深色加粗（`current_font`），分隔符为灰色 `›`，形成"可点击路径 / 当前层级"的清晰层级。
- 悬停/按下时在条目下方绘制圆角胶囊背景（`hover_bg_color` / `pressed_bg_color`），背景色留空可关闭。
- 文本颜色优先级：禁用 > 按下 > 悬停 > 当前 > 普通。
- 宽度不足时整体右对齐绘制、左端裁剪，保证末项 / 当前项可见；条目过多时建议业务侧先截短路径。
- 默认皮肤在 global.xml 中定义 `breadcrumb` Class（仅颜色与字体，无图片资源），自动适配深浅色主题。

## CascadingSelect 控件（级联选择器）

CascadingSelect 是级联选择控件，继承自 `Control`，纯自绘（不创建子控件），对应头文件 `duilib/Control/CascadingSelect.h`。控件在自身矩形内按数据深度绘制多列：点击某一列后，子列内容立即过滤为该节点的子集，深层旧选择立即重置。这正是它与 `Combo`（Tree 形态只能手工模拟两级）和 `TreeView`（树形展开，非列式）的核心语义差异。控件是内嵌形态，可直接放进任意面板；需要下拉效果时由业务侧把它放进 `Flyout` 中。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| items | 空 | string | 字面路径列表，`\|` 分隔多条路径，路径内用 `path_separator` 分隔层级，如 `华东/上海/浦东\|华南/深圳/南山` |
| items_id | 空 | string | 多语言路径列表，格式同 `items` 但每段是语言 ID；语言切换时自动刷新 |
| path_separator | / | string | 路径层级分隔符 |
| value_separator | " / " | string | `GetSelectionText()` 使用的连接符 |
| column_width | 0 | int | 固定列宽（像素，自动 DPI 缩放）；0 表示按控件宽度等分 |
| row_height | 28 | int | 行高（像素，自动 DPI 缩放） |
| text_padding | 10 | int | 行内文本左右内边距（像素，自动 DPI 缩放） |
| disabled_paths | 空 | string | 禁用路径列表，`\|` 分隔；路径等于节点文本路径或 ID 路径、或作为其前缀时，该节点及整棵子树禁用 |
| item_color | text_default | string | 普通行文本颜色（语义色名或颜色值） |
| selected_color | color_accent | string | 选中行文本颜色 |
| disabled_color | text_disabled | string | 禁用行文本颜色 |
| hover_bg_color | bg_list_item_hovered | string | 悬停行背景色（留空则不绘制） |
| selected_bg_color | bg_list_item_selected | string | 选中行背景色（留空则不绘制） |
| divider_color | border_split_level1 | string | 列分隔线颜色 |
| chevron_color | color_gray | string | 有子项的节点右侧箭头颜色 |
| scrollbar_color | color_gray | string | 列内容溢出时右缘滚动条颜色 |
| font | system_regular_14 | string | 文本字体 ID |

XML 示例（`class` 属性必须写在所有属性最前面）：

```xml
<!-- 基础用法：三级级联 + 多语言路径（相同前缀自动合并） -->
<CascadingSelect class="cascading_select" name="cascading_basic" width="stretch" height="174"
                 items_id="STRID_CS_EAST/STRID_CS_SHANGHAI/STRID_CS_PUDONG|STRID_CS_EAST/STRID_CS_SHANGHAI/STRID_CS_XUHUI|STRID_CS_SOUTH/STRID_CS_SHENZHEN/STRID_CS_NANSHAN"/>

<!-- 自定义路径分隔符 + 固定列宽 + 禁用路径（含 ID 路径） -->
<CascadingSelect class="cascading_select" path_separator="&gt;" column_width="150"
                 disabled_paths="STRID_CS_SYSTEM&gt;STRID_CS_NETWORK&gt;STRID_CS_DNS"
                 items_id="STRID_CS_SYSTEM&gt;STRID_CS_NETWORK&gt;STRID_CS_PROXY|STRID_CS_SYSTEM&gt;STRID_CS_DISPLAY&gt;STRID_CS_THEME"/>
```

对应的 C++ 接口：

```cpp
void SetItems(const std::vector<DString>& paths);       // 字面路径，替换全部并清空选择
void SetItemIds(const std::vector<DString>& pathIds);   // 每段为语言 ID
void AddPath(const DString& path);                      void ClearItems();
size_t GetDepth() const;                                // 列数
size_t GetItemCount(size_t level) const;                // 该列当前可见条目数
bool GetItemText(size_t level, size_t index, DString& text) const;
const std::vector<int32_t>& GetSelection() const;       // 每级索引，-1 = 未选
bool SetSelection(const std::vector<size_t>& indexes, bool bTriggerEvent = true);
bool SetSelectionByText(const DString& strText, bool bTriggerEvent = true);  // 按 value_separator 文本匹配
void ClearSelection(bool bTriggerEvent = true);
DString GetSelectionText() const;                       // value_separator 连接
DString GetSelectionPath() const;                       // path_separator 连接
void SetDisabledPaths(const std::vector<DString>& paths);  bool IsItemDisabled(size_t level, size_t index) const;
void SetPathSeparator(const DString& strSeparator);     void SetValueSeparator(const DString& strSeparator);
void SetColumnWidth(int32_t nWidth);                    void SetRowHeight(int32_t nHeight);
void SetTextPadding(int32_t nPadding);                  void SetFont(const DString& strFontId);
//颜色：SetItemColor / SetSelectedColor / SetDisabledColor / SetDividerColor / SetChevronColor / SetScrollbarColor
//背景：SetHoverBgColor / SetSelectedBgColor（留空则不绘制）
void AttachSelectionChanged(const EventCallback& callback, EventCallbackID callbackID = 0);
```

选择变化事件：`AttachSelectionChanged`——选择实际变化时触发一次，事件编码为 `kEventValueChanged`，`wParam` 为 4（级联选择模式标记），新选择通过 `EventArgs::pEventData` 携带（指向 `CascadingSelectValue`，含 `indexes` / `text` / `path`；仅在本次回调派发期间有效，请勿保存该指针）。`SetSelection(..., false)` / `ClearSelection(false)` 不派发事件。

行为说明：
- 列数等于数据最大深度；第 `i` 列的条目由前 `i` 级选择决定，前缀未选满时该列留空。
- 路径列表按首次出现顺序合并相同前缀；选择只影响显示，不会修改数据顺序。
- 点击某列某行后，选择被截断到该层（深层旧选择立即重置），子列在同一帧内切换内容，随后派发一次事件（值未变化不派发）。
- 禁用优先级最高：`disabled_paths` 命中节点后，该节点及其整棵子树置灰且不可点击；若此前已选中被禁用节点，选择会自动截断到其上层并派发一次事件。
- 列内容超出可见行数时，鼠标在该列上滚轮可滚动（每格 3 行），右缘绘制按比例缩放的滚动条。
- 文本按 `TEXT_END_ELLIPSIS` 截断；有子项的节点在行右侧绘制矢量箭头，不依赖字体字形。
- 语言切换只重建显示文本，树形与选择索引保持不变；`items`（字面路径）不参与多语言刷新。
- 本控件暂不支持键盘导航；需要键盘操作时由业务侧转发方向键 / 回车到 `SetSelection`。
- 默认皮肤在 global.xml 中定义 `cascading_select` Class（仅颜色、字体与尺寸，无图片资源），自动适配深浅色主题。

## TreeGrid 控件（树表格）

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| column_title |  | string | 列标题列表，`\|` 分隔（每项可为多语言 ID 或字面文本） |
| column_width |  | string | 列宽列表，`\|` 分隔（原始像素值，自动 DPI 缩放） |
| column_id |  | string | 列 ID 列表，`\|` 分隔（缺省按列索引从 0 起） |
| column_align |  | string | 列对齐列表，`\|` 分隔：left / center / right |
| default_column_width | 120 | int | 未显式指定列宽时使用的默认列宽（原始像素） |
| tree_column | 0 | int | 承载缩进与展开开关的树列索引 |
| indent | 20 | int | 每一层级的缩进量（原始像素，自动 DPI 缩放） |
| header_height | 30 | int | 表头高度（原始像素，自动 DPI 缩放） |
| row_height | 28 | int | 数据行高（原始像素，自动 DPI 缩放） |
| show_header | true | bool | 是否显示表头 |
| multi_select | false | bool | 是否允许选择多行 |
| check_mode | false | bool | 是否在行首显示复选框 |
| check_cascade | true | bool | 父子复选框是否级联（关闭时仅改自身） |
| expand_image_class | tree_grid_expand | string | 展开 / 收起开关引用的皮肤 Class（两态图片） |
| check_image_class | tree_grid_check | string | 行复选框引用的皮肤 Class（三态图片） |
| header_cell_class | tree_grid_header_cell | string | 表头标题单元格引用的皮肤 Class |
| cell_class | tree_grid_cell | string | 数据单元格引用的皮肤 Class |
| row_class | tree_grid_row | string | 数据行引用的皮肤 Class |

TreeGrid 控件继承了 `VBox` 的全部布局属性；表头固定不滚动，列表体自带纵向滚动条。

树表格在同一控件内呈现"可展开 / 收起的层级行 + 多列表格"，用于文件树、报表、组织架构等场景：

- 控件内部由"固定不滚动的表头 + 自带滚动条的列表体"组成；表头与数据行复用同一套列宽数组，天然逐列对齐，末列右侧预留滚动条宽度避免重叠；
- 展开 / 收起通过切换子孙行的可见性实现（隐藏的行不占空间），因此无需重建行；
- 缩进使用"树列单元格内嵌缩进占位"实现，而非给整行加 padding，故其它列不会随层级错位；
- 列标题支持多语言 ID：优先按语言管理器解析，未命中则按字面文本显示。

XML 示例：
```xml
<TreeGrid class="tree_grid" name="tree_grid_demo" width="stretch" height="230"
          column_title="名称|大小|类型" column_width="260|120|140" column_align="left|right|left"
          tree_column="0" indent="18" header_height="30" row_height="26"
          multi_select="true" check_mode="true" check_cascade="true"/>
```

对应的 C++ 接口：
```cpp
//列管理
void AddColumn(const TreeGridColumn& column);            //追加一列（重建表头并刷新所有行）
size_t GetColumnCount() const;
bool GetColumn(size_t nColumnIndex, TreeGridColumn& column) const;
bool SetColumnWidthById(size_t nColumnId, int32_t nWidth);
int32_t GetColumnWidthById(size_t nColumnId) const;
bool SetColumnTitleById(size_t nColumnId, const DString& title);
void ClearColumns();

//节点与数据
TreeGridNode* GetRootNode() const;                       //虚拟根（不可见）
TreeGridNode* AddNode(TreeGridNode* pParent);            //在父节点下新建并返回节点
bool RemoveNode(TreeGridNode* pNode);
void RemoveAllNodes();
bool SetCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text);
DString GetCellText(TreeGridNode* pNode, size_t nColumnId) const;
bool SetNodeRowHeight(TreeGridNode* pNode, int32_t nHeight);  //0 = 使用全局 row_height

//展开 / 收起
bool SetNodeExpand(TreeGridNode* pNode, bool bExpand, bool bTriggerEvent = false);
bool IsNodeExpand(TreeGridNode* pNode) const;
bool ExpandNode(TreeGridNode* pNode);                    //级联展开祖先 + 自身
bool EnsureNodeVisible(TreeGridNode* pNode);             //展开祖先并滚动到可见

//选择
void SetMultiSelect(bool bMultiSelect);   bool IsMultiSelect() const;
size_t GetSelectNodeCount() const;        TreeGridNode* GetSelectNodeAt(size_t nIndex) const;
std::vector<TreeGridNode*> GetSelectNodes() const;
bool IsNodeSelected(TreeGridNode* pNode) const;
bool SelectNode(TreeGridNode* pNode, bool bSelect = true, bool bTriggerEvent = true);
void SelectAllNodes(bool bSelect = true); void ClearSelectNodes();

//复选框（级联三态）
bool IsCheckMode() const;      void SetCheckMode(bool bCheckMode);
bool IsCheckCascade() const;   void SetCheckCascade(bool bCascade);
bool IsNodeChecked(TreeGridNode* pNode) const;
bool SetNodeChecked(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent = true);
TreeGridCheck GetNodeCheckState(TreeGridNode* pNode) const;  //UnCheck / CheckedAll / CheckedPartly
void SetAllNodesChecked(bool bChecked);
std::vector<TreeGridNode*> GetCheckedNodes() const;
```

`TreeGridNode`（数据节点，不继承 `Control`）常用接口：
```cpp
TreeGridNode* AddChildNode(TreeGridNode* pNode);   bool RemoveChildNode(TreeGridNode* pNode);
void RemoveAllChildNodes();
size_t GetChildNodeCount() const;                  TreeGridNode* GetChildNode(size_t iIndex) const;
size_t GetChildNodeIndex(TreeGridNode* pNode) const;
size_t GetDescendantNodeCount() const;             bool IsChildOf(TreeGridNode* pNode) const;
TreeGridNode* GetParentNode() const;               size_t GetDepth() const;
bool IsExpand() const;   bool SetExpand(bool bExpand);
bool IsChecked() const;  bool SetChecked(bool bChecked);   TreeGridCheck GetCheckState() const;
bool IsSelected() const; bool SetSelected(bool bSelected);
bool SetCellText(size_t nColumnId, const DString& text);   DString GetCellText(size_t nColumnId) const;
void SetUserData(void* pData);                     void* GetUserData() const;
```

交互与事件：

- 点击展开开关 → 切换展开并派发 `kEventExpand` / `kEventCollapse`；
- 点击行复选框 → 级联勾选（不改变行选择）；`check_cascade` 打开时向下级联子孙、向上回溯祖先；
- 点击行 → 选择该行并派发 `kEventClick`；
- 键盘 `Left` / `Right` → 收起 / 展开当前选中节点（方向键上下沿用 `ListBox` 原生导航）。

```cpp
void AttachNodeExpanded(const EventCallback& callback, EventCallbackID callbackID = 0);  //kEventExpand
void AttachNodeCollapsed(const EventCallback& callback, EventCallbackID callbackID = 0); //kEventCollapse
void AttachNodeClick(const EventCallback& callback, EventCallbackID callbackID = 0);     //kEventClick
void AttachNodeSelect(const EventCallback& callback, EventCallbackID callbackID = 0);   //kEventSelect
void AttachNodeUnSelect(const EventCallback& callback, EventCallbackID callbackID = 0); //kEventUnSelect
void AttachSelectChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //kEventSelChanged
void AttachCheckChanged(const EventCallback& callback, EventCallbackID callbackID = 0); //kEventTreeGridCheckChanged
```

上述前 6 个事件的 `EventArgs::pEventData` 均为 `TreeGridNode*`（发生事件的数据节点）；勾选变化事件携带的是 `TreeGridCheckValue`（含 `pNode` 与 `bChecked`，仅在本次回调派发期间有效，请勿保存该指针）：

```cpp
pTreeGrid->AttachCheckChanged([](const ui::EventArgs& msg) {
    if (msg.pEventData != nullptr) {
        auto* pValue = static_cast<const ui::TreeGridCheckValue*>(msg.pEventData);
        // 读取 pValue->pNode / pValue->bChecked
    }
    return true;
});
```

默认皮肤的 `tree_grid` 类及其子类（`tree_grid_header_cell` / `tree_grid_cell` / `tree_grid_row` / `tree_grid_expand` / `tree_grid_check`）均走语义色；展开箭头与三态复选框复用 `${PATH_TREE}` / `${PATH_CHECKBOX}` 下的既有 SVG 图片。

## CircleProgress的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| circular | true | bool | 功能开关：是否为环形进度条 |
| circle_width | 1 | int | 环形进度条的宽度，如"10" |
| indicator |  | string | 设置进度指示移动图标 |
| clockwise | true | bool |设置递增方向 |
| bgcolor |  | string | 设置进度条背景颜色 |
| fgcolor |  | string | 设置进度条前景色 |
| gradient_color |  | string | 设置进度条前景渐变颜色，与 fgcolor 同时使用，可以不设置则无渐变效果 |
| indeterminate | false | bool | 是否不确定进度（忽略 value，一段弧持续旋转） |
| start_angle | 270 | float | 圆弧起始角度（度），0 为12 点方向，顺时针递增；360 会被归一化为 0 |
| span_angle | 360 | float | 圆弧总跨度（度），取值范围 (0, 360]；小于 360 即为缺口环/仪表盘 |
| indeterminate_arc | 90 | float | 不确定进度模式下旋转弧的弧长（度） |
| indeterminate_speed | 6 | float | 不确定进度模式下旋转角速度（度/次刷新） |
| indeterminate_interval | 16 | int | 不确定进度模式的刷新间隔（毫秒），默认约 60 帧/秒 |
| center_ratio | 100 | float | 中心内容区占控件的百分比（0~100），如"70"表示中心区为控件的 70% |
| center_padding | 0,0,0,0 | rect | 中心内容区内边距，如"4,4,4,4"（XML 中按 DPI 缩放） |

CircleProgress 控件继承了 `Progress` 属性，更多可用属性请参考`Progress`的属性

### 仪表盘与缺口环（start_angle / span_angle）

原本圆环固定从 270° 起、扫满 360°，无法表达"半圆表盘""带缺口的环"。现在起点与跨度均可配置：

```xml
<!-- 180° 半圆速度表：0~240 km/h -->
<CircleProgress name="gauge_half" circular="true" width="170" height="110" valign="bottom"
                circle_width="10" start_angle="270" span_angle="180"
                min="0" max="240" value="120" show_percent="true"/>

<!-- 270° 缺口环：缺口 90°，用于评分/完成度 -->
<CircleProgress name="gauge_gap" circular="true" width="140" height="140"
                circle_width="12" start_angle="135" span_angle="270"
                gradient_color="color_accent" min="0" max="100" value="65"/>
```

行为要点：
- **前景弧**按 `(value-min)/(max-min) * span_angle` 扫过，**背景弧**始终绘制完整的 `span_angle`，因此缺口部分固定呈轨道色
- **圆心自动按弧的包围盒居中**：非整圆时不是简单取控件中心，而是采样起止角与途经的 0/90/180/270 象限极值点求包围盒，再把圆心放在该包围盒中心。这样 180° 半圆表盘的上半弧能完整落在控件内，不会被裁掉
- 半径取各方向所需半径的最大值，保证弧既不溢出控件也不过度缩小
- `start_angle`/`span_angle` 对背景弧、前景弧、指示点（indicator）同时生效，指示点跟随进度末端
- `clockwise="false"` 时扫过方向反转（起点与跨度不变）
- 对应C++ 接口：`SetStartAngle/GetStartAngle`、`SetSpanAngle/GetSpanAngle`

### 中心自定义内容（绘制回调）

圆环中心原本只能放文本（继承自 Label）。仪表盘场景常需要在中心放指针、图标或组合数字，可用 `SetCenterCallback` 接管中心绘制：

```cpp
pGauge->SetCenterCallback([pGauge](ui::IRender* pRender, const ui::UiRect& rcCenter, double fValue) {
    if (pRender == nullptr || rcCenter.IsEmpty()) {
        return true;
    }
    const int nWidth = rcCenter.right - rcCenter.left;
    const int nHeight = rcCenter.bottom - rcCenter.top;
    const float fCx = rcCenter.left + nWidth / 2.0f;
    const float fCy = rcCenter.top + nHeight / 2.0f;
    const float fLen = (nWidth < nHeight ? nWidth : nHeight) * 0.42f;
    // 起始 135°、跨度 270°，顺时针；按 fValue 换算指针角度
    const float fRad = (135.0f + 270.0f * (float)(fValue / 100.0)) * 3.14159265f / 180.0f;
    const ui::UiPointF ptEnd(fCx + fLen * std::cos(fRad), fCy + fLen * std::sin(fRad));
    pRender->DrawLine({fCx, fCy}, ptEnd, pGauge->GetUiColor(_T("bg_progress")), 2.0f);
    pRender->FillCircle({fCx, fCy}, 3.0f, pGauge->GetUiColor(_T("text_default")));
    return true;
    });
```

- 回调签名：`bool(IRender* pRender, const UiRect& rcCenter, double fValue)`，返回 `true` 表示绘制完成
- `rcCenter` 是**控件内相对坐标**的矩形，由 `center_ratio`（按比例收缩）与 `center_padding`（内边距）共同决定，可直接交给 `IRender` 使用
- `fValue` 为钳位到 `[min, max]`、并已应用 `reverse` 的显示值
- **不设置回调时行为不变**：中心文本仍由基类的 `text`/`text_padding`/`show_percent` 流程绘制，不会出现文本与自定义内容重复叠加
- 若需要在中心放真实子控件（按钮、图片等），可在控件上叠加一个居中的子控件实现，本回调仅负责自绘内容
- 对应 C++ 接口：`SetCenterCallback`、`GetCenterRect()`、`SetCenterRatio/GetCenterRatio`、`SetCenterPadding/GetCenterPadding`

### indeterminate 参数化

不确定进度模式原本固定弧长 90°、转速 6°/16ms，现在可调：

```xml
<CircleProgress name="loading" circular="true" indeterminate="true" width="140" height="140"
                start_angle="135" span_angle="270"
                indeterminate_arc="240" indeterminate_speed="3" indeterminate_interval="24"/>
```

| 属性 | 默认值 | 效果 |
| :--- | :--- | :--- |
| `indeterminate_arc` | 90 | 旋转弧的弧长。弧长小于 `span_angle` 时可看到明显的缺口拖尾 |
| `indeterminate_speed` | 6 | 每次刷新前进的角度（度）。越小越慢 |
| `indeterminate_interval` | 16 | 刷新间隔（毫秒）。默认 16ms 约 60 帧/秒；放大到 24~33ms 可获得更"卡顿"的机械感 |

三者在运行期修改会自动重启定时器，**无需重新设置 `indeterminate`**。`span_angle` 同样作用于旋转弧的绘制范围。
对应 C++ 接口：`SetIndeterminateArc/GetIndeterminateArc`、`SetIndeterminateSpeed/GetIndeterminateSpeed`、`SetIndeterminateInterval/GetIndeterminateInterval`。

## RichEdit/RichEdit2的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vscrollbar | false | bool | 是否使用竖向滚动条,如"true"|
| hscrollbar | false | bool | 是否使用横向滚动条,如"true"|
| auto_vscroll | false | bool | 是否随输入竖向滚动,如"true"(当为SDL实现时，该选项无效)|
| auto_hscroll | false | bool | 是否随输入横向滚动,如"true" (当为SDL实现时，该选项无效)|
| want_tab | false | bool | 是否接受tab按键消息,如"true" |
| want_return | false | bool | 是否接受回车按键消息,如"true" |
| want_ctrl_return | false | bool | 是否接受ctrl+return按键消息,如"true"|
| rich_text | false | bool | 是否使用富格式,如"true"(当为SDL实现时，该选项无效)|
| single_line | true | bool | 是否使用单行,如"true"|
| multi_line | false | bool | 是否使用多行,该属性与single_line互斥,如"true"|
| readonly | false | bool | 是否只读,如"false" |
| password | false | bool | 是否为密码模式,如"true"|
| show_password | false | bool | 是否显示密码符,如"true"|
| password_char || string | 设置密码字符，默认为 " * " 字符，可用通过这个属性改变|
| flash_password_char | false | bool | 先显示字符，然后再显示密码字符|
| number_only | false | bool | 是否只允许输入数字,如"false" |
| max_number | INT64_MAX | int64 | 允许的最大数字(仅当number_only为true的时候有效) |
| min_number | INT64_MIN | int64 | 允许的最小数字(仅当number_only为true的时候有效) |
| text_align | "left,top" | string | 文字的水平与垂直对齐方式, 可取值: left、right、hcenter、top、vcenter、bottom，用逗号分割，如"hcenter,vcenter" |
| text_padding |  | rect | 文本内边距，如："2,2,2,2" |
| text |  | string | 显示文本 |
| text_id |  | string | 显示文本的多语言功能ID |
| font | | string | 字体ID |
| normal_text_color |  | string | 普通状态文字颜色,不指定则使用默认颜色,如"blue" |
| disabled_text_color |  | string | 禁用状态文字颜色,不指定则使用默认颜色,如"blue" |
| caret_color |  | string | 光标的颜色 |
| prompt_mode | false | bool | 是否显示提示文字,如"true"|
| prompt_text |  | string | 文本框内提示文字,当文本框text为空时显示 |
| prompt_text_id |  | string | 多语言功能的ID,如"TEXT_OUT" |
| prompt_color |  | string | 文本框内提示文字的颜色 |
| focused_image |  | string | 焦点状态下的图片 |
| auto_detect_url | false | bool | 是否自动检测URL，如果是URL则显示为超链接 (当为SDL实现时，该选项无效)|
| limit_text | | int | 限制最多字符数 |
| limit_chars | | string | 限制允许输入哪些字符，比如"abc"表示只允许输入a、b、c字符，不允许输入其他字符 |
| allow_beep | false | bool | 是否允许发出Beep声音 (当为SDL实现时，该选项无效)|
| word_wrap | false| bool | 是否自动换行 |
| no_caret_readonly |false| bool | 只读模式，不显示光标 |
| save_selection |false| bool | 如果 为 true，则当控件处于非活动状态时，应保存所选内容的边界 (当为SDL实现时，该选项无效)|
| hide_selection | true | bool | 是否隐藏选择内容 |
| zoom | | size | 设置缩放比例：设 wParam：缩放比例的分子，lParam：缩放比例的分母。"wParam,lParam" 表示按缩放比例分子/分母显示的缩放，取值范围：1/64 < (wParam / lParam) < 64。举例：则："0,0"表示关闭缩放功能，"2,1"表示放大到200%，"1,2"表示缩小到50% |
| wheel_zoom | | bool | 是否允许Ctrl + 滚轮来调整缩放比例 |
| default_context_menu | false | bool | 是否使用默认的右键菜单 |
| spin_class | | string | 设置Spin功能的Class名称，如果不为空则显示Spin按钮，详细用法参见示例程序|
| clear_btn_class | | string | 设置清除按钮功能的Class名称，如果不为空则显示清楚按钮，详细用法参见示例程序 |
| show_password_btn_class | | string |设置显示密码按钮功能的Class名称，如果不为空则显示显示密码按钮 ，详细用法参见示例程序 |
| selection_bkcolor | "CornflowerBlue" | string |选择文本的背景色（焦点状态） 如果设置为空，则不显示|
| inactive_selection_bkcolor | "DarkGray" | string | 选择文本的背景色（非焦点状态），如果设置为空，则不显示 |
| current_row_bkcolor | "" | string | 当前行的背景色（焦点状态），如果设置为空，则在焦点状态不显示当前行的背景色|
| inactive_current_row_bkcolor | "" | string |当前行的背景色（非焦点状态），如果设置为空，则在非焦点状态不显示当前行的背景色  |
| select_all_on_focused |false| bool | 获取焦点的时候，是否全选 |
| focused_bottom_border_size |0| int | 焦点状态时，底部边框的大小 |
| focused_bottom_border_color || string | 焦点状态时，底部边框的颜色 |
| enable_drag_drop |false| bool   | 是否允许拖放操作|
| enable_drop_file |false| bool   | 是否允许拖放文件操作|
| enable_drag_out  |true | bool   | 是否允许文本拖出功能（作为拖放源），仅Windows平台支持，其他平台不支持，该选项仅在RichEdit2中支持|
| drop_file_types  |     | string | 拖放文件操作支持的后缀名列表，比如:".txt;.csv"，表示仅支持txt和csv文件；如果为空，表示支持所有文件|
| row_spacing_mul  | 1.0 | float  | 行间距倍数, 比如1.5代表1.5倍行间距<br>Windows平台：仅当rich_text属性"true"时有效，因为Windows平台的RichEdit控件只有富文本模式时支持设置行间距；<br>使用SDL时，始终有效，即其他平台时，行间距属性均有效|
| row_spacing_add  |0    | float  | 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素），仅当使用SDL时有效|

RichEdit 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性    
RichEdit2类的功能说明：    
（1）在Windows平台，RichEdit类是使用Windows系统本身的ITextServices接口实现的，RichEdit2是本项目自己实现的，两者实现方式不同，但功能基本一致    
（2）在非Windows平台，RichEdit是RichEdit2的别名，两者没有区别。    

## SpinBox的属性
SpinBox是数字输入框控件，继承自RichEdit，默认开启`number_only`模式，提供数字输入 + 步进按钮 + 上下方向键调整的完整数字输入能力。    
当设置小数（`decimals` 大于 0）或前后缀/千分位时，控件自动切换为格式化输入模式：允许输入数字、正负号、小数点、千分位分隔符以及前后缀中的字符；失焦时按当前格式重新生成文本，无法解析的输入恢复为最近一次有效值。    
除RichEdit的通用属性（`spin_class`、`limit_text`等）外，新增以下属性：    

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| decimals | 0 | int(0~10) | 小数位数；0 时与普通整数 SpinBox 行为一致 |
| step | 1 | double | 步长值（必须大于 0），步进按钮和上下方向键每次调整的数值 |
| value | 0 | double | 初始数值（超出min/max范围时会被修正到边界值，并按 decimals 四舍五入） |
| min_number / max_number | 无限制 | double | 数值范围（别名 `min_value` / `max_value`） |
| prefix | "" | string | 显示前缀（如 `"$ "`） |
| suffix | "" | string | 显示后缀（如 `" kg"`） |
| group_separator | "" | string | 千分位分隔符（如 `","`），空表示不分组；别名 `thousand_separator` |

对应C++接口：`SetStep/GetStep/GetStepDouble`、`SetValue/GetValue/GetValueDouble`、`SetRange(min, max)`、`SetMinValue/GetMinValue`、`SetMaxValue/GetMaxValue`、`SetDecimals/GetDecimals`、`SetPrefix/GetPrefix`、`SetSuffix/GetSuffix`、`SetGroupSeparator/GetGroupSeparator`。    
数值变化时触发`kEventTextChanged`事件（与RichEdit一致）。    
使用示例（参考 rich_edit 示例程序）：    

```xml
<SpinBox class="simple simple_border rich_edit_spin" min_number="0" max_number="100" step="5" value="50"/>
<!-- 小数 + 千分位 + 前后缀：显示为 "$ 1,234.50 kg" -->
<SpinBox class="simple simple_border rich_edit_spin" min_number="-10000" max_number="10000"
         step="0.25" decimals="2" value="1234.5" group_separator=","
         prefix="$ " suffix=" kg"/>
```

其中`spin_class`样式（rich_edit_spin_box / rich_edit_spin_btn_up / rich_edit_spin_btn_down）在 global.xml 的`rich_edit_spin`类中定义。    
注意：需设置 `spin_class` 属性才会显示步进按钮；否则只有上下方向键能调整数值。若自行设置了 `limit_chars`，控件不再自动生成输入字符集。    

## SearchBox的属性
SearchBox是搜索框组合控件，继承自HBox，内部组合了左侧搜索图标、文本编辑框（RichEdit）与右侧清除按钮，用于关键词输入与搜索场景。对应头文件`duilib/Control/SearchBox.h`。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text | "" | string | 搜索框初始文本 |
| text_id | "" | string | 搜索框初始文本的多语言字符串ID（与text同时设置时，后设置的属性生效） |
| prompt_text | "" | string | 编辑框为空时显示的占位提示文本 |
| prompt_text_id | "" | string | 占位提示文本的多语言字符串ID |
| prompt_color | "" | string | 占位提示文本的颜色（语义色名） |
| debounce_interval | 0 | int(ms) | 防抖间隔（毫秒）：搜索文本连续变化时，仅在输入停顿达到该间隔后才触发 `kEventTextChanged`；0=每次变化立即触发（默认，向后兼容），用于避免搜索场景每次按键都触发搜索请求 |
| suggestions | "" | string | 建议数据源，多项以分号";"分隔（搜索历史由控件在提交搜索时自动记录，无需在此配置） |
| max_visible | 8 | int | 下拉浮层最多可见候选行数（超出显示滚动条） |
| history_enabled | true | bool | 是否记录并展示搜索历史（历史同时作为候选项展示） |
| max_history | 10 | int | 搜索历史最多保留条数（超过后丢弃最旧，最近使用的置顶） |
| filter_enabled | true | bool | 是否按输入文本过滤候选：true（默认）时，输入达到 `min_chars` 才按 `match_mode`/`ignore_case` 过滤候选项；false 时始终全量展示（不过滤），适合候选项较少或希望总览全部候选项的场景 |
| min_chars | 1 | int | 过滤模式下触发过滤的最少输入字符数；输入为空或不足该值时展示全部（聚焦态） |
| match_mode | prefix | string | 过滤匹配模式：`prefix`前缀匹配（候选项以输入文本开头）；`contains`包含匹配（任意位置包含） |
| ignore_case | true | bool | 过滤时是否忽略大小写 |

对应C++接口：`SetSearchText/GetSearchText`、`GetEditControl()`（获取内部RichEdit控件）、`SetDebounceInterval/GetDebounceInterval`（防抖间隔，单位毫秒）。
联想（建议+历史）相关接口：`SetCandidates/GetCandidates/AddCandidate/ClearCandidates`（建议数据源）、`SetMaxVisible/GetMaxVisible`、`SetHistoryEnabled/IsHistoryEnabled`、`SetMaxHistory/GetMaxHistory`、`GetHistory/ClearHistory/AddSearchHistory`（搜索历史）、`SetFilterEnabled/IsFilterEnabled`、`SetMinChars/GetMinChars`、`SetMatchMode/GetMatchMode`、`SetIgnoreCase/IsIgnoreCase`（过滤参数）。
文本变化时触发`kEventTextChanged`事件（可通过 `debounce_interval` 设置防抖间隔，仅在输入停顿后才触发，避免每次按键都发起搜索）；**回车语义**：若用户未用上下方向键导航候选（即未高亮任何候选项），回车提交“输入框当前文本”并触发`kEventReturn`事件（可用于发起搜索）；若用户已用上下方向键导航并高亮某候选项，回车选中该候选项（回填编辑框）并触发`kEventAutoCompleteSelect`事件，其 WPARAM 为候选项在当前候选列表中的序号，此时`GetSearchText()`已被回填为候选项文本。
下拉联想浮层复用 `AutoComplete` 的皮肤类 `auto_complete_flyout`/`auto_complete_list`/`auto_complete_item`（定义在 global.xml），跟随搜索框宽度在下方弹出，不抢焦点。

联想浮层展示规则（明确且一致）：
- **聚焦即评估**：编辑框获得焦点时立即评估，有可展示内容则展开，否则不弹。
- **输入实时刷新**：文本每次变化实时刷新；无可展示内容时浮层关闭。
- **过滤开关 `filter_enabled`（默认 true）**：开启时，输入达到 `min_chars` 才按 `match_mode`/`ignore_case` 过滤候选项；输入为空或不足 `min_chars` 时展示全部（历史置顶、最近使用的在前）。关闭时始终全量展示（历史与建议合并去重，不做输入过滤），适合候选项较少的场景。
- **浮层不抢焦点**：编辑框失焦不作为关闭条件；点击浮层与搜索框以外区域时浮层自动关闭。
- **选中后焦点归还**：鼠标点击候选项时，框架会先把原生键盘焦点移到浮层窗口，浮层关闭前会统一交还搜索框所在窗口，保证选中后可以继续用键盘输入（按 Esc 收起浮层时同样归还焦点，且不会重新弹出）。
- **选中不重开**：选中候选项回填文本后不会因重新聚焦而再次弹出（避免刚选中又抖出浮层）。
- **回车不误选**：未导航候选时回车提交输入文本，不会把列表里默认高亮的首项误当作搜索词。
- 重复提交同一搜索词不会在列表中产生重复项（去重）。
清除按钮复用RichEdit内置的`clear_btn_class`机制：编辑框聚焦且有文本时显示，点击清空文本，失焦后自动隐藏。
使用示例（参考 rich_edit 示例程序）：

```xml
<SearchBox class="search_box" prompt_text="输入关键词搜索"/>
```

默认皮肤在global.xml中定义了`search_box`（外框）、`search_box_icon`（放大镜图标）、`search_box_edit`（内部编辑框）、`search_box_clear_btn`（清除按钮）4个Class。

## TagInput的属性
TagInput是标签输入框组合控件，继承自Box，内部使用水平流式布局组合了若干标签（HBox+文本Label+关闭按钮）和一个文本编辑框（RichEdit）。输入文本后按回车生成标签，点击标签右侧X按钮或在空编辑框按Backspace删除标签，标签过多时自动换行、控件高度自动增长。常用于收件人、标签选择、关键词录入等场景。对应头文件`duilib/Control/TagInput.h`。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| prompt_text | "" | string | 编辑框为空时显示的占位提示文本 |
| prompt_text_id | "" | string | 占位提示文本的多语言字符串ID |
| prompt_color | "" | string | 占位提示文本的颜色（语义色名） |
| allow_duplicate | false | bool | 是否允许重复标签（false时重复文本的标签不会被添加） |
| max_count | 0 | int | 最多标签个数，0表示不限制 |
| delimiter | "" | string | 输入过程中直接生成标签的分隔符（如英文逗号`,`）；为空表示仅按回车键生成 |
| tags | "" | string | 初始标签文本，多个标签以分号`;`分隔 |

对应C++接口：`AddTag`（新增标签，文本为空/重复/超限时返回false）、`RemoveTagAt`、`RemoveAllTags`、`GetTagCount`、`GetTagText`、`SetAllowDuplicate/IsAllowDuplicate`、`SetMaxCount/GetMaxCount`、`GetEditControl`（获取内部RichEdit）、`SetFocus`（焦点设置到内部编辑框）。

事件：
- `kEventTagAdded`：新增标签后触发，WPARAM为新标签序号（从0开始）；
- `kEventTagRemoved`：标签被删除前触发，WPARAM为被删除标签序号，回调中仍可通过`GetTagText`读取其文本；
- `kEventTagClick`：点击标签正文区域触发，WPARAM为标签序号（点关闭按钮不触发）；
- `kEventTextChanged`：内部编辑框文本变化时转发触发。

使用示例（参考 controls 示例程序）：

```xml
<!-- 回车成标签，带3个初始标签 -->
<TagInput class="tag_input" tags="nim_duilib;C++;桌面UI" prompt_text="输入后按回车生成标签"/>

<!-- 逗号或回车均可成标签，最多5个且允许重复 -->
<TagInput class="tag_input" delimiter="," max_count="5" allow_duplicate="true" prompt_text="回车或英文逗号生成标签"/>
```

默认皮肤在global.xml中定义了`tag_input`（外框容器）、`tag_input_tag`（标签背景）、`tag_input_tag_text`（标签文本）、`tag_input_tag_close`（标签关闭按钮）、`tag_input_edit`（内部编辑框）5个Class，标签底色使用语义色`bg_tag`/`bg_tag_hover`，关闭按钮悬浮/按下时显示`bg_tag_close_hover`/`bg_tag_close_pressed`圆形底色与白色叉形图标，自动适配深浅色主题。

## AutoComplete的属性
AutoComplete是自动完成输入框组合控件，继承自Box，内部包含一个占满整个控件的单行编辑框（RichEdit）。输入达到`min_chars`个字符后，在编辑框下方弹出不抢焦点的候选浮层（`AutoCompleteFlyout`，复用Flyout），浮层宽度与编辑框一致、不占用布局空间，候选项数量变化时浮层高度自适应。键盘上下方向键移动候选项、回车确认、Esc关闭浮层，鼠标悬停高亮、点击直接确认；确认后用候选项文本回填编辑框（程序化回填不再触发联想）。与FilterCombo的区别：AutoComplete以编辑框为主体、边输入边实时过滤、浮层跟随编辑框；FilterCombo以下拉框为主体、点击按钮展开固定选项列表。常用于搜索历史、城市/地址补全等场景。对应头文件`duilib/Control/AutoComplete.h`。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| items | "" | string | 初始候选数据源，多个候选项以分号`;`分隔（也可通过代码`SetCandidates`设置） |
| prompt_text | "" | string | 编辑框为空时显示的占位提示文本 |
| prompt_text_id | "" | string | 占位提示文本的多语言字符串ID |
| prompt_color | "" | string | 占位提示文本的颜色（语义色名） |
| min_chars | 1 | int | 输入达到多少个字符后开始联想 |
| max_visible | 8 | int | 浮层最多可见候选行数，超出显示滚动条 |
| match_mode | prefix | string | 文本匹配模式：`prefix`前缀匹配（候选项以输入文本开头）；`contains`包含匹配（任意位置包含） |
| ignore_case | true | bool | 匹配时是否忽略大小写 |

对应C++接口：`SetCandidates`/`GetCandidates`（设置/获取完整候选数据源）、`AddCandidate`（追加候选项，相同文本自动去重）、`ClearCandidates`、`SetText`/`GetText`（程序化设置文本不会弹出联想浮层）、`SetMinChars`/`GetMinChars`、`SetMaxVisible`/`GetMaxVisible`、`SetMatchMode`/`GetMatchMode`（参数为`AutoCompleteMatchMode::kPrefix/kContains`）、`SetIgnoreCase`/`IsIgnoreCase`、`GetEditControl`（获取内部RichEdit）、`SetFocus`（焦点设置到内部编辑框）。

事件：
- `kEventAutoCompleteSelect`：选中候选项（鼠标点击或键盘回车）后触发，WPARAM为选中项在当前候选列表中的序号（从0开始），XML内联事件名为`select`；
- `kEventTextChanged`：内部编辑框文本变化时转发触发（程序化回填不触发）。

使用示例（参考 controls 示例程序）：

```xml
<!-- 前缀匹配，18个城市候选，输入 b/sh 等前缀即联想 -->
<AutoComplete class="auto_complete" name="auto_complete"
              items="Beijing;Berlin;Budapest;Bangkok;Boston;Brisbane;Chicago;London"
              prompt_text="输入城市名前缀，如 b / sh"/>

<!-- 包含匹配，最多显示6行；数据源也可完全由代码 SetCandidates 提供 -->
<AutoComplete class="auto_complete" name="auto_complete2" width="260" margin="8,2,0,2"
              match_mode="contains" max_visible="6" prompt_text="输入关键词包含匹配"/>
```

默认皮肤在global.xml中定义了`auto_complete`（外框容器）、`auto_complete_edit`（内部编辑框）2个控件Class，以及浮层使用的`auto_complete_flyout`（浮层卡片）、`auto_complete_list`（候选列表）、`auto_complete_item`（候选项）3个Class；候选项悬浮/选中底色复用列表语义色`bg_list_item_hovered`/`bg_list_item_selected`，浮层卡片使用`bg_window_card`/`border_window`，自动适配深浅色主题。

## Switch的属性
Switch是滑块开关控件，继承自CheckBox，用于开关状态的切换。对应头文件`duilib/Control/Switch.h`。
轨道和滑块既可以用语义色自绘，也可以配置SVG图片皮肤：配置图片后off/on两张图片按动画进度交叉淡入淡出，滑块图片同时沿轨道平移；未配置图片时使用颜色自绘。

除CheckBox的通用属性外，新增以下属性：

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| switch_animation_ms | 160 | int | 切换动画时长（毫秒），0表示不使用动画 |
| track_off_color | bg_switch_track_off | string | 未选中状态的轨道颜色（语义色名，无轨道图片时生效） |
| track_on_color | bg_switch_track_on | string | 选中状态的轨道颜色（语义色名，无轨道图片时生效） |
| thumb_off_color | bg_switch_thumb_off | string | 未选中状态的滑块颜色（语义色名，无滑块图片时生效） |
| thumb_on_color | bg_switch_thumb_on | string | 选中状态的滑块颜色（语义色名，无滑块图片时生效） |
| track_off_image | | string | 未选中状态的轨道图片（图片属性串，与track_on_image成对配置） |
| track_on_image | | string | 选中状态的轨道图片（图片属性串，与track_off_image成对配置） |
| thumb_off_image | | string | 未选中状态的滑块图片（方形图片，与thumb_on_image成对配置） |
| thumb_on_image | | string | 选中状态的滑块图片（方形图片，与thumb_off_image成对配置） |
| thumb_padding | 0 | int | 滑块图片方形框与轨道边缘的距离（自动DPI缩放）。方形框边长等于轨道高度，阴影等留白需绘制在图片内部 |

说明：
- 轨道图片会被拉伸铺满整个轨道矩形，滑块图片在轨道高度的方形框内平移，图片属性串中不要再设置width/height/valign/halign，否则绘制位置将固定不动。
- 图片内的固定颜色可用`svg_replace_colors`替换为语义色（多组替换用分号分隔），以适配深浅色主题。
- 轨道和滑块两层独立：只配置滑块图片时，轨道仍由颜色自绘（iOS风格皮肤即如此）。

对应C++接口：`SetAnimationDuration/GetAnimationDuration`、`SetTrackOffColor/GetTrackOffColor`、`SetTrackOnColor/GetTrackOnColor`、`SetThumbOffColor/GetThumbOffColor`、`SetThumbOnColor/GetThumbOnColor`、`SetTrackOffImage/GetTrackOffImage`、`SetTrackOnImage/GetTrackOnImage`、`SetThumbOffImage/GetThumbOffImage`、`SetThumbOnImage/GetThumbOnImage`、`SetThumbPadding/GetThumbPadding`。
选中状态变化触发`kEventSelect`/`kEventUnSelect`事件（与CheckBox一致）。

### XML使用示例（参考 controls 和 color_theme 示例程序）

```xml
<!-- 使用默认皮肤（global.xml 中定义的 Class） -->
<Switch class="switch" selected="false"/>
<Switch class="switch_fluent" selected="true"/>
<Switch class="switch_material" selected="true"/>

<!-- 纯颜色自绘：不配置图片，指定轨道/滑块颜色（支持语义色名或颜色值） -->
<Switch width="51" height="31" selected="true"
        track_off_color="bg_switch_track_off" track_on_color="bg_switch_track_on"
        thumb_off_color="bg_switch_thumb_off" thumb_on_color="bg_switch_thumb_on"/>

<!-- 自定义图片皮肤：轨道 off/on 两张图铺满轨道 + 滑块 off/on 两张方形图（参考 switch_fluent） -->
<Switch width="48" height="24" padding="2,2,2,2" cursor_type="hand"
        track_off_image="file='public/switch/switch_fluent_track_off.svg' svg_replace_colors='#E9E9EA|bg_switch_track_off;#8A8886|bg_switch_thumb_off'"
        track_on_image="file='public/switch/switch_fluent_track_on.svg' svg_replace_colors='#0078D4|bg_switch_track_on'"
        thumb_off_image="file='public/switch/switch_fluent_thumb_off.svg' svg_replace_colors='#8A8886|bg_switch_thumb_off'"
        thumb_on_image="file='public/switch/switch_fluent_thumb_on.svg'"/>

<!-- XML事件：状态切换时联动其他控件（type 取 select/unselect，与 CheckBox 一致） -->
<Switch class="switch">
    <Event type="select" receiver="target_control" apply_attribute="visible='true'"/>
    <Event type="unselect" receiver="target_control" apply_attribute="visible='false'"/>
</Switch>
```

默认皮肤在global.xml中定义了3套Class（SVG资源在`public/switch/`目录）：
- `switch`（别名`switch_ios`）：iOS风格，51x31，白色大滑块带柔和投影，轨道颜色随主题色；
- `switch_fluent`：Windows 11 Fluent风格，48x24（padding=2，轨道44x20），描边小轨道+14px小滑块；
- `switch_material`：Material Design 3风格，52x32，未选中为描边轨道+16px小滑块，选中为主题色轨道+24px带对勾大滑块。

语义色在color_light和color_dark的global.xml中分别定义了`bg_switch_track_on/off`、`bg_switch_thumb_on/off`，Fluent/Material皮肤中的SVG颜色通过`svg_replace_colors`映射到这些语义色或`color_accent`。

### C++使用示例

```cpp
// 获取控件指针（XML 中 name="switch_test"）
ui::Switch* pSwitch = dynamic_cast<ui::Switch*>(pWindow->FindControl(_T("switch_test")));
if (pSwitch != nullptr) {
    // 监听状态切换事件（与 CheckBox 一致）
    pSwitch->AttachSelect([](const ui::EventArgs& args) {
        // 已切换为选中状态
        return true;
        });
    pSwitch->AttachUnSelect([](const ui::EventArgs& args) {
        // 已切换为未选中状态
        return true;
        });

    // 动态读取/切换选中状态（触发滑动动画与 select/unselect 事件）
    bool bSelected = pSwitch->IsSelected();
    pSwitch->Selected(!bSelected);

    // 动态设置外观（颜色自绘；如需图片皮肤改用 SetTrackXxxImage/SetThumbXxxImage）
    pSwitch->SetTrackOnColor(_T("#FF0078D4"));
    pSwitch->SetAnimationDuration(200);
}
```

## Badge的属性
Badge是角标控件，继承自Label，用于TabCtrl标签页、按钮、图标等控件上的未读数/新消息数量/小红点展示。对应头文件`duilib/Control/Badge.h`。
两种形态：数字角标（圆角胶囊背景+数字，超过上限显示"99+"）和红点角标（dot模式，纯小圆点）。数字角标数量count<=0时自动隐藏（无未读不显示角标）；红点角标不受count影响，显隐由visible属性控制。
Badge继承Label的文本属性（font、normal_text_color、text_padding等），背景为语义色自绘的圆角胶囊（圆角=高度一半），自动适配深浅色主题。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| count | 0 | int64 | 角标数量：数字模式下大于0时显示角标，小于等于0时自动隐藏（红点模式不受影响） |
| max_count | 99 | int64 | 数量上限，超过上限时显示"上限+"（如"99+"） |
| dot | false | bool | 红点模式：true为纯小圆点，不显示数字 |
| badge_color | bg_badge | string | 角标背景颜色（语义色名或颜色值） |

说明：
- Badge作为子控件放在任意Box容器内使用；悬浮定位在宿主控件角上时配合`float="true"`与`margin`属性。
- 数字气泡的宽度随数字位数自适应（`width="auto"`），单个数字时呈圆形；红点尺寸由XML的width/height决定（默认皮肤8x8）。
- 子控件超出父容器边界的部分会被裁剪，悬浮角标需保证父容器足够容纳（参考controls示例：父Box宽度=按钮宽度+角标外露部分）。
- Badge不响应鼠标（`mouse_enabled="false"`），不会挡住宿主控件的点击。

对应C++接口：`SetCount/GetCount`、`SetMaxCount/GetMaxCount`、`SetDotMode/IsDotMode`、`SetBadgeColor/GetBadgeColor`。
使用示例（参考 controls 示例程序）：

```xml
<!-- 默认皮肤：数字角标（count<=0 自动隐藏） -->
<Badge class="badge" count="5"/>
<Badge class="badge" count="120"/>   <!-- 显示"99+" -->

<!-- 红点角标 -->
<Badge class="badge_dot"/>

<!-- 悬浮在按钮右上角：父Box宽度=按钮宽度+角标外露部分，float+margin定位 -->
<Box width="66" height="30">
    <Button class="btn_global" text="消息" width="48" height="30"/>
    <Badge class="badge" count="6" float="true" margin="42,0,0,0"/>
</Box>

<!-- TabCtrl 标签页角标：TabCtrlItem 内置 badge_count/badge_dot/badge_class 属性，无需手动嵌套 -->
<TabCtrlItem class="tab_ctrl_item" title="消息" badge_count="3"/>
```

C++动态更新未读数：

```cpp
ui::Badge* pBadge = dynamic_cast<ui::Badge*>(pWindow->FindControl(_T("badge_msg")));
if (pBadge != nullptr) {
    pBadge->SetCount(pBadge->GetCount() + 1);   // 未读+1，超过99显示"99+"
    pBadge->SetCount(0);                        // 清零，角标自动隐藏
}
```

默认皮肤在global.xml中定义了2个Class：
- `badge`：数字角标，高18，白字红底（语义色`text_badge`/`bg_badge`），宽度随数字自适应，text_padding="5,0,5,0"；
- `badge_dot`：红点角标，8x8，dot="true"。

语义色`bg_badge`在color_light和color_dark的global.xml中定义，派生自`color_error`（红色系），数字文本色`text_badge`为白色。

## Chart的属性
Chart是轻量图表控件，继承自Control，纯自绘实现（不依赖第三方图表库、无需嵌入CEF），对应头文件`duilib/Control/Chart.h`。支持折线图（line）、柱状图（bar）、饼图（pie）三种形态，对标Qt Charts精简版，数据通过XML属性或C++接口绑定。

除Control的通用属性外，新增以下属性：

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| chart_type | line | string | 图表类型："line"折线图 / "bar"柱状图 / "pie"饼图 |
| data | 空 | string | 逗号分隔的数值序列，如 data="12,35,28,40"，作为（单）系列数据 |
| data_labels | 空 | string | 逗号分隔的类别标签，用于折线/柱状图的类目轴（X轴）与饼图的图例 |
| title | 空 | string | 图表标题（居中显示在顶部） |
| x_axis_title | 空 | string | X 轴标题（显示在类目轴下方） |
| y_axis_title | 空 | string | Y 轴标题（纵向显示在左侧） |
| title_id | 空 | string | 图表标题的语言 ID（text_id，多语言支持，优先于 title） |
| x_axis_title_id | 空 | string | X 轴标题的语言 ID（多语言支持） |
| y_axis_title_id | 空 | string | Y 轴标题的语言 ID（多语言支持） |
| data_labels_id | 空 | string | 类目标签的语言 ID（逗号分隔多个 STRID，多语言支持，优先于 data_labels） |
| series_color | color_accent | string | 系列主色（语义色名或颜色值）；饼图各扇区在此基础上按索引自动生成同色系明暗渐变 |
| axis_color | border_control_normal | string | 坐标轴/网格线颜色（折线、柱状图有效） |
| label_color | text_default | string | 文本标签颜色 |
| show_value | true | bool | 是否在数据点/柱顶/扇区旁显示数值 |
| show_grid | true | bool | 是否显示网格线（折线、柱状图有效） |
| show_axis_values | true | bool | 是否显示 Y 轴刻度值 |
| axis_divisions | 4 | int | Y 轴刻度分段数（1~10） |
| legend_visible | pie:true,其他:false | bool | 是否显示图例（饼图默认显示；折线/柱状多系列时可开启） |
| line_width | 2 | int | 折线粗细（折线图有效，自动DPI缩放） |
| line_mode | straight | string | 折线绘制模式："straight"折线 / "curve"平滑曲线（贝塞尔） |
| area_fill | false | bool | 折线下方是否填充半透明面积（折线图） |
| show_data_points | true | bool | 是否绘制数据点圆点（折线图） |
| bar_mode | grouped | string | 柱状排列模式："grouped"分组并排 / "stacked"堆叠累加 |
| donut | false | bool | 饼图是否环形（donut） |
| show_percent | false | bool | 饼图是否显示百分比（替代数值标签） |
| bar_radius | 3 | int | 柱状图圆角半径（像素，0 表示直角，自动DPI缩放） |
| bar_gradient | true | bool | 柱状图是否渐变填充（顶部亮、底部暗） |
| line_glow | false | bool | 折线图是否绘制发光/阴影效果 |
| animation_enabled | true | bool | 数据更新动画是否启用（柱体生长/折线渐入） |
| animation_duration | 300 | int | 数据更新动画时长（毫秒） |

说明：
- 三种形态共用同一套数据（`data`），切换`chart_type`即可改变呈现方式，无需重建数据。
- **多系列**：折线/柱状图支持多系列叠加对比（C++ `SetSeriesData`/`AddSeries`），系列颜色未指定时按内置 8 色色板自动分配；多系列时可开启图例显示系列名称。
- 颜色均使用语义色名（color_light/color_dark的global.xml中的ThemeColor），自动适配深浅色主题；也可直接传颜色值（如`#FF0078D4`）。
- 柱状图支持负值：数据含负值时自动绘制零基线，负值柱向下延伸；分组模式各系列并排，堆叠模式正向累加、负向累减。
- 饼图负值会被忽略（不计入总和、不绘制扇区）；数据全为0或空时图表为空。
- 折线/柱状图数据范围自动计算（上下各留余量），无需手动设置坐标轴范围。
- 数值标签自动格式化：整数不显示小数，非整数保留两位小数。
- **交互能力**：①数据点悬停 Tooltip（显示"系列名 / 类目: 值"）；②点击选中（饼图扇区分离高亮 + 引导线外部标签、柱状单柱描边高亮，再次点击取消）；③折线图滚轮缩放 + 拖拽平移（实时曲线场景）。Tooltip/选中均可用 `SetEnableTooltip`/`SetEnableSelect` 开关。
- **视觉打磨**：柱状图支持圆角（`bar_radius`）与渐变填充（`bar_gradient`）；折线图支持发光/阴影（`line_glow`）；数据更新时柱体生长/折线渐入动画（`animation_enabled`/`animation_duration`）。
- **增量更新（实时监控）**：除全量 `SetSeriesData` 外，提供按系列的点级增量接口 `AddDataPoint`/`InsertDataPoint`/`SetDataPoint`/`RemoveDataPoint`（越界安全返回 `false`），以及 `ClearSeries`/`RemoveSeries`、`GetDataPoint`/`GetSeriesData`（`Get` 系列只读、越界返回空/0.0）；可配合 `SetMaxDataPoints` 设置滚动窗口（超出上限自动丢弃最旧数据点），避免无限累积，适合实时曲线 / 监控场景。
- **坐标轴格式化回调**：提供 `SetYAxisLabelFormatter`/`SetXAxisLabelFormatter`（回调签名为 `std::function<DString(double value, int32_t index)>`，类比 Slider 的 TickLabelCallback），可将 Y 轴刻度值（如 `"50%"`、`"1.2k"`）与 X 轴类目标签（如相对时间）自定义格式化；回调置空时恢复默认的 `FormatValue` 数值格式（X 轴回退为 `data_labels` 字符串）。

对应C++接口：`SetChartType/GetChartType`、`SetTitle`、`SetXAxisTitle`、`SetYAxisTitle`、`SetTitleId`、`SetXAxisTitleId`、`SetYAxisTitleId`、`SetData/AddData/ClearData/GetData`、`SetSeriesData/AddSeries/GetSeries/GetSeriesCount`、`SetDataLabels/GetDataLabels`、`SetDataLabelsId`、`SetSeriesColor`、`SetAxisColor`、`SetLabelColor`、`SetShowValue`、`SetShowGrid`、`SetShowAxisValues`、`SetAxisDivisions`、`SetLegendVisible`、`SetLineWidth`、`SetLineMode`、`SetAreaFill`、`SetShowDataPoints`、`SetBarMode`、`SetDonut`、`SetShowPercent`、`SetBarRadius`、`SetBarGradient`、`SetLineGlow`、`SetAnimationEnabled`、`SetAnimationDuration`；增量更新：`AddDataPoint/InsertDataPoint/SetDataPoint/RemoveDataPoint/GetDataPoint/GetSeriesData/ClearSeries/RemoveSeries/SetMaxDataPoints/GetMaxDataPoints`；坐标轴格式化：`SetYAxisLabelFormatter/GetYAxisLabelFormatter/SetXAxisLabelFormatter/GetXAxisLabelFormatter`；交互：`SetEnableTooltip/IsEnableTooltip`、`SetEnableSelect/IsEnableSelect`、`GetSelected/ClearSelection`、`AttachPointClick/AttachPointHover`、`GetHitFromEvent`。

### XML使用示例

```xml
<!-- 折线图：一周访问量 -->
<Chart width="stretch" height="200" chart_type="line"
       data="120,356,280,410,398,452,380"
       data_labels="周一,周二,周三,周四,周五,周六,周日"
       series_color="color_accent" show_value="true"/>

<!-- 柱状图：季度销售额（含标题与坐标轴标题） -->
<Chart width="stretch" height="220" chart_type="bar"
       title="季度销售额" x_axis_title="季度" y_axis_title="销售额（万元）"
       data="82,116,98,135"
       data_labels="Q1,Q2,Q3,Q4"
       series_color="color_accent"/>

<!-- 柱状图：含负值（自动绘制零基线） -->
<Chart width="stretch" height="220" chart_type="bar"
       data="45,-18,62,-25,38,80" data_labels="1月,2月,3月,4月,5月,6月"/>

<!-- 饼图：环形 + 百分比（右侧自动绘制图例） -->
<Chart width="300" height="300" chart_type="pie"
       data="45,30,15,10"
       data_labels="产品A,产品B,产品C,其他"
       series_color="color_accent" legend_visible="true"
       donut="true" show_percent="true"/>
```

### C++使用示例

```cpp
// 获取控件指针（XML 中 name="chart_sales"）
ui::Chart* pChart = dynamic_cast<ui::Chart*>(pWindow->FindControl(_T("chart_sales")));
if (pChart != nullptr) {
    // 单系列：动态绑定数据（替换原有数据）
    pChart->SetData({ 82.0, 116.0, 98.0, 135.0 });
    pChart->SetDataLabels({ _T("Q1"), _T("Q2"), _T("Q3"), _T("Q4") });

    // 多系列：叠加对比（折线/柱状）
    std::vector<ui::Chart::Series> series;
    ui::Chart::Series s1; s1.name = _T("今年"); s1.data = { 82.0, 116.0, 98.0, 135.0 }; s1.color = _T("color_accent");
    ui::Chart::Series s2; s2.name = _T("去年"); s2.data = { 70.0, 90.0, 110.0, 120.0 }; //颜色自动分配
    series.push_back(s1); series.push_back(s2);
    pChart->SetSeriesData(series);
    pChart->SetLegendVisible(true);

    // 切换图表类型 / 曲线 + 面积填充
    pChart->SetChartType(ui::ChartType::kLine);
    pChart->SetLineMode(ui::ChartLineMode::kCurve);
    pChart->SetAreaFill(true);

    // 追加一个数据点（实时曲线场景）
    pChart->AddData(152.0);
}
```

### 增量更新与坐标轴格式化示例

```cpp
ui::Chart* pChart = dynamic_cast<ui::Chart*>(pWindow->FindControl(_T("chart_rt")));
if (pChart != nullptr) {
    // ① 增量更新：构造两个系列后，逐个追加点（无需每次替换全量数据）
    pChart->SetSeriesData({
        { _T("CPU"),   { 30.0, 32.0, 28.0 }, _T("color_accent") },
        { _T("内存"), { 55.0, 57.0, 53.0 }, _T("color_warning") }
    });
    pChart->AddDataPoint(0, 36.0);   // 向第 0 个系列末尾追加（返回 true）
    pChart->AddDataPoint(1, 60.0);   // 向第 1 个系列末尾追加
    pChart->SetDataPoint(0, 1, 40.0); // 原地修改第 0 系列第 1 个点（不改变长度）
    double v = pChart->GetDataPoint(0, 2); // 读取第 0 系列第 2 个点

    // 滚动窗口：最多保留最近 60 个点，超出自动丢弃最旧点（实时监控内存有界）
    pChart->SetMaxDataPoints(60);

    // ③ 坐标轴格式化回调：Y 轴刻度显示为百分比，X 轴类目标签显示为相对时间
    pChart->SetYAxisLabelFormatter([](double value, int32_t /*index*/) {
        return ui::StringUtil::Printf(_T("%.0f%%"), value);
    });
    pChart->SetXAxisLabelFormatter([](double /*value*/, int32_t index) {
        return ui::StringUtil::Printf(_T("%.1fs"), static_cast<double>(index) * 0.25);
    });
}
```

> 说明：增量接口（如 `AddDataPoint`）越界时安全返回 `false`，不会抛异常；`GetDataPoint`/`GetSeriesData` 越界时返回 `0.0`/空向量。坐标轴格式化回调置空即恢复默认数值格式。

### 交互使用示例

```cpp
// 悬停 Tooltip 与点击选中默认已开启；可通过以下接口开关
pChart->SetEnableTooltip(true);
pChart->SetEnableSelect(true);

// 监听数据点点击（折线点 / 柱 / 饼图扇区 / 图例项）
pChart->AttachPointClick([](const ui::EventArgs& args) {
    ui::Chart::HitResult hit = ui::Chart::GetHitFromEvent(args);
    if (hit.type == ui::Chart::HitType::kSlice) {
        // 命中了饼图第 hit.dataIndex 个扇区
        // ... 业务处理
    }
    else if (hit.type == ui::Chart::HitType::kBar) {
        // 命中了第 hit.seriesIndex 个系列的第 hit.dataIndex 根柱
    }
    return true;
});

// 查询当前选中元素
ui::Chart::HitResult sel = pChart->GetSelected();
if (sel.type != ui::Chart::HitType::kNone) {
    pChart->ClearSelection(); // 清除选中
}
```

> 说明：`HitResult` 含 `type`（kPoint/kBar/kSlice/kLegend）、`seriesIndex`（系列索引）、`dataIndex`（数据索引）。折线图支持滚轮缩放（围绕鼠标位置）与左键拖拽平移。

## Timeline的属性
Timeline是垂直时间轴控件，继承自Control，纯自绘实现，对应头文件`duilib/Control/Timeline.h`。用于日志记录、任务流、审批流等场景：左侧节点+状态连接线，右侧内容（时间/标题/正文）。数据驱动渲染，支持正序/倒序、节点状态配色、连接线样式、节点点击事件、空数据与加载状态、滚动。

除Control的通用属性外，新增以下属性：

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_time | 空 | string | 时间/日期文本，`\|` 分隔多条（数据绑定） |
| item_title | 空 | string | 标题，`\|` 分隔多条（数据绑定） |
| item_content | 空 | string | 正文内容（支持换行），`\|` 分隔多条（数据绑定） |
| item_status | 空 | string | 节点状态，`\|` 分隔多条：pending / inprogress / completed / failed（数据绑定） |
| item_color | 空 | string | 自定义节点颜色，`\|` 分隔多条（status 为 custom 时生效） |
| item_time_id | 空 | string | 多语言数据绑定：时间字段，`\|` 分隔多个STRID，语言切换时自动刷新 |
| item_title_id | 空 | string | 多语言数据绑定：标题字段，`\|` 分隔多个STRID |
| item_content_id | 空 | string | 多语言数据绑定：正文字段，`\|` 分隔多个STRID |
| node_color_pending | text_muted | string | 待处理状态节点颜色（语义色名或颜色值） |
| node_color_inprogress | color_accent | string | 进行中状态节点颜色 |
| node_color_completed | color_success | string | 已完成状态节点颜色 |
| node_color_failed | color_error | string | 失败状态节点颜色 |
| line_color | border_control_normal | string | 连接线颜色 |
| line_style | solid | string | 连接线样式："solid"实线 / "dashed"虚线 |
| scrollbar_track_color | bg_scrollbar_normal | string | 滚动条轨道颜色（语义色名或颜色值） |
| scrollbar_thumb_color | bg_scrollbar_thumb_normal | string | 滚动条滑块颜色（语义色名或颜色值） |
| auto_hide_scrollbar | true | bool | 是否仅在鼠标悬停时显示滚动条（默认开启，更美观） |
| reverse | false | bool | 是否倒序显示（默认正序，即数据顺序从上到下） |
| node_size | 12 | int | 节点直径（像素，自动DPI缩放，4~64） |
| item_spacing | 24 | int | 相邻条目间距（像素，自动DPI缩放） |
| empty_text | 空 | string | 空数据提示文本（无条目时居中显示） |
| empty_text_id | 空 | string | 空数据提示文本的多语言ID（语言切换时自动刷新） |
| loading | false | bool | 是否显示加载中状态 |

说明：
- **数据绑定**：`item_time`/`item_title`/`item_content`/`item_status` 四个属性用 `\|` 分隔条目，长度以最长者为准（不足的条目字段为空）；也可用 C++ 接口 `AddItem`/`SetItems` 动态驱动。
- **多语言数据绑定**：`item_time_id`/`item_title_id`/`item_content_id`（及 `empty_text_id`）用 `\|` 分隔多个 STRID，按语言文件解析为实际文本；语言切换时自动重新解析刷新（重写 `OnLanguageChanged` 实现）。与直接文本属性混用时，后设置的一方生效。
- **节点状态**：pending（灰）、inprogress（强调色）、completed（绿）、failed（红）四态，颜色均可自定义。
- **滚动**：内容超出控件高度时支持鼠标滚轮滚动，并自动显示右侧滚动条（含滑块，可拖动）；轨道/滑块颜色可用 `scrollbar_track_color`/`scrollbar_thumb_color` 自定义，默认对齐主题滚动条语义色（`bg_scrollbar_normal`/`bg_scrollbar_thumb_normal`）；默认仅鼠标悬停时显示滚动条（`auto_hide_scrollbar`）；正文宽度随控件宽度自适应换行（响应式）。
- **追加可见**：`AddItem` 追加条目后自动滚动到新条目（正序滚到底部、倒序滚动到顶部）。
- **事件**：点击节点触发 `kEventTimelineItemClick`（`EventArgs::wParam` 为条目序号）。
- **自定义渲染**：`SetItemDrawCallback` 可覆盖单条条目的默认绘制。

对应C++接口：`AddItem`、`InsertItem`、`RemoveItemAt`、`ClearItems`、`SetItems`、`GetItemCount`、`GetItemAt`、`GetItems`、`SetNodeColor/GetNodeColor`、`SetLineColor/GetLineColor`、`SetLineStyle`、`SetScrollbarTrackColor/GetScrollbarTrackColor`、`SetScrollbarThumbColor/GetScrollbarThumbColor`、`SetAutoHideScrollbar/IsAutoHideScrollbar`、`SetReverse/IsReverse`、`SetNodeSize`、`SetItemSpacing`、`SetEmptyText`、`SetLoading/IsLoading`、`SetItemDrawCallback`、`AttachItemClick`、`GetScrollPos/SetScrollPos`、`ScrollToItem`、`SetEmptyTextId`。

### XML使用示例

```xml
<!-- 审批流程时间轴（数据绑定，含状态配色） -->
<Timeline width="360" height="320"
          item_time="2026-10-01|2026-10-02|2026-10-03"
          item_title="提交申请|初审通过|复审驳回"
          item_content="申请已提交，等待审批。|材料齐全，初审通过。|缺少补充材料，已驳回。"
          item_status="completed|completed|failed"
          line_style="solid" reverse="false"/>

<!-- 任务流时间轴（倒序 + 虚线） -->
<Timeline width="stretch" height="260"
          item_time="09:00|10:30|11:00"
          item_title="启动|执行中|待处理"
          item_content="任务启动。|正在执行核心逻辑。|等待下一阶段。"
          item_status="completed|inprogress|pending"
          line_style="dashed" reverse="true"/>
```

### C++使用示例

```cpp
ui::Timeline* pTimeline = dynamic_cast<ui::Timeline*>(pWindow->FindControl(_T("timeline")));
if (pTimeline != nullptr) {
    // 数据驱动渲染
    pTimeline->AddItem(_T("2026-10-01"), _T("提交申请"), _T("申请已提交，等待审批。"), ui::TimelineNodeStatus::kCompleted);
    pTimeline->AddItem(_T("2026-10-02"), _T("初审通过"), _T("材料齐全，初审通过。"), ui::TimelineNodeStatus::kCompleted);
    pTimeline->AddItem(_T("2026-10-03"), _T("复审中"), _T("正在复审。"), ui::TimelineNodeStatus::kInProgress);

    // 倒序显示 + 虚线
    pTimeline->SetReverse(true);
    pTimeline->SetLineStyle(ui::TimelineLineStyle::kDashed);

    // 节点点击事件（args.wParam 为条目序号）
    pTimeline->AttachItemClick([](const ui::EventArgs& args) {
        size_t index = static_cast<size_t>(args.wParam);
        // ... 处理点击
        return true;
    });

    // 空数据提示与加载状态
    pTimeline->SetEmptyText(_T("暂无数据"));
    pTimeline->SetLoading(true);
}
```

## Flyout浮层窗口
Flyout是通用浮层（弹出卡片）窗口，继承自WindowImplBase，对应头文件`duilib/Control/Flyout.h`。用于在锚点控件周围浮出任意Box内容（操作面板、确认卡片、富内容提示等），是日历弹层、搜索建议、气泡设置等控件的公共基座。

核心特性：
- 锚点定位：支持8个弹出方位（下/上/左/右 × 左对齐/右对齐），主方位空间不足时自动翻转到对侧，并整体夹持在显示器工作区内；
- 默认不抢焦点（`WS_EX_NOACTIVATE`），浮层显示期间父窗口保持激活，可继续操作父窗口；
- 默认点击浮层外部、按Esc键自动关闭；也可调用`Dismiss`主动关闭；
- 非模态窗口：窗口关闭后框架自动释放对象，调用方不需要手动delete；
- 同一时刻只保留一个活动浮层，弹出新浮层时旧浮层自动关闭；
- 浮层为带阴影的层窗口（`WS_POPUP | WS_EX_LAYERED`），内容XML按当前主题语义色自绘，深浅色主题自动适配。

### C++接口

| 接口 | 说明 |
| :--- | :--- |
| `Flyout(Window* pParentWindow)` | 构造浮层，传入父窗口（锚点控件必须属于该窗口） |
| `SetSkinFolder(path)` | 设置内容XML所在资源文件夹；不设置时默认使用父窗口的资源路径 |
| `bool ShowAt(Control* anchor, const DString& xmlFile, Placement = Bottom)` | 在锚点周围显示浮层；返回false时对象已自动销毁，不可再访问 |
| `Dismiss()` | 主动关闭浮层（关闭原因kManual） |
| `SetAutoDismiss(bool)` / `IsAutoDismiss()` | 点击外部/Esc是否自动关闭，默认true |
| `SetNoFocus(bool)` / `IsNoFocus()` | 是否不抢焦点，默认true；设为false后通过失去焦点感知外部点击 |
| `SetGap(int)` / `GetGap()` | 浮层与锚点的间距（未缩放逻辑像素），默认6 |
| `SetAllowFlip(bool)` / `IsAllowFlip()` | 空间不足时是否允许翻转到对侧，默认true |
| `GetAnchor()` | 获取锚点控件（浮层关闭或锚点销毁后可能为nullptr） |
| `GetPlacement()` | 获取期望的弹出方位 |
| `IsOpen()` | 浮层是否处于打开状态 |
| `AttachOpened(callback)` / `AttachClosed(callback)` | 注册打开后/关闭前回调，回调参数为`CloseReason` |
| `static GetActiveFlyout()` | 获取当前活动浮层，没有时返回nullptr |
| `static DismissActive()` | 关闭当前活动浮层（无活动浮层时为空操作） |

`Placement`枚举：`Bottom`（下方左对齐，默认）、`BottomEnd`（下方右对齐）、`Top`、`TopEnd`、`Right`、`RightEnd`、`Left`、`LeftEnd`。

`CloseReason`枚举：`kManual`（主动关闭）、`kClickOutside`（点击外部/失去焦点）、`kEscape`（Esc键）、`kAnchorLost`（锚点或父窗口失效：销毁/隐藏/最小化/DPI变化）。

### 内容XML编写要点

浮层窗口本身由C++创建并设置样式，调用方只需提供**内容XML**：根节点为普通`<Window>`，需开启阴影与层窗口，并按内容给定固定尺寸（框架在其外层包裹阴影）：

```xml
<Window size="240,210" caption="0,0,0,0" use_system_caption="false"
        shadow_type="default" shadow_attached="true"
        layered_window="true" size_box="0,0,0,0">
    <VBox class="flyout" width="240" height="auto">
        <Label class="flyout_title" text="浮层标题"/>
        <Label class="flyout_desc" text="说明文字……" margin="0,6,0,10"/>
        <Button class="btn_primary_global" name="flyout_action" text="主要操作"
                width="stretch" height="30" border_round="4,4"/>
    </VBox>
</Window>
```

说明：
- 窗口尺寸按未缩放逻辑像素填写，高度需要保证内容完整（内容超出时会绘制到阴影区域）；
- 内容中需要交互的控件通过`name`命名，在`ShowAt`成功后用`FindControl`查找并绑定事件；
- 默认皮肤在global.xml中提供3个Class：`flyout`（240宽卡片，`bg_window_card`底色、`border_window`边框、圆角8）、`flyout_title`（14号粗体标题）、`flyout_desc`（12号`text_muted`多行说明文字）。

### 使用示例

```cpp
//点击锚点按钮时弹出（同一锚点再次点击则关闭）
void ShowFlyout(ui::Control* pAnchor)
{
    ui::Flyout* pActive = ui::Flyout::GetActiveFlyout();
    if (pActive != nullptr) {
        if (pActive->GetAnchor() == pAnchor) {
            pActive->Dismiss();      // 同锚点：切换关闭
            return;
        }
        pActive->Dismiss();          // 不同锚点：旧浮层自动关闭
    }

    ui::Flyout* pFlyout = new ui::Flyout(this);
    pFlyout->SetSkinFolder(GetResourcePath().ToString());
    //XML文件名相对SkinFolder解析，不要带子目录前缀
    if (!pFlyout->ShowAt(pAnchor, _T("my_flyout.xml"), ui::Flyout::Placement::Bottom)) {
        return;                      // 创建失败时对象已自动销毁
    }

    ui::Button* pAction = dynamic_cast<ui::Button*>(pFlyout->FindControl(_T("flyout_action")));
    if (pAction != nullptr) {
        pAction->AttachClick([pFlyout](const ui::EventArgs&) {
            //执行操作后关闭浮层
            pFlyout->Dismiss();
            return true;
            });
    }
}
```

注意事项：
- 不抢焦点模式下，浮层内控件可以正常响应点击，但键盘焦点仍在父窗口；需要在浮层内输入文字时调用`SetNoFocus(false)`；
- 外部点击检测基于全局鼠标按键轮询（仅Windows平台），非Windows平台请使用`SetNoFocus(false)`的焦点模式；
- 锚点位于滚动容器内时定位会自动扣除各级滚动偏移，滚动后点击仍在控件视觉位置弹出；
- 父窗口通过`OpenColorTheme`使用私有颜色主题（如ColorTheme示例的深色窗口）时，浮层自动继承同一套配色，无需额外设置；
- 跨显示器DPI变化时浮层会自动关闭（原因`kAnchorLost`），避免位置与尺寸错位。

完整演示见controls示例（"浮层"分组，4个方位按钮）与ColorTheme示例。

## Mask的属性
Mask（遮罩/蒙版层）是继承自`Box`的容器控件（XML节点为`Mask`），用于覆盖在其它控件之上形成半透明蒙层，覆盖弹层体系的三种典型场景：

- **模态对话框背景遮罩**：蒙层铺满整个区域并拦截鼠标输入（默认`mouse_enabled=true`），使下层控件不可交互；
- **新手引导蒙层**：用`highlight_control`在蒙层上"镂空"高亮某个控件，其余区域保持半透明；配合`click_through="true"`，镂空区域内的点击可穿透到目标控件，引导用户点击；
- **加载遮罩**：蒙层覆盖内容区并拦截交互，可在蒙层上放置任意子控件（加载动画、提示文案、按钮等）。

Mask 本质是容器，可以在其上继续放置任意子控件。作为浮动覆盖层使用时，建议配置`float="true" width="stretch" height="stretch"`（框架内置的`mask` Class 已包含该配置），并作为父容器的最后一个子节点（或配合`paint_order`），以保证绘制在最上层、命中测试优先命中。

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| mask_color | String | 语义色`bg_overlay` | 蒙层颜色（语义色名或颜色值）；为空表示不绘制蒙层底色 |
| highlight_control | String | 空 | 需要镂空(高亮)的控件名称（在整个窗口内按名称查找，目标控件可以是 Mask 之外的任意控件）；为空表示不镂空，整块区域均为蒙层 |
| highlight_padding | int | 4 | 镂空区域相对高亮控件的内边距（DPI自适应） |
| highlight_corner | int | 0 | 镂空区域的圆角半径（DPI自适应） |
| click_through | bool | false | 镂空区域内的鼠标点击是否穿透到下层控件；通常配合`highlight_control`用于新手引导 |

> 提示：`mouse_enabled`（继承自容器基类）控制是否拦截鼠标，默认`true`即为模态遮罩；设为`false`则表示"仅视觉遮挡、不拦截鼠标"。

### C++接口

| 方法 | 说明 |
|------|------|
| `SetMaskColor(strColor)` / `GetMaskColor()` | 设置/获取蒙层颜色（语义色名） |
| `SetHighlightControl(strName)` / `GetHighlightControl()` | 设置/获取需要镂空(高亮)的控件名称 |
| `SetHighlightPadding(nPadding)` / `GetHighlightPadding()` | 设置/获取镂空区域内边距（DPI自适应） |
| `SetHighlightCorner(nCorner)` / `GetHighlightCorner()` | 设置/获取镂空区域圆角半径（DPI自适应） |
| `SetClickThrough(bool)` / `IsClickThrough()` | 设置/获取镂空区域内点击是否穿透到下层控件 |
| `Mask::ShowOverWindow(pWindow)`（静态） | 便捷接口：在指定窗口上覆盖一个铺满整窗、拦截鼠标的遮罩，返回遮罩指针（失败返回`nullptr`） |
| `Remove()` | 从父容器中移除本遮罩（若父容器自动销毁子控件，调用后本对象即被销毁，不可再访问） |

### XML使用示例

```xml
<!-- ① 模态遮罩：铺满父容器、拦截鼠标；蒙层内放置一个居中的模态卡片 -->
<Mask class="mask">
    <VBox bkcolor="bg_window_card" border_round="6,6" float="true" halign="center" valign="center"
          width="300" height="170" padding="20,20,20,20">
        <Label text_id="STRID_MASK_MODAL_TITLE" width="stretch" height="auto" font="system_bold_14"/>
        <Button class="btn_global_blue_80x30" name="mask_modal_close" text_id="STRID_MASK_CLOSE"
                float="true" halign="center" valign="bottom" width="90" height="30" border_round="3,3"/>
    </VBox>
</Mask>

<!-- ② 新手引导蒙层：镂空高亮 btn_guide_target，镂空区域内点击可穿透到该按钮 -->
<Mask class="mask" highlight_control="btn_guide_target" highlight_padding="6" highlight_corner="6"
      click_through="true">
    <Label text="点击高亮的按钮继续" float="true" halign="center" valign="bottom"
           width="auto" height="auto" margin="0,0,0,60"/>
</Mask>

<!-- ③ 加载遮罩：覆盖内容区并拦截交互，蒙层内放置加载动画 -->
<Mask class="mask" visible="false">
    <VBox float="true" halign="center" valign="center" width="220" height="70">
        <Progress class="progress_horizontal" marquee="true" marquee_width="60" height="6"/>
        <Label text="加载中，请稍候..." width="stretch" height="auto" margin="0,16,0,0" text_align="center"/>
    </VBox>
</Mask>
```

### C++使用示例

```cpp
// 程序化在整个窗口上覆盖一个遮罩（默认蒙层色 bg_overlay、拦截鼠标）
ui::Mask* pMask = ui::Mask::ShowOverWindow(this);
if (pMask != nullptr) {
    // 追加内容：在蒙层中央放置提示文案
    ui::Label* pLabel = new ui::Label(this);
    pLabel->SetText(_T("Loading..."));
    pLabel->SetFloat(true);
    pLabel->SetHorAlignType(ui::HorAlignType::kAlignCenter);
    pLabel->SetVerAlignType(ui::VerAlignType::kAlignCenter);
    pMask->AddItem(pLabel);

    // 不再需要时移除（若父容器开启自动销毁子控件，Remove 后对象即被销毁）
    // pMask->Remove();
}
```

完整演示见controls示例（"遮罩/蒙版"分组：模态遮罩 / 新手引导 / 加载遮罩三个按钮）。

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
| text_align | "left,top" | string | 文字的水平与垂直对齐方式, 可取值: left、right、hcenter、top、vcenter、bottom，用逗号分割，如"hcenter,vcenter" |
| text_padding |  | rect | 文本内边距，如："2,2,2,2" |
| font | | string | 字体ID |
| text_color | | string | 默认文本颜色 |
| replace_brace | true | bool | 在设置text属性时，是否允许将'{'替换为'<' 和 将'}'替换为'>'，该属性需要放置在text属性前面才能生效，比如replace_brace="false"表示禁止替换 |
| text | | string | 设置格式文本内容，其中允许使用'{'代替'<'，'}'代替'>'，从而避免使用转移字符，便于阅读 |
| text_id | | string | 设置格式文本内容ID，其中对应的内容允许使用'{'代替'<'，'}'代替'>'，从而避免使用转移字符，便于阅读 |
| trim_policy | "all" | string | 设置Trim文本的策略："all"表示去除全部空格；"none"表示不需要去除空格；"keep_one"表示只保留一个空格 |
| default_link_font_color | | string | 超级链接：常规文本颜色值 |
| hovered_link_font_color | | string | 超级链接：Hover状态文本颜色值 |
| pressed_link_font_color | | string | 超级链接：鼠标按下状态文本颜色值 |
| link_font_underline | true | bool | 超级链接：是否使用带下划线的字体 |
| row_spacing_mul | 1.0 | float | 行间距倍数, 比如1.5代表1.5倍行间距 |
| row_spacing_add  |0    | float  | 行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）|
| word_wrap | true| bool | 是否自动换行，如果为false，则只有在`<br/>`标签的时候才换行 |

RichText 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## RichTextBox的属性
RichTextBox与RichText是基于相同模板的类，请参考 `RichText`的属性    
RichTextBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## RichTextHBox的属性
RichTextHBox与RichText是基于相同模板的类，请参考 `RichText`的属性    
RichTextHBox 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## RichTextVBox的属性
RichTextVBox与RichText是基于相同模板的类，请参考 `RichText`的属性    
RichTextVBox 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

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
| tab_box_name| | string | 绑定的TabBox控件名称，绑定后TabCtrl的选择项变化时，TabBox的选择项会跟随变化 |
| drag_order  | true | bool | 是否支持拖动调整顺序（在同一个标签内），默认是开启的 |
| drag_out_id | 0 | int | 设置是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出（拖出到drop_in_id==drag_out_id的容器）|
| drop_in_id | 0 | int | 设置是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入(从drag_out_id==drop_in_id的容器拖入到该容器)|
| selected_tab_item_outline_width  | 0 | float  | 选择标签项的外部轮廓边线的宽度 |
| selected_tab_item_outline_color  |   | string | 选择标签项的外部轮廓边线的颜色 |
| tab_ctrl_bottom_line_height      | 0 | float  | 标签栏底部的边线高度（这个边线，选择标签的区域不绘制，其他区域绘制） |
| tab_ctrl_bottom_line_color       |   | string | 设置标签栏底部的边线颜色 |
| tab_ctrl_orientation | horz | string | 标签栏的布局方向：`horz`(水平，默认值) / `vert`(垂直)。垂直时标签位于左侧、内容区在右侧，选中项轮廓与分隔线自动适配为右侧 |
| overflow_scroll | true | bool | 是否启用溢出滚动：标签总宽度（水平）或总高度（垂直）超出可视区域时，自动显示滚动条。滚动条悬浮在子控件之上，不占用布局空间；拖动标签靠近边缘时会自动滚动 |

**对应 C++ 接口：** `SetTabCtrlOrientation`/`GetTabCtrlOrientation`、`SetEnableOverflowScroll`/`IsEnableOverflowScroll`。

TabCtrl 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## TabCtrlItem的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :---     | :---   | :---     | :--- |
| tab_box_item_index | | int | 绑定的TabBox子项索引号（即点击这个标签页，切换到此索引号的TabBox页面） |
| title | | string | 标签页的标题文字 |
| title_id | | string | 标签页的标题文字ID（用于支持多语版） |
| title_class | | string | 标签页的标题文字资源属性Class值|
| icon | | string | 标签页的图标资源字符串 |
| icon_class | | string | 标签页的图标资源属性Class值|
| close_button_class | | string | 标签页的关闭按钮资源属性Class值|
| line_class | | string | 标签页的分割线资源属性Class值|
| selected_round_corner | | size | 标签页选择状态时的圆角大小|
| hovered_round_corner | | size | 标签页悬停状态时的圆角大小|
| hovered_padding | | UiPadding | 标签页悬停状态的背景色的内边距|
| auto_hide_close_button | false | bool | 关闭按钮是否自动隐藏|
| fixed | false | bool | 是否为固定标签：固定标签不可拖动、不可作为拖放目标，且不显示关闭按钮；多个固定标签始终位于标签栏前端，构成“固定块”（普通标签只能在固定块之后移动，且不可越过固定块）|
| badge_count | 0 | int64 | 角标数量：大于0时显示角标，小于等于0时自动隐藏；未设置badge_class时不创建角标控件|
| badge_max_count | 99 | int64 | 角标数量上限，超过上限时显示"上限+"（如"99+"）|
| badge_dot | false | bool | 角标红点模式：true为纯小圆点，不显示数字；需配合badge_class或badge属性使用|
| badge_class | | string | 角标控件的Class值（如`tab_ctrl_item_badge`），为空时不创建角标控件|

**说明：**
- TabCtrlItem 内置角标能力：通过 `badge_class` 指定一个基于 `Badge` 控件的 Class，再使用 `badge_count`/`badge_dot`/`badge_max_count` 配置显示内容。未设置 `badge_class` 时，角标属性仅保存数值，不会创建 Badge 子控件。
- 默认皮肤在 global.xml 中定义了 `tab_ctrl_item_badge`（数字角标，高16，白字红底）和 `tab_ctrl_item_badge_dot`（红点角标，8x8），已挂载在 `tab_ctrl_item` 的 `badge_class` 上。
- 角标子控件不参与鼠标事件（`mouse_enabled="false"`），不会阻挡标签页的点击。

**对应 C++ 接口：** `SetBadgeCount/GetBadgeCount`、`SetBadgeMaxCount/GetBadgeMaxCount`、`SetBadgeDot/IsBadgeDot`、`SetBadgeClass/GetBadgeClass`、`GetBadgeControl`。

**XML 示例：**

```xml
<TabCtrl class="tab_ctrl" width="stretch" height="36">
    <!-- 数字角标（count=5） -->
    <TabCtrlItem class="tab_ctrl_item" title="消息" badge_count="5"/>
    <!-- 超限角标（显示"99+"） -->
    <TabCtrlItem class="tab_ctrl_item" title="通知" badge_count="120"/>
    <!-- 红点角标（使用独立 dot Class） -->
    <TabCtrlItem class="tab_ctrl_item" title="动态" badge_class="tab_ctrl_item_badge_dot" badge_dot="true"/>
    <!-- 无角标 -->
    <TabCtrlItem class="tab_ctrl_item" title="设置"/>
</TabCtrl>
```

C++ 动态更新角标：

```cpp
ui::TabCtrlItem* pTabItem = dynamic_cast<ui::TabCtrlItem*>(pWindow->FindControl(_T("tab_msg")));
if (pTabItem != nullptr) {
    pTabItem->SetBadgeCount(pTabItem->GetBadgeCount() + 1);   // 未读+1
    pTabItem->SetBadgeCount(0);                               // 清零，角标自动隐藏
}
```

**垂直标签栏、固定标签与溢出滚动：**

```xml
<!-- 垂直标签栏：标签位于左侧，内容区在右侧；选中项轮廓与分隔线自动适配为右侧 -->
<TabCtrl class="tab_ctrl" tab_ctrl_orientation="vert" width="160" height="stretch">
    <!-- 固定标签：始终位于前端、不可拖动、无关闭按钮 -->
    <TabCtrlItem class="tab_ctrl_item" title="首页" fixed="true"/>
    <TabCtrlItem class="tab_ctrl_item" title="收藏" fixed="true"/>
    <!-- 普通标签：可拖动调序，但不能越过前端的固定块 -->
    <TabCtrlItem class="tab_ctrl_item" title="消息" badge_count="5"/>
    <TabCtrlItem class="tab_ctrl_item" title="设置"/>
</TabCtrl>

<!-- 水平标签栏 + 溢出滚动：标签过多时自动显示滚动条 -->
<TabCtrl class="tab_ctrl" overflow_scroll="true" width="stretch" height="36">
    <TabCtrlItem class="tab_ctrl_item" title="标签1"/>
    <TabCtrlItem class="tab_ctrl_item" title="标签2"/>
    <TabCtrlItem class="tab_ctrl_item" title="标签3"/>
    ...
</TabCtrl>
```

- `tab_ctrl_orientation="vert"` 将标签栏切换为垂直布局（内部改用 `VLayout`，选中项轮廓与分隔线自动适配为右侧）。
- `fixed="true"` 的标签构成前端的"固定块"：不可拖动、不可作为拖放目标、不显示关闭按钮；普通标签只能在固定块之后移动，且不可越过固定块；固定标签始终位于标签栏前端（即使通过 `AddItem` 追加也会插入到固定块末尾）。
- `overflow_scroll="true"`（默认值）在标签总尺寸超出可视区域时自动显示滚动条，滚动条悬浮在子控件之上、不占用布局空间；拖动标签靠近可视区域边缘时会自动滚动。

**触发溢出的前提：标签项要有确定宽度**

默认皮肤中 `tab_ctrl_item` 未定义 `width`，控件基类默认宽度为 `stretch`（见 `PlaceHolder` 构造函数），
因此标签项会被**等分拉伸**填满标签栏——这种情况下无论有多少个标签，内容宽度都不会超出，滚动条也就不会出现。
要让溢出滚动生效，需要给标签项一个确定宽度（`width="96"` 等）或自适应内容宽度（`width="auto"`）。

- 水平溢出：`TabCtrl` 宽度 < 标签项宽度之和；
- 垂直溢出：`TabCtrl` 高度 < 标签项高度之和（`tab_ctrl_orientation="vert"` 时由 `SetTabCtrlOrientation` 自动启用纵向滚动条）。

```xml
<!-- 水平溢出：标签栏宽 300，标签项宽 96，第 4 个标签起开始溢出 -->
<TabCtrl class="tab_ctrl" width="300" height="36" overflow_scroll="true">
    <TabCtrlItem class="tab_ctrl_item" title="标签1" width="96"/>
    ...
</TabCtrl>
```

**固定块的归位规则**（`SetFixed` 运行时切换同样遵循）：

- 固定块恒为标签栏前端的一段连续区间，XML 中未按顺序书写时会在初始化阶段自动归位（保持各自的相对顺序）；
- 运行期调用 `TabCtrlItem::SetFixed(true)` 会把该标签移动到固定块末尾，`SetFixed(false)` 会移动到固定块之后的第一个位置；
- 插入与移动均受锚定约束：`AddItem` 固定标签插到固定块末尾、普通标签追加到末尾；`AddItemAt`/`SetItemIndex` 固定标签只能落在 `[0, 固定数]`，普通标签只能落在 `[固定数, 标签数]`。

```cpp
ui::TabCtrl* pTabCtrl = dynamic_cast<ui::TabCtrl*>(pWindow->FindControl(_T("tab_ctrl")));
if (pTabCtrl != nullptr) {
    //固定/取消固定：标签会自动移动到对应区块的边界
    ui::TabCtrlItem* pItem = dynamic_cast<ui::TabCtrlItem*>(pTabCtrl->GetItemAt(0));
    if (pItem != nullptr) {
        pItem->SetFixed(true);
    }
    const size_t nFixedCount = pTabCtrl->GetFixedItemCount();   //固定标签数量
}
```

TabCtrlItem 控件继承了 `ControlDragableT` 属性，更多可用属性请参考`ControlDragableT`的属性

## ControlDragableT的属性(模板类)
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :---     | :---   | :---     | :--- |
| drag_order  | true | bool | 是否支持拖动调整顺序（在同一个容器内），默认是开启的 |
| drag_out    | true| bool | 是否支持拖出操作（在相同窗口的不同容器内），默认是开启的 |
| drag_alpha  | 216 | uint8_t | 拖动顺序时，控件的透明度 |

## ControlDragable的属性
ControlDragable 控件继承了`ControlDragableT`和`Control`属性，更多可用属性请参考`ControlDragableT`和`Control`的属性

## BoxDragable的属性
BoxDragable 控件继承了`ControlDragableT`和`Box`属性，更多可用属性请参考`ControlDragableT`和`Box`的属性

## HBoxDragable的属性
HBoxDragable 控件继承了`ControlDragableT`和`HBox`属性，更多可用属性请参考`ControlDragableT`和`HBox`的属性

## VBoxDragable的属性
VBoxDragable 控件继承了`ControlDragableT`和`VBox`属性，更多可用属性请参考`ControlDragableT`和`VBox`的属性

## ControlMovableT的属性(模板类)
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :---     | :---   | :---     | :--- |
| enable_move_pos  | true | bool    | 是否支持拖动调整控件的位置，默认是开启的 |
| move_pos_draggable_border |   | UiPadding | 控件可移动矩形的边框范围（四周可点击拖动，但中心区域不可拖动） |
| move_pos_non_draggable_margin |  | UiMargin | 控件可移动矩形的外边距（外边距定义的四周区域不可点击拖动，仅中心区域可拖动） |
| move_parent_pos  | false| bool    | 执行拖动调整控件位置操作时，是否调整父容器的位置，"true"表示调整父容器的位置，"false"表示调整控件自身的位置 |
| move_pos_alpha   | 216  | uint8_t | 拖动调整位置时，控件的透明度 |
| move_pos_reserve_width   | 20  | int | 横向移动时，在父容器内保留的高度，避免控件完全溢出父容器(未经DPI缩放) |
| move_pos_reserve_height   | 20  | int | 纵向移动时，在父容器内保留的宽度，避免控件完全溢出父容器(未经DPI缩放) |
| move_pos_keep_within_parent   | false  | bool | 移动控件时，确保子控件位于父容器内，无溢出 |

## ControlMovable的属性
ControlMovable 控件继承了`ControlMovableT`和`Control`属性，更多可用属性请参考`ControlMovableT`和`Control`的属性

## BoxMovable的属性
BoxMovable 控件继承了`ControlMovableT`和`Box`属性，更多可用属性请参考`ControlMovableT`和`Box`的属性

## HBoxMovable的属性
HBoxMovable 控件继承了`ControlMovableT`和`HBox`属性，更多可用属性请参考`ControlMovableT`和`HBox`的属性

## VBoxMovable的属性
VBoxMovable 控件继承了`ControlMovableT`和`VBox`属性，更多可用属性请参考`ControlMovableT`和`VBox`的属性

## ControlResizableT的属性(模板类)
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :---     | :---   | :---     | :--- |
| enable_resize   | true  | bool | 是否支持鼠标拖动改变控件的大小 |
| enable_move_pos  | false | bool    | 是否支持拖动调整控件的位置，默认是关闭的；若开启，则相关属性可参考`ControlMovableT`的属性 |
| resize_size_box   | | UiRect | 设置控件四边调整大小时的可拉伸范围的大小 |
| resize_reserve_width   | 10| int | 设置调整大小时，保留的最小宽度(未经DPI缩放) |
| resize_reserve_height   | 10| int | 设置调整大小时，保留的最小高度(未经DPI缩放) |
| resize_keep_within_parent| false | bool | 设置调整控件大小时，是否确保子控件位于父容器内，无溢出 |

ControlResizableT 控件继承了`ControlMovableT`的属性，更多可用属性请参考`ControlMovableT`的属性

## ControlResizable的属性
ControlResizable 控件继承了`ControlResizableT`和`Control`属性，更多可用属性请参考`ControlResizableT`和`Control`的属性

## BoxResizable的属性
BoxResizable 控件继承了`ControlResizableT`和`Box`属性，更多可用属性请参考`ControlResizableT`和`Box`的属性

## HBoxResizable的属性
HBoxResizable 控件继承了`ControlResizableT`和`HBox`属性，更多可用属性请参考`ControlResizableT`和`HBox`的属性

## VBoxResizable的属性
VBoxResizable 控件继承了`ControlResizableT`和`VBox`属性，更多可用属性请参考`ControlResizableT`和`VBox`的属性

## ListBoxItem的属性
ListBoxItem是模板ListBoxItemTemplate类的一个具体实现，在`duilib/Box/ListBoxItem.h`文件中定义，相关的类型定义有三个：    
```
typedef ListBoxItemTemplate<Box> ListBoxItem;
typedef ListBoxItemTemplate<HBox> ListBoxItemH;
typedef ListBoxItemTemplate<VBox> ListBoxItemV;
```
ListBoxItem作为ListBox容器中的子项，其本身没有定义任何属性。
ListBoxItem 继承了 `Option` 的属性，更多可用属性请参考`Option`的属性

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
| expand_hovered_image | | string | 展开时，悬停状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_pressed_image | | string | 展开时，按下状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_disabled_image | | string | 展开时，禁止状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_normal_image | | string | 收起时，正常状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_hovered_image | | string | 收起时，悬停状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_pressed_image | | string | 收起时，按下状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| collapse_disabled_image | | string | 收起时，禁止状态的图片，定义方法请参考`global.xml` 中的对应内容和示例程序|
| expand_image_right_space | | int | 展开图片右侧的空隙 |
| check_box_image_right_space | | int | CheckBox图片右侧的空隙 |
| icon_image_right_space | | int | 图标右侧的空隙 |

TreeNode 控件继承了 `ListBoxItem` 属性，更多可用属性请参考`ListBoxItem`的属性

## DirectoryTree 控件（继承 TreeView 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| small_icon_size | 16 | int | 树节点的图标大小 |
| large_icon_size | 32 | int | 大图标大小，用于展示目录里面的内容，树节点本身未使用该属性 |
| show_hiden_files | false | bool | 是否显示隐藏文件 |
| show_system_files | false | bool | 是否显示系统文件 |

DirectoryTree 控件继承了 `TreeView` 属性，更多可用属性请参考`TreeView`的属性

## ListCtrl 控件（继承 VBox 属性）
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
| row_grid_line_width | 1.0 | float | 横向网格线的宽度|
| row_grid_line_color | | int | 横向网格线的颜色|
| column_grid_line_width | 1.0 | float | 纵向网格线的宽度|
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
| frozen_column_count | 0 | int | 冻结列的个数：固定列表左侧的前N列，横向滚动时这些列固定不动（0表示不冻结）|
| selection_mode | "row" | string | 选择模式，可选值："row"（行选择，默认）、"cell"（单元格选择）|
| enable_copy | true | bool | 是否支持复制选中内容到剪贴板（`Ctrl+C` 复制为 TSV：行模式复制选中行、单元格模式复制选中区域）|
| enable_csv_export | true | bool | 是否支持 CSV 导出（`Ctrl+Shift+C` 复制为 CSV 到剪贴板，以及 `ExportToCsvFile` 导出到文件）|

ListCtrl 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性    
ListCtrl 控件的各个视图继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性设置:[Box.md](Box.md)，视图的属性需要在`global.xml` 中设置。
单元格编辑器类型不通过 XML 属性设置，而是在代码里通过 `ListCtrlColumn::nEditorType` 声明（详见下文"ListCtrl 表格增强"中的"按列编辑器类型"）。

### ListCtrl 表格增强

面向报表类场景（类似 Qt QTableView / WPF DataGrid），ListCtrl 在 Report 视图下提供以下增强能力：

| 能力 | 接口 / 属性 | 说明 |
| :--- | :--- | :--- |
| 列宽拖拽调整 | Header 分割条（`enable_column_width_auto` 控制双击自适应） | 已有能力：拖动表头分割条调整列宽，双击分割条按内容自适应 |
| 列头拖拽换序 | `enable_header_drag_order` | 已有能力：拖动表头改变列的顺序（默认开启） |
| 单元格图标 | `SetSubItemImageId(ById)` | 已有能力：为单元格设置图标 |
| 列冻结（固定首列） | `frozen_column_count` / `SetFrozenColumnCount` / `GetFrozenColumnCount` | 冻结列表左侧前 N 列，横向滚动时固定不动；表头同步冻结，网格线与鼠标命中测试均已适配 |
| 单元格进度条 | `SetSubItemProgress(ById)` / `GetSubItemProgress(ById)` | 单元格以进度条方式显示，进度值有效范围 `[0, 100]`，`-1` 表示关闭进度条（按普通文本显示） |
| 单元格富文本 | `SetSubItemRichText(ById)` / `IsSubItemRichText(ById)` | 单元格文本按富文本渲染（支持 `<b>/<i>/<u>/<font>/<a>` 等标签，见 [RichText的属性](#richtext的属性)） |
| 单元格自动换行 | `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` | 控制单元格文本单行/多行：`true` 为单行（超出按省略号裁剪，默认），`false` 为自动换行（行高需由 `SetDataItemHeight` 调够，否则被裁剪）；仅 Report 视图有效 |
| 单格刷新 | `RefreshSubItem` / `RefreshSubItemById` | 只刷新指定单元格（进度、状态类列的局部更新），避免整行刷新 |
| 列显示/隐藏 | `SetColumnVisible(ById)` / `IsColumnVisible(ById)` | ListCtrl 一级的列显隐接口（等价于表头项的显隐，内部自动同步数据列） |
| 列布局事件 | `AttachColumnWidthChanged` / `AttachColumnOrderChanged` / `AttachColumnVisibleChanged` / `AttachColumnSorted` | 列宽/列序/列显隐/排序变化时通知应用层，便于持久化列布局 |
| 单元格选择模式 | `selection_mode`（XML，`row`/`cell`）/ `SetSelectionMode` / `GetSelectionMode` | 选择模式：`row`（行选择，默认，既有行为不变）/ `cell`（单元格选择）；切到 `cell` 会清空行选择，切回 `row` 会清空单元格选择 |
| 单元格选择 | `SetCurrentCell` / `GetCurrentCell` / `SetCellSelection` / `ClearCellSelection` / `GetCellSelection` / `SetSelectedCells` / `GetSelectedCells` / `IsCellSelected` / `IsCellCurrent` | 单元格选择模式下的当前格与选择集合：底层按行身份（RowId）+ 列ID 逐单元格保存选中状态（与显示顺序解耦，排序后不漂移），支持**不连续**选择。单击替换选中该格（锚点=焦点），`Ctrl+单击`切换该格选中状态，`Shift+单击`/拖动按矩形替换，`Ctrl+拖动`/`Ctrl+Shift+单击`按矩形追加；`GetSelectedCells` 展开为单元格列表，`SetSelectedCells` 为行选择 `SetSelectedDataItems` 的单元格等价接口 |
| 单元格选择事件 | `AttachCurrentCellChanged` / `AttachCellSelectionChanged` | `kEventCurrentCellChanged`（当前单元格变化，wParam=行显示索引，lParam=列ID）/ `kEventCellSelectionChanged`（选择区域变化，wParam=区域个数，pEventData 指向 `std::vector<ui::ListCtrlCellRange>*`（选中区域列表，可能多项，空表示无选择））；XML 内联事件名 `on_current_cell_changed` / `on_cell_selection_changed` |
| 复制 / 导出 | `enable_copy` / `SetEnableCopy` / `IsEnableCopy`、`enable_csv_export` / `SetEnableCsvExport` / `IsEnableCsvExport`、`CopySelectionToClipboard(bCsvFormat)`、`ExportToCsvFile(路径, 是否仅选中)` | TSV 复制（`Ctrl+C`：行模式复制选中行、单元格模式复制选中区域）与 CSV 导出（`Ctrl+Shift+C` 复制为 CSV 到剪贴板；`ExportToCsvFile` 写 UTF-8 BOM 的 CSV 文件，便于 Excel 直接打开、中文不乱码）。`kCell` 多区域选择取**并集外接矩形**，矩形内未选中单元格输出空串以保证行列对齐；整表导出不含表头行；与编辑态互斥（编辑中先提交再复制）。两个开关默认开启，关闭时对应快捷键与接口均 inert（返回 `false`，不改变编辑态与数据） |
| 行/单元格模式分派 | `SetSelectAll` / `SetSelectNone`；`SetSelectedDataItems` / `GetSelectedDataItems` | `SetSelectAll` / `SetSelectNone` 按当前模式分派（`row` 全选/清空行，`cell` 全选/清空单元格）；行专用接口 `SetSelectedDataItems` / `GetSelectedDataItems` **保留原有行为**（仍正常读写底层行选择状态），必要时可用 `SetSelectedCells` / `GetSelectedCells` 作为单元格等价接口。`cell` 模式下不通过屏蔽接口来实现隔离，而是由**绘制层**屏蔽行选中视觉表现（`ListCtrlReportView::CanPaintSelectedColors` 返回 false），因此无论行选择状态由哪个入口（鼠标右键、键盘、API）改动，界面都看不出行被选中 |
| 列级比较模式 | `SetColumnSortFlag(ById)` / `GetColumnSortFlag(ById)`，标志位 `ListCtrlSubItemSortFlag` | 按列声明排序的比较方式（文本/数值/日期/进度等），鼠标点击该列表头排序时自动采用；`kSortByProgress` 按进度值排序，`kSortByTextAsNumber` 文本按数值比较（`"10" > "9"`），`kSortByTextAsDate` 文本按日期时间先后比较（`"2024-3-7" < "2024-10-1"`） |
| 批量删除行 | `DeleteDataItems(const std::vector<size_t>&)` | 一次删除多行（索引可乱序、可含越界/重复项，重复与越界项被忽略），内部按行身份去重后对显示顺序表做单趟压缩，避免逐行删除的 O(K×N) 搬移；返回实际删除行数 |
| 存储压缩 | `CompactStorage()` | 显式的整表维护操作：行身份重编号 + 存储页全量压缩，回收多次增删后遗留的墓碑槽位与空页内存。仅在无填充/绘制进行时调用；调用后内部已发出的单元格原始指针失效。单行/批量删除时，整页全部变为墓碑的存储页会被自动回收 |
| 单元格编辑 | `EnterCellEdit` / `EnterCurrentCellEdit` / `IsCellEditing` / `GetEditingCell` | kCell 模式 + Report 视图；`F2` 或"选中后单击一次"进入编辑（单击链路沿用原有、未改动）；`Enter` 提交并下移一行（仅 `kCell` 模式；`kRow` 行模式提交候选不移动选中行、不设置当前格、不滚动视图）、`Esc` 取消、失焦提交不移动；需 `enable_item_edit="true"` |
| 按列编辑器类型 | `SetColumnEditorType` / `SetColumnEditorTypeById` / `GetColumnEditorType` / `GetColumnEditorTypeById` | 枚举 `ListCtrlEditorType { kEditText, kEditNumber, kEditDate, kEditCombo }`；列声明字段 `ListCtrlColumn::nEditorType`（默认 `kEditText`）；提交时按类型校验输入（空串视为清空单元格合法；数值列为十进制数值；日期列为 `yyyy-mm-dd` / `yyyy/mm/dd`，复用 `Calendar::ParseDateString`），非法输入丢弃（不写回、不派发 `kEventLeaveEdit`）；`kEditCombo` 为下拉候选编辑器（需配合 `SetColumnCandidates(ById)` 配置候选，kCell 模式与行选择模式均支持）；**不引入 XML 属性**（列由代码声明） |
| 按列候选值（下拉编辑器） | `SetColumnCandidates` / `SetColumnCandidatesById` / `GetColumnCandidates` / `GetColumnCandidatesById` | 按列保存纯文本候选列表（`std::vector<DString>`，按列ID存储），供 `kEditCombo` 使用；传入空列表表示清空候选；`kEditCombo` 列未配置候选时按 `F2` / 单击不会进入编辑状态；候选浮层复用 `Flyout` + `VListBox`（主题类 `list_ctrl_combo_flyout` / `list_ctrl_combo_list` / `list_ctrl_combo_item`），鼠标点选或 `Enter` 提交、`↑`/`↓` 移动高亮、`Esc` 或点击外部取消不写回 |

配套演示：`examples/ListCtrl` 左侧设置面板的「单元格选择与勾选」「复制与导出」「进度条与编辑器」「事件与状态显示」四个分组，逐项提供上述能力的手工测试入口——单元格选择 API 与单元格选择事件（含当前格 / 选中个数显示）、行选择事件（`Select` / `SelChanged`，含选中行个数与行索引显示）、CheckBox 勾选 API 与 `auto_check_select`、复制为 CSV、整表导出 CSV、复制 / CSV 导出开关、`progress_text_pos` / `progress_color` 运行时切换与单格进度修改、逐列编辑器类型切换（文本 / 数值 / 日期 / 下拉候选）、`SetSortCompareFunction` 自定义排序、编辑状态事件，以及列布局事件（"最近列事件"标签实时显示）。界面标签与弹窗中的列定位统一以**列索引（ColumnIndex）**显示（而非列ID），便于阅读。

**列级比较模式（排序）：**

排序的比较方式按列声明，鼠标点击表头排序时会自动采用该列声明的模式（由"列的语义"决定，而非按单元格当前状态推断）：

```cpp
// 第 2 列（列索引 1）按进度值排序；第 4 列文本按数值大小排序；第 5 列文本按日期先后排序
pListCtrl->SetColumnSortFlag(1, ui::ListCtrlSubItemSortFlag::kSortByProgress);
pListCtrl->SetColumnSortFlag(3, ui::ListCtrlSubItemSortFlag::kSortByTextAsNumber);
pListCtrl->SetColumnSortFlag(4, ui::ListCtrlSubItemSortFlag::kSortByTextAsDate);
uint8_t flag = pListCtrl->GetColumnSortFlag(3); // 读取该列的比较模式
```

常用标志位（`ListCtrlSubItemSortFlag`，可按位组合）：`kSortByText`（默认，按文本字典序）、`kSortByTextAsNumber`（文本按数值比较，`"10" > "9"`）、`kSortByTextAsDate`（文本按日期时间先后比较，如 `"2024-3-7" < "2024-10-1"`；字段间允许任意非数字分隔符、月/日可不补零，无法解析为日期的文本视为最小值）、`kSortByProgress`（按 `nProgress` 进度值）、`kSortByUserDataN` / `kSortByUserDataS`（按用户数据）、`kSortByGroup`（先按 `nSortGroup` 分组）、`kSortNoCase`（字符串忽略大小写）。

**排序稳定性：**

排序使用稳定排序（`std::stable_sort`）：比较结果相等（同值）的元素保持原始相对顺序，升序与降序均保持该特性。因此同组（如文件夹/文件）或同值的行不会因排序而乱序、交错；降序通过转置比较实现，而非"升序排完再整体反转"（后者会把同值行的相对顺序一并颠倒）。该列无单元格数据的行统一视为最小值：升序排在最前，降序排在最后。自定义比较函数（`SetSortCompareFunction` 或 `SortDataItems` 的 `pfnCompareFunc` 参数）只需实现 `(a < b)` 的语义，稳定性由排序器保证。

**列冻结（冻结首列）：**

```cpp
// 固定左侧第 1 列，横向滚动时该列不移动
pListCtrl->SetFrozenColumnCount(1);
size_t nCount = pListCtrl->GetFrozenColumnCount();
```

XML 中也可直接配置：`<ListCtrl class="list_ctrl" name="list_ctrl" frozen_column_count="1"/>`

**单元格进度条：**

进度条的样式由 `ListCtrlSubItem` 的 `progress_color`（前景）与 `progress_bkcolor`（背景轨道）两个语义色属性控制，默认值在 `global.xml` 的 `list_ctrl_sub_item` Class 中配置（`bg_progress` / `bg_progress_track`）。进度条为圆角胶囊形、垂直居中，高度自动保证可容纳文本。若单元格同时设置了文本，文本会叠加显示在进度条之上（可用于显示百分比），其位置由 `progress_text_pos` 控制：

| 属性 | 默认值 | 说明 |
| :--- | :--- | :--- |
| `progress_text_pos` | `center` | 文本位置：`center`（在单元格内居中）/ `left`（在单元格内左对齐）/ `right`（在单元格内右对齐）/ `outside`（显示在进度条右侧外部，此时进度条宽度相应缩短）。C++ 接口：`SetProgressTextPos()` / `GetProgressTextPos()` |

```cpp
// 第2列（columnIndex=1）以进度条方式显示，进度值 60
pListCtrl->SetSubItemProgress(0, 1, 60);
int32_t nProgress = pListCtrl->GetSubItemProgress(0, 1); // 返回 60

// 关闭该单元格的进度条（恢复为普通文本显示）
pListCtrl->SetSubItemProgress(0, 1, -1);
```

也可在填充数据时一次性设置：

```cpp
ui::ListCtrlSubItemData subItemData;
subItemData.text = _T("60%");
subItemData.nProgress = 60;      // -1 表示不显示进度条
pListCtrl->SetSubItemData(itemIndex, columnIndex, subItemData);
```

**单元格编辑（C3）：**

仅 kCell 模式 + Report 视图生效，且需 `enable_item_edit="true"`。按 `F2`（等价于调用 `EnterCurrentCellEdit`，绕过 600ms 延时立即进入编辑；行不在可视区时会先滚动到可视区）或"选中该格后单击一次"（沿用原有单击链路）进入编辑；`Enter` 提交并把当前格下移一行（仅 `kCell` 模式；`kRow` 行模式提交候选不移动选中行、不设置当前格、不滚动视图）、`Esc` 取消且不移动、点击其它区域失焦则提交但不移动；置顶行 / 不可编辑单元格 / `enable_item_edit="false"` 时按 `F2` 无反应。

```cpp
// 声明第 2 列为数值、第 5 列为日期（列序从 0 开始）
pListCtrl->SetColumnEditorType(1, ui::ListCtrlEditorType::kEditNumber);
pListCtrl->SetColumnEditorType(4, ui::ListCtrlEditorType::kEditDate);
// 或按列ID（列序变化不影响）
// pListCtrl->SetColumnEditorTypeById(columnId, ui::ListCtrlEditorType::kEditNumber);
// 对当前单元格进入编辑（等价于按 F2）
pListCtrl->EnterCurrentCellEdit();
```

提交时按列声明的编辑器类型校验输入：空串一律合法（视为清空单元格），`kEditText` 恒合法，`kEditNumber` 仅接受合法十进制数值，`kEditDate` 仅接受 `yyyy-mm-dd` / `yyyy/mm/dd`（复用 `Calendar::ParseDateString`）；非法输入被丢弃（不写回数据、不派发 `kEventLeaveEdit`）。其中 `kEditNumber` 列在编辑过程中即限制输入字符：只允许输入数字、正负号和小数点（基于 RichEdit 的字符白名单，键盘输入与粘贴均生效），非数字字符无法录入。`kEditDate` 列使用 `DateTime` 控件编辑（编辑格式 `date_calendar`，主题类 `list_ctrl_datetime`，不再复用 RichEdit）：进入编辑后自动弹出月日历浮层，在日历中选中日期即提交写回（`kCell` 模式提交成功后当前格下移一行），点击日历"清除"写回空单元格，`Esc` 关闭日历不写回；提交前仍以 `Calendar::ParseDateString` 校验非法日期并丢弃。复用既有事件 `kEventEnterEdit` / `kEventLeaveEdit`，不新增事件；`kEditText` / `kEditNumber` 共用同一个 RichEdit 编辑器，`kEditDate` 使用 `DateTime` 控件。

**单元格编辑器：下拉候选（kEditCombo）：**

候选值按列配置（纯文本列表，按列ID保存），不使用 RichEdit，而是在单元格下方弹出候选浮层（内部复用 `Flyout` + `VListBox`，不抢焦点：父窗口保持激活，键盘事件仍由 `ListCtrl` 处理）。

```cpp
// 声明第 8 列为下拉候选编辑器，并配置候选值（列序从 0 开始）
std::vector<DString> candidates = { _T("Red"), _T("Green"), _T("Blue") };
pListCtrl->SetColumnCandidates(7, candidates);
pListCtrl->SetColumnEditorType(7, ui::ListCtrlEditorType::kEditCombo);

// 按列ID（列ID稳定，列序可变）
// pListCtrl->SetColumnCandidatesById(columnId, candidates);
// std::vector<DString> items = pListCtrl->GetColumnCandidatesById(columnId);
// 传入空列表即清空候选：pListCtrl->SetColumnCandidates(7, std::vector<DString>());
```

交互说明：
- 进入编辑：按 `F2` 或"选中该格后单击一次"，与其它编辑器同一链路；浮层初始选中项＝当前单元格文本在候选列表中的序号（无匹配取第 1 项）。
- 鼠标点击候选项 或 按 `Enter`：立即提交并关闭浮层，写回单元格文本（`kCell` 模式提交成功后当前格下移一行，与 RichEdit 编辑器的 `Enter` 语义一致；`kRow` 行模式提交候选后不移动选中行、不设置当前格、不滚动视图）。
- 按 `↑` / `↓`：移动候选高亮；按 `Esc`、点击浮层外部、或在列表其它位置点击/滚动：关闭浮层且**不写回**（视为放弃本次编辑）。
- 该列未配置候选值时，按 `F2` / 单击不会进入编辑状态（不弹出空浮层）。
- kCell 模式与行选择模式 + Report 视图均生效，仍受 `enable_item_edit="true"` 与单元格可编辑属性约束；行选择模式下由既有"选中后单击一次"链路进入编辑，`F2` 快捷键仍仅 kCell 模式支持。

**单元格富文本：**

```cpp
ui::ListCtrlSubItemData subItemData;
subItemData.bRichText = true;
subItemData.text = _T("<b>标题</b> <font color=\"#1E90FF\">说明</font>");
pListCtrl->SetSubItemData(itemIndex, columnIndex, subItemData);

// 也可单独设置：
pListCtrl->SetSubItemRichText(0, 2, true);
bool bRichText = pListCtrl->IsSubItemRichText(0, 2); // 返回 true
```

**单元格自动换行（Report 视图）：**

Report 视图的单元格默认 `single_line="true"`（单行 + 末尾省略号）。调用下面的接口可切换为多行（自动换行）：

```cpp
// 第 0 行、第 1 列单元格开启自动换行
pListCtrl->SetSubItemSingleLine(0, 1, false);
bool bSingleLine = pListCtrl->IsSubItemSingleLine(0, 1); // 返回 false

// 按列ID操作
size_t nColumnId = pListCtrl->GetColumnId(1);
pListCtrl->SetSubItemSingleLineById(0, nColumnId, false);
```

说明：
- 仅对 Report 视图有效（List / Icon 视图的换行由视图自身的 `SetTextSingleLine` 控制）；非 Report 视图下 `Set*` 返回 `false`。
- 由于是虚表实现，单元格属性存于数据模型，并在**数据填充时**应用到复用的单元格控件上，滚动复用不会导致属性错乱。
- 自动换行后必须把行高调够，否则超出部分会被裁剪。按需调用 `SetDataItemHeight(itemIndex, height, true)`（逐行）或 `SetDataItemHeight(height, true)`（默认行高）：

```cpp
// 开启换行，并把默认行高调大以容纳多行文本
pListCtrl->SetSubItemSingleLine(0, 1, false);
pListCtrl->SetDataItemHeight(64, true);
```

**单格刷新：**

当只有个别单元格的数据发生变化时（例如进度、状态列），可使用 `RefreshSubItem` 只更新该单元格，避免整行刷新：

```cpp
// 场景：进度/状态类列的数值由后台任务推进，应用层自己维护数值
// 1) 用 SetEnableRefresh(false) 包裹"数据写入"，避免数据变化触发整行自动刷新界面；写完后再恢复
bool bOldRefresh = pListCtrl->SetEnableRefresh(false);
pListCtrl->SetSubItemProgress(0, 1, 42);   // 只写数据，不刷新界面
pListCtrl->SetEnableRefresh(bOldRefresh);

// 2) 恢复刷新后，只刷新这一个单元格（填充回调会把最新数值写入该单元格）
pListCtrl->RefreshSubItem(0, 1);

// 也可按列ID刷新
size_t nColumnId = pListCtrl->GetColumnId(1);
pListCtrl->RefreshSubItemById(0, nColumnId);
```

说明：该接口仅用于 Report 视图。当 `IsEnableRefresh()` 为 `false`（即仍处于 `SetEnableRefresh(false)` 的"禁刷窗口"内）时返回 `false`，因此需先恢复刷新再调用；如果该单元格当前不在可见范围内（未生成对应的 UI 控件），同样返回 `false`，此时数据本身已更新，滚动到可见范围后会显示最新数据。

> 说明：逐格填充事件 `kEventReportViewSubItemFilled` 已移除（触发过于频繁、对性能影响较大），原有功能可使用 `kEventReportViewItemFilled` 替代（`pEventData` 为 `ListCtrlItem*`；`pExtraData` 为本次填充的 `ListCtrlSubItem*`：整行填充时为 `nullptr`，调用 `RefreshSubItem` 单格刷新时指向该单元格；可再通过 `GetSubItem(columnIndex)` 取到单元格）。

**列显示/隐藏：**

```cpp
// 隐藏第 2 列
pListCtrl->SetColumnVisible(1, false);
bool bVisible = pListCtrl->IsColumnVisible(1); // 返回 false

// 按列ID操作
pListCtrl->SetColumnVisibleById(nColumnId, true);
```

**列布局事件（持久化列布局）：**

列宽、列序、列显隐、排序状态变化时均会触发对应事件，应用可在此保存列布局：

```cpp
// 列宽变化：wParam 是列ID，lParam 是关联的另一个列ID（可能为 Box::InvalidIndex）
pListCtrl->AttachColumnWidthChanged([](const ui::EventArgs& args) {
    size_t nColumnId = (size_t)args.wParam;
    // 保存列宽：pListCtrl->GetColumnWidthById(nColumnId);
    return true;
});

// 列顺序变化（拖动表头调整列顺序后触发）
pListCtrl->AttachColumnOrderChanged([](const ui::EventArgs& args) {
    // 通过 GetColumnId(columnIndex) 遍历保存列顺序
    return true;
});

// 列显示/隐藏变化：wParam 是列ID，lParam 是新状态（1显示，0隐藏）
pListCtrl->AttachColumnVisibleChanged([](const ui::EventArgs& args) {
    size_t nColumnId = (size_t)args.wParam;
    bool bVisible = (args.lParam != 0);
    return true;
});

// 列排序变化：wParam 是列ID，lParam 是新状态（1升序，0降序）
pListCtrl->AttachColumnSorted([](const ui::EventArgs& args) {
    size_t nColumnId = (size_t)args.wParam;
    bool bSortedUp = (args.lParam != 0);
    return true;
});
```

**单元格选择模式（C1 / C1b）：**

默认 `selection_mode="row"`（行选择），既有行选择、键盘导航与事件行为完全不变；设为 `cell` 后进入单元格选择模式：

```cpp
// 切换为单元格选择模式
pListCtrl->SetSelectionMode(ui::ListCtrlSelectionMode::kCell);
ui::ListCtrlSelectionMode mode = pListCtrl->GetSelectionMode(); // 返回 kCell

// 当前单元格：单击命中的单元格即为当前格（同时也是区域锚点）
ui::ListCtrlCellId cell = pListCtrl->GetCurrentCell(); // cell.nRowIndex / cell.nColumnId，IsValid() 判断有效性
pListCtrl->SetCurrentCell(0, nColumnId); // 替换语义：清除其它选择，仅选中该格

// 设置 / 取回 / 清除选择区域：anchorCell 为锚点，focusCell 为焦点
ui::ListCtrlCellId anchorCell, focusCell;
pListCtrl->SetCellSelection(anchorCell, focusCell, false); // 第三个参数 true=追加该矩形（不清除已有），false=先清除再设置
pListCtrl->GetCellSelection(anchorCell, focusCell);       // 取回锚点与焦点
pListCtrl->ClearCellSelection(); // 仅在确实存在选择状态（有选中单元格或当前格）时才刷新并派发 kEventCellSelectionChanged；无变化时不派发

// 单元格等价接口（对应行选择的 SetSelectedDataItems / GetSelectedDataItems）
std::vector<ui::ListCtrlCellId> cellList;
pListCtrl->SetSelectedCells(cellList, true); // 批量设置选中单元格，true=先清除其它
pListCtrl->GetSelectedCells(cellList);       // 展开为选中单元格列表

// 查询（供应用自定义绘制 / 编辑判定）
bool bSelected = pListCtrl->IsCellSelected(0, nColumnId); // 该单元格是否被选中
bool bCurrent  = pListCtrl->IsCellCurrent(0, nColumnId);  // 该单元格是否为当前格

// 全选 / 清空：按当前模式分派（cell 模式为单元格，row 模式为行）
pListCtrl->SetSelectAll();
pListCtrl->SetSelectNone(); // cell 模式下复用 ClearCellSelection 的条件派发：无选择时不会派发 kEventCellSelectionChanged

// 事件
pListCtrl->AttachCurrentCellChanged([](const ui::EventArgs& args) {
    size_t nRowIndex = (size_t)args.wParam; // 行的显示索引
    size_t nColumnId = (size_t)args.lParam; // 列ID
    return true;
});
pListCtrl->AttachCellSelectionChanged([](const ui::EventArgs& args) {
    // wParam 为选中区域个数；无选择时 wParam 为 0、pEventData 为 nullptr
    auto* pRangeList = (std::vector<ui::ListCtrlCellRange>*)args.pEventData;
    if (pRangeList != nullptr) {
        // 反序列化后可能得到多项不连续区域，每项含 nRowBegin / nRowEnd / nColumnIdBegin / nColumnIdEnd
        for (const ui::ListCtrlCellRange& range : *pRangeList) {
            if (range.IsValid()) {
                // 使用 range ...
            }
        }
    }
    return true;
});
```

XML 中可直接配置并绑定内联事件：`<ListCtrl class="list_ctrl" selection_mode="cell"><Event type="current_cell_changed" .../><Event type="cell_selection_changed" .../></ListCtrl>`。

交互说明（对齐 Windows 资源管理器）：`cell` 模式下
- 鼠标：单击替换选中该格（清除其它）；`Ctrl+单击`切换该格选中状态（再次点击取消，支持不连续选择）；`Shift+单击`以锚点扩展到该格（替换）；`Ctrl+Shift+单击`以锚点扩展到该格（追加）；按住**左键或右键**拖动：出现与行选择模式**完全一致**的浅蓝半透明矩形框选蒙层（复用 `ListBoxHelper`，同色/同透明度/同 DPI 缩放边框/顶部裁剪到表头下沿），松手后矩形内的单元格被选中（替换），`Ctrl+拖动`为追加（不清除已有）；拖到视口边缘会自动滚动视图（与行选择模式同款）。
- 键盘：方向键移动当前格（单选替换该格）、`Shift+方向键`扩展区域（替换）、`Ctrl+方向键`仅移动焦点不改变选择、`Ctrl+Shift+方向键`追加扩展；`Home`/`End` 到当前行首/末列，`Ctrl+Home`/`Ctrl+End` 到表格首/末格；`PageUp`/`PageDown` 翻页；`Ctrl+A` 全选单元格（仅多选，单选下忽略）；`Ctrl+Space` 切换当前格选中状态（仅多选，与行选择模式一致，单选下忽略）。
- 模式隔离：`cell` 模式下所有行选择入口（鼠标点击行、行 Hover/Pushed 高亮、键盘 `Ctrl+A`/方向键、行框选、`SetSelectedDataItems` 等）均不对行选择生效（鼠标操作不会改动行选择状态，切回 `row` 模式不会出现残留选中行）；空白处单击清空单元格选择。注意：单元格模式下的鼠标框选复用 ListBox 的框选能力，需要 `multi_select == true`（ListCtrl 默认为 `true`）。
- 绘制：选中区域半透明填充、当前格焦点框；冻结列单元格同样参与选择。
- 置顶行：表头与置顶行（不可选行）不参与单元格选择与高亮绘制；鼠标点击/悬停在其上时不会"穿透"选中被其遮挡的下方单元格，单元格高亮也不会绘制到置顶行内容之上。
- 单选语义：`multi_select="false"`（单选）时，`cell` 模式与行模式一致，**至多选中一个单元格** —— `Shift+单击`/`Ctrl+单击` 按普通单击处理、`Shift+方向键` 退化为普通移动当前格、`Ctrl+A`/`Ctrl+Space` 不做任何改变（行选择模式同样不支持 `Ctrl+Space`）、单击后不进入拖动框选状态；`SetSelectedCells` / `SetCellSelection` 只保留第一个（或焦点）单元格；由多选切换到单选时，已选中的多个单元格自动裁剪为保留首格（与行模式的 `OnSwitchToSingleSelect` 对称）。"当前格焦点框"仍然保留，仅把选中集合限制为 1。

完整演示见 ListCtrl 示例程序（表格默认冻结首列，第2列为进度条、第3列为富文本）。

## PropertyGrid 控件（继承 VBox 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| property_grid_xml |   | string | 配置文件XML，如果为空，默认为："public/property_grid/property_grid.xml" |
| row_grid_line_width | 1.0 | float | 横向网格线的宽度 |
| row_grid_line_color |   | string | 横向网格线的颜色 |
| column_grid_line_width | 1.0 | float | 纵向网格线的宽度 |
| column_grid_line_color |   | string | 纵向网格线的颜色 |
| header_class |   | string | 表头的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序 |
| group_class |   | string | 分组的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序 |
| group_label_class |   | string | 分组的文本控件Class |
| property_class |   | string | 属性的Class属性，定义方法请参考`global.xml` 中的对应内容和示例程序 |
| property_name_label_class |   | string | 属性的名称文本控件Class |
| property_value_label_class |   | string | 属性的值文本控件Class |
| left_column_width |   | int | 左侧一列的宽度 |
| property_font_normal |   | string | 设置属性值的字体Id（正常状态） |
| property_font_modified |   | string | 设置属性值的字体Id（已修改状态） |

PropertyGrid 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

## ColorPicker 控件（继承 Window 属性）
ColorPicker是一个窗口，具体用法请参考示例程序中的菜单    
ColorPicker 控件继承了 `Window` 属性，更多可用属性请参考`Window`的属性

## ControlDragable 控件（继承 Control 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| drag_order | true | bool | 是否支持拖动调整顺序（在同一个容器内） |
| drag_alpha | 216 | int | 设置拖动顺序时，控件的透明度（0 - 255） |
| drag_out | true | bool | 是否支持拖出操纵（在相同窗口的不同容器内） |

ControlDragable 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## CefControl 控件（继承 Control 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| url |  | string | 控件创建成功后，导航到此URL网址 |
| url_is_local_file |  | string | url指定的URL网址否为本地文件，如果是本地文件并且指定的是相对路径，则根目录是可执行程序所在目录 |
| F11 | true | bool | 是否允许F11快捷键(页面全屏/页面退出全屏) |
| F12 | true | bool | 是否允许F12快捷键(显示/隐藏开发者工具) |
| download_favicon_image | false | bool | 是否下载网站的FavIcon图标 |

CefControl 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## WebView2Control 控件（继承 Control 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| url |  | string | 控件创建成功后，导航到此URL网址 |
| url_is_local_file |  | string | url指定的URL网址否为本地文件，如果是本地文件并且指定的是相对路径，则根目录是可执行程序所在目录 |
| F11 | true | bool | 是否允许F11快捷键(页面全屏/页面退出全屏) |
| F12 | true | bool | 是否允许F12快捷键(显示/隐藏开发者工具) |
| devtools_enabled | true | bool | 是否允许打开开发者工具 |

WebView2Control 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## IconControl 控件（继承 Control 属性）
IconControl 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

## BitmapControl 控件（继承 Box 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| bitmap_halign  | left | string | 图片的水平对齐方式，可取值: "left"、"center"、"right" |
| bitmap_valign  | top | string | 图片的垂直对齐方式，可取值: "top"、"center"、"bottom" |
| bitmap_alpha  | 255 | int | 图片绘制时的透明度，可取值: 0 - 255 |
| bitmap_dest  |  | rect | 图片绘制目标区域位置和大小(相对于控件区域的位置)|
| bitmap_src  |  | rect | 图片绘制源区域位置和大小|
| bitmap_margin  |  | rect | 绘制目标区域中的外边距(如果指定了dest值，此值无效)|
| bitmap_adaptive_dest_rect  | false | bool | 绘制时是否自动适应目标区域（等比例缩放图片）|
| bitmap_stretch  | false | bool | 绘制时是否拉伸绘制图片（与IsAdaptiveDestRect()互斥，优先级低于IsAdaptiveDestRect()）|
| bitmap_multi_thread  | true | bool | 是否支持多线程操作位图数据（如果无调用，则默认为true，默认是支持多线程操作位图数据的）|

BitmapControl 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## AddressBar 控件（继承 HBox 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| address_path |  | string | 设置路径 |
| path_tooltip | true | bool | 设置是否显示路径的tooltip |
| return_update_ui | true | bool | 设置按回车时自动更新显示控件 |
| esc_update_ui | true | bool | 设置按ESC时自动更新显示控件 |
| kill_focus_update_ui | true | bool | 设置失去焦点时自动更新显示控件 |
| rich_edit_class | "address_bar_edit" | string | 设置编辑框的Class |
| rich_edit_clear_btn_class | "rich_edit_clear_btn" | string | 设置编辑框的清除按钮Class |
| sub_path_hbox_class | "address_bar_sub_path_hbox"| string | 设置地址栏路径的容器（HBox）Class，每个子路径一个HBox容器 |
| sub_path_button_class | "address_bar_sub_path_button"| string | 设置地址栏子路径按钮的Class |
| sub_path_root_class | "address_bar_sub_path_root" | string | 设置地址栏根路径的Class（"/"路径） |
| path_separator_class | "address_bar_path_separator" | string | 设置地址栏路径分隔符的Class |

AddressBar 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## ChildWindow 控件（继承 Box 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| child_window_margin |   | UiMargin | 设置子窗口的外边距，外边距的空间可以放置其他控件 |

ChildWindow 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## StatusBar 控件（继承 HBox 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| separator_width | 1 | int | 相邻状态面板之间分隔线的宽度（像素，0 表示不绘制分隔线）|
| separator_color | "border_window" | string | 分隔线颜色（语义色名或颜色值）|
| size_grip | false | bool | 是否绘制右下角的尺寸手柄|
| size_grip_resize | true | bool | 尺寸手柄是否参与窗口拖拽缩放：true 时若状态栏停靠在窗口客户区右下角，会自动把窗口右下角可拉伸区域（size_box 的 right/bottom）扩大到覆盖手柄，从而能真正拖拽调整窗口大小；false 时手柄仅作视觉提示。注意该效果是窗口级的，扩大后窗口底边一带也会成为可拉伸区域。前提是窗口本身具备可缩放能力：Windows 默认具备；SDL 等平台需在 XML 中给 Window 声明非零 `size_box`|
| size_grip_color | "border_window" | string | 尺寸手柄颜色（语义色名或颜色值）|

StatusBar 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

状态栏用于显示光标位置、字数、连接状态、后台进度等状态提示，是编辑器 / IDE / 通信类软件的常用界面元素。
StatusBar 是一个水平容器（继承 `HBox`），其中的每个子控件就是一个"状态面板"（通常为 `Label` / `IconControl` / `Progress` 等）：
面板可以设置固定宽度、拉伸（width="stretch"）等，沿用 `HBox` 的全部布局能力；相邻面板之间会自动绘制竖直分隔线，
可通过 `separator_width` 关闭或调整宽度、通过 `separator_color` 调整颜色；右下角可选绘制尺寸手柄（`size_grip`）。

状态面板支持**按索引访问**，索引从 0 开始、按布局顺序编号，且只统计可见面板（不可见面板不占索引），与视觉上看到的分格顺序一致。
业务代码无需再 `FindControl` + `dynamic_cast` 找子 `Label`，可直接用索引读写面板内容；面板内容变化（增删子控件、改可见性）后索引会重新编号。

XML 示例：
```xml
<StatusBar name="status_bar" width="stretch" height="30" bkcolor="bg_window_card" border_size="0,1,0,0" bordercolor="border_window" size_grip="true">
    <Label name="sb_state" text="就绪" width="110" valign="center" padding="10,0,10,0"/>
    <Label name="sb_conn" text="已连接" width="140" valign="center" padding="10,0,10,0"/>
    <Progress name="sb_progress" width="stretch" min="0" max="100" value="60" valign="center" bkcolor="bg_progress_track" progress_color="bg_progress"/>
</StatusBar>
```

对应的 C++ 接口：
```cpp
// 分隔线
void SetSeparatorWidth(int32_t nWidth);      // 分隔线宽度（0 表示不绘制）
void SetSeparatorColor(const DString& strColor);

// 尺寸手柄
void SetShowSizeGrip(bool bShow);            // 右下角尺寸手柄
void SetSizeGripResize(bool bResize);        // 手柄是否参与窗口拖拽缩放（默认 true）
bool  IsSizeGripResize() const;
void SetSizeGripColor(const DString& strColor);

// 状态面板（分格）索引接口
size_t   GetPanelCount() const;                          // 面板数量（不含不可见面板）
Control* GetPanel(size_t nIndex) const;                  // 按索引取面板，越界返回 nullptr
int32_t  GetPanelIndex(Control* pPanel) const;           // 取面板索引，不是面板返回 -1
Control* FindPanel(const DString& strName) const;        // 按 name 找面板
bool     SetPanelText(size_t nIndex, const DString& strText); // 设面板文本（需 Label/RichEdit 类）
DString  GetPanelText(size_t nIndex) const;              // 取面板文本
```

分格索引用法示例：
```cpp
ui::StatusBar* pStatusBar = dynamic_cast<ui::StatusBar*>(FindControl(_T("status_bar")));
if (pStatusBar != nullptr) {
    //第 0 格是"状态"面板，直接按索引设置文本，无需 FindControl 找子 Label
    pStatusBar->SetPanelText(0, _T("状态：忙碌"));

    //回读与反查
    DString strText = pStatusBar->GetPanelText(0);
    ui::Control* pPanel = pStatusBar->GetPanel(0);
    int32_t nIndex = pStatusBar->GetPanelIndex(pPanel);   // 结果为 0
}
```

## ToolBar 控件（继承 HBox 属性）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| overflow | true | bool | 是否启用溢出收纳（可用宽度不足时自动隐藏尾部子控件）|
| overflow_button_class |  | string | 右端"更多"按钮的样式 Class 名称（引用 global.xml 中的样式，为空表示不设置样式）|
| overflow_button_width | 0 | int | "更多"按钮的宽度（像素，0 表示按内容自适应）|
| overflow_button_text | ">>" | string | "更多"按钮显示的文本 |
| tool_class |  | string | 动态创建工具按钮时使用的默认样式 Class |
| tool_checkbox_class |  | string | 动态创建勾选类工具项时使用的默认样式 Class（为空时回退 `tool_class`）|
| tool_separator_class |  | string | 动态创建分隔线时使用的默认样式 Class |

ToolBar 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

工具条用于承载一排工具按钮，是编辑器 / IDE / 办公类软件的常用壳层元素。
ToolBar 是一个水平容器（继承 `HBox`），子控件水平排列，沿用 `HBox` 的全部布局能力；工具条中的分隔线可直接使用 `Line` 控件（`vertical="true"`）。
当可用宽度不足、尾部子控件放不下时，会自动隐藏这些子控件，并在右端显示一个"更多"按钮（默认文本 `>>`）：
点击该按钮会弹出浮层，列出被收纳的子控件；点击浮层中的项会触发 `AttachOverflowItemActivated` 注册的回调（参数为被点击子控件的 `name`），
便于应用层派发命令。窗口宽度变化时会自动重新计算收纳，宽度足够时被收纳的项会自动恢复显示。

### 工具项管理

除在 XML 中直接写 `Button` / `CheckBox` / `Line` 外，ToolBar 还提供工具项管理 API，适合动态创建与增删：

- `AddToolButton` / `AddToolCheckBox` / `AddToolSeparator` / `AddTool` / `InsertTool`：创建并加入工具项；
- `RemoveTool` / `RemoveToolAt` / `RemoveAllTools`：删除工具项（控件随之销毁）；
- `FindTool` / `GetToolAt` / `GetToolCount` / `GetToolIndex`：按名称或索引查询；
- `SetToolChecked` / `IsToolChecked`、`SetToolEnabled` / `IsToolEnabled`、`SetToolVisible` / `IsToolVisible`：状态控制。

约定：

- `name` 为空时自动生成 `tool_1`、`tool_2`……；与已有工具项重名时添加失败；
- 索引类接口（`GetToolAt` / `RemoveToolAt` / `GetToolIndex`）只统计工具项，**不含内部"更多"按钮**；
- 勾选类工具项使用 `CheckBox` 的**选中态**（`IsSelected` / `Selected`）表示勾选，而不是勾选图标模式；
- 通过 `AddTool*` 创建的工具项、以及 XML 中静态写的 `Button` / `CheckBox` 都会自动绑定工具项事件。

工具项事件：

```cpp
//工具项被点击（直接点击或从"更多"浮层激活），参数为工具项的 name
void AttachToolItemClick(ToolItemClickEvent callback);
void ClearToolItemClick();

//勾选类工具项的勾选状态变化，参数为工具项的 name 与变化后的状态
void AttachToolItemCheckedChanged(ToolItemCheckedChangedEvent callback);
void ClearToolItemCheckedChanged();
```

从"更多"浮层激活勾选类工具项时，会先切换其选中态，再派发点击与勾选变化回调，随后仍触发 `OverflowItemActivated`。

XML 示例：
```xml
<ToolBar name="main_toolbar" width="stretch" height="36" padding="8,0,8,0" child_margin_x="4"
         overflow_button_class="btn_global" overflow_button_width="28" overflow_button_text="&#187;">
    <Button class="btn_global" name="tb_new" text="新建" width="64" height="28"/>
    <Button class="btn_global" name="tb_open" text="打开" width="64" height="28"/>
    <Line vertical="true" line_color="border_window" line_width="1" width="9" height="18" valign="center"/>
    <Button class="btn_global" name="tb_save" text="保存" width="64" height="28"/>
</ToolBar>
```

动态创建示例：
```xml
<ToolBar name="dynamic_toolbar" width="stretch" height="30"
         tool_class="toolbar_button" tool_checkbox_class="toolbar_check"
         tool_separator_class="toolbar_separator"/>
```

```cpp
ui::ToolBar* pToolBar = dynamic_cast<ui::ToolBar*>(FindControl(_T("dynamic_toolbar")));
pToolBar->AttachToolItemClick([](const DString& itemName) {
    //itemName 即工具项的 name
});
pToolBar->AttachToolItemCheckedChanged([](const DString& itemName, bool bChecked) {
    //勾选状态变化
});
pToolBar->AddToolButton(_T("tb_bold"), _T("加粗"));
pToolBar->AddToolCheckBox(_T("tb_italic"), _T("斜体"), false);
pToolBar->AddToolSeparator();
pToolBar->SetToolChecked(_T("tb_italic"), true);       //不触发回调
pToolBar->RemoveTool(_T("tb_bold"));                   //删除并销毁控件
```

对应的 C++ 接口：
```cpp
//工具项管理与查询
Button* AddToolButton(const DString& strName, const DString& strText,
                      const DString& strClass = DString(), bool bTextId = false);
CheckBox* AddToolCheckBox(const DString& strName, const DString& strText,
                          bool bChecked = false, const DString& strClass = DString(),
                          bool bTextId = false);
Line* AddToolSeparator(const DString& strName = DString(), const DString& strClass = DString());
bool AddTool(Control* pControl, const DString& strName = DString());
bool InsertTool(size_t nIndex, Control* pControl, const DString& strName = DString());

size_t GetToolCount() const;                           //工具项数量（不含"更多"按钮）
Control* GetToolAt(size_t nIndex) const;               //按索引获取工具项
Control* FindTool(const DString& strName) const;       //按名称查找
size_t GetToolIndex(const DString& strName) const;     //按名称取索引，找不到返回 Box::InvalidIndex
bool HasTool(const DString& strName) const;

bool RemoveTool(const DString& strName);               //删除并销毁控件
bool RemoveToolAt(size_t nIndex);
void RemoveAllTools();

//工具项状态
bool SetToolChecked(const DString& strName, bool bChecked, bool bTriggerEvent = false);
bool IsToolChecked(const DString& strName) const;
bool SetToolEnabled(const DString& strName, bool bEnabled);
bool IsToolEnabled(const DString& strName) const;
bool SetToolVisible(const DString& strName, bool bVisible);
bool IsToolVisible(const DString& strName) const;

//工具项事件
void AttachToolItemClick(ToolItemClickEvent callback);
void ClearToolItemClick();
void AttachToolItemCheckedChanged(ToolItemCheckedChangedEvent callback);
void ClearToolItemCheckedChanged();

//默认样式 Class
void SetToolClass(const DString& strClass);
void SetToolCheckBoxClass(const DString& strClass);
void SetToolSeparatorClass(const DString& strClass);

//溢出项被激活（点击浮层中的项）时的回调（参数为被点击子控件的 name）
void AttachOverflowItemActivated(OverflowItemActivatedEvent callback);
void ClearOverflowItemActivated();

void SetOverflowEnabled(bool bEnabled);                //是否启用溢出收纳（默认启用）
bool IsOverflowEnabled() const;
void SetOverflowButtonClass(const DString& strClass);  //"更多"按钮的样式 Class
DString GetOverflowButtonClass() const;
void SetOverflowButtonWidth(int32_t nWidth);           //"更多"按钮宽度（0 表示自适应）
int32_t GetOverflowButtonWidth() const;
void SetOverflowButtonText(const DString& strText);    //"更多"按钮显示的文本
DString GetOverflowButtonText() const;

bool IsOverflowing() const;                            //当前是否存在被自动收纳的子控件
```

## NavigationView 控件（继承 HBox 属性）

NavigationView 是侧边栏导航容器，内部组合"左侧导航窗格 + 右侧内容区（页头 + TabBox 页面）"：
窗格含汉堡按钮、窗格标题、可纵向滚动的导航项列表与底部可选的设置项；点击导航项会按 `page` 名联动切换内容页并更新页头；
汉堡按钮在展开/紧凑两态间切换（紧凑态仅显示图标，分组标题/分隔线隐藏，导航项以 tooltip 显示完整文字）。
适用于设置页、桌面客户端等"多页面 + 侧边导航"场景。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| pane_width | 220 | int | 展开态窗格宽度（像素，DPI 自适应；仅接受正值，<=0 保持默认值）|
| compact_pane_width | 48 | int | 紧凑态窗格宽度（像素，DPI 自适应；仅接受正值，<=0 保持默认值）|
| pane_title |  | string | 窗格标题文字（紧凑态下标题自动隐藏）|
| pane_title_id |  | string | 窗格标题文字 ID（多语言，优先于 pane_title）|
| toggle_icon |  | string | 汉堡按钮图标图片属性串，如 file='nav/hamburger.svg' width='16' height='16' |
| collapsed | false | bool | 初始是否为紧凑态（true=仅显示图标）|
| show_header | true | bool | 是否显示内容区页头（显示当前选中项文字）|
| settings_item | false | bool | 是否在窗格底部显示设置项（不随导航列表滚动，不参与序号统计）|
| settings_page | settings | string | 设置项关联的内容页 name |
| settings_text_id |  | string | 设置项文字 ID（多语言，由使用方提供）|
| settings_icon |  | string | 设置项图标图片属性串（由使用方提供）|
| selected_id |  | string | 初始选中项对应的 page name；为空或找不到时选中第一个可选项（不含设置项）|

NavigationView 继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

### 直接子节点的分流规则

NavigationView 重写了 `AddItem`，直接子节点按类型分流：

- `NavigationViewItem`：进入左侧窗格的导航项列表（header/separator 同样进入列表，仅渲染形态不同）；
- `TabBox`：接管为内容区页面容器（仅允许一个；显式提供便于外部直接操控页面切换）；
- 其他控件：作为内容页进入内容区；未显式提供 TabBox 时自动创建一个。

导航项与内容页按 `name` 联动：`NavigationViewItem` 的 `page` 属性写内容页控件的 `name`，
选中该导航项时内容区 TabBox 自动切换到对应页面，并同步页头文字。
**序号约定**：`GetSelectedIndex` 等接口返回的序号仅统计 item 形态的可选项（不含 header/separator 与底部设置项），
无选中时返回 `Box::InvalidIndex`。

### NavigationViewItem 控件（继承 Control 属性）

`NavigationViewItem` 是导航项控件，作为 NavigationView 的直接子节点使用，自绘"图标 + 文字"行，支持三种形态：

- `item`（默认）：可点击导航项，选中时灰底圆角热区 + 强调色文字；
- `header`：分组标题（小号、弱化色，不可点击，紧凑态下隐藏）；
- `separator`：分隔线（紧凑态下隐藏）。

各形态使用固定高度（DIP，DPI 自适应）：item 36、header 32、separator 9。
支持键盘导航：item 形态可通过 `Tab` 聚焦，上/下方向键在可选项之间循环移动焦点，回车/空格激活当前项；
禁用态（`enabled="false"`）不响应点击与键盘，文字与图标弱化；禁用当前选中项会取消其选中态并清空页头（不会自动切换到其它项）。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text |  | string | 项文字 |
| text_id |  | string | 项文字 ID（多语言，优先于 text）|
| icon |  | string | 图标图片属性串，如 file='nav/home.svg' width='16' height='16' |
| page |  | string | 关联的内容页控件 name（与内容区页面联动）|
| item_type | item | string | 形态：item（默认，可点击导航项）/ header（分组标题，兼容 group_header）/ separator（分隔线，兼容 sep）|
| icon_size | 16 | int | 图标尺寸（DIP，DPI 自适应；仅接受正值）|
| icon_left | 16 | int | 展开态图标左边距（DIP）|
| text_left | 44 | int | 展开态文字左边距（DIP）|
| text_right | 8 | int | 展开态文字右边距（DIP）|
| pill_radius | 6 | int | 选中/悬停热区圆角半径（DIP）|
| pill_inset_x | 6 | int | 热区水平内缩量（DIP）|
| pill_inset_y | 3 | int | 热区垂直内缩量（DIP）|
| selected_bg_color | bg_list_item_selected | string | 选中背景色名 |
| hovered_bg_color | bg_list_item_hovered | string | 悬停背景色名 |
| selected_text_color | color_accent | string | 选中文字色名 |
| normal_text_color | text_default | string | 普通文字色名 |
| disabled_text_color | text_disabled | string | 禁用文字色名 |
| header_text_color | text_muted | string | 分组标题文字色名 |
| separator_color | border_window | string | 分隔线颜色名 |

NavigationViewItem 继承了 `Control` 属性，更多可用属性请参考`Control`的属性

### XML 使用示例

```xml
<NavigationView name="nav" height="stretch"
                pane_width="220" compact_pane_width="48"
                pane_title_id="STRID_NAV_DEMO_TITLE" show_header="true"
                toggle_icon="file='nav/hamburger.svg' width='16' height='16'"
                settings_item="true" settings_page="page_settings"
                settings_text_id="STRID_NAV_SETTINGS"
                settings_icon="file='nav/settings.svg' width='16' height='16'"
                selected_id="page_home">
    <!-- 可点击导航项：page 与内容页 name 对应，点击即联动切换 -->
    <NavigationViewItem text_id="STRID_NAV_PAGE_HOME"
                        icon="file='nav/home.svg' width='16' height='16'"
                        page="page_home"/>
    <NavigationViewItem text_id="STRID_NAV_PAGE_DASHBOARD"
                        icon="file='nav/dashboard.svg' width='16' height='16'"
                        page="page_dashboard"/>

    <!-- 分组标题与分隔线：不可点击，不参与序号统计 -->
    <NavigationViewItem item_type="header" text_id="STRID_NAV_GROUP_WORKSPACE"/>
    <NavigationViewItem item_type="separator"/>

    <!-- 内容区：显式 TabBox 承载所有页面，便于外部直接操控页面容器 -->
    <TabBox name="nav_content">
        <VBox name="page_home" bkcolor="bg_window_main">
            <Label text_id="STRID_NAV_PAGE_HOME" font="system_bold_16" text_color="text_default"/>
        </VBox>
        <VBox name="page_dashboard" bkcolor="bg_window_main">
            <Label text_id="STRID_NAV_PAGE_DASHBOARD" font="system_bold_16" text_color="text_default"/>
        </VBox>
        <VBox name="page_settings" bkcolor="bg_window_main">
            <Label text_id="STRID_NAV_SETTINGS" font="system_bold_16" text_color="text_default"/>
        </VBox>
    </TabBox>
</NavigationView>
```

完整演示可参考 controls 示例程序中的"导航测试"按钮（`bin/resources/themes/default/controls/navigation.xml`）：
含 11 个可点击导航项、2 个分组标题、2 条分隔线与底部设置项，内容区为显式 TabBox 多页面，并演示动态添加/移除导航项与程序化切换。

### C++ 使用示例

```cpp
ui::NavigationView* pNav = dynamic_cast<ui::NavigationView*>(FindControl(_T("nav")));

//事件监听（均由 NavigationView 自身作为发送者触发）
//选中变化：wParam 新序号，lParam 旧序号（无选中时为 Box::InvalidIndex）
pNav->AttachNavSelectionChanged([](const ui::EventArgs& args) {
    size_t nNewIndex = (size_t)args.wParam;
    size_t nOldIndex = (size_t)args.lParam;
    return true;
});
//重复点击已选中项也会触发（适合"刷新/回顶"类需求）：wParam 为可选项序号
pNav->AttachNavItemClick([](const ui::EventArgs& args) {
    size_t nIndex = (size_t)args.wParam;
    return true;
});
//窗格即将收起/展开：wParam 为目标状态（1=收起，0=展开），回调返回 false 可取消本次切换
pNav->AttachNavPaneToggling([](const ui::EventArgs& args) {
    return true;
});
//窗格收起/展开完成：wParam 为当前状态（1=收起，0=展开）
pNav->AttachNavPaneToggled([](const ui::EventArgs& args) {
    return true;
});

//程序化选中：按内容页 name / 按可选项序号 / 按控件指针
pNav->SelectItem(_T("page_tools"));
pNav->SelectItemByIndex(0);

//回读当前选中状态
size_t nIndex = pNav->GetSelectedIndex();       //无选中返回 Box::InvalidIndex
DString strPage = pNav->GetSelectedPage();

//展开/收起导航窗格
pNav->SetCollapsed(!pNav->IsCollapsed());

//动态追加导航项：第 4 参传入页面控件时，自动挂到内容区并与导航项按 name 联动
ui::VBox* pPage = new ui::VBox(pNav->GetWindow());
pPage->SetName(_T("page_dynamic"));
ui::NavigationViewItem* pItem = pNav->AddNavItem(DString(),
                                                 _T("file='nav/star.svg' width='16' height='16'"),
                                                 _T("page_dynamic"), pPage);
if (pItem != nullptr) {
    pItem->SetItemText(_T("动态项"));
    pNav->SelectItem(pItem);   //立即选中新建项
}

//移除导航项：第 2 参 true 时一并移除关联内容页。
//注意：容器默认 auto_destroy_child=true，移除后指针即失效，不要继续使用
pNav->RemoveNavItem(pItem, true);
```

### 事件

| 事件 | 参数(wParam) | 参数(lParam) | 说明 |
| :--- | :--- | :--- | :--- |
| kEventNavigationItemClick | 可选项序号 |  | 点击可选择导航项时触发，重复点击已选中项也会触发 |
| kEventNavigationSelectionChanged | 新序号 | 旧序号 | 选中项发生变化时触发，无选中时为 Box::InvalidIndex |
| kEventNavigationPaneToggling | 目标状态：1=收起，0=展开 |  | 窗格即将收起/展开时触发，回调返回 false 可取消本次切换 |
| kEventNavigationPaneToggled | 当前状态：1=收起，0=展开 |  | 窗格收起/展开完成时触发 |

主要 C++ 接口：

```cpp
//选中相关
bool SelectItem(NavigationViewItem* pItem, bool bFireEvent = true);  //按控件指针选中
bool SelectItem(const DString& strPageName);                          //按内容页 name 选中
bool SelectItemByIndex(size_t nIndex);                                //按可选项序号选中
NavigationViewItem* GetSelectedItem() const;                          //当前选中项（无选中返回 nullptr）
size_t GetSelectedIndex() const;                                      //当前选中项序号（无选中返回 Box::InvalidIndex）
DString GetSelectedPage() const;                                      //当前选中项关联的内容页 name

//窗格状态
void SetCollapsed(bool bCollapsed);   //展开/收起导航窗格
bool IsCollapsed() const;

//事件监听（等价于 AttachEvent，事件参数见上表）
void AttachNavItemClick(const EventCallback& callback, EventCallbackID callbackID = 0);
void AttachNavSelectionChanged(const EventCallback& callback, EventCallbackID callbackID = 0);
void AttachNavPaneToggling(const EventCallback& callback, EventCallbackID callbackID = 0);  //返回 false 可取消
void AttachNavPaneToggled(const EventCallback& callback, EventCallbackID callbackID = 0);

//动态增删导航项
NavigationViewItem* AddNavItem(const DString& strTextId, const DString& strIcon,
                               const DString& strPage, Control* pPage = nullptr);
bool AddPage(Control* pPage);                                            //单独把页面加入内容区
bool RemoveNavItem(NavigationViewItem* pItem, bool bRemovePage = false); //一并移除关联内容页
void RemoveAllNavItems();                                                //清空导航项（不移除内容页）
void SetItemEnabled(NavigationViewItem* pItem, bool bEnabled);           //禁用时不响应点击/键盘
```

NavigationViewItem 的常用接口：

```cpp
void SetItemText(const DString& strText);        DString GetItemText() const;
void SetItemTextId(const DString& strTextId);    DString GetItemTextId() const;
DString GetDisplayText() const;                  //text_id 优先解析为当前语言文本
void SetIconAttr(const DString& strIcon);        DString GetIconAttr() const;
void SetPageName(const DString& strPage);        DString GetPageName() const;
void SetItemType(ItemType itemType);             ItemType GetItemType() const;  //kItem/kHeader/kSeparator
bool IsSelectable() const;                       //是否可点击选择（仅 item 形态）
void SetSelected(bool bSelected);                bool IsSelected() const;       //选中态由 NavigationView 管理
void SetCompact(bool bCompact);                  bool IsCompact() const;        //紧凑态由 NavigationView 统一切换
```
