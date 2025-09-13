#ifndef EXAMPLES_ANIMATION_FORM_H_
#define EXAMPLES_ANIMATION_FORM_H_

// duilib
#include "duilib/duilib.h"

class AnimationForm : public ui::WindowImplBase
{
public:
    AnimationForm();
    virtual ~AnimationForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    //动画播放相关事件处理
    void OnAnimationEvents(ui::EventType eventType, const ui::ImageAnimationStatus& status);

private:
    //是否正在播放中
    bool m_bImagePlaying;
};

#endif //EXAMPLES_ANIMATION_FORM_H_
