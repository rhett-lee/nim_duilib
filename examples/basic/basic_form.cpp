#include "basic_form.h"

const DString BasicForm::kClassName = _T("Basic");

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

DString BasicForm::GetWindowClassName() const
{
    return kClassName;
}

void BasicForm::OnInitWindow()
{
}

void BasicForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}
