#include "WebView2ControlImpl.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include "WebView2Manager.h"
#include "WebView2EnvironmentOptions.h"
#include "ComCallback.h"

#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/ImageInfo.h"

#include <shlwapi.h>
#include <sstream>

#include <wininet.h>

#ifdef _MSC_VER
    #pragma comment(lib, "WinInet.lib")
#endif

namespace ui {

WebView2Control::Impl::Impl(Control* pControl)
    : m_pControl(pControl)
    , m_bInitializing(false)
    , m_bInitialized(false)
    , m_bNavigating(false)
    , m_lastError(S_OK)
    , m_bAreDevToolsEnabled(true)
    , m_bAreDevToolsEnabledSet(false)
    , m_bAreDefaultContextMenusEnabled(true)
    , m_bAreDefaultContextMenusEnabledSet(false)
    , m_bAreDefaultScriptDialogsEnabled(true)
    , m_bAreDefaultScriptDialogsEnabledSet(false)
    , m_bWebMessageEnabled(true)
    , m_bWebMessageEnabledSet(false)
    , m_bZoomControlEnabled(true)
    , m_bZoomControlEnabledSet(false)
{
    ASSERT(m_pControl != nullptr);
}

WebView2Control::Impl::~Impl()
{
    Cleanup();
}

HRESULT WebView2Control::Impl::InitializeAsync(const DString& userDataFolder, InitializeCompletedCallback callback)
{
    if (m_bInitialized) {
        if (callback) {
            callback(S_OK);
        }
        return S_OK;
    }

    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return E_FAIL;
    }
    ASSERT(m_pControl->GetWindow() != nullptr);
    if (m_pControl->GetWindow() == nullptr) {
        return E_FAIL;
    }
    ASSERT(m_pControl->GetWindow()->NativeWnd()->GetHWND() != nullptr);
    if (m_pControl->GetWindow()->NativeWnd()->GetHWND() == nullptr) {
        return E_FAIL;
    }

    ASSERT(!m_bInitializing);
    if (m_bInitializing) {
        return E_FAIL;
    }

    m_userDataFolder = userDataFolder;
    m_initializeCompletedCallback = callback;

    // 开始异步初始化流程
    m_bInitializing = true;
    return CreateEnvironmentAsync();
}

HRESULT WebView2Control::Impl::CallCreateCoreWebView2EnvironmentWithOptions(PCWSTR browserExecutableFolder,
    PCWSTR userDataFolder,
    ICoreWebView2EnvironmentOptions* environmentOptions,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler)
{
#ifndef DUILIB_USE_WEBVIEW2_LOADER_DLL
    return CreateCoreWebView2EnvironmentWithOptions(browserExecutableFolder,
                                                    userDataFolder,
                                                    environmentOptions,
                                                    environmentCreatedHandler);
#else
    if (m_hWebView2Loader == nullptr) {
        FilePath runPath = FilePathUtil::GetCurrentModuleDirectory();
        runPath.NormalizeDirectoryPath();
        runPath += _T("WebView2Loader.dll");
        if (runPath.IsExistsFile()) {
            m_hWebView2Loader = ::LoadLibrary(runPath.NativePath().c_str());
        }        
    }
    ASSERT(m_hWebView2Loader != nullptr);
    if (m_hWebView2Loader == nullptr) {
        return E_FAIL;
    }
    typedef HRESULT (STDAPICALLTYPE *PFNCreateCoreWebView2EnvironmentWithOptions)(PCWSTR browserExecutableFolder, PCWSTR userDataFolder, ICoreWebView2EnvironmentOptions * environmentOptions, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler * environmentCreatedHandler);
    PFNCreateCoreWebView2EnvironmentWithOptions pfnCreateCoreWebView2EnvironmentWithOptions = (PFNCreateCoreWebView2EnvironmentWithOptions)::GetProcAddress(m_hWebView2Loader, "CreateCoreWebView2EnvironmentWithOptions");
    if (pfnCreateCoreWebView2EnvironmentWithOptions == nullptr) {
        return E_FAIL;
    }
    return pfnCreateCoreWebView2EnvironmentWithOptions(browserExecutableFolder,
                                                       userDataFolder,
                                                       environmentOptions,
                                                       environmentCreatedHandler);
#endif
}

ui::ComPtr<ICoreWebView2EnvironmentOptions> WebView2Control::Impl::CreateCoreWebView2EnvironmentOptionsObj()
{
    ui::ComPtr<ICoreWebView2EnvironmentOptions> spEnvironmentOptions;
    WebView2EnvironmentOptions::CreateInstance(&spEnvironmentOptions);
    ASSERT(spEnvironmentOptions != nullptr);
    return spEnvironmentOptions;
}

HRESULT WebView2Control::Impl::CreateEnvironmentAsync()
{
    if (m_spWebView2Environment != nullptr) {
        return CreateControllerAsync();
    }
    DString userDataFolder = m_userDataFolder;
    if (userDataFolder.empty()) {
        userDataFolder = WebView2Manager::GetInstance().GetUserDataFolder();
    }
    DString browserExecutableFolder = WebView2Manager::GetInstance().GetBrowserExecutableFolder();

    ui::ComPtr<ICoreWebView2EnvironmentOptions> spEnvironmentOptions;
    DString language = WebView2Manager::GetInstance().GetLanguage();
    DString additionalBrowserArguments = WebView2Manager::GetInstance().GetAdditionalBrowserArguments();
    if (!language.empty() || !additionalBrowserArguments.empty()) {
        spEnvironmentOptions = CreateCoreWebView2EnvironmentOptionsObj();
        if (spEnvironmentOptions != nullptr) {
            if (!language.empty()) {
                spEnvironmentOptions->put_Language(StringConvert::TToWString(language).c_str());
            }
            if (!additionalBrowserArguments.empty()) {
                spEnvironmentOptions->put_AdditionalBrowserArguments(StringConvert::TToWString(additionalBrowserArguments).c_str());
            }
        }
    }

    // 创建WebView2环境
    std::wstring browserExecutableFolderW = StringConvert::TToWString(browserExecutableFolder);
    std::wstring userDataFolderW = StringConvert::TToWString(userDataFolder);
    return CallCreateCoreWebView2EnvironmentWithOptions(
        !browserExecutableFolderW.empty() ? browserExecutableFolderW.c_str() : nullptr,
        !userDataFolderW.empty() ? userDataFolderW.c_str() : nullptr,
        spEnvironmentOptions.Get(),
        ui::ComCallback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
                        IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                ASSERT(m_spWebView2Environment == nullptr);
                if (SUCCEEDED(result) && (env != nullptr)) {
                    m_spWebView2Environment = env;
                    HRESULT hr = CreateControllerAsync();
                    return hr;
                }
                else {
                    HRESULT hr = result;
                    if (SUCCEEDED(hr)) {
                        hr = E_FAIL;
                    }
                    OnInitializationCompleted(hr);
                    return hr;
                }
            }).Get());
}

HRESULT WebView2Control::Impl::CreateControllerAsync()
{
    if ((m_spWebView2Environment == nullptr) || (m_pControl == nullptr)) {
        HRESULT hr = E_FAIL;
        OnInitializationCompleted(hr);
        return hr;
    }

    HWND hWnd = nullptr;
    Window* pWindow = m_pControl->GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow != nullptr) {
        hWnd = pWindow->NativeWnd()->GetHWND();
    }
    if (!::IsWindow(hWnd)) {
        HRESULT hr = E_FAIL;
        OnInitializationCompleted(hr);
        return hr;
    }

    if ((m_spWebView2Controller != nullptr) && (m_spWebView2 != nullptr)) {
        HRESULT hr = S_OK;
        OnInitializationCompleted(hr);
        return hr;
    }

    // 创建WebView2控制器
    return m_spWebView2Environment->CreateCoreWebView2Controller(
        hWnd,
        ui::ComCallback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                        IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                ASSERT(m_spWebView2Controller == nullptr);
                ASSERT(m_spWebView2 == nullptr);
                bool bInitWebView = false;
                if (SUCCEEDED(result) && (controller != nullptr)) {
                    m_spWebView2Controller = controller;
                    // 获取WebView实例 - 使用同步调用
                    ui::ComPtr<ICoreWebView2> webview;
                    HRESULT hr = controller->get_CoreWebView2(&webview);
                    if (SUCCEEDED(hr) && (webview != nullptr)) {
                        m_spWebView2 = webview;
                        bInitWebView = true;                     
                    }
                }
                HRESULT hr = bInitWebView ? S_OK : result;
                if (!bInitWebView && SUCCEEDED(hr)) {
                    hr = E_FAIL;
                }
                OnInitializationCompleted(hr);
                return hr;
            }).Get());
}

void WebView2Control::Impl::OnInitializationCompleted(HRESULT result)
{
    if (SUCCEEDED(result)) {
        ASSERT(m_spWebView2Environment != nullptr);
        ASSERT(m_spWebView2Controller != nullptr);
        ASSERT(m_spWebView2 != nullptr);
        if ((m_spWebView2Environment == nullptr) || (m_spWebView2Controller == nullptr) || (m_spWebView2 == nullptr)) {
            result = E_FAIL;
        }
    }
    if (!SUCCEEDED(result)) {
        m_bInitialized = false;
        m_bInitializing = false;
        m_bNavigating = false;
        auto callback = std::move(m_initializeCompletedCallback);
        if (callback) {
            callback(result);
        }
        return;
    }

    //注册事件回调函数
    AddNewWindowRequestedCallback();
    AddNavigationStateChangedCallback();

    if (m_historyChangedCallback != nullptr) {
        SetHistoryChangedCallback(m_historyChangedCallback);
    }

    if (m_sourceChangedCallback != nullptr) {
        SetSourceChangedCallback(m_sourceChangedCallback);
    }

    if (m_documentTitleChangedCallback != nullptr) {
        SetDocumentTitleChangedCallback(m_documentTitleChangedCallback);
    }

    if (m_webMessageReceivedCallback != nullptr) {
        SetWebMessageReceivedCallback(m_webMessageReceivedCallback);
    }

    if (m_zoomFactorChangedCallback != nullptr) {
        SetZoomFactorChangedCallback(m_zoomFactorChangedCallback);
    }

    //初始化各个选项
    InitializeSettings();

    //初始化焦点事件
    if (m_spWebView2Controller != nullptr) {
        HRESULT hr = m_spWebView2Controller->add_GotFocus(
            ui::ComCallback<ICoreWebView2FocusChangedEventHandler,
                            IID_ICoreWebView2FocusChangedEventHandler>(
                [this](ICoreWebView2Controller* sender, IUnknown* /*args*/) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2Controller.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());
                    //页面获取焦点时，同步设置关联控件的焦点
                    if (m_pControl != nullptr) {
                        m_pControl->SetFocus();
                    }
                    return S_OK;
                }).Get(), &m_gotFocusToken);
        ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
    }

    //初始化控件的位置
    if ((m_pControl != nullptr) && (m_spWebView2Controller != nullptr)) {
        UiRect rc = m_pControl->GetRect();
        UiPadding rcPadding = m_pControl->GetPadding();
        rc.Deflate(rcPadding);
        RECT rect;
        rect.left = rc.left;
        rect.top = rc.top;
        rect.right = rc.right;
        rect.bottom = rc.bottom;
        m_spWebView2Controller->put_Bounds(rect);

        bool bVisible = m_pControl->IsVisible();
        m_spWebView2Controller->put_IsVisible(bVisible ? TRUE : FALSE);
    }

    if (m_favIconChangedCallback != nullptr) {
        SetFavIconChangedCallback(m_favIconChangedCallback);
    }

    //初始化的URL，导航到该网址
    if (!m_navigateUrl.empty()) {
        std::wstring url = StringConvert::TToWString(m_navigateUrl);
        m_navigateUrl.clear();
        if (m_spWebView2 != nullptr) {
            m_spWebView2->Navigate(url.c_str());
        }
    }

    m_bInitialized = true;
    m_bInitializing = false;
    m_bNavigating = false;

    // 调用初始化完成回调
    auto callback = std::move(m_initializeCompletedCallback);
    if (callback) {
        callback(result);
    }
}

void WebView2Control::Impl::InitializeSettings()
{
    ASSERT(m_spWebView2 != nullptr);
    if (m_spWebView2 == nullptr) {
        return;
    }

    ui::ComPtr<ICoreWebView2Settings> settings;
    HRESULT hr = m_spWebView2->get_Settings(&settings);
    ASSERT(SUCCEEDED(hr) && (settings != nullptr));
    if (FAILED(hr) || (settings == nullptr)) {
        return;
    }

    // 初始化设置 - 同步操作，但包装在异步流程中
    if (m_bScriptEnabledSet) {
        hr = settings->put_IsScriptEnabled(m_bScriptEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_IsScriptEnabled(WebView2Manager::GetInstance().IsScriptEnabled() ? TRUE : FALSE);
    }
    ASSERT(SUCCEEDED(hr));

    if (m_bWebMessageEnabledSet) {
        hr = settings->put_IsWebMessageEnabled(m_bWebMessageEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_IsWebMessageEnabled(WebView2Manager::GetInstance().IsWebMessageEnabled() ? TRUE : FALSE);
    }
    ASSERT(SUCCEEDED(hr));

    if (m_bAreDefaultScriptDialogsEnabledSet) {
        hr = settings->put_AreDefaultScriptDialogsEnabled(m_bAreDefaultScriptDialogsEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_AreDefaultScriptDialogsEnabled(WebView2Manager::GetInstance().AreDefaultScriptDialogsEnabled() ? TRUE : FALSE);
    }
    ASSERT(SUCCEEDED(hr));

    if (m_bAreDefaultContextMenusEnabledSet) {
        hr = settings->put_AreDefaultContextMenusEnabled(m_bAreDefaultContextMenusEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_AreDefaultContextMenusEnabled(WebView2Manager::GetInstance().AreDefaultContextMenusEnabled() ? TRUE : FALSE);
    }
    ASSERT(SUCCEEDED(hr));

    if (m_bAreDevToolsEnabledSet) {
        hr = settings->put_AreDevToolsEnabled(m_bAreDevToolsEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_AreDevToolsEnabled(WebView2Manager::GetInstance().AreDevToolsEnabled() ? TRUE : FALSE);
    }    
    ASSERT(SUCCEEDED(hr));

    if (m_bZoomControlEnabledSet) {
        hr = settings->put_IsZoomControlEnabled(m_bZoomControlEnabled ? TRUE : FALSE);
    }
    else {
        hr = settings->put_IsZoomControlEnabled(WebView2Manager::GetInstance().IsZoomControlEnabled() ? TRUE : FALSE);
    }
    ASSERT(SUCCEEDED(hr));

    DString userAgent = m_userAgent;
    if (userAgent.empty()) {
        userAgent = WebView2Manager::GetInstance().GetUserAgent();
    }
    if (!userAgent.empty()) {
        ui::ComPtr<ICoreWebView2Settings2> settings2;
        settings->QueryInterface(IID_ICoreWebView2Settings2, (void**)&settings2);
        if (settings2 != nullptr) {
            settings2->put_UserAgent(StringConvert::TToWString(userAgent).c_str());
        }
    }
}

HRESULT WebView2Control::Impl::Navigate(const DString& url)
{
    if (m_spWebView2 != nullptr) {
        m_navigateUrl.clear();
        return m_spWebView2->Navigate(StringConvert::TToWString(url).c_str());
    }
    else {
        m_navigateUrl = url;
        return S_OK;
    }    
}

HRESULT WebView2Control::Impl::NavigateBack()
{
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    return m_spWebView2->GoBack();
}

HRESULT WebView2Control::Impl::NavigateForward()
{
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    return m_spWebView2->GoForward();
}

HRESULT WebView2Control::Impl::Refresh()
{
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    return m_spWebView2->Reload();
}

HRESULT WebView2Control::Impl::Stop()
{
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    return m_spWebView2->Stop();
}

HRESULT WebView2Control::Impl::ExecuteScript(const DString& script,
                                             std::function<void(const DString& result, HRESULT hr)> callback)
{
    ASSERT(!script.empty());
    if (script.empty()) {
        return E_INVALIDARG;
    }
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }

    DStringW scriptW = StringConvert::TToWString(script);
    return m_spWebView2->ExecuteScript(
        scriptW.c_str(),
        ui::ComCallback<ICoreWebView2ExecuteScriptCompletedHandler,
                        IID_ICoreWebView2ExecuteScriptCompletedHandler>(
            [callback](HRESULT errorCode, PCWSTR resultObjectAsJson) -> HRESULT {
                if (callback) {
                    std::wstring result = resultObjectAsJson ? resultObjectAsJson : L"";
                    DString resultT = StringConvert::WStringToT(result);
                    callback(resultT, errorCode);
                }
                return S_OK;
            }).Get());
}

HRESULT WebView2Control::Impl::PostWebMessageAsJson(const DString& json)
{
    ASSERT(!json.empty());
    if (json.empty()) {
        return E_INVALIDARG;
    }
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    DStringW jsonW = StringConvert::TToWString(json);
    return m_spWebView2->PostWebMessageAsJson(jsonW.c_str());
}

HRESULT WebView2Control::Impl::PostWebMessageAsString(const DString& message)
{
    ASSERT(!message.empty());
    if (message.empty()) {
        return E_INVALIDARG;
    }
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    DStringW messageW = StringConvert::TToWString(message);
    return m_spWebView2->PostWebMessageAsString(messageW.c_str());
}

HRESULT WebView2Control::Impl::SetUserAgent(const DString& userAgent)
{
    m_userAgent = userAgent;
    if ((m_spWebView2 != nullptr) && !userAgent.empty()) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        ui::ComPtr<ICoreWebView2Settings2> settings2;
        hr = settings->QueryInterface(IID_ICoreWebView2Settings2, (void**)&settings2);
        if (FAILED(hr) || (settings2 == nullptr)) {
            return hr;
        }
        hr = settings2->put_UserAgent(StringConvert::TToWString(userAgent).c_str());
        return hr;
    }
    return S_OK;
}

DString WebView2Control::Impl::GetUserAgent() const
{
    DString userAgent = m_userAgent;
    if ((m_spWebView2 != nullptr) && !userAgent.empty()) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return userAgent;
        }
        ui::ComPtr<ICoreWebView2Settings2> settings2;
        hr = settings->QueryInterface(IID_ICoreWebView2Settings2, (void**)&settings2);
        if (FAILED(hr) || (settings2 == nullptr)) {
            return userAgent;
        }
        LPWSTR value = nullptr;
        hr = settings2->get_UserAgent(&value);
        if (value != nullptr) {
            userAgent = StringConvert::WStringToT(value);
            ::CoTaskMemFree(value);
            value = nullptr;
        }
    }
    return userAgent;
}

HRESULT WebView2Control::Impl::SetZoomFactor(double zoomFactor)
{
    if (m_spWebView2Controller == nullptr) {
        return E_FAIL;
    }
    return m_spWebView2Controller->put_ZoomFactor(zoomFactor);
}

double WebView2Control::Impl::GetZoomFactor() const
{
    if (m_spWebView2Controller == nullptr) {
        return 1.0;
    }
    double zoomFactor = 1.0;
    m_spWebView2Controller->get_ZoomFactor(&zoomFactor);
    return zoomFactor;
}

HRESULT WebView2Control::Impl::SetScriptEnabled(bool enabled)
{
    m_bScriptEnabled = enabled;
    m_bScriptEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        return settings->put_IsScriptEnabled(enabled ? TRUE : FALSE);
    }
    return S_OK;
}

bool WebView2Control::Impl::IsScriptEnabled() const
{
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (SUCCEEDED(hr) && (settings != nullptr)) {
            BOOL bEnabled = TRUE;
            if (SUCCEEDED(settings->get_IsScriptEnabled(&bEnabled))) {
                return bEnabled ? true : false;
            }
        }
    }
    return m_bScriptEnabled;
}

HRESULT WebView2Control::Impl::SetWebMessageEnabled(bool enabled)
{
    m_bWebMessageEnabled = enabled;
    m_bWebMessageEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        return settings->put_IsWebMessageEnabled(enabled ? TRUE : FALSE);
    }
    return S_OK;
}

bool WebView2Control::Impl::IsWebMessageEnabled() const
{
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (SUCCEEDED(hr) && (settings != nullptr)) {
            BOOL bEnabled = TRUE;
            if (SUCCEEDED(settings->get_IsWebMessageEnabled(&bEnabled))) {
                return bEnabled ? true : false;
            }
        }
    }
    return m_bWebMessageEnabled;
}

HRESULT WebView2Control::Impl::SetAreDefaultScriptDialogsEnabled(bool enabled)
{
    m_bAreDefaultScriptDialogsEnabled = enabled;
    m_bAreDefaultScriptDialogsEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        return settings->put_AreDefaultScriptDialogsEnabled(enabled ? TRUE : FALSE);
    }
    return S_OK;
}

bool WebView2Control::Impl::AreDefaultScriptDialogsEnabled() const
{
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (SUCCEEDED(hr) && (settings != nullptr)) {
            BOOL bEnabled = TRUE;
            if (SUCCEEDED(settings->get_AreDefaultScriptDialogsEnabled(&bEnabled))) {
                return bEnabled ? true : false;
            }
        }
    }
    return m_bAreDefaultScriptDialogsEnabled;
}

HRESULT WebView2Control::Impl::SetAreDefaultContextMenusEnabled(bool enabled)
{
    m_bAreDefaultContextMenusEnabled = enabled;
    m_bAreDefaultContextMenusEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        return settings->put_AreDefaultContextMenusEnabled(enabled ? TRUE : FALSE);
    }
    return S_OK;
}

bool WebView2Control::Impl::AreDefaultContextMenusEnabled() const
{
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (SUCCEEDED(hr) && (settings != nullptr)) {
            BOOL bEnabled = TRUE;
            if (SUCCEEDED(settings->get_AreDefaultContextMenusEnabled(&bEnabled))) {
                return bEnabled ? true : false;
            }
        }
    }
    return m_bAreDefaultContextMenusEnabled;
}

HRESULT WebView2Control::Impl::SetZoomControlEnabled(bool enabled)
{
    m_bZoomControlEnabled = enabled;
    m_bZoomControlEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (FAILED(hr) || (settings == nullptr)) {
            return hr;
        }
        return settings->put_IsZoomControlEnabled(enabled ? TRUE : FALSE);
    }
    return S_OK;
}

bool WebView2Control::Impl::IsZoomControlEnabled() const
{
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = m_spWebView2->get_Settings(&settings);
        if (SUCCEEDED(hr) && (settings != nullptr)) {
            BOOL bEnabled = TRUE;
            if (SUCCEEDED(settings->get_IsZoomControlEnabled(&bEnabled))) {
                return bEnabled ? true : false;
            }
        }
    }
    return m_bZoomControlEnabled;
}

HRESULT WebView2Control::Impl::SetWebMessageReceivedCallback(WebMessageReceivedCallback callback)
{
    if ((m_webMessageReceivedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_WebMessageReceived(m_webMessageReceivedToken);
        m_webMessageReceivedToken.value = 0;
    }
    m_webMessageReceivedCallback = callback;
    if (callback && m_spWebView2) {
        HRESULT hr = m_spWebView2->add_WebMessageReceived(
            ui::ComCallback<ICoreWebView2WebMessageReceivedEventHandler,
                            IID_ICoreWebView2WebMessageReceivedEventHandler>(
                [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    LPWSTR uri = nullptr;
                    args->get_Source(&uri);
                    DString url;
                    if (uri != nullptr) {
                        url = StringConvert::WStringToT(std::wstring(uri));
                        ::CoTaskMemFree(uri);
                        uri = nullptr;
                    }

                    LPWSTR messageString = nullptr;
                    args->TryGetWebMessageAsString(&messageString);
                    DString webMessageAsString;
                    if (messageString != nullptr) {
                        webMessageAsString = StringConvert::WStringToT(std::wstring(messageString));
                        ::CoTaskMemFree(messageString);
                        messageString = nullptr;
                    }

                    LPWSTR messageJson = nullptr;
                    args->get_WebMessageAsJson(&messageJson);
                    DString webMessageAsJson;
                    if (messageJson != nullptr) {
                        webMessageAsJson = StringConvert::WStringToT(std::wstring(messageJson));
                        ::CoTaskMemFree(messageJson);
                        messageJson = nullptr;
                    }

                    if (m_webMessageReceivedCallback) {
                        m_webMessageReceivedCallback(url, webMessageAsJson, webMessageAsString);
                    }
                    return S_OK;
                }).Get(), &m_webMessageReceivedToken);
        ASSERT(SUCCEEDED(hr));
        return hr;
    }
    return S_OK;
}

HRESULT WebView2Control::Impl::SetNavigationStateChangedCallback(NavigationStateChangedCallback callback)
{
    m_navigationStateChangedCallback = callback;
    return S_OK;
}

void WebView2Control::Impl::AddNavigationStateChangedCallback()
{
    if (m_spWebView2 == nullptr) {
        return;
    }

    if (m_navigationStartingToken.value != 0) {
        m_spWebView2->remove_NavigationStarting(m_navigationStartingToken);
        m_navigationStartingToken.value = 0;
    }
    if (m_navigationCompletedToken.value != 0) {
        m_spWebView2->remove_NavigationCompleted(m_navigationCompletedToken);
        m_navigationCompletedToken.value = 0;
    }
    //注册导航开始事件
    HRESULT hr = m_spWebView2->add_NavigationStarting(
        ui::ComCallback<ICoreWebView2NavigationStartingEventHandler,
                        IID_ICoreWebView2NavigationStartingEventHandler>(
            [this](ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* /*args*/) -> HRESULT {
                ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                ASSERT(GlobalManager::Instance().IsInUIThread());

                m_bNavigating = true;
                if (m_navigationStateChangedCallback) {
                    m_navigationStateChangedCallback(NavigationState::Started, S_OK);
                }
                return S_OK;
            }).Get(), &m_navigationStartingToken);
    ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
    
    //注册导航完成事件
    hr = m_spWebView2->add_NavigationCompleted(
        ui::ComCallback<ICoreWebView2NavigationCompletedEventHandler,
                        IID_ICoreWebView2NavigationCompletedEventHandler>(
            [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                ASSERT(GlobalManager::Instance().IsInUIThread());

                BOOL isSuccess = TRUE;
                args->get_IsSuccess(&isSuccess);

                COREWEBVIEW2_WEB_ERROR_STATUS errorCode = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                args->get_WebErrorStatus(&errorCode);

                m_bNavigating = false;
                if (m_navigationStateChangedCallback) {
                    m_navigationStateChangedCallback(isSuccess ? NavigationState::Completed : NavigationState::Failed,
                                                     static_cast<HRESULT>(errorCode));
                }
                return S_OK;
            }).Get(), &m_navigationCompletedToken);
    ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
}

HRESULT WebView2Control::Impl::SetDocumentTitleChangedCallback(DocumentTitleChangedCallback callback)
{
    if ((m_documentTitleChangedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_DocumentTitleChanged(m_documentTitleChangedToken);
        m_documentTitleChangedToken.value = 0;
    }
    m_documentTitleChangedCallback = callback;
    if (callback && m_spWebView2) {
        HRESULT hr = m_spWebView2->add_DocumentTitleChanged(
            ui::ComCallback<ICoreWebView2DocumentTitleChangedEventHandler,
                            IID_ICoreWebView2DocumentTitleChangedEventHandler>(
                [this](ICoreWebView2* sender, IUnknown* /*args*/) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    LPWSTR title = nullptr;
                    sender->get_DocumentTitle(&title);
                    DString titleStr;
                    if (title != nullptr) {
                        titleStr = StringConvert::WStringToT(std::wstring(title));
                        ::CoTaskMemFree(title);
                        title = nullptr;
                    }
                    if (m_documentTitleChangedCallback) {
                        m_documentTitleChangedCallback(titleStr);
                    }
                    return S_OK;
                }).Get(), &m_documentTitleChangedToken);
        ASSERT(SUCCEEDED(hr));
        return hr;
    }
    return S_OK;
}

HRESULT WebView2Control::Impl::SetSourceChangedCallback(SourceChangedCallback callback)
{
    if ((m_sourceChangedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_SourceChanged(m_sourceChangedToken);
        m_sourceChangedToken.value = 0;
    }
    m_sourceChangedCallback = callback;
    if (callback && m_spWebView2) {
        HRESULT hr = m_spWebView2->add_SourceChanged(
            ui::ComCallback<ICoreWebView2SourceChangedEventHandler,
                            IID_ICoreWebView2SourceChangedEventHandler>(
                [this](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs* /*args*/) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    LPWSTR uri = nullptr;
                    sender->get_Source(&uri);
                    DString url;
                    if (uri != nullptr) {
                        url = StringConvert::WStringToT(std::wstring(uri));
                        ::CoTaskMemFree(uri);
                        uri = nullptr;
                    }
                    if (m_sourceChangedCallback) {
                        m_sourceChangedCallback(url);
                    }
                    return S_OK;
                }).Get(), &m_sourceChangedToken);
        ASSERT(SUCCEEDED(hr));
        return hr;
    }
    return S_OK;
}

void WebView2Control::Impl::AddNewWindowRequestedCallback()
{
    if ((m_newWindowRequestedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_NewWindowRequested(m_newWindowRequestedToken);
        m_newWindowRequestedToken.value = 0;
    }
    if (m_spWebView2 != nullptr) {
        HRESULT hr = m_spWebView2->add_NewWindowRequested(
            ui::ComCallback<ICoreWebView2NewWindowRequestedEventHandler,
                            IID_ICoreWebView2NewWindowRequestedEventHandler>(
                [this](ICoreWebView2* sender, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    LPWSTR uri = nullptr;
                    args->get_Uri(&uri);
                    DString targetUrl;
                    if (uri != nullptr) {
                        targetUrl = StringConvert::WStringToT(std::wstring(uri));
                        ::CoTaskMemFree(uri);
                        uri = nullptr;
                    }

                    bool bUserInitiated = false;
                    BOOL isUserInitiated = FALSE;
                    if (SUCCEEDED(args->get_IsUserInitiated(&isUserInitiated))) {
                        bUserInitiated = isUserInitiated ? true : false;
                    }

                    ui::ComPtr<ICoreWebView2NewWindowRequestedEventArgs2> args2;
                    ui::ComPtr<ICoreWebView2NewWindowRequestedEventArgs3> args3;
                    args->QueryInterface(IID_ICoreWebView2NewWindowRequestedEventArgs2, (void**)&args2);
                    args->QueryInterface(IID_ICoreWebView2NewWindowRequestedEventArgs3, (void**)&args3);

                    DString sourceFrameName;
                    if (args2 != nullptr) {                        
                        LPWSTR name = nullptr;
                        args2->get_Name(&name);
                        if (name != nullptr) {
                            sourceFrameName = StringConvert::WStringToT(std::wstring(name));
                            ::CoTaskMemFree(name);
                            name = nullptr;
                        }
                    }

                    DString sourceUrl;
                    DString targetFrameName;
                    if (args3 != nullptr) {
                        ui::ComPtr<ICoreWebView2FrameInfo> frameInfo;
                        args3->get_OriginalSourceFrameInfo(&frameInfo);
                        if (frameInfo != nullptr) {                            
                            LPWSTR name = nullptr;
                            frameInfo->get_Name(&name);
                            if (name != nullptr) {
                                targetFrameName = StringConvert::WStringToT(std::wstring(name));
                                ::CoTaskMemFree(name);
                                name = nullptr;
                            }
                            LPWSTR source;
                            frameInfo->get_Source(&source);
                            if (source != nullptr) {
                                sourceUrl = StringConvert::WStringToT(std::wstring(source));
                                ::CoTaskMemFree(source);
                                source = nullptr;
                            }
                        }
                    }

                    // 旧版API没有TargetFrameName，使用空字符串替代
                    bool bAllow = false;
                    if (m_newWindowRequestedCallback) {
                        bAllow = m_newWindowRequestedCallback(sourceUrl, sourceFrameName,
                                                              targetUrl, targetFrameName, bUserInitiated);
                    }
                    else {
                        if (bUserInitiated) {
                            bAllow = true;
                        }
                    }

                    if (bAllow) {
                        //覆盖当前页面(暂时不支持弹出新窗口或者创建新标签)
                        args->put_NewWindow(m_spWebView2.Get());
                    }

                    // 默认取消新窗口，由应用程序处理
                    args->put_Handled(!bAllow);
                    return S_OK;
                }).Get(), &m_newWindowRequestedToken);
        ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
    }
}

HRESULT WebView2Control::Impl::SetNewWindowRequestedCallback(NewWindowRequestedCallback callback)
{    
    m_newWindowRequestedCallback = callback;
    return S_OK;
}

HRESULT WebView2Control::Impl::SetHistoryChangedCallback(HistoryChangedCallback callback)
{
    if ((m_historyChangedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_HistoryChanged(m_historyChangedToken);
        m_historyChangedToken.value = 0;
    }
    m_historyChangedCallback = callback;    
    // 如果已经初始化且有回调，则注册事件
    if (callback && m_spWebView2) {
        HRESULT hr = m_spWebView2->add_HistoryChanged(
            ui::ComCallback<ICoreWebView2HistoryChangedEventHandler,
                            IID_ICoreWebView2HistoryChangedEventHandler>(
                [this](ICoreWebView2* sender, IUnknown* /*args*/) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    if (m_historyChangedCallback) {
                        m_historyChangedCallback();
                    }
                    return S_OK;
                }).Get(), &m_historyChangedToken);
        ASSERT(SUCCEEDED(hr));
        return hr;
    }
    return S_OK;
}

HRESULT WebView2Control::Impl::SetZoomFactorChangedCallback(ZoomFactorChangedCallback callback)
{
    if ((m_zoomFactorChangedToken.value != 0) && (m_spWebView2 != nullptr)) {
        m_spWebView2->remove_HistoryChanged(m_zoomFactorChangedToken);
        m_zoomFactorChangedToken.value = 0;
    }
    m_zoomFactorChangedCallback = callback;
    // 如果已经初始化且有回调，则注册事件
    if (callback && m_spWebView2Controller) {
        HRESULT hr = m_spWebView2Controller->add_ZoomFactorChanged(
            ui::ComCallback<ICoreWebView2ZoomFactorChangedEventHandler,
                            IID_ICoreWebView2ZoomFactorChangedEventHandler>(
                [this](ICoreWebView2Controller* sender, IUnknown* /*args*/) -> HRESULT {
                    ASSERT_UNUSED_VARIABLE(sender == m_spWebView2Controller.Get());
                    ASSERT(GlobalManager::Instance().IsInUIThread());

                    if (m_zoomFactorChangedCallback && m_spWebView2Controller) {
                        double zoomFactor = 1.0;
                        m_spWebView2Controller->get_ZoomFactor(&zoomFactor);
                        m_zoomFactorChangedCallback(zoomFactor);
                    }
                    return S_OK;
                }).Get(), &m_zoomFactorChangedToken);
        ASSERT(SUCCEEDED(hr));
        return hr;
    }
    return S_OK;
}

HRESULT WebView2Control::Impl::CapturePreview(const DString& filePath,
                                              std::function<void(const DString& filePath, HRESULT hr)> callback)
{
    if (m_spWebView2 == nullptr) {
        return E_FAIL;
    }
    ASSERT(!filePath.empty());
    if (filePath.empty()) {
        return E_FAIL;
    }
#ifdef DUILIB_UNICODE
    DString filePathW = filePath;
#else
    DStringW filePathW = StringConvert::MBCSToUnicode(filePath);
#endif
    // 创建文件流
    ui::ComPtr<IStream> stream;
    HRESULT hr = ::SHCreateStreamOnFileEx(filePathW.c_str(),
                                          STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
                                          FILE_ATTRIBUTE_NORMAL,
                                          TRUE,
                                          nullptr,
                                          &stream);
    
    if (FAILED(hr)) {
        return hr;
    }
    //根据保存的文件名后缀确定图片格式
    COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT imageFormat;
    DString ext = FilePath(filePathW).GetFileExtension();
    if (StringUtil::IsEqualNoCase(ext, _T(".png"))) {
        imageFormat = COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG;
    }
    else {
        imageFormat = COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_JPEG;
    }
    return m_spWebView2->CapturePreview(imageFormat,
                                       stream.Get(),
                                       ui::ComCallback<ICoreWebView2CapturePreviewCompletedHandler,
                                                       IID_ICoreWebView2CapturePreviewCompletedHandler>(
                                            [filePath, callback](HRESULT errorCode) -> HRESULT {
                                                if (callback) {
                                                    callback(filePath, errorCode);
                                                }
                                                return S_OK;
                                            }).Get());
}

bool WebView2Control::Impl::IsInitializing() const
{
    return m_bInitializing;
}

bool WebView2Control::Impl::IsInitialized() const
{
    return m_bInitialized;
}

DString WebView2Control::Impl::GetUrl() const
{
    if (m_spWebView2 == nullptr) {
        return _T("");
    }
    DString retUrl;
    LPWSTR url = nullptr;
    m_spWebView2->get_Source(&url);
    if (url != nullptr) {
        retUrl = StringConvert::WStringToT(url);
        ::CoTaskMemFree(url);
        url = nullptr;
    }
    return retUrl;
}

DString WebView2Control::Impl::GetTitle() const
{
    if (m_spWebView2 == nullptr) {
        return _T("");
    }
    DString retTitle;
    LPWSTR title = nullptr;
    m_spWebView2->get_DocumentTitle(&title);
    if (title != nullptr) {
        retTitle = StringConvert::WStringToT(title);
        ::CoTaskMemFree(title);
        title = nullptr;
    }
    return retTitle;
}

bool WebView2Control::Impl::IsNavigating() const
{
    return m_bNavigating;
}

bool WebView2Control::Impl::CanGoBack() const
{
    if (m_spWebView2 == nullptr) {
        return false;
    }
    BOOL canGoBack = FALSE;
    m_spWebView2->get_CanGoBack(&canGoBack);
    return !!canGoBack;
}

bool WebView2Control::Impl::CanGoForward() const
{
    if (m_spWebView2 == nullptr) {
        return false;
    }
    BOOL canGoForward = FALSE;
    m_spWebView2->get_CanGoForward(&canGoForward);
    return !!canGoForward;
}

void WebView2Control::Impl::Resize(RECT rect)
{
    if (m_spWebView2Controller) {
        m_spWebView2Controller->put_Bounds(rect);
    }
}

void WebView2Control::Impl::Cleanup()
{
    // 取消注册事件
    if (m_spWebView2 != nullptr) {
        if (m_webMessageReceivedToken.value) {
            m_spWebView2->remove_WebMessageReceived(m_webMessageReceivedToken);
            m_webMessageReceivedToken.value = 0;
        }
            
        if (m_documentTitleChangedToken.value) {
            m_spWebView2->remove_DocumentTitleChanged(m_documentTitleChangedToken);
            m_documentTitleChangedToken.value = 0;
        }

        if (m_navigationStartingToken.value != 0) {
            m_spWebView2->remove_NavigationStarting(m_navigationStartingToken);
            m_navigationStartingToken.value = 0;
        }
            
        if (m_navigationCompletedToken.value) {
            m_spWebView2->remove_NavigationCompleted(m_navigationCompletedToken);
            m_navigationCompletedToken.value = 0;
        }
            
        if (m_sourceChangedToken.value) {
            m_spWebView2->remove_SourceChanged(m_sourceChangedToken);
            m_sourceChangedToken.value = 0;
        }
            
        if (m_newWindowRequestedToken.value) {
            m_spWebView2->remove_NewWindowRequested(m_newWindowRequestedToken);
            m_newWindowRequestedToken.value = 0;
        }

        if (m_historyChangedToken.value) {
            m_spWebView2->remove_HistoryChanged(m_historyChangedToken);
            m_historyChangedToken.value = 0;
        }

        if (m_faviconChangedToken.value != 0) {
            ui::ComPtr<ICoreWebView2_15> spWebView2_15;
            HRESULT hr = m_spWebView2->QueryInterface(IID_ICoreWebView2_15, (void**)&spWebView2_15);
            if (SUCCEEDED(hr) && (spWebView2_15 != nullptr)) {
                spWebView2_15->remove_FaviconChanged(m_faviconChangedToken);
            }
            m_faviconChangedToken.value = 0;
        }
    }

    if (m_spWebView2Controller != nullptr) {
        if (m_zoomFactorChangedToken.value) {
            m_spWebView2Controller->remove_ZoomFactorChanged(m_zoomFactorChangedToken);
            m_zoomFactorChangedToken.value = 0;
        }

        if (m_gotFocusToken.value) {
            m_spWebView2Controller->remove_GotFocus(m_gotFocusToken);
            m_gotFocusToken.value = 0;
        }
    }

    if (m_spWebView2Controller != nullptr) {
        m_spWebView2Controller->Close();
    }

    // 释放COM对象
    m_spWebView2 = nullptr;
    m_spWebView2Controller = nullptr;
    m_spWebView2Environment = nullptr;
    m_bInitialized = false;
    m_bInitializing = false;
    m_bNavigating = false;
    m_lastError = S_OK;

#ifdef DUILIB_USE_WEBVIEW2_LOADER_DLL
    if (m_hWebView2Loader != nullptr) {
        ::FreeLibrary(m_hWebView2Loader);
        m_hWebView2Loader = nullptr;
    }
#endif
}

void WebView2Control::Impl::SetLastErrorCode(HRESULT hr)
{
    m_lastError = hr;
}

HRESULT WebView2Control::Impl::GetLastErrorCode() const
{
    return m_lastError;
}

void WebView2Control::Impl::SetWindow(Window* pWindow)
{
    if (m_spWebView2Controller != nullptr) {
        ASSERT(pWindow != nullptr);
        if (pWindow == nullptr) {
            return;
        }
        ASSERT(pWindow->NativeWnd()->GetHWND() != nullptr);
        if (pWindow->NativeWnd()->GetHWND() == nullptr) {
            return;
        }
        ASSERT(::IsWindow(pWindow->NativeWnd()->GetHWND()));    
        m_spWebView2Controller->put_ParentWindow(pWindow->NativeWnd()->GetHWND());
    }
}

void WebView2Control::Impl::SetVisible(bool bVisible)
{
    if (m_spWebView2Controller != nullptr) {
        m_spWebView2Controller->put_IsVisible(bVisible ? TRUE : FALSE);
    }
}

void WebView2Control::Impl::SetAreDevToolsEnabled(bool bAreDevToolsEnabled)
{
    m_bAreDevToolsEnabled = bAreDevToolsEnabled;
    m_bAreDevToolsEnabledSet = true;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        if (SUCCEEDED(m_spWebView2->get_Settings(&settings)) && (settings != nullptr)) {
            HRESULT hr = settings->put_AreDevToolsEnabled(bAreDevToolsEnabled ? TRUE : FALSE);
            ASSERT_UNUSED_VARIABLE(SUCCEEDED(hr));
        }
    }
}

bool WebView2Control::Impl::AreDevToolsEnabled() const
{
    bool bDevToolsEnabled = false;
    if (m_spWebView2 != nullptr) {
        ui::ComPtr<ICoreWebView2Settings> settings;
        if (SUCCEEDED(m_spWebView2->get_Settings(&settings)) && (settings != nullptr)) {
            BOOL areDevToolsEnabled = FALSE;
            if (SUCCEEDED(settings->get_AreDevToolsEnabled(&areDevToolsEnabled)) && areDevToolsEnabled) {
                bDevToolsEnabled = true;
            }
        }
    }
    return bDevToolsEnabled;
}

bool WebView2Control::Impl::OpenDevToolsWindow()
{
    if (!AreDevToolsEnabled()) {
        return false;
    }
    HRESULT hr = E_FAIL;
    if (m_spWebView2 != nullptr) {
        hr = m_spWebView2->OpenDevToolsWindow();
    }
    return SUCCEEDED(hr);
}

void WebView2Control::Impl::SetFavIconChangedCallback(FavIconChangedCallback callback)
{
    m_favIconChangedCallback = callback;
    if ((m_faviconChangedToken.value != 0) && (m_spWebView2 != nullptr)) {
        ui::ComPtr<ICoreWebView2_15> spWebView2_15;
        HRESULT hr = m_spWebView2->QueryInterface(IID_ICoreWebView2_15, (void**)&spWebView2_15);
        if (SUCCEEDED(hr) && (spWebView2_15 != nullptr)) {
            spWebView2_15->remove_FaviconChanged(m_faviconChangedToken);
        }
        m_faviconChangedToken.value = 0;
    }
    if ((m_favIconChangedCallback != nullptr) && (m_spWebView2 != nullptr)) {
        ui::ComPtr<ICoreWebView2_15> spWebView2_15;
        HRESULT hr = m_spWebView2->QueryInterface(IID_ICoreWebView2_15, (void**)&spWebView2_15);
        if (SUCCEEDED(hr) && (spWebView2_15 != nullptr)) {
            hr = spWebView2_15->add_FaviconChanged(
                ui::ComCallback<ICoreWebView2FaviconChangedEventHandler,
                                IID_ICoreWebView2FaviconChangedEventHandler>(
                    [this](ICoreWebView2* sender, IUnknown* /*args*/) -> HRESULT
                    {
                        ASSERT_UNUSED_VARIABLE(sender == m_spWebView2.Get());
                        ASSERT(GlobalManager::Instance().IsInUIThread());
                        
                        ui::ComPtr<ICoreWebView2_15> webview2;
                        HRESULT hr = sender->QueryInterface(IID_ICoreWebView2_15, (void**)&webview2);
                        ASSERT(SUCCEEDED(hr));

                        if (SUCCEEDED(hr) && (webview2 != nullptr)) {
                            LPWSTR url = nullptr;
                            hr = webview2->get_FaviconUri(&url);
                            ASSERT(SUCCEEDED(hr));
                            std::wstring strUrl;
                            if (url != nullptr) {
                                strUrl = url;
                                ::CoTaskMemFree(url);
                                url = nullptr;
                            }
                            m_favIconImageUrl = strUrl;
                            DownloadFavIconImage();                            
                        }
                        return S_OK;
                    }).Get(), &m_faviconChangedToken);
                ASSERT(SUCCEEDED(hr));
        }
    }
}

static bool ConvertFavIconImageData(std::vector<uint8_t>& pngImageData, uint32_t nWindowDpi, const DString& fileName,
                                    int32_t& nWidth, int32_t& nHeight, std::vector<uint8_t>& imageData)
{
    ui::ImageLoadAttribute imageLoadAttribute(_T(""), _T(""), false, false, 32);
    imageLoadAttribute.SetImageFullPath(fileName);
    uint32_t nFrameCount = 0;
    ui::ImageDecoder decoder;
    auto imageInfo = decoder.LoadImageData(pngImageData, imageLoadAttribute, true, 100, nWindowDpi, true, nFrameCount);
    if (imageInfo == nullptr) {
        return false;
    }
    ui::IBitmap* pBitmap = imageInfo->GetBitmap(0);
    if (pBitmap == nullptr) {
        return false;
    }
    nWidth = pBitmap->GetWidth();
    nHeight = pBitmap->GetHeight();
    if ((nWidth < 1) || (nHeight < 1)) {
        return false;
    }
    void* pBits = pBitmap->LockPixelBits();
    if (pBits == nullptr) {
        return false;
    }
    size_t nDataSize = nHeight * nWidth * 4;
    imageData.resize(nDataSize);
    memcpy(imageData.data(), pBits, nDataSize);
    pBitmap->UnLockPixelBits();
    pBits = 0;
    return true;
}

//下载网站图标
static bool DownloadFaviconToVector(const wchar_t* url, std::vector<uint8_t>& outData)
{
    HINTERNET hInternet = ::InternetOpenW(L"FaviconDL", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        return false;
    }

    HINTERNET hUrl = ::InternetOpenUrlW(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hUrl) {
        ::InternetCloseHandle(hInternet);
        return false;
    }

    outData.clear();
    uint8_t buffer[4096];
    DWORD bytesRead = 0;

    while (::InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        outData.insert(outData.end(), buffer, buffer + bytesRead);
    }

    ::InternetCloseHandle(hUrl);
    ::InternetCloseHandle(hInternet);
    return !outData.empty();
}

bool WebView2Control::Impl::DownloadFavIconImage()
{
    ASSERT(GlobalManager::Instance().IsInUIThread());
    std::wstring strUrl = m_favIconImageUrl;
    if (strUrl.empty()) {
        return false;
    }
    if ((m_pControl == nullptr) || (m_favIconChangedCallback == nullptr)) {
        return false;
    }

    int32_t nThreadIdentifier = ui::kThreadUI;
    if (GlobalManager::Instance().Thread().HasThread(ui::kThreadWorker)) {
        nThreadIdentifier = ui::kThreadWorker;
    }
    else if (GlobalManager::Instance().Thread().HasThread(ui::kThreadMisc)) {
        nThreadIdentifier = ui::kThreadMisc;
    }
    GlobalManager::Instance().Thread().PostTask(nThreadIdentifier, m_pControl->ToWeakCallback([this, strUrl]() {
            //转到子线程中，下载图标
            std::vector<uint8_t> iconData;
            if (DownloadFaviconToVector(strUrl.c_str(), iconData)) {
                uint32_t nWindowDpi = 100;
                if (m_pControl != nullptr) {
                    nWindowDpi = m_pControl->Dpi().GetScale();
                }
                DString fileName = StringConvert::WStringToT(strUrl);
                size_t pos = fileName.rfind(_T("/"));
                if (pos != DString::npos) {
                    fileName = fileName.substr(pos + 1);
                }

                int32_t nWidth = 0;
                int32_t nHeight = 0;
                std::vector<uint8_t> imageData;
                if (ConvertFavIconImageData(iconData, nWindowDpi, fileName, nWidth, nHeight, imageData)) {
                    if ((m_pControl != nullptr) && (m_favIconChangedCallback != nullptr)) {
                        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
                                                                    m_pControl->ToWeakCallback([this, nWidth, nHeight, imageData]() {
                                //转到UI线程执行回调函数
                                if (m_favIconChangedCallback) {
                                    m_favIconChangedCallback(nWidth, nHeight, imageData);
                                }
                            }));
                    }                    
                }
            }
        }));
    return true;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2
