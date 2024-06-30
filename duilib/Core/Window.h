#ifndef UI_CORE_WINDOW_H_
#define UI_CORE_WINDOW_H_

#include "duilib/Core/WindowBase.h"
#include "duilib/Utils/Delegate.h"
#include "duilib/Core/ControlFinder.h"
#include "duilib/Core/ColorManager.h"

namespace ui
{

class Box;
class Control;
class IRender;
class Shadow;
class ToolTip;

/** 窗口类
*  //外部调用需要初始化的基本流程:
*  1. 调用Window::CreateWnd创建窗口;
*  //以下内容，可用实现在OnInitWindow函数中:
*  2. Window::SetResourcePath;
*  3. WindowBuilder::Create, 得到Box* pRoot;
*  4. Window::AttachShadow(pRoot), 得到附加阴影的Box* pRoot, 以支持窗口阴影效果;
*  5. Window::AttachBox(pRoot);
*/
class UILIB_API Window : public WindowBase
{
public:
    Window();
    Window(const Window& r) = delete;
    Window& operator=(const Window& r) = delete;
    virtual ~Window() override;

public:
    /** 设置窗口资源路径(相对于资源根目录的路径)
    * @param [in] strPath 要设置的路径
    */
    void SetResourcePath(const DString& strPath);

    /** 获取窗口资源路径
    */
    const DString& GetResourcePath() const;

    /** 设置窗口关联的XML文件所在路径(相对于GetResourcePath()的路径)
    * @param [in] xmlPath 要设置的路径
    */
    void SetXmlPath(const DString& xmlPath);

    /** 获取窗口关联的XML文件所在路径
    * @return 返回XML文件所在子目录，实际XML文件的所在路径是：GetResourcePath() + GetXmlPath()
    */
    const DString& GetXmlPath() const;

    /** 绑定窗口的顶层容器
    * @param [in] pRoot 容器指针
    */
    bool AttachBox(Box* pRoot);

    /** 获取窗口最外层的容器
    */
    Box* GetRoot() const;

    /** 获取父窗口
    */
    Window* GetParentWindow() const;

    /** 监听窗口关闭事件
    * @param [in] callback 指定关闭后的回调函数，参数的wParam代表窗口关闭的触发情况：
                          0 - 表示 "确认" 关闭本窗口
                          1 - 表示点击窗口的 "关闭" 按钮关闭本窗口(默认值)
                          2 - 表示 "取消" 关闭本窗口
    */
    void AttachWindowClose(const EventCallback& callback);

    /** 主动发起一个消息, 发送给该窗口的事件回调管理器（m_OnEvent）中注册的消息处理函数
    * @param [in] eventType 转化后的消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    */
    bool SendNotify(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0);

    /** 窗口关闭的时候，发送退出消息循环的请求
    * @param [in] bPostQuitMsg 如果为true，表示窗口关闭的时候，发送退出消息循环请求
    */
    void PostQuitMsgWhenClosed(bool bPostQuitMsg);

public:
    /** @name 窗口阴影相关接口
    * @{
    */
    /** 附加窗口阴影
    */
    virtual Box* AttachShadow(Box* pRoot);

    /** 设置窗口是否附加阴影效果
    * @param [in] bShadowAttached 为 true 时附加，false 时不附加
    */
    void SetShadowAttached(bool bShadowAttached);

    /** 获取是否附加阴影效果
    */
    bool IsShadowAttached() const;

    /** 当前阴影效果值，是否为默认值
    */
    bool IsUseDefaultShadowAttached() const;

    /** 设置当前阴影效果值，是否为默认值
    */
    void SetUseDefaultShadowAttached(bool isDefault);

    /** 获取阴影图片
    */
    DString GetShadowImage() const;

    /** 设置窗口阴影图片
    * @param [in] strImage 图片位置
    */
    void SetShadowImage(const DString& strImage);

    /** 获取阴影的九宫格描述信息
    */
    UiPadding GetShadowCorner() const;

    /** 指定阴影素材的九宫格描述
    * @param [in] padding 九宫格描述信息
    * @param [in] bNeedDpiScale 为 false 表示不需要把 rc 根据 DPI 自动调整
    */
    void SetShadowCorner(const UiPadding& padding, bool bNeedDpiScale);

    /** @}*/

public:
    /** 获取当前持有焦点的控件
    */
    Control* GetFocusControl() const;

    /** 获取当前鼠标事件的控件
    */
    Control* GetEventClick() const;

    /** 设置焦点到指定控件上(设置窗口为焦点窗口，并设置该控件为焦点控件)
    * @param [in] pControl 控件指针
    */
    void SetFocusControl(Control* pControl);

    /** 让控件失去焦点（不影响窗口焦点）
    */
    void KillFocusControl();

    /** 获取当前鼠标在哪个控件上
    */
    Control* GetHoverControl() const;

    /** 切换控件焦点到下一个（或上一个）控件
    * @param [in] bForward true 为上一个控件，否则为 false，默认为 true
    */
    bool SetNextTabControl(bool bForward = true);

public:
    /** @name 窗口绘制相关接口
    * @{
    */
    /** 设置控件是否已经布局
    * @param [in] bArrange true 为已经排列，否则为 false
    */
    void SetArrange(bool bArrange);

    /** 获取绘制对象
    */
    IRender* GetRender() const;

     /** 判断当前是否渲染透明图层
    */
    bool IsRenderTransparent() const;

    /** 设置渲染透明图层
    * @param [in] bCanvasTransparent 设置 true 为渲染透明图层，否则为 false
    */
    bool SetRenderTransparent(bool bCanvasTransparent);

    /** 清理图片缓存
    */
    void ClearImageCache();

    /** 标记窗口区域无效
    */
    void InvalidateAll();

    /** @} */

public:
    /**@name 控件查找相关接口
    * @{
    */
    /** 根据坐标查找指定控件，采用默认属性：UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST
    * @param [in] pt 指定坐标
    */
    Control* FindControl(const UiPoint& pt) const;

    /**
    *  根据坐标查找可以响应WM_CONTEXTMENU的控件
    * @param [in] pt 指定坐标
    */
    Control* FindContextMenuControl(const UiPoint* pt) const;

    /** 查找可以支持拖放的Box容器
    * @param [in] pt 指定坐标
    * @param [in] nDropInId 拖放的ID值（每个控件可以设置一个ID，来接收拖放）
    */
    Box* FindDroppableBox(const UiPoint& pt, uint8_t nDropInId) const;

    /** 根据控件名称查找控件
    * @param [in] strName 控件名称
    */
    Control* FindControl(const DString& strName) const;

    /** 根据坐标查找子控件
    * @param [in] pParent 要搜索的控件
    * @param [in] pt 要查找的坐标
    */
    Control* FindSubControlByPoint(Control* pParent, const UiPoint& pt) const;

    /** 根据名字查找子控件
    * @param [in] pParent 要搜索的控件
    * @param [in] strName 要查找的名称
    */
    Control* FindSubControlByName(Control* pParent, const DString& strName) const;

    /** @} */

public:
    /** 获取一个点对应的窗口接口
    */
    Window* WindowFromPoint(const UiPoint& pt);

    /** 更新ToolTip信息（此时ToolTip的信息已经发生变化）
    */
    void UpdateToolTip();

public:
    /** 获取透明通道修补范围的的九宫格描述，对应 XML 中 alphafixcorner 属性
    */
    const UiRect& GetAlphaFixCorner() const;

    /** 设置透明通道修补范围的的九宫格描述
    * @param [in] rc 要设置的修补范围
    * @param [in] bNeedDpiScale 是否进行DPI缩放
    */
    void SetAlphaFixCorner(const UiRect& rc, bool bNeedDpiScale);

    /** 设置窗口初始大小, 对应XML文件中的 size 属性
    * @param [in] cx 宽度
    * @param [in] cy 高度
    * @param [in] bContainShadow 为 false 表示 cx cy 不包含阴影
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetInitSize(int cx, int cy, bool bContainShadow /*= false*/, bool bNeedDpiScale);

    /** 初始化控件，在容器中添加控件时会被调用（用于对控件名称做缓存）
    * @param [in] pControl 控件指针
    */
    bool InitControls(Control* pControl);

    /** 回收控件
    * @param [in] pControl 控件指针
    */
    void ReapObjects(Control* pControl);

    /** 添加一个通用样式
    * @param [in] strClassName 通用样式的名称
    * @param [in] strControlAttrList 通用样式的 XML 转义格式数据
    */
    void AddClass(const DString& strClassName, const DString& strControlAttrList);

    /** 获取指定通用样式的内容
    * @param [in] strClassName 通用样式名称
    * @return 返回指定名称的通用样式内容，XML 转义格式数据
    */
    DString GetClassAttributes(const DString& strClassName) const;

    /** 删除一个通用样式
    * @param [in] strClassName 要删除的通用样式名称
    */
    bool RemoveClass(const DString& strClassName);

    /** 删除所有通用样式
    */
    void RemoveAllClass();

    /** 添加一个颜色值提供窗口内使用
    * @param [in] strName 颜色名称（如 white）
    * @param [in] strValue 颜色具体数值（如 #FFFFFFFF）
    */
    void AddTextColor(const DString& strName, const DString& strValue);

    /** 添加一个颜色值提供窗口内使用
    * @param [in] strName 颜色名称（如 white）
    * @param [in] argb 颜色具体数值, 以ARGB格式表示
    */
    void AddTextColor(const DString& strName, UiColor argb);

    /** 根据名称获取一个颜色的具体数值
    * @param [in] strName 要获取的颜色名称
    * @return 返回 DWORD 格式的颜色描述值
    */
    UiColor GetTextColor(const DString& strName) const;

    /** 添加一个选项组
    * @param [in] strGroupName 组名称
    * @param [in] pControl 控件指针
    */
    bool AddOptionGroup(const DString& strGroupName, Control* pControl);

    /** 获取指定选项组中控件列表
    * @param [in] strGroupName 指定组名称
    * @return 返回该组下的所有控件列表
    */
    std::vector<Control*>* GetOptionGroup(const DString& strGroupName);

    /** 删除一个选项组
    * @param [in] strGroupName 组名称
    * @param [in] pControl 控件名称
    */
    void RemoveOptionGroup(const DString& strGroupName, Control* pControl);

    /** 删除所有选项组
    */
    void RemoveAllOptionGroups();

protected:
    /** 初始化窗口数据(内部函数，子类重写后，必须调用基类函数，否则影响功能)
    */
    virtual void InitWindow() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override {};

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() {};

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是调用delete销毁自身，若不想销毁自身，可重写该函数）
    */
    virtual void OnFinalMessage();

protected:
    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() override;

    /** 窗口的透明度发生变化
    */
    virtual void OnWindowAlphaChanged() override;

    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullScreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() override;

    /** 窗口的DPI发生了变化(供子类使用)
    * @param [in] nOldDPI 旧的DPI值
    * @param [in] nNewDPI 新的DPI值
    */
    virtual void OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI) override;

    /** 获取窗口阴影的大小
    * @param [out] rcShadow 获取圆角的大小
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const override;

    /** 判断一个点是否在放置在标题栏上的控件上
    */
    virtual bool IsPtInCaptionBarControl(const UiPoint& pt) const override;

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

    /** 窗口绘制(WM_SHOWWINDOW)
    * @param [in] bShow true表示窗口正在显示，false表示窗口正在隐藏
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnShowWindowMsg(bool bShow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口绘制(WM_PAINT)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnPaintMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

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

    /** 输入法开始生成组合字符串(WM_IME_STARTCOMPOSITION)
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnImeStartCompositionMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

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
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYUP 或者 WM_SYSKEYUP)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyUpMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_CHAR)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCharMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 快捷键消息（WM_HOTKEY）
    * @param [in] hotkeyId 热键的ID
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnHotKeyMsg(int32_t hotkeyId, VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 旋转鼠标滚轮(WM_MOUSEWHEEL)
    * @param [in] wheelDelta 滚轮旋转的距离，以 WHEEL_DELTA (120) 的倍数或除法表示。 正值表示滚轮向前旋转（远离用户）；负值表示滚轮向后旋转（朝向用户）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseWheelMsg(int32_t wheelDelta, const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标移动消息（WM_MOUSEMOVE）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseMoveMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标悬停消息（WM_MOUSEHOVER）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
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
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键弹起消息（WM_LBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标左键双击消息（WM_LBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseLButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键按下消息（WM_RBUTTONDOWN）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键弹起消息（WM_RBUTTONUP）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 鼠标右键双击消息（WM_RBUTTONDBLCLK）
    * @param [in] pt 鼠标所在位置，客户区坐标
    * @param [in] modifierKey 按键标志位，参见 Keyboard.h中的enum ModifierKey定义
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口丢失鼠标捕获（WM_CAPTURECHANGED）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnCaptureChangedMsg(const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** @}*/

private:
    /** 窗口正在关闭，处理内部状态（内部函数）
    */
    virtual void PreCloseWindow() override final;

    /** 窗口已经关闭，处理内部状态（内部函数）
    */
    virtual void PostCloseWindow() override final;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息
    */
    virtual void FinalMessage() override final;

    /** 窗口的DPI发生变化，更新控件大小和布局(该函数不允许重写，如果需要此事件，可以重写OnWindowDpiChanged函数，实现功能)
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override final;

private:
    //鼠标等按下消息处理函数
    void OnButtonDown(EventType eventType, const UiPoint& pt, const NativeMsg& nativeMsg, uint32_t modifierKey);

    //鼠标等弹起消息处理函数
    void OnButtonUp(EventType eventType, const UiPoint& pt, const NativeMsg& nativeMsg, uint32_t modifierKey);

    //清除鼠标键盘操作状态
    void ClearStatus();

    /** 判断是否需要发送鼠标进入或离开消息
    * @param [in] pt 鼠标当前位置
    * @param [in] modifierKey 按键标志
    * @return 返回 true 需要发送鼠标进入或离开消息，返回 false 为不需要
    */
    bool HandleMouseEnterLeave(const UiPoint& pt, uint32_t modifierKey);

private:
    /**@name 动画效果相关接口
    * @{
    */
    /** 设置绘制偏移
    * @param [in] renderOffset 偏移值
    */
    void SetRenderOffset(UiPoint renderOffset);

    /** 设置绘制偏移 x 坐标
    * @param [in] renderOffsetX 坐标值
    */
    void SetRenderOffsetX(int renderOffsetX);

    /** 设置绘制偏移 y 坐标
    * @param [in] renderOffsetY 坐标值
    */
    void SetRenderOffsetY(int renderOffsetY);

    /** @} */

private:
    /** 根据root自动调整窗口大小
    */
    void AutoResizeWindow(bool bRepaint);

    /** 对控件进行布局调整
    */
    void ArrangeRoot();

    /** 清理窗口资源
    * @param [in] bSendClose 是否发送关闭事件
    */
    void ClearWindow(bool bSendClose);

    /** 初始化布局
    */
    void OnInitLayout();

    /** 窗口显示或者隐藏
    */
    void OnShowWindow(bool bShow);

    /** 更新窗口状态，并创建Render等
    */
    bool PreparePaint(bool bArrange);

    /** 绘制函数体
    * @return 如果执行了绘制返回true，否则返回false
    */
    bool Paint();

private:
    //事件回调管理器
    EventMap m_OnEvent;

    //焦点控件
    Control* m_pFocus;

    //鼠标在悬停控件
    Control* m_pEventHover;

    /** 点击的控件：
        在 WM_LBUTTONDOWN/WM_RBUTTONDOWN/WM_LBUTTONDBLCLK 赋值
        在 WM_LBUTTONUP中/WM_KILLFOCUS 清空
    */
    Control* m_pEventClick;

    /** 键盘按下的控件
    *   在 WM_KEYDOWN / WM_SYSKEYDOWN 赋值
    *   在 WM_KEYUP / WM_SYSKEYUP清空
    */
    Control* m_pEventKey;

    /** 控件查找辅助类
    */
    ControlFinder m_controlFinder;

    /** 窗口关联的容器，根节点
    */
    Box* m_pRoot;

    //窗口阴影
    std::unique_ptr<Shadow> m_shadow;

private:
    //透明通道修补范围的的九宫格描述
    UiRect m_rcAlphaFix;

    //布局是否变化，如果变化(true)则需要重新计算布局
    bool m_bIsArranged;

    //布局是否需要初始化
    bool m_bFirstLayout;

    //绘制时的偏移量（动画用）
    UiPoint m_renderOffset;

    //绘制引擎
    std::unique_ptr<IRender> m_render;

private:
    //每个窗口的资源路径(相对于资源根目录的路径)
    DString m_resourcePath;

    //窗口关联的XML文件所在路径(相对于m_resourcePath目录的路径), 实际XML文件所在目录是：m_resourcePath + m_xmlPath
    DString m_xmlPath;

    //窗口配置中class名称与属性映射关系
    std::map<DString, DString> m_defaultAttrHash;

    //窗口颜色字符串与颜色值（ARGB）的映射关系
    ColorMap m_colorMap;

    //该窗口下每个Option group下的控件（即单选控件是分组的）
    std::map<DString, std::vector<Control*>> m_mOptionGroup;

    //Tooltip
    std::unique_ptr<ToolTip> m_toolTip;

    /** 窗口关闭的时候，发送退出消息循环的请求
    */
    bool m_bPostQuitMsgWhenClosed;
};

} // namespace ui

#endif // UI_CORE_WINDOW_H_
