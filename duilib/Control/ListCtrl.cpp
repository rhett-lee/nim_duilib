#include "ListCtrl.h"
#include "duilib/Control/ListCtrlDataView.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

ListCtrl::ListCtrl():
    m_bInited(false),
    m_pHeaderCtrl(nullptr),
    m_pDataView(nullptr),
    m_bEnableHeaderDragOrder(true),
    m_bShowHeaderCtrl(true),
    m_bEnableRefresh(true),
    m_bMultiSelect(true),
    m_bEnableColumnWidthAuto(true),
    m_bAutoCheckSelect(false),
    m_bHeaderShowCheckBox(false),
    m_bDataItemShowCheckBox(false)
{
    m_pDataProvider = new ListCtrlDataProvider;
    m_nRowGridLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
    m_nColumnGridLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
    m_nItemHeight = GlobalManager::Instance().Dpi().GetScaleInt(32);
    m_nHeaderHeight = m_nItemHeight;
}

ListCtrl::~ListCtrl()
{
    if (m_pDataProvider != nullptr) {
        delete m_pDataProvider;
        m_pDataProvider = nullptr;
    }
}

std::wstring ListCtrl::GetType() const { return DUI_CTR_LISTCTRL; }

void ListCtrl::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"header_class") {
        SetHeaderClass(strValue);
    }
    else if (strName == L"header_item_class") {
        SetHeaderItemClass(strValue);
    }
    else if (strName == L"header_split_box_class") {
        SetHeaderSplitBoxClass(strValue);
    }
    else if (strName == L"header_split_control_class") {
        SetHeaderSplitControlClass(strValue);
    }
    else if (strName == L"enable_header_drag_order") {
        SetEnableHeaderDragOrder(strValue == L"true");
    }
    else if (strName == L"check_box_class") {
        SetCheckBoxClass(strValue);
    }
    else if (strName == L"data_item_class") {
        SetDataItemClass(strValue);
    }
    else if (strName == L"data_sub_item_class") {
        SetDataSubItemClass(strValue);
    }
    else if (strName == L"row_grid_line_width") {
        SetRowGridLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"row_grid_line_color") {
        SetRowGridLineColor(strValue);
    }
    else if (strName == L"column_grid_line_width") {
        SetColumnGridLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"column_grid_line_color") {
        SetColumnGridLineColor(strValue);
    }
    else if (strName == L"data_view_class") {
        SetDataViewClass(strValue);
    }
    else if (strName == L"header_height") {
        SetHeaderHeight(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"data_item_height") {
        SetDataItemHeight(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"show_header") {
        SetHeaderVisible(strValue == L"true");
    }
    else if (strName == L"multi_select") {
        SetMultiSelect(strValue == L"true");
    }
    else if (strName == L"enable_column_width_auto") {
        SetEnableColumnWidthAuto(strValue == L"true");
    }
    else if (strName == L"auto_check_select") {
        SetAutoCheckSelect(strValue == L"true");
    }
    else if (strName == L"show_header_checkbox") {
        SetHeaderShowCheckBox(strValue == L"true");
    }
    else if (strName == L"show_data_item_checkbox") {
        SetDataItemShowCheckBox(strValue == L"true");
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void ListCtrl::SetHeaderClass(const std::wstring& className)
{
    m_headerClass = className;
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetClass(className);
    }
}

void ListCtrl::SetHeaderItemClass(const std::wstring& className)
{
    m_headerItemClass = className;
}

std::wstring ListCtrl::GetHeaderItemClass() const
{
    return m_headerItemClass.c_str();
}

void ListCtrl::SetHeaderSplitBoxClass(const std::wstring& className)
{
    m_headerSplitBoxClass = className;
}

std::wstring ListCtrl::GetHeaderSplitBoxClass() const
{
    return m_headerSplitBoxClass.c_str();
}

void ListCtrl::SetHeaderSplitControlClass(const std::wstring& className)
{
    m_headerSplitControlClass = className;
}

std::wstring ListCtrl::GetHeaderSplitControlClass() const
{
    return m_headerSplitControlClass.c_str();
}

void ListCtrl::SetCheckBoxClass(const std::wstring& className)
{
    m_checkBoxClass = className;
}

std::wstring ListCtrl::GetCheckBoxClass() const
{
    return m_checkBoxClass.c_str();
}

void ListCtrl::SetDataViewClass(const std::wstring& className)
{
    m_dataViewClass = className;
}

std::wstring ListCtrl::GetDataViewClass() const
{
    return m_dataViewClass.c_str();
}

void ListCtrl::SetDataItemClass(const std::wstring& className)
{
    m_dataItemClass = className;
}

std::wstring ListCtrl::GetDataItemClass() const
{
    return m_dataItemClass.c_str();
}

void ListCtrl::SetDataSubItemClass(const std::wstring& className)
{
    m_dataSubItemClass = className;
}

std::wstring ListCtrl::GetDataSubItemClass() const
{
    return m_dataSubItemClass.c_str();
}

void ListCtrl::SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nRowGridLineWidth != nLineWidth) {
        m_nRowGridLineWidth = nLineWidth;
        if (m_pDataView != nullptr) {
            m_pDataView->Invalidate();
        }
    }
}

int32_t ListCtrl::GetRowGridLineWidth() const
{
    return m_nRowGridLineWidth;
}

void ListCtrl::SetRowGridLineColor(const std::wstring& color)
{
    if (m_rowGridLineColor != color) {
        m_rowGridLineColor = color;
        if (m_pDataView != nullptr) {
            m_pDataView->Invalidate();
        }
    }
}

std::wstring ListCtrl::GetRowGridLineColor() const
{
    return m_rowGridLineColor.c_str();
}

void ListCtrl::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nColumnGridLineWidth != nLineWidth) {
        m_nColumnGridLineWidth = nLineWidth;
        if (m_pDataView != nullptr) {
            m_pDataView->Invalidate();
        }
    }    
}

int32_t ListCtrl::GetColumnGridLineWidth() const
{
    return m_nColumnGridLineWidth;
}

void ListCtrl::SetColumnGridLineColor(const std::wstring& color)
{
    if (m_columnGridLineColor != color) {
        m_columnGridLineColor = color;
        if (m_pDataView != nullptr) {
            m_pDataView->Invalidate();
        }
    }
}

std::wstring ListCtrl::GetColumnGridLineColor() const
{
    return m_columnGridLineColor.c_str();
}

void ListCtrl::SetEnableColumnWidthAuto(bool bEnable)
{
    m_bEnableColumnWidthAuto = bEnable;
}

bool ListCtrl::IsEnableColumnWidthAuto() const
{
    return m_bEnableColumnWidthAuto;
}

void ListCtrl::DoInit()
{
    if (m_bInited) {
        return;
    }
    m_bInited = true;

    //初始化Header
    ASSERT(m_pHeaderCtrl == nullptr);
    if (m_pHeaderCtrl == nullptr) {
        m_pHeaderCtrl = new ListCtrlHeader;
    }
    m_pHeaderCtrl->SetListCtrl(this);

    if (!m_headerClass.empty()) {
        m_pHeaderCtrl->SetClass(m_headerClass.c_str());
    }
    m_pHeaderCtrl->SetAutoCheckSelect(false);
    m_pHeaderCtrl->SetShowCheckBox(m_bHeaderShowCheckBox); //是否显示CheckBox
    m_pHeaderCtrl->SetFixedHeight(UiFixedInt(m_nHeaderHeight), true, false);
    if (!m_bShowHeaderCtrl) {
        SetHeaderVisible(false);
    }
    m_pDataProvider->SetListCtrl(this);

    //初始化Body
    ASSERT(m_pDataView == nullptr);
    m_pDataView = new ListCtrlDataView;
    m_pDataView->SetListCtrl(this);
    if (!m_dataViewClass.empty()) {
        m_pDataView->SetClass(m_dataViewClass.c_str());
    }

    AddItem(m_pDataView);

    // Header添加到数据视图中管理，作为第一个元素，在Layout的实现中控制显示属性
    m_pDataView->AddItem(m_pHeaderCtrl);

    //同步单选和多选的状态
    m_pDataView->SetMultiSelect(IsMultiSelect());
    m_pDataView->SetDataProvider(m_pDataProvider);

    //事件转接函数
    auto OnDataViewEvent = [this](const EventArgs & args) {
        size_t nItemIndex = args.wParam;
        Control* pControl = m_pDataView->GetItemAt(nItemIndex);
        ListCtrlItem* pItem = nullptr;
        if (pControl != nullptr) {
            pItem = dynamic_cast<ListCtrlItem*>(pControl);
        }
        if (pItem != nullptr) {
            EventArgs msg = args;
            msg.wParam = (WPARAM)pItem;
            msg.lParam = pItem->GetElementIndex();
            msg.pSender = this;
            SendEvent(msg);
        }
        else if (args.Type == kEventSelChange) {
            EventArgs msg = args;
            msg.pSender = this;
            SendEvent(msg);
        }
    };

    //挂载事件，转接给外层
    m_pDataView->AttachSelect([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
    m_pDataView->AttachSelChange([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
    m_pDataView->AttachDoubleClick([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
    m_pDataView->AttachClick([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
    m_pDataView->AttachRClick([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
    m_pDataView->AttachReturn([this, OnDataViewEvent](const EventArgs& args) {
        OnDataViewEvent(args);
        return true;
        });
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

bool ListCtrl::SetColumnWidthAuto(size_t columnIndex)
{
    bool bRet = false;
    size_t nColumnId = GetColumnId(columnIndex);
    if (nColumnId == Box::InvalidIndex) {
        return bRet;
    }
    //计算该列的宽度
    int32_t nMaxWidth = m_pDataProvider->GetColumnWidthAuto(nColumnId);
    if (nMaxWidth > 0) {
        bRet = SetColumnWidth(columnIndex, nMaxWidth, false);
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
        return pHeaderItem->GetColomnId();
    }
    return Box::InvalidIndex;
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
    else if(!m_bInited){
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
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nHeaderHeight);
    }
    if (m_pHeaderCtrl != nullptr) {
        m_pHeaderCtrl->SetFixedHeight(UiFixedInt(nHeaderHeight), true, false);
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
    }
    return nHeaderHeight;
}

void ListCtrl::SetDataItemHeight(int32_t nItemHeight, bool bNeedDpiScale)
{
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nItemHeight);
    }
    m_nItemHeight = nItemHeight;
    Refresh();
}

int32_t ListCtrl::GetDataItemHeight() const
{
    return m_nItemHeight;
}

ListCtrlItem* ListCtrl::GetFirstDisplayItem() const
{
    ListCtrlItem* pItem = nullptr;
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        size_t itemCount = m_pDataView->GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            pItem = dynamic_cast<ListCtrlItem*>(m_pDataView->GetItemAt(index));
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
        ASSERT(m_pDataView != nullptr);
        if (m_pDataView != nullptr) {
            size_t itemCount = m_pDataView->GetItemCount();
            size_t nStartIndex = m_pDataView->GetItemIndex(pItem);
            if ((itemCount > 0) && (nStartIndex < (itemCount - 1))) {
                for (size_t index = nStartIndex + 1; index < itemCount; ++index) {
                    ListCtrlItem* pCheckItem = dynamic_cast<ListCtrlItem*>(m_pDataView->GetItemAt(index));
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
    if ((m_pDataView == nullptr) || (m_pHeaderCtrl == nullptr)){
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

    if ((nColumnWidth1 < 0) && (nColumnWidth2 < 0)) {
        return;
    }

    size_t itemCount = m_pDataView->GetItemCount();
    for (size_t index = 1; index < itemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(m_pDataView->GetItemAt(index));
        if (pItem == nullptr) {
            continue;
        }
        size_t columnCount = pItem->GetItemCount();
        for (size_t nColumn = 0; nColumn < columnCount; ++nColumn) {
            if ((nColumn == nColumn1) && (nColumnWidth1 >= 0)) {
                ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nColumn));
                if (pSubItem != nullptr) {
                    pSubItem->SetFixedWidth(UiFixedInt(nColumnWidth1), true, false);
                }
            }
            if ((nColumn == nColumn2) && (nColumnWidth2 >= 0)) {
                ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nColumn));
                if (pSubItem != nullptr) {
                    pSubItem->SetFixedWidth(UiFixedInt(nColumnWidth2), true, false);
                }
            }
        }
    }
    Arrange();
}

void ListCtrl::OnHeaderColumnAdded(size_t nColumnId)
{
    m_pDataProvider->AddColumn(nColumnId);
}

void ListCtrl::OnHeaderColumnRemoved(size_t nColumnId)
{
    m_pDataProvider->RemoveColumn(nColumnId);
}

void ListCtrl::OnColumnSorted(size_t nColumnId, bool bSortedUp)
{
    //对数据排序，然后刷新界面显示
    m_pDataProvider->SortDataItems(nColumnId, bSortedUp, nullptr, nullptr);
    Refresh();
}

void ListCtrl::OnHeaderColumnOrderChanged()
{
    Refresh();
}

void ListCtrl::OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked)
{
    //界面状态变化，同步到底层存储
    m_pDataProvider->SetColumnCheck(nColumnId, bChecked);
    Refresh();
}

void ListCtrl::OnHeaderCheckStateChanged(bool bChecked)
{
    //界面状态变化，同步到底层存储
    if (m_pDataProvider->SetAllDataItemsCheck(bChecked)) {
        Refresh();
    }    
}

void ListCtrl::OnHeaderColumnVisibleChanged()
{
    Refresh();
}

void ListCtrl::OnHeaderColumnSplitDoubleClick(ListCtrlHeaderItem* pHeaderItem)
{
    if (!IsEnableColumnWidthAuto()) {
        //功能关闭
        return;
    }
    //自动调整该列的宽度
    if (pHeaderItem != nullptr) {
        size_t nColumnId = pHeaderItem->GetColomnId();
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
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
                columnIdList.push_back(pHeaderItem->GetColomnId());
            }
        }
    }
    else {
        size_t columnIndex = GetColumnIndex(nColumnId);
        if (columnIndex != Box::InvalidIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())){
                columnIdList.push_back(nColumnId);
            }
        }
    }
    for (size_t columnId : columnIdList) {
        bool bChecked = false;
        bool bPartChecked = false;
        m_pDataProvider->GetCheckBoxCheckStatus(columnId, bChecked, bPartChecked);
        ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumnById(columnId);
        if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
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
    m_pDataProvider->GetDataItemsCheckStatus(bChecked, bPartChecked);
    if ((m_pHeaderCtrl->IsChecked() != bChecked) || 
        (m_pHeaderCtrl->IsPartSelected() != bPartChecked)) {
        bool bOldValue = m_pHeaderCtrl->SetEnableCheckChangeEvent(false);
        m_pHeaderCtrl->SetChecked(bChecked, false);
        m_pHeaderCtrl->SetPartSelected(bPartChecked);
        m_pHeaderCtrl->SetEnableCheckChangeEvent(bOldValue);
    }
}

size_t ListCtrl::GetDataItemCount() const
{
    return m_pDataProvider->GetDataItemCount();
}

bool ListCtrl::SetDataItemCount(size_t itemCount)
{
    bool bRet = m_pDataProvider->SetDataItemCount(itemCount);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

size_t ListCtrl::AddDataItem(const ListCtrlDataItem& dataItem)
{
    bool bRet = m_pDataProvider->AddDataItem(dataItem);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::InsertDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem)
{
    bool bRet = m_pDataProvider->InsertDataItem(itemIndex, dataItem);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::SetDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem)
{
    bool bCheckChanged = false;
    ListCtrlDataItem oldDataItem;
    if (GetDataItem(itemIndex, dataItem.nColumnIndex, oldDataItem)) {
        bCheckChanged = oldDataItem.bChecked != dataItem.bChecked;
    }
    bool bRet = m_pDataProvider->SetDataItem(itemIndex, dataItem);
    if (bCheckChanged) {
        UpdateHeaderColumnCheckBox(GetColumnId(itemIndex));
    }    
    return bRet;
}

bool ListCtrl::GetDataItem(size_t itemIndex, size_t columnIndex, ListCtrlDataItem& dataItem) const
{
    return m_pDataProvider->GetDataItem(itemIndex, columnIndex, dataItem);
}

bool ListCtrl::DeleteDataItem(size_t itemIndex)
{
    bool bRet = m_pDataProvider->DeleteDataItem(itemIndex);
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::DeleteAllDataItems()
{
    bool bRet = m_pDataProvider->DeleteAllDataItems();
    if (bRet) {
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::SetDataItemRowData(size_t itemIndex, const ListCtrlRowData& itemData)
{
    bool bChanged = false;
    ListCtrlRowData oldItemData;
    bool bRet = m_pDataProvider->SetDataItemRowData(itemIndex, itemData, bChanged);
    if (bChanged) {
        Refresh();
        if (oldItemData.bChecked != itemData.bChecked) {
            UpdateHeaderCheckBox();
        }
    }
    return bRet;
}

bool ListCtrl::GetDataItemRowData(size_t itemIndex, ListCtrlRowData& itemData) const
{
    return m_pDataProvider->GetDataItemRowData(itemIndex, itemData);
}

bool ListCtrl::SetDataItemData(size_t itemIndex, size_t itemData)
{
    return m_pDataProvider->SetDataItemData(itemIndex, itemData);
}

bool ListCtrl::SetDataItemVisible(size_t itemIndex, bool bVisible)
{
    bool bChanged = false;
    bool bRet = m_pDataProvider->SetDataItemVisible(itemIndex, bVisible, bChanged);
    if (bChanged) {
        Refresh();
        UpdateHeaderColumnCheckBox(Box::InvalidIndex);
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemVisible(size_t itemIndex) const
{
    return m_pDataProvider->IsDataItemVisible(itemIndex);
}

bool ListCtrl::SetDataItemSelected(size_t itemIndex, bool bSelected)
{
    bool bChanged = false;
    bool bOldChecked = m_pDataProvider->IsDataItemChecked(itemIndex);
    bool bRet = m_pDataProvider->SetDataItemSelected(itemIndex, bSelected, bChanged);
    if (bChanged) {
        Refresh();
        if (m_pDataProvider->IsDataItemChecked(itemIndex) != bOldChecked) {
            UpdateHeaderCheckBox();
        }
    }
    return bRet;
}

bool ListCtrl::IsDataItemSelected(size_t itemIndex) const
{
    return m_pDataProvider->IsDataItemSelected(itemIndex);
}

bool ListCtrl::SetDataItemImageId(size_t itemIndex, int32_t imageId)
{
    bool bChanged = false;
    bool bRet = m_pDataProvider->SetDataItemImageId(itemIndex, imageId, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemImageId(size_t itemIndex) const
{
    return m_pDataProvider->GetDataItemImageId(itemIndex);
}

bool ListCtrl::SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop)
{
    bool bChanged = false;
    bool bRet = m_pDataProvider->SetDataItemAlwaysAtTop(itemIndex, nAlwaysAtTop, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int8_t ListCtrl::GetDataItemAlwaysAtTop(size_t itemIndex) const
{
    return m_pDataProvider->GetDataItemAlwaysAtTop(itemIndex);
}

bool ListCtrl::SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale)
{
    bool bChanged = false;
    bool bRet = m_pDataProvider->SetDataItemHeight(itemIndex, nItemHeight, bNeedDpiScale, bChanged);
    if (bChanged) {
        Refresh();
    }
    return bRet;
}

int32_t ListCtrl::GetDataItemHeight(size_t itemIndex) const
{
    return m_pDataProvider->GetDataItemHeight(itemIndex);
}

size_t ListCtrl::GetDataItemData(size_t itemIndex) const
{
    return m_pDataProvider->GetDataItemData(itemIndex);
}

bool ListCtrl::SetDataItemText(size_t itemIndex, size_t columnIndex, const std::wstring& text)
{
    return m_pDataProvider->SetDataItemText(itemIndex, columnIndex, text);
}

std::wstring ListCtrl::GetDataItemText(size_t itemIndex, size_t columnIndex) const
{
    return m_pDataProvider->GetDataItemText(itemIndex, columnIndex);
}

bool ListCtrl::SetDataItemTextColor(size_t itemIndex, size_t columnIndex, const UiColor& textColor)
{
    return m_pDataProvider->SetDataItemTextColor(itemIndex, columnIndex, textColor);
}

UiColor ListCtrl::GetDataItemTextColor(size_t itemIndex, size_t columnIndex) const
{
    UiColor textColor;
    m_pDataProvider->GetDataItemTextColor(itemIndex, columnIndex, textColor);
    return textColor;
}

bool ListCtrl::SetDataItemTextFormat(size_t itemIndex, size_t columnIndex, int32_t nTextFormat)
{
    return m_pDataProvider->SetDataItemTextFormat(itemIndex, columnIndex, nTextFormat);
}

int32_t ListCtrl::GetDataItemTextFormat(size_t itemIndex, size_t columnIndex) const
{
    return m_pDataProvider->GetDataItemTextFormat(itemIndex, columnIndex);
}

bool ListCtrl::SetDataItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor)
{
    return m_pDataProvider->SetDataItemBkColor(itemIndex, columnIndex, bkColor);
}

UiColor ListCtrl::GetDataItemBkColor(size_t itemIndex, size_t columnIndex) const
{
    UiColor bkColor;
    m_pDataProvider->GetDataItemBkColor(itemIndex, columnIndex, bkColor);
    return bkColor;
}

bool ListCtrl::IsShowCheckBox(size_t itemIndex, size_t columnIndex) const
{
    return m_pDataProvider->IsShowCheckBox(itemIndex, columnIndex);
}

bool ListCtrl::SetShowCheckBox(size_t itemIndex, size_t columnIndex, bool bShowCheckBox)
{
    return m_pDataProvider->SetShowCheckBox(itemIndex, columnIndex, bShowCheckBox);
}

bool ListCtrl::SetCheckBoxCheck(size_t itemIndex, size_t columnIndex, bool bChecked)
{
    return m_pDataProvider->SetCheckBoxCheck(itemIndex, columnIndex, bChecked);
}

bool ListCtrl::IsCheckBoxChecked(size_t itemIndex, size_t columnIndex) const
{
    bool bChecked = false;
    m_pDataProvider->GetCheckBoxCheck(itemIndex, columnIndex, bChecked);
    return bChecked;
}

bool ListCtrl::SetDataItemImageId(size_t itemIndex, size_t columnIndex, int32_t imageId)
{
    return m_pDataProvider->SetDataItemImageId(itemIndex, columnIndex, imageId);
}

int32_t ListCtrl::GetDataItemImageId(size_t itemIndex, size_t columnIndex) const
{
    return m_pDataProvider->GetDataItemImageId(itemIndex, columnIndex);
}

bool ListCtrl::SortDataItems(size_t columnIndex, bool bSortedUp, 
                             ListCtrlDataCompareFunc pfnCompareFunc,
                             void* pUserData)
{
    size_t nColumnId = GetColumnId(columnIndex);
    ASSERT(nColumnId != Box::InvalidIndex);
    if (nColumnId == Box::InvalidIndex) {
        return false;
    }
    return m_pDataProvider->SortDataItems(nColumnId, bSortedUp, pfnCompareFunc, pUserData);
}

void ListCtrl::SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    m_pDataProvider->SetSortCompareFunction(pfnCompareFunc, pUserData);
}

bool ListCtrl::IsMultiSelect() const
{
    return m_bMultiSelect;
}

void ListCtrl::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
    if (m_pDataView != nullptr) {
        m_pDataView->SetMultiSelect(bMultiSelect);
    }
    UpdateHeaderCheckBox();
}

void ListCtrl::SetSelectedDataItems(const std::vector<size_t>& selectedIndexs, bool bClearOthers)
{
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        std::vector<size_t> refreshIndexs;
        m_pDataView->SetSelectedElements(selectedIndexs, bClearOthers, refreshIndexs);
        if (!refreshIndexs.empty()) {
            m_pDataView->RefreshElements(refreshIndexs);
            UpdateHeaderCheckBox();
        }
    }
}

void ListCtrl::GetSelectedDataItems(std::vector<size_t>& itemIndexs) const
{
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->GetSelectedElements(itemIndexs);
    }
}

void ListCtrl::SetCheckedDataItems(const std::vector<size_t>& itemIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    m_pDataProvider->SetCheckedDataItems(itemIndexs, bClearOthers, refreshIndexs);
    if (!refreshIndexs.empty() && (m_pDataView != nullptr)) {
        m_pDataView->RefreshElements(refreshIndexs);
    }
    if (!refreshIndexs.empty()) {
        UpdateHeaderCheckBox();
    }
}

void ListCtrl::GetCheckedDataItems(std::vector<size_t>& itemIndexs) const
{
    m_pDataProvider->GetCheckedDataItems(itemIndexs);
}

void ListCtrl::SetSelectAll()
{
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->SetSelectAll();
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }
    }
}

void ListCtrl::SetSelectNone()
{
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->SetSelectNone();
        if (IsAutoCheckSelect()) {
            UpdateHeaderCheckBox();
        }        
    }
}

void ListCtrl::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList.clear();
    if (m_pDataView != nullptr) {
        m_pDataView->GetDisplayDataItems(itemIndexList);
    }
}

size_t ListCtrl::GetTopDataItem() const
{
    size_t nTopItemIndex = Box::InvalidIndex;
    if (m_pDataView != nullptr) {
        nTopItemIndex = m_pDataView->GetTopElementIndex();
    }
    return nTopItemIndex;
}

bool ListCtrl::IsDataItemDisplay(size_t itemIndex) const
{
    bool bItemVisible = false;
    if (m_pDataView != nullptr) {
        bItemVisible = m_pDataView->IsDataItemDisplay(itemIndex);
    }
    return bItemVisible;
}

bool ListCtrl::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    bool bRet = false;
    if (m_pDataView != nullptr) {
        bRet = m_pDataView->EnsureDataItemVisible(itemIndex, bToTop);
    }
    return bRet;
}

void ListCtrl::Refresh()
{
    if (m_bEnableRefresh) {
        if (m_pDataView != nullptr) {
            m_pDataView->Refresh();
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

void ListCtrl::SetAutoCheckSelect(bool bAutoCheckSelect)
{
    m_bAutoCheckSelect = bAutoCheckSelect;
}

bool ListCtrl::IsAutoCheckSelect() const
{
    return m_bAutoCheckSelect;
}

bool ListCtrl::SetHeaderShowCheckBox(bool bShow)
{
    m_bHeaderShowCheckBox = bShow;
    bool bRet = false;
    if (m_bInited) {
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
    if (m_bInited) {
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
        if (m_bInited) {
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
    bool bRet = m_pDataProvider->SetDataItemChecked(itemIndex, bCheck, bChanged);
    if (bChanged) {
        UpdateHeaderCheckBox();
    }
    return bRet;
}

bool ListCtrl::IsDataItemCheck(size_t itemIndex) const
{
    return m_pDataProvider->IsDataItemChecked(itemIndex);
}

ImageList& ListCtrl::GetImageList()
{
    return m_imageList;
}

const ImageList& ListCtrl::GetImageList() const
{
    return m_imageList;
}

}//namespace ui

