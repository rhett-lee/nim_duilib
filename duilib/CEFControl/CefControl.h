#ifndef UI_CEF_CONTROL_CEF_CONTROL_H_
#define UI_CEF_CONTROL_CEF_CONTROL_H_

#include "duilib/Core/Control.h"
#include "duilib/CEFControl/CefControlEvent.h"
#include "duilib/CEFControl/internal/CefBrowserHandlerDelegate.h"
#include <memory>

namespace ui {

class CefJSBridge;
class CefBrowserHandler;
class IBitmap;

/**@brief 封装Cef浏览器对象为duilib控件基类
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2019/3/20
 */
class CefControl: public Control, public CefBrowserHandlerDelegate
{
    typedef Control BaseClass;
public:
    explicit CefControl(ui::Window* pWindow);
    virtual ~CefControl() override;

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
    CefWindowHandle GetCefWindowHandle() const;

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

    /** 修复浏览器对象（如果Browser对象未创建，则重新创建Browser对象）
    */
    virtual void RepairBrowser();

    /** 关闭所有的Browser对象
    */
    virtual void CloseAllBrowsers();

    /** 将网页保存为一张图片, 图片大小与控件大小相同
    */
    virtual std::shared_ptr<IBitmap> MakeImageSnapshot();

    /** 关联窗口关闭事件
    */
    virtual void OnHostWindowClosed();

public:

    /** 设置开发者工具关联的控件(设置后，开发者的内容就显示在这个控件中；如果为nullptr，则开发者工具显示为弹出式窗口)
    *   仅当离屏渲染时，此参数设置才会生效；不是离屏渲染时，该参数被忽略
    */
    void SetDevToolsView(CefControl* pDevToolsView);

    /** 打开开发者工具
    * @return 成功返回 true，失败返回 false
    */
    bool AttachDevTools();

    /** 关闭开发者工具
    */
    void DettachDevTools();

    /** 判断是否打开开发者工具
    * @return 返回 true 表示已经绑定，false 为未绑定
    */
    bool IsAttachedDevTools() const;

    /** 清除DevTools绑定标志，并按需触发事件
    */
    void ResetDevToolAttachedState();

    /** 设置是否允许F12快捷键(开发者工具)
    */
    void SetEnableF12(bool bEnableF12);

    /** 是否允许F12快捷键(开发者工具)
    */
    bool IsEnableF12() const;

    /** 设置是否下载网站的FavIcon图标
    * @param [in] bDownload true表示下载，false表示不下载
    */
    void SetDownloadFaviconImage(bool bDownload);

    /** 是否下载网站的FavIcon图标
    */
    bool IsDownloadFaviconImage() const;

    /** 设置初始加载的URL(仅在控件初始化前调用有效)
    */
    void SetInitURL(const DString& url);

    /** 获取初始加载的URL
    */
    DString GetInitURL() const;

    /** 设置初始加载的URL是否为本地文件
    */
    void SetInitUrlIsLocalFile(bool bUrlIsLocalFile);

    /** 获取初始加载的URL是否为本地文件
    */
    bool IsInitUrlIsLocalFile() const;

public:
    /** 绑定一个回调函数用于监听右键菜单弹出（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforeContextMenuEvent 声明
    */
    void AttachBeforeContextMenu(const OnBeforeContextMenuEvent& callback){ m_pfnBeforeContextMenu = callback; }

    /** 绑定一个回调函数用于监听选择了哪个右键菜单（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnContextMenuCommandEvent 声明
    */
    void AttachContextMenuCommand(const OnContextMenuCommandEvent& callback){ m_pfnContextMenuCommand = callback; }

    /** 绑定一个回调函数用于监听右键菜单消失（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnContextMenuDismissedEvent 声明
    */
    void AttachContextMenuDismissed(const OnContextMenuDismissedEvent& callback) { m_pfnContextMenuDismissed = callback; }

    /** 绑定一个回调函数用于监听页面 Title 改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnTitleChangeEvent 声明
    */
    void AttachTitleChange(const OnTitleChangeEvent& callback){ m_pfnTitleChange = callback; }

    /** 绑定一个回调函数用于监听页面中 frame URL 地址改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnUrlChangeEvent 声明
    */
    void AttachUrlChange(const OnUrlChangeEvent& callback){ m_pfnUrlChange = callback; }

    /** 绑定一个回调函数用于监听主页面 URL 地址改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnMainUrlChangeEvent 声明
    */
    void AttachMainUrlChange(OnMainUrlChangeEvent callback){ m_pfnMainUrlChange = callback; }

    /** 绑定一个回调函数用于监听主页面 FaviconURL 地址改变（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnFaviconURLChangeEvent 声明
    */
    void AttachFaviconURLChange(OnFaviconURLChangeEvent callback) { m_pfnFaviconURLChange = callback; }

    /** 绑定一个回调函数用于监听页面的全屏状态改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnFullscreenModeChangeEvent 声明
    */
    void AttachFullscreenModeChange(OnFullscreenModeChangeEvent callback) { m_pfnFullscreenModeChange = callback; }

    /** 绑定一个回调函数用于监听页面的状态信息改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnStatusMessageEvent 声明
    */
    void AttachStatusMessage(OnStatusMessageEvent callback) { m_pfnStatusMessage = callback; }

    /** 绑定一个回调函数用于监听页面的加载进度改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnLoadingProgressChangeEvent 声明
    */
    void AttachLoadingProgressChange(OnLoadingProgressChangeEvent callback) { m_pfnLoadingProgressChange = callback; }

    /** 绑定一个回调函数用于监听页面的多媒体访问情况改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnMediaAccessChangeEvent 声明
    */
    void AttachMediaAccessChange(OnMediaAccessChangeEvent callback) { m_pfnMediaAccessChange = callback; }

    /** 绑定一个回调函数用于监听页面的拖动操作（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnDragEnterEvent 声明
    */
    void AttachDragEnter(OnDragEnterEvent callback) { m_pfnDragEnter = callback; }

    /** 绑定一个回调函数用于监听页面的可拖动区域变化（回调函数的调用线程：主线程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnDraggableRegionsChangedEvent 声明
    */
    void AttachDraggableRegionsChanged(OnDraggableRegionsChangedEvent callback) { m_pfnDraggableRegionsChanged = callback; }

    /** 绑定一个回调函数用于监听一个弹出窗口弹出的通知（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforePopupEvent 声明
    */
    void AttachBeforePopup(const OnBeforePopupEvent& callback){ m_pfnBeforePopup = callback; }

    /** 绑定一个回调函数用于监听一个弹出窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforePopupAbortedEvent 声明
    */
    void AttachBeforePopupAborted(const OnBeforePopupAbortedEvent& callback) { m_pfnBeforePopupAborted = callback; }

    /** 绑定一个回调函数用于监听页面资源加载事件（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforeResourceLoadEvent 声明
    */
    void AttachBeforeResourceLoad(const OnBeforeResourceLoadEvent& callback) { m_pfnBeforeResourceLoad = callback; }

    /** 绑定一个回调函数用于监听页面资源加载重定向事件（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnResourceRedirectEvent 声明
    */
    void AttachResourceRedirect(const OnResourceRedirectEvent& callback) { m_pfnResourceRedirect = callback; }

    /** 绑定一个回调函数用于监听页面资源加载收到回应事件（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnResourceResponseEvent 声明
    */
    void AttachResourceResponse(const OnResourceResponseEvent& callback) { m_pfnResourceResponse = callback; }

    /** 绑定一个回调函数用于监听页面资源加载完成事件（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnResourceLoadCompleteEvent 声明
    */
    void AttachResourceLoadComplete(const OnResourceLoadCompleteEvent& callback) { m_pfnResourceLoadComplete = callback; }

    /** 绑定一个回调函数用于监听一个未知的 Protocol（回调函数的调用线程：CEF的UI线程）
    * @param[in] callback 一个回调函数，参考 OnProtocolExecutionEvent 声明
    */
    void AttachProtocolExecution(const OnProtocolExecutionEvent& callback) { m_pfnProtocolExecution = callback; }

    /** 绑定一个回调函数用于监听页面加载状态改变（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnLoadingStateChangeEvent 声明
    */
    void AttachLoadingStateChange(const OnLoadingStateChangeEvent& callback){ m_pfnLoadingStateChange = callback; }

    /** 绑定一个回调函数用于监听页面开始加载通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnLoadStartEvent 声明
    */
    void AttachLoadStart(const OnLoadStartEvent& callback){ m_pfnLoadStart = callback; }

    /** 绑定一个回调函数用于监听页面加载完毕通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnLoadEndEvent 声明
    */
    void AttachLoadEnd(const OnLoadEndEvent& callback){ m_pfnLoadEnd = callback; }

    /** 绑定一个回调函数用于监听界面加载错误通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnLoadErrorEvent 声明
    */
    void AttachLoadError(const OnLoadErrorEvent& callback){ m_pfnLoadError = callback; }

    /** 绑定一个回调函数用于监听开发者工具状态改变通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnDevToolAttachedStateChangeEvent 声明
    */
    void AttachDevToolAttachedStateChange(const OnDevToolAttachedStateChangeEvent& callback){ m_pfnDevToolVisibleChange = callback; };

    /** 绑定一个回调函数用于监听一个新的浏览器实例创建完毕通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnAfterCreatedEvent 声明
    */
    void AttachAfterCreated(const OnAfterCreatedEvent& callback){ m_pfnAfterCreated = callback; }

    /** 绑定一个回调函数用于监听一个浏览器实例关闭前的通知（回调函数的调用线程：主进程的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforeCloseEvent 声明
    */
    void AttachBeforeClose(const OnBeforeCloseEvent& callback) { m_pfnBeforeClose = callback; }

    /** 绑定一个回调函数用于监听一个浏览器实例加载通知（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforeBrowseEvent 声明
    */
    void AttachBeforeBrowse(const OnBeforeBrowseEvent& callback) { m_pfnBeforeBrowse = callback; }

    /** 绑定一个回调函数用于监听一个是否可执行下载任务的通知（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnCanDownloadEvent 声明
    */
    void AttachCanDownload(const OnCanDownloadEvent& callback) { m_pfnCanDownload = callback; }

    /** 绑定一个回调函数用于监听一个下载任务开始之前的通知（回调函数的调用线程：CEF的UI线程）
    * @param [in] callback 一个回调函数，参考 OnBeforeDownloadEvent 声明
    */
    void AttachBeforeDownload(const OnBeforeDownloadEvent& callback) { m_pfnBeforeDownload = callback; }

    /** 绑定一个回调函数用于监听下载过程中任务的状态改变通知（回调函数的调用线程：CEF的UI线程）
    * @param[in] callback 一个回调函数，参考 OnDownloadUpdatedEvent 声明
    */
    void AttachDownloadUpdated(const OnDownloadUpdatedEvent& callback) { m_pfnDownloadUpdated = callback; }

    /** 绑定一个回调函数用于监听一个从对话框中打开文件的通知（回调函数的调用线程：CEF的UI线程）
    * @param[in] callback 一个回调函数，参考 OnFileDialogEvent 声明
    */
    void AttachFileDialog(const OnFileDialogEvent& callback) { m_pfnFileDialog = callback; }

    /** 绑定一个回调函数用于监听一个主框架的文档加载完成的通知（回调函数的调用线程：主线程的UI线程）
    * @param[in] callback 一个回调函数，参考 OnDocumentAvailableInMainFrameEvent 声明
    */
    void AttachDocumentAvailableInMainFrame(const OnDocumentAvailableInMainFrameEvent& callback) { m_pfnDocumentAvailableInMainFrame = callback; }

    /** 绑定一个回调函数用于监听网站图标下载完成的通知（回调函数的调用线程：主线程的UI线程）
    * @param[in] callback 一个回调函数，参考 OnDownloadFavIconFinishedEvent 声明
    */
    void AttachDownloadFavIconFinished(const OnDownloadFavIconFinishedEvent& callback) { m_pfnDownloadFavIconFinished = callback; }

public:
    /** 设置CEF控件事件的回调接口
        （1）如果设置此回调接口，那么可以不使用上述的AttachXXX事件回调函数，二者可以取其一使用，但功能互斥，AttachXXX为高优先级）
        （2）对于接口中的回调函数，如果同时设置了对应的AttachXXX事件回调函数，那么AttachXXX为高优先级，pCefControlEventHandler对应的函数不会被回调
    * @param [in] pCefControlEventHandler CEF控件事件的回调接口
    */
    void SetCefEventHandler(CefControlEvent* pCefControlEventHandler);

    /** 获取CEF控件事件的回调接口
    */
    CefControlEvent* GetCefEventHandler() const;

    /** 获取CEF的Browser对象
    */
    CefRefPtr<CefBrowser> GetCefBrowser();

    /** 获取CEF的BrowserHost对象
    */
    CefRefPtr<CefBrowserHost> GetCefBrowserHost();

    /** 重新下载网站的FavIcon
    */
    bool ReDownloadFavIcon();

public:
    /** 获取控件类型
    */
    virtual DString GetType() const override;

    /** 设置控件指定属性
     */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

protected:
    /** 重新创建Browser对象
    */
    virtual void ReCreateBrowser() = 0;

    /** 关闭所有的Browser对象
    */
    void DoCloseAllBrowsers(bool bForceClose);

    /** 获取拖放接口
    * @return 返回拖放目标接口，如果返回nullptr表示不支持拖放操作
    */
    virtual ControlDropTarget_Windows* GetControlDropTarget() override;

protected:
    /** CefRenderHandler接口, 在非UI线程中被调用
    *   当浏览器渲染数据变化时，会触发此接口，此时把渲染数据保存到内存dc
        并且通知窗体刷新控件，在控件的Paint函数里把内存dc的位图画到窗体上
        由此实现离屏渲染数据画到窗体上
    */
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                         const CefRenderHandler::RectList& dirtyRects, const void* buffer,
                         int width, int height) override;//CefRenderHandler接口
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;//CefRenderHandler接口
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;//CefRenderHandler接口
    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const std::vector<CefRect>& character_bounds) override;

    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;

    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params,
                                      int command_id, CefContextMenuHandler::EventFlags event_flags) override;
    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;

    //CefDisplayHandler接口
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) override;
    virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value) override;
    virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) override;
    virtual void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access) override;

    //CefDragHandler接口的实现
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask) override;
    virtual void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions) override;

    //CefLoadHandler接口
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             CefLoadHandler::ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) override;

    //CefLifeSpanHandler接口, 在非UI线程中被调用
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               int popup_id,//仅在CEF 133及以上版本存在
                               const CefString& target_url,
                               const CefString& target_frame_name,
                               CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                               bool user_gesture,
                               const CefPopupFeatures& popupFeatures,
                               CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings& settings,
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool* no_javascript_access) override;
    virtual void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id) override;
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    //CefRequestHandler接口, 在非UI线程中被调用
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) override;

    //CefResourceRequestHandler接口, 在非UI线程中被调用
    virtual cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefFrame> frame,
                                                    CefRefPtr<CefRequest> request,
                                                    CefRefPtr<CefCallback> callback) override;
    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url) override;
    virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) override;
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length) override;
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution) override;

    //CefRequestHandler接口
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           CefRequestHandler::TerminationStatus status,
                                           int error_code,
                                           CefString error_string) override;
    virtual void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) override;

    //CefDownloadHandler接口 文件下载相关
    virtual bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method) override;
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) override;
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) override;

    //CefDialogHandler接口, 打开文件 Dialog
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefBrowserHost::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback) override;

    //CefKeyboardHandler接口
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                               const CefKeyEvent& event,
                               CefEventHandle os_event,
                               bool* is_keyboard_shortcut) override;
    virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                            const CefKeyEvent& event,
                            CefEventHandle os_event) override;

protected:
    /** 客户区坐标转换为控件坐标
    */
    virtual void ClientToControl(UiPoint& pt) override;

    /** 更新CEF控件窗口的位置（子窗口模式）
    */
    virtual void UpdateCefWindowPos() override;

    /** 执行C++函数（JS 与 C++ 交互的功能）
    */
    virtual bool OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;

    /** 执行C++回调函数（JS 与 C++ 交互的功能）
    */
    virtual bool OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string) override;

    /** 检查是否存在如下回调函数（用以减少向UI线程发消息，以提高效率）
    * @param [in] nCallbackID 回调函数的ID，参见枚举类型定义
    */
    virtual bool IsCallbackExists(CefCallbackID nCallbackID) override;

    /** 焦点元素发生变化（在主线程中调用）
    */
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefDOMNode::Type type,
                                      bool bText,
                                      bool bEditable,
                                      const CefRect& nodeRect) override;

    /** 设置光标(仅离屏渲染模式有效)
    */
    virtual void OnCursorChange(cef_cursor_type_t type) override;

    /** 页面获得了焦点
    */
    virtual void OnGotFocus() override;

    /** 获取关联的CEF控件接口
    */
    virtual Control* GetCefControl() override;

    /// Method that will be executed when the image download has completed.
    /// |image_url| is the URL that was downloaded and |http_status_code| is the
    /// resulting HTTP status code. |image| is the resulting image, possibly at
    /// multiple scale factors, or empty if the download failed.
    ///
    /*--cef(optional_param=image)--*/
    friend class CefControlDownloadImageCallback;
    virtual void OnDownloadImageFinished(const CefString& image_url,
                                         int http_status_code,
                                         CefRefPtr<CefImage> image);

protected:
    /** 设置打开开发者工具标志
    * @param [in] bAttachedDevTools 打开/关闭关联开发者工具
    * @param [in] bPopup 开发者工具是否为弹出式窗口模式
    */
    void SetAttachedDevTools(bool bAttachedDevTools, bool bPopup);

    /** 开发者工具的可见性发生变化, 触发接口回调（确保在主进程的UI线程回调）
    */
    void OnDevToolVisibleChanged(bool bAttachedDevTools, bool bPopup);

protected:
    /** 控件关联的CEF浏览器对象
    */
    CefRefPtr<CefBrowserHandler> m_pBrowserHandler;

    /** JS 功能管理接口
    */
    std::shared_ptr<CefJSBridge> m_jsBridge;

private:

    /** 初始化加载的网址
    */
    UiString m_initUrl;

    /** 设置初始加载的URL是否为本地文件
    */
    bool m_bUrlIsLocalFile;

    /** 当前主Frame的URL
    */
    UiString m_url;

    // 保存接收到 JS 调用 CPP 函数的代码所属线程，以后触发 JS 回调时把回调转到那个线程
    int32_t m_jsCallbackThreadId = -1;

    OnBeforeContextMenuEvent        m_pfnBeforeContextMenu = nullptr;
    OnContextMenuCommandEvent       m_pfnContextMenuCommand = nullptr;
    OnContextMenuDismissedEvent     m_pfnContextMenuDismissed = nullptr;
    OnTitleChangeEvent              m_pfnTitleChange = nullptr;
    OnBeforeResourceLoadEvent       m_pfnBeforeResourceLoad = nullptr;
    OnResourceRedirectEvent         m_pfnResourceRedirect = nullptr;
    OnResourceResponseEvent         m_pfnResourceResponse = nullptr;
    OnResourceLoadCompleteEvent     m_pfnResourceLoadComplete = nullptr;
    OnProtocolExecutionEvent        m_pfnProtocolExecution = nullptr;
    OnUrlChangeEvent                m_pfnUrlChange = nullptr;
    OnMainUrlChangeEvent            m_pfnMainUrlChange = nullptr;
    OnFaviconURLChangeEvent         m_pfnFaviconURLChange = nullptr;
    OnFullscreenModeChangeEvent     m_pfnFullscreenModeChange = nullptr;
    OnStatusMessageEvent            m_pfnStatusMessage = nullptr;
    OnLoadingProgressChangeEvent    m_pfnLoadingProgressChange = nullptr;
    OnMediaAccessChangeEvent        m_pfnMediaAccessChange = nullptr;
    OnBeforePopupEvent              m_pfnBeforePopup = nullptr;
    OnBeforePopupAbortedEvent       m_pfnBeforePopupAborted = nullptr;
    OnLoadingStateChangeEvent       m_pfnLoadingStateChange = nullptr;
    OnLoadStartEvent                m_pfnLoadStart = nullptr;
    OnLoadEndEvent                  m_pfnLoadEnd = nullptr;
    OnLoadErrorEvent                m_pfnLoadError = nullptr;
    OnAfterCreatedEvent             m_pfnAfterCreated = nullptr;
    OnBeforeCloseEvent              m_pfnBeforeClose = nullptr;
    OnBeforeBrowseEvent             m_pfnBeforeBrowse = nullptr;
    OnCanDownloadEvent              m_pfnCanDownload = nullptr;
    OnBeforeDownloadEvent           m_pfnBeforeDownload = nullptr;
    OnDownloadUpdatedEvent          m_pfnDownloadUpdated = nullptr;
    OnFileDialogEvent               m_pfnFileDialog = nullptr;
    OnDevToolAttachedStateChangeEvent   m_pfnDevToolVisibleChange = nullptr;
    OnDocumentAvailableInMainFrameEvent m_pfnDocumentAvailableInMainFrame = nullptr;
    OnDownloadFavIconFinishedEvent      m_pfnDownloadFavIconFinished = nullptr;
    OnDragEnterEvent                    m_pfnDragEnter = nullptr;
    OnDraggableRegionsChangedEvent      m_pfnDraggableRegionsChanged = nullptr;

    /** CEF控件的事件回调接口
    */
    CefControlEvent* m_pCefControlEventHandler = nullptr;

private:
    //开发者工具是否显示
    bool m_bAttachedDevTools;

    //开发者工具是否为弹出窗口模式
    bool m_bDevToolsPopup;

    //开发者工具关联的控件
    CefControl* m_pDevToolsView;

    //开发者工具关联的控件的生命周期
    std::weak_ptr<WeakFlag> m_pDevToolsViewFlag;

    //是否允许F12快捷键(开发者工具)
    bool m_bEnableF12;

    //是否下载网站的FavIcon图标
    bool m_bDownloadFaviconImage;

    //网站FavIcon图标的URL
    CefString m_favIconUrl;
};
}

#endif //UI_CEF_CONTROL_CEF_CONTROL_H_
