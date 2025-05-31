#include "CefControlNative.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"

#ifdef DUILIB_BUILD_FOR_LINUX
    //Linux OS
    #include <X11/Xlib.h>
    
    //判断窗口是否有效
    static bool IsX11WindowValid(Display* display, ::Window window)
    {
        // 尝试获取窗口属性
        XWindowAttributes attrs;        
        Status status = XGetWindowAttributes(display, window, &attrs);
        return (status != 0);  // 返回1有效，0无效
    }

#endif

namespace ui {

CefControlNative::CefControlNative(ui::Window* pWindow):
    CefControl(pWindow),
    m_bWindowFirstShown(false)
{
}

CefControlNative::~CefControlNative(void)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        // Request that the main browser close.
        if (m_pBrowserHandler->GetBrowserHost() != nullptr) {
            m_pBrowserHandler->GetBrowserHost()->CloseBrowser(true);
        }
        m_pBrowserHandler->SetHostWindow(nullptr);
        m_pBrowserHandler->SetHandlerDelegate(nullptr);
    }
}

void CefControlNative::Init()
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pBrowserHandler.get() == nullptr) {
#ifdef DUILIB_BUILD_FOR_WIN
        //检测是否在分层窗口中创建控件
        HWND hWnd = GetWindow()->NativeWnd()->GetHWND();
        LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
        ::SetWindowLong(hWnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        ASSERT((::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) == 0 && _T("CefControlNative：无法在分层窗口内使用本控件"));
#endif
        m_pBrowserHandler = new CefBrowserHandler;
        m_pBrowserHandler->SetHostWindow(GetWindow());
        m_pBrowserHandler->SetHandlerDelegate(this);

        //异步创建Browser对象, 避免阻塞主界面的解析和显示速度
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControlNative::ReCreateBrowser, this));
    }

    if (!m_jsBridge.get()) {
        m_jsBridge.reset(new ui::CefJSBridge);
    }
    BaseClass::Init();
}

void CefControlNative::ReCreateBrowser()
{
    GlobalManager::Instance().AssertUIThread();
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return;
    }
    ASSERT(pWindow->IsWindow());
    ASSERT(m_pBrowserHandler != nullptr);
    if (m_pBrowserHandler == nullptr) {
        return;
    }
    if (m_pBrowserHandler->GetBrowser() != nullptr) {
        //已经完成创建，不再重复创建Browser对象
        return;
    }

    //如果窗口没有完成首次显示，则等首次显示时再创建，避免出现闪黑屏现象
    if (!pWindow->IsWindowFirstShown()) {
        if (!m_bWindowFirstShown) {
            std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
            pWindow->AttachWindowFirstShown([weakFlag, this](const EventArgs& /*args*/) {
                if (!weakFlag.expired()) {
                    ReCreateBrowser();
                }
                return true;
                });
            m_bWindowFirstShown = true;
        }
        return;
    }

    // 使用有窗模式
    CefWindowInfo window_info;
    CefRect rect = { GetRect().left, GetRect().top, GetRect().right, GetRect().bottom};
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows
    window_info.SetAsChild(pWindow->NativeWnd()->GetHWND(), rect);
#elif defined DUILIB_BUILD_FOR_LINUX
    //Linux
    window_info.SetAsChild(pWindow->NativeWnd()->GetX11WindowNumber(), rect);
#endif

    CefBrowserSettings browser_settings;
    CefString url = GetInitURL();//创建成功后，立即加载的URL
    CefBrowserHost::CreateBrowser(window_info, m_pBrowserHandler, url, browser_settings, nullptr, nullptr);
}

#ifdef DUILIB_BUILD_FOR_LINUX
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
#endif

void CefControlNative::SetPos(ui::UiRect rc)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetPos(rc);
#ifdef DUILIB_BUILD_FOR_WIN
    HWND hwnd = GetCefWindowHandle();
    if (::IsWindow(hwnd)) {
        ::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
    }
#elif defined DUILIB_BUILD_FOR_LINUX
    CefPostTask(TID_UI, new SetX11WindowPosTask(this));
#endif
}

bool CefControlNative::OnSetFocus(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(true);
    }
    return BaseClass::OnSetFocus(msg);
}

bool CefControlNative::OnKillFocus(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(false);
    }
    return BaseClass::OnKillFocus(msg);
}

#ifdef DUILIB_BUILD_FOR_LINUX
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
#endif

void CefControlNative::SetVisible(bool bVisible)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetVisible(bVisible);
#ifdef DUILIB_BUILD_FOR_WIN
    HWND hwnd = GetCefWindowHandle();
    if (hwnd) {
        if (bVisible) {
            ShowWindow(hwnd, SW_SHOW);
        }
        else {
            ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }
#elif defined DUILIB_BUILD_FOR_LINUX
    CefPostTask(TID_UI, new SetX11WindowVisibleTask(this));
#endif
}

void CefControlNative::UpdateCefWindowPos()
{
#ifdef DUILIB_BUILD_FOR_WIN
    HWND hwnd = GetCefWindowHandle();
    ASSERT(::IsWindow(hwnd));
    if (!::IsWindow(hwnd)) {
        //CEF页面的窗口还没完成创建
        return;
    }
#endif

    BaseClass::UpdateCefWindowPos();
    //在Browser控件创建完成后，更新窗口位置和可见性
    SetPos(GetPos());
    SetVisible(IsVisible());
}

void CefControlNative::CloseAllBrowsers()
{
#ifdef DUILIB_BUILD_FOR_WIN
    //关闭窗口时，取消父子关系，避免导致退出时的崩溃问题
    HWND hWnd = GetCefWindowHandle();
    if (::IsWindow(hWnd)) {
        ::SetParent(hWnd, nullptr);
    }
#endif
    BaseClass::CloseAllBrowsers();
}

#ifdef DUILIB_BUILD_FOR_LINUX
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
#endif

void CefControlNative::SetWindow(ui::Window* pWindow)
{
    GlobalManager::Instance().AssertUIThread();
    if ((pWindow == nullptr) || (BaseClass::GetWindow() == pWindow)) {
        return;
    }
    BaseClass::SetWindow(pWindow);

    if (m_pBrowserHandler) {
        m_pBrowserHandler->SetHostWindow(pWindow);
    }
#ifdef DUILIB_BUILD_FOR_WIN
    // 设置Cef窗口句柄为新的主窗口的子窗口
    auto hwnd = GetCefWindowHandle();
    if (hwnd) {
        ::SetParent(hwnd, pWindow->NativeWnd()->GetHWND());
    }

    // 为新的主窗口重新设置WS_CLIPSIBLINGS、WS_CLIPCHILDREN样式，否则Cef窗口刷新会出问题
    LONG style = ::GetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE);
    ::SetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
#elif defined DUILIB_BUILD_FOR_LINUX
    CefPostTask(TID_UI, new SetX11WindowParentWindowTask(this));
#endif    
}

void CefControlNative::OnFocusedNodeChanged(CefRefPtr<CefBrowser> /*browser*/,
                                            CefRefPtr<CefFrame> /*frame*/,
                                            CefDOMNode::Type /*type*/,
                                            bool /*bText*/,
                                            bool /*bEditable*/,
                                            const CefRect& /*nodeRect*/)
{
    if (!IsVisible() || !IsEnabled()) {
        return;
    }
    if (!IsFocused()) {
        //避免双焦点控件的出现
        SetFocus();
    }
}

}
