#include "FileInfoList.h"
#include "FileInfoItem.h"
#include "MainForm.h"
#include "MainThread.h"

FileInfoList::FileInfoList(MainForm* pMainForm):
    m_pMainForm(pMainForm),
    m_pTreeNode(nullptr)
{
}

FileInfoList::~FileInfoList()
{
    ClearFileList(m_pathList);
}

ui::Control* FileInfoList::CreateElement(ui::VirtualListBox* pVirtualListBox)
{
    ASSERT(pVirtualListBox != nullptr);
    if (pVirtualListBox == nullptr) {
        return nullptr;
    }
    ASSERT(pVirtualListBox->GetWindow() != nullptr);
    FileInfoItem* item = new FileInfoItem(pVirtualListBox->GetWindow());
    item->AttachEvent(ui::kEventMouseDoubleClick, UiBind(&FileInfoList::OnDoubleClickItem, this, std::placeholders::_1));
    ui::GlobalManager::Instance().FillBoxWithCache(item, ui::FilePath(_T("tree_view/tree_node.xml")));
    return item;
}

bool FileInfoList::FillElement(ui::Control* pControl, size_t nElementIndex)
{
    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(pControl);
    ASSERT(pItem != nullptr);
    ASSERT(nElementIndex < m_pathList.size());
    if ((pItem == nullptr) || (nElementIndex >= m_pathList.size())) {
        return false;
    }
    const PathInfo& fileInfo = m_pathList[nElementIndex];
    pItem->InitSubControls(fileInfo, nElementIndex);
    pItem->SetUserDataID(nElementIndex);
    return true;
}

size_t FileInfoList::GetElementCount() const
{
    return m_pathList.size();
}

void FileInfoList::SetElementSelected(size_t nElementIndex, bool bSelected)
{

}

bool FileInfoList::IsElementSelected(size_t nElementIndex) const
{
    return false;
}

void FileInfoList::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
}

bool FileInfoList::IsMultiSelect() const
{
    return false;
}

void FileInfoList::SetMultiSelect(bool /*bMultiSelect*/)
{
}

void FileInfoList::SetFileList(ui::TreeNode* pTreeNode, const std::vector<PathInfo>& pathList)
{
    std::vector<PathInfo> oldPathList;
    oldPathList.swap(m_pathList);
    
    m_pTreeNode = pTreeNode;
    m_pathList.reserve(m_pathList.size() + pathList.size());
    for (const PathInfo& fileInfo : pathList) {
        m_pathList.emplace_back(fileInfo);
    }
    // 通知ListBox数据总数变动
    EmitCountChanged();

    //清理原来的图标资源
    ClearFileList(oldPathList);
}

void FileInfoList::ClearFileList(std::vector<PathInfo>& pathList) const
{
    for (const PathInfo& fileInfo : pathList) {
        if (!fileInfo.m_bIconShared) {
            ui::GlobalManager::Instance().Icon().RemoveIcon(fileInfo.m_nIconID);
        }        
    }
    pathList.clear();
}

bool FileInfoList::OnDoubleClickItem(const ui::EventArgs& args)
{
    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(args.GetSender());
    if (pItem != nullptr) {
        size_t nElementIndex = pItem->GetUserDataID();
        if (nElementIndex < m_pathList.size()) {
            const PathInfo& fileInfo = m_pathList[nElementIndex];
            if (fileInfo.m_bFolder && (m_pMainForm != nullptr)) {
                //双击在一个目录上
                m_pMainForm->CheckExpandTreeNode(m_pTreeNode, fileInfo.m_filePath);
            }
        }
    }
    return true;
}

ui::TreeNode* FileInfoList::GetTreeNode() const
{
    return m_pTreeNode;
}
