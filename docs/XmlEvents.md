## XML文件中直接响应控件事件的说明文档
### 一、使用示例    
在XML文件中，支持直接写控件事件的响应代码，看几个使用示例。    
示例1：`examples/layout`示例程序中，以下XML文件片段，演示如何使用Event标签在Option标签上挂载事件，实现容器的对齐方式设置：    
```xml
<HBox height="28">            
    <Label text="容器内容水平对齐方式：" valign="center"/>
    <Option class="option_1" group="gridbox_group_halign" text="靠左对齐(left)" height="28" valign="center" margin="8,0,0,0" selected="false">
        <Event type="select" receiver="gridbox_test" apply_attribute="child_halign='left'" />
    </Option>
    <Option class="option_1" group="gridbox_group_halign" text="居中对齐(center)" height="28" valign="center" margin="8,0,0,0" selected="true">
        <Event type="select" receiver="gridbox_test" apply_attribute="child_halign='center'" />
    </Option>
    <Option class="option_1" group="gridbox_group_halign" text="靠右对齐(right)" height="28" valign="center" margin="8,0,0,0" selected="false">
        <Event type="select" receiver="gridbox_test" apply_attribute="child_halign='right'" />
    </Option>
</HBox>
<HBox child_margin="2">
    <GridBox name="gridbox_test" padding="8,8,8,8" width="70%" height="100%"
             grid_width="120" grid_height="90" rows="0" columns="0" scale_down="false"
             child_margin_x="1" child_margin_y="1" child_halign="center" child_valign="top" 
             bkcolor="gray" border_size="1" border_color="red">
    </GridBox>
</HBox>
```
示例2：`examples/render`示例程序中，以下XML文件片段，演示如何使用Event标签在Option标签上挂载事件，实现窗口的阴影效果切换：    
```xml
<VBox>
    <Label text="窗口阴影的功能测试：" valign="center" margin="12,30,10,0"/>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：default, 默认阴影" selected="true" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={default}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：big, 阴影(大)，直角（适合普通窗口）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={big}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：big_round, 阴影(大)，圆角（适合普通窗口）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={big_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：small, 阴影(小)，直角（适合普通窗口）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={small}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：small_round, 阴影(小)，圆角（适合普通窗口）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={small_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：menu, 阴影(小)，直角（适合弹出式窗口，比如菜单等）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={menu}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：menu_round, 阴影(小)，圆角（适合弹出式窗口，比如菜单等）" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={menu_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：none, 无阴影, 有边框, 直角" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={none}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="阴影类型：none_round, 无阴影, 有边框, 圆角" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={none_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="关闭阴影功能" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_attached={false}" />
    </Option>
</VBox>
```
示例3：`examples/render`示例程序中，以下XML文件片段，演示如何使用Event标签在Control标签上挂载事件，实现鼠标在图片上时播放动画：    
```xml
<VBox width="200" height="220">
    <Control width="100%" height="100%" bkimage="file='fan.gif' width='190' height='190' auto_play='false' play_count='-1' valign='center' halign='center'">
        <Event type="mouse_enter" receiver="" apply_attribute="start_image_animation={}" />
        <Event type="mouse_leave" receiver="" apply_attribute="stop_image_animation={}" />
    </Control>
    <Label width="100%" height="20" text="鼠标在图片上时播放动画" text_align="center" margin="8"/>
</VBox>
```

### 二、使用方法    
1. 在控件/容器的标签下，添加形如    
`<Event type="event_name" receiver="receiver_name" apply_attribute="attribute_name='attribute_value'"/>` 或者    
`<BubbledEvent type="event_name" receiver="receiver_name" apply_attribute="attribute_name='attribute_value'"/>`     
的子标签，每个子标签响应一个事件。    
`<Event />` 标签对应于控件的`Control::AttachXmlEvent`函数    
`<BubbledEvent />` 标签对应于控件的`Control::AttachXmlBubbledEvent`函数    
相关解析逻辑，可以参考代码`WindowBuilder::AttachXmlEvent`函数，代码在`duilib/Core/WindowBuilder.cpp`文件中。    

上述示例实现的代码逻辑，其伪代码如下：    
```
EventArgs msg; //假定在<Event>父节点的容器/控件上触发了某个事件
ui::EventType eventType = ui::StringToEventType("event_name");
if (msg.eventType == eventType) {
    ui::Control* pControl = FindControl("receiver_name");
    if (pControl != nullptr) {
        pControl->SetAttribute("attribute_name", "attribute_value");
    }
}
```   
2. 每个Event标签包含三个属性：`type`、`receiver`、`apply_attribute`，属性的含义如下：    
 - 事件类型(type)    
   事件类型的字符串形式，详见下面的`可用事件列表`文档，该字符串的值最终转换为事件类型为`ui::EventType`对应的属性值。
 - 目标控件的名称(receiver)    
   通过该属性设置目标控件的名称（对应于目标控件的name属性），通过该名称，找到目标控件的接口，该属性的几种应用形式：    
   `receiver="name"`: 在关联的窗口下，按控件名称查找控件（`通过Window::FindControl(const DString& name)`函数查找）    
   `receiver="./name"`: 在当前容器内查找子控件（`通过Box::FindSubControl(const DString& name)`函数查找）    
   `receiver=""`: receiver 是控件自身，不需要按名称查找       
   `receiver="#window#"`: receiver 控件关联的窗口，窗口的`Window::SetAttribute`支持的属性比较少，能实现的功能有限        
   相关的解析代码可以参考以下函数：`Control::OnApplyAttributeList`，位于`duilib/Core/Control.cpp`文件中。    
   如果receiver是多个目标控件，其名称需要用空格作为分隔符。    
   
 - 应用属性参数(apply_attribute)    
   该属性参数值的形式为：`attribute_name=attribute_value`，该属性的几种应用形式：    
   `apply_attribute="child_halign='left'"`: 最终会转换成调用代码：`Control::SetAttribute("child_halign", "left");`    
   `apply_attribute="child_halign={left}"`: 最终会转换成调用代码：`Control::SetAttribute("child_halign", "left");`    
    上述示例中，使用括号`{ 或者 }` 和使用单引号`' 或者 '` 是等效的，可以混合使用。    
    如果apply_attribute中包含多个属性，其不同属性之间需要用空格作为分隔符。    
   
### 三、可用事件列表    
可用事件列表在`duilib/Core/EventArgs.cpp`文件中的`InitEventStringMap`函数中定义，详细内容如下：    
| 事件类型 | 事件的XML名称1 | 事件的XML名称2|
| :---     | :---           |:---           |
|kEventAll|"All"|"all"|
|kEventDestroy|"Destroy"|"destroy"|
|kEventKeyDown|"KeyDown"|"key_down"|
|kEventKeyUp|"KeyUp"|"key_up"|
|kEventChar|"Char"|"char"|
|kEventMouseEnter|"MouseEnter"|"mouse_enter"|
|kEventMouseLeave|"MouseLeave"|"mouse_leave"|
|kEventMouseMove|"MouseMove"|"mouse_move"|
|kEventMouseHover|"MouseHover"|"mouse_hover"|
|kEventMouseWheel|"MouseWheel"|"mouse_wheel"|
|kEventMouseButtonDown|"MouseButtonDown"|"mouse_button_down"|
|kEventMouseButtonUp|"MouseButtonUp"|"mouse_button_up"|
|kEventMouseDoubleClick|"MouseDoubleClick"|"mouse_double_click"|
|kEventMouseRButtonDown|"MouseRButtonDown"|"mouse_rbutton_down"|
|kEventMouseRButtonUp|"MouseRButtonUp"|"mouse_rbutton_up"|
|kEventMouseRDoubleClick|"MouseRDoubleClick"|"mouse_rdouble_click"|
|kEventMouseMButtonDown|"MouseMButtonDown"|"mouse_mbutton_down"|
|kEventMouseMButtonUp|"MouseMButtonUp"|"mouse_mbutton_up"|
|kEventMouseMDoubleClick|"MouseMDoubleClick"|"mouse_mdouble_click"|
|kEventContextMenu|"ContextMenu"|"context_menu"|
|kEventSetFocus|"SetFocus"|"set_focus"|
|kEventKillFocus|"KillFocus"|"kill_focus"|
|kEventSetCursor|"SetCursor"|"set_cursor"|
|kEventCaptureChanged|"CaptureChanged"|"capture_changed"|
|kEventImeSetContext|"ImeSetContext"|"ime_set_context"|
|kEventImeStartComposition|"ImeStartComposition"|"ime_start_composition"|
|kEventImeComposition|"ImeComposition"|"ime_composition"|
|kEventImeEndComposition|"ImeEndComposition"|"ime_end_composition"|
|kEventWindowSetFocus|"WindowSetFocus"|"window_set_focus"|
|kEventWindowKillFocus|"WindowKillFocus"|"window_kill_focus"|
|kEventWindowPosChanged|"WindowPosChanged"|"window_pos_changed"|
|kEventWindowSize|"WindowSize"|"window_size"|
|kEventWindowMove|"WindowMove"|"window_move"|
|kEventWindowCreate|"WindowCreate"|"window_create"|
|kEventWindowClose|"WindowClose"|"window_close"|
|kEventClick|"Click"|"click"|
|kEventRClick|"RClick"|"rclick"|
|kEventMouseClickChanged|"MouseClickChanged"|"mouse_click_changed"|
|kEventMouseClickEsc|"MouseClickEsc"|"mouse_click_esc"|
|kEventSelect|"Select"|"select"|
|kEventUnSelect|"UnSelect"|"unselect"|
|kEventCheck|"Check"|"check"|
|kEventUnCheck|"UnCheck"|"uncheck"|
|kEventTabSelect|"TabSelect"|"tab_select"|
|kEventExpand|"Expand"|"expand"|
|kEventCollapse|"Collapse"|"collapse"|
|kEventZoom|"Zoom"|"zoom"|
|kEventTextChanged|"TextChanged"|"text_changed"|
|kEventSelChanged|"SelChanged"|"sel_changed"|
|kEventReturn|"Return"|"return"|
|kEventEsc|"Esc"|"esc"|
|kEventTab|"Tab"|"tab"|
|kEventLinkClick|"LinkClick"|"link_click"|
|kEventScrollPosChanged|"ScrollPosChanged"|"scroll_pos_changed"|
|kEventValueChanged|"ValueChanged"|"value_changed"|
|kEventPosChanged|"PosChanged"|"pos_changed"|
|kEventSizeChanged|"SizeChanged"|"size_changed"|
|kEventVisibleChanged|"VisibleChanged"|"visible_changed"|
|kEventStateChanged|"StateChanged"|"state_changed"|
|kEventSelectColor|"SelectColor"|"select_color"|
|kEventSplitDraged|"SplitDraged"|"split_draged"|
|kEventElementFilled|"kEventElementFilled"|"element_filled"|
|kEventEnterEdit|"EnterEdit"|"enter_edit"|
|kEventLeaveEdit|"LeaveEdit"|"leave_edit"|
|kEventDataItemCountChanged|"DataItemCountChanged"|"data_item_count_changed"|
|kEventItemMouseEnter|"ItemMouseEnter"|"item_mouse_enter"|
|kEventItemMouseLeave|"ItemMouseLeave"|"item_mouse_leave"|
|kEventSubItemMouseEnter|"SubItemMouseEnter"|"sub_item_mouse_enter"|
|kEventSubItemMouseLeave|"SubItemMouseLeave"|"sub_item_mouse_leave"|
|kEventReportViewItemFilled|"ReportViewItemFilled"|"report_view_item_filled"|
|kEventReportViewSubItemFilled|"ReportViewSubItemFilled"|"report_view_sub_item_filled"|
|kEventListViewItemFilled|"ListViewItemFilled"|"list_view_item_filled"|
|kEventIconViewItemFilled|"IconViewItemFilled"|"icon_view_item_filled"|
|kEventViewTypeChanged|"ViewTypeChanged"|"view_type_changed"|
|kEventViewPosChanged|"ViewPosChanged"|"view_pos_changed"|
|kEventViewSizeChanged|"ViewSizeChanged"|"view_size_changed"|
|kEventPathChanged|"PathChanged"|"path_changed"|
|kEventPathClick|"PathClick"|"path_click"|
|kEventDropEnter|"DropEnter"|"drop_enter"|
|kEventDropOver|"DropOver"|"drop_over"|
|kEventDropLeave|"DropLeave"|"drop_leave"|
|kEventDropData|"DropData"|"drop_data"|
|kEventImageAnimationStart|"ImageAnimationStart"|"image_animation_start"|
|kEventImageAnimationPlayFrame|"ImageAnimationPlayFrame"|"image_animation_play_frame"|
|kEventImageAnimationStop|"ImageAnimationStop"|"image_animation_stop"|
|kEventLoadingStart|"LoadingStart"|"loading_start"|
|kEventLoading|"Loading"|"loading"|
|kEventLoadingStop|"LoadingStop"|"loading_stop"|
|kEventImageLoad|"ImageLoad"|"image_load"|
|kEventImageDecode|"ImageDecode"|"image_decode"|

以上列表中，在XML中使用"事件的XML名称1"或者"事件的XML名称2"均可，两个名称都认。
