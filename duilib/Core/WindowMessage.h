#ifndef UI_CORE_WINDOW_MESSAGE_H_
#define UI_CORE_WINDOW_MESSAGE_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui {

/** 窗口大小改变的类型（WM_SIZE消息的参数）
*/
enum class WindowSizeType
{
    kSIZE_RESTORED  = 0,    //窗口已调整大小，但 kSIZE_MINIMIZED 和 kSIZE_MAXIMIZED 值都不适用
    kSIZE_MINIMIZED = 1,    //窗口已最小化
    kSIZE_MAXIMIZED = 2,    //窗口已最大化
    kSIZE_MAXSHOW   = 3,    //当其他一些窗口还原到其之前的大小时，消息将发送到所有弹出窗口
    kSIZE_MAXHIDE   = 4     //当其他一些窗口最大化时，消息将发送到所有弹出窗口
};

/** ShowWindow的命令
*/
enum ShowWindowCommands
{
    kSW_HIDE                = 0, //隐藏窗口并激活另一个窗口
    kSW_SHOW_NORMAL         = 1, //激活并显示窗口。 如果窗口最小化、最大化或排列，系统会将其还原到其原始大小和位置。应用程序应在首次显示窗口时指定此标志。
    kSW_SHOW_MINIMIZED      = 2, //激活窗口并将其显示为最小化窗口
    kSW_SHOW_MAXIMIZED      = 3, //激活窗口并显示最大化的窗口
    kSW_SHOW_NOACTIVATE     = 4, //以最近的大小和位置显示窗口。 此值类似于 kSW_SHOW_NORMAL，只是窗口未激活
    kSW_SHOW                = 5, //激活窗口并以当前大小和位置显示窗口
    kSW_MINIMIZE            = 6, //最小化指定的窗口，并按 Z 顺序激活下一个顶级窗口
    kSW_SHOW_MIN_NOACTIVE   = 7, //将窗口显示为最小化窗口。 此值类似于 kSW_SHOW_MINIMIZED，但窗口未激活
    kSW_SHOW_NA             = 8, //以当前大小和位置显示窗口。 此值类似于 SW_SHOW，只是窗口未激活
    kSW_RESTORE             = 9  //激活并显示窗口。 如果窗口最小化、最大化或排列，系统会将其还原到其原始大小和位置。 还原最小化窗口时，应用程序应指定此标志。
};

/* SetWindowPos Flags
 */
enum WindowPosFlags
{
    kSWP_NOSIZE         = 0x0001,
    kSWP_NOMOVE         = 0x0002,
    kSWP_NOZORDER       = 0x0004,
    kSWP_NOREDRAW       = 0x0008,
    kSWP_NOACTIVATE     = 0x0010,
    kSWP_FRAMECHANGED   = 0x0020,  /* The frame changed: send WM_NCCALCSIZE */
    kSWP_SHOWWINDOW     = 0x0040,
    kSWP_HIDEWINDOW     = 0x0080,
    kSWP_NOCOPYBITS     = 0x0100,
    kSWP_NOOWNERZORDER  = 0x0200,  /* Don't do owner Z ordering */
    kSWP_NOSENDCHANGING = 0x0400,  /* Don't send WM_WINDOWPOSCHANGING */
    kSWP_DEFERERASE     = 0x2000,  // same as SWP_DEFERDRAWING
    kSWP_ASYNCWINDOWPOS = 0x4000   // same as SWP_CREATESPB
};

/** SetWindowPos 的 hWndInsertAfter 参数标识
*/
enum class InsertAfterFlag
{
    kHWND_DEFAULT   =  0,
    kHWND_NOTOPMOST = -2,
    kHWND_TOPMOST   = -1,
    kHWND_TOP       =  0,
    kHWND_BOTTOM    =  1
};

class WindowBase;
/** SetWindowPos 的 hWndInsertAfter 参数
*/
class InsertAfterWnd
{
public:
    InsertAfterWnd():
        m_pWindow(nullptr),
        m_hwndFlag(InsertAfterFlag::kHWND_TOP)
    {
    }
    explicit InsertAfterWnd(WindowBase* pWindow):
        m_pWindow(pWindow),
        m_hwndFlag(InsertAfterFlag::kHWND_TOP)
    {
    }
    explicit InsertAfterWnd(InsertAfterFlag flag) :
        m_pWindow(nullptr),
        m_hwndFlag(flag)
    {
    }
    /** 窗口指针
    */
    WindowBase* m_pWindow;

    /** 窗口标志(仅当m_pWindow为nullptr时生效)
    */
    InsertAfterFlag m_hwndFlag;
};

/** 窗口消息原始数据
*/
class NativeMsg
{
public:
    NativeMsg():
        uMsg(0),
        wParam(0),
        lParam(0)
    {
    }
    NativeMsg(uint32_t u, WPARAM w, LPARAM l):
        uMsg(u),
        wParam(w),
        lParam(l)
    {
    }
    /** 消息ID
    */
    uint32_t uMsg;

    /** 消息第一个参数
    */
    WPARAM wParam;

    /** 消息第一个参数
    */
    LPARAM lParam;
};


/** 窗口消息定义，只定义部分使用到的消息(和Windows系统定义一致，WinUser.h)
*/
enum WindowMessage{
#if defined(DUILIB_BUILD_FOR_SDL)
    kWM_USER = 0x8000 + 32, //SDL_EVENT_USER
#else
    kWM_USER = 0x0400, //WM_USER
#endif
};


} // namespace ui

#endif // UI_CORE_WINDOW_MESSAGE_H_

