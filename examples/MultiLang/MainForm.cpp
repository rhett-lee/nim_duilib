#include "MainForm.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("MultiLang");
}

DString MainForm::GetSkinFile()
{
    return _T("MultiLang.xml");
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化
}
