#include "CefControlNative.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"

namespace ui {

CefControlNative::CefControlNative(ui::Window* pWindow):
    CefControl(pWindow)
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
    if (m_pBrowserHandler.get() == nullptr) {
#if DUILIB_BUILD_FOR_WIN
        //检测是否在分层窗口中创建控件
        HWND hWnd = GetWindow()->NativeWnd()->GetHWND();
        LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
        ::SetWindowLong(hWnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        ASSERT((::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) == 0 && _T("CefControlNative：无法在分层窗口内使用本控件"));
#endif
        m_pBrowserHandler = new CefBrowserHandler;
        m_pBrowserHandler->SetHostWindow(GetWindow());
        m_pBrowserHandler->SetHandlerDelegate(this);
        ReCreateBrowser();
    }

    if (!m_jsBridge.get()) {
        m_jsBridge.reset(new ui::CefJSBridge);
    }
    BaseClass::Init();
}

void CefControlNative::ReCreateBrowser()
{
    if (m_pBrowserHandler->GetBrowser() == nullptr) {
        // 使用有窗模式
        CefWindowInfo window_info;
        CefRect rect = { GetRect().left, GetRect().top, GetRect().right, GetRect().bottom};
#if DUILIB_BUILD_FOR_WIN
        window_info.SetAsChild(this->GetWindow()->NativeWnd()->GetHWND(), rect);
#else
        //TODO:
#endif

        CefBrowserSettings browser_settings;
        CefBrowserHost::CreateBrowser(window_info, m_pBrowserHandler, _T(""), browser_settings, nullptr, nullptr);
    }
}

void CefControlNative::SetPos(ui::UiRect rc)
{
    BaseClass::SetPos(rc);
#if DUILIB_BUILD_FOR_WIN
    HWND hwnd = GetCefHandle();
    if (hwnd) {
        ::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
    }
#endif
}

void CefControlNative::HandleEvent(const ui::EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        ui::Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() == nullptr) {
        return BaseClass::HandleEvent(msg);
    }

    else if (msg.eventType == ui::kEventSetFocus) {
        m_pBrowserHandler->GetBrowserHost()->SetFocus(true);
    }
    else if (msg.eventType == ui::kEventKillFocus) {
        m_pBrowserHandler->GetBrowserHost()->SetFocus(false);
    }
    BaseClass::HandleEvent(msg);
}

void CefControlNative::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);
#if DUILIB_BUILD_FOR_WIN
    HWND hwnd = GetCefHandle();
    if (hwnd) {
        if (bVisible) {
            ShowWindow(hwnd, SW_SHOW);
        }
        else {
            ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }
#endif
}

void CefControlNative::SetWindow(ui::Window* pWindow)
{
    if (pWindow == nullptr) {
        return;
    }
    if (m_pBrowserHandler) {
        m_pBrowserHandler->SetHostWindow(pWindow);
    }
#if DUILIB_BUILD_FOR_WIN
    // 设置Cef窗口句柄为新的主窗口的子窗口
    auto hwnd = GetCefHandle();
    if (hwnd) {
        ::SetParent(hwnd, pWindow->NativeWnd()->GetHWND());
    }

    // 为新的主窗口重新设置WS_CLIPSIBLINGS、WS_CLIPCHILDREN样式，否则Cef窗口刷新会出问题
    LONG style = ::GetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE);
    ::SetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
#endif

    BaseClass::SetWindow(pWindow);
}

bool CefControlNative::AttachDevTools(Control* /*view*/)
{
    if (IsAttachedDevTools()) {
        return true;
    }
    if (m_pBrowserHandler == nullptr) {
        return false;
    }

    auto browser = m_pBrowserHandler->GetBrowser();
    if (browser == nullptr) {
        auto task = ToWeakCallback([this]() {
                ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this](){
                AttachDevTools(nullptr);
            }));
        });
        m_pBrowserHandler->AddAfterCreateTask(task);
    }
    else {
        CefWindowInfo windowInfo;
#if DUILIB_BUILD_FOR_WIN
        windowInfo.SetAsPopup(nullptr, _T("cef_devtools"));
#endif
        CefBrowserSettings settings;
        if (browser->GetHost() != nullptr) {
            browser->GetHost()->ShowDevTools(windowInfo, new ui::CefBrowserHandler, settings, CefPoint());
            SetAttachedDevTools(true);
            OnDevToolsVisibleChanged();
        }
    }
    return IsAttachedDevTools();
}

}
