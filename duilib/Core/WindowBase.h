#ifndef UI_CORE_WINDOW_BASE_H_
#define UI_CORE_WINDOW_BASE_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Core/Keyboard.h"
#include <string>

namespace ui
{
#define UI_WNDSTYLE_FRAME       (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define UI_WNDSTYLE_DIALOG      (WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define UI_CLASSSTYLE_FRAME     (CS_VREDRAW | CS_HREDRAW)
#define UI_CLASSSTYLE_DIALOG    (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)

/**  窗口消息过滤接口，用于截获窗口过程的消息，优先于Window类进行消息处理
*/
class IUIMessageFilter
{
public:
    /**  消息处理函数，处理优先级高于Window类的消息处理函数
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，返回 true 表示不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class WindowDropTarget;
class ControlDropTarget;

/** 窗口的基本功能封装（平台相关的窗口功能封装）
*/
class UILIB_API WindowBase : public virtual SupportWeakCallback
{
public:
    WindowBase();
    WindowBase(const WindowBase& r) = delete;
    WindowBase& operator=(const WindowBase& r) = delete;
    virtual ~WindowBase();

public:

    /** @name 窗口创建、销毁、事件监听等相关接口
    * @{
    */

    /** 创建窗口, 可使用 OnInitWindow 接口来实现窗口创建完成后的自定义需求
    * @param [in] pParentWindow 父窗口
    * @param [in] windowName 窗口名称
    * @param [in] dwStyle 窗口样式
    * @param [in] dwExStyle 窗口拓展样式, 可以设置层窗口（WS_EX_LAYERED）等属性
    * @param [in] rc 窗口大小
    */
    virtual bool CreateWnd(WindowBase* pParentWindow,
                           const DString& windowName,
                           uint32_t dwStyle,
                           uint32_t dwExStyle,
                           const UiRect& rc = UiRect(0, 0, 0, 0));

    /** 获取资源的句柄
    * @return 默认返回当前进程exe的句柄
    */
    virtual HMODULE GetResModuleHandle() const;

    /** 创建窗口时被调用，由子类实现用以获取窗口唯一的类名称
    * @return 基类返回空串，在子类中需实现并返回窗口唯一的类名称
    */
    virtual DString GetWindowClassName() const;

    /** 获取窗口类的样式，该方法由实例化的子类实现，https://docs.microsoft.com/en-us/windows/desktop/winmsg/window-class-styles
    * @return 返回窗口类的样式，该方法基类返回 CS_DBLCLKS
    */
    virtual UINT GetClassStyle() const;

    /** 获取窗口样式
    * @return 默认返回当前窗口的样式去掉WS_CAPTION属性
    *         如果子类重写该函数后，返回值为0，则不改变当前窗口的样式
    */
    virtual uint32_t GetWindowStyle() const;

    /** 设置是否使用系统标题栏
    */
    void SetUseSystemCaption(bool bUseSystemCaption);

    /** 获取是否使用系统标题栏
    */
    bool IsUseSystemCaption() const;

    /** 设置窗口透明度
    * @param [in] nAlpha 透明度数值[0, 255]
    */
    void SetWindowAlpha(int nAlpha);

    /** 获取窗口透明度
    * @param [in] nAlpha 透明度数值[0, 255]
    */
    uint8_t GetWindowAlpha() const;

    /** 设置是否为层窗口
    */
    void SetLayeredWindow(bool bIsLayeredWindow);

    /** 是否为层窗口
    */
    bool IsLayeredWindow() const;

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
    bool IsClosingWnd() const { return m_bCloseing; }

    /** @} */
public:
    /** @name 窗口显示、隐藏、大小、位置等状态相关接口
    * @{
    */
    /** 显示或隐藏窗口
     * @param [in] bShow 为 true 时显示窗口，为 false 时为隐藏窗口，默认为 true
     * @param [in] bTakeFocus 是否获得焦点（激活窗口），默认为 true
    */
    void ShowWindow(bool bShow = true, bool bTakeFocus = true);

    /** 显示模态对话框(父窗口在创建的时候指定)
    */
    void ShowModalFake();

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

    /** 激活窗口
    *   如果当前窗口为最小化，则进行还原；
    *   如果当前窗口为隐藏，则显示，并设置为前端窗口
    *   如果当前窗口为显示，则设置为前端窗口
    */
    void ActiveWindow();

    /** 窗口最大化
   */
    bool Maximized();

    /** 还原窗口，退出最大化
    */
    bool Restore();

    /** 窗口最小化
    */
    bool Minimized();

    /** 使窗口进入全屏状态
    */
    bool EnterFullScreen();

    /** 使窗口退出全屏状态 (默认按ESC键时，退出全屏)
    */
    bool ExitFullScreen();

    /** 设置窗口为前端窗口
    */
    bool SetForeground();

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

    /** 当前窗口是否为前端窗口
    */
    bool IsWindowForeground() const;

    /** 窗口是否为最大化状态
    */
    bool IsWindowMaximized() const;

    /** 窗口是否为最小化状态
    */
    bool IsWindowMinimized() const;

    /** 窗口是否为全屏状态
    */
    bool IsWindowFullScreen() const;

    /** 更新窗口，执行重绘
    */
    bool UpdateWindow() const;

    /** 窗口是否为最大化状态
    */
    bool IsZoomed() const;

    /** 窗口是否为最小花状态
    */
    bool IsIconic() const;

    /** 将窗口的Enable状态
    * @param [in] bEnable true表示设置为Enable状态，false表示设置为disable状态
    */
    bool EnableWindow(bool bEnable);

    /** 获取窗口的Enable状态
    */
    bool IsWindowEnabled() const;

    /** 设置窗口位置（对 ::SetWindowPos API 的一层封装，内部无DPI缩放）
    * @param [in] hWndInsertAfter 对应 SetWindowPos 的 hWndInsertAfter 选项
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] cx 窗口的宽度
    * @param [in] cy 窗口的高度
    * @param [in] uFlags 对应 ::SetWindowPos API 的 uFlags 选项
    */
    bool SetWindowPos(HWND hWndInsertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, UINT uFlags);

    /** 设置窗口位置（对 ::SetWindowPos API 的一层封装）
    * @param [in] rc 窗口位置
    * @param [in] bNeedDpiScale 为 false 表示不需要把 rc 根据 DPI 自动调整
    * @param [in] uFlags 对应 ::SetWindowPos API 的 uFlags 选项
    * @param [in] hWndInsertAfter 对应 SetWindowPos 的 hWndInsertAfter 选项
    * @param [in] bContainShadow rc区域是否包含阴影范围，默认为 false
    */
    bool SetWindowPos(const UiRect& rc, bool bNeedDpiScale, UINT uFlags, HWND hWndInsertAfter = NULL, bool bContainShadow = false);

    /** 设置窗口位置和大小
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] nWidth 窗口的宽度
    * @param [in] nHeight 窗口的高度
    * @param [in] uFlags 对应 ::SetWindowPos API 的 uFlags 选项
    */
    bool MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint);

    /** 获取窗口位置信息
     * @param [in] bContainShadow 是否包含阴影，true 为包含，默认为 false 不包含
     */
    UiRect GetWindowPos(bool bContainShadow /*= false*/) const;

    /** 重置窗口大小
    * @param [in] cx 宽度
    * @param [in] cy 高度
    * @param [in] bContainShadow 为 false 表示 cx cy 不包含阴影
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void Resize(int cx, int cy, bool bContainShadow /*= false*/, bool bNeedDpiScale);

    /** @} */

    /** 设置窗口图标
    *  @param [in] nRes 窗口图标资源 ID
    */
    void SetIcon(UINT nRes);

    /** 设置窗口标题栏文本
    * @param [in] strText 窗口标题栏文本
    */
    void SetText(const DString& strText);

    /** 获取窗口标题栏文本
    */
    DString GetText() const;

    /** 根据语言列表中的文本 ID， 根据ID设置窗口标题栏文本
    * @param [in] strTextId 语言 ID，该 ID 必须在语言文件中存在
    */
    void SetTextId(const DString& strTextId);

    /** 获取窗口标题栏文本的文本ID
    */
    DString GetTextId() const;

    /** 获取该窗口对应的DPI管理器
    */
    const DpiManager& Dpi() const;

    /** 主动调整窗口的DPI
    *   注意事项：该函数可用改变窗口的显示DPI，从而改变窗口、控件的界面百分比；
    *            但如果进程的DPI感知类型是kPerMonitorDpiAware/kPerMonitorDpiAware_V2时，
    *            当系统DPI改变或者跨越不同DPI的多屏幕时，窗口的大小和DPI会被系统调整，进而会有冲突，导致窗口控件大小显示异常。
    * @param [in] nNewDPI 新的DPI值，比如96代表界面DPI缩放比为100%
    */
    bool ChangeDpi(uint32_t nNewDPI);

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

public:
    /** @name 窗口大小、标题栏等相关接口
    * @{
    */

    /** 获取窗口四边可拉伸范围的大小
    */
    const UiRect& GetSizeBox() const;

    /** 设置窗口四边可拉伸范围的大小
    * @param [in] rcSizeBox 要设置的大小
    * @param [in] bNeedDpiScale 是否支持DPI缩放
    */
    void SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale);

    /** 获取窗口标题栏区域（可拖动区域），对应 XML 中 caption 属性
    */
    const UiRect& GetCaptionRect() const;

    /** 设置窗口标题栏区域
    * @param [in] rcCaption 要设置的区域范围
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale);

    /** 获取窗口圆角大小，对应 XML 中 roundcorner 属性
    */
    const UiSize& GetRoundCorner() const;

    /** 设置窗口圆角大小
    * @param [in] cx 圆角宽
    * @param [in] cy 圆角高
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetRoundCorner(int cx, int cy, bool bNeedDpiScale);

    /** 获取窗口最小范围，对应 XML 中 mininfo 属性
    * @param [in] bContainShadow 是否包含阴影范围，默认为 false
    */
    UiSize GetMinInfo(bool bContainShadow /*= false*/) const;

    /** 设置窗口最小范围
    * @param [in] cx 宽度
    * @param [in] cy 高度
    * @param [in] bContainShadow 为 false 表示 cx cy 不包含阴影
    * @param [in] bNeedDpiScale 为 false 表示不需要把 rc 根据 DPI 自动调整
    */
    void SetMinInfo(int cx, int cy, bool bContainShadow /*= false*/, bool bNeedDpiScale);

    /** 获取窗口最大范围，对应 XML 中 maxinfo 属性
    * @param [in] bContainShadow 是否包含阴影范围，默认为 false
    */
    UiSize GetMaxInfo(bool bContainShadow /*= false*/) const;

    /** 设置窗口最大范围
    * @param [in] cx 宽度
    * @param [in] cy 高度
    * @param [in] bContainShadow 为 false 表示 cx cy 不包含阴影
    * @param [in] bNeedDpiScale 为 false 表示不需要把 rc 根据 DPI 自动调整
    */
    void SetMaxInfo(int cx, int cy, bool bContainShadow /*= false*/, bool bNeedDpiScale);

    /** @}*/

public:
    /** 获取窗口所属的 Windows 句柄
    */
    HWND GetHWND() const;

    /** 是否含有有效的窗口句柄
    */
    bool IsWindow() const;

    /** @name 窗口消息相关的接口
    * @{

    /** 添加一个消息过滤器，此时消息已经派发，该接口的消息处理优先级高于Window类的消息处理函数
    * @param [in] pFilter 一个继承了 IUIMessageFilter 的对象实例，需要实现接口中的方法
    */
    bool AddMessageFilter(IUIMessageFilter* pFilter);

    /** 移除一个消息过滤器
    * @param [in] pFilter 一个继承了 IUIMessageFilter 的对象实例
    */
    bool RemoveMessageFilter(IUIMessageFilter* pFilter);

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

    /** 调用系统默认的窗口处理函数，对 CallWindowProc API 的一层封装
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    LRESULT CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** @}*/

public:
    /** 获取绘制区域 DC
    */
    HDC GetPaintDC() const;

    /** 发出重绘消息
    * @param [in] rcItem 重绘范围，为客户区坐标
    */
    void Invalidate(const UiRect& rcItem);

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

    /** 注册一个拖放接口
    */
    bool RegisterDragDrop(ControlDropTarget* pDropTarget);

    /** 注销一个拖放接口
    */
    bool UnregisterDragDrop(ControlDropTarget* pDropTarget);

protected:
    /** 初始化窗口数据
    */
    virtual void InitWindow();

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow();

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow();

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并调用OnDeleteSelf函数销毁该窗口对象）
    */
    virtual void OnFinalMessage();

    /** 销毁自己（子类可用重载这个方法，避免自身被销毁）
    */
    virtual void OnDeleteSelf();

    /** 窗口消息的派发函数
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，
                返回 true  表明已经成功处理消息，不需要再传递给窗口过程；
                返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果
    */
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /** 自定义窗口消息的派发函数，仅供内部实现使用
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，否则将消息继续传递给窗口过程
    * @return 返回消息的处理结果
    */
    virtual LRESULT HandleUserMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;

    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullScreen() {}

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() {}

    /** 进入最大化状态
    */
    virtual void OnWindowMaximized() {}

    /** 从最大化还原
    */
    virtual void OnWindowRestored() {}

    /** 进入最小化状态
    */
    virtual void OnWindowMinimized() {}

    /** 窗口的DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale);

    /** 窗口的DPI发生了变化(供子类使用)
    * @param [in] nOldDPI 旧的DPI值
    * @param [in] nNewDPI 新的DPI值
    */
    virtual void OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI);

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged();

    /** 窗口的透明度发生变化
    */
    virtual void OnWindowAlphaChanged() {};

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() {};

    /** 获取窗口阴影的大小
    * @param [out] rcShadow 获取圆角的大小 
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const { (void)rcShadow; }

    /** 判断一个点是否在放置在标题栏上的控件上
    */
    virtual bool IsPtInCaptionBarControl(const UiPoint& pt) const { (void)pt;  return false; }

    /** @name 窗口消息处理相关
     * @{
     */
    enum class WindowSizeType
    {
        kSIZE_RESTORED  = 0,    //窗口已调整大小，但 kSIZE_MINIMIZED 和 kSIZE_MAXIMIZED 值都不适用
        kSIZE_MINIMIZED = 1,    //窗口已最小化
        kSIZE_MAXSHOW   = 2,    //当其他一些窗口还原到其之前的大小时，消息将发送到所有弹出窗口
        kSIZE_MAXIMIZED = 3,    //窗口已最大化
        kSIZE_MAXHIDE   = 4     //当其他一些窗口最大化时，消息将发送到所有弹出窗口
    };
    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, bool& bHandled);

    /** 窗口移动(WM_MOVE)
    * @param [in] ptTopLeft 窗口客户端区域左上角的 x 坐标和 y 坐标（坐标为屏幕坐标）
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMoveMsg(const UiPoint& ptTopLeft, bool& bHandled);

    /** 窗口绘制(WM_PAINT)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnPaintMsg(bool& bHandled);

    /** 窗口获得焦点(WM_SETFOCUS)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSetFocusMsg(bool& bHandled);

    /** 窗口失去焦点(WM_KILLFOCUS)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKillFocusMsg(bool& bHandled);

    /** 输入法开始生成组合字符串(WM_IME_STARTCOMPOSITION)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeStartCompositionMsg(bool& bHandled);

    /** 输入法结束组合(WM_IME_ENDCOMPOSITION)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeEndCompositionMsg(bool& bHandled);

    /** 设置光标(WM_SETCURSOR)
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续
    */
    virtual LRESULT OnSetCursorMsg(bool& bHandled);

    /** 通知窗口用户希望显示上下文菜单(WM_CONTEXTMENU)，用户可能单击了鼠标右键 (在窗口中右键单击) ，按下了 Shift+F10 或按下了应用程序键， (上下文菜单键) 某些键盘上可用。
    * @param [in] pt 鼠标所在位置，客户区坐标, 如果是(-1,-1)表示用户键入了 SHIFT+F10
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnContextMenuMsg(const UiPoint& pt, bool& bHandled);

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled);

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled);

    /** 键盘按下(WM_CHAR)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled);

    /** 快捷键消息（WM_HOTKEY）
    * @param [in] hotkeyId 热键的ID
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, bool& bHandled);

    /** 旋转鼠标滚轮(WM_MOUSEWHEEL)
    * @param [in] wheelDelta 滚轮旋转的距离，以 WHEEL_DELTA (120) 的倍数或除法表示。 正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标悬停消息（WM_MOUSEHOVER）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标离开消息（WM_MOUSELEAVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLeaveMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标左键按下消息（WM_LBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标左键双击消息（WM_LBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标右键按下消息（WM_RBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标右键弹起消息（WM_RBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 鼠标右键双击消息（WM_RBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, bool& bHandled);

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(bool& bHandled);

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, bool& bHandled);

    /** @}*/

protected:
    /** 获取一个点对应的窗口接口
    */
    WindowBase* WindowBaseFromPoint(const UiPoint& pt);

    /** 处理DPI变化的系统通知消息
    * @param [in] nNewDPI 新的DPI值
    * @param [in] rcNewWindow 新的窗口位置（建议值）
    */
    void ProcessDpiChangedMsg(uint32_t nNewDPI, const UiRect& rcNewWindow);

    /** 检查并确保当前窗口为焦点窗口
    */
    void CheckSetWindowFocus();

private:
    /** 注册窗口类
    */
    bool RegisterWindowClass(const DString& className);

    /** 窗口消息的处理函数, 从系统接收到消息后，进入的第一个处理函数
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息的处理结果
    */
    LRESULT WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    /** 窗口过程函数
    * @param [in] hWnd 窗口句柄
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回消息处理结果
    */
    static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    /** @name 私有窗口消息处理相关
     * @{
     */
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
    LRESULT OnSysCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

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
    /** @}*/
private:
    /** 将屏幕坐标转换为指定窗口的客户区坐标
    */
    void ScreenToClient(HWND hWnd, UiPoint& pt) const;

    /** 将指定窗口的客户区坐标转换为屏幕坐标
    */
    void ClientToScreen(HWND hWnd, UiPoint& pt) const;

    /** 获取指定窗口的客户区矩形
    */
    void GetClientRect(HWND hWnd, UiRect& rcClient) const;

    /** 获取指定窗口的窗口区矩形
    */
    void GetWindowRect(HWND hWnd, UiRect& rcWindow) const;

    /** 获取指定窗口所在显示器的工作区矩形，以虚拟屏幕坐标表示。
        请注意，如果显示器不是主显示器，则一些矩形的坐标可能是负值。
    */
    bool GetMonitorWorkRect(HWND hWnd, UiRect& rcWork) const;

    /** 获取指定窗口所在显示器的显示器矩形和工作区矩形
    */
    bool GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const;

    /* 将rc的左上角坐标和右下角坐标点从相对于一个窗口的坐标空间转换为相对于另一个窗口的坐标空间
    */
    void MapWindowRect(HWND hwndFrom, HWND hwndTo, UiRect& rc) const;

    /** 获取当前窗口Owner窗口句柄
    */
    HWND GetWindowOwner() const;

    /** 清理窗口资源
    * @param [in] bSendClose 是否发送关闭事件
    */
    void ClearWindow();

    /** 获取一个消息的按键标志位
    * @param [out] modifierKey 返回标志位，参见类型定义：ModifierKey
    */
    bool GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const;

protected:
    //窗口句柄
    HWND m_hWnd;

    /** 父窗口
    */
    WindowBase* m_pParentWindow;

    /** 父窗口的WeakFlag
    */
    std::weak_ptr<WeakFlag> m_parentFlag;

    //是否为层窗口
    bool m_bIsLayeredWindow;

    //窗口透明度(仅当使用层窗口时有效)
    uint8_t m_nWindowAlpha;

    //该窗口消息过滤器列表
    std::vector<IUIMessageFilter*> m_aMessageFilters;

    //当前窗口是否显示为模态对话框
    bool m_bFakeModal;

    //窗口已经延迟关闭：add by djj 20200428 调用Close时会延迟Post WM_CLOSE, 这个期间需要有一个标识此种'待关闭状态'
    bool m_bCloseing;

    //绘制DC
    HDC m_hDcPaint;

    //鼠标事件的捕获状态
    bool m_bMouseCapture;

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

private:
    /** 窗口标题栏文本的文本ID
    */
    DString m_text;

    /** 窗口标题栏文本的文本ID
    */
    DString m_textId;

    //窗口自己的DPI管理器
    std::unique_ptr<DpiManager> m_dpi;

    //是否使用系统的标题栏
    bool m_bUseSystemCaption;

private:
    //窗口最小信息
    UiSize m_szMinWindow;

    //窗口最大信息
    UiSize m_szMaxWindow;

    //窗口四边可拉伸范围信息
    UiRect m_rcSizeBox;

    //窗口圆角信息
    UiSize m_szRoundCorner;

    //标题栏区域信息
    UiRect m_rcCaption;

private:
    /** 系统全局热键的ID
    */
    std::vector<int32_t> m_hotKeyIds;

    /** 窗口的拖放操作管理接口
    */
    WindowDropTarget* m_pWindowDropTarget;
};

} // namespace ui

#endif // UI_CORE_WINDOW_BASE_H_
