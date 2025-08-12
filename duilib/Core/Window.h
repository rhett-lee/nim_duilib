#ifndef UI_CORE_WINDOW_H_
#define UI_CORE_WINDOW_H_

#include "duilib/Core/WindowBase.h"
#include "duilib/Core/Shadow.h"
#include "duilib/Core/ControlFinder.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/ControlPtrT.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/Delegate.h"
#include "duilib/Utils/FilePath.h"

namespace ui
{

class Box;
class Control;
class ToolTip;
class WindowBuilder;

/** 窗口类
*  //外部调用需要初始化的基本流程:
*  1. 调用Window::CreateWnd创建窗口;
*  //以下内容，可用实现在PreInitWindow函数中:
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
    void SetResourcePath(const FilePath& strPath);

    /** 获取窗口资源路径
    */
    const FilePath& GetResourcePath() const;

    /** 设置窗口关联的XML文件所在路径(相对于GetResourcePath()的路径)
    * @param [in] xmlPath 要设置的路径
    */
    void SetXmlPath(const FilePath& xmlPath);

    /** 获取窗口关联的XML文件所在路径
    * @return 返回XML文件所在子目录，实际XML文件的所在路径是：GetResourcePath() + GetXmlPath()
    */
    const FilePath& GetXmlPath() const;

    /** 绑定窗口的顶层容器
    * @param [in] pRoot 容器指针，一般是Xml里面配置的最外层的容器指针
    */
    bool AttachBox(Box* pRoot);

    /** 获取窗口顶层的容器
    @return 返回窗口顶层的容器，可能是阴影的Box容器（当调用AttachBox时），也可能是Xml里面配置的Box容器
    */
    Box* GetRoot() const;

    /** 获取Xml里面配置的顶层的容器
    @return 返回Xml里面配置的Box容器
    */
    Box* GetXmlRoot() const;

    /** 获取父窗口
    */
    Window* GetParentWindow() const;

    /** 界面是否完成首次显示
    */
    bool IsWindowFirstShown() const;

    /** 监听窗口首次显示事件
    * @param [in] callback 当窗口第一次显示时回调此事件（必须在界面显示前设置回调，即当IsWindowFirstShown()返回false的情况下设置，否则没有机会再回调）
    * @return 如果窗口已经完成第一次显示返回false，表示不会有回调函数；如果窗口未完成第一次显示，返回true
    */
    bool AttachWindowFirstShown(const EventCallback& callback);

    /** 监听窗口创建并初始化完成事件
    * @param [in] callback 指定创建并初始化完成后的回调函数，参数的wParam代表：wParam为1表示DoModal对话框，为0表示普通窗口
    */
    void AttachWindowCreate(const EventCallback& callback);

    /** 监听窗口关闭事件
    * @param [in] callback 指定关闭后的回调函数，参数的wParam代表窗口关闭的触发情况, 参见enum WindowCloseParam
    */
    void AttachWindowClose(const EventCallback& callback);

    /** 监听窗口获取焦点事件
    * @param [in] callback 指定的回调函数
    */
    void AttachWindowSetFocus(const EventCallback& callback);

    /** 监听窗口失去焦点事件
    * @param [in] callback 指定的回调函数
    */
    void AttachWindowKillFocus(const EventCallback& callback);

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

    /** 设置该窗口的渲染引擎绘制后台类型
    *   该属性值只能在创建窗口前设置，或者在XML配置中指定，应用后不支持修改
    * @param [in] backendType 渲染引擎绘制后台类型
    * @return 如果设置成功返回true, 否则返回false
    */
    bool SetRenderBackendType(RenderBackendType backendType);

    /** 获取渲染引擎绘制后台类型
    */
    RenderBackendType GetRenderBackendType() const;

    /** 设置窗口图标（支持*.ico格式）
    *  @param [in] iconFilePath ico文件的路径（在资源根目录内的相对路径）
    */
    bool SetWindowIcon(const DString& iconFilePath);

public:
    /** @name 窗口阴影相关接口
    * @{
    */
    /** 附加窗口阴影
    * @param pXmlRoot XML文件中配置的顶层容器
    * @return 如果IsShadowAttached()为true，返回阴影的容器指针；如果IsShadowAttached()为false，则返回pXmlRoot
    */
    virtual Box* AttachShadow(Box* pXmlRoot);

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
    void SetUseDefaultShadowAttached(bool bDefault);

    /** 设置阴影类型
    */
    void SetShadowType(Shadow::ShadowType nShadowType);

    /** 获取阴影类型
    */
    Shadow::ShadowType GetShadowType() const;

    /** 获取阴影图片
    */
    DString GetShadowImage() const;

    /** 设置窗口阴影图片
    * @param [in] shadowImage 图片位置
    */
    void SetShadowImage(const DString& shadowImage);

    /** 设置阴影的边框大小(未经DPI缩放)
    */
    void SetShadowBorderSize(int32_t nShadowBorderSize);

    /** 获取阴影的边框大小(未经DPI缩放)
    */
    int32_t GetShadowBorderSize() const;

    /** 设置阴影的边框颜色
    */
    void SetShadowBorderColor(const DString& shadowBorderColor);

    /** 获取阴影的边框颜色
    */
    DString GetShadowBorderColor() const;

    /** 获取当前的阴影九宫格属性（已经做过DPI缩放）
     *@return 如果阴影未Attached或者窗口最大化，返回UiPadding(0, 0, 0, 0)，否则返回设置的九宫格属性（已经做过DPI缩放）
     */
    UiPadding GetCurrentShadowCorner() const;

    /** 获取已经设置的阴影九宫格属性
     *@return 返回通过SetShadowCorner函数设置的九宫格属性，未经DPI缩放的值
     */
    UiPadding GetShadowCorner() const;

    /** 设置阴影素材的九宫格描述
    * @param [in] rcShadowCorner 阴影图片的九宫格属性，未经DPI缩放的值
    */
    void SetShadowCorner(const UiPadding& rcShadowCorner);

    /** 获取阴影的圆角大小
    * @return 返回阴影的圆角大小，未经DPI缩放的值
    */
    UiSize GetShadowBorderRound() const;

    /** 设置阴影的圆角大小
    * @param [in] szBorderRound 阴影的圆角大小，未经DPI缩放的值
    */
    void SetShadowBorderRound(UiSize szBorderRound);

    /** 设置阴影是否支持窗口贴边操作
    */
    void SetEnableShadowSnap(bool bEnable);

    /** 获取阴影是否支持窗口贴边操作
    */
    bool IsEnableShadowSnap() const;

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

    /** 设置控件焦点时(即调用SetFocusControl函数时)，是否同时设置窗口焦点
    */
    void SetCheckSetWindowFocus(bool bCheckSetWindowFocus);

    /** 控件焦点时(即调用SetFocusControl函数)，是否同时设置窗口焦点
    */
    bool IsCheckSetWindowFocus() const;

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
    * @param [in] pt 屏幕坐标点
    * @param [in] bIgnoreChildWindow true表示忽略子窗口，false表示不忽略子窗口
    */
    Window* WindowFromPoint(const UiPoint& pt, bool bIgnoreChildWindow = false);

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
    */
    void SetInitSize(int cx, int cy);

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

public:
    /** 这个消息中，指定的按键是否按下
    * @param [in] msg 当前处理的消息
    * @param [in] modifierKey 需要判断的键盘状态
    */
    bool IsKeyDown(const EventArgs& msg, ModifierKey modifierKey) const;

    /** 获取绘制引擎对象
    */
    virtual IRender* GetRender() const override;

    /** 获取为Render使用的本窗口关联的DPI转换对象
    */
    std::shared_ptr<IRenderDpi> GetRenderDpi();

    /** 设置窗口的属性是否已经设置完成(避免重复设置窗口属性)
    */
    void SetWindowAttributesApplied(bool bApplied);

    /** 获取窗口的属性是否已经设置完成
    */
    bool IsWindowAttributesApplied() const;

    /** 初始化皮肤配置文件
    * @param [in] skinFolder 窗口皮肤目录, 为相对路径
    * @param [in] skinFile 窗口皮肤 XML 描述文件
    */
    void InitSkin(const DString& skinFolder, const DString& skinFile);

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录, 为相对路径
    */
    virtual DString GetSkinFolder();

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile();

    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const DString& strClass);

public:
    // 窗口的属性设置
    /** 设置窗口指定属性
     * @param[in] strName 要设置的属性名称（如 width）
     * @param[in] strValue 要设置的属性值（如 100）
     */
    virtual void SetAttribute(const DString& strName, const DString& strValue);

    /** 设置控件的 class 全局属性
     * @param[in] strClass 要设置的 class 名称，该名称必须在 global.xml 中存在
     */
    void SetClass(const DString& strClass);

    /** 应用一套属性列表
     * @param[in] strList 属性列表的字符串表示，如 `width="800" height="600"`
     */
    void ApplyAttributeList(const DString& strList);

    /** 设置是否允许拖放操作
    * @param [in] bEnable true表示允许拖放操作，false表示禁止拖放操作
    */
    void SetEnableDragDrop(bool bEnable);

    /** 注销一个拖放接口
    */
    bool IsEnableDragDrop() const;

protected:
    /** 正在初始化窗口数据(内部函数，子类重写后，必须调用基类函数，否则影响功能)
    */
    virtual void PreInitWindow() override;

    /** 初始化窗口数据（当窗口创建完成以后调用此函数），供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override {};

    /** 完成初始化窗口数据
    */
    virtual void PostInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnPreCloseWindow() {};

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() {};

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是调用delete销毁自身，若不想销毁自身，可重写该函数）
    */
    virtual void OnFinalMessage();

    /** 窗口的阴影类型发生了变化
    */
    virtual void OnWindowShadowTypeChanged() {};

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
    virtual void OnWindowEnterFullScreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() override;

    /** 窗口的DPI发生了变化(供子类使用)
    * @param [in] nOldDPI 旧的DPI值
    * @param [in] nNewDPI 新的DPI值
    */
    virtual void OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI) override;

    /** 获取设置的窗口阴影的大小
    * @param [out] rcShadow 返回设置窗口阴影的大小，未经过DPI缩放
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const override;

    /** 获取当前的阴影九宫格属性（已经做过DPI缩放）
     *@param [out] rcShadow 如果阴影未Attached或者窗口最大化，返回UiPadding(0, 0, 0, 0)，否则返回设置的九宫格属性（已经做过DPI缩放）
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

    /** 获取指定坐标点的控件接口
    * @param [in] pt 客户区坐标点
    */
    virtual Control* OnNativeFindControl(const UiPoint& pt) const override;

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
    * @param [in] rcPaint 本次绘制，需要更新的矩形区域
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnPaintMsg(const UiRect& rcPaint, const NativeMsg& nativeMsg, bool& bHandled) override;

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

    /** 窗口关闭消息（WM_CLOSE）
    * @param [in] wParam 消息的wParam参数
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnWindowCloseMsg(uint32_t wParam, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口创建成功的事件(WM_CREATE/WM_INITDIALOG)
    * @param [in] bDoModal 当前是否为通过DoModal函数显示的模态对话框
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual void OnCreateWndMsg(bool bDoModal, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 窗口位置的贴边操作
    * @param [in] bLeftSnap 窗口左侧贴边
    * @param [in] bRightSnap 窗口右侧贴边
    * @param [in] bTopSnap 窗口上侧贴边
    * @param [in] bBottomSnap 窗口下侧贴边
    */
    virtual void OnWindowPosSnapped(bool bLeftSnap, bool bRightSnap, bool bTopSnap, bool bBottomSnap) override;

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
    * @param [in] bHideToolTip 是否需要隐藏ToolTip
    * @return 返回 true 需要发送鼠标进入或离开消息，返回 false 为不需要
    */
    bool HandleMouseEnterLeave(const UiPoint& pt, uint32_t modifierKey, bool bHideToolTip);

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
    * @param [in] rcPaint 本次绘制更新的矩形区域
    * @return 如果执行了绘制返回true，否则返回false
    */
    bool Paint(const UiRect& rcPaint);

    /** 调整Render的尺寸，与当前客户区的大小一致
    */
    bool ResizeRenderToClientSize() const;

    /** 解析窗口关联的XML文件
    */
    void ParseWindowXml();

    /** 焦点控件发生变化
    */
    void OnFocusControlChanged();

private:
    //事件回调管理器
    EventMap m_OnEvent;

    //焦点控件
    ControlPtr m_pFocus;

    //鼠标在悬停控件
    ControlPtr m_pEventHover;

    /** 点击的控件：
        在 WM_LBUTTONDOWN/WM_RBUTTONDOWN/WM_LBUTTONDBLCLK 赋值
        在 WM_LBUTTONUP中/WM_KILLFOCUS 清空
    */
    ControlPtr m_pEventClick;

    /** 键盘按下的控件
    *   在 WM_KEYDOWN / WM_SYSKEYDOWN 赋值
    *   在 WM_KEYUP / WM_SYSKEYUP清空
    */
    ControlPtr m_pEventKey;

    /** 控件查找辅助类
    */
    ControlFinder m_controlFinder;

    /** 窗口关联的容器，根节点
    */
    BoxPtr m_pRoot;

    //窗口阴影
    std::unique_ptr<Shadow> m_shadow;

private:
    //透明通道修补范围的的九宫格描述
    UiRect m_rcAlphaFix;

    //布局是否变化，如果变化(true)则需要重新计算布局
    bool m_bIsArranged;

    //布局是否需要初始化
    bool m_bFirstLayout;

    //界面是否完成首次显示
    bool m_bWindowFirstShown;

    //设置控件焦点时，是否同时设置窗口焦点
    bool m_bCheckSetWindowFocus;

    //绘制时的偏移量（动画用）
    UiPoint m_renderOffset;

    //全屏状态下的外边距
    UiMargin m_rcFullscreenMargin;

    //绘制引擎
    std::unique_ptr<IRender> m_render;

private:
    /** 每个窗口的资源路径(相对于资源根目录的路径)
    */
    FilePath m_resourcePath;

    /** 窗口关联的XML文件所在路径(相对于m_resourcePath目录的路径), 实际XML文件所在目录是：m_resourcePath + m_xmlPath
    */
    FilePath m_xmlPath;

    /** 皮肤路径
    */
    DString m_skinFolder;

    /** 皮肤配置文件
    */
    DString m_skinFile;

    /** XML解析及控件创建
    */
    std::unique_ptr<WindowBuilder> m_windowBuilder;

private:
    /** 窗口配置中class名称与属性映射关系
    */
    std::map<DString, DString> m_defaultAttrHash;

    /** 窗口颜色字符串与颜色值（ARGB）的映射关系
    */
    ColorMap m_colorMap;

    /** 该窗口下每个Option group下的控件（即单选控件是分组的）
    */
    std::map<DString, std::vector<Control*>> m_mOptionGroup;

    /** Tooltip
    */
    std::unique_ptr<ToolTip> m_toolTip;

    /** 窗口关闭的时候，发送退出消息循环的请求
    */
    bool m_bPostQuitMsgWhenClosed;

    /** 渲染引擎的后台绘制方式（CPU、OpenGL等）
    */
    RenderBackendType m_renderBackendType;

    /** 窗口的初始大小
    */
    UiSize m_szInitSize;

    /** 窗口的属性是否已经设置完成
    */
    bool m_bWindowAttributesApplied;
};

} // namespace ui

#endif // UI_CORE_WINDOW_H_
