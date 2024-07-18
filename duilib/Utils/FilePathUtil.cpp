#include "FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"
#include <filesystem>

namespace ui
{

FilePath FilePathUtil::JoinFilePath(const FilePath& path1, const FilePath& path2)
{
    FilePath filePath(path1);
    filePath.JoinFilePath(path2);
    return filePath;
}

FilePath FilePathUtil::NormalizeFilePath(const FilePath& filePath)
{
    DStringW nativePath;
    try {
        std::filesystem::path file_path(filePath.ToStringW());
        nativePath = file_path.lexically_normal().native();
    }
    catch (...) {
    }
    return FilePath(nativePath, true);
}

DString FilePathUtil::NormalizeFilePath(const DString& filePath)
{
    DStringW nativePath;
    try {
#ifdef DUILIB_UNICODE
        std::filesystem::path file_path(filePath);
#else
        std::filesystem::path file_path(StringUtil::UTF8ToUTF16(filePath));
#endif
        nativePath = file_path.lexically_normal().native();
    }
    catch (...) {
    }
#ifdef DUILIB_UNICODE
    return nativePath;
#else
    return StringUtil::UTF16ToUTF8(nativePath);
#endif
}

bool FilePathUtil::CreateOneDirectory(const DString& filePath)
{
    bool bCreated = false;
    try {
        if (filePath.empty()) {
            return false;
        }
        bCreated = std::filesystem::create_directory(std::filesystem::path(filePath));
    }
    catch (...) {
    }
    return bCreated;
}

bool FilePathUtil::CreateDirectories(const DString& filePath)
{
    bool bCreated = false;
    try {
        if (filePath.empty()) {
            return false;
        }
        bCreated = std::filesystem::create_directories(std::filesystem::path(filePath));
    }
    catch (...) {
    }
    return bCreated;
}

FilePath FilePathUtil::GetCurrentModuleDirectory()
{
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW dirPath;
    dirPath.resize(1024, 0);
    dirPath.resize(::GetModuleFileNameW(nullptr, &dirPath[0], (uint32_t)dirPath.size()));
    FilePath currentDir(dirPath);
    currentDir.RemoveFileName();
    return currentDir;
#else
    DStringW dirPath = std::filesystem::current_path().native();
    return FilePath(dirPath);
#endif
}

} // namespace ui
