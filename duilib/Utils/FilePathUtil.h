#ifndef UI_UTILS_FILE_PATH_UTIL_H_
#define UI_UTILS_FILE_PATH_UTIL_H_

#include "duilib/Utils/FilePath.h"

namespace ui
{
/** 路径操作的辅助类
*/
class UILIB_API FilePathUtil
{
public:
    /** 连接两个路径，生成一个新的路径，用于路径拼接
    * @param [in] path1 第一个路径
    * @param [in] path2 第二个路径
    * @return 返回拼接后的路径
    */
    static FilePath JoinFilePath(const FilePath& path1, const FilePath& path2);

    /** 对路径中的分隔符/点（"/\\."）字符进行规范化处理（适用于文件, 支持绝对路径和相对路径）
    */
    static FilePath NormalizeFilePath(const FilePath& filePath);

    /** 对路径中的分隔符/点（"/\\."）字符进行规范化处理（适用于文件, 支持绝对路径和相对路径）
    */
    static DString NormalizeFilePath(const DString& filePath);

    /** 创建目录（不包含多级目录）
    * @param [in] filePath 需要创建的目录
    */
    static bool CreateOneDirectory(const DString& filePath);

    /** 创建目录（包含多级子目录）
    * @param [in] filePath 需要创建的目录
    */
    static bool CreateDirectories(const DString& filePath);

    /** 获取当前进程的程序所在目录
    * @return 返回获取的目录
    */
    static FilePath GetCurrentModuleDirectory();

#ifdef DUILIB_BUILD_FOR_MACOS
    /** 获取当前进程的程序app bundle资源所在目录
    * @return 返回获取的目录
    */
    static FilePath GetBundleResourcesPath();
#endif
};

} // namespace ui

#endif // UI_UTILS_FILE_PATH_UTIL_H_
