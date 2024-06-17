#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "resource.h"

class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm();

    /**
     * 一下三个接口是必须要覆写的接口，父类会调用这三个接口来构建窗口
     * GetSkinFolder        接口设置你要绘制的窗口皮肤资源路径
     * GetSkinFile            接口设置你要绘制的窗口的 xml 描述文件
     */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

private:
    /** 填充数据
    */
    void InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId);

    /** 执行一些功能测试
    */
    void RunListCtrlTest();

    /** 控制该列
    */
    void OnColumnChanged(size_t nColumnId);
};

#endif //EXAMPLES_MAIN_FORM_H_
