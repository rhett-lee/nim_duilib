#include "FilePathUtil.h"
#include "duilib/Utils/StringConvert.h"
#include <filesystem>

#ifdef DUILIB_BUILD_FOR_LINUX
    #include <unistd.h>
    #include <limits.h>
#endif

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
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW nativePath;
#else
    DStringA nativePath;
#endif
    try {
#ifdef DUILIB_BUILD_FOR_WIN
        std::filesystem::path file_path(filePath.ToStringW());
#else
        std::filesystem::path file_path(filePath.ToStringA());
#endif
        nativePath = file_path.lexically_normal().native();
    }
    catch (...) {
    }
    return FilePath(nativePath, true);
}

DString FilePathUtil::NormalizeFilePath(const DString& filePath)
{
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
    DStringW nativePath;
    try {
#ifdef DUILIB_UNICODE
        std::filesystem::path file_path(filePath);
#else
#ifdef DUILIB_BUILD_FOR_WIN
        std::filesystem::path file_path(StringConvert::UTF8ToWString(filePath));
#else
        std::filesystem::path file_path(filePath);
#endif
#endif
        nativePath = file_path.lexically_normal().native();
    }
    catch (...) {
    }
#ifdef DUILIB_UNICODE
    return nativePath;
#else
    return StringConvert::WStringToUTF8(nativePath);
#endif

#else
    //Linux平台
    DString nativePath;
    try {
        std::filesystem::path file_path(filePath);
        nativePath = file_path.lexically_normal().native();
    }
    catch (...) {
    }
    return nativePath;
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
#elif defined (DUILIB_BUILD_FOR_LINUX)
    DString dirPath;
    char exe_path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0'; // 确保字符串终止
        dirPath = exe_path;
        size_t pos = dirPath.rfind("/");
        if (pos != DString::npos) {
            dirPath = dirPath.substr(0, pos);
        }
    }
#else 
    DString dirPath;
#endif
    if (dirPath.empty()) {
        dirPath = std::filesystem::current_path().native();
    }    
    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;

}

} // namespace ui
