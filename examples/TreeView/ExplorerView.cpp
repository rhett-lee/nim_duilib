#include "ExplorerView.h"
#include "MainForm.h"

#include <algorithm>

ExplorerView::ExplorerView(MainForm* pMainForm, ui::ListCtrl* pListCtrl):
    m_pMainForm(pMainForm),
    m_pListCtrl(pListCtrl),
    m_nRemoveIconCallbackId(0)
{
    Initialize();
}

ExplorerView::~ExplorerView()
{
    ui::GlobalManager::Instance().Icon().DetachRemoveIconEvent(m_nRemoveIconCallbackId);
    ui::DirectoryTree::ClearPathInfoList(m_pathInfoList);
    m_pathInfoList.clear();
}

ui::ListCtrl* ExplorerView::GetListCtrl() const
{
    return m_pListCtrl.get();
}

bool ExplorerView::GetSortColumnInfo(ExplorerViewColumn& viewColumn, bool& bSortUp) const
{
    if (m_pListCtrl == nullptr) {
        return false;
    }
    size_t nSortColumnId = ui::Box::InvalidIndex;
    if (!m_pListCtrl->GetSortColumn(nSortColumnId, bSortUp)) {
        return false;
    }
    //用列ID反查列的含义：用户拖动表头调整列顺序后，列索引会变化，不能再用列索引判断
    for (const auto& item : m_columnIdMap) {
        if (item.second == nSortColumnId) {
            viewColumn = item.first;
            return true;
        }
    }
    return false;
}

void ExplorerView::SortByColumn(ExplorerViewColumn viewColumn, bool bSortUp)
{
    if (m_pListCtrl != nullptr) {
        size_t nColumnId = GetColumnId(viewColumn);
        uint8_t nSortFlag = m_pListCtrl->GetColumnSortFlagById(nColumnId);
        m_pListCtrl->SortDataItemsById(nColumnId, bSortUp, nSortFlag);
    }
}

void ExplorerView::Initialize()
{
    ASSERT(m_pMainForm != nullptr);
    ASSERT(m_pListCtrl != nullptr);
    if ((m_pListCtrl == nullptr) || (m_pMainForm == nullptr)) {
        return;
    }

    //挂载图标删除事件
    m_nRemoveIconCallbackId = ui::GlobalManager::Instance().Icon().AttachRemoveIconEvent(ui::UiBind(&ExplorerView::OnRemoveIcon, this, std::placeholders::_1));

    InitViewHeader();

    if (m_pListCtrl != nullptr) {
        //关联图片列表: 三个视图共享一个图片列表
        ui::ImageListPtr pImageList = std::make_shared<ui::ImageList>();
        pImageList->SetImageSize(ui::UiSize(20, 20), m_pMainForm->Dpi(), true);
        m_pListCtrl->SetImageList(ui::ListCtrlType::Report, pImageList);
        m_pListCtrl->SetImageList(ui::ListCtrlType::Icon, pImageList);
        m_pListCtrl->SetImageList(ui::ListCtrlType::List, pImageList);

        //挂载列表项鼠标双击事件
        m_pListCtrl->AttachDoubleClick(UiBind(&ExplorerView::OnExplorerViewDoubleClick, this, std::placeholders::_1));

        //挂载列表项鼠标右键事件（弹出右键菜单）
        m_pListCtrl->AttachRClick(UiBind(&ExplorerView::OnExplorerViewRClick, this, std::placeholders::_1));

        //点击表头排序后，同步更新状态栏中的排序信息
        m_pListCtrl->AttachColumnSorted([this](const ui::EventArgs&) {
            if (m_pMainForm != nullptr) {
                m_pMainForm->OnColumnSorted();
            }
            return true;
            });

        //选择项发生变化时，同步更新状态栏中的选中项信息
        m_pListCtrl->AttachSelChanged([this](const ui::EventArgs&) {
            if (m_pMainForm != nullptr) {
                m_pMainForm->OnViewSelectionChanged();
            }
            return true;
            });
        m_pListCtrl->AttachSelect([this](const ui::EventArgs&) {
            if (m_pMainForm != nullptr) {
                m_pMainForm->OnViewSelectionChanged();
            }
            return true;
            });
    }
}

bool ExplorerView::OnExplorerViewDoubleClick(const ui::EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) && (msg.wParam != 0) && (m_pMainForm != nullptr)) {
        size_t nItemIndex = msg.lParam;
        size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex < m_pathInfoList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathInfoList[nIndex];
            if (!pathInfo.m_filePath.IsEmpty()) {
                //双击目录则进入其中，双击文件则用系统默认程序打开
                m_pMainForm->OpenPath(pathInfo.m_filePath);
            }
        }
    }
    return true;
}

bool ExplorerView::OnExplorerViewRClick(const ui::EventArgs& msg)
{
    if ((m_pListCtrl == nullptr) || (m_pMainForm == nullptr)) {
        return true;
    }
    ui::FilePath targetPath;
    ui::ListCtrlItem* pItem = static_cast<ui::ListCtrlItem*>(msg.pEventData);
    if (pItem != nullptr) {
        //通过UserData反查路径列表的索引（数据项索引与路径列表索引不一定相同）
        size_t nIndex = m_pListCtrl->GetDataItemUserData(pItem->GetDataItemIndex());
        if (nIndex < m_pathInfoList.size()) {
            targetPath = m_pathInfoList[nIndex].m_filePath;
        }
    }
    m_pMainForm->ShowViewContextMenu(m_pListCtrl.get(), targetPath);
    return true;
}

void ExplorerView::OnRemoveIcon(uint32_t nIconId)
{
    if (!ui::GlobalManager::Instance().IsInUIThread()) {
        //如果不是在主线程中执行，则转到主线程去执行
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, this->ToWeakCallback([this, nIconId]() {
            OnRemoveIcon(nIconId);
            }));
        return;
    }

    //在主线程中执行
    ui::GlobalManager::Instance().AssertUIThread();
    auto iter = m_iconToImageMap.find(nIconId);
    if (iter != m_iconToImageMap.end()) {
        int32_t nImageId = iter->second;
        m_iconToImageMap.erase(iter);

        ui::ImageListPtr pImageList;
        if (m_pListCtrl != nullptr) {
            pImageList = m_pListCtrl->GetImageList(ui::ListCtrlType::Report);
        }
        if (pImageList != nullptr) {
            pImageList->RemoveImageString(nImageId);
        }
    }
}

void ExplorerView::InitViewHeader()
{
    if (m_pListCtrl == nullptr) {
        return;
    }
    ui::ListCtrlHeaderItem* pHeaderItem = nullptr;
    ui::ListCtrlColumn columnInfo;
    columnInfo.textId = _T("STRID_TREEVIEW_EXPLORER_FILE_NAME");//文件名称
    columnInfo.nColumnWidth = 360;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kName] = pHeaderItem->GetColumnId();

    columnInfo.textId = _T("STRID_TREEVIEW_EXPLORER_MODIFY_DATE");//修改日期
    columnInfo.nColumnWidth = 160;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kModifyDateTime] = pHeaderItem->GetColumnId();

    columnInfo.textId = _T("STRID_TREEVIEW_EXPLORER_FILE_TYPE");//文件类型
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kType] = pHeaderItem->GetColumnId();

    columnInfo.textId = _T("STRID_TREEVIEW_EXPLORER_FILE_SIZE");//文件大小
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kSize] = pHeaderItem->GetColumnId();
}

size_t ExplorerView::GetColumnId(ExplorerViewColumn nOriginIndex) const
{
    size_t nColumnIndex = ui::Box::InvalidIndex;
    auto iter = m_columnIdMap.find(nOriginIndex);
    if (iter != m_columnIdMap.end()) {
        return iter->second;
    }
    return nColumnIndex;
}

void ExplorerView::SetFileList(const ui::FilePath& currentPath, std::vector<PathInfo> pathList, const ui::FilePath& selectedPath,
                               const std::vector<DString>& searchKeywords)
{
    //在主线程中执行
    ui::GlobalManager::Instance().AssertUIThread();
    ASSERT(m_pMainForm != nullptr);
    ASSERT(m_pListCtrl != nullptr);
    if ((m_pMainForm == nullptr) || (m_pListCtrl == nullptr)) {
        ui::DirectoryTree::ClearPathInfoList(m_pathInfoList);
        m_pathInfoList = std::move(pathList);
        return;
    }
    m_currentPath = currentPath;

    //保存旧的文件列表（用于按路径比较，识别可以复用的列表项），新列表替换原有列表
    std::vector<PathInfo> oldPathInfoList;
    oldPathInfoList.swap(m_pathInfoList);
    //移动数据，避免大量路径信息的深拷贝
    m_pathInfoList = std::move(pathList);

    //首先禁止刷新（避免每条数据更新均刷新，导致文件多的时候速度很慢）
    bool bOldEnableRefresh = m_pListCtrl->SetEnableRefresh(false);

    //各列的列ID与排序方式固定不变，在循环外设置一次即可
    const size_t nNameColumnId = GetColumnId(ExplorerViewColumn::kName);
    const size_t nTimeColumnId = GetColumnId(ExplorerViewColumn::kModifyDateTime);
    const size_t nTypeColumnId = GetColumnId(ExplorerViewColumn::kType);
    const size_t nSizeColumnId = GetColumnId(ExplorerViewColumn::kSize);
    uint8_t nNameSortFlag = ui::kSortByGroup;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows下，文件名排序不区分大小写
    nNameSortFlag |= ui::kSortNoCase;
#endif
    m_pListCtrl->SetColumnSortFlagById(nNameColumnId, nNameSortFlag);
    m_pListCtrl->SetColumnSortFlagById(nTimeColumnId, ui::kSortByUserDataN | ui::kSortByGroup);
    m_pListCtrl->SetColumnSortFlagById(nTypeColumnId, ui::kSortByGroup);
    m_pListCtrl->SetColumnSortFlagById(nSizeColumnId, ui::kSortByUserDataN | ui::kSortByGroup);

    //"文件路径 -> 新列表下标"的映射表：用于按路径做增量更新
    std::map<DString, size_t> pathToNewIndex;
    for (size_t nIndex = 0; nIndex < m_pathInfoList.size(); ++nIndex) {
        pathToNewIndex[m_pathInfoList[nIndex].m_filePath.ToString()] = nIndex;
    }
    //"新列表下标 -> 数据项索引号"的映射表（用于定位需要选择的列表项）
    std::vector<size_t> newIndexToItem(m_pathInfoList.size(), ui::Box::InvalidIndex);

    //图标列表在多次刷新之间复用，避免每次切换目录都重新解码所有图标；仅当缓存过大时才整体清空
    const size_t kMaxIconCacheSize = 1024;
    const size_t nOldItemCount = m_pListCtrl->GetDataItemCount();
    ui::ImageListPtr pImageList = m_pListCtrl->GetImageList(m_pListCtrl->GetListCtrlType());
    bool bRebuildAllRows = false;
    if ((pImageList != nullptr) && (nOldItemCount > 0) && (m_iconToImageMap.size() > kMaxIconCacheSize)) {
        //图标缓存过多：整体清空后重建，避免占用过多的内存
        pImageList->Clear();
        //图片列表已清空，之前的IconID与ImageID的映射关系已经失效
        m_iconToImageMap.clear();
        bRebuildAllRows = true;
    }

    //刷新一个列表项的内容：pOldInfo为空表示新增的列表项（所有列都需要设置）
    ui::ListCtrlSubItemData itemData;
    auto updateDataItem = [&](size_t nItemIndex, size_t nIndex, const PathInfo* pOldInfo) {
        const PathInfo& pathInfo = m_pathInfoList[nIndex];
        //记录数据项与路径列表的关联关系
        m_pListCtrl->SetDataItemUserData(nItemIndex, nIndex);
        newIndexToItem[nIndex] = nItemIndex;

        //设置图标：同一个图标ID只加载一次
        if ((pOldInfo != nullptr) && (pOldInfo->m_nIconID == pathInfo.m_nIconID)) {
            //图标没有变化，无需更新
        }
        else if (pImageList != nullptr) {
            auto iter = m_iconToImageMap.find(pathInfo.m_nIconID);
            if (iter != m_iconToImageMap.end()) {
                m_pListCtrl->SetDataItemImageId(nItemIndex, iter->second);
            }
            else {
                DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(pathInfo.m_nIconID);
                if (!iconString.empty()) {
                    int32_t nImageId = pImageList->AddImageStringWithSize(iconString, m_pMainForm->Dpi());
                    m_iconToImageMap[pathInfo.m_nIconID] = nImageId;
                    m_pListCtrl->SetDataItemImageId(nItemIndex, nImageId);
                }
                else {
                    m_pListCtrl->SetDataItemImageId(nItemIndex, -1);
                }
            }
        }

        ui::ListCtrlSubItemData subItemData;
        subItemData.nSortGroup = pathInfo.m_bFolder ? SortGroup::kFolder : SortGroup::kFile; //排序分组
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;

        //文件名称列
        if ((pOldInfo == nullptr) || (pOldInfo->m_displayName != pathInfo.m_displayName) || (pOldInfo->m_bFolder != pathInfo.m_bFolder)) {
            subItemData.text = pathInfo.m_displayName;
            m_pListCtrl->SetSubItemDataById(nItemIndex, nNameColumnId, subItemData);
        }

        //修改日期列
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileTime(pathInfo.m_lastWriteTime);
        if ((pOldInfo == nullptr) || (pOldInfo->m_lastWriteTime.GetValue() != pathInfo.m_lastWriteTime.GetValue())) {
            m_pListCtrl->SetSubItemDataById(nItemIndex, nTimeColumnId, subItemData);
            //设置"修改日期"列的排序方式
            m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nTimeColumnId, pathInfo.m_lastWriteTime.GetValue());
        }

        //文件类型列
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = pathInfo.m_typeName;
        if ((pOldInfo == nullptr) || (pOldInfo->m_typeName != pathInfo.m_typeName)) {
            m_pListCtrl->SetSubItemDataById(nItemIndex, nTypeColumnId, subItemData);
        }

        //文件大小列
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_RIGHT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileSize(pathInfo.m_bFolder, pathInfo.m_fileSize);
        if ((pOldInfo == nullptr) || (pOldInfo->m_fileSize != pathInfo.m_fileSize) || (pOldInfo->m_bFolder != pathInfo.m_bFolder)) {
            m_pListCtrl->SetSubItemDataById(nItemIndex, nSizeColumnId, subItemData);
            //设置"文件大小"列的排序方式
            m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nSizeColumnId, pathInfo.m_fileSize);
        }
    };

    //统计旧列表中可以直接复用的列表项（按文件路径匹配）
    std::vector<size_t> removedItems;
    size_t nKeptCount = 0;
    if (!bRebuildAllRows) {
        removedItems.reserve(nOldItemCount);
        for (size_t nItemIndex = 0; nItemIndex < nOldItemCount; ++nItemIndex) {
            const size_t nOldIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
            const bool bKept = (nOldIndex < oldPathInfoList.size()) &&
                (pathToNewIndex.find(oldPathInfoList[nOldIndex].m_filePath.ToString()) != pathToNewIndex.end());
            if (bKept) {
                ++nKeptCount;
            }
            else {
                removedItems.push_back(nItemIndex);
            }
        }
        if ((nOldItemCount > 0) && (nKeptCount == 0)) {
            //没有可复用的列表项（如切换目录），按新的文件列表重建所有行
            bRebuildAllRows = true;
        }
        else if ((nKeptCount > 0) && (nKeptCount * 4 < m_pathInfoList.size())) {
            //可复用的比例过低时，增量更新的插入开销高于重建所有行，按新的文件列表重建所有行
            bRebuildAllRows = true;
        }
    }

    //按文件列表的顺序重建列表内容：复用已有的列表项（就地重写每行的内容），
    //只增、删差异的行，避免"删除全部列表项后逐条添加"带来的行数据分配与释放开销
    auto rebuildRowsInOrder = [&]() {
        removedItems.clear();
        //已重建的行会在下面重新填充索引映射，这里先清空（避免选择恢复时用到已失效的索引）
        std::fill(newIndexToItem.begin(), newIndexToItem.end(), ui::Box::InvalidIndex);

        const size_t nItemCount = m_pListCtrl->GetDataItemCount();
        const size_t nNewCount = m_pathInfoList.size();
        //1. 复用已有的列表项：第i行直接显示新列表中的第i项
        const size_t nReuseCount = (std::min)(nItemCount, nNewCount);
        for (size_t nIndex = 0; nIndex < nReuseCount; ++nIndex) {
            updateDataItem(nIndex, nIndex, nullptr);
        }
        //2. 行数多于新列表时，删除末尾多余的列表项；行数少于新列表时，为新增的条目添加列表项
        if (nItemCount > nNewCount) {
            std::vector<size_t> surplusItems;
            surplusItems.reserve(nItemCount - nNewCount);
            for (size_t nItemIndex = nNewCount; nItemIndex < nItemCount; ++nItemIndex) {
                surplusItems.push_back(nItemIndex);
            }
            m_pListCtrl->DeleteDataItems(surplusItems);
        }
        else {
            for (size_t nIndex = nItemCount; nIndex < nNewCount; ++nIndex) {
                const size_t nItemIndex = m_pListCtrl->AddDataItem(itemData);
                if (!ui::Box::IsValidItemIndex(nItemIndex)) {
                    continue;
                }
                updateDataItem(nItemIndex, nIndex, nullptr);
            }
        }
        //3. 保持用户选择的排序方式（列表处于排序状态时，显示顺序与文件列表的顺序不同）
        size_t nSortColumnId = ui::Box::InvalidIndex;
        bool bSortUp = false;
        if (m_pListCtrl->GetSortColumn(nSortColumnId, bSortUp)) {
            const uint8_t nSortFlag = m_pListCtrl->GetColumnSortFlagById(nSortColumnId);
            m_pListCtrl->SortDataItemsById(nSortColumnId, bSortUp, nSortFlag);
        }
    };

    if (bRebuildAllRows) {
        rebuildRowsInOrder();
    }
    else {
        //增量更新：先删除已经不存在于新列表中的列表项（删除后，剩余列表项的索引号会前移）
        if (!removedItems.empty()) {
            m_pListCtrl->DeleteDataItems(removedItems);
        }

        //重新建立"数据项索引号 -> 新列表下标"的关联，并刷新内容有变化的列
        std::vector<uint8_t> usedNewIndex(m_pathInfoList.size(), 0);
        std::vector<size_t> keptNewIndexList;           //已复用的列表项，按显示顺序记录其对应的新列表下标
        const size_t nItemCount = m_pListCtrl->GetDataItemCount();
        keptNewIndexList.reserve(nItemCount);
        for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
            const size_t nOldIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
            if (nOldIndex >= oldPathInfoList.size()) {
                continue;
            }
            auto iter = pathToNewIndex.find(oldPathInfoList[nOldIndex].m_filePath.ToString());
            if (iter == pathToNewIndex.end()) {
                continue;
            }
            const size_t nNewIndex = iter->second;
            if ((nNewIndex >= usedNewIndex.size()) || (usedNewIndex[nNewIndex] != 0)) {
                //异常数据（路径重复）：不在此处处理
                continue;
            }
            usedNewIndex[nNewIndex] = 1;
            keptNewIndexList.push_back(nNewIndex);
            updateDataItem(nItemIndex, nNewIndex, &oldPathInfoList[nOldIndex]);
        }

        //判断列表当前是否处于排序状态（用户点击表头排序后，显示顺序与文件列表的顺序不同）
        size_t nSortColumnId = ui::Box::InvalidIndex;
        bool bSortUp = false;
        const bool bSorted = m_pListCtrl->GetSortColumn(nSortColumnId, bSortUp);

        //判断已复用的列表项在新列表中的顺序是否保持递增：递增时才能按位置插入新增的列表项
        bool bKeptOrdered = true;
        if (!bSorted) {
            for (size_t nIndex = 1; nIndex < keptNewIndexList.size(); ++nIndex) {
                if (keptNewIndexList[nIndex] < keptNewIndexList[nIndex - 1]) {
                    bKeptOrdered = false;
                    break;
                }
            }
        }
        if (bSorted) {
            //保持用户选择的排序方式：先把新增的列表项追加到末尾，再对完整数据重新排序
            for (size_t nNewIndex = 0; nNewIndex < m_pathInfoList.size(); ++nNewIndex) {
                if (usedNewIndex[nNewIndex] != 0) {
                    continue;
                }
                const size_t nItemIndex = m_pListCtrl->AddDataItem(itemData);
                if (!ui::Box::IsValidItemIndex(nItemIndex)) {
                    continue;
                }
                updateDataItem(nItemIndex, nNewIndex, nullptr);
            }
            const uint8_t nSortFlag = m_pListCtrl->GetColumnSortFlagById(nSortColumnId);
            m_pListCtrl->SortDataItemsById(nSortColumnId, bSortUp, nSortFlag);
        }
        else if (!bKeptOrdered) {
            //出现乱序：按文件列表的顺序重写所有行（复用已有的列表项，保证显示顺序与文件列表一致）
            rebuildRowsInOrder();
        }
        else {
            //按文件列表的顺序，把新增的列表项插入到对应的显示位置
            size_t nInsertPos = 0;      //插入位置（显示顺序）
            for (size_t nNewIndex = 0; nNewIndex < m_pathInfoList.size(); ++nNewIndex) {
                if (usedNewIndex[nNewIndex] != 0) {
                    ++nInsertPos;
                    continue;
                }
                if (!m_pListCtrl->InsertDataItem(nInsertPos, itemData)) {
                    continue;
                }
                updateDataItem(nInsertPos, nNewIndex, nullptr);
                ++nInsertPos;
            }
        }
    }

    //应用视觉效果：斑马纹背景、搜索命中高亮
    ApplyRowStyle(searchKeywords);

    m_pListCtrl->SetEnableRefresh(bOldEnableRefresh);
    m_pListCtrl->Refresh(true);

    //恢复选择状态：只选择selectedPath对应的列表项，其余选择状态清除（避免保留已失效的选择）
    size_t nSelectedItemIndex = ui::Box::InvalidIndex;
    if (!selectedPath.IsEmpty()) {
        auto iter = pathToNewIndex.find(selectedPath.ToString());
        if (iter != pathToNewIndex.end()) {
            nSelectedItemIndex = newIndexToItem[iter->second];
        }
    }
    std::vector<size_t> selectedItemIndexs;
    m_pListCtrl->GetSelectedDataItems(selectedItemIndexs);
    for (size_t nItemIndex : selectedItemIndexs) {
        if (nItemIndex != nSelectedItemIndex) {
            m_pListCtrl->SetDataItemSelected(nItemIndex, false);
        }
    }
    if (nSelectedItemIndex != ui::Box::InvalidIndex) {
        m_pListCtrl->SetDataItemSelected(nSelectedItemIndex, true);
        m_pListCtrl->EnsureDataItemVisible(nSelectedItemIndex, false);
    }

    //旧的文件列表不再使用，释放其占用的图标资源（共享的图标资源由目录内容缓存负责释放）
    ui::DirectoryTree::ClearPathInfoList(oldPathInfoList);
}

void ExplorerView::GetCurrentPath(ui::FilePath& currentPath, ui::FilePath& selectedPath) const
{
    currentPath = m_currentPath;
    if (m_pListCtrl != nullptr) {
        std::vector<size_t> itemIndexs;
        m_pListCtrl->GetSelectedDataItems(itemIndexs);
        for (size_t nItemIndex : itemIndexs) {
            size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
            if (nIndex < m_pathInfoList.size()) {
                const ui::DirectoryTree::PathInfo& pathInfo = m_pathInfoList[nIndex];
                if (!pathInfo.m_filePath.IsEmpty()) {
                    //记录当前所选的目录
                    selectedPath = pathInfo.m_filePath;
                    break;
                }
            }
        }
    }       
}

ui::FilePath ExplorerView::GetSelectedPath() const
{
    std::vector<ui::FilePath> paths;
    GetSelectedPaths(paths);
    if (!paths.empty()) {
        return paths.front();
    }
    return ui::FilePath();
}

void ExplorerView::GetSelectedPaths(std::vector<ui::FilePath>& paths) const
{
    paths.clear();
    if (m_pListCtrl == nullptr) {
        return;
    }
    std::vector<size_t> itemIndexs;
    m_pListCtrl->GetSelectedDataItems(itemIndexs);
    for (size_t nItemIndex : itemIndexs) {
        size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex < m_pathInfoList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathInfoList[nIndex];
            if (!pathInfo.m_filePath.IsEmpty()) {
                paths.push_back(pathInfo.m_filePath);
            }
        }
    }
}

void ExplorerView::GetSelectedItemInfo(int32_t& nSelectedCount, uint64_t& nSelectedFileSize) const
{
    nSelectedCount = 0;
    nSelectedFileSize = 0;
    if (m_pListCtrl == nullptr) {
        return;
    }
    std::vector<size_t> itemIndexs;
    m_pListCtrl->GetSelectedDataItems(itemIndexs);
    for (size_t nItemIndex : itemIndexs) {
        size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex < m_pathInfoList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathInfoList[nIndex];
            ++nSelectedCount;
            if (!pathInfo.m_bFolder) {
                nSelectedFileSize += pathInfo.m_fileSize;
            }
        }
    }
}

void ExplorerView::SelectAll()
{
    if (m_pListCtrl != nullptr) {
        m_pListCtrl->SetSelectAll();
    }
}

ui::UiColor ExplorerView::GetStripeColor() const
{
    //斑马纹的背景色：由主题定义（半透明色，不覆盖选中行/悬停行的状态底色）
    ui::UiColor stripeColor = ui::GlobalManager::Instance().Color().GetColor(_T("bg_list_ctrl_item_stripe"));
    if (!stripeColor.IsEmpty()) {
        return stripeColor;
    }
    //主题中没有定义该颜色时，由列表项的悬停背景色派生（自动适配深色/浅色主题）
    const ui::UiColor hoveredColor = ui::GlobalManager::Instance().Color().GetColor(_T("bg_list_ctrl_item_hovered"));
    if (hoveredColor.IsEmpty()) {
        return ui::UiColor();
    }
    return ui::UiColor(32, hoveredColor.GetR(), hoveredColor.GetG(), hoveredColor.GetB());
}

void ExplorerView::ApplyRowStyle(const std::vector<DString>& searchKeywords)
{
    if (m_pListCtrl == nullptr) {
        return;
    }
    //视觉效果仅在"详细信息"视图中生效：图标视图和列表视图显示的是名称列的纯文本，不适合显示高亮色
    const bool bReportView = (m_pListCtrl->GetListCtrlType() == ui::ListCtrlType::Report);
    const size_t nNameColumnId = GetColumnId(ExplorerViewColumn::kName);
    const size_t columnIds[] = { nNameColumnId,
                                 GetColumnId(ExplorerViewColumn::kModifyDateTime),
                                 GetColumnId(ExplorerViewColumn::kType),
                                 GetColumnId(ExplorerViewColumn::kSize) };

    //斑马纹背景色（仅详细信息视图）
    const ui::UiColor stripeColor = bReportView ? GetStripeColor() : ui::UiColor();
    //搜索命中高亮色（仅详细信息视图，并且搜索过滤生效时才启用）
    ui::UiColor highlightColor;
    if (bReportView && !searchKeywords.empty()) {
        highlightColor = ui::GlobalManager::Instance().Color().GetColor(_T("color_accent"));
    }

    //预先计算每个路径的"文件名称"是否命中搜索关键字（关键字已转换为小写，多个关键字之间是"或"的关系）
    std::vector<uint8_t> nameHits(m_pathInfoList.size(), 0);
    if (!highlightColor.IsEmpty()) {
        for (size_t nIndex = 0; nIndex < m_pathInfoList.size(); ++nIndex) {
            const DString displayName = ui::StringUtil::MakeLowerString(m_pathInfoList[nIndex].m_displayName);
            for (const DString& keyword : searchKeywords) {
                if (!keyword.empty() && (displayName.find(keyword) != DString::npos)) {
                    nameHits[nIndex] = 1;
                    break;
                }
            }
        }
    }

    //逐行应用视觉效果
    const size_t nItemCount = m_pListCtrl->GetDataItemCount();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        const size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex >= m_pathInfoList.size()) {
            continue;
        }
        //斑马纹：按行的显示顺序（奇数行显示背景色）
        const ui::UiColor rowBkColor = ((nItemIndex % 2) == 1) ? stripeColor : ui::UiColor();
        for (size_t nColumnId : columnIds) {
            m_pListCtrl->SetSubItemBkColorById(nItemIndex, nColumnId, rowBkColor);
        }
        //搜索命中高亮：命中的文件名称使用高亮色显示
        if (!highlightColor.IsEmpty() && (nameHits[nIndex] != 0)) {
            m_pListCtrl->SetSubItemTextColorById(nItemIndex, nNameColumnId, highlightColor);
        }
        else {
            m_pListCtrl->SetSubItemTextColorById(nItemIndex, nNameColumnId, ui::UiColor());
        }
    }
}

DString ExplorerView::FormatFileSize(bool bFolder, uint64_t nFileSize)
{
    DString value;
    if (bFolder) {
        return value;
    }
    if (nFileSize > 1 * 1024 * 1024 * 1024) {
        //GB
        double total_gb = static_cast<double>(nFileSize) / (1024 * 1024 * 1024);
        value = ui::StringUtil::Printf(_T("%.01lf GB"), total_gb);
    }
    else if (nFileSize > 1 * 1024 * 1024) {
        //MB
        double total_mb = static_cast<double>(nFileSize) / (1024 * 1024);
        value = ui::StringUtil::Printf(_T("%.01lf MB"), total_mb);
    }
    else if (nFileSize > 1 * 1024) {
        //KB
        double total_kb = static_cast<double>(nFileSize) / (1024);
        value = ui::StringUtil::Printf(_T("%.01lf KB"), total_kb);
    }
    else if (nFileSize == 0) {
        value = _T("0");
    }
    else {
        //B
        value = ui::StringUtil::Printf(_T("%d B"), (int32_t)nFileSize);
    }
    return value;
}

DString ExplorerView::FormatFileTime(const ui::FileTime& fileTime) const
{
    return fileTime.ToString();
}
