#include "CefForm.h"

CefForm::CefForm()
{
}

CefForm::~CefForm()
{
}

DString CefForm::GetSkinFolder()
{
    return _T("cef");
}

DString CefForm::GetSkinFile()
{
    return _T("cef.xml");
}

void CefForm::OnInitWindow()
{
    // 监听鼠标单击事件
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&CefForm::OnClicked, this, std::placeholders::_1));

    // 从 XML 中查找指定控件
    m_pCefControl = dynamic_cast<ui::CefControl*>(FindControl(_T("cef_control")));
    m_pCefControlDev = dynamic_cast<ui::CefControl*>(FindControl(_T("cef_control_dev")));
    m_pDevToolBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_dev_tool")));
    m_pEditUrl = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_url")));
    ASSERT(m_pCefControl != nullptr);
    ASSERT(m_pCefControlDev != nullptr);
    ASSERT(m_pDevToolBtn != nullptr);
    ASSERT(m_pEditUrl != nullptr);

    // 设置输入框样式
    m_pEditUrl->SetSelAllOnFocus(true);
    m_pEditUrl->AttachReturn(UiBind(&CefForm::OnNavigate, this, std::placeholders::_1));

    m_pCefControl->SetCefEventHandler(this);

    // 打开开发者工具
    //m_pCefControl->AttachDevTools(m_pCefControlDev);

    // 加载皮肤目录下的 html 文件
    ui::FilePath cefHtml = ui::FilePathUtil::GetCurrentModuleDirectory();
    cefHtml.NormalizeDirectoryPath();
#if defined (DUILIB_BUILD_FOR_WIN)
    cefHtml += _T("resources\\themes\\default\\cef\\cef.html");
#else
    cefHtml += _T("resources/themes/default/cef/cef.html");
#endif
    m_pCefControl->LoadURL(cefHtml.ToString());

    if (!ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        m_pCefControlDev->SetFadeVisible(false);
    }
}

void CefForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    ui::CefManager::GetInstance()->PostQuitMessage(0L);
}

bool CefForm::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.GetSender()->GetName();

    if (name == _T("btn_dev_tool")) {
        if (m_pCefControl->IsAttachedDevTools()) {
            m_pCefControl->DettachDevTools();
        }
        else {
            m_pCefControl->AttachDevTools(m_pCefControlDev);
        }

        if (ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
            m_pCefControlDev->SetFadeVisible(m_pCefControl->IsAttachedDevTools());
        }
    }
    else if (name == _T("btn_back")) {
        m_pCefControl->GoBack();
    }
    else if (name == _T("btn_forward")) {
        m_pCefControl->GoForward();
    }
    else if (name == _T("btn_navigate")) {
        ui::EventArgs emptyMsg;
        OnNavigate(emptyMsg);
    }
    else if (name == _T("btn_refresh")) {
        m_pCefControl->Refresh();
    }
    return true;
}

bool CefForm::OnNavigate(const ui::EventArgs& /*msg*/)
{
    if (!m_pEditUrl->GetText().empty()) {
        m_pCefControl->LoadURL(m_pEditUrl->GetText());
        m_pCefControl->SetFocus();
    }
    return true;
}

void CefForm::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
}

void CefForm::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
}

void CefForm::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefContextMenuParams> params,
                                  CefRefPtr<CefMenuModel> model)
{
}

bool CefForm::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefContextMenuParams> params,
                                   int command_id,
                                   cef_event_flags_t event_flags)
{
    return false;
}
    
void CefForm::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
}

void CefForm::OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title)
{
}
    
void CefForm::OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url)
{
}
    
void CefForm::OnMainUrlChange(const DString& oldUrl, const DString& newUrl)
{
}
    
void CefForm::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
}
        
void CefForm::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool bFullscreen)
{
}
    
void CefForm::OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value)
{
}
    
void CefForm::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
}
    
void CefForm::OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access)
{
}

bool CefForm::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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

    //拦截弹窗，并导航到弹出网址
    if ((browser != nullptr) && (browser->GetMainFrame() != nullptr) && !target_url.empty()) {
        browser->GetMainFrame()->LoadURL(target_url);
    }
    return true;
}

void CefForm::OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id)
{
}

bool CefForm::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefRequest> request,
                             bool user_gesture,
                             bool is_redirect)
{
    return false;
}

cef_return_value_t CefForm::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefRequest> request,
                                                 CefRefPtr<CefCallback> callback)
{
    return RV_CONTINUE;
}

void CefForm::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefRequest> request,
                                 CefRefPtr<CefResponse> response,
                                 CefString& new_url)
{
}
    
bool CefForm::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefRequest> request,
                                 CefRefPtr<CefResponse> response)
{
    return false;
}

void CefForm::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     CefRefPtr<CefResponse> response,
                                     cef_urlrequest_status_t status,
                                     int64_t received_content_length)
{
}

void CefForm::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
}

void CefForm::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
}
    
void CefForm::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type)
{
}
    
void CefForm::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    ui::Control* pControl = FindControl(_T("btn_back"));
    if ((pControl != nullptr) && (m_pCefControl != nullptr)) {
        pControl->SetEnabled(m_pCefControl->CanGoBack());
    }

    pControl = FindControl(_T("btn_forward"));
    if ((pControl != nullptr) && (m_pCefControl != nullptr)) {
        pControl->SetEnabled(m_pCefControl->CanGoForward());
    }

    // 注册一个方法提供前端调用
    if (m_pCefControl != nullptr) {
        m_pCefControl->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([this](const std::string& params, ui::ReportResultFunction callback) {
            DString value = ui::StringConvert::UTF8ToT(params);
            ui::SystemUtil::ShowMessageBox(this, value.c_str(), _T("C++ 接收到 JavaScript 发来的消息"));
            callback(false, R"({ "message": "Success." })");
            }));
    }
}
    
void CefForm::OnLoadError(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          cef_errorcode_t errorCode,
                          const DString& errorText,
                          const DString& failedUrl)
{
}

void CefForm::OnDevToolAttachedStateChange(bool bVisible)
{
}

bool CefForm::OnCanDownload(CefRefPtr<CefBrowser> browser,
                            const CefString& url,
                            const CefString& request_method)
{
    return true;
}

bool CefForm::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefDownloadItem> download_item,
                               const CefString& suggested_name,
                               CefRefPtr<CefBeforeDownloadCallback> callback)
{
    return true;
}

void CefForm::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDownloadItem> download_item,
                                CefRefPtr<CefDownloadItemCallback> callback)
{
}

bool CefForm::OnFileDialog(CefRefPtr<CefBrowser> browser,
                           cef_file_dialog_mode_t mode,
                           const CefString& title,
                           const CefString& default_file_path,
                           const std::vector<CefString>& accept_filters,
                           const std::vector<CefString>& accept_extensions,
                           const std::vector<CefString>& accept_descriptions,
                           CefRefPtr<CefFileDialogCallback> callback)
{
    return false;
}

void CefForm::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
}
