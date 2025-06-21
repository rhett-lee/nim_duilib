#include "ClickThrough.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse)
{
    if (pWindow == nullptr) {
        return false;
    }
    POINT ptScreen;
    ptScreen.x = ptMouse.x;
    ptScreen.y = ptMouse.y;

    HWND hWnd = pWindow->NativeWnd()->GetHWND();
    ::SetWindowLong(hWnd, GWL_EXSTYLE, ::GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
    HWND hWndUnder = ::WindowFromPoint(ptScreen);
    ::SetWindowLong(hWnd, GWL_EXSTYLE, ::GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

    // 如果获取的是子窗口，则获取其父窗口
    while (::IsWindow(hWndUnder)) {
        HWND hParentWnd = ::GetParent(hWndUnder);
        if ((hParentWnd == nullptr) || (hParentWnd == ::GetDesktopWindow())) {
            break;
        }
        hWndUnder = hParentWnd;
    }
    if (::IsWindow(hWnd) && ::IsWindow(hWndUnder)) {
        DWORD dwThreadId = ::GetWindowThreadProcessId(hWnd, nullptr);
        DWORD dwUnderThreadId = ::GetWindowThreadProcessId(hWndUnder, nullptr);
        ::AttachThreadInput(dwThreadId, dwUnderThreadId, TRUE);
        ::SetActiveWindow(hWndUnder);
        ::AttachThreadInput(dwThreadId, dwUnderThreadId, FALSE);
        return true;
    }
    return false;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_WIN
