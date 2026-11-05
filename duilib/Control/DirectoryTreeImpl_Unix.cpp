#include "DirectoryTreeImpl.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD) || defined (DUILIB_BUILD_FOR_MACOS)

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FileTime.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"

#include <filesystem>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <strings.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#if defined (DUILIB_BUILD_FOR_MACOS) || defined (DUILIB_BUILD_FOR_FREEBSD)
#include <sys/mount.h>
#else
#include <sys/statvfs.h>
#endif

namespace ui
{

namespace
{
    /** 挂载点信息（各平台统一的数据结构）
    */
    struct MountRecord
    {
        std::string m_device;      //设备名，如"/dev/sda2"
        std::string m_mountPoint;  //挂载点，如"/"
        std::string m_fileSystem;  //文件系统类型，如"ext4"
        uint64_t m_totalBytes = 0; //总空间大小（字节）
        uint64_t m_freeBytes = 0;  //可用空间大小（字节）
    };

    /** 比较两个显示名称（忽略大小写），用于目录内容的排序
    */
    bool CompareDisplayName(const DirectoryTree::PathInfo& lhs, const DirectoryTree::PathInfo& rhs)
    {
        return ::strcasecmp(lhs.m_displayName.c_str(), rhs.m_displayName.c_str()) < 0;
    }

    /** 还原挂载表中的八进制转义字符（如"\\040"表示空格）
    */
    std::string UnescapeMountPath(const std::string& value)
    {
        std::string result;
        result.reserve(value.size());
        for (size_t i = 0; i < value.size(); ++i) {
            if ((value[i] == '\\') && ((i + 3) < value.size()) &&
                (value[i + 1] >= '0') && (value[i + 1] <= '7') &&
                (value[i + 2] >= '0') && (value[i + 2] <= '7') &&
                (value[i + 3] >= '0') && (value[i + 3] <= '7')) {
                int ch = (value[i + 1] - '0') * 64 + (value[i + 2] - '0') * 8 + (value[i + 3] - '0');
                result.push_back((char)ch);
                i += 3;
            }
            else {
                result.push_back(value[i]);
            }
        }
        return result;
    }

    /** 判断是否为需要跳过的虚拟文件系统
    */
    bool ShouldSkipMount(const MountRecord& mountRecord)
    {
        const std::string& fsType = mountRecord.m_fileSystem;
        if ((fsType == "proc") || (fsType == "sysfs") || (fsType == "tmpfs") ||
            (fsType == "devtmpfs") || (fsType == "devpts") || (fsType == "cgroup") ||
            (fsType == "cgroup2") || (fsType == "debugfs") || (fsType == "tracefs") ||
            (fsType == "securityfs") || (fsType == "pstore") || (fsType == "mqueue") ||
            (fsType == "hugetlbfs") || (fsType == "fusectl") || (fsType == "configfs") ||
            (fsType == "bpf") || (fsType == "rpc_pipefs") || (fsType == "nfsd") ||
            (fsType == "devfs") || (fsType == "autofs") || (fsType == "fdescfs")) {
            return true;
        }
#if defined (DUILIB_BUILD_FOR_MACOS)
        //macOS的内部APFS卷不作为用户可见的磁盘
        if ((mountRecord.m_mountPoint.find("/System/Volumes/") == 0) ||
            (mountRecord.m_mountPoint.find("/private/var/vm") == 0)) {
            return true;
        }
#endif
        return false;
    }

    /** 判断是否为网络文件系统
    */
    bool IsNetworkFileSystem(const std::string& fsType)
    {
        return (fsType == "nfs") || (fsType == "nfs4") || (fsType == "cifs") ||
               (fsType == "smbfs") || (fsType == "afpfs") || (fsType == "webdav") ||
               (fsType == "9p") || (fsType == "sshfs") || (fsType == "davfs");
    }

    /** 获取设备名的基名（去掉分区号）
    * 例如: sda1 -> sda, nvme0n1p1 -> nvme0n1, mmcblk0p1 -> mmcblk0
    */
    std::string GetBaseDevice(const std::string& devicePath)
    {
        const std::string devPrefix = "/dev/";
        if (devicePath.compare(0, devPrefix.size(), devPrefix) != 0) {
            return std::string();
        }
        std::string devName = devicePath.substr(devPrefix.size());
        if (devName.empty()) {
            return std::string();
        }

        //nvme/mccblk 类型的分区使用"p<数字>"后缀
        size_t pos = devName.find_last_of('p');
        if ((pos != std::string::npos) && (pos > 0) && ((pos + 1) < devName.size())) {
            bool bAllDigits = true;
            for (size_t i = pos + 1; i < devName.size(); ++i) {
                if (!std::isdigit((unsigned char)devName[i])) {
                    bAllDigits = false;
                    break;
                }
            }
            if (bAllDigits) {
                return devName.substr(0, pos);
            }
        }

        //nvme/mccblk 的整盘设备名本身以数字结尾，不能再去掉数字
        if ((devName.compare(0, 4, "nvme") == 0) || (devName.compare(0, 6, "mmcblk") == 0)) {
            return devName;
        }

        //普通磁盘的分区使用末尾数字后缀，如 sda1 -> sda
        size_t end = devName.size();
        while ((end > 0) && std::isdigit((unsigned char)devName[end - 1])) {
            --end;
        }
        if (end > 0 && end < devName.size()) {
            return devName.substr(0, end);
        }
        return devName;
    }

    /** 读取文件中的第一个内容（用于读取sysfs属性）
    */
    std::string ReadFirstLine(const std::string& filePath)
    {
        std::ifstream file(filePath.c_str());
        if (!file.is_open()) {
            return std::string();
        }
        std::string line;
        std::getline(file, line);
        return line;
    }

    /** 判断设备的类型
    */
    DirectoryTree::DeviceType DetectDeviceType(const std::string& device, const std::string& fsType)
    {
        if (IsNetworkFileSystem(fsType)) {
            return (fsType == "nfs") || (fsType == "nfs4") ? DirectoryTree::DeviceType::NFS
                                                           : DirectoryTree::DeviceType::SHARE;
        }
        if ((fsType == "iso9660") || (fsType == "udf") || (fsType == "cd9660")) {
            return DirectoryTree::DeviceType::CDROM;
        }
        if ((fsType == "tmpfs") || (fsType == "ramfs")) {
            return DirectoryTree::DeviceType::RAMDISK;
        }
        if (device.compare(0, 9, "/dev/loop") == 0) {
            return DirectoryTree::DeviceType::LOOP;
        }
        if ((device.compare(0, 11, "/dev/mapper") == 0) || (device.compare(0, 8, "/dev/dm-") == 0)) {
            return DirectoryTree::DeviceType::VIRT_DISK;
        }
        if (device.compare(0, 5, "/dev/") != 0) {
            //非块设备(如 "server:/share")
            return DirectoryTree::DeviceType::SHARE;
        }

#if defined (DUILIB_BUILD_FOR_LINUX)
        std::string baseDevice = GetBaseDevice(device);
        if (baseDevice.empty()) {
            return DirectoryTree::DeviceType::UNKNOWN;
        }
        if (baseDevice.compare(0, 4, "nvme") == 0) {
            return DirectoryTree::DeviceType::NVME;
        }
        if (baseDevice.compare(0, 6, "mmcblk") == 0) {
            return DirectoryTree::DeviceType::SD_CARD;
        }
        std::string sysBlockPath = "/sys/block/" + baseDevice;
        struct stat statBuffer;
        if (::stat(sysBlockPath.c_str(), &statBuffer) != 0) {
            return DirectoryTree::DeviceType::UNKNOWN;
        }
        std::string removable = ReadFirstLine(sysBlockPath + "/removable");
        if (!removable.empty() && (removable[0] == '1')) {
            return DirectoryTree::DeviceType::USB;
        }
        std::string rotational = ReadFirstLine(sysBlockPath + "/queue/rotational");
        if (!rotational.empty()) {
            if (rotational[0] == '0') {
                return DirectoryTree::DeviceType::SSD;
            }
            return DirectoryTree::DeviceType::HDD;
        }
        return DirectoryTree::DeviceType::HDD;
#else
        if (device.compare(0, 10, "/dev/disk") == 0) {
            return DirectoryTree::DeviceType::UNKNOWN;
        }
        return DirectoryTree::DeviceType::UNKNOWN;
#endif
    }

    /** 获取驱动器的图标文件
    */
    DString GetDriveIconImage(DirectoryTree::DeviceType deviceType)
    {
        if (deviceType == DirectoryTree::DeviceType::CDROM) {
            return _T("public/filesystem/drive-cdrom.svg");
        }
        if ((deviceType == DirectoryTree::DeviceType::USB) || (deviceType == DirectoryTree::DeviceType::SD_CARD)) {
            return _T("public/filesystem/drive-harddisk-usb.svg");
        }
        return _T("public/filesystem/drive-harddisk.svg");
    }

#if defined (DUILIB_BUILD_FOR_MACOS) || defined (DUILIB_BUILD_FOR_FREEBSD)
    /** 获取挂载点列表（macOS/FreeBSD使用getmntinfo）
    */
    void GetMountRecords(std::vector<MountRecord>& mountRecords)
    {
        struct statfs* mounts = nullptr;
        int count = ::getmntinfo(&mounts, MNT_NOWAIT);
        if ((count <= 0) || (mounts == nullptr)) {
            return;
        }
        for (int i = 0; i < count; ++i) {
            MountRecord record;
            record.m_device = mounts[i].f_mntfromname;
            record.m_mountPoint = mounts[i].f_mntonname;
            record.m_fileSystem = mounts[i].f_fstypename;
            record.m_totalBytes = (uint64_t)mounts[i].f_blocks * (uint64_t)mounts[i].f_bsize;
            record.m_freeBytes = (uint64_t)mounts[i].f_bavail * (uint64_t)mounts[i].f_bsize;
            if (ShouldSkipMount(record)) {
                continue;
            }
            if ((record.m_mountPoint != "/") &&
                (record.m_mountPoint.find("/Volumes/") != 0) &&
                !IsNetworkFileSystem(record.m_fileSystem) &&
                (record.m_fileSystem != "cd9660") && (record.m_fileSystem != "udf")) {
                continue;
            }
            mountRecords.push_back(record);
        }
    }
#else
    /** 获取挂载点列表（Linux使用/proc/mounts）
    */
    void GetMountRecords(std::vector<MountRecord>& mountRecords)
    {
        std::ifstream file("/proc/mounts");
        if (!file.is_open()) {
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            char device[512] = { 0 };
            char mountPoint[512] = { 0 };
            char fsType[512] = { 0 };
            if (::sscanf(line.c_str(), "%511s %511s %511s", device, mountPoint, fsType) != 3) {
                continue;
            }
            MountRecord record;
            record.m_device = device;
            record.m_mountPoint = UnescapeMountPath(mountPoint);
            record.m_fileSystem = fsType;
            if (ShouldSkipMount(record)) {
                continue;
            }
            struct statvfs vfsBuffer;
            if (::statvfs(record.m_mountPoint.c_str(), &vfsBuffer) == 0) {
                record.m_totalBytes = (uint64_t)vfsBuffer.f_blocks * (uint64_t)vfsBuffer.f_frsize;
                record.m_freeBytes = (uint64_t)vfsBuffer.f_bavail * (uint64_t)vfsBuffer.f_frsize;
            }
            mountRecords.push_back(record);
        }
    }
#endif

    /** 获取当前用户的主目录
    */
    std::string GetHomePath()
    {
        const char* home = ::getenv("HOME");
        if ((home == nullptr) || (home[0] == '\0')) {
            return "/";
        }
        return home;
    }
}

struct DirectoryTreeImpl::TImpl
{
    /** 共享的文件夹图标
    */
    uint32_t m_nFolderIconID = 0;

    /** 共享的文件图标
    */
    uint32_t m_nFileIconID = 0;

    /** 共享的"计算机"图标
    */
    uint32_t m_nMyComputerIconID = 0;

    /** 共享的磁盘图标（按设备类型缓存）
    */
    std::map<DString, uint32_t> m_driveIconMap;
};

DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree)
{
    m_impl = new TImpl;
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    if (m_impl->m_nFolderIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nFolderIconID);
        m_impl->m_nFolderIconID = 0;
    }
    if (m_impl->m_nFileIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nFileIconID);
        m_impl->m_nFileIconID = 0;
    }
    if (m_impl->m_nMyComputerIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nMyComputerIconID);
        m_impl->m_nMyComputerIconID = 0;
    }
    for (const auto& iter : m_impl->m_driveIconMap) {
        if (iter.second != 0) {
            GlobalManager::Instance().Icon().RemoveIcon(iter.second);
        }
    }
    m_impl->m_driveIconMap.clear();

    delete m_impl;
    m_impl = nullptr;
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath.Clear();
    displayName.clear();
    nIconID = 0;

    std::string subDir;
    DString iconFile;
    switch (type) {
    case VirtualDirectoryType::kUserHome:
        subDir.clear();
        iconFile = _T("public/filesystem/folder-home.svg");
        break;
    case VirtualDirectoryType::kDesktop:
        subDir = "Desktop";
        iconFile = _T("public/filesystem/folder-desktop.svg");
        break;
    case VirtualDirectoryType::kDocuments:
        subDir = "Documents";
        iconFile = _T("public/filesystem/folder-documents.svg");
        break;
    case VirtualDirectoryType::kPictures:
        subDir = "Pictures";
        iconFile = _T("public/filesystem/folder-pictures.svg");
        break;
    case VirtualDirectoryType::kMusic:
        subDir = "Music";
        iconFile = _T("public/filesystem/folder-music.svg");
        break;
    case VirtualDirectoryType::kVideos:
        subDir = "Videos";
        iconFile = _T("public/filesystem/folder-videos.svg");
        break;
    case VirtualDirectoryType::kDownloads:
        subDir = "Downloads";
        iconFile = _T("public/filesystem/folder-download.svg");
        break;
    default:
        return false;
    }

    std::string path = GetHomePath();
    if (!subDir.empty()) {
        if (path.size() > 1) {
            path += "/";
        }
        path += subDir;
    }
    FilePath dirPath(path);
    if (!dirPath.IsExistsDirectory()) {
        return false;
    }
    filePath = dirPath;
    nIconID = GlobalManager::Instance().Icon().AddIcon(iconFile);
    //displayName为空时，由调用方使用传入的语言ID
    return true;
}

uint32_t DirectoryTreeImpl::GetFolderIconID(bool /*bLargeIcon*/)
{
    if (m_impl->m_nFolderIconID == 0) {
        m_impl->m_nFolderIconID = GlobalManager::Instance().Icon().AddIcon(_T("public/filesystem/folder.svg"));
    }
    return m_impl->m_nFolderIconID;
}

uint32_t DirectoryTreeImpl::GetFileIconID(const FilePath& /*filePath*/, const DString& /*fileName*/,
                                          bool /*bLargeIcon*/, DString* pTypeName)
{
    if (pTypeName != nullptr) {
        pTypeName->clear();
    }
    if (m_impl->m_nFileIconID == 0) {
        m_impl->m_nFileIconID = GlobalManager::Instance().Icon().AddIcon(_T("public/filesystem/file.svg"));
    }
    return m_impl->m_nFileIconID;
}

uint32_t DirectoryTreeImpl::GetDriveIconID(DirectoryTree::DeviceType deviceType)
{
    DString iconFile = GetDriveIconImage(deviceType);
    auto iter = m_impl->m_driveIconMap.find(iconFile);
    if (iter != m_impl->m_driveIconMap.end()) {
        return iter->second;
    }
    uint32_t nIconID = GlobalManager::Instance().Icon().AddIcon(iconFile);
    m_impl->m_driveIconMap[iconFile] = nIconID;
    return nIconID;
}

void DirectoryTreeImpl::GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList)
{
    pathInfoList.clear();

    //根目录
    DirectoryTree::PathInfo rootInfo;
    rootInfo.m_bFolder = true;
    rootInfo.m_displayName = _T("/");
    rootInfo.m_filePath = FilePath(_T("/"));
    rootInfo.m_nIconID = GetDriveIconID(DirectoryTree::DeviceType::UNKNOWN);
    rootInfo.m_bIconShared = true;
    pathInfoList.push_back(std::move(rootInfo));

    std::vector<MountRecord> mountRecords;
    GetMountRecords(mountRecords);
    for (const MountRecord& record : mountRecords) {
        if (record.m_mountPoint == "/") {
            continue;
        }
        DirectoryTree::PathInfo pathInfo;
        pathInfo.m_bFolder = true;
        pathInfo.m_displayName = record.m_mountPoint;
        pathInfo.m_filePath = FilePath(record.m_mountPoint);
        pathInfo.m_nIconID = GetDriveIconID(DetectDeviceType(record.m_device, record.m_fileSystem));
        pathInfo.m_bIconShared = true;
        pathInfoList.push_back(std::move(pathInfo));
    }
    UNUSED_VARIABLE(bLargeIcon);
}

void DirectoryTreeImpl::GetFolderContents(const FilePath& path,
                                          const std::weak_ptr<WeakFlag>& weakFlag,
                                          bool bLargeIcon,
                                          bool bShowHiddenFiles,
                                          bool bShowSystemFiles,
                                          std::vector<DirectoryTree::PathInfo>& folderList,
                                          std::vector<DirectoryTree::PathInfo>* fileList)
{
    folderList.clear();
    if (fileList != nullptr) {
        fileList->clear();
    }

    FilePath dirPath = path;
    dirPath.NormalizeDirectoryPath();
    std::error_code errorCode;
    std::filesystem::directory_iterator iter(dirPath.NativePath(), errorCode);
    if (errorCode) {
        return;
    }
    std::filesystem::directory_iterator endIter;

    const uint32_t nFolderIconID = GetFolderIconID(bLargeIcon);
    for (; iter != endIter; iter.increment(errorCode)) {
        if (errorCode) {
            errorCode.clear();
            continue;
        }
        if (weakFlag.expired()) {
            break;
        }

        const std::filesystem::directory_entry& entry = *iter;
        std::error_code entryError;
        if (entry.is_symlink(entryError)) {
            entryError.clear();
            continue;
        }
        DString fileName = entry.path().filename().string();
        if (fileName.empty() || (fileName == _T(".")) || (fileName == _T(".."))) {
            continue;
        }
        if ((fileName[0] == _T('.')) && !bShowHiddenFiles) {
            continue;
        }

        bool bFolder = entry.is_directory(entryError);
        if (entryError) {
            entryError.clear();
            continue;
        }
        if (!bFolder) {
            if (fileList == nullptr) {
                continue;
            }
            if (!entry.is_regular_file(entryError)) {
                entryError.clear();
                continue;
            }
        }

        struct stat statBuffer;
        if (::stat(entry.path().native().c_str(), &statBuffer) != 0) {
            continue;
        }

        DirectoryTree::PathInfo pathInfo;
        pathInfo.m_bFolder = bFolder;
        pathInfo.m_displayName = fileName;
        pathInfo.m_filePath = dirPath;
        pathInfo.m_filePath.JoinFilePath(FilePath(fileName));
        pathInfo.m_lastWriteTime.FromSecondsSinceEpoch((uint64_t)statBuffer.st_mtime);
        if (bFolder) {
            pathInfo.m_nIconID = nFolderIconID;
            pathInfo.m_bIconShared = true;
            folderList.push_back(std::move(pathInfo));
        }
        else {
            DString typeName;
            pathInfo.m_nIconID = GetFileIconID(pathInfo.m_filePath, fileName, bLargeIcon, &typeName);
            pathInfo.m_typeName = typeName;
            pathInfo.m_fileSize = (uint64_t)statBuffer.st_size;
            pathInfo.m_bIconShared = true;
            fileList->push_back(std::move(pathInfo));
        }
    }

    std::sort(folderList.begin(), folderList.end(), CompareDisplayName);
    if (fileList != nullptr) {
        std::sort(fileList->begin(), fileList->end(), CompareDisplayName);
    }
    UNUSED_VARIABLE(bShowSystemFiles);
}

void DirectoryTreeImpl::GetDiskInfoList(const std::weak_ptr<WeakFlag>& weakFlag,
                                        bool bLargeIcon,
                                        std::vector<DirectoryTree::DiskInfo>& diskInfoList)
{
    diskInfoList.clear();
    std::vector<MountRecord> mountRecords;
    GetMountRecords(mountRecords);
    for (const MountRecord& record : mountRecords) {
        if (weakFlag.expired()) {
            break;
        }
        if (record.m_mountPoint == "/") {
            continue;
        }
        DirectoryTree::DiskInfo diskInfo;
        diskInfo.m_displayName = record.m_mountPoint;
        diskInfo.m_filePath = FilePath(record.m_mountPoint);
        diskInfo.m_volumeName = record.m_device;
        diskInfo.m_deviceType = DetectDeviceType(record.m_device, record.m_fileSystem);
        diskInfo.m_fileSystem = record.m_fileSystem;
        diskInfo.m_mountOn = record.m_mountPoint;
        diskInfo.m_totalBytes = record.m_totalBytes;
        diskInfo.m_freeBytes = record.m_freeBytes;
        diskInfo.m_nIconID = GetDriveIconID(diskInfo.m_deviceType);
        diskInfo.m_bIconShared = true;
        diskInfoList.push_back(std::move(diskInfo));
    }
    UNUSED_VARIABLE(bLargeIcon);
}

bool DirectoryTreeImpl::NeedShowDirPath(const FilePath& path, bool bShowHiddenFiles, bool bShowSystemFiles) const
{
    if (path.IsEmpty()) {
        return true;
    }
    if (!path.IsExistsDirectory()) {
        return false;
    }
    DString fileName = path.GetFileName();
    if (!bShowHiddenFiles && !fileName.empty() && (fileName[0] == _T('.'))) {
        return false;
    }
#if defined (DUILIB_BUILD_FOR_MACOS)
    struct stat statBuffer;
    if (::stat(path.NativePath().c_str(), &statBuffer) == 0) {
        if (!bShowHiddenFiles && (statBuffer.st_flags & UF_HIDDEN)) {
            return false;
        }
    }
#endif
    UNUSED_VARIABLE(bShowSystemFiles);
    return true;
}

uint32_t DirectoryTreeImpl::GetMyComputerIconID() const
{
    if (m_impl->m_nMyComputerIconID == 0) {
        m_impl->m_nMyComputerIconID = GlobalManager::Instance().Icon().AddIcon(_T("public/filesystem/computer.svg"));
    }
    return m_impl->m_nMyComputerIconID;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_LINUX || DUILIB_BUILD_FOR_FREEBSD || DUILIB_BUILD_FOR_MACOS

