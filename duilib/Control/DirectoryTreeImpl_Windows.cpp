#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/DiskUtils_Windows.h"
#include "duilib/Utils/DllManager_Windows.h"
#include "duilib/Utils/StringUtil.h"

#include <ShellApi.h>
#include <Shlobj.h>
#include <algorithm>
#include <cstring>

namespace ui
{

namespace
{
    /** 缓存的文件图标（按扩展名共享）
    */
    struct CachedFileIcon
    {
        uint32_t m_nIconID = 0;  //图标ID
        DString m_typeName;      //文件类型显示名称
    };

    /** 比较两个显示名称（忽略大小写），用于目录内容的排序
    */
    bool CompareDisplayName(const DirectoryTree::PathInfo& lhs, const DirectoryTree::PathInfo& rhs)
    {
        return _wcsicmp(lhs.m_displayName.c_str(), rhs.m_displayName.c_str()) < 0;
    }

    /** 将驱动器的类型转换为设备类型
    */
    DirectoryTree::DeviceType GetDeviceTypeFromDrive(const DString& drivePath)
    {
        switch (::GetDriveTypeW(drivePath.c_str())) {
        case DRIVE_FIXED:
            return DirectoryTree::DeviceType::HDD;
        case DRIVE_REMOVABLE:
            return DirectoryTree::DeviceType::USB;
        case DRIVE_CDROM:
            return DirectoryTree::DeviceType::CDROM;
        case DRIVE_REMOTE:
            return DirectoryTree::DeviceType::SHARE;
        default:
            break;
        }
        return DirectoryTree::DeviceType::UNKNOWN;
    }

    /** 通过Shell获取指定路径的图标ID（调用方需要保证图标管理器可用）
    * @param [in] shellPath 传给Shell的路径
    * @param [in] bLargeIcon 是否为大图标
    * @param [in] bNeedTypeName 是否需要获取文件类型名称
    * @param [out] typeName 返回文件类型名称（可为nullptr）
    * @return 返回图标ID，0表示获取失败
    */
    uint32_t GetShellIconID(const DString& shellPath, bool bLargeIcon, bool bNeedTypeName, DString* typeName)
    {
        DWORD flags = (bLargeIcon ? SHGFI_LARGEICON : SHGFI_SMALLICON);
        //必须显式指定SHGFI_ICON，否则Shell不会返回图标句柄
        flags |= SHGFI_ICON;
        if (bNeedTypeName) {
            flags |= SHGFI_TYPENAME;
        }
        SHFILEINFO shFileInfo;
        memset(&shFileInfo, 0, sizeof(shFileInfo));
        if (SHGetFileInfoW(shellPath.c_str(), 0, &shFileInfo, sizeof(shFileInfo), flags) == 0) {
            return 0;
        }
        if (typeName != nullptr) {
            *typeName = shFileInfo.szTypeName;
        }
        if (shFileInfo.hIcon == nullptr) {
            return 0;
        }
        uint32_t nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
        ::DestroyIcon(shFileInfo.hIcon);
        return nIconID;
    }
}

struct DirectoryTreeImpl::TImpl
{
    /** _T("Shell32.dll") 句柄
    */
    HMODULE m_hShell32Dll = nullptr;

    /** 共享的文件夹图标(大图标)
    */
    uint32_t m_nLargeFolderIconID = 0;

    /** 共享的文件夹图标(小图标)
    */
    uint32_t m_nSmallFolderIconID = 0;

    /** 共享的"计算机"图标(小图标)
    */
    uint32_t m_nMyComputerIconID = 0;

    /** 文件图标缓存（Key: 大小前缀 + 小写扩展名）
    */
    std::map<DString, CachedFileIcon> m_fileIconCache;
};

DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree)
{
    m_impl = new TImpl;
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    //释放所有共享图标
    if (m_impl->m_nLargeFolderIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nLargeFolderIconID);
        m_impl->m_nLargeFolderIconID = 0;
    }
    if (m_impl->m_nSmallFolderIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nSmallFolderIconID);
        m_impl->m_nSmallFolderIconID = 0;
    }
    if (m_impl->m_nMyComputerIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nMyComputerIconID);
        m_impl->m_nMyComputerIconID = 0;
    }
    for (const auto& iter : m_impl->m_fileIconCache) {
        if (iter.second.m_nIconID != 0) {
            GlobalManager::Instance().Icon().RemoveIcon(iter.second.m_nIconID);
        }
    }
    m_impl->m_fileIconCache.clear();

    delete m_impl;
    m_impl = nullptr;
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath.Clear();
    displayName.clear();
    nIconID = 0;

    if (m_impl->m_hShell32Dll == nullptr) {
        m_impl->m_hShell32Dll = DllManager::Instance().LoadDll(_T("Shell32.dll"));
    }
    if (m_impl->m_hShell32Dll == nullptr) {
        return false;
    }

    KNOWNFOLDERID folderId = FOLDERID_Desktop;
    int csidl = CSIDL_DESKTOP;
    switch (type) {
    case VirtualDirectoryType::kUserHome:
        folderId = FOLDERID_Profile;
        csidl = CSIDL_PROFILE;
        break;
    case VirtualDirectoryType::kDesktop:
        folderId = FOLDERID_Desktop;
        csidl = CSIDL_DESKTOP;
        break;
    case VirtualDirectoryType::kDocuments:
        folderId = FOLDERID_Documents;
        csidl = CSIDL_PERSONAL;
        break;
    case VirtualDirectoryType::kPictures:
        folderId = FOLDERID_Pictures;
        csidl = CSIDL_MYPICTURES;
        break;
    case VirtualDirectoryType::kMusic:
        folderId = FOLDERID_Music;
        csidl = CSIDL_MYMUSIC;
        break;
    case VirtualDirectoryType::kVideos:
        folderId = FOLDERID_Videos;
        csidl = CSIDL_MYVIDEO;
        break;
    case VirtualDirectoryType::kDownloads:
        folderId = FOLDERID_Downloads;
        csidl = -1;
        break;
    default:
        return false;
    }

    //优先使用SHGetKnownFolderPath获取路径（Vista及以上）
    DString folder;
    typedef HRESULT(WINAPI* PFnSHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
    PFnSHGetKnownFolderPath pfnSHGetKnownFolderPath = nullptr;
    pfnSHGetKnownFolderPath = (PFnSHGetKnownFolderPath)::GetProcAddress(m_impl->m_hShell32Dll, "SHGetKnownFolderPath");
    if (pfnSHGetKnownFolderPath != nullptr) {
        PWSTR ppszPath = nullptr;
        if (SUCCEEDED(pfnSHGetKnownFolderPath(folderId, KF_FLAG_DEFAULT, nullptr, &ppszPath)) && (ppszPath != nullptr)) {
            folder = ppszPath;
            ::CoTaskMemFree(ppszPath);
        }
    }

    //回退到SHGetFolderPath
    if (folder.empty() && (csidl >= 0)) {
        WCHAR szPath[MAX_PATH] = { 0 };
        if (::SHGetFolderPathW(nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, szPath) == S_OK) {
            folder = szPath;
        }
    }
    if (folder.empty() || !FilePath(folder).IsExistsDirectory()) {
        return false;
    }
    filePath = folder;

    //获取显示名称与图标
    SHFILEINFO shFileInfo;
    memset(&shFileInfo, 0, sizeof(shFileInfo));
    if (SHGetFileInfoW(folder.c_str(), 0, &shFileInfo, sizeof(shFileInfo),
                       SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON) == 0) {
        displayName = folder;
        return true;
    }
    displayName = shFileInfo.szDisplayName;
    if (shFileInfo.hIcon != nullptr) {
        nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
        ::DestroyIcon(shFileInfo.hIcon);
    }
    return true;
}

void DirectoryTreeImpl::GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList)
{
    for (wchar_t driveChar = L'C'; driveChar <= L'Z'; ++driveChar) {
        DString driverName(1, driveChar);
        driverName += _T(":\\");
        UINT uDriveType = ::GetDriveTypeW(driverName.c_str());
        if ((uDriveType != DRIVE_FIXED) &&
            (uDriveType != DRIVE_REMOVABLE) &&
            (uDriveType != DRIVE_CDROM) &&
            (uDriveType != DRIVE_REMOTE)) {
            continue;
        }

        SHFILEINFO shFileInfo;
        memset(&shFileInfo, 0, sizeof(shFileInfo));
        DWORD flags = (bLargeIcon ? SHGFI_LARGEICON : SHGFI_SMALLICON);
        flags |= SHGFI_DISPLAYNAME | SHGFI_ICON;
        if (SHGetFileInfoW(driverName.c_str(), 0, &shFileInfo, sizeof(shFileInfo), flags) == 0) {
            continue;
        }

        DirectoryTree::PathInfo pathInfo;
        pathInfo.m_bFolder = true;
        pathInfo.m_filePath = driverName;
        pathInfo.m_displayName = shFileInfo.szDisplayName;
        if (shFileInfo.hIcon != nullptr) {
            pathInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
            ::DestroyIcon(shFileInfo.hIcon);
        }
        pathInfo.m_bIconShared = false;
        pathInfoList.push_back(std::move(pathInfo));
    }
}

uint32_t DirectoryTreeImpl::GetFolderIconID(bool bLargeIcon)
{
    uint32_t& nIconID = bLargeIcon ? m_impl->m_nLargeFolderIconID : m_impl->m_nSmallFolderIconID;
    if (nIconID == 0) {
        //用系统目录获取一次文件夹图标，之后所有文件夹共享该图标
        DString shellPath;
        WCHAR szWindowsDir[MAX_PATH] = { 0 };
        if (::GetWindowsDirectoryW(szWindowsDir, MAX_PATH) > 0) {
            shellPath = szWindowsDir;
        }
        if (shellPath.empty()) {
            shellPath = _T("C:\\");
        }
        nIconID = GetShellIconID(shellPath, bLargeIcon, false, nullptr);
    }
    return nIconID;
}

uint32_t DirectoryTreeImpl::GetFileIconID(const FilePath& filePath, const DString& fileName, bool bLargeIcon, DString* pTypeName)
{
    DString extension = FilePath(fileName).GetFileExtension();
    if (extension.empty()) {
        extension = _T("_noext");
    }
    extension = StringUtil::MakeLowerString(extension);
    DString cacheKey = (bLargeIcon ? _T("L:") : _T("S:"));
    cacheKey += extension;

    auto iter = m_impl->m_fileIconCache.find(cacheKey);
    if (iter != m_impl->m_fileIconCache.end()) {
        if (pTypeName != nullptr) {
            *pTypeName = iter->second.m_typeName;
        }
        return iter->second.m_nIconID;
    }

    CachedFileIcon cachedIcon;
    cachedIcon.m_nIconID = GetShellIconID(filePath.ToString(), bLargeIcon, pTypeName != nullptr, &cachedIcon.m_typeName);
    m_impl->m_fileIconCache[cacheKey] = cachedIcon;
    if (pTypeName != nullptr) {
        *pTypeName = cachedIcon.m_typeName;
    }
    return cachedIcon.m_nIconID;
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
    DString findPath = dirPath.ToString();
    if (!findPath.empty() && (findPath.back() != _T('\\'))) {
        findPath += _T("\\");
    }
    findPath += _T("*");

    WIN32_FIND_DATAW findData;
    memset(&findData, 0, sizeof(findData));
    HANDLE hFind = ::FindFirstFileW(findPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    const uint32_t nFolderIconID = GetFolderIconID(bLargeIcon);
    do {
        if (weakFlag.expired()) {
            break;
        }
        DString fileName = findData.cFileName;
        if ((fileName == _T(".")) || (fileName == _T(".."))) {
            continue;
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            if (!bShowHiddenFiles) {
                continue;
            }
        }
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
            if (!bShowSystemFiles) {
                continue;
            }
        }

        const bool bFolder = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        if (bFolder) {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                //跳过目录符号链接/联接点，避免递归
                continue;
            }
            DirectoryTree::PathInfo pathInfo;
            pathInfo.m_bFolder = true;
            pathInfo.m_displayName = fileName;
            pathInfo.m_filePath = dirPath;
            pathInfo.m_filePath.JoinFilePath(FilePath(fileName));
            pathInfo.m_nIconID = nFolderIconID;
            pathInfo.m_bIconShared = true;
            pathInfo.m_lastWriteTime.FromFileTime(findData.ftLastWriteTime);
            folderList.push_back(std::move(pathInfo));
        }
        else if (fileList != nullptr) {
            DirectoryTree::PathInfo pathInfo;
            pathInfo.m_bFolder = false;
            pathInfo.m_displayName = fileName;
            pathInfo.m_filePath = dirPath;
            pathInfo.m_filePath.JoinFilePath(FilePath(fileName));

            DString typeName;
            pathInfo.m_nIconID = GetFileIconID(pathInfo.m_filePath, fileName, bLargeIcon, &typeName);
            pathInfo.m_typeName = typeName;
            pathInfo.m_fileSize = ((uint64_t)findData.nFileSizeHigh << 32) | (uint64_t)findData.nFileSizeLow;
            pathInfo.m_bIconShared = true;
            pathInfo.m_lastWriteTime.FromFileTime(findData.ftLastWriteTime);
            fileList->push_back(std::move(pathInfo));
        }
    } while (::FindNextFileW(hFind, &findData));
    ::FindClose(hFind);

    std::sort(folderList.begin(), folderList.end(), CompareDisplayName);
    if (fileList != nullptr) {
        std::sort(fileList->begin(), fileList->end(), CompareDisplayName);
    }
}

void DirectoryTreeImpl::GetDiskInfoList(const std::weak_ptr<WeakFlag>& weakFlag,
                                        bool bLargeIcon,
                                        std::vector<DirectoryTree::DiskInfo>& diskInfoList)
{
    diskInfoList.clear();
    std::vector<DirectoryTree::PathInfo> pathInfoList;
    GetRootPathInfoList(bLargeIcon, pathInfoList);
    for (DirectoryTree::PathInfo& pathInfo : pathInfoList) {
        if (weakFlag.expired()) {
            break;
        }
        if (pathInfo.m_filePath.IsEmpty()) {
            continue;
        }
        DiskUtils::DiskInfo diskInfo;
        if (!DiskUtils::GetLogicalDriveInfo(pathInfo.m_filePath.ToString(), diskInfo)) {
            //获取失败时释放该磁盘的图标，避免资源泄漏
            if ((pathInfo.m_nIconID != 0) && !pathInfo.m_bIconShared) {
                GlobalManager::Instance().Icon().RemoveIcon(pathInfo.m_nIconID);
            }
            pathInfo.m_nIconID = 0;
            continue;
        }

        DirectoryTree::DiskInfo info;
        info.m_displayName = pathInfo.m_displayName;
        info.m_nIconID = pathInfo.m_nIconID;
        info.m_bIconShared = pathInfo.m_bIconShared;
        info.m_filePath = pathInfo.m_filePath;
        info.m_volumeName = diskInfo.m_volumeName;
        info.m_volumeType = diskInfo.m_volumeType;
        info.m_deviceType = GetDeviceTypeFromDrive(pathInfo.m_filePath.ToString());
        info.m_fileSystem = diskInfo.m_fileSystem;
        info.m_mountOn = pathInfo.m_filePath.ToString();
        info.m_totalBytes = diskInfo.m_totalBytes;
        info.m_freeBytes = diskInfo.m_freeBytes;
        diskInfoList.push_back(std::move(info));

        //图标所有权已转移到diskInfoList
        pathInfo.m_nIconID = 0;
    }
    //释放剩余未转移的图标
    DirectoryTree::ClearPathInfoList(pathInfoList);
}

bool DirectoryTreeImpl::NeedShowDirPath(const FilePath& path, bool bShowHiddenFiles, bool bShowSystemFiles) const
{
    if (path.IsEmpty()) {
        return true;
    }

    //驱动器根目录总是显示
    DString nativePath = path.ToString();
    if ((nativePath.size() == 3) && (nativePath[1] == _T(':')) && (nativePath[2] == _T('\\'))) {
        wchar_t ch = nativePath[0];
        if (ch >= L'a' && ch <= L'z') {
            ch = (wchar_t)(ch - L'a' + L'A');
        }
        if ((ch >= L'C') && (ch <= L'Z')) {
            return true;
        }
    }

    if (!path.IsExistsDirectory()) {
        return false;
    }

    DWORD attributes = ::GetFileAttributesW(path.NativePath().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    if (!bShowHiddenFiles && (attributes & FILE_ATTRIBUTE_HIDDEN)) {
        return false;
    }
    if (!bShowSystemFiles && (attributes & FILE_ATTRIBUTE_SYSTEM)) {
        return false;
    }
    return true;
}

uint32_t DirectoryTreeImpl::GetMyComputerIconID() const
{
    if (m_impl->m_nMyComputerIconID == 0) {
        LPITEMIDLIST pidl = nullptr;
        if (SUCCEEDED(::SHGetFolderLocation(nullptr, CSIDL_DRIVES, nullptr, 0, &pidl)) && (pidl != nullptr)) {
            SHFILEINFO shFileInfo;
            memset(&shFileInfo, 0, sizeof(shFileInfo));
            HICON hIcon = nullptr;
            if (SHGetFileInfo((LPCWSTR)pidl, 0, &shFileInfo, sizeof(shFileInfo),
                              SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON) != 0) {
                hIcon = shFileInfo.hIcon;
            }
            ::ILFree(pidl);
            if (hIcon != nullptr) {
                m_impl->m_nMyComputerIconID = GlobalManager::Instance().Icon().AddIcon(hIcon);
                ::DestroyIcon(hIcon);
            }
        }
    }
    return m_impl->m_nMyComputerIconID;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN

