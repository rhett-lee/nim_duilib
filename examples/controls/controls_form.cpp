#include "stdafx.h"
#include "controls_form.h"
#include "about_form.h"
#include "ui_components/comboex/CheckCombo.h"
#include "ui_components/comboex/FilterCombo.h"
#include "duilib/Utils/StringUtil.h"

#include <fstream>

const std::wstring ControlForm::kClassName = L"Controls";

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

ui::Control* ControlForm::CreateControl(const std::wstring& pstrClass)
{
	ui::Control* control = nullptr;
	if (pstrClass == L"CheckCombo")
	{
		control = new nim_comp::CheckCombo;
	}
	else if (pstrClass == L"FilterCombo")
	{
		control = new nim_comp::FilterCombo;
	}
	return control;
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
	for (auto i = 0; i < 30; i++)
	{
		ui::ListBoxElement* element = new ui::ListBoxElement;
		element->SetText(nbase::StringPrintf(L"ui::VListBox::ListBoxElement %d", i));
		element->SetClass(L"listitem");
		element->SetFixedHeight(20, true);
		list->AddItem(element);
	}

	/* Initialize TreeView data */
	ui::TreeView* tree = static_cast<ui::TreeView*>(FindControl(L"tree"));
	ui::TreeNode* parent_node = nullptr;
	for (auto j = 0; j < 8; j++)
	{
		ui::TreeNode* node = new ui::TreeNode;
		node->SetClass(L"listitem");
		node->SetFixedHeight(20, true);
		if (parent_node)
		{
			node->SetText(nbase::StringPrintf(L"ui::TreeView::TreeNode %d", j));
			node->SetMargin({ 10, 0, 0, 0 }, true);
			parent_node->AddChildNode(node);
		}
		else
		{
			node->SetText(nbase::StringPrintf(L"ui::TreeView Parent Node", j));
			tree->GetRootNode()->AddChildNode(node);
			parent_node = node;
		}
	}

	/* Initialize ComboBox data */
	ui::Combo* combo = static_cast<ui::Combo*>(FindControl(L"combo"));
	if (combo != nullptr) {
		for (auto i = 0; i < 10; i++)
		{
			ui::ListBoxElement* element = new ui::ListBoxElement;
			element->SetClass(L"listitem");
			element->SetFixedHeight(30, true);
			element->SetBkColor(L"white");
			element->SetTextPadding({ 6,0,6,0 });
			element->SetText(nbase::StringPrintf(L"ui::Combo::ListBoxElement %d", i));
			combo->AddItem(element);
		}
	}

	std::string checks[7] = { "nim_comp::CheckCombo", "check1", "check2", "check3", "check4", "check5", "check6" };
	nim_comp::CheckCombo* check_combo = static_cast<nim_comp::CheckCombo*>(FindControl(L"check_combo"));
	if (check_combo != nullptr) {
		for (auto i = 0; i < 7; i++)
		{
			ui::CheckBox* item = new ui::CheckBox;
			item->SetFixedWidth(DUI_LENGTH_STRETCH, true, true);
			item->SetFixedHeight(24, true);
			item->SetUTF8Text(checks[i]);
			item->SetUTF8DataID(checks[i]);
			item->SetToolTipText(L"nim_comp::CheckCombo");

			item->SetTextPadding({ 20, 2, 2, 0 });
			item->SetTextStyle(ui::TEXT_LEFT | ui::TEXT_VCENTER);
			std::wstring image_normal = nbase::StringPrintf(L"file='../public/checkbox/check_no.png' dest='%d,4,%d,20'", 2, 18);
			std::wstring image_select = nbase::StringPrintf(L"file='../public/checkbox/check_yes.png' dest='%d,4,%d,20'", 2, 18);

			item->SetStateImage(ui::kControlStateNormal, image_normal);
			item->SetSelectedStateImage(ui::kControlStateNormal, image_select);

			check_combo->AddItem(item);
		}
	}

	nim_comp::FilterCombo* filter_combo = static_cast<nim_comp::FilterCombo*>(FindControl(L"filter_combo"));
	if (filter_combo != nullptr) {
		char buf[16] = {};
		for (auto i = 0; i < 100; i++)
		{
			nim_comp::ListElementMatch* item = new nim_comp::ListElementMatch;
			item->SetFixedHeight(20, true);
			//ui::GlobalManager::FillBoxWithCache(item, L"date_export/combo/date_item.xml");
			//Label *label = new label;

			std::string str = "nim_comp::FilterCombo item";
			_itoa_s(i, buf, 10);
			str += buf;
			item->SetText(nbase::UTF8ToUTF16(str));
			item->SetUTF8DataID(str);
			filter_combo->AddItem(item);
		}
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
		RECT rect = args.pSender->GetPos();
		ui::UiPoint point;
		point.x = rect.left - 175;
		point.y = rect.top + 10;
		::ClientToScreen(GetHWND(), &point);

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
				POINT pt = args.ptMouse;
				if ((pt.x != -1) && (pt.y != -1)) {
					ui::Control* pControl = (ui::Control*)args.lParam;//当前点击点所在的控件

					//鼠标消息产生的上下文菜单
					::ClientToScreen(GetHWND(), &pt);
					ShowPopupMenu(ui::UiPoint(pt));
				}
				else {
					//按Shif + F10，由系统产生上下文菜单
					pt = { 100, 100 };
					::ClientToScreen(GetHWND(), &pt);
					ShowPopupMenu(ui::UiPoint(pt));
				}
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
    ui::CMenuElementUI* menu_item = new ui::CMenuElementUI;
    menu_item->SetText(L"Dynamically created");
    menu_item->SetClass(L"menu_element");
    menu_item->SetFixedWidth(180, true, true);
    menu_item->SetFontId(L"system_14");
    menu_item->SetTextPadding({ 20, 0, 20, 0 });
    menu_fourth->AddSubMenuItemAt(menu_item, 1);//添加后，资源由菜单统一管理

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
    ASSERT(menu_check_01 != nullptr);
    menu_check_01->AttachClick([&flag](const ui::EventArgs& args) {
        flag = true;
        return true;
        });
    ui::CheckBox* menuCheckBox01 = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"menu_checkbox_01"));
    ASSERT(menuCheckBox01 != nullptr);
    menuCheckBox01->Selected(s_is_checked_01_flag);

    ui::CMenuElementUI* menu_check_02 = dynamic_cast<ui::CMenuElementUI*>(menu->FindControl(L"menu_check_02"));
    ASSERT(menu_check_02 != nullptr);
    menu_check_02->AttachClick([&flag](const ui::EventArgs& args) {
        flag = false;
        return true;
        });

    ui::CheckBox* menuCheckBox02 = dynamic_cast<ui::CheckBox*>(menu->FindControl(L"menu_checkbox_02"));
    ASSERT(menuCheckBox02 != nullptr);
    menuCheckBox02->Selected(!s_is_checked_01_flag);


    /* About menu */
    ui::CMenuElementUI* menu_about = static_cast<ui::CMenuElementUI*>(menu->FindControl(L"about"));
    menu_about->AttachClick([this](const ui::EventArgs& args) {
        AboutForm* about_form = (AboutForm*)(nim_comp::WindowsManager::GetInstance()->GetWindow(AboutForm::kClassName, AboutForm::kClassName));
        if (!about_form)
        {
            about_form = new AboutForm();
            about_form->CreateWnd(GetHWND(), AboutForm::kClassName, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_LAYERED);
            about_form->CenterWindow();
            about_form->ShowWindow();
        }
        else
        {
            about_form->ActiveWindow();
        }
        return true;
        });
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
	std::wstring controls_xml = ui::GlobalManager::GetResourcePath() + GetResourcePath() + GetSkinFile();

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
	control_edit->StartLoading();
	//control_edit->StartGifPlayForUI();

	// Show about form
	//nim_comp::WindowsManager::SingletonShow<AboutForm>(AboutForm::kClassName);
}

void ControlForm::OnProgressValueChagned(float value)
{
	//回调给的进度范围是：[0, 99), 转换为[0, 100]
	value = value * 100 / 99 + 0.5f;
	auto progress = static_cast<ui::Progress*>(FindControl(L"progress"));
	if (progress) {
		progress->SetValue(value);
	}

	auto circleprogress = static_cast<ui::Progress*>(FindControl(L"circleprogress"));
	if (circleprogress)	{
		circleprogress->SetValue(value);
		circleprogress->SetText(nbase::StringPrintf(L"%.0f%%", value));
	}

	if ((int)value == progress->GetMaxValue()) {
		//进度达到最大值，停止加载动画
		auto control_edit = static_cast<ui::RichEdit*>(FindControl(L"edit"));
		if (control_edit) {
			control_edit->StopLoading();
		}
	}
}
