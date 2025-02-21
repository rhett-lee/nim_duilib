#include "cef_control_base.h"
#include "ui_components/cef_control/handler/browser_handler.h"
#include "ui_components/cef_control/manager/cef_manager.h"
#include "ui_components/cef_control/app/cef_js_bridge.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#pragma warning (pop)

namespace nim_comp {

CefControlBase::CefControlBase(ui::Window* pWindow):
    ui::Control(pWindow)
{
    devtool_attached_ = false;
#if !defined(SUPPORT_CEF)
    ASSERT(FALSE && _T("要使用Cef功能请开启SUPPORT_CEF宏"));
#endif
    //这个标记必须为false，否则绘制有问题
    SetUseCache(false);
}

CefControlBase::~CefControlBase(void)
{

}
void CefControlBase::LoadURL(const CefString& url)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        CefRefPtr<CefFrame> frame = browser_handler_->GetBrowser()->GetMainFrame();
        if (!frame)
            return;

        frame->LoadURL(url);
    }
    else
    {
        if (browser_handler_.get())
        {
            ui::StdClosure cb = ToWeakCallback([this, url]()
            {
                LoadURL(url);
            });
            browser_handler_->AddAfterCreateTask(cb);
        }
    }
}

void CefControlBase::GoBack()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->GoBack();
    }
}

void CefControlBase::GoForward()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->GoForward();
    }
}

bool CefControlBase::CanGoBack()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->CanGoBack();
    }
    return false;
}

bool CefControlBase::CanGoForward()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->CanGoForward();
    }
    return false;
}

void CefControlBase::Refresh()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->Reload();
    }
}

void CefControlBase::StopLoad()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->StopLoad();
    }
}

bool CefControlBase::IsLoading()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->IsLoading();
    }
    return false;
}

void CefControlBase::StartDownload(const CefString& url)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        browser_handler_->GetBrowser()->GetHost()->StartDownload(url);
    }
}

void CefControlBase::SetZoomLevel(float zoom_level)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        browser_handler_->GetBrowser()->GetHost()->SetZoomLevel(zoom_level);
    }
}

HWND CefControlBase::GetCefHandle() const
{
    if (browser_handler_.get() && browser_handler_->GetBrowserHost().get())
        return browser_handler_->GetBrowserHost()->GetWindowHandle();

    return NULL;
}

CefString CefControlBase::GetURL()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return browser_handler_->GetBrowser()->GetMainFrame()->GetURL();
    }

    return CefString();
}

std::string CefControlBase::GetUTF8URL()
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get())
    {
        return ui::StringConvert::WStringToUTF8((const wchar_t*)GetURL().c_str());
    }

    return CefString();
}

CefString CefControlBase::GetMainURL(const CefString& url)
{
    std::string temp = url.ToString();
    size_t end_pos = temp.find("#") == std::string::npos ? temp.length() : temp.find("#");
    temp = temp.substr(0, end_pos);
    return CefString(temp.c_str());
}

bool CefControlBase::RegisterCppFunc(const DString& function_name, nim_comp::CppFunction function, bool global_function/* = false*/)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        return js_bridge_->RegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), function, global_function ? nullptr : browser_handler_->GetBrowser());
    }

    return false;
}

void CefControlBase::UnRegisterCppFunc(const DString& function_name)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        js_bridge_->UnRegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), browser_handler_->GetBrowser());
    }
}

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, nim_comp::CallJsFunctionCallback callback, const DString& frame_name /*= _T("")*/)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        CefRefPtr<CefFrame> frame = frame_name == _T("") ? browser_handler_->GetBrowser()->GetMainFrame() : browser_handler_->GetBrowser()->GetFrameByName(frame_name);

        if (!js_bridge_->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, nim_comp::CallJsFunctionCallback callback, const CefString& frame_id)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        CefRefPtr<CefFrame> frame = browser_handler_->GetBrowser()->GetFrameByIdentifier(frame_id);

        if (!js_bridge_->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback))
        {
            return false;
        }

        return true;
    }

    return false;
}

void CefControlBase::RepairBrowser()
{
    ReCreateBrowser();
}

void CefControlBase::DettachDevTools()
{
    if (!devtool_attached_)
        return;
    auto browser = browser_handler_->GetBrowser();
    if (browser != nullptr)
    {
        browser->GetHost()->CloseDevTools();
        devtool_attached_ = false;
        if (cb_devtool_visible_change_ != nullptr)
            cb_devtool_visible_change_(devtool_attached_);
    }
}

}
