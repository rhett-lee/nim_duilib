#include "msgbox.h"
#include "duilib/Core/GlobalManager.h"

namespace nim_comp {

void ShowMsgBox(ui::Window* pWindow, MsgboxCallback cb,
                const DString &content, bool content_is_id,
                const DString &title, bool title_is_id,
                const DString &yes, bool btn_yes_is_id,
                const DString &no, bool btn_no_is_id)
{
    MsgBox* msgbox = new MsgBox;
    ui::WindowCreateParam createWndParam;
    //TODO: 平台相关
    createWndParam.m_dwStyle = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
    createWndParam.m_dwExStyle = WS_EX_LAYERED;
    if (!msgbox->CreateWnd(pWindow, createWndParam)) {
        delete msgbox;
        return;
    }
    msgbox->SetTitle(title_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(title) : title);
    msgbox->SetContent(content_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(content) : content);
    msgbox->SetButton(btn_yes_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(yes) : yes, btn_no_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(no) : no);
    msgbox->Show(pWindow, cb);
}

const DString MsgBox::kClassName = _T("MsgBox");

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

DString MsgBox::GetWindowId() const
{
    return kClassName;
}

void MsgBox::OnEsc(BOOL &bHandled)
{
    bHandled = TRUE;
    EndMsgBox(MB_NO);
}

void MsgBox::OnInitWindow()
{
    if (GetRoot() != nullptr) {
        GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MsgBox::OnClicked, this, std::placeholders::_1));
    }
    m_title = dynamic_cast<ui::Label*>(FindControl(_T("title")));
    m_content = dynamic_cast<ui::RichEdit*>(FindControl(_T("content")));
    m_btn_yes = dynamic_cast<ui::Button*>(FindControl(_T("btn_yes")));
    m_btn_no = dynamic_cast<ui::Button*>(FindControl(_T("btn_no")));
    __super::OnInitWindow();
}

void MsgBox::OnCloseWindow()
{
    // 提示框关闭之前先Enable父窗口，防止父窗口隐到后面去。
    if (m_pParentWindow != nullptr) {
        m_pParentWindow->EnableWindow(true);
        m_pParentWindow->SetWindowFocus();
    }
    __super::OnCloseWindow();
}

bool MsgBox::OnClicked(const ui::EventArgs& msg)
{
    DString name;
    if (msg.GetSender() != nullptr) {
        name = msg.GetSender()->GetName();
    }
    if (name == _T("btn_yes")) {
        EndMsgBox(MB_YES);
    }
    else if (name == _T("btn_no")) {
        EndMsgBox(MB_NO);
    }
    else if (name == _T("closebtn")) {
        EndMsgBox(MB_NO);
    }
    return true;
}

void MsgBox::SetTitle(const DString &str)
{
    ASSERT(m_title != nullptr);
    if (m_title != nullptr) {
        m_title->SetText(str);
    }    
    SetText(str);
}

void MsgBox::SetContent(const DString &str)
{
    ASSERT(m_content != nullptr);
    if (m_content != nullptr) {
        m_content->SetText(str);
        int32_t width = m_content->GetFixedWidth().GetInt32();
        ui::UiSize sz = m_content->GetNaturalSize(width, 0);
        m_content->SetFixedHeight(ui::UiFixedInt(sz.cy), true, false);
    }
}

void MsgBox::SetButton(const DString& yes, const DString& no)
{
    ASSERT(m_btn_yes != nullptr);
    if (m_btn_yes != nullptr) {
        if (!yes.empty()) {
            m_btn_yes->SetText(yes);
            m_btn_yes->SetFadeVisible(true);
            m_btn_yes->SetFocus();
        }
        else {
            m_btn_yes->SetFadeVisible(false);
        }
    }

    ASSERT(m_btn_no != nullptr);
    if (m_btn_no != nullptr) {
        if (!no.empty()) {
            m_btn_no->SetText(no);
            m_btn_no->SetFadeVisible(true);
            if (yes.empty()) {
                m_btn_no->SetFocus();
            }
        }
        else {
            m_btn_no->SetFadeVisible(false);
        }
    }
}

void MsgBox::Show(ui::Window* pParentWindow, MsgboxCallback cb)
{
    m_msgbox_callback = cb;
    if (pParentWindow != nullptr) {
        pParentWindow->EnableWindow(false);
    }
    m_pParentWindow = pParentWindow;
    CenterWindow();
    ShowWindow(ui::kSW_SHOW_NORMAL);
}

void MsgBox::EndMsgBox(MsgBoxRet ret)
{
    this->CloseWnd(0);
    if (m_msgbox_callback) {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ui::UiBind(m_msgbox_callback, ret));
    }
}

}
