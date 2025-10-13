#include "MainForm.h"

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("layout");
}

DString MainForm::GetSkinFile()
{
    return _T("layout.xml");
}

void MainForm::OnInitWindow()
{
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMsg(0L);
}
