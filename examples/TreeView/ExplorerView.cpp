#include "ExplorerView.h"
#include "MainForm.h"

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
    bool bRet = false;
    size_t nSortColumnId = ui::Box::InvalidIndex;
    if (m_pListCtrl->GetSortColumn(nSortColumnId, bSortUp)) {
        bRet = true;
        size_t nColumnIndex = m_pListCtrl->GetColumnIndex(nSortColumnId);
        if (nColumnIndex == (size_t)ExplorerViewColumn::kName) {
            viewColumn = ExplorerViewColumn::kName;
        }
        else if (nColumnIndex == (size_t)ExplorerViewColumn::kModifyDateTime) {
            viewColumn = ExplorerViewColumn::kModifyDateTime;
        }
        else if (nColumnIndex == (size_t)ExplorerViewColumn::kType) {
            viewColumn = ExplorerViewColumn::kType;
        }
        else if (nColumnIndex == (size_t)ExplorerViewColumn::kSize) {
            viewColumn = ExplorerViewColumn::kSize;
        }
        else {
            bRet = false;
        }
    }
    return bRet;
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
    }
}

bool ExplorerView::OnExplorerViewDoubleClick(const ui::EventArgs& msg)
{
    if ((m_pListCtrl != nullptr) && (msg.wParam != 0) && (m_pMainForm != nullptr)) {
        size_t nItemIndex = msg.lParam;
        size_t nIndex = m_pListCtrl->GetDataItemUserData(nItemIndex);
        if (nIndex < m_pathInfoList.size()) {
            const ui::DirectoryTree::PathInfo& pathInfo = m_pathInfoList[nIndex];
            if (!pathInfo.m_filePath.IsEmpty() && pathInfo.m_filePath.IsExistsDirectory()) {
                //进入所选的目录
                m_pMainForm->SelectSubPath(pathInfo.m_filePath);
            }
        }
    }
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
    columnInfo.text = _T("文件名称");
    columnInfo.nColumnWidth = 360;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kName] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("修改日期");
    columnInfo.nColumnWidth = 160;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kModifyDateTime] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("文件类型");
    columnInfo.nColumnWidth = 120;
    pHeaderItem = m_pListCtrl->InsertColumn(-1, columnInfo);
    ASSERT(pHeaderItem != nullptr);
    m_columnIdMap[ExplorerViewColumn::kType] = pHeaderItem->GetColumnId();

    columnInfo.text = _T("文件大小");
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

void ExplorerView::SetFileList(const ui::FilePath& currentPath, const std::vector<PathInfo>& pathList, const ui::FilePath& selectedPath)
{
    //在主线程中执行
    ui::GlobalManager::Instance().AssertUIThread();
    ui::DirectoryTree::ClearPathInfoList(m_pathInfoList);
    m_pathInfoList = pathList;
    ASSERT(m_pMainForm != nullptr);
    ASSERT(m_pListCtrl != nullptr);
    if ((m_pMainForm == nullptr) || (m_pListCtrl == nullptr)) {
        return;
    }
    m_currentPath = currentPath;
    //首先禁止刷新（避免每条数据更新均刷新，导致文件多的时候速度很慢）
    bool bOldEnableRefresh = m_pListCtrl->SetEnableRefresh(false);
    m_pListCtrl->DeleteAllDataItems();
    ui::ImageListPtr pImageList;
    if (m_pListCtrl != nullptr) {
        pImageList = m_pListCtrl->GetImageList(m_pListCtrl->GetListCtrlType());
    }
    if (pImageList != nullptr) {
        pImageList->Clear();
    }

    //选择的子项元素索引号
    size_t nSelectedItemIndex = ui::Box::InvalidIndex;

    ui::ListCtrlSubItemData itemData;
    size_t nItemIndex = 0;
    size_t nColumnId = 0;
    for (size_t nIndex = 0; nIndex < pathList.size(); ++nIndex) {
        const ui::DirectoryTree::PathInfo& pathInfo = pathList[nIndex];
        nItemIndex = m_pListCtrl->AddDataItem(itemData);
        if (!ui::Box::IsValidItemIndex(nItemIndex)) {
            continue;
        }
        //记录关联关系
        m_pListCtrl->SetDataItemUserData(nItemIndex, nIndex);

        if ((nSelectedItemIndex == ui::Box::InvalidIndex) && !selectedPath.IsEmpty() && (selectedPath == pathInfo.m_filePath)) {
            //保存选择状态
            nSelectedItemIndex = nItemIndex;
        }

        //设置图标
        if (pImageList != nullptr) {
            DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(pathInfo.m_nIconID);
            if (!iconString.empty()) {
                int32_t nImageId = pImageList->AddImageStringWithSize(iconString, m_pMainForm->Dpi());
                m_iconToImageMap[pathInfo.m_nIconID] = nImageId;
                m_pListCtrl->SetDataItemImageId(nItemIndex, nImageId);
            }
        }
        ui::ListCtrlSubItemData subItemData;
        subItemData.nSortGroup = pathInfo.m_bFolder ? SortGroup::kFolder : SortGroup::kFile; //排序分组
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = pathInfo.m_displayName;
        nColumnId = GetColumnId(ExplorerViewColumn::kName);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件名称

        //设置"文件名称"列的排序方式
        uint8_t nSortFlag = ui::kSortByGroup;
#ifdef DUILIB_BUILD_FOR_WIN
        //Windows下，文件名排序不区分大小写
        nSortFlag |= ui::kSortNoCase;
#endif
        m_pListCtrl->SetColumnSortFlagById(nColumnId, nSortFlag);

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileTime(pathInfo.m_lastWriteTime);
        nColumnId = GetColumnId(ExplorerViewColumn::kModifyDateTime);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //修改日期

        //设置"修改日期"列的排序方式
        m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nColumnId, pathInfo.m_lastWriteTime.GetValue());
        m_pListCtrl->SetColumnSortFlagById(nColumnId, ui::kSortByUserDataN | ui::kSortByGroup);

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = pathInfo.m_typeName;
        nColumnId = GetColumnId(ExplorerViewColumn::kType);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件类型
        m_pListCtrl->SetColumnSortFlagById(nColumnId, ui::kSortByGroup);

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_RIGHT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileSize(pathInfo.m_bFolder, pathInfo.m_fileSize);
        nColumnId = GetColumnId(ExplorerViewColumn::kSize);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件大小

        //设置"文件大小"列的排序方式
        m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nColumnId, pathInfo.m_fileSize);
        m_pListCtrl->SetColumnSortFlagById(nColumnId, ui::kSortByUserDataN | ui::kSortByGroup);
    }

    m_pListCtrl->SetEnableRefresh(bOldEnableRefresh);
    m_pListCtrl->Refresh(true);

    if (nSelectedItemIndex != ui::Box::InvalidIndex) {
        m_pListCtrl->SetDataItemSelected(nSelectedItemIndex, true);
        m_pListCtrl->EnsureDataItemVisible(nSelectedItemIndex, false);
    }
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

DString ExplorerView::FormatFileSize(bool bFolder, uint64_t nFileSize) const
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
