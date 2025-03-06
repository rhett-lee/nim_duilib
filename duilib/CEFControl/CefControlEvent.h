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

    typedef std::function<CefResourceRequestHandler::ReturnValue(CefRefPtr<CefRequest> request, bool is_redirect)> OnBeforeResourceLoadEvent;
    

    typedef std::function<void(bool isLoading, bool canGoBack, bool canGoForward)> OnLoadingStateChangeEvent;
    typedef std::function<void()> OnLoadStartEvent;
    typedef std::function<void(int httpStatusCode)> OnLoadEndEvent;
    typedef std::function<void(CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)> OnLoadErrorEvent;
    typedef std::function<void(bool visible)> OnDevToolAttachedStateChangeEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser)> OnAfterCreatedEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser)> OnBeforeCloseEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)> OnBeforeBrowseEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)> OnProtocolExecutionEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)> OnBeforeDownloadEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)> OnDownloadUpdatedEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser,
                               CefBrowserHost::FileDialogMode mode,
                               const CefString& title,
                               const CefString& default_file_path,
                               const std::vector<CefString>& accept_filters,
                               const std::vector<CefString>& accept_extensions,
                               const std::vector<CefString>& accept_descriptions,
                               CefRefPtr<CefFileDialogCallback> callback)> OnFileDialogEvent;
}

#endif //UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
