#include "WebView2Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "WebView2ControlImpl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Render/IRender.h"

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

void WebView2Control::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    m_pImpl->SetVisible(IsVisible());
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

ui::ComPtr<ICoreWebView2Environment> WebView2Control::GetWebView2Environment() const
{
    return m_pImpl->GetWebView2Environment();
}

ui::ComPtr<ICoreWebView2Controller> WebView2Control::GetWebView2Controller() const
{
    return m_pImpl->GetWebView2Controller();
}

ui::ComPtr<ICoreWebView2> WebView2Control::GetWebView2() const
{
    return m_pImpl->GetWebView2();
}

static bool CaptureWindowBitmap_Win32(HWND hwnd, RECT rect, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    if (!::IsWindow(hwnd)) {
        return false;
    }

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0) {
        return false;
    }

    // 创建设备上下文
    HDC hdcScreen = ::GetDC(nullptr);
    if (hdcScreen == nullptr) {
        return false;
    }
    HDC hdcWindow = ::GetDC(hwnd);
    if (hdcWindow == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        return false;
    }

    HDC hdcMemDC = ::CreateCompatibleDC(hdcWindow);
    if (hdcMemDC == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    // 创建位图
    HBITMAP hBitmap = ::CreateCompatibleBitmap(hdcWindow, width, height);
    if (hBitmap == nullptr) {
        ::DeleteDC(hdcMemDC);
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    HGDIOBJ hOldObj = ::SelectObject(hdcMemDC, hBitmap);

    // 拷贝屏幕内容到位图
    ::BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, rect.left, rect.top, SRCCOPY);
    
    // 获取位图信息
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // 正数表示从下到上，负数表示从上到下
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // 分配内存并获取位图数据
    bitmap.resize(width * height * 4);
    ::GetDIBits(hdcMemDC, hBitmap, 0, height, bitmap.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 清理资源
    ::SelectObject(hdcMemDC, hOldObj);
    ::DeleteObject(hBitmap);
    ::DeleteDC(hdcMemDC);
    ::ReleaseDC(nullptr, hdcScreen);
    ::ReleaseDC(hwnd, hdcWindow);

    return true;
}

/** 抓取网页的预览图（仅控件显示区域，并不是整个网页）
*/
static bool CaptureWindowBitmap_Win32(WebView2Control* pWebView2Control, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    if (pWebView2Control == nullptr) {
        return false;
    }
    ui::ComPtr<ICoreWebView2Controller> spWebView2Controller = pWebView2Control->GetWebView2Controller();
    if (spWebView2Controller == nullptr) {
        return false;
    }

    HWND hParentWnd = nullptr;
    if (FAILED(spWebView2Controller->get_ParentWindow(&hParentWnd)) || !::IsWindow(hParentWnd)) {
        return false;
    }
    
    UiRect rcControl = pWebView2Control->GetRect();
    if (rcControl.IsEmpty()) {
        return false;
    }

    UiPoint ptOffset = pWebView2Control->GetScrollOffsetInScrollBox();
    rcControl.Offset(-ptOffset.x, -ptOffset.y);

    UiPoint ptLeftTop = { rcControl.left, rcControl.top};
    pWebView2Control->ClientToScreen(ptLeftTop);
    RECT rcScreenWebView2Control;
    rcScreenWebView2Control.left = ptLeftTop.x;
    rcScreenWebView2Control.top = ptLeftTop.y;
    rcScreenWebView2Control.right = rcScreenWebView2Control.left + rcControl.Width();
    rcScreenWebView2Control.bottom = rcScreenWebView2Control.top + rcControl.Height();

    RECT rcWindow = { 0, 0, 0, 0 };
    ::GetWindowRect(hParentWnd, &rcWindow);

    RECT rcValidWebView2Control = { 0, 0, 0, 0 };
    ::IntersectRect(&rcValidWebView2Control, &rcScreenWebView2Control, &rcWindow);

    HWND hDesktopWnd = ::GetDesktopWindow();
    if (!::IsWindow(hDesktopWnd)) {
        return false;
    }
    RECT rcDesktop = { 0 ,0 ,0 ,0 };
    ::GetWindowRect(hDesktopWnd, &rcDesktop);

    RECT rcWebView2Control = { 0, 0, 0, 0 };
    ::IntersectRect(&rcWebView2Control, &rcScreenWebView2Control, &rcDesktop);

    ::ScreenToClient(hDesktopWnd, (LPPOINT)&rcWebView2Control);
    return CaptureWindowBitmap_Win32(hDesktopWnd, rcWebView2Control, bitmap, width, height);
}

std::shared_ptr<IBitmap> WebView2Control::MakeImageSnapshot()
{
    std::vector<uint8_t> bitmap;
    int32_t width = 0;
    int32_t height = 0;
    bool bRet = CaptureWindowBitmap_Win32(this, bitmap, width, height);
    if (bRet && (width > 0) && (height > 0) && ((int32_t)bitmap.size() == (width * height * 4))) {
        std::shared_ptr<IBitmap> spBitmap;
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            spBitmap.reset(pRenderFactory->CreateBitmap());
            if (spBitmap != nullptr) {
                if (!spBitmap->Init(width, height, true, bitmap.data())) {
                    spBitmap.reset();
                }
            }
        }
        return spBitmap;
    }
    return nullptr;
}
} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2
