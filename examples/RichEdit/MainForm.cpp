#include "MainForm.h"
#include "MainThread.h"
#include <ShellApi.h>

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm()
{
}

MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
	return L"rich_edit";
}

std::wstring MainForm::GetSkinFile()
{
	return L"rich_edit.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
	return kClassName;
}

void MainForm::OnInitWindow()
{
	ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"test_url"));
	if (pRichEdit != nullptr) {
		pRichEdit->AttachCustomLinkClick([this, pRichEdit](const ui::EventArgs& args) {
				//点击了超级链接
				if (args.pSender == pRichEdit) {
					const wchar_t* pUrl = (const wchar_t*)args.wParam;
					if (pUrl != nullptr) {
						::ShellExecute(GetHWND(), L"open", pUrl, NULL, NULL, SW_SHOWNORMAL);
					}
				}
				return true;
			});
	}
}

LRESULT MainForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}
