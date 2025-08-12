#include "SDL_Linux.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace ui
{
/** 封装Linux的SetFocus函数，功能类似于Windows的SetFocus(HWND)
*/
bool SetFocus_Linux(uint64_t x11WindowNumber)
{
    // 获取显示连接
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        return false; // 无法打开显示
    }
    // RAII资源管理
    struct DisplayCloser {
        Display* d;
        ~DisplayCloser() { if (d) ::XCloseDisplay(d); }
    } closer{ display };

    // 获取当前窗口句柄
    ::Window hWnd = x11WindowNumber;
    if (hWnd == None) {
        return false;
    }

    // 获取当前焦点窗口
    ::Window focusWindow = None;
    int revertTo = 0;
    XGetInputFocus(display, &focusWindow, &revertTo);

    // 如果当前焦点不在目标窗口，则设置焦点
    if (focusWindow != hWnd) {
        // 设置焦点到目标窗口，最后一个参数是时间戳，CurrentTime表示立即
        XSetInputFocus(display, hWnd, RevertToParent, CurrentTime);
        // 刷新显示以确保操作生效
        XFlush(display);
    }
    return true;
}

}

#endif //DUILIB_BUILD_FOR_LINUX
