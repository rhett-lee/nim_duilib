#ifndef EXAMPLES_RENDER_FORM_H_
#define EXAMPLES_RENDER_FORM_H_

// duilib
#include "duilib/duilib.h"

class RenderForm : public ui::WindowImplBase
{
public:
    RenderForm();
    virtual ~RenderForm() override;

    /** 资源相关接口
     * GetSkinFolder 接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile 接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 当窗口已经被关闭时调用此函数，供子类中做一些收尾工作
    */
    virtual void OnCloseWindow() override;

    /**@brief 当要创建的控件不是标准的控件名称时会调用该函数
     * @param[in] strClass 控件名称
     * @return 返回一个自定义控件指针，一般情况下根据 strClass 参数创建自定义的控件
     */
    virtual ui::Control* CreateControl(const DString& strClass) override;

private:
    //测试PropertyGrid控件
    void TestPropertyGrid();

    //获取PropertyGrid控件的结果
    void CheckPropertyGridResult();
};

#endif //EXAMPLES_RENDER_FORM_H_
