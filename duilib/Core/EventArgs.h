#ifndef UI_CORE_EVENTARGS_H_
#define UI_CORE_EVENTARGS_H_

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/Keycode.h"
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <memory>

namespace ui
{
class Control;
class WeakFlag;

/** 事件通知的参数
*/
struct UILIB_API EventArgs
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

    /** 消息关联的指针数据
    */
    void* pEventData;

    /** ListCtrl控件专用数据，用于表示该事件对应的视图类型
    */
    int32_t listCtrlType;

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

/** 事件回调函数的原型定义
*/
typedef std::function<bool (const ui::EventArgs&)> EventCallback;

/** 事件回调函数的ID
*/
typedef size_t EventCallbackID;

/** 事件回调函数的容器
*/
class UILIB_API EventSource
{
public:
    /** 添加一个回调函数
    * @param [in] callback 回调函数
    * @param [in] callbackID 该回调函数对应的ID（可以不唯一）
    */
    void AddEventCallback(const EventCallback& callback, EventCallbackID callbackID);

    /** 删除回调函数（可能是对应多个回调函数）
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数），必须是大于0的值
    * @return 如果删除了回调函数，返回true；否则返回false，未匹配到该ID的函数时返回false
    */
    bool RemoveEventCallbackByID(EventCallbackID callbackID);

    /** 判断回调函数是否存在
    * @param [in] callbackID 该回调函数对应的ID
    */
    bool HasEventCallbackByID(EventCallbackID callbackID) const;

    /** 容器是否为空
    */
    bool IsEmpty() const;

    /** 触发回调事件（对应于容器中的每个回调函数）
     * @param [in] args 回调函数的参数
     */
    bool operator() (const ui::EventArgs& args) const;

private:
    /** 事件对应的数据
    */
    struct EventCallbackData
    {
        //回调函数
        EventCallback m_callback;

        //回调函数对应的ID，用于删除回调
        EventCallbackID m_callbackID;
    };

private:
    /** 事件回调的容器
    */
    std::vector<EventCallbackData> m_callbackList;
};

/** 事件类型回调的map容器
*/
typedef std::map<EventType, EventSource> EventMap;

/** 辅助函数
*/
namespace EventUtils
{
    /** 从事件类型回调的map容器中删除指定ID的事件回调函数
    * @param [in] eventMap 需要操作的容器
    * @param [in] callbackID 回调函数的ID，必须大于0
    */
    bool RemoveEventCallbackByID(EventMap& eventMap, EventCallbackID callbackID);

    /** 从事件类型回调的map容器中删除指定ID的事件回调函数
    * @param [in] eventMap 需要操作的容器
    * @param [in] eventType 事件类型
    * @param [in] callbackID 回调函数的ID，必须大于0
    */
    bool RemoveEventCallbackByID(EventMap& eventMap, EventType eventType, EventCallbackID callbackID);

    /** 判断事件类型回调的map容器中是否包含指定ID的事件回调函数
    * @param [in] eventMap 需要操作的容器
    * @param [in] callbackID 回调函数的ID，必须大于0
    */
    bool HasEventCallbackByID(const EventMap& eventMap, EventCallbackID callbackID);

    /** 判断事件类型回调的map容器中是否包含指定ID的事件回调函数
    * @param [in] eventMap 需要操作的容器
    * @param [in] eventType 事件类型
    * @param [in] callbackID 回调函数的ID，必须大于0
    */
    bool HasEventCallbackByID(const EventMap& eventMap, EventType eventType, EventCallbackID callbackID);

    /** 将字符串转换为事件类型
    */
    EventType StringToEventType(const DString& eventName);

    /** 将事件类型转换为字符串
    */
    DString EventTypeToString(EventType eventType);
}

}// namespace ui

#endif // UI_CORE_EVENTARGS_H_
