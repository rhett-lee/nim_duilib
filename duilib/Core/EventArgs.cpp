#include "EventArgs.h"

namespace ui
{

EventType StringToEventType(const std::wstring& messageType)
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
		return kEventMouseMenu;
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
		ASSERT(FALSE);
		return kEventNone;
	}
}

std::wstring EventTypeToString(EventType eventType)
{
	switch (eventType) {
	case kEventNone:
		return L"kEventNone";
	case kEventFirst:
		return L"kEventFirst";
	case kEventAll:
		return L"kEventAll";
	case kEventKeyBegin:
		return L"kEventKeyBegin";
	case kEventKeyDown:
		return L"kEventKeyDown";
	case kEventKeyUp:
		return L"kEventKeyUp";
	case kEventSysKeyDown:
		return L"kEventSysKeyDown";
	case kEventSysKeyUp:
		return L"kEventSysKeyUp";
	case kEventChar:
		return L"kEventChar";
	case kEventKeyEnd:
		return L"kEventKeyEnd";
	case kEventMouseBegin:
		return L"kEventMouseBegin";
	case kEventMouseEnter:
		return L"kEventMouseEnter";
	case kEventMouseLeave:
		return L"kEventMouseLeave";
	case kEventMouseMove:
		return L"kEventMouseMove";
	case kEventMouseHover:
		return L"kEventMouseHover";
	case kEventMouseWheel:
		return L"kEventMouseWheel";
	case kEventMouseButtonDown:
		return L"kEventMouseButtonDown";
	case kEventMouseButtonUp:
		return L"kEventMouseButtonUp";
	case kEventMouseDoubleClick:
		return L"kEventMouseDoubleClick";
	case kEventMouseRButtonDown:
		return L"kEventMouseRButtonDown";
	case kEventMouseRButtonUp:
		return L"kEventMouseRButtonUp";
	case kEventMouseRDoubleClick:
		return L"kEventMouseRDoubleClick";
	case kEventMouseMenu:
		return L"kEventMouseMenu";
	case kEventMouseEnd:
		return L"kEventMouseEnd";
	case kEventSetFocus:
		return L"kEventSetFocus";
	case kEventKillFocus:
		return L"kEventKillFocus";
	case kEventSetCursor:
		return L"kEventSetCursor";
	case kEventImeStartComposition:
		return L"kEventImeStartComposition";
	case kEventImeEndComposition:
		return L"kEventImeEndComposition";
	case kEventWindowKillFocus:
		return L"kEventWindowKillFocus";
	case kEventWindowSize:
		return L"kEventWindowSize";
	case kEventWindowMove:
		return L"kEventWindowMove";
	case kEventWindowClose:
		return L"kEventWindowClose";
	case kEventClick:
		return L"kEventClick";
	case kEventRClick:
		return L"kEventRClick";
	case kEventMouseClickChanged:
		return L"kEventMouseClickChanged";
	case kEventMouseClickEsc:
		return L"kEventMouseClickEsc";
	case kEventSelect:
		return L"kEventSelect";
	case kEventUnSelect:
		return L"kEventUnSelect";
	case kEventChecked:
		return L"kEventChecked";
	case kEventUnCheck:
		return L"kEventUnCheck";
	case kEventTabSelect:
		return L"kEventTabSelect";
	case kEventExpand:
		return L"kEventExpand";
	case kEventCollapse:
		return L"kEventCollapse";
	case kEventZoom:
		return L"kEventZoom";
	case kEventTextChange:
		return L"kEventTextChange";
	case kEventSelChange:
		return L"kEventSelChange";
	case kEventReturn:
		return L"kEventReturn";
	case kEventTab:
		return L"kEventTab";
	case kEventLinkClick:
		return L"kEventLinkClick";
	case kEventScrollChange:
		return L"kEventScrollChange";
	case kEventValueChange:
		return L"kEventValueChange";
	case kEventResize:
		return L"kEventResize";
	case kEventVisibleChange:
		return L"kEventVisibleChange";
	case kEventStateChange:
		return L"kEventStateChange";
	case kEventSelectColor:
		return L"kEventSelectColor";
	case kEventSplitDraged:
		return L"kEventSplitDraged";
	case kEventEnterEdit:
		return L"kEventEnterEdit";
	case kEventLeaveEdit:
		return L"kEventLeaveEdit";
	case kEventLast:
		return L"kEventLast";
	default:
		break;
	}
	return std::wstring();
}

}