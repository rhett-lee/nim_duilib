#ifndef EXAMPLES_CONTROLS_RICH_EDIT_FORM_H_
#define EXAMPLES_CONTROLS_RICH_EDIT_FORM_H_

#include "duilib/duilib.h"

/** RichEdit 文本输入控件演示窗口
*/
class RichEditForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    RichEditForm();
    virtual ~RichEditForm() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
    virtual bool OnLanguageChanged() override;

private:
    /** 更新事件回显
    */
    void UpdateEventText(const DString& text);

    /** 多行编辑框的示例文本（用于语言切换时判断内容是否被用户修改） */
    DString m_strMultiSample;

    /** RichEdit2 的示例文本（用于语言切换时判断内容是否被用户修改） */
    DString m_strRichEdit2Sample;
};

#endif // EXAMPLES_CONTROLS_RICH_EDIT_FORM_H_
