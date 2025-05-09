#include "ListBoxForm.h"
#include "Item.h"

ListBoxForm::ListBoxForm():
    m_pListBox(nullptr)
{
}


ListBoxForm::~ListBoxForm()
{
}

DString ListBoxForm::GetSkinFolder()
{
    return _T("list_box");
}

DString ListBoxForm::GetSkinFile()
{
    return _T("list_box.xml");
}

void ListBoxForm::OnInitWindow()
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

    if (bVTileListBox) {
        //VTileListBox: 设置为固定2列, 自动计算Item宽度
        m_pListBox->SetAttribute(_T("columns"), _T("2"));
        m_pListBox->SetAttribute(_T("auto_calc_item_size"), _T("true"));
    }
    else if (bHTileListBox) {
        //HTileListBox: 设置为固定2行, 自动计算Item高度
        m_pListBox->SetAttribute(_T("rows"), _T("2"));
        m_pListBox->SetAttribute(_T("auto_calc_item_size"), _T("true"));
    }

    for (auto i = 0; i < 300; i++) {
        Item* item = new Item(this);
        ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("list_box/item.xml")));

        if (bVListBox) {
            //VListBox: 设置为 拉伸类型
            item->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, true);
        }
        else if(bHListBox) {
            //HListBox: 设置为 固定宽度
            item->SetFixedWidth(ui::UiFixedInt(200), true, true);
        }
        else if (bVTileListBox) {
            //VTileListBox: 高度为自动计算
            item->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
        }
        else if (bHTileListBox) {
            //HTileListBox: 固定宽度，高度为自动计算
            item->SetFixedWidth(ui::UiFixedInt(200), true, true);
            item->SetFixedHeight(ui::UiFixedInt::MakeAuto(), true, true);
        }

        DString img = _T("icon.png");
        DString title = ui::StringUtil::Printf(_T("任务 [%02d]"), i);

        item->InitSubControls(img, title);
        m_pListBox->AddItem(item);
    }

    m_pListBox->SetFocus();
    // 监听列表中点击选择子项的事件
    m_pListBox->AttachSelect(UiBind(&ListBoxForm::OnSelected, this, std::placeholders::_1));
}

bool ListBoxForm::OnSelected(const ui::EventArgs& args)
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

