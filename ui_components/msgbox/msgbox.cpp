#include "msgbox.h"
#include "ui_components/public_define.h"
#include "base/thread/thread_manager.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Core/GlobalManager.h"

namespace nim_comp {

void ShowMsgBox(HWND hwnd, MsgboxCallback cb,
	const std::wstring &content, bool content_is_id,
	const std::wstring &title, bool title_is_id,
	const std::wstring &yes, bool btn_yes_is_id,
	const std::wstring &no, bool btn_no_is_id)
{
	MsgBox* msgbox = new MsgBox;
	if (!msgbox->CreateWnd(hwnd, L"", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED)) {
		delete msgbox;
		return;
	}
	msgbox->SetTitle(title_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(title) : title);
	msgbox->SetContent(content_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(content) : content);
	msgbox->SetButton(btn_yes_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(yes) : yes, btn_no_is_id ? ui::GlobalManager::Instance().Lang().GetStringViaID(no) : no);
	msgbox->Show(hwnd, cb);
}

const LPCTSTR MsgBox::kClassName = L"MsgBox";

MsgBox::MsgBox()
{
}

MsgBox::~MsgBox()
{
}

std::wstring MsgBox::GetSkinFolder()
{
	return L"msgbox";
}

std::wstring MsgBox::GetSkinFile()
{
	return L"msgbox.xml";
}

std::wstring MsgBox::GetZIPFileName() const
{
	return (L"msgbox.zip");
}

std::wstring MsgBox::GetWindowClassName() const
{
	return kClassName;
}

std::wstring MsgBox::GetWindowId() const
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
	GetRoot()->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MsgBox::OnClicked, this, std::placeholders::_1));

	title_ = (ui::Label*)FindControl(L"title");
	content_ = (ui::RichEdit*)FindControl(L"content");
	btn_yes_ = (ui::Button*)FindControl(L"btn_yes");
	btn_no_ = (ui::Button*)FindControl(L"btn_no");
}

bool MsgBox::OnClicked(const ui::EventArgs& msg)
{
	std::wstring name = msg.pSender->GetName();
	if (name == L"btn_yes")
	{
		EndMsgBox(MB_YES);
	}
	else if (name == L"btn_no")
	{
		EndMsgBox(MB_NO);
	}
	else if (name == L"closebtn")
	{
		EndMsgBox(MB_NO);
	}
	return true;
}

void MsgBox::SetTitle(const std::wstring &str)
{
	title_->SetText(str);

	::SetWindowText(GetHWND(), str.c_str());
}

void MsgBox::SetContent(const std::wstring &str)
{
	content_->SetText(str);

	int width = content_->GetFixedWidth().GetInt32();

	ui::UiSize sz = content_->GetNaturalSize(width, 0);
	content_->SetFixedHeight(ui::UiFixedInt(sz.cy), true, false);
}

void MsgBox::SetButton(const std::wstring &yes, const std::wstring &no)
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
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(msgbox_callback_, ret));
	}
}

}
