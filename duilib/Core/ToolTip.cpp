#include "ToolTip.h"

namespace ui
{

//ToolTip最大长度
#define TOOLTIP_MAX_LEN 511

ToolTip::ToolTip():
    m_bMouseTracking(false),
    m_hwndTooltip(nullptr),
	m_hParentWnd(nullptr)
{
    ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
    m_ToolTip.cbSize = sizeof(TOOLINFO);
}

ToolTip::~ToolTip()
{
    if (m_hwndTooltip != nullptr) {
        ::DestroyWindow(m_hwndTooltip);
        m_hwndTooltip = nullptr;
    }
}

void ToolTip::SetMouseTracking(HWND hParentWnd, bool bTracking)
{
	if (bTracking && !m_bMouseTracking) {
		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.hwndTrack = hParentWnd;
		tme.dwHoverTime = (m_hwndTooltip == nullptr) ? 400UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
		_TrackMouseEvent(&tme);
	}
	m_hParentWnd = hParentWnd;
	m_bMouseTracking = bTracking;
}

void ToolTip::ShowToolTip(HWND hParentWnd,
					 	  HMODULE hModule,
						  const UiRect& rect,
						  uint32_t maxWidth,
						  const UiPoint& trackPos,
						  const std::wstring& text)
{
	if (text.empty()) {
		return;
	}
	//如果超过长度，则截断
	std::wstring newText(text);
	if (newText.size() > TOOLTIP_MAX_LEN) {
		newText = newText.substr(0, TOOLTIP_MAX_LEN);
	}

	if ((m_hwndTooltip != nullptr) && IsWindowVisible(m_hwndTooltip)) {
		TOOLINFO toolTip = { 0 };
		toolTip.cbSize = sizeof(TOOLINFO);
		toolTip.hwnd = hParentWnd;
		toolTip.uId = (UINT_PTR)hParentWnd;
		std::wstring oldText;
		oldText.resize(TOOLTIP_MAX_LEN + 1);
		toolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)oldText.c_str());
		::SendMessage(m_hwndTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&toolTip);
		oldText = std::wstring(oldText.c_str());
		if (newText == oldText) {
			//文本内容均没有变化，不再设置
			return;
		}
	}

	::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
	m_ToolTip.cbSize = sizeof(TOOLINFO);
	m_ToolTip.uFlags = TTF_IDISHWND;
	m_ToolTip.hwnd = hParentWnd;
	m_ToolTip.uId = (UINT_PTR)hParentWnd;
	m_ToolTip.hinst = hModule;
	m_ToolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)newText.c_str());
	m_ToolTip.rect.left = rect.left;
	m_ToolTip.rect.top = rect.top;
	m_ToolTip.rect.right = rect.right;
	m_ToolTip.rect.bottom = rect.bottom;
	if (m_hwndTooltip == nullptr) {
		m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, 
									     WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
										 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
										 hParentWnd, NULL, hModule, NULL);
		::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
		::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	if (!::IsWindowVisible(m_hwndTooltip)) {
		::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, maxWidth);
		::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&m_ToolTip);
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
	}
	::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD)MAKELONG(trackPos.x, trackPos.y));
	m_hParentWnd = hParentWnd;
}

void ToolTip::HideToolTip()
{
	if (m_hwndTooltip != nullptr) {
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
	}
}

void ToolTip::ClearMouseTracking()
{
	if (m_bMouseTracking) {
		::SendMessage(m_hParentWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
	}
	m_bMouseTracking = false;
}

} // namespace ui
