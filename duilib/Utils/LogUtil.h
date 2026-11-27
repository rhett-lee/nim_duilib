#ifndef UI_UTILS_LOG_UTIL_H_
#define UI_UTILS_LOG_UTIL_H_

#include "duilib/duilib_defs.h"

namespace ui 
{

/** 日志输出工具（全部为静态函数）
 *  将日志输出到系统调试输出：Windows平台调用OutputDebugString，SDL平台调用SDL_Log；
 *  可选在日志前附加自程序启动以来的时间戳（格式为"时:分:秒.毫秒"）。
 */
class DUILIB_API LogUtil
{
public:
    /** 输出Debug日志（不追加换行符）
    * @param [in] log 日志内容
    * @param [in] bPrintTime true表示在日志前打印时间戳，false表示不打印时间戳；默认为true
    */
    static void Output(const DString& log, bool bPrintTime = true);

    /** 输出Debug日志（自动追加换行符：Windows平台为"\r\n"，其他平台为"\n"）
    * @param [in] log 日志内容
    * @param [in] bPrintTime true表示在日志前打印时间戳，false表示不打印时间戳；默认为true
    */
    static void OutputLine(const DString& log, bool bPrintTime = true);

private:
    /** 获取时间戳字符串
    * @return 返回自程序启动以来经过的时间，格式为"时:分:秒.毫秒 "（末尾带一个空格，便于与日志内容分隔）
    */
    static DString GetTimeStamp();
};

}

#endif // UI_UTILS_LOG_UTIL_H_
