#include "ListCtrl.h"
#include "duilib/Control/ListCtrlReportView.h"
#include "duilib/Control/ListCtrlIconView.h"
#include "duilib/Control/ListCtrlListView.h"
#include "duilib/Control/RichEdit.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Keyboard.h"
#include <set>

namespace ui
{

ListCtrl::ListCtrl(Window* pWindow):
    VBox(pWindow),
    m_pHeaderCtrl(nullptr),
    m_pReportView(nullptr),
    m_pIconView(nullptr),
    m_pListView(nullptr),
    m_bEnableHeaderDragOrder(true),
    m_nFrozenColumnCount(0),
    m_bShowHeaderCtrl(true),
    m_bEnableRefresh(true),
    m_bMultiSelect(true),
    m_selectionMode(ListCtrlSelectionMode::kRow),
    m_bCellDragSelecting(false),
    m_hoverCell(),
    m_bEnableColumnWidthAuto(true),
    m_bAutoCheckSelect(false),
    m_bHeaderShowCheckBox(false),
    m_bDataItemShowCheckBox(false),
    m_listCtrlType(ListCtrlType::Report),
    m_pRichEdit(nullptr),
    m_bEnableItemEdit(true),
    m_nItemHeight(0),
    m_nHeaderHeight(0),
    m_pData(nullptr),
    m_nSortedColumnId(Box::InvalidIndex),
    m_bSortedUp(false)
{
    size_t nCount = sizeof(m_imageList) / sizeof(m_imageList[0]);
    for (size_t i = 0; i < nCount; ++i) {
        m_imageList[i] = nullptr;
    }

    m_pData = new ListCtrlData;
    m_pData->SetAutoCheckSelect(IsAutoCheckSelect());

    m_pReportView = new ListCtrlReportView(pWindow);
    m_pReportView->SetListCtrl(this);
    m_pReportView->SetDataProvider(m_pData);

    SetDataItemHeight(32, true);
    SetHeaderHeight(32, true);
}

ListCtrl::~ListCtrl()
{
    if (m_pData != nullptr) {
        delete m_pData;
        m_pData = nullptr;
    }
    if (!IsInited() && (m_pReportView != nullptr)) {
        delete m_pReportView;
        m_pReportView = nullptr;
    }
    if (!IsInited() && (m_pIconView != nullptr)) {
        delete m_pIconView;
        m_pIconView = nullptr;
    }
    if (!IsInited() && (m_pListView != nullptr)) {
        delete m_pListView;
        m_pListView = nullptr;
    }
    if (!IsInited() && (m_pRichEdit != nullptr)) {
        delete m_pRichEdit;
        m_pRichEdit = nullptr;
    }
}

DString ListCtrl::GetType() const { return DUI_CTR_LISTCTRL; }

void ListCtrl::SetImageList(ListCtrlType type, const ImageListPtr& spImageList)
{
    if (type == ListCtrlType::Report) {
        m_imageList[0] = spImageList;
    }
    else if (type == ListCtrlType::Icon) {
        m_imageList[1] = spImageList;
    }
    else if (type == ListCtrlType::List) {
        m_imageList[2] = spImageList;
    }
    else {
        ASSERT(0);
    }
}

ImageListPtr ListCtrl::GetImageList(ListCtrlType type) const
{
    if (type == ListCtrlType::Report) {
        return m_imageList[0];
    }
    else if (type == ListCtrlType::Icon) {
        return m_imageList[1];
    }
    else if (type == ListCtrlType::List) {
        return m_imageList[2];
    }
    else {
        return nullptr;
    }
}

void ListCtrl::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("header_class")) {
        SetHeaderClass(strValue);
    }
    else if (strName == _T("header_item_class")) {
        SetHeaderItemClass(strValue);
    }
    else if (strName == _T("header_split_box_class")) {
        SetHeaderSplitBoxClass(strValue);
    }
    else if (strName == _T("header_split_control_class")) {
        SetHeaderSplitControlClass(strValue);
    }
    else if (strName == _T("enable_header_drag_order")) {
        SetEnableHeaderDragOrder(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("frozen_column_count")) {
        SetFrozenColumnCount((size_t)StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("check_box_class")) {
        SetCheckBoxClass(strValue);
    }
    else if (strName == _T("data_item_class")) {
        SetDataItemClass(strValue);
    }
    else if (strName == _T("data_sub_item_class")) {
        SetDataSubItemClass(strValue);
    }
    else if (strName == _T("row_grid_line_width")) {
        //注意：StringToFloat 调用 1 次，提取到局部变量避免重复计算且防止结果不一致
        float fValue = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        ASSERT(fValue >= 0);
        SetRowGridLineWidth(fValue, true);
    }
    else if (strName == _T("row_grid_line_color")) {
        SetRowGridLineColor(strValue);
    }
    else if (strName == _T("column_grid_line_width")) {
        //注意：StringToFloat 调用 1 次，提取到局部变量避免重复计算且防止结果不一致
        float fValue = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        ASSERT(fValue >= 0);
        SetColumnGridLineWidth(fValue, true);
    }
    else if (strName == _T("column_grid_line_color")) {
        SetColumnGridLineColor(strValue);
    }
    else if (strName == _T("report_view_class")) {
        SetReportViewClass(strValue);
    }
    else if (strName == _T("header_height")) {
        SetHeaderHeight(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("data_item_height")) {
        SetDataItemHeight(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("show_header")) {
        SetHeaderVisible(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("multi_select")) {
        SetMultiSelect(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("selection_mode")) {
        if (strValue == _T("cell")) {
            SetSelectionMode(ListCtrlSelectionMode::kCell);
        }
        else if (strValue == _T("row")) {
            SetSelectionMode(ListCtrlSelectionMode::kRow);
        }
    }
    else if (strName == _T("enable_column_width_auto")) {
        SetEnableColumnWidthAuto(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("auto_check_select")) {
        SetAutoCheckSelect(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("show_header_checkbox")) {
        SetHeaderShowCheckBox(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("show_data_item_checkbox")) {
        SetDataItemShowCheckBox(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("type")) {
        if (strValue == _T("report")) {
            SetListCtrlType(ListCtrlType::Report);
        }
        else if (strValue == _T("icon")) {
            SetListCtrlType(ListCtrlType::Icon);
        }
        else if (strValue == _T("list")) {
            SetListCtrlType(ListCtrlType::List);
        }
    }
    else if (strName == _T("icon_view_class")) {
        SetIconViewClass(strValue);
    }
    else if (strName == _T("icon_view_item_class")) {
        SetIconViewItemClass(strValue);
    }
    else if (strName == _T("icon_view_item_image_class")) {
        SetIconViewItemImageClass(strValue);
    }
    else if (strName == _T("icon_view_item_label_class")) {
        SetIconViewItemLabelClass(strValue);
    }
    else if (strName == _T("list_view_class")) {
        SetListViewClass(strValue);
    }
    else if (strName == _T("list_view_item_class")) {
        SetListViewItemClass(strValue);
    }
    else if (strName == _T("list_view_item_image_class")) {
        SetListViewItemImageClass(strValue);
    }
    else if (strName == _T("list_view_item_label_class")) {
        SetListViewItemLabelClass(strValue);
    }
    else if (strName == _T("enable_item_edit")) {
        SetEnableItemEdit(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("list_ctrl_richedit_class")) {
        SetRichEditClass(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void ListCtrl::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }

    //首先禁止界面刷新
    bool bOldValue = SetEnableRefresh(false);
    float fValue = GetRowGridLineWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetRowGridLineWidth(fValue, false);

    fValue = GetColumnGridLineWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetColumnGridLineWidth(fValue, false);

    if (m_nHeaderHeight > 0) {
        m_nHeaderHeight = Dpi().GetScaleInt(m_nHeaderHeight, nOldDpiScale);
    }

    int32_t iValue = GetDataItemHeight();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetDataItemHeight(iValue, false);

    if (m_pData != nullptr) {
        m_pData->ChangeDpiScale(Dpi(), nOldDpiScale);
    }

    size_t nCount = sizeof(m_imageList) / sizeof(m_imageList[0]);
    for (size_t i = 0; i < nCount; ++i) {
        if (m_imageList[i] != nullptr) {
            m_imageList[i]->ChangeDpiScale(Dpi(), nOldDpiScale);
        }
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);

    //更新列宽
    size_t nColumnCount = GetColumnCount();
    for (size_t columnIndex = 0; columnIndex < nColumnCount; ++columnIndex) {
        int32_t nColumnWidth = GetColumnWidth(columnIndex);
        if (nColumnWidth > 0) {
            SetColumnWidth(columnIndex, nColumnWidth, false);
        }        
    }
    //刷新数据
    SetEnableRefresh(bOldValue);
    Refresh();
}

void ListCtrl::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if ((msg.eventType > kEventKeyBegin) && (msg.eventType < kEventKeyEnd)) {
        OnViewKeyboardEvents(msg);
    }
    else if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
        OnViewMouseEvents(msg);
    }
}

void ListCtrl::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    //初始化Body
    ASSERT(m_pReportView != nullptr);
    AddItem(m_pReportView);
    m_pReportView->SetClass(GetReportViewClass());

    //初始化Header
    ASSERT(m_pHeaderCtrl == nullptr);
    if (m_pHeaderCtrl == nullptr) {
        m_pHeaderCtrl = new ListCtrlHeader(GetWindow());
    }
    m_pHeaderCtrl->SetListCtrl(this);
    // Header添加到数据视图中管理，作为第一个元素，在Layout的实现中控制显示属性
    m_pReportView->AddItem(m_pHeaderCtrl);

    if (!m_headerClass.empty()) {
        m_pHeaderCtrl->SetClass(m_headerClass.c_str());
    }
    m_pHeaderCtrl->SetAutoCheckSelect(false);
    m_pHeaderCtrl->SetShowCheckBox(m_bHeaderShowCheckBox); //是否显示CheckBox
    int32_t nHeaderHeight = m_nHeaderHeight;
    if (nHeaderHeight < 0) {
        nHeaderHeight = GetDataItemHeight();
    }
    m_pHeaderCtrl->SetFixedHeight(UiFixedInt(nHeaderHeight), true, false);
    if (!m_bShowHeaderCtrl) {
        SetHeaderVisible(false);
    }

    //同步单选和多选的状态
    m_pReportView->SetDataProvider(m_pData);

    //更新默认的文本属性
    SetDataSubItemClass(GetDataSubItemClass());

    //更新默认的行高
    m_pData->SetDefaultItemHeight(GetDataItemHeight());

    InitReportView();
    DoSetListCtrlType(GetListCtrlType(), true);
}

void ListCtrl::InitReportView()
{
    ASSERT(m_pReportView != nullptr);
    if (m_pReportView == nullptr) {
        return;
    }
    m_pReportView->SetMultiSelect(IsMultiSelect());

    //挂载事件，转接给外层
    m_pReportView->AttachSelect([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachSelChanged([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachItemMouseEnter([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachItemMouseLeave([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachDoubleClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachRClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachReturn([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachKeyDown([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });
    m_pReportView->AttachKeyUp([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Report, args);
        return true;
        });

    m_pReportView->AttachReportViewItemFilled([this](const EventArgs& args) {
        EventArgs newMsg = args;
        newMsg.SetSender(this);
        SendEventMsg(newMsg);
        return true;
        });
    m_pReportView->AttachReportViewSubItemFilled([this](const EventArgs& args) {
        EventArgs newMsg = args;
        newMsg.SetSender(this);
        SendEventMsg(newMsg);
        return true;
        });
    m_pReportView->AttachPosChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::Report;
        newMsg.pEventData = m_pReportView;
        SendEvent(kEventViewPosChanged, newMsg);
        return true;
        });
    m_pReportView->AttachSizeChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::Report;
        newMsg.pEventData = m_pReportView;
        SendEvent(kEventViewSizeChanged, newMsg);
        return true;
        });
}

void ListCtrl::InitIconView()
{
    ASSERT(m_pIconView != nullptr);
    if (m_pIconView == nullptr) {
        return;
    }
    m_pIconView->SetMultiSelect(IsMultiSelect());

    //挂载事件，转接给外层
    m_pIconView->AttachSelect([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachSelChanged([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachItemMouseEnter([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachItemMouseLeave([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachDoubleClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachRClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachReturn([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachKeyDown([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });
    m_pIconView->AttachKeyUp([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::Icon, args);
        return true;
        });

    m_pIconView->AttachIconViewItemFilled([this](const EventArgs& args) {
        EventArgs newMsg = args;
        newMsg.SetSender(this);
        SendEventMsg(newMsg);
        return true;
        });
    m_pIconView->AttachPosChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::Icon;
        newMsg.pEventData = m_pIconView;
        SendEvent(kEventViewPosChanged, newMsg);
        return true;
        });
    m_pIconView->AttachSizeChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::Icon;
        newMsg.pEventData = m_pIconView;
        SendEvent(kEventViewSizeChanged, newMsg);
        return true;
        });
}

void ListCtrl::InitListView()
{
    ASSERT(m_pListView != nullptr);
    if (m_pListView == nullptr) {
        return;
    }
    if (m_pIconView != nullptr) {
        m_pIconView->SetMultiSelect(IsMultiSelect());
    }

    //挂载事件，转接给外层
    m_pListView->AttachSelect([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachSelChanged([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachItemMouseEnter([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachItemMouseLeave([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachDoubleClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachRClick([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachReturn([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachKeyDown([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });
    m_pListView->AttachKeyUp([this](const EventArgs& args) {
        OnListCtrlViewEvent(ListCtrlType::List, args);
        return true;
        });

    m_pListView->AttachListViewItemFilled([this](const EventArgs& args) {
        EventArgs newMsg = args;
        newMsg.SetSender(this);
        SendEventMsg(newMsg);
        return true;
        });
    m_pListView->AttachPosChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::List;
        newMsg.pEventData = m_pListView;
        SendEvent(kEventViewPosChanged, newMsg);
        return true;
        });
    m_pListView->AttachSizeChanged([this](const EventArgs&) {
        EventArgs newMsg;
        newMsg.listCtrlType = (int32_t)ListCtrlType::List;
        newMsg.pEventData = m_pListView;
        SendEvent(kEventViewSizeChanged, newMsg);
        return true;
        });
}

void ListCtrl::OnListCtrlViewEvent(ListCtrlType listCtrlType, const EventArgs& args)
{
    EventArgs msg = args;
    msg.SetSender(this);
    msg.listCtrlType = (int32_t)listCtrlType;//列表视图类型
    msg.pEventData = nullptr;

    if (args.eventType != kEventSelChanged) {
        if (listCtrlType == ListCtrlType::Report) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pReportView->GetItemAt(nItemIndex);
            ListCtrlItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlItem*>(pControl);
            }
            if ((pItem == nullptr) && (nItemIndex == 0)) {
                if (dynamic_cast<ListCtrlHeader*>(pControl) != nullptr) {
                    //在表头时，不触发事件
                    return;
                }
            }
            if (nItemIndex != Box::InvalidIndex) {
                ASSERT(pItem != nullptr);
            }
            if (pItem != nullptr) {
                ASSERT(pItem->GetDataItemIndex() == (size_t)msg.lParam);
                msg.pEventData = pItem;
            }
        }
        else if (listCtrlType == ListCtrlType::Icon) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pIconView->GetItemAt(nItemIndex);
            ListCtrlIconViewItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlIconViewItem*>(pControl);
            }
            if (nItemIndex != Box::InvalidIndex) {
                ASSERT(pItem != nullptr);
            }
            if (pItem != nullptr) {
                ASSERT(pItem->GetDataItemIndex() == (size_t)msg.lParam);
                msg.pEventData = pItem;
            }
        }
        else if (listCtrlType == ListCtrlType::List) {
            size_t nItemIndex = args.wParam;
            Control* pControl = m_pListView->GetItemAt(nItemIndex);
            ListCtrlListViewItem* pItem = nullptr;
            if (pControl != nullptr) {
                pItem = dynamic_cast<ListCtrlListViewItem*>(pControl);
            }
            if (nItemIndex != Box::InvalidIndex) {
                ASSERT(pItem != nullptr);
            }
            if (pItem != nullptr) {
                ASSERT(pItem->GetDataItemIndex() == (size_t)msg.lParam);
                msg.pEventData = pItem;
            }
        }
        else {
            ASSERT(0);
            return;
        }
    }

    //触发事件给应用层
    SendEventMsg(msg);
}

void ListCtrl::SetListCtrlType(ListCtrlType type)
{
    DoSetListCtrlType(type, false);
}

void ListCtrl::DoSetListCtrlType(ListCtrlType type, bool bInit)
{
    ListCtrlType oldListCtrlType = m_listCtrlType;
    m_listCtrlType = type;
    if (!IsInited()) {
        return;
    }
    void* pEventData = nullptr;
    if (m_listCtrlType == ListCtrlType::Report) {
        //切换为Report视图
        ASSERT(m_pReportView != nullptr);
        if (m_pReportView != nullptr) {
            m_pReportView->SetDataProvider(m_pData);
            m_pData->SetListView(m_pReportView);
            m_pReportView->SetVisible(true);
            if (m_pReportView->IsMultiSelect() != IsMultiSelect()) {
                m_pReportView->SetMultiSelect(IsMultiSelect());
            }
        }
        if (m_pIconView != nullptr) {
            m_pIconView->SetVisible(false);
            m_pIconView->SetDataProvider(nullptr);            
        }
        if (m_pListView != nullptr) {
            m_pListView->SetVisible(false);
            m_pListView->SetDataProvider(nullptr);
        }
        pEventData = m_pReportView;
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        //切换为Icon视图
        if (m_pIconView == nullptr) {
            m_pIconView = new ListCtrlIconView(GetWindow());
            m_pIconView->SetListCtrl(this);
            AddItem(m_pIconView);
            m_pIconView->SetClass(GetIconViewClass());
            InitIconView();
        }
        m_pIconView->SetDataProvider(m_pData);
        m_pData->SetListView(m_pIconView);
        m_pIconView->SetVisible(true);
        if (m_pIconView->IsMultiSelect() != IsMultiSelect()) {
            m_pIconView->SetMultiSelect(IsMultiSelect());
        }
        if (m_pReportView != nullptr) {
            m_pReportView->SetVisible(false);
            m_pReportView->SetDataProvider(nullptr);
        }
        if (m_pListView != nullptr) {
            m_pListView->SetVisible(false);
            m_pListView->SetDataProvider(nullptr);
        }
        pEventData = m_pIconView;
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        //切换为List视图
        if (m_pListView == nullptr) {
            m_pListView = new ListCtrlListView(GetWindow());
            m_pListView->SetListCtrl(this);
            AddItem(m_pListView);
            m_pListView->SetClass(GetListViewClass());
            InitListView();
        }
        m_pListView->SetDataProvider(m_pData);
        m_pData->SetListView(m_pListView);
        m_pListView->SetVisible(true);
        if (m_pListView->IsMultiSelect() != IsMultiSelect()) {
            m_pListView->SetMultiSelect(IsMultiSelect());
        }
        if (m_pReportView != nullptr) {
            m_pReportView->SetVisible(false);
            m_pReportView->SetDataProvider(nullptr);
        }
        if (m_pIconView != nullptr) {
            m_pIconView->SetVisible(false);
            m_pIconView->SetDataProvider(nullptr);
        }
        pEventData = m_pListView;
    }

    //触发视图类型变化事件
    ListCtrlType newListCtrlType = m_listCtrlType;
    if (bInit || (oldListCtrlType != newListCtrlType)) {
        EventArgs msg;
        msg.wParam = (WPARAM)newListCtrlType;
        msg.lParam = (LPARAM)oldListCtrlType;
        msg.pEventData = pEventData;
        msg.listCtrlType = (int32_t)newListCtrlType;
        SendEvent(kEventViewTypeChanged, msg);
    }
}

ListCtrlType ListCtrl::GetListCtrlType() const
{
    return m_listCtrlType;
}

ListCtrlReportView* ListCtrl::GetReportView() const
{
    return m_pReportView;
}

ListCtrlIconView* ListCtrl::GetIconView() const
{
    return m_pIconView;
}

ListCtrlListView* ListCtrl::GetListView() const
{
    return m_pListView;
}

void ListCtrl::SetHeaderClass(const DString& className)
{
    m_headerClass = className;
    if (IsInited() && (m_pHeaderCtrl != nullptr)) {
        m_pHeaderCtrl->SetClass(className);
    }
}

void ListCtrl::SetHeaderItemClass(const DString& className)
{
    m_headerItemClass = className;
}

DString ListCtrl::GetHeaderItemClass() const
{
    return m_headerItemClass.c_str();
}

void ListCtrl::SetHeaderSplitBoxClass(const DString& className)
{
    m_headerSplitBoxClass = className;
}

DString ListCtrl::GetHeaderSplitBoxClass() const
{
    return m_headerSplitBoxClass.c_str();
}

void ListCtrl::SetHeaderSplitControlClass(const DString& className)
{
    m_headerSplitControlClass = className;
}

DString ListCtrl::GetHeaderSplitControlClass() const
{
    return m_headerSplitControlClass.c_str();
}

void ListCtrl::SetCheckBoxClass(const DString& className)
{
    m_checkBoxClass = className;
}

DString ListCtrl::GetCheckBoxClass() const
{
    return m_checkBoxClass.c_str();
}

void ListCtrl::SetReportViewClass(const DString& className)
{
    m_reportViewClass = className;
    if (IsInited() && (m_pReportView != nullptr)) {
        m_pReportView->SetClass(className);
    }
}

DString ListCtrl::GetReportViewClass() const
{
    return m_reportViewClass.c_str();
}

void ListCtrl::SetDataItemClass(const DString& className)
{
    m_dataItemClass = className;
}

DString ListCtrl::GetDataItemClass() const
{
    return m_dataItemClass.c_str();
}

void ListCtrl::SetDataSubItemClass(const DString& className)
{
    m_dataSubItemClass = className;
    if (IsInited() && !className.empty()) {
        ListCtrlSubItem defaultSubItem(GetWindow());
        defaultSubItem.SetClass(className);
        m_pData->SetDefaultTextStyle(defaultSubItem.GetTextStyle());
    }
}

DString ListCtrl::GetDataSubItemClass() const
{
    return m_dataSubItemClass.c_str();
}

void ListCtrl::SetIconViewClass(const DString& className)
{
    m_iconViewClass = className;
    if (IsInited() && (m_pIconView != nullptr)) {
        m_pIconView->SetClass(className);
    }
}

DString ListCtrl::GetIconViewClass() const
{
    return m_iconViewClass.c_str();
}

void ListCtrl::SetIconViewItemClass(const DString& className)
{
    m_iconViewItemClass = className;
}

DString ListCtrl::GetIconViewItemClass() const
{
    return m_iconViewItemClass.c_str();
}

void ListCtrl::SetIconViewItemImageClass(const DString& className)
{
    m_iconViewItemImageClass = className;
}

DString ListCtrl::GetIconViewItemImageClass() const
{
    return m_iconViewItemImageClass.c_str();
}

void ListCtrl::SetIconViewItemLabelClass(const DString& className)
{
    m_iconViewItemLabelClass = className;
}

DString ListCtrl::GetIconViewItemLabelClass() const
{
    return m_iconViewItemLabelClass.c_str();
}

void ListCtrl::SetListViewClass(const DString& className)
{
    m_listViewClass = className;
    if (IsInited() && (m_pListView != nullptr)) {
        m_pListView->SetClass(className);
    }
}

DString ListCtrl::GetListViewClass() const
{
    return m_listViewClass.c_str();
}

void ListCtrl::SetListViewItemClass(const DString& className)
{
    m_listViewItemClass = className;
}

DString ListCtrl::GetListViewItemClass() const
{
    return m_listViewItemClass.c_str();
}

void ListCtrl::SetListViewItemImageClass(const DString& className)
{
    m_listViewItemImageClass = className;
}

DString ListCtrl::GetListViewItemImageClass() const
{
    return m_listViewItemImageClass.c_str();
}

void ListCtrl::SetListViewItemLabelClass(const DString& className)
{
    m_listViewItemLabelClass = className;
}

DString ListCtrl::GetListViewItemLabelClass() const
{
    return m_listViewItemLabelClass.c_str();
}

void ListCtrl::SetRichEditClass(const DString& richEditClass)
{
    if (m_listCtrlRichEditClass != richEditClass) {
        m_listCtrlRichEditClass = richEditClass;
        if (IsInited() && (m_pRichEdit != nullptr)) {
            m_pRichEdit->SetClass(richEditClass);
        }
    }
}

void ListCtrl::SetEnableItemEdit(bool bEnableItemEdit)
{
    m_bEnableItemEdit = bEnableItemEdit;
}

bool ListCtrl::IsEnableItemEdit() const
{
    return m_bEnableItemEdit;
}

RichEdit* ListCtrl::GetRichEdit() const
{
    return m_pRichEdit;
}

DString ListCtrl::GetRichEditClass() const
{
    return m_listCtrlRichEditClass.c_str();
}

void ListCtrl::SetRowGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    m_pReportView->SetRowGridLineWidth(fLineWidth, bNeedDpiScale);
}

float ListCtrl::GetRowGridLineWidth() const
{
    return m_pReportView->GetRowGridLineWidth();
}

void ListCtrl::SetRowGridLineColor(const DString& color)
{
    m_pReportView->SetRowGridLineColor(color);
}

DString ListCtrl::GetRowGridLineColor() const
{
    return m_pReportView->GetRowGridLineColor();
}

void ListCtrl::SetColumnGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    m_pReportView->SetColumnGridLineWidth(fLineWidth, bNeedDpiScale);
}

float ListCtrl::GetColumnGridLineWidth() const
{
    return m_pReportView->GetColumnGridLineWidth();
}

void ListCtrl::SetColumnGridLineColor(const DString& color)
{
    m_pReportView->SetColumnGridLineColor(color);
}

DString ListCtrl::GetColumnGridLineColor() const
{
    return m_pReportView->GetColumnGridLineColor();
}

void ListCtrl::SetEnableColumnWidthAuto(bool bEnable)
{
    m_bEnableColumnWidthAuto = bEnable;
}

bool ListCtrl::IsEnableColumnWidthAuto() const
{
    return m_bEnableColumnWidthAuto;
}

ListCtrlHeaderItem* ListCtrl::InsertColumn(int32_t columnIndex, const ListCtrlColumn& columnInfo)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->InsertColumn(columnIndex, columnInfo);
    }
}

size_t ListCtrl::GetColumnCount() const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return 0;
    }
    else {
        return m_pHeaderCtrl->GetColumnCount();
    }
}

int32_t ListCtrl::GetColumnWidth(size_t columnIndex) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return 0;
    }
    else {
        return m_pHeaderCtrl->GetColumnWidth(columnIndex);
    }
}

int32_t ListCtrl::GetColumnWidthById(size_t columnId) const
{
    size_t columnIndex = GetColumnIndex(columnId);
    if (Box::IsValidItemIndex(columnIndex)) {
        return GetColumnWidth(columnIndex);
    }
    return 0;
}

bool ListCtrl::SetColumnWidth(size_t columnIndex, int32_t nWidth, bool bNeedDpiScale)
{
    bool bRet = false;
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl != nullptr) {
        bRet = m_pHeaderCtrl->SetColumnWidth(columnIndex, nWidth, bNeedDpiScale);
    }
    if(bRet) {
        OnColumnWidthChanged(GetColumnId(columnIndex), Box::InvalidIndex);
    }
    return true;
}

bool ListCtrl::SetColumnWidthById(size_t columnId, int32_t nWidth, bool bNeedDpiScale)
{
    size_t columnIndex = GetColumnIndex(columnId);
    if (Box::IsValidItemIndex(columnIndex)) {
        return SetColumnWidth(columnIndex, nWidth, bNeedDpiScale);
    }
    return false;
}

bool ListCtrl::SetColumnWidthAuto(size_t columnIndex)
{
    bool bRet = false;
    size_t nColumnId = GetColumnId(columnIndex);
    if (nColumnId == Box::InvalidIndex) {
        return bRet;
    }
    //计算该列的宽度
    int32_t nMaxWidth = m_pData->GetMaxColumnWidth(nColumnId);
    if (nMaxWidth > 0) {
        bRet = SetColumnWidth(columnIndex, nMaxWidth, false);
    }
    return bRet;
}

bool ListCtrl::SetColumnWidthAutoById(size_t columnId)
{
    size_t columnIndex = GetColumnIndex(columnId);
    if (Box::IsValidItemIndex(columnIndex)) {
        return SetColumnWidthAuto(columnIndex);
    }
    return false;
}

bool ListCtrl::SetColumnWidth(const std::vector<UiFixedInt>& columnWidthList, bool bNeedDpiScale)
{
    if (columnWidthList.empty()) {
        return false;
    }
    bool bRet = true;
    std::map<size_t, UiFixedInt> stretchWidthMap;
    const size_t nColumnWidthCount = std::min(columnWidthList.size(), GetColumnCount());
    if (nColumnWidthCount != columnWidthList.size()) {
        bRet = false;
    }
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnWidthCount; ++nColumnIndex) {
        const UiFixedInt& fixedInt = columnWidthList[nColumnIndex];
        if (fixedInt.IsInt32()) {
            //固定类型
            if (fixedInt.GetInt32() < 0) {
                bRet = false;
            }
            else if (!SetColumnWidth(nColumnIndex, fixedInt.GetInt32(), bNeedDpiScale)) {
                bRet = false;
            }
        }
        else if (fixedInt.IsAuto()) {
            //自动适应宽度
            if (!SetColumnWidthAuto(nColumnIndex)) {
                bRet = false;
            }
        }
        else if (fixedInt.IsStretch()) {
            //拉伸类型
            stretchWidthMap[nColumnIndex] = fixedInt;
        }
    }

    //计算拉伸类型的各个列的宽度
    if (!stretchWidthMap.empty()) {
        if ((m_pReportView != nullptr) && (m_pHeaderCtrl != nullptr)) {            
            int32_t nToltalFixedWidth = 0;
            const size_t nColumnCount = m_pHeaderCtrl->GetColumnCount();
            for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
                if (stretchWidthMap.find(nColumnIndex) == stretchWidthMap.end()) {
                    //非拉伸类型的列
                    ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(nColumnIndex);
                    if ((pHeaderItem != nullptr) && pHeaderItem->IsColumnVisible()) {
                        nToltalFixedWidth += pHeaderItem->GetColumnWidth();
                    }
                }
            }

            UiRect rcViewRect = m_pReportView->GetRect();
            UiPadding rcViewPadding = m_pReportView->GetPadding();
            UiPadding rcHeaderPadding = m_pHeaderCtrl->GetPadding();
            rcViewRect.Deflate(rcViewPadding);
            rcViewRect.Deflate(rcHeaderPadding);
            const int32_t nAvailableWidth = rcViewRect.Width() - nToltalFixedWidth; //可分配宽度            
            if (nAvailableWidth >= (int32_t)stretchWidthMap.size()) {
                //首先计算总的比例
                int32_t nStretchTotal = 0;
                for (const auto& iter : stretchWidthMap) {
                    nStretchTotal += iter.second.GetStretchPercentValue();
                }
                const int32_t nStretchBase = std::max(100, nStretchTotal);
                int32_t nAvailableSpace = nAvailableWidth;
                int32_t nAvailableCount = (int32_t)stretchWidthMap.size();
                for (const auto& iter : stretchWidthMap) {
                    const size_t nColumnIndex = iter.first;
                    int32_t nColunmWidth = iter.second.GetStretchPercentValue() * nAvailableWidth / nStretchBase;
                    if ((nStretchTotal >= 100) && (nAvailableCount == 1)) {
                        //需要完全分配，且是最后一个拉伸列时
                        nColunmWidth = nAvailableSpace;
                    }

                    //调整列的宽度
                    SetColumnWidth(nColumnIndex, nColunmWidth, false);

                    nAvailableSpace -= nColunmWidth;
                    --nAvailableCount;
                }
            }
            else {
                bRet = false;
            }
        }
        else {
            bRet = false;
        }
    }
    return bRet;
}

ListCtrlHeaderItem* ListCtrl::GetColumn(size_t columnIndex) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->GetColumn(columnIndex);
    }
}

ListCtrlHeaderItem* ListCtrl::GetColumnById(size_t columnId) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return nullptr;
    }
    else {
        return m_pHeaderCtrl->GetColumnById(columnId);
    }
}

size_t ListCtrl::GetColumnIndex(size_t columnId) const
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return Box::InvalidIndex;
    }
    else {
        return m_pHeaderCtrl->GetColumnIndex(columnId);
    }
}

size_t ListCtrl::GetColumnId(size_t columnIndex) const
{
    ListCtrlHeaderItem* pHeaderItem = GetColumn(columnIndex);
    if (pHeaderItem != nullptr) {
        return pHeaderItem->GetColumnId();
    }
    return Box::InvalidIndex;
}

bool ListCtrl::IsValidColumnId(size_t columnId) const
{
    size_t nColumnIndex = GetColumnIndex(columnId);
    return nColumnIndex != Box::InvalidIndex;
}

bool ListCtrl::DeleteColumn(size_t columnIndex)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return false;
    }
    else {
        return m_pHeaderCtrl->DeleteColumn(columnIndex);
    }
}

bool ListCtrl::DeleteColumnById(size_t columnId)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return false;
    }
    else {
        return m_pHeaderCtrl->DeleteColumnById(columnId);
    }
}

void ListCtrl::DeleteAllColumns()
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl != nullptr) {
        size_t nColumnCount = m_pHeaderCtrl->GetColumnCount();
        for (size_t nColumn = 0; nColumn < nColumnCount; ++nColumn) {
            m_pHeaderCtrl->DeleteColumn(0);
        }
    }
}

ListCtrlHeader* ListCtrl::GetHeaderCtrl() const
{
    return m_pHeaderCtrl;
}

void ListCtrl::SetEnableHeaderDragOrder(bool bEnable)
{
    m_bEnableHeaderDragOrder = bEnable;
}

bool ListCtrl::IsEnableHeaderDragOrder() const
{
    return m_bEnableHeaderDragOrder;
}

void ListCtrl::SetFrozenColumnCount(size_t nCount)
{
    if (m_nFrozenColumnCount != nCount) {
        m_nFrozenColumnCount = nCount;
        Refresh();
    }
}

size_t ListCtrl::GetFrozenColumnCount() const
{
    return m_nFrozenColumnCount;
}

void ListCtrl::SetHeaderVisible(bool bVisible)
{
    m_bShowHeaderCtrl = bVisible;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetVisible(bVisible);
    }
    Refresh();
}

bool ListCtrl::IsHeaderVisible() const
{
    if (m_pHeaderCtrl != nullptr) {
        return m_pHeaderCtrl->IsVisible();
    }
    else if(!IsInited()){
        return m_bShowHeaderCtrl;
    }
    else {
        return false;
    }
}

void ListCtrl::SetHeaderHeight(int32_t nHeaderHeight, bool bNeedDpiScale)
{
    if (nHeaderHeight < 0) {
        nHeaderHeight = 0;
    }
    //注意：增加上限检查，防止 XML 中传入超大值（>10000）导致 DPI 缩放后整数溢出及显示异常
    if (nHeaderHeight > 10000) {
        ASSERT(!"ListCtrl::SetHeaderHeight: nHeaderHeight too large, clamp to 10000");
        nHeaderHeight = 10000;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nHeaderHeight);
    }
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetFixedHeight(UiFixedInt(nHeaderHeight), true, false);
        RelayoutOrRedraw();
    }
    m_nHeaderHeight = nHeaderHeight;
    Refresh();
}

int32_t ListCtrl::GetHeaderHeight() const
{
    int32_t nHeaderHeight = 0;
    if (m_pHeaderCtrl != nullptr) {
        nHeaderHeight = m_pHeaderCtrl->GetFixedHeight().GetInt32();
    }
    else {
        nHeaderHeight = m_nHeaderHeight;
        if (nHeaderHeight < 0) {
            nHeaderHeight = GetDataItemHeight();
        }
    }
    return nHeaderHeight;
}

void ListCtrl::SetDataItemHeight(int32_t nItemHeight, bool bNeedDpiScale)
{
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return;
    }
    //注意：增加上限检查，防止 XML 中传入超大值（>10000）导致 DPI 缩放后整数溢出及显示异常
    if (nItemHeight > 10000) {
        ASSERT(!"ListCtrl::SetDataItemHeight: nItemHeight too large, clamp to 10000");
        nItemHeight = 10000;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nItemHeight);
    }
    if (m_nItemHeight != nItemHeight) {
        m_nItemHeight = nItemHeight;
        //更新默认的行高
        m_pData->SetDefaultItemHeight(nItemHeight);
        Refresh();
    }
}

int32_t ListCtrl::GetDataItemHeight() const
{
    return m_nItemHeight;
}

ListCtrlItem* ListCtrl::GetFirstDisplayItem() const
{
    ListCtrlItem* pItem = nullptr;
    ASSERT(m_pReportView != nullptr);
    if (m_pReportView != nullptr) {
        size_t itemCount = m_pReportView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlItem*>(m_pReportView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlItem* ListCtrl::GetNextDisplayItem(ListCtrlItem* pItem) const
{
    ListCtrlItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayItem();
    }
    else {
        ASSERT(m_pReportView != nullptr);
        if (m_pReportView != nullptr) {
            size_t itemCount = m_pReportView->GetItemCount();
            size_t nStartIndex = m_pReportView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlItem* pCheckItem = dynamic_cast<ListCtrlItem*>(m_pReportView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

ListCtrlIconViewItem* ListCtrl::GetFirstDisplayIconItem() const
{
    ListCtrlIconViewItem* pItem = nullptr;
    ASSERT(m_pIconView != nullptr);
    if (m_pIconView != nullptr) {
        size_t itemCount = m_pIconView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlIconViewItem*>(m_pIconView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlIconViewItem* ListCtrl::GetNextDisplayIconItem(ListCtrlIconViewItem* pItem) const
{
    ListCtrlIconViewItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayIconItem();
    }
    else {
        ASSERT(m_pIconView != nullptr);
        if (m_pIconView != nullptr) {
            size_t itemCount = m_pIconView->GetItemCount();
            size_t nStartIndex = m_pIconView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlIconViewItem* pCheckItem = dynamic_cast<ListCtrlIconViewItem*>(m_pIconView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

ListCtrlListViewItem* ListCtrl::GetFirstDisplayListItem() const
{
    ListCtrlListViewItem* pItem = nullptr;
    ASSERT(m_pListView != nullptr);
    if (m_pListView != nullptr) {
        size_t itemCount = m_pListView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlListViewItem*>(m_pListView->GetItemAt(index));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                break;
            }
        }
    }
    return pItem;
}

ListCtrlListViewItem* ListCtrl::GetNextDisplayListItem(ListCtrlListViewItem* pItem) const
{
    ListCtrlListViewItem* pNextItem = nullptr;
    if (pItem == nullptr) {
        pNextItem = GetFirstDisplayListItem();
    }
    else {
        ASSERT(m_pListView != nullptr);
        if (m_pListView != nullptr) {
            size_t itemCount = m_pListView->GetItemCount();
            size_t nStartIndex = m_pListView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlListViewItem* pCheckItem = dynamic_cast<ListCtrlListViewItem*>(m_pListView->GetItemAt(index));
                    if ((pCheckItem != nullptr) && pCheckItem->IsVisible()) {
                        pNextItem = pCheckItem;
                        break;
                    }
                }
            }
        }
    }
    return pNextItem;
}

void ListCtrl::OnColumnWidthChanged(size_t nColumnId1, size_t nColumnId2)
{
    if ((m_pReportView == nullptr) || (m_pHeaderCtrl == nullptr)){
        return;
    }

    size_t nColumn1 = Box::InvalidIndex;
    size_t nColumn2 = Box::InvalidIndex;
    int32_t nColumnWidth1 = -1;
    int32_t nColumnWidth2 = -1;
    if (!m_pHeaderCtrl->GetColumnInfo(nColumnId1, nColumn1, nColumnWidth1)) {
        nColumnWidth1 = -1;
    }
    if (!m_pHeaderCtrl->GetColumnInfo(nColumnId2, nColumn2, nColumnWidth2)) {
        nColumnWidth2 = -1;
    }
    std::map<size_t, int32_t> subItemWidths;
    if (nColumnWidth1 >= 0) {
        subItemWidths[nColumn1] = nColumnWidth1;
    }
    if (nColumnWidth2 >= 0) {
        subItemWidths[nColumn2] = nColumnWidth2;
    }
    if (subItemWidths.empty()) {
        return;
    }
    m_pReportView->AdjustSubItemWidth(subItemWidths);
    Arrange();
    //列宽发生变化，通知应用层（可用于持久化列宽）
    SendEvent(kEventColumnWidthChanged, (WPARAM)nColumnId1, (LPARAM)nColumnId2);
}

void ListCtrl::OnHeaderColumnAdded(size_t nColumnId)
{
    m_pData->AddColumn(nColumnId);
}

void ListCtrl::OnHeaderColumnRemoved(size_t nColumnId)
{
    m_columnSortFlagMap.erase(nColumnId);
    m_pData->RemoveColumn(nColumnId);
}

void ListCtrl::OnColumnSorted(size_t nColumnId, bool bSortedUp)
{
    //对数据排序，然后刷新界面显示
    m_nSortedColumnId = nColumnId;
    m_bSortedUp = bSortedUp;
    uint8_t nSortFlag = GetColumnSortFlagById(m_nSortedColumnId);
    m_pData->SortDataItems(nColumnId, GetColumnIndex(nColumnId), bSortedUp, nSortFlag, nullptr, nullptr);
    Refresh();
    //列排序状态发生变化，通知应用层（可用于持久化排序状态）
    SendEvent(kEventColumnSorted, (WPARAM)nColumnId, (LPARAM)(bSortedUp ? 1 : 0));
}

bool ListCtrl::GetSortColumn(size_t& nSortColumnId, bool& bSortUp) const
{
    nSortColumnId = Box::InvalidIndex;
    if (Box::IsValidItemIndex(GetColumnIndex(m_nSortedColumnId))) {
        nSortColumnId = m_nSortedColumnId;
        bSortUp = m_bSortedUp;
        return true;
    }
    return false;
}

bool ListCtrl::SortDataItems(size_t columnIndex, bool bSortedUp, uint8_t nSortFlag,
                             ListCtrlDataCompareFunc pfnCompareFunc,
                             void* pUserData)
{
    size_t nColumnId = GetColumnId(columnIndex);
    ASSERT(nColumnId != Box::InvalidIndex);
    if (nColumnId == Box::InvalidIndex) {
        return false;
    }
    return SortDataItemsById(nColumnId, bSortedUp, nSortFlag, pfnCompareFunc, pUserData);
}

bool ListCtrl::SortDataItemsById(size_t columnId, bool bSortedUp, uint8_t nSortFlag,
                                 ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    size_t columnIndex = GetColumnIndex(columnId);
    ASSERT(columnIndex != Box::InvalidIndex);
    if (columnIndex == Box::InvalidIndex) {
        return false;
    }
    m_nSortedColumnId = columnId;
    m_bSortedUp = bSortedUp;
    SetColumnSortFlagById(columnId, nSortFlag);
    if (m_pHeaderCtrl != nullptr) {
        //更新UI排序显示
        m_pHeaderCtrl->SetSortColumnId(columnId, bSortedUp, false);
    }    
    bool bRet = m_pData->SortDataItems(columnId, columnIndex, bSortedUp, nSortFlag, pfnCompareFunc, pUserData);
    if (bRet) {
        //列排序状态发生变化，通知应用层（可用于持久化排序状态）
        SendEvent(kEventColumnSorted, (WPARAM)columnId, (LPARAM)(bSortedUp ? 1 : 0));
    }
    return bRet;
}

void ListCtrl::SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    m_pData->SetSortCompareFunction(pfnCompareFunc, pUserData);
}

void ListCtrl::SetColumnSortFlag(size_t columnIndex, uint8_t nSortFlag)
{
    SetColumnSortFlagById(GetColumnId(columnIndex), nSortFlag);
}

uint8_t ListCtrl::GetColumnSortFlag(size_t columnIndex)
{
    return GetColumnSortFlagById(GetColumnId(columnIndex));
}

void ListCtrl::SetColumnSortFlagById(size_t columnId, uint8_t nSortFlag)
{
    if (IsValidColumnId(columnId)) {
        m_columnSortFlagMap[columnId] = nSortFlag;
    }
}

uint8_t ListCtrl::GetColumnSortFlagById(size_t columnId)
{
    auto iter = m_columnSortFlagMap.find(columnId);
    if (iter != m_columnSortFlagMap.end()) {
        return iter->second;
    }
    return ListCtrlSubItemSortFlag::kDefault;
}

void ListCtrl::OnHeaderColumnOrderChanged()
{
    Refresh();
    //列的显示顺序发生变化，通知应用层（可用于持久化列顺序）
    SendEvent(kEventColumnOrderChanged, 0, 0);
}

void ListCtrl::OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked)
{
    //界面状态变化，同步到底层存储
    m_pData->SetColumnCheck(nColumnId, bChecked, true);
}

void ListCtrl::OnHeaderCheckStateChanged(bool bChecked)
{
    //界面状态变化，同步到底层存储
    m_pData->SetAllDataItemsCheck(bChecked);
}

void ListCtrl::OnHeaderColumnVisibleChanged(size_t nColumnId)
{
    Refresh();
    //列的显示/隐藏状态发生变化，通知应用层（可用于持久化列显示状态）
    bool bVisible = IsColumnVisibleById(nColumnId);
    SendEvent(kEventColumnVisibleChanged, (WPARAM)nColumnId, (LPARAM)(bVisible ? 1 : 0));
}

void ListCtrl::OnHeaderColumnSplitDoubleClick(ListCtrlHeaderItem* pHeaderItem)
{
    if (!IsEnableColumnWidthAuto()) {
        //功能关闭
        return;
    }
    //自动调整该列的宽度
    if (pHeaderItem != nullptr) {
        size_t nColumnId = pHeaderItem->GetColumnId();
        size_t nColumIndex = GetColumnIndex(nColumnId);
        if (nColumIndex < GetColumnCount()) {
            SetColumnWidthAuto(nColumIndex);
        }
    }
}

void ListCtrl::UpdateHeaderColumnCheckBox(size_t nColumnId)
{
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return;
    }
    std::vector<size_t> columnIdList;
    if (nColumnId == Box::InvalidIndex) {
        const size_t columnCount = GetColumnCount();
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsShowCheckBox())) {
                columnIdList.push_back(pHeaderItem->GetColumnId());
            }
        }
    }
    else {
        size_t columnIndex = GetColumnIndex(nColumnId);
        if (columnIndex != Box::InvalidIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsShowCheckBox())){
                columnIdList.push_back(nColumnId);
            }
        }
    }
    for (size_t columnId : columnIdList) {
        bool bChecked = false;
        bool bPartChecked = false;
        m_pData->GetColumnCheckStatus(columnId, bChecked, bPartChecked);
        ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumnById(columnId);
        if ((pHeaderItem != nullptr) && (pHeaderItem->IsShowCheckBox())) {
            pHeaderItem->SetCheckBoxCheck(bChecked, bPartChecked);
        }
    }
}

void ListCtrl::UpdateHeaderCheckBox()
{
    if (!IsDataItemShowCheckBox()) {
        //不显示CheckBox，忽略
        return;
    }

    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return;
    }
    bool bChecked = false;
    bool bPartChecked = false;
    m_pData->GetDataItemsCheckStatus(bChecked, bPartChecked);
    if ((m_pHeaderCtrl->IsChecked() != bChecked) || 
        (m_pHeaderCtrl->IsPartChecked() != bPartChecked)) {
        bool bOldValue = m_pHeaderCtrl->SetEnableCheckChangeEvent(false);
        m_pHeaderCtrl->SetChecked(bChecked, false);
        m_pHeaderCtrl->SetPartChecked(bPartChecked);
        m_pHeaderCtrl->SetEnableCheckChangeEvent(bOldValue);
    }
}

size_t ListCtrl::GetDataItemCount() const
{
    return m_pData->GetDataItemCount();
}

bool ListCtrl::SetDataItemCount(size_t itemCount)
{
    const size_t nOldDataItemCount = m_pData->GetDataItemCount();
    bool bRet = m_pData->SetDataItemCount(itemCount);
    if (bRet) {
        const size_t nNewDataItemCount = m_pData->GetDataItemCount();
        bool bChanged = (nOldDataItemCount != nNewDataItemCount);
        if (bChanged) {
            //列表数据个数发生变化
            UpdateHeaderColumnCheckBox(Box::InvalidIndex);
            UpdateHeaderCheckBox();
            SendEvent(kEventDataItemCountChanged, (WPARAM)nNewDataItemCount, (LPARAM)nOldDataItemCount);
        }
    }
    return bRet;
}

size_t ListCtrl::AddDataItem(const ListCtrlSubItemData& dataItem)
{
    size_t columnId = GetColumnId(0);
    ASSERT(columnId != Box::InvalidIndex);
    if (columnId == Box::InvalidIndex) {
        return Box::InvalidIndex;
    }
    size_t nItemIndex = m_pData->AddDataItem(columnId, dataItem);
    if (nItemIndex != Box::InvalidIndex) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }    
    return nItemIndex;
}

bool ListCtrl::InsertDataItem(size_t itemIndex, const ListCtrlSubItemData& dataItem)
{
    size_t columnId = GetColumnId(0);
    ASSERT(columnId != Box::InvalidIndex);
    if (columnId == Box::InvalidIndex) {
        return Box::InvalidIndex;
    }
    bool bRet = m_pData->InsertDataItem(itemIndex, columnId, dataItem);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::SetSubItemData(size_t itemIndex, size_t columnIndex, const ListCtrlSubItemData& subItemData)
{
    return SetSubItemDataById(itemIndex, GetColumnId(columnIndex), subItemData);
}

bool ListCtrl::SetSubItemDataById(size_t itemIndex, size_t columnId, const ListCtrlSubItemData& subItemData)
{
    bool bCheckChanged = false;
    bool bRet = m_pData->SetSubItemData(itemIndex, columnId, subItemData, bCheckChanged);
    if (bCheckChanged) {
        UpdateHeaderColumnCheckBox(GetColumnId(itemIndex));
    }
    return bRet;
}

bool ListCtrl::GetSubItemData(size_t itemIndex, size_t columnIndex, ListCtrlSubItemData& subItemData) const
{
    return GetSubItemDataById(itemIndex, GetColumnId(columnIndex), subItemData);
}

bool ListCtrl::GetSubItemDataById(size_t itemIndex, size_t columnId, ListCtrlSubItemData& subItemData) const
{
    return m_pData->GetSubItemData(itemIndex, columnId, subItemData);
}

bool ListCtrl::DeleteDataItem(size_t itemIndex)
{
    bool bRet = m_pData->DeleteDataItem(itemIndex);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::DeleteAllDataItems()
{
    bool bRet = m_pData->DeleteAllDataItems();
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

size_t ListCtrl::DeleteDataItems(const std::vector<size_t>& itemIndexs)
{
    size_t nDeleted = m_pData->DeleteDataItems(itemIndexs);
    if (nDeleted > 0) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return nDeleted;
}

void ListCtrl::CompactStorage()
{
    m_pData->CompactStorage();
}

bool ListCtrl::SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData)
{
    bool bChanged = false;
    bool bCheckChanged = false;
    bool bRet = m_pData->SetDataItemData(itemIndex, itemData, bChanged, bCheckChanged);
    if (bRet && bCheckChanged) {
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const
{
    return m_pData->GetDataItemData(itemIndex, itemData);
}

bool ListCtrl::SetDataItemUserData(size_t itemIndex, size_t userData)
{
    return m_pData->SetDataItemUserData(itemIndex, userData);
}

size_t ListCtrl::GetDataItemUserData(size_t itemIndex) const
{
    return m_pData->GetDataItemUserData(itemIndex);
}

bool ListCtrl::SetDataItemVisible(size_t itemIndex, bool bVisible)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemVisible(itemIndex, bVisible, bChanged);
    if (bChanged) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemVisible(size_t itemIndex) const
{
    return m_pData->IsDataItemVisible(itemIndex);
}

bool ListCtrl::SetDataItemSelected(size_t itemIndex, bool bSelected)
{
    bool bChanged = false;
    bool bOldChecked = m_pData->IsDataItemChecked(itemIndex);
    bool bRet = m_pData->SetDataItemSelected(itemIndex, bSelected, bChanged);
    if (bChanged) {
        if (m_pData->IsDataItemChecked(itemIndex) != bOldChecked) {
            UpdateHeaderCheckBox();
        }
    }
    return bRet;
}

bool ListCtrl::IsDataItemSelected(size_t itemIndex) const
{
    return m_pData->IsDataItemSelected(itemIndex);
}

bool ListCtrl::SetDataItemImageId(size_t itemIndex, int32_t imageId)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemImageId(itemIndex, imageId, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemImageId(size_t itemIndex) const
{
    return m_pData->GetDataItemImageId(itemIndex);
}

bool ListCtrl::SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemAlwaysAtTop(itemIndex, nAlwaysAtTop, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int8_t ListCtrl::GetDataItemAlwaysAtTop(size_t itemIndex) const
{
    return m_pData->GetDataItemAlwaysAtTop(itemIndex);
}

bool ListCtrl::SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale)
{
    bool bChanged = false;
    if (nItemHeight < 0) {
        nItemHeight = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nItemHeight);
    }
    bool bRet = m_pData->SetDataItemHeight(itemIndex, nItemHeight, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemHeight(size_t itemIndex) const
{
    return m_pData->GetDataItemHeight(itemIndex);
}

bool ListCtrl::SetSubItemText(size_t itemIndex, size_t columnIndex, const DString& text)
{
    return SetSubItemTextById(itemIndex, GetColumnId(columnIndex), text);
}

bool ListCtrl::SetSubItemTextById(size_t itemIndex, size_t columnId, const DString& text)
{
    return m_pData->SetSubItemText(itemIndex, columnId, text);
}

DString ListCtrl::GetSubItemText(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemTextById(itemIndex, GetColumnId(columnIndex));
}

DString ListCtrl::GetSubItemTextById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemText(itemIndex, columnId);
}

bool ListCtrl::SetSubItemSortGroup(size_t itemIndex, size_t columnIndex, int32_t nSortGroup)
{
    return SetSubItemSortGroupById(itemIndex, GetColumnId(columnIndex), nSortGroup);
}

bool ListCtrl::SetSubItemSortGroupById(size_t itemIndex, size_t columnId, int32_t nSortGroup)
{
    return m_pData->SetSubItemSortGroup(itemIndex, columnId, nSortGroup);
}

int32_t ListCtrl::GetSubItemGroup(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemGroupById(itemIndex, GetColumnId(columnIndex));
}

int32_t ListCtrl::GetSubItemGroupById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemSortGroup(itemIndex, columnId);
}

bool ListCtrl::SetSubItemUserDataN(size_t itemIndex, size_t columnIndex, uint64_t userDataN)
{
    return SetSubItemUserDataNById(itemIndex, GetColumnId(columnIndex), userDataN);
}

bool ListCtrl::SetSubItemUserDataNById(size_t itemIndex, size_t columnId, uint64_t userDataN)
{
    return m_pData->SetSubItemUserDataN(itemIndex, columnId, userDataN);
}

uint64_t ListCtrl::GetSubItemUserDataN(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemUserDataNById(itemIndex, GetColumnId(columnIndex));
}

uint64_t ListCtrl::GetSubItemUserDataNById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemUserDataN(itemIndex, columnId);
}

bool ListCtrl::SetSubItemUserDataS(size_t itemIndex, size_t columnIndex, const DString& userDataS)
{
    return SetSubItemUserDataSById(itemIndex, GetColumnId(columnIndex), userDataS);
}

bool ListCtrl::SetSubItemUserDataSById(size_t itemIndex, size_t columnId, const DString& userDataS)
{
    return m_pData->SetSubItemUserDataS(itemIndex, columnId, userDataS);
}

DString ListCtrl::GetSubItemUserDataS(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemUserDataSById(itemIndex, GetColumnId(columnIndex));
}

DString ListCtrl::GetSubItemUserDataSById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemUserDataS(itemIndex, columnId);
}

bool ListCtrl::SetSubItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor)
{
    return SetSubItemTextColorById(itemIndex, GetColumnId(columnIndex), textColor);
}

bool ListCtrl::SetSubItemTextColorById(size_t itemIndex, size_t columnId, const UiColor& textColor)
{
    return m_pData->SetSubItemTextColor(itemIndex, columnId, textColor);
}

UiColor ListCtrl::GetSubItemTextColor(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemTextColorById(itemIndex, GetColumnId(columnIndex));
}

UiColor ListCtrl::GetSubItemTextColorById(size_t itemIndex, size_t columnId) const
{
    UiColor textColor;
    m_pData->GetSubItemTextColor(itemIndex, columnId, textColor);
    return textColor;
}

bool ListCtrl::SetSubItemTextFormat(size_t itemIndex, size_t columnIndex, int32_t nTextFormat)
{
    return SetSubItemTextFormatById(itemIndex, GetColumnId(columnIndex), nTextFormat);
}

bool ListCtrl::SetSubItemTextFormatById(size_t itemIndex, size_t columnId, int32_t nTextFormat)
{
    return m_pData->SetSubItemTextFormat(itemIndex, columnId, nTextFormat);
}

int32_t ListCtrl::GetSubItemTextFormat(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemTextFormatById(itemIndex, GetColumnId(columnIndex));
}

int32_t ListCtrl::GetSubItemTextFormatById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemTextFormat(itemIndex, columnId);
}

bool ListCtrl::SetSubItemSingleLine(size_t itemIndex, size_t columnIndex, bool bSingleLine)
{
    return SetSubItemSingleLineById(itemIndex, GetColumnId(columnIndex), bSingleLine);
}

bool ListCtrl::SetSubItemSingleLineById(size_t itemIndex, size_t columnId, bool bSingleLine)
{
    //仅Report视图支持（Icon/List视图没有列的概念，其单行/多行由视图自身的接口控制）
    if (m_listCtrlType != ListCtrlType::Report) {
        return false;
    }
    return m_pData->SetSubItemSingleLine(itemIndex, columnId, bSingleLine);
}

bool ListCtrl::IsSubItemSingleLine(size_t itemIndex, size_t columnIndex) const
{
    return IsSubItemSingleLineById(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::IsSubItemSingleLineById(size_t itemIndex, size_t columnId) const
{
    if (m_listCtrlType != ListCtrlType::Report) {
        return true;
    }
    return m_pData->IsSubItemSingleLine(itemIndex, columnId);
}

bool ListCtrl::SetSubItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor)
{
    return SetSubItemBkColorById(itemIndex, GetColumnId(columnIndex), bkColor);
}

bool ListCtrl::SetSubItemBkColorById(size_t itemIndex, size_t columnId, const UiColor& bkColor)
{
    return m_pData->SetSubItemBkColor(itemIndex, columnId, bkColor);
}

UiColor ListCtrl::GetSubItemBkColor(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemBkColorById(itemIndex, GetColumnId(columnIndex));
}

UiColor ListCtrl::GetSubItemBkColorById(size_t itemIndex, size_t columnId) const
{
    UiColor bkColor;
    m_pData->GetSubItemBkColor(itemIndex, columnId, bkColor);
    return bkColor;
}

bool ListCtrl::IsSubItemShowCheckBox(size_t itemIndex, size_t columnIndex) const
{
    return IsSubItemShowCheckBoxById(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::IsSubItemShowCheckBoxById(size_t itemIndex, size_t columnId) const
{
    return m_pData->IsSubItemShowCheckBox(itemIndex, columnId);
}

bool ListCtrl::SetSubItemShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox)
{
    return SetSubItemShowCheckBoxById(itemIndex, GetColumnId(columnIndex), bShowCheckBox);
}

bool ListCtrl::SetSubItemShowCheckBoxById(size_t itemIndex, size_t columnId, bool bShowCheckBox)
{
    return m_pData->SetSubItemShowCheckBox(itemIndex, columnId, bShowCheckBox);
}

bool ListCtrl::SetSubItemCheck(size_t itemIndex, size_t columnIndex, bool bChecked)
{
    return SetSubItemCheckById(itemIndex, GetColumnId(columnIndex), bChecked);
}

bool ListCtrl::SetSubItemCheckById(size_t itemIndex, size_t columnId, bool bChecked)
{
    return m_pData->SetSubItemCheck(itemIndex, columnId, bChecked, true);
}

bool ListCtrl::IsSubItemChecked(size_t itemIndex, size_t columnIndex) const
{
    return IsSubItemCheckedById(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::IsSubItemCheckedById(size_t itemIndex, size_t columnId) const
{
    bool bChecked = false;
    m_pData->GetSubItemCheck(itemIndex, columnId, bChecked);
    return bChecked;
}

bool ListCtrl::SetSubItemImageId(size_t itemIndex, size_t columnIndex, int32_t imageId)
{
    return SetSubItemImageIdById(itemIndex, GetColumnId(columnIndex), imageId);
}

bool ListCtrl::SetSubItemImageIdById(size_t itemIndex, size_t columnId, int32_t imageId)
{
    return m_pData->SetSubItemImageId(itemIndex, columnId, imageId);
}

int32_t ListCtrl::GetSubItemImageId(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemImageIdById(itemIndex, GetColumnId(columnIndex));
}

int32_t ListCtrl::GetSubItemImageIdById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemImageId(itemIndex, columnId);
}

bool ListCtrl::SetSubItemEditable(size_t itemIndex, size_t columnIndex, bool bEditable)
{
    return SetSubItemEditableById(itemIndex, GetColumnId(columnIndex), bEditable);
}

bool ListCtrl::SetSubItemEditableById(size_t itemIndex, size_t columnId, bool bEditable)
{
    return m_pData->SetSubItemEditable(itemIndex, columnId, bEditable);
}

bool ListCtrl::IsSubItemEditable(size_t itemIndex, size_t columnIndex) const
{
    return IsSubItemEditableById(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::IsSubItemEditableById(size_t itemIndex, size_t columnId) const
{
    return m_pData->IsSubItemEditable(itemIndex, columnId);
}

bool ListCtrl::SetSubItemProgress(size_t itemIndex, size_t columnIndex, int32_t nProgress)
{
    return SetSubItemProgressById(itemIndex, GetColumnId(columnIndex), nProgress);
}

bool ListCtrl::SetSubItemProgressById(size_t itemIndex, size_t columnId, int32_t nProgress)
{
    return m_pData->SetSubItemProgress(itemIndex, columnId, nProgress);
}

int32_t ListCtrl::GetSubItemProgress(size_t itemIndex, size_t columnIndex) const
{
    return GetSubItemProgressById(itemIndex, GetColumnId(columnIndex));
}

int32_t ListCtrl::GetSubItemProgressById(size_t itemIndex, size_t columnId) const
{
    return m_pData->GetSubItemProgress(itemIndex, columnId);
}

bool ListCtrl::SetSubItemRichText(size_t itemIndex, size_t columnIndex, bool bRichText)
{
    return SetSubItemRichTextById(itemIndex, GetColumnId(columnIndex), bRichText);
}

bool ListCtrl::SetSubItemRichTextById(size_t itemIndex, size_t columnId, bool bRichText)
{
    return m_pData->SetSubItemRichText(itemIndex, columnId, bRichText);
}

bool ListCtrl::IsSubItemRichText(size_t itemIndex, size_t columnIndex) const
{
    return IsSubItemRichTextById(itemIndex, GetColumnId(columnIndex));
}

bool ListCtrl::IsSubItemRichTextById(size_t itemIndex, size_t columnId) const
{
    return m_pData->IsSubItemRichText(itemIndex, columnId);
}

bool ListCtrl::IsMultiSelect() const
{
    return m_bMultiSelect;
}

void ListCtrl::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
    if (m_pReportView != nullptr) {
        m_pReportView->SetMultiSelect(bMultiSelect);
    }
    if (m_pIconView != nullptr) {
        m_pIconView->SetMultiSelect(bMultiSelect);
    }
    if (m_pListView != nullptr) {
        m_pListView->SetMultiSelect(bMultiSelect);
    }
    UpdateHeaderCheckBox();
    if (!bMultiSelect) {
        //切换为单选模式：单元格选择模式下把已选择的多个单元格裁剪为 1（保留第一个选中的单元格），
        //与行选择模式（ListBox::OnSwitchToSingleSelect）的行为保持一致
        std::vector<ListCtrlCellId> cellList;
        if (GetSelectedCells(cellList) > 1) {
            std::vector<ListCtrlCellId> keepCellList;
            keepCellList.push_back(cellList.front());
            SetSelectedCells(keepCellList, true);
        }
    }
}

void ListCtrl::SetSelectedDataItems(const std::vector<size_t>& selectedIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    m_pData->SetSelectedElements(selectedIndexs, bClearOthers, refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        UpdateHeaderCheckBox();
    }
}

void ListCtrl::GetSelectedDataItems(std::vector<size_t>& itemIndexs) const
{
    m_pData->GetSelectedElements(itemIndexs);
}

void ListCtrl::SetCheckedDataItems(const std::vector<size_t>& itemIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    m_pData->SetCheckedDataItems(itemIndexs, bClearOthers, refreshIndexs);    
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        UpdateHeaderCheckBox();
    }
}

void ListCtrl::GetCheckedDataItems(std::vector<size_t>& itemIndexs) const
{
    m_pData->GetCheckedDataItems(itemIndexs);
}

void ListCtrl::SetSelectAll()
{
    if (!IsMultiSelect()) {
        //单选模式：至多一个选中单元（行或单元格），"全选"无意义，不做任何改变
        //（同时避免行模式下继续走到 ListCtrlData::SelectAll 内的 IsMultiSelect 断言）
        return;
    }
    if (m_selectionMode == ListCtrlSelectionMode::kCell) {
        //单元格选择模式：全选所有单元格
        std::vector<size_t> columnIdList;
        const size_t nColumnCount = GetColumnCount();
        for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
            const size_t nColumnId = GetColumnId(nColumnIndex);
            if (IsValidColumnId(nColumnId)) {
                columnIdList.push_back(nColumnId);
            }
        }
        m_pData->SelectAllCells(columnIdList);
        //若当前格无效，则把当前格/锚点定位到左上角单元格
        if (!m_pData->GetCurrentCell().IsValid() &&
            (GetDataItemCount() > 0) && !columnIdList.empty()) {
            ListCtrlCellId cell;
            cell.nRowIndex = 0;
            cell.nColumnId = columnIdList[0];
            m_pData->SetAnchorCell(cell);
            m_pData->SetCurrentCell(cell);
        }
        NotifyCellSelectionChanged();
        RefreshCellSelectionView();
        return;
    }
    std::vector<size_t> refreshIndexs;
    m_pData->SelectAll(refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }
    }    
}

void ListCtrl::SetSelectNone()
{
    if (m_selectionMode == ListCtrlSelectionMode::kCell) {
        //单元格选择模式：清空所有单元格选择
        m_pData->ClearCellSelection();
        NotifyCellSelectionChanged();
        RefreshCellSelectionView();
        return;
    }
    std::vector<size_t> refreshIndexs;
    m_pData->SelectNone(refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshDataItems(refreshIndexs);
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }
    }
}

ListCtrlSelectionMode ListCtrl::GetSelectionMode() const
{
    return m_selectionMode;
}

void ListCtrl::SetSelectionMode(ListCtrlSelectionMode mode)
{
    if (m_selectionMode == mode) {
        return;
    }
    if (mode == ListCtrlSelectionMode::kCell) {
        //切换到单元格选择模式：先清除当前的行选择
        if (m_pData != nullptr) {
            std::vector<size_t> refreshIndexs;
            m_pData->SelectNone(refreshIndexs);
            if (!refreshIndexs.empty()) {
                RefreshDataItems(refreshIndexs);
                if (IsAutoCheckSelect()) {
                    UpdateHeaderCheckBox();
                }
            }
            //单元格选择状态也一并重置
            m_pData->ClearCellSelection();
        }
    }
    else {
        //切换回行选择模式：清除单元格选择状态
        if (m_pData != nullptr) {
            m_pData->ClearCellSelection();
        }
    }
    m_selectionMode = mode;
    m_bCellDragSelecting = false;
    m_bCellDragAdditive = false;
    m_hoverCell = HoverCellId();
    NotifyCellSelectionChanged();
    RefreshCellSelectionView();
}

ListCtrlCellId ListCtrl::GetCurrentCell() const
{
    if (m_pData != nullptr) {
        return m_pData->GetCurrentCell();
    }
    return ListCtrlCellId();
}

bool ListCtrl::SetCurrentCell(size_t nRowIndex, size_t nColumnId)
{
    if (m_selectionMode != ListCtrlSelectionMode::kCell) {
        return false;
    }
    if ((nRowIndex >= GetDataItemCount()) || !IsValidColumnId(nColumnId)) {
        return false;
    }
    ListCtrlCellId cell;
    cell.nRowIndex = nRowIndex;
    cell.nColumnId = nColumnId;

    ListCtrlCellId oldCell = GetCurrentCell();
    //替换语义：清除其它已选中的单元格，仅选中该单元格，并同步锚点/当前格
    std::vector<size_t> columnIdList;
    columnIdList.push_back(nColumnId);
    m_pData->SetCellSelectionRect(nRowIndex, nRowIndex, columnIdList, false);
    m_pData->SetAnchorCell(cell);
    m_pData->SetCurrentCell(cell);
    if (oldCell != cell) {
        NotifyCurrentCellChanged(cell);
    }
    NotifyCellSelectionChanged();
    RefreshCellSelectionView();
    return true;
}

bool ListCtrl::SetCellSelection(const ListCtrlCellId& anchorCell, const ListCtrlCellId& focusCell, bool bAdditive)
{
    if (m_selectionMode != ListCtrlSelectionMode::kCell) {
        return false;
    }
    if ((anchorCell.nRowIndex >= GetDataItemCount()) || !IsValidColumnId(anchorCell.nColumnId)) {
        return false;
    }
    //焦点格无效时仅更新锚点，不改变选中集合
    if (!focusCell.IsValid()) {
        m_pData->SetAnchorCell(anchorCell);
        return true;
    }
    if ((focusCell.nRowIndex >= GetDataItemCount()) || !IsValidColumnId(focusCell.nColumnId)) {
        return false;
    }
    ListCtrlCellId oldCell = GetCurrentCell();

    //按当前列序把"锚点列 → 焦点列"展开为连续的列ID列表（列序可变，故以列索引区间为准）
    size_t nIndexBegin = GetColumnIndex(anchorCell.nColumnId);
    size_t nIndexEnd = GetColumnIndex(focusCell.nColumnId);
    if (!Box::IsValidItemIndex(nIndexBegin) || !Box::IsValidItemIndex(nIndexEnd)) {
        return false;
    }
    if (nIndexBegin > nIndexEnd) {
        const size_t nTemp = nIndexBegin;
        nIndexBegin = nIndexEnd;
        nIndexEnd = nTemp;
    }
    std::vector<size_t> columnIdList;
    for (size_t nColumnIndex = nIndexBegin; nColumnIndex <= nIndexEnd; ++nColumnIndex) {
        const size_t nColumnId = GetColumnId(nColumnIndex);
        if (IsValidColumnId(nColumnId)) {
            columnIdList.push_back(nColumnId);
        }
    }
    if (!IsMultiSelect()) {
        //单选模式：忽略追加语义，仅选中焦点格（至多一个单元格）
        columnIdList.clear();
        columnIdList.push_back(focusCell.nColumnId);
        m_pData->SetCellSelectionRect(focusCell.nRowIndex, focusCell.nRowIndex, columnIdList, false);
    }
    else {
        m_pData->SetCellSelectionRect(anchorCell.nRowIndex, focusCell.nRowIndex, columnIdList, bAdditive);
    }
    m_pData->SetAnchorCell(anchorCell);
    m_pData->SetCurrentCell(focusCell);
    if (oldCell != focusCell) {
        NotifyCurrentCellChanged(focusCell);
    }
    NotifyCellSelectionChanged();
    RefreshCellSelectionView();
    return true;
}

void ListCtrl::ClearCellSelection()
{
    if (m_pData != nullptr) {
        m_pData->ClearCellSelection();
    }
    NotifyCellSelectionChanged();
    RefreshCellSelectionView();
}

void ListCtrl::GetCellSelection(ListCtrlCellId& anchorCell, ListCtrlCellId& focusCell) const
{
    if (m_pData != nullptr) {
        m_pData->GetCellSelection(anchorCell, focusCell);
    }
    else {
        anchorCell = ListCtrlCellId();
        focusCell = ListCtrlCellId();
    }
}

void ListCtrl::SetSelectedCells(const std::vector<ListCtrlCellId>& cellList, bool bClearOthers)
{
    if (m_selectionMode != ListCtrlSelectionMode::kCell) {
        return;
    }
    if (bClearOthers || !IsMultiSelect()) {
        //单选模式下，即使调用方要求"不清除其它"也必须清空：单选语义下至多一个单元格
        m_pData->ClearCellSelection();
    }
    bool bFirst = true;
    for (const ListCtrlCellId& cell : cellList) {
        if ((cell.nRowIndex >= GetDataItemCount()) || !IsValidColumnId(cell.nColumnId)) {
            continue;
        }
        if (!m_pData->IsSelectableElement(cell.nRowIndex)) {
            //置顶行、不可见行不参与单元格选择
            continue;
        }
        m_pData->SetCellSelected(cell.nRowIndex, cell.nColumnId, true);
        if (bFirst) {
            m_pData->SetAnchorCell(cell);
            m_pData->SetCurrentCell(cell);
            bFirst = false;
        }
        if (!IsMultiSelect()) {
            //单选模式：至多保留一个单元格，取列表中第一个有效单元格后即结束
            break;
        }
    }
    NotifyCellSelectionChanged();
    RefreshCellSelectionView();
}

size_t ListCtrl::GetSelectedCells(std::vector<ListCtrlCellId>& cellList) const
{
    cellList.clear();
    if ((m_selectionMode != ListCtrlSelectionMode::kCell) || (m_pData == nullptr)) {
        return 0;
    }
    const size_t nCount = GetDataItemCount();
    const size_t nColumnCount = GetColumnCount();
    for (size_t nRowIndex = 0; nRowIndex < nCount; ++nRowIndex) {
        if (!m_pData->IsSelectableElement(nRowIndex)) {
            //置顶行、不可见行不参与单元格选择
            continue;
        }
        for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
            const size_t nColumnId = GetColumnId(nColumnIndex);
            if (!IsValidColumnId(nColumnId)) {
                continue;
            }
            if (!m_pData->IsCellSelected(nRowIndex, nColumnId)) {
                continue;
            }
            ListCtrlCellId cell;
            cell.nRowIndex = nRowIndex;
            cell.nColumnId = nColumnId;
            cellList.push_back(cell);
        }
    }
    return cellList.size();
}

bool ListCtrl::IsCellSelected(size_t nRowIndex, size_t nColumnId) const
{
    if ((m_selectionMode != ListCtrlSelectionMode::kCell) || (m_pData == nullptr)) {
        return false;
    }
    return m_pData->IsCellSelected(nRowIndex, nColumnId);
}

bool ListCtrl::IsCellCurrent(size_t nRowIndex, size_t nColumnId) const
{
    if (m_selectionMode != ListCtrlSelectionMode::kCell) {
        return false;
    }
    const ListCtrlCellId cell = GetCurrentCell();
    return cell.IsValid() && (cell.nRowIndex == nRowIndex) && (cell.nColumnId == nColumnId);
}

void ListCtrl::GetSelectedCellRanges(std::vector<ListCtrlCellRange>& rangeList) const
{
    rangeList.clear();
    if ((m_selectionMode != ListCtrlSelectionMode::kCell) || (m_pData == nullptr)) {
        return;
    }
    const size_t nRowCount = GetDataItemCount();
    const size_t nColumnCount = GetColumnCount();
    if ((nRowCount == 0) || (nColumnCount == 0)) {
        return;
    }

    //正在纵向延伸的区域（按列索引区间归并）
    struct PendingRange
    {
        size_t nColumnIndexBegin;
        size_t nColumnIndexEnd;
        size_t nRowBegin;
        size_t nRowEnd;
    };
    std::vector<PendingRange> pending;   //上一行仍在延伸的区域
    std::vector<PendingRange> nextPending;
    std::vector<std::pair<size_t, size_t>> rowRuns;  //本行的连续选中列区段（列索引区间）
    std::vector<bool> runUsed;

    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        //1. 求本行的连续选中列区段（置顶行、不可见行不参与单元格选择，按空区段处理使其上方区域在此结算）
        rowRuns.clear();
        if (m_pData->IsSelectableElement(nRowIndex)) {
            bool bInRun = false;
            size_t nRunBegin = 0;
            for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
                const size_t nColumnId = GetColumnId(nColumnIndex);
                const bool bSelected = IsValidColumnId(nColumnId) &&
                                       m_pData->IsCellSelected(nRowIndex, nColumnId);
                if (bSelected && !bInRun) {
                    bInRun = true;
                    nRunBegin = nColumnIndex;
                }
                else if (!bSelected && bInRun) {
                    bInRun = false;
                    rowRuns.emplace_back(nRunBegin, nColumnIndex - 1);
                }
            }
            if (bInRun) {
                rowRuns.emplace_back(nRunBegin, nColumnCount - 1);
            }
        }

        //2. 与上一行的区域纵向合并（列索引区间完全一致才能延伸）
        nextPending.clear();
        runUsed.assign(rowRuns.size(), false);
        for (const PendingRange& p : pending) {
            bool bMatched = false;
            for (size_t r = 0; r < rowRuns.size(); ++r) {
                if (!runUsed[r] && (rowRuns[r].first == p.nColumnIndexBegin) &&
                    (rowRuns[r].second == p.nColumnIndexEnd)) {
                    runUsed[r] = true;
                    PendingRange np = p;
                    np.nRowEnd = nRowIndex;
                    nextPending.push_back(np);
                    bMatched = true;
                    break;
                }
            }
            if (!bMatched) {
                //该区域不再延伸：结算并输出为对外区域
                ListCtrlCellRange range;
                range.nRowBegin = p.nRowBegin;
                range.nRowEnd = p.nRowEnd;
                range.nColumnIdBegin = GetColumnId(p.nColumnIndexBegin);
                range.nColumnIdEnd = GetColumnId(p.nColumnIndexEnd);
                if (range.IsValid()) {
                    rangeList.push_back(range);
                }
            }
        }
        //3. 本行新出现的区段，作为新区域的起点
        for (size_t r = 0; r < rowRuns.size(); ++r) {
            if (runUsed[r]) {
                continue;
            }
            PendingRange np;
            np.nColumnIndexBegin = rowRuns[r].first;
            np.nColumnIndexEnd = rowRuns[r].second;
            np.nRowBegin = nRowIndex;
            np.nRowEnd = nRowIndex;
            nextPending.push_back(np);
        }
        pending.swap(nextPending);
    }

    //收尾：仍在延伸的区域输出
    for (const PendingRange& p : pending) {
        ListCtrlCellRange range;
        range.nRowBegin = p.nRowBegin;
        range.nRowEnd = p.nRowEnd;
        range.nColumnIdBegin = GetColumnId(p.nColumnIndexBegin);
        range.nColumnIdEnd = GetColumnId(p.nColumnIndexEnd);
        if (range.IsValid()) {
            rangeList.push_back(range);
        }
    }
}

void ListCtrl::NotifyCurrentCellChanged(const ListCtrlCellId& cell)
{
    SendEvent(kEventCurrentCellChanged, (WPARAM)cell.nRowIndex, (LPARAM)cell.nColumnId);
}

void ListCtrl::NotifyCellSelectionChanged()
{
    //选择区域列表：按列索引连续 + 纵向相邻合并后，可能得到多项不连续区域
    std::vector<ListCtrlCellRange> rangeList;
    GetSelectedCellRanges(rangeList);
    if (rangeList.empty()) {
        SendEvent(kEventCellSelectionChanged, (WPARAM)0, (LPARAM)0, nullptr);
    }
    else {
        SendEvent(kEventCellSelectionChanged, (WPARAM)rangeList.size(), (LPARAM)0, &rangeList);
    }
}

void ListCtrl::RefreshCellSelectionView()
{
    if (m_pReportView != nullptr) {
        m_pReportView->Invalidate();
    }
}

void ListCtrl::UpdateHoverCell(size_t nRowIndex, size_t nColumnId)
{
    if ((nRowIndex == Box::InvalidIndex) || !IsValidColumnId(nColumnId) || (m_pData == nullptr)) {
        //未命中单元格：清除悬停状态
        ClearHoverCell();
        return;
    }
    //把"显示索引"换算为"行身份"存储，与显示顺序解耦（排序/增删行后悬停高亮仍跟随原数据行）
    HoverCellId hoverCell;
    hoverCell.nRowId = m_pData->GetRowIdByDisplayIndex(nRowIndex);
    hoverCell.nColumnId = nColumnId;
    if (!hoverCell.IsValid()) {
        ClearHoverCell();
        return;
    }
    if (m_hoverCell != hoverCell) {
        m_hoverCell = hoverCell;
        RefreshCellSelectionView();
    }
}

void ListCtrl::ClearHoverCell()
{
    if (m_hoverCell.IsValid()) {
        m_hoverCell = HoverCellId();
        RefreshCellSelectionView();
    }
}

bool ListCtrl::IsCellHovered(size_t nRowIndex, size_t nColumnId) const
{
    if ((m_selectionMode != ListCtrlSelectionMode::kCell) || !m_hoverCell.IsValid() || (m_pData == nullptr)) {
        return false;
    }
    //按行身份比对：与显示顺序解耦
    return (m_pData->GetRowIdByDisplayIndex(nRowIndex) == m_hoverCell.nRowId) && (m_hoverCell.nColumnId == nColumnId);
}

bool ListCtrl::OnCellMouseEvent(const EventArgs& msg, size_t nRowIndex, size_t nColumnId)
{
    if (m_selectionMode != ListCtrlSelectionMode::kCell) {
        return false;
    }
    const bool bHitCell = (nRowIndex != Box::InvalidIndex) && IsValidColumnId(nColumnId);
    switch (msg.eventType) {
    case kEventMouseButtonDown:
    case kEventMouseRButtonDown:
        //左键与右键按下：语义相同（Shift 扩展 / Ctrl 切换 / 普通单击），并进入拖动框选状态
        BeginCellDrag(msg, nRowIndex, nColumnId);
        break;
    case kEventMouseHover:
        //更新鼠标悬停的单元格（用于绘制单元格悬停高亮），同时抑制行级悬停背景
        UpdateHoverCell(nRowIndex, nColumnId);
        break;
    case kEventMouseMove:
        //更新鼠标悬停的单元格
        UpdateHoverCell(nRowIndex, nColumnId);
        if (m_bCellDragSelecting && bHitCell) {
            //拖动过程中的"选择"由框选矩形驱动（ListCtrlReportView::OnFrameSelectionForCells）；
            //这里仅让"当前格焦点框"跟随鼠标命中的单元格（不清除已选择的单元格）
            ListCtrlCellId cell;
            cell.nRowIndex = nRowIndex;
            cell.nColumnId = nColumnId;
            const ListCtrlCellId oldCell = GetCurrentCell();
            if (oldCell != cell) {
                if (m_pData != nullptr) {
                    m_pData->SetCurrentCell(cell);
                }
                NotifyCurrentCellChanged(cell);
                RefreshCellSelectionView();
            }
        }
        break;
    case kEventMouseButtonUp:
    case kEventMouseRButtonUp:
        m_bCellDragSelecting = false;
        m_bCellDragAdditive = false;
        break;
    default:
        return false;
    }
    return true;
}

void ListCtrl::BeginCellDrag(const EventArgs& msg, size_t nRowIndex, size_t nColumnId)
{
    if (m_pData == nullptr) {
        return;
    }
    m_bCellDragSelecting = false;
    m_bCellDragAdditive = false;
    if ((nRowIndex == Box::InvalidIndex) || !IsValidColumnId(nColumnId)) {
        //未命中单元格（空白区域）：清空单元格选择
        ClearCellSelection();
        return;
    }
    ListCtrlCellId cell;
    cell.nRowIndex = nRowIndex;
    cell.nColumnId = nColumnId;

    const bool bShiftDown = IsKeyDown(msg, ModifierKey::kShift);
    const bool bCtrlDown = IsKeyDown(msg, ModifierKey::kControl);

    if ((msg.eventType == kEventMouseRButtonDown) && m_pData->IsCellSelected(nRowIndex, nColumnId)) {
        //右键点击"已选中"的单元格：保持现有选择状态不变（供弹出右键菜单，对已选中的单元格执行操作），
        //仅把锚点/当前格同步到该单元格（不改变选择集合，也不触发选择变化事件）。
        //语义与行选择模式保持一致：参见 ListBox::ListCtrlSelectItem 对右键命中选择项的处理
        m_pData->SetAnchorCell(cell);
        const ListCtrlCellId oldCell = GetCurrentCell();
        m_pData->SetCurrentCell(cell);
        if (oldCell != cell) {
            NotifyCurrentCellChanged(cell);
        }
        RefreshCellSelectionView();
        //注意：此处直接返回，不进入拖动框选状态；否则继续拖动时会被框选逻辑按"替换"语义清除已有选择
        return;
    }
    if (!IsMultiSelect()) {
        //单选模式：忽略 Shift/Ctrl 的多选语义，一律按"普通单击"处理（至多选中一个单元格）；
        //同时不进入拖动框选状态，避免拖动时"当前格焦点框"跟随鼠标而选中集合不变造成视觉错位
        SetCurrentCell(nRowIndex, nColumnId);
        m_bCellDragAdditive = false;
        return;
    }
    if (bShiftDown) {
        //Shift（含Ctrl+Shift）：以锚点为起点扩展到点击的单元格；Ctrl+Shift 为追加，否则替换
        ListCtrlCellId anchorCell;
        ListCtrlCellId focusCell;
        GetCellSelection(anchorCell, focusCell);
        if (!anchorCell.IsValid()) {
            anchorCell = cell;
        }
        SetCellSelection(anchorCell, cell, bCtrlDown);
        m_bCellDragAdditive = bCtrlDown;
    }
    else if (bCtrlDown) {
        //Ctrl+单击：切换该单元格的选中状态（再次点击取消），不清除其它已选择的单元格
        const bool bSelected = m_pData->IsCellSelected(nRowIndex, nColumnId);
        m_pData->SetCellSelected(nRowIndex, nColumnId, !bSelected);
        m_pData->SetAnchorCell(cell);
        const ListCtrlCellId oldCell = GetCurrentCell();
        m_pData->SetCurrentCell(cell);
        if (oldCell != cell) {
            NotifyCurrentCellChanged(cell);
        }
        NotifyCellSelectionChanged();
        RefreshCellSelectionView();
        //Ctrl+拖动：在其后进入追加模式
        m_bCellDragAdditive = true;
    }
    else {
        //普通单击：选中该单元格（清除其它），并同步锚点/当前格
        SetCurrentCell(nRowIndex, nColumnId);
        m_bCellDragAdditive = false;
    }
    //鼠标按下后，进入拖动框选状态
    m_bCellDragSelecting = true;
}

bool ListCtrl::OnCellKeyDown(const EventArgs& msg)
{
    if ((m_selectionMode != ListCtrlSelectionMode::kCell) || (msg.eventType != kEventKeyDown)) {
        return false;
    }
    const bool bCtrlDown = Keyboard::IsKeyDown(kVK_CONTROL);
    const bool bShiftDown = Keyboard::IsKeyDown(kVK_SHIFT);

    //Ctrl + A：全选单元格（仅多选模式有效；单选模式至多一个单元格，忽略该按键）
    if (bCtrlDown && ((msg.vkCode == _T('A')) || (msg.vkCode == _T('a')))) {
        if (IsMultiSelect()) {
            SetSelectAll();
        }
        return true;
    }
    //Ctrl + Space：切换当前单元格的选中状态
    if (bCtrlDown && (msg.vkCode == kVK_SPACE)) {
        const ListCtrlCellId cell = GetCurrentCell();
        if (cell.IsValid()) {
            const bool bSelected = m_pData->IsCellSelected(cell.nRowIndex, cell.nColumnId);
            m_pData->SetCellSelected(cell.nRowIndex, cell.nColumnId, !bSelected);
            NotifyCellSelectionChanged();
            RefreshCellSelectionView();
        }
        return true;
    }

    //方向键：移动当前单元格
    const bool bArrowKey = (msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN) ||
                            (msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT) ||
                            (msg.vkCode == kVK_PRIOR) || (msg.vkCode == kVK_NEXT) ||
                            (msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END);
    if (!bArrowKey) {
        return false;
    }
    const size_t nRowCount = GetDataItemCount();
    const size_t nColumnCount = GetColumnCount();
    if ((nRowCount == 0) || (nColumnCount == 0)) {
        return true;
    }

    ListCtrlCellId cell = GetCurrentCell();
    if (!cell.IsValid()) {
        //无当前格：定位到首个单元格
        SetCurrentCell(0, GetColumnId(0));
        return true;
    }

    const size_t nOldColumnIndex = GetColumnIndex(cell.nColumnId);
    size_t nNewRowIndex = cell.nRowIndex;
    size_t nNewColumnIndex = Box::IsValidItemIndex(nOldColumnIndex) ? nOldColumnIndex : 0;

    if (msg.vkCode == kVK_UP) {
        if (nNewRowIndex > 0) {
            --nNewRowIndex;
        }
    }
    else if (msg.vkCode == kVK_DOWN) {
        if (nNewRowIndex + 1 < nRowCount) {
            ++nNewRowIndex;
        }
    }
    else if (msg.vkCode == kVK_LEFT) {
        if (nNewColumnIndex > 0) {
            --nNewColumnIndex;
        }
    }
    else if (msg.vkCode == kVK_RIGHT) {
        if (nNewColumnIndex + 1 < nColumnCount) {
            ++nNewColumnIndex;
        }
    }
    else if (msg.vkCode == kVK_PRIOR) {
        nNewRowIndex = (nNewRowIndex > 5) ? (nNewRowIndex - 5) : 0;
    }
    else if (msg.vkCode == kVK_NEXT) {
        nNewRowIndex = nNewRowIndex + 5;
        if (nNewRowIndex >= nRowCount) {
            nNewRowIndex = nRowCount - 1;
        }
    }
    else if (msg.vkCode == kVK_HOME) {
        //Home：当前行的首列；Ctrl+Home：表格首个单元格
        if (bCtrlDown) {
            nNewRowIndex = 0;
        }
        nNewColumnIndex = 0;
    }
    else if (msg.vkCode == kVK_END) {
        //End：当前行的末列；Ctrl+End：表格最后一个单元格
        if (bCtrlDown) {
            nNewRowIndex = nRowCount - 1;
        }
        nNewColumnIndex = nColumnCount - 1;
    }

    const size_t nNewColumnId = GetColumnId(nNewColumnIndex);
    if (!IsValidColumnId(nNewColumnId)) {
        return true;
    }

    ListCtrlCellId newFocusCell;
    newFocusCell.nRowIndex = nNewRowIndex;
    newFocusCell.nColumnId = nNewColumnId;

    if (bCtrlDown && !bShiftDown) {
        //Ctrl+方向：只移动当前格焦点，不改变选择集合
        const ListCtrlCellId oldCell = GetCurrentCell();
        m_pData->SetCurrentCell(newFocusCell);
        if (oldCell != newFocusCell) {
            NotifyCurrentCellChanged(newFocusCell);
        }
        RefreshCellSelectionView();
    }
    else if (bShiftDown && IsMultiSelect()) {
        //Shift(+Ctrl)：保持锚点扩展区域；Ctrl+Shift 为追加，Shift 为替换（仅多选模式；单选模式退化为普通移动）
        ListCtrlCellId anchorCell;
        ListCtrlCellId focusCell;
        GetCellSelection(anchorCell, focusCell);
        if (!anchorCell.IsValid()) {
            anchorCell = cell;
        }
        SetCellSelection(anchorCell, newFocusCell, bCtrlDown);
    }
    else {
        //普通方向键：移动当前格（单选替换该格）
        SetCurrentCell(nNewRowIndex, nNewColumnId);
    }
    //确保当前格所在行可见
    if (m_pReportView != nullptr) {
        m_pReportView->EnsureDataItemVisible(nNewRowIndex, false);
    }
    return true;
}

void ListCtrl::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList.clear();
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            m_pReportView->GetDisplayDataItems(itemIndexList);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            m_pIconView->GetDisplayDataItems(itemIndexList);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            m_pListView->GetDisplayDataItems(itemIndexList);
        }
    }
}

size_t ListCtrl::GetTopDataItem() const
{
    size_t nTopItemIndex = Box::InvalidIndex;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            nTopItemIndex = m_pReportView->GetTopElementIndex();
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            nTopItemIndex = m_pIconView->GetTopElementIndex();
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            nTopItemIndex = m_pListView->GetTopElementIndex();
        }
    }
    return nTopItemIndex;
}

bool ListCtrl::IsDataItemDisplay(size_t itemIndex) const
{
    bool bItemVisible = false;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            bItemVisible = m_pReportView->IsDataItemDisplay(itemIndex);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            bItemVisible = m_pIconView->IsDataItemDisplay(itemIndex);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            bItemVisible = m_pListView->IsDataItemDisplay(itemIndex);
        }
    }
    return bItemVisible;
}

bool ListCtrl::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    bool bRet = false;
    if (m_listCtrlType == ListCtrlType::Report) {
        if (m_pReportView != nullptr) {
            bRet = m_pReportView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    else if (m_listCtrlType == ListCtrlType::Icon) {
        if (m_pIconView != nullptr) {
            bRet = m_pIconView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    else if (m_listCtrlType == ListCtrlType::List) {
        if (m_pListView != nullptr) {
            bRet = m_pListView->EnsureDataItemVisible(itemIndex, bToTop);
        }
    }
    return bRet;
}

void ListCtrl::Refresh(bool bSync)
{
    if (!IsInited()) {
        return;
    }
    if (m_bEnableRefresh) {
        if (m_listCtrlType == ListCtrlType::Report) {
            if (m_pReportView != nullptr) {
                m_pReportView->Refresh(bSync);
            }
        }
        else if (m_listCtrlType == ListCtrlType::Icon) {
            if (m_pIconView != nullptr) {
                m_pIconView->Refresh(bSync);
            }
        }
        else if (m_listCtrlType == ListCtrlType::List) {
            if (m_pListView != nullptr) {
                m_pListView->Refresh(bSync);
            }
        }
    }
}

void ListCtrl::RefreshDataItems(const std::vector<size_t>& dataItemIndexs)
{
    if (m_bEnableRefresh && !dataItemIndexs.empty()) {
        if (m_listCtrlType == ListCtrlType::Report) {
            if (m_pReportView != nullptr) {
                m_pReportView->RefreshElements(dataItemIndexs);
            }
        }
        else if (m_listCtrlType == ListCtrlType::Icon) {
            if (m_pIconView != nullptr) {
                m_pIconView->RefreshElements(dataItemIndexs);
            }
        }
        else if (m_listCtrlType == ListCtrlType::List) {
            if (m_pListView != nullptr) {
                m_pListView->RefreshElements(dataItemIndexs);
            }
        }        
    }
}

bool ListCtrl::SetEnableRefresh(bool bEnable)
{
    bool bOldEnable = m_bEnableRefresh;
    m_bEnableRefresh = bEnable;
    return bOldEnable;
}

bool ListCtrl::IsEnableRefresh() const
{
    return m_bEnableRefresh;
}

bool ListCtrl::RefreshSubItem(size_t dataItemIndex, size_t columnIndex)
{
    size_t columnId = GetColumnId(columnIndex);
    return RefreshSubItemById(dataItemIndex, columnId);
}

bool ListCtrl::RefreshSubItemById(size_t dataItemIndex, size_t columnId)
{
    if (!m_bEnableRefresh) {
        return false;
    }
    //单格刷新仅支持Report视图：Icon/List视图没有列的概念
    if (m_listCtrlType != ListCtrlType::Report) {
        return false;
    }
    if (m_pReportView == nullptr) {
        return false;
    }
    if ((dataItemIndex == Box::InvalidIndex) || (dataItemIndex >= GetDataItemCount())) {
        return false;
    }
    if (!IsValidColumnId(columnId)) {
        return false;
    }
    return m_pReportView->RefreshSubItemData(dataItemIndex, columnId);
}

bool ListCtrl::SetColumnVisible(size_t columnIndex, bool bColumnVisible)
{
    ListCtrlHeaderItem* pHeaderItem = GetColumn(columnIndex);
    if (pHeaderItem == nullptr) {
        return false;
    }
    if (pHeaderItem->IsColumnVisible() == bColumnVisible) {
        return true;
    }
    pHeaderItem->SetColumnVisible(bColumnVisible);
    return true;
}

bool ListCtrl::SetColumnVisibleById(size_t columnId, bool bColumnVisible)
{
    size_t columnIndex = GetColumnIndex(columnId);
    if (columnIndex == Box::InvalidIndex) {
        return false;
    }
    return SetColumnVisible(columnIndex, bColumnVisible);
}

bool ListCtrl::IsColumnVisible(size_t columnIndex) const
{
    ListCtrlHeaderItem* pHeaderItem = GetColumn(columnIndex);
    if (pHeaderItem == nullptr) {
        return false;
    }
    return pHeaderItem->IsColumnVisible();
}

bool ListCtrl::IsColumnVisibleById(size_t columnId) const
{
    size_t columnIndex = GetColumnIndex(columnId);
    if (columnIndex == Box::InvalidIndex) {
        return false;
    }
    return IsColumnVisible(columnIndex);
}

void ListCtrl::SetAutoCheckSelect(bool bAutoCheckSelect)
{
    m_bAutoCheckSelect = bAutoCheckSelect;
    m_pData->SetAutoCheckSelect(bAutoCheckSelect);
}

bool ListCtrl::IsAutoCheckSelect() const
{
    return m_bAutoCheckSelect;
}

bool ListCtrl::SetHeaderShowCheckBox(bool bShow)
{
    m_bHeaderShowCheckBox = bShow;
    bool bRet = false;
    if (IsInited()) {
        ListCtrlHeader* pHeaderCtrl = GetHeaderCtrl();
        ASSERT(pHeaderCtrl != nullptr);
        if (pHeaderCtrl != nullptr) {
            bRet = pHeaderCtrl->SetShowCheckBox(bShow);
        }
    }
    else {
        bRet = true;
    }
    return bRet;
}

bool ListCtrl::IsHeaderShowCheckBox() const
{
    bool bRet = false;
    if (IsInited()) {
        ListCtrlHeader* pHeaderCtrl = GetHeaderCtrl();
        ASSERT(pHeaderCtrl != nullptr);
        if (pHeaderCtrl != nullptr) {
            bRet = pHeaderCtrl->IsShowCheckBox();
        }
    }
    else {
        bRet = m_bHeaderShowCheckBox;
    }
    return bRet;
}

void ListCtrl::SetDataItemShowCheckBox(bool bShow)
{
    if (m_bDataItemShowCheckBox != bShow) {
        m_bDataItemShowCheckBox = bShow;
        if (IsInited()) {
            Refresh();
        }
    }
}

bool ListCtrl::IsDataItemShowCheckBox() const
{
    return m_bDataItemShowCheckBox;
}

bool ListCtrl::SetDataItemCheck(size_t itemIndex, bool bCheck)
{
    bool bChanged = false;
    bool bRet = m_pData->SetDataItemChecked(itemIndex, bCheck, bChanged);
    if (bChanged) {
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemCheck(size_t itemIndex) const
{
    return m_pData->IsDataItemChecked(itemIndex);
}

void ListCtrl::OnItemEnterEditMode(size_t itemIndex, size_t nColumnId,
                                   IListBoxItem* pItem, ListCtrlLabel* pSubItem)
{
    ASSERT(itemIndex < GetDataItemCount());
    ASSERT(GetColumnIndex(nColumnId) < GetColumnCount());
    ASSERT((pItem != nullptr) && (pSubItem != nullptr));
    ASSERT(pItem->GetElementIndex() == itemIndex);
    if (itemIndex >= GetDataItemCount()) {
        return;
    }
    if (pItem->GetElementIndex() != itemIndex) {
        return;
    }
    size_t nColumnIndex = GetColumnIndex(nColumnId);
    if (nColumnIndex >= GetColumnCount()) {
        return;
    }

    ListCtrlEditParam editParam;
    editParam.listCtrlType = m_listCtrlType;
    editParam.nItemIndex = itemIndex;
    editParam.nColumnId = nColumnId;
    editParam.nColumnIndex = nColumnIndex;
    editParam.pItem = pItem;
    editParam.pSubItem = pSubItem;

    if (!IsValidItemEditState(editParam) && !IsValidItemEditParam(editParam)) {
        ASSERT(0);
        return;
    }

    DString editClass = GetRichEditClass();
    ASSERT(!editClass.empty());
    if (editClass.empty()) {
        return;
    }

    //启动定时器, 只执行一次(使用定时器的原因：避免影响双击操作)
    m_editModeFlag.Cancel();
    std::function<void()> editModeCallback = UiBind(&ListCtrl::OnItemEditMode, this, editParam);
    TimerManager& timer = GlobalManager::Instance().Timer();
    timer.AddTimer(m_editModeFlag.GetWeakFlag(), editModeCallback, 600, 1);
}

bool ListCtrl::IsValidItemEditState(const ListCtrlEditParam& editParam) const
{
    if (!IsEnableItemEdit()) {
        return false;
    }
    if (editParam.listCtrlType != m_listCtrlType) {
        return false;
    }

    if (editParam.listCtrlType == ListCtrlType::Icon) {
        //Icon视图
        ListCtrlIconViewItem* pItem = dynamic_cast<ListCtrlIconViewItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pIconView != nullptr);
        if (m_pIconView == nullptr) {
            return false;
        }
    }
    else if (editParam.listCtrlType == ListCtrlType::List) {
        //List视图
        ListCtrlListViewItem* pItem = dynamic_cast<ListCtrlListViewItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pListView != nullptr);
        if (m_pListView == nullptr) {
            return false;
        }
    }
    else {
        //Report视图
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(editParam.pItem);
        ASSERT((pItem != nullptr) && pItem->IsVisible() && pItem->IsSelected() && pItem->IsFocused());
        if ((pItem == nullptr) || !pItem->IsVisible() || !pItem->IsSelected() || !pItem->IsFocused()) {
            return false;
        }
        ASSERT(m_pReportView != nullptr);
        if (m_pReportView == nullptr) {
            return false;
        }
    }
    return true;
}

bool ListCtrl::IsValidItemEditParam(const ListCtrlEditParam& editParam) const
{
    if (editParam.listCtrlType != m_listCtrlType) {
        return false;
    }
    if (editParam.listCtrlType == ListCtrlType::Icon) {
        //Icon视图
        ListCtrlIconViewItem* pItem = dynamic_cast<ListCtrlIconViewItem*>(editParam.pItem);
        ASSERT(pItem != nullptr);
        if (pItem == nullptr) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;
        ListCtrlIconViewItem* pDestItem = nullptr;
        ListCtrlIconViewItem* pNextItem = GetFirstDisplayIconItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayIconItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }

        size_t nSubItemIndex = pItem->GetItemIndex(editParam.pSubItem);
        ASSERT(nSubItemIndex < pItem->GetItemCount());
        if (nSubItemIndex >= pItem->GetItemCount()) {
            return false;
        }

        size_t nColumnIndex = editParam.nColumnIndex;
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    else if (editParam.listCtrlType == ListCtrlType::List) {
        //List视图
        ListCtrlListViewItem* pItem = dynamic_cast<ListCtrlListViewItem*>(editParam.pItem);
        ASSERT(pItem != nullptr);
        if (pItem == nullptr) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;        
        ListCtrlListViewItem* pDestItem = nullptr;
        ListCtrlListViewItem* pNextItem = GetFirstDisplayListItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayListItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }

        size_t nSubItemIndex = pItem->GetItemIndex(editParam.pSubItem);
        ASSERT(nSubItemIndex < pItem->GetItemCount());
        if (nSubItemIndex >= pItem->GetItemCount()) {
            return false;
        }

        size_t nColumnIndex = editParam.nColumnIndex;
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    else {
        //Report视图
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(editParam.pItem);
        ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(editParam.pSubItem);
        ASSERT((pItem != nullptr) && (pSubItem != nullptr));
        if ((pItem == nullptr) || (pSubItem == nullptr)) {
            return false;
        }
        size_t nDataItemIndex = editParam.nItemIndex;
        size_t nColumnIndex = editParam.nColumnIndex;
        ListCtrlItem* pDestItem = nullptr;
        ListCtrlItem* pNextItem = GetFirstDisplayItem();
        while (pNextItem != nullptr) {
            if (pNextItem->GetElementIndex() == nDataItemIndex) {
                pDestItem = pNextItem;
                break;
            }
            pNextItem = GetNextDisplayItem(pNextItem);
        }
        ASSERT(pDestItem == pItem);
        if (pDestItem != pItem) {
            //已经发生变化
            return false;
        }
        if (pItem->GetSubItemIndex(pSubItem) != nColumnIndex) {
            return false;
        }
        ASSERT(GetColumnId(nColumnIndex) == editParam.nColumnId);
        if (GetColumnId(nColumnIndex) != editParam.nColumnId) {
            return false;
        }
    }
    return true;
}

void ListCtrl::OnItemEditMode(ListCtrlEditParam editParam)
{
    DString editClass = GetRichEditClass();
    ASSERT(!editClass.empty());
    if (editClass.empty()) {
        return;
    }
    if (!IsValidItemEditState(editParam) && !IsValidItemEditParam(editParam)) {
        //状态已经失效
        return;
    }
    if (m_pRichEdit == nullptr) {
        m_pRichEdit = new RichEdit(GetWindow());
        AddItem(m_pRichEdit);
        m_pRichEdit->SetClass(editClass);
    }
    else if(GetItemCount() > 0) {
        //将Edit控件，调整到最后
        size_t nItem = GetItemIndex(m_pRichEdit);
        size_t nLastItemIndex = GetItemCount() - 1;
        if (nItem != nLastItemIndex) {
            SetItemIndex(m_pRichEdit, nLastItemIndex);
        }
    }
    if (m_pRichEdit == nullptr) {
        return;
    }

    ListCtrlLabel* pSubItem = editParam.pSubItem;
    if (pSubItem == nullptr) {
        return;
    }
    size_t nDataItemIndex = editParam.nItemIndex;

    DString sOldItemText = pSubItem->GetText();
    UiRect rcItem = pSubItem->GetTextRect();
    UiPoint offsetPt = pSubItem->GetScrollOffsetInScrollBox();
    rcItem.Offset(-offsetPt.x, -offsetPt.y);
    UiRect rect = GetRect();
    UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
    m_pRichEdit->SetMargin(rcMargin, false);
    m_pRichEdit->SetFloat(true);
    m_pRichEdit->SetVisible(true);
    m_pRichEdit->SetText(sOldItemText);
    m_pRichEdit->SetSelAll();
    UpdateRichEditSize(pSubItem);

    //还原焦点：将编辑框的焦点还原会原来的列表控件
    auto RestoreItemFocus = [this, nDataItemIndex]() {
            if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible() && m_pRichEdit->IsFocused()) {
                ListCtrlItem* pDestItem = nullptr;
                ListCtrlItem* pNextItem = GetFirstDisplayItem();
                while (pNextItem != nullptr) {
                    if (pNextItem->GetElementIndex() == nDataItemIndex) {
                        pDestItem = pNextItem;
                        break;
                    }
                    pNextItem = GetNextDisplayItem(pNextItem);
                }
                if (pDestItem != nullptr) {
                    pDestItem->SetFocus();
                }
            }
        };

    //触发事件：开始编辑
    ListCtrlEditParam enterEditParam = editParam;
    enterEditParam.sNewText.clear();
    enterEditParam.bCancelled = false;
    SendEvent(kEventEnterEdit, (WPARAM)&enterEditParam);
    if (enterEditParam.bCancelled || !IsValidItemEditState(editParam) || !IsValidItemEditParam(editParam)) {
        //状态已经失效, 或者用户取消编辑
        ClearEditEvents();
        RestoreItemFocus();
        LeaveEditMode();
        return;
    }

    m_pRichEdit->SetFloat(true);
    m_pRichEdit->SetVisible(true);
    m_pRichEdit->SetFocus();

    //文本变化的时候，自动调整编辑框的大小
    m_pRichEdit->DetachEvent(kEventTextChanged);
    m_pRichEdit->AttachTextChanged([this, pSubItem](const EventArgs&) {
        UpdateRichEditSize(pSubItem);
        return true;
        });

    //编辑结束的时候，触发事件
    auto OnLeaveRichEdit = [this, sOldItemText, RestoreItemFocus, editParam]() {
        DString sNewItemText;
        if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible()) {
            sNewItemText = m_pRichEdit->GetText();
            m_pRichEdit->SetVisible(false);                     
        }
        if ((sNewItemText != sOldItemText) && IsValidItemEditParam(editParam)) {
            //文本内容发生变化
            OnItemEdited(editParam, sNewItemText);
        }
        };

    //按回车：应用修改
    m_pRichEdit->DetachEvent(kEventReturn);
    m_pRichEdit->AttachReturn([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });
    //按ESC键：取消修改
    m_pRichEdit->DetachEvent(kEventKeyDown);
    m_pRichEdit->AttachEvent(kEventKeyDown, [this, RestoreItemFocus](const EventArgs& msg) {
        if ((msg.eventType == kEventKeyDown) && (msg.vkCode == kVK_ESCAPE)) {
            ClearEditEvents();
            RestoreItemFocus();
            if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible()) {
                m_pRichEdit->SetVisible(false);
            }
            return false;
        }
        else {
            return true;
        }
        }, 0);

    //控件失去焦点: 应用修改
    m_pRichEdit->DetachEvent(kEventKillFocus);
    m_pRichEdit->AttachKillFocus([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });

    //窗口失去焦点: 应用修改
    m_pRichEdit->DetachEvent(kEventWindowKillFocus);
    m_pRichEdit->AttachWindowKillFocus([this, RestoreItemFocus, OnLeaveRichEdit](const EventArgs&) {
        ClearEditEvents();
        RestoreItemFocus();
        OnLeaveRichEdit();
        return false;
        });
}

void ListCtrl::UpdateRichEditSize(ListCtrlLabel* pSubItem)
{
    if ((pSubItem == nullptr) || (m_pRichEdit == nullptr)) {
        return;
    }
    UiRect rcItem = pSubItem->GetTextRect();
    UiPoint offsetPt = pSubItem->GetScrollOffsetInScrollBox();
    rcItem.Offset(-offsetPt.x, -offsetPt.y);
    UiRect rect = GetRect();
    bool bSingleLine = pSubItem->IsSingleLine();
    if (bSingleLine) {
        if (m_pRichEdit->IsMultiLine()) {
            m_pRichEdit->SetMultiLine(false);
        }
        m_pRichEdit->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        m_pRichEdit->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        UiEstSize sz = m_pRichEdit->EstimateSize(UiSize(0, 0));
        //设置宽度
        m_pRichEdit->SetFixedWidth(UiFixedInt(sz.cx.GetInt32()), true, false);

        //设置高度
        const int32_t nEditHeight = sz.cy.GetInt32() + sz.cy.GetInt32() / 2;
        m_pRichEdit->SetFixedHeight(UiFixedInt(nEditHeight), true, false);
        m_pRichEdit->SetMinHeight(nEditHeight, false);

        //设置文本框所在位置和大小限制
        uint32_t textStyle = pSubItem->GetTextStyle();
        if (textStyle & TEXT_VCENTER) {
            //CENTER对齐
            rcItem.top = rcItem.CenterY() - nEditHeight / 2;
            rcItem.bottom = rcItem.top + nEditHeight;
        }
        else if (textStyle & TEXT_BOTTOM) {
            //BOTTOM对齐
            rcItem.top = rcItem.bottom - nEditHeight;
        }

        UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
        rcMargin.Validate();
        m_pRichEdit->SetMargin(rcMargin, false);

        m_pRichEdit->SetMinWidth(rcItem.Width(), false);
        int32_t nMaxWidth = std::max(rcItem.Width(), rect.Width() - rcMargin.left);
        m_pRichEdit->SetMaxWidth(nMaxWidth, false);
    }
    else {
        if (!m_pRichEdit->IsMultiLine()) {
            m_pRichEdit->SetMultiLine(true);
        }

        //宽度固定：与原控件相同
        m_pRichEdit->SetFixedWidth(UiFixedInt(rcItem.Width()), false, false);
        m_pRichEdit->SetMaxWidth(rcItem.Width(), false);
        m_pRichEdit->SetMinWidth(rcItem.Width(), false);
        
        //设置高度和位置
        m_pRichEdit->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);        
        UiEstSize sz = m_pRichEdit->EstimateSize(UiSize(rcItem.Width(), 0));
        m_pRichEdit->SetFixedHeight(UiFixedInt(sz.cy.GetInt32()), true, false);

        UiMargin rcMargin(rcItem.left - rect.left, rcItem.top - rect.top, 0, 0);
        m_pRichEdit->SetMargin(rcMargin, false);

        int32_t nMaxHeight = std::max(rcItem.Height(), rect.Height() - rcMargin.top);
        m_pRichEdit->SetMaxHeight(nMaxHeight, false);
        m_pRichEdit->SetMinHeight(rcItem.Height(), false);
    }
}

void ListCtrl::OnItemEdited(const ListCtrlEditParam& editParam, const DString& newItemText)
{
    //触发事件：结束编辑，如果用户取消编辑，那么不执行修改操作
    ListCtrlEditParam leaveEditParam = editParam;
    leaveEditParam.sNewText = newItemText;
    leaveEditParam.bCancelled = false;
    SendEvent(kEventLeaveEdit, (WPARAM)&leaveEditParam);
    if (!leaveEditParam.bCancelled) {
        //用户未取消编辑
        SetSubItemText(editParam.nItemIndex, editParam.nColumnIndex, newItemText);
    }    
}

void ListCtrl::OnViewMouseEvents(const EventArgs& msg)
{
    if ((msg.eventType == kEventMouseWheel) ||
        (msg.eventType == kEventMouseButtonDown) ||
        (msg.eventType == kEventMouseButtonUp) ||
        (msg.eventType == kEventMouseDoubleClick) ||
        (msg.eventType == kEventMouseRButtonDown) ||
        (msg.eventType == kEventMouseRButtonUp) ||
        (msg.eventType == kEventMouseRDoubleClick) ||
        (msg.eventType == kEventContextMenu)) {
        if (msg.GetSender() != m_pRichEdit) {
            LeaveEditMode();
        }
    }    
}

void ListCtrl::OnViewKeyboardEvents(const EventArgs& msg)
{
    if (msg.GetSender() != m_pRichEdit) {
        LeaveEditMode();
    }    
}

void ListCtrl::LeaveEditMode()
{
    //取消编辑状态的定时器
    m_editModeFlag.Cancel();
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible() && m_pRichEdit->IsFocused()) {
        m_pRichEdit->SendEvent(kEventKillFocus);
    }
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsVisible()) {
        m_pRichEdit->SetVisible(false);
    }
    ClearEditEvents();
}

void ListCtrl::ClearEditEvents()
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->DetachEvent(kEventWindowKillFocus);
        m_pRichEdit->DetachEvent(kEventKillFocus);
        m_pRichEdit->DetachEvent(kEventReturn);
        m_pRichEdit->DetachEvent(kEventTextChanged);
        m_pRichEdit->DetachEvent(kEventKeyDown);
    }
}

}//namespace ui

