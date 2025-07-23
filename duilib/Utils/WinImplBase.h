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
    /** 窗口大小发生改变(WM_SIZE)
    * @param [in] sizeType 触发窗口大小改变的类型
    * @param [in] newWindowSize 新的窗口大小（宽度和高度）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnSizeMsg(WindowSizeType sizeType, const UiSize& newWindowSize, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullScreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() override;

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 窗口的DPI发生了变化
    * @param [in] nOldDPI 旧的DPI值
    * @param [in] nNewDPI 新的DPI值
    */
    virtual void OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI) override;

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

    /** 判断是否含有最大化和最小化按钮
    * @param [out] bMinimizeBox 返回true表示含有最小化按钮
    * @param [out] bMaximizeBox 返回true表示含有最大化按钮
    */
    virtual bool HasMinMaxBox(bool& bMinimizeBox, bool& bMaximizeBox) const override;

    /** 判断一个点是否在最大化或者还原按钮上
    */
    virtual bool IsPtInMaximizeRestoreButton(const UiPoint& pt) const override;

protected:
    /** 正在初始化窗口数据（内部函数）
    */
    virtual void PreInitWindow() override;

private:
    /** 窗口功能按钮被点击时调用
    * @param [in] param 携带的参数
    * @return 始终返回 true
    */
    bool OnButtonClick(const EventArgs& param);

    /** 标题栏被双击时调用
    * @param [in] param 携带的参数
    * @return 始终返回 true
    */
    bool OnTitleBarDoubleClick(const EventArgs& param);

    /** 处理最大化/还原按钮的状态
    */
    void ProcessMaxRestoreStatus();

private:
    /** 最大化按钮的接口
    */
    Control* m_pMaxButton;
    std::weak_ptr<WeakFlag> m_maxButtonFlag;

    /** 最小化按钮的接口
    */
    Control* m_pMinButton;
    std::weak_ptr<WeakFlag> m_minButtonFlag;

    /** 还原按钮的接口
    */
    Control* m_pRestoreButton;
    std::weak_ptr<WeakFlag> m_restoreButtonFlag;
};
}

#endif // UI_UTILS_WINIMPLBASE_H_
