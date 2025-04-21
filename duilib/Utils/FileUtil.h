#ifndef UI_UTILS_FILEUTIL_H_
#define UI_UTILS_FILEUTIL_H_

#include "duilib/Utils/FilePath.h"

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

    /** 写入文件内容
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据
    */
    static bool WriteFileData(const FilePath& filePath, const std::vector<uint8_t>& fileData);

    /** 写入文件内容
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据
    */
    static bool WriteFileData(const FilePath& filePath, const DStringW& fileData);

    /** 写入文件内容
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据
    */
    static bool WriteFileData(const FilePath& filePath, const DStringA& fileData);
};

}

#endif // UI_UTILS_FILEUTIL_H_
