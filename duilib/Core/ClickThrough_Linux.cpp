#include "ClickThrough.h"
#include "duilib/Core/Window.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h> 
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <sys/types.h>
#include <iostream>

#include "duilib/Utils/LogUtil.h"

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

// 获取窗口对应的进程ID（仅调试时使用）
[[maybe_unused]] static pid_t GetWindowPID(Display* display, ::Window window)
{
    if (!display) {
        return 0;
    }

    // 获取_NET_WM_PID原子
    Atom netWmPid = XInternAtom(display, "_NET_WM_PID", False);
    if (netWmPid == None) {
        return -1;
    }
        
    // 查询窗口属性
    Atom actualType;
    int format;
    unsigned long numItems, bytesAfter;
    unsigned char* prop = nullptr;
    
    // 请求PID属性
    Status status = XGetWindowProperty(
        display, window, netWmPid, 0, 1, False, XA_CARDINAL,
        &actualType, &format, &numItems, &bytesAfter, &prop
    );
    
    if (status != Success || prop == nullptr || numItems == 0) {
        if (prop) {
            XFree(prop);
        }
        return -1;
    }
    
    // 获取PID值
    pid_t pid = static_cast<pid_t>(*reinterpret_cast<unsigned long*>(prop));
    XFree(prop);
    
    return pid;
}

// 获取窗口标题（调试辅助函数，当前仅在调试时使用）
[[maybe_unused]] static std::string GetWindowTitle(Display* display, ::Window window)
{
    if (!display) {
        return "";
    }
    
    std::string title;
    
    // 尝试获取UTF-8编码的窗口标题 (_NET_WM_NAME)
    Atom utf8Atom = XInternAtom(display, "_NET_WM_NAME", False);
    Atom actualType;
    int format;
    unsigned long numItems, bytesAfter;
    unsigned char* prop = nullptr;
    
    if (XGetWindowProperty(display, window, utf8Atom, 0, 1024, False, 
                          XA_STRING, &actualType, &format, &numItems, 
                          &bytesAfter, &prop) == Success && prop) {
        title = reinterpret_cast<char*>(prop);
        XFree(prop);
        prop = nullptr;
    }
    
    // 如果没有获取到UTF-8标题，尝试获取传统WM_NAME
    if (title.empty()) {
        Atom wmNameAtom = XInternAtom(display, "WM_NAME", False);
        if (XGetWindowProperty(display, window, wmNameAtom, 0, 1024, False, 
                              AnyPropertyType, &actualType, &format, &numItems, 
                              &bytesAfter, &prop) == Success && prop) {
            // 检查类型是否为字符串或UTF8_STRING
            if (actualType == XA_STRING || actualType == utf8Atom) {
                title = reinterpret_cast<char*>(prop);
            }
            XFree(prop);
            prop = nullptr;
        }
    }

    if (title.empty()) {
        char *name = NULL;
        XFetchName(display, window, &name);  // 获取窗口标题
        if (name != nullptr) {
            title = name;
        }        
        XFree(name);
    }
    return title;
}

// 获取窗口的父窗口/属主窗口(WM_TRANSIENT_FOR)链，按由近及远排列
static void GetTransientForChain(Display* display, ::Window window, std::vector<::Window>& parentChain)
{
    parentChain.clear();
    if ((display == nullptr) || (window == None)) {
        return;
    }
    Atom transientFor = XInternAtom(display, "WM_TRANSIENT_FOR", False);
    if (transientFor == None) {
        return;
    }
    ::Window current = window;
    //限制最大层数并检测重复，避免异常情况下出现死循环
    const int32_t kMaxParentCount = 32;
    while (parentChain.size() < (size_t)kMaxParentCount) {
        Atom actualType = None;
        int format = 0;
        unsigned long numItems = 0;
        unsigned long bytesAfter = 0;
        unsigned char* prop = nullptr;
        if (XGetWindowProperty(display, current, transientFor, 0, 1, False, XA_WINDOW,
                               &actualType, &format, &numItems, &bytesAfter, &prop) != Success) {
            break;
        }
        if ((prop == nullptr) || (numItems == 0) || (actualType != XA_WINDOW)) {
            if (prop != nullptr) {
                XFree(prop);
            }
            break;
        }
        ::Window parent = *reinterpret_cast<::Window*>(prop);
        XFree(prop);
        if ((parent == None) || (parent == current) ||
            (std::find(parentChain.begin(), parentChain.end(), parent) != parentChain.end())) {
            break;
        }
        parentChain.push_back(parent);
        current = parent;
    }
}

// 递归获取窗口及其所有子窗口
static void GetAllChildWindows(Display* display, ::Window window, std::vector<::Window>& windows) 
{
    if ((display == nullptr) || (window == None)) {
        return;
    }
    ::Window root = None;
    ::Window parent = None;
    ::Window* children = nullptr;
    unsigned int nchildren =0 ;    
    if (XQueryTree(display, window, &root, &parent, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; i++) {
            windows.push_back(children[i]);
            GetAllChildWindows(display, children[i], windows);
        }
        XFree(children);
    }
}

// 获取桌面下的所有窗口，按Z序排序
static void GetDisplayOrderedWindows(Display* display, std::vector<::Window>& windows)
{
    windows.clear();
    if (display == nullptr) {
        return;
    }
    ::Window root = DefaultRootWindow(display);
    
    // 获取Z序窗口列表（EWMH方式）
    Atom stack_atom = XInternAtom(display, "_NET_CLIENT_LIST_STACKING", False);
    Atom type;
    int format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    ::Window* stack_windows = nullptr;    
    if(::XGetWindowProperty(display, root, stack_atom, 0, ~0L, False,
                            XA_WINDOW, &type, &format, &nitems, &bytes_after,
                            (unsigned char**)&stack_windows) == Success) {
        
        //从顶层到底层的窗口Z序
        if (stack_windows != nullptr) {
            for(int i = (int)nitems-1; i >=0; i--) {
                windows.push_back(stack_windows[i]);
            }
            XFree(stack_windows);
        }
        if (!windows.empty()) {
            std::reverse(windows.begin(), windows.end());
        }
    }
    else {
        // 回退到XQueryTree方式
        ::Window root_return = None;
        ::Window parent_return = None;
        ::Window* children = nullptr;
        unsigned int nchildren = 0;
        if (XQueryTree(display, root, &root_return, &parent_return, &children, &nchildren)) {
            //从底层到顶层的窗口Z序
            if (children != nullptr) {
                for(int i = (int)nchildren-1; i >=0; i--) {
                    windows.push_back(children[i]);
                }
                XFree(children);
            }
            //与EWMH分支保持一致：结果按"从底层到顶层"排列
            if (!windows.empty()) {
                std::reverse(windows.begin(), windows.end());
            }
        }
    }
}

static ::Window FindWindowBelow(Display* display, ::Window window, int rootX, int rootY) 
{
    // 获取当前窗口及其所有子窗口
    std::vector<::Window> allWindows;
    allWindows.push_back(window);
    GetAllChildWindows(display, window, allWindows);

    // 获取根窗口的子窗口列表（已按Z序排列）
    std::vector<::Window> allDisplayWindows;
    GetDisplayOrderedWindows(display, allDisplayWindows);

    ////TEST
    //if(0) {
    //    std::cout << "allDisplayWindows: " << allDisplayWindows.size() << std::endl;
    //    for (::Window windowNum : allDisplayWindows) {
    //        XWindowAttributes attrs;
    //        if (XGetWindowAttributes(display, windowNum, &attrs)) {
    //            std::string windowText = GetWindowTitle(display, windowNum);
    //            pid_t targetPid = GetWindowPID(display, windowNum);    
    //            std::cout << "Window ID: " << windowNum 
    //                        << " | Name: " << (!windowText.empty() ? windowText.c_str() : "N/A") 
    //                        << " | PID: " << targetPid ;

    //            std::cout << " | 坐标: (" << attrs.x << ", " << attrs.y << ")"
    //                    << " | 尺寸: " << attrs.width << "x" << attrs.height
    //                    << " | 可视状态: " << (attrs.map_state == IsUnmapped ? "未映射" : 
    //                                        attrs.map_state == IsUnviewable ? "不可见" : "可见")
    //                    << std::endl;
    //        }
    //    }
    //}
    ////TEST

    // 查找目标窗口在Z序中的位置
    int targetIndex = -1;
    for (size_t i = 0; i < allDisplayWindows.size(); ++i) {
        for (const auto& w : allWindows) {
            if (allDisplayWindows[i] == w) {
                targetIndex = (int)i;
                break;
            }
        }
        if (targetIndex != -1) {
            break;
        }
    }

    ::Window result = None;
    if (targetIndex == -1) {
        //_NET_CLIENT_LIST_STACKING 不包含 override-redirect 窗口（例如菜单类弹层），
        //回退到 XQueryTree 获取根窗口子窗口的Z序（从底层到顶层），重新定位目标窗口
        std::vector<::Window> rootChildren;
        ::Window rootReturn = None;
        ::Window parentReturn = None;
        ::Window* children = nullptr;
        unsigned int nchildren = 0;
        if (::XQueryTree(display, DefaultRootWindow(display), &rootReturn, &parentReturn, &children, &nchildren) && (children != nullptr)) {
            for (unsigned int i = 0; i < nchildren; i++) {
                rootChildren.push_back(children[i]);
            }
            ::XFree(children);
        }
        allDisplayWindows.swap(rootChildren);
        for (size_t i = 0; i < allDisplayWindows.size(); ++i) {
            for (const auto& w : allWindows) {
                if (allDisplayWindows[i] == w) {
                    targetIndex = (int)i;
                    break;
                }
            }
            if (targetIndex != -1) {
                break;
            }
        }
    }
    if (targetIndex != -1) {
        // 检查Z序中位于目标窗口下方的窗口
        ::Window root = DefaultRootWindow(display);
        for (int i = targetIndex - 1; i >= 0; i--) {
            ::Window candidate = allDisplayWindows[i];
            XWindowAttributes attrs;
            if (XGetWindowAttributes(display, candidate, &attrs) && (attrs.map_state == IsViewable)) {                
                // 坐标转换和命中测试
                int x = 0;
                int y = 0;
                ::Window dummy = None;
                if (XTranslateCoordinates(display, candidate, root, 0, 0, &x, &y, &dummy)) {                
                    if (rootX >= (x - attrs.border_width) && 
                        rootX < (x + attrs.width + attrs.border_width) &&
                        rootY >= (y - attrs.border_width) && 
                        rootY < (y + attrs.height + attrs.border_width)) {
                        result = candidate;
                        break;
                    }
                }
            }
        }
    }
    return result;
}

/** 激活指定窗口（提升到前端并获取焦点）
 * @param display 已经打开的X11连接
 * @param window  目标窗口ID
 * @param force   是否强制激活（忽略焦点策略）
 * @return        成功返回1，失败返回0
 */
static int ActivateX11Window(Display* display, ::Window window, int force) 
{
    if ((display == nullptr) || (window == None)) {
        return 0;
    }

    // 获取当前窗口属性
    ::XWindowAttributes currentAttrs;
    if (!::XGetWindowAttributes(display, window, &currentAttrs)) {
        return 0;
    }

    // 获取根窗口和必要的原子
    ::Window root = RootWindow(display, DefaultScreen(display));
    Atom wm_active = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    Atom wm_supported = XInternAtom(display, "_NET_SUPPORTED", False);
    Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
    Atom wm_take_focus = XInternAtom(display, "WM_TAKE_FOCUS", False);
    
    // 检查窗口管理器是否支持_NET_ACTIVE_WINDOW协议
    Atom actual_type = None;
    int format = 0;
    unsigned long num_items = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = nullptr;
    int supports_netwm = 0;
    
    if (wm_active && (::XGetWindowProperty(display, root, wm_supported, 0, 1024, False,
                                           XA_ATOM, &actual_type, &format, &num_items, &bytes_after, &prop) == Success) &&
        (actual_type == XA_ATOM) && (format == 32) && (prop != nullptr)) {
        Atom *supported = (Atom*)prop;
        for (unsigned long i = 0; i < num_items; i++) {
            if (supported[i] == wm_active) {
                supports_netwm = 1;
                break;
            }
        }
    }
    if (prop != nullptr) {
        XFree(prop);
        prop = nullptr;
    }
    
    // 1. 使用_NET_ACTIVE_WINDOW协议请求窗口管理器激活窗口（优先方法）
    if (supports_netwm) {
        XClientMessageEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.window = window;         //需要激活的窗口
        ev.message_type = wm_active;
        ev.format = 32;
        ev.data.l[0] = 1;           //source indication: application
        ev.data.l[1] = CurrentTime; //时间戳
        ev.data.l[2] = None;        //当前活动窗口（未知）
        ev.data.l[3] = 0;
        ev.data.l[4] = 0;
        
        ::XSendEvent(display, root, False, 
                     SubstructureRedirectMask | SubstructureNotifyMask,
                     (XEvent*)&ev);
        ::XFlush(display);
        return 1;
    }
    
    // 2. 窗口管理器不支持EWMH协议时，回退到Xlib的方式
    if (wm_protocols && wm_take_focus) {
        XClientMessageEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.window = window;
        ev.message_type = wm_protocols;
        ev.format = 32;
        ev.data.l[0] = wm_take_focus;
        ev.data.l[1] = CurrentTime;
        
        ::XSendEvent(display, window, False, 0, (XEvent*)&ev);
    }
    if (currentAttrs.map_state != IsViewable) {
        ::XMapRaised(display, window);
    }
    ::XRaiseWindow(display, window);
    if (force) {
        ::XSetInputFocus(display, window, RevertToParent, CurrentTime);
    }
    else {
        ::XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);
    }
    ::XSync(display, False);
    return 1;
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse)
{
    // 参数校验
    if (!pWindow || !pWindow->NativeWnd()) {
        return false;
    }

    const char* sessionType = std::getenv("XDG_SESSION_TYPE");
    //只支持X11桌面环境
    if ((sessionType == nullptr) ||
        ((std::string(sessionType) != "x11") && (std::string(sessionType) != "X11"))) {
        //Wayland等非X11环境不支持窗口激活，仅输出一次日志，避免反复输出
        static bool bLoggedOnce = false;
        if (!bLoggedOnce) {
            bLoggedOnce = true;
            LogUtil::OutputLine(_T("ClickThroughWindow: 当前会话为Wayland，阴影区域的点击穿透由窗口输入区域实现，此处不再激活窗口"), false);
        }
        return false;
    }

    Display* display = ::XOpenDisplay(nullptr);
    if (!display) {
        return false;
    }

    // RAII资源管理
    struct DisplayCloser {
        Display* d;
        ~DisplayCloser() { if(d) ::XCloseDisplay(d); }
    } closer{display};

    // 获取当前窗口属性
    ::Window x11Window = pWindow->NativeWnd()->GetX11WindowNumber();
    ::XWindowAttributes currentAttrs;
    if (!::XGetWindowAttributes(display, x11Window, &currentAttrs)) {
        return false;
    }

    // 临时禁用当前窗口事件
    ::XSetWindowAttributes newAttrs;
    newAttrs.event_mask = currentAttrs.all_event_masks & ~(ButtonPressMask|ButtonReleaseMask);
    ::XChangeWindowAttributes(display, x11Window, CWEventMask, &newAttrs);
    
    //ptMouse为屏幕坐标，在X11中即根窗口坐标
    const int rootX = ptMouse.x;
    const int rootY = ptMouse.y;

    // 递归查找鼠标位置下的可见窗口
    ::Window targetWindow = FindWindowBelow(display, x11Window, rootX, rootY);

    // 恢复窗口事件
    ::XSetWindowAttributes restoreAttrs;
    restoreAttrs.event_mask = currentAttrs.all_event_masks;
    ::XChangeWindowAttributes(display, x11Window, CWEventMask, &restoreAttrs);

    // 激活目标窗口
    if ((targetWindow != None) && (targetWindow != x11Window)) {
        //鼠标下方是当前窗口的父窗口/属主窗口(WM_TRANSIENT_FOR，典型情况是子界面后方的主界面)时，
        //保持现状，不切换前台窗口。
        //注意：不能按进程判断，因为同一进程可能有多个独立的顶层窗口(例如多标签浏览器)
        std::vector<::Window> parentChain;
        GetTransientForChain(display, x11Window, parentChain);
        if (std::find(parentChain.begin(), parentChain.end(), targetWindow) != parentChain.end()) {
            return false;
        }
        int nRet = ActivateX11Window(display, targetWindow, 1);
        if (nRet == 0) {
            LogUtil::OutputLine(_T("ClickThroughWindow: 激活窗口失败(_NET_ACTIVE_WINDOW)"), false);
        }
        return nRet != 0;
    }
    return false;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_LINUX
