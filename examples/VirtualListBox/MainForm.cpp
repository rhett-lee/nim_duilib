#include "MainForm.h"
#include "DataProvider.h"

#include <vector>

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
    m_pEditChildMarginY(nullptr),
    m_pLabelSelection(nullptr),
    m_pLogCheck(nullptr),
    m_pLogPanel(nullptr),
    m_pEditEventLog(nullptr),
    m_nEventLogLines(0)
{

}

MainForm::~MainForm()
{
    if (m_pDataProvider != nullptr) {
        //避免数据提供者在界面销毁后回调已销毁的窗口
        m_pDataProvider->ClearDataChangedNotify();
        //数据提供者由窗口管理生命周期，窗口销毁时统一释放
        delete m_pDataProvider;
        m_pDataProvider = nullptr;
    }
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
    m_pLabelSelection = dynamic_cast<ui::Label*>(FindControl(_T("label_selection")));
    m_pLogCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_show_log")));
    m_pLogPanel = dynamic_cast<ui::Box*>(FindControl(_T("log_panel")));
    m_pEditEventLog = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_event_log")));

    if (m_pTileList == nullptr) {
        return;
    }

    //事件日志默认隐藏，勾选“显示日志”后才显示
    if (m_pLogCheck != nullptr) {
        m_pLogCheck->Selected((m_pLogPanel != nullptr) && m_pLogPanel->IsVisible(), false);
        m_pLogCheck->AttachSelect([this](const ui::EventArgs& args) {
            return OnShowLogChanged(args);
            });
        m_pLogCheck->AttachUnSelect([this](const ui::EventArgs& args) {
            return OnShowLogChanged(args);
            });
    }

    GetRoot()->AttachBubbledEvent(ui::kEventClick, UiBind(&MainForm::OnClicked, this, std::placeholders::_1), 0);

    // 设置提供者
    m_pDataProvider = new DataProvider;
    m_pDataProvider->SetDataChangedNotify([this]() {
        //数据数量变化时刷新已选择数量的显示
        UpdateSelectionStats();
        });
    m_pTileList->SetDataProvider(m_pDataProvider);

    ui::Layout* pLayout = m_pTileList->GetLayout();
    const ui::LayoutType layoutType = (pLayout != nullptr) ? pLayout->GetLayoutType() : ui::LayoutType::VirtualVTileLayout;
    if ((layoutType != ui::LayoutType::VirtualHTileLayout) &&
        (layoutType != ui::LayoutType::VirtualVTileLayout)){
        //隐藏列的设置，其他的不支持设置列
        if (m_pEditColumn != nullptr) {
            m_pEditColumn->SetEnabled(false);
            m_pEditColumn->SetText(_T("  "));
        }
    }

    if (m_pOptionColumnFix != nullptr) {
        if (layoutType == ui::LayoutType::VirtualHTileLayout) {
            m_pOptionColumnFix->SetTextId(_T("STRID_VIRTUALLISTBOX_FIX_ROWS"));
        }
        else if (layoutType == ui::LayoutType::VirtualVTileLayout) {
            m_pOptionColumnFix->SetTextId(_T("STRID_VIRTUALLISTBOX_FIX_COLS"));
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
    }

    ui::Option* pOptionAlign1 = dynamic_cast<ui::Option*>(FindControl(_T("option_align1")));
    ui::Option* pOptionAlign2 = dynamic_cast<ui::Option*>(FindControl(_T("option_align2")));
    ui::Option* pOptionAlign3 = dynamic_cast<ui::Option*>(FindControl(_T("option_align3")));
    if ((pOptionAlign1 != nullptr) && (pOptionAlign2 != nullptr) && (pOptionAlign3 != nullptr)) {
        if ((layoutType == ui::LayoutType::VirtualHTileLayout) || (layoutType == ui::LayoutType::VirtualHLayout)) {
            //水平布局
            pOptionAlign1->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_TOP"));
            pOptionAlign2->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_CENTER"));
            pOptionAlign3->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_BOTTOM"));
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
            pOptionAlign1->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_LEFT"));
            pOptionAlign2->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_CENTER"));
            pOptionAlign3->SetTextId(_T("STRID_VIRTUALLISTBOX_ALIGN_RIGHT"));
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
        else {
            //其他容器类型直接显示控件的实际类型名
            pListType->SetText(m_pTileList->GetType());
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
    UpdateSelectionStats();

    //测试虚表的事件
    TestVirtualListBoxEvents(m_pTileList);

    ClearEventLog();
    AppendEventLog(GetLangString(_T("STRID_VIRTUALLISTBOX_LOG_READY")));
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();
    if (m_pDataProvider != nullptr) {
        //语言变化时，刷新数据
        m_pDataProvider->Refresh();
        UpdateSelectionStats();
        ClearEventLog();
        AppendEventLog(GetLangString(_T("STRID_VIRTUALLISTBOX_LOG_READY")));
    }
    return bRet;
}

bool MainForm::OnClicked(const ui::EventArgs& args)
{
    ui::Control* pSender = args.GetSender();
    if (pSender == nullptr) {
        return true;
    }
    DString sName = pSender->GetName();
    if (sName == _T("btn_set_total"))
    {
        if (m_pTileList == nullptr || m_pDataProvider == nullptr) {
            return true;
        }
        if ((m_pEditChildMarginX != nullptr) && !m_pEditChildMarginX->GetText().empty()) {
            int32_t nMarginX = ui::StringUtil::StringToInt32(m_pEditChildMarginX->GetText().c_str());
            if (nMarginX >= 0) {
                m_pTileList->SetAttribute(_T("child_margin_x"), m_pEditChildMarginX->GetText());
            }
        }
        if ((m_pEditChildMarginY != nullptr) && !m_pEditChildMarginY->GetText().empty()) {
            int32_t nMarginY = ui::StringUtil::StringToInt32(m_pEditChildMarginY->GetText().c_str());
            if (nMarginY >= 0) {
                m_pTileList->SetAttribute(_T("child_margin_y"), m_pEditChildMarginY->GetText());
            }
        }
        if ((m_pOptionColumnFix != nullptr) && m_pOptionColumnFix->IsSelected() && (m_pEditColumn != nullptr)) {
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

        if (m_pEditTotal != nullptr) {
            int nTotal = ui::StringUtil::StringToInt32(m_pEditTotal->GetText().c_str());
            if (nTotal > 0) {
                m_pDataProvider->SetTotal(nTotal);
            }
        }
        m_pTileList->Refresh();
        UpdateSelectionStats();
    }
    else if (sName == _T("btn_update")) {
        if ((m_pDataProvider == nullptr) || (m_pEditUpdate == nullptr) || (m_pEditTaskName == nullptr)) {
            return true;
        }
        size_t nElementCount = m_pDataProvider->GetElementCount();
        size_t nIndex = 0;
        if (!GetElementIndexFromEdit(m_pEditUpdate, nElementCount, nIndex)) {
            LogInvalidIndex(m_pEditUpdate, nElementCount);
            return true;
        }
        DString sTaskName = m_pEditTaskName->GetText();
        if (sTaskName.empty()) {
            sTaskName = GetLangString(_T("STRID_VIRTUALLISTBOX_TASK_NAME"));
        }
        m_pDataProvider->ChangeTaskName(nIndex, sTaskName);
        if (m_pTileList != nullptr) {
            m_pTileList->EnsureVisible(nIndex, false);
        }
    }
    else if (sName == _T("btn_add_task")) {
        if ((m_pDataProvider == nullptr) || (m_pTileList == nullptr)) {
            return true;
        }
        DString sTaskName = (m_pEditTaskName != nullptr) ? m_pEditTaskName->GetText() : DString();
        if (sTaskName.empty()) {
            sTaskName = GetLangString(_T("STRID_VIRTUALLISTBOX_TASK_NAME"));
        }
        m_pDataProvider->AddTask(sTaskName);
        size_t nElementCount = m_pDataProvider->GetElementCount();
        if (nElementCount > 0) {
            m_pTileList->EnsureVisible(nElementCount - 1, false);
        }
        UpdateSelectionStats();
    }
    else if (sName == _T("btn_delete")) {
        if ((m_pDataProvider == nullptr) || (m_pEditDelete == nullptr)) {
            return true;
        }
        size_t nElementCount = m_pDataProvider->GetElementCount();
        size_t nIndex = 0;
        if (!GetElementIndexFromEdit(m_pEditDelete, nElementCount, nIndex)) {
            LogInvalidIndex(m_pEditDelete, nElementCount);
            return true;
        }
        m_pDataProvider->RemoveTask(nIndex);
        UpdateSelectionStats();
    }
    else if (sName == _T("btn_select_all")) {
        if (m_pTileList != nullptr) {
            m_pTileList->SetSelectAll();
        }
        UpdateSelectionStats();
    }
    else if (sName == _T("btn_select_none")) {
        if (m_pTileList != nullptr) {
            m_pTileList->SetSelectNone();
        }
        UpdateSelectionStats();
    }
    else if (sName == _T("btn_clear_log")) {
        ClearEventLog();
        AppendEventLog(GetLangString(_T("STRID_VIRTUALLISTBOX_LOG_READY")));
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
        AppendEventLog(sInfo);
        if ((args.eventType == ui::kEventSelect) || (args.eventType == ui::kEventSelChanged)) {
            UpdateSelectionStats();
        }
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
    DString sInfo = ui::EventUtils::EventTypeToString(args.eventType);
    while (sInfo.size() < 24) {
        sInfo += _T(" ");
    }
    if (args.eventType == ui::kEventSelect) {
        size_t nNewItemIndex = (size_t)args.wParam;
        size_t nOldItemIndex = (size_t)args.lParam;
        if (nNewItemIndex == ui::Box::InvalidIndex) {
            sInfo += _T("NewItemIndex=Invalid");
        }
        else {
            size_t nNewElementID = pListBox->GetDisplayItemElementIndex(nNewItemIndex);
            sInfo += ui::StringUtil::Printf(_T("NewItemIndex=%zu, NewElementID=%zu"), nNewItemIndex, nNewElementID);
        }
        if (nOldItemIndex != ui::Box::InvalidIndex) {
            size_t nOldElementID = pListBox->GetDisplayItemElementIndex(nOldItemIndex);
            sInfo += ui::StringUtil::Printf(_T("; OldItemIndex=%zu, OldElementID=%zu"), nOldItemIndex, nOldElementID);
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
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu"), nItemIndex, nCalcElementID);
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
            sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu"), nItemIndex, nCalcElementID);
        }
    }
    else if (args.eventType == ui::kEventElementFilled) {
        size_t nItemIndex = (size_t)args.wParam;
        size_t nElementID = (size_t)args.lParam;
        size_t nCalcElementID = pListBox->GetDisplayItemElementIndex(nItemIndex);
        ASSERT(nElementID == nCalcElementID);
        sInfo += ui::StringUtil::Printf(_T("ItemIndex=%zu, ElementID=%zu"), nItemIndex, nCalcElementID);
    }
    else {
        ASSERT(0);
    }
    return sInfo;
}

void MainForm::OutputDebugLog(const DString& logMsg)
{
#if defined DUILIB_BUILD_FOR_WIN && defined _DEBUG
    ::OutputDebugString(logMsg.c_str());
#else
    (void)logMsg;
#endif
}

void MainForm::AppendEventLog(const DString& logMsg)
{
    if (m_pEditEventLog == nullptr) {
        return;
    }
    //限制日志行数，避免长时间操作后内容过多
    const int32_t nMaxLogLines = 40;
    if (m_nEventLogLines >= nMaxLogLines) {
        m_pEditEventLog->SetText(_T(""));
        m_nEventLogLines = 0;
    }
    m_pEditEventLog->AppendText(logMsg + _T("\r\n"), false, true);
    ++m_nEventLogLines;
}

void MainForm::ClearEventLog()
{
    if (m_pEditEventLog != nullptr) {
        m_pEditEventLog->SetText(_T(""));
    }
    m_nEventLogLines = 0;
}

void MainForm::UpdateSelectionStats()
{
    if ((m_pLabelSelection == nullptr) || (m_pTileList == nullptr)) {
        return;
    }
    std::vector<size_t> selectedIndexs;
    m_pTileList->GetSelectedElements(selectedIndexs);
    size_t nSelected = selectedIndexs.size();
    size_t nTotal = m_pTileList->GetElementCount();

    DString format = GetLangString(_T("STRID_VIRTUALLISTBOX_SELECTION_STATS"));
    if (format.empty()) {
        format = _T("Selected %zu / %zu");
    }
    m_pLabelSelection->SetText(ui::StringUtil::Printf(format.c_str(), nSelected, nTotal));
}

bool MainForm::GetElementIndexFromEdit(ui::RichEdit* pEdit, size_t nElementCount, size_t& nIndex) const
{
    if (pEdit == nullptr) {
        return false;
    }
    DString sText = pEdit->GetText();
    if (sText.empty()) {
        return false;
    }
    int32_t nValue = ui::StringUtil::StringToInt32(sText.c_str());
    if ((nValue < 0) || ((size_t)nValue >= nElementCount)) {
        return false;
    }
    nIndex = (size_t)nValue;
    return true;
}

void MainForm::LogInvalidIndex(ui::RichEdit* pEdit, size_t nElementCount)
{
    if (pEdit == nullptr) {
        return;
    }
    DString sText = pEdit->GetText();
    if (sText.empty()) {
        DString format = GetLangString(_T("STRID_VIRTUALLISTBOX_INVALID_INPUT"));
        if (format.empty()) {
            format = _T("Please enter a valid item index");
        }
        AppendEventLog(format);
        return;
    }

    int32_t nValue = ui::StringUtil::StringToInt32(sText.c_str());
    size_t nInvalidValue = (nValue < 0) ? 0 : (size_t)nValue;
    DString format = GetLangString(_T("STRID_VIRTUALLISTBOX_INVALID_INDEX"));
    if (format.empty()) {
        format = _T("Index %zu is out of range (total %zu)");
    }
    AppendEventLog(ui::StringUtil::Printf(format.c_str(), nInvalidValue, nElementCount));
}

bool MainForm::OnShowLogChanged(const ui::EventArgs& /*args*/)
{
    if ((m_pLogPanel == nullptr) || (m_pLogCheck == nullptr)) {
        return true;
    }
    //日志区显示时占用界面底部空间，隐藏后列表自动占满剩余区域
    m_pLogPanel->SetVisible(m_pLogCheck->IsSelected());
    return true;
}

DString MainForm::GetLangString(const DString& stringId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(stringId);
}
