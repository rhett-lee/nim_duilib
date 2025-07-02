#include "WebView2Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "WebView2ControlImpl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui {

WebView2Control::WebView2Control(Window* pWindow):
    Control(pWindow),
    m_pImpl(std::make_unique<Impl>(this)),
    m_bUrlIsLocalFile(false)
{
}

WebView2Control::~WebView2Control()
{
}

DString WebView2Control::GetType() const { return DUI_CTR_WEBVIEW2; }

void WebView2Control::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("url")) {
        //初始化加载的URL
        SetInitURL(strValue);
    }
    else if (strName == _T("url_is_local_file")) {
        //初始化加载的URL是否为本地文件
        SetInitUrlIsLocalFile(strValue == _T("true"));
    }
    else if (strName == _T("F12")) {
        //是否允许按F12打开开发者工具
        SetAreDevToolsEnabled(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void WebView2Control::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    DString initUrl = GetInitURL();
    if (!initUrl.empty()) {
        Navigate(initUrl);
    }
}

void WebView2Control::SetInitURL(const DString& url)
{
    m_initUrl = url;
}

DString WebView2Control::GetInitURL() const
{
    DString initUrl = m_initUrl.c_str();
    if (IsInitUrlIsLocalFile() && !initUrl.empty()) {
        //该URL是本地路径
        DString url = StringUtil::MakeLowerString(initUrl);
        if ((url.find(_T("http://")) != 0) && (url.find(_T("https://")) != 0) && (url.find(_T("file:///")) != 0)) {
            //有明确的协议前缀时，不做任何转换，否则按照本地exe所在路径的资源文件加载
            FilePath webViewHtml = GlobalManager::GetDefaultResourcePath(true);
            webViewHtml.NormalizeDirectoryPath();
            webViewHtml += initUrl;
            webViewHtml.NormalizeFilePath();
            initUrl = _T("file:///");
            initUrl += webViewHtml.ToString();
            StringUtil::ReplaceAll(_T("\\"), _T("/"), initUrl);
        }
    }
    return initUrl;
}

void WebView2Control::SetInitUrlIsLocalFile(bool bUrlIsLocalFile)
{
    m_bUrlIsLocalFile = bUrlIsLocalFile;
}

bool WebView2Control::IsInitUrlIsLocalFile() const
{
    return m_bUrlIsLocalFile;
}

bool WebView2Control::DownloadFavIconImage()
{
    return m_pImpl->DownloadFavIconImage();
}

void WebView2Control::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    UiPadding rcPadding = GetPadding();
    rc.Deflate(rcPadding);
    RECT rect;
    rect.left = rc.left;
    rect.top = rc.top;
    rect.right = rc.right;
    rect.bottom = rc.bottom;
    m_pImpl->Resize(rect);
}

bool WebView2Control::OnSetFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnSetFocus(msg);
    return bRet;
}

bool WebView2Control::OnKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnKillFocus(msg);
    return bRet;
}

void WebView2Control::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);
    m_pImpl->SetVisible(bVisible);
}

void WebView2Control::SetWindow(Window* pWindow)
{
    Window* pOldWindow = BaseClass::GetWindow();
    BaseClass::SetWindow(pWindow);
    if (pOldWindow != pWindow) {
        m_pImpl->SetWindow(pWindow);
    }
}

// 其他成员函数实现，转发到Impl类
bool WebView2Control::InitializeAsync(const DString& userDataFolder, InitializeCompletedCallback callback)
{
    HRESULT hr = m_pImpl->InitializeAsync(userDataFolder, callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::Navigate(const DString& url)
{
    DString navUrl = url;
    if (navUrl.find(_T("://")) == DString::npos) {
        navUrl = _T("https://") + navUrl;
    }
    if (!IsInitializing() && !IsInitialized()) {
        //自动初始化（使用默认参数）
        InitializeAsync();
    }

    HRESULT hr = m_pImpl->Navigate(navUrl);
    m_pImpl->SetLastErrorCode(hr);
    ASSERT(SUCCEEDED(hr));
    return SUCCEEDED(hr);
}

bool WebView2Control::NavigateBack()
{
    HRESULT hr = m_pImpl->NavigateBack();
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::NavigateForward()
{
    HRESULT hr = m_pImpl->NavigateForward();
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::Refresh()
{
    HRESULT hr = m_pImpl->Refresh();
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::Stop()
{
    HRESULT hr = m_pImpl->Stop();
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::ExecuteScript(const DString& script, std::function<void(const DString& result, HRESULT hr)> callback)
{
    HRESULT hr = m_pImpl->ExecuteScript(script, callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::PostWebMessageAsJson(const DString& json)
{
    HRESULT hr = m_pImpl->PostWebMessageAsJson(json);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::PostWebMessageAsString(const DString& message)
{
    HRESULT hr = m_pImpl->PostWebMessageAsString(message);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetUserAgent(const DString& userAgent)
{
    HRESULT hr = m_pImpl->SetUserAgent(userAgent);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

DString WebView2Control::GetUserAgent() const
{
    return m_pImpl->GetUserAgent();
}

bool WebView2Control::SetZoomFactor(double zoomFactor)
{
    HRESULT hr = m_pImpl->SetZoomFactor(zoomFactor);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

double WebView2Control::GetZoomFactor() const
{
    return m_pImpl->GetZoomFactor();
}

bool WebView2Control::SetScriptEnabled(bool enabled)
{
    HRESULT hr = m_pImpl->SetScriptEnabled(enabled);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::IsScriptEnabled() const
{
    return m_pImpl->IsScriptEnabled();
}

bool WebView2Control::SetWebMessageEnabled(bool enabled)
{
    HRESULT hr = m_pImpl->SetWebMessageEnabled(enabled);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::IsWebMessageEnabled() const
{
    return m_pImpl->IsWebMessageEnabled();
}

bool WebView2Control::SetAreDefaultScriptDialogsEnabled(bool enabled)
{
    HRESULT hr = m_pImpl->SetAreDefaultScriptDialogsEnabled(enabled);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::AreDefaultScriptDialogsEnabled() const
{
    return m_pImpl->AreDefaultScriptDialogsEnabled();
}

bool WebView2Control::SetAreDefaultContextMenusEnabled(bool enabled)
{
    HRESULT hr = m_pImpl->SetAreDefaultContextMenusEnabled(enabled);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::AreDefaultContextMenusEnabled() const
{
    return m_pImpl->AreDefaultContextMenusEnabled();
}

bool WebView2Control::SetZoomControlEnabled(bool enabled)
{
    HRESULT hr = m_pImpl->SetZoomControlEnabled(enabled);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::IsZoomControlEnabled() const
{
    return m_pImpl->IsZoomControlEnabled();
}

bool WebView2Control::SetWebMessageReceivedCallback(WebMessageReceivedCallback callback)
{
    HRESULT hr = m_pImpl->SetWebMessageReceivedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetNavigationStateChangedCallback(NavigationStateChangedCallback callback)
{
    HRESULT hr = m_pImpl->SetNavigationStateChangedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetDocumentTitleChangedCallback(DocumentTitleChangedCallback callback)
{
    HRESULT hr = m_pImpl->SetDocumentTitleChangedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetSourceChangedCallback(SourceChangedCallback callback)
{
    HRESULT hr = m_pImpl->SetSourceChangedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetNewWindowRequestedCallback(NewWindowRequestedCallback callback)
{
    HRESULT hr = m_pImpl->SetNewWindowRequestedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetHistoryChangedCallback(HistoryChangedCallback callback)
{
    HRESULT hr = m_pImpl->SetHistoryChangedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::SetZoomFactorChangedCallback(ZoomFactorChangedCallback callback)
{
    HRESULT hr = m_pImpl->SetZoomFactorChangedCallback(callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

void WebView2Control::SetFavIconChangedCallback(FavIconChangedCallback callback)
{
    m_pImpl->SetFavIconChangedCallback(callback);
}

bool WebView2Control::CapturePreview(const DString& filePath,
                                     std::function<void(const DString& filePath, HRESULT hr)> callback)
{
    HRESULT hr = m_pImpl->CapturePreview(filePath, callback);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
}

bool WebView2Control::IsInitializing() const
{
    return m_pImpl->IsInitializing();
}

bool WebView2Control::IsInitialized() const
{
    return m_pImpl->IsInitialized();
}

DString WebView2Control::GetUrl() const
{
    return m_pImpl->GetUrl();
}

DString WebView2Control::GetTitle() const
{
    return m_pImpl->GetTitle();
}

bool WebView2Control::IsNavigating() const
{
    return m_pImpl->IsNavigating();
}

bool WebView2Control::CanGoBack() const
{
    return m_pImpl->CanGoBack();
}

bool WebView2Control::CanGoForward() const
{
    return m_pImpl->CanGoForward();
}

HRESULT WebView2Control::GetLastErrorCode() const
{
    return m_pImpl->GetLastErrorCode();
}

void WebView2Control::SetAreDevToolsEnabled(bool bAreDevToolsEnabled)
{
    return m_pImpl->SetAreDevToolsEnabled(bAreDevToolsEnabled);
}

bool WebView2Control::AreDevToolsEnabled() const
{
    return m_pImpl->AreDevToolsEnabled();
}

bool WebView2Control::OpenDevToolsWindow()
{
    return m_pImpl->OpenDevToolsWindow();
}

wil::com_ptr<ICoreWebView2Environment> WebView2Control::GetWebView2Environment() const
{
    return m_pImpl->GetWebView2Environment();
}

wil::com_ptr<ICoreWebView2Controller> WebView2Control::GetWebView2Controller() const
{
    return m_pImpl->GetWebView2Controller();
}

/** 获取ICoreWebView2接口
*/
wil::com_ptr<ICoreWebView2> WebView2Control::GetWebView2() const
{
    return m_pImpl->GetWebView2();
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2
