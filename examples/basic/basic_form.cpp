#include "basic_form.h"

BasicForm::BasicForm()
{
}

BasicForm::~BasicForm()
{
}

DString BasicForm::GetSkinFolder()
{
    return _T("basic");
}

DString BasicForm::GetSkinFile()
{
    return _T("basic.xml");
}

void BasicForm::OnInitWindow()
{
    ui::Button* pButton = (ui::Button*)FindControl(L"test");
    pButton->AttachClick([this](const ui::EventArgs& ){
        ui::RichEdit* pEdit = (ui::RichEdit*)FindControl(L"RichEdit");
        pEdit->LineScroll(423);
        //int32_t n = pEdit->LineIndex(423);
        //pEdit->EnsureCharVisible(n);
        return true;
        });
    __super::OnInitWindow();

}
