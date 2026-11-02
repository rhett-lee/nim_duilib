#ifndef EXAMPLES_MAIN_FORM_H_
#define EXAMPLES_MAIN_FORM_H_

// duilib
#include "duilib/duilib.h"

/** 应用程序的主窗口实现
*/
class MainForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;

public:
    MainForm();
    virtual ~MainForm() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤目录
    * @return 子类需实现并返回窗口皮肤目录
    */
    virtual DString GetSkinFolder() override;

    /**  创建窗口时被调用，由子类实现用以获取窗口皮肤 XML 描述文件
    * @return 子类需实现并返回窗口皮肤 XML 描述文件
    *         返回的内容，可以是XML文件内容（以字符'<'为开始的字符串），
    *         或者是文件路径（不是以'<'字符开始的字符串），文件要在GetSkinFolder()路径中能够找到
    */
    virtual DString GetSkinFile() override;

    /** 当窗口创建完成以后调用此函数，供子类中做一些初始化的工作
    */
    virtual void OnInitWindow() override;

    /** 语言切换后调用：XML 里通过 text_id 指定的文本由框架自动刷新，
    *   而代码中动态设置的文本需要在此处重新获取一次
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 刷新"当前语言"区域：语言显示名称与语言文件名
    */
    void UpdateLanguageInfo();

private:
    /** 语言显示名称
    */
    ui::Label* m_pLangDisplayName;

    /** 语言文件名
    */
    ui::Label* m_pLangFileName;
};

#endif //EXAMPLES_MAIN_FORM_H_
