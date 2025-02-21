#include "richlist_form.h"
#include "item.h"

RichlistForm::RichlistForm():
    m_pListBox(nullptr)
{
}


RichlistForm::~RichlistForm()
{
}

DString RichlistForm::GetSkinFolder()
{
    return _T("list_box");
}

DString RichlistForm::GetSkinFile()
{
    return _T("list_box.xml");
}

void RichlistForm::OnInitWindow()
{
    m_pListBox = dynamic_cast<ui::ListBox*>(FindControl(_T("list")));
    ASSERT(m_pListBox != nullptr);
    if (m_pListBox == nullptr) {
        return;
    }
    bool bHListBox = dynamic_cast<ui::HListBox*>(m_pListBox) != nullptr;
    bool bVListBox = dynamic_cast<ui::VListBox*>(m_pListBox) != nullptr;
    bool bHTileListBox = dynamic_cast<ui::HTileListBox*>(m_pListBox) != nullptr;
    bool bVTileListBox = dynamic_cast<ui::VTileListBox*>(m_pListBox) != nullptr;

    for (auto i = 0; i < 300; i++) {
        Item* item = new Item(this);
        ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("list_box/item.xml")));

        if (bHListBox || bHTileListBox || bVTileListBox) {
            item->SetFixedWidth(ui::UiFixedInt(200), true, true);
        }

        DString img = _T("icon.png");
        DString title = ui::StringUtil::Printf(_T("任务 [%02d]"), i);

        item->InitSubControls(img, title);
        m_pListBox->AddItem(item);
    }

    m_pListBox->SetFocus();
    // 监听列表中点击选择子项的事件
    m_pListBox->AttachSelect(UiBind(&RichlistForm::OnSelected, this, std::placeholders::_1));
}

bool RichlistForm::OnSelected(const ui::EventArgs& args)
{
    int current = static_cast<int>(args.wParam);
    int old = static_cast<int>(args.lParam);

    /*auto message = ui::StringUtil::Printf(_T("您选择了索引为 %d 的子项，上一次选择子项索引为 %d\n"), current, old);
    ::OutputDebugStringW(message.c_str());

    ui::ListBoxVerVisible vVisibleType = ui::ListBoxVerVisible::kVisible;
    ui::ListBoxHorVisible hVisibleType = ui::ListBoxHorVisible::kVisible;
    if (m_pListBox != nullptr) {
        if (current == 20) {
            m_pListBox->EnsureVisible(200, vVisibleType, hVisibleType);
        }
        else if (current == 210) {
            m_pListBox->EnsureVisible(21, vVisibleType, hVisibleType);
        }
    }*/
    //ui::ShowMsgBox(GetHWND(), ui::MsgboxCallback(), message, false, _T("提示"), false);

    return true;
}

