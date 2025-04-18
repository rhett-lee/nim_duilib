#include "EventArgs.h"
#include "duilib/Core/Control.h"

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

EventType StringToEventType(const DString& messageType)
{
    if (messageType == EVENTSTR_ALL) {
        return kEventAll;
    }
    else if (messageType == EVENTSTR_KEYDOWN) {
        return kEventKeyDown;
    }
    else if (messageType == EVENTSTR_KEYUP) {
        return kEventKeyUp;
    }
    else if (messageType == EVENTSTR_CHAR) {
        return kEventChar;
    }
    else if (messageType == EVENTSTR_SETFOCUS) {
        return kEventSetFocus;
    }
    else if (messageType == EVENTSTR_KILLFOCUS) {
        return kEventKillFocus;
    }
    else if (messageType == EVENTSTR_SETCURSOR) {
        return kEventSetCursor;
    }
    else if (messageType == EVENTSTR_MOUSEMOVE) {
        return kEventMouseMove;
    }
    else if (messageType == EVENTSTR_MOUSEENTER) {
        return kEventMouseEnter;
    }
    else if (messageType == EVENTSTR_MOUSELEAVE) {
        return kEventMouseLeave;
    }
    else if (messageType == EVENTSTR_MOUSEHOVER) {
        return kEventMouseHover;
    }
    else if (messageType == EVENTSTR_BUTTONDOWN) {
        return kEventMouseButtonDown;
    }
    else if (messageType == EVENTSTR_BUTTONUP) {
        return kEventMouseButtonUp;
    }
    else if (messageType == EVENTSTR_DOUBLECLICK) {
        return kEventMouseDoubleClick;
    }
    else if (messageType == EVENTSTR_RBUTTONDOWN) {
        return kEventMouseRButtonDown;
    }
    else if (messageType == EVENTSTR_RBUTTONUP) {
        return kEventMouseRButtonUp;
    }
    else if (messageType == EVENTSTR_RDOUBLECLICK) {
        return kEventMouseRDoubleClick;
    }
    else if (messageType == EVENTSTR_SELECT) {
        return kEventSelect;
    }
    else if (messageType == EVENTSTR_UNSELECT) {
        return kEventUnSelect;
    }
    else if (messageType == EVENTSTR_CHECKED) {
        return kEventChecked;
    }
    else if (messageType == EVENTSTR_UNCHECK) {
        return kEventUnCheck;
    }
    else if (messageType == EVENTSTR_TAB_SELECT) {
        return kEventTabSelect;
    }
    else if (messageType == EVENTSTR_MENU) {
        return kEventContextMenu;
    }
    else if (messageType == EVENTSTR_EXPAND) {
        return kEventExpand;
    }
    else if (messageType == EVENTSTR_COLLAPSE) {
        return kEventCollapse;
    }
    else if (messageType == EVENTSTR_SCROLLWHEEL) {
        return kEventMouseWheel;
    }
    else if (messageType == EVENTSTR_SCROLLCHANGE) {
        return kEventScrollChange;
    }
    else if (messageType == EVENTSTR_VALUECHANGE) {
        return kEventValueChange;
    }
    else if (messageType == EVENTSTR_RETURN) {
        return kEventReturn;
    }
    else if (messageType == EVENTSTR_TAB) {
        return kEventTab;
    }
    else if (messageType == EVENTSTR_WINDOWCLOSE) {
        return kEventWindowClose;
    }
    else {
        ASSERT(0);
        return kEventNone;
    }
}

DString EventTypeToString(EventType eventType)
{
    switch (eventType) {
    case kEventNone:
        return _T("kEventNone");
    case kEventAll:
        return _T("kEventAll");
    case kEventDestroy:
        return _T("kEventDestroy");
    case kEventKeyBegin:
        return _T("kEventKeyBegin");
    case kEventKeyDown:
        return _T("kEventKeyDown");
    case kEventKeyUp:
        return _T("kEventKeyUp");
    case kEventChar:
        return _T("kEventChar");
    case kEventKeyEnd:
        return _T("kEventKeyEnd");
    case kEventMouseBegin:
        return _T("kEventMouseBegin");
    case kEventMouseEnter:
        return _T("kEventMouseEnter");
    case kEventMouseLeave:
        return _T("kEventMouseLeave");
    case kEventMouseMove:
        return _T("kEventMouseMove");
    case kEventMouseHover:
        return _T("kEventMouseHover");
    case kEventMouseWheel:
        return _T("kEventMouseWheel");
    case kEventMouseButtonDown:
        return _T("kEventMouseButtonDown");
    case kEventMouseButtonUp:
        return _T("kEventMouseButtonUp");
    case kEventMouseDoubleClick:
        return _T("kEventMouseDoubleClick");
    case kEventMouseRButtonDown:
        return _T("kEventMouseRButtonDown");
    case kEventMouseRButtonUp:
        return _T("kEventMouseRButtonUp");
    case kEventMouseRDoubleClick:
        return _T("kEventMouseRDoubleClick");
    case kEventMouseMButtonDown:
        return _T("kEventMouseMButtonDown");
    case kEventMouseMButtonUp:
        return _T("kEventMouseMButtonUp");
    case kEventMouseMDoubleClick:
        return _T("kEventMouseMDoubleClick");
    case kEventContextMenu:
        return _T("kEventContextMenu");
    case kEventMouseEnd:
        return _T("kEventMouseEnd");
    case kEventSetFocus:
        return _T("kEventSetFocus");
    case kEventKillFocus:
        return _T("kEventKillFocus");
    case kEventSetCursor:
        return _T("kEventSetCursor");
    case kEventCaptureChanged:
        return _T("kEventCaptureChanged");
    case kEventImeSetContext:
        return _T("kEventImeSetContext");
    case kEventImeStartComposition:
        return _T("kEventImeStartComposition");
    case kEventImeComposition:
        return _T("kEventImeComposition");
    case kEventImeEndComposition:
        return _T("kEventImeEndComposition");
    case kEventWindowSetFocus:
        return _T("kEventWindowSetFocus");
    case kEventWindowKillFocus:
        return _T("kEventWindowKillFocus");
    case kEventWindowSize:
        return _T("kEventWindowSize");
    case kEventWindowMove:
        return _T("kEventWindowMove");
    case kEventWindowClose:
        return _T("kEventWindowClose");
    case kEventClick:
        return _T("kEventClick");
    case kEventRClick:
        return _T("kEventRClick");
    case kEventMouseClickChanged:
        return _T("kEventMouseClickChanged");
    case kEventMouseClickEsc:
        return _T("kEventMouseClickEsc");
    case kEventSelect:
        return _T("kEventSelect");
    case kEventUnSelect:
        return _T("kEventUnSelect");
    case kEventChecked:
        return _T("kEventChecked");
    case kEventUnCheck:
        return _T("kEventUnCheck");
    case kEventTabSelect:
        return _T("kEventTabSelect");
    case kEventExpand:
        return _T("kEventExpand");
    case kEventCollapse:
        return _T("kEventCollapse");
    case kEventZoom:
        return _T("kEventZoom");
    case kEventTextChange:
        return _T("kEventTextChange");
    case kEventSelChange:
        return _T("kEventSelChange");
    case kEventReturn:
        return _T("kEventReturn");
    case kEventTab:
        return _T("kEventTab");
    case kEventLinkClick:
        return _T("kEventLinkClick");
    case kEventScrollChange:
        return _T("kEventScrollChange");
    case kEventValueChange:
        return _T("kEventValueChange");
    case kEventResize:
        return _T("kEventResize");
    case kEventVisibleChange:
        return _T("kEventVisibleChange");
    case kEventStateChange:
        return _T("kEventStateChange");
    case kEventSelectColor:
        return _T("kEventSelectColor");
    case kEventSplitDraged:
        return _T("kEventSplitDraged");
    case kEventEnterEdit:
        return _T("kEventEnterEdit");
    case kEventLeaveEdit:
        return _T("kEventLeaveEdit");
    default:
        break;
    }
    return DString();
}

}
