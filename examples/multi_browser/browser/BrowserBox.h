#ifndef EXAMPLES_BROWSER_BOX_H_
#define EXAMPLES_BROWSER_BOX_H_

// duilib
#include "duilib/duilib.h"

// CEF
#include "duilib/duilib_cef.h"

/** @file browser_box.h
* @brief 标签页盒子
* @copyright (c) 2016, NetEase Inc. All rights reserved
* @author Redrain
* @date 2019/3/20
*/
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    class TaskbarTabItem;
#endif

class MultiBrowserForm;
class BrowserBox : public ui::VBox
{
    typedef ui::VBox BaseClass;
public:
    friend class MultiBrowserForm;

    /**
    * 构造函数
    * @param[in] id    浏览器盒子的唯一标识，用于区分不同的标签页
    */
    BrowserBox(ui::Window* pWindow, std::string id);
    ~BrowserBox() { };

    /**
    * 获取id
    * @return string 标识id
    */
    std::string GetId() const { return m_browserId; }

    /**
    * 获取本浏览器盒子所属的浏览器窗口指针
    * @return MultiBrowserForm* 浏览器窗口
    */
    MultiBrowserForm* GetBrowserForm() const;

    /**
    * 获取Cef控件指针
    * @return ui::CefControlBase* Cef控件
    */
    ui::CefControlBase* GetCefControl();

    /**
    * 获取网页标题
    * @return DString& 网页标题
    */
    DString& GetTitle();

    /**
    * 初始化浏览器盒子
    * @param[in] url 初始化URL
    * @return void    无返回值
    */
    virtual void InitBrowserBox(const DString &url);

    /**
    * 反初始化浏览器盒子
    * @return void    无返回值
    */
    virtual void UninitBrowserBox();

    //////////////////////////////////////////////////////////////////////////
    //窗口合并功能相关的操作
public:

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    /** 获取与本浏览器盒子绑定的Tab指针
    * @return TaskbarItem*    Tab指针
    */
    TaskbarTabItem* GetTaskbarItem();
#endif

    /**
    * 覆盖父类虚函数，用于指定本控件所属窗体
    * @param[in] pWindow 所属窗口指针
    */
    virtual void SetWindow(ui::Window* pWindow) override;

    /**
    * 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    * @return void    无返回值
    */
    virtual void Invalidate() override;

    /**
    * 覆盖基类虚函数，为了在重绘任务栏显示的缩略图
    * @param[in] rc 要设置的控件的位置
    * @return void    无返回值
    */
    virtual void SetPos(ui::UiRect rc) override;

private:
    void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefContextMenuParams> params,
                             CefRefPtr<CefMenuModel> model);
    bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefContextMenuParams> params,
                              int command_id,
                              CefContextMenuHandler::EventFlags event_flags);
    void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);

    void OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title);
    void OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url);
    void OnMainUrlChange(const DString& oldUrl, const DString& newUrl);

    bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
    void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id);

    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> request,
                        bool user_gesture,
                        bool is_redirect);


    cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefCallback> callback);

    void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            CefRefPtr<CefResponse> response,
                            CefString& new_url);
    bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            CefRefPtr<CefResponse> response);
    void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                cef_urlrequest_status_t status,
                                int64_t received_content_length);
    void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution);

    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward);
    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type);
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_errorcode_t errorCode, const DString& errorText, const DString& failedUrl);

    void OnDevToolAttachedStateChange(bool bVisible);

    bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                       const CefString& url,
                       const CefString& request_method);

    bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefDownloadItem> download_item,
                          const CefString& suggested_name,
                          CefRefPtr<CefBeforeDownloadCallback> callback);

    void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDownloadItem> download_item,
                           CefRefPtr<CefDownloadItemCallback> callback);

    bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                      cef_file_dialog_mode_t mode,
                      const CefString& title,
                      const CefString& default_file_path,
                      const std::vector<CefString>& accept_filters,
                      const std::vector<CefString>& accept_extensions,
                      const std::vector<CefString>& accept_descriptions,
                      CefRefPtr<CefFileDialogCallback> callback);


private:
    ui::CefControlBase* m_pCefControl;
    MultiBrowserForm* m_pBrowserForm;
    std::string m_browserId;
    DString m_url;
    DString m_title;

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    // 任务栏缩略图管理
    TaskbarTabItem* m_pTaskBarItem;
#endif
};

#endif //EXAMPLES_BROWSER_BOX_H_
