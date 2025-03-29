#include "BrowserBox.h"
#include "browser/MultiBrowserForm.h"
#include "browser/MultiBrowserManager.h"
#include "taskbar/TaskbarManager.h"

using namespace ui;
using namespace std;

BrowserBox::BrowserBox(ui::Window* pWindow, std::string id):
    ui::VBox(pWindow)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pTaskBarItem = nullptr;
#endif
    m_pBrowserForm = nullptr;
    m_pCefControl = nullptr;
    m_browserId = id;
}

MultiBrowserForm* BrowserBox::GetBrowserForm() const
{
    ASSERT(nullptr != m_pBrowserForm);
    ASSERT(m_pBrowserForm->IsWindow());
    return m_pBrowserForm;
}

ui::CefControl* BrowserBox::GetCefControl()
{
    return m_pCefControl;
}

DString& BrowserBox::GetTitle()
{
    return m_title;
}

void BrowserBox::InitBrowserBox(const DString &url)
{
    m_pCefControl = static_cast<ui::CefControl*>(FindSubControl(_T("cef_control")));
    ASSERT(m_pCefControl != nullptr);
    if (m_pCefControl == nullptr) {
        return;
    }

    m_pCefControl->AttachAfterCreated(UiBind(&BrowserBox::OnAfterCreated, this, std::placeholders::_1));
    m_pCefControl->AttachBeforeClose(UiBind(&BrowserBox::OnBeforeClose, this, std::placeholders::_1));

    m_pCefControl->AttachBeforeContextMenu(UiBind(&BrowserBox::OnBeforeContextMenu, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachContextMenuCommand(UiBind(&BrowserBox::OnContextMenuCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_pCefControl->AttachContextMenuDismissed(UiBind(&BrowserBox::OnContextMenuDismissed, this, std::placeholders::_1, std::placeholders::_2));

    m_pCefControl->AttachTitleChange(UiBind(&BrowserBox::OnTitleChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachUrlChange(UiBind(&BrowserBox::OnUrlChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachMainUrlChange(UiBind(&BrowserBox::OnMainUrlChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachFaviconURLChange(UiBind(&BrowserBox::OnFaviconURLChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachFullscreenModeChange(UiBind(&BrowserBox::OnFullscreenModeChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachStatusMessage(UiBind(&BrowserBox::OnStatusMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachLoadingProgressChange(UiBind(&BrowserBox::OnLoadingProgressChange, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachMediaAccessChange(UiBind(&BrowserBox::OnMediaAccessChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_pCefControl->AttachBeforePopup(UiBind(&BrowserBox::OnBeforePopup, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9, std::placeholders::_10, std::placeholders::_11, std::placeholders::_12, std::placeholders::_13));
    m_pCefControl->AttachBeforePopupAborted(UiBind(&BrowserBox::OnBeforePopupAborted, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachBeforeBrowse(UiBind(&BrowserBox::OnBeforeBrowse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pCefControl->AttachBeforeResourceLoad(UiBind(&BrowserBox::OnBeforeResourceLoad, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachResourceRedirect(UiBind(&BrowserBox::OnResourceRedirect, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    m_pCefControl->AttachResourceResponse(UiBind(&BrowserBox::OnResourceResponse, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachResourceLoadComplete(UiBind(&BrowserBox::OnResourceLoadComplete, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    m_pCefControl->AttachProtocolExecution(UiBind(&BrowserBox::OnProtocolExecution, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_pCefControl->AttachLoadingStateChange(UiBind(&BrowserBox::OnLoadingStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachLoadStart(UiBind(&BrowserBox::OnLoadStart, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachLoadEnd(UiBind(&BrowserBox::OnLoadEnd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachLoadError(UiBind(&BrowserBox::OnLoadError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pCefControl->AttachDevToolAttachedStateChange(UiBind(&BrowserBox::OnDevToolAttachedStateChange, this, std::placeholders::_1));

    m_pCefControl->AttachCanDownload(UiBind(&BrowserBox::OnCanDownload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachBeforeDownload(UiBind(&BrowserBox::OnBeforeDownload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pCefControl->AttachDownloadUpdated(UiBind(&BrowserBox::OnDownloadUpdated, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    m_pCefControl->AttachFileDialog(UiBind(&BrowserBox::OnFileDialog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8));

    m_pCefControl->AttachDocumentAvailableInMainFrame(UiBind(&BrowserBox::OnDocumentAvailableInMainFrame, this, std::placeholders::_1));
    m_pCefControl->AttachDownloadFavIconFinished(UiBind(&BrowserBox::OnDownloadFavIconFinished, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

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

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    // 初始化任务栏缩略图
    if (GetWindow()->IsLayeredWindow()) {
        m_pTaskBarItem = new TaskbarTabItem(m_pCefControl);
        if (m_pTaskBarItem) {
            m_pTaskBarItem->Init(url, m_browserId);
        }
    }
#endif
}

void BrowserBox::UninitBrowserBox()
{
    MultiBrowserManager::GetInstance()->RemoveBorwserBox(m_browserId, this);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->UnInit();
    }
#endif
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
TaskbarTabItem* BrowserBox::GetTaskbarItem()
{
    return m_pTaskBarItem;
}
#endif

void BrowserBox::SetWindow(Window* pWindow)
{
    m_pBrowserForm = dynamic_cast<MultiBrowserForm*>(pWindow);
    ASSERT(nullptr != m_pBrowserForm);

    BaseClass::SetWindow(pWindow);
}

void BrowserBox::Invalidate()
{
    BaseClass::Invalidate();
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->InvalidateTab();
    }
#endif
}

void BrowserBox::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->InvalidateTab();
    }
#endif
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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->SetTaskbarTitle(title);
    }
#endif
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

void BrowserBox::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if ((m_pTaskBarItem != nullptr) && (image != nullptr)) {
        HICON hIcon = ConvertCefImageToHICON(*image);
        m_pTaskBarItem->SetTaskbarIcon(hIcon);
    }
#endif
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
HICON BrowserBox::ConvertCefImageToHICON(CefImage& cefImage) const
{
    // 1. 获取CefImage参数
    int32_t nWidth = cefImage.GetWidth();
    int32_t nHeight = cefImage.GetHeight();
    if ((nWidth < 1) || (nHeight < 1)) {
        return nullptr;
    }
    CefRefPtr<CefBinaryValue> cefImageData = cefImage.GetAsBitmap(1.0f, CEF_COLOR_TYPE_BGRA_8888, CEF_ALPHA_TYPE_PREMULTIPLIED, nWidth, nHeight);
    if (cefImageData == nullptr) {
        return nullptr;
    }
    size_t nDataSize = cefImageData->GetSize();
    if (nDataSize == 0) {
        return nullptr;
    }
    ASSERT((int32_t)nDataSize == nHeight * nWidth * sizeof(uint32_t));
    if ((int32_t)nDataSize != nHeight * nWidth * sizeof(uint32_t)) {
        return nullptr;
    }

    std::vector<uint8_t> imageData;
    imageData.resize(nDataSize);
    nDataSize = cefImageData->GetData(imageData.data(), imageData.size(), 0);
    ASSERT(nDataSize == imageData.size());
    if (nDataSize != imageData.size()) {
        return nullptr;
    }

    // 2. 创建颜色位图
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = nWidth;
    bmi.bmiHeader.biHeight = -nHeight;  // 顶部到底部布局
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBitsColor = nullptr;
    HBITMAP hBmpColor = ::CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &pBitsColor, nullptr, 0);
    memcpy(pBitsColor, imageData.data(), nWidth * nHeight * 4);  // 复制ARGB数据

    // 3. 创建掩码位图（此处可优化为实际掩码生成逻辑）
    HBITMAP hBmpMask = ::CreateBitmap(nWidth, nHeight, 1, 1, nullptr);

    // 4. 生成HICON
    ICONINFO iconInfo = { 0 };
    iconInfo.fIcon = TRUE;
    iconInfo.hbmColor = hBmpColor;
    iconInfo.hbmMask = hBmpMask;
    HICON hIcon = ::CreateIconIndirect(&iconInfo);

    // 5. 释放临时资源
    ::DeleteObject(hBmpColor);
    ::DeleteObject(hBmpMask);

    return hIcon;
}
#endif
