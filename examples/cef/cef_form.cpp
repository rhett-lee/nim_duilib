#include "cef_form.h"

const DString CefForm::kClassName = _T("CEF_Control_Example");

CefForm::CefForm()
{
}

CefForm::~CefForm()
{
}

DString CefForm::GetSkinFolder()
{
    return _T("cef");
}

DString CefForm::GetSkinFile()
{
    return _T("cef.xml");
}

DString CefForm::GetWindowClassName() const
{
    return kClassName;
}

ui::Control* CefForm::CreateControl(const DString& pstrClass)
{
    // 扫描 XML 发现有名称为 CefControl 的节点，则创建一个 ui::CefControl 控件
    if (pstrClass == _T("CefControl"))
    {
        if (nim_comp::CefManager::GetInstance()->IsEnableOffsetRender())
            return new nim_comp::CefControl(this);
        else
            return new nim_comp::CefNativeControl(this);
    }

    return NULL;
}

void CefForm::OnInitWindow()
{
    // 监听鼠标单击事件
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&CefForm::OnClicked, this, std::placeholders::_1));

    // 从 XML 中查找指定控件
    cef_control_        = dynamic_cast<nim_comp::CefControlBase*>(FindControl(_T("cef_control")));
    cef_control_dev_    = dynamic_cast<nim_comp::CefControlBase*>(FindControl(_T("cef_control_dev")));
    btn_dev_tool_        = dynamic_cast<ui::Button*>(FindControl(_T("btn_dev_tool")));
    edit_url_            = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_url")));

    // 设置输入框样式
    edit_url_->SetSelAllOnFocus(true);
    edit_url_->AttachReturn(UiBind(&CefForm::OnNavigate, this, std::placeholders::_1));

    // 监听页面加载完毕通知
    cef_control_->AttachLoadEnd(UiBind(&CefForm::OnLoadEnd, this, std::placeholders::_1));

    // 打开开发者工具
    cef_control_->AttachDevTools(cef_control_dev_);

    // 加载皮肤目录下的 html 文件
    cef_control_->LoadURL(ui::PathUtil::GetCurrentModuleDirectory() + _T("resources\\themes\\default\\cef\\cef.html"));

    if (!nim_comp::CefManager::GetInstance()->IsEnableOffsetRender())
        cef_control_dev_->SetFadeVisible(false);
}

void CefForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    nim_comp::CefManager::GetInstance()->PostQuitMessage(0L);
}

bool CefForm::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.pSender->GetName();

    if (name == _T("btn_dev_tool"))
    {
        if (cef_control_->IsAttachedDevTools())
        {
            cef_control_->DettachDevTools();
        }
        else
        {
            cef_control_->AttachDevTools(cef_control_dev_);
        }

        if (nim_comp::CefManager::GetInstance()->IsEnableOffsetRender())
        {
            cef_control_dev_->SetFadeVisible(cef_control_->IsAttachedDevTools());
        }
    }
    else if (name == _T("btn_back"))
    {
        cef_control_->GoBack();
    }
    else if (name == _T("btn_forward"))
    {
        cef_control_->GoForward();
    }
    else if (name == _T("btn_navigate"))
    {
        ui::EventArgs emptyMsg;
        OnNavigate(emptyMsg);
    }
    else if (name == _T("btn_refresh"))
    {
        cef_control_->Refresh();
    }

    return true;
}

bool CefForm::OnNavigate(const ui::EventArgs& /*msg*/)
{
    if (!edit_url_->GetText().empty())
    {
        cef_control_->LoadURL(edit_url_->GetText());
        cef_control_->SetFocus();
    }

    return true;
}

void CefForm::OnLoadEnd(int httpStatusCode)
{
    FindControl(_T("btn_back"))->SetEnabled(cef_control_->CanGoBack());
    FindControl(_T("btn_forward"))->SetEnabled(cef_control_->CanGoForward());

    // 注册一个方法提供前端调用
    cef_control_->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([this](const std::string& params, nim_comp::ReportResultFunction callback) {
        DString value = ui::StringUtil::UTF8ToT(params);
        nim_comp::Toast::ShowToast(value, 3000, GetHWND());
        callback(false, R"({ "message": "Success." })");
    }));
}
