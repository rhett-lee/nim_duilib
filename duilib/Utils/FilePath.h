#ifndef UI_UTILS_FILE_PATH_H_
#define UI_UTILS_FILE_PATH_H_

#include "duilib/duilib_defs.h"
#include <filesystem>
#include <string>

namespace ui
{
/** 文件路径(封装std::filesystem::path，所有函数均不会抛出异常)
*/
class UILIB_API FilePath
{
public:
    FilePath() = default;
    FilePath(const FilePath&) = default;
    FilePath(FilePath&&) = default;
    ~FilePath() = default;
    FilePath& operator=(const FilePath&) = default;
    FilePath& operator=(FilePath&&) noexcept = default;

    /** 从字符串构造
    * @param [in] filePath 路径字符串，UTF8编码
    */
    explicit FilePath(const std::string& filePath);

    /** 从字符串构造
    * @param [in] filePath 路径字符串，UTF16编码
    */
    explicit FilePath(const std::wstring& filePath);

    /** 从字符串构造
    * @param [in] filePath 路径字符串，UTF8编码
    * @param [in] bLexicallyNormal 路径是否已经规范化
    */
    FilePath(const std::string& filePath, bool bLexicallyNormal);

    /** 从字符串构造
    * @param [in] filePath 路径字符串，UTF16编码
    * @param [in] bLexicallyNormal 路径是否已经规范化
    */
    FilePath(const std::wstring& filePath, bool bLexicallyNormal);

public:
    /** 是否为空
    */
    bool IsEmpty() const noexcept;

    /** 路径是否为相对路径
    */
    bool IsRelativePath() const noexcept;

    /** 路径是否为绝对路径
    */
    bool IsAbsolutePath() const noexcept;

    /** 在本地文件系统中是否存在路径对应的文件或者目录
    */
    bool IsExistsPath() const noexcept;

    /** 在本地文件系统中是否存在路径对应的文件（排除目录）
    */
    bool IsExistsFile() const noexcept;

    /** 在本地文件系统中是否存在路径对应的目录（排除文件）
    */
    bool IsExistsDirectory() const noexcept;

    /** 获取路径分隔符
    */
    DString::value_type GetPathSeparator() const;
    DString GetPathSeparatorStr() const;

    /** 以字符串形式获取本机的路径
    * @return 如果DString是Unicode版，则返回UTF16格式的字符串
    *         如果DString不是Unicode版本: Window平台返回的是Ansi编码的字符串(MBCS), 其他平台返回UTF8编码的字符串
    */
#ifdef DUILIB_UNICODE
    const DString& NativePath() const;
#else
    DString NativePath() const;
#endif

    /** 以字符串形式获取本机的路径
    * @return Window平台返回的是Ansi编码的字符串(MBCS), 其他平台返回UTF8编码的字符串
    */
    DStringA NativePathA() const;

#ifdef DUILIB_UNICODE
    /** 转换为字符串(UTF16编码 或 UTF8编码)
    */
    const DString& ToString() const;
#else
    DString ToString() const;
#endif

    /** 转换为字符串(UTF16编码)
    */
    DStringW ToStringW() const;

    /** 转换为字符串(UTF8编码)
    */
    DStringA ToStringA() const;

    /** 获取当前路径中的文件名部分(UTF16编码/UTF8编码)
    */
    DString GetFileName() const;

public:
    /** 规范目录，确保以分隔符结束
    */
    void FormatPathAsDirectory();

    /** 对路径中的分隔符 / 点（"/\\."）字符进行规范化处理, 确保目录以分隔符结束(适用于目录, 仅限绝对路径)
    */
    void NormalizeDirectoryPath();

    /** 对路径中的分隔符/点（"/\\."）字符进行规范化处理（适用于文件, 仅限绝对路径）
    */
    void NormalizeFilePath();

    /** 删除文件名，只保留文件所在的目录
    */
    void RemoveFileName() noexcept;

    /** 路径赋值
    * @param [in] rightPath 跟随DString的编码：路径为UTF8编码或者UTF16编码
    */
    FilePath& operator = (const DString& rightPath);

    /** 路径连接: 将当前路径与右侧的路径连接后生成一个新的路径
    */
    FilePath& JoinFilePath(const FilePath& rightPath);

    /** 将两个路径拼接成一个路径（按字符串形式拼接，不会在两个路径之间主动添加路径分隔符）
    * @param [in] rightPath 右侧路径
    */
    FilePath& operator += (const FilePath& rightPath);

    /** 将两个路径拼接成一个路径（按字符串形式拼接，不会在两个路径之间主动添加路径分隔符）
    * @param [in] rightPath 跟随DString的编码：路径为UTF8编码或者UTF16编码
    */
    FilePath& operator += (const DString& rightPath);

    /** 比较操作符
    */
    bool operator != (const FilePath& otherPath) const noexcept;
    bool operator == (const FilePath& otherPath) const noexcept;
    bool operator < (const FilePath& otherPath) const noexcept;

    /** 计算路径的Hash值
    */
    size_t HashValue() const noexcept;

    /** 清空
    */
    void Clear() noexcept;

private:
    /** 文件路径(UTF16编码)
    */
    std::filesystem::path m_filePath;

    /** 文件路径是否做过规范化处理
    */
    bool m_bLexicallyNormal = false;
};

}

//为FilePath定义hash算法, 使其可用作为基于哈希表的容器的KEY值
namespace std {
    template <> struct hash<ui::FilePath> {
        size_t operator()(const ui::FilePath& p) const {
            return p.HashValue();
        }
    };
}

#endif // UI_UTILS_FILE_PATH_H_
