#ifndef UI_UTILS_FILE_TIME_H_
#define UI_UTILS_FILE_TIME_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 文件时间类
*/
class UILIB_API FileTime
{
public:
    FileTime();
    ~FileTime();

public:
#ifdef DUILIB_BUILD_FOR_WIN
    /** 从Windows FILETIME转换
    */
    void FromFileTime(const FILETIME& ft);

    /** 转换为Windows FILETIME
    */
    FILETIME ToFileTime() const;
#endif

    /** 获取时间值为整型(使用Windows中的文件时间格式：表示从1601年1月1日开始的100纳秒间隔数)
    */
    uint64_t GetValue() const;

    /** 将文件时间转换为字符串（本机时间: 年-月-日 时:分:秒）
    */
    DString ToString() const;

private:
    /** 使用Windows中的文件时间格式：表示从1601年1月1日开始的100纳秒间隔数
    */
    uint64_t m_uFileTime;
};

} //namespace ui

#endif // UI_UTILS_FILE_TIME_H_
