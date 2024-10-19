#include "toast.h"
#include "duilib/Core/Keyboard.h"

namespace nim_comp {

const DString Toast::kClassName = _T("Toast");

void Toast::ShowToast(const DString &content, int duration, Window* pParentWindow)
{
    Toast *toast = new Toast;
    if (!toast->CreateWnd(pParentWindow, ui::WindowCreateParam(_T(""), true))) {
        delete toast;
        return;
    }
    toast->SetContent(content);
    toast->SetDuration(duration);
    toast->ShowWindow(ui::kSW_SHOW_NORMAL);
}

DString Toast::GetSkinFolder()
{
    return _T("toast");
}

DString Toast::GetSkinFile()
{
    return _T("toast.xml");
}

DString Toast::GetWindowId() const
{
    return kClassName;
}

LRESULT Toast::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (uMsg == WM_KEYDOWN && wParam == ui::kVK_ESCAPE) {
        this->CloseWnd();
    }
    // 整个toast界面都在标题栏，所以要处理WM_NC消息
    else if (uMsg == WM_NCLBUTTONDBLCLK || uMsg == WM_LBUTTONDBLCLK) {
        this->CloseWnd();
    }
    // duilib在WM_MOUSELEAVE消息中会发送一个lparam为-1的WM_MOUSEMOVE消息
    else if ((uMsg == WM_NCMOUSEMOVE || uMsg == WM_MOUSEMOVE) && lParam != -1) {
        if (nullptr != m_close_button && !m_close_button->IsVisible()) {
            m_close_button->SetFadeVisible(true);
        }
    }
    else if (uMsg == WM_NCMOUSELEAVE || uMsg == WM_MOUSELEAVE) {
        ui::UiPoint pt;
        GetCursorPos(pt);
        ScreenToClient(pt);        
        ui::UiRect clientRect;
        GetClientRect(clientRect);
        // leave消息触发时，获取的鼠标坐标有可能还在client_rect范围内，会偏差1像素，这里缩减1像素
        clientRect.Deflate(1, 1, 1, 1);
        if (nullptr != m_close_button && !clientRect.ContainsPt(ui::UiPoint(pt.x, pt.y))) {
            m_close_button->SetFadeVisible(false);
        }
    }
    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

void Toast::OnInitWindow()
{
    if (GetRoot() != nullptr) {
        GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&Toast::OnClicked, this, std::placeholders::_1));
    }

    m_content = dynamic_cast<ui::RichEdit*>(FindControl(_T("content")));
    m_close_button = dynamic_cast<ui::Button*>(FindControl(_T("close_btn")));
    ASSERT(m_content != nullptr);
    ASSERT(m_close_button != nullptr);
}

void Toast::SetDuration(int duration)
{
    ASSERT(duration > 0);
    if (duration <= 0){
        return;
    }
    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, ToWeakCallback([this]()
    {
        this->CloseWnd();
    }), duration);
}

bool Toast::OnClicked(const ui::EventArgs& msg)
{
    DString name;
    if (msg.GetSender() != nullptr) {
        name = msg.GetSender()->GetName();
    }
    if (name == _T("close_btn")) {
        this->CloseWnd();
    }
    return true;
}

void Toast::SetContent(const DString &str)
{
    ASSERT(m_content != nullptr);
    if (m_content != nullptr) {
        m_content->SetText(str);
        m_content->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
    }
}
}
