#include "MainForm.h"
#include "MainThread.h"

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
	return L"list_ctrl";
}

std::wstring MainForm::GetSkinFile()
{
	return L"list_ctrl.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    PostQuitMessage(0L);
    return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

void MainForm::OnInitWindow()
{

}
