#ifndef EXAMPLES_CONTROLS_DATE_TIME_FORM_H_
#define EXAMPLES_CONTROLS_DATE_TIME_FORM_H_

#include "duilib/duilib.h"

/** DateTime 日期时间控件演示窗口
*/
class DateTimeForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    DateTimeForm();
    virtual ~DateTimeForm() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
    virtual bool OnLanguageChanged() override;

private:
    /** 绑定日期时间控件的值变化事件
    */
    void AttachValueChanged(const DString& controlName);

    /** 更新事件回显
    */
    void UpdateEventText(const DString& text);
};

#endif // EXAMPLES_CONTROLS_DATE_TIME_FORM_H_
