#include "msgbox.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Core/GlobalManager.h"

namespace nim_comp {

void ShowMsgBox(HWND hwnd, MsgboxCallback cb,
    const DString &content, bool content_is_id,
    const DString &title, bool title_is_id,
    const DString &yes, bool btn_yes_is_id,
    const DString &no, bool btn_no_is_id)
{
    MsgBox* msgbox = new MsgBox;
    if (!msgbox->CreateWnd(hwnd, _T(""), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED)) {
        delete msgbox;
        return;
    }
    msgbox->SetTitle(title_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(title) : title);
    msgbox->SetContent(content_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(content) : content);
    msgbox->SetButton(btn_yes_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(yes) : yes, btn_no_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(no) : no);
    msgbox->Show(hwnd, cb);
}

const LPCTSTR MsgBox::kClassName = _T("MsgBox");

MsgBox::MsgBox()
{
}

MsgBox::~MsgBox()
{
}

DString MsgBox::GetSkinFolder()
{
    return _T("msgbox");
}

DString MsgBox::GetSkinFile()
{
    return _T("msgbox.xml");
}

DString MsgBox::GetZIPFileName() const
{
    return (_T("msgbox.zip"));
}

DString MsgBox::GetWindowClassName() const
{
    return kClassName;
}

DString MsgBox::GetWindowId() const
{
    return kClassName;
}

UINT MsgBox::GetClassStyle() const
{
    return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

void MsgBox::OnEsc(BOOL &bHandled)
{
    bHandled = TRUE;
    EndMsgBox(MB_NO);
}
void MsgBox::CloseWnd(UINT nRet)
{
    // 提示框关闭之前先Enable父窗口，防止父窗口隐到后面去。
    HWND hWndParent = ::GetWindow(GetHWND(), GW_OWNER);
    if (hWndParent) {
        ::EnableWindow(hWndParent, TRUE);
        ::SetFocus(hWndParent);
    }
    __super::CloseWnd(nRet);
}

void MsgBox::OnInitWindow()
{
    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MsgBox::OnClicked, this, std::placeholders::_1));

    title_ = (ui::Label*)FindControl(_T("title"));
    content_ = (ui::RichEdit*)FindControl(_T("content"));
    btn_yes_ = (ui::Button*)FindControl(_T("btn_yes"));
    btn_no_ = (ui::Button*)FindControl(_T("btn_no"));
}

bool MsgBox::OnClicked(const ui::EventArgs& msg)
{
    DString name = msg.pSender->GetName();
    if (name == _T("btn_yes"))
    {
        EndMsgBox(MB_YES);
    }
    else if (name == _T("btn_no"))
    {
        EndMsgBox(MB_NO);
    }
    else if (name == _T("closebtn"))
    {
        EndMsgBox(MB_NO);
    }
    return true;
}

void MsgBox::SetTitle(const DString &str)
{
    title_->SetText(str);

    ::SetWindowText(GetHWND(), str.c_str());
}

void MsgBox::SetContent(const DString &str)
{
    content_->SetText(str);

    int width = content_->GetFixedWidth().GetInt32();

    ui::UiSize sz = content_->GetNaturalSize(width, 0);
    content_->SetFixedHeight(ui::UiFixedInt(sz.cy), true, false);
}

void MsgBox::SetButton(const DString &yes, const DString &no)
{
    if (!yes.empty())
    {
        btn_yes_->SetText(yes);
        btn_yes_->SetFadeVisible(true);
        btn_yes_->SetFocus();
    }
    else
    {
        btn_yes_->SetFadeVisible(false);
    }

    if (!no.empty())
    {
        btn_no_->SetText(no);
        btn_no_->SetFadeVisible(true);
        if (yes.empty())
            btn_no_->SetFocus();
    }
    else
    {
        btn_no_->SetFadeVisible(false);
    }
}

void MsgBox::Show(HWND hwnd, MsgboxCallback cb)
{
    msgbox_callback_ = cb;

    ::EnableWindow(hwnd, FALSE);
    CenterWindow();
    ShowWindow(true);
}

void MsgBox::EndMsgBox(MsgBoxRet ret)
{
    this->CloseWnd(0);

    if (msgbox_callback_)
    {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ui::UiBind(msgbox_callback_, ret));
    }
}

}
