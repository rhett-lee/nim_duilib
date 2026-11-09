#ifndef EXAMPLES_CONTROLS_COMBO_FORM_H_
#define EXAMPLES_CONTROLS_COMBO_FORM_H_

#include "duilib/duilib.h"

/** Combo 下拉选择控件演示窗口
*/
class ComboForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ComboForm();
    virtual ~ComboForm() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;

private:
    /** 填充 Combo 的下拉选项
    */
    void InitComboItems(ui::Combo* pCombo, const DString& prefix, int32_t nCount);

    /** 更新事件回显
    */
    void UpdateEventText(const DString& text);
};

#endif // EXAMPLES_CONTROLS_COMBO_FORM_H_
