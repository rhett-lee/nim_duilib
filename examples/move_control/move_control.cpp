#include "stdafx.h"
#include "move_control.h"

using namespace ui;
using namespace std;


MoveControlForm::MoveControlForm(const std::wstring& class_name, const std::wstring& theme_directory, const std::wstring& layout_xml)
	: m_class_name(class_name)
	, m_theme_directory(theme_directory)
	, m_layout_xml(layout_xml)
{
}


MoveControlForm::~MoveControlForm()
{
}

std::wstring MoveControlForm::GetSkinFolder()
{
	return m_theme_directory;
}

std::wstring MoveControlForm::GetSkinFile()
{
	return m_layout_xml;
}

std::wstring MoveControlForm::GetWindowClassName() const
{
	return m_class_name;
}

void MoveControlForm::OnInitWindow()
{
	//添加应用。应用有可能是服务器下发的，一般本地也有保存的
	//loadFromDb
	//getFromServer---->后台可以先保存到db，再post个消息出来，界面重新从db load。

	//作为demo，先写死
	std::vector<AppItem> applist;
	CAppDb::GetInstance().LoadFromDb(applist);

	m_frequent_app = static_cast<ui::Box*>(FindControl(L"frequent_app"));
	m_my_app = static_cast<ui::Box*>(FindControl(L"my_app"));
	ASSERT(m_frequent_app != nullptr);
	ASSERT(m_my_app != nullptr);
	
	for (const auto& item: applist) {
		AppItemUi* pAppUi = AppItemUi::Create(item);
		pAppUi->SetFrequentBox(m_frequent_app);
		if (item.m_isFrequent) {
			m_frequent_app->AddItem(pAppUi);
		}
		else
		{
			m_my_app->AddItem(pAppUi);
		}
	}
}

LRESULT MoveControlForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
