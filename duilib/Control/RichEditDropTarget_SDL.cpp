#include "RichEditDropTarget_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "duilib/Control/RichEdit2.h"
#include "duilib/Control/RichEditDropTargetHelper.h"
#include "duilib/Core/ControlDropTargetUtils.h"
#include "duilib/Core/DpiManager.h"

namespace ui 
{

RichEditDropTarget_SDL::RichEditDropTarget_SDL(RichEdit2* pRichEdit) :
    m_pRichEdit(pRichEdit),
    m_nStartChar(0),
    m_nEndChar(0)
{
}

int32_t RichEditDropTarget_SDL::OnDropBegin(const UiPoint& /*pt*/)
{
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return S_FALSE;
    }

    m_dropTextList.clear();
    m_dropFileList.clear();

    m_pRichEdit->GetSel(m_nStartChar, m_nEndChar);
    m_scrollPos = m_pRichEdit->GetScrollPos();

    return S_OK;
}

void RichEditDropTarget_SDL::OnDropPosition(const UiPoint& pt)
{
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return;
    }
    if (!m_pRichEdit->IsFocused()) {
        m_pRichEdit->SetFocus();
    }
    int32_t pos = m_pRichEdit->CharFromPos(pt);
    if (pos >= 0) {
        UiPoint charPt = m_pRichEdit->PosFromChar(pos);
        m_pRichEdit->SetCaretPos(charPt.x, charPt.y);
        m_pRichEdit->ShowCaret(true);

        //检查是否需要滚动
        CheckTextScroll(pt);
    }
}

void RichEditDropTarget_SDL::OnDropTexts(const std::vector<DString>& textList, const UiPoint& pt)
{
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return;
    }
    if (textList.empty()) {
        return;
    }

    m_dropTextList = textList;

    if (!CheckDropText(pt)) {
        return;
    }

    int32_t nDropPos = m_pRichEdit->CharFromPos(pt);
    if ((nDropPos >= 0) && !m_dropTextList.empty()) {
        DString dropText;
        for (size_t i = 0; i < m_dropTextList.size(); ++i) {
            if (i > 0) {
                dropText += _T("\n");
            }
            dropText += m_dropTextList[i];
        }
        m_pRichEdit->SetDropTextPosition(nDropPos);
        m_pRichEdit->SetSel(nDropPos, nDropPos);
        m_pRichEdit->ReplaceSel(dropText, true);
    }

    m_dropTextList.clear();
}

void RichEditDropTarget_SDL::OnDropFiles(const DString& /*source*/, const std::vector<DString>& fileList, const UiPoint& /*pt*/)
{
    if ((m_pRichEdit == nullptr) || m_pRichEdit->IsReadOnly() || !m_pRichEdit->IsEnabled() || m_pRichEdit->IsPasswordMode()) {
        return;
    }
    if (fileList.empty()) {
        return;
    }

    m_dropFileList = fileList;

    if (!m_pRichEdit->IsEnableDropFile()) {
        m_dropFileList.clear();
        return;
    }

    DString fileTypes = m_pRichEdit->GetDropFileTypes();
    if (!ControlDropTargetUtils::IsFilteredFileTypes(fileTypes, m_dropFileList)) {
        m_dropFileList.clear();
        return;
    }

    std::vector<DString> filteredFileList = m_dropFileList;
    ControlDropTargetUtils::RemoveUnsupportedFiles(filteredFileList, fileTypes);
    if (filteredFileList.empty()) {
        m_dropFileList.clear();
        return;
    }

    ControlDropData_SDL data;
    data.m_ptClientX = 0;
    data.m_ptClientY = 0;
    data.m_bTextData = false;
    data.m_source = _T("");
    data.m_fileList = filteredFileList;
    data.m_bHandled = false;

    EventArgs msg;
    msg.SetSender(m_pRichEdit);
    msg.eventType = EventType::kEventDropData;
    msg.vkCode = VirtualKeyCode::kVK_None;
    msg.wParam = kControlDropTypeSDL;
    msg.lParam = (LPARAM)&data;
    msg.modifierKey = 0;
    msg.eventData = 0;

    m_pRichEdit->SendEventMsg(msg);

    m_dropFileList.clear();
}

void RichEditDropTarget_SDL::OnDropLeave()
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetSel(m_nStartChar, m_nEndChar);
        m_pRichEdit->SetScrollPos(m_scrollPos);
    }
    m_dropTextList.clear();
    m_dropFileList.clear();
}

bool RichEditDropTarget_SDL::CheckDropText(const UiPoint& clientPt) const
{
    RichEditDropTargetHelper dropTargetHelper(m_pRichEdit, m_dropTextList);
    return dropTargetHelper.CheckDropText(clientPt);
}

void RichEditDropTarget_SDL::CheckTextScroll(const UiPoint& clientPt)
{
    RichEditDropTargetHelper dropTargetHelper(m_pRichEdit, m_dropTextList);
    dropTargetHelper.CheckTextScroll(clientPt);
}

} // namespace ui

#endif // DUILIB_BUILD_FOR_SDL
