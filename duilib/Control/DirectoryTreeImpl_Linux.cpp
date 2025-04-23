#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_LINUX

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"

#include <sys/statvfs.h>
#include <stdio.h>

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

void DirectoryTreeImpl::GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList)
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
            pathInfo.m_bIconShared = true;
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
            if ((m_pTree != nullptr) && !m_pTree->IsShowHidenFiles()) {
                if (entry.path().has_filename()) {
                    const std::filesystem::path::string_type s = entry.path().filename().native();
                    if (!s.empty() && (s[0] == '.')) {
                        //不显示隐藏文件
                        continue;
                    }
                }
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

bool DirectoryTreeImpl::NeedShowDirPath(const FilePath& path) const
{
    if ((m_pTree == nullptr) || path.IsEmpty()) {
        return false;
    }
    if (!path.IsExistsDirectory()) {
        return false;
    }

    if (!m_pTree->IsShowHidenFiles()) {
        DString s = path.NativePath();
        if (!s.empty() && (s[0] == '.')) {
            //不显示隐藏文件
            return false;
        }
    }
    return true;
}

uint32_t DirectoryTreeImpl::GetMyComputerIconID() const
{
    return GlobalManager::Instance().Icon().AddIcon(_T("file='public/filesystem/computer.svg' width='16' height='16' valign='center'"));
}

void DirectoryTreeImpl::GetDiskInfoList(const std::weak_ptr<WeakFlag>& weakFlag,
                                        bool bLargeIcon,
                                        std::vector<DirectoryTree::DiskInfo>& diskInfoList)
{
    FILE* fp = ::fopen("/proc/mounts", "r");
    if (fp == nullptr) {
        return;
    }

    char line[2048] = { 0 };
    while (::fgets(line, sizeof(line) - 8, fp)) {
        char device[256] = { 0, };
        char mount_point[256] = { 0, };
        char fs_type[256] = { 0, };
        ::sscanf(line, "%255s %255s %255s", device, mount_point, fs_type);

        // 跳过虚拟文件系统（如proc、sysfs）
        if (::strcmp(fs_type, "proc") == 0 || ::strcmp(fs_type, "sysfs") == 0 ||
            ::strcmp(fs_type, "tmpfs") == 0 || ::strcmp(fs_type, "devtmpfs") == 0) {
            continue;
        }

        struct statvfs vfs = { 0, };
        if (::statvfs(mount_point, &vfs) != 0) {
            continue; // 跳过无法访问的挂载点
        }

        // 计算磁盘使用情况
        unsigned long total = vfs.f_blocks * vfs.f_frsize;
        unsigned long avail = vfs.f_bavail * vfs.f_frsize;

        if (total == 0) {
            continue;
        }

        DString volumeType = _T("[Local Disk]");
        if (strstr(device, "/dev/sr") || strstr(device, "/dev/cdrom")) {
            volumeType = _T("[CD/DVD]");
        }
        else if (strstr(device, "/dev/mapper") || strstr(device, "/dev/dm-")) {
            volumeType = _T("[Virtual]");
        }

        DirectoryTree::DiskInfo diskInfo;
        diskInfo.m_displayName = StringConvert::UTF8ToT(device);
        diskInfo.m_bIconShared = true;
        diskInfo.m_nIconID = 0;
        diskInfo.m_filePath = FilePath(StringConvert::UTF8ToT(mount_point));

        diskInfo.m_volumeName = diskInfo.m_displayName;
        diskInfo.m_volumeType = volumeType;   //分区类型，如"本地磁盘"
        diskInfo.m_mountOn = StringConvert::UTF8ToT(mount_point);
        diskInfo.m_fileSystem = StringConvert::UTF8ToT(fs_type);
        diskInfo.m_totalBytes = total;
        diskInfo.m_freeBytes = avail;

        diskInfoList.emplace_back(std::move(diskInfo));
    }

    ::fclose(fp);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_LINUX
