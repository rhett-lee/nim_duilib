#ifndef UI_CONTROL_CHILD_WINDOW_EVENTS_H_
#define UI_CONTROL_CHILD_WINDOW_EVENTS_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/Keycode.h"

namespace ui
{
/** 子窗口的事件接口
 */
class ChildWindowEvents: public virtual SupportWeakCallback
{
public:
    virtual ~ChildWindowEvents() = default;

public:
    /** 窗口创建成功的事件(WM_CREATE/WM_INITDIALOG)
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual void OnWindowCreateMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)nativeMsg; (void)bHandled; }

    /** 窗口关闭消息（WM_CLOSE）
     * @param [in] wParam 消息的wParam参数
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)wParam; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口位置大小发生改变(WM_WINDOWPOSCHANGED)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowPosChangedMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口大小发生改变(WM_SIZE)
     * @param [in] sizeType 触发窗口大小改变的类型
     * @param [in] newWindowSize 新的窗口大小（宽度和高度）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnSizeMsg(ui::WindowSizeType sizeType, const ui::UiSize& newWindowSize, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)sizeType; (void)newWindowSize; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口移动(WM_MOVE)
     * @param [in] ptTopLeft 窗口客户端区域左上角的 x 坐标和 y 坐标（坐标为屏幕坐标）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMoveMsg(const ui::UiPoint& ptTopLeft, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)ptTopLeft; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口显示或者隐藏(WM_SHOWWINDOW)
     * @param [in] bShow true表示窗口正在显示，false表示窗口正在隐藏
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnShowWindowMsg(bool bShow, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)bShow; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口绘制(SDL_EVENT_WINDOW_EXPOSED/WM_PAINT)
     * @param [in] rcPaint 本次绘制，需要更新的矩形区域
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     *             SDL实现：nativeMsg.uMsg值为SDL_EVENT_WINDOW_EXPOSED，nativeMsg.wParam的值为SDL_Window*指针
     *             Windows实现：nativeMsg.uMsg值为WM_PAINT，nativeMsg.wParam的值为窗口的HWND句柄
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnPaintMsg(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)rcPaint; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口获得焦点(WM_SETFOCUS)
     * @param [in] pLostFocusWindow 已失去键盘焦点的窗口（可以为nullptr）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnSetFocusMsg(ui::WindowBase* pLostFocusWindow, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pLostFocusWindow; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口失去焦点(WM_KILLFOCUS)
     * @param [in] pSetFocusWindow 接收键盘焦点的窗口（可以为nullptr）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnKillFocusMsg(ui::WindowBase* pSetFocusWindow, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pSetFocusWindow; (void)nativeMsg; (void)bHandled; return 0; }

    /** 设置光标(WM_SETCURSOR)
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续
     */
    virtual LRESULT OnSetCursorMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)nativeMsg; (void)bHandled; return 0; }

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
     * @param [in] vkCode 虚拟键盘代码
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)vkCode; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
     * @param [in] vkCode 虚拟键盘代码
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)vkCode; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 旋转鼠标滚轮(WM_MOUSEWHEEL)
     * @param [in] wheelDelta 滚轮旋转的距离，以 WHEEL_DELTA (120) 的倍数或除法表示。 正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseWheelMsg(int32_t wheelDelta, const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)wheelDelta; (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标移动消息（WM_MOUSEMOVE）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseMoveMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标悬停消息（WM_MOUSEHOVER）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseHoverMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标离开消息（WM_MOUSELEAVE）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseLeaveMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标左键按下消息（WM_LBUTTONDOWN）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseLButtonDownMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseLButtonUpMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标左键双击消息（WM_LBUTTONDBLCLK）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseLButtonDbClickMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标右键按下消息（WM_RBUTTONDOWN）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseRButtonDownMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标右键弹起消息（WM_RBUTTONUP）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseRButtonUpMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标右键双击消息（WM_RBUTTONDBLCLK）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseRButtonDbClickMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标中键按下消息（WM_MBUTTONDOWN）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseMButtonDownMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标中键弹起消息（WM_MBUTTONUP）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseMButtonUpMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 鼠标中键双击消息（WM_MBUTTONDBLCLK）
     * @param [in] pt 鼠标所在位置，客户区坐标
     * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnMouseMButtonDbClickMsg(const ui::UiPoint& pt, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)pt; (void)modifierKey; (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnCaptureChangedMsg(const ui::NativeMsg& nativeMsg, bool& bHandled) { (void)nativeMsg; (void)bHandled; return 0; }

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局(供子类使用)
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致，该值可能与nOldScaleFactor相同
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) { (void)nOldScaleFactor; (void)nNewScaleFactor; }
};

}//namespace ui

#endif //UI_CONTROL_CHILD_WINDOW_EVENTS_H_
