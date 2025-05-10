#include "DiskUtils_Windows.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Utils/StringConvert.h"
#include <shellapi.h>
#include <memory>

bool DiskUtils::GetLogicalDriveList(std::vector<DString>& driveList)
{
    const int32_t maxBufLen = 1022;
    DStringW::value_type tempBuf[maxBufLen + 2];
    DWORD dwSize = ::GetLogicalDriveStringsW(maxBufLen, tempBuf);
    if (dwSize == 0) {
        return false;
    }
    const DStringW::value_type* driveStr = tempBuf;
    std::unique_ptr<DStringW::value_type> spBuf;
    if (dwSize > maxBufLen) {
        DStringW::value_type* szBuf = new DStringW::value_type[dwSize + 2];
        spBuf.reset(szBuf);
        DWORD dwRetSize = ::GetLogicalDriveStringsW(dwSize, szBuf);
        if ( (dwRetSize == 0) || (dwRetSize > dwSize) )
        {
            return false;
        }
        szBuf[dwRetSize] = 0;
        szBuf[dwRetSize + 1] = 0;
        
        dwSize = dwRetSize;
        driveStr = szBuf;
    }
    else {
        tempBuf[dwSize] = 0;
        tempBuf[dwSize + 1] = 0;
    }

    DStringW driveName;
    DWORD begin = 0;
    for (DWORD i = 0; i < dwSize; ++i) {
        if (driveStr[i] == L'\0' && begin != i) {
            driveName = driveStr + begin;
            driveList.push_back(ui::StringConvert::WStringToT(driveName));
            begin = i + 1;
        }
    }
    return true;
}

bool DiskUtils::GetLogicalDriveInfo(const DString& driveString, DiskInfo& diskInfo)
{
    HMODULE hShell32Dll = ::LoadLibrary(_T("Shell32.dll"));
    ASSERT(hShell32Dll != nullptr);
    if (hShell32Dll == nullptr) {
        return false;
    }

    typedef DWORD_PTR (__stdcall *PFNSHGetFileInfo)( LPCWSTR pszPath, DWORD dwFileAttributes, SHFILEINFOW * psfi,
                                           UINT cbFileInfo, UINT uFlags);

    PFNSHGetFileInfo pfnSHGetFileInfo = (PFNSHGetFileInfo)::GetProcAddress(hShell32Dll, "SHGetFileInfoW");
    ASSERT(pfnSHGetFileInfo != nullptr);
    if (pfnSHGetFileInfo == nullptr) {
        ::FreeLibrary(hShell32Dll);
        return false;
    }
    DStringW driveStringW = ui::StringConvert::TToWString(driveString);

    DiskInfo currentDiskInfo;
    SHFILEINFOW shellInfo = {0, };
    DWORD_PTR result = pfnSHGetFileInfo(driveStringW.c_str(),  0, &shellInfo, sizeof(shellInfo), SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME| SHGFI_TYPENAME);
    if (result == 0) {
        ::FreeLibrary(hShell32Dll);
        return false;
    }
    currentDiskInfo.m_volumeName = ui::StringConvert::WStringToT(shellInfo.szDisplayName);
    currentDiskInfo.m_volumeType = ui::StringConvert::WStringToT(shellInfo.szTypeName);
    
    DStringW::value_type volumeNameBuffer[MAX_PATH + 1] = {0};
    DWORD volumeNameSize = MAX_PATH;
    DWORD volumeSerialNumber = 0;
    DWORD maximumComponentLength = 0;
    DWORD fileSystemFlags = 0;
    DStringW::value_type fileSystemNameBuffer[MAX_PATH + 1] = {0};
    DWORD fileSystemNameSize = MAX_PATH;

    if (::GetVolumeInformationW(driveStringW.c_str(), 
                                volumeNameBuffer, 
                                volumeNameSize, 
                                &volumeSerialNumber, 
                                &maximumComponentLength, 
                                &fileSystemFlags, 
                                fileSystemNameBuffer, 
                                fileSystemNameSize) != FALSE) {
        fileSystemNameBuffer[MAX_PATH] = 0;
        currentDiskInfo.m_fileSystem = ui::StringConvert::WStringToT(fileSystemNameBuffer);
        currentDiskInfo.m_hasFileSystem = true;
        
        DWORD dSectorsPerCluster = 0;
        DWORD dBytesPerSector = 0;
        DWORD dNumberOfFreeClusters = 0;
        DWORD dTotalNumberOfClusters = 0;
        if (::GetDiskFreeSpaceW(driveStringW.c_str(), &dSectorsPerCluster, &dBytesPerSector, &dNumberOfFreeClusters, &dTotalNumberOfClusters)) {
            currentDiskInfo.m_clusterBytes = dSectorsPerCluster * dBytesPerSector;
            currentDiskInfo.m_totalBytes   = ((uint64_t)dTotalNumberOfClusters) * dSectorsPerCluster * dBytesPerSector;
            currentDiskInfo.m_freeBytes    = ((uint64_t)dNumberOfFreeClusters)  * dSectorsPerCluster * dBytesPerSector;
        }
    }
    else {
        currentDiskInfo.m_clusterBytes = 0;
        currentDiskInfo.m_totalBytes = 0;
        currentDiskInfo.m_freeBytes = 0;
        currentDiskInfo.m_hasFileSystem = false;
    }
    diskInfo = currentDiskInfo;
    ::FreeLibrary(hShell32Dll);
    return true;
}

DString DiskUtils::GetDriveFromDirectoryPath(const DString& path) 
{
    if ( (path.size() < 2)) {
        return DString();
    }
    if ( (path[1] != _T(':'))) {
        return DString();
    }    
    return path.substr(0, 3);
}

DString DiskUtils::GetMaxFreeSpaceLocalDisk()
{
    uint64_t freeBytes = 0;
    DString maxFreedrive;
    std::vector<DString> driveList;
    GetLogicalDriveList(driveList);
    for (size_t i = 0; i < driveList.size(); ++i) {
        DStringW driveW = ui::StringConvert::TToWString(driveList[i]);
        UINT uType = ::GetDriveTypeW(driveW.c_str());
        if (uType != DRIVE_FIXED) {
            //只需要本地磁盘
            continue;
        }

        DiskInfo diskInfo;
        bool bRet = GetLogicalDriveInfo(ui::StringConvert::WStringToT(driveW), diskInfo);
        if (!bRet) {
            continue;
        }

        if (diskInfo.m_freeBytes > freeBytes) {
            freeBytes = diskInfo.m_freeBytes;
            maxFreedrive = driveList[i];
        }
    }
    return maxFreedrive;
}

uint64_t DiskUtils::GetFreeDiskSpace(const DString& fullDirectory)
{
    uint64_t freeSize = 0;
    DString drive = GetDriveFromDirectoryPath(fullDirectory);
    if (!drive.empty()) {
        DiskInfo diskInfo;
        GetLogicalDriveInfo(drive, diskInfo);
        freeSize = diskInfo.m_freeBytes;
    }
    return freeSize;
}

#endif //DUILIB_BUILD_FOR_WIN
