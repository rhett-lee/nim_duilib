#ifndef EXAMPLES_CONTROLS_STEPPER_FORM_H_
#define EXAMPLES_CONTROLS_STEPPER_FORM_H_

#include "duilib/duilib.h"

/** Stepper 向导步骤条控件演示窗口
*/
class StepperForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    StepperForm();
    virtual ~StepperForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

private:
    /** 把当前步骤回显到标签
    * @param [in] pStepper 步骤条控件
    */
    void UpdateStepText(ui::Stepper* pStepper);
};

#endif // EXAMPLES_CONTROLS_STEPPER_FORM_H_
