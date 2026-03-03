#include "MainForm.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("basic");
}

DString MainForm::GetSkinFile()
{
    return _T("basic.xml");
}

void MainForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
}
