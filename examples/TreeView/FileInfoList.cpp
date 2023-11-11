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
    ClearFileList();
}

ui::Control* FileInfoList::CreateElement()
{
    FileInfoItem* item = new FileInfoItem;
    item->AttachEvent(ui::kEventMouseDoubleClick, nbase::Bind(&FileInfoList::OnDoubleClickItem, this, std::placeholders::_1));
    ui::GlobalManager::Instance().FillBoxWithCache(item, L"tree_view/tree_node.xml");
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
    const FileInfo& fileInfo = m_pathList[nElementIndex];
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

void FileInfoList::SetFileList(ui::TreeNode* pTreeNode, const std::vector<FileInfo>& pathList)
{
    ClearFileList();
    m_pTreeNode = pTreeNode;
    m_pathList.reserve(m_pathList.size() + pathList.size());
    for (const FileInfo& fileInfo : pathList) {
        m_pathList.emplace_back(fileInfo);
        ui::GlobalManager::Instance().Icon().AddIcon(fileInfo.m_hIcon);
    }
    // 通知ListBox数据总数变动
    EmitCountChanged();
}

void FileInfoList::ClearFileList()
{
    for (const FileInfo& fileInfo : m_pathList) {
        if (fileInfo.m_hIcon != nullptr) {
            ui::GlobalManager::Instance().Icon().RemoveIcon(fileInfo.m_hIcon);
            ::DestroyIcon(fileInfo.m_hIcon);
        }
    }
    m_pathList.clear();
}

bool FileInfoList::OnDoubleClickItem(const ui::EventArgs& args)
{
    FileInfoItem* pItem = dynamic_cast<FileInfoItem*>(args.pSender);
    if (pItem != nullptr) {
        size_t nElementIndex = pItem->GetUserDataID();
        if (nElementIndex < m_pathList.size()) {
            const FileInfo& fileInfo = m_pathList[nElementIndex];
            if (fileInfo.m_isFolder && (m_pMainForm != nullptr)) {
                //双击在一个目录上
                m_pMainForm->CheckExpandTreeNode(m_pTreeNode, fileInfo.m_filePath);
            }
        }
    }
    return true;
}