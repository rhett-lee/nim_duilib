#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

class MainForm : public ui::WindowImplBase
{
public:
    MainForm();
    virtual ~MainForm() override;

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
    /** 填充数据
    */
    void InsertItemData(int32_t nRows, int32_t nColumns, int32_t nImageId);

    /** 执行一些功能测试
    */
    void RunListCtrlTest();

    /** 控制该列
    */
    void OnColumnChanged(size_t nColumnId);

    /** 测试Loading功能
    */
    void OnTestLoadingProgress();


private:
    /** loading状态的加载进度（模拟值，实际应用可用按情况计算进度）
    */
    float m_fLoadingPercent;
};

#endif //EXAMPLES_MAIN_FORM_H_
