#include "RichEditFindReplace.h"
#include <algorithm>

RichEditFindReplace::RichEditFindReplace():
    m_pRichEdit(nullptr),
    m_bFindDown(true),
    m_bMatchCase(true),
    m_bMatchWholeWord(false),
    m_nLastReplaceCount(0),
    m_bFirstSearch(true),
    m_nInitialSearchPos(0),
    m_nOldCursor(0)
{
}

void RichEditFindReplace::SetRichEdit(ui::RichEdit* pRichEdit)
{
    m_pRichEdit = pRichEdit;
}

bool RichEditFindReplace::FindRichText(const RichEditFindOptions& options, ui::Window* pWndDialog)
{
    ApplyOptions(options);
    m_sReplaceWith.clear();

    if (FindTextSimple()) {
        //找到
        AdjustDialogPosition(pWndDialog);
        return true;
    }
    else {
        //没找到
        TextNotFound(m_sFindNext);
        return false;
    }
}

bool RichEditFindReplace::HasFindText() const
{
    return !m_sFindNext.empty();
}

bool RichEditFindReplace::FindNext()
{
    if (m_sFindNext.empty()) {
        return false;
    }
    if (FindTextSimple()) {
        //找到
        return true;
    }
    else {
        //没找到
        TextNotFound(m_sFindNext);
        return false;
    }
}

bool RichEditFindReplace::ReplaceRichText(const RichEditFindOptions& options, ui::Window* pWndDialog)
{
    ApplyOptions(options);
    m_sReplaceWith = options.replaceText;

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
    m_nLastReplaceCount = bReplaced ? 1 : 0;

    //未替换时，先找到下一处匹配并选中；返回是否找到了匹配
    const bool bFoundNext = FindTextSimple();
    if (!bFoundNext) {
        TextNotFound(m_sFindNext);
    }
    else {
        AdjustDialogPosition(pWndDialog);
    }
    return bReplaced || bFoundNext;
}

bool RichEditFindReplace::ReplaceAllRichText(const RichEditFindOptions& options)
{
    ApplyOptions(options);
    m_sReplaceWith = options.replaceText;

    if (m_pRichEdit == nullptr) {
        return false;
    }
    if (m_sFindNext.empty()) {
        return false;
    }

    if (!SameAsSelected(m_sFindNext, m_bMatchCase))    {
        if (!FindTextSimple()) {
            TextNotFound(m_sFindNext);
            return false;
        }
    }

    OnReplaceAllCoreBegin();
    int replaceCount = 0;
    do {
        ++replaceCount;
        m_pRichEdit->ReplaceSel(m_sReplaceWith, false);
    } while (FindTextSimple());

    OnReplaceAllCoreEnd(replaceCount);
    m_nLastReplaceCount = replaceCount;
    return replaceCount > 0;
}

int32_t RichEditFindReplace::GetLastReplaceCount() const
{
    return m_nLastReplaceCount;
}

void RichEditFindReplace::GetMatchInfo(int32_t& nIndex, int32_t& nTotal) const
{
    nIndex = 0;
    nTotal = 0;
    if ((m_pRichEdit == nullptr) || m_sFindNext.empty()) {
        return;
    }
    const int32_t nTextLength = m_pRichEdit->GetTextLength();
    if (nTextLength <= 0) {
        return;
    }
    int32_t nSelStartChar = 0;
    int32_t nSelEndChar = 0;
    m_pRichEdit->GetSel(nSelStartChar, nSelEndChar);

    //从头开始正向扫描，统计匹配总数以及当前选区之前（不含）的匹配个数
    int32_t nBefore = 0;
    int32_t nPos = 0;
    while (nPos <= nTextLength) {
        ui::FindTextParam findParam;
        findParam.chrg.cpMin = nPos;
        findParam.chrg.cpMax = nTextLength;
        findParam.findText = m_sFindNext;
        findParam.bFindDown = true;
        findParam.bMatchCase = m_bMatchCase;
        findParam.bMatchWholeWord = m_bMatchWholeWord;

        ui::TextCharRange chrgText;
        if (!m_pRichEdit->FindRichText(findParam, chrgText)) {
            break;
        }
        if ((chrgText.cpMin < 0) || (chrgText.cpMax < chrgText.cpMin)) {
            break;
        }
        ++nTotal;
        if (chrgText.cpMin < nSelStartChar) {
            ++nBefore;
        }
        //推进到本次匹配之后，避免重复统计或死循环
        nPos = (chrgText.cpMax > chrgText.cpMin) ? chrgText.cpMax : (chrgText.cpMin + 1);
    }

    //当前选区是匹配项时，它位于第几个匹配
    const DString selectedText = m_pRichEdit->GetSelText();
    bool bSelectedIsMatch = false;
    if (!selectedText.empty()) {
        if (m_bMatchCase) {
            bSelectedIsMatch = (selectedText == m_sFindNext);
        }
        else {
            bSelectedIsMatch = ui::StringUtil::IsEqualNoCase(selectedText, m_sFindNext);
        }
    }
    if (bSelectedIsMatch && ((nBefore + 1) <= nTotal)) {
        nIndex = nBefore + 1;
    }
}

void RichEditFindReplace::ApplyOptions(const RichEditFindOptions& options)
{
    const bool bChanged = (m_sFindNext != options.findText) ||
                          (m_bFindDown != options.bFindDown) ||
                          (m_bMatchCase != options.bMatchCase) ||
                          (m_bMatchWholeWord != options.bMatchWholeWord);

    m_sFindNext = options.findText;
    m_bFindDown = options.bFindDown;
    m_bMatchCase = options.bMatchCase;
    m_bMatchWholeWord = options.bMatchWholeWord;

    if (bChanged) {
        //查找条件变化后，重新从当前选区开始查找
        m_bFirstSearch = true;
        m_nInitialSearchPos = 0;
    }
}

bool RichEditFindReplace::FindTextSimple()
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
        if (m_bFindDown) {
            m_nInitialSearchPos = nStartChar;
        }
        else {
            m_nInitialSearchPos = nEndChar;
        }
        m_bFirstSearch = false;
    }
    findParam.chrg.cpMin = nStartChar;
    findParam.chrg.cpMax = nEndChar;
    findParam.findText = m_sFindNext;
    findParam.bFindDown = m_bFindDown;
    findParam.bMatchCase = m_bMatchCase;
    findParam.bMatchWholeWord = m_bMatchWholeWord;
    if (findParam.chrg.cpMin != findParam.chrg.cpMax)    {
        if (m_bFindDown) {
            findParam.chrg.cpMin++;
        }
        else {
            findParam.chrg.cpMin = std::max(findParam.chrg.cpMin, 0);
        }
    }

    findParam.chrg.cpMax = m_pRichEdit->GetTextLength() + m_nInitialSearchPos;

    if (m_bFindDown) {
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
        if (m_bFindDown) {
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

    //对话框窗口矩形（屏幕坐标）
    ui::UiRect rect;
    pWndDialog->GetWindowRect(rect);
    if (!rect.ContainsPt(pt)) {
        //查找结果未被对话框遮挡，无需移动
        return;
    }

    //将对话框移动到查找结果的下一行或上一行，避免遮挡
    ui::UiRect rcWork;
    if (!pWndDialog->GetMonitorWorkRect(rcWork)) {
        return;
    }
    int32_t nDialogHeight = rect.Height();
    int32_t nNewTop = pt.y + 1;
    if ((nNewTop + nDialogHeight) > rcWork.bottom) {
        nNewTop = pt.y - nDialogHeight - 1;
    }
    if (nNewTop < rcWork.top) {
        nNewTop = rcWork.top;
    }
    if (nNewTop != rect.top) {
        pWndDialog->MoveWindow(rect.left, nNewTop, rect.Width(), nDialogHeight, true);
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
