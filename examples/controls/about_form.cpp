#include "about_form.h"
#include <shellapi.h>

const LPCTSTR AboutForm::kClassName = _T("About");

AboutForm::AboutForm()
{
}


AboutForm::~AboutForm()
{
}

std::wstring AboutForm::GetSkinFolder()
{
    return _T("controls");
}

std::wstring AboutForm::GetSkinFile()
{
    return _T("about.xml");
}

std::wstring AboutForm::GetWindowClassName() const
{
    return kClassName;
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

