#ifndef UI_CORE_EVENTARGS_H_
#define UI_CORE_EVENTARGS_H_

#pragma once

#include "duilib/Core/UiPoint.h"
#include <string>

namespace ui
{
class Control;

/** 事件通知的参数
*/
struct EventArgs
{
    EventArgs() :
        Type(kEventNone),
        pSender(nullptr),
        dwTimestamp(0),
        chKey(0),
        wParam(0),
        lParam(0)
    {
        ptMouse.x = 0;
        ptMouse.y = 0;
    }

    /** 事件类型
    */
    EventType Type;

    /** 发送事件的控件
    */
    Control* pSender;

    /** 产生事件时的时间戳
    */
    size_t dwTimestamp;

    /** 产生事件时的鼠标所在坐标
    */
    UiPoint ptMouse;

    /** 产生事件时的按键
    */
    wchar_t chKey;

    /** 产生事件时的参数
    */
    WPARAM wParam;

    /** 产生事件时的参数
    */
    LPARAM lParam;
};

EventType StringToEnum(const std::wstring& messageType);

}// namespace ui

#endif // UI_CORE_EVENTARGS_H_