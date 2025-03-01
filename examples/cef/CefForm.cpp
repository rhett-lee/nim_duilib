#include "CefForm.h"

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
    m_pCefControl = dynamic_cast<ui::CefControlBase*>(FindControl(_T("cef_control")));
    m_pCefControlDev = dynamic_cast<ui::CefControlBase*>(FindControl(_T("cef_control_dev")));
    m_pDevToolBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_dev_tool")));
    m_pEditUrl = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_url")));
    ASSERT(m_pCefControl != nullptr);
    ASSERT(m_pCefControlDev != nullptr);
    ASSERT(m_pDevToolBtn != nullptr);
    ASSERT(m_pEditUrl != nullptr);

    // 设置输入框样式
    m_pEditUrl->SetSelAllOnFocus(true);
    m_pEditUrl->AttachReturn(UiBind(&CefForm::OnNavigate, this, std::placeholders::_1));

    // 监听页面加载完毕通知
    m_pCefControl->AttachLoadEnd(UiBind(&CefForm::OnLoadEnd, this, std::placeholders::_1));

    // 打开开发者工具
    //m_pCefControl->AttachDevTools(m_pCefControlDev);

    // 加载皮肤目录下的 html 文件
    ui::FilePath cefHtml = ui::FilePathUtil::GetCurrentModuleDirectory();
    cefHtml.NormalizeDirectoryPath();
    cefHtml += _T("resources\\themes\\default\\cef\\cef.html");
    m_pCefControl->LoadURL(cefHtml.ToString());

    if (!ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        m_pCefControlDev->SetFadeVisible(false);
    }
}

void CefForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    ui::CefManager::GetInstance()->PostQuitMessage(0L);
}

bool CefForm::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.GetSender()->GetName();

    if (name == _T("btn_dev_tool")) {
        if (m_pCefControl->IsAttachedDevTools()) {
            m_pCefControl->DettachDevTools();
        }
        else {
            m_pCefControl->AttachDevTools(m_pCefControlDev);
        }

        if (ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
            m_pCefControlDev->SetFadeVisible(m_pCefControl->IsAttachedDevTools());
        }
    }
    else if (name == _T("btn_back")) {
        m_pCefControl->GoBack();
    }
    else if (name == _T("btn_forward")) {
        m_pCefControl->GoForward();
    }
    else if (name == _T("btn_navigate")) {
        ui::EventArgs emptyMsg;
        OnNavigate(emptyMsg);
    }
    else if (name == _T("btn_refresh")) {
        m_pCefControl->Refresh();
    }
    return true;
}

bool CefForm::OnNavigate(const ui::EventArgs& /*msg*/)
{
    if (!m_pEditUrl->GetText().empty()) {
        m_pCefControl->LoadURL(m_pEditUrl->GetText());
        m_pCefControl->SetFocus();
    }
    return true;
}

void CefForm::OnLoadEnd(int httpStatusCode)
{
    FindControl(_T("btn_back"))->SetEnabled(m_pCefControl->CanGoBack());
    FindControl(_T("btn_forward"))->SetEnabled(m_pCefControl->CanGoForward());

    // 注册一个方法提供前端调用
    m_pCefControl->RegisterCppFunc(_T("ShowMessageBox"), ToWeakCallback([this](const std::string& params, ui::ReportResultFunction callback) {
        DString value = ui::StringConvert::UTF8ToT(params);
        //ui::Toast::ShowToast(value, 3000, this);
        callback(false, R"({ "message": "Success." })");
    }));
}
