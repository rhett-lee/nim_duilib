#ifndef EXAMPLES_CONTROLS_BUTTON_CHECKBOX_FORM_H_
#define EXAMPLES_CONTROLS_BUTTON_CHECKBOX_FORM_H_

#include "duilib/duilib.h"

/** Button / CheckBox / Option 控件演示窗口
*/
class ButtonCheckBoxForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ButtonCheckBoxForm();
    virtual ~ButtonCheckBoxForm() override;

    /** 加载资源文件的目录，位于 bin/resources/ 目录下
    */
    virtual DString GetSkinFolder() override;

    /** 窗口使用的 XML 布局文件
    */
    virtual DString GetSkinFile() override;

    /** 窗口初始化
    */
    virtual void OnInitWindow() override;

private:
    /** 更新三态复选框的状态文字
    */
    void UpdateTristateText();

    /** 更新勾选/取消勾选事件的回显文字
    */
    void UpdateCheckText(const ui::CheckBox* pCheckBox, bool bChecked);

    /** 更新单选项被选中的回显文字
    */
    void UpdateOptionText(const ui::Option* pOption);

private:
    /** 按钮点击次数
    */
    int m_nClickCount = 0;
};

#endif // EXAMPLES_CONTROLS_BUTTON_CHECKBOX_FORM_H_
