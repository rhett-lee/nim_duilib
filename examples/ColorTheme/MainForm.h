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

private:
    /** 生成主题颜色的功能开关
    */
    void OnGenColorEnableStateChanged(bool bEnabled);

    /** 颜色配置参数变化，更新界面
    */
    void OnGenColorParamChanged();

    /** 显示所有颜色
    */
    void ShowAllColors();

private:
    /** 主题生成器
    */
    std::unique_ptr<ui::ThemeGenerator> m_pThemeGenerator;

    /** 当前预览主题的XML文件数据
    */
    std::string m_colorThemeXmlData;
};

#endif //EXAMPLES_MAIN_FORM_H_
