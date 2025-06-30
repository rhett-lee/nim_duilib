#ifndef UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_
#define UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_

#include "WebView2Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include <combaseapi.h>
#include <wrl.h>
#include "duilib/third_party/Microsoft.Web.WebView2/build/native/include/WebView2.h"
#include "duilib/third_party/Microsoft.Windows.ImplementationLibrary/include/wil/com.h"

#include <atomic>

namespace ui {

class Control;

/** WebView2Control类的实现类（PIMPL模式）
    包含WebView2封装类的具体实现细节，采用纯异步初始化方式
 */
class WebView2Control::Impl
{
public:
    explicit Impl(Control* pControl);
    ~Impl();

    // 初始化和生命周期管理
    HRESULT InitializeAsync(const DString& userDataFolder, InitializeCompletedCallback callback);
    void Cleanup();

    // 导航控制
    HRESULT Navigate(const DString& url);
    HRESULT NavigateBack();
    HRESULT NavigateForward();
    HRESULT Refresh();
    HRESULT Stop();

    // JavaScript交互
    HRESULT ExecuteScript(const DString& script, std::function<void(const DString& result, HRESULT hr)> callback);
    HRESULT PostWebMessageAsJson(const DString& json);
    HRESULT PostWebMessageAsString(const DString& message);

    // 设置相关
    HRESULT SetUserAgent(const DString& userAgent);
    DString GetUserAgent() const;

    HRESULT SetZoomFactor(double zoomFactor);
    double GetZoomFactor() const;

    /** 设置JavaScript是否启用
    */
    HRESULT SetScriptEnabled(bool enabled);
    bool IsScriptEnabled() const;

    /** 设置Web消息是否启用
    */
    HRESULT SetWebMessageEnabled(bool enabled);
    bool IsWebMessageEnabled() const;

    /** 是否启用默认脚本对话框
    */
    HRESULT SetAreDefaultScriptDialogsEnabled(bool enabled);
    bool AreDefaultScriptDialogsEnabled() const;

    /** 是否允许默认的右键菜单
    */
    HRESULT SetAreDefaultContextMenusEnabled(bool enabled);
    bool AreDefaultContextMenusEnabled() const;

    /** 是否禁用缩放控制
    */
    HRESULT SetZoomControlEnabled(bool enabled);
    bool IsZoomControlEnabled() const;

    // 事件注册
    HRESULT SetWebMessageReceivedCallback(WebMessageReceivedCallback callback);
    HRESULT SetNavigationStateChangedCallback(NavigationStateChangedCallback callback);
    HRESULT SetDocumentTitleChangedCallback(DocumentTitleChangedCallback callback);
    HRESULT SetSourceChangedCallback(SourceChangedCallback callback);
    HRESULT SetContentLoadingCallback(ContentLoadingCallback callback);
    HRESULT SetNewWindowRequestedCallback(NewWindowRequestedCallback callback);
    HRESULT SetHistoryChangedCallback(HistoryChangedCallback callback);
    HRESULT SetZoomFactorChangedCallback(ZoomFactorChangedCallback callback);

    // 内容管理
    HRESULT AddScriptToExecuteOnDocumentCreated(const DString& script);
    HRESULT CapturePreview(const DString& filePath,
                           std::function<void(const DString& filePath, HRESULT hr)> callback);

    // 访问器
    bool IsInitializing() const;
    bool IsInitialized() const;
    DString GetUrl() const;
    DString GetTitle() const;
    bool IsNavigating() const;
    bool CanGoBack() const;
    bool CanGoForward() const;

    // 窗口管理
    void Resize(RECT rect);

    //设置最后一次操作的错误码
    void SetLastErrorCode(HRESULT hr);

    //获取最后一次操作的错误码
    HRESULT GetLastErrorCode() const;

    /** 设置关联窗口
    */
    void SetWindow(Window* pWindow);

    /** 设置显示属性
    */
    void SetVisible(bool bVisible);

    /** 设置是否启用开发者工具
    */
    void SetAreDevToolsEnabled(bool bAreDevToolsEnabled);

    /** 获取是否启用开发者工具
    */
    bool AreDevToolsEnabled() const;

    /** 打开开发者工具
    */
    bool OpenDevToolsWindow();

private:

    //初始化流程
    HRESULT CreateEnvironmentAsync();
    HRESULT CreateControllerAsync();
    void InitializeSettings();
    void OnInitializationCompleted(HRESULT result);

    //添加新窗口回调函数
    void AddNewWindowRequestedCallback();

    //添加导航状态变化事件
    void AddNavigationStateChangedCallback();

private:
    // COM对象
    wil::com_ptr<ICoreWebView2Environment> m_spWebViewEnvironment;
    wil::com_ptr<ICoreWebView2Controller> m_spWebViewController;
    wil::com_ptr<ICoreWebView2> m_spWebView;

    // 事件令牌
    EventRegistrationToken m_webMessageReceivedToken = {0};
    EventRegistrationToken m_documentTitleChangedToken = {0};
    EventRegistrationToken m_contentLoadingToken = {0};
    EventRegistrationToken m_navigationStartingToken = { 0 };
    EventRegistrationToken m_navigationCompletedToken = {0};
    EventRegistrationToken m_sourceChangedToken = {0};
    EventRegistrationToken m_newWindowRequestedToken = {0};
    EventRegistrationToken m_historyChangedToken = {0};
    EventRegistrationToken m_zoomFactorChangedToken = { 0 };

    // 回调函数
    InitializeCompletedCallback m_initializeCompletedCallback = nullptr;
    WebMessageReceivedCallback m_webMessageReceivedCallback = nullptr;
    NavigationStateChangedCallback m_navigationStateChangedCallback = nullptr;
    DocumentTitleChangedCallback m_documentTitleChangedCallback = nullptr;
    SourceChangedCallback m_sourceChangedCallback = nullptr;
    ContentLoadingCallback m_contentLoadingCallback = nullptr;
    NewWindowRequestedCallback m_newWindowRequestedCallback = nullptr;
    HistoryChangedCallback m_historyChangedCallback = nullptr;
    ZoomFactorChangedCallback m_zoomFactorChangedCallback = nullptr;

    //初始化URL
    DString m_navigateUrl;

    // 状态变量
    Control* m_pControl;
    DString m_userDataFolder;
    DString m_userAgent;

    //文档完成时执行的脚本
    DString m_scriptToExecuteOnDocumentCreated;
    std::wstring m_scriptIdToExecuteOnDocumentCreated;

    HRESULT m_lastError;
    bool m_bInitializing;
    bool m_bInitialized;
    bool m_bNavigating;

    //是否启用开发者工具
    bool m_bAreDevToolsEnabled;
    bool m_bAreDevToolsEnabledSet;

    //是否启用右键菜单
    bool m_bAreDefaultContextMenusEnabled;
    bool m_bAreDefaultContextMenusEnabledSet;

    //是否允许JS对话框
    bool m_bAreDefaultScriptDialogsEnabled;
    bool m_bAreDefaultScriptDialogsEnabledSet;

    //是否允许网页消息
    bool m_bWebMessageEnabled;
    bool m_bWebMessageEnabledSet;

    //是否允许JS
    bool m_bScriptEnabled;
    bool m_bScriptEnabledSet;

    //是否允许页面缩放
    bool m_bZoomControlEnabled;
    bool m_bZoomControlEnabledSet;
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif //UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_
