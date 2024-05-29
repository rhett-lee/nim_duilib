#include "basic_form.h"

const std::wstring BasicForm::kClassName = _T("Basic");

BasicForm::BasicForm()
{
}

BasicForm::~BasicForm()
{
}

std::wstring BasicForm::GetSkinFolder()
{
    return _T("basic");
}

std::wstring BasicForm::GetSkinFile()
{
    return _T("basic.xml");
}

std::wstring BasicForm::GetWindowClassName() const
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
