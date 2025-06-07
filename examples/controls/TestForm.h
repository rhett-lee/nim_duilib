#ifndef EXAMPLES_TEST_FORM_H_
#define EXAMPLES_TEST_FORM_H_

// duilib
#include "duilib/duilib.h"

class TestForm : public ui::WindowImplBase
{
public:
    TestForm();
    virtual ~TestForm() override;

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
    /** 定时器函数
    */
    void OnTimer();

private:
    /** 进度值
    */
    double m_nProgressValue;
};

#endif //EXAMPLES_TEST_FORM_H_
