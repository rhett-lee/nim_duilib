#include "WebView2Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "WebView2ControlImpl.h"

namespace ui {

WebView2Control::WebView2Control(Window* pWindow):
    Control(pWindow),
    m_pImpl(std::make_unique<Impl>(this))
{
}

WebView2Control::~WebView2Control()
{
}

DString WebView2Control::GetType() const { return DUI_CTR_WEBVIEW2; }

void WebView2Control::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("url")) {        
        Navigate(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
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
    //TODO:
    bool bRet = BaseClass::OnSetFocus(msg);
    return bRet;
}

bool WebView2Control::OnKillFocus(const EventArgs& msg)
{
    //TODO:
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

bool WebView2Control::SetNavigationCompletedCallback(NavigationCompletedCallback callback)
{
    HRESULT hr = m_pImpl->SetNavigationCompletedCallback(callback);
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

bool WebView2Control::SetContentLoadingCallback(ContentLoadingCallback callback)
{
    HRESULT hr = m_pImpl->SetContentLoadingCallback(callback);
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

bool WebView2Control::AddScriptToExecuteOnDocumentCreated(const DString& script)
{
    HRESULT hr = m_pImpl->AddScriptToExecuteOnDocumentCreated(script);
    m_pImpl->SetLastErrorCode(hr);
    return SUCCEEDED(hr);
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

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2
