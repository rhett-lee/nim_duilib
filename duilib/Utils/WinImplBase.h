#ifndef UI_UTILS_WINIMPLBASE_H_
#define UI_UTILS_WINIMPLBASE_H_

#include "duilib/duilib_defs.h"
#include "duilib/Core/Window.h"

namespace ui
{
/** 实现一个带有标题栏的窗体，带有最大化、最小化、还原按钮的支持
* 该实现支持窗口阴影，单仅提供带有 WS_EX_LAYERED 属性窗口阴影。
* 该类实现，封装了2种窗口模式：
* （1）正常模式，不附加阴影的窗口：可以通过XML配置文件<Window>如下配置（举例）：
*              shadow_attached="false" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
*              上述配置等同于：layered_window="false"，不设置shadow_attached属性
* （2）附加阴影窗口模式，需要使用层窗口(带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
*              shadow_attached="true" layered_window="true" ，alpha为可选，设置窗口透明度，如果不设置默认为255；
*              上述配置等同于：layered_window="true"，不设置shadow_attached属性
*/
class UILIB_API WindowImplBase : public Window
{
    typedef Window BaseClass;
public:
    WindowImplBase();
    virtual ~WindowImplBase() override;

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录, 为相对路径
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() override;

public:
    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const DString& strClass) override;

protected:
    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 完成界面布局的初始化，各个控件的位置大小等布局信息完成初始化，供子类在界面启动后调整界面布局等操作
    */
    virtual void OnInitLayout() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnPreCloseWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并销毁该窗口对象）
    */
    virtual void OnFinalMessage() override;

protected:
    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullscreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullscreen() override;

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 窗口的DPI缩放比发生变化，更新控件大小和布局(供子类使用)
    * @param [in] nOldScaleFactor 旧的DPI缩放百分比
    * @param [in] nNewScaleFactor 新的DPI缩放百分比，与Dpi().GetDisplayScaleFactor()的值一致，该值可能与nOldScaleFactor相同
    */
    virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor) override;

protected:
    /** 进入最大化状态
    */
    virtual void OnWindowMaximized();

    /** 从最大化还原
    */
    virtual void OnWindowRestored();

    /** 进入最小化状态
    */
    virtual void OnWindowMinimized();

protected:
    /** 选择语言
    * @param [in] pBtnSelectLanguage 选择语言按钮的接口
    */
    virtual void OnSelectLanguage(Control* pBtnSelectLanguage);

    /** 选择主题
    * @param [in] pBtnSelectTheme 选择主题按钮的接口
    */
    virtual void OnSelectTheme(Control* pBtnSelectTheme);

protected:
    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 判断是否含有最大化和最小化按钮
    * @param [out] bMinimizeBox 返回true表示含有最小化按钮
    * @param [out] bMaximizeBox 返回true表示含有最大化按钮
    */
    virtual bool HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const override;

    /** 判断一个点是否在最大化或者还原按钮上
    */
    virtual bool IsPtInMaximizeRestoreButton(const UiPoint& pt) const override;

    /** 正在初始化窗口数据（内部函数）
    */
    virtual void PreInitWindow() override;

    /** 语言切换事件（当前语言可通过GlobalManager获取）
    * @return 返回false表示不再触发窗口的kWindowLanguageChangedMsg事件, 否则触发该事件
    */
    virtual bool OnLanguageChanged() override;

    /** 主题切换事件（当前主题可通过ThemeManager获取）
    * @return 返回false表示不再触发窗口的kWindowThemeChangedMsg事件, 否则触发该事件
    */
    virtual bool OnThemeChanged() override;

protected:
    /** 获取窗口标题栏控件
    */
    Control* GetWindowTitleBar() const;

    /** 获取窗口最大化按钮
    */
    Control* GetBtnWindowMax() const;

    /** 获取窗口还原按钮
    */
    Control* GetBtnWindowRestore() const;

    /** 获取窗口最小化按钮
    */
    Control* GetBtnWindowMin() const;

    /** 获取窗口关闭按钮
    */
    Control* GetBtnWindowClose() const;

    /** 获取窗口全屏按钮
    */
    Control* GetBtnWindowFullscreen() const;

    /** 获取选择语言按钮
    */
    Control* GetBtnSelectLanguage() const;

    /** 获取选择主题按钮
    */
    Control* GetBtnSelectTheme() const;

private:
    /** 获取窗口按钮
    * @param [in] newCtrlName 控件最新的名称
    * @param [in] oldCtrlName 控件旧的名称（为了保持兼容性）
    */
    Control* GetBtnWindowByName(const DString& newCtrlName, const DString& oldCtrlName) const;

private:
    /** 标题栏被双击时调用
    * @param [in] param 携带的参数
    * @return 始终返回 true
    */
    bool OnTitleBarDoubleClick(const EventArgs& param);

    /** 处理最大化/还原按钮的状态
    */
    void ProcessMaxRestoreStatus();

private:
    /** 标题栏的接口
    */
    ControlPtr m_pTitleBar;

    /** 最大化按钮的接口
    */
    ControlPtr m_pMaxButton;

    /** 最小化按钮的接口
    */
    ControlPtr m_pMinButton;

    /** 还原按钮的接口
    */
    ControlPtr m_pRestoreButton;
};
} // namespace ui

#endif // UI_UTILS_WINIMPLBASE_H_
