#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_MACOS

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"

#include <fstream>
#include <sys/statvfs.h>
#include <stdio.h>

#ifdef DUILIB_BIT_64
    #define __USE_FILE_OFFSET64
#endif
#include <sys/stat.h>

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

/** 获取图片属性字符串
*/
DString DirectoryTreeImplGetImageString(DirectoryTree* pTree, bool bLargeFile, const DString& imageFileName)
{
    if (imageFileName.empty()) {
        return DString();
    }
    int32_t nSmallIconSize = 20;
    int32_t nLargeIconSize = 32;
    if (pTree != nullptr) {
        nSmallIconSize = pTree->GetSmallIconSize();
        nLargeIconSize = pTree->GetLargeIconSize();
    }
    if (nSmallIconSize < 1) {
        nSmallIconSize = 20;
    }
    if (nLargeIconSize < 1) {
        nLargeIconSize = 32;
    }
    int32_t nIconSize = bLargeFile ? nLargeIconSize : nSmallIconSize;
    DString imageString = StringUtil::Printf(_T("file='public/filesystem/%s' width='%d' height='%d' valign='center'"), imageFileName.c_str(), nIconSize, nIconSize);
    return imageString;
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
    }

    userHomeDir.NormalizeDirectoryPath();
    switch (type) {
    case VirtualDirectoryType::kUserHome:
        filePath = userHomeDir;
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-home.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-desktop.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-documents.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-pictures.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-music.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-videos.svg")));
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
        nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("folder-download.svg")));
        break;
    default:
        break;
    }
    return true;
}

void DirectoryTreeImpl::GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList)
{
    pathInfoList.clear();
    const std::filesystem::path rootPath("/");
    DirectoryTree::PathInfo pathInfo;
    pathInfo.m_bFolder = true;
    pathInfo.m_filePath = FilePath(rootPath.native());
    pathInfo.m_displayName = pathInfo.m_filePath.ToString();
    pathInfo.m_bIconShared = false;
    pathInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("drive-harddisk.svg")));
    //文件系统根目录
    pathInfoList.push_back(pathInfo);

    std::weak_ptr<WeakFlag> weakFlag;
    std::vector<DirectoryTree::DiskInfo> diskInfoList;
    GetDiskInfoList(weakFlag, bLargeIcon, diskInfoList);
    for (const DirectoryTree::DiskInfo& diskInfo : diskInfoList) {
        if (diskInfo.m_filePath == FilePath(rootPath.native())) {
            continue;
        }

        pathInfo.m_filePath = diskInfo.m_filePath;
        pathInfo.m_displayName = diskInfo.m_volumeName;
        pathInfo.m_bIconShared = diskInfo.m_bIconShared;
        pathInfo.m_nIconID = diskInfo.m_nIconID;

        pathInfoList.push_back(pathInfo);
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
        std::filesystem::path stdPath(path.NativePathA());
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
                        m_impl->m_nLargeFolderIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("folder.svg")));
                    }
                    pathInfo.m_nIconID = m_impl->m_nLargeFolderIconID;
                }
                else {
                    if (m_impl->m_nSmallFolderIconID == 0) {
                        m_impl->m_nSmallFolderIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("folder.svg"))); 
                    }
                    pathInfo.m_nIconID = m_impl->m_nSmallFolderIconID;
                }                
                pathInfo.m_bIconShared = true;
                struct stat buf{0, };
                int result = ::stat(entry.path().native().c_str(), &buf);
                if (result == 0) {
                    //目录的最后修改时间
                    pathInfo.m_lastWriteTime.FromSecondsSinceEpoch(buf.st_mtime);
                }
                folderList.emplace_back(std::move(pathInfo));
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
                            m_impl->m_nLargeFileIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("file.svg")));
                        }
                        pathInfo.m_nIconID = m_impl->m_nLargeFileIconID;
                    }
                    else {
                        if (m_impl->m_nSmallFileIconID == 0) {
                            m_impl->m_nSmallFileIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("file.svg")));
                        }
                        pathInfo.m_nIconID = m_impl->m_nSmallFileIconID;
                    }
                    pathInfo.m_bIconShared = true;

                    //文件的最后修改时间和文件大小
                    struct stat buf{0, };
                    int result = ::stat(entry.path().native().c_str(), &buf);
                    if (result == 0) {
                        pathInfo.m_fileSize = buf.st_size;
                        pathInfo.m_lastWriteTime.FromSecondsSinceEpoch(buf.st_mtime);
                    }
                    fileList->emplace_back(std::move(pathInfo));
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
    return GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, false, _T("computer.svg")));
}

// 从设备路径提取基础设备名（如 /dev/sda1 -> sda）
static std::string get_base_device(const std::string& dev_path)
{
    if (dev_path.find("/dev/") != 0) {
        if (dev_path.find(":/") != std::string::npos) {
            return "nfs";
        }
        return dev_path;
    }

    std::string dev_name = dev_path.substr(5); // 去掉"/dev/"
    
    // 处理分区号（如sda1->sda, nvme0n1p1->nvme0n1）
    size_t pos = dev_name.find_first_of("0123456789", 0);
    while ((pos != std::string::npos) && ((pos + 1) < dev_name.size()) && ::isdigit(dev_name[pos + 1])) {
        ++pos;
    }
    if (pos != std::string::npos) {
        dev_name = dev_name.substr(0, pos);
    }
    return dev_name;
}

// 检测设备物理类型
static DirectoryTree::DeviceType detect_device_type(const std::string& base_dev)
{
    // 网络文件系统
    if (base_dev == "nfs") {
        return DirectoryTree::DeviceType::NFS;
    }
    
    // 虚拟设备
    if (base_dev.find("loop") == 0) {
        return DirectoryTree::DeviceType::LOOP;
    }
    if ((base_dev.find("mapper") == 0) || (base_dev.find("dm-") == 0)){
        return DirectoryTree::DeviceType::VIRT_DISK;
    }
    
    // 检查/sys/block下是否存在该设备
    std::filesystem::path sys_block = "/sys/block/" + base_dev;
    if (!std::filesystem::exists(sys_block)) {
        return DirectoryTree::DeviceType::UNKNOWN;
    }

    // NVMe设备
    if (base_dev.find("nvme") == 0) {
        return DirectoryTree::DeviceType::NVME;
    }

    // SD卡
    if (base_dev.find("mmcblk") == 0) {
        return DirectoryTree::DeviceType::SD_CARD;
    }

    // 光驱
    std::ifstream type_file("/sys/block/" + base_dev + "/device/type");
    if (type_file) {
        int type = 0;
        type_file >> type;
        if (type == 5) {
            return DirectoryTree::DeviceType::CDROM;
        }
    }

    // USB设备（检查driver链接）
    std::filesystem::path driver_link = "/sys/block/" + base_dev + "/device/../driver";
    if (std::filesystem::exists(driver_link)) {
        std::string driver = std::filesystem::read_symlink(driver_link).filename();
        if (driver.find("usb") != std::string::npos) {
            return DirectoryTree::DeviceType::USB;
        }
    }

    // 机械硬盘/SSD检测
    std::ifstream rotational_file("/sys/block/" + base_dev + "/queue/rotational");
    if (rotational_file) {
        int rotational;
        rotational_file >> rotational;
        return rotational ? DirectoryTree::DeviceType::HDD : DirectoryTree::DeviceType::SSD;
    }

    return DirectoryTree::DeviceType::UNKNOWN;
}

void DirectoryTreeImpl::GetDiskInfoList(const std::weak_ptr<WeakFlag>& /*weakFlag*/,
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

        std::string base_dev = get_base_device(device);
        DirectoryTree::DeviceType deviceType = detect_device_type(base_dev);
        if (deviceType == DirectoryTree::DeviceType::UNKNOWN) {
            if (strstr(device, "/dev/sr") || strstr(device, "/dev/cdrom")) {
                deviceType = DirectoryTree::DeviceType::CDROM;
            }
            else if (std::string(device) == "vmhgfs-fuse") {
                deviceType = DirectoryTree::DeviceType::SHARE;
            }
        }

        DirectoryTree::DiskInfo diskInfo;
        diskInfo.m_displayName = StringConvert::UTF8ToT(device);        
        diskInfo.m_filePath = FilePath(StringConvert::UTF8ToT(mount_point));

        diskInfo.m_volumeName = diskInfo.m_displayName;
        diskInfo.m_deviceType = deviceType;
        diskInfo.m_mountOn = StringConvert::UTF8ToT(mount_point);
        diskInfo.m_fileSystem = StringConvert::UTF8ToT(fs_type);
        diskInfo.m_totalBytes = total;
        diskInfo.m_freeBytes = avail;

        diskInfo.m_bIconShared = false;
        diskInfo.m_nIconID = 0;
        if (diskInfo.m_deviceType == DirectoryTree::DeviceType::CDROM) {
            diskInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("drive-cdrom.svg")));
        }
        else if (diskInfo.m_deviceType == DirectoryTree::DeviceType::USB) {
            diskInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("drive-harddisk-usb.svg")));
        }
        else {
            diskInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(DirectoryTreeImplGetImageString(m_pTree, bLargeIcon, _T("drive-harddisk.svg")));
        }

        diskInfoList.emplace_back(std::move(diskInfo));
    }

    ::fclose(fp);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_MACOS
