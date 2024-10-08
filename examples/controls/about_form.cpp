#include "about_form.h"

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
    if (link != nullptr) {
        link->AttachButtonUp([link](const ui::EventArgs& args) {
            ui::SystemUtil::OpenUrl(link->GetText());
            return true;
        });
    }
}

