#include "ZipManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ZipStreamIO.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePathUtil.h"

#include "duilib/third_party/zlib/zlib.h"
#include "duilib/third_party/zlib/contrib/minizip/unzip.h"

namespace ui 
{
/** 压缩包内的路径，最大长度
*/
#define MAX_PATH_LEN (size_t)(1024)

ZipManager::ZipManager():
    m_hzip(nullptr)
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

#ifdef DUILIB_BUILD_FOR_WIN

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
    return m_hzip != nullptr;
}
#endif

bool ZipManager::OpenZipFile(const FilePath& path, const DString& password)
{
    CloseResZip();
    DStringA nativePath = path.NativePathA();
    if (nativePath.empty()) {
        return false;
    }
    m_password = password;
    m_hzip = ::unzOpen(nativePath.c_str());
    return m_hzip != nullptr;
}

bool ZipManager::GetZipData(const FilePath& path, std::vector<unsigned char>& fileData) const
{
    fileData.clear();
    GlobalManager::Instance().AssertUIThread();
    ASSERT(m_hzip != nullptr);
    if (m_hzip == nullptr) {
        return false;
    }
    const FilePath normalizePath = FilePathUtil::NormalizeFilePath(path);
    std::string filePathA;
    if (!LocateFile(normalizePath, filePathA)) {
        return false;
    }

    size_t fileNameLen = std::max(filePathA.size() + 1, MAX_PATH_LEN);
    std::vector<char> szFileName;
    szFileName.resize(fileNameLen, 0);
    unz_file_info file_info = {0, };
    int nRet = ::unzGetCurrentFileInfo(m_hzip, &file_info, &szFileName[0], (uLong)szFileName.size() - 1, nullptr, 0, nullptr, 0);
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
#ifdef DUILIB_BUILD_FOR_WIN
    #ifdef DUILIB_UNICODE
        password = StringUtil::UnicodeToMBCS(m_password);
    #else
        password = m_password;
    #endif
#else
        password = StringUtil::TToUTF8(m_password);
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

bool ZipManager::IsZipResExist(const FilePath& path) const
{
    GlobalManager::Instance().AssertUIThread();
    if ((m_hzip == nullptr) || path.IsEmpty()) {
        return false;
    }
    if (m_zipPathCache.empty()) {
        //首次查询时，建立缓存，避免每次都需要遍历整个压缩包的文件（::unzLocateFile函数是采用遍历所有文件的方式实现的，性能比较差）
        int nRet = ::unzGoToFirstFile(m_hzip);
        while (nRet == UNZ_OK) {
            size_t fileNameLen = MAX_PATH_LEN;
            std::vector<char> szFileName;
            szFileName.resize(fileNameLen, 0);
            unz_file_info file_info = { 0, };
            nRet = ::unzGetCurrentFileInfo(m_hzip, &file_info, &szFileName[0], (uLong)szFileName.size() - 1, nullptr, 0, nullptr, 0);
            if (nRet == UNZ_OK) {
                //文件名的编码是否为UTF8格式
                bool bUtf8 = file_info.flag & (1 << 11);
                DString fileName = GetZipFilePath(szFileName.data(), bUtf8);

#ifdef DUILIB_BUILD_FOR_WIN
                DStringW innerFilePath = StringUtil::MBCSToUnicode(szFileName.data(), bUtf8 ? CP_UTF8 : CP_ACP);
#else
                DStringW innerFilePath = StringUtil::UTF8ToUTF16(szFileName.data());
#endif
                //压缩包内的文件名，都不区分大小写，转换为小写再比较
                innerFilePath = StringUtil::MakeLowerString(innerFilePath);
                //加入缓存
                m_zipPathCache.insert(innerFilePath);

                //下一个文件
                nRet = ::unzGoToNextFile(m_hzip);
            }
        }
    }

    const FilePath normalizePath = FilePathUtil::NormalizeFilePath(path);
    DStringW innerFilePath = normalizePath.ToStringW();
    innerFilePath = StringUtil::MakeLowerString(innerFilePath);
    NormalizeZipFilePath(innerFilePath);
    auto it = m_zipPathCache.find(innerFilePath);
    if (it != m_zipPathCache.end()) {
        return true;
    }
    return false;
}

bool ZipManager::LocateFile(const FilePath& normalizePath, std::string& filePathA) const
{
    //压缩包内文件路径的编码是每个文件都不同的，定位的时候无法确定文件名编码，只能依次尝试
    filePathA = normalizePath.NativePathA();//优先尝试MBCS编码
    ASSERT(!filePathA.empty());
    if (filePathA.empty()) {
        return false;
    }
    NormalizeZipFilePath(filePathA);
    const int iCaseSensitivity = 2; //不区分大小写
    int nRet = ::unzLocateFile(m_hzip, filePathA.c_str(), iCaseSensitivity);
    if (nRet == UNZ_OK) {
        return true;
    }

    std::string oldFilePathA = filePathA;
    filePathA = normalizePath.ToStringA();//再尝试UTF8编码
    ASSERT(!filePathA.empty());
    if (filePathA.empty()) {
        return false;
    }
    NormalizeZipFilePath(filePathA);
    if (oldFilePathA == filePathA) {
        //路径无变化，不再重复查询
        return false;
    }
    nRet = ::unzLocateFile(m_hzip, filePathA.c_str(), iCaseSensitivity);
    if (nRet == UNZ_OK) {
        return true;
    }
    filePathA.clear();
    return false;
}

void ZipManager::CloseResZip()
{
    if (m_hzip != nullptr) {
        ::unzClose(m_hzip);
        m_hzip = nullptr;
    }
    m_zipPathCache.clear();
    m_pZipStreamIO.reset();
}

bool ZipManager::GetZipFileList(const FilePath& dirPath, std::vector<DString>& fileList) const
{
    fileList.clear();
    GlobalManager::Instance().AssertUIThread();
    DString filePath = dirPath.NativePath();
    if (!filePath.empty() &&
        (filePath[filePath.size() - 1] != _T('\\')) &&
        (filePath[filePath.size() - 1] != _T('/'))) {
        filePath += _T("/");
    }
    DString innerPath = FilePathUtil::NormalizeFilePath(filePath);
    if (innerPath.empty() || (m_hzip == nullptr)) {
        return false;
    }
    //路径分隔符统一替换成 '/'
    NormalizeZipFilePath(innerPath);
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

void ZipManager::NormalizeZipFilePath(std::string& innerFilePath) const
{
    const size_t nCount = innerFilePath.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (innerFilePath[nIndex] == '\\') {
            innerFilePath[nIndex] = '/';
        }
    }
}

void ZipManager::NormalizeZipFilePath(std::wstring& innerFilePath) const
{
    const size_t nCount = innerFilePath.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        if (innerFilePath[nIndex] == L'\\') {
            innerFilePath[nIndex] = L'/';
        }
    }
}

DString ZipManager::GetZipFilePath(const char* szInZipFilePath, bool bUtf8) const
{
    DString filePath;
    if (szInZipFilePath == nullptr) {
        return filePath;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    #ifdef DUILIB_UNICODE
        filePath = StringUtil::MBCSToUnicode(szInZipFilePath, bUtf8 ? CP_UTF8 : CP_ACP);
    #else
        if (bUtf8) {
            filePath = szInZipFilePath;            
        }
        else {
            filePath = StringUtil::MBCSToT(szInZipFilePath);
        }
    #endif
#else
    UNUSED_VARIABLE(bUtf8);
    filePath = StringUtil::UTF8ToT(szInZipFilePath);
#endif
    return filePath;
}

}

