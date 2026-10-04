#ifndef EXAMPLES_MESSAGE_BOX_FORM_H_
#define EXAMPLES_MESSAGE_BOX_FORM_H_

// duilib
#include "duilib/duilib.h"

/** MessageBox 功能演示窗口
*/
class MessageBoxForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MessageBoxForm();
    virtual ~MessageBoxForm() override;

    /** 资源相关接口
    */
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;

    /** 窗口创建完成后的初始化工作
    */
    virtual void OnInitWindow() override;

    /** 语言切换通知：刷新需要动态拼接的文本（结果标签）
    */
    virtual bool OnLanguageChanged() override;

private:
    /** 在结果标签上显示本次消息框的返回值
    */
    void ShowResult(int32_t nResult);

private:
    //结果显示标签
    ui::Label* m_pResultLabel;

    //上次返回值（语言切换后用于重新拼接结果文本）
    int32_t m_lastResult;

    //是否已有返回值
    bool m_bHasResult;
};

#endif // EXAMPLES_MESSAGE_BOX_FORM_H_
