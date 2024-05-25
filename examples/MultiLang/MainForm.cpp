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

void MainForm::OnCloseWindow()
{
	//关闭窗口后，退出主线程的消息循环，关闭程序
	PostQuitMessage(0L);
}

void MainForm::ShowPopupMenu(const ui::UiPoint& point)
{
	ui::Menu* menu = new ui::Menu(GetHWND());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
	//设置菜单xml所在的目录
	menu->SetSkinFolder(GetResourcePath());
	std::wstring xml(L"lang_menu.xml");
	menu->ShowMenu(xml, point);

	//当前语言文件
	std::wstring currentLangFileName = ui::GlobalManager::Instance().GetLanguageFileName();

	//可用语言文件列表和显示名称
	std::vector<std::pair<std::wstring, std::wstring>> languageList;
	ui::GlobalManager::Instance().GetLanguageList(languageList);
	if (languageList.empty()) {
		languageList.push_back({ currentLangFileName , L""});
	}

	//动态添加菜单项
	for (auto& lang : languageList) {
		const std::wstring fileName = lang.first;
		std::wstring& displayName = lang.second;

		ui::MenuItem* pMenuItem = new ui::MenuItem(this);
		pMenuItem->SetClass(L"menu_element");
		ui::CheckBox* pCheckBox = new ui::CheckBox(this);
		pCheckBox->SetClass(L"menu_checkbox");
		pCheckBox->SetAttribute(L"margin", L"0,5,0,10");
		pCheckBox->SetText(!displayName.empty() ? displayName : fileName);
		pMenuItem->AddItem(pCheckBox);
		menu->AddMenuItem(pMenuItem);

		if (ui::StringHelper::IsEqualNoCase(fileName, currentLangFileName)) {
			pCheckBox->Selected(true);
		}

		//挂载选择语言事件
		pMenuItem->AttachClick([fileName](const ui::EventArgs& args) {
			//切换语言
			ui::GlobalManager& globalManager = ui::GlobalManager::Instance();
			if (globalManager.GetLanguageFileName() != fileName) {
				globalManager.ReloadLanguage(L"", fileName, true);
			}			
			return true;
			});
	}
}