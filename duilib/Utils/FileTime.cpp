#include "FileTime.h"
#include "duilib/Utils/StringUtil.h"

#include <iomanip>
#include <sstream>
#include <ctime>

namespace ui
{
FileTime::FileTime():
    m_uFileTime(0)
{
}

FileTime::~FileTime()
{
}

uint64_t FileTime::GetValue() const
{
    return m_uFileTime;
}

#ifdef DUILIB_BUILD_FOR_WIN
void FileTime::FromFileTime(const FILETIME& ft)
{
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    m_uFileTime = li.QuadPart;
}

FILETIME FileTime::ToFileTime() const
{
    ULARGE_INTEGER li;
    li.QuadPart = m_uFileTime;

    FILETIME ft;
    ft.dwHighDateTime = li.HighPart;
    ft.dwLowDateTime = li.LowPart;
    return ft;
}

DString FileTime::ToString() const
{
    FILETIME ft = ToFileTime();

    //转换为UTC时间
    SYSTEMTIME stUTC = { 0 };
    if (::FileTimeToSystemTime(&ft, &stUTC) == FALSE) {
        //无效的文件时间(如默认值为0的异常值)，返回空字符串，避免使用未初始化的SYSTEMTIME
        return DString();
    }

    //转换为本地时区SYSTEMTIME
    SYSTEMTIME stLocal = { 0 };
    if (::SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal) == FALSE) {
        stLocal = stUTC;
    }

    // 格式化为字符串
    return StringUtil::Printf(_T("%04d-%02d-%02d %02d:%02d:%02d"),
                              stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                              stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
}

#else

void FileTime::FromSecondsSinceEpoch(uint64_t secondsSinceEpoch)
{
    m_uFileTime = secondsSinceEpoch;
}

uint64_t FileTime::ToSecondsSinceEpoch() const
{
    return m_uFileTime;
}

DString FileTime::ToString() const
{
    uint64_t secondsSinceEpoch = ToSecondsSinceEpoch();
    
    struct tm tm = { 0 };
    std::time_t time = (std::time_t)secondsSinceEpoch;
    if (localtime_r(&time, &tm) == nullptr) {
        //时间值非法(超出time_t可表示范围)时，返回空字符串
        return DString();
    }

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

#endif

} //namespace ui
