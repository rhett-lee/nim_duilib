#ifndef UI_UTILS_WINIMPLBASE_H_
#define UI_UTILS_WINIMPLBASE_H_

#pragma once

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
public:
    WindowImplBase();

protected:
    //析构函数不允许外部调用，因在OnFinalMessage函数中，有delete this操作
    virtual ~WindowImplBase();

public:
    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() = 0;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() = 0;

public:
    /** 当要创建的控件不是标准的控件名称时会调用该函数
    * @param [in] strClass 控件名称
    * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
    */
    virtual Control* CreateControl(const DString& strClass);

protected:
    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口即将被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并销毁该窗口对象）
    */
    virtual void OnFinalMessage() override;

protected:
    /** 进入全屏状态
    */
    virtual void OnWindowEnterFullScreen() override;

    /** 退出全屏状态
    */
    virtual void OnWindowExitFullScreen() override;

    /** 进入最大化状态
    */
    virtual void OnWindowMaximized() override;

    /** 从最大化还原
    */
    virtual void OnWindowRestored() override;

    /** 进入最小化状态
    */
    virtual void OnWindowMinimized() override;

    /** 切换系统标题栏与自绘标题栏
    */
    virtual void OnUseSystemCaptionBarChanged() override;

    /** 窗口的DPI发生了变化
    * @param [in] nOldDPI 旧的DPI值
    * @param [in] nNewDPI 新的DPI值
    */
    virtual void OnWindowDpiChanged(uint32_t nOldDPI, uint32_t nNewDPI) override;

private:
    /** 初始化窗口数据（内部函数）
    */
    virtual void InitWindow() override final;

private:
    /** 窗口功能按钮被点击时调用
    * @param [in] param 携带的参数
    * @return 始终返回 true
    */
    bool OnButtonClick(const EventArgs& param);

    /** 处理最大化/还原按钮的状态
    */
    void ProcessMaxRestoreStatus();
};
}

#endif // UI_UTILS_WINIMPLBASE_H_
