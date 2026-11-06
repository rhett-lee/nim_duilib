#ifndef EXAMPLES_RICHEDIT_REPLACE_FORM_H_
#define EXAMPLES_RICHEDIT_REPLACE_FORM_H_

#include "FindForm.h"

class ReplaceForm : public FindReplaceFormBase
{
    typedef FindReplaceFormBase BaseClass;
public:
    explicit ReplaceForm(MainForm* pMainForm);
    virtual ~ReplaceForm() override;

    virtual DString GetSkinFile() override;

    virtual void OnInitWindow() override;

private:
    void OnReplace();
    void OnReplaceAll();

private:
    ui::RichEdit* m_pReplaceText;
};

#endif //EXAMPLES_RICHEDIT_REPLACE_FORM_H_
