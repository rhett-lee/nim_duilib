#ifndef EXAMPLES_MARQUEE_FORM_H_
#define EXAMPLES_MARQUEE_FORM_H_

// duilib
#include "duilib/duilib.h"

/** Marquee 文本跑马灯演示窗口
 *  演示横向（向左/向右）与纵向（向上/向下）滚动、自定义速度、
 *  鼠标悬停暂停、暂停/恢复、文本动态更新、文本未超出时静态显示等全部能力。
*/
class MarqueeForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MarqueeForm();
    virtual ~MarqueeForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 演示暂停/恢复：切换所有跑马灯的暂停状态
    */
    void TogglePause();

    /** 演示文本动态更新：轮换更新横向跑马灯的文本内容
    */
    void UpdateText();
};

#endif //EXAMPLES_MARQUEE_FORM_H_
