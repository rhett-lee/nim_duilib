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
    * @return ui::CefControl* Cef控件
    */
    ui::CefControl* GetCefControl();

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

    /** Browser对象创建完成（回调函数的调用线程：主进程的UI线程）
    */
    void OnAfterCreated(CefRefPtr<CefBrowser> browser);

    /** Browser对象即将关闭（回调函数的调用线程：主进程的UI线程）
    */
    void OnBeforeClose(CefRefPtr<CefBrowser> browser);

    /** 菜单弹出（回调函数的调用线程：CEF的UI线程）
    */
    void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefContextMenuParams> params,
                             CefRefPtr<CefMenuModel> model);

    /** 执行了菜单命令（回调函数的调用线程：CEF的UI线程）
    */
    bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefContextMenuParams> params,
                              int command_id,
                              cef_event_flags_t event_flags);
    
    /** 菜单消失（回调函数的调用线程：CEF的UI线程）
    */
    void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame);

    /** 标题变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title);
    
    /** URL变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url);
    
    /** 主Frame的URL变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnMainUrlChange(const DString& oldUrl, const DString& newUrl);
    
    /** FaviconURL变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls);
        
    /** 全屏状态变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool bFullscreen);
    
    /** 状态信息变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value);
    
    /** 加载进度变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress);
    
    /** 多媒体访问情况变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access);

    /** 点击了超级链接，即将弹出新窗口（回调函数的调用线程：CEF的UI线程）
    */
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

    /** 弹出新窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    */
    void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id);

    /** 导航前回调函数（回调函数的调用线程：CEF的UI线程）
    */
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> request,
                        bool user_gesture,
                        bool is_redirect);

    /** 资源加载前回调函数（回调函数的调用线程：CEF的IO线程）
    */
    cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefCallback> callback);

    /** 资源重定向回调函数（回调函数的调用线程：CEF的IO线程）
    */
    void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            CefRefPtr<CefResponse> response,
                            CefString& new_url);
    
    /** 资源收到回应回调函数（回调函数的调用线程：CEF的IO线程）
    */
    bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                            CefRefPtr<CefFrame> frame,
                            CefRefPtr<CefRequest> request,
                            CefRefPtr<CefResponse> response);

    /** 资源加载完成回调函数（回调函数的调用线程：CEF的IO线程）
    */
    void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                CefRefPtr<CefResponse> response,
                                cef_urlrequest_status_t status,
                                int64_t received_content_length);

    /** 资源执行协议回调函数（回调函数的调用线程：CEF的IO线程）
    */
    void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution);

    /** 页面加载状态发生变化的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward);
    
    /** 页面开始加载的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type);
    
    /** 页面加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode);
    
    /** 页面加载发生错误的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     cef_errorcode_t errorCode,
                     const DString& errorText,
                     const DString& failedUrl);

    /** 开发者工具的显示属性发生变化（回调函数的调用线程：主进程的UI线程）
    */
    void OnDevToolAttachedStateChange(bool bVisible);

    /** 是否可以下载文件（回调函数的调用线程：CEF的UI线程）
    */
    bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                       const CefString& url,
                       const CefString& request_method);

    /** 下载文件之前事件的回调函数，CEF109版本忽略返回值（回调函数的调用线程：CEF的UI线程）
    */
    bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefDownloadItem> download_item,
                          const CefString& suggested_name,
                          CefRefPtr<CefBeforeDownloadCallback> callback);

    /** 下载文件信息更新事件的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefDownloadItem> download_item,
                           CefRefPtr<CefDownloadItemCallback> callback);

    /** 打开文件/保存文件/选择文件夹对话框的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                      cef_file_dialog_mode_t mode,
                      const CefString& title,
                      const CefString& default_file_path,
                      const std::vector<CefString>& accept_filters,
                      const std::vector<CefString>& accept_extensions,
                      const std::vector<CefString>& accept_descriptions,
                      CefRefPtr<CefFileDialogCallback> callback);

    /** 主框架的文档加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser);

private:
    ui::CefControl* m_pCefControl;
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
