#include "ZipManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ZipStreamIO.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PathUtil.h"

#include "duilib/third_party/zlib/zlib.h"
#include "duilib/third_party/zlib/contrib/minizip/unzip.h"

namespace ui 
{
/** 压缩包内的路径，最大长度
*/
#define MAX_PATH_LEN (size_t)(1024)

ZipManager::ZipManager():
    m_hzip(nullptr),
    m_bUtf8(false)
{
}

ZipManager::~ZipManager()
{
    CloseResZip();
}


bool ZipManager::IsUseZip() const
{
    return m_hzip != nullptr;
}

#ifdef DUILIB_PLATFORM_WIN

bool ZipManager::OpenResZip(HMODULE hModule, LPCTSTR resourceName, LPCTSTR resourceType, const DString& password)
{
    HRSRC rsc = ::FindResource(hModule, resourceName, resourceType);
    ASSERT(rsc != nullptr);
    if (rsc == nullptr) {
        return false;
    }
    uint8_t* pData = (uint8_t*)::LoadResource(NULL, rsc);
    uint32_t nDataSize = ::SizeofResource(NULL, rsc);
    ASSERT((pData != nullptr) && (nDataSize > 0));
    if ((pData == nullptr) || (nDataSize == 0)) {
        return false;
    }
    CloseResZip();
    m_password = password;
    m_pZipStreamIO = std::make_unique<ZipStreamIO>(pData, nDataSize);
    zlib_filefunc_def pzlib_filefunc_def;
    m_pZipStreamIO->FillFopenFileFunc(&pzlib_filefunc_def);
    m_hzip = ::unzOpen2(nullptr, &pzlib_filefunc_def);
    InitUtf8();
    return m_hzip != nullptr;
}
#endif

bool ZipManager::OpenZipFile(const DString& path, const DString& password)
{
    CloseResZip();
    m_password = password;
    std::string filePath;
#ifdef DUILIB_PLATFORM_WIN
    StringUtil::UnicodeToMBCS(path, filePath);
#else
    filePath = TToUTF8(path);
#endif
    m_hzip = ::unzOpen(filePath.c_str());
    InitUtf8();
    return m_hzip != nullptr;
}

void ZipManager::InitUtf8()
{
    //读取压缩包内文件名编码是否为UTF8格式（只读取第一个文件）
    m_bUtf8 = false;
    if (m_hzip != nullptr) {
        unz_file_info file_info = { 0, };
        int nRet = ::unzGetCurrentFileInfo(m_hzip, &file_info, nullptr, 0, nullptr, 0, nullptr, 0);
        if (nRet == UNZ_OK) {
            m_bUtf8 = file_info.flag & (1 << 11);
        }
    }
}

bool ZipManager::GetZipData(const DString& path, std::vector<unsigned char>& fileData) const
{
    fileData.clear();
    GlobalManager::Instance().AssertUIThread();
    ASSERT(m_hzip != nullptr);
    if (m_hzip == nullptr) {
        return false;
    }
    std::string filePathA = GetZipFilePathA(path, m_bUtf8);
    ASSERT(!filePathA.empty());
    if (filePathA.empty()) {
        return false;
    }

    int nRet = ::unzLocateFile(m_hzip, filePathA.c_str(), 0);
    if (nRet != UNZ_OK) {
        return false;
    }
    size_t fileNameLen = std::max(filePathA.size() + 1, MAX_PATH_LEN);
    std::vector<char> szFileName;
    szFileName.resize(fileNameLen, 0);
    unz_file_info file_info = {0, };
    nRet = ::unzGetCurrentFileInfo(m_hzip, &file_info, &szFileName[0], (uLong)szFileName.size() - 1, nullptr, 0, nullptr, 0);
    if (nRet != UNZ_OK) {
        return false;
    }
    if (file_info.uncompressed_size == 0) {
        return false;
    }

    //文件是否加密
    bool bFileEncrypted = file_info.flag & 1;

    nRet = ::unzStringFileNameCompare(szFileName.data(), filePathA.c_str(), 0);
    ASSERT(nRet == 0);
    if (nRet != 0) {
        return false;
    }
    if (!m_password.empty() && bFileEncrypted) {
        //密码是本地编码的（ANSI）
        std::string password;
#ifdef DUILIB_PLATFORM_WIN
        StringUtil::UnicodeToMBCS(m_password, password);
#else
        password = TToUTF8(m_password);
#endif
        nRet = ::unzOpenCurrentFilePassword(m_hzip, password.c_str());
    }
    else {
        nRet = ::unzOpenCurrentFile(m_hzip);
    }
    if (nRet != UNZ_OK) {
        return false;
    }

    fileData.resize(file_info.uncompressed_size);
    nRet = ::unzReadCurrentFile(m_hzip, &fileData[0], (uLong)fileData.size());
    ::unzCloseCurrentFile(m_hzip);
    ASSERT(nRet == fileData.size());
    if (nRet != fileData.size()) {
        fileData.clear();
        return false;
    }
    return true;
}

bool ZipManager::IsZipResExist(const DString& path) const
{
    GlobalManager::Instance().AssertUIThread();
    if ((m_hzip != nullptr) && !path.empty()) {
        std::string filePathA = GetZipFilePathA(path, m_bUtf8);
        ASSERT(!filePathA.empty());
        if (filePathA.empty()) {
            return false;
        }
        auto it = m_zipPathCache.find(filePathA);
        if (it != m_zipPathCache.end()) {
            return true;
        }
        int nRet = ::unzLocateFile(m_hzip, filePathA.c_str(), 0);
        if (nRet == UNZ_OK) {
            m_zipPathCache.insert(filePathA);
            return true;
        }
    }
    return false;
}

void ZipManager::CloseResZip()
{
    if (m_hzip != nullptr) {
        ::unzClose(m_hzip);
        m_hzip = nullptr;
    }
    m_zipPathCache.clear();
    m_bUtf8 = false;
    m_pZipStreamIO.reset();
}

bool ZipManager::GetZipFileList(const DString& path, std::vector<DString>& fileList) const
{
    fileList.clear();
    GlobalManager::Instance().AssertUIThread();
    DString filePath = path;
    if (!filePath.empty() &&
        (filePath[filePath.size() - 1] != _T('\\')) &&
        (filePath[filePath.size() - 1] != _T('/'))) {
        filePath += _T("/");
    }
    DString innerPath = GetZipFilePath(filePath);
    if (innerPath.empty() || (m_hzip == nullptr)) {
        return false;
    }
    int nRet = ::unzGoToFirstFile(m_hzip);
    if (nRet != UNZ_OK) {
        return false;
    }
    DString fileName;
    while (nRet == UNZ_OK) {
        size_t fileNameLen = MAX_PATH_LEN;
        std::vector<char> szFileName;
        szFileName.resize(fileNameLen, 0);
        unz_file_info file_info = { 0, };
        nRet = ::unzGetCurrentFileInfo(m_hzip, &file_info, &szFileName[0], (uLong)szFileName.size() - 1, nullptr, 0, nullptr, 0);
        if (nRet != UNZ_OK) {
            return false;
        }

        //文件名的编码是否为UTF8格式
        bool bUtf8 = file_info.flag & (1 << 11);
        fileName = GetZipFilePath(szFileName.data(), bUtf8);

        // zip has an 'attribute' 32bit value. Its lower half is windows stuff
        // its upper half is standard unix stat.st_mode. We'll start trying
        // to read it in unix mode

        //文件名是否是目录
        bool bDir = (file_info.external_fa & 0x40000000) != 0;
        // but in normal hostmodes these are overridden by the lower half...
        int host = file_info.version >> 8;
        if (host == 0 || host == 7 || host == 11 || host == 14) {
            //0 - FAT filesystem (MS-DOS, OS/2, NT/Win32)
            //7 - Macintosh
            //11 - NTFS filesystem (NT)
            //14 - VFAT
            bDir = (file_info.external_fa & 0x00000010) != 0;
        }
        if (!bDir) {
            size_t nPos = fileName.find(innerPath);
            if ((nPos == 0) && (fileName.size() > innerPath.size())) {
                fileName = fileName.substr(innerPath.size());
                if (fileName.find(_T('/')) == DString::npos) {
                    fileList.push_back(fileName);
                }
            }
        }

        //跳到下一个文件
        nRet = ::unzGoToNextFile(m_hzip);
    }
    return true;
}


DString ZipManager::GetZipFilePath(const DString& path) const
{
    if (path.empty() || !PathUtil::IsRelativePath(path)) {
        return _T("");
    }
    DString filePath;
    if ((path[path.size() - 1] == _T('\\')) || (path[path.size() - 1] == _T('/'))) {
        filePath = PathUtil::NormalizeDirPath(path);
    }
    else {
        filePath = PathUtil::NormalizeFilePath(path);
    }
    StringUtil::ReplaceAll(_T("\\"), _T("/"), filePath);
    StringUtil::ReplaceAll(_T("//"), _T("/"), filePath);
    return filePath;
}

std::string ZipManager::GetZipFilePathA(const DString& path, bool bUtf8) const
{
    DString filePath = GetZipFilePath(path);
    ASSERT(!filePath.empty());
    if (filePath.empty()) {
        return std::string();
    }
    std::string filePathA;
#ifdef DUILIB_PLATFORM_WIN
    StringUtil::UnicodeToMBCS(filePath, filePathA, bUtf8 ? CP_UTF8 : CP_ACP);
#else
    filePathA = TToUTF8(filePath);
#endif
    return filePathA;
}

DString ZipManager::GetZipFilePath(const char* szInZipFilePath, bool bUtf8) const
{
    DString filePath;
    if (szInZipFilePath == nullptr) {
        return filePath;
    }
#ifdef DUILIB_PLATFORM_WIN
    StringUtil::MBCSToUnicode(szInZipFilePath, filePath, bUtf8 ? CP_UTF8 : CP_ACP);
#else
    filePath = UTF8ToT(szInZipFilePath);
#endif
    return filePath;
}

}

