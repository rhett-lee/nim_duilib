#ifndef UI_WEBVIEW2_ENVIRONMENT_OPTIONS_H_
#define UI_WEBVIEW2_ENVIRONMENT_OPTIONS_H_

#include "duilib/duilib_defs.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include <combaseapi.h>
#include "duilib/third_party/Microsoft.Web.WebView2/build/native/include/WebView2.h"

#include <vector>

// 前置声明
interface ICoreWebView2CustomSchemeRegistration;

//WebView2的版本：注意更新WebView2.h的时候，需要检查该值是否需要更新
#define CORE_WEBVIEW_TARGET_PRODUCT_VERSION L"137.0.3296.44"

namespace ui
{
class WebView2EnvironmentOptions :
    public ICoreWebView2EnvironmentOptions,
    public ICoreWebView2EnvironmentOptions2,
    public ICoreWebView2EnvironmentOptions3,
    public ICoreWebView2EnvironmentOptions4,
    public ICoreWebView2EnvironmentOptions5,
    public ICoreWebView2EnvironmentOptions6,
    public ICoreWebView2EnvironmentOptions7,
    public ICoreWebView2EnvironmentOptions8
{
public:
    // 构造函数和析构函数
    WebView2EnvironmentOptions();
    virtual ~WebView2EnvironmentOptions();

    // IUnknown 实现
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // ICoreWebView2EnvironmentOptions 实现
    IFACEMETHODIMP get_AdditionalBrowserArguments(LPWSTR* value) override;
    IFACEMETHODIMP put_AdditionalBrowserArguments(LPCWSTR value) override;
    IFACEMETHODIMP get_Language(LPWSTR* value) override;
    IFACEMETHODIMP put_Language(LPCWSTR value) override;
    IFACEMETHODIMP get_TargetCompatibleBrowserVersion(LPWSTR* value) override;
    IFACEMETHODIMP put_TargetCompatibleBrowserVersion(LPCWSTR value) override;
    IFACEMETHODIMP get_AllowSingleSignOnUsingOSPrimaryAccount(BOOL* allow) override;
    IFACEMETHODIMP put_AllowSingleSignOnUsingOSPrimaryAccount(BOOL allow) override;

    // ICoreWebView2EnvironmentOptions2 实现
    IFACEMETHODIMP get_ExclusiveUserDataFolderAccess(BOOL* value) override;
    IFACEMETHODIMP put_ExclusiveUserDataFolderAccess(BOOL value) override;

    // ICoreWebView2EnvironmentOptions3 实现
    IFACEMETHODIMP get_IsCustomCrashReportingEnabled(BOOL* value) override;
    IFACEMETHODIMP put_IsCustomCrashReportingEnabled(BOOL value) override;

    // ICoreWebView2EnvironmentOptions4 实现
    IFACEMETHODIMP GetCustomSchemeRegistrations(UINT32* count, ICoreWebView2CustomSchemeRegistration*** schemeRegistrations) override;
    IFACEMETHODIMP SetCustomSchemeRegistrations(UINT32 count, ICoreWebView2CustomSchemeRegistration** schemeRegistrations) override;

    // ICoreWebView2EnvironmentOptions5 实现
    IFACEMETHODIMP get_EnableTrackingPrevention(BOOL* value) override;
    IFACEMETHODIMP put_EnableTrackingPrevention(BOOL value) override;

    // ICoreWebView2EnvironmentOptions6 实现
    IFACEMETHODIMP get_AreBrowserExtensionsEnabled(BOOL* value) override;
    IFACEMETHODIMP put_AreBrowserExtensionsEnabled(BOOL value) override;

    // ICoreWebView2EnvironmentOptions7 实现
    IFACEMETHODIMP get_ChannelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND* value) override;
    IFACEMETHODIMP put_ChannelSearchKind(COREWEBVIEW2_CHANNEL_SEARCH_KIND value) override;
    IFACEMETHODIMP get_ReleaseChannels(COREWEBVIEW2_RELEASE_CHANNELS* value) override;
    IFACEMETHODIMP put_ReleaseChannels(COREWEBVIEW2_RELEASE_CHANNELS value) override;

    // ICoreWebView2EnvironmentOptions8 实现
    IFACEMETHODIMP get_ScrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE* value) override;
    IFACEMETHODIMP put_ScrollBarStyle(COREWEBVIEW2_SCROLLBAR_STYLE value) override;

    // 创建实例的静态方法
    static HRESULT CreateInstance(ICoreWebView2EnvironmentOptions** ppOptions);

private:
    LONG m_refCount;

    // ICoreWebView2EnvironmentOptions 属性
    std::wstring m_additionalBrowserArguments;
    std::wstring m_language;
    std::wstring m_targetCompatibleBrowserVersion;
    BOOL m_allowSingleSignOn;

    // ICoreWebView2EnvironmentOptions2 属性
    BOOL m_exclusiveUserDataFolderAccess;

    // ICoreWebView2EnvironmentOptions3 属性
    BOOL m_isCustomCrashReportingEnabled;

    // ICoreWebView2EnvironmentOptions4 属性
    std::vector<ICoreWebView2CustomSchemeRegistration*> m_customSchemeRegistrations;

    // ICoreWebView2EnvironmentOptions5 属性
    BOOL m_enableTrackingPrevention;

    // ICoreWebView2EnvironmentOptions6 属性
    BOOL m_areBrowserExtensionsEnabled;

    // ICoreWebView2EnvironmentOptions7 属性
    COREWEBVIEW2_CHANNEL_SEARCH_KIND m_channelSearchKind;
    COREWEBVIEW2_RELEASE_CHANNELS m_releaseChannels;

    // ICoreWebView2EnvironmentOptions8 属性
    COREWEBVIEW2_SCROLLBAR_STYLE m_scrollBarStyle;
};

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#endif //UI_WEBVIEW2_ENVIRONMENT_OPTIONS_H_
