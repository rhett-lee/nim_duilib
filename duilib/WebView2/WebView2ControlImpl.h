#ifndef UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_
#define UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_

#include "WebView2Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#ifdef DUILIB_COMPILER_MINGW
    //使用"WebView2Loader.dll"
    #define DUILIB_USE_WEBVIEW2_LOADER_DLL 1
#endif

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
    /** 设置 User-Agent 字符串（为空时使用 WebView2 的默认值）
    * @param [in] userAgent 要设置的 User-Agent
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetUserAgent(const DString& userAgent);

    /** 获取当前生效的 User-Agent
    * @return 返回 User-Agent 字符串
    */
    DString GetUserAgent() const;

    /** 设置页面缩放比例
    * @param [in] zoomFactor 缩放比例（1.0 表示 100%）
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetZoomFactor(double zoomFactor);

    /** 获取页面缩放比例
    * @return 返回缩放比例（1.0 表示 100%）
    */
    double GetZoomFactor() const;

    /** 设置JavaScript是否启用
    * @param [in] enabled true 表示启用，false 表示禁用
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetScriptEnabled(bool enabled);

    /** 是否启用JavaScript
    * @return 返回 true 表示启用
    */
    bool IsScriptEnabled() const;

    /** 设置Web消息是否启用
    * @param [in] enabled true 表示启用，false 表示禁用
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetWebMessageEnabled(bool enabled);

    /** 是否启用Web消息
    * @return 返回 true 表示启用
    */
    bool IsWebMessageEnabled() const;

    /** 是否启用默认脚本对话框
    * @param [in] enabled true 表示启用，false 表示禁用
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetAreDefaultScriptDialogsEnabled(bool enabled);

    /** 是否启用默认脚本对话框
    * @return 返回 true 表示启用
    */
    bool AreDefaultScriptDialogsEnabled() const;

    /** 是否允许默认的右键菜单
    * @param [in] enabled true 表示允许，false 表示禁用
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetAreDefaultContextMenusEnabled(bool enabled);

    /** 是否允许默认的右键菜单
    * @return 返回 true 表示允许
    */
    bool AreDefaultContextMenusEnabled() const;

    /** 是否禁用缩放控制
    * @param [in] enabled true 表示启用缩放控制，false 表示禁用
    * @return 返回 S_OK 表示成功，否则为错误码
    */
    HRESULT SetZoomControlEnabled(bool enabled);

    /** 是否启用缩放控制
    * @return 返回 true 表示启用
    */
    bool IsZoomControlEnabled() const;

    // 事件注册
    HRESULT SetWebMessageReceivedCallback(WebMessageReceivedCallback callback);
    HRESULT SetNavigationStateChangedCallback(NavigationStateChangedCallback callback);
    HRESULT SetDocumentTitleChangedCallback(DocumentTitleChangedCallback callback);
    HRESULT SetSourceChangedCallback(SourceChangedCallback callback);
    HRESULT SetNewWindowRequestedCallback(NewWindowRequestedCallback callback);
    HRESULT SetHistoryChangedCallback(HistoryChangedCallback callback);
    HRESULT SetZoomFactorChangedCallback(ZoomFactorChangedCallback callback);
    void SetFavIconChangedCallback(FavIconChangedCallback callback);
    void SetAcceleratorKeyPressedCallback(AcceleratorKeyPressedCallback callback);

    // 内容管理
    /** 捕获当前网页内容为图片并保存到文件
    * @param [in] filePath 图片保存路径
    * @param [in] callback 捕获完成后的回调函数，参数为保存路径与执行结果
    * @return 返回 S_OK 表示成功发起捕获请求，否则为错误码
    */
    HRESULT CapturePreview(const DString& filePath,
                           std::function<void(const DString& filePath, HRESULT hr)> callback);

    // 访问器
    /** 是否正在初始化
    * @return 返回 true 表示正在初始化
    */
    bool IsInitializing() const;

    /** 是否已经初始化完成
    * @return 返回 true 表示已初始化完成
    */
    bool IsInitialized() const;

    /** 获取当前页面地址
    * @return 返回当前页面的 URL
    */
    DString GetUrl() const;

    /** 获取当前页面标题
    * @return 返回当前页面的标题
    */
    DString GetTitle() const;

    /** 是否正在导航（加载页面）
    * @return 返回 true 表示正在导航
    */
    bool IsNavigating() const;

    /** 是否可以后退
    * @return 返回 true 表示可以后退
    */
    bool CanGoBack() const;

    /** 是否可以前进
    * @return 返回 true 表示可以前进
    */
    bool CanGoForward() const;

    // 窗口管理
    /** 调整 WebView2 控件的显示区域
    * @param [in] rect 新的显示区域（控件坐标）
    */
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

    /** 下载网站图标
    */
    bool DownloadFavIconImage();

    /** 设置是否允许F12快捷键(开发者工具)
    */
    void SetEnableF12(bool bEnableF12);

    /** 是否允许F12快捷键(开发者工具)
    */
    bool IsEnableF12() const;

    /** 设置是否允许F11快捷键(页面全屏/页面退出全屏)
    */
    void SetEnableF11(bool bEnableF11);

    /** 是否允许F11快捷键(页面全屏/页面退出全屏)
    */
    bool IsEnableF11() const;

public:
    /** 获取ICoreWebView2Environment接口
    */
    ui::ComPtr<ICoreWebView2Environment> GetWebView2Environment() const { return m_spWebView2Environment; }

    /** 获取ICoreWebView2Controller接口
    */
    ui::ComPtr<ICoreWebView2Controller> GetWebView2Controller() const { return m_spWebView2Controller; }

    /** 获取ICoreWebView2接口
    */
    ui::ComPtr<ICoreWebView2> GetWebView2() const { return m_spWebView2; }

private:

    //初始化流程
    /** 创建 WebView2 运行环境（封装 ICoreWebView2Environment 的创建接口）
    * @param [in] browserExecutableFolder WebView2 运行时所在目录，nullptr 表示使用已安装的版本
    * @param [in] userDataFolder 用户数据目录
    * @param [in] environmentOptions 运行环境的选项
    * @param [in] environmentCreatedHandler 环境创建完成后的回调处理器
    * @return 返回 S_OK 表示成功发起创建请求，否则为错误码
    */
    HRESULT CallCreateCoreWebView2EnvironmentWithOptions(PCWSTR browserExecutableFolder,
                                                         PCWSTR userDataFolder,
                                                         ICoreWebView2EnvironmentOptions* environmentOptions,
                                                         ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler);

    /** 创建 WebView2 运行环境的选项对象（按当前控件的配置填充）
    * @return 返回运行环境的选项对象
    */
    ui::ComPtr<ICoreWebView2EnvironmentOptions> CreateCoreWebView2EnvironmentOptionsObj();

    /** 异步创建 WebView2 运行环境
    * @return 返回 S_OK 表示成功发起创建请求，否则为错误码
    */
    HRESULT CreateEnvironmentAsync();

    /** 异步创建 WebView2 控制器
    * @return 返回 S_OK 表示成功发起创建请求，否则为错误码
    */
    HRESULT CreateControllerAsync();

    /** 初始化 WebView2 的各项设置（JS、右键菜单、缩放、开发者工具等）
    */
    void InitializeSettings();

    /** 初始化焦点变化事件
    */
    void InitializeFocusChanged();//焦点变化事件
    void InitializeAcceleratorKeyPressed();//F11/F12/ESC快捷键的支持

    /** 初始化完成后的统一处理
    * @param [in] result 初始化的执行结果
    */
    void OnInitializationCompleted(HRESULT result);

    //添加新窗口回调函数
    void AddNewWindowRequestedCallback();

    //添加导航状态变化事件
    void AddNavigationStateChangedCallback();

private:
    // COM对象
    ui::ComPtr<ICoreWebView2Environment> m_spWebView2Environment;
    ui::ComPtr<ICoreWebView2Controller> m_spWebView2Controller;
    ui::ComPtr<ICoreWebView2> m_spWebView2;

    // 事件令牌
    EventRegistrationToken m_webMessageReceivedToken = {0};
    EventRegistrationToken m_documentTitleChangedToken = {0};
    EventRegistrationToken m_navigationStartingToken = { 0 };
    EventRegistrationToken m_navigationCompletedToken = {0};
    EventRegistrationToken m_sourceChangedToken = {0};
    EventRegistrationToken m_newWindowRequestedToken = {0};
    EventRegistrationToken m_historyChangedToken = {0};
    EventRegistrationToken m_zoomFactorChangedToken = { 0 };
    EventRegistrationToken m_faviconChangedToken = { 0 };
    EventRegistrationToken m_gotFocusToken = { 0 };

    // 事件令牌，内部使用
    EventRegistrationToken m_myAcceleratorKeyPressedToken = { 0 };

    // 回调函数
    InitializeCompletedCallback m_initializeCompletedCallback = nullptr;
    WebMessageReceivedCallback m_webMessageReceivedCallback = nullptr;
    NavigationStateChangedCallback m_navigationStateChangedCallback = nullptr;
    DocumentTitleChangedCallback m_documentTitleChangedCallback = nullptr;
    SourceChangedCallback m_sourceChangedCallback = nullptr;
    NewWindowRequestedCallback m_newWindowRequestedCallback = nullptr;
    HistoryChangedCallback m_historyChangedCallback = nullptr;
    ZoomFactorChangedCallback m_zoomFactorChangedCallback = nullptr;
    FavIconChangedCallback m_favIconChangedCallback = nullptr;
    AcceleratorKeyPressedCallback m_acceleratorKeyPressedCallback = nullptr;

    //初始化URL
    DString m_navigateUrl;

    //网站图标的URL
    DStringW m_favIconImageUrl;

    // 状态变量
    Control* m_pControl;
    DString m_userDataFolder;
    DString m_userAgent;

    HRESULT m_lastError;
    bool m_bInitializing;
    bool m_bInitialized;
    bool m_bNavigating;

    /** 异步初始化回调的存活标志
        控件析构时置为false，避免尚未执行的异步回调访问已释放的对象
    */
    std::shared_ptr<bool> m_bAlive;

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

    //是否允许F12快捷键(开发者工具)
    bool m_bEnableF12;

    //是否允许F11快捷键(页面全屏/页面退出全屏)
    bool m_bEnableF11;

#ifdef DUILIB_USE_WEBVIEW2_LOADER_DLL
private:
    /** WebView2Loader.dll 的句柄
    */
    HMODULE m_hWebView2Loader = nullptr;
#endif
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif //UI_WEBVIEW2_WEBVIEW2_CONTROL_IMPL_H_
