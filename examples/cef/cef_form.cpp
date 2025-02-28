#include "cef_form.h"

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

void CefForm::OnInitWindow()
{
    // 监听鼠标单击事件
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&CefForm::OnClicked, this, std::placeholders::_1));

    // 从 XML 中查找指定控件
    cef_control_        = dynamic_cast<ui::CefControlBase*>(FindControl(_T("cef_control")));
    cef_control_dev_    = dynamic_cast<ui::CefControlBase*>(FindControl(_T("cef_control_dev")));
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
    ui::FilePath cefHtml = ui::FilePathUtil::GetCurrentModuleDirectory();
    cefHtml.NormalizeDirectoryPath();
    cefHtml += _T("resources\\themes\\default\\cef\\cef.html");
    cef_control_->LoadURL(cefHtml.ToString());

    if (!ui::CefManager::GetInstance()->IsEnableOffsetRender())
        cef_control_dev_->SetFadeVisible(false);
}

void CefForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    ui::CefManager::GetInstance()->PostQuitMessage(0L);
}

bool CefForm::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.GetSender()->GetName();

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

        if (ui::CefManager::GetInstance()->IsEnableOffsetRender())
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
    cef_control_->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([this](const std::string& params, ui::ReportResultFunction callback) {
        DString value = ui::StringConvert::UTF8ToT(params);
        //ui::Toast::ShowToast(value, 3000, this);
        callback(false, R"({ "message": "Success." })");
    }));
}
