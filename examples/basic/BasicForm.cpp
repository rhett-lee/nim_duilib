#include "BasicForm.h"

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
    BaseClass::OnInitWindow();
    ui::Control* p = FindControl(_T("test"));
    p->AttachImageLoad([](const ui::EventArgs& args) {
        ui::ImageDecodeResult* pImageDecodeResult = (ui::ImageDecodeResult*)args.wParam;
        return true;
        });

    p->AttachImageDecode([](const ui::EventArgs& args) {
        ui::ImageDecodeResult* pImageDecodeResult = (ui::ImageDecodeResult*)args.wParam;
        return true;
        });
}
