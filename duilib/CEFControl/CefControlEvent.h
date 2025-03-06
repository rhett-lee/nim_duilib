#ifndef UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
#define UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_

#include "duilib/duilib_config.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_client.h"
#pragma warning (pop)

#include <functional>

/** Cef控件事件处理接口
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

    //资源加载前回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<cef_return_value_t (CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefRequest> request,
                                              CefRefPtr<CefCallback> callback)> OnBeforeResourceLoadEvent;

    //资源重定向回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                CefString& new_url)> OnResourceRedirectEvent;

    //资源收到回应回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response)> OnResourceResponseEvent;

    //资源加载完成回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                cef_urlrequest_status_t status,
                                int64_t received_content_length)> OnResourceLoadCompleteEvent;

    //资源执行协议回调函数（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                const CefString& url,
                                bool& allow_os_execution)> OnProtocolExecutionEvent;


    //页面加载状态发生变化的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)> OnLoadingStateChangeEvent;

    //页面开始加载的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type)> OnLoadStartEvent;

    //页面加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)> OnLoadEndEvent;

    //页面加载发生错误的回调函数（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_errorcode_t errorCode, const DString& errorText, const DString& failedUrl)> OnLoadErrorEvent;

    //Browser对象创建完成（回调函数的调用线程：主进程的UI线程）
    typedef std::function<bool(CefRefPtr<CefBrowser> browser)> OnAfterCreatedEvent;

    //Browser对象即将关闭（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void(CefRefPtr<CefBrowser> browser)> OnBeforeCloseEvent;

    //菜单弹出（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefContextMenuParams> params,
                                CefRefPtr<CefMenuModel> model)
                          > OnBeforeContextMenuEvent;

    //执行了菜单命令（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefContextMenuParams> params,
                                int command_id,
                                CefContextMenuHandler::EventFlags event_flags)> OnContextMenuCommandEvent;

    //菜单消失（回调函数的调用线程：CEF的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame)> OnContextMenuDismissedEvent;

    //标题变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser, const DString& title)> OnTitleChangeEvent;

    //URL变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url)> OnUrlChangeEvent;

    //主Frame的URL变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (const DString& oldUrl, const DString& newUrl)> OnMainUrlChangeEvent;

    //点击了超级链接，即将弹出新窗口（回调函数的调用线程：CEF的UI线程）
    typedef std::function<bool (CefRefPtr<CefBrowser> browser,
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
                                bool* no_javascript_access)> OnBeforePopupEvent;

    //弹出新窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (CefRefPtr<CefBrowser> browser, int popup_id)> OnBeforePopupAbortedEvent;

    //开发者工具的显示属性发生变化（回调函数的调用线程：主进程的UI线程）
    typedef std::function<void (bool bVisible)> OnDevToolAttachedStateChangeEvent;

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
}

#endif //UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
