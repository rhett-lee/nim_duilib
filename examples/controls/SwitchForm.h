#ifndef EXAMPLES_CONTROLS_SWITCH_FORM_H_
#define EXAMPLES_CONTROLS_SWITCH_FORM_H_

#include "duilib/duilib.h"

/** Switch 开关控件演示窗口
*/
class SwitchForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    SwitchForm();
    virtual ~SwitchForm() override;

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
    /** 更新选中/取消选中事件的回显文字
    */
    void UpdateEventText(bool bSelected);
};

#endif // EXAMPLES_CONTROLS_SWITCH_FORM_H_
