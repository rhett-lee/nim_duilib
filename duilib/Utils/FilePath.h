#ifndef UI_UTILS_FILE_PATH_H_
#define UI_UTILS_FILE_PATH_H_

#include "duilib/duilib_defs.h"
#include <filesystem>
#include <string>
#include <vector>

namespace ui
{
/** 文件路径（封装 std::filesystem::path，所有函数均不会抛出异常）
 *  主要功能：
 *  1. 由 UTF8 或 UTF16 字符串构造路径，并提供 UTF8、UTF16、本机编码之间的相互转换；
 *  2. 判断路径是否为相对/绝对路径，判断文件或目录是否存在，获取文件大小；
 *  3. 对路径进行规范化，追加/去除结尾的路径分隔符，以及路径的拼接与比较。
 *  注意：Windows 平台路径分隔符为反斜杠"\\"，其他平台为正斜杠"/"；字符串形式的路径拼接不会自动添加分隔符。
 */
class DUILIB_API FilePath
{
public:
    FilePath();
    FilePath(const FilePath&);
    FilePath(FilePath&&);
    ~FilePath();
    FilePath& operator=(const FilePath&);
    FilePath& operator=(FilePath&&) noexcept;

    /** 从字符串构造路径（路径不视为已规范化）
    * @param [in] filePath 路径字符串，UTF8编码（可为空字符串，表示空路径）
    */
    explicit FilePath(const std::string& filePath);

    /** 从字符串构造路径（路径不视为已规范化）
    * @param [in] filePath 路径字符串，UTF16编码（可为空字符串，表示空路径）
    */
    explicit FilePath(const std::wstring& filePath);

    /** 从字符串构造路径
    * @param [in] filePath 路径字符串，UTF8编码（可为空字符串，表示空路径）
    * @param [in] bLexicallyNormal true表示路径已经是规范化后的形式（如已调用过 lexically_normal），
    *                              false表示未规范化；该标记仅用于状态传递和 Debug 模式下的断言，不影响构造结果
    */
    FilePath(const std::string& filePath, bool bLexicallyNormal);

    /** 从字符串构造路径
    * @param [in] filePath 路径字符串，UTF16编码（可为空字符串，表示空路径）
    * @param [in] bLexicallyNormal true表示路径已经是规范化后的形式（如已调用过 lexically_normal），
    *                              false表示未规范化；该标记仅用于状态传递和 Debug 模式下的断言，不影响构造结果
    */
    FilePath(const std::wstring& filePath, bool bLexicallyNormal);

    /** 与另外一个对象交换数据
    * @param [in,out] r 参与交换的另一个路径对象
    */
    void Swap(FilePath& r);

public:
    /** 是否为空
    * @return 路径为空（未设置）时返回true
    */
    bool IsEmpty() const noexcept;

    /** 路径是否为相对路径
    * @return 路径为相对路径时返回true
    */
    bool IsRelativePath() const noexcept;

    /** 路径是否为绝对路径
    * @return 路径为绝对路径时返回true
    */
    bool IsAbsolutePath() const noexcept;

    /** 在本地文件系统中是否存在路径对应的文件或者目录
    * @return 存在文件或目录时返回true；不存在、无访问权限或获取状态失败时返回false
    */
    bool IsExistsPath() const noexcept;

    /** 在本地文件系统中是否存在路径对应的文件（排除目录）
    * @return 存在普通文件时返回true；不存在、是目录或获取状态失败时返回false
    */
    bool IsExistsFile() const noexcept;

    /** 在本地文件系统中是否存在路径对应的目录（排除文件）
    * @return 存在目录时返回true；不存在、是文件或获取状态失败时返回false
    */
    bool IsExistsDirectory() const noexcept;

    /** 获取文件大小
    * @return 返回文件大小（单位：字节）；路径不存在、路径是目录或获取失败时返回0
    */
    uint64_t GetFileSize() const noexcept;

    /** 获取路径分隔符（字符）
    * @return 返回本机路径分隔符：Windows平台为'\\'，其他平台为'/'
    */
    static DString::value_type GetPathSeparator();

    /** 获取路径分隔符（字符串）
    * @return 返回本机路径分隔符字符串：Windows平台为"\\"，其他平台为"/"
    */
    static DString GetPathSeparatorStr();

    /** 以本机编码的字符串形式获取路径
    * @return Unicode版本(DString为宽字符)返回UTF16编码的字符串引用；
    *         非Unicode版本：Windows平台返回Ansi编码的字符串(MBCS)，其他平台返回UTF8编码的字符串；
    *         路径为空时返回空字符串（非Unicode版本返回的是字符串副本）
    */
#ifdef DUILIB_UNICODE
    const DString& NativePath() const;
#else
    DString NativePath() const;
#endif

    /** 以Ansi编码(MBCS)的字符串形式获取路径
    * @return Windows平台返回Ansi编码的字符串(MBCS)，其他平台返回UTF8编码的字符串；
    *         路径为空时返回空字符串
    */
    DStringA NativePathA() const;

#ifdef DUILIB_UNICODE
    /** 转换为字符串(UTF16编码)
    * @return 返回UTF16编码的字符串引用（直接引用内部路径数据，不产生临时字符串）
    */
    const DString& ToString() const;
#else
#ifdef DUILIB_BUILD_FOR_WIN
    /** 转换为字符串(UTF8编码)
    * @return 返回UTF8编码的字符串（由内部UTF16路径转换而来）；路径为空时返回空字符串
    */
    DString ToString() const;
#else
    /** 转换为字符串(UTF8编码)
    * @return 返回UTF8编码的字符串引用（直接引用内部路径数据，不产生临时字符串）
    */
    const DString& ToString() const;
#endif
#endif

    /** 转换为字符串(UTF16编码)
    * @return 返回UTF16编码的字符串（其他平台的UTF8路径会转换为UTF16）；路径为空时返回空字符串
    */
    DStringW ToStringW() const;

    /** 转换为字符串(UTF8编码)
    * @return 返回UTF8编码的字符串（Windows平台的UTF16路径会转换为UTF8）；路径为空时返回空字符串
    */
    DStringA ToStringA() const;

    /** 获取当前路径中的文件名部分(UTF16编码/UTF8编码)
    * @return 返回文件名（不含目录部分）；路径为空或以路径分隔符结尾时返回空字符串
    */
    DString GetFileName() const;

    /** 获取当前路径中的文件名的后缀部分(UTF16编码/UTF8编码)
    * @return 返回扩展名，包含起始的"."（例如".txt"，大小写保持原样）；无扩展名或路径为空时返回空字符串
    */
    DString GetFileExtension() const;

    /** 获取父路径
    * @return 返回父路径；当前路径为空或没有父路径时返回空路径
    */
    FilePath GetParentPath() const;

public:
    /** 确保以分隔符（Windows平台，以反斜杠"\\"结尾，其他平台以正斜杠"/"结尾）结尾
    *   比如Windows: "C:\\abc" 变成 "C:\\abc\\"；路径为空时不做处理
    */
    void FormatPathAsDirectory();

    /** 去除结尾的路径分隔符
    *   比如Windows: "C:\\abc\\" 变成 "C:\\abc"
    *   若去除后只剩盘符（如"C:"）或根目录（如"\\"），则保留原路径不变，避免改变路径语义
    */
    void TrimRightPathSeparator();

    /** 规范路径，对路径中的分隔符（"/\\."）字符进行规范化处理, 确保目录以分隔符结尾(适用于目录, 仅限绝对路径)
     *  Windows平台，以反斜杠"\\"结尾，其他平台以正斜杠"/"结尾
     *  Windows平台下，将正斜杠"/"替换为反斜杠"\\"，其他平台下，将反斜杠"\\"替换为正斜杠"/"
     *  同时对路径中的相对路径（"." 和 ".."）进行规范化处理，去除相对路径，形成绝对路径
     *  注意：相对路径不做上述规范化处理，仅将分隔符转换为平台首选形式；路径为空时不做处理
     */
    void NormalizeDirectoryPath();

    /** 对路径中的分隔符（"/\\."）字符进行规范化处理（适用于文件、目录, 仅限绝对路径）
     *  Windows平台下，将正斜杠"/"替换为反斜杠"\\"，其他平台下，将反斜杠"\\"替换为正斜杠"/"
     *  同时对路径中的相对路径（"." 和 ".."）进行规范化处理，去除相对路径，形成绝对路径
     *  注意：相对路径不做上述规范化处理，仅将分隔符转换为平台首选形式；路径为空时不做处理
     */
    void NormalizeFilePath();

    /** 删除文件名，只保留文件所在的目录（路径为空时不做处理）
    */
    void RemoveFileName() noexcept;

    /** 当前目录是否为另外一个目录的子目录
    * @param [in] parentPath 父目录（比较前会先规范化路径，并确保以路径分隔符结尾）
    * @return 是子目录时返回true；任一路径为空或规范化后不是其子目录时返回false；
    *         比较时Windows/macOS不区分大小写，Linux/FreeBSD区分大小写
    */
    bool IsSubDirectory(const FilePath& parentPath) const;

    /** 获取父目录列表
    * @param [out] parentPathList 输出参数，函数会向列表末尾追加父目录（不清空原有内容）；
    *                             元素为完整路径，顺序从根目录到直接父目录，不包含当前路径自身；路径为空时不做处理
    */
    void GetParentPathList(std::vector<FilePath>& parentPathList) const;

    /** 路径赋值
    * @param [in] rightPath 右侧路径字符串，编码跟随DString的类型：UTF8编码或者UTF16编码
    * @return 返回自身引用；赋值后路径标记为未规范化
    */
    FilePath& operator = (const DString& rightPath);

    /** 路径连接: 将当前路径与右侧的路径连接后生成一个新的路径
    * @param [in] rightPath 右侧路径；若为绝对路径，则当前路径被替换为该路径
    * @return 返回自身引用；连接后路径标记为未规范化
    */
    FilePath& JoinFilePath(const FilePath& rightPath);

    /** 路径连接: 将当前路径与右侧的路径连接后生成一个新的路径（与JoinFilePath相同）
    * @param [in] rightPath 右侧路径；若为绝对路径，则当前路径被替换为该路径
    * @return 返回自身引用；连接后路径标记为未规范化
    */
    FilePath& operator /= (const FilePath& rightPath);

    /** 将两个路径拼接成一个路径（按字符串形式拼接，不会在两个路径之间主动添加路径分隔符）
    * @param [in] rightPath 右侧路径
    * @return 返回自身引用；拼接后路径标记为未规范化
    */
    FilePath& operator += (const FilePath& rightPath);

    /** 将两个路径拼接成一个路径（按字符串形式拼接，不会在两个路径之间主动添加路径分隔符）
    * @param [in] rightPath 右侧路径字符串，编码跟随DString的类型：UTF8编码或者UTF16编码
    * @return 返回自身引用；拼接后路径标记为未规范化
    */
    FilePath& operator += (const DString& rightPath);

    /** 比较操作符（按std::filesystem::path的比较规则逐字符比较；绝对路径建议先规范化，Debug模式下有断言检查）
    * @param [in] otherPath 参与比较的另一个路径
    * @return operator != 返回两者是否不相等；operator == 返回两者是否相等；
    *         operator < 返回当前路径是否按字典序小于otherPath
    */
    bool operator != (const FilePath& otherPath) const noexcept;
    bool operator == (const FilePath& otherPath) const noexcept;
    bool operator < (const FilePath& otherPath) const noexcept;

    /** 计算路径的Hash值
    * @return 返回路径的哈希值；路径为空时返回0
    */
    size_t HashValue() const noexcept;

    /** 清空路径数据
    */
    void Clear() noexcept;

private:
    /** 文件路径（Windows平台为UTF16编码的宽字符串，其他平台为UTF8编码的窄字符串）
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
