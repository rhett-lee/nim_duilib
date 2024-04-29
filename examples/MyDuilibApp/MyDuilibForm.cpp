//MyDuilibForm.cpp
#include "MyDuilibForm.h"

const std::wstring MyDuilibForm::kClassName = L"MyDuilibForm";

MyDuilibForm::MyDuilibForm()
{
}


MyDuilibForm::~MyDuilibForm()
{
}

std::wstring MyDuilibForm::GetSkinFolder()
{
	return L"my_duilib_app";
}

std::wstring MyDuilibForm::GetSkinFile()
{
	return L"MyDuilibForm.xml";
}

std::wstring MyDuilibForm::GetWindowClassName() const
{
	return kClassName;
}

void MyDuilibForm::OnInitWindow()
{
	//窗口初始化完成，可以进行本Form的初始化

}

LRESULT MyDuilibForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	//窗口关闭时，终止主线程的消息循环，从而退出程序
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
