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
    * @param [in] ft Windows平台的文件时间，从‌1601年1月1日UTC‌开始计算，单位为100纳秒
    */
    void FromFileTime(const FILETIME& ft);

    /** 转换为Windows FILETIME
    */
    FILETIME ToFileTime() const;
#else
    /** 从Linux系统时间转换
    * @param [in] secondsSinceEpoch Linux系统的文件时间，以 ‌1970年1月1日 UTC‌ 为起点（Epoch），时间单位为秒
    */
    void FromSecondsSinceEpoch(uint64_t secondsSinceEpoch);

    /** 转换成Linux系统时间
    * @return Linux系统的文件时间，以 ‌1970年1月1日 UTC‌ 为起点（Epoch），时间单位为秒
    */
    uint64_t ToSecondsSinceEpoch() const;

#endif

    /** 获取时间值为整型, 文件时间的具体时间值，不同系统有不同含义:
     *  Windows系统中：从‌1601年1月1日UTC‌开始计算，单位为100纳秒
     *  Linux系统中：以 ‌1970年1月1日 UTC‌ 为起点（Epoch），时间单位为纳秒
     */
    uint64_t GetValue() const;

    /** 将文件时间转换为字符串（本机时间: 年-月-日 时:分:秒）
    */
    DString ToString() const;

private:
    /** 文件时间的具体时间值，不同系统有不同含义:
     *    Windows系统中：从‌1601年1月1日UTC‌开始计算，单位为100纳秒
     *    Linux系统中：以 ‌1970年1月1日 UTC‌ 为起点（Epoch），时间单位为秒
     */
    uint64_t m_uFileTime;
};

} //namespace ui

#endif // UI_UTILS_FILE_TIME_H_
