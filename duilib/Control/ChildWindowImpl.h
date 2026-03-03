#ifndef UI_CORE_CHILD_WINDOW_IMPL_H_
#define UI_CORE_CHILD_WINDOW_IMPL_H_

#include "duilib/Core/WindowBase.h"
#include "duilib/Control/ChildWindowEvents.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui
{
/** 子窗口控件，控件自身是一个操作系统的子窗口，界面库内部负责子窗口的创建和销毁，但界面库内部不执行子窗口的绘制
 *  由应用层自己负责子窗口的绘制
 */
class ChildWindowImpl: public WindowBase
{
public:
    ChildWindowImpl(ChildWindowEvents* pChildWindowEvents);
    virtual ~ChildWindowImpl() override;
    ChildWindowImpl(const ChildWindowImpl&) = delete;
    ChildWindowImpl& operator=(const ChildWindowImpl&) = delete;

public:
    /** 设置子窗口可见属性
    * @param [in] bVisible 是否可见
    */
    void SetChildWindowVisible(bool bVisible);

    /** 设置子窗口可用属性
    * @param [in] bEnabled 可用状态事件
    */
    void SetChildWindowEnabled(bool bEnabled);

    /** 设置子窗口的消息回调接口
    * @param [in] pChildWindowEvents 子窗口的消息回调接口
    */
    void SetChildWindowEvents(ChildWindowEvents* pChildWindowEvents);

protected:
    /** 正在初始化窗口数据
    */
    virtual void PreInitWindow() override;

    /** 初始化窗口数据（当窗口创建完成以后调用此函数），供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 完成初始化窗口数据
    */
    virtual void PostInitWindow() override;

    /** 完成界面布局的初始化，各个控件的位置大小等布局信息完成初始化
    */
    virtual void OnInitLayout() override;

    /** 窗口正在关闭，处理内部状态
    */
    virtual void PreCloseWindow() override;

    /** 窗口已经关闭，处理内部状态
    */
    virtual void PostCloseWindow() override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息
    */
    virtual void FinalMessage() override;

protected:
    /** 进入全屏状态
    */
    virtual void NotifyWindowEnterFullscreen() override;

    /** 退出全屏状态
    */
    virtual void NotifyWindowExitFullscreen() override;

protected:
    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 准备绘制
    * @return 返回true表示继续绘制，返回false表示不再继续绘制
    */
    virtual bool OnPreparePaint() override;

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() override;

    /** 窗口的透明度发生变化
    */
    virtual void OnWindowAlphaChanged() override;

    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullscreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullscreen() override;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局(供子类使用)
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致，该值可能与nOldScaleFactor相同
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

    /** 获取设置的窗口阴影的大小
    * @param [out] rcShadow 返回设置窗口阴影的大小，未经过DPI缩放
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const override;

    /** 获取当前窗口阴影的大小
    * @param [out] rcShadow 返回当前窗口阴影的大小，已经过DPI缩放
    */
    virtual void GetCurrentShadowCorner(UiPadding& rcShadow) const override;

    /** 判断一个点是否在放置在标题栏上的控件上
    */
    virtual bool IsPtInCaptionBarControl(const UiPoint& pt) const override;

    /** 判断是否含有最大化和最小化按钮
    * @param [out] bMinimizeBox 返回true表示含有最小化按钮
    * @param [out] bMaximizeBox 返回true表示含有最大化按钮
    */
    virtual bool HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const override;

    /** 判断一个点是否在最大化或者还原按钮上
    */
    virtual bool IsPtInMaximizeRestoreButton(const UiPoint& pt) const override;

    /** 获取创建窗口的属性（从XML文件的Window标签中读取的属性值）
    * @param [out] createAttributes 返回从XML文件的Window标签中读取的创建窗口的属性
    */
    virtual void GetCreateWindowAttributes(WindowCreateAttributes& createAttributes) override;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致
    */
    virtual void OnDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

    /** 获取绘制引擎对象
    */
    virtual IRender* GetRender() const override;

    /** 获取指定坐标点的控件接口
    * @param [in] pt 客户区坐标点
    */
    virtual Control* OnFindControl(const UiPoint& pt) const override;

protected:
    /** @name 窗口消息处理相关
     * @{
     */
     /** 窗口消息的派发函数
     * @param [in] uMsg 消息体
     * @param [in] wParam 消息附加参数
     * @param [in] lParam 消息附加参数
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果
     */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** 窗口创建成功的事件(WM_CREATE/WM_INITDIALOG)
    * @param [in] bDoModal 当前是否为通过DoModal函数显示的模态对话框
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual void OnWindowCreateMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口显示或者隐藏(WM_SHOWWINDOW)
    * @param [in] bShow true表示窗口正在显示，false表示窗口正在隐藏
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口绘制(SDL_EVENT_WINDOW_EXPOSED/WM_PAINT)
    * @param [in] rcPaint 本次绘制，需要更新的矩形区域
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    *             SDL实现：nativeMsg.uMsg值为SDL_EVENT_WINDOW_EXPOSED，nativeMsg.wParam的值为SDL_Window*指针
    *             Windows实现：nativeMsg.uMsg值为WM_PAINT，nativeMsg.wParam的值为窗口的HWND句柄
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnPaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口位置大小发生改变(WM_WINDOWPOSCHANGED)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowPosChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口移动(WM_MOVE)
    * @param [in] ptTopLeft 窗口客户端区域左上角的 x 坐标和 y 坐标（坐标为屏幕坐标）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口获得焦点(WM_SETFOCUS)
    * @param [in] pLostFocusWindow 已失去键盘焦点的窗口（可以为nullptr）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSetFocusMsg(WindowBase* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口失去焦点(WM_KILLFOCUS)
    * @param [in] pSetFocusWindow 接收键盘焦点的窗口（可以为nullptr）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 通知应用程序输入焦点变化(WM_IME_SETCONTEXT)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeSetContextMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 输入法开始生成组合字符串(WM_IME_STARTCOMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 更改按键组合状态(WM_IME_COMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 输入法结束组合(WM_IME_ENDCOMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeEndCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 设置光标(WM_SETCURSOR)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续
    */
    virtual LRESULT OnSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 通知窗口用户希望显示上下文菜单(WM_CONTEXTMENU)，用户可能单击了鼠标右键 (在窗口中右键单击) ，按下了 Shift+F10 或按下了应用程序键， (上下文菜单键) 某些键盘上可用。
    * @param [in] pt 鼠标所在位置，客户区坐标, 如果是(-1,-1)表示用户键入了 SHIFT+F10
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_CHAR)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 快捷键消息（WM_HOTKEY）
    * @param [in] hotkeyId 热键的ID
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt, ModifierKey::kControl, ModifierKey::kShift, ModifierKey::kWin
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 旋转鼠标滚轮(WM_MOUSEWHEEL)
    * @param [in] wheelDelta 滚轮旋转的距离，以 WHEEL_DELTA (120) 的倍数或除法表示。 正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] bFromNC true表示这是NC消息（WM_NCMOUSEMOVE）, false 表示是WM_MOUSEMOVE消息
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标悬停消息（WM_MOUSEHOVER）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标离开消息（WM_MOUSELEAVE）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键按下消息（WM_LBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键双击消息（WM_LBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键按下消息（WM_RBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键弹起消息（WM_RBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键双击消息（WM_RBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标中键按下消息（WM_MBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标中键弹起消息（WM_MBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标中键双击消息（WM_MBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口位置的贴边操作
    * @param [in] bLeftSnap 窗口左侧贴边
    * @param [in] bRightSnap 窗口右侧贴边
    * @param [in] bTopSnap 窗口上侧贴边
    * @param [in] bBottomSnap 窗口下侧贴边
    */
    virtual void OnWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap) override;

    /** 窗口拖放相关的操作接口(接口参数是与实现方式相关的)
    * @param [in] dropType 拖放操作的来源类型
    * @param [in,out] pDropData 具体类型根据dropType判断：
    *                 当dropType为kControlDropTypeWindows时（代表Windows平台SDK实现），pDropData的类型是ControlDropData_Windows*
    *                 当dropType为kControlDropTypeSDL时（代表SDL实现），pDropData的类型是ControlDropData_SDL*
    *                 pDropData->m_bHandled是消息处理标志，如果返回true表示该事件已经处理，不再转发给界面中的其他UI控件处理，相当于截获此消息
    *                 pDropData->m_hResult是消息处理后的返回值，最终返回给操作系统，Windows平台成功是返回S_OK
    */
    virtual void OnDropEnterMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropOverMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropMsg(ui::ControlDropType dropType, void* pDropData) override;
    virtual void OnDropLeaveMsg() override;

    /** @}*/

private:
    //子窗口的外部消息回调接口
    ControlPtrT<ChildWindowEvents> m_pChildWindowEvents;
};

}//namespace ui

#endif //UI_CORE_CHILD_WINDOW_IMPL_H_
