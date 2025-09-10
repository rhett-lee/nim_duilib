#include "FilePath.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

FilePath::FilePath(const std::string& filePath) :
#ifdef DUILIB_BUILD_FOR_WIN
    m_filePath(StringConvert::UTF8ToWString(filePath)),
#else
    m_filePath(filePath),
#endif
    m_bLexicallyNormal(false)
{
}

FilePath::FilePath(const std::wstring& filePath) :
#ifdef DUILIB_BUILD_FOR_WIN
    m_filePath(filePath),
#else
    m_filePath(StringConvert::WStringToUTF8(filePath)),
#endif
    m_bLexicallyNormal(false)
{
}

FilePath::FilePath(const std::string& filePath, bool bLexicallyNormal):
#ifdef DUILIB_BUILD_FOR_WIN
    m_filePath(StringConvert::UTF8ToWString(filePath)),
#else
    m_filePath(filePath),
#endif
    m_bLexicallyNormal(bLexicallyNormal)
{
}

FilePath::FilePath(const std::wstring& filePath, bool bLexicallyNormal) :
#ifdef DUILIB_BUILD_FOR_WIN
    m_filePath(filePath),
#else
    m_filePath(StringConvert::WStringToUTF8(filePath)),
#endif
    m_bLexicallyNormal(bLexicallyNormal)
{
}

void FilePath::Swap(FilePath& r)
{
    m_filePath.swap(r.m_filePath);
    std::swap(m_bLexicallyNormal, r.m_bLexicallyNormal);
}

bool FilePath::IsEmpty() const noexcept
{
    return m_filePath.empty();
}

bool FilePath::IsRelativePath() const noexcept
{
    return m_filePath.is_relative();
}

bool FilePath::IsAbsolutePath() const noexcept
{
    return m_filePath.is_absolute();
}

bool FilePath::IsExistsPath() const noexcept
{
    std::error_code errorCode;
    std::filesystem::file_status fileStatus = std::filesystem::status(m_filePath, errorCode);
    if (errorCode.value() != 0) {
        return false;
    }
    return std::filesystem::is_regular_file(fileStatus) || std::filesystem::is_directory(fileStatus);
}

bool FilePath::IsExistsFile() const noexcept
{
    std::error_code errorCode;
    std::filesystem::file_status fileStatus = std::filesystem::status(m_filePath, errorCode);
    if (errorCode.value() != 0) {
        return false;
    }
    return std::filesystem::is_regular_file(fileStatus);
}

bool FilePath::IsExistsDirectory() const noexcept
{
    std::error_code errorCode;
    std::filesystem::file_status fileStatus = std::filesystem::status(m_filePath, errorCode);
    if (errorCode.value() != 0) {
        return false;
    }
    return std::filesystem::is_directory(fileStatus);
}

DString::value_type FilePath::GetPathSeparator()
{
#ifdef DUILIB_BUILD_FOR_WIN
    return _T('\\');
#else
    return _T('/');
#endif
}

DString FilePath::GetPathSeparatorStr()
{
#ifdef DUILIB_BUILD_FOR_WIN
    return _T("\\");
#else
    return _T("/");
#endif
}

#ifdef DUILIB_UNICODE
    //Unicode版本
    const DStringW& FilePath::NativePath() const
    {
        return m_filePath.native();
    }
#else
    //非Unicode版本
    DStringA FilePath::NativePath() const
    {
        if (m_filePath.empty()) {
            return DStringA();
        }
        #ifdef DUILIB_BUILD_FOR_WIN
            //转换为本机编码类型的字符串
            return StringConvert::UnicodeToMBCS(m_filePath.native());
        #else
            return m_filePath.native();
        #endif
    }
#endif

DStringA FilePath::NativePathA() const
{
    if (m_filePath.empty()) {
        return DStringA();
    }
#ifdef DUILIB_BUILD_FOR_WIN
    //转换为本机编码类型的字符串
    return StringConvert::UnicodeToMBCS(m_filePath.native());
#else
    return m_filePath.native();
#endif
}

#ifdef DUILIB_UNICODE
const DString& FilePath::ToString() const
{
    return m_filePath.native();
}
#else
#ifdef DUILIB_BUILD_FOR_WIN
DString FilePath::ToString() const
{
    if (m_filePath.empty()) {
        return DStringA();
    }
    return StringConvert::WStringToUTF8(m_filePath.native());
}
#else
const DString& FilePath::ToString() const
{
    return m_filePath.native();
}
#endif
#endif

DStringW FilePath::ToStringW() const
{
#ifdef DUILIB_BUILD_FOR_WIN
    return m_filePath.native();
#else
    return StringConvert::UTF8ToWString(m_filePath.native());
#endif
}
 
DStringA FilePath::ToStringA() const
{
    if (m_filePath.empty()) {
        return DStringA();
    }
#ifdef DUILIB_BUILD_FOR_WIN
    return StringConvert::WStringToUTF8(m_filePath.native());
#else
    return m_filePath.native();
#endif
}

DString FilePath::GetFileName() const
{
    if (m_filePath.empty()) {
        return DString();
    }
#ifdef DUILIB_UNICODE
    return m_filePath.filename().native();
#else
    #ifdef DUILIB_BUILD_FOR_WIN
        return StringConvert::WStringToUTF8(m_filePath.filename().native());
    #else
        return m_filePath.filename().native();
    #endif
#endif
}

DString FilePath::GetFileExtension() const
{
    if (m_filePath.empty()) {
        return DString();
    }
#ifdef DUILIB_UNICODE
    return m_filePath.extension().native();
#else
    #ifdef DUILIB_BUILD_FOR_WIN
        return StringConvert::WStringToUTF8(m_filePath.extension().native());
    #else
        return m_filePath.extension().native();
    #endif
#endif
}

FilePath FilePath::GetParentPath() const
{
    FilePath filePath;
    if (m_filePath.empty()) {
        return filePath;
    }
    filePath.m_filePath = m_filePath.parent_path();
    filePath.m_bLexicallyNormal = m_bLexicallyNormal;
    return filePath;
}

void FilePath::FormatPathAsDirectory()
{
    //确保路径最后字符是分割字符
    const std::filesystem::path::string_type& filePath = m_filePath.native();
    if (!filePath.empty()) {
        if (filePath.back() != GetPathSeparator()) {
            //结尾不是路径分隔符的话，追加路径分隔符
            m_filePath += GetPathSeparatorStr();
        }
    }    
}

void FilePath::TrimRightPathSeparator()
{
    if (m_filePath.empty()) {
        return;
    }
    const std::filesystem::path::string_type& str = m_filePath.native();
#ifdef DUILIB_BUILD_FOR_WIN
    if (str == L"/") {
#else
    if (str == _T("/")) {
#endif
        return;
    }
    if (!str.empty() && str[str.size() - 1] == FilePath::GetPathSeparator()) {
        std::filesystem::path::string_type filePath = m_filePath.native();
        m_filePath = filePath.substr(0, str.size() - 1);
    }
}

void FilePath::NormalizeDirectoryPath()
{
    if (!m_filePath.empty()) {
        //路径规范化
        NormalizeFilePath();

        //确保路径最后字符是分割字符
        FormatPathAsDirectory();
    }
}

void FilePath::NormalizeFilePath()
{
    try {
        //只对绝对路径进行规则化处理，相对路径在规则化的时候，会有错误的结果
        if (m_filePath.is_absolute()) {
#ifndef DUILIB_BUILD_FOR_WIN
            if (m_filePath.native().find(_T('\\')) != std::filesystem::path::string_type::npos) {
                std::filesystem::path::string_type oldValue = m_filePath.native();
                StringUtil::ReplaceAll(_T("\\"), _T("/"), oldValue);
                m_filePath = oldValue;
            }
#endif
            m_filePath = m_filePath.lexically_normal();
            m_bLexicallyNormal = true;
        }
        else {
            //将"/"替换成"\\"
            m_filePath.make_preferred();
            m_bLexicallyNormal = false;
        }
    }
    catch (...) {
    }
}

void FilePath::RemoveFileName() noexcept
{
    m_filePath.remove_filename();
}

bool FilePath::IsSubDirectory(const FilePath& parentPath) const
{
    if (IsEmpty() || parentPath.IsEmpty()) {
        return false;
    }
    FilePath parent(parentPath);
    parent.NormalizeDirectoryPath();

    FilePath child(*this);
    child.NormalizeDirectoryPath();

    std::filesystem::path::string_type parentStr = parent.m_filePath.native();
    std::filesystem::path::string_type childStr = child.m_filePath.native();
    if (childStr.size() <= parentStr.size()) {
        return false;
    }
#if !defined (DUILIB_BUILD_FOR_LINUX) && !defined (DUILIB_BUILD_FOR_FREEBSD)
    //Windows/MacOS文件名不区分大小写，Linux/FreeBSD区分大小写
    parentStr = StringUtil::MakeLowerString(parentStr);
    childStr = StringUtil::MakeLowerString(childStr);
#endif
    return childStr.find(parentStr) == 0;
}

void FilePath::GetParentPathList(std::vector<FilePath>& parentPathList) const
{
    const std::filesystem::path& path = m_filePath;
    if (path.empty()) {
        return;
    }
    std::vector<std::filesystem::path> path_list;
    auto p = path.parent_path();
    auto rootPath = path.root_path();
    while (!p.empty()) {
        path_list.push_back(p);
        if (p == rootPath) {
            break;
        }
        p = p.parent_path();
        if (p == path_list.back()) {
            break;
        }
    }
    std::reverse(path_list.begin(), path_list.end());
    for (const auto& pathP : path_list) {
        FilePath parentPath;
        parentPath.m_filePath = pathP;
        parentPath.m_bLexicallyNormal = m_bLexicallyNormal;
        parentPathList.emplace_back(std::move(parentPath));
    }
}

FilePath& FilePath::operator = (const DString& rightPath)
{
#ifdef DUILIB_UNICODE
    m_filePath = rightPath;
#else
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW rightPathW = StringConvert::UTF8ToWString(rightPath);
    m_filePath = rightPathW;
#else
    m_filePath = rightPath;
#endif
#endif
    m_bLexicallyNormal = false;
    return *this;
}

FilePath& FilePath::JoinFilePath(const FilePath& rightPath)
{
    if (&rightPath == this) {
        return *this;
    }
    m_filePath /= rightPath.m_filePath;
    m_bLexicallyNormal = false;
    return *this;
}

FilePath& FilePath::operator /= (const FilePath& rightPath)
{
    if (&rightPath == this) {
        return *this;
    }
    m_filePath /= rightPath.m_filePath;
    m_bLexicallyNormal = false;
    return *this;
}

FilePath& FilePath::operator += (const FilePath& rightPath)
{
    m_filePath += rightPath.m_filePath;
    m_bLexicallyNormal = false;
    return *this;
}

FilePath& FilePath::operator += (const DString& rightPath)
{
#ifdef DUILIB_UNICODE
    m_filePath += rightPath;
#else
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW rightPathW = StringConvert::UTF8ToWString(rightPath);
    m_filePath += rightPathW;
#else
    m_filePath += rightPath;
#endif
#endif
    m_bLexicallyNormal = false;
    return *this;
}

bool FilePath::operator != (const FilePath& otherPath) const noexcept
{
    return m_filePath != otherPath.m_filePath;
}

bool FilePath::operator == (const FilePath& otherPath) const noexcept
{
    return m_filePath == otherPath.m_filePath;
}

bool FilePath::operator < (const FilePath& otherPath) const noexcept
{
#ifdef _DEBUG
    //如果是绝对路径，应使用规范化的路径，避免不一致
    if (m_filePath.is_absolute()) {
        ASSERT(m_bLexicallyNormal);
    }
    if (otherPath.m_filePath.is_absolute()) {
        ASSERT(otherPath.m_bLexicallyNormal);
    }
#endif
    return m_filePath < otherPath.m_filePath;
}

size_t FilePath::HashValue() const noexcept
{
    if (m_filePath.empty()) {
        return 0;
    }
    else {
        return std::filesystem::hash_value(m_filePath);
    }
}

void FilePath::Clear() noexcept
{
    m_filePath.clear();
    m_bLexicallyNormal = false;
}

}
