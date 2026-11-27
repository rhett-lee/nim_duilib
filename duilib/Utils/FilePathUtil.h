#ifndef UI_UTILS_FILE_PATH_UTIL_H_
#define UI_UTILS_FILE_PATH_UTIL_H_

#include "duilib/Utils/FilePath.h"

namespace ui
{
/** 路径操作的辅助类（全部为静态函数）
 *  提供路径拼接、路径规范化、目录创建、文件扩展名获取、程序目录获取等常用路径操作；
 *  所有函数均不抛出异常，失败时返回空路径或false。
 */
class DUILIB_API FilePathUtil
{
public:
    /** 连接两个路径，生成一个新的路径，用于路径拼接
    * @param [in] path1 第一个路径（基础路径）
    * @param [in] path2 第二个路径；若为绝对路径，则结果直接采用path2，否则拼接到path1之后
    * @return 返回拼接后的新路径（path2没有根路径时，会自动添加必要的路径分隔符）
    */
    static FilePath JoinFilePath(const FilePath& path1, const FilePath& path2);

    /** 对路径中的分隔符和"."/".."进行规范化处理（适用于文件, 支持绝对路径和相对路径）
    * @param [in] filePath 待规范化的路径
    * @return 返回规范化后的新路径；发生异常时返回空路径
    */
    static FilePath NormalizeFilePath(const FilePath& filePath);

    /** 对路径中的分隔符和"."/".."进行规范化处理（适用于文件, 支持绝对路径和相对路径）
    * @param [in] filePath 待规范化的路径字符串，编码跟随DString的类型：UTF8编码或者UTF16编码
    * @return 返回规范化后的新字符串（编码与入口参数一致）；发生异常时返回空字符串
    */
    static DString NormalizeFilePath(const DString& filePath);

    /** 创建目录（不包含多级目录）
    * @param [in] filePath 需要创建的目录（父目录必须已存在）
    * @return 创建成功返回true；目录已存在、filePath为空或发生异常时返回false
    */
    static bool CreateOneDirectory(const DString& filePath);

    /** 创建目录（包含多级子目录）
    * @param [in] filePath 需要创建的目录，路径中不存在中间目录时会逐级创建
    * @return 创建成功返回true；目录已存在、filePath为空或发生异常时返回false
    */
    static bool CreateDirectories(const DString& filePath);

    /** 获取文件路径中的文件名的后缀部分(UTF16编码/UTF8编码)
    * @param [in] filePath 文件路径
    * @return 返回文件的扩展名部分，不包含"."，且已转换为大写；无扩展名或路径以分隔符结尾时返回空字符串
    */
    static DString GetFileExtension(const DString& filePath);

    /** 获取当前进程的程序所在目录
    * @return 返回获取的目录；Windows平台获取失败时返回空路径，其他平台获取失败时退回当前工作目录
    */
    static FilePath GetCurrentModuleDirectory();

#ifdef DUILIB_BUILD_FOR_MACOS
    /** 获取当前进程的程序app bundle资源所在目录（仅macOS平台）
    * @return 返回获取的目录；获取失败时返回空路径
    */
    static FilePath GetBundleResourcesPath();
#endif
};

} // namespace ui

#endif // UI_UTILS_FILE_PATH_UTIL_H_
