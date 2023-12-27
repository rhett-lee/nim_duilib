#include "stdafx.h"
#include "controls_form.h"
#include "about_form.h"
#include "duilib/Utils/StringUtil.h"

#include <fstream>

const std::wstring ControlForm::kClassName = L"Controls";

//系统全局热键的ID
#define SYSTEM_HOTKEY_ID 111

ControlForm::ControlForm()
{
}


ControlForm::~ControlForm()
{
}

std::wstring ControlForm::GetSkinFolder()
{
	return L"controls";
}

std::wstring ControlForm::GetSkinFile()
{
	return L"controls.xml";
}

std::wstring ControlForm::GetWindowClassName() const
{
	return kClassName;
}

void ControlForm::OnInitWindow()
{
	/**
	 * 为了让代码看起来相对容易理解，不需要频繁跟进才能看明白示例代码
	 * 我们将一些控件储存为局部变量，正确的使用应该是将他们作为成员变量
	 * 不要在每次使用的时候都需要 FindControl，否则会影响性能代码不易读
	 */

	/* Initialize ListBox data */
	ui::ListBox* list = static_cast<ui::ListBox*>(FindControl(L"list"));
	if (list != nullptr) {
		for (auto i = 0; i < 30; i++)
		{
			ui::ListBoxItem* element = new ui::ListBoxItem;
			element->SetText(nbase::StringPrintf(L"ui::VListBox::ListBoxItem %d", i));
			element->SetClass(L"listitem");
			element->SetFixedHeight(ui::UiFixedInt(20), true, true);
			list->AddItem(element);
		}
	}

	//初始化Combo的数据
	ui::Combo* combo = static_cast<ui::Combo*>(FindControl(L"combo"));
	if (combo != nullptr) {
		ui::TreeView* pTreeView = combo->GetTreeView();
		ui::TreeNode* pTreeNode = pTreeView->GetRootNode();
		for (auto i = 0; i < 10; i++) {
			ui::TreeNode* node = new ui::TreeNode;
			node->SetWindow(this);
			node->SetClass(L"tree_node");
			node->SetText(nbase::StringPrintf(L"ui::Combo::TreeNode %d", i));
			pTreeNode->AddChildNode(node);
		}
	}
//#ifdef _DEBUG
//	ASSERT(combo->GetCount() == 10);
//	ASSERT(combo->GetCurSel() == -1);
//	combo->SetCurSel(1);
//	ASSERT(combo->GetCurSel() == 1);
//	combo->SetItemData(1, 0xFF1234);
//	ASSERT(combo->GetItemData(1) == 0xFF1234);
//	combo->SetItemText(2, L"2");
//	ASSERT(combo->GetItemText(2) == L"2");
//
//	size_t nIndex = combo->AddTextItem(L"Last");
//	ASSERT(combo->GetItemText(nIndex) == L"Last");
//
//	nIndex = combo->InsertTextItem(nIndex, L"Last2");
//	ASSERT(combo->GetItemText(nIndex) == L"Last2");
//
//	ASSERT(combo->DeleteItem(nIndex));
//
//	ASSERT(combo->GetItemText(combo->GetCurSel()) == combo->GetText());
//	combo->SetText(L"Test");
//	ASSERT(combo->GetText() == L"Test");
//#endif

	ui::FilterCombo* filterCombo = static_cast<ui::FilterCombo*>(FindControl(L"filter_combo"));
	if (filterCombo != nullptr) {
		for (auto i = 0; i < 100; i++) {
			filterCombo->AddTextItem(nbase::StringPrintf(L"Item %d FilterCombo", i));
		}
	}

	ui::CheckCombo* check_combo = static_cast<ui::CheckCombo*>(FindControl(L"check_combo"));
	if (check_combo != nullptr) {
		check_combo->AddTextItem(L"星期一");
		check_combo->AddTextItem(L"星期二");
		check_combo->AddTextItem(L"星期三");
		check_combo->AddTextItem(L"星期四");
		check_combo->AddTextItem(L"星期五");
		check_combo->AddTextItem(L"星期六");
		check_combo->AddTextItem(L"星期日");
	}

	/* Load xml file content in global misc thread, and post update RichEdit task to UI thread */
	nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(&ControlForm::LoadRichEditData, this));

	/* Post repeat task to update progress value 200 milliseconds once */
	/* Using ToWeakCallback to protect closure when if [ControlForm] was destoryed */
	nbase::ThreadManager::PostRepeatedTask(kThreadGlobalMisc, ToWeakCallback([this](){
		nbase::TimeDelta time_delta = nbase::TimeDelta::FromMicroseconds(nbase::Time::Now().ToInternalValue());
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&ControlForm::OnProgressValueChagned, this, (float)(time_delta.ToMilliseconds() % 100)));
	}), nbase::TimeDelta::FromMilliseconds(300));

	/* Show settings menu */
	ui::Button* settings = static_cast<ui::Button*>(FindControl(L"settings"));
	settings->AttachClick([this](const ui::EventArgs& args) {
		ui::UiRect rect = args.pSender->GetPos();
		ui::UiPoint point;
		point.x = rect.left - 175;
		point.y = rect.top + 10;
		ClientToScreen(point);

		ShowPopupMenu(point);
		return true;
	});

	//注册一个Context Menu，演示功能（用这两种方法都可以注册上下文菜单功能）
	//m_pRoot->AttachAllEvents([this](ui::EventArgs& args) {
	//m_pRoot->AttachMenu([this](ui::EventArgs& args) {
	ui::RichEdit* edit = static_cast<ui::RichEdit*>(FindControl(L"edit"));
	if (edit != nullptr) {
		edit->AttachMenu([this](const ui::EventArgs& args) {
			if (args.Type == ui::kEventMouseMenu) {
				ui::UiPoint pt = args.ptMouse;
				if ((pt.x != -1) && (pt.y != -1)) {
					ui::Control* pControl = (ui::Control*)args.lParam;//当前点击点所在的控件

					//鼠标消息产生的上下文菜单
					ClientToScreen(pt);
					ShowPopupMenu(pt);
				}
				else {
					//按Shift + F10，由系统产生上下文菜单
					pt = { 100, 100 };
					ClientToScreen(pt);
					ShowPopupMenu(pt);
				}
			}
			return true;
			});
	}

	//显示拾色器
	ui::Button* pShowColorPicker = dynamic_cast<ui::Button*>(FindControl(L"show_color_picker"));
	if (pShowColorPicker != nullptr) {
		pShowColorPicker->AttachClick([this](const ui::EventArgs& args) {
			ShowColorPicker();			
			return true;
		});
	}

	//RichText显示超级链接
	ui::RichText* pRichText = dynamic_cast<ui::RichText*>(FindControl(L"rich_text_demo"));
	if (pRichText != nullptr) {
		pRichText->AttachLinkClick([this](const ui::EventArgs& args) {
			const wchar_t* url = (const wchar_t*)args.wParam;
			if (url != nullptr) {
				::MessageBox(GetHWND(), url, L"RichText点击超链接", MB_OK);
			}
			return true;
			});
	}

	//HyperLink控件
	ui::HyperLink* pHyperLink = dynamic_cast<ui::HyperLink*>(FindControl(L"hyper_link1"));
	if (pHyperLink != nullptr) {
		pHyperLink->AttachLinkClick([this](const ui::EventArgs& args) {
			const wchar_t* url = (const wchar_t*)args.wParam;
			if (url != nullptr) {
				::MessageBox(GetHWND(), url, L"HyperLink点击超链接", MB_OK);
			}
			return true;
			});
	}

	pHyperLink = dynamic_cast<ui::HyperLink*>(FindControl(L"hyper_link2"));
	if (pHyperLink != nullptr) {
		pHyperLink->AttachLinkClick([this](const ui::EventArgs& /*args*/) {
			::MessageBox(GetHWND(), L"文字按钮事件响应", L"HyperLink点击", MB_OK);
			return true;
			});
	}

	//热键设置
	ui::HotKey* pHotKey = dynamic_cast<ui::HotKey*>(FindControl(L"set_hot_key"));
	ui::Button* pHotKeyButton = dynamic_cast<ui::Button*>(FindControl(L"btn_set_hot_key"));
	if (pHotKey && pHotKeyButton) {
		pHotKeyButton->AttachClick([this, pHotKey](const ui::EventArgs& args) {
			uint8_t wVirtualKeyCode = 0;
			uint8_t wModifiers = 0;
			pHotKey->GetHotKey(wVirtualKeyCode, wModifiers);
			if (wVirtualKeyCode != 0) {
				//设置为激活窗口的热键
				int32_t nRet = SetWindowHotKey(wVirtualKeyCode, wModifiers);
				ASSERT(nRet == 1);
			}

			//if (1) {
			//	//测试代码
			//	std::wstring hotKeyName = pHotKey->GetHotKeyName();
			//	uint8_t wVirtualKeyCode = 0;
			//	uint8_t wModifiers = 0;
			//	pHotKey->GetHotKey(wVirtualKeyCode, wModifiers);
			//	ASSERT(pHotKey->GetHotKey() == MAKEWORD(wVirtualKeyCode, wModifiers));

			//	const uint8_t wModifiers2 = ui::kHotKey_Shift | ui::kHotKey_Contrl | ui::kHotKey_Alt | ui::kHotKey_Ext;
			//	const uint8_t wVirtualKeyCode2 = VK_HOME;

			//	pHotKey->SetHotKey(wVirtualKeyCode2, wModifiers2);
			//	pHotKey->GetHotKey(wVirtualKeyCode, wModifiers);
			//	ASSERT(wModifiers2 == wModifiers);
			//	ASSERT(wVirtualKeyCode2 == wVirtualKeyCode);

			//	ASSERT(pHotKey->GetHotKey() == MAKEWORD(wVirtualKeyCode, wModifiers));

			//	pHotKey->SetHotKey(MAKEWORD(wVirtualKeyCode2, wModifiers2));
			//	ASSERT(pHotKey->GetHotKey() == MAKEWORD(wVirtualKeyCode2, wModifiers2));
			//}

			return true;
			});
	}

	pHotKey = dynamic_cast<ui::HotKey*>(FindControl(L"set_system_hot_key"));
	pHotKeyButton = dynamic_cast<ui::Button*>(FindControl(L"btn_set_system_hot_key"));
	if (pHotKey && pHotKeyButton) {
		pHotKeyButton->AttachClick([this, pHotKey](const ui::EventArgs& args) {
			uint8_t wVirtualKeyCode = 0;
			uint8_t wModifiers = 0;
			pHotKey->GetHotKey(wVirtualKeyCode, wModifiers);
			if (wVirtualKeyCode != 0) {
				//设置为系统全局的热键
				bool nRet = RegisterHotKey(wVirtualKeyCode, wModifiers, SYSTEM_HOTKEY_ID);
				ASSERT(nRet);
			}
			return true;
		});
	}
}

void ControlForm::ShowColorPicker()
{
	ui::ColorPicker* pColorPicker = new ui::ColorPicker;
	pColorPicker->CreateWnd(GetHWND(), ui::ColorPicker::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	pColorPicker->CenterWindow();
	pColorPicker->ShowModalFake(this->GetHWND());

	ui::RichEdit* pEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"edit"));
	if (pEdit != nullptr) {
		std::wstring oldTextColor = pEdit->GetTextColor();
		if (!oldTextColor.empty() && (pColorPicker != nullptr)) {
			pColorPicker->SetSelectedColor(pEdit->GetUiColor(oldTextColor));
		}
		//如果在界面选择颜色，则临时更新RichEdit控件文本的颜色
		pColorPicker->AttachSelectColor([this, pEdit](const ui::EventArgs& args) {
			ui::UiColor newColor = ui::UiColor((uint32_t)args.wParam);
			pEdit->SetTextColor(pEdit->GetColorString(newColor));
			return true;
			});

		//窗口关闭事件
		pColorPicker->AttachWindowClose([this, pColorPicker, pEdit, oldTextColor](const ui::EventArgs& args) {
			ui::UiColor newColor = pColorPicker->GetSelectedColor();
			if ((args.wParam == 0) && !newColor.IsEmpty()) {
				//如果是"确认"，则设置RichEdit控件的文本颜色
				pEdit->SetTextColor(pEdit->GetColorString(newColor));
			}
			else {
				//如果是"取消"或者关闭窗口，则恢复原来的颜色
				pEdit->SetTextColor(oldTextColor);
			}
			return true;
			});

	}
}

void ControlForm::ShowPopupMenu(const ui::UiPoint& point)
{
    ui::CMenuWnd* menu = new ui::CMenuWnd(GetHWND());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    std::wstring xml(L"settings_menu.xml");
    menu->ShowMenu(xml, point);

    //在二级菜单中，添加子菜单项
    ui::CMenuElementUI* menu_fourth = static_cast<ui::CMenuElementUI*>(menu->FindControl(L"fourth"));
	if (menu_fourth != nullptr) {
		ui::CMenuElementUI* menu_item = new ui::CMenuElementUI;
		menu_item->SetText(L"Dynamically created");
		menu_item->SetClass(L"menu_element");
		menu_item->SetFixedWidth(ui::UiFixedInt(180), true, true);
		menu_item->SetFontId(L"system_14");
		menu_item->SetTextPadding({ 20, 0, 20, 0 });
		menu_fourth->AddSubMenuItemAt(menu_item, 1);//添加后，资源由菜单统一管理
	}
	

    //在一级菜单中，添加子菜单项
    /*
    menu_item = new ui::CMenuElementUI;
    menu_item->SetText(L"Dynamically created");
    menu_item->SetClass(L"menu_element");
    menu_item->SetFixedWidth(180);
    menu_item->SetFontId(L"system_14");
    menu_item->SetTextPadding({ 20, 0, 20, 0 });
    menu->AddMenuItemAt(menu_item, 4);//添加后，资源由菜单统一管理
    */

    //CheckBox菜单项的功能演示
    static bool s_is_checked_01_flag = false;
    bool& flag = s_is_checked_01_flag;
    ui::CMenuElementUI* menu_check_01 = dynamic_cast<ui::CMenuElementUI*>(menu->FindControl(L"menu_check_01"));
	if (menu_check_01 != nullptr) {
		menu_check_01->AttachClick([&flag](const ui::EventArgs& args) {
			flag = true;
			return true;
			});
	}
    ui::CheckBox* menuCheckBox01 = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"menu_checkbox_01"));
	if (menuCheckBox01 != nullptr) {
		menuCheckBox01->Selected(s_is_checked_01_flag);
	}

    ui::CMenuElementUI* menu_check_02 = dynamic_cast<ui::CMenuElementUI*>(menu->FindControl(L"menu_check_02"));
	if (menu_check_02 != nullptr) {
		menu_check_02->AttachClick([&flag](const ui::EventArgs& args) {
			flag = false;
			return true;
			});
	}

    ui::CheckBox* menuCheckBox02 = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"menu_checkbox_02"));
	if (menuCheckBox02 != nullptr) {
		menuCheckBox02->Selected(!s_is_checked_01_flag);
	}


    /* About menu */
    ui::CMenuElementUI* menu_about = static_cast<ui::CMenuElementUI*>(menu->FindControl(L"about"));
	if (menu_about != nullptr) {
		menu_about->AttachClick([this](const ui::EventArgs& args) {
			AboutForm* about_form = new AboutForm();
			about_form->CreateWnd(GetHWND(), AboutForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
			about_form->CenterWindow();
			about_form->ShowModalFake(GetHWND());
			return true;
			});
	}
}

LRESULT ControlForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void ControlForm::LoadRichEditData()
{
	std::streamoff length = 0;
	std::string xml;
	std::wstring controls_xml = ui::GlobalManager::Instance().GetResourcePath() + GetResourcePath() + GetSkinFile();

	std::ifstream ifs(controls_xml.c_str());
	if (ifs.is_open())
	{
		ifs.seekg(0, std::ios_base::end);
		length = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);

		xml.resize(static_cast<unsigned int>(length)+1);
		ifs.read(&xml[0], length);

		ifs.close();
	}
	std::wstring xmlU;
	ui::StringHelper::MBCSToUnicode(xml.c_str(), xmlU, CP_UTF8);

	// Post task to UI thread
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(&ControlForm::OnResourceFileLoaded, this, xmlU));
}

void ControlForm::OnResourceFileLoaded(const std::wstring& xml)
{
	if (xml.empty())
		return;

	auto control_edit = static_cast<ui::RichEdit*>(FindControl(L"edit"));
	if (control_edit)
	{
		control_edit->SetText(xml);
		control_edit->SetFocus();
		control_edit->HomeUp();
	}

	//启动加载动画
	//control_edit->StartLoading();
	//control_edit->StartGifPlayForUI();

	// Show about form
	//nim_comp::WindowsManager::SingletonShow<AboutForm>(AboutForm::kClassName);
}

void ControlForm::OnProgressValueChagned(float value)
{
	//回调给的进度范围是：[0, 99), 转换为[0, 100]
	int lastValue = 0;
	value = value * 100 / 99 + 0.5f;
	auto progress = static_cast<ui::Progress*>(FindControl(L"progress"));
	if (progress) {
		lastValue = (int)progress->GetValue();
		progress->SetValue(value);
	}

	auto circleprogress = static_cast<ui::Progress*>(FindControl(L"circleprogress"));
	if (circleprogress)	{
		circleprogress->SetValue(value);
		circleprogress->SetText(nbase::StringPrintf(L"%.0f%%", value));
	}

	if (((int)value == progress->GetMaxValue()) || (value < lastValue)) {
		//进度达到最大值，停止加载动画
		auto control_edit = static_cast<ui::RichEdit*>(FindControl(L"edit"));
		if (control_edit) {
			control_edit->StopLoading();
		}
	}
}

LRESULT ControlForm::OnHotKey(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = __super::OnHotKey(uMsg, wParam, lParam, bHandled);
	bHandled = true;
	if (wParam == SYSTEM_HOTKEY_ID) {
		SetForeground();
		::MessageBox(GetHWND(), L"接收到系统热键命令", L"ControlForm::OnHotKey", MB_OK);
	}
	return lResult;
}
