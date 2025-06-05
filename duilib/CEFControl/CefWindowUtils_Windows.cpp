#include "CefWindowUtils.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hwnd = (HWND)cefWindow;
    if (::IsWindow(hwnd)) {
        UiRect rc = pCefControl->GetPos();
        ::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
    }
}

void SetCefWindowVisible(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hwnd = (HWND)cefWindow;
    if (hwnd) {
        if (pCefControl->IsVisible()) {
            ShowWindow(hwnd, SW_SHOW);
        }
        else {
            ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }
}

void SetCefWindowParent(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    if (!::IsWindow((HWND)cefWindow)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hParent = pWindow->NativeWnd()->GetHWND();
    if (!::IsWindow(hParent)) {
        return;
    }
    ::SetParent((HWND)cefWindow, hParent);

    // 为新的主窗口重新设置WS_CLIPSIBLINGS、WS_CLIPCHILDREN样式，否则Cef窗口刷新会出问题
    LONG style = ::GetWindowLong(hParent, GWL_STYLE);
    ::SetWindowLong(hParent, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
