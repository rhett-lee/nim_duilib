#ifndef UI_CORE_EVENTARGS_H_
#define UI_CORE_EVENTARGS_H_

#pragma once

#include "duilib/Core/UiPoint.h"
#include <string>
#include <memory>

namespace ui
{
class Control;
class WeakFlag;

/** 事件通知的参数
*/
struct EventArgs
{
public:
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

public:
    /** 设置发送事件的控件
    */
    void SetSender(Control* pControl);

    /** 获取发送事件的控件
    */
    Control* GetSender() const;

    /** 判断发送事件的控件是否失效
    */
    bool IsSenderExpired() const;

private:
    /** 发送事件的控件
    */
    Control* pSender;

    /** 控件的生命周期标志
    */
    std::weak_ptr<WeakFlag> m_senderFlag;
};

/** 将字符串转换为事件类型
*/
EventType StringToEventType(const DString& messageType);

/** 将事件类型转换为字符串
*/
DString EventTypeToString(EventType eventType);

}// namespace ui

#endif // UI_CORE_EVENTARGS_H_
