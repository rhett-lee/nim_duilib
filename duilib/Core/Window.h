#ifndef UI_CORE_WINDOW_H_
#define UI_CORE_WINDOW_H_

#pragma once

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
*  //以下内容，可用实现在OnCreate函数中:
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
    virtual ~Window();

public:
    /** @name 窗口创建、销毁、事件监听等相关接口
    * @{
    */

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

    /** @} */

public:
    /** 获取透明通道修补范围的的九宫格描述，对应 XML 中 alphafixcorner 属性
    */
    const UiRect& GetAlphaFixCorner() const;

    /** 设置透明通道修补范围的的九宫格描述
    * @param [in] rc 要设置的修补范围
    * @param [in] bNeedDpiScale 是否进行DPI缩放
    */
    void SetAlphaFixCorner(const UiRect& rc, bool bNeedDpiScale);

    /** 设置窗口初始大小
    * @param [in] cx 宽度
    * @param [in] cy 高度
    * @param [in] bContainShadow 为 false 表示 cx cy 不包含阴影
    * @param [in] bNeedDpiScale 为 false 表示不根据 DPI 调整
    */
    void SetInitSize(int cx, int cy, bool bContainShadow /*= false*/, bool bNeedDpiScale);

public:
    /** @name 窗口布局相关接口
    * @{
    */
    /** 绑定窗口的顶层容器
    * @param [in] pRoot 容器指针
    */
    bool AttachBox(Box* pRoot);

    /** 获取窗口最外层的容器
    */
    Box* GetRoot() const;

    /** 获取窗口资源路径
    */
    const DString& GetResourcePath() const;

    /** 设置窗口资源路径
    * @param [in] strPath 要设置的路径
    */
    void SetResourcePath(const DString& strPath);

    /** 初始化控件，在容器中添加控件时会被调用（用于对控件名称做缓存）
    * @param [in] pControl 控件指针
    */
    bool InitControls(Control* pControl);

    /** 初始化布局
    */
    virtual void OnInitLayout();

    /** 回收控件
    * @param [in] pControl 控件指针
    */
    void ReapObjects(Control* pControl);

    /** 延迟销毁一个控件
    * @param [in] pControl 控件指针
    */
    void AddDelayedCleanup(Control* pControl);

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

    /** @}*/

public:
    /** @name 窗口消息相关的接口
    * @{
    */

    /** 主动发起一个消息, 发送给该窗口的事件回调管理器（m_OnEvent）中注册的消息处理函数
    * @param [in] eventType 转化后的消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    */
    bool SendNotify(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0);

protected:

    /** 窗口消息的派发函数，优先调用内部处理函数，如果内部无处理逻辑，则调用Windows默认处理函数处理此消息
    * @param [in] uMsg 消息体
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，否则将消息继续传递给窗口过程
    * @return 返回消息的处理结果
    */
    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    /** @}*/

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 窗口的透明度发生变化
    */
    virtual void OnWindowAlphaChanged() override;

    /** 窗口的层窗口属性发生变化
    */
    virtual void OnLayeredWindowChanged() override;

    /** 获取窗口阴影的大小
    * @param [out] rcShadow 获取圆角的大小
    */
    virtual void GetShadowCorner(UiPadding& rcShadow) const override;

private:
    /** @name 私有窗口消息处理相关
    * @{
    */
    //部分消息处理函数，以实现基本功能    
    LRESULT OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcCalcSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcHitTestMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNcLButtonDbClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnCloseMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnGetMinMaxInfoMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnWindowPosChangingMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSizeMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnMoveMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnEraseBkGndMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnPaintMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnMouseHoverMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnMouseLeaveMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnMouseMoveMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnMouseWheelMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnLButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnRButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnLButtonDoubleClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnRButtonDoubleClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnLButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnRButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnContextMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnIMEStartCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnIMEEndCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSetFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnKillFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCharMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnKeyDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnKeyUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSysKeyDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSysKeyUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSetCusorMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnSysCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnHotKeyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    LRESULT OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
    LRESULT OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

private:
    //鼠标等按下消息处理函数
    void OnButtonDown(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt);

    //鼠标等弹起消息处理函数
    void OnButtonUp(EventType eventType, WPARAM wParam, LPARAM lParam, const UiPoint& pt);

    //鼠标移动消息处理函数
    void OnMouseMove(WPARAM wParam, LPARAM lParam, const UiPoint& pt);

    /** 鼠标滚轮消息处理函数
    *  @param [in] wParam 滚轮旋转的距离，正值表示滚轮向前旋转，远离用户;负值表示滚轮向后向用户旋转。
    *  @param [in] lParam 含有特殊处理， 0表示严格按照wParam指定的距离滚动, 非0表示按照常规逻辑处理滚轮旋转的距离
    *  @param [in] pt 鼠标所在的坐标值，为窗口客户区坐标
    */
    void OnMouseWheel(WPARAM wParam, LPARAM lParam, const UiPoint& pt);

    //清除鼠标键盘操作状态
    void ClearStatus();

    /** 判断是否需要发送鼠标进入或离开消息
    * @param [in] pt 鼠标当前位置
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @return 返回 true 需要发送鼠标进入或离开消息，返回 false 为不需要
    */
    bool HandleMouseEnterLeave(const UiPoint& pt, WPARAM wParam, LPARAM lParam);

    /** @}*/

public:
    /** @name 窗口焦点相关接口
    * @{
    */
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

    /** 获取鼠标最后的坐标
    */
    const UiPoint& GetLastMousePos() const;

    /** 切换控件焦点到下一个（或上一个）控件
    * @param [in] bForward true 为上一个控件，否则为 false，默认为 true
    */
    bool SetNextTabControl(bool bForward = true);

    /** @}*/

public:
    /** @name 窗口阴影、层窗口透明度相关接口
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

    /** 绘制函数体
    */
    void Paint();

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

protected:
    /** 初始化窗口数据
    */
    virtual void InitWindow() override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并调用OnDeleteSelf函数销毁该窗口对象）
    */
    virtual void OnFinalMessage() override;

private:
    /** 窗口的DPI发生变化，更新控件大小和布局(该函数不允许重写，如果需要此事件，可以重写OnWindowDpiChanged函数，实现功能)
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override final;

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

    //鼠标所在位置
    UiPoint m_ptLastMousePos;

    /** 控件查找辅助类
    */
    ControlFinder m_controlFinder;

    /** 窗口关联的容器，根节点
    */
    Box* m_pRoot;

    //延迟释放的控件接口
    std::vector<Control*> m_aDelayedCleanup;

    //窗口阴影
    std::unique_ptr<Shadow> m_shadow;

private:
    //透明通道修补范围的的九宫格描述
    UiRect m_rcAlphaFix;

    //布局是否变化，如果变化(true)则需要重新计算布局
    bool m_bIsArranged;

    //布局是否需要初始化
    bool m_bFirstLayout;

private:
    //绘制时的偏移量（动画用）
    UiPoint m_renderOffset;

    //绘制引擎
    std::unique_ptr<IRender> m_render;

private:
    //每个窗口的资源路径(相对于资源根目录的路径)
    DString m_strResourcePath;

    //窗口配置中class名称与属性映射关系
    std::map<DString, DString> m_defaultAttrHash;

    //窗口颜色字符串与颜色值（ARGB）的映射关系
    ColorMap m_colorMap;

    //该窗口下每个Option group下的控件（即单选控件是分组的）
    std::map<DString, std::vector<Control*>> m_mOptionGroup;

private:
    //Tooltip
    std::unique_ptr<ToolTip> m_toolTip;
};

} // namespace ui

#endif // UI_CORE_WINDOW_H_
