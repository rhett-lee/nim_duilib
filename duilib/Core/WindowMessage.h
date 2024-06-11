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
    kSIZE_MAXSHOW   = 2,    //当其他一些窗口还原到其之前的大小时，消息将发送到所有弹出窗口
    kSIZE_MAXIMIZED = 3,    //窗口已最大化
    kSIZE_MAXHIDE   = 4     //当其他一些窗口最大化时，消息将发送到所有弹出窗口
};

/** 窗口消息定义，只定义部分使用到的消息(和Windows系统定义一致，WinUser.h)
*/
enum WindowMessage{
    
};


} // namespace ui

#endif // UI_CORE_WINDOW_MESSAGE_H_

