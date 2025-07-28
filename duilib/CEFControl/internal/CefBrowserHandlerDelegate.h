/** @brief 实现CefClient接口，处理Cef浏览器对象发出的各个事件和消息，并与上层控件进行数据交互
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/7/19
*/
#ifndef UI_CEF_CONTROL_BROWSER_HANDLER_DELEGATE_H_
#define UI_CEF_CONTROL_BROWSER_HANDLER_DELEGATE_H_

#include "duilib/Core/Callback.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_client.h"
    #include "include/cef_browser.h"
    #include "include/cef_version.h"
#pragma warning (pop)

namespace ui
{
enum class CefCallbackID;

/** CefBrowserHandler的消息委托类接口，CefBrowserHandler类会处理多数Cef浏览器对象的事件
*  其中一些需要与上层交互的事件接口封装到此类中，CefBrowserHandler把这些事件传递到委托接口中
*  可以根据需求来扩展此接口
*/
class CefBrowserHandlerDelegate : public virtual SupportWeakCallback
{
public:
    /** CefRenderHandler接口, 在非UI线程中被调用
    *   当浏览器渲染数据变化时，会触发此接口，此时把渲染数据保存到内存dc
        并且通知窗体刷新控件，在控件的Paint函数里把内存dc的位图画到窗体上
        由此实现离屏渲染数据画到窗体上
    */
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                         const CefRenderHandler::RectList& dirtyRects, const void* buffer,
                         int width, int height) = 0;//CefRenderHandler接口
    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) = 0;//CefRenderHandler接口
    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) = 0;//CefRenderHandler接口
    virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const std::vector<CefRect>& character_bounds) = 0;

    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) = 0;

    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params,
                                      int command_id, CefContextMenuHandler::EventFlags event_flags) = 0;
    //CefContextMenuHandler接口, 在非UI线程中被调用
    virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) = 0;

    //CefDisplayHandler接口
    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) = 0;
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) = 0;
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) = 0;
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) = 0;
    virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value) = 0;
    virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) = 0;
    virtual void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access) = 0;

    //CefLoadHandler接口
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) = 0;
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type) = 0;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) = 0;
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             CefLoadHandler::ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) = 0;

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
                               bool* no_javascript_access) = 0;
    virtual void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id) = 0;//仅在CEF 133及以上版本存在
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) = 0;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;

    //CefRequestHandler接口, 在非UI线程中被调用
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) = 0;

    //CefResourceRequestHandler接口, 在非UI线程中被调用
    virtual cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefFrame> frame,
                                                    CefRefPtr<CefRequest> request,
                                                    CefRefPtr<CefCallback> callback) = 0;
    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url) = 0;
    virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) = 0;
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length) = 0;
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution) = 0;

    //CefRequestHandler接口
    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           CefRequestHandler::TerminationStatus status,
                                           int error_code,
                                           CefString error_string) = 0;
    virtual void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) = 0;

    //CefDownloadHandler接口 文件下载相关
    virtual bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method) = 0;
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) = 0;
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) = 0;

    //CefDialogHandler接口, 打开文件 Dialog
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefBrowserHost::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback)  = 0;

    //CefKeyboardHandler接口
    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                               const CefKeyEvent& event,
                               CefEventHandle os_event,
                               bool* is_keyboard_shortcut) = 0;
    virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
                            const CefKeyEvent& event,
                            CefEventHandle os_event) = 0 ;

    //CefDragHandler接口
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask) = 0;
    virtual void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions) = 0;

public:
    /** 客户区坐标转换为控件坐标
    */
    virtual void ClientToControl(UiPoint& pt) = 0;

    /** 更新CEF控件窗口的位置（子窗口模式）
    */
    virtual void UpdateCefWindowPos() = 0;

    /** 执行C++函数（JS 与 C++ 交互的功能）
    */
    virtual bool OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) = 0;

    /** 执行C++回调函数（JS 与 C++ 交互的功能）
    */
    virtual bool OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string) = 0;

    /** 检查是否存在如下回调函数（用以减少向UI线程发消息，以提高效率）
    * @param [in] nCallbackID 回调函数的ID，参见枚举类型定义
    */
    virtual bool IsCallbackExists(CefCallbackID nCallbackID) = 0;

    /** 焦点元素发生变化（在主线程中调用）
    */
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefDOMNode::Type type,
                                      bool bText,
                                      bool bEditable,
                                      const CefRect& nodeRect) = 0;

    /** 设置光标(仅离屏渲染模式有效，仅使用SDL时有效)
    */
    virtual void OnCursorChange(cef_cursor_type_t type) = 0;

    /** 页面获得了焦点
    */
    virtual void OnGotFocus() = 0;

    /** 获取关联的CEF控件接口
    */
    virtual Control* GetCefControl() = 0;
};

/** 回调函数的ID
*/
enum class CefCallbackID
{
    OnPopupShow,
    OnPopupSize,
    OnImeCompositionRangeChanged,
    OnBeforeContextMenu,
    OnContextMenuCommand,
    OnContextMenuDismissed,
    OnAddressChange,
    OnTitleChange,
    OnFaviconURLChange,
    OnFullscreenModeChange,
    OnStatusMessage,
    OnLoadingProgressChange,
    OnMediaAccessChange,
    OnLoadingStateChange,
    OnLoadStart,
    OnLoadEnd,
    OnLoadError,
    OnBeforePopup,
    OnBeforePopupAborted,
    OnAfterCreated,
    OnBeforeClose,
    OnBeforeBrowse,
    OnBeforeResourceLoad,
    OnResourceRedirect,
    OnResourceResponse,
    OnResourceLoadComplete,
    OnProtocolExecution,
    OnRenderProcessTerminated,
    OnDocumentAvailableInMainFrame,
    OnCanDownload,
    OnBeforeDownload,
    OnDownloadUpdated,
    OnFileDialog,
    OnPreKeyEvent,
    OnKeyEvent,
    OnDragEnter,
    OnDraggableRegionsChanged
};

}//namespace ui

#endif //UI_CEF_CONTROL_BROWSER_HANDLER_DELEGATE_H_
