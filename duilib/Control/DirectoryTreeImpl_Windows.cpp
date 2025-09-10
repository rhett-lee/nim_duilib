#include "DirectoryTreeImpl.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/DiskUtils_Windows.h"

#include <ShellApi.h>
#include <Shlobj.h>

namespace ui
{

struct DirectoryTreeImpl::TImpl
{
    /** _T("Shell32.dll") 句柄
    */
    HMODULE m_hShell32Dll = nullptr;

    /** 共享的文件夹图标(大图标)
    */
    uint32_t m_nLargeIconID = 0;

    /** 共享的文件夹图标(小图标)
    */
    uint32_t m_nSmallIconID = 0;
};

DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree)
{
    m_impl = new TImpl;
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    if (m_impl->m_nLargeIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nLargeIconID);
        m_impl->m_nLargeIconID = 0;
    }
    if (m_impl->m_nSmallIconID != 0) {
        GlobalManager::Instance().Icon().RemoveIcon(m_impl->m_nSmallIconID);
        m_impl->m_nSmallIconID = 0;
    }
    if (m_impl->m_hShell32Dll != nullptr) {
        ::FreeLibrary(m_impl->m_hShell32Dll);
        m_impl->m_hShell32Dll = nullptr;
    }
    delete m_impl;
    m_impl = nullptr;
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath.Clear();
    displayName.clear();
    nIconID = 0;

    if (m_impl->m_hShell32Dll == nullptr) {
        m_impl->m_hShell32Dll = ::LoadLibrary(_T("Shell32.dll"));
    }
    if (m_impl->m_hShell32Dll == nullptr) {
        return false;
    }
    int csidl = CSIDL_DESKTOP;
    KNOWNFOLDERID fid = FOLDERID_Desktop;
    switch (type) {
    case VirtualDirectoryType::kUserHome:
        csidl = -1;
        fid = FOLDERID_UserProfiles;        
        break;
    case VirtualDirectoryType::kDesktop:
        csidl = CSIDL_DESKTOP;
        fid = FOLDERID_Desktop;
        break;
    case VirtualDirectoryType::kDocuments:
        csidl = CSIDL_PERSONAL;
        fid = FOLDERID_Documents;
        break;
    case VirtualDirectoryType::kPictures:
        csidl = CSIDL_MYPICTURES;
        fid = FOLDERID_Pictures;
        break;
    case VirtualDirectoryType::kMusic:
        csidl = CSIDL_MYMUSIC;
        fid = FOLDERID_Music;
        break;
    case VirtualDirectoryType::kVideos:
        csidl = CSIDL_MYVIDEO;
        fid = FOLDERID_Videos;
        break;
    case VirtualDirectoryType::kDownloads:
        csidl = -1;
        fid = FOLDERID_Downloads;
        break;
    default:
        break;
    }

    typedef HRESULT (CALLBACK *PFN_SHGetKnownFolderPath)( REFKNOWNFOLDERID rfid,
                                                 DWORD            dwFlags,
                                                 HANDLE           hToken,
                                                 PWSTR * ppszPath );
    typedef HRESULT (CALLBACK *PFN_SHGetKnownFolderIDList)( REFKNOWNFOLDERID rfid,
                                                   DWORD            dwFlags,
                                                   HANDLE           hToken,
                                                   PIDLIST_ABSOLUTE * ppidl );

    PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)::GetProcAddress(m_impl->m_hShell32Dll, "SHGetKnownFolderPath");
    PFN_SHGetKnownFolderIDList pfnSHGetKnownFolderIDList = (PFN_SHGetKnownFolderIDList)::GetProcAddress(m_impl->m_hShell32Dll, "SHGetKnownFolderIDList");

    WCHAR folder[MAX_PATH] = { 0 };
    LPITEMIDLIST lpPidl = nullptr;

    if (pfnSHGetKnownFolderPath != nullptr) {
        PWSTR ppszPath = nullptr;
        pfnSHGetKnownFolderPath(fid, 0, nullptr, &ppszPath);
        if (ppszPath != nullptr) {
            wcscpy_s(folder, ppszPath);
            ::CoTaskMemFree(ppszPath);
        }
    }

    if (pfnSHGetKnownFolderIDList != nullptr) {
        pfnSHGetKnownFolderIDList(fid, 0, nullptr, &lpPidl);
    }

    if (folder[0] == _T('\0')) {
        if (csidl < 0) {
            return false;
        }
        if (!::SHGetSpecialFolderPathW(nullptr, folder, csidl, FALSE)) {
            return false;
        }
    }

    if (lpPidl == nullptr) {
        if (csidl < 0) {
            return false;
        }

        HWND hWnd = nullptr;
        if ((m_pTree != nullptr) && (m_pTree->GetWindow() != nullptr)) {
            m_pTree->GetWindow()->NativeWnd()->GetHWND();
        }

        HRESULT hr = ::SHGetSpecialFolderLocation(hWnd, csidl, &lpPidl);
        if ((hr != S_OK) || (lpPidl == nullptr)) {
            return false;
        }
    }

    bool bRet = false;
    SHFILEINFO shFileInfo = { 0 };
    if (::SHGetFileInfo((LPCTSTR)lpPidl,
                        0,
                        &shFileInfo,
                        sizeof(SHFILEINFO),
                        SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)) {
        displayName = StringConvert::LocalToT(shFileInfo.szDisplayName);
        filePath = FilePath(folder);
        nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
        if (shFileInfo.hIcon != nullptr) {
            ::DestroyIcon(shFileInfo.hIcon);
            shFileInfo.hIcon = nullptr;
        }
        bRet = true;
    }

    if (lpPidl != nullptr) {
        ::CoTaskMemFree(lpPidl);
    }
    if (filePath.IsEmpty()) {
        bRet = false;
    }
    return bRet;
}

void DirectoryTreeImpl::GetRootPathInfoList(bool bLargeIcon, std::vector<DirectoryTree::PathInfo>& pathInfoList)
{
    pathInfoList.clear();
    std::vector<DString> driveList;
    DiskUtils::GetLogicalDriveList(driveList);
    for (auto iter = driveList.begin(); iter != driveList.end(); ++iter) {
        DString driverName = *iter;
        // 过滤A:盘和B:盘
        if (StringUtil::IsEqualNoCase(driverName, _T("A:\\")) ||
            StringUtil::IsEqualNoCase(driverName, _T("B:\\"))) {
            continue;
        }

        uint32_t type = ::GetDriveType(driverName.c_str());
        if ((type != DRIVE_FIXED) && (type != DRIVE_REMOVABLE)) {
            continue;
        }

        FilePath driverPath(driverName);
        if (!driverPath.IsExistsPath()) {
            continue;
        }

        SHFILEINFO shFileInfo;
        ZeroMemory(&shFileInfo, sizeof(SHFILEINFO));
        UINT uFlags = SHGFI_ICON | SHGFI_DISPLAYNAME;
        if (bLargeIcon) {
            uFlags |= SHGFI_LARGEICON;
        }
        else {
            uFlags |= SHGFI_SMALLICON;
        }
        if (::SHGetFileInfo(driverName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), uFlags)) {
            DirectoryTree::PathInfo pathInfo;
            pathInfo.m_filePath = driverPath;
            pathInfo.m_bFolder = true;
            pathInfo.m_displayName = shFileInfo.szDisplayName;
            pathInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
            pathInfo.m_bIconShared = false;
            pathInfoList.push_back(pathInfo);

            if (shFileInfo.hIcon != nullptr) {
                ::DestroyIcon(shFileInfo.hIcon);
                shFileInfo.hIcon = nullptr;
            }
        }
    }
}

/** 获取一个路径的图标和文件类型信息
*/
static bool GetFileInfo_Windows(const DStringW& filePath, uint32_t* pIconId, bool bLargeIcon, DString* szTypeName)
{
    if (pIconId != nullptr) {
        *pIconId = 0;
    }
    if (szTypeName) {
        szTypeName->clear();
    }
    if ((pIconId == nullptr) && (szTypeName == nullptr)) {
        return false;
    }
    SHFILEINFOW shFileInfo;
    ZeroMemory(&shFileInfo, sizeof(SHFILEINFOW));
    UINT uFlags = (pIconId != nullptr) ? SHGFI_ICON : 0;
    if (uFlags & SHGFI_ICON) {
        if (bLargeIcon) {
            uFlags |= SHGFI_LARGEICON;
        }
        else {
            uFlags |= SHGFI_SMALLICON;
        }
    }
    if (szTypeName) {
        uFlags |= SHGFI_TYPENAME;
    }

    bool bRet = false;
    if (::SHGetFileInfoW(filePath.c_str(), 0, &shFileInfo, sizeof(SHFILEINFOW), uFlags)) {
        if (shFileInfo.hIcon != nullptr) {
            if (pIconId != nullptr) {
                *pIconId = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
            }
            ::DestroyIcon(shFileInfo.hIcon);
            shFileInfo.hIcon = nullptr;
        }
        if (szTypeName) {
            *szTypeName = StringConvert::WStringToT(shFileInfo.szTypeName);
        }
        bRet = true;
    }
    return bRet;
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
    FilePath findPath = FilePathUtil::JoinFilePath(path, FilePath(_T("*.*")));
    WIN32_FIND_DATAW findData;
    HANDLE hFile = ::FindFirstFileW(findPath.ToStringW().c_str(), &findData);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }   

    do {
        if (weakFlag.expired()) {
            //已经取消，终止
            break;
        }
        bool bFolder = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false;
        if (!bFolder && (fileList == nullptr)) {
            //如果不是目录，且不需要获取文件时，跳过
            continue;
        }

        if ((m_pTree != nullptr) && !m_pTree->IsShowHidenFiles()) {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
                //不包含隐藏的
                continue;
            }
        }

        if ((m_pTree != nullptr) && !m_pTree->IsShowSystemFiles()) {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
                //不包含系统的
                continue;
            }
        }

        if ((StringUtil::StringCompare(findData.cFileName, L".") == 0) ||
            (StringUtil::StringCompare(findData.cFileName, L"..") == 0)) {
            continue;
        }

        FilePath folderPath = FilePathUtil::JoinFilePath(path, FilePath(findData.cFileName));

        DirectoryTree::PathInfo pathInfo;
        pathInfo.m_filePath = folderPath;
        pathInfo.m_bFolder = bFolder;
        pathInfo.m_displayName = StringConvert::WStringToT(findData.cFileName);
        pathInfo.m_lastWriteTime.FromFileTime(findData.ftLastWriteTime);

        ULARGE_INTEGER li;
        li.LowPart = findData.nFileSizeLow;
        li.HighPart = findData.nFileSizeHigh;
        pathInfo.m_fileSize = li.QuadPart;

        if (bFolder) {
            //文件夹(图标共享，只获取一次)
            pathInfo.m_bIconShared = true;
            pathInfo.m_nIconID = bLargeIcon ? m_impl->m_nLargeIconID : m_impl->m_nSmallIconID;
            uint32_t* pIconId = nullptr;
            if (pathInfo.m_nIconID == 0) {
                pIconId = &pathInfo.m_nIconID;
            }
            if (GetFileInfo_Windows(folderPath.ToStringW(), pIconId, bLargeIcon, &pathInfo.m_typeName)) {
                if (pIconId != nullptr) {
                    if (bLargeIcon) {
                        m_impl->m_nLargeIconID = *pIconId;
                    }
                    else {
                        m_impl->m_nSmallIconID = *pIconId;
                    }
                }
            }
            else {
                ASSERT(0);
            }
        }
        else {
            //文件(图标不共享，每个文件获取一次)
            pathInfo.m_bIconShared = true;
            if (!GetFileInfo_Windows(folderPath.ToStringW(), &pathInfo.m_nIconID, bLargeIcon, &pathInfo.m_typeName)) {
                ASSERT(0);
            }
        }
        if (bFolder) {
            //文件夹
            folderList.emplace_back(std::move(pathInfo));
        }
        else {
            //普通文件
            ASSERT(fileList != nullptr);
            if (fileList != nullptr) {
                fileList->emplace_back(std::move(pathInfo));
            }            
        }
    } while (::FindNextFileW(hFile, &findData));
    ::FindClose(hFile);
    hFile = INVALID_HANDLE_VALUE;
}

bool DirectoryTreeImpl::NeedShowDirPath(const FilePath& path) const
{
    if ((m_pTree == nullptr) || path.IsEmpty()) {
        return false;
    }
    DWORD dwAttributes = ::GetFileAttributesW(path.ToStringW().c_str());
    if (dwAttributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        //不是目录
        return false;
    }

    if (path.NativePath().size() == 3) {
        DString s = path.NativePath();
        if (s.size() == 3) {
            if (((s[0] >= _T('C')) || (s[0] <= _T('Z'))) && (s[1] == _T(':')) && (s[2]) == _T('\\')) {
                //根目录，始终显示，因为后续判断逻辑不正确
                return true;
            }
        }
    }

    if (!m_pTree->IsShowHidenFiles()) {
        if (dwAttributes & FILE_ATTRIBUTE_HIDDEN) {
            //不包含隐藏的
            return false;
        }
    }

    if (!m_pTree->IsShowSystemFiles()) {
        if (dwAttributes & FILE_ATTRIBUTE_SYSTEM) {
            //不包含系统的
            return false;
        }
    }
    return true;
}

/** 获取我的电脑图标
*/
static HICON GetMyComputerIcon_Windows()
{
    HICON hMyComputerIcon = nullptr;
    LPITEMIDLIST pidl;
    if (SUCCEEDED(::SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl))) {
        // 成功获取PIDL
        SHFILEINFO sfi = { 0 };
        if (::SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi),
            SHGFI_PIDL | SHGFI_ICON | SHGFI_LARGEICON)) {
            // 成功获取图标句柄
            hMyComputerIcon = sfi.hIcon;
        }
    }
    ILFree(pidl);
    if (hMyComputerIcon == nullptr) {
        SHFILEINFO sfi = { 0 };
        if (::SHGetFileInfo(_T("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"),
            FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
            SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON)) {
            hMyComputerIcon = sfi.hIcon;
        }
    }
    return hMyComputerIcon;
}

uint32_t DirectoryTreeImpl::GetMyComputerIconID() const
{
    HICON hIcon = GetMyComputerIcon_Windows();
    return GlobalManager::Instance().Icon().AddIcon(hIcon);
}

void DirectoryTreeImpl::GetDiskInfoList(const std::weak_ptr<WeakFlag>& weakFlag,
                                        bool bLargeIcon,
                                        std::vector<DirectoryTree::DiskInfo>& diskInfoList)
{
    std::vector<DirectoryTree::PathInfo> pathInfoList;
    GetRootPathInfoList(bLargeIcon, pathInfoList);
    for (const DirectoryTree::PathInfo& pathInfo : pathInfoList) {
        if (weakFlag.expired()) {
            break;
        }
        DiskUtils::DiskInfo winDiskInfo;
        if (DiskUtils::GetLogicalDriveInfo(pathInfo.m_filePath.ToString(), winDiskInfo)) {
            DirectoryTree::DiskInfo diskInfo;
            diskInfo.m_displayName = pathInfo.m_displayName;
            diskInfo.m_bIconShared = pathInfo.m_bIconShared;
            diskInfo.m_nIconID = pathInfo.m_nIconID;
            diskInfo.m_filePath = pathInfo.m_filePath;

            diskInfo.m_volumeName = winDiskInfo.m_volumeName;
            diskInfo.m_volumeType = winDiskInfo.m_volumeType;
            diskInfo.m_fileSystem = winDiskInfo.m_fileSystem;
            diskInfo.m_totalBytes = winDiskInfo.m_totalBytes;
            diskInfo.m_freeBytes = winDiskInfo.m_freeBytes;

            diskInfoList.emplace_back(std::move(diskInfo));
        }
    }
    if (weakFlag.expired()) {
        diskInfoList.clear();
        for (const DirectoryTree::PathInfo& pathInfo : pathInfoList) {
            if (!pathInfo.m_bIconShared) {
                GlobalManager::Instance().Icon().RemoveIcon(pathInfo.m_nIconID);
            }
        }
    }
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
