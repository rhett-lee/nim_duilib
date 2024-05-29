#include "toast.h"

namespace nim_comp {

const LPCTSTR Toast::kClassName = _T("Toast");

void Toast::ShowToast(const std::wstring &content, int duration, HWND parent)
{
    Toast *toast = new Toast;
    if (!toast->CreateWnd(parent, _T(""), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED)) {
        delete toast;
        return;
    }
    toast->SetContent(content);
    toast->SetDuration(duration);
    toast->CenterWindow();
    toast->ShowWindow(true);
}

std::wstring Toast::GetSkinFolder()
{
    return _T("toast");
}

std::wstring Toast::GetSkinFile()
{
    return _T("toast.xml");
}

std::wstring Toast::GetWindowClassName() const
{
    return kClassName;
}

std::wstring Toast::GetWindowId() const
{
    return kClassName;
}

UINT Toast::GetClassStyle() const
{
    return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

LRESULT Toast::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
    {
        this->CloseWnd();
    }
    // 整个toast界面都在标题栏，所以要处理WM_NC消息
    else if (uMsg == WM_NCLBUTTONDBLCLK || uMsg == WM_LBUTTONDBLCLK)
    {
        this->CloseWnd();
    }
    // duilib在WM_MOUSELEAVE消息中会发送一个lparam为-1的WM_MOUSEMOVE消息
    else if ((uMsg == WM_NCMOUSEMOVE || uMsg == WM_MOUSEMOVE) && lParam != -1)
    {
        if (NULL != close_button_ && !close_button_->IsVisible())
            close_button_->SetFadeVisible(true);
    }
    else if (uMsg == WM_NCMOUSELEAVE || uMsg == WM_MOUSELEAVE)
    {
        ui::UiPoint pt;
        GetCursorPos(pt);
        ScreenToClient(pt);        
        ui::UiRect clientRect;
        GetClientRect(clientRect);
        // leave消息触发时，获取的鼠标坐标有可能还在client_rect范围内，会偏差1像素，这里缩减1像素
        clientRect.Deflate(1, 1, 1, 1);
        if (NULL != close_button_ && !clientRect.ContainsPt(ui::UiPoint(pt.x, pt.y)))
            close_button_->SetFadeVisible(false);
    }
    return __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

void Toast::OnInitWindow()
{
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&Toast::OnClicked, this, std::placeholders::_1));

    content_ = dynamic_cast<ui::RichEdit*>(FindControl(_T("content")));
    close_button_ = dynamic_cast<ui::Button*>(FindControl(_T("close_btn")));
    ASSERT(content_ != nullptr);
    ASSERT(close_button_ != nullptr);
}

void Toast::SetDuration(int duration)
{
    if (duration <= 0)
        return;

    ui::GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, ToWeakCallback([this]()
    {
        this->CloseWnd();
    }), duration);
}

bool Toast::OnClicked(const ui::EventArgs& msg)
{
    std::wstring name = msg.pSender->GetName();
    if (name == _T("close_btn"))
    {
        this->CloseWnd();
    }

    return true;
}

void Toast::SetContent(const std::wstring &str)
{
    content_->SetText(str);

    int width = content_->GetFixedWidth().GetInt32();

    ui::UiSize sz = content_->GetNaturalSize(width, 0);
    content_->SetFixedHeight(ui::UiFixedInt(sz.cy), true, true);
}
}
