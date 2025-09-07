#ifndef UI_CORE_NATIVE_WINDOW_WINDOWS_H_
#define UI_CORE_NATIVE_WINDOW_WINDOWS_H_

#include "duilib/Core/INativeWindow.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Core/WindowCreateAttributes.h"
#include "duilib/Utils/FilePath.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/duilib_config_windows.h"

namespace ui {

class WindowDropTarget;
class Control;

/** 窗口功能的Windows平台实现
*/
class NativeWindow_Windows
{
public:
    explicit NativeWindow_Windows(INativeWindow* pOwner);
    NativeWindow_Windows(const NativeWindow_Windows& r) = delete;
    NativeWindow_Windows& operator=(const NativeWindow_Windows& r) = delete;
    ~NativeWindow_Windows();

public:
    /** 创建窗口
    * @param [in] pParentWindow 父窗口
    * @param [in] createParam 创建窗口所需的参数
    * @param [in] createAttributes XML文件中Window的相关属性
    */
    bool CreateWnd(NativeWindow_Windows* pParentWindow,
                  const WindowCreateParam& createParam,
                  const WindowCreateAttributes& createAttributes);

    /** 显示模态窗口
    * @param [in] pParentWindow 父窗口
    * @param [in] createParam 创建窗口所需的参数
    * @param [in] bCloseByEsc 按ESC键的时候，是否关闭窗口
    * @param [in] bCloseByEnter 按Enter键的时候，是否关闭窗口
    * @return 窗口退出时的返回值, 如果失败则返回-1
    */
    int32_t DoModal(NativeWindow_Windows* pParentWindow,
                    const WindowCreateParam& createParam,
                    const WindowCreateAttributes& createAttributes,
                    bool bCloseByEsc = true,
                    bool bCloseByEnter = false);

    /** 获取窗口所属的 Windows 句柄
    */
    HWND GetHWND() const;

    /** 获取本地实现的窗口句柄
    */
    void* GetWindowHandle() const;

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
    * @param [in] nRet 关闭的参数，参见：enum WindowCloseParam
    */
    void CloseWnd(int32_t nRet = kWindowCloseNormal);

    /** 关闭窗口, 同步关闭
    */
    void Close();

    /** 是否将要关闭
    */
    bool IsClosingWnd() const;

    /** 获取窗口关闭的参数
    * @return 参见enum WindowCloseParam, 也可能是自定义值
    */
    int32_t GetCloseParam() const;

public:
    /** 设置是否为层窗口
    * @param [in] bIsLayeredWindow true表示设置为层窗口，否则设置为非层窗口
    * @param [in] bRedraw 是否重绘窗口（属性更改后，如果不重绘，则界面可能显示异常）
    */
    bool SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw);

    /** 是否为层窗口
    */
    bool IsLayeredWindow() const;

    /** 设置窗口透明度(仅当IsLayeredWindow()为true的时候有效)
    * @param [in] nAlpha 透明度数值[0, 255]，当 nAlpha 为 0 时，窗口是完全透明的。 当 nAlpha 为 255 时，窗口是不透明的。
    *             该参数在UpdateLayeredWindow函数中作为参数使用。
    */
    void SetLayeredWindowAlpha(int32_t nAlpha);

    /** 获取窗口透明度(仅当IsLayeredWindow()为true的时候有效)
    * @param [in] nAlpha 透明度数值[0, 255]，当 nAlpha 为 0 时，窗口是完全透明的。 当 nAlpha 为 255 时，窗口是不透明的。
    */
    uint8_t GetLayeredWindowAlpha() const;

    /** 设置窗口不透明度(仅当IsLayeredWindow()为true的时候有效，所以如果当前不是分层窗口，内部会自动设置为分层窗口)
    * @param [in] nAlpha 透明度数值[0, 255]，当 nAlpha 为 0 时，窗口是完全透明的。 当 nAlpha 为 255 时，窗口是不透明的。
    *             该参数在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)。
    */
    void SetLayeredWindowOpacity(int32_t nAlpha);

    /** 获取窗口不透明度(仅当IsLayeredWindow()为true的时候有效)
    * @param [in] nAlpha 透明度数值[0, 255]，当 nAlpha 为 0 时，窗口是完全透明的。 当 nAlpha 为 255 时，窗口是不透明的。
    */
    uint8_t GetLayeredWindowOpacity() const;

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
    void ShowModalFake(NativeWindow_Windows* pParentWindow);

    /** 模态对话框关闭，同步状态
    */
    void OnCloseModalFake(NativeWindow_Windows* pParentWindow);

    /** 是否是模拟的模态显示窗口（通过ShowModalFake函数显示的窗口）
    */
    bool IsFakeModal() const;

    /** 是否是模态对话框模式（通过DoModal函数显示的对话框窗口）
    */
    bool IsDoModal() const;

    /** 居中窗口，支持扩展屏幕
    */
    void CenterWindow();

    /** 是否将窗口设置为置顶窗口
    * @param [in] bOnTop true表示设置为置顶窗口，false表示取消置顶窗口
    */
    void SetWindowAlwaysOnTop(bool bOnTop);

    /** 判断当前窗口是否为置顶窗口
    * @return true表示当前为置顶窗口，false表示当前不是置顶窗口
    */
    bool IsWindowAlwaysOnTop() const;

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

    /** 检查并确保当前窗口为焦点窗口
    */
    void CheckSetWindowFocus();

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
    static void PostQuitMsg(int32_t nExitCode);

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
    * @param [in] pInsertAfterWindow 对应 SetWindowPos 的 hWndInsertAfter 选项
    * @param [in] insertAfterFlag 对应 SetWindowPos 的 hWndInsertAfter 选项, 仅当pWindow为nullptr时有效
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] cx 窗口的宽度
    * @param [in] cy 窗口的高度
    * @param [in] uFlags 参考 enum WindowPosFlags 选项
    */
    bool SetWindowPos(const NativeWindow_Windows* pInsertAfterWindow,
                      InsertAfterFlag insertAfterFlag,
                      int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags);

    /** 设置窗口位置和大小
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] nWidth 窗口的宽度
    * @param [in] nHeight 窗口的高度
    * @param [in] bRepaint 是否重绘窗口
    */
    bool MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint);

    /** 设置窗口图标（支持*.ico格式）
    *  @param [in] iconFilePath ico文件的路径（绝对路径）
    */
    bool SetWindowIcon(const FilePath& iconFilePath);

    /** 设置窗口图标（支持*.ico格式）
    *  @param [in] iconFileData ico文件的数据
    *  @param [in] iconFileName 包含扩展名的文件名，用于识别图片类型
    */
    bool SetWindowIcon(const std::vector<uint8_t>& iconFileData, const DString& iconFileName);

    /** 设置窗口标题栏文本
    * @param [in] strText 窗口标题栏文本
    */
    void SetText(const DString& strText);

    /** 获取窗口标题栏文本
    */
    DString GetText() const;

    /** 设置窗口大小的最小值（宽度和高度，内部不按DPI调整大小，DPI自适应需要调用方来做）
    * @param [in] szMaxWindow 窗口的最大宽度和最小高度，如果值为0，表示不做限制
    */
    void SetWindowMaximumSize(const UiSize& szMaxWindow);

    /** 获取窗口大小的最小值（宽度和高度）
    */
    const UiSize& GetWindowMaximumSize() const;

    /** 设置窗口大小的最大值（宽度和高度，内部不按DPI调整大小，DPI自适应需要调用方来做）
    * @param [in] szMinWindow 窗口的最小宽度和最小高度，如果值为0，表示不做限制
    */
    void SetWindowMinimumSize(const UiSize& szMinWindow);

    /** 获取窗口大小的最大值（宽度和高度）
    */
    const UiSize& GetWindowMinimumSize() const;

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

    /** 获取指定窗口所在显示器的显示器矩形
    * @param [out] rcMonitor 显示器的矩形区域
    */
    bool GetMonitorRect(UiRect& rcMonitor) const;

    /** 获取当前主显示器的工作区矩形
    * @param [out] rcWork 返回主屏幕坐标
    */
    static bool GetPrimaryMonitorWorkRect(UiRect& rcWork);

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
    * @param [in] pt 屏幕坐标点
    * @param [in] bIgnoreChildWindow true表示忽略子窗口，false表示不忽略子窗口
    */
    INativeWindow* WindowBaseFromPoint(const UiPoint& pt, bool bIgnoreChildWindow = false);

    /** 设置是否支持显示贴靠布局菜单（Windows 11新功能：通过将鼠标悬停在窗口的最大化按钮上或按 Win + Z，可以轻松访问对齐布局。）
    *   该功能默认是开启的。
    * @param [in] bEnable true表示支持，false表示不支持
    */
    void SetEnableSnapLayoutMenu(bool bEnable);

    /** 判断是否支持显示贴靠布局菜单(仅Windows 11及以后版本支持)
    */
    bool IsEnableSnapLayoutMenu() const;

    /** 设置在右键点击标题栏时，是否显示系统的窗口菜单（可进行调整窗口状态，关闭窗口等操作）
    * @param [in] bEnable true表示支持，false表示不支持
    */
    void SetEnableSysMenu(bool bEnable);

    /** 获取在右键点击标题栏时，是否显示系统的窗口菜单
    */
    bool IsEnableSysMenu() const;

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

    /** 设置输入法的开关状态（关闭再打开以后，能够保持原输入法状态）
    * @param [in] bOpen true标识打开输入法，false标识关闭输入法
    */
    void SetImeOpenStatus(bool bOpen);

    /** 设置输入区域
    * @param [in] rect 文本输入矩形区域
    * @param [in] nCursor 文本输入的位置(相对于rect.left的偏移)
    */
    void SetTextInputArea(const UiRect* rect, int32_t nCursor);

    /** 设置是否允许拖放操作
    * @param [in] bEnable true表示允许拖放操作，false表示禁止拖放操作
    */
    void SetEnableDragDrop(bool bEnable);

    /** 注销一个拖放接口
    */
    bool IsEnableDragDrop() const;

    /** 获取指定坐标点的控件接口
    * @param [in] pt 客户区坐标点
    */
    Control* FindControl(const UiPoint& pt) const;

private:
    /** 窗口过程函数
    * @param [in] hWnd 窗口句柄
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** 窗口过程函数(模态对话框入口函数)
    * @param [in] hWnd 窗口句柄
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    static INT_PTR CALLBACK __DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** 窗口过程函数(模态对话框)
    * @param [in] hWnd 窗口句柄
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    static LRESULT CALLBACK __DialogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

    //部分NC消息处理函数，以实现基本功能
    LRESULT OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcCalcSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcHitTestMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    //部分消息处理函数，以实现基本功能
    LRESULT OnGetMinMaxInfoMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnEraseBkGndMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnWindowPosChangingMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnCreateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnInitDialogMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 窗口消息的派发函数，将Window消息转换为内部格式，然后派发出去
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果
    */
    LRESULT ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
    /** 初始化窗口资源
    */
    void InitNativeWindow();

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并调用OnDeleteSelf函数销毁该窗口对象）
    */
    void OnFinalMessage();

    /** 设置Owner窗口为焦点窗口
    */
    bool SetOwnerWindowFocus();

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

    /** 更新最大化/最小化按钮的窗口风格，与程序的逻辑保持一致
    */
    void UpdateMinMaxBoxStyle() const;

    /** 显示系统的窗口菜单
    */
    bool ShowWindowSysMenu(HWND hWnd, const POINT& pt) const;

    /** 停止系统的窗口菜单定时器
    */
    void StopSysMenuTimer();

    /** 执行绘制操作
    */
    LRESULT OnPaintMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 同步创建窗口的属性
    */
    void SyncCreateWindowAttributes(const WindowCreateAttributes& createAttributes);

    /** 计算窗口居中的位置
    */
    bool CalculateCenterWindowPos(HWND hCenterWindow, int32_t& xPos, int32_t& yPos) const;

    /** 启用/禁用输入法
    */
    void EnableIME(HWND hwnd, bool bEnable);

    /** 检查窗口贴边操作，并给应用层回调
    */
    void CheckWindowSnap(HWND hWnd);

private:
    /** 接收窗口事件的接口
    */
    INativeWindow* m_pOwner;

    /** 窗口句柄
    */
    HWND m_hWnd;

    /** 创建时指定的父窗口
    */
    HWND m_hParentWnd;

    /** 资源模块句柄
    */
    HMODULE m_hResModule;

    /** 创建窗口时的初始化参数
    */
    WindowCreateParam m_createParam;

    /** 原来的窗口函数(仅限模式对话框使用)
    */
    WNDPROC m_pfnOldWndProc;

    /** 是否支持拖放操作
    */
    bool m_bEnableDragDrop;

    /** 当前窗口是否显示为模态对话框
    */
    bool m_bDoModal;

    /** 按ESC键的时候，是否关闭窗口(仅模态对话框有效)
    */
    bool m_bCloseByEsc;

    /** 按Enter键的时候，是否关闭窗口(仅模态对话框有效)
    */
    bool m_bCloseByEnter;

    //是否为层窗口
    bool m_bIsLayeredWindow;

    //窗口透明度，该值在UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)
    uint8_t m_nLayeredWindowAlpha;

    //窗口不透明度，该值在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)
    uint8_t m_nLayeredWindowOpacity;

    //是否使用系统的标题栏
    bool m_bUseSystemCaption;

    //窗口已经延迟关闭
    bool m_bCloseing;

    //窗口关闭的参数
    int32_t m_closeParam;

    //当前窗口是否显示为模态对话框
    bool m_bFakeModal;

    //是否支持显示贴靠布局菜单（Windows 11新功能：通过将鼠标悬停在窗口的最大化按钮上或按 Win + Z，可以轻松访问对齐布局。）
    //参考：https://learn.microsoft.com/zh-cn/windows/apps/desktop/modernize/apply-snap-layout-menu
    bool m_bSnapLayoutMenu;

    //在右键点击标题栏时，是否显示系统的窗口菜单（可进行调整窗口状态，关闭窗口等操作）
    bool m_bEnableSysMenu;

    //鼠标是否点击在最大按钮上
    bool m_bNCLButtonDownOnMaxButton;

    //系统菜单延迟显示的定时器ID
    UINT_PTR m_nSysMenuTimerId;

    //鼠标所在位置
    UiPoint m_ptLastMousePos;

    //绘制DC
    HDC m_hDcPaint;

private:
    /**@name 全屏相关状态
    * @{ */

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

    /** 窗口大小的最小值（宽度和高度）
    */
    UiSize m_szMinWindow;

    /** 窗口大小的最大值（宽度和高度）
    */
    UiSize m_szMaxWindow;

    /** 输入法的上下文
    */
    HIMC m_hImc;

    /** 拖放功能的实现接口
    */
    WindowDropTarget* m_pWindowDropTarget;
};

/** 定义别名
*/
typedef NativeWindow_Windows NativeWindow;

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN

#endif // UI_CORE_NATIVE_WINDOW_WINDOWS_H_

