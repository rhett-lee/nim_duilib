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
#pragma warning (pop)

namespace ui
{
/** CefBrowserHandler的消息委托类接口，CefBrowserHandler类会处理多数Cef浏览器对象的事件
*  其中一些需要与上层交互的事件接口封装到此类中，CefBrowserHandler把这些事件传递到委托接口中
*  可以根据需求来扩展此接口
*/
class CefBrowserHandlerDelegate : public virtual SupportWeakCallback
{
public:

    // 在非UI线程中被调用
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        CefRenderHandler::PaintElementType type,
        const CefRenderHandler::RectList& dirtyRects,
        const std::string* buffer,
        int width,
        int height) = 0;

    virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) = 0;

    virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) = 0;

    virtual void ClientToControl(POINT &pt) = 0;

    virtual void UpdateWindowPos() = 0;

    // 在非UI线程中被调用
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) = 0;

    // 在非UI线程中被调用
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        int command_id,
        CefContextMenuHandler::EventFlags event_flags) = 0;

    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser,    CefRefPtr<CefFrame> frame, const CefString& url) = 0;

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) = 0;

    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) = 0;

    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,    CefRefPtr<CefFrame> frame) = 0;

    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) = 0;

    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefLoadHandler::ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) = 0;

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
        bool* no_javascript_access) = 0;

    virtual bool OnAfterCreated(CefRefPtr<CefBrowser> browser) = 0;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) = 0;

    // 在非UI线程中被调用
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect) = 0;
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution) = 0;

    // 在非UI线程中被调用
    virtual cef_return_value_t OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefCallback> callback) = 0;

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, CefRequestHandler::TerminationStatus status) = 0;

    // 文件下载相关
    virtual void OnBeforeDownload(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        const CefString& suggested_name,
        CefRefPtr<CefBeforeDownloadCallback> callback) = 0;

    virtual void OnDownloadUpdated(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDownloadItem> download_item,
        CefRefPtr<CefDownloadItemCallback> callback) = 0;

    // 打开文件 Dialog
    virtual bool OnFileDialog(
        CefRefPtr<CefBrowser> browser,
        CefBrowserHost::FileDialogMode mode,
        const CefString& title,
        const CefString& default_file_path,
        const std::vector<CefString>& accept_filters,
        int selected_accept_filter,
        CefRefPtr<CefFileDialogCallback> callback) = 0;

    // 封装一些 JS 与 C++ 交互的功能
    virtual bool OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) = 0;

    virtual bool OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string) = 0;
};

}//namespace ui

#endif //UI_CEF_CONTROL_BROWSER_HANDLER_DELEGATE_H_
