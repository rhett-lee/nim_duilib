#ifndef UI_UTILS_SHADOW_WND_H__
#define UI_UTILS_SHADOW_WND_H__

#include "duilib/Utils/WinImplBase.h"

namespace ui {

/** 使用附加阴影窗口的基类，实现了创建窗口并附加的功能，提供没有 WS_EX_LAYERED 属性的窗口继承
 * 该类实现，封装了3种窗口模式：
 * （1）正常模式，不附加阴影的窗口：可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="false" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
 * （2）附加阴影窗口模式，需要使用层窗口(带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="true" layered_window="true" ，alpha为可选配置，设置窗口透明度，如果不设置，默认为255。
 * （3）附加阴影窗口模式，不需要使用层窗口(不带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="true" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
 *             上述配置等同于：layered_window="false"，不设置shadow_attached属性。
 */
class ShadowWndBase;
class ShadowWnd: public WindowImplBase
{
    typedef WindowImplBase BaseClass;
public:
    ShadowWnd();
    virtual ~ShadowWnd() override;

protected:
    /** 附加窗口阴影
    */
    virtual Box* AttachShadow(Box* pRoot) override;

private:
    /** 判断是否需要创建透明窗口的跟随阴影
    */
    bool NeedCreateShadowWnd() const;

    /** 初始化阴影
    */
    void InitShadow();

private:
    //阴影窗口，透明的
    ShadowWndBase* m_pShadowWnd;
};
} // namespace ui

#endif // UI_UTILS_SHADOW_WND_H__
