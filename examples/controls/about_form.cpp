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
            ::ShellExecuteW(NULL, L"open", ui::StringUtil::TToUTF16(link->GetText()).c_str(), NULL, NULL, SW_SHOWDEFAULT);
            return true;
        });
    }
}

