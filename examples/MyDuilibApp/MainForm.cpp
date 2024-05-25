//MainForm.cpp
#include "MainForm.h"

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm()
{
}


MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
    return L"my_duilib_app";
}

std::wstring MainForm::GetSkinFile()
{
    return L"MyDuilibForm.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化

}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}
