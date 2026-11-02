#include "ListBoxForm.h"
#include "Item.h"

namespace
{
    /** 列表项的总数 */
    const int32_t kItemCount = 300;

    /** 日志区的最大行数：超过后清空重来，避免 RichEdit 文本无限增长 */
    const int32_t kMaxLogLines = 1000;

    /** 列表项图标：按序号循环使用，避免整个列表都是同一个图标 */
    const DString kIconFiles[] = {
        _T("task_doc.svg"),
        _T("task_image.svg"),
        _T("task_code.svg"),
        _T("task_chart.svg"),
    };
}

ListBoxForm::ListBoxForm():
    m_pListBox(nullptr),
    m_pTypeLabel(nullptr),
    m_pItemCountLabel(nullptr),
    m_pSelCountLabel(nullptr),
    m_pMultiCheck(nullptr),
    m_pLogCheck(nullptr),
    m_pLogPanel(nullptr),
    m_pLogView(nullptr),
    m_nLogId(0)
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
    m_pTypeLabel = dynamic_cast<ui::Label*>(FindControl(_T("list_type")));
    m_pItemCountLabel = dynamic_cast<ui::Label*>(FindControl(_T("item_count")));
    m_pSelCountLabel = dynamic_cast<ui::Label*>(FindControl(_T("sel_count")));
    m_pLogView = dynamic_cast<ui::RichEdit*>(FindControl(_T("log_view")));
    m_pMultiCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("chk_multi")));
    m_pLogCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("chk_log")));
    m_pLogPanel = dynamic_cast<ui::Box*>(FindControl(_T("log_panel")));

    //根据实际创建的容器类型，决定列表项的布局参数
    bool bHListBox = dynamic_cast<ui::HListBox*>(m_pListBox) != nullptr;
    bool bVListBox = dynamic_cast<ui::VListBox*>(m_pListBox) != nullptr;
    bool bHTileListBox = dynamic_cast<ui::HTileListBox*>(m_pListBox) != nullptr;
    bool bVTileListBox = dynamic_cast<ui::VTileListBox*>(m_pListBox) != nullptr;

    DString sTypeName = _T("ListBox");
    if (bVTileListBox) {
        sTypeName = _T("VTileListBox");
    }
    else if (bHTileListBox) {
        sTypeName = _T("HTileListBox");
    }
    else if (bVListBox) {
        sTypeName = _T("VListBox");
    }
    else if (bHListBox) {
        sTypeName = _T("HListBox");
    }
    if (m_pTypeLabel != nullptr) {
        m_pTypeLabel->SetText(sTypeName);
    }

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

    for (int32_t i = 0; i < kItemCount; i++) {
        Item* item = new Item(this);
        ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("list_box/item.xml")));

        if (bVListBox) {
            //VListBox: 设置为 拉伸类型
            item->SetFixedWidth(ui::UiFixedInt::MakeStretch(), true, true);
        }
        else if (bHListBox) {
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

        DString title = ui::StringUtil::Printf(_T("Task [%02d]"), i);

        //每个列表项的进度值不同：原来的实现是 std::time(nullptr) % 100，
        //由于所有项在同一秒内创建，取到的值完全相同（界面上全部显示同一个百分比）
        int32_t nProgress = (i * 37 + 11) % 101;

        item->InitSubControls(kIconFiles[i % 4], title, nProgress);
        item->SetOnRemoved(UiBind(&ListBoxForm::OnItemRemoved, this));
        m_pListBox->AddItem(item);
    }

    m_pListBox->SetFocus();

    //多选开关
    if (m_pMultiCheck != nullptr) {
        m_pMultiCheck->Selected(m_pListBox->IsMultiSelect(), false);
        m_pMultiCheck->AttachSelect([this](const ui::EventArgs& args) {
            return OnMultiSelectChanged(args);
            });
        m_pMultiCheck->AttachUnSelect([this](const ui::EventArgs& args) {
            return OnMultiSelectChanged(args);
            });
    }

    //日志区显示开关：默认不显示，界面更简洁
    if (m_pLogCheck != nullptr) {
        m_pLogCheck->Selected((m_pLogPanel != nullptr) && m_pLogPanel->IsVisible(), false);
        m_pLogCheck->AttachSelect([this](const ui::EventArgs& args) {
            return OnShowLogChanged(args);
            });
        m_pLogCheck->AttachUnSelect([this](const ui::EventArgs& args) {
            return OnShowLogChanged(args);
            });
    }

    //清空日志按钮
    ui::Button* pClearLogBtn = dynamic_cast<ui::Button*>(FindControl(_T("clear_log")));
    if (pClearLogBtn != nullptr) {
        pClearLogBtn->AttachClick([this](const ui::EventArgs&) {
            ClearLog();
            return true;
            });
    }

    //测试ListBox的事件
    TestListBoxEvents(m_pListBox);

    RefreshStatus();

    //提示：容器类型由 XML 决定，改标签即可切换，代码无需改动
    PrintLog(ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_LISTBOX_LOG_HINT")));
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
        //选择状态可能变化了，刷新状态栏
        RefreshStatus();
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
    //注意：不挂载 kEventItemMouseEnter / kEventItemMouseLeave。
    //这两个事件在鼠标滑过列表时会逐项触发，数量远大于其他事件，
    //一旦写进日志会瞬间把 Select/Click 等关键事件刷走，失去演示意义。
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
    DString sInfo = ui::EventUtils::EventTypeToString(args.eventType);
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
    PrintLog(logMsg);
}

void ListBoxForm::PrintLog(const DString& logMsg)
{
    if (m_pLogView == nullptr) {
        return;
    }
    if (m_nLogId >= kMaxLogLines) {
        //控制日志规模，避免长时间运行后 RichEdit 文本无限增长
        m_pLogView->SetText(_T(""));
        m_nLogId = 0;
    }
    DString line = ui::StringUtil::Printf(_T("%03d: "), ++m_nLogId);
    line += logMsg;
    line += _T("\n");
    m_pLogView->AppendText(line);
}

void ListBoxForm::ClearLog()
{
    m_nLogId = 0;
    if (m_pLogView != nullptr) {
        m_pLogView->SetText(_T(""));
    }
}

void ListBoxForm::RefreshStatus()
{
    if (m_pListBox == nullptr) {
        return;
    }
    if (m_pItemCountLabel != nullptr) {
        m_pItemCountLabel->SetText(ui::StringUtil::Printf(_T("%d"), (int32_t)m_pListBox->GetItemCount()));
    }
    if (m_pSelCountLabel != nullptr) {
        std::vector<size_t> selectedItems;
        m_pListBox->GetSelectedItems(selectedItems);
        m_pSelCountLabel->SetText(ui::StringUtil::Printf(_T("%d"), (int32_t)selectedItems.size()));
    }
}

void ListBoxForm::OnItemRemoved()
{
    RefreshStatus();
}

bool ListBoxForm::OnShowLogChanged(const ui::EventArgs& args)
{
    if (m_pLogPanel == nullptr) {
        return true;
    }
    //日志区显示时占用界面底部空间，隐藏后列表自动占满剩余区域（布局会重新计算）
    m_pLogPanel->SetVisible((m_pLogCheck != nullptr) && m_pLogCheck->IsSelected());
    return true;
}

bool ListBoxForm::OnMultiSelectChanged(const ui::EventArgs& args)
{
    if ((m_pListBox == nullptr) || (m_pMultiCheck == nullptr)) {
        return true;
    }
    const bool bMultiSelect = m_pMultiCheck->IsSelected();
    m_pListBox->SetMultiSelect(bMultiSelect);
    PrintLog(ui::GlobalManager::Instance().Lang().GetStringByID(bMultiSelect ?
             _T("STRID_LISTBOX_MULTI_ON") : _T("STRID_LISTBOX_MULTI_OFF")));
    RefreshStatus();
    return true;
}
