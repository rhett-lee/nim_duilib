#include "FileUtil.h"
#include <stdio.h>

namespace ui
{

bool FileUtil::ReadFileData(const FilePath& filePath, std::vector<uint8_t>& fileData)
{
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
        ::fseek(f, 0, SEEK_END);
        int fileSize = ::ftell(f);
        ::fseek(f, 0, SEEK_SET);
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
            size_t nWriteLen = ::fwrite(fileData.data(), 2, fileData.size(), f);
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
