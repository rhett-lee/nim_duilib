#ifndef UI_CORE_WINDOW_MESSAGE_H_
#define UI_CORE_WINDOW_MESSAGE_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui {

/** 窗口大小改变的类型
*/
enum class WindowSizeType
{
    kSIZE_RESTORED  = 0,    //窗口已调整大小，但 kSIZE_MINIMIZED 和 kSIZE_MAXIMIZED 值都不适用
    kSIZE_MINIMIZED = 1,    //窗口已最小化
    kSIZE_MAXIMIZED = 2,    //窗口已最大化
    kSIZE_MAXSHOW   = 3,    //当其他一些窗口还原到其之前的大小时，消息将发送到所有弹出窗口
    kSIZE_MAXHIDE   = 4     //当其他一些窗口最大化时，消息将发送到所有弹出窗口
};

enum ShowWindowCommands
{
    kSW_HIDE                = 0, //隐藏窗口并激活另一个窗口
    kSW_SHOW_NORMAL         = 1, //激活并显示窗口。 如果窗口最小化、最大化或排列，系统会将其还原到其原始大小和位置。应用程序应在首次显示窗口时指定此标志。
    kSW_SHOW_MINIMIZED      = 2, //激活窗口并将其显示为最小化窗口
    kSW_SHOW_MAXIMIZED      = 3, //激活窗口并显示最大化的窗口
    kSW_SHOW_NOACTIVATE     = 4, //以最近的大小和位置显示窗口。 此值类似于 kSW_SHOW_NORMAL，只是窗口未激活
    kSW_SHOW                = 5, //激活窗口并以当前大小和位置显示窗口
    kSW_MINIMIZE            = 6, //最小化指定的窗口，并按 Z 顺序激活下一个顶级窗口
    kSW_SHOW_MIN_NOACTIVE   = 7, //将窗口显示为最小化窗口。 此值类似于 kSW_SHOW_MINIMIZED，但窗口未激活
    kSW_SHOW_NA             = 8, //以当前大小和位置显示窗口。 此值类似于 SW_SHOW，只是窗口未激活
    kSW_RESTORE             = 9  //激活并显示窗口。 如果窗口最小化、最大化或排列，系统会将其还原到其原始大小和位置。 还原最小化窗口时，应用程序应指定此标志。
};

/** 窗口消息定义，只定义部分使用到的消息(和Windows系统定义一致，WinUser.h)
*/
enum WindowMessage{
    
};


} // namespace ui

#endif // UI_CORE_WINDOW_MESSAGE_H_

