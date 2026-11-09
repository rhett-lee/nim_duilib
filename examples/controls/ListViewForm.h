#ifndef EXAMPLES_CONTROLS_LIST_VIEW_FORM_H_
#define EXAMPLES_CONTROLS_LIST_VIEW_FORM_H_

#include "duilib/duilib.h"

/** ListBox / TreeView 列表与树演示窗口
*/
class ListViewForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ListViewForm();
    virtual ~ListViewForm() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
    virtual bool OnLanguageChanged() override;

private:
    /** 更新事件回显
    */
    void UpdateEventText(const DString& text);
};

#endif // EXAMPLES_CONTROLS_LIST_VIEW_FORM_H_
