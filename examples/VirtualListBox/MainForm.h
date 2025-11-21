#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

#include "DataProvider.h"

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
    bool OnClicked(const ui::EventArgs& args);

private:
    ui::VirtualListBox* m_pTileList;
    DataProvider* m_pDataProvider;

private:
    ui::RichEdit* m_pEditColumn;
    ui::RichEdit* m_pEditTotal;
    ui::Option* m_pOptionColumnFix;
    ui::RichEdit* m_pEditUpdate;
    ui::RichEdit* m_pEditTaskName;
    ui::RichEdit* m_pEditDelete;
    ui::RichEdit* m_pEditChildMarginX;
    ui::RichEdit* m_pEditChildMarginY;
};

#endif //EXAMPLES_MAIN_FORM_H_
