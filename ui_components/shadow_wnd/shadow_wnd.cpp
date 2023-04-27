#include "shadow_wnd.h"

namespace nim_comp {

ShadowWnd::ShadowWnd()
{

}

std::wstring ShadowWnd::GetSkinFolder()
{
	return L"shadow";
}

std::wstring ShadowWnd::GetSkinFile()
{
	return L"shadow.xml";
}

std::wstring ShadowWnd::GetWindowClassName() const
{
	return L"ShadowWindow";
}

HWND ShadowWnd::Create(Window* window)
{
	window_ = window;
	return Window::CreateWnd(NULL, L"ShadowWindow", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, true);
}

LRESULT ShadowWnd::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	if (window_ == NULL || window_->GetHWND() == NULL)
	{
		return FALSE;
	}

	switch (uMsg)
	{
	case WM_ERASEBKGND:
	case WM_PAINT:
	case WM_MOVE:
	case WM_ACTIVATE:
	case WM_NCACTIVATE:
	{
		if (::IsWindowVisible(window_->GetHWND()))
		{
			RECT rc = {0};
			::GetWindowRect(window_->GetHWND(), &rc);
			SetPos(ui::UiRect(rc), false, SWP_SHOWWINDOW | SWP_NOACTIVATE, window_->GetHWND());
		}
		break;
	}
	case WM_CLOSE:
	{
		ShowWindow(false, false);
		CloseWnd(0);
		break;
	}
	case WM_SHOWWINDOW:
	{
		ShowWindow(wParam == 0 ? false : true, false);
		break;
	}
	default:
	{
		break;
	}

	}

	bHandled = FALSE;
	return FALSE;
}

ShadowWndBase::ShadowWndBase()
{
	shadow_wnd_ = new ShadowWnd;
	this->AddMessageFilter(shadow_wnd_);
}

HWND ShadowWndBase::CreateWnd(HWND hwndParent, const wchar_t* windowName, uint32_t dwStyle, uint32_t dwExStyle,
	bool /*isLayeredWindow*/ /*= false*/, const ui::UiRect& rc /*= ui::UiRect(0, 0, 0, 0)*/)
{
	__super::CreateWnd(hwndParent, windowName, dwStyle, dwExStyle, false, rc);

	shadow_wnd_->Create(this);
	::EnableWindow(shadow_wnd_->GetHWND(), FALSE);
	if (::IsWindowVisible(GetHWND()))
		shadow_wnd_->ShowWindow();
	return GetHWND();
}

} // namespace ui
