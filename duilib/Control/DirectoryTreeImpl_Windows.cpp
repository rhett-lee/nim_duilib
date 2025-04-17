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
DirectoryTreeImpl::DirectoryTreeImpl(DirectoryTree* pTree):
    m_pTree(pTree),
    m_hShell32Dll(nullptr),
    m_nSharedIconID(0)
{
}

DirectoryTreeImpl::~DirectoryTreeImpl()
{
    if (m_hShell32Dll != nullptr) {
        ::FreeLibrary(m_hShell32Dll);
        m_hShell32Dll = nullptr;
    }
}

bool DirectoryTreeImpl::GetVirtualDirectoryInfo(VirtualDirectoryType type, FilePath& filePath, DString& displayName, uint32_t& nIconID)
{
    filePath.Clear();
    displayName.clear();
    nIconID = 0;

    if (m_hShell32Dll == nullptr) {
        m_hShell32Dll = ::LoadLibrary(_T("Shell32.dll"));
    }
    if (m_hShell32Dll == nullptr) {
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

    PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)::GetProcAddress(m_hShell32Dll, "SHGetKnownFolderPath");
    PFN_SHGetKnownFolderIDList pfnSHGetKnownFolderIDList = (PFN_SHGetKnownFolderIDList)::GetProcAddress(m_hShell32Dll, "SHGetKnownFolderIDList");

    WCHAR folder[MAX_PATH] = { 0 };
    LPITEMIDLIST lpPidl = nullptr;

    if (pfnSHGetKnownFolderPath != nullptr) {
        PWSTR ppszPath = nullptr;
        pfnSHGetKnownFolderPath(fid, 0, nullptr, &ppszPath);
        wcscpy_s(folder, ppszPath);
        if (ppszPath != nullptr) {
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

void DirectoryTreeImpl::GetRootPathInfoList(std::vector<DirectoryTree::PathInfo>& pathInfoList)
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
        if (::SHGetFileInfo(driverName.c_str(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_DISPLAYNAME)) {
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

void DirectoryTreeImpl::GetFolderContents(const FilePath& path,
                                          const std::weak_ptr<WeakFlag>& weakFlag,
                                          std::vector<DirectoryTree::PathInfo>& folderList,
                                          std::vector<DirectoryTree::PathInfo>* fileList)
{
    folderList.clear();
    FilePath findPath = FilePathUtil::JoinFilePath(path, FilePath(_T("*.*")));
    WIN32_FIND_DATAW findData;
    HANDLE hFile = ::FindFirstFileW(findPath.ToStringW().c_str(), &findData);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }   

    do {
        if (weakFlag.expired()) {
            //已经取消，终止
            folderList.clear();
            break;
        }
        bool bFolder = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? true : false;
        if (!bFolder && (fileList == nullptr)) {
            //如果不是目录，则跳过
            continue;
        }

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
            (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
            //不包含系统和隐藏的
            continue;
        }

        if (StringUtil::IsEqualNoCase(findData.cFileName, L".") ||
            StringUtil::IsEqualNoCase(findData.cFileName, L"..")) {
            continue;
        }

        FilePath folderPath = FilePathUtil::JoinFilePath(path, FilePath(findData.cFileName));

        DirectoryTree::PathInfo pathInfo;
        pathInfo.m_filePath = folderPath;
        pathInfo.m_bFolder = bFolder;
        pathInfo.m_displayName = StringConvert::WStringToT(findData.cFileName);
        pathInfo.m_nIconID = m_nSharedIconID;
        pathInfo.m_bIconShared = pathInfo.m_nIconID != 0 ? true : false;
        if (!bFolder || (m_nSharedIconID == 0)) {
            SHFILEINFOW shFileInfo;
            ZeroMemory(&shFileInfo, sizeof(SHFILEINFOW));
            if (::SHGetFileInfoW(folderPath.ToStringW().c_str(), 0, &shFileInfo, sizeof(SHFILEINFOW), SHGFI_ICON | SHGFI_SMALLICON)) {
                pathInfo.m_nIconID = GlobalManager::Instance().Icon().AddIcon(shFileInfo.hIcon);
                pathInfo.m_bIconShared = bFolder ? true : false;//文件夹的图标共享，只取一次
                if (pathInfo.m_bIconShared) {
                    m_nSharedIconID = pathInfo.m_nIconID;
                }
                if (shFileInfo.hIcon != nullptr) {
                    ::DestroyIcon(shFileInfo.hIcon);
                    shFileInfo.hIcon = nullptr;
                }
            }
            else {
                pathInfo.m_nIconID = 0;
                pathInfo.m_bIconShared = false;
            }
        }
        if (bFolder) {
            //文件夹
            folderList.push_back(pathInfo);
        }
        else {
            //普通文件
            ASSERT(fileList != nullptr);
            if (fileList != nullptr) {
                fileList->push_back(pathInfo);
            }            
        }
    } while (::FindNextFileW(hFile, &findData));
    ::FindClose(hFile);
    hFile = INVALID_HANDLE_VALUE;
}


} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
