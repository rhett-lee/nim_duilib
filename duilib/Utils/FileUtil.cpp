#include "FileUtil.h"
#include <stdio.h>

namespace ui
{

bool FileUtil::ReadFileData(const FilePath& filePath, std::vector<uint8_t>& fileData)
{
    bool isReadOk = false;
    FILE* f = nullptr;
#ifdef DUILIB_UNICODE
    errno_t ret = ::_wfopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
#else
    errno_t ret = ::fopen_s(&f, filePath.NativePath().c_str(), _T("rb"));
#endif
    if ((ret == 0) && (f != nullptr)) {
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

}//namespace ui
