#ifndef EXAMPLES_FIND_FORM_H_
#define EXAMPLES_FIND_FORM_H_

#include "resource.h"

// duilib
#include "duilib/duilib.h"

class MainForm;
class FindForm : public ui::WindowImplBase
{
public:
    explicit FindForm(MainForm* pMainForm);
    ~FindForm();

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

private:
    //查找下一个
    void OnFindNext();

private:
    //查找接口
    MainForm* m_pMainForm;

    //搜索方向选项
    ui::Option* m_pDirectionOption;

    //是否区分大小写
    ui::CheckBox* m_pCaseSensitive;

    //是否全字匹配
    ui::CheckBox* m_pMatchWholeWord;

    //查找内容
    ui::RichEdit* m_pFindText;
};

#endif //EXAMPLES_FIND_FORM_H_
