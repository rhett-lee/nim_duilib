#include "CefWindowUtils.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_LINUX
//Linux OS
#include <X11/Xlib.h>

namespace ui
{
//判断窗口是否有效
static bool IsX11WindowValid(Display* display, ::Window window)
{
    // 尝试获取窗口属性
    XWindowAttributes attrs;        
    Status status = XGetWindowAttributes(display, window, &attrs);
    return (status != 0);  // 返回1有效，0无效
}

//设置X窗口的父窗口
class SetX11WindowParentWindowTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowParentWindowTask);
public:
    explicit SetX11WindowParentWindowTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired()) {
            return;
        }
        CefWindowHandle hParentHandle = 0;
        Window* pWindow = m_pCefControl->GetWindow();
        if (pWindow != nullptr) {
            hParentHandle = pWindow->NativeWnd()->GetX11WindowNumber();
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        if ((handle != 0) && (hParentHandle != 0)) {
            Display* display = XOpenDisplay(nullptr);
            if ((display != nullptr) && IsX11WindowValid(display, handle) && IsX11WindowValid(display, hParentHandle)) {
                UiRect rc = m_pCefControl->GetPos();
                XReparentWindow(display, handle, hParentHandle, rc.left, rc.top);
                XFlush(display);
                XCloseDisplay(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

//设置X11窗口的显示或者隐藏
class SetX11WindowVisibleTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowVisibleTask);
public:
    explicit SetX11WindowVisibleTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired()) {
            return;
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        if (handle != 0) {
            Display* display = XOpenDisplay(nullptr);
            if ((display != nullptr) && IsX11WindowValid(display, handle)){
                if (m_pCefControl->IsVisible()) {
                    XMapWindow(display, handle);
                }
                else {
                    XUnmapWindow(display, handle);
                }
                XFlush(display);
                XCloseDisplay(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

//设置X11窗口的位置和大小
class SetX11WindowPosTask : public CefTask
{
    IMPLEMENT_REFCOUNTING(SetX11WindowPosTask);
public:
    explicit SetX11WindowPosTask(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        m_pCefControlFlag = pCefControl->GetWeakFlag();
    }
public:
    virtual void Execute() override
    {
        if (m_pCefControlFlag.expired() || !m_pCefControl->IsVisible()) {
            //窗口隐藏的时候，不需要设置；如果设置的话，会导致程序崩溃
            return;
        }
        CefWindowHandle handle = m_pCefControl->GetCefWindowHandle();
        ui::UiRect rc = m_pCefControl->GetPos();
        if (handle != 0) {
            Display* display = XOpenDisplay(nullptr);
            if ((display != nullptr) && IsX11WindowValid(display, handle)){
                XMoveResizeWindow(display, handle, rc.left, rc.top, rc.Width(), rc.Height());
                XFlush(display);
                XCloseDisplay(display);
            }
        }
    }
private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    CefPostTask(TID_UI, new SetX11WindowPosTask(pCefControl));
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
    //需要在CEF的UI线程调整
    CefPostTask(TID_UI, new SetX11WindowVisibleTask(pCefControl));
}

void SetCefWindowParent(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    //需要在CEF的UI线程调整
    CefPostTask(TID_UI, new SetX11WindowParentWindowTask(pCefControl));
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_LINUX
