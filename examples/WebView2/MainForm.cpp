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

    //页面全屏
    ui::Button* pFullscreenBtn = dynamic_cast<ui::Button*>(FindControl(_T("webview2_full_screen_btn")));
    if (pFullscreenBtn != nullptr) {
        pFullscreenBtn->AttachClick([this](const ui::EventArgs&) {
            ui::Control* pWebView2Control = FindControl(_T("webview2_control"));
            if (pWebView2Control != nullptr) {
                this->SetFullscreenControl(pWebView2Control);
            }
            return true;
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


LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    if (vkCode == ui::kVK_F11) {
        if (ui::WebView2Manager::GetInstance().IsEnableF11()) {
            //页面全屏或者退出全屏
            if (IsWindowFullscreen() && (GetFullscreenControl() != nullptr)) {
                bHandled = true;
                ExitControlFullscreen();
            }
            else {
                //当前页面，全屏显示
                if (m_pWebView2Control != nullptr) {
                    bHandled = true;
                    SetFullscreenControl(m_pWebView2Control);
                }
            }
        }
    }
    else if (vkCode == ui::kVK_F12) {
        if (ui::WebView2Manager::GetInstance().IsEnableF12()) {
            //显示或者隐藏开发者工具
            bHandled = true;
            if (m_pWebView2Control != nullptr) {
                //只有打开功能，没有关闭功能
                if (m_pWebView2Control->AreDevToolsEnabled()) {
                    m_pWebView2Control->OpenDevToolsWindow();
                }
            }
        }
    }
    if (bHandled) {
        return 0;
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}
