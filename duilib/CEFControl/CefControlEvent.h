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
    typedef std::function<void(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)> OnBeforeMenuEvent;
    typedef std::function<bool(CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)> OnMenuCommandEvent;

    typedef std::function<void(const DString& title)> OnTitleChangeEvent;
    typedef std::function<void(const DString& url)> OnUrlChangeEvent;

    typedef std::function<bool(const DString& url)> OnLinkClickEvent;
    typedef std::function<CefResourceRequestHandler::ReturnValue(CefRefPtr<CefRequest> request, bool is_redirect)> OnBeforeResourceLoadEvent;
    typedef std::function<void(const CefString& old_url, const CefString& new_url)> OnMainURLChengeEvent;

    typedef std::function<void(bool isLoading, bool canGoBack, bool canGoForward)> OnLoadingStateChangeEvent;
    typedef std::function<void()> OnLoadStartEvent;
    typedef std::function<void(int httpStatusCode)> OnLoadEndEvent;
    typedef std::function<void(CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)> OnLoadErrorEvent;
    typedef std::function<void(bool visible)> OnDevToolAttachedStateChangeEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser)> OnAfterCreatedEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser)> OnBeforeCloseEvent;

    typedef std::function<bool(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)> OnBeforeBrowseEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)> OnProtocolExecutionEvent;

    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)> OnBeforeDownloadEvent;
    typedef std::function<void(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)> OnDownloadUpdatedEvent;

    //TODO: 修正参数
    typedef std::function<bool(CefDialogHandler::FileDialogMode mode, const CefString& title, const CefString& default_file_path, const std::vector<CefString>& accept_filters, int selected_accept_filter, CefRefPtr<CefFileDialogCallback> callback)> OnFileDialogEvent;
}

#endif //UI_CEF_CONTROL_CEF_CONTROL_EVENT_H_
