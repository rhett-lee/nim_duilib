#include "cef_native_control.h"
#include "ui_components/cef_control/handler/browser_handler.h"
#include "ui_components/cef_control/manager/cef_manager.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#pragma warning (pop)

namespace nim_comp {

CefNativeControl::CefNativeControl(ui::Window* pWindow):
    CefControlBase(pWindow)
{
}

CefNativeControl::~CefNativeControl(void)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        // Request that the main browser close.
        browser_handler_->GetBrowserHost()->CloseBrowser(true);
        browser_handler_->SetHostWindow(NULL);
        browser_handler_->SetHandlerDelegate(NULL);
    }
}

void CefNativeControl::Init()
{
    if (browser_handler_.get() == nullptr)
    {
        HWND hWnd = GetWindow()->NativeWnd()->GetHWND();
        LONG style = ::GetWindowLong(hWnd, GWL_STYLE);
        ::SetWindowLong(hWnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        ASSERT((::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) == 0 && _T("无法在分层窗口内使用本控件"));

        browser_handler_ = new nim_comp::BrowserHandler;
        browser_handler_->SetHostWindow(GetWindow());
        browser_handler_->SetHandlerDelegate(this);
        ReCreateBrowser();
    }

    if (!js_bridge_.get())
    {
        js_bridge_.reset(new nim_comp::CefJSBridge);
    }

    BaseClass::Init();
}

void CefNativeControl::ReCreateBrowser()
{
    if (browser_handler_->GetBrowser() == nullptr)
    {
        // 使用有窗模式
        CefWindowInfo window_info;
        RECT rect = { GetRect().left, GetRect().top, GetRect().right, GetRect().bottom};
        window_info.SetAsChild(this->GetWindow()->NativeWnd()->GetHWND(), rect);

        CefBrowserSettings browser_settings;
        CefBrowserHost::CreateBrowser(window_info, browser_handler_, _T(""), browser_settings, NULL);
    }
}

void CefNativeControl::SetPos(ui::UiRect rc)
{
    BaseClass::SetPos(rc);

    HWND hwnd = GetCefHandle();
    if (hwnd) 
    {
        ::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
    }
}

void CefNativeControl::HandleEvent(const ui::EventArgs& msg)
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
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() == NULL) {
        return BaseClass::HandleEvent(msg);
    }

    else if (msg.eventType == ui::kEventSetFocus) {
        browser_handler_->GetBrowserHost()->SetFocus(true);
    }
    else if (msg.eventType == ui::kEventKillFocus) {
        browser_handler_->GetBrowserHost()->SetFocus(false);
    }
    BaseClass::HandleEvent(msg);
}

void CefNativeControl::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);

    HWND hwnd = GetCefHandle();
    if (hwnd)
    {
        if (bVisible)
        {
            ShowWindow(hwnd, SW_SHOW);
        }
        else
        {
            ::SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }
}

void CefNativeControl::SetWindow(ui::Window* pWindow)
{
    if (pWindow == nullptr) {
        return;
    }
    if (browser_handler_)
        browser_handler_->SetHostWindow(pWindow);

    // 设置Cef窗口句柄为新的主窗口的子窗口
    auto hwnd = GetCefHandle();
    if (hwnd)
        ::SetParent(hwnd, pWindow->NativeWnd()->GetHWND());

    // 为新的主窗口重新设置WS_CLIPSIBLINGS、WS_CLIPCHILDREN样式，否则Cef窗口刷新会出问题
    LONG style = ::GetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE);
    ::SetWindowLong(pWindow->NativeWnd()->GetHWND(), GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    BaseClass::SetWindow(pWindow);
}

bool CefNativeControl::AttachDevTools(Control* /*view*/)
{
    if (devtool_attached_)
        return true;

    auto browser = browser_handler_->GetBrowser();
    if (browser == nullptr)
    {
        auto task = ToWeakCallback([this]()
        {
                ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this](){
                AttachDevTools(nullptr);
            }));
        });

        browser_handler_->AddAfterCreateTask(task);
    }
    else
    {
        CefWindowInfo windowInfo;
        windowInfo.SetAsPopup(NULL, _T("cef_devtools"));
        CefBrowserSettings settings;
        windowInfo.width = 900;
        windowInfo.height = 700;
        browser->GetHost()->ShowDevTools(windowInfo, new nim_comp::BrowserHandler, settings, CefPoint());
        devtool_attached_ = true;
        if (cb_devtool_visible_change_ != nullptr)
            cb_devtool_visible_change_(devtool_attached_);
    }
    return true;
}

}
