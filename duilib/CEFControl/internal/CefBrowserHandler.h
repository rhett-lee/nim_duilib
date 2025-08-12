/** @brief 实现CefClient接口，处理Cef浏览器对象发出的各个事件和消息，并与上层控件进行数据交互
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/7/19
*/
#ifndef UI_CEF_CONTROL_BROWSER_HANDLER_H_
#define UI_CEF_CONTROL_BROWSER_HANDLER_H_

#include "duilib/duilib_config.h"
#include "duilib/CEFControl/internal/CefAutoUnregister.h"
#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/CEFControl/internal/osr_dragdrop_events.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_client.h"
    #include "include/cef_browser.h"
    #include "include/cef_version.h"
#pragma warning (pop)

#include "duilib/Core/Window.h"
#include "duilib/Core/ControlPtrT.h"
#include <mutex>
#include <atomic>

namespace ui
{
class CefBrowserHandlerDelegate;
class ControlDropTarget_Windows;

#ifdef DUILIB_BUILD_FOR_WIN

//Windows平台的DropTarget实现
class CefOsrDropTarget;

#endif

//实现CefClient接口，处理Cef浏览器对象发出的各个事件和消息，并与上层控件进行数据交互
class CefBrowserHandler : public virtual ui::SupportWeakCallback,
    public CefClient,
    public CefLifeSpanHandler,
    public CefRenderHandler,
    public CefContextMenuHandler,
    public CefDisplayHandler,
    public CefDragHandler,
    public CefJSDialogHandler,
    public CefKeyboardHandler,    
    public CefLoadHandler,
    public CefRequestHandler,
    public CefResourceRequestHandler,
    public CefCookieAccessFilter,
    public CefDownloadHandler,
    public CefDialogHandler,
    public CefFocusHandler,
    public client::OsrDragEvents
{
public:
    CefBrowserHandler();
    virtual ~CefBrowserHandler() override;

public:

    /** 设置Cef浏览器对象所属的窗口
    */
    void SetHostWindow(ui::Window* window);

    // 设置委托类指针，浏览器对象的一些事件会交给此指针对象来处理
    // 当指针所指的对象不需要处理事件时，应该给参数传入nullptr
    void SetHandlerDelegate(CefBrowserHandlerDelegate* handler);

    // 设置Cef渲染内容的大小
    void SetViewRect(const UiRect& rc);
    UiRect GetViewRect();

    CefRefPtr<CefBrowser> GetBrowser(){ return m_browser; }

    CefRefPtr<CefBrowserHost> GetBrowserHost();
    CefWindowHandle GetCefWindowHandle();

    // 添加一个任务到队列中，当Browser对象创建成功后，会依次触发任务
    // 比如创建Browser后调用LoadUrl加载网页，但是这时Browser很可能还没有创建成功，就把LoadUrl任务添加到队列
     CefUnregisterCallback AddAfterCreateTask(const ui::StdClosure& cb);

     /** 关闭所有的Browser对象
     */
     void CloseAllBrowsers(bool bForceClose);

     /** 设置关联窗口是否已经关闭
     */
     void SetHostWindowClosed(bool bHostWindowClosed);

     /** 获取拖放接口
    * @return 返回拖放目标接口，如果返回nullptr表示不支持拖放操作
    */
     ControlDropTarget_Windows* GetControlDropTarget();

public:
    
    // CefClient的接口实现
    virtual CefRefPtr<CefAudioHandler> GetAudioHandler() override { return nullptr; }
    virtual CefRefPtr<CefCommandHandler> GetCommandHandler() override { return nullptr; }
    virtual CefRefPtr<CefFindHandler> GetFindHandler() override { return nullptr; }    
    virtual CefRefPtr<CefFrameHandler> GetFrameHandler() override { return nullptr; }
    virtual CefRefPtr<CefPermissionHandler> GetPermissionHandler() override { return nullptr; }
    virtual CefRefPtr<CefPrintHandler> GetPrintHandler() override { return nullptr; }    

    virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override {    return this; }
    virtual CefRefPtr<CefRenderHandler>  GetRenderHandler() override { return this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override{ return this; }
    virtual CefRefPtr<CefDragHandler> GetDragHandler() override{ return this; }
    virtual CefRefPtr<CefJSDialogHandler> GetJSDialogHandler() override { return this; }
    virtual CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override{ return this; }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override{ return this; }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override{ return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override{ return this; }
    virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override { return this; }
    virtual CefRefPtr<CefDialogHandler> GetDialogHandler() override { return this; }
    virtual CefRefPtr<CefFocusHandler> GetFocusHandler() override { return this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    // CefLifeSpanHandler接口的实现
#if CEF_VERSION_MAJOR > 109
    //CEF 高版本
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               int popup_id,
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
    virtual void OnBeforeDevToolsPopup(CefRefPtr<CefBrowser> browser,
                                       CefWindowInfo& windowInfo,
                                       CefRefPtr<CefClient>& client,
                                       CefBrowserSettings& settings,
                                       CefRefPtr<CefDictionaryValue>& extra_info,
                                       bool* use_default_window) override;
#else
    //CEF 109版本
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
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool* no_javascript_access) override;
#endif
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefRenderHandler接口的实现
    virtual CefRefPtr<CefAccessibilityHandler> GetAccessibilityHandler() override;
    virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) override;
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, void* shared_handle) override;
#else
    //CEF 高版本
    virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const CefAcceleratedPaintInfo& info) override;
#endif
    virtual void GetTouchHandleSize(CefRefPtr<CefBrowser> browser, cef_horizontal_alignment_t orientation, CefSize& size) override;
    virtual void OnTouchHandleStateChanged(CefRefPtr<CefBrowser> browser, const CefTouchHandleState& state) override;
    virtual bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) override;
    virtual void UpdateDragCursor(CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation) override;
    virtual void OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y) override;
    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const RectList& character_bounds) override;
    virtual void OnTextSelectionChanged(CefRefPtr<CefBrowser> browser, const CefString& selected_text, const CefRange& selected_range) override;
    virtual void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser, TextInputMode input_mode) override;

    // OsrDragEvents接口的实现
    virtual CefBrowserHost::DragOperationsMask OnDragEnter( CefRefPtr<CefDragData> drag_data, CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect) override;
    virtual CefBrowserHost::DragOperationsMask OnDragOver(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect) override;
    virtual void OnDragLeave() override;
    virtual CefBrowserHost::DragOperationsMask OnDrop(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect) override;

    // CefContextMenuHandler接口的实现
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) override;
    virtual bool RunContextMenu(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefContextMenuParams> params,
                                CefRefPtr<CefMenuModel> model,
                                CefRefPtr<CefRunContextMenuCallback> callback)  override;
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      EventFlags event_flags) override;

    virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;
    virtual bool RunQuickMenu(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              const CefPoint& location,
                              const CefSize& size,
                              QuickMenuEditStateFlags edit_state_flags,
                              CefRefPtr<CefRunQuickMenuCallback> callback) override;
    virtual bool OnQuickMenuCommand(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    int command_id,
                                    EventFlags event_flags) override;
    virtual void OnQuickMenuDismissed(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame) override;

    // CefDisplayHandler接口的实现
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) override;
    virtual bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text) override;
    virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  cef_log_severity_t level,
                                  const CefString& message,
                                  const CefString& source,
                                  int line) override;
    virtual bool OnAutoResize(CefRefPtr<CefBrowser> browser, const CefSize& new_size) override;
    virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) override;
    virtual bool OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, cef_cursor_type_t type, const CefCursorInfo& custom_cursor_info) override;
    virtual void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access) override;

    //CefDragHandler接口的实现
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask) override;
    virtual void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions) override;

    // CefLoadHandler接口的实现
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override;

    // CefJSDialogHandler接口的实现
    virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser,
                            const CefString& origin_url,
                            JSDialogType dialog_type,
                            const CefString& message_text,
                            const CefString& default_prompt_text,
                            CefRefPtr<CefJSDialogCallback> callback,
                            bool& suppress_message) override;
    virtual bool OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
                                      const CefString& message_text,
                                      bool is_reload,
                                      CefRefPtr<CefJSDialogCallback> callback) override;
    virtual void OnResetDialogState(CefRefPtr<CefBrowser> browser) override;
    virtual void OnDialogClosed(CefRefPtr<CefBrowser> browser) override;

    //CefKeyboardHandler接口的实现
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut) override;
    virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event) override;

    // CefRequestHandler接口的实现
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect) override;
    virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefString& target_url,
                                  CefRequestHandler::WindowOpenDisposition target_disposition,
                                  bool user_gesture) override;
    virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
                                                                           CefRefPtr<CefFrame> frame,
                                                                           CefRefPtr<CefRequest> request,
                                                                           bool is_navigation,
                                                                           bool is_download,
                                                                           const CefString& request_initiator,
                                                                           bool& disable_default_handling) override;
    virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                    const CefString& origin_url,
                                    bool isProxy,
                                    const CefString& host,
                                    int port,
                                    const CefString& realm,
                                    const CefString& scheme,
                                    CefRefPtr<CefAuthCallback> callback) override;
    virtual bool OnCertificateError(CefRefPtr<CefBrowser> browser,
                                    cef_errorcode_t cert_error,
                                    const CefString& request_url,
                                    CefRefPtr<CefSSLInfo> ssl_info,
                                    CefRefPtr<CefCallback> callback) override;

    virtual bool OnSelectClientCertificate(CefRefPtr<CefBrowser> browser,
                                           bool isProxy,
                                           const CefString& host,
                                           int port,
                                           const X509CertificateList& certificates,
                                           CefRefPtr<CefSelectClientCertificateCallback> callback) override;
    virtual void OnRenderViewReady(CefRefPtr<CefBrowser> browser) override;

#if CEF_VERSION_MAJOR > 109
    //CEF 高版本
    virtual bool OnRenderProcessUnresponsive(CefRefPtr<CefBrowser> browser,
                                             CefRefPtr<CefUnresponsiveProcessCallback> callback) override;
    virtual void OnRenderProcessResponsive(CefRefPtr<CefBrowser> browser) override;

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           TerminationStatus status,
                                           int error_code,
                                           const CefString& error_string) override;
#else
    //CEF 109版本
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) override;
#endif

    virtual void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) override;

    //CefResourceRequestHandler接口的实现
    virtual CefRefPtr<CefCookieAccessFilter> GetCookieAccessFilter(CefRefPtr<CefBrowser> browser,
                                                                   CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefRequest> request) override;
    virtual CefResourceRequestHandler::ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                                        CefRefPtr<CefFrame> frame,
                                                                        CefRefPtr<CefRequest> request,
                                                                        CefRefPtr<CefCallback> callback) override;
    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                             CefRefPtr<CefFrame> frame,
                                                             CefRefPtr<CefRequest> request) override;
    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url) override;
    virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) override;
    virtual CefRefPtr<CefResponseFilter> GetResourceResponseFilter(CefRefPtr<CefBrowser> browser,
                                                                   CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefRequest> request,
                                                                   CefRefPtr<CefResponse> response) override;
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        URLRequestStatus status,
                                        int64_t received_content_length) override;
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution) override;

    //CefCookieAccessFilter接口的实现
    virtual bool CanSendCookie(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefRequest> request,
                               const CefCookie& cookie) override;
    virtual bool CanSaveCookie(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame> frame,
                               CefRefPtr<CefRequest> request,
                               CefRefPtr<CefResponse> response,
                               const CefCookie& cookie) override;

    // CefDownloadHandler接口的实现
    virtual bool CanDownload(CefRefPtr<CefBrowser> browser,
                             const CefString& url,
                             const CefString& request_method) override;
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    virtual void OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) override;
#else
    //CEF 高版本
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) override;
#endif
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) override;

    // CefDialogHandler接口的实现
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              CefRefPtr<CefFileDialogCallback> callback) override;
#else
    //CEF 高版本
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback) override;
#endif

    //CefFocusHandler 相关接口
    virtual void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;
    virtual bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) override;
    virtual void OnGotFocus(CefRefPtr<CefBrowser> browser) override;

private:
    bool DoOnBeforePopup(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int popup_id,
                         const CefString& target_url,
                         const CefString& target_frame_name,
                         CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                         bool user_gesture,
                         const CefPopupFeatures& popupFeatures,
                         CefWindowInfo& windowInfo,
                         CefRefPtr<CefClient>& client,
                         CefBrowserSettings& settings,
                         CefRefPtr<CefDictionaryValue>& extra_info,
                         bool* no_javascript_access);

#ifdef DUILIB_BUILD_FOR_WIN
    void DoDragDrop(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y);
#endif

    /** 注册DragDrop接口
    */
    void RegisterDropTarget();

    /** 注销DragDrop接口
    */
    void UnregisterDropTarget();

private:
    /** 数据多线程同步锁
    */
    std::mutex m_dataMutex;

    CefRefPtr<CefBrowser> m_browser;
    CefWindowHandle m_hCefWindowHandle;
    std::vector<CefRefPtr<CefBrowser>> m_browserList;
    ControlPtrT<ui::Window> m_spWindow; 
    CefBrowserHandlerDelegate* m_pHandlerDelegate;
    //控件的位置
    UiRect m_rcCefControl;
    CefUnregistedCallbackList<ui::StdClosure> m_taskListAfterCreated;
    CefRenderHandler::DragOperation m_currentDragOperation;

    //关联窗口是否已经关闭
    std::atomic<bool> m_bHostWindowClosed;

#ifdef DUILIB_BUILD_FOR_WIN
    std::shared_ptr<CefOsrDropTarget> m_pDropTarget;
#endif

    IMPLEMENT_REFCOUNTING(CefBrowserHandler);
};
}

#endif //UI_CEF_CONTROL_BROWSER_HANDLER_H_
