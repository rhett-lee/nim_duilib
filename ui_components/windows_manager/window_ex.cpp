#include "window_ex.h"
#include "windows_manager.h"

namespace nim_comp
{
WindowEx::WindowEx()
{
}

WindowEx::~WindowEx()
{
}

HWND WindowEx::CreateWnd(HWND hwndParent, const wchar_t* windowName, uint32_t dwStyle, uint32_t dwExStyle, bool isLayeredWindow, const ui::UiRect& rc)
{
	if (!RegisterWnd())
	{
		return NULL;
	}

	HWND hwnd = __super::CreateWnd(hwndParent, windowName, dwStyle, dwExStyle, isLayeredWindow, rc);
	ASSERT(hwnd);
	return hwnd;
}

LRESULT WindowEx::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	UnRegisterWnd();
	return __super::OnDestroy(uMsg, wParam, lParam, bHandled);
}

void WindowEx::OnEsc(bool &bHandled)
{
	bHandled = false;
}

bool WindowEx::RegisterWnd()
{
	std::wstring wnd_class_name = GetWindowClassName();
	std::wstring wnd_id = GetWindowId();
	if (!WindowsManager::GetInstance()->RegisterWindow(wnd_class_name, wnd_id, this))
	{
		return false;
	}
	return true;
}

void WindowEx::UnRegisterWnd()
{
	std::wstring wnd_class_name = GetWindowClassName();
	std::wstring wnd_id = GetWindowId();
	WindowsManager::GetInstance()->UnRegisterWindow(wnd_class_name, wnd_id, this);
}

LRESULT WindowEx::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = true;
	if (uMsg == WM_CLOSE)
	{
		if (!::IsWindowEnabled(GetHWND()))
		{
			::SetForegroundWindow(GetHWND());
			return FALSE;
		}
	}
	else if (uMsg == WM_KILLFOCUS)
	{
		KillFocus();
	}
	else if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_ESCAPE)
		{
			bHandled = false;
			OnEsc(bHandled);
			if (!bHandled) {
				this->CloseWnd();
			}
			else {
				return 0;
			}
		}
	}
	bHandled = false;
	return __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

POINT GetPopupWindowPos(WindowEx* window)
{
	ASSERT(window && IsWindow(window->GetHWND()));

	//ÆÁÄ»´óÐ¡
	MONITORINFO oMonitor = { sizeof(oMonitor) };
	::GetMonitorInfo(::MonitorFromWindow(window->GetHWND(), MONITOR_DEFAULTTONEAREST), &oMonitor);
	RECT screen = oMonitor.rcWork;

	ui::UiRect rect = window->GetPos(true);

	POINT pt = { 0, 0 };
	pt.x = screen.right - rect.GetWidth();
	pt.y = screen.bottom - rect.GetHeight();

	return pt;
}

}