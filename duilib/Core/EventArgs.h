#ifndef UI_CORE_EVENTARGS_H_
#define UI_CORE_EVENTARGS_H_

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/Keycode.h"
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
    /** 事件类型
    */
    EventType eventType;

    /** 产生事件时的参数1（可用于传递指针）
    */
    WPARAM wParam;

    /** 产生事件时的参数2（可用于传递指针）
    */
    LPARAM lParam;

    /** 消息关联的按键
    */
    VirtualKeyCode vkCode;

    /** 消息关联的鼠标所在坐标
    */
    UiPoint ptMouse;

    /** 消息关联的按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    */
    uint32_t modifierKey;

    /** 消息关联的整型数据
    */
    int32_t eventData;

public:
    /** 构造函数
    */
    EventArgs();

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
