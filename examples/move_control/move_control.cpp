#include "move_control.h"

using namespace ui;
using namespace std;


MoveControlForm::MoveControlForm(const DString& theme_directory, const DString& layout_xml):
    m_theme_directory(theme_directory),
    m_layout_xml(layout_xml)
{
}

MoveControlForm::~MoveControlForm()
{
}

DString MoveControlForm::GetSkinFolder()
{
    return m_theme_directory;
}

DString MoveControlForm::GetSkinFile()
{
    return m_layout_xml;
}

void MoveControlForm::OnInitWindow()
{
    //添加应用。应用有可能是服务器下发的，一般本地也有保存的
    //loadFromDb
    //getFromServer---->后台可以先保存到db，再post个消息出来，界面重新从db load。

    //作为demo，先写死
    std::vector<AppItem> applist;
    CAppDb::GetInstance().LoadFromDb(applist);

    m_frequent_app = static_cast<ui::Box*>(FindControl(_T("frequent_app")));
    m_my_app = static_cast<ui::Box*>(FindControl(_T("my_app")));
    ASSERT(m_frequent_app != nullptr);
    ASSERT(m_my_app != nullptr);
    
    for (const auto& item: applist) {
        AppItemUi* pAppUi = AppItemUi::Create(item, m_frequent_app);
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
