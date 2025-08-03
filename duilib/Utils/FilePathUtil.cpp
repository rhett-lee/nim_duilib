#include "FilePathUtil.h"
#include "duilib/Utils/StringConvert.h"
#include <filesystem>

#ifdef DUILIB_BUILD_FOR_LINUX
    #include <unistd.h>
    #include <limits.h>
#endif

#ifdef DUILIB_BUILD_FOR_FREEBSD
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <unistd.h>
#endif

#ifdef DUILIB_BUILD_FOR_MACOS
    #include <mach-o/dyld.h>
    #include <CoreFoundation/CoreFoundation.h>
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
    std::error_code ec;
    std::filesystem::path exeFullPath = std::filesystem::canonical("/proc/self/exe", ec);
    DString dirPath = exeFullPath.parent_path().native();
    if (dirPath.empty()) {
        dirPath = std::filesystem::current_path().native();
    }
    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;
#elif defined (DUILIB_BUILD_FOR_FREEBSD)
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;  // -1表示当前进程

    // 首先获取路径所需的缓冲区大小
    size_t len = 0;
    if ((sysctl(mib, 4, nullptr, &len, nullptr, 0) == -1) || (len == 0)) {
        // 出错时使用当前工作目录
        std::string dirPath = std::filesystem::current_path().native();
        FilePath filePath(dirPath);
        filePath.NormalizeDirectoryPath();
        return filePath;
    }
    // 分配缓冲区并获取实际路径
    char* path = new char[len];
    if (sysctl(mib, 4, path, &len, nullptr, 0) == -1) {
        delete[] path;
        std::string dirPath = std::filesystem::current_path().native();
        FilePath filePath(dirPath);
        filePath.NormalizeDirectoryPath();
        return filePath;
    }

    // 处理获取到的路径
    std::filesystem::path exeFullPath(path);
    delete[] path;

    std::string dirPath = exeFullPath.parent_path().native();
    if (dirPath.empty()) {
        dirPath = std::filesystem::current_path().native();
    }

    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;
#elif defined (DUILIB_BUILD_FOR_MACOS)
    std::filesystem::path exeFullPath;
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::error_code ec;
        exeFullPath = std::filesystem::canonical(path, ec);
    }
    DString dirPath = exeFullPath.parent_path().native();
    if (dirPath.empty()) {
        dirPath = std::filesystem::current_path().native();
    }
    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;
#else
    DString dirPath = std::filesystem::current_path().native(); 
    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;
#endif
}

#ifdef DUILIB_BUILD_FOR_MACOS
FilePath FilePathUtil::GetBundleResourcesPath()
{
    DString dirPath;
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle) {
        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (resourcesURL) {
            char path[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)path, PATH_MAX)) {
                dirPath = std::string(path);
            }
            CFRelease(resourcesURL);
        }
    }
    FilePath filePath(dirPath);
    filePath.NormalizeDirectoryPath();
    return filePath;
}
#endif

} // namespace ui
