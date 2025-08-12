#include "DragForm.h"

DragForm::DragForm()
{
}

DragForm::~DragForm()
{
}

DString DragForm::GetSkinFolder()
{
    return _T("cef_browser");
}

DString DragForm::GetSkinFile()
{
    return _T("drag_form.xml");
}
