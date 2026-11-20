//MainForm.cpp
#include "MainForm.h"

MainForm::MainForm()
{
    m_pWebView2Control = nullptr;
    m_pUrlBar = nullptr;
    m_pTitleLabel = nullptr;
    m_pBtnBack = nullptr;
    m_pBtnForward = nullptr;
    m_pBtnRefresh = nullptr;
    m_pBtnStop = nullptr;
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
    m_pUrlBar = dynamic_cast<ui::UrlBar*>(FindControl(_T("url_bar")));
    m_pTitleLabel = dynamic_cast<ui::Label*>(FindControl(_T("page_title")));
    m_pBtnBack = dynamic_cast<ui::Button*>(FindControl(_T("btn_back")));
    m_pBtnForward = dynamic_cast<ui::Button*>(FindControl(_T("btn_forward")));
    m_pBtnRefresh = dynamic_cast<ui::Button*>(FindControl(_T("btn_refresh")));
    m_pBtnStop = dynamic_cast<ui::Button*>(FindControl(_T("btn_stop")));

    //地址栏提交网址后执行导航
    if (m_pUrlBar != nullptr) {
        m_pUrlBar->AttachUrlChanged(UiBind(&MainForm::OnNavigate, this, std::placeholders::_1));
    }

    //初始状态：不允许后退、前进；显示刷新按钮，隐藏停止按钮
    if (m_pBtnBack != nullptr) {
        m_pBtnBack->SetEnabled(false);
    }
    if (m_pBtnForward != nullptr) {
        m_pBtnForward->SetEnabled(false);
    }
    if (m_pBtnRefresh != nullptr) {
        m_pBtnRefresh->SetVisible(true);
    }
    if (m_pBtnStop != nullptr) {
        m_pBtnStop->SetVisible(false);
    }

    if (m_pWebView2Control != nullptr) {
        //更新前进后退按钮的状态
        m_pWebView2Control->SetHistoryChangedCallback([this]() {
            ui::GlobalManager::Instance().AssertUIThread();
            UpdateNavigationButtonState();
            });

        //导航状态变化时，切换"刷新"和"停止"按钮
        m_pWebView2Control->SetNavigationStateChangedCallback(
            [this](ui::WebView2Control::NavigationState state, HRESULT /*errorCode*/) {
                ui::GlobalManager::Instance().AssertUIThread();
                const bool bLoading = (state == ui::WebView2Control::NavigationState::Started);
                if (m_pBtnRefresh != nullptr) {
                    m_pBtnRefresh->SetVisible(!bLoading);
                }
                if (m_pBtnStop != nullptr) {
                    m_pBtnStop->SetVisible(bLoading);
                }
                UpdateNavigationButtonState();
            });

        //更新URL
        m_pWebView2Control->SetSourceChangedCallback([this](const DString& url) {
            ui::GlobalManager::Instance().AssertUIThread();
            if (m_pUrlBar != nullptr) {
                m_pUrlBar->SetUrl(url);
            }
            });

        //更新标题
        m_pWebView2Control->SetDocumentTitleChangedCallback([this](const DString& title) {
            ui::GlobalManager::Instance().AssertUIThread();
            if (m_pTitleLabel != nullptr) {
                m_pTitleLabel->SetText(title);
            }
            });

        //WebView2的子窗口获得焦点时，键盘消息不会到达本窗口，需要注册回调处理快捷键
        m_pWebView2Control->SetAcceleratorKeyPressedCallback([this](uint32_t vkCode) {
            ui::GlobalManager::Instance().AssertUIThread();
            return HandleShortcutKey(static_cast<ui::VirtualKeyCode>(vkCode));
            });
    }

    //页面全屏
    ui::Button* pFullscreenBtn = dynamic_cast<ui::Button*>(FindControl(_T("webview2_full_screen_btn")));
    if (pFullscreenBtn != nullptr) {
        pFullscreenBtn->AttachClick([this](const ui::EventArgs&) {
            if (m_pWebView2Control != nullptr) {
                this->SetFullscreenControl(m_pWebView2Control);
            }
            return true;
            });
    }
}

bool MainForm::OnClicked(const ui::EventArgs& msg)
{
    if (msg.GetSender() == nullptr) {
        return false;
    }
    DString name = msg.GetSender()->GetName();

    if (name == _T("btn_dev_tool")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->OpenDevToolsWindow();
        }
        return true;
    }
    else if (name == _T("btn_back")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->NavigateBack();
        }
        return true;
    }
    else if (name == _T("btn_forward")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->NavigateForward();
        }
        return true;
    }
    else if (name == _T("btn_navigate")) {
        //提交地址栏中的输入（触发 kEventUrlChanged 事件后执行导航）
        if (m_pUrlBar != nullptr) {
            m_pUrlBar->Submit();
        }
        return true;
    }
    else if (name == _T("btn_refresh")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->Refresh();
        }
        return true;
    }
    else if (name == _T("btn_stop")) {
        if (m_pWebView2Control != nullptr) {
            m_pWebView2Control->Stop();
        }
        return true;
    }
    // 未处理的点击事件继续向上传递
    return false;
}

bool MainForm::OnNavigate(const ui::EventArgs& /*msg*/)
{
    if ((m_pUrlBar == nullptr) || (m_pWebView2Control == nullptr)) {
        return true;
    }

    //UrlBar 已完成网址归一化（补全协议、本地路径转 file://、搜索模板），直接导航
    DString url = m_pUrlBar->GetUrl();
    if (url.empty()) {
        return true;
    }
    m_pWebView2Control->Navigate(url);
    m_pWebView2Control->SetFocus();
    return true;
}

void MainForm::UpdateNavigationButtonState()
{
    if (m_pWebView2Control == nullptr) {
        return;
    }
    if (m_pBtnBack != nullptr) {
        m_pBtnBack->SetEnabled(m_pWebView2Control->CanGoBack());
    }
    if (m_pBtnForward != nullptr) {
        m_pBtnForward->SetEnabled(m_pWebView2Control->CanGoForward());
    }
}


LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    if (HandleShortcutKey(vkCode)) {
        bHandled = true;
        return 0;
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

bool MainForm::HandleShortcutKey(ui::VirtualKeyCode vkCode)
{
    if (vkCode == ui::kVK_F11) {
        if (!ui::WebView2Manager::GetInstance().IsEnableF11()) {
            return false;
        }
        //页面全屏或者退出全屏
        if (IsWindowFullscreen() && (GetFullscreenControl() != nullptr)) {
            ExitControlFullscreen();
            return true;
        }
        if (m_pWebView2Control != nullptr) {
            //当前页面，全屏显示
            SetFullscreenControl(m_pWebView2Control);
            return true;
        }
        return false;
    }
    else if (vkCode == ui::kVK_F12) {
        if (!ui::WebView2Manager::GetInstance().IsEnableF12()) {
            return false;
        }
        //打开开发者工具（只有打开功能，没有关闭功能）
        if ((m_pWebView2Control != nullptr) && m_pWebView2Control->AreDevToolsEnabled()) {
            m_pWebView2Control->OpenDevToolsWindow();
        }
        return true;
    }
    return false;
}
