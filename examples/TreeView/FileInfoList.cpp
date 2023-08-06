#include "FileInfoList.h"
#include "FileInfoItem.h"

FileInfoList::FileInfoList()
{
}

FileInfoList::~FileInfoList()
{
    ClearFileList();
}

ui::Control* FileInfoList::CreateElement()
{
    FileInfoItem* item = new FileInfoItem;
    ui::GlobalManager::Instance().FillBoxWithCache(item, L"tree_view/list_item.xml");
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
    return true;
}

size_t FileInfoList::GetElementCount()
{
    return m_pathList.size();
}

void FileInfoList::SetElementSelected(size_t nElementIndex, bool bSelected)
{

}

bool FileInfoList::IsElementSelected(size_t nElementIndex)
{
    return false;
}

void FileInfoList::AddFileList(const std::vector<FileInfo>& pathList)
{
    m_pathList.reserve(m_pathList.size() + pathList.size());
    for (const FileInfo& fileInfo : pathList) {
        m_pathList.emplace_back(fileInfo);
    }
    // 通知ListBox数据总数变动
    EmitCountChanged();
}

void FileInfoList::SetFileList(const std::vector<FileInfo>& pathList)
{
    ClearFileList();
    AddFileList(pathList);
}

void FileInfoList::ClearFileList()
{
    for (const FileInfo& fileInfo : m_pathList) {
        if (fileInfo.m_hIcon != nullptr) {
            ::DestroyIcon(fileInfo.m_hIcon);
        }
    }
    m_pathList.clear();
}
