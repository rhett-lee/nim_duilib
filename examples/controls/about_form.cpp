#include "about_form.h"
#include <shellapi.h>

AboutForm::AboutForm()
{
}

AboutForm::~AboutForm()
{
}

DString AboutForm::GetSkinFolder()
{
    return _T("controls");
}

DString AboutForm::GetSkinFile()
{
    return _T("about.xml");
}

void AboutForm::OnInitWindow()
{
    ui::Label* link = static_cast<ui::Label*>(FindControl(_T("link")));
    if (link)
    {
        link->AttachButtonUp([link](const ui::EventArgs& args) {
            ShellExecute(NULL, _T("open"), link->GetText().c_str(), NULL, NULL, SW_SHOWDEFAULT);
            return true;
        });
    }
}

