#include "ReplaceForm.h"
#include "MainForm.h"

const LPCTSTR ReplaceForm::kClassName = L"FindForm";

ReplaceForm::ReplaceForm(MainForm* pMainForm):
	m_pMainForm(pMainForm),
	m_pDirectionOption(nullptr),
	m_pCaseSensitive(nullptr),
	m_pMatchWholeWord(nullptr),
	m_pFindText(nullptr),
	m_pReplaceText(nullptr)
{
}


ReplaceForm::~ReplaceForm()
{
}

std::wstring ReplaceForm::GetSkinFolder()
{
	return L"rich_edit";
}

std::wstring ReplaceForm::GetSkinFile()
{
	return L"replace.xml";
}

std::wstring ReplaceForm::GetWindowClassName() const
{
	return kClassName;
}

void ReplaceForm::OnInitWindow()
{
	m_pFindText = dynamic_cast<ui::RichEdit*>(FindControl(L"btn_find_text"));
	m_pReplaceText = dynamic_cast<ui::RichEdit*>(FindControl(L"btn_replace_text"));
	m_pDirectionOption = dynamic_cast<ui::Option*>(FindControl(L"option_direction_down"));
	m_pCaseSensitive = dynamic_cast<ui::CheckBox*>(FindControl(L"check_box_case_sensitive"));
	m_pMatchWholeWord = dynamic_cast<ui::CheckBox*>(FindControl(L"check_box_match_whole_word"));
	ASSERT(m_pFindText != nullptr);
	ASSERT(m_pReplaceText != nullptr);
	ASSERT(m_pDirectionOption != nullptr);
	ASSERT(m_pCaseSensitive != nullptr);
	ASSERT(m_pMatchWholeWord != nullptr);

	if (m_pFindText != nullptr) {
		m_pFindText->SetFocus();

		ui::RichEdit* pRichEdit = nullptr;
		if (m_pMainForm != nullptr) {
			pRichEdit = m_pMainForm->GetRichEdit();
		}
		std::wstring selText;
		if (pRichEdit != nullptr) {
			selText = pRichEdit->GetSelText();
		}
		if (!selText.empty()) {
			m_pFindText->SetText(selText);
			m_pFindText->SetSelAll();
		}
	}

	ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_cancel"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					CloseWnd();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_find_next"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					OnFindNext();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_replace"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					OnReplace();
				}
				return true;
			});
	}
	pButton = dynamic_cast<ui::Button*>(FindControl(L"btn_replace_all"));
	if (pButton != nullptr) {
		pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
				if (args.pSender == pButton) {
					OnReplaceAll();
				}
				return true;
			});
	}
}

void ReplaceForm::OnFindNext()
{
	if (m_pFindText == nullptr) {
		return;
	}
	std::wstring findText = m_pFindText->GetText();
	if (findText.empty()) {
		return;
	}

	bool bFindDown = true;
	if (m_pDirectionOption != nullptr) {
		bFindDown = m_pDirectionOption->IsSelected();
	}

	bool bMatchCase = true;
	if (m_pCaseSensitive != nullptr) {
		bMatchCase = m_pCaseSensitive->IsSelected();
	}

	bool bMatchWholeWord = false;
	if (m_pMatchWholeWord != nullptr) {
		bMatchWholeWord = m_pMatchWholeWord->IsSelected();
	}
	if (m_pMainForm != nullptr) {
		m_pMainForm->FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, GetHWND());
	}
}

void ReplaceForm::OnReplace()
{
	if (m_pFindText == nullptr) {
		return;
	}
	std::wstring findText = m_pFindText->GetText();
	if (findText.empty()) {
		return;
	}
	if (m_pReplaceText == nullptr) {
		return;
	}
	std::wstring replaceText = m_pReplaceText->GetText();
	if (replaceText.empty()) {
		return;
	}

	bool bFindDown = true;
	if (m_pDirectionOption != nullptr) {
		bFindDown = m_pDirectionOption->IsSelected();
	}

	bool bMatchCase = true;
	if (m_pCaseSensitive != nullptr) {
		bMatchCase = m_pCaseSensitive->IsSelected();
	}

	bool bMatchWholeWord = false;
	if (m_pMatchWholeWord != nullptr) {
		bMatchWholeWord = m_pMatchWholeWord->IsSelected();
	}
	if (m_pMainForm != nullptr) {
		m_pMainForm->ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, GetHWND());
	}
}

void ReplaceForm::OnReplaceAll()
{
	if (m_pFindText == nullptr) {
		return;
	}
	std::wstring findText = m_pFindText->GetText();
	if (findText.empty()) {
		return;
	}
	if (m_pReplaceText == nullptr) {
		return;
	}
	std::wstring replaceText = m_pReplaceText->GetText();
	if (replaceText.empty()) {
		return;
	}

	bool bFindDown = true;
	if (m_pDirectionOption != nullptr) {
		bFindDown = m_pDirectionOption->IsSelected();
	}

	bool bMatchCase = true;
	if (m_pCaseSensitive != nullptr) {
		bMatchCase = m_pCaseSensitive->IsSelected();
	}

	bool bMatchWholeWord = false;
	if (m_pMatchWholeWord != nullptr) {
		bMatchWholeWord = m_pMatchWholeWord->IsSelected();
	}
	if (m_pMainForm != nullptr) {
		m_pMainForm->ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, GetHWND());
	}
}
