#ifndef EXAMPLES_NAVIGATION_FORM_H_
#define EXAMPLES_NAVIGATION_FORM_H_

// duilib
#include "duilib/duilib.h"

/** NavigationView 侧边栏导航演示窗口
*/
class NavigationForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    NavigationForm();
    virtual ~NavigationForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;
};

#endif //EXAMPLES_NAVIGATION_FORM_H_
