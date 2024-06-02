#ifndef EXAMPLES_ABOUT_FORM_H_
#define EXAMPLES_ABOUT_FORM_H_

// duilib
#include "duilib/duilib.h"

class AboutForm : public ui::WindowImplBase
{
public:
    AboutForm();
    ~AboutForm();

    /**
     * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     * GetWindowClassName    接口设置窗口唯一的类名称
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual DString GetWindowClassName() const override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /**
     * 标识窗口 class name
     */
    static const DString kClassName;
};

#endif //EXAMPLES_ABOUT_FORM_H_
