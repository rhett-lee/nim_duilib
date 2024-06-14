#include "browser_box.h"
#include "browser/multi_browser_form.h"
#include "browser/multi_browser_manager.h"
#include "taskbar/taskbar_manager.h"

using namespace ui;
using namespace std;

BrowserBox::BrowserBox(ui::Window* pWindow, std::string id):
    ui::VBox(pWindow)
{
    taskbar_item_ = nullptr;
    browser_form_ = nullptr;
    cef_control_ = nullptr;
    browser_id_ = id;
}

MultiBrowserForm* BrowserBox::GetBrowserForm() const
{
    ASSERT(NULL != browser_form_);
    ASSERT(browser_form_->IsWindow());
    return browser_form_;
}

nim_comp::CefControlBase* BrowserBox::GetCefControl()
{
    return cef_control_;
}

DString& BrowserBox::GetTitle()
{
    return title_;
}

void BrowserBox::InitBrowserBox(const DString &url)
{
    cef_control_ = static_cast<nim_comp::CefControlBase*>(FindSubControl(_T("cef_control")));
    cef_control_->AttachBeforeContextMenu(UiBind(&BrowserBox::OnBeforeMenu, this, std::placeholders::_1, std::placeholders::_2));
    cef_control_->AttachMenuCommand(UiBind(&BrowserBox::OnMenuCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    cef_control_->AttachTitleChange(UiBind(&BrowserBox::OnTitleChange, this, std::placeholders::_1));
    cef_control_->AttachUrlChange(UiBind(&BrowserBox::OnUrlChange, this, std::placeholders::_1));
    cef_control_->AttachLinkClick(UiBind(&BrowserBox::OnLinkClick, this, std::placeholders::_1));
    cef_control_->AttachBeforeNavigate(UiBind(&BrowserBox::OnBeforeNavigate, this, std::placeholders::_1, std::placeholders::_2));
    cef_control_->AttachLoadingStateChange(UiBind(&BrowserBox::OnLoadingStateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    cef_control_->AttachLoadStart(UiBind(&BrowserBox::OnLoadStart, this));
    cef_control_->AttachLoadEnd(UiBind(&BrowserBox::OnLoadEnd, this, std::placeholders::_1));
    cef_control_->AttachLoadError(UiBind(&BrowserBox::OnLoadError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 加载默认网页
    DString html_path = url;
    if (html_path.empty())
        html_path = ui::PathUtil::GetCurrentModuleDirectory() + _T("resources\\themes\\default\\cef\\cef.html");

    cef_control_->LoadURL(html_path);

    // 初始化任务栏缩略图
    if (GetWindow()->IsLayeredWindow()) {
        taskbar_item_ = new TaskbarTabItem(this);
        if (taskbar_item_) {
            taskbar_item_->Init(url, browser_id_);
        }
    }

    // Box获取焦点时把焦点转移给Cef控件
    this->AttachSetFocus([this](const ui::EventArgs& param)->bool
    {
        cef_control_->SetFocus();
        return true;
    }); 
}

void BrowserBox::UninitBrowserBox()
{
    MultiBrowserManager::GetInstance()->RemoveBorwserBox(browser_id_, this);

    if (taskbar_item_)
        taskbar_item_->UnInit();
}

ui::Control* BrowserBox::CreateControl(const DString& pstrClass)
{
    if (pstrClass == _T("CefControl"))
    {
        if (nim_comp::CefManager::GetInstance()->IsEnableOffsetRender())
            return new nim_comp::CefControl(GetWindow());
        else
            return new nim_comp::CefNativeControl(GetWindow());
    }

    return NULL;
}

TaskbarTabItem* BrowserBox::GetTaskbarItem()
{
    return taskbar_item_;
}

void BrowserBox::SetWindow(Window* pManager)
{
    browser_form_ = dynamic_cast<MultiBrowserForm*>(pManager);
    ASSERT(NULL != browser_form_);

    __super::SetWindow(pManager);
}

void BrowserBox::Invalidate()
{
    __super::Invalidate();

    if (taskbar_item_)
        taskbar_item_->InvalidateTab();
}

void BrowserBox::SetPos(UiRect rc)
{
    __super::SetPos(rc);

    if (taskbar_item_)
        taskbar_item_->InvalidateTab();
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
    title_ = title;
    browser_form_->SetTabItemName(ui::StringUtil::UTF8ToUTF16(browser_id_), title);
}

void BrowserBox::OnUrlChange(const DString& url)
{
    url_ = url;
    browser_form_->SetURL(browser_id_, url);
}

bool BrowserBox::OnLinkClick(const DString& url)
{
    return true;
}

CefRequestHandler::ReturnValue BrowserBox::OnBeforeNavigate(CefRefPtr<CefRequest> request, bool is_redirect)
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
    cef_control_->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([](const std::string& params, nim_comp::ReportResultFunction callback) {
        MessageBox(NULL, ui::StringUtil::UTF8ToUTF16(params).c_str(), _T("接收到 JavaScript 发来的消息"), MB_OK);
        callback(false, R"({ "message": "Success." })");
    }));
}

void BrowserBox::OnLoadError(CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    return;
}
