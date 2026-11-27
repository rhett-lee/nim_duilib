#ifndef UI_UTILS_FILE_TIME_H_
#define UI_UTILS_FILE_TIME_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 文件时间类
 *  用于保存文件和目录的时间戳，并支持与各平台的系统时间结构相互转换：
 *  Windows平台使用FILETIME（1601年1月1日UTC起算，单位100纳秒），
 *  其他平台使用1970年1月1日UTC起算的秒数（Epoch）。
 *  默认构造的时间值为0；ToString()以本机时区格式化为"年-月-日 时:分:秒"。
 */
class DUILIB_API FileTime
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
    * @return 返回该文件时间对应的FILETIME结构（时间值为0时各字段均为0）
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
    *  Linux系统中：以 ‌1970年1月1日 UTC‌ 为起点（Epoch），时间单位为秒
    * @return 返回文件时间的整型值，单位含义见上述说明
    */
    uint64_t GetValue() const;

    /** 将文件时间转换为字符串（本机时间: 年-月-日 时:分:秒）
    * @return 返回格式化后的时间字符串；时间值无效（超出可表示范围）时返回空字符串
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
