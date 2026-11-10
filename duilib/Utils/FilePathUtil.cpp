#include "FilePathUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/StringUtil.h"
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

DString FilePathUtil::GetFileExtension(const DString& filePath)
{
#ifdef DUILIB_BUILD_FOR_WIN
    const size_t sepPos = filePath.find_last_of(_T("/\\"));
#else
    const size_t sepPos = filePath.find_last_of(_T("/"));
#endif
    //文件名部分的起始位置(路径以分隔符结尾时，文件名部分为空)
    const size_t nameBegin = (sepPos == DString::npos) ? 0 : (sepPos + 1);
    if (nameBegin >= filePath.size()) {
        return DString();
    }
    //性能优化：只取文件名部分的下标进行查找，避免原来对整个路径字符串的拷贝
    //同时避免路径以分隔符结尾时，把目录名中的"."误判为扩展名
    const size_t dotPos = filePath.rfind(_T('.'));
    if ((dotPos == DString::npos) || (dotPos < nameBegin) || ((dotPos + 1) >= filePath.size())) {
        return DString();
    }
    return StringUtil::MakeUpperString(filePath.substr(dotPos + 1, DString::npos));
}

FilePath FilePathUtil::GetCurrentModuleDirectory()
{
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW dirPath;
    dirPath.resize(1024, 0);
    for (;;) {
        const uint32_t nSize = (uint32_t)dirPath.size();
        const uint32_t nRet = ::GetModuleFileNameW(nullptr, &dirPath[0], nSize);
        if (nRet == 0) {
            dirPath.clear();
            break;
        }
        if (nRet < nSize) {
            //获取成功，截断到实际长度
            dirPath.resize(nRet);
            break;
        }
        //返回值等于缓冲区大小表示缓冲区不足(路径被截断)，扩大缓冲区后重试，避免长路径被静默截断
        dirPath.resize((size_t)nSize * 2, 0);
    }
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
