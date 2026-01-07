#ifndef UI_CORE_WINDOW_BASE_H_
#define UI_CORE_WINDOW_BASE_H_

#include "duilib/Core/INativeWindow.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Core/EventArgs.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Core/NativeWindow_SDL.h"
#elif defined (DUILIB_BUILD_FOR_WIN)
    #include "duilib/Core/NativeWindow_Windows.h"
#else
    class NativeWindow;
#endif

namespace ui
{
class IRender;
class WindowCreateParam;
class WindowCreateAttributes;

/** 窗口的基本功能封装（平台相关的窗口功能封装）
*/
class UILIB_API WindowBase: public INativeWindow
{
public:
    WindowBase();
    WindowBase(const WindowBase& r) = delete;
    WindowBase& operator=(const WindowBase& r) = delete;
    virtual ~WindowBase() override;

public:
    /** 创建窗口, 可使用 OnInitWindow 接口来实现窗口创建完成后的自定义需求
    *   注意事项：子类Window对象在创建窗口成功以后，在窗口销毁时，会调用Window::OnFinalMessage函数删除自身。
    * @param [in] pParentWindow 父窗口
    * @param [in] createParam 创建窗口所需的参数
    */
    bool CreateWnd(WindowBase* pParentWindow, const WindowCreateParam& createParam);

    /** 显示模态窗口, 可使用 OnInitWindow 接口来实现窗口创建完成后的自定义需求
    *   注意事项：
    *       (1) DoModal 是阻塞式的，窗口窗口后，直到窗口销毁后，该函数才返回
    *       (2) DoModal的窗口在窗口销毁时，不会销毁自身（包括Window子类）对象，需要自己管理对象的生命周期
    * @param [in] pParentWindow 父窗口
    * @param [in] createParam 创建窗口所需的参数
    * @param [in] bCloseByEsc 按ESC键的时候，是否关闭窗口
    * @param [in] bCloseByEnter 按Enter键的时候，是否关闭窗口
    * @return 窗口退出时的返回值, 如果失败则返回-1
    */
    int32_t DoModal(WindowBase* pParentWindow, const WindowCreateParam& createParam,
                    bool bCloseByEsc = true, bool bCloseByEnter = false);

    /** 是否含有有效的窗口句柄
    */
    bool IsWindow() const;

    /** 当前窗口是不是子窗口（非弹出窗口类型的子窗口, 对于Windows系统，是只含有WS_CHILD风格的窗口）
    */
    bool IsChildWindow() const;

    /** 获取父窗口
    */
    WindowBase* GetParentWindow() const;

    /** 获取窗口的实现接口
    */
    NativeWindow* NativeWnd() const;

    /** 设置是否使用系统标题栏
    */
    void SetUseSystemCaption(bool bUseSystemCaption);

    /** 获取是否使用系统标题栏
    */
    bool IsUseSystemCaption() const;

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
    *             该参数在UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)。
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
    /** @name 窗口显示、隐藏、大小、位置等状态相关接口
    * @{
    */
    /** 窗口的显示、隐藏、最大化、还原、最小化操作
     * @param [in] nCmdShow 显示或者隐藏窗口的命令
    */
    bool ShowWindow(ShowWindowCommands nCmdShow);

    /** 显示模态对话框(父窗口在创建的时候指定, 将父窗口设置为Disable状态)
    */
    void ShowModalFake();

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

    /** 使窗口进入全屏状态
    */
    bool EnterFullscreen();

    /** 使窗口退出全屏状态 (默认按ESC键时，退出全屏)
    */
    bool ExitFullscreen();

    /** 窗口是否为最大化状态
    */
    bool IsWindowMaximized() const;

    /** 窗口是否为最小化状态
    */
    bool IsWindowMinimized() const;

    /** 窗口是否为全屏状态
    */
    bool IsWindowFullscreen() const;

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

    /** 设置窗口位置（对 ::SetWindowPos API 的一层封装，内部无DPI缩放）
    * @param [in] insertAfter 对应 SetWindowPos 的 hWndInsertAfter 选项
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] cx 窗口的宽度
    * @param [in] cy 窗口的高度
    * @param [in] uFlags 参考 enum WindowPosFlags 选项
    */
    bool SetWindowPos(const InsertAfterWnd& insertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags);

    /** 设置窗口位置和大小
    * @param [in] X 窗口的X坐标
    * @param [in] Y 窗口的Y坐标
    * @param [in] nWidth 窗口的宽度
    * @param [in] nHeight 窗口的高度
    * @param [in] bRepaint 是否重绘窗口
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

    /** 根据语言列表中的文本 ID， 根据ID设置窗口标题栏文本
    * @param [in] strTextId 语言 ID，该 ID 必须在语言文件中存在
    */
    void SetTextId(const DString& strTextId);

    /** 获取窗口标题栏文本的文本ID
    */
    const DString& GetTextId() const;

    /** 获取窗口ID
    */
    const DString& GetWindowId() const;

    /** 设置窗口ID
    */
    void SetWindowId(const DString& windowId);

    /** 获取窗口的Class名称
    */
    const DString& GetWindowClassName() const;

    /** 获取该窗口对应的DPI管理器
    */
    const DpiManager& Dpi() const;

    /** 主动调整窗口的界面显示比例
    *   注意事项：该函数可用改变窗口的显示比例，从而改变窗口、控件的界面百分比。
    * @param [in] nNewDisplayScaleFactor 新的界面显示比例值，比如100代表界面DPI缩放比为100%
    * @param [in] bDisableDpiAware true表示禁止DPI感知，当显示器的DPI变化时，该窗口的DPI不再跟随其变化
                                   false表示仍然保持DPI感知功能，当显示器的DPI变化时，仍然跟随其变化
    */
    bool ChangeDisplayScale(uint32_t nNewDisplayScaleFactor, bool bDisableDpiAware = true);

    /** 设置窗口的圆角矩形RGN
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] szRoundCorner 圆角大小，其值不能为0
    * @param [in] bRedraw 是否重绘
    */
    bool SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw);

    /** 设置窗口的矩形RGN
    * @param [in] rcWnd 需要设置RGN的区域，坐标为屏幕坐标
    * @param [in] bRedraw 是否重绘
    */
    bool SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw);

    /** 清除窗口的RGN, 回复系统默认值
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

    /** 设置窗口标题栏区域，对应 XML 中 caption 属性
    * @param [in] rcCaption 要设置的标题栏区域的矩形范围(rcClient代表窗口的客户区矩形范围)：
    *              标题栏的left  : rcClient.left + rcCaption.left
    *              标题栏的top   : rcClient.top + rcCaption.top
    *              标题栏的right : rcClient.right - rcCaption.right
    *              标题栏的bottom: rcClient.top + rcCaption.bottom
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale);

    /** 获取窗口菜单区域（双击该区域可退出窗口，点击显示系统的窗口菜单）
    */
    const UiRect& GetSysMenuRect() const;

    /** 设置获取窗口菜单区域（双击该区域可退出窗口，点击显示系统的窗口菜单）
    * @param [in] rcSysMenuRect 要设置的区域范围
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetSysMenuRect(const UiRect& rcSysMenuRect, bool bNeedDpiScale);

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

    /** 获取窗口圆角大小，对应 XML 中 roundcorner 属性
    */
    const UiSize& GetRoundCorner() const;

    /** 设置窗口圆角大小
    * @param [in] cx 圆角宽
    * @param [in] cy 圆角高
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetRoundCorner(int cx, int cy, bool bNeedDpiScale);

    /** 设置窗口大小的最小值（宽度和高度）
    * @param [in] szMinWindow 窗口的最小宽度和最小高度，如果值为0，表示不做限制
    * @param [in] bNeedDpiScale 为 false 表示不需要根据 DPI 自动调整
    */
    void SetWindowMaximumSize(const UiSize& szMinWindow, bool bNeedDpiScale);

    /** 获取窗口大小的最小值（宽度和高度）
    */
    const UiSize& GetWindowMaximumSize() const;

    /** 设置窗口大小的最大值（宽度和高度）
    * @param [in] szMaxWindow 窗口的最大宽度和最小高度，如果值为0，表示不做限制
    * @param [in] bNeedDpiScale 为 false 表示不需要根据 DPI 自动调整
    */
    void SetWindowMinimumSize(const UiSize& szMaxWindow, bool bNeedDpiScale);

    /** 获取窗口大小的最大值（宽度和高度）
    */
    const UiSize& GetWindowMinimumSize() const;

    /** @}*/

public:
    /** @name 窗口消息相关的接口
    * @{ */

    /** 添加一个消息过滤器，此时消息已经派发，该接口的消息处理优先级高于Window类的消息处理函数
    * @param [in] pFilter 一个继承了 IUIMessageFilter 的对象实例，需要实现接口中的方法
    */
    bool AddMessageFilter(IUIMessageFilter* pFilter);

    /** 移除一个消息过滤器
    * @param [in] pFilter 一个继承了 IUIMessageFilter 的对象实例
    */
    bool RemoveMessageFilter(IUIMessageFilter* pFilter);

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

    /** @}*/

public:
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

    /* 将rc坐标从客户区坐标转换为屏幕坐标
    * @param [out] rc 返回屏幕坐标
    */
    void ClientToScreen(UiRect& rc) const;

    /* 将rc坐标从屏幕坐标转换为客户区坐标
    * @param [out] rc 返回客户区坐标
    */
    void ScreenToClient(UiRect& rc) const;

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

    /** 获取鼠标最后的坐标
    */
    const UiPoint& GetLastMousePos() const;

    /** 设置鼠标最后的坐标
    */
    void SetLastMousePos(const UiPoint& pt);

    /** 获取窗口的句柄（Windows平台返回的是窗口句柄HWND, SDL实现时返回的是SDL_Window*）
    */
    void* GetWindowHandle() const;

#ifdef DUILIB_BUILD_FOR_SDL
    /** 获取当前窗口实现的驱动名称
    */
    DString GetVideoDriverName() const;

    /** 获取当前Render绘制引擎的名称
    */
    DString GetWindowRenderName() const;
#endif

    /** 界面是否完成首次显示
    */
    bool IsWindowFirstShown() const;

public:
    /** 监听窗口创建事件
    * @param [in] callback 指定的回调函数，wParam是1表示为通过DoModal函数显示的模态对话框，wParam为0表示为普通窗口
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowCreateMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口关闭事件
    * @param [in] callback 指定的回调函数，wParam是窗口关闭时的退出码
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowCloseMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口第一次显示事件（已完成界面布局设置）
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowFirstShown(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口位置大小变化事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowPosChangedMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口大小变化事件
    * @param [in] callback 指定的回调函数, wParam是WindowSizeType类型的值
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowSizeMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口位置变化事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMoveMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口显示属性事件
    * @param [in] callback 指定的回调函数, wParam为1表示显示，wParam为0表示隐藏
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowShowWindowMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口绘制事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowPaintMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口获取焦点事件
    * @param [in] callback 指定的回调函数, wParam是失去焦点的窗口的指针(WindowBase*)
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowSetFocusMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口失去焦点事件
    * @param [in] callback 指定的回调函数, wParam是获取焦点的窗口的指针(WindowBase*)
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowKillFocusMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口设置光标事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowSetCursorMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口键盘按下事件
    * @param [in] callback 指定的回调函数: vkCode 虚拟键盘代码，modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowKeyDownMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口键盘弹起事件
    * @param [in] callback 指定的回调函数: vkCode 虚拟键盘代码，modifierKey 按键标志位，有效值：ModifierKey::kAlt
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowKeyUpMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标滚轮事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置
    *                                    modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    *                                    eventData  滚轮旋转的距离，正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMouseWheelMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标移动事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMouseMoveMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标悬停事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMouseHoverMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标离开事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMouseLeaveMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标左键按下事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowLButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标左键弹起事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowLButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标左键双击事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowLButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标右键按下事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowRButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标右键弹起事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowRButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标右键双击事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowRButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标中键按下事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMButtonDownMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标中键弹起事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMButtonUpMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口鼠标中键双击事件
    * @param [in] callback 指定的回调函数：ptMouse是鼠标所在位置，modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowMButtonDbClickMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

    /** 监听窗口丢失鼠标捕获事件
    * @param [in] callback 指定的回调函数
    * @param [in] callbackID 该回调函数对应的ID（用于删除回调函数）
    */
    void AttachWindowCaptureChangedMsg(const EventCallback& callback, EventCallbackID callbackID = 0);

public:
    /** 本窗口是否含有回调函数（根据回调事件类型）
    * @param [in] eventType 回调事件类型
    */
    bool HasWindowEventCallback(EventType eventType) const;

    /** 本窗口是否含有回调函数（根据回调函数ID）
    * @param [in] callbackID 该回调函数对应的ID
    */
    bool HasWindowEventCallbackByID(EventCallbackID callbackID) const;

    /** 删除回调函数（根据回调事件类型）
    * @param [in] eventType 回调事件类型
    */
    void DetachWindowEventCallback(EventType eventType);

    /** 删除回调函数（根据回调函数ID）
    * @param [in] callbackID 该回调函数对应的ID
    */
    void DetachWindowEventCallbackByID(EventCallbackID callbackID);

protected:
    /** 正在初始化窗口数据
    */
    virtual void PreInitWindow() = 0;

    /** 初始化窗口数据（当窗口创建完成以后调用此函数），供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() = 0;

    /** 完成初始化窗口数据
    */
    virtual void PostInitWindow() = 0;

    /** 完成界面布局的初始化，各个控件的位置大小等布局信息完成初始化
    */
    virtual void OnInitLayout() = 0;

    /** 窗口正在关闭，处理内部状态
    */
    virtual void PreCloseWindow() = 0;

    /** 窗口已经关闭，处理内部状态
    */
    virtual void PostCloseWindow() = 0;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息
    */
    virtual void FinalMessage() = 0;

protected:
    /** 进入全屏状态
    */
    virtual void NotifyWindowEnterFullscreen() = 0;

    /** 退出全屏状态
    */
    virtual void NotifyWindowExitFullscreen() = 0;

protected:
    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() = 0;

    /** 准备绘制
    * @return 返回true表示继续绘制，返回false表示不再继续绘制
    */
    virtual bool OnPreparePaint() = 0;

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() = 0;

    /** 窗口的透明度发生变化
    */
    virtual void OnWindowAlphaChanged() = 0;

    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullscreen() = 0;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullscreen() = 0;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局(供子类使用)
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致，该值可能与nOldScaleFactor相同
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) = 0;

    /** 获取设置的窗口阴影的大小
    * @param [out] rcShadow 返回设置窗口阴影的大小，未经过DPI缩放
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const = 0;

    /** 获取当前窗口阴影的大小
    * @param [out] rcShadow 返回当前窗口阴影的大小，已经过DPI缩放
    */
    virtual void GetCurrentShadowCorner(UiPadding& rcShadow) const = 0;

    /** 判断一个点是否在放置在标题栏上的控件上
    */
    virtual bool IsPtInCaptionBarControl(const UiPoint& pt) const = 0;

    /** 判断是否含有最大化和最小化按钮
    * @param [out] bMinimizeBox 返回true表示含有最小化按钮
    * @param [out] bMaximizeBox 返回true表示含有最大化按钮
    */
    virtual bool HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const = 0;

    /** 判断一个点是否在最大化或者还原按钮上
    */
    virtual bool IsPtInMaximizeRestoreButton(const UiPoint& pt) const = 0;

    /** 获取创建窗口的属性（从XML文件的Window标签中读取的属性值）
    * @param [out] createAttributes 返回从XML文件的Window标签中读取的创建窗口的属性
    */
    virtual void GetCreateWindowAttributes(WindowCreateAttributes& createAttributes) = 0;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致
    */
    virtual void OnDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor);

    /** 获取绘制引擎对象
    */
    virtual IRender* GetRender() const = 0;

    /** 获取指定坐标点的控件接口
    * @param [in] pt 客户区坐标点
    */
    virtual Control* OnFindControl(const UiPoint& pt) const = 0;

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
    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;

    /** 窗口位置大小发生改变(WM_WINDOWPOSCHANGED)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowPosChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口移动(WM_MOVE)
    * @param [in] ptTopLeft 窗口客户端区域左上角的 x 坐标和 y 坐标（坐标为屏幕坐标）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口显示或者隐藏(WM_SHOWWINDOW)
    * @param [in] bShow true表示窗口正在显示，false表示窗口正在隐藏
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口绘制(WM_PAINT)
    * @param [in] rcPaint 本次绘制，需要更新的矩形区域
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnPaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口获得焦点(WM_SETFOCUS)
    * @param [in] pLostFocusWindow 已失去键盘焦点的窗口（可以为nullptr）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSetFocusMsg(WindowBase* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口失去焦点(WM_KILLFOCUS)
    * @param [in] pSetFocusWindow 接收键盘焦点的窗口（可以为nullptr）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 通知应用程序输入焦点变化(WM_IME_SETCONTEXT)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeSetContextMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 输入法开始生成组合字符串(WM_IME_STARTCOMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 更改按键组合状态(WM_IME_COMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 输入法结束组合(WM_IME_ENDCOMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeEndCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 设置光标(WM_SETCURSOR)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 如果应用程序处理了此消息，它应返回 TRUE 以停止进一步处理或 FALSE 以继续
    */
    virtual LRESULT OnSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 通知窗口用户希望显示上下文菜单(WM_CONTEXTMENU)，用户可能单击了鼠标右键 (在窗口中右键单击) ，按下了 Shift+F10 或按下了应用程序键， (上下文菜单键) 某些键盘上可用。
    * @param [in] pt 鼠标所在位置，客户区坐标, 如果是(-1,-1)表示用户键入了 SHIFT+F10
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 键盘按下(WM_CHAR)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 快捷键消息（WM_HOTKEY）
    * @param [in] hotkeyId 热键的ID
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kAlt, ModifierKey::kControl, ModifierKey::kShift, ModifierKey::kWin
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 旋转鼠标滚轮(WM_MOUSEWHEEL)
    * @param [in] wheelDelta 滚轮旋转的距离，以 WHEEL_DELTA (120) 的倍数或除法表示。 正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] bFromNC true表示这是NC消息（WM_NCMOUSEMOVE）, false 表示是WM_MOUSEMOVE消息
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标悬停消息（WM_MOUSEHOVER）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标离开消息（WM_MOUSELEAVE）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标左键按下消息（WM_LBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标左键双击消息（WM_LBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标右键按下消息（WM_RBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标右键弹起消息（WM_RBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标右键双击消息（WM_RBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标中键按下消息（WM_MBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标中键弹起消息（WM_MBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 鼠标中键双击消息（WM_MBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kControl, ModifierKey::kShift
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口创建成功的事件(WM_CREATE/WM_INITDIALOG)
    * @param [in] bDoModal 当前是否为通过DoModal函数显示的模态对话框
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual void OnCreateWndMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled) = 0;

    /** 窗口位置的贴边操作
    * @param [in] bLeftSnap 窗口左侧贴边
    * @param [in] bRightSnap 窗口右侧贴边
    * @param [in] bTopSnap 窗口上侧贴边
    * @param [in] bBottomSnap 窗口下侧贴边
    */
    virtual void OnWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap) = 0;

    /** @}*/

protected:
    /** 获取一个点对应的窗口接口
    * @param [in] pt 屏幕坐标点
    * @param [in] bIgnoreChildWindow true表示忽略子窗口，false表示不忽略子窗口
    */
    WindowBase* WindowBaseFromPoint(const UiPoint& pt, bool bIgnoreChildWindow = false);

    /** 处理DPI变化的系统通知消息
    * @param [in] fNewDisplayScale 新的窗口的界面显示比例值，1.0f时表示无缩放
    * @param [in] fNewPixelDensity 新的窗口的像素密度值（仅SDL实现时使用）
    */
    void OnDisplayScaleChangedMsg(float fNewDisplayScale, float fNewPixelDensity);

    /** 清理窗口资源
    */
    void ClearWindowBase();

    /** 主动发起一个消息(kWindowMsgBegin - kWindowMsgEnd), 发送给该窗口的事件回调管理器中注册的消息处理函数
    * @param [in] eventType 转化后的消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param [in] pt 鼠标事件对应的坐标点
    * @param [in] modifierKey 鼠标事件对应的键盘按键状态（有效值：ModifierKey::kControl, ModifierKey::kShift）
    * @param [in] msg 消息内容
    */
    bool SendWindowEvent(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0);
    bool SendWindowMouseEvent(EventType eventType, const UiPoint& pt, uint32_t modifierKey);
    bool SendWindowEvent(const EventArgs& msg);

private:
    /** 初始化窗口数据（内部函数，子类重写后，必须调用基类函数，否则影响功能）
    */
    void InitWindowBase();

    /** 窗口大小变化，处理内部业务
    */
    void OnWindowSize(WindowSizeType sizeType);

private:
    //来自实现窗口的事件
    virtual void OnNativeWindowEnterFullscreen() override final;
    virtual void OnNativeWindowExitFullscreen() override final;
    virtual UiRect OnNativeGetSizeBox() const override final;
    virtual void OnNativeGetShadowCorner(UiPadding& rcShadow) const override final;
    virtual const DpiManager& OnNativeGetDpi() const override final;
    virtual void OnNativeGetCaptionRect(UiRect& captionRect) const override final;
    virtual void OnNativeGetSysMenuRect(UiRect& sysMenuRect) const override final;
    virtual bool OnNativeIsPtInCaptionBarControl(const UiPoint& pt) const override final;
    virtual bool OnNativeHasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const override final;
    virtual bool OnNativeIsPtInMaximizeRestoreButton(const UiPoint& pt) const override final;
    virtual void OnNativePreCloseWindow() override final;
    virtual void OnNativePostCloseWindow() override final;
    virtual void OnNativeUseSystemCaptionBarChanged() override final;
    virtual bool OnNativePreparePaint() override final;
    virtual IRender* OnNativeGetRender() const override final;
    virtual Control* OnNativeFindControl(const UiPoint& pt) const override final;

    virtual void    OnNativeFinalMessage() override final;
    virtual LRESULT OnNativeWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override final;
    virtual void    OnNativeDisplayScaleChangedMsg(float fNewDisplayScale, float fNewPixelDensity) override final;
    virtual void    OnNativeCreateWndMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeWindowPosChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMoveMsg(const UiPoint& ptTopLeft, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativePaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnNativeSetFocusMsg(INativeWindow* pLostFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeKillFocusMsg(INativeWindow* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeImeSetContextMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeImeCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeImeEndCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeSetCursorMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, bool bFromNC, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseHoverMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseLeaveMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseMButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseMButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeMouseMButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual LRESULT OnNativeWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) override final;
    virtual void OnNativeWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap) override final;

private:
    //父窗口
    ControlPtrT<WindowBase> m_pParentWindow;

    //该窗口消息过滤器列表
    std::vector<IUIMessageFilter*> m_aMessageFilters;

    //窗口自己的DPI管理器
    std::unique_ptr<DpiManager> m_dpi;

private:
    //窗口标题栏文本的文本ID
    DString m_textId;

    //窗口ID
    DString m_windowId;

    //窗口的Class名称
    DString m_windowClassName;

    //窗口四边可拉伸范围信息
    UiRect m_rcSizeBox;

    //窗口圆角信息
    UiSize m_szRoundCorner;

    //标题栏区域信息
    UiRect m_rcCaption;

    //窗口菜单区域（双击该区域可退出窗口，点击显示系统的窗口菜单）
    UiRect m_rcSysMenuRect;

private:
    /** 窗口的实现类
    */
    NativeWindow* m_pNativeWindow;

    /** 窗口的事件
    */
    EventMap m_windowEventMap;

    //界面是否完成首次显示
    bool m_bWindowFirstShown;
};

} // namespace ui

#endif // UI_CORE_WINDOW_BASE_H_
