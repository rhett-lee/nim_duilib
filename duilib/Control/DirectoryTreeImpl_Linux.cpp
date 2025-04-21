#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_LINUX

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
struct DirectoryTreeImpl::TImpl
{
};

DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree),
    m_nSharedIconID(0)
{
    m_impl = new TImpl;
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    delete m_impl;
    m_impl = nullptr;
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath = _T("/");
    displayName.clear();
    nIconID = 0;

    
    return true;
}

void DirectoryTreeImpl::GetRootPathInfoList(std::vector<DirectoryTree::PathInfo>& pathInfoList)
{
    pathInfoList.clear();
    try {
        std::filesystem::path path("/");
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::error_code errorCode;
            bool bDirectory = entry.is_directory(errorCode);
            if (!bDirectory) {
                continue;
            }
            DirectoryTree::PathInfo pathInfo;
            pathInfo.m_bFolder = true;
            pathInfo.m_filePath = FilePath(entry.path().native());
            pathInfo.m_displayName = pathInfo.m_filePath.GetFileName();
            
            pathInfoList.push_back(pathInfo);
        }
    }
    catch (const std::filesystem::filesystem_error& /*e*/) {
    }
}

void DirectoryTreeImpl::GetFolderContents(const FilePath& path,
                                          const std::weak_ptr<WeakFlag>& weakFlag,
                                          std::vector<DirectoryTree::PathInfo>& folderList,
                                          std::vector<DirectoryTree::PathInfo>* fileList)
{
    folderList.clear();
    if (fileList != nullptr) {
        fileList->clear();
    }
    try {
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(path.ToStringA()))) {
            if (weakFlag.expired()) {
                //取消
                break;
            }
            std::error_code errorCode;
            bool bSymlink = entry.is_symlink(errorCode);
            if (bSymlink) {
                //不支持symlink
                continue;
            }

            bool bDirectory = entry.is_directory(errorCode);
            if (bDirectory) {
                //目录
                DirectoryTree::PathInfo pathInfo;
                pathInfo.m_bFolder = true;
                pathInfo.m_filePath = FilePath(entry.path().native());
                pathInfo.m_displayName = pathInfo.m_filePath.GetFileName();

                folderList.push_back(pathInfo);
            }
            else if (fileList != nullptr) {
                bool bRegularFile = entry.is_regular_file(errorCode);
                if (bRegularFile) {
                    //普通文件
                    DirectoryTree::PathInfo pathInfo;
                    pathInfo.m_bFolder = false;
                    pathInfo.m_filePath = FilePath(entry.path().native());
                    pathInfo.m_displayName = pathInfo.m_filePath.GetFileName();

                    fileList->push_back(pathInfo);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& /*e*/) {
    }
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_LINUX
