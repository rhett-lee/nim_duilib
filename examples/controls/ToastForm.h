#ifndef EXAMPLES_TOAST_FORM_H_
#define EXAMPLES_TOAST_FORM_H_

// duilib
#include "duilib/duilib.h"

/** Toast 通知控件功能演示窗口
*/
class ToastForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ToastForm();
    virtual ~ToastForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后的初始化工作
    */
    virtual void OnInitWindow() override;
};

#endif // EXAMPLES_TOAST_FORM_H_
