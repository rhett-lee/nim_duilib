#ifndef UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
#define UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_

#include "duilib/duilib_config.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_client.h"

#include <functional>

/** Cef控件事件处理接口（分为回调函数和接口类两种，使用时可以二选一）
*  @copyright (c) 2016, NetEase Inc. All rights reserved
*  @author Redrain
*  @date 2016/7/22
*/
namespace ui
{
    //JS相关的回调函数
    typedef std::function<void(bool has_error, const std::string& result)> ReportResultFunction;
    typedef std::function<void(const std::string& result)> CallJsFunctionCallback;
    typedef std::function<void(const std::string& params, ReportResultFunction callback)> CppFunction;


    //浏览器控件相关的回调函数

    //导航前回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect)> OnBeforeBrowseEvent;

    //资源加载前回调函数（回调函数的调用线程：CEF的IO线程）
    typedef std::function<cef_return_value_t (CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefRequest> request,
                                              CefRefPtr<CefCallback> callback)> OnBeforeResourceLoadEvent;

    //资源重定向回调函数（回调函数的调用线程：CEF的IO线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                CefString& new_url)> OnResourceRedirectEvent;

    //资源收到回应回调函数（回调函数的调用线程：CEF的IO线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response)> OnResourceResponseEvent;

    //资源加载完成回调函数（回调函数的调用线程：CEF的IO线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                cef_urlrequest_status_t status,
                                int64_t received_content_length)> OnResourceLoadCompleteEvent;

    //资源执行协议回调函数（回调函数的调用线程：CEF的IO线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool& allow_os_execution)> OnProtocolExecutionEvent;


    //页面加载状态发生变化的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                bool isLoading,
                                bool canGoBack,
                                bool canGoForward)> OnLoadingStateChangeEvent;

    //页面开始加载的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                cef_transition_type_t transition_type)> OnLoadStartEvent;

    //页面加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                int httpStatusCode)> OnLoadEndEvent;

    //页面加载发生错误的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                cef_errorcode_t errorCode,
                                const DString& errorText,
                                const DString& failedUrl)> OnLoadErrorEvent;

    //Browser对象创建完成（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser)> OnAfterCreatedEvent;

    //Browser对象即将关闭（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser)> OnBeforeCloseEvent;

    //菜单弹出（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefContextMenuParams> params,
                                CefRefPtr<CefMenuModel> model)> OnBeforeContextMenuEvent;

    //执行了菜单命令（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefContextMenuParams> params,
                                int command_id,
                                cef_event_flags_t event_flags)> OnContextMenuCommandEvent;

    //菜单消失（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame)> OnContextMenuDismissedEvent;

    //标题变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                const DString& title)> OnTitleChangeEvent;

    //URL变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                const DString& url)> OnUrlChangeEvent;

    //主Frame的URL变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (const DString& oldUrl,
                                const DString& newUrl)> OnMainUrlChangeEvent;

    //FaviconURL变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                const std::vector<CefString>& icon_urls)> OnFaviconURLChangeEvent;

    //全屏状态变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                bool bFullscreen)> OnFullscreenModeChangeEvent;

    //状态信息变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                const DString& value)> OnStatusMessageEvent;

    //加载进度变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                double progress)> OnLoadingProgressChangeEvent;

    //多媒体访问情况变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                bool has_video_access,
                                bool has_audio_access)> OnMediaAccessChangeEvent;

    //点击了超级链接，即将弹出新窗口（回调函数的调用线程：CEF的UI线程）
    //由于部分编译器的std::placeholders最多支持10个占位符，所以参数要控制在最多10个
    struct BeforePopupEventParam
    {
        CefString target_frame_name;
        CefLifeSpanHandler::WindowOpenDisposition target_disposition;
        bool user_gesture;
        CefPopupFeatures popupFeatures;
    };
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                int popup_id,
                                const CefString& target_url,
                                const BeforePopupEventParam& param,
                                CefWindowInfo& windowInfo,
                                CefRefPtr<CefClient>& client,
                                CefBrowserSettings& settings,
                                CefRefPtr<CefDictionaryValue>& extra_info,
                                bool* no_javascript_access)> OnBeforePopupEvent;

    //弹出新窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                int popup_id)> OnBeforePopupAbortedEvent;

    //开发者工具的显示属性发生变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (bool bVisible, bool bPopup)> OnDevToolAttachedStateChangeEvent;

    //是否可以下载文件（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                const CefString& url,
                                const CefString& request_method)> OnCanDownloadEvent;

    //下载文件之前事件的回调函数，CEF109版本忽略返回值（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDownloadItem> download_item,
                                const CefString& suggested_name,
                                CefRefPtr<CefBeforeDownloadCallback> callback)> OnBeforeDownloadEvent;

    //下载文件信息更新事件的回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDownloadItem> download_item,
                                CefRefPtr<CefDownloadItemCallback> callback)> OnDownloadUpdatedEvent;

    //打开文件/保存文件/选择文件夹对话框的回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                cef_file_dialog_mode_t mode,
                                const CefString& title,
                                const CefString& default_file_path,
                                const std::vector<CefString>& accept_filters,
                                const std::vector<CefString>& accept_extensions,
                                const std::vector<CefString>& accept_descriptions,
                                CefRefPtr<CefFileDialogCallback> callback)> OnFileDialogEvent;

    //主框架的文档加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser)> OnDocumentAvailableInMainFrameEvent;

    //网站图标下载完成事件（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                const CefString& image_url,
                                int http_status_code,
                                CefRefPtr<CefImage> image)> OnDownloadFavIconFinishedEvent;

    //CefDragHandler接口
    //拖动操作（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                CefDragHandler::DragOperationsMask mask)> OnDragEnterEvent;
    //可拖动区域发生变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                const std::vector<CefDraggableRegion>& regions)> OnDraggableRegionsChangedEvent;

}

namespace ui
{

/** Cef控件事件处理接口
*/
class CefControlEvent
{
public:
    /** Browser对象创建完成（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) {}

    /** Browser对象即将关闭（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) {}

    /** 菜单弹出（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) {}

    /** 执行了菜单命令（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      cef_event_flags_t event_flags)  { return false; }
    
    /** 菜单消失（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) {}

    /** 标题变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title) {}
    
    /** URL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url) {}
    
    /** 主Frame的URL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnMainUrlChange(const DString& oldUrl, const DString& newUrl) {}
    
    /** FaviconURL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) {}
        
    /** 全屏状态变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool bFullscreen) {}
    
    /** 状态信息变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value) {}
    
    /** 加载进度变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) {}
    
    /** 多媒体访问情况变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access) {}

    /** 点击了超级链接，即将弹出新窗口（回调函数的调用线程：CEF的UI线程）
    */
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
                               bool* no_javascript_access) { return true; }

    /** 弹出新窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id) {}

    /** 导航前回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect) { return false; }

    /** 资源加载前回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefFrame> frame,
                                                    CefRefPtr<CefRequest> request,
                                                    CefRefPtr<CefCallback> callback) { return RV_CONTINUE; }

    /** 资源重定向回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url) {}
    
    /** 资源收到回应回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) { return false; }

    /** 资源加载完成回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length) {}

    /** 资源执行协议回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution) {}

    /** 页面加载状态发生变化的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) {}
    
    /** 页面开始加载的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type) {}
    
    /** 页面加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) {}
    
    /** 页面加载发生错误的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             cef_errorcode_t errorCode,
                             const DString& errorText,
                             const DString& failedUrl) {}

    /** 开发者工具的显示属性发生变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDevToolAttachedStateChange(bool bVisible) {}

    /** 是否可以下载文件（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method) { return true; }

    /** 下载文件之前事件的回调函数，CEF109版本忽略返回值（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) { return true; }

    /** 下载文件信息更新事件的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) {}

    /** 打开文件/保存文件/选择文件夹对话框的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              cef_file_dialog_mode_t mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback) { return false; }

    /** 主框架的文档加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) {}

    /** 网站图标下载完成事件（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDownloadFavIconFinished(CefRefPtr<CefBrowser> browser,
                                           const CefString& image_url,
                                           int http_status_code,
                                           CefRefPtr<CefImage> image) {}

    //CefDragHandler接口
    //拖动操作（回调函数的调用线程：CEF的UI线程）
    virtual bool OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask) { return false; };

    //可拖动区域发生变化（回调函数的调用线程：主进程的UI线程）
    virtual void OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions) {};

public:
    virtual ~CefControlEvent() = default;
};

}

#pragma warning (pop)

#endif //UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
