#ifndef EXAMPLES_CEF_FORM_H_
#define EXAMPLES_CEF_FORM_H_

// duilib
#include "duilib/duilib.h"

// CEF
#include "duilib/duilib_cef.h"

// 控制是否使用离屏渲染：
// 当为 true  时将使用默认窗口阴影方案，离屏渲染模式下窗口有 WS_EX_LAYERED 属性；
// 当为 false 时因使用了真窗口模式不支持带有 WS_EX_LAYERED 属性窗口，所以使用外置窗口阴影方案；
const bool kEnableOffScreenRendering = false;

/** CEF控件的窗口
*/
#if defined (DUILIB_BUILD_FOR_WIN)
class CefForm: public std::conditional<kEnableOffScreenRendering, ui::WindowImplBase, ui::ShadowWnd>::type,
               public ui::CefControlEvent
#else
class CefForm : public ui::WindowImplBase,
                public ui::CefControlEvent
#endif
{
public:
    CefForm();
    virtual ~CefForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnPreCloseWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

private:
    bool OnClicked(const ui::EventArgs& msg);
    bool OnNavigate(const ui::EventArgs& msg);

    /** 控制主进程单例的回调函数
    */
    void OnAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList);

private:
    /** Browser对象创建完成（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    /** Browser对象即将关闭（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    /** 菜单弹出（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefContextMenuParams> params,
                                     CefRefPtr<CefMenuModel> model) override;

    /** 执行了菜单命令（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefContextMenuParams> params,
                                      int command_id,
                                      cef_event_flags_t event_flags) override;
    
    /** 菜单消失（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;

    /** 标题变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const DString& title) override;
    
    /** URL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnUrlChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const DString& url) override;
    
    /** 主Frame的URL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnMainUrlChange(const DString& oldUrl, const DString& newUrl) override;
    
    /** FaviconURL变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;
        
    /** 全屏状态变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool bFullscreen) override;
    
    /** 状态信息变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value) override;
    
    /** 加载进度变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress) override;
    
    /** 多媒体访问情况变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access) override;

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
                               bool* no_javascript_access) override;

    /** 弹出新窗口失败的通知（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id) override;

    /** 导航前回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool is_redirect) override;

    /** 资源加载前回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefFrame> frame,
                                                    CefRefPtr<CefRequest> request,
                                                    CefRefPtr<CefCallback> callback) override;

    /** 资源重定向回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response,
                                    CefString& new_url) override;
    
    /** 资源收到回应回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response) override;

    /** 资源加载完成回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length) override;

    /** 资源执行协议回调函数（回调函数的调用线程：CEF的IO线程）
    */
    virtual void OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution) override;

    /** 页面加载状态发生变化的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward) override;
    
    /** 页面开始加载的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type) override;
    
    /** 页面加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    
    /** 页面加载发生错误的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             cef_errorcode_t errorCode,
                             const DString& errorText, const DString& failedUrl) override;

    /** 开发者工具的显示属性发生变化（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDevToolAttachedStateChange(bool bVisible) override;

    /** 是否可以下载文件（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method) override;

    /** 下载文件之前事件的回调函数，CEF109版本忽略返回值（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) override;

    /** 下载文件信息更新事件的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefDownloadItem> download_item,
                                   CefRefPtr<CefDownloadItemCallback> callback) override;

    /** 打开文件/保存文件/选择文件夹对话框的回调函数（回调函数的调用线程：CEF的UI线程）
    */
    virtual bool OnFileDialog(CefRefPtr<CefBrowser> browser,
                              cef_file_dialog_mode_t mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback) override;

    /** 主框架的文档加载完成的回调函数（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) override;

    /** 网站图标下载完成事件（回调函数的调用线程：主进程的UI线程）
    */
    virtual void OnDownloadFavIconFinished(CefRefPtr<CefBrowser> browser,
                                           const CefString& image_url,
                                           int http_status_code,
                                           CefRefPtr<CefImage> image) override;

private:
    /** 开发者工具的状态发生变化
    */
    void OnDevToolVisibleStateChanged(bool bVisible, bool bPopup);

private:
    ui::CefControl* m_pCefControl;
    ui::CefControl* m_pCefControlDev;
    ui::Button* m_pDevToolBtn;
    ui::RichEdit* m_pEditUrl;
};

#endif //EXAMPLES_CEF_FORM_H_
