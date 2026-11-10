#include "FileUtil.h"
#include <stdio.h>

namespace ui
{

bool FileUtil::ReadFileData(const FilePath& filePath, std::vector<uint8_t>& fileData)
{
    //先清空输出数据，避免文件为空时返回true但保留调用方上次的旧数据
    fileData.clear();
    bool isReadOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
    #ifdef DUILIB_UNICODE
        ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
    #else
        ::fopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
    #endif
#else
    //Linux平台
    f = fopen(filePath.NativePath().c_str(), _T("rb"));
#endif

    if (f != nullptr) {
        isReadOk = true;
        //使用64位文件长度接口，避免超过2GB的文件在用32位ftell时获取大小失败
#ifdef DUILIB_BUILD_FOR_WIN
        ::fseek(f, 0, SEEK_END);
        const int64_t fileSize = ::_ftelli64(f);
        ::fseek(f, 0, SEEK_SET);
#else
        ::fseek(f, 0, SEEK_END);
        const int64_t fileSize = (int64_t)::ftello(f);
        ::fseek(f, 0, SEEK_SET);
#endif
        if (fileSize > 0) {
            fileData.resize((size_t)fileSize);
            size_t readLen = ::fread(fileData.data(), 1, fileData.size(), f);
            ASSERT_UNUSED_VARIABLE(readLen == fileData.size());
            if (readLen != fileData.size()) {
                fileData.clear();
                isReadOk = false;
            }
        }
        ::fclose(f);
    }
    return isReadOk;
}

bool FileUtil::ReadFileHeaderData(const FilePath& filePath, uint32_t nReadSize, std::vector<uint8_t>& fileHeaderData)
{
    ASSERT(nReadSize > 0);
    if (nReadSize == 0) {
        return false;
    }
    //先清空输出数据，避免文件为空时返回true但保留调用方上次的旧数据
    fileHeaderData.clear();
    bool isReadOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
#ifdef DUILIB_UNICODE
    ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
#else
    ::fopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
#endif
#else
    //Linux平台
    f = fopen(filePath.NativePath().c_str(), _T("rb"));
#endif

    if (f != nullptr) {
        isReadOk = true;
        //使用64位文件长度接口，避免超过2GB的文件在用32位ftell时获取大小失败
#ifdef DUILIB_BUILD_FOR_WIN
        ::fseek(f, 0, SEEK_END);
        const int64_t fileSize = ::_ftelli64(f);
        ::fseek(f, 0, SEEK_SET);
#else
        ::fseek(f, 0, SEEK_END);
        const int64_t fileSize = (int64_t)::ftello(f);
        ::fseek(f, 0, SEEK_SET);
#endif
        if (fileSize > 0) {
            if ((uint64_t)nReadSize > (uint64_t)fileSize) {
                nReadSize = (uint32_t)fileSize;
            }
            fileHeaderData.resize((size_t)nReadSize);
            size_t readLen = ::fread(fileHeaderData.data(), 1, fileHeaderData.size(), f);
            ASSERT_UNUSED_VARIABLE(readLen == fileHeaderData.size());
            if (readLen != fileHeaderData.size()) {
                fileHeaderData.clear();
                isReadOk = false;
            }
        }
        ::fclose(f);
    }
    return isReadOk;
}

bool FileUtil::WriteFileData(const FilePath& filePath, const std::vector<uint8_t>& fileData)
{
    bool isWriteOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
    #ifdef DUILIB_UNICODE
        ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
    #else
        ::fopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
    #endif
#else
    //Linux平台
    f = fopen(filePath.NativePath().c_str(), _T("w+b"));
#endif

    if (f != nullptr) {
        if (!fileData.empty()) {
            size_t nWriteLen = ::fwrite(fileData.data(), 1, fileData.size(), f);
            ASSERT_UNUSED_VARIABLE(nWriteLen == fileData.size());
            if (nWriteLen == fileData.size()) {
                isWriteOk = true;
            }
        }
        ::fclose(f);
    }
    return isWriteOk;
}

bool FileUtil::WriteFileData(const FilePath& filePath, const DStringW& fileData)
{
    bool isWriteOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
#ifdef DUILIB_UNICODE
    ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
#else
    ::fopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
#endif
#else
    //Linux平台
    f = fopen(filePath.NativePath().c_str(), _T("w+b"));
#endif

    if (f != nullptr) {
        if (!fileData.empty()) {
            //按字符类型的实际大小写入，避免非Windows平台(wchar_t为4字节)时数据被截断
            size_t nWriteLen = ::fwrite(fileData.data(), sizeof(DStringW::value_type), fileData.size(), f);
            ASSERT_UNUSED_VARIABLE(nWriteLen == fileData.size());
            if (nWriteLen == fileData.size()) {
                isWriteOk = true;
            }
        }
        ::fclose(f);
    }
    return isWriteOk;
}

bool FileUtil::WriteFileData(const FilePath& filePath, const DStringA& fileData)
{
    bool isWriteOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
#ifdef DUILIB_UNICODE
    ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
#else
    ::fopen_s(&f, filePath.NativePath().c_str(), _T("w+b"));
#endif
#else
    //Linux平台
    f = fopen(filePath.NativePath().c_str(), _T("w+b"));
#endif

    if (f != nullptr) {
        if (!fileData.empty()) {
            size_t nWriteLen = ::fwrite(fileData.data(), 1, fileData.size(), f);
            ASSERT_UNUSED_VARIABLE(nWriteLen == fileData.size());
            if (nWriteLen == fileData.size()) {
                isWriteOk = true;
            }
        }
        ::fclose(f);
    }
    return isWriteOk;
}

}//namespace ui
