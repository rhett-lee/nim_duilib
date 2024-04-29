#include "RichEditFindReplace.h"
#include <commdlg.h>

RichEditFindReplace::RichEditFindReplace():
    m_pRichEdit(nullptr),
	m_bFindDown(true),
	m_bMatchCase(true),
	m_bMatchWholeWord(false),
	m_bFirstSearch(true),
	m_nInitialSearchPos(0),
	m_hOldCursor(nullptr)
{
}

void RichEditFindReplace::SetRichEdit(ui::RichEdit* pRichEdit)
{
    m_pRichEdit = pRichEdit;
}

bool RichEditFindReplace::FindRichText(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	bool bChanged = false;
	if (m_sFindNext != findText) {
		bChanged = true;
	}
	else if (m_bFindDown != bFindDown) {
		bChanged = true;
	}
	else if (m_bMatchCase != bMatchCase) {
		bChanged = true;
	}
	else if (m_bMatchWholeWord != bMatchWholeWord) {
		bChanged = true;
	}

	m_sFindNext = findText;
	m_sReplaceWith.clear();
	m_bFindDown = bFindDown;
	m_bMatchCase = bMatchCase;
	m_bMatchWholeWord = bMatchWholeWord;

	if (bChanged) {
		m_bFirstSearch = true;
		m_nInitialSearchPos = 0;
	}	
	if (FindTextSimple(findText, bFindDown, bMatchCase, bMatchWholeWord)) {
		//找到
		AdjustDialogPosition(hWndDialog);
		return true;
	}
	else {
		//没找到
		TextNotFound(findText);
		return false;
	}
}

bool RichEditFindReplace::FindNext()
{
	if (m_sFindNext.empty()) {
		return false;
	}
	if (FindTextSimple(m_sFindNext, m_bFindDown, m_bMatchCase, m_bMatchWholeWord)) {
		//找到
		return true;
	}
	else {
		//没找到
		TextNotFound(m_sFindNext);
		return false;
	}
}

bool RichEditFindReplace::ReplaceRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	m_sFindNext = findText;
	m_sReplaceWith = replaceText;
	m_bFindDown = bFindDown;
	m_bMatchCase = bMatchCase;
	m_bMatchWholeWord = bMatchWholeWord;
	bool bChanged = false;
	if (m_sFindNext != findText) {
		bChanged = true;
	}
	else if (m_bFindDown != bFindDown) {
		bChanged = true;
	}
	else if (m_bMatchCase != bMatchCase) {
		bChanged = true;
	}
	else if (m_bMatchWholeWord != bMatchWholeWord) {
		bChanged = true;
	}
	if (bChanged) {
		m_bFirstSearch = true;
		m_nInitialSearchPos = 0;
	}
	if (m_pRichEdit == nullptr) {
		return false;
	}
	if (m_sFindNext.empty()) {
		return false;
	}

	bool bReplaced = false;
	if (SameAsSelected(m_sFindNext, m_bMatchCase)) {
		m_pRichEdit->ReplaceSel(m_sReplaceWith, false);
		bReplaced = true;
	}

	if (!FindTextSimple(m_sFindNext, m_bMatchCase, m_bMatchWholeWord, m_bFindDown)) {
		TextNotFound(m_sFindNext);
	}
	else {
		AdjustDialogPosition(hWndDialog);
	}
	return bReplaced;
}

bool RichEditFindReplace::ReplaceAllRichText(const std::wstring& findText, const std::wstring& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, HWND hWndDialog)
{
	m_sFindNext = findText;
	m_sReplaceWith = replaceText;
	m_bFindDown = bFindDown;
	m_bMatchCase = bMatchCase;
	m_bMatchWholeWord = bMatchWholeWord;
	bool bChanged = false;
	if (m_sFindNext != findText) {
		bChanged = true;
	}
	else if (m_bFindDown != bFindDown) {
		bChanged = true;
	}
	else if (m_bMatchCase != bMatchCase) {
		bChanged = true;
	}
	else if (m_bMatchWholeWord != bMatchWholeWord) {
		bChanged = true;
	}
	if (bChanged) {
		m_bFirstSearch = true;
		m_nInitialSearchPos = 0;
	}
	if (m_pRichEdit == nullptr) {
		return false;
	}
	if (m_sFindNext.empty()) {
		return false;
	}

	if (!SameAsSelected(m_sFindNext, m_bMatchCase))	{
		if (!FindTextSimple(m_sFindNext, m_bFindDown, m_bMatchCase, m_bMatchWholeWord)) {
			TextNotFound(m_sFindNext);
			return false;
		}
	}

	OnReplaceAllCoreBegin();
	int replaceCount = 0;
	do {
		++replaceCount;
		m_pRichEdit->ReplaceSel(m_sReplaceWith, false);
	} while (FindTextSimple(m_sFindNext, m_bFindDown, m_bMatchCase, m_bMatchWholeWord));

	OnReplaceAllCoreEnd(replaceCount);
	return replaceCount > 0;
}

bool RichEditFindReplace::FindTextSimple(const std::wstring& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	if (m_sFindNext.empty()) {
		return false;
	}

	FINDTEXTEX ft = {0, };
	m_pRichEdit->GetSel(ft.chrg);
	if (m_bFirstSearch) {
		if (bFindDown) {
			m_nInitialSearchPos = ft.chrg.cpMin;
		}
		else {
			m_nInitialSearchPos = ft.chrg.cpMax;
		}
		m_bFirstSearch = false;
	}

	ft.lpstrText = (LPTSTR)findText.c_str();

	if (ft.chrg.cpMin != ft.chrg.cpMax)	{
		if (bFindDown) {
			ft.chrg.cpMin++;
		}
		else {
			ft.chrg.cpMin = std::max(ft.chrg.cpMin, 0L);
		}
	}

	DWORD dwFlags = bMatchCase ? FR_MATCHCASE : 0;
	dwFlags |= bMatchWholeWord ? FR_WHOLEWORD : 0;

	ft.chrg.cpMax = m_pRichEdit->GetTextLength() + m_nInitialSearchPos;

	if (bFindDown) {
		if (m_nInitialSearchPos >= 0) {
			ft.chrg.cpMax = m_pRichEdit->GetTextLength();
		}
		dwFlags |= FR_DOWN;
		ASSERT(ft.chrg.cpMax >= ft.chrg.cpMin);
	}
	else {
		if (m_nInitialSearchPos >= 0) {
			ft.chrg.cpMax = 0;
		}
		dwFlags &= ~FR_DOWN;
		ASSERT(ft.chrg.cpMax <= ft.chrg.cpMin);
	}

	bool bRet = false;
	if (FindAndSelect(dwFlags, ft) != -1) {
		bRet = true;
	}
	else if (m_nInitialSearchPos > 0) {
		if (bFindDown) {
			ft.chrg.cpMin = 0;
			ft.chrg.cpMax = m_nInitialSearchPos;
		}
		else {
			ft.chrg.cpMin = m_pRichEdit->GetTextLength();
			ft.chrg.cpMax = m_nInitialSearchPos;
		}
		m_nInitialSearchPos = m_nInitialSearchPos - m_pRichEdit->GetTextLength();
		bRet = (FindAndSelect(dwFlags, ft) != -1) ? true : false;
	}
	return bRet;
}

long RichEditFindReplace::FindAndSelect(DWORD dwFlags, FINDTEXTEX& ft)
{
	if (m_pRichEdit == nullptr) {
		return -1;
	}
	LONG index = m_pRichEdit->FindRichText(dwFlags, ft);
	if (index != -1) {
		//查找到了内容，选择所查到的内容
		m_pRichEdit->SetSel(ft.chrgText);
	}
	return index;
}

bool RichEditFindReplace::SameAsSelected(const std::wstring& replaceText, BOOL bMatchCase)
{
	if (m_pRichEdit == nullptr) {
		return false;
	}
	std::wstring selectedText = m_pRichEdit->GetSelText();
	if (bMatchCase) {
		return selectedText == replaceText;
	}
	else {
		return ui::StringHelper::IsEqualNoCase(selectedText, replaceText);
	}
}

void RichEditFindReplace::TextNotFound(const std::wstring& findText)
{
	m_bFirstSearch = true;
	m_nInitialSearchPos = 0;
	OnTextNotFound(findText);
}

void RichEditFindReplace::AdjustDialogPosition(HWND hWndDialog)
{
	if (!::IsWindow(hWndDialog)) {
		return;
	}
	if (m_pRichEdit == nullptr) {
		return;
	}

	long nStartChar = 0;
	long nEndChar = 0;
	m_pRichEdit->GetSel(nStartChar, nEndChar);
	ui::UiPoint pt = m_pRichEdit->PosFromChar(nStartChar);
	m_pRichEdit->ClientToScreen(pt);
	RECT rect = {};
	::GetWindowRect(hWndDialog, &rect);
	POINT point = { pt.x, pt.y };
	if (::PtInRect(&rect, point) != FALSE) {
		if (point.y > (rect.bottom - rect.top))	{
			::OffsetRect(&rect, 0, point.y - rect.bottom - 20);
		}
		else {
			int nVertExt = GetSystemMetrics(SM_CYSCREEN);
			if ((point.y + (rect.bottom - rect.top)) < nVertExt)
				::OffsetRect(&rect, 0, 40 + point.y - rect.top);
		}
		::MoveWindow(hWndDialog, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	}
}

void RichEditFindReplace::OnTextNotFound(const std::wstring& findText)
{
}

void RichEditFindReplace::OnReplaceAllCoreBegin()
{
	m_hOldCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	if (m_pRichEdit != nullptr) {
		m_pRichEdit->HideSelection(true, false);
	}
}

void RichEditFindReplace::OnReplaceAllCoreEnd(int /*replaceCount*/)
{
	if (m_pRichEdit != nullptr) {
		m_pRichEdit->HideSelection(false, false);
	}
	if (m_hOldCursor != nullptr) {
		::SetCursor(m_hOldCursor);
		m_hOldCursor = nullptr;
	}
}