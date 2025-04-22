#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_MACOS

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
struct DirectoryTreeImpl::TImpl
{
    /** 共享的文件夹图标(大图标)
    */
    uint32_t m_nLargeFolderIconID = 0;

    /** 共享的文件夹图标(小图标)
    */
    uint32_t m_nSmallFolderIconID = 0;

    /** 共享的文件图标(大图标)
    */
    uint32_t m_nLargeFileIconID = 0;

    /** 共享的文件图标(小图标)
    */
    uint32_t m_nSmallFileIconID = 0;
};

DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree)
{
    m_impl = new TImpl;
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    if (m_impl->m_nLargeFolderIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nLargeFolderIconID);
        m_impl->m_nLargeFolderIconID = 0;
    }
    if (m_impl->m_nSmallFolderIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nSmallFolderIconID);
        m_impl->m_nSmallFolderIconID = 0;
    }
    if (m_impl->m_nLargeFileIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nLargeFileIconID);
        m_impl->m_nLargeFileIconID = 0;
    }
    if (m_impl->m_nSmallFileIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nSmallFileIconID);
        m_impl->m_nSmallFileIconID = 0;
    }
    delete m_impl;
    m_impl = nullptr;
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath.Clear();
    displayName.clear();
    nIconID = 0;

    //用户的HOME目录
    FilePath userHomeDir;
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
        userHomeDir = FilePath(home);
    }
    else {
        userHomeDir = _T("/");
#ifdef DUILIB_BUILD_FOR_WIN
        //测试用
        userHomeDir = _T("D:\\temp\\");
#endif
    }

    userHomeDir.NormalizeDirectoryPath();
    switch (type) {
    case VirtualDirectoryType::kUserHome:
        filePath = userHomeDir;
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-home.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kDesktop:
        filePath = userHomeDir;
        filePath += _T("Desktop");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("桌面");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-desktop.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kDocuments:
        filePath = userHomeDir;
        filePath += _T("Documents");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("文档");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-documents.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kPictures:
        filePath = userHomeDir;
        filePath += _T("Pictures");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("图片");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-pictures.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kMusic:
        filePath = userHomeDir;
        filePath += _T("Music");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("音乐");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-music.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kVideos:
        filePath = userHomeDir;
        filePath += _T("Videos");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("视频");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-videos.svg' width='16' height='16' valign='center'"));
        break;
    case VirtualDirectoryType::kDownloads:
        filePath = userHomeDir;
        filePath += _T("Downloads");
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
            filePath += _T("下载");
        }
        if (!filePath.IsExistsDirectory()) {
            filePath = userHomeDir;
        }
        nIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder-download.svg' width='16' height='16' valign='center'"));
        break;
    default:
        break;
    }
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
            if (m_impl->m_nSmallFolderIconID == 0) {
                m_impl->m_nSmallFolderIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder.svg' width='16' height='16' valign='center'"));
            }
            pathInfo.m_nIconID = m_impl->m_nSmallFolderIconID;
            pathInfo.m_bIconShared = true;
            pathInfoList.push_back(pathInfo);
        }
    }
    catch (const std::filesystem::filesystem_error& /*e*/) {
    }
}

void DirectoryTreeImpl::GetFolderContents(const FilePath& path,
                                          const std::weak_ptr<WeakFlag>& weakFlag,
                                          bool bLargeIcon,
                                          std::vector<DirectoryTree::PathInfo>& folderList,
                                          std::vector<DirectoryTree::PathInfo>* fileList)
{
    folderList.clear();
    if (fileList != nullptr) {
        fileList->clear();
    }
    try {
#ifdef DUILIB_BUILD_FOR_WIN
        std::filesystem::path stdPath(path.ToStringW());
#else
        std::filesystem::path stdPath(path.NativePathA());
#endif
        for (const auto& entry : std::filesystem::directory_iterator(stdPath)) {
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

                if (bLargeIcon) {
                    if (m_impl->m_nLargeFolderIconID == 0) {
                        m_impl->m_nLargeFolderIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder.svg' width='32' height='32' valign='center'"));
                    }
                    pathInfo.m_nIconID = m_impl->m_nLargeFolderIconID;
                }
                else {
                    if (m_impl->m_nSmallFolderIconID == 0) {
                        m_impl->m_nSmallFolderIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/folder.svg' width='16' height='16' valign='center'"));
                    }
                    pathInfo.m_nIconID = m_impl->m_nSmallFolderIconID;
                }                
                pathInfo.m_bIconShared = true;
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

                    if (bLargeIcon) {
                        if (m_impl->m_nLargeFileIconID == 0) {
                            m_impl->m_nLargeFileIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/file.svg' width='32' height='32' valign='center'"));
                        }
                        pathInfo.m_nIconID = m_impl->m_nLargeFileIconID;
                    }
                    else {
                        if (m_impl->m_nSmallFileIconID == 0) {
                            m_impl->m_nSmallFileIconID = GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/file.svg' width='16' height='16' valign='center'"));
                        }
                        pathInfo.m_nIconID = m_impl->m_nSmallFileIconID;
                    }
                    pathInfo.m_bIconShared = true;
                    fileList->push_back(pathInfo);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& /*e*/) {
        ASSERT(0);
    }
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_MACOS
