#ifndef UI_UTILS_FILEUTIL_H_
#define UI_UTILS_FILEUTIL_H_

#include "duilib/Utils/FilePath.h"
#include <vector>

namespace ui
{

class UILIB_API FileUtil
{
public:
    /** 读取文件内容
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [out] fileData 文件数据，按二进制数据读取
    */
    static bool ReadFileData(const FilePath& filePath, std::vector<uint8_t>& fileData);
};

}

#endif // UI_UTILS_FILEUTIL_H_
