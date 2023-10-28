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
    m_bCanUpdateHeaderCheckStatus(true),
    m_bShowHeaderCtrl(true)
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
    else if (strName == L"data_item_label_class") {
        SetDataItemLabelClass(strValue);
    }
    else if (strName == L"row_grid_line_width") {
        SetRowGridLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"column_grid_line_width") {
        SetColumnGridLineWidth(_wtoi(strValue.c_str()), true);
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

void ListCtrl::SetDataItemLabelClass(const std::wstring& className)
{
    m_dataItemLabelClass = className;
}

std::wstring ListCtrl::GetDataItemLabelClass() const
{
    return m_dataItemLabelClass.c_str();
}

void ListCtrl::SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    m_nRowGridLineWidth = nLineWidth;
}

int32_t ListCtrl::GetRowGridLineWidth() const
{
    return m_nRowGridLineWidth;
}

void ListCtrl::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    m_nColumnGridLineWidth = nLineWidth;
}

int32_t ListCtrl::GetColumnGridLineWidth() const
{
    return m_nColumnGridLineWidth;
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
    m_pHeaderCtrl->SetFixedHeight(UiFixedInt(m_nHeaderHeight), true, false);
    if (!m_bShowHeaderCtrl) {
        SetHeaderVisible(false);
    }
    m_pDataProvider->SetListCtrl(this);

    //初始化Body
    ASSERT(m_pDataView == nullptr);
    m_pDataView = new ListCtrlDataView;
    m_pDataView->SetListCtrl(this);
    m_pDataView->SetDataProvider(m_pDataProvider);
    if (!m_dataViewClass.empty()) {
        m_pDataView->SetClass(m_dataViewClass.c_str());
    }
    AddItem(m_pDataView);

    // Header添加到数据视图中管理，作为第一个元素，在Layout的实现中控制显示属性
    m_pDataView->AddItem(m_pHeaderCtrl);

    //TEST
    const size_t columnCount = 10;
    const size_t rowCount = 200;
    //添加列
    for (size_t i = 0; i < columnCount; ++i) {
        ListCtrlColumn columnInfo;
        columnInfo.nColumnWidth = 200;
        //columnInfo.nTextFormat = TEXT_LEFT | TEXT_VCENTER;
        columnInfo.text = StringHelper::Printf(L"第 %d 列", i);
        m_pHeaderCtrl->InsertColumn(-1, columnInfo);
    }
    //填充数据
    SetDataItemCount(rowCount);
    for (size_t itemIndex = 0; itemIndex < rowCount; ++itemIndex) {
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            SetDataItem(itemIndex, { columnIndex, StringHelper::Printf(L"第 %02d 行/第 %02d 列", itemIndex, columnIndex), });
        }
    }
    //排序，默认为升序
    SortDataItems(0, true);
    //TESTs
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

ListCtrlHeader* ListCtrl::GetListCtrlHeader() const
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
    if (nItemHeight < 0) {
        nItemHeight = 0;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nItemHeight);
    }
    m_nItemHeight = nItemHeight;
}

int32_t ListCtrl::GetDataItemHeight() const
{
    return m_nItemHeight;
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
                LabelBox* pLabelBox = dynamic_cast<LabelBox*>(pItem->GetItemAt(nColumn));
                if (pLabelBox != nullptr) {
                    pLabelBox->SetFixedWidth(UiFixedInt(nColumnWidth1), true, false);
                }
            }
            if ((nColumn == nColumn2) && (nColumnWidth2 >= 0)) {
                LabelBox* pLabelBox = dynamic_cast<LabelBox*>(pItem->GetItemAt(nColumn));
                if (pLabelBox != nullptr) {
                    pLabelBox->SetFixedWidth(UiFixedInt(nColumnWidth2), true, false);
                }
            }
        }
    }
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

    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->Refresh();
    }
}

void ListCtrl::OnHeaderColumnOrderChanged()
{
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->Refresh();
    }
}

void ListCtrl::OnHeaderColumnCheckStateChanged(size_t nColumnId, bool bChecked)
{
    m_bCanUpdateHeaderCheckStatus = false;
    m_pDataProvider->SetColumnCheck(nColumnId, bChecked);
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView != nullptr) {
        m_pDataView->Refresh();
    }
    m_bCanUpdateHeaderCheckStatus = true;
}

void ListCtrl::UpdateControlCheckStatus(size_t nColumnId)
{
    if (!m_bCanUpdateHeaderCheckStatus) {
        //避免不必要的更新
        return;
    }
    ASSERT(m_pHeaderCtrl != nullptr);
    if (m_pHeaderCtrl == nullptr) {
        return;
    }
    ASSERT(m_pDataView != nullptr);
    if (m_pDataView == nullptr) {
        return;
    }
    std::vector<size_t> columnIndexList;
    if (nColumnId == Box::InvalidIndex) {
        const size_t columnCount = GetColumnCount();
        for (size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
                columnIndexList.push_back(columnIndex);
            }
        }
    }
    else {
        size_t columnIndex = GetColumnIndex(nColumnId);
        if (columnIndex != Box::InvalidIndex) {
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())){
                columnIndexList.push_back(columnIndex);
            }
        }
    }
    for (size_t columnIndex : columnIndexList) {        
        std::vector<bool> checkList;
        const size_t itemCount = m_pDataView->GetItemCount();
        for (size_t itemIndex = 1; itemIndex < itemCount; ++itemIndex) {
            ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(m_pDataView->GetItemAt(itemIndex));
            if ((pItem != nullptr) && pItem->IsVisible()) {
                const size_t columnCount = pItem->GetItemCount();
                LabelBox* pLabelBox = nullptr;
                if (columnIndex < columnCount) {
                    pLabelBox = dynamic_cast<LabelBox*>(pItem->GetItemAt(columnIndex));
                }
                CheckBox* pCheckBox = nullptr;
                if (pLabelBox != nullptr) {
                    pCheckBox = dynamic_cast<CheckBox*>(pLabelBox->GetItemAt(0));
                }
                if (pCheckBox != nullptr) {
                    checkList.push_back(pCheckBox->IsSelected());
                }
            }
        }

        if (!checkList.empty()) {
            bool bHasChecked = false;
            bool bHasUnChecked = false;
            for (bool bChecked : checkList) {
                if (bChecked) {
                    bHasChecked = true;
                }
                else {
                    bHasUnChecked = true;
                }
            }

            bool bSelected = bHasChecked;
            bool bPartSelect = bSelected && bHasUnChecked;
            ListCtrlHeaderItem* pHeaderItem = m_pHeaderCtrl->GetColumn(columnIndex);
            if ((pHeaderItem != nullptr) && (pHeaderItem->IsCheckBoxVisible())) {
                pHeaderItem->SetCheckBoxSelect(bSelected, bPartSelect);
            }
        }
    }
}

size_t ListCtrl::GetDataItemCount() const
{
    return m_pDataProvider->GetDataItemCount();
}

bool ListCtrl::SetDataItemCount(size_t itemCount)
{
    return m_pDataProvider->SetDataItemCount(itemCount);
}

size_t ListCtrl::AddDataItem(const ListCtrlDataItem& dataItem)
{
    return m_pDataProvider->AddDataItem(dataItem);
}

bool ListCtrl::InsertDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem)
{
    return m_pDataProvider->InsertDataItem(itemIndex, dataItem);
}

bool ListCtrl::SetDataItem(size_t itemIndex, const ListCtrlDataItem& dataItem)
{
    return m_pDataProvider->SetDataItem(itemIndex, dataItem);
}

bool ListCtrl::DeleteDataItem(size_t itemIndex)
{
    return m_pDataProvider->DeleteDataItem(itemIndex);
}

bool ListCtrl::DeleteAllDataItems()
{
    return m_pDataProvider->DeleteAllDataItems();
}

bool ListCtrl::SetDataItemData(size_t itemIndex, size_t itemData)
{
    return m_pDataProvider->SetDataItemData(itemIndex, itemData);
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

bool ListCtrl::GetDataItemTextColor(size_t itemIndex, size_t columnIndex, UiColor& textColor) const
{
    return m_pDataProvider->GetDataItemTextColor(itemIndex, columnIndex, textColor);
}

bool ListCtrl::SetDataItemBkColor(size_t itemIndex, size_t columnIndex, const UiColor& bkColor)
{
    return m_pDataProvider->SetDataItemBkColor(itemIndex, columnIndex, bkColor);
}

bool ListCtrl::GetDataItemBkColor(size_t itemIndex, size_t columnIndex, UiColor& bkColor) const
{
    return m_pDataProvider->GetDataItemBkColor(itemIndex, columnIndex, bkColor);
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

}//namespace ui

