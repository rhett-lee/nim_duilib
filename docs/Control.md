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

- **`valign` 只能填 `top` / `center` / `bottom`**，`vcenter` 只在 `text_align` 属性里有效。填错值时 `Control::SetAttribute` 末尾是 `else { ASSERT(0); }`，debug 版直接崩溃。`halign` 同理只能填 `left` / `center` / `right`。
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
| edit_format | | string | 编辑状态时，日期的编辑格式，可选值："date_calendar"：年-月-日，通过下拉框展示月日历的方式来修改日期；"date_up_down"： 编辑时显示：年-月-日，通过控件的右侧放置一个向上-向下的控件以修改日期；"date_time_up_down"：编辑时显示：年-月-日 时:分:秒；"date_minute_up_down"：编辑时显示：年-月-日 时:分；"time_up_down"：编辑时显示：时:分:秒；"minute_up_down"：编辑时显示：时:分|
| spin_class | | string | 日期中的Spin控件的Class属性，仅当使用SDL时有效，默认值为："rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down" |
| current_time | | bool | 初始化为当前本地时间（`"true"` 或 `"1"` 时生效），方便使用，无需在代码中手动调用 `InitLocalTime()`；与 `text` 属性同时使用时，`current_time` 优先 |

DateTime 控件继承了 `Label` 属性，更多可用属性请参考`Label`的属性

## Calendar的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| calendar_mode | "single" | string | 选择模式："single"单选，"range"范围选择 |
| first_day_of_week | "monday" | string | 每周第一天："monday"周一，"sunday"周日 |
| min_date | | string | 最小可选日期（格式：yyyy-mm-dd），空表示不限制 |
| max_date | | string | 最大可选日期（格式：yyyy-mm-dd），空表示不限制 |

Calendar 控件继承了 `Control` 属性，更多可用属性请参考`Control`的属性

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
- 焦点移动与翻页都会收敛到 `min_date`/`max_date` 限制范围内，焦点环不会落在禁用日期上；翻到最早/最晚可选周期边界后不再继续前翻/后翻。
- 范围模式的键盘扩展与鼠标拖拽共用同一套起止逻辑与实时预览绘制。

## CalendarFlyout日历浮层
CalendarFlyout是基于Flyout承载Calendar控件的日期选择浮层，对应头文件`duilib/Control/CalendarFlyout.h`。用于在锚点控件周围弹出日历面板，支持单选/范围选择、月/年/十年三级导航。

核心特性：
- 继承Flyout：自动获得8方位弹出、空间不足自动翻转、点击外部/Esc关闭等能力；
- 内嵌XML布局：头部导航（上一月/标题/下一月）+ Calendar + 底部按钮（今天/清除）；
- 单选模式：点击日期后自动关闭并触发`AttachDateSelected`回调；
- 范围模式：拖拽选择起止日期后自动关闭并触发`AttachDateSelected`回调；
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
```

DateTime控件（`edit_format="date_calendar"`）已内置使用CalendarFlyout，无需手动创建。

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
除RichEdit的通用属性（`spin_class`、`min_number`、`max_number`、`limit_text`等）外，新增以下属性：    

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| step | 1 | int | 步长值（正整数），步进按钮和上下方向键每次调整的数值 |
| value | 0 | int64 | 初始数值（超出min/max范围时会被修正到边界值） |

对应C++接口：`SetStep/GetStep`、`SetValue/GetValue`、`SetRange(min, max)`。    
数值变化时触发`kEventTextChanged`事件（与RichEdit一致）。    
使用示例（参考 rich_edit 示例程序）：    

```xml
<SpinBox class="simple simple_border rich_edit_spin" min_number="0" max_number="100" step="5" value="50"/>
```

其中`spin_class`样式（rich_edit_spin_box / rich_edit_spin_btn_up / rich_edit_spin_btn_down）在 global.xml 的`rich_edit_spin`类中定义。    
注意：需设置 `spin_class` 属性才会显示步进按钮；否则只有上下方向键能调整数值。    

## SearchBox的属性
SearchBox是搜索框组合控件，继承自HBox，内部组合了左侧搜索图标、文本编辑框（RichEdit）与右侧清除按钮，用于关键词输入与搜索场景。对应头文件`duilib/Control/SearchBox.h`。

| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| text | "" | string | 搜索框初始文本 |
| text_id | "" | string | 搜索框初始文本的多语言字符串ID（与text同时设置时，后设置的属性生效） |
| prompt_text | "" | string | 编辑框为空时显示的占位提示文本 |
| prompt_text_id | "" | string | 占位提示文本的多语言字符串ID |
| prompt_color | "" | string | 占位提示文本的颜色（语义色名） |

对应C++接口：`SetSearchText/GetSearchText`、`GetEditControl()`（获取内部RichEdit控件）。
文本变化时触发`kEventTextChanged`事件；按下回车键时触发`kEventReturn`事件（可用于发起搜索）。
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
| items | "" | string | 初始候选数据源，多个候选项以分号`;`分隔（也可通过代码`SetItems`设置） |
| prompt_text | "" | string | 编辑框为空时显示的占位提示文本 |
| prompt_text_id | "" | string | 占位提示文本的多语言字符串ID |
| prompt_color | "" | string | 占位提示文本的颜色（语义色名） |
| min_chars | 1 | int | 输入达到多少个字符后开始联想 |
| max_visible | 8 | int | 浮层最多可见候选行数，超出显示滚动条 |
| match_mode | prefix | string | 文本匹配模式：`prefix`前缀匹配（候选项以输入文本开头）；`contains`包含匹配（任意位置包含） |
| ignore_case | true | bool | 匹配时是否忽略大小写 |

对应C++接口：`SetItems`/`GetItems`（设置/获取完整候选数据源）、`AddItem`（追加候选项，相同文本自动去重）、`ClearItems`、`SetText`/`GetText`（程序化设置文本不会弹出联想浮层）、`SetMinChars`/`GetMinChars`、`SetMaxVisible`/`GetMaxVisible`、`SetMatchMode`/`GetMatchMode`（参数为`AutoCompleteMatchMode::kPrefix/kContains`）、`SetIgnoreCase`/`IsIgnoreCase`、`GetEditControl`（获取内部RichEdit）、`SetFocus`（焦点设置到内部编辑框）。

事件：
- `kEventAutoCompleteSelect`：选中候选项（鼠标点击或键盘回车）后触发，WPARAM为选中项在当前候选列表中的序号（从0开始），XML内联事件名为`select`；
- `kEventTextChanged`：内部编辑框文本变化时转发触发（程序化回填不触发）。

使用示例（参考 controls 示例程序）：

```xml
<!-- 前缀匹配，18个城市候选，输入 b/sh 等前缀即联想 -->
<AutoComplete class="auto_complete" name="auto_complete"
              items="Beijing;Berlin;Budapest;Bangkok;Boston;Brisbane;Chicago;London"
              prompt_text="输入城市名前缀，如 b / sh"/>

<!-- 包含匹配，最多显示6行；数据源也可完全由代码 SetItems 提供 -->
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

对应C++接口：`SetChartType/GetChartType`、`SetTitle`、`SetXAxisTitle`、`SetYAxisTitle`、`SetTitleId`、`SetXAxisTitleId`、`SetYAxisTitleId`、`SetData/AddData/ClearData/GetData`、`SetSeriesData/AddSeries/GetSeries/GetSeriesCount`、`SetDataLabels/GetDataLabels`、`SetDataLabelsId`、`SetSeriesColor`、`SetAxisColor`、`SetLabelColor`、`SetShowValue`、`SetShowGrid`、`SetShowAxisValues`、`SetAxisDivisions`、`SetLegendVisible`、`SetLineWidth`、`SetLineMode`、`SetAreaFill`、`SetShowDataPoints`、`SetBarMode`、`SetDonut`、`SetShowPercent`、`SetBarRadius`、`SetBarGradient`、`SetLineGlow`、`SetAnimationEnabled`、`SetAnimationDuration`；交互：`SetEnableTooltip/IsEnableTooltip`、`SetEnableSelect/IsEnableSelect`、`GetSelected/ClearSelection`、`AttachPointClick/AttachPointHover`、`GetHitFromEvent`。

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

ListCtrl 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性    
ListCtrl 控件的各个视图继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性设置:[Box.md](Box.md)，视图的属性需要在`global.xml` 中设置。

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

