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
    __super::OnInitWindow();
}

void BasicForm::OnFinalMessage()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMsg(0L);

    //调用基类，删除自身
    __super::OnFinalMessage();
}
