#ifndef UI_WEBVIEW2_WEBVIEW2_CONTROL_H_
#define UI_WEBVIEW2_WEBVIEW2_CONTROL_H_

#include "duilib/Core/Control.h"

#if defined (DUILIB_BUILD_FOR_WIN) && defined (DUILIB_BUILD_FOR_WEBVIEW2)

#include <functional>
#include <memory>

namespace ui {

class Window;

/** WebView2控件的C++封装类
 */
class WebView2Control: public Control
{
    typedef Control BaseClass;
public:
    /** 导航状态枚举, 表示WebView2的导航状态
     */
    enum class NavigationState
    {
        Started,    /**< 导航开始 */
        Completed,  /**< 导航完成 */
        Failed      /**< 导航失败 */
    };

    /** 初始化完成回调函数类型
     * @param result 初始化结果（S_OK表示成功）
     */
    using InitializeCompletedCallback = std::function<void(HRESULT result)>;

    /** Web消息接收回调函数类型
     * @param message 接收到的消息内容
     */
    using WebMessageReceivedCallback = std::function<void(const DString& message)>;

    /** 导航状态变化回调函数类型
     * @param state 新的导航状态
     * @param errorCode 错误码（如果导航失败）
     */
    using NavigationCompletedCallback = std::function<void(NavigationState state, HRESULT errorCode)>;

    /** 文档标题变化回调函数类型
     * @param title 新的文档标题
     */
    using DocumentTitleChangedCallback = std::function<void(const DString& title)>;

    /** 源URL变化回调函数类型
     * @param uri 新的源URL
     */
    using SourceChangedCallback = std::function<void(const DString& uri)>;

    /** 内容加载回调函数类型
     * @param isErrorPage 是否为错误页面
     */
    using ContentLoadingCallback = std::function<void(bool isErrorPage)>;

    /** 新窗口请求回调函数类型
     * @param sourceUrl 源的URL
     * @param sourceFrame 源框架名称
     * @param targetUrl 请求的URL
     * @param targetFrame 目标框架名称
     * @param bUserInitiated 是否由用户触发的弹窗
     * @return 是否允许创建弹窗页面
     */
    using NewWindowRequestedCallback = std::function<bool(const DString& sourceUrl, const DString& sourceFrame,
                                                          const DString& targetUrl, const DString& targetFrame,
                                                          bool bUserInitiated)>;

    /** 导航历史变化事件回调函数类型
     */
    using HistoryChangedCallback = std::function<void()>;

    /** 页面缩放比变化事件回调函数类型
     */
    using ZoomFactorChangedCallback = std::function<void(double zoomFactor)>;

public:
    /** 构造函数
     */
    explicit WebView2Control(Window* pWindow);

    /** 析构函数
     */
    virtual ~WebView2Control() override;

    /** 异步初始化WebView
     * @param userDataFolder 用户数据文件夹路径（可选）
     * @param callback 初始化完成回调函数（可选）
     */
    bool InitializeAsync(const DString& userDataFolder = _T(""),
                         InitializeCompletedCallback callback = nullptr);

    /** 检查WebView是否正在初始化
     * @return 是否正在初始化
     */
    bool IsInitializing() const;

    /** 检查WebView是否已完成初始化
     * @return 是否已初始化
     */
    bool IsInitialized() const;
    
    /** 导航到指定URL（可能是异步完成）
     * @param url 要导航的URL
     */
    bool Navigate(const DString& url);
    
    /** 导航到上一页
     */
    bool NavigateBack();
    
    /** 导航到下一页
     */
    bool NavigateForward();
    
    /** 刷新当前页面
     */
    bool Refresh();
    
    /** 停止加载
     * @return HRESULT错误码
     */
    bool Stop();
    
    /** 执行JavaScript脚本
     * @param script 要执行的JavaScript脚本
     * @param callback 执行完成后的回调函数（可选）
     */
    bool ExecuteScript(const DString& script, std::function<void(const DString& result, HRESULT hr)> callback = nullptr);
    
    /** 以JSON格式发送Web消息
     * @param json 要发送的JSON字符串
     */
    bool PostWebMessageAsJson(const DString& json);
    
    /** 以字符串格式发送Web消息
     * @param message 要发送的消息字符串
     */
    bool PostWebMessageAsString(const DString& message);
    
    /** 设置User-Agent
     * @param userAgent 要设置的User-Agent字符串
     */
    bool SetUserAgent(const DString& userAgent);

    /** 获取User-Agent
    */
    DString GetUserAgent() const;
    
    /** 设置缩放因子
     * @param zoomFactor 缩放因子
     */
    bool SetZoomFactor(double zoomFactor);

    /** 获取缩放因子
    */
    double GetZoomFactor() const;
    
    /** 设置JavaScript是否启用
    */
    bool SetScriptEnabled(bool enabled);
    bool IsScriptEnabled() const;
    
    /** 设置Web消息是否启用
    */
    bool SetWebMessageEnabled(bool enabled);
    bool IsWebMessageEnabled() const;

    /** 是否启用默认脚本对话框
    */
    bool SetAreDefaultScriptDialogsEnabled(bool enabled);
    bool AreDefaultScriptDialogsEnabled() const;

    /** 是否允许默认的右键菜单
    */
    bool SetAreDefaultContextMenusEnabled(bool enabled);
    bool AreDefaultContextMenusEnabled() const;

    /** 是否禁用缩放控制
    */
    bool SetZoomControlEnabled(bool enabled);
    bool IsZoomControlEnabled() const;
    
    /** 设置Web消息接收回调函数
     * @param callback 回调函数
     */
    bool SetWebMessageReceivedCallback(WebMessageReceivedCallback callback);
    
    /** 设置导航状态变化回调函数
     * @param callback 回调函数
     */
    bool SetNavigationCompletedCallback(NavigationCompletedCallback callback);
    
    /** 设置文档标题变化回调函数
     * @param callback 回调函数
     */
    bool SetDocumentTitleChangedCallback(DocumentTitleChangedCallback callback);
    
    /** 设置源URL变化回调函数
     * @param callback 回调函数
     */
    bool SetSourceChangedCallback(SourceChangedCallback callback);
    
    /** 设置内容加载回调函数
     * @param callback 回调函数
     */
    bool SetContentLoadingCallback(ContentLoadingCallback callback);
    
    /** 设置新窗口请求回调函数
     * @param callback 回调函数
     */
    bool SetNewWindowRequestedCallback(NewWindowRequestedCallback callback);

    /** 设置导航历史变化回调函数
     * @param callback 回调函数
     */
    bool SetHistoryChangedCallback(HistoryChangedCallback callback);

    /** 设置页面缩放比变化事件回调函数
     */
    bool SetZoomFactorChangedCallback(ZoomFactorChangedCallback callback);
    
    /** 添加文档创建时执行的脚本
     * @param script 要执行的JavaScript脚本
     */
    bool AddScriptToExecuteOnDocumentCreated(const DString& script);
    
    /** 捕获当前页面的预览图(异步完成)，保存为PNG或者JPG格式
     * @param filePath 保存预览图的文件路径，根据保存的图片文件名后缀自动判断保存的格式
     * @param callback 操作完成回调函数（可选）
     */
    bool CapturePreview(const DString& filePath,
                        std::function<void(const DString& filePath, HRESULT hr)> callback = nullptr);
    
    /** 获取当前URL
     * @return 当前URL
     */
    DString GetUrl() const;
    
    /** 获取当前文档标题
     * @return 当前文档标题
     */
    DString GetTitle() const;
    
    /** 检查是否可以导航到上一页
     * @return 是否可以导航到上一页
     */
    bool CanGoBack() const;
    
    /** 检查是否可以导航到下一页
     * @return 是否可以导航到下一页
     */
    bool CanGoForward() const;

    /** 获取最后一次操作的错误码
    */
    HRESULT GetLastErrorCode() const;

public:
    /** 设置是否启用开发者工具
    */
    void SetAreDevToolsEnabled(bool bAreDevToolsEnabled);

    /** 获取是否启用开发者工具
    */
    bool AreDevToolsEnabled() const;

    /** 打开开发者工具
    */
    bool OpenDevToolsWindow();

public:
    // 控件类型相关的属性
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void SetPos(UiRect rc) override;
    virtual bool OnSetFocus(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;
    virtual void SetVisible(bool bVisible) override;
    virtual void SetWindow(Window* pWindow) override;

private:
    // PIMPL实现
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};

} //namespace ui

#endif //DUILIB_BUILD_FOR_WEBVIEW2

#endif UI_WEBVIEW2_WEBVIEW2_CONTROL_H_
