#include "CefControl.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePathUtil.h"

#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"
#include "duilib/CEFControl/CefManager.h"
#include <thread>

namespace ui {

CefControl::CefControl(ui::Window* pWindow):
    ui::Control(pWindow),
    m_bAttachedDevTools(false),
    m_bDevToolsPopup(false),
    m_pDevToolsView(nullptr),
    m_bEnableF12(true),
    m_bDownloadFaviconImage(false),
    m_bUrlIsLocalFile(false)
{
    //这个标记必须为false，否则绘制有问题
    SetUseCache(false);

    //默认开启拖放操作
    BaseClass::SetEnableDragDrop(true);
}

CefControl::~CefControl(void)
{
}

DString CefControl::GetType() const { return DUI_CTR_CEF; }

void CefControl::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("url")) {
        //初始化加载的URL
        SetInitURL(strValue);
    }
    else if (strName == _T("url_is_local_file")) {
        //初始化加载的URL是否为本地文件
        SetInitUrlIsLocalFile(strValue == _T("true"));
    }
    else if (strName == _T("F12")) {
        //是否允许按F12打开开发者工具
        SetEnableF12(strValue == _T("true"));
    }
    else if (strName == _T("download_favicon_image")) {
        //是否下载网站的FavIcon图标
        SetDownloadFaviconImage(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }    
}

void CefControl::LoadURL(const CefString& url)
{
    if (url.empty()) {
        return;
    }
    //该函数必须在主线程执行, 如果在其他线程中调用，则转发到主线程执行
    if (!GlobalManager::Instance().IsInUIThread()) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControl::LoadURL, this, url));
        return;
    }

    //在主线程中执行该函数
    if ((m_pBrowserHandler != nullptr) && (m_pBrowserHandler->GetBrowser() != nullptr)) {
        CefRefPtr<CefFrame> frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        ASSERT(frame != nullptr);
        if (frame != nullptr) {
            frame->LoadURL(url);
        }        
    }
    else {
        if ((m_pBrowserHandler != nullptr)) {
            ui::StdClosure cb = ToWeakCallback([this, url]() {
                LoadURL(url);
            });
            m_pBrowserHandler->AddAfterCreateTask(cb);
        }
    }
}

void CefControl::GoBack()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GoBack();
    }
}

void CefControl::GoForward()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GoForward();
    }
}

bool CefControl::CanGoBack()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->CanGoBack();
    }
    return false;
}

bool CefControl::CanGoForward()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->CanGoForward();
    }
    return false;
}

void CefControl::Refresh()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->Reload();
    }
}

void CefControl::StopLoad()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->StopLoad();
    }
}

bool CefControl::IsLoading()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->IsLoading();
    }
    return false;
}

void CefControl::StartDownload(const CefString& url)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        m_pBrowserHandler->GetBrowser()->GetHost()->StartDownload(url);
    }
}

void CefControl::SetZoomLevel(float zoom_level)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        m_pBrowserHandler->GetBrowser()->GetHost()->SetZoomLevel(zoom_level);
    }
}

CefWindowHandle CefControl::GetCefWindowHandle() const
{
    if (m_pBrowserHandler.get()) {
        return m_pBrowserHandler->GetCefWindowHandle();
    }
    return 0;
}

CefString CefControl::GetURL()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return m_pBrowserHandler->GetBrowser()->GetMainFrame()->GetURL();
    }
    return CefString();
}

std::string CefControl::GetUTF8URL()
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get()) {
        return ui::StringConvert::WStringToUTF8((const wchar_t*)GetURL().c_str());
    }
    return CefString();
}

CefString CefControl::GetMainURL(const CefString& url)
{
    std::string temp = url.ToString();
    size_t end_pos = temp.find("#") == std::string::npos ? temp.length() : temp.find("#");
    temp = temp.substr(0, end_pos);
    return CefString(temp.c_str());
}

bool CefControl::RegisterCppFunc(const DString& function_name, ui::CppFunction function, bool global_function/* = false*/)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        return m_jsBridge->RegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), function, global_function ? nullptr : m_pBrowserHandler->GetBrowser());
    }
    return false;
}

void CefControl::UnRegisterCppFunc(const DString& function_name)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        m_jsBridge->UnRegisterCppFunc(ui::StringConvert::TToUTF8(function_name).c_str(), m_pBrowserHandler->GetBrowser());
    }
}

bool CefControl::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const DString& frame_name /*= _T("")*/)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        CefRefPtr<CefFrame> frame;
        if (frame_name.empty()) {
            frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrame(frame_name);
#else
            //CEF 高版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrameByName(frame_name);
#endif
        }

        if (!m_jsBridge->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback)) {
            return false;
        }
        return true;
    }
    return false;
}

bool CefControl::CallJSFunction(const DString& js_function_name, const DString& params, ui::CallJsFunctionCallback callback, const CefString& frame_id)
{
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() && m_jsBridge.get()) {
        CefRefPtr<CefFrame> frame;
        if (frame_id.empty()) {
            frame = m_pBrowserHandler->GetBrowser()->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrame(StringUtil::StringToInt64(frame_id.c_str()));
#else
            //CEF 高版本
            frame = m_pBrowserHandler->GetBrowser()->GetFrameByIdentifier(frame_id);
#endif
        }
        if (!m_jsBridge->CallJSFunction(ui::StringConvert::TToUTF8(js_function_name).c_str(),
            ui::StringConvert::TToUTF8(params).c_str(), frame, callback)) {
            return false;
        }
        return true;
    }
    return false;
}


bool CefControl::OnExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (m_jsBridge.get()) {
        m_jsCallbackThreadId = ui::GlobalManager::Instance().Thread().GetCurrentThreadIdentifier();
        return m_jsBridge->ExecuteCppFunc(function_name, params, js_callback_id, browser, frame);
    }
    return false;
}

bool CefControl::OnExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string)
{
    if (m_jsBridge.get()) {
        if (m_jsCallbackThreadId != -1) {
            ui::GlobalManager::Instance().Thread().PostTask(m_jsCallbackThreadId, [this, cpp_callback_id, json_string]
                {
                    m_jsBridge->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
                });
        }
        else {
            return m_jsBridge->ExecuteCppCallbackFunc(cpp_callback_id, json_string);
        }

    }
    return false;
}

void CefControl::RepairBrowser()
{
    ReCreateBrowser();
}

void CefControl::CloseAllBrowsers()
{
    DoCloseAllBrowsers(true);
}

void CefControl::DoCloseAllBrowsers(bool bForceClose)
{
    if (m_pBrowserHandler != nullptr) {
        m_pBrowserHandler->CloseAllBrowsers(bForceClose);
    }
}

ControlDropTarget_Windows* CefControl::GetControlDropTarget()
{
    if (IsEnableDragDrop() && IsEnabled() && (m_pBrowserHandler != nullptr)) {
        return m_pBrowserHandler->GetControlDropTarget();
    }
    return nullptr;
}

void CefControl::OnHostWindowClosed()
{
    CloseAllBrowsers();
    if (m_pBrowserHandler != nullptr) {
        m_pBrowserHandler->SetHostWindowClosed(true);
    }
    //释放一次CPU时间片，让CEF UI线程执行
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void CefControl::ResetDevToolAttachedState()
{
    bool bAttachedDevTools = m_bAttachedDevTools;
    m_bAttachedDevTools = false;
    if (bAttachedDevTools) {
        OnDevToolVisibleChanged(false, m_bDevToolsPopup);
    }
}

void CefControl::SetDevToolsView(CefControl* pDevToolsView)
{
    if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //离屏渲染模式
        m_pDevToolsView = pDevToolsView;
        m_pDevToolsViewFlag.reset();
        if (pDevToolsView != nullptr) {
            m_pDevToolsViewFlag = pDevToolsView->GetWeakFlag();
            pDevToolsView->SetEnableF12(false);
        }        
    }
    else {
        //子窗口模式
        m_pDevToolsView = nullptr;
        m_pDevToolsViewFlag.reset();
    }
}

void CefControl::SetEnableF12(bool bEnableF12)
{
    m_bEnableF12 = bEnableF12;
}

bool CefControl::IsEnableF12() const
{
    return m_bEnableF12;
}

void CefControl::SetDownloadFaviconImage(bool bDownload)
{
    m_bDownloadFaviconImage = bDownload;
}

bool CefControl::IsDownloadFaviconImage() const
{
    return m_bDownloadFaviconImage;
}

void CefControl::SetInitURL(const DString& url)
{
    m_initUrl = url;
}

DString CefControl::GetInitURL() const
{
    DString initUrl = m_initUrl.c_str();
    if (IsInitUrlIsLocalFile() && !initUrl.empty()) {
        //该URL是本地路径
        DString url = StringUtil::MakeLowerString(initUrl);
        if ((url.find(_T("http://")) != 0) && (url.find(_T("https://")) != 0) && (url.find(_T("file:///")) != 0)) {
            //有明确的协议前缀时，不做任何转换，否则按照本地exe所在路径的资源文件加载
            FilePath cefHtml = GlobalManager::GetDefaultResourcePath(true);
            cefHtml.NormalizeDirectoryPath();
            cefHtml += initUrl;            
            cefHtml.NormalizeFilePath();
            initUrl = _T("file:///");
            initUrl += cefHtml.ToString();
            StringUtil::ReplaceAll(_T("\\"), _T("/"), initUrl);
        }
    }
    return initUrl;
}

void CefControl::SetInitUrlIsLocalFile(bool bUrlIsLocalFile)
{
    m_bUrlIsLocalFile = bUrlIsLocalFile;
}

bool CefControl::IsInitUrlIsLocalFile() const
{
    return m_bUrlIsLocalFile;
}

class DevToolBrowserHandler: public CefBrowserHandler
{
public:
    explicit DevToolBrowserHandler(CefControl* pCefControl) :
        m_pCefControl(pCefControl)
    {
        if (pCefControl) {
            m_pCefControlFlag = pCefControl->GetWeakFlag();
        }
    }
    virtual ~DevToolBrowserHandler() override
    {
        //窗口关闭后，发出一个通知
        if (!m_pCefControlFlag.expired() && (m_pCefControl != nullptr)) {
            CefControl* pCefControl = m_pCefControl;
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, pCefControl->ToWeakCallback([pCefControl]() {
                if (pCefControl->IsAttachedDevTools()) {
                    pCefControl->ResetDevToolAttachedState();
                }
                }));
        }
    }

private:
    //关联的CEF控件接口
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;
};

bool CefControl::AttachDevTools()
{
    if (IsAttachedDevTools()) {
        return true;
    }
    if (m_pBrowserHandler == nullptr) {
        return false;
    }

    auto browser = m_pBrowserHandler->GetBrowser();
    if (browser == nullptr) {
        auto task = ToWeakCallback([this]() {
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this]() {
                AttachDevTools();
                }));
            });
        m_pBrowserHandler->AddAfterCreateTask(task);
    }
    else {
#if CEF_VERSION_MAJOR > 109
        bool bPopup = true;  //CEF 133 只支持弹出式的窗口显示开发者工具
#else
        bool bPopup = false; //CEF 109 支持将开发者工具内嵌在一个Browser控件中显示
#endif

        CefWindowInfo windowInfo;
        CefBrowserSettings settings;
        if (browser->GetHost() != nullptr) {
            CefRefPtr<CefBrowserHost> viewBrowserHost;
            if (!bPopup) {
                CefRefPtr<CefBrowser> viewBrowser;
                if (!m_pDevToolsViewFlag.expired() && (m_pDevToolsView != nullptr) && (m_pDevToolsView->m_pBrowserHandler != nullptr)) {
                    viewBrowser = m_pDevToolsView->m_pBrowserHandler->GetBrowser();
                }
                if (viewBrowser != nullptr) {
                    viewBrowserHost = viewBrowser->GetHost();
                }
            }

            if (viewBrowserHost != nullptr) {
                //内嵌在一个Browser对象中显示
#ifdef DUILIB_BUILD_FOR_WIN
                ASSERT(viewBrowserHost->GetWindowHandle() != nullptr);
                windowInfo.SetAsWindowless(viewBrowserHost->GetWindowHandle());
#endif
                browser->GetHost()->ShowDevTools(windowInfo, viewBrowserHost->GetClient(), settings, CefPoint());
                SetAttachedDevTools(true, false);
            }
            else {
                //弹出式窗口显示
#ifdef DUILIB_BUILD_FOR_WIN
                windowInfo.SetAsPopup(nullptr, _T("cef_devtools"));
#endif
                browser->GetHost()->ShowDevTools(windowInfo, new DevToolBrowserHandler(this), settings, CefPoint());
                SetAttachedDevTools(true, true);
            }
        }
    }
    return true;
}

void CefControl::DettachDevTools()
{
    bool bAttachedDevTools = m_bAttachedDevTools;
    m_bAttachedDevTools = false;

    auto browser = m_pBrowserHandler->GetBrowser();
    if ((browser != nullptr) && (browser->GetHost() != nullptr)) {
        browser->GetHost()->CloseDevTools();
    }

    if (bAttachedDevTools) {
        OnDevToolVisibleChanged(false, m_bDevToolsPopup);
    }
}

bool CefControl::IsAttachedDevTools() const
{
    return m_bAttachedDevTools;
}

void CefControl::SetAttachedDevTools(bool bAttachedDevTools, bool bPopup)
{
    bool bChanged = m_bAttachedDevTools != bAttachedDevTools;
    m_bAttachedDevTools = bAttachedDevTools;
    m_bDevToolsPopup = bPopup;
    if (bChanged) {
        OnDevToolVisibleChanged(m_bAttachedDevTools, bPopup);
    }
}

void CefControl::OnDevToolVisibleChanged(bool bAttachedDevTools, bool bPopup)
{
    if (m_pfnDevToolVisibleChange != nullptr) {
        if (CefCurrentlyOn(TID_UI)) {
            //当前在CEF的UI线程, 转接到主进程的UI线程
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControl::OnDevToolVisibleChanged, this, bAttachedDevTools, bPopup));
        }
        else {
            m_pfnDevToolVisibleChange(bAttachedDevTools, bPopup);
        }
    }
}

void CefControl::OnPaint(CefRefPtr<CefBrowser> /*browser*/, CefRenderHandler::PaintElementType /*type*/, const CefRenderHandler::RectList& /*dirtyRects*/, const void* /*buffer*/, int /*width*/, int /*height*/)
{
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);
}

void CefControl::ClientToControl(UiPoint&/*pt*/)
{
}

void CefControl::OnPopupShow(CefRefPtr<CefBrowser> /*browser*/, bool /*show*/)
{
}

void CefControl::OnPopupSize(CefRefPtr<CefBrowser> /*rowser*/, const CefRect& /*rect*/)
{
}

void CefControl::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> /*browser*/, const CefRange& /*selected_range*/, const std::vector<CefRect>& /*character_bounds*/)
{
}

void CefControl::UpdateCefWindowPos()
{
    GlobalManager::Instance().AssertUIThread();
}

void CefControl::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnBeforeContextMenu) {
        m_pfnBeforeContextMenu(browser, frame, params, model);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforeContextMenu(browser, frame, params, model);
    }
}

bool CefControl::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnContextMenuCommand) {
        return m_pfnContextMenuCommand(browser, frame, params, command_id, event_flags);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnContextMenuCommand(browser, frame, params, command_id, event_flags);
    }
    return false;
}

void CefControl::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnContextMenuDismissed) {
        m_pfnContextMenuDismissed(browser, frame);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnContextMenuDismissed(browser, frame);
    }
}

void CefControl::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnUrlChange) {
        m_pfnUrlChange(browser, frame, url);
    }

    if ((frame != nullptr) && frame->IsMain()) {
        DString oldUrl = m_url.c_str();
        DString newUrl = frame->GetURL();
        m_url = newUrl;
        if (m_pfnMainUrlChange != nullptr && GetMainURL(oldUrl).compare(GetMainURL(newUrl)) != 0) {
            m_pfnMainUrlChange(oldUrl, newUrl);
        }
    }
    if (!m_pfnUrlChange && m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnUrlChange(browser, frame, url);
    }
}

void CefControl::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnTitleChange) {
        m_pfnTitleChange(browser, title);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnTitleChange(browser, title);
    }
}

class CefControlDownloadImageCallback : public CefDownloadImageCallback {
public:
    explicit CefControlDownloadImageCallback(CefControl* pCefControl):
        m_pCefControl(pCefControl)
    {
        if (pCefControl != nullptr) {
            m_pCefControlFlag = pCefControl->GetWeakFlag();
        }
    }

    void OnDownloadImageFinished(const CefString& image_url, int http_status_code, CefRefPtr<CefImage> image) override
    {
        if (image && !m_pCefControlFlag.expired() && (m_pCefControl != nullptr)) {
            m_pCefControl->OnDownloadImageFinished(image_url, http_status_code, image);
        }
    }

private:
    CefControl* m_pCefControl;
    std::weak_ptr<WeakFlag> m_pCefControlFlag;

    IMPLEMENT_REFCOUNTING(CefControlDownloadImageCallback);
    DISALLOW_COPY_AND_ASSIGN(CefControlDownloadImageCallback);
};

void CefControl::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
    GlobalManager::Instance().AssertUIThread();    
    if (m_bDownloadFaviconImage && !icon_urls.empty() && (browser != nullptr) && (browser->GetHost() != nullptr)) {
        //下载网站图标
        for (const CefString& iconUrl : icon_urls) {
            if (!iconUrl.empty()) {
                browser->GetHost()->DownloadImage(iconUrl, true, 32, false, new CefControlDownloadImageCallback(this));
                break;
            }
        }
    }
    if (m_pfnFaviconURLChange) {
        m_pfnFaviconURLChange(browser, icon_urls);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnFaviconURLChange(browser, icon_urls);
    }
}

void CefControl::OnDownloadImageFinished(const CefString& image_url,
                                         int http_status_code,
                                         CefRefPtr<CefImage> image)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (!GlobalManager::Instance().IsInUIThread()) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, image_url, http_status_code, image]() {
            m_favIconUrl = image_url;
            CefRefPtr<CefBrowser> browser = GetCefBrowser();
            if (m_pfnDownloadFavIconFinished) {
                m_pfnDownloadFavIconFinished(browser, image_url, http_status_code, image);
            }
            else if (m_pCefControlEventHandler) {
                m_pCefControlEventHandler->OnDownloadFavIconFinished(browser, image_url, http_status_code, image);
            }
            }));
    }
    
}

bool CefControl::ReDownloadFavIcon()
{
    ASSERT(GlobalManager::Instance().IsInUIThread());
    if (!m_favIconUrl.empty()) {
        CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
        if (browserHost != nullptr) {
            browserHost->DownloadImage(m_favIconUrl, true, 32, false, new CefControlDownloadImageCallback(this));
            return true;
        }
    }
    return false;
}

void CefControl::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnFullscreenModeChange) {
        m_pfnFullscreenModeChange(browser, fullscreen);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnFullscreenModeChange(browser, fullscreen);
    }
}

void CefControl::OnStatusMessage(CefRefPtr<CefBrowser> browser, const DString& value)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnStatusMessage) {
        m_pfnStatusMessage(browser, value);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnStatusMessage(browser, value);
    }
}

void CefControl::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnLoadingProgressChange) {
        m_pfnLoadingProgressChange(browser, progress);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadingProgressChange(browser, progress);
    }
}

void CefControl::OnMediaAccessChange(CefRefPtr<CefBrowser> browser, bool has_video_access, bool has_audio_access)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnMediaAccessChange) {
        m_pfnMediaAccessChange(browser, has_video_access, has_audio_access);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnMediaAccessChange(browser, has_video_access, has_audio_access);
    }
}

bool CefControl::OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask)
{
    bool bRet = false;
    if (m_pfnDragEnter) {
        bRet = m_pfnDragEnter(browser, dragData, mask);
    }
    else if (m_pCefControlEventHandler) {
        bRet = m_pCefControlEventHandler->OnDragEnter(browser, dragData, mask);
    }
    return bRet;
}

void CefControl::OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnDraggableRegionsChanged) {
        m_pfnDraggableRegionsChanged(browser, frame, regions);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnDraggableRegionsChanged(browser, frame, regions);
    }
}

void CefControl::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnLoadingStateChange) {
        m_pfnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
}

void CefControl::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, cef_transition_type_t transition_type)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnLoadStart) {
        m_pfnLoadStart(browser, frame, transition_type);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadStart(browser, frame, transition_type);
    }
}

void CefControl::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnLoadEnd) {
        m_pfnLoadEnd(browser, frame, httpStatusCode);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadEnd(browser, frame, httpStatusCode);
    }
}

void CefControl::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnLoadError) {
        m_pfnLoadError(browser, frame, errorCode, errorText, failedUrl);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnLoadError(browser, frame, errorCode, errorText, failedUrl);
    }
}

bool CefControl::OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
                                   bool* no_javascript_access)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnBeforePopup) {
        BeforePopupEventParam param;
        param.target_frame_name = target_frame_name;
        param.target_disposition = target_disposition;
        param.user_gesture = user_gesture;
        param.popupFeatures = popupFeatures;
        return m_pfnBeforePopup(browser, frame, popup_id, target_url, param, windowInfo,
                                client, settings, extra_info, no_javascript_access);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforePopup(browser, frame, popup_id, target_url, target_frame_name,
                                                        target_disposition, user_gesture, popupFeatures, windowInfo,
                                                        client, settings, extra_info, no_javascript_access);
    }
    return true;
}

void CefControl::OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnBeforePopupAborted) {
        return m_pfnBeforePopupAborted(browser, popup_id);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforePopupAborted(browser, popup_id);
    }
}

void CefControl::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnAfterCreated) {
        m_pfnAfterCreated(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnAfterCreated(browser);
    }
}

void CefControl::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnBeforeClose) {
        m_pfnBeforeClose(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnBeforeClose(browser);
    }
}

bool CefControl::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnBeforeBrowse) {
        return m_pfnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
    }
    return false;
}

cef_return_value_t CefControl::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    if (m_pfnBeforeResourceLoad) {
        return m_pfnBeforeResourceLoad(browser, frame, request, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeResourceLoad(browser, frame, request, callback);
    }
    return RV_CONTINUE;
}

void CefControl::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        CefString& new_url)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    if (m_pfnResourceRedirect) {
        m_pfnResourceRedirect(browser, frame, request, response, new_url);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnResourceRedirect(browser, frame, request, response, new_url);
    }
}

bool CefControl::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    CefRefPtr<CefResponse> response)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    if (m_pfnResourceResponse) {
        return m_pfnResourceResponse(browser, frame, request, response);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnResourceResponse(browser, frame, request, response);
    }
    return false;
}

void CefControl::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        cef_urlrequest_status_t status,
                                        int64_t received_content_length)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    if (m_pfnResourceLoadComplete) {
        m_pfnResourceLoadComplete(browser, frame, request, response, status, received_content_length);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnResourceLoadComplete(browser, frame, request, response, status, received_content_length);
    }
}

void CefControl::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefRequest> request,
                                     bool& allow_os_execution)
{
    ASSERT(CefCurrentlyOn(TID_IO));
    if (m_pfnProtocolExecution) {
        m_pfnProtocolExecution(browser, frame, request, allow_os_execution);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnProtocolExecution(browser, frame, request, allow_os_execution);
    }
}

void CefControl::OnRenderProcessTerminated(CefRefPtr<CefBrowser> /*browser*/,
                                               CefRequestHandler::TerminationStatus /*status*/,
                                               int /*error_code*/,
                                               CefString /*error_string*/)
{
}

void CefControl::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pfnDocumentAvailableInMainFrame) {
        m_pfnDocumentAvailableInMainFrame(browser);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnDocumentAvailableInMainFrame(browser);
    }
}

bool CefControl::OnCanDownload(CefRefPtr<CefBrowser> browser,
                               const CefString& url,
                               const CefString& request_method)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnCanDownload) {
        return m_pfnCanDownload(browser, url, request_method);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnCanDownload(browser, url, request_method);
    }
    return true;
}

bool CefControl::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnBeforeDownload) {
        return m_pfnBeforeDownload(browser, download_item, suggested_name, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnBeforeDownload(browser, download_item, suggested_name, callback);
    }
    return false;
}

void CefControl::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnDownloadUpdated) {
        m_pfnDownloadUpdated(browser, download_item, callback);
    }
    else if (m_pCefControlEventHandler) {
        m_pCefControlEventHandler->OnDownloadUpdated(browser, download_item, callback);
    }
}

bool CefControl::OnFileDialog(CefRefPtr<CefBrowser> browser,
                              CefBrowserHost::FileDialogMode mode,
                              const CefString& title,
                              const CefString& default_file_path,
                              const std::vector<CefString>& accept_filters,
                              const std::vector<CefString>& accept_extensions,
                              const std::vector<CefString>& accept_descriptions,
                              CefRefPtr<CefFileDialogCallback> callback)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_pfnFileDialog) {
        return m_pfnFileDialog(browser, mode, title, default_file_path, accept_filters, accept_extensions, accept_descriptions, callback);
    }
    else if (m_pCefControlEventHandler) {
        return m_pCefControlEventHandler->OnFileDialog(browser, mode, title, default_file_path, accept_filters, accept_extensions, accept_descriptions, callback);
    }
    return false;        
}

bool CefControl::OnPreKeyEvent(CefRefPtr<CefBrowser> /*browser*/,
                               const CefKeyEvent& event,
                               CefEventHandle /*os_event*/,
                               bool* /*is_keyboard_shortcut*/)
{
    if (event.type == KEYEVENT_RAWKEYDOWN) {
        if (event.windows_key_code == kVK_F12) {
            if (m_bEnableF12) {
                //按F12，显示开发者工具
                if (IsAttachedDevTools()) {
                    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControl::DettachDevTools, this));
                }
                else {
                    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControl::AttachDevTools, this));
                }
            }
            //拦截该快捷键
            return true;
        }
    }
    return false;
}

bool CefControl::OnKeyEvent(CefRefPtr<CefBrowser> /*browser*/,
                            const CefKeyEvent& /*event*/,
                            CefEventHandle /*os_event*/)
{
    return false;
}

void CefControl::SetCefEventHandler(CefControlEvent* pCefControlEventHandler)
{
    m_pCefControlEventHandler = pCefControlEventHandler;
}

CefControlEvent* CefControl::GetCefEventHandler() const
{
    return m_pCefControlEventHandler;
}

CefRefPtr<CefBrowser> CefControl::GetCefBrowser()
{
    CefRefPtr<CefBrowser> browser;
    if (m_pBrowserHandler != nullptr) {
        browser = m_pBrowserHandler->GetBrowser();
    }
    return browser;
}

CefRefPtr<CefBrowserHost> CefControl::GetCefBrowserHost()
{
    CefRefPtr<CefBrowserHost> browserHost;
    if (m_pBrowserHandler != nullptr) {
        browserHost = m_pBrowserHandler->GetBrowserHost();
    }
    return browserHost;
}

bool CefControl::IsCallbackExists(CefCallbackID nCallbackID)
{
    if (m_pCefControlEventHandler != nullptr) {
        //该接口会接收所有的回调函数
        return true;
    }
    switch (nCallbackID) {
    case CefCallbackID::OnBeforeContextMenu:        return m_pfnBeforeContextMenu != nullptr;
    case CefCallbackID::OnContextMenuCommand:       return m_pfnContextMenuCommand != nullptr;
    case CefCallbackID::OnContextMenuDismissed:     return m_pfnContextMenuDismissed != nullptr;
    case CefCallbackID::OnTitleChange:              return m_pfnTitleChange != nullptr;
    case CefCallbackID::OnBeforeResourceLoad:       return m_pfnBeforeResourceLoad != nullptr;
    case CefCallbackID::OnResourceRedirect:         return m_pfnResourceRedirect != nullptr;
    case CefCallbackID::OnResourceResponse:         return m_pfnResourceResponse != nullptr;
    case CefCallbackID::OnResourceLoadComplete:     return m_pfnResourceLoadComplete != nullptr;
    case CefCallbackID::OnProtocolExecution:        return m_pfnProtocolExecution != nullptr;
    case CefCallbackID::OnAddressChange:            return m_pfnUrlChange != nullptr;
    case CefCallbackID::OnFaviconURLChange:         return m_pfnFaviconURLChange != nullptr;
    case CefCallbackID::OnFullscreenModeChange:     return m_pfnFullscreenModeChange != nullptr;
    case CefCallbackID::OnStatusMessage:            return m_pfnStatusMessage != nullptr;
    case CefCallbackID::OnLoadingProgressChange:    return m_pfnLoadingProgressChange != nullptr;
    case CefCallbackID::OnMediaAccessChange:        return m_pfnMediaAccessChange != nullptr;
    case CefCallbackID::OnBeforePopup:              return m_pfnBeforePopup != nullptr;
    case CefCallbackID::OnBeforePopupAborted:       return m_pfnBeforePopupAborted != nullptr;
    case CefCallbackID::OnLoadingStateChange:       return m_pfnLoadingStateChange != nullptr;
    case CefCallbackID::OnLoadStart:                return m_pfnLoadStart != nullptr;
    case CefCallbackID::OnLoadEnd:                  return m_pfnLoadEnd != nullptr;
    case CefCallbackID::OnLoadError:                return m_pfnLoadError != nullptr;
    case CefCallbackID::OnAfterCreated:             return m_pfnAfterCreated != nullptr;
    case CefCallbackID::OnBeforeClose:              return m_pfnBeforeClose != nullptr;
    case CefCallbackID::OnBeforeBrowse:             return m_pfnBeforeBrowse != nullptr;
    case CefCallbackID::OnCanDownload:              return m_pfnCanDownload != nullptr;
    case CefCallbackID::OnBeforeDownload:           return m_pfnBeforeDownload != nullptr;
    case CefCallbackID::OnDownloadUpdated:          return m_pfnDownloadUpdated != nullptr;
    case CefCallbackID::OnFileDialog:               return m_pfnFileDialog != nullptr;
    case CefCallbackID::OnDocumentAvailableInMainFrame: return m_pfnDocumentAvailableInMainFrame != nullptr;
    case CefCallbackID::OnDragEnter:                    return m_pfnDragEnter != nullptr;
    case CefCallbackID::OnDraggableRegionsChanged:      return m_pfnDraggableRegionsChanged != nullptr;
    default:
        break;
    }
    return false;
}

void CefControl::OnFocusedNodeChanged(CefRefPtr<CefBrowser> /*browser*/,
                                      CefRefPtr<CefFrame> /*frame*/,
                                      CefDOMNode::Type /*type*/,
                                      bool /*bText*/,
                                      bool /*bEditable*/,
                                      const CefRect& /*nodeRect*/)
{
}

void CefControl::OnCursorChange(cef_cursor_type_t /*type*/)
{
}

void CefControl::OnGotFocus()
{
    if (!IsVisible() || !IsEnabled()) {
        return;
    }
    if (!IsFocused()) {
        //避免双焦点控件的出现
        SetFocus();
    }
}

Control* CefControl::GetCefControl()
{
    return this;
}

std::shared_ptr<IBitmap> CefControl::MakeImageSnapshot()
{
    return nullptr;
}

} //namespace ui
