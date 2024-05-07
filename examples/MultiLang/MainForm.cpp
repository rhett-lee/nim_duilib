#include "MainForm.h"

const std::wstring MainForm::kClassName = L"MultiLang";

MainForm::MainForm()
{
}


MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
    return L"MultiLang";
}

std::wstring MainForm::GetSkinFile()
{
    return L"MultiLang.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化

	/* Show select language menu */
	ui::Button* select = static_cast<ui::Button*>(FindControl(L"language"));
	ASSERT(select != nullptr);
	if (select == nullptr) {
		return;
	}
	select->AttachClick([this](const ui::EventArgs& args) {
		ui::UiRect rect = args.pSender->GetPos();
		ui::UiPoint point;
		point.x = rect.left;
		point.y = rect.bottom;
		ClientToScreen(point);

		ShowPopupMenu(point);
		return true;
		});
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    //窗口关闭时，终止主线程的消息循环，从而退出程序
    PostQuitMessage(0L);
    return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void MainForm::ShowPopupMenu(const ui::UiPoint& point)
{
	ui::CMenuWnd* menu = new ui::CMenuWnd(GetHWND());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
	//设置菜单xml所在的目录
	menu->SetSkinFolder(GetResourcePath());
	std::wstring xml(L"lang_menu.xml");
	menu->ShowMenu(xml, point);

	//更新当前选择的语言
	ui::CheckBox* checkBoxChinese = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"checkbox_lang_chinese"));
	if (checkBoxChinese != nullptr) {
		checkBoxChinese->Selected(false);
	}
	ui::CheckBox* checkBoxEnglish = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"checkbox_lang_english"));
	if (checkBoxEnglish != nullptr) {
		checkBoxEnglish->Selected(true);
	}
}