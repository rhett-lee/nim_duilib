#ifndef UI_UTILS_FILEUTIL_H_
#define UI_UTILS_FILEUTIL_H_

#include "duilib/Utils/FilePath.h"

namespace ui
{

/** 文件读写工具类（全部为静态函数）
 *  提供以二进制方式读取整个文件或文件头数据，以及写入二进制数据、宽字符串、窄字符串内容的功能；
 *  所有函数均不抛出异常，成功返回true，失败返回false。
 */
class DUILIB_API FileUtil
{
public:
    /** 读取文件内容（全部内容）
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [out] fileData 输出参数，文件数据，按二进制数据读取；读取前会先清空该缓冲区
    * @return 读取成功返回true（文件为空时也返回true，此时fileData为空）；打开文件失败或读取长度不完整时返回false
    */
    static bool ReadFileData(const FilePath& filePath, std::vector<uint8_t>& fileData);

    /** 读取文件头部分数据内容（部分内容）
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] nReadSize 读取的数据长度（单位：字节，必须大于0；大于文件实际长度时按文件实际长度读取）
    * @param [out] fileHeaderData 输出参数，文件头数据，按二进制数据读取；读取前会先清空该缓冲区
    * @return 读取成功返回true（文件为空时也返回true，此时fileHeaderData为空）；
    *         nReadSize为0、打开文件失败或读取长度不完整时返回false
    */
    static bool ReadFileHeaderData(const FilePath& filePath, uint32_t nReadSize, std::vector<uint8_t>& fileHeaderData);

    /** 写入文件内容（二进制数据）
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据，按字节原样写入
    * @return 写入成功返回true（fileData为空时创建空文件也返回true）；打开文件失败或写入长度不完整时返回false
    */
    static bool WriteFileData(const FilePath& filePath, const std::vector<uint8_t>& fileData);

    /** 写入文件内容（宽字符串数据）
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据，按字符类型原样写入（Windows平台为UTF16，其他平台为UTF32），
    *                      不做编码转换，也不写入BOM头
    * @return 写入成功返回true（fileData为空时创建空文件也返回true）；打开文件失败或写入长度不完整时返回false
    */
    static bool WriteFileData(const FilePath& filePath, const DStringW& fileData);

    /** 写入文件内容（窄字符串数据）
    * @param [in] filePath 本地文件路径(绝对路径)
    * @param [in] fileData 文件数据，按字节原样写入，不做编码转换，也不写入BOM头
    * @return 写入成功返回true（fileData为空时创建空文件也返回true）；打开文件失败或写入长度不完整时返回false
    */
    static bool WriteFileData(const FilePath& filePath, const DStringA& fileData);
};

}

#endif // UI_UTILS_FILEUTIL_H_
