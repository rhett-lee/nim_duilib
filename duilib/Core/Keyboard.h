#ifndef UI_CORE_KEYBOARD_H_
#define UI_CORE_KEYBOARD_H_

#include "duilib/duilib_defs.h"
#include "duilib/Core/KeyCode.h"

namespace ui {

/** 键盘的按键标志位
*/
enum ModifierKey {
    kNone       = 0,
    kShift      = 1 << 0,   //按住了Shift键
    kControl    = 1 << 1,   //按住了Control键
    kAlt        = 1 << 2,   //按住了Alt键
    kWin        = 1 << 3,   //按住了Win键
    kFirstPress = 1 << 4,   //第一次按键标志（即如果在发送消息之前，键处于未按下状态）
};

/** 键盘操作
*/
class UILIB_API Keyboard
{
public:
    /** 检索指定虚拟键的状态（适用于在处理窗口消息过程中调用）
    * @param [in] nVirtKey 虚拟键编码
    * @return 返回状态值
    */
    static int16_t GetKeyState(VirtualKeyCode nVirtKey);

    /** 判断一个虚拟键是否按下（适用于在处理窗口消息过程中调用）
    */
    static bool IsKeyDown(VirtualKeyCode nVirtKey);

    /** NUM LOCK 键 是否开启（适用于在处理窗口消息过程中调用）
    * @return 返回true表示开启，否则关闭
    */
    static bool IsNumLockOn();

    /** SCROLL LOCK 键 是否开启（适用于在处理窗口消息过程中调用）
    */
    static bool IsScrollLockOn();

    /** CAPS LOCK 键是否开启（适用于在处理窗口消息过程中调用）
    */
    static bool IsCapsLockOn();

    /** 确定调用函数时键的状态，以及上次调用 GetAsyncKeyState 后是否按下了该键。
    * @param [in] nVirtKey 虚拟键编码
    * @return 返回状态值
    */
    static int16_t GetAsyncKeyState(VirtualKeyCode nVirtKey);

    /** 获取键的显示名称
    * @param [in] nVirtKey 虚拟键编码
    * @param [in] fExtended 是否含有扩展标志
    */
    static DString GetKeyName(VirtualKeyCode nVirtKey, bool fExtended);

    /** 获取键的显示名称
    * @param [in] lParam 虚拟键的扫描码，键盘消息的第二个参数
    */
    static DString GetKeyName(LPARAM lParam);
};

} // namespace ui

#endif // UI_CORE_KEYBOARD_H_

