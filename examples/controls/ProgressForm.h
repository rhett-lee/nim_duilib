#ifndef EXAMPLES_PROGRESS_FORM_H_
#define EXAMPLES_PROGRESS_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 进度条控件测试窗口（独立演示：水平/垂直/环形/滑块/浮点动画/百分比/marquee/indeterminate/事件）
*/
class ProgressForm : public ui::WindowImplBase
{
public:
    ProgressForm();
    virtual ~ProgressForm() override;

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
    /** 定时器函数：统一驱动各进度控件递增演示
    */
    void OnTimer();

private:
    /** 进度值
    */
    double m_nProgressValue;
};

#endif //EXAMPLES_PROGRESS_FORM_H_
