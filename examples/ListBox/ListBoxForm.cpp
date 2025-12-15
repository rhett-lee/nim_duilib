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
        //m_pListBox->SetAttribute(_T("item_size"), _T("200，80"));
        m_pListBox->SetAttribute(_T("columns"), _T("2"));
        m_pListBox->SetAttribute(_T("auto_calc_item_size"), _T("true"));
    }
    else if (bHTileListBox) {
        //HTileListBox: 设置为固定2行, 自动计算Item高度
        //m_pListBox->SetAttribute(_T("item_size"), _T("200，80"));
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

    //测试ListBox的事件
    TestListBoxEvents(m_pListBox);
}

void ListBoxForm::TestListBoxEvents(ui::ListBox* pListBox)
{
    if (pListBox == nullptr) {
        return;
    }
    //事件挂载，测试事件接口
    auto OnVirtualListBoxEvents = [this, pListBox](const ui::EventArgs& args) {
        ASSERT(pListBox == args.GetSender());
        DString sInfo = GetEventDisplayInfo(args, pListBox);
        OutputDebugLog(sInfo);
        };

    //挂载事件
    pListBox->AttachSelect([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachUnSelect([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachSelChanged([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachDoubleClick([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachClick([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachRClick([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachItemMouseEnter([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachItemMouseLeave([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachReturn([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachKeyDown([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
    pListBox->AttachKeyUp([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
}

DString ListBoxForm::GetEventDisplayInfo(const ui::EventArgs& args, ui::ListBox* pListBox)
{
    DString sInfo = ui::EventTypeToString(args.eventType);
    while (sInfo.size() < 24) {
        sInfo += _T(" ");
    }
    if (args.eventType == ui::kEventSelect) {
        size_t nNewItemIndex = (size_t)args.wParam;
        size_t nOldItemIndex = (size_t)args.lParam;
        if (nOldItemIndex != ui::Box::InvalidIndex) {
            sInfo += ui::StringUtil::Printf(_T("NewItemIndex=%zu, OldItemIndex=%zu"),
                                            nNewItemIndex, nOldItemIndex);
        }
        else {
            sInfo += ui::StringUtil::Printf(_T("NewItemIndex=%zu"), nNewItemIndex);
        }
    }
    else if (args.eventType == ui::kEventUnSelect) {
        size_t nItemIndex = (size_t)args.wParam;
        sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu"), nItemIndex);
    }
    else if (args.eventType == ui::kEventSelChanged) {
        //无参数
    }
    else if ((args.eventType == ui::kEventItemMouseEnter) ||
             (args.eventType == ui::kEventItemMouseLeave) ||
             (args.eventType == ui::kEventMouseDoubleClick) ||
             (args.eventType == ui::kEventClick) ||
             (args.eventType == ui::kEventRClick) ||
             (args.eventType == ui::kEventReturn)) {
        size_t nItemIndex = (size_t)args.wParam;
        if (nItemIndex == ui::Box::InvalidIndex) {
            sInfo += _T("no params");
        }
        else {
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu"), nItemIndex);
        }
    }
    else if ((args.eventType == ui::kEventKeyDown) || (args.eventType == ui::kEventKeyUp)) {
        //键盘消息
        DString keyName = ui::Keyboard::GetKeyName(args.vkCode, false);
        DString modifierKey;
        if (args.vkCode != ui::VirtualKeyCode::kVK_CONTROL) {
            if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_CONTROL)) {
                modifierKey += _T("Ctrl+");
            }
        }
        if (args.vkCode != ui::VirtualKeyCode::kVK_SHIFT) {
            if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_SHIFT)) {
                modifierKey += _T("Shift+");
            }
        }
        if (args.vkCode != ui::VirtualKeyCode::kVK_MENU) {
            if (ui::Keyboard::IsKeyDown(ui::VirtualKeyCode::kVK_MENU)) {
                modifierKey += _T("Alt+");
            }
        }
        sInfo += _T("<");
        sInfo += modifierKey;
        sInfo += keyName;
        sInfo += _T(">");
        sInfo += _T(" ");

        size_t nItemIndex = (size_t)args.wParam;
        if (nItemIndex == ui::Box::InvalidIndex) {
            sInfo += _T("no params");
        }
        else {
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu"), nItemIndex);
        }
    }
    else {
        ASSERT(0);
    }
    return sInfo;
}

void ListBoxForm::OutputDebugLog(const DString& logMsg)
{
#if defined DUILIB_BUILD_FOR_WIN && defined _DEBUG
    ::OutputDebugString(logMsg.c_str());
#endif
}


