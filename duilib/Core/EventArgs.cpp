#include "EventArgs.h"
#include "duilib/Core/Control.h"
#include <unordered_map>
#include <vector>

namespace ui
{
EventArgs::EventArgs() :
    eventType(EventType::kEventNone),
    vkCode(VirtualKeyCode::kVK_None),
    wParam(0),
    lParam(0),
    pSender(nullptr),
    modifierKey(0),
    eventData(0)
{
    ptMouse.x = 0;
    ptMouse.y = 0;
}

void EventArgs::SetSender(Control* pControl)
{
    if (pControl != nullptr) {
        pSender = pControl;
        m_senderFlag = pControl->GetWeakFlag();
    }
    else {
        pSender = nullptr;
        m_senderFlag.reset();
    }
}

Control* EventArgs::GetSender() const
{
    if (m_senderFlag.expired()) {
        return nullptr;
    }
    else {
        return pSender;
    }
}

bool EventArgs::IsSenderExpired() const
{
    if (pSender != nullptr) {
        return m_senderFlag.expired();
    }
    return false;
}

//EventType 与 String 相互转换的数据结构
struct EventTypeString
{
    EventType m_eventType;      //事件类型
    DString m_displayName;      //事件的显示名称
    DString m_xmlName1;         //事件的XML中名称1
    DString m_xmlName2;         //事件的XML中名称2
};

//初始化EventType 与 String 相互转换的数据容器
static void InitEventStringMap(std::unordered_map<EventType, DString>* typeMap,
                               std::unordered_map<DString, EventType>* nameMap)
{
    if ((typeMap == nullptr) && (nameMap == nullptr)) {
        return;
    }
    const std::vector<EventTypeString> eventTypeStringList = {
        {kEventAll, _T("kEventAll"), _T("All"), _T("all")},
        {kEventDestroy, _T("kEventDestroy"), _T("Destroy"), _T("destroy")},
        {kEventKeyDown, _T("kEventKeyDown"), _T("KeyDown"), _T("key_down")},
        {kEventKeyUp, _T("kEventKeyUp"), _T("KeyUp"), _T("key_up")},
        {kEventChar, _T("kEventChar"), _T("Char"), _T("char")},
        {kEventMouseEnter, _T("kEventMouseEnter"), _T("MouseEnter"), _T("mouse_enter")},
        {kEventMouseLeave, _T("kEventMouseLeave"), _T("MouseLeave"), _T("mouse_leave")},
        {kEventMouseMove, _T("kEventMouseMove"), _T("MouseMove"), _T("mouse_move")},
        {kEventMouseHover, _T("kEventMouseHover"), _T("MouseHover"), _T("mouse_hover")},
        {kEventMouseWheel, _T("kEventMouseWheel"), _T("MouseWheel"), _T("mouse_wheel")},
        {kEventMouseButtonDown, _T("kEventMouseButtonDown"), _T("MouseButtonDown"), _T("mouse_button_down")},
        {kEventMouseButtonUp, _T("kEventMouseButtonUp"), _T("MouseButtonUp"), _T("mouse_button_up")},
        {kEventMouseDoubleClick, _T("kEventMouseDoubleClick"), _T("MouseDoubleClick"), _T("mouse_double_click")},
        {kEventMouseRButtonDown, _T("kEventMouseRButtonDown"), _T("MouseRButtonDown"), _T("mouse_rbutton_down")},
        {kEventMouseRButtonUp, _T("kEventMouseRButtonUp"), _T("MouseRButtonUp"), _T("mouse_rbutton_up")},
        {kEventMouseRDoubleClick, _T("kEventMouseRDoubleClick"), _T("MouseRDoubleClick"), _T("mouse_rdouble_click")},
        {kEventMouseMButtonDown, _T("kEventMouseMButtonDown"), _T("MouseMButtonDown"), _T("mouse_mbutton_down")},
        {kEventMouseMButtonUp, _T("kEventMouseMButtonUp"), _T("MouseMButtonUp"), _T("mouse_mbutton_up")},
        {kEventMouseMDoubleClick, _T("kEventMouseMDoubleClick"), _T("MouseMDoubleClick"), _T("mouse_mdouble_click")},
        {kEventContextMenu, _T("kEventContextMenu"), _T("ContextMenu"), _T("context_menu")},
        {kEventSetFocus, _T("kEventSetFocus"), _T("SetFocus"), _T("set_focus")},
        {kEventKillFocus, _T("kEventKillFocus"), _T("KillFocus"), _T("kill_focus")},
        {kEventSetCursor, _T("kEventSetCursor"), _T("SetCursor"), _T("set_cursor")},
        {kEventCaptureChanged, _T("kEventCaptureChanged"), _T("CaptureChanged"), _T("capture_changed")},
        {kEventImeSetContext, _T("kEventImeSetContext"), _T("ImeSetContext"), _T("ime_set_context")},
        {kEventImeStartComposition, _T("kEventImeStartComposition"), _T("ImeStartComposition"), _T("ime_start_composition")},
        {kEventImeComposition, _T("kEventImeComposition"), _T("ImeComposition"), _T("ime_composition")},
        {kEventImeEndComposition, _T("kEventImeEndComposition"), _T("ImeEndComposition"), _T("ime_end_composition")},
        {kEventWindowSetFocus, _T("kEventWindowSetFocus"), _T("WindowSetFocus"), _T("window_set_focus")},
        {kEventWindowKillFocus, _T("kEventWindowKillFocus"), _T("WindowKillFocus"), _T("window_kill_focus")},
        {kEventWindowSize, _T("kEventWindowSize"), _T("WindowSize"), _T("window_size")},
        {kEventWindowMove, _T("kEventWindowMove"), _T("WindowMove"), _T("window_move")},
        {kEventWindowCreate, _T("kEventWindowCreate"), _T("WindowCreate"), _T("window_create")},
        {kEventWindowClose, _T("kEventWindowClose"), _T("WindowClose"), _T("window_close")},
        {kEventWindowFirstShown, _T("kEventWindowFirstShown"), _T("WindowFirstShown"), _T("window_first_shown")},
        {kEventClick, _T("kEventClick"), _T("Click"), _T("click")},
        {kEventRClick, _T("kEventRClick"), _T("RClick"), _T("rclick")},
        {kEventMouseClickChanged, _T("kEventMouseClickChanged"), _T("MouseClickChanged"), _T("mouse_click_changed")},
        {kEventMouseClickEsc, _T("kEventMouseClickEsc"), _T("MouseClickEsc"), _T("mouse_click_esc")},
        {kEventSelect, _T("kEventSelect"), _T("Select"), _T("select")},
        {kEventUnSelect, _T("kEventUnSelect"), _T("UnSelect"), _T("unselect")},
        {kEventCheck, _T("kEventCheck"), _T("Check"), _T("check")},
        {kEventUnCheck, _T("kEventUnCheck"), _T("UnCheck"), _T("uncheck")},
        {kEventTabSelect, _T("kEventTabSelect"), _T("TabSelect"), _T("tab_select")},
        {kEventExpand, _T("kEventExpand"), _T("Expand"), _T("expand")},
        {kEventCollapse, _T("kEventCollapse"), _T("Collapse"), _T("collapse")},
        {kEventZoom, _T("kEventZoom"), _T("Zoom"), _T("zoom")},
        {kEventTextChanged, _T("kEventTextChanged"), _T("TextChanged"), _T("text_changed")},
        {kEventSelChanged, _T("kEventSelChanged"), _T("SelChanged"), _T("sel_changed")},
        {kEventReturn, _T("kEventReturn"), _T("Return"), _T("return")},
        {kEventEsc, _T("kEventEsc"), _T("Esc"), _T("esc")},
        {kEventTab, _T("kEventTab"), _T("Tab"), _T("tab")},
        {kEventLinkClick, _T("kEventLinkClick"), _T("LinkClick"), _T("link_click")},
        {kEventScrollPosChanged, _T("kEventScrollPosChanged"), _T("ScrollPosChanged"), _T("scroll_pos_changed")},
        {kEventValueChanged, _T("kEventValueChanged"), _T("ValueChanged"), _T("value_changed")},
        {kEventPosChanged, _T("kEventPosChanged"), _T("PosChanged"), _T("pos_changed")},
        {kEventSizeChanged, _T("kEventSizeChanged"), _T("SizeChanged"), _T("size_changed")},
        {kEventVisibleChanged, _T("kEventVisibleChanged"), _T("VisibleChanged"), _T("visible_changed")},
        {kEventStateChanged, _T("kEventStateChanged"), _T("StateChanged"), _T("state_changed")},
        {kEventSelectColor, _T("kEventSelectColor"), _T("SelectColor"), _T("select_color")},
        {kEventSplitDraged, _T("kEventSplitDraged"), _T("SplitDraged"), _T("split_draged")},
        {kEventEnterEdit, _T("kEventEnterEdit"), _T("EnterEdit"), _T("enter_edit")},
        {kEventLeaveEdit, _T("kEventLeaveEdit"), _T("LeaveEdit"), _T("leave_edit")},
        {kEventDataItemCountChanged, _T("kEventDataItemCountChanged"), _T("DataItemCountChanged"), _T("data_item_count_changed")},
        {kEventReportViewItemFilled, _T("kEventReportViewItemFilled"), _T("ReportViewItemFilled"), _T("report_view_item_filled")},
        {kEventReportViewSubItemFilled, _T("kEventReportViewSubItemFilled"), _T("ReportViewSubItemFilled"), _T("report_view_sub_item_filled")},
        {kEventListViewItemFilled, _T("kEventListViewItemFilled"), _T("ListViewItemFilled"), _T("list_view_item_filled")},
        {kEventIconViewItemFilled, _T("kEventIconViewItemFilled"), _T("IconViewItemFilled"), _T("icon_view_item_filled")},
        {kEventPathChanged, _T("kEventPathChanged"), _T("PathChanged"), _T("path_changed")},
        {kEventPathClick, _T("kEventPathClick"), _T("PathClick"), _T("path_click")},
        {kEventDropEnter, _T("kEventDropEnter"), _T("DropEnter"), _T("drop_enter")},
        {kEventDropOver, _T("kEventDropOver"), _T("DropOver"), _T("drop_over")},
        {kEventDropLeave, _T("kEventDropLeave"), _T("DropLeave"), _T("drop_leave")},
        {kEventDropData, _T("kEventDropData"), _T("DropData"), _T("drop_data")},
        {kEventImageAnimationStart, _T("kEventImageAnimationStart"), _T("ImageAnimationStart"), _T("image_animation_start")},
        {kEventImageAnimationPlayFrame, _T("kEventImageAnimationPlayFrame"), _T("ImageAnimationPlayFrame"), _T("image_animation_play_frame")},
        {kEventImageAnimationStop, _T("kEventImageAnimationStop"), _T("ImageAnimationStop"), _T("image_animation_stop")},
        {kEventLoadingStart, _T("kEventLoadingStart"), _T("LoadingStart"), _T("loading_start")},
        {kEventLoading, _T("kEventLoading"), _T("Loading"), _T("loading")},
        {kEventLoadingStop, _T("kEventLoadingStop"), _T("LoadingStop"), _T("loading_stop")},
        {kEventImageLoad, _T("kEventImageLoad"), _T("ImageLoad"), _T("image_load")},
        {kEventImageDecode, _T("kEventImageDecode"), _T("ImageDecode"), _T("image_decode")}
    };

    for (const EventTypeString& typeString : eventTypeStringList) {
        if (typeMap != nullptr) {
            (*typeMap)[typeString.m_eventType] = typeString.m_displayName;
        }
        if (nameMap != nullptr) {
            (*nameMap)[typeString.m_xmlName1] = typeString.m_eventType;
            (*nameMap)[typeString.m_xmlName2] = typeString.m_eventType;
        }
    }
}

EventType StringToEventType(const DString& eventName)
{
    static std::unordered_map<DString, EventType> nameMap;
    InitEventStringMap(nullptr, &nameMap);
    auto iter = nameMap.find(eventName);
    ASSERT(iter != nameMap.end());
    if (iter != nameMap.end()) {
        return iter->second;
    }
    return EventType::kEventNone;
}

DString EventTypeToString(EventType eventType)
{
    static std::unordered_map<EventType, DString> typeMap;
    InitEventStringMap(&typeMap, nullptr);
    auto iter = typeMap.find(eventType);
    ASSERT(iter != typeMap.end());
    if (iter != typeMap.end()) {
        return iter->second;
    }
    return DString();
}

}
