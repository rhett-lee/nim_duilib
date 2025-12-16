#include "MainForm.h"
#include "DataProvider.h"

MainForm::MainForm():
    m_pTileList(nullptr),
    m_pDataProvider(nullptr),
    m_pEditColumn(nullptr),
    m_pEditTotal(nullptr),
    m_pOptionColumnFix(nullptr),
    m_pEditUpdate(nullptr),
    m_pEditTaskName(nullptr),
    m_pEditDelete(nullptr),
    m_pEditChildMarginX(nullptr),
    m_pEditChildMarginY(nullptr)
{

}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("virtual_list_box");
}

DString MainForm::GetSkinFile()
{
    return _T("main.xml");
}

void MainForm::OnInitWindow()
{
    m_pEditColumn = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_column")));
    m_pEditTotal = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_total")));
    m_pTileList = dynamic_cast<ui::VirtualListBox*>(FindControl(_T("list")));
    m_pOptionColumnFix = dynamic_cast<ui::Option*>(FindControl(_T("option_column_fix")));
    m_pEditUpdate = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_update")));
    m_pEditTaskName = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_task_name")));
    m_pEditDelete = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_delete")));
    m_pEditChildMarginX = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_x")));
    m_pEditChildMarginY = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_child_margin_y")));

    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MainForm::OnClicked, this, std::placeholders::_1));

    // 设置提供者
    m_pDataProvider = new DataProvider;
    m_pTileList->SetDataProvider(m_pDataProvider);

    const ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
    if ((layoutType != ui::LayoutType::VirtualHTileLayout) &&
        (layoutType != ui::LayoutType::VirtualVTileLayout)){
        //隐藏列的设置，其他的不支持设置列
        if (m_pEditColumn != nullptr) {
            m_pEditColumn->SetEnabled(false);
            m_pEditColumn->SetText(_T("  "));
        }
    }

    if (layoutType == ui::LayoutType::VirtualHTileLayout) {
        m_pOptionColumnFix->SetText(_T("固定行数"));
    }
    else if (layoutType == ui::LayoutType::VirtualVTileLayout) {
        m_pOptionColumnFix->SetText(_T("固定列数"));
    }
    else {
        m_pOptionColumnFix->SetEnabled(false);
        ui::Control* pControl = FindControl(_T("option_column_auto"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
        pControl = FindControl(_T("label_column_row"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }

    ui::Option* pOptionAlign1 = dynamic_cast<ui::Option*>(FindControl(_T("option_align1")));
    ui::Option* pOptionAlign2 = dynamic_cast<ui::Option*>(FindControl(_T("option_align2")));
    ui::Option* pOptionAlign3 = dynamic_cast<ui::Option*>(FindControl(_T("option_align3")));
    if ((pOptionAlign1 != nullptr) && (pOptionAlign2 != nullptr) && (pOptionAlign3 != nullptr)) {
        if ((layoutType == ui::LayoutType::VirtualHTileLayout) || (layoutType == ui::LayoutType::VirtualHLayout)) {
            //水平布局
            pOptionAlign1->SetText(_T("靠上对齐(top)"));
            pOptionAlign2->SetText(_T("居中对齐(center)"));
            pOptionAlign3->SetText(_T("靠下对齐(bottom)"));
            pOptionAlign2->Selected(true, true);

            //相应对齐方式变化，动态调整控件的对齐方式
            pOptionAlign1->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("top"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign2->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("center"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign3->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_valign"), _T("bottom"));
                    m_pTileList->Refresh();
                }
                return true;
                });
        }
        else {
            //垂直布局
            pOptionAlign1->SetText(_T("靠左对齐(left)"));
            pOptionAlign2->SetText(_T("居中对齐(center)"));
            pOptionAlign3->SetText(_T("靠右对齐(right)"));
            pOptionAlign2->Selected(true, true);

            //相应对齐方式变化，动态调整控件的对齐方式
            pOptionAlign1->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("left"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign2->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("center"));
                    m_pTileList->Refresh();
                }
                return true;
                });
            pOptionAlign3->AttachSelect([this](const ui::EventArgs&) {
                if (m_pTileList) {
                    m_pTileList->SetAttribute(_T("child_halign"), _T("right"));
                    m_pTileList->Refresh();
                }
                return true;
                });
        }
    }
    //显示当前容器类型, 并更新默认属性
    ui::Label* pListType = dynamic_cast<ui::Label*>(FindControl(_T("list_box_type")));
    if (pListType != nullptr) {
        if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_HLISTBOX) {
            pListType->SetText(_T("VirtualHListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_VLISTBOX) {
            pListType->SetText(_T("VirtualVListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_HTILE_LISTBOX) {
            pListType->SetText(_T("VirtualHTileListBox"));
        }
        else if (m_pTileList->GetType() == DUI_CTR_VIRTUAL_VTILE_LISTBOX) {
            pListType->SetText(_T("VirtualVTileListBox"));
        }
    }

    //更新子项间隙的状态
    if (layoutType == ui::LayoutType::VirtualHLayout) {
        if (m_pEditChildMarginY != nullptr) {
            m_pEditChildMarginY->SetEnabled(false);
        }
        ui::Control* pControl = FindControl(_T("label_child_margin_y"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }
    else if (layoutType == ui::LayoutType::VirtualVLayout) {
        if (m_pEditChildMarginX != nullptr) {
            m_pEditChildMarginX->SetEnabled(false);
        }
        ui::Control* pControl = FindControl(_T("label_child_margin_x"));
        if (pControl) {
            pControl->SetEnabled(false);
        }
    }

    //设置初始数据
    if ((m_pDataProvider != nullptr) && (m_pEditTotal != nullptr)) {
        int nTotal = ui::StringUtil::StringToInt32(m_pEditTotal->GetText().c_str());
        if (nTotal > 0) {
            m_pDataProvider->SetTotal(nTotal);
        }
    }

    //测试虚表的事件
    TestVirtualListBoxEvents(m_pTileList);
}

bool MainForm::OnClicked(const ui::EventArgs& args)
{
    auto sName = args.GetSender()->GetName();
    if (sName == _T("btn_set_total"))
    {
        if (!m_pEditChildMarginX->GetText().empty())    {
            m_pTileList->SetAttribute(_T("child_margin_x"), m_pEditChildMarginX->GetText());
        }
        if (!m_pEditChildMarginY->GetText().empty()) {
            m_pTileList->SetAttribute(_T("child_margin_y"), m_pEditChildMarginY->GetText());
        }
        if (m_pOptionColumnFix->IsSelected()) {
            //固定列数/行数
            ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
            if (layoutType == ui::LayoutType::VirtualVTileLayout) {
                m_pTileList->SetAttribute(_T("columns"), m_pEditColumn->GetText());
            }
            if (layoutType == ui::LayoutType::VirtualHTileLayout) {
                m_pTileList->SetAttribute(_T("rows"), m_pEditColumn->GetText());
            }            
        }
        else {
            //自动计算列数
            m_pTileList->SetAttribute(_T("width"), _T("stretch"));
            ui::LayoutType layoutType = m_pTileList->GetLayout()->GetLayoutType();
            if (layoutType == ui::LayoutType::VirtualVTileLayout) {
                m_pTileList->SetAttribute(_T("columns"), _T("auto"));
            }
            if (layoutType == ui::LayoutType::VirtualHTileLayout) {
                m_pTileList->SetAttribute(_T("rows"), _T("auto"));
            }
        }

        int nTotal = ui::StringUtil::StringToInt32(m_pEditTotal->GetText().c_str());
        if (nTotal > 0) {
            m_pDataProvider->SetTotal(nTotal);
        }
    }
    else if (sName == _T("btn_update")) {
        size_t nIndex = (size_t)ui::StringUtil::StringToInt32(m_pEditUpdate->GetText().c_str());
        ASSERT(nIndex < m_pDataProvider->GetElementCount());
        m_pDataProvider->ChangeTaskName(nIndex, m_pEditTaskName->GetText());
    }
    else if (sName == _T("btn_delete")) {
        size_t nIndex = (size_t)ui::StringUtil::StringToInt32(m_pEditDelete->GetText().c_str());
        ASSERT(nIndex < m_pDataProvider->GetElementCount());
        m_pDataProvider->RemoveTask(nIndex);
    }
    return true;
}

void MainForm::TestVirtualListBoxEvents(ui::VirtualListBox* pListBox)
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
    pListBox->AttachElementFilled([this, OnVirtualListBoxEvents](const ui::EventArgs& args) {
        OnVirtualListBoxEvents(args);
        return true;
        });
}

DString MainForm::GetEventDisplayInfo(const ui::EventArgs& args, ui::VirtualListBox* pListBox)
{
    DString sInfo = ui::EventTypeToString(args.eventType);
    while (sInfo.size() < 24) {
        sInfo += _T(" ");
    }
    if (args.eventType == ui::kEventSelect) {
        size_t nNewItemIndex = (size_t)args.wParam;
        size_t nOldItemIndex = (size_t)args.lParam;
        size_t nNewElementID = pListBox->GetDisplayItemElementIndex(nNewItemIndex);
        if (nOldItemIndex != ui::Box::InvalidIndex) {
            size_t nOldElementID = pListBox->GetDisplayItemElementIndex(nOldItemIndex);
            sInfo += ui::StringUtil::Printf(_T("NewItemIndex=%zu, NewElementID=%zu; OldItemIndex=%zu, OldElementID=%zu"),
                                            nNewItemIndex, nNewElementID, nOldItemIndex, nOldElementID);
        }
        else {
            sInfo += ui::StringUtil::Printf(_T("NewItemIndex=%zu, NewElementID=%zu"), nNewItemIndex, nNewElementID);
        }
    }
    else if (args.eventType == ui::kEventSelChanged) {
        //无参数
    }
    else if ((args.eventType == ui::kEventItemMouseEnter) ||
             (args.eventType == ui::kEventItemMouseLeave) ||
             (args.eventType == ui::kEventMouseDoubleClick) ||
             (args.eventType == ui::kEventClick)  ||
             (args.eventType == ui::kEventRClick) ||
             (args.eventType == ui::kEventReturn)) {
        size_t nItemIndex = (size_t)args.wParam;
        size_t nElementID = (size_t)args.lParam;
        if (nItemIndex == ui::Box::InvalidIndex) {
            sInfo += _T("no params");
        }
        else {
            size_t nCalcElementID = pListBox->GetDisplayItemElementIndex(nItemIndex);
            ASSERT(nElementID == nCalcElementID);
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu"), nItemIndex, nElementID);
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
        size_t nElementID = (size_t)args.lParam;
        if (nItemIndex == ui::Box::InvalidIndex) {
            sInfo += _T("no params");
        }
        else {
            size_t nCalcElementID = pListBox->GetDisplayItemElementIndex(nItemIndex);
            ASSERT(nElementID == nCalcElementID);
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu"), nItemIndex, nElementID);
        }
    }
    else if (args.eventType == ui::kEventElementFilled) {
        size_t nItemIndex = (size_t)args.wParam;
        size_t nElementID = (size_t)args.lParam;
        size_t nCalcElementID = pListBox->GetDisplayItemElementIndex(nItemIndex);
        ASSERT(nElementID == nCalcElementID);
        sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu, ListBoxItem: 0x%p"), nItemIndex, nElementID, args.pEventData);
        ui::IListBoxItem* pListBoxItem = dynamic_cast<ui::IListBoxItem*>((ui::Control*)args.pEventData);
        ASSERT(pListBoxItem != nullptr);
        if ((pListBoxItem != nullptr)) {
            ASSERT(pListBoxItem->GetListBoxIndex() == nItemIndex);
            ASSERT(pListBoxItem->GetElementIndex() == nElementID);
        }
    }
    else {
        ASSERT(0);
    }
    return sInfo;
}

void MainForm::OutputDebugLog(const DString& logMsg)
{
#if defined DUILIB_BUILD_FOR_WIN && defined _DEBUG
    //::OutputDebugString(logMsg.c_str());
#endif
}

