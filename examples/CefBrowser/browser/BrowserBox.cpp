#include "BrowserBox.h"
#include "browser/BrowserForm.h"
#include "browser/BrowserManager.h"

using namespace ui;

BrowserBox::BrowserBox(ui::Window* pWindow, std::string id):
    ui::VBox(pWindow)
{
    m_pBrowserForm = nullptr;
    m_pCefControl = nullptr;
    m_browserId = id;
}

BrowserForm* BrowserBox::GetBrowserForm() const
{
    ASSERT(nullptr != m_pBrowserForm);
    ASSERT(m_pBrowserForm->IsWindow());
    return m_pBrowserForm;
}

ui::CefControl* BrowserBox::GetCefControl()
{
    return m_pCefControl;
}

const DString& BrowserBox::GetTitle() const
{
    return m_title;
}

void BrowserBox::InitBrowserBox(const DString& url)
{
    m_pCefControl = static_cast<ui::CefControl*>(FindSubControl(_T("cef_control")));
    ASSERT(m_pCefControl != nullptr);
    if (m_pCefControl == nullptr) {
        return;
    }

    m_pCefControl->AttachAfterCreated(ui::UiBind(&BrowserBox::OnAfterCreated, this, std::placeholders::_1));
    m_pCefControl->AttachBeforeClose(ui::UiBind(&BrowserBox::OnBeforeClose, this, std::placeholders::_1));

    m_pCefControl->AttachBeforeContextMenu(ui::UiBind(&BrowserBox::OnBeforeContextMenu, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachContextMenuCommand(ui::UiBind(&BrowserBox::OnContextMenuCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_pCefControl->AttachContextMenuDismissed(ui::UiBind(&BrowserBox::OnContextMenuDismissed, this, std::placeholders::_1, std::placeholders::_2));

    m_pCefControl->AttachTitleChange(ui::UiBind(&BrowserBox::OnTitleChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachUrlChange(ui::UiBind(&BrowserBox::OnUrlChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachMainUrlChange(ui::UiBind(&BrowserBox::OnMainUrlChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachFaviconURLChange(ui::UiBind(&BrowserBox::OnFaviconURLChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachFullscreenModeChange(ui::UiBind(&BrowserBox::OnFullscreenModeChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachStatusMessage(ui::UiBind(&BrowserBox::OnStatusMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachLoadingProgressChange(ui::UiBind(&BrowserBox::OnLoadingProgressChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachMediaAccessChange(ui::UiBind(&BrowserBox::OnMediaAccessChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_pCefControl->AttachBeforePopup(ui::UiBind(&BrowserBox::OnBeforePopup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9, std::placeholders::_10));
    m_pCefControl->AttachBeforePopupAborted(ui::UiBind(&BrowserBox::OnBeforePopupAborted, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachBeforeBrowse(ui::UiBind(&BrowserBox::OnBeforeBrowse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pCefControl->AttachBeforeResourceLoad(ui::UiBind(&BrowserBox::OnBeforeResourceLoad, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachResourceRedirect(ui::UiBind(&BrowserBox::OnResourceRedirect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_pCefControl->AttachResourceResponse(ui::UiBind(&BrowserBox::OnResourceResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachResourceLoadComplete(ui::UiBind(&BrowserBox::OnResourceLoadComplete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    m_pCefControl->AttachProtocolExecution(ui::UiBind(&BrowserBox::OnProtocolExecution, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    m_pCefControl->AttachLoadingStateChange(ui::UiBind(&BrowserBox::OnLoadingStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachLoadStart(ui::UiBind(&BrowserBox::OnLoadStart, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachLoadEnd(ui::UiBind(&BrowserBox::OnLoadEnd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachLoadError(ui::UiBind(&BrowserBox::OnLoadError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pCefControl->AttachDevToolAttachedStateChange(ui::UiBind(&BrowserBox::OnDevToolAttachedStateChange, this, std::placeholders::_1));

    m_pCefControl->AttachCanDownload(ui::UiBind(&BrowserBox::OnCanDownload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachBeforeDownload(ui::UiBind(&BrowserBox::OnBeforeDownload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachDownloadUpdated(ui::UiBind(&BrowserBox::OnDownloadUpdated, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_pCefControl->AttachFileDialog(ui::UiBind(&BrowserBox::OnFileDialog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8));

    m_pCefControl->AttachDocumentAvailableInMainFrame(ui::UiBind(&BrowserBox::OnDocumentAvailableInMainFrame, this, std::placeholders::_1));
    m_pCefControl->AttachDownloadFavIconFinished(ui::UiBind(&BrowserBox::OnDownloadFavIconFinished, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    // 加载默认网页
    DString html_path = url;
    if (html_path.empty()) {
        ui::FilePath localPath = ui::FilePathUtil::GetCurrentModuleDirectory();
        localPath.NormalizeDirectoryPath();
        localPath += _T("resources/themes/default/cef/cef.html");
        html_path = localPath.ToString();
        html_path = _T("file:///") + html_path;
    }
    m_pCefControl->LoadURL(html_path);
}

void BrowserBox::UninitBrowserBox()
{
    BrowserManager::GetInstance()->RemoveBorwserBox(m_browserId, this);
}

void BrowserBox::SetWindow(Window* pWindow)
{
    m_pBrowserForm = dynamic_cast<BrowserForm*>(pWindow);
    ASSERT(nullptr != m_pBrowserForm);

    BaseClass::SetWindow(pWindow);
}

bool BrowserBox::OnSetFocus(const ui::EventArgs& msg)
{
    // Box获取焦点时把焦点转移给Cef控件
    if (m_pCefControl) {
        m_pCefControl->SetFocus();
    }

    //不再调用基类的方法，避免覆盖输入法管理的逻辑（基类会关闭输入法）
    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHot);
        Invalidate();
    }
    return true;
}

void BrowserBox::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

// 定义自定义菜单项ID（避免与默认ID冲突）
static const int MENU_ID_OPEN_LINK_IN_NEW_TAB       = MENU_ID_USER_FIRST + 1;
static const int MENU_ID_OPEN_LINK_IN_NEW_WINDOW    = MENU_ID_USER_FIRST + 2;
static const int MENU_ID_COPY_LINK                  = MENU_ID_USER_FIRST + 3;

void BrowserBox::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if ((params != nullptr) && (model != nullptr)  && !params->GetLinkUrl().empty()) {
        // 在菜单顶部添加自定义项
        if (model->GetCount() > 0) {
            model->InsertSeparatorAt(0);
        }
        model->InsertItemAt(0, MENU_ID_COPY_LINK, "复制链接");
        model->InsertItemAt(0, MENU_ID_OPEN_LINK_IN_NEW_WINDOW, "在新窗口中打开链接");
        model->InsertItemAt(0, MENU_ID_OPEN_LINK_IN_NEW_TAB, "在新标签页中打开链接");
    }
}

bool BrowserBox::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      cef_event_flags_t event_flags)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (params != nullptr) {
        if (command_id == MENU_ID_OPEN_LINK_IN_NEW_TAB) {
            CefString url = params->GetLinkUrl();
            if (!url.empty() && (m_pBrowserForm != nullptr)) {
                //在新标签页中打开链接
                m_pBrowserForm->OpenLinkUrl(url, false);
            }
            return true;
        }
        else if (command_id == MENU_ID_OPEN_LINK_IN_NEW_WINDOW) {
            CefString url = params->GetLinkUrl();
            if (!url.empty() && (m_pBrowserForm != nullptr)) {
                //在新窗口中打开链接
                m_pBrowserForm->OpenLinkUrl(url, true);
            }
            return true;
        }
        else if (command_id == MENU_ID_COPY_LINK) {
            CefString url = params->GetLinkUrl();
            if (!url.empty()) {
                //复制链接
                DStringW urlW = url;
                ui::Clipboard::SetClipboardText(urlW);
            }
            return true;
        }
    }
    return false;
}

void BrowserBox::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    ASSERT(CefCurrentlyOn(TID_UI));
}

void BrowserBox::OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title)
{
    ui::GlobalManager::Instance().AssertUIThread();
    m_title = title;
    m_pBrowserForm->SetTabItemName(ui::StringConvert::UTF8ToT(m_browserId), title);
}

void BrowserBox::OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url)
{
    ui::GlobalManager::Instance().AssertUIThread();
    m_url = url;
    m_pBrowserForm->SetURL(m_browserId, url);
}

void BrowserBox::OnMainUrlChange(const DString& oldUrl, const DString& newUrl)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool bFullscreen)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

bool BrowserBox::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               int popup_id,
                               const CefString& target_url,
                               const ui::BeforePopupEventParam& param,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings,
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool* no_javascript_access)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    //拦截弹窗，并导航到弹出网址
    if ((browser != nullptr) && (browser->GetMainFrame() != nullptr) && !target_url.empty()) {
        browser->GetMainFrame()->LoadURL(target_url);
    }
    return true;
}

void BrowserBox::OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id)
{
    ASSERT(CefCurrentlyOn(TID_UI));
}

bool BrowserBox::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    return false;
}

cef_return_value_t BrowserBox::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefFrame> frame,
                                                    CefRefPtr<CefRequest> request,
                                                    CefRefPtr<CefCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    // 返回RV_CANCEL截断导航
    return RV_CONTINUE;
}

void BrowserBox::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url)
{
    ASSERT(CefCurrentlyOn(TID_IO));
}

bool BrowserBox::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    return false;
}

void BrowserBox::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length)
{
    ASSERT(CefCurrentlyOn(TID_IO));
}

void BrowserBox::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution)
{
    ASSERT(CefCurrentlyOn(TID_IO));
}

void BrowserBox::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    ui::GlobalManager::Instance().AssertUIThread();
    if (m_pBrowserForm != nullptr) {
        m_pBrowserForm->OnLoadingStateChange(this);
    }
}

void BrowserBox::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    ui::GlobalManager::Instance().AssertUIThread();
    // 注册一个方法提供前端调用
    m_pCefControl->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([this](const std::string& params, ui::ReportResultFunction callback) {
        DString value = ui::StringConvert::UTF8ToT(params);
        ui::SystemUtil::ShowMessageBox(GetWindow(), value.c_str(), _T("C++ 接收到 JavaScript 发来的消息"));
        callback(false, R"({ "message": "Success." })");
    }));
}

void BrowserBox::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_errorcode_t errorCode, const DString& errorText, const DString& failedUrl)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnDevToolAttachedStateChange(bool bVisible)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

bool BrowserBox::OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    return true;
}

bool BrowserBox::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    return false;
}

void BrowserBox::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
}

bool BrowserBox::OnFileDialog(CefRefPtr<CefBrowser> browser,
                              cef_file_dialog_mode_t mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    return false;
}

void BrowserBox::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
    ui::GlobalManager::Instance().AssertUIThread();
}

void BrowserBox::OnDownloadFavIconFinished(CefRefPtr<CefBrowser> browser,
                                           const CefString& image_url,
                                           int http_status_code,
                                           CefRefPtr<CefImage> image)
{
    ui::GlobalManager::Instance().AssertUIThread();
    if ((m_pBrowserForm != nullptr) && (image != nullptr)) {
        m_pBrowserForm->NotifyFavicon(this, image);
    }
}

