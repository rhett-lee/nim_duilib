#include "AddressBarPathUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include <algorithm>
#include <cstdlib>
#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/Utils/DiskUtils_Windows.h"
#endif

namespace ui
{

namespace
{
/** 是否为目录分隔符（Windows下同时接受'/'与'\\'）
*/
bool IsDirSeparator(DString::value_type ch)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return (ch == _T('\\')) || (ch == _T('/'));
#else
    return ch == _T('/');
#endif
}

/** 判断字符串是否以指定前缀开头（不区分大小写）
*/
bool StartsWithNoCase(const DString& text, const DString& prefix)
{
    if (text.size() < prefix.size()) {
        return false;
    }
    const DString head(text.c_str(), prefix.size());
    return StringUtil::MakeUpperString(head) == StringUtil::MakeUpperString(prefix);
}

/** 按目录分隔符拆分路径组件（跳过空组件）
*/
std::vector<DString> SplitPathComponents(const DString& path, size_t nStartIndex)
{
    std::vector<DString> parts;
    size_t i = nStartIndex;
    while (i < path.size()) {
        while ((i < path.size()) && IsDirSeparator(path[i])) {
            ++i;
        }
        if (i >= path.size()) {
            break;
        }
        size_t j = i;
        while ((j < path.size()) && !IsDirSeparator(path[j])) {
            ++j;
        }
        parts.push_back(path.substr(i, j - i));
        i = j;
    }
    return parts;
}
}//namespace

DString ExpandAddressBarUserPath(const DString& path)
{
    if (path.empty() || (path[0] != _T('~'))) {
        return path;
    }
    //仅处理"~"或者"~/..."、"~\\..."形式，避免把"~abc"这类普通文件名当作主目录
    if ((path.size() > 1) && !IsDirSeparator(path[1])) {
        return path;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    DStringW home;
    const DWORD nLength = ::GetEnvironmentVariableW(L"USERPROFILE", nullptr, 0);
    if (nLength > 0) {
        home.resize(nLength + 1);
        const DWORD nCopied = ::GetEnvironmentVariableW(L"USERPROFILE", &home[0], nLength + 1);
        if (nCopied > 0) {
            home.resize(nCopied);
            return StringConvert::WStringToT(home) + path.substr(1);
        }
    }
    return path;
#else
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
        return DString(home) + path.substr(1);
    }
    return path;
#endif
}

DString NormalizeAddressBarHistoryPath(const DString& path)
{
    DString result = path;
    StringUtil::Trim(result);
    if (result.empty()) {
        return result;
    }
    result = ExpandAddressBarUserPath(result);
#ifdef DUILIB_BUILD_FOR_WIN
    //去掉末尾多余的路径分隔符，保留盘符根（如"C:\"）
    while ((result.size() >= 2) && IsDirSeparator(result.back())) {
        if ((result.size() == 3) && (result[1] == _T(':'))) {
            break;
        }
        result.pop_back();
    }
    //仅输入盘符时统一补上根分隔符（如"C:"归一化为"C:\"）
    if ((result.size() == 2) && (result[1] == _T(':'))) {
        result += _T('\\');
    }
#else
    //Unix：保留根"/"，其余去掉末尾多余的'/'
    while ((result.size() > 1) && (result.back() == _T('/'))) {
        result.pop_back();
    }
#endif
    return result;
}

void BuildAddressBarPathSegments(const DString& addressPath,
                                 const DString& emptyDisplayText,
                                 std::vector<AddressBarPathSegment>& segments)
{
    segments.clear();
    if (addressPath.empty()) {
        if (!emptyDisplayText.empty()) {
            AddressBarPathSegment segment;
            segment.m_displayName = emptyDisplayText;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = true;
            segments.push_back(segment);
        }
        return;
    }

    //展开以"~"开头的用户主目录路径
    DString pathText = ExpandAddressBarUserPath(addressPath);
    FilePath filePath(pathText);
    filePath.NormalizeFilePath();
    filePath.TrimRightPathSeparator();
    pathText = filePath.ToString();

#ifdef DUILIB_BUILD_FOR_WIN
    //盘符路径（C:、C:\\...）与UNC路径（\\\\server\\...）均视为绝对路径
    const bool bAbsolutePath = ((pathText.size() >= 2) && (pathText[1] == _T(':'))) ||
                               ((pathText.size() >= 2) && IsDirSeparator(pathText[0]) && IsDirSeparator(pathText[1]));
#else
    const bool bAbsolutePath = !pathText.empty() && (pathText[0] == _T('/'));
#endif
    if (!bAbsolutePath) {
        //相对路径：仅按分隔符拆分为普通路径段，不生成根段
        DString fullPath;
        for (const DString& part : SplitPathComponents(pathText, 0)) {
            if (fullPath.empty()) {
                fullPath = part;
            }
            else {
                fullPath += FilePath::GetPathSeparator();
                fullPath += part;
            }
            AddressBarPathSegment segment;
            segment.m_fullPath = fullPath;
            segment.m_displayName = part;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = false;
            segments.push_back(segment);
        }
        if (segments.empty()) {
            AddressBarPathSegment segment;
            segment.m_fullPath = pathText;
            segment.m_displayName = pathText;
            segment.m_bRoot = false;
            segment.m_bDrive = false;
            segment.m_bComputer = false;
            segments.push_back(segment);
        }
        return;
    }

#ifdef DUILIB_BUILD_FOR_WIN
    //解析根路径：支持盘符、UNC(\\\\server\\share)、扩展长度路径(\\\\?\\、\\\\?\\UNC\\)
    DString rootPath;
    DString rootDisplay;
    size_t nRootEnd = 0;
    bool bDriveRoot = false;

    if (StartsWithNoCase(pathText, _T("\\\\?\\UNC\\"))) {
        //\\\\?\\UNC\\server\\share\\...
        const size_t nPrefixLen = 8;
        size_t nServerEnd = pathText.find_first_of(_T("\\/"), nPrefixLen);
        if (nServerEnd == DString::npos) {
            nRootEnd = pathText.size();
            rootPath = pathText;
            rootDisplay = pathText.substr(nPrefixLen);
        }
        else {
            size_t nShareEnd = pathText.find_first_of(_T("\\/"), nServerEnd + 1);
            if (nShareEnd == DString::npos) {
                nShareEnd = pathText.size();
            }
            nRootEnd = nShareEnd;
            rootPath = pathText.substr(0, nShareEnd);
            //显示为普通的UNC根：\\\\server\\share
            rootDisplay = DString(_T("\\\\")) + pathText.substr(nPrefixLen, nShareEnd - nPrefixLen);
        }
    }
    else if (StartsWithNoCase(pathText, _T("\\\\?\\"))) {
        //\\\\?\\C:\\... 或 \\\\?\\Volume{GUID}\\...
        const size_t nPrefixLen = 4;
        size_t nNameEnd = pathText.find_first_of(_T("\\/"), nPrefixLen);
        if (nNameEnd == DString::npos) {
            nNameEnd = pathText.size();
        }
        nRootEnd = nNameEnd;
        rootPath = pathText.substr(0, nNameEnd);
        rootDisplay = pathText.substr(nPrefixLen, nNameEnd - nPrefixLen);
        if ((rootDisplay.size() == 2) && (rootDisplay[1] == _T(':'))) {
            bDriveRoot = true;
        }
    }
    else if (IsDirSeparator(pathText[0])) {
        //UNC: \\\\server\\share\\...
        size_t nServerEnd = pathText.find_first_of(_T("\\/"), 2);
        if (nServerEnd == DString::npos) {
            nRootEnd = pathText.size();
            rootPath = pathText;
            rootDisplay = pathText;
        }
        else {
            size_t nShareEnd = pathText.find_first_of(_T("\\/"), nServerEnd + 1);
            if (nShareEnd == DString::npos) {
                nShareEnd = pathText.size();
            }
            nRootEnd = nShareEnd;
            rootPath = pathText.substr(0, nShareEnd);
            rootDisplay = rootPath;
        }
    }
    else {
        //盘符根：C:\\...
        nRootEnd = 2;
        rootPath = pathText.substr(0, 2);
        rootDisplay = rootPath;
        bDriveRoot = true;
    }

    //根段
    AddressBarPathSegment rootSegment;
    rootSegment.m_fullPath = rootPath;
    if (!rootPath.empty() && !IsDirSeparator(rootPath.back())) {
        rootSegment.m_fullPath += FilePath::GetPathSeparator();
    }
    if (bDriveRoot) {
        //查询卷名（\\\\?\\ 前缀的盘符使用普通路径查询）
        DString diskPath = rootPath;
        if (StartsWithNoCase(diskPath, _T("\\\\?\\"))) {
            diskPath = diskPath.substr(4);
        }
        if ((diskPath.size() == 2) && (diskPath[1] == _T(':'))) {
            diskPath += FilePath::GetPathSeparator();
        }
        DiskUtils::DiskInfo diskInfo;
        DiskUtils::GetLogicalDriveInfo(diskPath, diskInfo);
        if (!diskInfo.m_volumeName.empty()) {
            //卷名通常已包含盘符（如“本地磁盘 (C:)”），优先显示卷名
            rootDisplay = diskInfo.m_volumeName;
        }
    }
    rootSegment.m_displayName = rootDisplay;
    rootSegment.m_bRoot = true;
    rootSegment.m_bDrive = bDriveRoot;
    rootSegment.m_bComputer = false;
    segments.push_back(rootSegment);

    //根路径下的各级子路径
    DString currentPath = rootSegment.m_fullPath;
    for (const DString& part : SplitPathComponents(pathText, nRootEnd)) {
        currentPath += part;
        AddressBarPathSegment segment;
        segment.m_fullPath = currentPath;
        segment.m_displayName = part;
        segment.m_bRoot = false;
        segment.m_bDrive = false;
        segment.m_bComputer = false;
        segments.push_back(segment);
        currentPath += FilePath::GetPathSeparator();
    }
#else
    //Unix：根为"/"
    AddressBarPathSegment rootSegment;
    rootSegment.m_fullPath = _T("/");
    rootSegment.m_displayName = _T("/");
    rootSegment.m_bRoot = true;
    rootSegment.m_bDrive = false;
    rootSegment.m_bComputer = false;
    segments.push_back(rootSegment);

    DString currentPath = _T("/");
    for (const DString& part : SplitPathComponents(pathText, 0)) {
        currentPath += part;
        AddressBarPathSegment segment;
        segment.m_fullPath = currentPath;
        segment.m_displayName = part;
        segment.m_bRoot = false;
        segment.m_bDrive = false;
        segment.m_bComputer = false;
        segments.push_back(segment);
        currentPath += FilePath::GetPathSeparator();
    }
#endif
}

}//namespace ui
