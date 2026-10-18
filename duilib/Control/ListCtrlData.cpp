#include "ListCtrlData.h"
#include "duilib/Control/ListCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include <set>
#include <algorithm>

namespace ui
{
ListCtrlData::ListCtrlData() :
    m_pListView(nullptr),
    m_pfnCompareFunc(nullptr),
    m_pUserData(nullptr),
    m_hideRowCount(0),
    m_heightRowCount(0),
    m_atTopRowCount(0),
    m_bMultiSelect(true),
    m_nSelectedRowId(kInvalidRowId),
    m_nextRowId(0),
    m_nDefaultTextStyle(0),
    m_nDefaultItemHeight(-1),
    m_bAutoCheckSelect(false)
{
}

Control* ListCtrlData::CreateElement(ui::VirtualListBox* pVirtualListBox)
{
    ASSERT(pVirtualListBox != nullptr);
    if (pVirtualListBox == nullptr) {
        return nullptr;
    }
    ASSERT(pVirtualListBox->GetWindow() != nullptr);
    Control* pItem = nullptr;
    ASSERT(m_pListView != nullptr);
    if (m_pListView != nullptr) {
        pItem = m_pListView->CreateDataItem();
    }
    return pItem;
}

bool ListCtrlData::FillElement(Control* pControl, size_t nElementIndex)
{
    ASSERT(pControl != nullptr);
    ASSERT(nElementIndex < m_displayOrder.size());
    if ((nElementIndex >= m_displayOrder.size()) || (pControl == nullptr)) {
        return false;
    }
    const RowId rowId = m_displayOrder[nElementIndex];
    ASSERT(rowId < m_rowDataList.size());
    if (rowId >= m_rowDataList.size()) {
        return false;
    }
    const ListCtrlItemData& itemData = m_rowDataList[rowId];
    //复用成员缓冲区，避免每次填充行数据都重新分配临时容器
    std::vector<ListCtrlSubItemData2Pair>& subItemList = m_subItemListBuffer;
    if (!GetSubItemStorageList(nElementIndex, subItemList)) {//内部会对subItemList做clear操作
        return false;
    }

    bool bRet = false;
    ASSERT(m_pListView != nullptr);
    if (m_pListView != nullptr) {
        bRet = m_pListView->FillDataItem(pControl, nElementIndex, itemData, subItemList);
    }
    return bRet;
}

int32_t ListCtrlData::GetMaxColumnWidth(size_t columnId) const
{
    int32_t nMaxWidth = -1;
    std::vector<const ListCtrlSubItemData2*> subItemList;
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        const StoragePageList& pageList = iter->second;
        const size_t nPageCount = pageList.size();
        for (size_t nPage = 0; nPage < nPageCount; ++nPage) {
            const StoragePage* pPage = pageList[nPage].get();
            if (pPage == nullptr) {
                continue;
            }
            for (size_t nSlot = 0; nSlot < kPageCells; ++nSlot) {
                if (pPage->usedMask[nSlot] != 0) {
                    subItemList.push_back(&pPage->cells[nSlot]);
                }
            }
        }
    }
    if (!subItemList.empty()) {
        ASSERT(m_pListView != nullptr);
        if (m_pListView != nullptr) {
            nMaxWidth = m_pListView->GetMaxDataItemWidth(subItemList);
        }
    }    
    return nMaxWidth;
}

size_t ListCtrlData::GetElementCount() const
{
    return GetDataItemCount();
}

void ListCtrlData::SetElementSelected(size_t nElementIndex, bool bSelected)
{
    if (nElementIndex == Box::InvalidIndex) {
        //如果选中的是Header，忽略
        return;
    }
    const RowId rowId = GetRowIdByDisplayIndex(nElementIndex);
    if (rowId == kInvalidRowId) {
        return;
    }
    ListCtrlItemData& rowData = m_rowDataList[rowId];
    if (rowData.bSelected != bSelected) {
        rowData.bSelected = bSelected;//多选或者单选的情况下，都更新
    }
    if (IsAutoCheckSelect() && (rowData.bChecked != rowData.bSelected)) {
        rowData.bChecked = rowData.bSelected;
    }

    if (!m_bMultiSelect) {
        //单选的情况（记录行身份，排序/增删后仍指向同一行）
        if (bSelected) {
            m_nSelectedRowId = rowId;
        }
        else if (m_nSelectedRowId == rowId) {
            m_nSelectedRowId = kInvalidRowId;
        }
    }
}

bool ListCtrlData::IsElementSelected(size_t nElementIndex) const
{
    if (nElementIndex == Box::InvalidIndex) {
        //如果选中的是Header，忽略
        return false;
    }
    bool bSelected = false;
    if (m_bMultiSelect) {
        //多选
        const RowId rowId = GetRowIdByDisplayIndex(nElementIndex);
        if (rowId != kInvalidRowId) {
            const ListCtrlItemData& rowData = m_rowDataList[rowId];
            bSelected = rowData.bSelected;
        }
    }
    else {
        //单选
        const RowId rowId = GetRowIdByDisplayIndex(nElementIndex);
        bSelected = (rowId != kInvalidRowId) && (m_nSelectedRowId == rowId);
    }
    return bSelected;
}

void ListCtrlData::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    if (m_bMultiSelect) {
        const size_t nCount = m_displayOrder.size();
        for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
            const ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[nElementIndex]];
            if (rowData.bSelected) {
                selectedIndexs.push_back(nElementIndex);
            }
        }
    }
    else {
        const size_t nDisplayIndex = GetDisplayIndexByRowId(m_nSelectedRowId);
        if (nDisplayIndex != Box::InvalidIndex) {
            selectedIndexs.push_back(nDisplayIndex);
        }
    }
}

bool ListCtrlData::IsMultiSelect() const
{
    return m_bMultiSelect;
}

void ListCtrlData::SetMultiSelect(bool bMultiSelect)
{
    bool bChanged = m_bMultiSelect != bMultiSelect;
    m_bMultiSelect = bMultiSelect;
    if (bChanged && bMultiSelect) {
        //从单选变多选，需要清空选项，只保留一个单选项
        const size_t nItemCount = m_displayOrder.size();
        for (size_t itemIndex = 0; itemIndex < nItemCount; ++itemIndex) {
            const RowId rowId = m_displayOrder[itemIndex];
            ListCtrlItemData& rowData = m_rowDataList[rowId];
            if (rowData.bSelected) {
                if (m_nSelectedRowId != rowId) {
                    rowData.bSelected = false;
                }
            }
            if (IsAutoCheckSelect() && (rowData.bChecked != rowData.bSelected)) {
                rowData.bChecked = rowData.bSelected;
            }
        }
    }    
}

void ListCtrlData::SetListView(IListCtrlView* pListView)
{
    m_pListView = pListView;
}

void ListCtrlData::SetAutoCheckSelect(bool bAutoCheckSelect)
{
    m_bAutoCheckSelect = bAutoCheckSelect;
}

bool ListCtrlData::IsAutoCheckSelect() const
{
    return m_bAutoCheckSelect;
}

void ListCtrlData::SetDefaultTextStyle(int32_t nTextStyle)
{
    m_nDefaultTextStyle = nTextStyle;
}

void ListCtrlData::SetDefaultItemHeight(int32_t nItemHeight)
{
    m_nDefaultItemHeight = nItemHeight;
}

void ListCtrlData::ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale)
{
    for (ListCtrlItemData& data : m_rowDataList) {
        if (data.nItemHeight > 0) {
            data.nItemHeight = ui::TruncateToUInt16(dpiManager.GetScaleInt((int32_t)data.nItemHeight, nOldDpiScale));
        }
    }
}

void ListCtrlData::SubItemToStorage(const ListCtrlSubItemData& item, Storage& storage) const
{
    storage.text = item.text;
    if (item.nTextFormat >= 0) {
        storage.nTextFormat = TruncateToUInt16(item.nTextFormat);
    }
    else {
        storage.nTextFormat = 0;
    }
    storage.nImageId = item.nImageId;
    storage.textColor = item.textColor;
    storage.bkColor = item.bkColor;
    storage.bShowCheckBox = item.bShowCheckBox;
    storage.bChecked = item.bChecked;
    storage.bEditable = item.bEditable;

    storage.userDataN = item.userDataN;
    storage.userDataS = item.userDataS;
    storage.nSortGroup = item.nSortGroup;
    storage.nProgress = static_cast<int8_t>(item.nProgress);
    storage.bRichText = item.bRichText;
}

void ListCtrlData::StorageToSubItem(const Storage& storage, ListCtrlSubItemData& item) const
{
    item.text = storage.text.c_str();
    if (storage.nTextFormat == 0) {
        item.nTextFormat = -1;
    }
    else {
        item.nTextFormat = storage.nTextFormat;
    }
    item.nImageId = storage.nImageId;
    item.textColor = storage.textColor;
    item.bkColor = storage.bkColor;
    item.bShowCheckBox = storage.bShowCheckBox;
    item.bChecked = storage.bChecked;
    item.bEditable = storage.bEditable;

    item.userDataN = storage.userDataN;
    item.userDataS = storage.userDataS;
    item.nSortGroup = storage.nSortGroup;
    item.nProgress = static_cast<int32_t>(storage.nProgress);
    item.bRichText = storage.bRichText;
}

bool ListCtrlData::IsValidDataItemIndex(size_t itemIndex) const
{
    return itemIndex < m_displayOrder.size();
}

bool ListCtrlData::IsValidDataColumnId(size_t nColumnId) const
{
    return m_dataMap.find(nColumnId) != m_dataMap.end();
}

bool ListCtrlData::AddColumn(size_t columnId)
{
    ASSERT((columnId != Box::InvalidIndex) && (columnId != 0));
    if ((columnId == Box::InvalidIndex) || (columnId == 0)) {
        return false;
    }
    //新建该列的存储页列表（页为懒分配：某页首次写入时才创建，此处不预分配）
    if (m_dataMap.find(columnId) == m_dataMap.end()) {
        m_dataMap.emplace(columnId, StoragePageList());
    }
    EmitCountChanged();
    return true;
}

bool ListCtrlData::RemoveColumn(size_t columnId)
{
    auto iter = m_dataMap.find(columnId);
    if (iter != m_dataMap.end()) {
        m_dataMap.erase(iter);
        //该列的单元格选中位图一并丢弃，避免同ID列重新添加时继承陈旧选择状态
        m_cellSelected.erase(columnId);
        if (m_dataMap.empty()) {
            //如果所有列都删除了，行也清空为0
            m_displayOrder.clear();
            m_rowDataList.clear();
            m_nextRowId = 0;
            m_nSelectedRowId = kInvalidRowId;
            //行身份已全部失效，单元格选择状态一并清空
            ClearCellSelection();
            m_hideRowCount = 0;
            m_heightRowCount = 0;
            m_atTopRowCount = 0;
        }
        EmitCountChanged();
        return true;
    }
    return false;
}

bool ListCtrlData::SetColumnCheck(size_t columnId, bool bChecked, bool bRefresh)
{
    bool bRet = false;
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        StoragePageList& pageList = iter->second;
        //仅遍历存活的行(按显示顺序)，避免为已删除的行(墓碑)重建数据
        const size_t nCount = m_displayOrder.size();
        for (size_t index = 0; index < nCount; ++index) {
            Storage* pStorage = GetStorageAtForWrite(pageList, m_displayOrder[index]);
            if (pStorage != nullptr) {
                pStorage->bChecked = bChecked;
            }
        }
        bRet = true;
    }
    if (bRefresh && bRet) {
        EmitCountChanged();
    }
    return bRet;
}

const ListCtrlData::Storage* ListCtrlData::GetStorageAt(const StoragePageList& pageList, RowId rowId)
{
    const size_t nPage = static_cast<size_t>(rowId) / kPageCells;
    if (nPage >= pageList.size()) {
        //该页尚未分配（懒分配），视为无数据
        return nullptr;
    }
    const StoragePage* pPage = pageList[nPage].get();
    if (pPage == nullptr) {
        return nullptr;
    }
    const size_t nSlot = static_cast<size_t>(rowId) % kPageCells;
    if (pPage->usedMask[nSlot] == 0) {
        //该单元格未写入（墓碑），视为无数据
        return nullptr;
    }
    return &pPage->cells[nSlot];
}

ListCtrlData::Storage* ListCtrlData::GetStorageAtForWrite(StoragePageList& pageList, RowId rowId)
{
    const size_t nPage = static_cast<size_t>(rowId) / kPageCells;
    const size_t nSlot = static_cast<size_t>(rowId) % kPageCells;
    if (nPage >= pageList.size()) {
        //懒分配：仅在首次写入该页时增长页列表（中间未触及的页保持为空指针）
        pageList.resize(nPage + 1);
    }
    if (pageList[nPage] == nullptr) {
        //懒分配：创建存储页（页内单元格一次性构造，页内连续）
        pageList[nPage] = std::make_unique<StoragePage>();
    }
    StoragePage* pPage = pageList[nPage].get();
    pPage->usedMask[nSlot] = 1;
    return &pPage->cells[nSlot];
}

void ListCtrlData::ResetStorageAt(StoragePageList& pageList, RowId rowId)
{
    const size_t nPage = static_cast<size_t>(rowId) / kPageCells;
    if (nPage >= pageList.size()) {
        return;
    }
    StoragePage* pPage = pageList[nPage].get();
    if (pPage == nullptr) {
        return;
    }
    const size_t nSlot = static_cast<size_t>(rowId) % kPageCells;
    if (pPage->usedMask[nSlot] != 0) {
        //置墓碑：清空数据并标记为未写入
        pPage->cells[nSlot] = Storage();
        pPage->usedMask[nSlot] = 0;
    }
    //页回收：该页已全部为墓碑（无任何已写入单元格）时释放该页，并裁剪尾部已释放的页以收缩页列表
    for (size_t n = 0; n < kPageCells; ++n) {
        if (pPage->usedMask[n] != 0) {
            return;
        }
    }
    pageList[nPage].reset();
    while (!pageList.empty() && (pageList.back() == nullptr)) {
        pageList.pop_back();
    }
}

const ListCtrlData::Storage* ListCtrlData::GetSubItemStorage(
    size_t itemIndex, size_t nColumnId) const
{
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    if (rowId == kInvalidRowId) {
        return nullptr;
    }
    auto iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter == m_dataMap.end()) {
        return nullptr;
    }
    //关联列：按页/槽读取数据（未写入的单元格返回nullptr）
    return GetStorageAt(iter->second, rowId);
}

ListCtrlData::Storage* ListCtrlData::GetSubItemStorageForWrite(
    size_t itemIndex, size_t nColumnId)
{
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    if (rowId == kInvalidRowId) {
        return nullptr;
    }
    auto iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter == m_dataMap.end()) {
        return nullptr;
    }
    //关联列：按页/槽获取可写单元格（必要时分配存储页）
    return GetStorageAtForWrite(iter->second, rowId);
}

bool ListCtrlData::GetSubItemStorageList(size_t itemIndex, std::vector<ListCtrlSubItemData2Pair>& subItemList) const
{
    subItemList.clear();
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId == kInvalidRowId) {
        return false;
    }
    ListCtrlSubItemData2Pair dataPair;
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        dataPair.nColumnId = iter->first;        
        dataPair.pSubItemData = GetStorageAt(iter->second, rowId);
        subItemList.push_back(dataPair);
    }
    return true;
}

const ListCtrlItemData* ListCtrlData::GetDisplayItemData(size_t itemIndex) const
{
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    if (rowId == kInvalidRowId) {
        return nullptr;
    }
    return &m_rowDataList[rowId];
}

ListCtrlData::RowId ListCtrlData::GetRowIdByDisplayIndex(size_t itemIndex) const
{
    if (itemIndex == Box::InvalidIndex) {
        return kInvalidRowId;
    }
    if (itemIndex >= m_displayOrder.size()) {
        return kInvalidRowId;
    }
    return m_displayOrder[itemIndex];
}

size_t ListCtrlData::GetDisplayIndexByRowId(RowId rowId) const
{
    if (rowId == kInvalidRowId) {
        return Box::InvalidIndex;
    }
    const size_t nCount = m_displayOrder.size();
    for (size_t index = 0; index < nCount; ++index) {
        if (m_displayOrder[index] == rowId) {
            return index;
        }
    }
    return Box::InvalidIndex;
}

ListCtrlData::RowId ListCtrlData::AllocRowId()
{
    const RowId rowId = m_nextRowId;
    ++m_nextRowId;
    //仅行属性池增长到与行身份数量一致；列数据池为分页懒分配，无需在此增长
    if (m_rowDataList.size() < m_nextRowId) {
        m_rowDataList.resize(m_nextRowId);
    }
    //单元格选中位图按行身份索引，同步增长（新行默认为未选中）
    for (auto iter = m_cellSelected.begin(); iter != m_cellSelected.end(); ++iter) {
        std::vector<uint8_t>& bitmap = iter->second;
        if (bitmap.size() < m_nextRowId) {
            bitmap.resize(m_nextRowId, static_cast<uint8_t>(0));
        }
    }
    return rowId;
}

void ListCtrlData::FreeRowId(RowId rowId)
{
    if (rowId >= m_rowDataList.size()) {
        return;
    }
    //修正个性化数据的计数
    const ListCtrlItemData oldData = m_rowDataList[rowId];
    if (!oldData.bVisible) {
        m_hideRowCount -= 1;
        ASSERT(m_hideRowCount >= 0);
    }
    if (oldData.nItemHeight >= 0) {
        m_heightRowCount -= 1;
        ASSERT(m_heightRowCount >= 0);
    }
    if (oldData.nAlwaysAtTop >= 0) {
        m_atTopRowCount -= 1;
        ASSERT(m_atTopRowCount >= 0);
    }
    //行属性置墓碑
    m_rowDataList[rowId] = ListCtrlItemData();
    //各列数据置墓碑（清空该单元格并标记为未写入；存储页本身不释放）
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        ResetStorageAt(iter->second, rowId);
    }
    //修正单选状态
    if (m_nSelectedRowId == rowId) {
        m_nSelectedRowId = kInvalidRowId;
    }
    //单元格选择：清除被删除行的选中位；若锚点/当前格所在行被删除，则锚点/当前格失效
    for (auto iter = m_cellSelected.begin(); iter != m_cellSelected.end(); ++iter) {
        std::vector<uint8_t>& bitmap = iter->second;
        if (rowId < bitmap.size()) {
            bitmap[rowId] = 0;
        }
    }
    if (m_anchorCell.IsValid() && (m_anchorCell.nRowId == rowId)) {
        m_anchorCell = CellIdentity();
    }
    if (m_currentCell.IsValid() && (m_currentCell.nRowId == rowId)) {
        m_currentCell = CellIdentity();
    }
}

void ListCtrlData::AssertDataConsistency() const
{
#ifdef _DEBUG
    //1. 显示顺序表内行身份均合法、无重复，且行属性可达
    std::set<RowId> rowIdSet;
    for (RowId rowId : m_displayOrder) {
        ASSERT(rowId < m_nextRowId);
        ASSERT(rowId < m_rowDataList.size());
        ASSERT(rowIdSet.find(rowId) == rowIdSet.end());
        rowIdSet.insert(rowId);
    }
    //2. 各列分页存储中，已写入的单元格必须属于存活的行身份；且页索引与页内偏移换算一致
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        const StoragePageList& pageList = iter->second;
        const size_t nPageCount = pageList.size();
        for (size_t nPage = 0; nPage < nPageCount; ++nPage) {
            const StoragePage* pPage = pageList[nPage].get();
            if (pPage == nullptr) {
                continue;
            }
            for (size_t nSlot = 0; nSlot < kPageCells; ++nSlot) {
                if (pPage->usedMask[nSlot] != 0) {
                    const RowId rowId = static_cast<RowId>(nPage * kPageCells + nSlot);
                    ASSERT(rowId < m_nextRowId);
                    ASSERT(rowIdSet.find(rowId) != rowIdSet.end());
                }
            }
        }
    }
    //3. 行属性池长度与已分配行身份数量一致
    ASSERT(m_rowDataList.size() == m_nextRowId);
    //4. 单选状态有效时，其行身份必须存在于显示顺序表中
    if (!m_bMultiSelect && (m_nSelectedRowId != kInvalidRowId)) {
        ASSERT(rowIdSet.find(m_nSelectedRowId) != rowIdSet.end());
    }
    //5. 个性化数据计数与按存活行扫描的结果一致
    int32_t nHideCount = 0;
    int32_t nHeightCount = 0;
    int32_t nAtTopCount = 0;
    for (RowId rowId : rowIdSet) {
        const ListCtrlItemData& rowData = m_rowDataList[rowId];
        if (!rowData.bVisible) {
            ++nHideCount;
        }
        if (rowData.nItemHeight >= 0) {
            ++nHeightCount;
        }
        if (rowData.nAlwaysAtTop >= 0) {
            ++nAtTopCount;
        }
    }
    ASSERT(nHideCount == m_hideRowCount);
    ASSERT(nHeightCount == m_heightRowCount);
    ASSERT(nAtTopCount == m_atTopRowCount);
#endif
}

bool ListCtrlData::IsNormalMode() const
{
    ASSERT((m_hideRowCount >= 0) && (m_heightRowCount >= 0) && (m_atTopRowCount >= 0));
    return (m_hideRowCount == 0) && (m_heightRowCount == 0) && (m_atTopRowCount == 0);
}

const std::vector<uint8_t>* ListCtrlData::FindCellBitmap(size_t nColumnId) const
{
    auto iter = m_cellSelected.find(nColumnId);
    if (iter == m_cellSelected.end()) {
        return nullptr;
    }
    return &(iter->second);
}

std::vector<uint8_t>& ListCtrlData::GetCellBitmapForWrite(size_t nColumnId, RowId rowId)
{
    std::vector<uint8_t>& bitmap = m_cellSelected[nColumnId];
    const size_t nNeed = static_cast<size_t>(rowId) + 1;
    if (bitmap.size() < nNeed) {
        bitmap.resize(nNeed, static_cast<uint8_t>(0));
    }
    return bitmap;
}

void ListCtrlData::SetCellSelectedByRowId(RowId rowId, size_t nColumnId, bool bSelected)
{
    if ((rowId == kInvalidRowId) || !IsValidDataColumnId(nColumnId)) {
        return;
    }
    std::vector<uint8_t>& bitmap = GetCellBitmapForWrite(nColumnId, rowId);
    bitmap[rowId] = bSelected ? 1 : 0;
}

bool ListCtrlData::IsCellSelectedByRowId(RowId rowId, size_t nColumnId) const
{
    if (rowId == kInvalidRowId) {
        return false;
    }
    const std::vector<uint8_t>* pBitmap = FindCellBitmap(nColumnId);
    if ((pBitmap == nullptr) || (rowId >= pBitmap->size())) {
        return false;
    }
    return (*pBitmap)[rowId] != 0;
}

ListCtrlCellId ListCtrlData::GetCurrentCell() const
{
    ListCtrlCellId cell;
    if (m_currentCell.IsValid()) {
        const size_t nDisplayIndex = GetDisplayIndexByRowId(m_currentCell.nRowId);
        if (nDisplayIndex != Box::InvalidIndex) {
            cell.nRowIndex = nDisplayIndex;
            cell.nColumnId = m_currentCell.nColumnId;
        }
    }
    return cell;
}

ListCtrlCellId ListCtrlData::GetAnchorCell() const
{
    ListCtrlCellId cell;
    if (m_anchorCell.IsValid()) {
        const size_t nDisplayIndex = GetDisplayIndexByRowId(m_anchorCell.nRowId);
        if (nDisplayIndex != Box::InvalidIndex) {
            cell.nRowIndex = nDisplayIndex;
            cell.nColumnId = m_anchorCell.nColumnId;
        }
    }
    return cell;
}

bool ListCtrlData::SetCurrentCell(const ListCtrlCellId& cell)
{
    if (!IsValidDataItemIndex(cell.nRowIndex) || !IsValidDataColumnId(cell.nColumnId)) {
        return false;
    }
    //把"显示索引"换算为"行身份"存储，使选择状态与显示顺序解耦（排序/增删后仍跟随原数据行）
    const RowId rowId = GetRowIdByDisplayIndex(cell.nRowIndex);
    if (rowId == kInvalidRowId) {
        return false;
    }
    m_currentCell.nRowId = rowId;
    m_currentCell.nColumnId = cell.nColumnId;
    return true;
}

bool ListCtrlData::SetAnchorCell(const ListCtrlCellId& cell)
{
    if (!IsValidDataItemIndex(cell.nRowIndex) || !IsValidDataColumnId(cell.nColumnId)) {
        return false;
    }
    const RowId rowId = GetRowIdByDisplayIndex(cell.nRowIndex);
    if (rowId == kInvalidRowId) {
        return false;
    }
    m_anchorCell.nRowId = rowId;
    m_anchorCell.nColumnId = cell.nColumnId;
    return true;
}

void ListCtrlData::GetCellSelection(ListCtrlCellId& anchorCell, ListCtrlCellId& focusCell) const
{
    anchorCell = GetAnchorCell();
    focusCell = GetCurrentCell();
}

bool ListCtrlData::IsCellSelected(size_t nRowIndex, size_t nColumnId) const
{
    return IsCellSelectedByRowId(GetRowIdByDisplayIndex(nRowIndex), nColumnId);
}

bool ListCtrlData::SetCellSelected(size_t nRowIndex, size_t nColumnId, bool bSelected)
{
    if (!IsValidDataItemIndex(nRowIndex) || !IsValidDataColumnId(nColumnId)) {
        return false;
    }
    if (!IsSelectableElement(nRowIndex)) {
        //置顶行、不可见行不参与单元格选择
        return false;
    }
    const RowId rowId = GetRowIdByDisplayIndex(nRowIndex);
    if (rowId == kInvalidRowId) {
        return false;
    }
    if (IsCellSelectedByRowId(rowId, nColumnId) == bSelected) {
        //选中状态未发生变化：返回false，避免调用方重复刷新与派发选择变化事件
        return false;
    }
    SetCellSelectedByRowId(rowId, nColumnId, bSelected);
    return true;
}

void ListCtrlData::SetCellSelectionRect(size_t nRowBegin, size_t nRowEnd,
                                        const std::vector<size_t>& columnIdList, bool bAdditive)
{
    if (!bAdditive) {
        //替换模式：仅清除选中集合，锚点/当前格由调用方维护（拖动/区域扩展过程中锚点需要保持不变）
        m_cellSelected.clear();
    }
    const size_t nCount = m_displayOrder.size();
    if ((nCount == 0) || columnIdList.empty() ||
        (nRowBegin == Box::InvalidIndex) || (nRowEnd == Box::InvalidIndex)) {
        return;
    }
    if (nRowBegin > nRowEnd) {
        const size_t nTemp = nRowBegin;
        nRowBegin = nRowEnd;
        nRowEnd = nTemp;
    }
    if (nRowEnd >= nCount) {
        nRowEnd = nCount - 1;
    }
    //按行身份收集选中单元格：排序/增删后选择状态仍跟随原数据行
    for (size_t nRowIndex = nRowBegin; nRowIndex <= nRowEnd; ++nRowIndex) {
        if (!IsSelectableElement(nRowIndex)) {
            //置顶行、不可见行不参与单元格选择
            continue;
        }
        const RowId rowId = m_displayOrder[nRowIndex];
        for (size_t nColumnId : columnIdList) {
            SetCellSelectedByRowId(rowId, nColumnId, true);
        }
    }
}

void ListCtrlData::SelectAllCells(const std::vector<size_t>& columnIdList)
{
    const size_t nCount = m_displayOrder.size();
    if ((nCount == 0) || columnIdList.empty()) {
        ClearCellSelection();
        return;
    }
    SetCellSelectionRect(0, nCount - 1, columnIdList, false);
}

bool ListCtrlData::ClearCellSelection()
{
    //判断清除前是否真的存在选择状态：位图可能残留（某格被选中后又取消，位图仍在但内容全为0），
    //故需遍历位图内容确认，不能仅以"位图容器非空"判定
    bool bHasSelectedCell = false;
    for (auto iter = m_cellSelected.begin(); (iter != m_cellSelected.end()) && !bHasSelectedCell; ++iter) {
        const std::vector<uint8_t>& bitmap = iter->second;
        for (size_t nIndex = 0; nIndex < bitmap.size(); ++nIndex) {
            if (bitmap[nIndex] != 0) {
                bHasSelectedCell = true;
                break;
            }
        }
    }
    //锚点仅用于区域扩展、无独立视觉表现，故不单独计为变化；当前格有焦点框，属于状态变化
    const bool bChanged = bHasSelectedCell || m_currentCell.IsValid();

    m_anchorCell = CellIdentity();
    m_currentCell = CellIdentity();
    m_cellSelected.clear();
    return bChanged;
}

size_t ListCtrlData::GetDataItemCount() const
{
    return m_displayOrder.size();
}

bool ListCtrlData::SetDataItemCount(size_t itemCount)
{
    ASSERT(itemCount != Box::InvalidIndex);
    if (itemCount == Box::InvalidIndex) {
        return false;
    }
    const size_t nOldCount = m_displayOrder.size();
    if (itemCount == nOldCount) {
        //没有变化
        return true;
    }
    if (itemCount < nOldCount) {
        //行数变少：释放被裁减掉的行身份（计数在FreeRowId内修正）
        for (size_t index = itemCount; index < nOldCount; ++index) {
            FreeRowId(m_displayOrder[index]);
        }
        m_displayOrder.resize(itemCount);
    }
    else {
        //行数变多：为新增的每个显示位置分配行身份
        m_displayOrder.reserve(itemCount);
        for (size_t index = nOldCount; index < itemCount; ++index) {
            m_displayOrder.push_back(AllocRowId());
        }
    }
    AssertDataConsistency();
    EmitCountChanged();
    return true;
}

void ListCtrlData::UpdateNormalMode()
{
    m_hideRowCount = 0;
    m_heightRowCount = 0;
    m_atTopRowCount = 0;
    const size_t nCount = m_displayOrder.size();
    for (size_t index = 0; index < nCount; ++index) {
        const ListCtrlItemData& data = m_rowDataList[m_displayOrder[index]];
        if (!data.bVisible) {
            m_hideRowCount += 1;
        }
        if (data.nItemHeight >= 0) {
            m_heightRowCount += 1;
        }
        if (data.nAlwaysAtTop >= 0) {
            m_atTopRowCount += 1;
        }
    }
}

size_t ListCtrlData::AddDataItem(size_t columnId, const ListCtrlSubItemData& dataItem)
{
    ASSERT(IsValidDataColumnId(columnId));
    if (!IsValidDataColumnId(columnId)) {
        return Box::InvalidIndex;
    }

    Storage storage;
    SubItemToStorage(dataItem, storage);

    //分配新的行身份（行属性池随之增长，列数据本体不搬移）
    const RowId rowId = AllocRowId();

    //关联列：写入数据（其他列为空）
    auto iter = m_dataMap.find(columnId);
    if (iter != m_dataMap.end()) {
        Storage* pStorage = GetStorageAtForWrite(iter->second, rowId);
        if (pStorage != nullptr) {
            *pStorage = storage;
        }
    }

    //追加到显示顺序表的末尾
    m_displayOrder.push_back(rowId);
    AssertDataConsistency();
    EmitCountChanged();
    return (m_displayOrder.size() - 1);
}

bool ListCtrlData::InsertDataItem(size_t itemIndex, size_t columnId, const ListCtrlSubItemData& dataItem)
{
    ASSERT(IsValidDataColumnId(columnId));
    if (!IsValidDataColumnId(columnId)) {
        return false;
    }
    if (!IsValidDataItemIndex(itemIndex)) {
        //如果索引号无效，则按追加的方式添加数据
        return (AddDataItem(columnId, dataItem) != Box::InvalidIndex);
    }

    Storage storage;
    SubItemToStorage(dataItem, storage);

    //分配新的行身份（行属性池随之增长，列数据本体不搬移）
    const RowId rowId = AllocRowId();

    //关联列：写入数据
    auto iter = m_dataMap.find(columnId);
    if (iter != m_dataMap.end()) {
        Storage* pStorage = GetStorageAtForWrite(iter->second, rowId);
        if (pStorage != nullptr) {
            *pStorage = storage;
        }
    }

    //仅在显示顺序表中插入新的行身份（单选状态跟随行身份，无需调整）
    ASSERT(itemIndex <= m_displayOrder.size());
    m_displayOrder.insert(m_displayOrder.begin() + itemIndex, rowId);
    AssertDataConsistency();
    EmitCountChanged();
    return true;
}

bool ListCtrlData::DeleteDataItem(size_t itemIndex)
{
    if (!IsValidDataItemIndex(itemIndex)) {
        //索引号无效
        return false;
    }

    //释放该行身份关联的数据（列池置墓碑、修正计数与单选状态），再从显示顺序表中移除
    const RowId rowId = m_displayOrder[itemIndex];
    FreeRowId(rowId);
    m_displayOrder.erase(m_displayOrder.begin() + itemIndex);
    AssertDataConsistency();
    EmitCountChanged();
    return true;
}

bool ListCtrlData::DeleteAllDataItems()
{
    bool bDeleted = !m_displayOrder.empty();
    //清空显示顺序表
    m_displayOrder.clear();
    //清空各列分页存储（释放所有存储页）
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        iter->second.clear();
    }
    //清空行属性池，并重置行身份生成器
    m_rowDataList.clear();
    m_nextRowId = 0;
    m_nSelectedRowId = kInvalidRowId;
    //行身份已全部失效，单元格选择状态一并清空
    ClearCellSelection();
    m_hideRowCount = 0;
    m_heightRowCount = 0;
    m_atTopRowCount = 0;

    if (bDeleted) {
        EmitCountChanged();
    }
    return bDeleted;
}

size_t ListCtrlData::DeleteDataItems(const std::vector<size_t>& displayIndexs)
{
    const size_t nCount = m_displayOrder.size();
    if ((nCount == 0) || displayIndexs.empty()) {
        return 0;
    }
    //按行身份收集待删除的行（去重；越界索引忽略）
    std::set<RowId> rowIdSet;
    for (size_t index : displayIndexs) {
        if (index < nCount) {
            rowIdSet.insert(m_displayOrder[index]);
        }
    }
    if (rowIdSet.empty()) {
        return 0;
    }

    //释放待删除行身份关联的数据（列池置墓碑、修正个性化计数与单选状态）
    for (RowId rowId : rowIdSet) {
        FreeRowId(rowId);
    }

    //单趟压缩显示顺序表：把存活的行身份前移，O(N) 搬 4B（避免逐行 erase 的 O(K×N) 搬移）
    size_t nWriteIndex = 0;
    for (size_t nReadIndex = 0; nReadIndex < nCount; ++nReadIndex) {
        const RowId rowId = m_displayOrder[nReadIndex];
        if (rowIdSet.find(rowId) == rowIdSet.end()) {
            m_displayOrder[nWriteIndex] = rowId;
            ++nWriteIndex;
        }
    }
    m_displayOrder.resize(nWriteIndex);

    AssertDataConsistency();
    EmitCountChanged();
    return rowIdSet.size();
}

void ListCtrlData::CompactStorage()
{
    const size_t nCount = m_displayOrder.size();
    //记录压缩前选中行所在显示位置（行身份被重编号后需要重新映射）
    size_t nSelectedIndex = Box::InvalidIndex;
    if (m_nSelectedRowId != kInvalidRowId) {
        nSelectedIndex = GetDisplayIndexByRowId(m_nSelectedRowId);
    }
    //单元格选择的锚点/当前格也记录显示位置，重编号后按新行身份恢复
    size_t nAnchorIndex = Box::InvalidIndex;
    if (m_anchorCell.IsValid()) {
        nAnchorIndex = GetDisplayIndexByRowId(m_anchorCell.nRowId);
    }
    size_t nCurrentIndex = Box::InvalidIndex;
    if (m_currentCell.IsValid()) {
        nCurrentIndex = GetDisplayIndexByRowId(m_currentCell.nRowId);
    }

    //1. 重建行属性池：新行身份 = 显示位置（按显示顺序紧凑存放，丢弃墓碑槽位）
    RowDataList newRowDataList;
    newRowDataList.resize(nCount);
    for (size_t index = 0; index < nCount; ++index) {
        newRowDataList[index] = m_rowDataList[m_displayOrder[index]];
    }
    m_rowDataList.swap(newRowDataList);

    //2. 重建各列的存储页：按新行身份紧凑排布，仅搬运已写入的单元格（未写入的不分配页）
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        StoragePageList& pageList = iter->second;
        StoragePageList newPageList;
        for (size_t index = 0; index < nCount; ++index) {
            const Storage* pOld = GetStorageAt(pageList, m_displayOrder[index]);
            if (pOld == nullptr) {
                continue;
            }
            Storage* pNew = GetStorageAtForWrite(newPageList, static_cast<RowId>(index));
            *pNew = *pOld;
        }
        //交换后，旧页列表随 newPageList 析构自动释放（回收已删除行占用的内存）
        pageList.swap(newPageList);
    }

    //2b. 重映射单元格选中位图：新行身份 = 显示位置，按显示顺序搬迁选中位
    for (auto iter = m_cellSelected.begin(); iter != m_cellSelected.end(); ++iter) {
        const std::vector<uint8_t>& oldBits = iter->second;
        std::vector<uint8_t> newBits(nCount, static_cast<uint8_t>(0));
        for (size_t index = 0; index < nCount; ++index) {
            const RowId oldRowId = m_displayOrder[index];
            if ((oldRowId < oldBits.size()) && (oldBits[oldRowId] != 0)) {
                newBits[index] = static_cast<uint8_t>(1);
            }
        }
        iter->second.swap(newBits);
    }

    //3. 显示顺序表变为恒等映射；行身份生成器归位
    for (size_t index = 0; index < nCount; ++index) {
        m_displayOrder[index] = static_cast<RowId>(index);
    }
    m_nextRowId = static_cast<RowId>(nCount);

    //4. 恢复选中行身份（显示位置不变，新行身份 = 该显示位置）
    if (nSelectedIndex != Box::InvalidIndex) {
        m_nSelectedRowId = static_cast<RowId>(nSelectedIndex);
    }
    //恢复单元格选择锚点/当前格（行身份被重编号）
    if (nAnchorIndex != Box::InvalidIndex) {
        m_anchorCell.nRowId = static_cast<RowId>(nAnchorIndex);
    }
    else {
        m_anchorCell = CellIdentity();
    }
    if (nCurrentIndex != Box::InvalidIndex) {
        m_currentCell.nRowId = static_cast<RowId>(nCurrentIndex);
    }
    else {
        m_currentCell = CellIdentity();
    }

    AssertDataConsistency();
}

bool ListCtrlData::SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData,
                                   bool& bChanged, bool& bCheckChanged)
{
    bChanged = false;
    bCheckChanged = false;
    bool bCountChanged = false;
    bool bItemChanged = false;
    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        const ListCtrlItemData oldItemData = m_rowDataList[rowId];
        m_rowDataList[rowId] = itemData;
        if (m_nDefaultItemHeight == m_rowDataList[rowId].nItemHeight) {
            //如果等于默认高度，则设置为标志值
            m_rowDataList[rowId].nItemHeight = -1;
        }
        const ListCtrlItemData& newItemData = m_rowDataList[rowId];
        if (newItemData.bSelected != oldItemData.bSelected) {
            bChanged = true;
            bItemChanged = true;
        }
        else if (newItemData.bChecked != oldItemData.bChecked) {
            bCheckChanged = true;
            bChanged = true;
            bItemChanged = true;
        }
        else if (newItemData.bVisible != oldItemData.bVisible) {
            bChanged = true;
            bCountChanged = true;
        }
        else if (newItemData.nAlwaysAtTop != oldItemData.nAlwaysAtTop) {
            bChanged = true;
            bCountChanged = true;
        }
        else if (newItemData.nItemHeight != oldItemData.nItemHeight) {
            bChanged = true;
            bCountChanged = true;
        }
        else if (newItemData.nUserData != oldItemData.nUserData) {
            bChanged = true;
        }

        //更新计数
        if (!oldItemData.bVisible && newItemData.bVisible) {
            m_hideRowCount -= 1;            
        }
        else if (oldItemData.bVisible && !newItemData.bVisible) {
            m_hideRowCount += 1;
        }
        ASSERT(m_hideRowCount >= 0);

        if ((oldItemData.nItemHeight >= 0) && (newItemData.nItemHeight < 0)) {
            m_heightRowCount -= 1;
        }
        else if ((oldItemData.nItemHeight < 0) && (newItemData.nItemHeight >= 0)) {
            m_heightRowCount += 1;
        }
        ASSERT(m_heightRowCount >= 0);

        if ((oldItemData.nAlwaysAtTop >= 0) && (newItemData.nAlwaysAtTop < 0)) {
            m_atTopRowCount -= 1;
        }
        else if ((oldItemData.nAlwaysAtTop < 0) && (newItemData.nAlwaysAtTop >= 0)) {
            m_atTopRowCount += 1;
        }
        ASSERT(m_atTopRowCount >= 0);
        bRet = true;
    }
    AssertDataConsistency();
    if (bCountChanged) {
        EmitCountChanged();
    }
    else if (bItemChanged) {
        EmitDataChanged(itemIndex, itemIndex);
    }
    return bRet;
}

bool ListCtrlData::GetDataItemData(size_t itemIndex, ListCtrlItemData& itemData) const
{
    bool bRet = false;
    itemData = ListCtrlItemData();
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        itemData = *pRowData;
        bRet = true;
    }
    return bRet;
}

bool ListCtrlData::SetDataItemVisible(size_t itemIndex, bool bVisible, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        bool bOldVisible = rowData.bVisible;
        bChanged = rowData.bVisible != bVisible;
        rowData.bVisible = bVisible;

        if (!bOldVisible && bVisible) {
            m_hideRowCount -= 1;
        }
        else if (bOldVisible && !bVisible) {
            m_hideRowCount += 1;
        }
        ASSERT(m_hideRowCount >= 0);
        bRet = true;
    }
    AssertDataConsistency();
    if (bChanged) {
        EmitCountChanged();
    }
    return bRet;
}

bool ListCtrlData::IsDataItemVisible(size_t itemIndex) const
{
    bool bValue = false;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        bValue = pRowData->bVisible;
    }
    return bValue;
}

bool ListCtrlData::SetDataItemSelected(size_t itemIndex, bool bSelected, bool& bChanged)
{
    bChanged = false;
    if (itemIndex >= m_displayOrder.size()) {
        return false;
    }
    bChanged = IsDataItemSelected(itemIndex) != bSelected;
    SetElementSelected(itemIndex, bSelected);
    if (bChanged) {
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

bool ListCtrlData::IsDataItemSelected(size_t itemIndex) const
{
    return IsElementSelected(itemIndex);
}

bool ListCtrlData::SetDataItemChecked(size_t itemIndex, bool bChecked, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        if (rowData.bChecked != bChecked) {
            bChanged = true;
            rowData.bChecked = bChecked;
        }        
        bRet = true;
    }
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

bool ListCtrlData::IsDataItemChecked(size_t itemIndex) const
{
    bool bChecked = false;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        bChecked = pRowData->bChecked;
    }
    return bChecked;
}

bool ListCtrlData::SetAllDataItemsCheck(bool bChecked)
{
    bool bChanged = false;
    size_t nCount = m_displayOrder.size();
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
        if (rowData.bChecked != bChecked) {
            rowData.bChecked = bChecked;
            bChanged = true;
        }
    }
    if (bChanged) {
        EmitCountChanged();
    }
    return bChanged;
}

void ListCtrlData::SetCheckedDataItems(const std::vector<size_t>& itemIndexs,
                                               bool bClearOthers,
                                               std::vector<size_t>& refreshIndexs)
{
    refreshIndexs.clear();
    const size_t nCount = m_displayOrder.size();
    if (!bClearOthers) {
        for (size_t itemIndex : itemIndexs) {
            if (itemIndex < nCount) {
                ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
                if (!rowData.bChecked) {
                    rowData.bChecked = true;
                    refreshIndexs.push_back(itemIndex);
                }
            }
        }
    }
    else {
        std::set<size_t> indexSet;
        for (size_t itemIndex : itemIndexs) {
            if (itemIndex < nCount) {
                indexSet.insert(itemIndex);
            }
        }

        for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
            ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
            if (indexSet.find(itemIndex) != indexSet.end()) {
                if (!rowData.bChecked) {
                    rowData.bChecked = true;
                    refreshIndexs.push_back(itemIndex);
                }
            }
            else if (rowData.bChecked) {
                rowData.bChecked = false;
                refreshIndexs.push_back(itemIndex);
            }
        }
    }
}

void ListCtrlData::GetCheckedDataItems(std::vector<size_t>& itemIndexs) const
{
    itemIndexs.clear();
    const size_t nCount = m_displayOrder.size();
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
        if (rowData.bChecked) {
            itemIndexs.push_back(itemIndex);
        }
    }
}

void ListCtrlData::GetDataItemsCheckStatus(bool& bChecked, bool& bPartChecked) const
{
    bChecked = false;
    bPartChecked = false;
    size_t nCheckCount = 0;
    size_t nUnCheckCount = 0;
    const size_t nCount = m_displayOrder.size();
    if (nCount == 0) {
        return;
    }
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
        if (!rowData.bVisible) {
            continue;
        }
        if (rowData.bChecked) {
            nCheckCount++;
        }
        else {
            nUnCheckCount++;
        }
        if ((nCheckCount > 0) && (nUnCheckCount > 0)){
            //确认是部分选择
            bChecked = true;
            bPartChecked = true;
            return;
        }
    }
    if ((nCheckCount > 0) && (nUnCheckCount == 0)) {
        bChecked = true;
    }
    else if ((nCheckCount == 0) && (nUnCheckCount > 0)) {
        bChecked = false;
    }
    else {
        ASSERT(0);
    }
}

void ListCtrlData::GetDataItemsSelectStatus(bool& bSelected, bool& bPartSelected) const
{
    bSelected = false;
    bPartSelected = false;
    size_t nSelectCount = 0;
    size_t nUnSelectCount = 0;
    const size_t nCount = m_displayOrder.size();
    if (nCount == 0) {
        return;
    }
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[m_displayOrder[itemIndex]];
        if (!rowData.bVisible) {
            continue;
        }
        if (rowData.bSelected) {
            nSelectCount++;
        }
        else {
            nUnSelectCount++;
        }
        if ((nSelectCount > 0) && (nUnSelectCount > 0)) {
            //确认是部分选择
            bSelected = true;
            bPartSelected = true;
            return;
        }
    }
    if ((nSelectCount > 0) && (nUnSelectCount == 0)) {
        bSelected = true;
    }
    else if ((nSelectCount == 0) && (nUnSelectCount > 0)) {
        bSelected = false;
    }
    else {
        ASSERT(0);
    }
}

void ListCtrlData::GetColumnCheckStatus(size_t columnId, bool& bChecked, bool& bPartChecked) const
{
    bChecked = false;
    bPartChecked = false;
    auto iter = m_dataMap.find(columnId);
    if (iter == m_dataMap.end()) {
        return;
    }
    const StoragePageList& pageList = iter->second;
    size_t nCheckCount = 0;
    size_t nUnCheckCount = 0;
    //按显示顺序遍历存活的行
    const size_t nCount = m_displayOrder.size();
    if (nCount == 0) {
        return;
    }

    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const RowId rowId = m_displayOrder[itemIndex];
        const ListCtrlItemData& rowData = m_rowDataList[rowId];
        if (!rowData.bVisible) {
            continue;
        }
        const Storage* pStorage = GetStorageAt(pageList, rowId);
        if (pStorage == nullptr) {
            continue;
        }
        if (!pStorage->bShowCheckBox) {
            continue;
        }
        if (pStorage->bChecked) {
            nCheckCount++;
        }
        else {
            nUnCheckCount++;
        }
        if ((nCheckCount > 0) && (nUnCheckCount > 0)) {
            //确认是部分选择
            bChecked = true;
            bPartChecked = true;
            return;
        }
    }
    if ((nCheckCount > 0) && (nUnCheckCount == 0)) {
        bChecked = true;
    }
    else if ((nCheckCount == 0) && (nUnCheckCount > 0)) {
        bChecked = false;
    }
}

bool ListCtrlData::SetDataItemImageId(size_t itemIndex, int32_t imageId, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    if (imageId < -1) {
        imageId = -1;
    }
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        if (rowData.nImageId != imageId) {
            rowData.nImageId = imageId;
            bChanged = true;
        }
        bRet = true;
    }
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

int32_t ListCtrlData::GetDataItemImageId(size_t itemIndex) const
{
    int32_t imageId = -1;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        imageId = pRowData->nImageId;
    }
    return imageId;
}

bool ListCtrlData::SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        int8_t nOldAlwaysAtTop = rowData.nAlwaysAtTop;
        bChanged = rowData.nAlwaysAtTop != nAlwaysAtTop;
        rowData.nAlwaysAtTop = nAlwaysAtTop;
        if ((nOldAlwaysAtTop >= 0) && (nAlwaysAtTop < 0)) {
            m_atTopRowCount -= 1;
        }
        else if ((nOldAlwaysAtTop < 0) && (nAlwaysAtTop >= 0)) {
            m_atTopRowCount += 1;
        }
        ASSERT(m_atTopRowCount >= 0);
        bRet = true;
    }
    AssertDataConsistency();
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

int8_t ListCtrlData::GetDataItemAlwaysAtTop(size_t itemIndex) const
{
    int8_t nValue = -1;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        nValue = pRowData->nAlwaysAtTop;
    }
    return nValue;
}

bool ListCtrlData::SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool& bChanged)
{
    bChanged = false;
    if (nItemHeight < 0) {
        nItemHeight = -1;
    }
    if (m_nDefaultItemHeight == nItemHeight) {
        //如果等于默认高度，则设置为标志值
        nItemHeight = -1;
    }

    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        int16_t nOldItemHeight = rowData.nItemHeight;
        bChanged = rowData.nItemHeight != nItemHeight;
        ASSERT(nItemHeight <= INT16_MAX);
        rowData.nItemHeight = (int16_t)nItemHeight;
        if ((nOldItemHeight >= 0) && (nItemHeight < 0)) {
            m_heightRowCount -= 1;
        }
        else if ((nOldItemHeight < 0) && (nItemHeight >= 0)) {
            m_heightRowCount += 1;
        }
        ASSERT(m_heightRowCount >= 0);
        bRet = true;
    }
    AssertDataConsistency();
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

int32_t ListCtrlData::GetDataItemHeight(size_t itemIndex) const
{
    int32_t nValue = 0;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        nValue = pRowData->nItemHeight;
        if ((nValue < 0) && (m_nDefaultItemHeight > 0)) {
            //取默认高度            
            nValue = m_nDefaultItemHeight;
        }
    }
    return nValue;
}

bool ListCtrlData::SetDataItemUserData(size_t itemIndex, size_t itemData)
{
    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    ASSERT(rowId != kInvalidRowId);
    if (rowId != kInvalidRowId) {
        ListCtrlItemData& rowData = m_rowDataList[rowId];
        rowData.nUserData = itemData;
        bRet = true;
    }
    return bRet;
}

size_t ListCtrlData::GetDataItemUserData(size_t itemIndex) const
{
    size_t nItemData = 0;
    const ListCtrlItemData* pRowData = GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if (pRowData != nullptr) {
        nItemData = pRowData->nUserData;
    }
    return nItemData;
}


bool ListCtrlData::SetSubItemData(size_t itemIndex, size_t columnId,
                                  const ListCtrlSubItemData& subItemData, bool& bCheckChanged)
{
    bCheckChanged = false;
    Storage storage;
    SubItemToStorage(subItemData, storage);

    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    if (rowId == kInvalidRowId) {
        return false;
    }
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        //关联列：更新数据（未写入时先分配存储页）
        StoragePageList& pageList = iter->second;
        const Storage* pOld = GetStorageAt(pageList, rowId);
        Storage* pStorage = GetStorageAtForWrite(pageList, rowId);
        if (pStorage != nullptr) {
            if (pOld == nullptr) {
                *pStorage = storage;
                if (storage.bChecked) {
                    bCheckChanged = true;
                }
            }
            else {
                if (storage.bChecked != pOld->bChecked) {
                    bCheckChanged = true;
                }
                *pStorage = storage;
            }
            bRet = true;
        }
    }

    if (bRet) {
        EmitDataChanged(itemIndex, itemIndex);
    }
    return bRet;
}

bool ListCtrlData::GetSubItemData(size_t itemIndex, size_t columnId, ListCtrlSubItemData& subItemData) const
{
    subItemData = ListCtrlSubItemData();

    bool bRet = false;
    const RowId rowId = GetRowIdByDisplayIndex(itemIndex);
    if (rowId == kInvalidRowId) {
        return false;
    }
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        const Storage* pStorage = GetStorageAt(iter->second, rowId);
        if (pStorage != nullptr) {
            StorageToSubItem(*pStorage, subItemData);
        }
        bRet = true;
    }
    return bRet;
}

bool ListCtrlData::SetSubItemText(size_t itemIndex, size_t columnId, const DString& text)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->text != text) {
        pStorage->text = text;
        EmitDataChanged(itemIndex, itemIndex);
    }    
    return true;
}

DString ListCtrlData::GetSubItemText(size_t itemIndex, size_t columnId) const
{
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return DString();
    }
    return pStorage->text.c_str();
}

bool ListCtrlData::SetSubItemSortGroup(size_t itemIndex, size_t columnId, int32_t nSortGroup)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->nSortGroup != nSortGroup) {
        pStorage->nSortGroup = nSortGroup;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

int32_t ListCtrlData::GetSubItemSortGroup(size_t itemIndex, size_t columnId) const
{
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return 0;
    }
    return pStorage->nSortGroup;
}

bool ListCtrlData::SetSubItemUserDataN(size_t itemIndex, size_t columnId, uint64_t userDataN)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->userDataN != userDataN) {
        pStorage->userDataN = userDataN;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

uint64_t ListCtrlData::GetSubItemUserDataN(size_t itemIndex, size_t columnId) const
{
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return 0;
    }
    return pStorage->userDataN;
}

bool ListCtrlData::SetSubItemUserDataS(size_t itemIndex, size_t columnId, const DString& userDataS)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->userDataS != userDataS) {
        pStorage->userDataS = userDataS;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

DString ListCtrlData::GetSubItemUserDataS(size_t itemIndex, size_t columnId) const
{
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return DString();
    }
    return pStorage->userDataS.c_str();
}

bool ListCtrlData::SetSubItemTextColor(size_t itemIndex, size_t columnId, const UiColor& textColor)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->textColor != textColor) {
        pStorage->textColor = textColor;
        EmitDataChanged(itemIndex, itemIndex);
    }    
    return true;
}

bool ListCtrlData::GetSubItemTextColor(size_t itemIndex, size_t columnId, UiColor& textColor) const
{
    textColor = UiColor();
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    textColor = pStorage->textColor;
    return true;
}

bool ListCtrlData::SetSubItemTextFormat(size_t itemIndex, size_t columnId, int32_t nTextFormat)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    int32_t nValidTextFormat = (int32_t)Label::GetValidTextStyle(nTextFormat);
    if (pStorage->nTextFormat != nValidTextFormat) {
        pStorage->nTextFormat = ui::TruncateToUInt16(nValidTextFormat);
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

int32_t ListCtrlData::GetSubItemTextFormat(size_t itemIndex, size_t columnId) const
{
    int32_t nTextFormat = 0;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        nTextFormat = pStorage->nTextFormat;
        if (nTextFormat <= 0) {
            nTextFormat = m_nDefaultTextStyle;
        }
    }    
    return nTextFormat;
}

bool ListCtrlData::SetSubItemSingleLine(size_t itemIndex, size_t columnId, bool bSingleLine)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    //基于当前生效的文本格式（含默认格式）切换单行/多行属性：
    //多行模式下，文本会按单元格可用宽度自动换行（行高需由调用方通过SetDataItemHeight调整）
    int32_t nTextFormat = GetSubItemTextFormat(itemIndex, columnId);
    if (bSingleLine) {
        nTextFormat |= TEXT_SINGLELINE;
    }
    else {
        nTextFormat &= ~TEXT_SINGLELINE;
    }
    return SetSubItemTextFormat(itemIndex, columnId, nTextFormat);
}

bool ListCtrlData::IsSubItemSingleLine(size_t itemIndex, size_t columnId) const
{
    int32_t nTextFormat = GetSubItemTextFormat(itemIndex, columnId);
    return (nTextFormat & TEXT_SINGLELINE) != 0;
}

bool ListCtrlData::SetSubItemBkColor(size_t itemIndex, size_t columnId, const UiColor& bkColor)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->bkColor != bkColor) {
        pStorage->bkColor = bkColor;
        EmitDataChanged(itemIndex, itemIndex);
    }    
    return true;
}

bool ListCtrlData::GetSubItemBkColor(size_t itemIndex, size_t columnId, UiColor& bkColor) const
{
    bkColor = UiColor();
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    bkColor = pStorage->bkColor;
    return true;
}

bool ListCtrlData::IsSubItemShowCheckBox(size_t itemIndex, size_t columnId) const
{
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    return pStorage->bShowCheckBox;
}

bool ListCtrlData::SetSubItemShowCheckBox(size_t itemIndex, size_t columnId, bool bShowCheckBox)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->bShowCheckBox != bShowCheckBox) {
        pStorage->bShowCheckBox = bShowCheckBox;
        EmitDataChanged(itemIndex, itemIndex);
    }    
    return true;
}

bool ListCtrlData::SetSubItemCheck(size_t itemIndex, size_t columnId, bool bChecked, bool bRefresh)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    ASSERT(pStorage->bShowCheckBox);
    if (pStorage->bShowCheckBox) {
        if (pStorage->bChecked != bChecked) {
            pStorage->bChecked = bChecked;
            if (bRefresh) {
                EmitDataChanged(itemIndex, itemIndex);
            }            
        }        
        return true;
    }
    return false;
}

bool ListCtrlData::GetSubItemCheck(size_t itemIndex, size_t columnId, bool& bChecked) const
{
    bChecked = false;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    ASSERT(pStorage->bShowCheckBox);
    if (pStorage->bShowCheckBox) {
        bChecked = pStorage->bChecked;
        return true;
    }
    return false;
}

bool ListCtrlData::SetSubItemImageId(size_t itemIndex, size_t columnId, int32_t imageId)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (imageId < -1) {
        imageId = -1;
    }
    if (pStorage->nImageId != imageId) {
        pStorage->nImageId = imageId;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

int32_t ListCtrlData::GetSubItemImageId(size_t itemIndex, size_t columnId) const
{
    int32_t nImageId = -1;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        nImageId = pStorage->nImageId;
    }
    return nImageId;
}

bool ListCtrlData::SetSubItemEditable(size_t itemIndex, size_t columnId, bool bEditable)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->bEditable != bEditable) {
        pStorage->bEditable = bEditable;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

bool ListCtrlData::IsSubItemEditable(size_t itemIndex, size_t columnId) const
{
    bool bEditable = false;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        bEditable = pStorage->bEditable;
    }
    return bEditable;
}

bool ListCtrlData::SetSubItemProgress(size_t itemIndex, size_t columnId, int32_t nProgress)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (nProgress > 100) {
        nProgress = 100;
    }
    if (nProgress < -1) {
        nProgress = -1;
    }
    if (pStorage->nProgress != static_cast<int8_t>(nProgress)) {
        pStorage->nProgress = static_cast<int8_t>(nProgress);
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

int32_t ListCtrlData::GetSubItemProgress(size_t itemIndex, size_t columnId) const
{
    int32_t nProgress = -1;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        nProgress = static_cast<int32_t>(pStorage->nProgress);
    }
    return nProgress;
}

bool ListCtrlData::SetSubItemRichText(size_t itemIndex, size_t columnId, bool bRichText)
{
    Storage* pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    if (pStorage->bRichText != bRichText) {
        pStorage->bRichText = bRichText;
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

bool ListCtrlData::IsSubItemRichText(size_t itemIndex, size_t columnId) const
{
    bool bRichText = false;
    const Storage* pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        bRichText = pStorage->bRichText;
    }
    return bRichText;
}

bool ListCtrlData::SortDataItems(size_t nColumnId, size_t nColumnIndex,
                                 bool bSortedUp, uint8_t nSortFlag,
                                 ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    StorageMap::iterator iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter == m_dataMap.end()) {
        return false;
    }
    const StoragePageList& pageList = iter->second;
    if (m_displayOrder.empty()) {
        return false;
    }

    //只对显示顺序表排序：比较时按行身份读取排序列的单元格数据，数据本体不搬移
    const size_t dataCount = m_displayOrder.size();
    std::vector<StorageData> sortedDataList;
    sortedDataList.reserve(dataCount);
    for (size_t index = 0; index < dataCount; ++index) {
        const RowId rowId = m_displayOrder[index];
        sortedDataList.push_back({index, GetStorageAt(pageList, rowId)});
    }
    if (!SortStorageData(sortedDataList, nColumnId, nColumnIndex, bSortedUp, nSortFlag, pfnCompareFunc, pUserData)) {
        return false;
    }

    //按排序结果重排显示顺序表（仅搬移4字节的行身份，不再对每列整列深拷贝）
    ASSERT(sortedDataList.size() == dataCount);
    std::vector<RowId> newDisplayOrder;
    newDisplayOrder.reserve(dataCount);
    for (size_t index = 0; index < dataCount; ++index) {
        newDisplayOrder.push_back(m_displayOrder[sortedDataList[index].index]);
    }
    m_displayOrder.swap(newDisplayOrder);

    //单选状态记录的是行身份，排序后自动跟随对应的行，无需调整
    AssertDataConsistency();
    EmitCountChanged();
    return true;
}

namespace
{
/** 按 fnLess 实现的 "(a < b)" 升序语义，对数据列表做稳定排序
* @param [in] first/last 待排序区间
* @param [in] bDescending true 表示降序，false 表示升序
* @param [in] fnLess 升序比较函数，实现 (a < b) 的语义
* 说明：
* 1) 使用 std::stable_sort 而非 std::sort：比较结果相等（同值）的元素保持原始相对顺序，
*    同组（如文件夹/文件）或同值的行不会因排序而乱序、交错；
* 2) 降序通过"转置比较"（交换 a/b 的位置）直接排序，而不是"升序排完再整体 std::reverse"——
*    整体反转会把同值元素的相对顺序一并颠倒，同样造成乱序；
* 3) 无单元格数据（pStorage 为 nullptr）的行统一视为最小值：升序排在最前，降序排在最后，
*    与转置前的行为一致。
*/
template <typename IterType, typename LessFunc>
void StableSortDisplayData(IterType first, IterType last, bool bDescending, LessFunc fnLess)
{
    if (!bDescending) {
        std::stable_sort(first, last, fnLess);
    }
    else {
        std::stable_sort(first, last, [&fnLess](const auto& a, const auto& b) {
                return fnLess(b, a);
            });
    }
}

}//namespace

bool ListCtrlData::SortStorageData(std::vector<StorageData>& dataList, size_t nColumnId, size_t nColumnIndex,
                                   bool bSortedUp, uint8_t nSortFlag,
                                   ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    if (dataList.empty()) {
        return false;
    }

    if (pfnCompareFunc == nullptr) {
        //如果无有效参数，则使用外部设置的排序函数
        pfnCompareFunc = m_pfnCompareFunc;
        pUserData = m_pUserData;
    }

    if (pfnCompareFunc != nullptr) {
        //使用自定义的比较函数排序（稳定排序：同值元素保持原始相对顺序）
        ListCtrlCompareParam param;
        param.nColumnId = nColumnId;
        param.nColumnIndex = nColumnIndex;
        param.nSortFlag = nSortFlag;
        param.pUserData = pUserData;
        auto fnLess = [&pfnCompareFunc, &param](const StorageData& a, const StorageData& b) -> bool {
            //实现(a < b)的比较逻辑
            if (b.pStorage == nullptr) {
                return false;
            }
            if (a.pStorage == nullptr) {
                return true;
            }
            return pfnCompareFunc(*a.pStorage, *b.pStorage, param);
        };
        StableSortDisplayData(dataList.begin(), dataList.end(), !bSortedUp, fnLess);
    }
    else {
        //排序：升序，使用默认的排序函数（稳定排序：同值元素保持原始相对顺序）
        auto fnLess = [this, nSortFlag](const StorageData& a, const StorageData& b) -> bool {
            //实现(a < b)的比较逻辑
            if (b.pStorage == nullptr) {
                return false;
            }
            if (a.pStorage == nullptr) {
                return true;
            }
            return SortDataCompareFunc(*a.pStorage, *b.pStorage, nSortFlag);
        };
        StableSortDisplayData(dataList.begin(), dataList.end(), !bSortedUp, fnLess);
    }
    return true;
}

namespace
{
/** 将日期时间文本解析为可比较的 64 位键值（用于 kSortByTextAsDate 排序）
 *  支持的写法：字段之间以任意非数字字符分隔，按"年、月、日、时、分、秒"的顺序取数字段，
 *  例如 "2024-3-7"、"2024/03/07"、"2024-03-07 13:45:00"（月、日、时等可不补零）。
 *  最少需要"年、月、日"三个字段才算有效，否则视为无效日期（无效值小于任何有效值）。
 * @param [in] text 待解析的文本
 * @param [out] nKey 解析得到的键值，解析无效时保持不变
 * @return 解析成功返回 true，否则返回 false
 */
bool ParseTextAsDateKey(const UiString& text, int64_t& nKey)
{
    const DString::value_type* pStr = text.c_str();
    if (pStr == nullptr) {
        return false;
    }
    //依次取"年、月、日、时、分、秒"六个数字段
    constexpr int32_t kFieldCount = 6;
    int64_t nFields[kFieldCount] = { 0, 0, 0, 0, 0, 0 };
    int32_t nFoundCount = 0;
    bool bInNumber = false;
    for (; *pStr != 0; ++pStr) {
        const DString::value_type ch = *pStr;
        if ((ch >= _T('0')) && (ch <= _T('9'))) {
            if (!bInNumber) {
                if (nFoundCount >= kFieldCount) {
                    //超出支持范围的数字段，忽略
                    break;
                }
                ++nFoundCount;    //开始一个新的数字段
                bInNumber = true;
            }
            nFields[nFoundCount - 1] = nFields[nFoundCount - 1] * 10 + (ch - _T('0'));
        }
        else {
            bInNumber = false;
        }
    }
    if (nFoundCount < 3) {
        //不足"年、月、日"三个字段，无法构成有效的日期
        return false;
    }
    //按"年、月、日、时、分、秒"逐级进位编码，使键值的大小顺序与日期时间的先后顺序一致
    int64_t nRetKey = 0;
    for (int32_t i = 0; i < kFieldCount; ++i) {
        nRetKey = nRetKey * 100 + nFields[i];
    }
    nKey = nRetKey;
    return true;
}
}//namespace

bool ListCtrlData::SortDataCompareFunc(const ListCtrlSubItemData2& a, const ListCtrlSubItemData2& b, uint8_t nSortFlag) const
{
    if (nSortFlag & ListCtrlSubItemSortFlag::kSortByGroup) {
        //支持分组排序
        if (a.nSortGroup != b.nSortGroup) {
            return a.nSortGroup < b.nSortGroup;
        }
    }
    if (nSortFlag & ListCtrlSubItemSortFlag::kSortByUserDataN) {
        //按 .userDataN 字段排序(整型值)
        return a.userDataN < b.userDataN;
    }
    else if (nSortFlag & ListCtrlSubItemSortFlag::kSortByUserDataS) {
        //按 .userDataS 字段排序(字符串值)
        if (nSortFlag & ListCtrlSubItemSortFlag::kSortNoCase) {
            //不区分大小写
            return StringUtil::StringICompare(a.userDataS.c_str(), b.userDataS.c_str()) < 0;
        }
        else {
            //区分大小写
            return StringUtil::StringCompare(a.userDataS.c_str(), b.userDataS.c_str()) < 0;
        }
    }
    else if (nSortFlag & ListCtrlSubItemSortFlag::kSortByProgress) {
        //按进度值排序（列级显式声明，替代早期"按单元格nProgress推断"的行为）
        //注：不显示进度条的单元格(nProgress为-1)视为最小值，保证比较结果是一个严格的全序
        return a.nProgress < b.nProgress;
    }
    else if (nSortFlag & ListCtrlSubItemSortFlag::kSortByTextAsNumber) {
        //将 text 按数值比较排序（如"10" > "9"），而非按字符串字典序
        double dValueA = StringUtil::StringToDouble(a.text.c_str());
        double dValueB = StringUtil::StringToDouble(b.text.c_str());
        return dValueA < dValueB;
    }
    else if (nSortFlag & ListCtrlSubItemSortFlag::kSortByTextAsDate) {
        //将 text 按日期时间比较排序（如"2024-3-7" < "2024-10-1"），而非按字符串字典序
        //注：无法解析为日期的文本视为最小值（与其他排序标志的"无效值最小"约定一致）；
        //    键值相同时再按文本比较，保证比较结果是一个严格的全序
        int64_t nKeyA = -1;
        int64_t nKeyB = -1;
        ParseTextAsDateKey(a.text, nKeyA);
        ParseTextAsDateKey(b.text, nKeyB);
        if (nKeyA != nKeyB) {
            return nKeyA < nKeyB;
        }
        return StringUtil::StringCompare(a.text.c_str(), b.text.c_str()) < 0;
    }
    else {
        //按 .text 字段排序(字符串值)
        if (nSortFlag & ListCtrlSubItemSortFlag::kSortNoCase) {
            //不区分大小写
            return StringUtil::StringICompare(a.text.c_str(), b.text.c_str()) < 0;
        }
        else {
            //区分大小写
            return StringUtil::StringCompare(a.text.c_str(), b.text.c_str()) < 0;
        }
    }
}

void ListCtrlData::SetSortCompareFunction(ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    m_pfnCompareFunc = pfnCompareFunc;
    m_pUserData = pUserData;
}

void ListCtrlData::SetSelectedElements(const std::vector<size_t>& selectedIndexs,
                                       bool bClearOthers,
                                       std::vector<size_t>& refreshIndexs)
{
    refreshIndexs.clear();
    ASSERT(IsMultiSelect());
    if (!IsMultiSelect()) {
        return;
    }
    std::set<size_t> selectSet;
    for (auto index : selectedIndexs) {
        selectSet.insert(index);
    }
    std::set<size_t> refreshSet;
    std::vector<size_t> oldSelectedIndexs;
    if (bClearOthers) {
        GetSelectedElements(oldSelectedIndexs);
        if (!oldSelectedIndexs.empty()) {
            for (size_t nElementIndex : oldSelectedIndexs) {
                if (selectSet.find(nElementIndex) != selectSet.end()) {
                    //过滤掉即将选择的
                    continue;
                }
                SetElementSelected(nElementIndex, false);
                refreshSet.insert(nElementIndex);
            }
        }
    }
    for (size_t nElementIndex : selectedIndexs) {
        if (IsElementSelected(nElementIndex)) {
            continue;
        }
        SetElementSelected(nElementIndex, true);
        refreshSet.insert(nElementIndex);
    }
    for (auto index : refreshSet) {
        refreshIndexs.push_back(index);
    }
}

bool ListCtrlData::SelectAll(std::vector<size_t>& refreshIndexs)
{
    ASSERT(IsMultiSelect());
    if (!IsMultiSelect()) {
        return false;
    }
    std::vector<size_t> selectedIndexs;
    size_t nCount = GetElementCount();
    for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
        if (IsSelectableElement(nElementIndex) && !IsElementSelected(nElementIndex)) {
            SetElementSelected(nElementIndex, true);
            selectedIndexs.push_back(nElementIndex);
        }
    }
    refreshIndexs.swap(selectedIndexs);
    return !refreshIndexs.empty();
}

bool ListCtrlData::IsSelectableRowData(const ListCtrlItemData& rowData) const
{
    //可见，并且不置顶显示
    return rowData.bVisible && (rowData.nAlwaysAtTop < 0);
}

bool ListCtrlData::IsSelectableElement(size_t nElementIndex) const
{
    bool bSelectable = true;
    const ListCtrlItemData* pRowData = GetDisplayItemData(nElementIndex);
    if (pRowData != nullptr) {
        bSelectable = IsSelectableRowData(*pRowData);
    }
    return bSelectable;
}

void ListCtrlData::SelectNone(std::vector<size_t>& refreshIndexs)
{
    SelectNoneExclude(std::vector<size_t>(), refreshIndexs);
}

void ListCtrlData::SelectNoneExclude(const std::vector<size_t>& excludeIndexs,
                                     std::vector<size_t>& refreshIndexs)
{
    refreshIndexs.clear();
    std::vector<size_t> selectedIndexs;
    GetSelectedElements(selectedIndexs);
    if (!selectedIndexs.empty()) {
        std::set<size_t> indexSet;
        for (size_t nElementIndex : excludeIndexs) {
            indexSet.insert(nElementIndex);
        }
        for (size_t nElementIndex : selectedIndexs) {
            if (!indexSet.empty()) {
                if (indexSet.find(nElementIndex) != indexSet.end()) {
                    //排除
                    continue;
                }
            }
            SetElementSelected(nElementIndex, false);
        }
        refreshIndexs.swap(selectedIndexs);
    }
}

}//namespace ui

