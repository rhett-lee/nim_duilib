/**@brief 封装Cef浏览器对象为duilib控件基类
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2019/3/20
 */
#ifndef UI_CEF_CONTROL_CEF_CONTROL_BASE_H_
#define UI_CEF_CONTROL_CEF_CONTROL_BASE_H_

#include "duilib/Core/Control.h"
#include "duilib/CEFControl/CefControlEvent.h"
#include "duilib/CEFControl/CefBrowserHandlerDelegate.h"

namespace ui {

class CefJSBridge;
class CefBrowserHandler;

class CefControlBase : public Control, public CefBrowserHandlerDelegate
{
public:
    explicit CefControlBase(ui::Window* pWindow);
    virtual ~CefControlBase() override;

public:
    /** 加载一个地址
    * @param [in] url 网站地址
    */
    void LoadURL(const CefString& url);

    /** 后退
    */
    void GoBack();

    /** 前进
    */
    void GoForward();

    /** 判断是否可以后退
    * @return 返回 true 表示可以，false 表示不可以
    */
    bool CanGoBack();

    /** 判断是否可以前进
    * @return 返回 true 表示可以，false 表示不可以
    */
    bool CanGoForward();

    /** 刷新
    */
    void Refresh();

    /** 停止加载
    */
    void StopLoad();

    /** 是否加载中
    * @return 返回 true 表示加载中，否则为 false
    */
    bool IsLoading();

    /** 开始一个下载任务
    * @param[in] url 要下载的文件地址
    */
    void StartDownload(const CefString& url);

    /** 设置页面缩放比例
    * @param[in] zoom_level 比例值
    */
    void SetZoomLevel(float zoom_level);

    /** 获取浏览器对象所属的窗体句柄
    * @return 窗口句柄
    */
    HWND GetCefHandle() const;//TODO: 跨平台

    /** 获取页面 URL
    * @return 返回 URL 地址
    */
    CefString GetURL();

    /** 获取 UTF8 格式 URL
    * @return 返回 URL 地址
    */
    std::string GetUTF8URL();

    /** 获取网址 # 号前的地址
    * @param[in] url 要获取的完整地址
    * @return 返回截取后的地址
    */
    CefString GetMainURL(const CefString& url);

    /** 注册一个 C++ 方法提供前端调用
    * @param[in] function_name 方法名称
    * @param[in] function 方法函数体
    * @param[in] global_function 是否是一个全局方法
    * @return 返回 true 表示注册成功，false 可能已经注册
    */
    bool RegisterCppFunc(const DString& function_name, CppFunction function, bool global_function = false);

    /** 反注册一个 C++ 方法
    * @param[in] function_name 方法名称
    */
    void UnRegisterCppFunc(const DString& function_name);

    /** 调用一个前端已经注册好的方法
    * @param[in] js_function_name 前端提供的方法名
    * @param[in] params 传递 JSON 字符串格式的参数
    * @param[in] callback 前端执行完成后的回调函数
    * @param[in] frame_name 要调用哪个名称 frame 下的方法，默认使用主 frame
    * @return 返回 true 表示成功调用，false 表示调用失败，方法可能不存在
    */
    bool CallJSFunction(const DString& js_function_name, const DString& params, CallJsFunctionCallback callback, const DString& frame_name = _T(""));

    /** 调用一个前端已经注册好的方法
    * @param[in] js_function_name 前端提供的方法名
    * @param[in] params 传递 JSON 字符串格式的参数
    * @param[in] callback 前端执行完成后的回调函数
    * @param[in] frame_id 要调用哪个 ID frame 下的方法，默认使用主 frame
    * @return 返回 true 表示成功调用，false 表示调用失败，方法可能不存在
    */
    bool CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const CefString& frame_id);

    /** 修复浏览器对象（重新创建Browser对象）
    */
    virtual void RepairBrowser();

    /** 打开开发者工具
    * @param[in] view 一个 CefControl 控件实例(仅在CefControl类里需要传入)
    * @return 成功返回 true，失败返回 false
    */
    virtual bool AttachDevTools(Control* view) = 0;

    /** 关闭开发者工具
    */
    virtual void DettachDevTools();

    /** 判断是否打开开发者工具
    * @return 返回 true 表示已经绑定，false 为未绑定
    */
    virtual bool IsAttachedDevTools() const { return devtool_attached_; }

protected:
    /** 重新创建Browser对象
    */
    virtual void ReCreateBrowser() = 0;

public:
    /** 绑定一个回调函数用于监听右键菜单弹出
    * @param [in] callback 一个回调函数，参考 OnBeforeMenuEvent 声明
    */
    void AttachBeforeContextMenu(const OnBeforeMenuEvent& callback){ cb_before_menu_ = callback; }

    /** 绑定一个回调函数用于监听选择了哪个右键菜单
    * @param [in] callback 一个回调函数，参考 OnMenuCommandEvent 声明
    */
    void AttachMenuCommand(const OnMenuCommandEvent& callback){ cb_menu_command_ = callback; }

    /** 绑定一个回调函数用于监听页面 Title 改变
    * @param [in] callback 一个回调函数，参考 OnTitleChangeEvent 声明
    */
    void AttachTitleChange(const OnTitleChangeEvent& callback){ cb_title_change_ = callback; }

    /** 绑定一个回调函数用于监听页面中 frame URL 地址改变
    * @param [in] callback 一个回调函数，参考 OnUrlChangeEvent 声明
    */
    void AttachUrlChange(const OnUrlChangeEvent& callback){ cb_url_change_ = callback; }

    /** 绑定一个回调函数用于监听主页面 URL 地址改变
    * @param [in] callback 一个回调函数，参考 OnMainURLChengeEvent 声明
    */
    void AttachMainURLChange(OnMainURLChengeEvent cb){ cb_main_url_change_ = cb; }

    /** 绑定一个回调函数用于监听页面资源全部加载完毕
    * @param [in] callback 一个回调函数，参考 OnBeforeResourceLoadEvent 声明
    */
    void AttachBeforeNavigate(const OnBeforeResourceLoadEvent& callback){ cb_before_resource_load_ = callback; }

    /** 绑定一个回调函数用于监听一个弹出窗口弹出的通知
    * @param [in] callback 一个回调函数，参考 OnLinkClickEvent 声明
    */
    void AttachLinkClick(const OnLinkClickEvent& callback){ cb_link_click_ = callback; }

    /** 绑定一个回调函数用于监听页面加载状态改变
    * @param [in] callback 一个回调函数，参考 OnLoadingStateChangeEvent 声明
    */
    void AttachLoadingStateChange(const OnLoadingStateChangeEvent& callback){ cb_loadstate_change_ = callback; }

    /** 绑定一个回调函数用于监听页面开始加载通知
    * @param [in] callback 一个回调函数，参考 OnLoadStartEvent 声明
    */
    void AttachLoadStart(const OnLoadStartEvent& callback){ cb_load_start_ = callback; }

    /** 绑定一个回调函数用于监听页面加载完毕通知
    * @param [in] callback 一个回调函数，参考 OnLoadEndEvent 声明
    */
    void AttachLoadEnd(const OnLoadEndEvent& callback){ cb_load_end_ = callback; }

    /** 绑定一个回调函数用于监听界面加载错误通知
    * @param [in] callback 一个回调函数，参考 OnLoadErrorEvent 声明
    */
    void AttachLoadError(const OnLoadErrorEvent& callback){ cb_load_error_ = callback; }

    /** 绑定一个回调函数用于监听开发者工具状态改变通知
    * @param [in] callback 一个回调函数，参考 OnDevToolAttachedStateChangeEvent 声明
    */
    void AttachDevToolAttachedStateChange(const OnDevToolAttachedStateChangeEvent& callback){ cb_devtool_visible_change_ = callback; };

    /** 绑定一个回调函数用于监听一个新的浏览器实例创建完毕通知
    * @param [in] callback 一个回调函数，参考 OnAfterCreatedEvent 声明
    */
    void AttachAfterCreated(const OnAfterCreatedEvent& callback){ cb_after_created_ = callback; }

    /** 绑定一个回调函数用于监听一个浏览器实例关闭前的通知
    * @param [in] callback 一个回调函数，参考 OnBeforeCloseEvent 声明
    */
    void AttachBeforeCLose(const OnBeforeCloseEvent& callback) { cb_before_close_ = callback; }

    /** 绑定一个回调函数用于监听一个浏览器实例加载通知
    * @param [in] callback 一个回调函数，参考 OnBeforeBrowserEvent 声明
    */
    void AttachBeforeBrowser(const OnBeforeBrowserEvent& callback) { cb_before_browser_ = callback; }

    /** 绑定一个回调函数用于监听一个未知的 Protocol
    * @param[in] callback 一个回调函数，参考 OnProtocolExecutionEvent 声明
    */
    void AttachProtocolExecution(const OnProtocolExecutionEvent& callback) { cb_protocol_execution_ = callback; }

    /** 绑定一个回调函数用于监听一个下载任务开始之前的通知
    * @param [in] callback 一个回调函数，参考 OnBeforeDownloadEvent 声明
    */
    void AttachBeforeDownload(const OnBeforeDownloadEvent& callback) { cb_before_download_ = callback; }

    /** 绑定一个回调函数用于监听下载过程中任务的状态改变通知
    * @param[in] callback 一个回调函数，参考 OnDownloadUpdatedEvent 声明
    */
    void AttachDownloadUpdated(const OnDownloadUpdatedEvent& callback) { cb_download_updated_ = callback; }

    /** 绑定一个回调函数用于监听一个从对话框中打开文件的通知
    * @param[in] callback 一个回调函数，参考 OnFileDialogEvent 声明
    */
    void AttachFileDialog(const OnFileDialogEvent& callback) { cb_file_dialog_ = callback; }

private:
    // 处理CefBrowserHandler的CefBrowserHandlerDelegate委托接口
    // 当浏览器渲染数据变化时，会触发此接口，此时把渲染数据保存到内存dc
    // 并且通知窗体刷新控件，在控件的Paint函数里把内存dc的位图画到窗体上
    // 由此实现离屏渲染数据画到窗体上
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        CefRenderHandler::PaintElementType type,
        const CefRenderHandler::RectList& dirtyRects,
        const std::string* buffer,
        int width,
        int height) override;
    virtual void ClientToControl(POINT &pt) override;

    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

    virtual void UpdateWindowPos() override;

protected:
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

public:
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        int command_id,
        CefContextMenuHandler::EventFlags event_flags) override;

    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefLoadHandler::ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) override;

    // 在非UI线程中被调用
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& target_url,
        const CefString& target_frame_name,
        CefLifeSpanHandler::WindowOpenDisposition target_disposition,
        bool user_gesture,
        const CefPopupFeatures& popupFeatures,
        CefWindowInfo& windowInfo,
        CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings,
        bool* no_javascript_access) override;

    virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // 在非UI线程中被调用
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect) override;

    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution) override;

    // 在非UI线程中被调用
    virtual cef_return_value_t OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefCallback> callback) override;

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, CefRequestHandler::TerminationStatus status) override;

    // 下载文件相关
    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback) override;

    virtual void OnDownloadUpdated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        CefRefPtr<CefDownloadItemCallback> callback) override;

    // 打开文件相关
    virtual bool OnFileDialog(
        CefRefPtr<CefBrowser> browser,
        CefDialogHandler::FileDialogMode mode,
        const CefString& title,
        const CefString& default_file_path,
        const std::vector<CefString>& accept_filters,
        int selected_accept_filter,
        CefRefPtr<CefFileDialogCallback> callback) override;

    virtual bool OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;

    virtual bool OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string) override;

protected:
    CefRefPtr<CefBrowserHandler> browser_handler_ = nullptr;
    std::shared_ptr<CefJSBridge> js_bridge_;
    CefString url_;
    bool devtool_attached_;

    OnBeforeMenuEvent            cb_before_menu_ = nullptr;
    OnMenuCommandEvent            cb_menu_command_ = nullptr;
    OnTitleChangeEvent            cb_title_change_ = nullptr;
    OnBeforeResourceLoadEvent    cb_before_resource_load_ = nullptr;
    OnUrlChangeEvent            cb_url_change_ = nullptr;
    OnMainURLChengeEvent        cb_main_url_change_ = nullptr;
    OnLinkClickEvent            cb_link_click_ = nullptr;
    OnLoadingStateChangeEvent    cb_loadstate_change_ = nullptr;
    OnLoadStartEvent            cb_load_start_ = nullptr;
    OnLoadEndEvent                cb_load_end_ = nullptr;
    OnLoadErrorEvent            cb_load_error_ = nullptr;
    OnAfterCreatedEvent            cb_after_created_ = nullptr;
    OnBeforeCloseEvent            cb_before_close_ = nullptr;
    OnBeforeBrowserEvent        cb_before_browser_ = nullptr;
    OnProtocolExecutionEvent    cb_protocol_execution_ = nullptr;
    OnBeforeDownloadEvent        cb_before_download_ = nullptr;
    OnDownloadUpdatedEvent        cb_download_updated_ = nullptr;
    OnFileDialogEvent            cb_file_dialog_ = nullptr;
    OnDevToolAttachedStateChangeEvent cb_devtool_visible_change_ = nullptr;
    int                            js_callback_thread_id_ = -1; // 保存接收到 JS 调用 CPP 函数的代码所属线程，以后触发 JS 回调时把回调转到那个线程
};
}

#endif //UI_CEF_CONTROL_CEF_CONTROL_BASE_H_
