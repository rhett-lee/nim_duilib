//MainForm.cpp
#include "MainForm.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("webview2");
}

DString MainForm::GetSkinFile()
{
    return _T("webview2.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    //窗口初始化完成，可以进行本Form的初始化

    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MainForm::OnClicked, this, std::placeholders::_1), 0);
    m_pWebView2Control = dynamic_cast<ui::WebView2Control*>(FindControl(_T("webview2_control")));
    m_pEditUrl = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_url")));

    // 设置输入框样式
    if (m_pEditUrl != nullptr) {
        m_pEditUrl->SetSelAllOnFocus(true);
        m_pEditUrl->AttachReturn(UiBind(&MainForm::OnNavigate, this, std::placeholders::_1));
    }

    ui::Control* pControl = FindControl(_T("btn_back"));
    if (pControl != nullptr) {
        pControl->SetEnabled(false);
    }

    pControl = FindControl(_T("btn_forward"));
    if (pControl != nullptr) {
        pControl->SetEnabled(false);
    }

    if (m_pWebView2Control != nullptr) {
        //更新前进后退按钮的状态
        m_pWebView2Control->SetHistoryChangedCallback([this]() {
            if (m_pWebView2Control != nullptr) {
                ui::Control* pControl = FindControl(_T("btn_back"));
                if (pControl != nullptr) {
                    pControl->SetEnabled(m_pWebView2Control->CanGoBack());
                }
                pControl = FindControl(_T("btn_forward"));
                if (pControl != nullptr) {
                    pControl->SetEnabled(m_pWebView2Control->CanGoForward());
                }
            }
            });

        //更新URL
        m_pWebView2Control->SetSourceChangedCallback([this](const DString& url) {
            ui::GlobalManager::Instance().AssertUIThread();
            ui::RichEdit* pEditUrl = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_url")));
            if (pEditUrl != nullptr) {
                pEditUrl->SetText(url);
            }
            });

        //更新标题
        m_pWebView2Control->SetDocumentTitleChangedCallback([this](const DString& title) {
            ui::GlobalManager::Instance().AssertUIThread();
            ui::Label* pLabelTitle = dynamic_cast<ui::Label*>(FindControl(_T("page_title")));
            if (pLabelTitle != nullptr) {
                pLabelTitle->SetText(title);
            }
            });
    }
}

bool MainForm::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.GetSender()->GetName();

    if (name == _T("btn_dev_tool")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->OpenDevToolsWindow();
        }
    }
    else if (name == _T("btn_back")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->NavigateBack();
        }
    }
    else if (name == _T("btn_forward")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->NavigateForward();
        }
    }
    else if (name == _T("btn_navigate")) {
        ui::EventArgs emptyMsg;
        OnNavigate(emptyMsg);
    }
    else if (name == _T("btn_refresh")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->Refresh();
        }
    }
    return true;
}

bool MainForm::OnNavigate(const ui::EventArgs& /*msg*/)
{
    if ((m_pEditUrl != nullptr) && !m_pEditUrl->GetText().empty()) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->Navigate(m_pEditUrl->GetText());
            m_pWebView2Control->SetFocus();
        }
    }
    return true;
}
