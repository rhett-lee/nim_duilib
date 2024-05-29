#include "PathUtil.h"
#include "duilib/Utils/StringUtil.h"
#include <filesystem>

namespace ui
{

std::wstring PathUtil::NormalizeDirPath(const std::wstring& strFilePath)
{
    try {
        std::wstring dirPath(strFilePath);
        StringUtil::ReplaceAll(_T("/"), _T("\\"), dirPath);
        StringUtil::ReplaceAll(_T("\\\\"), _T("\\"), dirPath);
        std::filesystem::path dir_path(dirPath);
        dir_path = dir_path.lexically_normal();
        dirPath = dir_path.native();
        if (!dirPath.empty()) {
            //确保路径最后字符是分割字符
            auto cEnd = dirPath.back();
            if (cEnd != _T('\\') && cEnd != _T('/')) {
#ifdef DUILIB_PLATFORM_WIN
                dirPath += _T('\\');
#else
                dirPath += _T('/');
#endif
            }
        }
        return dirPath;
    }
    catch (...) {
        return strFilePath;
    }
}

std::wstring PathUtil::NormalizeFilePath(const std::wstring& strFilePath)
{
    try {
        std::wstring tmp(strFilePath);
        StringUtil::ReplaceAll(_T("/"), _T("\\"), tmp);
        StringUtil::ReplaceAll(_T("\\\\"), _T("\\"), tmp);
        std::filesystem::path file_path(tmp);
        file_path = file_path.lexically_normal();
        tmp = file_path.native();
        return tmp;
    }
    catch (...) {
        return strFilePath;
    }
}

std::wstring PathUtil::JoinFilePath(const std::wstring& path1, const std::wstring& path2)
{
    try {
        std::filesystem::path file_path(path1);
        file_path /= path2;
        file_path = file_path.lexically_normal();
        std::wstring tmp = file_path.native();
        return tmp;
    }
    catch (...) {
        return std::wstring();
    }
}

bool PathUtil::IsExistsPath(const std::wstring& strFilePath)
{
    try {
        return std::filesystem::exists(strFilePath);
    }
    catch (...) {
        return false;
    }
}

bool PathUtil::IsRelativePath(const std::wstring& strFilePath)
{
    try {
        return std::filesystem::path(strFilePath).is_relative();
    }
    catch (...) {
        return false;
    }
}

bool PathUtil::IsAbsolutePath(const std::wstring& strFilePath)
{
    try {
        return std::filesystem::path(strFilePath).is_absolute();
    }
    catch (...) {
        return false;
    }
}

bool PathUtil::FilePathIsExist(const std::wstring& filePath, bool bDirectory)
{
    try {
        auto fileStatus = std::filesystem::status(std::filesystem::path(filePath));
        if ((fileStatus.type() != std::filesystem::file_type::none) &&
            (fileStatus.type() != std::filesystem::file_type::not_found)) {
            if (bDirectory) {
                return fileStatus.type() == std::filesystem::file_type::directory;
            }
            else {
                return (fileStatus.type() == std::filesystem::file_type::regular) ||
                       (fileStatus.type() == std::filesystem::file_type::symlink) ||
                       (fileStatus.type() == std::filesystem::file_type::directory);
            }
        }
    }
    catch (...) {        
    }
    return false;
}

bool PathUtil::CreateOneDirectory(const std::wstring& filePath)
{
    bool bCreated = false;
    try {
        bCreated = std::filesystem::create_directory(std::filesystem::path(filePath));
    }
    catch (...) {
    }
    return bCreated;
}

bool PathUtil::CreateDirectories(const std::wstring& filePath)
{
    bool bCreated = false;
    try {
        bCreated = std::filesystem::create_directories(std::filesystem::path(filePath));
    }
    catch (...) {
    }
    return bCreated;
}

std::wstring PathUtil::GetCurrentModuleDirectory()
{
#ifdef DUILIB_PLATFORM_WIN
    std::wstring dirPath;
    dirPath.resize(1024, 0);
    dirPath.resize(::GetModuleFileNameW(nullptr, &dirPath[0], (uint32_t)dirPath.size()));
    size_t nPos = dirPath.find_last_of(_T("/\\"));
    if (nPos != std::wstring::npos) {
        dirPath = dirPath.substr(0, nPos);
#ifdef DUILIB_PLATFORM_WIN
        dirPath += _T('\\');
#else
        dirPath += _T('/');
#endif
    }
    else {
        dirPath.clear();
    }
    return dirPath;
#else
    std::wstring path = std::filesystem::current_path().native();
    PathUtil::NormalizeDirPath(path);
    return path;
#endif
}

} // namespace ui
