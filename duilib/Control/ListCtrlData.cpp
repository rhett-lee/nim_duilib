#include "ListCtrlData.h"
#include "duilib/Control/ListCtrl.h"
#include "duilib/Core/GlobalManager.h"
#include <unordered_map>
#include <set>

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
    m_nSelectedIndex(Box::InvalidIndex),
    m_nDefaultTextStyle(0),
    m_nDefaultItemHeight(-1),
    m_bAutoCheckSelect(false)
{
}

Control* ListCtrlData::CreateElement()
{
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
    ASSERT(nElementIndex < m_rowDataList.size());
    if ((nElementIndex >= m_rowDataList.size()) || (pControl == nullptr)) {
        return false;
    }
    const ListCtrlItemData& itemData = m_rowDataList[nElementIndex];
    std::vector<ListCtrlSubItemData2Pair> subItemList;
    if (!GetSubItemStorageList(nElementIndex, subItemList)) {
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
    std::vector<ListCtrlSubItemData2Ptr> subItemList;
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        const StoragePtrList& storageList = iter->second;
        const size_t nCount = storageList.size();
        for (size_t index = 0; index < nCount; ++index) {
            if (storageList[index] != nullptr) {
                subItemList.push_back(storageList[index]);
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
    ASSERT(nElementIndex < m_rowDataList.size());
    if (nElementIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[nElementIndex];
        if (rowData.bSelected != bSelected) {
            rowData.bSelected = bSelected;//多选或者单选的情况下，都更新
        }
        if (IsAutoCheckSelect() && (rowData.bChecked != rowData.bSelected)) {
            rowData.bChecked = rowData.bSelected;
        }
    }

    if (!m_bMultiSelect) {
        //单选的情况
        if (bSelected) {
            m_nSelectedIndex = nElementIndex;
        }
        else if (m_nSelectedIndex == nElementIndex) {
            m_nSelectedIndex = Box::InvalidIndex;
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
        ASSERT(nElementIndex < m_rowDataList.size());
        if (nElementIndex < m_rowDataList.size()) {
            const ListCtrlItemData& rowData = m_rowDataList[nElementIndex];
            bSelected = rowData.bSelected;
        }
    }
    else {
        //单选
        bSelected = (m_nSelectedIndex == nElementIndex);
    }
    return bSelected;
}

void ListCtrlData::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    if (m_bMultiSelect) {
        size_t nCount = m_rowDataList.size();
        for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
            const ListCtrlItemData& rowData = m_rowDataList[nElementIndex];
            if (rowData.bSelected) {
                selectedIndexs.push_back(nElementIndex);
            }
        }
    }
    else {
        if (m_nSelectedIndex < m_rowDataList.size()) {
            selectedIndexs.push_back(m_nSelectedIndex);
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
        const size_t nItemCount = m_rowDataList.size();
        for (size_t itemIndex = 0; itemIndex < nItemCount; ++itemIndex) {
            ListCtrlItemData& rowData = m_rowDataList[itemIndex];
            if (rowData.bSelected) {
                if (m_nSelectedIndex != itemIndex) {
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
}

bool ListCtrlData::IsValidDataItemIndex(size_t itemIndex) const
{
    return itemIndex < m_rowDataList.size();
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
    StoragePtrList& storageList = m_dataMap[columnId];
    //列的长度与行保持一致
    storageList.resize(m_rowDataList.size());
    EmitCountChanged();
    return true;
}

bool ListCtrlData::RemoveColumn(size_t columnId)
{
    auto iter = m_dataMap.find(columnId);
    if (iter != m_dataMap.end()) {
        m_dataMap.erase(iter);
        if (m_dataMap.empty()) {
            //如果所有列都删除了，行也清空为0
            m_rowDataList.clear();
            m_nSelectedIndex = Box::InvalidIndex;
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
        StoragePtrList& storageList = iter->second;
        const size_t nCount = storageList.size();
        for (size_t index = 0; index < nCount; ++index) {
            if (storageList[index] == nullptr) {
                storageList[index] = std::make_shared<Storage>();
            }
            storageList[index]->bChecked = bChecked;
        }
        bRet = true;
    }
    if (bRefresh && bRet) {
        EmitCountChanged();
    }
    return bRet;
}

ListCtrlData::StoragePtr ListCtrlData::GetSubItemStorage(
    size_t itemIndex, size_t nColumnId) const
{
    StoragePtr pStorage;
    auto iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        const StoragePtrList& storageList = iter->second;
        ASSERT(itemIndex < storageList.size());
        if (itemIndex < storageList.size()) {
            //关联列：获取数据
            pStorage = storageList[itemIndex];
        }
    }
    return pStorage;
}

ListCtrlData::StoragePtr ListCtrlData::GetSubItemStorageForWrite(
    size_t itemIndex, size_t nColumnId)
{
    StoragePtr pStorage;
    auto iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        StoragePtrList& storageList = iter->second;
        ASSERT(itemIndex < storageList.size());
        if (itemIndex < storageList.size()) {
            //关联列：获取数据
            pStorage = storageList[itemIndex];
            if (pStorage == nullptr) {
                pStorage = std::make_shared<Storage>();
                storageList[itemIndex] = pStorage;
            }
        }
    }
    return pStorage;
}

bool ListCtrlData::GetSubItemStorageList(size_t itemIndex, std::vector<ListCtrlSubItemData2Pair>& subItemList) const
{
    subItemList.clear();
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex >= m_rowDataList.size()) {
        return false;
    }
    ListCtrlSubItemData2Pair dataPair;
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        dataPair.nColumnId = iter->first;        
        const StoragePtrList& dataList = iter->second;
        ASSERT(itemIndex < dataList.size());
        if (itemIndex < dataList.size()) {
            dataPair.pSubItemData = dataList.at(itemIndex);
        }
        else {
            dataPair.pSubItemData = nullptr;
        }
        subItemList.push_back(dataPair);
    }
    return true;
}

const ListCtrlData::RowDataList& ListCtrlData::GetItemDataList() const
{
    return m_rowDataList;
}

bool ListCtrlData::IsNormalMode() const
{
    ASSERT((m_hideRowCount >= 0) && (m_heightRowCount >= 0) && (m_atTopRowCount >= 0));
    return (m_hideRowCount == 0) && (m_heightRowCount == 0) && (m_atTopRowCount == 0);
}

size_t ListCtrlData::GetDataItemCount() const
{
#ifdef _DEBUG
    auto iter = m_dataMap.begin();
    for (; iter != m_dataMap.end(); ++iter) {
        ASSERT(iter->second.size() == m_rowDataList.size());
    }
#endif
    return m_rowDataList.size();
}

bool ListCtrlData::SetDataItemCount(size_t itemCount)
{
    ASSERT(itemCount != Box::InvalidIndex);
    if (itemCount == Box::InvalidIndex) {
        return false;
    }
    if (itemCount == m_rowDataList.size()) {
        //没有变化
        return true;
    }
    size_t nOldCount = m_rowDataList.size();
    m_rowDataList.resize(itemCount); 
    if (m_nSelectedIndex >= m_rowDataList.size()) {
        m_nSelectedIndex = Box::InvalidIndex;
    }
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        iter->second.resize(itemCount);
    }
    if (itemCount < nOldCount) {
        //行数变少了
        if ((m_hideRowCount != 0) || (m_heightRowCount != 0) || (m_atTopRowCount != 0)) {
            UpdateNormalMode();
        }
    }
    EmitCountChanged();
    return true;
}

void ListCtrlData::UpdateNormalMode()
{
    m_hideRowCount = 0;
    m_heightRowCount = 0;
    m_atTopRowCount = 0;
    for (const ListCtrlItemData& data : m_rowDataList) {
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

    size_t nDataItemIndex = Box::InvalidIndex;
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        size_t id = iter->first;
        StoragePtrList& storageList = iter->second;
        if (id == columnId) {
            //关联列：保存数据
            storageList.push_back(std::make_shared<Storage>(storage));
            nDataItemIndex = storageList.size() - 1;
        }
        else {
            //其他列：插入空数据
            storageList.push_back(nullptr);
        }
    }

    //行数据，插入1条数据
    m_rowDataList.push_back(ListCtrlItemData());

    EmitCountChanged();
    return nDataItemIndex;
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

    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        size_t id = iter->first;
        StoragePtrList& storageList = iter->second;
        if (id == columnId) {
            //关联列：保存数据
            storageList.insert(storageList.begin() + itemIndex, std::make_shared<Storage>(storage));
        }
        else {
            //其他列：插入空数据
            storageList.insert(storageList.begin() + itemIndex, nullptr);
        }
    }

    //行数据，插入1条数据
    ASSERT(itemIndex < m_rowDataList.size());
    if ((m_nSelectedIndex < m_rowDataList.size()) && (itemIndex <= m_nSelectedIndex)) {
        ++m_nSelectedIndex;
    }
    m_rowDataList.insert(m_rowDataList.begin() + itemIndex, ListCtrlItemData());

    EmitCountChanged();
    return true;
}

bool ListCtrlData::DeleteDataItem(size_t itemIndex)
{
    if (!IsValidDataItemIndex(itemIndex)) {
        //索引号无效
        return false;
    }

    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        StoragePtrList& storageList = iter->second;
        if (itemIndex < storageList.size()) {
            storageList.erase(storageList.begin() + itemIndex);
        }
    }

    //删除一行
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData oldData = m_rowDataList[itemIndex];
        if (m_nSelectedIndex < m_rowDataList.size()) {
            if (m_nSelectedIndex == itemIndex) {
                m_nSelectedIndex = Box::InvalidIndex;
            }
            else if (m_nSelectedIndex > itemIndex) {
                m_nSelectedIndex -= 1;
            }
        }
        m_rowDataList.erase(m_rowDataList.begin() + itemIndex);
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
    }
    EmitCountChanged();
    return true;
}

bool ListCtrlData::DeleteAllDataItems()
{
    bool bDeleted = false;
    for (auto iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        StoragePtrList& storageList = iter->second;
        if (!storageList.empty()) {
            bDeleted = true;
        }
        StoragePtrList emptyList;
        storageList.swap(emptyList);
    }
    //清空行数据
    if (!m_rowDataList.empty()) {
        bDeleted = true;
    }
    m_rowDataList.clear();
    m_nSelectedIndex = Box::InvalidIndex;
    m_hideRowCount = 0;
    m_heightRowCount = 0;
    m_atTopRowCount = 0;

    if (bDeleted) {
        EmitCountChanged();
    }
    return bDeleted;
}

bool ListCtrlData::SetDataItemData(size_t itemIndex, const ListCtrlItemData& itemData,
                                   bool& bChanged, bool& bCheckChanged)
{
    bChanged = false;
    bCheckChanged = false;
    bool bCountChanged = false;
    bool bItemChanged = false;
    bool bRet = false;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData oldItemData = m_rowDataList[itemIndex];
        m_rowDataList[itemIndex] = itemData;
        if (m_nDefaultItemHeight == m_rowDataList[itemIndex].nItemHeight) {
            //如果等于默认高度，则设置为标志值
            m_rowDataList[itemIndex].nItemHeight = -1;
        }
        const ListCtrlItemData& newItemData = m_rowDataList[itemIndex];
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        itemData = m_rowDataList[itemIndex];
        bRet = true;
    }
    return bRet;
}

bool ListCtrlData::SetDataItemVisible(size_t itemIndex, bool bVisible, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {        
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    if (bChanged) {
        EmitCountChanged();
    }
    return bRet;
}

bool ListCtrlData::IsDataItemVisible(size_t itemIndex) const
{
    bool bValue = false;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        bValue = rowData.bVisible;
    }
    return bValue;
}

bool ListCtrlData::SetDataItemSelected(size_t itemIndex, bool bSelected, bool& bChanged)
{
    bChanged = false;
    if (itemIndex >= m_rowDataList.size()) {
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        bChecked = rowData.bChecked;
    }
    return bChecked;
}

bool ListCtrlData::SetAllDataItemsCheck(bool bChecked)
{
    bool bChanged = false;
    size_t nCount = m_rowDataList.size();
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    const size_t nCount = m_rowDataList.size();
    if (!bClearOthers) {
        for (size_t itemIndex : itemIndexs) {
            if (itemIndex < nCount) {
                ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
            ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    const size_t nCount = m_rowDataList.size();
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    const size_t nCount = m_rowDataList.size();
    if (nCount == 0) {
        return;
    }
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
        ASSERT(FALSE);
    }
}

void ListCtrlData::GetDataItemsSelectStatus(bool& bSelected, bool& bPartSelected) const
{
    bSelected = false;
    bPartSelected = false;
    size_t nSelectCount = 0;
    size_t nUnSelectCount = 0;
    const size_t nCount = m_rowDataList.size();
    if (nCount == 0) {
        return;
    }
    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
        ASSERT(FALSE);
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
    const StoragePtrList& sortStorageList = iter->second;
    size_t nCheckCount = 0;
    size_t nUnCheckCount = 0;
    const size_t nCount = sortStorageList.size();
    if (nCount == 0) {
        return;
    }
    ASSERT(nCount == m_rowDataList.size());
    if (nCount != m_rowDataList.size()) {
        return;
    }

    for (size_t itemIndex = 0; itemIndex < nCount; ++itemIndex) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        if (!rowData.bVisible) {
            continue;
        }
        const StoragePtr& pStorage = sortStorageList[itemIndex];
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        imageId = rowData.nImageId;
    }
    return imageId;
}

bool ListCtrlData::SetDataItemAlwaysAtTop(size_t itemIndex, int8_t nAlwaysAtTop, bool& bChanged)
{
    bChanged = false;
    bool bRet = false;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

int8_t ListCtrlData::GetDataItemAlwaysAtTop(size_t itemIndex) const
{
    int8_t nValue = -1;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        nValue = rowData.nAlwaysAtTop;
    }
    return nValue;
}

bool ListCtrlData::SetDataItemHeight(size_t itemIndex, int32_t nItemHeight, bool bNeedDpiScale, bool& bChanged)
{
    bChanged = false;
    if (nItemHeight < 0) {
        nItemHeight = -1;
    }
    if (bNeedDpiScale && (nItemHeight > 0)) {
        GlobalManager::Instance().Dpi().ScaleInt(nItemHeight);
    }
    if (m_nDefaultItemHeight == nItemHeight) {
        //如果等于默认高度，则设置为标志值
        nItemHeight = -1;
    }

    bool bRet = false;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
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
    //不刷新，由外部判断是否需要刷新
    return bRet;
}

int32_t ListCtrlData::GetDataItemHeight(size_t itemIndex) const
{
    int32_t nValue = 0;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        nValue = rowData.nItemHeight;
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
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        rowData.nUserData = itemData;
        bRet = true;
    }
    return bRet;
}

size_t ListCtrlData::GetDataItemUserData(size_t itemIndex) const
{
    size_t nItemData = 0;
    ASSERT(itemIndex < m_rowDataList.size());
    if (itemIndex < m_rowDataList.size()) {
        const ListCtrlItemData& rowData = m_rowDataList[itemIndex];
        nItemData = rowData.nUserData;
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
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        //关联列：更新数据
        StoragePtrList& storageList = iter->second;
        ASSERT(itemIndex < storageList.size());
        if (itemIndex < storageList.size()) {
            StoragePtr pStorage = storageList[itemIndex];
            if (pStorage == nullptr) {
                storageList[itemIndex] = std::make_shared<Storage>(storage);
                if (storage.bChecked) {
                    bCheckChanged = true;
                }
            }
            else {
                if (storage.bChecked != pStorage->bChecked) {
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
    auto iter = m_dataMap.find(columnId);
    ASSERT(iter != m_dataMap.end());
    if (iter != m_dataMap.end()) {
        const StoragePtrList& storageList = iter->second;
        ASSERT(itemIndex < storageList.size());
        if (itemIndex < storageList.size()) {
            StoragePtr pStorage = storageList[itemIndex];
            if (pStorage != nullptr) {
                StorageToSubItem(*pStorage, subItemData);
            }
            bRet = true;
        }
    }
    return bRet;
}

bool ListCtrlData::SetSubItemText(size_t itemIndex, size_t columnId, const std::wstring& text)
{
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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

std::wstring ListCtrlData::GetSubItemText(size_t itemIndex, size_t columnId) const
{
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return std::wstring();
    }
    return pStorage->text.c_str();
}

bool ListCtrlData::SetSubItemTextColor(size_t itemIndex, size_t columnId, const UiColor& textColor)
{
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    int32_t nValidTextFormat = 0;
    if (nTextFormat & TEXT_SINGLELINE) {
        nValidTextFormat |= TEXT_SINGLELINE;
    }

    if (nTextFormat & TEXT_CENTER) {
        nValidTextFormat |= TEXT_CENTER;
    }
    else if (nTextFormat & TEXT_RIGHT) {
        nValidTextFormat |= TEXT_RIGHT;
    }
    else{
        nValidTextFormat |= TEXT_LEFT;
    }

    if (nTextFormat & TEXT_VCENTER) {
        nValidTextFormat |= TEXT_VCENTER;
    }
    else if (nTextFormat & TEXT_BOTTOM) {
        nValidTextFormat |= TEXT_BOTTOM;
    }
    else {
        nValidTextFormat |= TEXT_TOP;
    }
    if (nTextFormat & TEXT_END_ELLIPSIS) {
        nValidTextFormat |= TEXT_END_ELLIPSIS;
    }
    if (nTextFormat & TEXT_PATH_ELLIPSIS) {
        nValidTextFormat |= TEXT_PATH_ELLIPSIS;
    }
    if (nTextFormat & TEXT_NOCLIP) {
        nValidTextFormat |= TEXT_NOCLIP;
    }

    if (pStorage->nTextFormat != nValidTextFormat) {
        pStorage->nTextFormat = ui::TruncateToUInt16(nValidTextFormat);
        EmitDataChanged(itemIndex, itemIndex);
    }
    return true;
}

int32_t ListCtrlData::GetSubItemTextFormat(size_t itemIndex, size_t columnId) const
{
    int32_t nTextFormat = 0;
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        nTextFormat = pStorage->nTextFormat;
        if (nTextFormat <= 0) {
            nTextFormat = m_nDefaultTextStyle;
        }
    }    
    return nTextFormat;
}

bool ListCtrlData::SetSubItemBkColor(size_t itemIndex, size_t columnId, const UiColor& bkColor)
{
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage == nullptr) {
        //索引号无效
        return false;
    }
    return pStorage->bShowCheckBox;
}

bool ListCtrlData::SetSubItemShowCheckBox(size_t itemIndex, size_t columnId, bool bShowCheckBox)
{
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        nImageId = pStorage->nImageId;
    }
    return nImageId;
}

bool ListCtrlData::SetSubItemEditable(size_t itemIndex, size_t columnId, bool bEditable)
{
    StoragePtr pStorage = GetSubItemStorageForWrite(itemIndex, columnId);
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
    StoragePtr pStorage = GetSubItemStorage(itemIndex, columnId);
    ASSERT(pStorage != nullptr);
    if (pStorage != nullptr) {
        bEditable = pStorage->bEditable;
    }
    return bEditable;
}

bool ListCtrlData::SortDataItems(size_t nColumnId, size_t nColumnIndex, bool bSortedUp,
                                 ListCtrlDataCompareFunc pfnCompareFunc, void* pUserData)
{
    StorageMap::iterator iter = m_dataMap.find(nColumnId);
    ASSERT(iter != m_dataMap.end());
    if (iter == m_dataMap.end()) {
        return false;
    }
    StoragePtrList sortStorageList = iter->second;
    if (sortStorageList.empty()) {
        return false;
    }
    std::vector<StorageData> sortedDataList;
    const size_t dataCount = sortStorageList.size();
    for (size_t index = 0; index < dataCount; ++index) {
        sortedDataList.push_back({index, sortStorageList[index] });
    }    
    SortStorageData(sortedDataList, nColumnId, nColumnIndex, bSortedUp, pfnCompareFunc, pUserData);

    //对原数据进行顺序调整
    const size_t sortedDataCount = sortedDataList.size();
    StoragePtrList orgStorageList; //副本数据
    for (iter = m_dataMap.begin(); iter != m_dataMap.end(); ++iter) {
        orgStorageList = iter->second; 
        StoragePtrList& storageList = iter->second;   //修改目标
        ASSERT(storageList.size() == sortedDataList.size());
        if (storageList.size() != sortedDataList.size()) {
            return false;
        }
        for (size_t index = 0; index < sortedDataCount; ++index) {
            const StorageData& sortedData = sortedDataList[index];
            storageList[index] = orgStorageList[sortedData.index]; //赋值原数据
        }
    }

    //对行数据进行排序
    bool bFoundSelectedIndex = false;
    ASSERT(sortedDataCount == m_rowDataList.size());
    RowDataList rowDataList = m_rowDataList;
    for (size_t index = 0; index < sortedDataCount; ++index) {
        const StorageData& sortedData = sortedDataList[index];
        m_rowDataList[index] = rowDataList[sortedData.index]; //赋值原数据
        if (!bFoundSelectedIndex && (m_nSelectedIndex == sortedData.index)) {
            m_nSelectedIndex = index;
            bFoundSelectedIndex = true;
        }
    }

    EmitCountChanged();
    return true;
}

bool ListCtrlData::SortStorageData(std::vector<StorageData>& dataList,                                            
                                   size_t nColumnId, size_t nColumnIndex, bool bSortedUp,
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
        //使用自定义的比较函数排序
        ListCtrlCompareParam param;
        param.nColumnId = nColumnId;
        param.nColumnIndex = nColumnIndex;
        param.pUserData = pUserData;
        std::sort(dataList.begin(), dataList.end(), [this, pfnCompareFunc, &param](const StorageData& a, const StorageData& b) {
                //实现(a < b)的比较逻辑
                if (b.pStorage == nullptr) {
                    return false;
                }
                if (a.pStorage == nullptr) {
                    return true;
                }
                const Storage& storageA = *a.pStorage;
                const Storage& storageB = *b.pStorage;
                return pfnCompareFunc(storageA, storageB, param);
            });
    }
    else {
        //排序：升序，使用默认的排序函数
        std::sort(dataList.begin(), dataList.end(), [this](const StorageData& a, const StorageData& b) {
                //实现(a < b)的比较逻辑
                if (b.pStorage == nullptr) {
                    return false;
                }
                if (a.pStorage == nullptr) {
                    return true;
                }
                const Storage& storageA = *a.pStorage;
                const Storage& storageB = *b.pStorage;
                return SortDataCompareFunc(storageA, storageB);
            });
    }
    if (!bSortedUp) {
        //降序
        std::reverse(dataList.begin(), dataList.end());
    }
    return true;
}

bool ListCtrlData::SortDataCompareFunc(const ListCtrlSubItemData2& a, const ListCtrlSubItemData2& b) const
{
    //默认按字符串比较, 区分大小写
    return ::wcscmp(a.text.c_str(), b.text.c_str()) < 0;
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
    const ListCtrlData::RowDataList& itemDataList = GetItemDataList();
    if (nElementIndex < itemDataList.size()) {
        const ListCtrlItemData& rowData = itemDataList[nElementIndex];
        bSelectable = IsSelectableRowData(rowData);
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

