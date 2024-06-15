#ifndef UI_CORE_NATIVE_WINDOW_WINDOWS_H_
#define UI_CORE_NATIVE_WINDOW_WINDOWS_H_

#include "duilib/Core/INativeWindow.h"

namespace ui {

#define UI_CLASS_STYLE_FRAME        (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
#define UI_WINDOW_STYLE_FRAME       (WS_VISIBLE | WS_OVERLAPPEDWINDOW)

/** 创建窗口所需的参数
*/
class WindowCreateParam
{
public:
    /** 资源所在模块句柄，如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    HMODULE m_hResModule = nullptr;

    /** 窗口类名（可选参数）
    */
    DString m_className = _T("duilib_window");

    /** 窗口类的风格（可选参数）, 参考：https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-class-styles
    */
    uint32_t m_dwClassStyle = UI_CLASS_STYLE_FRAME;

    /** 该窗口类中的窗口图标的资源ID（可选参数）
    */
    uint32_t m_nClassLogoResId = 0;

public:
    /** 窗口风格（可选参数）
    */
    uint32_t m_dwStyle = UI_WINDOW_STYLE_FRAME;

    /** 窗口扩展风格（可选参数）
    */
    uint32_t m_dwExStyle = 0;

    /** 窗口的标题（可选参数）
    */
    DString m_windowTitle;
};

class WindowBase;
class IRender;

/** 窗口功能的Windows平台实现
*/
class NativeWindow
{
public:
    NativeWindow(INativeWindow* pOwner);
    NativeWindow(const NativeWindow& r) = delete;
    NativeWindow& operator=(const NativeWindow& r) = delete;
    ~NativeWindow();

public:
    /** 创建窗口
    * @param [in] pParentWindow 父窗口
    * @param [in] createParam 创建窗口所需的参数
    * @param [in] rc 窗口位置和大小
    */
    bool CreateWnd(WindowBase* pParentWindow,
                   const WindowCreateParam& createParam,
                   const UiRect& rc = UiRect(0, 0, 0, 0));

    /** 获取窗口所属的 Windows 句柄
    */
    HWND GetHWND() const;

    /** 是否含有有效的窗口句柄
    */
    bool IsWindow() const;

    /** 获取资源的句柄
    * @return 默认返回当前进程exe的句柄
    */
    HMODULE GetResModuleHandle() const;

    /** 获取绘制区域 DC
    */
    HDC GetPaintDC() const;

public:
    /** 关闭窗口, 异步关闭，当函数返回后，IsClosing() 状态为true
    * @param [in] nRet 关闭消息, 含义如下：
                0 - 表示 "确认" 关闭本窗口
                1 - 表示点击窗口的 "关闭" 按钮关闭本窗口(默认值)
                2 - 表示 "取消" 关闭本窗口
    */
    void CloseWnd(UINT nRet = 1);

    /** 关闭窗口, 同步关闭
    */
    void Close();

    /** 是否将要关闭
    */
    bool IsClosingWnd() const;

public:
    /** 设置是否为层窗口
    * @param [in] bIsLayeredWindow true表示设置为层窗口，否则设置为非层窗口
    * @param [in] bRedraw 是否重绘窗口（属性更改后，如果不重绘，则界面可能显示异常）
    */
    bool SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw);

    /** 是否为层窗口
    */
    bool IsLayeredWindow() const;

    /** 设置窗口透明度
    * @param [in] nAlpha 透明度数值[0, 255]
    */
    void SetWindowAlpha(int nAlpha);

    /** 获取窗口透明度
    * @param [in] nAlpha 透明度数值[0, 255]
    */
    uint8_t GetWindowAlpha() const;

    /** 设置是否使用系统标题栏
    */
    void SetUseSystemCaption(bool bUseSystemCaption);

    /** 获取是否使用系统标题栏
    */
    bool IsUseSystemCaption() const;

public:
    /** 窗口的显示、隐藏、最大化、还原、最小化操作
     * @param [in] nCmdShow 显示或者隐藏窗口的命令
    */
    bool ShowWindow(ShowWindowCommands nCmdShow);

    /** 显示模态对话框(父窗口在创建的时候指定)
    */
    void ShowModalFake(WindowBase* pParentWindow);

    /** 模态对话框关闭，同步状态
    */
    void OnCloseModalFake(WindowBase* pParentWindow);

    /** 是否是模态显示
    */
    bool IsFakeModal() const;

    /** 居中窗口，支持扩展屏幕
    */
    void CenterWindow();

    /** 将窗口设置为置顶窗口
    */
    void ToTopMost();

    /** 将窗口调整为顶层窗口
    */
    void BringToTop();

    /** 设置窗口为前端窗口
    */
    bool SetWindowForeground();

    /** 当前窗口是否为前端窗口
    */
    bool IsWindowForeground() const;

    /** 设置窗口为焦点窗口
    */
    bool SetWindowFocus();

    /** 让窗口失去焦点
    */
    bool KillWindowFocus();

    /** 当前窗口是否为输入焦点窗口
    */
    bool IsWindowFocused() const;

    /** 设置Owner窗口为焦点窗口
    */
    bool SetOwnerWindowFocus();

    /** 检查并确保当前窗口为焦点窗口
    */
    void CheckSetWindowFocus();

    /** 发送消息，对 Windows SendMessage 的一层封装
    * @param [in] uMsg 消息类型
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回窗口对消息的处理结果
    */
    LRESULT SendMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);

    /** 投递一个消息到消息队列
    * @param [in] uMsg 消息类型
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回窗口对消息的处理结果
    */
    LRESULT PostMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);

    /** 向消息队列发送退出消息
    * @param [in] nExitCode 退出码
    */
    void PostQuitMsg(int32_t nExitCode);

public:
    /** 使窗口进入全屏状态
    */
    bool EnterFullScreen();

    /** 使窗口退出全屏状态 (默认按ESC键时，退出全屏)
    */
    bool ExitFullScreen();

    /** 窗口是否为最大化状态
    */
    bool IsWindowMaximized() const;

    /** 窗口是否为最小化状态
    */
    bool IsWindowMinimized() const;

    /** 窗口是否为全屏状态
    */
    bool IsWindowFullScreen() const;

    /** 将窗口的Enable状态
    * @param [in] bEnable true表示设置为Enable状态，false表示设置为disable状态
    */
    bool EnableWindow(bool bEnable);

    /** 获取窗口的Enable状态
    */
    bool IsWindowEnabled() const;

    /** 窗口是否可见
    */
    bool IsWindowVisible() const;

public:
    /** 设置窗口位置（对 ::SetWindowPos API 的一层封装，内部无DPI缩放）
    * @param [in] hWndInsertAfter 对应 SetWindowPos 的 hWndInsertAfter 选项
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] cx 窗口的宽度
    * @param [in] cy 窗口的高度
    * @param [in] uFlags 对应 ::SetWindowPos API 的 uFlags 选项
    */
    bool SetWindowPos(HWND hWndInsertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, UINT uFlags);

    /** 设置窗口位置和大小
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] nWidth 窗口的宽度
    * @param [in] nHeight 窗口的高度
    * @param [in] uFlags 对应 ::SetWindowPos API 的 uFlags 选项
    */
    bool MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint);

    /** 设置窗口图标
    *  @param [in] nRes 窗口图标资源 ID
    */
    void SetIcon(UINT nRes);

    /** 设置窗口标题栏文本
    * @param [in] strText 窗口标题栏文本
    */
    void SetText(const DString& strText);

public:
    /** 设置当要捕获的鼠标窗口句柄为当前绘制窗口
    * @param [in]
    */
    void SetCapture();

    /** 当窗口不需要鼠标输入时释放资源
    */
    void ReleaseCapture();

    /** 判断当前是否捕获鼠标输入
    */
    bool IsCaptured() const;

    /** 设置窗口的圆角RGN
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] szRoundCorner 圆角大小，其值不能为0
    * @param [in] bRedraw 是否重绘
    */
    bool SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw);

    /** 清除窗口的RGN
    * @param [in] bRedraw 是否重绘
    */
    void ClearWindowRgn(bool bRedraw);

    /** 发出重绘消息
    * @param [in] rcItem 重绘范围，为客户区坐标
    */
    void Invalidate(const UiRect& rcItem);

    /** 更新窗口，执行重绘
    */
    bool UpdateWindow() const;

    /** 开始绘制
    * @param [out] rcPaint 需要绘制的区域
    * @return 成功返回true，失败则返回false
    */
    bool BeginPaint(UiRect& rcPaint);

    /** 结束绘制
    * @param [in] rcPaint 绘制的区域
    * @param [in] pRender 绘制引擎接口，用于将绘制结果应用到窗口
    * @return 成功返回true，失败则返回false
    */
    bool EndPaint(const UiRect& rcPaint, IRender* pRender);

    /** 获取需要绘制的区域
    * @param [out] rcPaint 需要绘制的区域
    * @return 如果无更新区域返回false，否则返回true
    */
    bool GetUpdateRect(UiRect& rcPaint);

    /** 使父窗口保持激活状态
    */
    void KeepParentActive();

    /** 获取当前窗口的客户区矩形
    * @param [out] rcClient 返回窗口的客户区坐标
    */
    void GetClientRect(UiRect& rcClient) const;

    /** 获取当前窗口的窗口区矩形
    * @param [out] rcWindow 返回窗口左上角和右下角的屏幕坐标
    */
    void GetWindowRect(UiRect& rcWindow) const;

    /** 将屏幕坐标转换为当前窗口的客户区坐标
    * @param [out] pt 返回客户区坐标
    */
    void ScreenToClient(UiPoint& pt) const;

    /** 将当前窗口的客户区坐标转换为屏幕坐标
    * @param [out] pt 返回屏幕坐标
    */
    void ClientToScreen(UiPoint& pt) const;

    /** 获取当前鼠标所在坐标
    * @param [out] pt 返回屏幕坐标
    */
    void GetCursorPos(UiPoint& pt) const;

    /* 将rc的左上角坐标和右下角坐标点从相对于当前窗口的坐标空间转换为相对于桌面窗口的坐标空间
    * @param [out] rc 返回屏幕坐标
    */
    void MapWindowDesktopRect(UiRect& rc) const;

    /** 获取指定窗口所在显示器的显示器矩形和工作区矩形
    * @param [out] rcMonitor 显示器的矩形区域
    * @param [out] rcWork 显示器的工作区矩形
    */
    bool GetMonitorRect(UiRect& rcMonitor, UiRect& rcWork) const;

    /** 获取当前窗口所在显示器的工作区矩形，以虚拟屏幕坐标表示。
        请注意，如果显示器不是主显示器，则一些矩形的坐标可能是负值。
    * @param [out] rcWork 返回屏幕坐标
    */
    bool GetMonitorWorkRect(UiRect& rcWork) const;

    /** 获取指定点所在显示器的工作区矩形，以虚拟屏幕坐标表示。
        请注意，如果显示器不是主显示器，则一些矩形的坐标可能是负值。
    * @param [out] pt 输入为屏幕坐标
    * @param [out] rcWork 返回屏幕坐标
    */
    bool GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const;

    /** 获取鼠标最后的坐标
    */
    const UiPoint& GetLastMousePos() const;

    /** 设置鼠标最后的坐标
    */
    void SetLastMousePos(const UiPoint& pt);

    /** 获取一个点对应的窗口接口
    */
    INativeWindow* WindowBaseFromPoint(const UiPoint& pt);

public:
    /** 设置系统激活窗口热键，注册后按此热键，系统可以自动激活本窗口
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：kVK_DOWN等，可参考：https://learn.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
    *             如果wVirtualKeyCode为0，表示取消管理窗口激活热键
    * @param [in] wModifiers 热键组合键标志位，参见HotKeyModifiers枚举类型的值
    * @return 返回值说明:
       -1: 函数不成功;热键无效。
        0: 函数不成功;窗口无效。
        1: 函数成功，并且没有其他窗口具有相同的热键。
        2: 函数成功，但另一个窗口已具有相同的热键。
    */
    int32_t SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers);

    /** 获取系统激活窗口热键
    * @param [out] wVirtualKeyCode 虚拟键盘码，比如：kVK_DOWN等
    * @param [out] wModifiers 热键组合键标志位，参见HotKeyModifiers枚举类型的值
    * @return 如果返回false表示没有注册窗口激活热键，否则表示有注册窗口激活热键
    */
    bool GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const;

    /** 注册系统全局热键，注册成功后，按此热键后，该窗口会收到WM_HOTKEY消息
    * @param [in] wVirtualKeyCode 虚拟键盘码，比如：kVK_DOWN等
    * @param [in] wModifiers 热键组合键标志位，参见HotKeyModifiers枚举类型的值
    * @param [in] id 命令ID，应用程序必须在0x0000到0xBFFF的范围内指定 ID 值。
                  为了避免与其他共享 DLL 定义的热键标识符冲突，DLL 应使用 GlobalAddAtom 函数获取热键标识符。
    */
    bool RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id);

    /** 注销系统全局热键
    * @param [in] id 命令ID，即注册时使用的命令ID
    */
    bool UnregisterHotKey(int32_t id);

    /** 清理窗口资源
    */
    void ClearNativeWindow();

public:
    /** 调用系统默认的窗口处理函数，对 CallWindowProc API 的一层封装
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    LRESULT CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    /** 窗口过程函数
    * @param [in] hWnd 窗口句柄
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** 窗口消息的处理函数, 从系统接收到消息后，进入的第一个处理函数
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息的处理结果
    */
    LRESULT WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** 窗口消息的内部处理函数
     * @param [in] uMsg 消息体
     * @param [in] wParam 消息附加参数
     * @param [in] lParam 消息附加参数
     * @param[out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果
    */
    LRESULT ProcessInternalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    //部分消息处理函数，以实现基本功能
    LRESULT OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcCalcSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcHitTestMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcLButtonDbClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnGetMinMaxInfoMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnEraseBkGndMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnWindowPosChangingMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 窗口消息的派发函数，将Window消息转换为内部格式，然后派发出去
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果
    */
    LRESULT ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并调用OnDeleteSelf函数销毁该窗口对象）
    */
    void OnFinalMessage();

    /** 初始化窗口数据
    */
    void InitWindow();

    /** 获取当前窗口Owner窗口句柄
    */
    HWND GetWindowOwner() const;

    /** 获取指定窗口所在显示器的显示器矩形和工作区矩形
    * @param [out] rcMonitor 显示器的矩形区域
    * @param [out] rcWork 显示器的工作区矩形
    */
    bool GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const;

    /** 获取当前窗口的窗口区矩形
    * @param [out] rcWindow 返回窗口左上角和右下角的屏幕坐标
    */
    void GetWindowRect(HWND hWnd, UiRect& rcWindow) const;

    /** 获取一个消息的按键标志位
    * @param [out] modifierKey 返回标志位，参见类型定义：ModifierKey
    */
    bool GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const;

    /** 设置是否为层窗口
    */
    bool SetLayeredWindowStyle(bool bIsLayeredWindow, bool& bChanged) const;

private:
    /** 接收窗口事件的接口
    */
    INativeWindow* m_pOwner;

    //窗口句柄
    HWND m_hWnd;

    /** 资源模块句柄
    */
    HMODULE m_hResModule;

    //绘制DC
    HDC m_hDcPaint;

    //绘制结构
    PAINTSTRUCT m_paintStruct;

    //是否为层窗口
    bool m_bIsLayeredWindow;

    //窗口透明度(仅当使用层窗口时有效)
    uint8_t m_nWindowAlpha;

    //是否使用系统的标题栏
    bool m_bUseSystemCaption;

    //鼠标事件的捕获状态
    bool m_bMouseCapture;

    //窗口已经延迟关闭
    bool m_bCloseing;

    //当前窗口是否显示为模态对话框
    bool m_bFakeModal;

    //鼠标所在位置
    UiPoint m_ptLastMousePos;

private:
    /**@name 全屏相关状态
    * @{

    /** 窗口是否为全屏状态
    */
    bool m_bFullScreen;

    /** 全屏前的窗口风格
    */
    DWORD m_dwLastStyle;

    /** 全屏前的窗口位置/窗口大小等信息
    */
    WINDOWPLACEMENT m_rcLastWindowPlacement;

    /** @} */

    /** 系统全局热键的ID
    */
    std::vector<int32_t> m_hotKeyIds;
};

} // namespace ui

#endif // UI_CORE_NATIVE_WINDOW_WINDOWS_H_

