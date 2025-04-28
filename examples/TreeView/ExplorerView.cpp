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
        //关联图片列表
        ui::ImageListPtr pImageList = std::make_shared<ui::ImageList>();
        pImageList->SetImageSize(ui::UiSize(20, 20), m_pMainForm->Dpi(), true);
        m_pListCtrl->SetImageList(ui::ListCtrlType::Report, pImageList);//该指针的资源生命周期由ListCtrl内部管理

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

void ExplorerView::SetFileList(const std::vector<PathInfo>& pathList)
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
    m_pListCtrl->DeleteAllDataItems();
    ui::ImageListPtr pImageList;
    if (m_pListCtrl != nullptr) {
        pImageList = m_pListCtrl->GetImageList(ui::ListCtrlType::Report);
    }
    if (pImageList != nullptr) {
        pImageList->Clear();
    }

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
        //设置图标
        if (pImageList != nullptr) {
            DString iconString = ui::GlobalManager::Instance().Icon().GetIconString(pathInfo.m_nIconID);
            if (!iconString.empty()) {
                int32_t nImageId = pImageList->AddImageString(iconString, m_pMainForm->Dpi());
                m_iconToImageMap[pathInfo.m_nIconID] = nImageId;
                m_pListCtrl->SetDataItemImageId(nItemIndex, nImageId);
            }
        }

        ui::ListCtrlSubItemData subItemData;
        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = pathInfo.m_displayName;
        nColumnId = GetColumnId(ExplorerViewColumn::kName);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件名称

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileTime(pathInfo.m_lastWriteTime);
        nColumnId = GetColumnId(ExplorerViewColumn::kModifyDateTime);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //修改日期

        //设置"文件大小"列的排序方式
        m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nColumnId, pathInfo.m_lastWriteTime.GetValue());
        m_pListCtrl->SetColumnSortFlagById(nColumnId, ui::ListCtrlSubItemSortFlag::kSortByUserDataN);

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = pathInfo.m_typeName;
        nColumnId = GetColumnId(ExplorerViewColumn::kType);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件类型

        subItemData.nTextFormat = ui::DrawStringFormat::TEXT_RIGHT | ui::DrawStringFormat::TEXT_VCENTER;
        subItemData.text = FormatFileSize(pathInfo.m_bFolder, pathInfo.m_fileSize);
        nColumnId = GetColumnId(ExplorerViewColumn::kSize);
        m_pListCtrl->SetSubItemDataById(nItemIndex, nColumnId, subItemData); //文件大小

        //设置"文件大小"列的排序方式
        m_pListCtrl->SetSubItemUserDataNById(nItemIndex, nColumnId, pathInfo.m_fileSize);
        m_pListCtrl->SetColumnSortFlagById(nColumnId, ui::ListCtrlSubItemSortFlag::kSortByUserDataN);
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

