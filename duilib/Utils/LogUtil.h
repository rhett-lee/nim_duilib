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
    */
    static void Output(const DString& log);

    /** 输出Debug日志(追加换行符)
    */
    static void OutputLine(const DString& log);

private:
    /** 获取时间戳字符串
    */
    static DString GetTimeStamp();
};

}

#endif // UI_UTILS_LOG_UTIL_H_
