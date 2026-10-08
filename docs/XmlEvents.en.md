English | [简体中文](XmlEvents.md)

> Last synced: 2026-09-29

## Documentation on Responding to Control Events Directly in XML Files
### I. Usage Examples    
In XML files, you can directly write response code for control events. Here are a few usage examples.    
Example 1: In the `examples/layout` example program, the following XML snippet demonstrates how to use the Event tag to attach an event on an Option tag to set the container's alignment:    
```xml
<HBox height="28">            
    <Label text="Container content horizontal alignment:" valign="center"/>
    <Option class="option_1" group="gridbox_group_halign" text="Align left (left)" height="28" valign="center" margin="8,0,0,0" selected="false">
        <Event type="select" receiver="gridbox_test" apply_attribute="child_halign='left'" />
    </Option>
    <Option class="option_1" group="gridbox_group_halign" text="Center align (center)" height="28" valign="center" margin="8,0,0,0" selected="true">
        <Event type="select" receiver="gridbox_test" apply_attribute="child_halign='center'" />
    </Option>
    <Option class="option_1" group="gridbox_group_halign" text="Align right (right)" height="28" valign="center" margin="8,0,0,0" selected="false">
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
Example 2: In the `examples/render` example program, the following XML snippet demonstrates how to use the Event tag to attach an event on an Option tag to switch the window's shadow effect:    
```xml
<VBox>
    <Label text="Window shadow feature test:" valign="center" margin="12,30,10,0"/>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: default, default shadow" selected="true" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={default}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: big, large shadow, square corners (suitable for ordinary windows)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={big}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: big_round, large shadow, rounded corners (suitable for ordinary windows)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={big_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: small, small shadow, square corners (suitable for ordinary windows)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={small}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: small_round, small shadow, rounded corners (suitable for ordinary windows)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={small_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: menu, small shadow, square corners (suitable for popup windows, such as menus)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={menu}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: menu_round, small shadow, rounded corners (suitable for popup windows, such as menus)" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={menu_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: none, no shadow, with border, square corners" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={none}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Shadow type: none_round, no shadow, with border, rounded corners" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_type={none_round}" />
    </Option>
    <Option class="checkbox_2" group="shadow_type" text="Disable shadow feature" selected="false" valign="center" margin="30,4,0,2">
        <Event type="select" receiver="#window#" apply_attribute="shadow_attached={false}" />
    </Option>
</VBox>
```
Example 3: In the `examples/render` example program, the following XML snippet demonstrates how to use the Event tag to attach an event on a Control tag to play an animation when the mouse is over the image:    
```xml
<VBox width="200" height="220">
    <Control width="100%" height="100%" bkimage="file='fan.gif' width='190' height='190' auto_play='false' play_count='-1' valign='center' halign='center'">
        <Event type="mouse_enter" receiver="" apply_attribute="start_image_animation={}" />
        <Event type="mouse_leave" receiver="" apply_attribute="stop_image_animation={}" />
    </Control>
    <Label width="100%" height="20" text="Play animation when mouse is over the image" text_align="center" margin="8"/>
</VBox>
```

### II. Usage Method    
1. Under the tag of a control/container, add a child tag of the form    
`<Event type="event_name" receiver="receiver_name" apply_attribute="attribute_name='attribute_value'"/>` or    
`<BubbledEvent type="event_name" receiver="receiver_name" apply_attribute="attribute_name='attribute_value'"/>`     
child tag, each child tag responds to one event.    
`<Event />` tags correspond to the control's `Control::AttachXmlEvent` function    
`<BubbledEvent />` tags correspond to the control's `Control::AttachXmlBubbledEvent` function    
The related parsing logic can be referred to in the code of the `WindowBuilder::AttachXmlEvent` function, in the `duilib/Core/WindowBuilder.cpp` file.    

The code logic implemented by the examples above is shown in the following pseudocode:    
```
EventArgs msg; // Assume a certain event is triggered on the container/control that is the parent node of <Event>
ui::EventType eventType = ui::StringToEventType("event_name");
if (msg.eventType == eventType) {
    ui::Control* pControl = FindControl("receiver_name");
    if (pControl != nullptr) {
        pControl->SetAttribute("attribute_name", "attribute_value");
    }
}
```   
2. Each Event tag contains three attributes: `type`, `receiver`, `apply_attribute`, whose meanings are as follows:    
 - Event type (type)    
   The string form of the event type; see the `Available Event List` document below. This string value is finally converted to the event type value corresponding to `ui::EventType`.
 - Name of the target control (receiver)    
   Set the name of the target control through this attribute (corresponding to the name attribute of the target control). Through this name, find the interface of the target control. The several application forms of this attribute:    
   `receiver="name"`: Find the control by control name under the associated window (looked up via the `Window::FindControl(const DString& name)` function)    
   `receiver="./name"`: Find the child control within the current container (looked up via the `Box::FindSubControl(const DString& name)` function)    
   `receiver=""`: receiver is the control itself, no need to look up by name       
   `receiver="#window#"`: receiver is the window associated with the control; the `Window::SetAttribute` of the window supports relatively few attributes, so the functionality it can achieve is limited        
   The related parsing code can be referred to in the following function: `Control::OnApplyAttributeList`, located in the `duilib/Core/Control.cpp` file.    
   If receiver is multiple target controls, their names need to be separated by spaces.    
   
 - Apply attribute parameter (apply_attribute)    
   The form of this attribute parameter value is: `attribute_name=attribute_value`. The several application forms of this attribute:    
   `apply_attribute="child_halign='left'"`: will finally be converted to the call: `Control::SetAttribute("child_halign", "left");`    
   `apply_attribute="child_halign={left}"`: will finally be converted to the call: `Control::SetAttribute("child_halign", "left");`    
   In the examples above, using braces `{ or }` and using single quotes `' or '` are equivalent and can be mixed.    
   If `apply_attribute` contains multiple attributes, the different attributes need to be separated by spaces.    
   
### III. Available Event List    
The available event list is defined in the `InitEventStringMap` function in the `duilib/Core/EventArgs.cpp` file; the details are as follows:    
| Event type | XML name 1 of the event | XML name 2 of the event|
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
|kEventPanelExpanding|"PanelExpanding"|"panel_expanding"|
|kEventPanelCollapsing|"PanelCollapsing"|"panel_collapsing"|
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
|kEventViewModeChanged|"ViewModeChanged"|"view_mode_changed"|
|kEventDisplayDateChanged|"DisplayDateChanged"|"display_date_changed"|

In the list above, either "XML name 1 of the event" or "XML name 2 of the event" can be used in XML; both names are recognized.
