#ifndef UI_UTILS_LOG_UTIL_H_
#define UI_UTILS_LOG_UTIL_H_

#include "duilib/duilib_defs.h"

namespace ui 
{

/** 日志输出工具
*/
class UILIB_API LogUtil
{
public:
    /** 输出Debug日志
    * @param [in] log 日志内容
    * @param [in] bPrintTime true表示打印时间戳，false表示不打印时间戳
    */
    static void Output(const DString& log, bool bPrintTime = true);

    /** 输出Debug日志(追加换行符)
    * @param [in] log 日志内容
    * @param [in] bPrintTime true表示打印时间戳，false表示不打印时间戳
    */
    static void OutputLine(const DString& log, bool bPrintTime = true);

private:
    /** 获取时间戳字符串
    */
    static DString GetTimeStamp();
};

}

#endif // UI_UTILS_LOG_UTIL_H_
