#include "WebView2EnvironmentOptions.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

namespace ui
{

WebView2EnvironmentOptions::WebView2EnvironmentOptions():
    m_refCount(0),
    m_allowSingleSignOn(FALSE),
    m_exclusiveUserDataFolderAccess(FALSE),
    m_isCustomCrashReportingEnabled(FALSE),
    m_enableTrackingPrevention(TRUE),
    m_areBrowserExtensionsEnabled(FALSE),
    m_channelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND_MOST_STABLE),
    m_releaseChannels(COREWEBVIEW2_RELEASE_CHANNELS_NONE),
    m_scrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE_DEFAULT)
{
}

WebView2EnvironmentOptions::~WebView2EnvironmentOptions()
{
    // 释放自定义方案注册的引用
    for (auto registration : m_customSchemeRegistrations) {
        if (registration) {
            registration->Release();
        }
    }
}

// IUnknown 实现
IFACEMETHODIMP WebView2EnvironmentOptions::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject) {
        return E_POINTER;
    }
        
    *ppvObject = nullptr;
    
    if (riid == IID_IUnknown) {
        *ppvObject = static_cast<IUnknown*>((ICoreWebView2EnvironmentOptions*)this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions2) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions2*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions3) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions3*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions4) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions4*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions5) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions5*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions6) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions6*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions7) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions7*>(this);
    }
    else if (riid == IID_ICoreWebView2EnvironmentOptions8) {
        *ppvObject = static_cast<ICoreWebView2EnvironmentOptions8*>(this);
    }
    else {
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) WebView2EnvironmentOptions::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) WebView2EnvironmentOptions::Release()
{
    ULONG refCount = InterlockedDecrement(&m_refCount);
    if (refCount == 0) {
        delete this;
    }
    return refCount;
}

// ICoreWebView2EnvironmentOptions 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_AdditionalBrowserArguments(LPWSTR* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = static_cast<LPWSTR>(CoTaskMemAlloc((m_additionalBrowserArguments.size() + 1) * sizeof(WCHAR)));
    if (!*value) {
        return E_OUTOFMEMORY;
    }
        
    wcscpy_s(*value, m_additionalBrowserArguments.size() + 1, m_additionalBrowserArguments.c_str());
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_AdditionalBrowserArguments(LPCWSTR value)
{
    m_additionalBrowserArguments = value ? value : L"";
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::get_Language(LPWSTR* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = static_cast<LPWSTR>(CoTaskMemAlloc((m_language.size() + 1) * sizeof(WCHAR)));
    if (!*value) {
        return E_OUTOFMEMORY;
    }
        
    wcscpy_s(*value, m_language.size() + 1, m_language.c_str());
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_Language(LPCWSTR value)
{
    m_language = value ? value : L"";
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::get_TargetCompatibleBrowserVersion(LPWSTR* value)
{
    if (!value) {
        return E_POINTER;
    }
    std::wstring targetCompatibleBrowserVersion = m_targetCompatibleBrowserVersion;
    if (targetCompatibleBrowserVersion.empty()) {
        targetCompatibleBrowserVersion = CORE_WEBVIEW_TARGET_PRODUCT_VERSION;
    }
        
    *value = static_cast<LPWSTR>(CoTaskMemAlloc((targetCompatibleBrowserVersion.size() + 1) * sizeof(WCHAR)));
    if (!*value) {
        return E_OUTOFMEMORY;
    }
        
    wcscpy_s(*value, targetCompatibleBrowserVersion.size() + 1, targetCompatibleBrowserVersion.c_str());
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_TargetCompatibleBrowserVersion(LPCWSTR value)
{
    m_targetCompatibleBrowserVersion = value ? value : L"";
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::get_AllowSingleSignOnUsingOSPrimaryAccount(BOOL* allow)
{
    if (!allow) {
        return E_POINTER;
    }
        
    *allow = m_allowSingleSignOn;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_AllowSingleSignOnUsingOSPrimaryAccount(BOOL allow)
{
    m_allowSingleSignOn = allow;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions2 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_ExclusiveUserDataFolderAccess(BOOL* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_exclusiveUserDataFolderAccess;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_ExclusiveUserDataFolderAccess(BOOL value)
{
    m_exclusiveUserDataFolderAccess = value;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions3 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_IsCustomCrashReportingEnabled(BOOL* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_isCustomCrashReportingEnabled;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_IsCustomCrashReportingEnabled(BOOL value)
{
    m_isCustomCrashReportingEnabled = value;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions4 实现
IFACEMETHODIMP WebView2EnvironmentOptions::GetCustomSchemeRegistrations(
    UINT32* count, 
    ICoreWebView2CustomSchemeRegistration*** schemeRegistrations)
{
    if (!count || !schemeRegistrations) {
        return E_POINTER;
    }
        
    *count = static_cast<UINT32>(m_customSchemeRegistrations.size());
    
    if (*count == 0) {
        *schemeRegistrations = nullptr;
        return S_OK;
    }
    
    *schemeRegistrations = static_cast<ICoreWebView2CustomSchemeRegistration**>(
        CoTaskMemAlloc(*count * sizeof(ICoreWebView2CustomSchemeRegistration*)));
    
    if (!*schemeRegistrations) {
        return E_OUTOFMEMORY;
    }
        
    for (UINT32 i = 0; i < *count; i++) {
        (*schemeRegistrations)[i] = m_customSchemeRegistrations[i];
        (*schemeRegistrations)[i]->AddRef();
    }
    
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::SetCustomSchemeRegistrations(
    UINT32 count, 
    ICoreWebView2CustomSchemeRegistration** schemeRegistrations)
{
    // 先释放现有注册
    for (auto registration : m_customSchemeRegistrations) {
        if (registration) {
            registration->Release();
        }
    }
    m_customSchemeRegistrations.clear();
    
    // 添加新注册
    if (count > 0 && schemeRegistrations) {
        m_customSchemeRegistrations.reserve(count);
        for (UINT32 i = 0; i < count; i++) {
            if (schemeRegistrations[i]) {
                schemeRegistrations[i]->AddRef();
                m_customSchemeRegistrations.push_back(schemeRegistrations[i]);
            }
        }
    }
    
    return S_OK;
}

// ICoreWebView2EnvironmentOptions5 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_EnableTrackingPrevention(BOOL* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_enableTrackingPrevention;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_EnableTrackingPrevention(BOOL value)
{
    m_enableTrackingPrevention = value;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions6 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_AreBrowserExtensionsEnabled(BOOL* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_areBrowserExtensionsEnabled;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_AreBrowserExtensionsEnabled(BOOL value)
{
    m_areBrowserExtensionsEnabled = value;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions7 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_ChannelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_channelSearchKind;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_ChannelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND value)
{
    m_channelSearchKind = value;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::get_ReleaseChannels(COREWEBVIEW2_RELEASE_CHANNELS* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_releaseChannels;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_ReleaseChannels(COREWEBVIEW2_RELEASE_CHANNELS value)
{
    m_releaseChannels = value;
    return S_OK;
}

// ICoreWebView2EnvironmentOptions8 实现
IFACEMETHODIMP WebView2EnvironmentOptions::get_ScrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE* value)
{
    if (!value) {
        return E_POINTER;
    }
        
    *value = m_scrollBarStyle;
    return S_OK;
}

IFACEMETHODIMP WebView2EnvironmentOptions::put_ScrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE value)
{
    m_scrollBarStyle = value;
    return S_OK;
}

// 创建实例的静态方法
HRESULT WebView2EnvironmentOptions::CreateInstance(ICoreWebView2EnvironmentOptions** ppOptions)
{
    if (!ppOptions) {
        return E_POINTER;
    }
        
    *ppOptions = nullptr;
    
    try {
        auto options = new (std::nothrow) WebView2EnvironmentOptions();
        if (!options) {
            return E_OUTOFMEMORY;
        }
            
        *ppOptions = options;
        options->AddRef();
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)
