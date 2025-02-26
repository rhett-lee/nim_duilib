#include "CefControlBase.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"

#include "duilib/CEFControl/internal/app/CefJSBridge.h"
#include "duilib/CEFControl/internal/handler/CefBrowserHandler.h"

namespace ui {

CefControlBase::CefControlBase(ui::Window* pWindow):
    ui::Control(pWindow)
{
    devtool_attached_ = false;
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

bool CefControlBase::RegisterCppFunc(const DString& function_name, ui::CppFunction function, bool global_function/* = false*/)
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

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const DString& frame_name /*= _T("")*/)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        CefRefPtr<CefFrame> frame;
        if (frame_name.empty()) {
            frame = browser_handler_->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = browser_handler_->GetBrowser()->GetFrame(frame_name);
#else
            //CEF 高版本
            frame = browser_handler_->GetBrowser()->GetFrameByName(frame_name);
#endif
        }

        if (!js_bridge_->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const CefString& frame_id)
{
    if (browser_handler_.get() && browser_handler_->GetBrowser().get() && js_bridge_.get())
    {
        CefRefPtr<CefFrame> frame;
        if (frame_id.empty()) {
            frame = browser_handler_->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = browser_handler_->GetBrowser()->GetFrame(StringUtil::StringToInt64(frame_id.c_str()));
#else
            //CEF 高版本
            frame = browser_handler_->GetBrowser()->GetFrameByIdentifier(frame_id);
#endif
        }
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

void CefControlBase::OnPaint(CefRefPtr<CefBrowser> /*browser*/, CefRenderHandler::PaintElementType /*type*/, const CefRenderHandler::RectList& /*dirtyRects*/, const std::string* /*buffer*/, int /*width*/, int /*height*/)
{
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);
    return;
}

void CefControlBase::ClientToControl(UiPoint&/*pt*/)
{
    return;
}

void CefControlBase::OnPopupShow(CefRefPtr<CefBrowser> /*browser*/, bool /*show*/)
{
    return;
}

void CefControlBase::OnPopupSize(CefRefPtr<CefBrowser> /*rowser*/, const CefRect& /*rect*/)
{
    return;
}

void CefControlBase::UpdateWindowPos()
{
    this->SetPos(this->GetPos());
}

void CefControlBase::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    if (cb_before_menu_)
        cb_before_menu_(params, model);
}

bool CefControlBase::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)
{
    if (cb_menu_command_)
        return cb_menu_command_(params, command_id, event_flags);

    return false;
}

void CefControlBase::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    if (cb_url_change_)
        cb_url_change_(url);

    if (frame->IsMain())
    {
        auto old_url = url_;
        url_ = frame->GetURL();
        if (cb_main_url_change_ != nullptr && GetMainURL(old_url).compare(GetMainURL(url_)) != 0)
        {
            cb_main_url_change_(old_url, url_);
        }
    }
}

void CefControlBase::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    if (cb_title_change_)
        cb_title_change_(title);
}

void CefControlBase::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    if (cb_loadstate_change_)
        cb_loadstate_change_(isLoading, canGoBack, canGoForward);
}

void CefControlBase::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (cb_load_start_)
        cb_load_start_();
}

void CefControlBase::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (cb_load_end_)
        cb_load_end_(httpStatusCode);
}

void CefControlBase::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    if (cb_load_error_)
        cb_load_error_(errorCode, errorText, failedUrl);
}

bool CefControlBase::OnBeforePopup(CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    const CefString& target_url,
    const CefString& /*target_frame_name*/,
    CefLifeSpanHandler::WindowOpenDisposition /*target_disposition*/,
    bool /*user_gesture*/,
    const CefPopupFeatures& /*popupFeatures*/,
    CefWindowInfo& /*windowInfo*/,
    CefRefPtr<CefClient>& /*client*/,
    CefBrowserSettings& /*settings*/,
    bool* /*no_javascript_access*/)
{
    if (cb_link_click_ && !target_url.empty())
        return cb_link_click_(target_url);

    return false;
}

bool CefControlBase::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    if (cb_after_created_)
        cb_after_created_(browser);

    return false;
}

void CefControlBase::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    if (cb_before_close_)
        cb_before_close_(browser);
}

bool CefControlBase::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)
{
    bool result = false;

    if (cb_before_browser_)
        result = cb_before_browser_(browser, frame, request, is_redirect);

    return result;
}

void CefControlBase::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
    if (cb_protocol_execution_)
        cb_protocol_execution_(browser, url, allow_os_execution);
}

cef_return_value_t CefControlBase::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
    if (cb_before_resource_load_)
        return cb_before_resource_load_(request, false);

    return RV_CONTINUE;
}

void CefControlBase::OnRenderProcessTerminated(CefRefPtr<CefBrowser> /*browser*/, CefRequestHandler::TerminationStatus /*status*/)
{
    return;
}

void CefControlBase::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (cb_before_download_)
        cb_before_download_(browser, download_item, suggested_name, callback);
}

void CefControlBase::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
    if (cb_download_updated_)
        cb_download_updated_(browser, download_item, callback);
}

bool CefControlBase::OnFileDialog(CefRefPtr<CefBrowser> browser, CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_path, const std::vector<CefString>& accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback)
{
    if (cb_file_dialog_)
        return cb_file_dialog_(mode, title, default_file_path, accept_filters, selected_accept_filter, callback);
    else
        return false;
}

bool CefControlBase::OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (js_bridge_.get())
    {
        js_callback_thread_id_ = ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier();
        return js_bridge_->ExecuteCppFunc(function_name, params, js_callback_id, browser, frame);
    }

    return false;
}

bool CefControlBase::OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string)
{
    if (js_bridge_.get())
    {
        if (js_callback_thread_id_ != -1)
        {
            ui::GlobalManager::Instance().Thread().PostTask(js_callback_thread_id_, [this, cpp_callback_id, json_string]
            {
                js_bridge_->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
            });
        }
        else
        {
            return js_bridge_->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
        }

    }

    return false;
}

} //namespace ui
