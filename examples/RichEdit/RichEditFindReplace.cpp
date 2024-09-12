#include "RichEditFindReplace.h"

RichEditFindReplace::RichEditFindReplace():
    m_pRichEdit(nullptr),
    m_bFindDown(true),
    m_bMatchCase(true),
    m_bMatchWholeWord(false),
    m_bFirstSearch(true),
    m_nInitialSearchPos(0),
    m_nOldCursor(0)
{
}

void RichEditFindReplace::SetRichEdit(ui::RichEdit* pRichEdit)
{
    m_pRichEdit = pRichEdit;
}

bool RichEditFindReplace::FindRichText(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
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
        AdjustDialogPosition(pWndDialog);
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

bool RichEditFindReplace::ReplaceRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
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
        AdjustDialogPosition(pWndDialog);
    }
    return bReplaced;
}

bool RichEditFindReplace::ReplaceAllRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
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

    if (!SameAsSelected(m_sFindNext, m_bMatchCase))    {
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

bool RichEditFindReplace::FindTextSimple(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    if (m_sFindNext.empty()) {
        return false;
    }
    ui::FindTextParam findParam;
    int32_t nStartChar = -1;
    int32_t nEndChar = -1;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    if (m_bFirstSearch) {
        if (bFindDown) {
            m_nInitialSearchPos = nStartChar;
        }
        else {
            m_nInitialSearchPos = nEndChar;
        }
        m_bFirstSearch = false;
    }
    findParam.chrg.cpMin = nStartChar;
    findParam.chrg.cpMax = nEndChar;
    findParam.findText = findText;
    findParam.bFindDown = bFindDown;
    findParam.bMatchCase = bMatchCase;
    findParam.bMatchWholeWord = bMatchWholeWord;
    if (findParam.chrg.cpMin != findParam.chrg.cpMax)    {
        if (bFindDown) {
            findParam.chrg.cpMin++;
        }
        else {
            findParam.chrg.cpMin = std::max(findParam.chrg.cpMin, 0);
        }
    }

    findParam.chrg.cpMax = m_pRichEdit->GetTextLength() + m_nInitialSearchPos;

    if (bFindDown) {
        if (m_nInitialSearchPos >= 0) {
            findParam.chrg.cpMax = m_pRichEdit->GetTextLength();
        }
        ASSERT(findParam.chrg.cpMax >= findParam.chrg.cpMin);
    }
    else {
        if (m_nInitialSearchPos >= 0) {
            findParam.chrg.cpMax = 0;
        }
        ASSERT(findParam.chrg.cpMax <= findParam.chrg.cpMin);
    }

    bool bRet = false;
    if (FindAndSelect(findParam)) {
        bRet = true;
    }
    else if (m_nInitialSearchPos > 0) {
        if (bFindDown) {
            findParam.chrg.cpMin = 0;
            findParam.chrg.cpMax = m_nInitialSearchPos;
        }
        else {
            findParam.chrg.cpMin = m_pRichEdit->GetTextLength();
            findParam.chrg.cpMax = m_nInitialSearchPos;
        }
        m_nInitialSearchPos = m_nInitialSearchPos - m_pRichEdit->GetTextLength();
        bRet = FindAndSelect(findParam);
    }
    return bRet;
}

bool RichEditFindReplace::FindAndSelect(const ui::FindTextParam& findParam)
{
    ui::TextCharRange chrgText;
    if ((m_pRichEdit != nullptr) && m_pRichEdit->FindRichText(findParam, chrgText)) {
        //查找到了内容，选择所查到的内容
        m_pRichEdit->SetSel(chrgText.cpMin, chrgText.cpMax);
        return true;
    }
    return false;
}

bool RichEditFindReplace::SameAsSelected(const DString& replaceText, bool bMatchCase)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString selectedText = m_pRichEdit->GetSelText();
    if (bMatchCase) {
        return selectedText == replaceText;
    }
    else {
        return ui::StringUtil::IsEqualNoCase(selectedText, replaceText);
    }
}

void RichEditFindReplace::TextNotFound(const DString& findText)
{
    m_bFirstSearch = true;
    m_nInitialSearchPos = 0;
    OnTextNotFound(findText);
}

void RichEditFindReplace::AdjustDialogPosition(ui::Window* pWndDialog)
{
    if ((pWndDialog == nullptr) || !pWndDialog->IsWindow()) {
        return;
    }
    if (m_pRichEdit == nullptr) {
        return;
    }

    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    ui::UiPoint pt = m_pRichEdit->PosFromChar(nStartChar);
    m_pRichEdit->ClientToScreen(pt);
    ui::UiRect rect;
    pWndDialog->GetWindowRect(rect);
    if (rect.ContainsPt(pt)) {
        if (pt.y > (rect.bottom - rect.top))    {
            rect.Offset(0, pt.y - rect.bottom - 20);
        }
        else {
            ui::UiRect rcWork;
            pWndDialog->GetMonitorWorkRect(rcWork);
            int32_t nVertExt = rcWork.Height();
            if ((pt.y + (rect.bottom - rect.top)) < nVertExt) {
                rect.Offset(0, 40 + pt.y - rect.top);
            }
        }
        pWndDialog->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height(), true);
    }
}

void RichEditFindReplace::OnTextNotFound(const DString& findText)
{
}

void RichEditFindReplace::OnReplaceAllCoreBegin()
{
    m_nOldCursor = ui::GlobalManager::Instance().Cursor().GetCursorID();
    ui::GlobalManager::Instance().Cursor().SetCursor(ui::CursorType::kCursorWait);
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->HideSelection(true);
    }
}

void RichEditFindReplace::OnReplaceAllCoreEnd(int /*replaceCount*/)
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->HideSelection(false);
    }
    if (m_nOldCursor != 0) {
        ui::GlobalManager::Instance().Cursor().SetCursorByID(m_nOldCursor);
        m_nOldCursor = 0;
    }
}
