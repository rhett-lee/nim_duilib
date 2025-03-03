#include "BrowserBox.h"
#include "browser/MultiBrowserForm.h"
#include "browser/MultiBrowserManager.h"
#include "taskbar/TaskbarManager.h"

using namespace ui;
using namespace std;

BrowserBox::BrowserBox(ui::Window* pWindow, std::string id):
    ui::VBox(pWindow)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pTaskBarItem = nullptr;
#endif
    m_pBrowserForm = nullptr;
    m_pCefControl = nullptr;
    m_browserId = id;
}

MultiBrowserForm* BrowserBox::GetBrowserForm() const
{
    ASSERT(nullptr != m_pBrowserForm);
    ASSERT(m_pBrowserForm->IsWindow());
    return m_pBrowserForm;
}

ui::CefControlBase* BrowserBox::GetCefControl()
{
    return m_pCefControl;
}

DString& BrowserBox::GetTitle()
{
    return m_title;
}

void BrowserBox::InitBrowserBox(const DString &url)
{
    m_pCefControl = static_cast<ui::CefControlBase*>(FindSubControl(_T("cef_control")));
    m_pCefControl->AttachBeforeContextMenu(UiBind(&BrowserBox::OnBeforeMenu, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachMenuCommand(UiBind(&BrowserBox::OnMenuCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachTitleChange(UiBind(&BrowserBox::OnTitleChange, this, std::placeholders::_1));
    m_pCefControl->AttachUrlChange(UiBind(&BrowserBox::OnUrlChange, this, std::placeholders::_1));
    m_pCefControl->AttachLinkClick(UiBind(&BrowserBox::OnLinkClick, this, std::placeholders::_1));
    m_pCefControl->AttachBeforeNavigate(UiBind(&BrowserBox::OnBeforeNavigate, this, std::placeholders::_1, std::placeholders::_2));
    m_pCefControl->AttachLoadingStateChange(UiBind(&BrowserBox::OnLoadingStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_pCefControl->AttachLoadStart(UiBind(&BrowserBox::OnLoadStart, this));
    m_pCefControl->AttachLoadEnd(UiBind(&BrowserBox::OnLoadEnd, this, std::placeholders::_1));
    m_pCefControl->AttachLoadError(UiBind(&BrowserBox::OnLoadError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 加载默认网页
    DString html_path = url;
    if (html_path.empty()) {
        ui::FilePath localPath = ui::FilePathUtil::GetCurrentModuleDirectory();
        localPath.NormalizeDirectoryPath();
        localPath += _T("resources\\themes\\default\\cef\\cef.html");
        html_path = localPath.ToString();
    }
    m_pCefControl->LoadURL(html_path);

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    // 初始化任务栏缩略图
    if (GetWindow()->IsLayeredWindow()) {
        m_pTaskBarItem = new TaskbarTabItem(this);
        if (m_pTaskBarItem) {
            m_pTaskBarItem->Init(url, m_browserId);
        }
    }
#endif

    // Box获取焦点时把焦点转移给Cef控件
    this->AttachSetFocus([this](const ui::EventArgs& param)->bool
    {
        m_pCefControl->SetFocus();
        return true;
    }); 
}

void BrowserBox::UninitBrowserBox()
{
    MultiBrowserManager::GetInstance()->RemoveBorwserBox(m_browserId, this);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->UnInit();
    }
#endif
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
TaskbarTabItem* BrowserBox::GetTaskbarItem()
{
    return m_pTaskBarItem;
}
#endif

void BrowserBox::SetWindow(Window* pWindow)
{
    m_pBrowserForm = dynamic_cast<MultiBrowserForm*>(pWindow);
    ASSERT(nullptr != m_pBrowserForm);

    BaseClass::SetWindow(pWindow);
}

void BrowserBox::Invalidate()
{
    BaseClass::Invalidate();
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->InvalidateTab();
    }
#endif
}

void BrowserBox::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pTaskBarItem) {
        m_pTaskBarItem->InvalidateTab();
    }
#endif
}

void BrowserBox::OnBeforeMenu(CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{

}

bool BrowserBox::OnMenuCommand(CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags)
{
    return false;
}

void BrowserBox::OnTitleChange(const DString& title)
{
    m_title = title;
    m_pBrowserForm->SetTabItemName(ui::StringConvert::UTF8ToT(m_browserId), title);
}

void BrowserBox::OnUrlChange(const DString& url)
{
    m_url = url;
    m_pBrowserForm->SetURL(m_browserId, url);
}

bool BrowserBox::OnLinkClick(const DString& url)
{
    return true;
}

cef_return_value_t BrowserBox::OnBeforeNavigate(CefRefPtr<CefRequest> request, bool is_redirect)
{
    // 返回RV_CANCEL截断导航
    return RV_CONTINUE;
}

void BrowserBox::OnLoadingStateChange(bool isLoading, bool canGoBack, bool canGoForward)
{
    return;
}

void BrowserBox::OnLoadStart()
{
    return;
}

void BrowserBox::OnLoadEnd(int httpStatusCode)
{
    // 注册一个方法提供前端调用
    m_pCefControl->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([](const std::string& params, ui::ReportResultFunction callback) {
        ::MessageBoxW(nullptr, ui::StringConvert::UTF8ToWString(params).c_str(), L"接收到 JavaScript 发来的消息", MB_OK);
        callback(false, R"({ "message": "Success." })");
    }));
}

void BrowserBox::OnLoadError(CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    return;
}
