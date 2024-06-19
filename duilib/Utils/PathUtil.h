#ifndef UI_UTILS_PATH_UTIL_H_
#define UI_UTILS_PATH_UTIL_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui
{

/** 路径操作的辅助类
*/
class UILIB_API PathUtil
{
public:

    //对路径中的分隔符/点（"/\\."）字符进行规范化处理, 确保目录以分隔符结束(适用于目录)
    static DString NormalizeDirPath(const DString& strFilePath);

    //对路径中的分隔符/点（"/\\."）字符进行规范化处理, （适用于文件）
    static DString NormalizeFilePath(const DString& strFilePath);

    //连接两个路径，生成一个新的路径，用于路径拼接
    static DString JoinFilePath(const DString& path1, const DString& path2);

    //规范目录，确保以分隔符结束
    static void FormatDirPath(DString& path);

    //判断路径指向的文件是否存在
    static bool IsExistsPath(const DString& strFilePath);

    //判断路径是否为相对路径
    static bool IsRelativePath(const DString& strFilePath);

    //判断路径是否为绝对路径
    static bool IsAbsolutePath(const DString& strFilePath);

    //判断路径执行的文件/文件夹是否存在
    static bool FilePathIsExist(const DString& filePath, bool bDirectory);

    //创建目录（不包含多级目录）
    static bool CreateOneDirectory(const DString& filePath);

    //创建目录（包含多级子目录）
    static bool CreateDirectories(const DString& filePath);

    /** 获取当前进程的程序所在目录
    */
    static DString GetCurrentModuleDirectory();
};

} // namespace ui

#endif // UI_UTILS_PATH_UTIL_H_
