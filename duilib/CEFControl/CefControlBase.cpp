#include "CefControlBase.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"

#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

namespace ui {

CefControlBase::CefControlBase(ui::Window* pWindow):
    ui::Control(pWindow)
{
    m_bAttachedDevTools = false;
    //这个标记必须为false，否则绘制有问题
    SetUseCache(false);
}

CefControlBase::~CefControlBase(void)
{

}
void CefControlBase::LoadURL(const CefString& url)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        CefRefPtr<CefFrame> frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        if (!frame) {
            return;
        }
        frame->LoadURL(url);
    }
    else {
        if (m_pBrowserHandler.get()) {
            ui::StdClosure cb = ToWeakCallback([this, url]() {
                LoadURL(url);
            });
            m_pBrowserHandler->AddAfterCreateTask(cb);
        }
    }
}

void CefControlBase::GoBack()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GoBack();
    }
}

void CefControlBase::GoForward()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GoForward();
    }
}

bool CefControlBase::CanGoBack()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->CanGoBack();
    }
    return false;
}

bool CefControlBase::CanGoForward()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->CanGoForward();
    }
    return false;
}

void CefControlBase::Refresh()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->Reload();
    }
}

void CefControlBase::StopLoad()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->StopLoad();
    }
}

bool CefControlBase::IsLoading()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->IsLoading();
    }
    return false;
}

void CefControlBase::StartDownload(const CefString& url)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        m_pBrowserHandler->GetBrowser()->GetHost()->StartDownload(url);
    }
}

void CefControlBase::SetZoomLevel(float zoom_level)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        m_pBrowserHandler->GetBrowser()->GetHost()->SetZoomLevel(zoom_level);
    }
}

CefWindowHandle CefControlBase::GetCefHandle() const
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowserHost().get()) {
        return m_pBrowserHandler->GetBrowserHost()->GetWindowHandle();
    }
    return 0;
}

CefString CefControlBase::GetURL()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GetMainFrame()->GetURL();
    }
    return CefString();
}

std::string CefControlBase::GetUTF8URL()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
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
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        return m_jsBridge->RegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), function, global_function ? nullptr : m_pBrowserHandler->GetBrowser());
    }
    return false;
}

void CefControlBase::UnRegisterCppFunc(const DString& function_name)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        m_jsBridge->UnRegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), m_pBrowserHandler->GetBrowser());
    }
}

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const DString& frame_name /*= _T("")*/)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        CefRefPtr<CefFrame> frame;
        if (frame_name.empty()) {
            frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrame(frame_name);
#else
            //CEF 高版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrameByName(frame_name);
#endif
        }

        if (!m_jsBridge->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback)) {
            return false;
        }
        return true;
    }
    return false;
}

bool CefControlBase::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const CefString& frame_id)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        CefRefPtr<CefFrame> frame;
        if (frame_id.empty()) {
            frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrame(StringUtil::StringToInt64(frame_id.c_str()));
#else
            //CEF 高版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrameByIdentifier(frame_id);
#endif
        }
        if (!m_jsBridge->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback)) {
            return false;
        }
        return true;
    }
    return false;
}


bool CefControlBase::OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (m_jsBridge.get()) {
        m_jsCallbackThreadId = ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier();
        return m_jsBridge->ExecuteCppFunc(function_name, params, js_callback_id, browser, frame);
    }
    return false;
}

bool CefControlBase::OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string)
{
    if (m_jsBridge.get()) {
        if (m_jsCallbackThreadId != -1) {
            ui::GlobalManager::Instance().Thread().PostTask(m_jsCallbackThreadId, [this, cpp_callback_id, json_string]
                {
                    m_jsBridge->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
                });
        }
        else {
            return m_jsBridge->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
        }

    }
    return false;
}

void CefControlBase::RepairBrowser()
{
    ReCreateBrowser();
}

void CefControlBase::DettachDevTools()
{
    if (!m_bAttachedDevTools) {
        return;
    }
    auto browser = m_pBrowserHandler->GetBrowser();
    if ((browser != nullptr) && (browser->GetHost() != nullptr)) {
        browser->GetHost()->CloseDevTools();
        m_bAttachedDevTools = false;
        if (m_pfnDevToolVisibleChange != nullptr) {
            m_pfnDevToolVisibleChange(m_bAttachedDevTools);
        }
    }
    else {
        m_bAttachedDevTools = false;
    }
}

bool CefControlBase::IsAttachedDevTools() const
{
    return m_bAttachedDevTools;
}

void CefControlBase::SetAttachedDevTools(bool bAttachedDevTools)
{
    m_bAttachedDevTools = bAttachedDevTools;
}

void CefControlBase::OnDevToolsVisibleChanged()
{
    if (m_pfnDevToolVisibleChange != nullptr) {
        m_pfnDevToolVisibleChange(IsAttachedDevTools());
    }
}

void CefControlBase::OnPaint(CefRefPtr<CefBrowser> /*browser*/, CefRenderHandler::PaintElementType /*type*/, const CefRenderHandler::RectList& /*dirtyRects*/, const void* /*buffer*/, int /*width*/, int /*height*/)
{
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);
}

void CefControlBase::ClientToControl(UiPoint&/*pt*/)
{
}

void CefControlBase::OnPopupShow(CefRefPtr<CefBrowser> /*browser*/, bool /*show*/)
{
}

void CefControlBase::OnPopupSize(CefRefPtr<CefBrowser> /*rowser*/, const CefRect& /*rect*/)
{
}

void CefControlBase::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> /*browser*/, const CefRange& /*selected_range*/, const std::vector<CefRect>& /*character_bounds*/)
{
}

void CefControlBase::UpdateWindowPos()
{
    this->SetPos(this->GetPos());
}

void CefControlBase::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    if (m_pfnBeforeContextMenu) {
        m_pfnBeforeContextMenu(browser, frame, params, model);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforeContextMenu(browser, frame, params, model);
    }
}

bool CefControlBase::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)
{
    if (m_pfnContextMenuCommand) {
        return m_pfnContextMenuCommand(browser, frame, params, command_id, event_flags);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnContextMenuCommand(browser, frame, params, command_id, event_flags);
    }
    return false;
}

void CefControlBase::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (m_pfnContextMenuDismissed) {
        m_pfnContextMenuDismissed(browser, frame);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnContextMenuDismissed(browser, frame);
    }
}

void CefControlBase::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    if (m_pfnUrlChange) {
        m_pfnUrlChange(browser, frame, url);
    }

    if ((frame != nullptr) && frame->IsMain()) {
        auto old_url = m_url;
        m_url = frame->GetURL();
        if (m_pfnMainUrlChange != nullptr && GetMainURL(old_url).compare(GetMainURL(m_url)) != 0) {
            m_pfnMainUrlChange(old_url, m_url);
        }
    }
    if (!m_pfnUrlChange && m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnUrlChange(browser, frame, url);
    }
}

void CefControlBase::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    if (m_pfnTitleChange) {
        m_pfnTitleChange(browser, title);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnTitleChange(browser, title);
    }
}

void CefControlBase::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
    if (m_pfnFaviconURLChange) {
        m_pfnFaviconURLChange(browser, icon_urls);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnFaviconURLChange(browser, icon_urls);
    }
}

void CefControlBase::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
    if (m_pfnFullscreenModeChange) {
        m_pfnFullscreenModeChange(browser, fullscreen);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnFullscreenModeChange(browser, fullscreen);
    }
}

void CefControlBase::OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value)
{
    if (m_pfnStatusMessage) {
        m_pfnStatusMessage(browser, value);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnStatusMessage(browser, value);
    }
}

void CefControlBase::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
    if (m_pfnLoadingProgressChange) {
        m_pfnLoadingProgressChange(browser, progress);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadingProgressChange(browser, progress);
    }
}

void CefControlBase::OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access)
{
    if (m_pfnMediaAccessChange) {
        m_pfnMediaAccessChange(browser, has_video_access, has_audio_access);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnMediaAccessChange(browser, has_video_access, has_audio_access);
    }
}

void CefControlBase::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    if (m_pfnLoadingStateChange) {
        m_pfnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
}

void CefControlBase::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type)
{
    if (m_pfnLoadStart) {
        m_pfnLoadStart(browser, frame, transition_type);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadStart(browser, frame, transition_type);
    }
}

void CefControlBase::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (m_pfnLoadEnd) {
        m_pfnLoadEnd(browser, frame, httpStatusCode);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadEnd(browser, frame, httpStatusCode);
    }
}

void CefControlBase::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    if (m_pfnLoadError) {
        m_pfnLoadError(browser, frame, errorCode, errorText, failedUrl);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadError(browser, frame, errorCode, errorText, failedUrl);
    }
}

bool CefControlBase::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   int popup_id,
                                   const CefString& target_url,
                                   const CefString& target_frame_name,
                                   CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                   bool user_gesture,
                                   const CefPopupFeatures& popupFeatures,
                                   CefWindowInfo& windowInfo,
                                   CefRefPtr<CefClient>& client,
                                   CefBrowserSettings& settings,
                                   CefRefPtr<CefDictionaryValue>& extra_info,
                                   bool* no_javascript_access)
{
    if (m_pfnBeforePopup) {
        return m_pfnBeforePopup(browser, frame, popup_id, target_url, target_frame_name,
                                target_disposition, user_gesture, popupFeatures, windowInfo,
                                client, settings, extra_info, no_javascript_access);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforePopup(browser, frame, popup_id, target_url, target_frame_name,
                                                        target_disposition, user_gesture, popupFeatures, windowInfo,
                                                        client, settings, extra_info, no_javascript_access);
    }
    return true;
}

void CefControlBase::OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id)
{
    if (m_pfnBeforePopupAborted) {
        return m_pfnBeforePopupAborted(browser, popup_id);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforePopupAborted(browser, popup_id);
    }
}

void CefControlBase::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    if (m_pfnAfterCreated) {
        m_pfnAfterCreated(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnAfterCreated(browser);
    }
}

void CefControlBase::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    if (m_pfnBeforeClose) {
        m_pfnBeforeClose(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforeClose(browser);
    }
}

bool CefControlBase::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
{
    if (m_pfnBeforeBrowse) {
        return m_pfnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
    }
    return false;
}

cef_return_value_t CefControlBase::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
    if (m_pfnBeforeResourceLoad) {
        return m_pfnBeforeResourceLoad(browser, frame, request, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeResourceLoad(browser, frame, request, callback);
    }
    return RV_CONTINUE;
}

void CefControlBase::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        CefString& new_url)
{
    if (m_pfnResourceRedirect) {
        m_pfnResourceRedirect(browser, frame, request, response, new_url);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnResourceRedirect(browser, frame, request, response, new_url);
    }
}

bool CefControlBase::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response)
{
    if (m_pfnResourceResponse) {
        return m_pfnResourceResponse(browser, frame, request, response);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnResourceResponse(browser, frame, request, response);
    }
    return false;
}

void CefControlBase::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefResponse> response,
                                            cef_urlrequest_status_t status,
                                            int64_t received_content_length)
{
    if (m_pfnResourceLoadComplete) {
        m_pfnResourceLoadComplete(browser, frame, request, response, status, received_content_length);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnResourceLoadComplete(browser, frame, request, response, status, received_content_length);
    }
}

void CefControlBase::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
    if (m_pfnProtocolExecution) {
        m_pfnProtocolExecution(browser, url, allow_os_execution);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnProtocolExecution(browser, url, allow_os_execution);
    }
}

void CefControlBase::OnRenderProcessTerminated(CefRefPtr<CefBrowser> /*browser*/,
                                               CefRequestHandler::TerminationStatus /*status*/,
                                               int /*error_code*/,
                                               CefString /*error_string*/)
{
}

void CefControlBase::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
    if (m_pfnDocumentAvailableInMainFrame) {
        m_pfnDocumentAvailableInMainFrame(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnDocumentAvailableInMainFrame(browser);
    }
}

bool CefControlBase::OnCanDownload(CefRefPtr<CefBrowser> browser,
                                   const CefString& url,
                                   const CefString& request_method)
{
    if (m_pfnCanDownload) {
        return m_pfnCanDownload(browser, url, request_method);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnCanDownload(browser, url, request_method);
    }
    return true;
}

bool CefControlBase::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (m_pfnBeforeDownload) {
        return m_pfnBeforeDownload(browser, download_item, suggested_name, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeDownload(browser, download_item, suggested_name, callback);
    }
    return false;
}

void CefControlBase::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
    if (m_pfnDownloadUpdated) {
        m_pfnDownloadUpdated(browser, download_item, callback);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnDownloadUpdated(browser, download_item, callback);
    }
}

bool CefControlBase::OnFileDialog(CefRefPtr<CefBrowser> browser,
                                  CefBrowserHost::FileDialogMode mode,
                                  const CefString& title,
                                  const CefString& default_file_path,
                                  const std::vector<CefString>& accept_filters,
                                  const std::vector<CefString>& accept_extensions,
                                  const std::vector<CefString>& accept_descriptions,
                                  CefRefPtr<CefFileDialogCallback> callback)
{
    if (m_pfnFileDialog) {
        return m_pfnFileDialog(browser, mode, title, default_file_path, accept_filters, accept_extensions, accept_descriptions, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnFileDialog(browser, mode, title, default_file_path, accept_filters, accept_extensions, accept_descriptions, callback);
    }
    return false;        
}

void CefControlBase::SetCefEventHandler(CefControlEvent* pCefControlEventHandler)
{
    m_pCefControlEventHandler = pCefControlEventHandler;
}

CefControlEvent* CefControlBase::GetCefEventHandler() const
{
    return m_pCefControlEventHandler;
}

} //namespace ui
