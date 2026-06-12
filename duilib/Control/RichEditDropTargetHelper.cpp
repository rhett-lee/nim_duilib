#include "RichEditDropTargetHelper.h"
#include "duilib/Control/RichEdit2.h"
#include "duilib/Core/DpiManager.h"

namespace ui 
{
RichEditDropTargetHelper::RichEditDropTargetHelper(RichEdit2* pRichEdit, const std::vector<DString>& dropTextList):
    m_pRichEdit(pRichEdit),
    m_dropTextList(dropTextList)
{
}

bool RichEditDropTargetHelper::CheckDropText(const UiPoint& clientPt) const
{
    if (m_dropTextList.empty() || (m_pRichEdit == nullptr)) {
        return false;
    }

    //拖入文本操作：进行有效性判断
    if (!m_pRichEdit->IsMultiLine()) {
        if (m_dropTextList.size() > 1) {
            //拖入为多行文本，无效
            return false;
        }
    }
    if (m_pRichEdit->IsNumberOnly()) {
        //数字模式
        if (m_dropTextList.size() > 1) {
            //拖入为多行文本，无效
            return false;
        }

        DString dropText = m_dropTextList.front();
        if (!dropText.empty()) {
            size_t count = dropText.size();
            for (size_t index = 0; index < count; ++index) {
                if (dropText[index] == L'\0') {
                    break;
                }
                if ((dropText[index] > L'9') || (dropText[index] < L'0')) {
                    //有不是数字的字符，禁止拖入
                    return false;
                }
            }
        }
    }
    DString limitChars = m_pRichEdit->GetLimitChars();
    if (!limitChars.empty()) {
        //有设置限制字符
        for (const DString& dropText : m_dropTextList) {
            size_t count = dropText.size();
            for (size_t index = 0; index < count; ++index) {
                if (dropText[index] == L'\0') {
                    break;
                }
                bool bMatch = false;
                for (const DString::value_type ch : limitChars) {
                    if (ch == dropText[index]) {
                        bMatch = true;
                        break;
                    }
                }
                if (!bMatch) {
                    //有不是有效的字符，禁止拖入
                    return false;
                }
            }
        }
    }
    return m_pRichEdit->CanDropTextOnMousePosition(clientPt);
}

void RichEditDropTargetHelper::CheckTextScroll(const UiPoint& clientPt)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    UiSize64 scrollRange = m_pRichEdit->GetScrollRange();
    if ((scrollRange.cx <= 0) && (scrollRange.cy <= 0)) {
        return;
    }

    UiPoint pt(clientPt);
    pt.Offset(m_pRichEdit->GetScrollOffsetInScrollBox());
    UiRect rcRichEdit = m_pRichEdit->GetPos();

    const int32_t nScrollSize = m_pRichEdit->Dpi().GetScaleInt(18); //距离边缘多少像素时开始滚动
    ScrollBar* pVScrollBar = m_pRichEdit->GetVScrollBar();
    ScrollBar* pHScrollBar = m_pRichEdit->GetHScrollBar();
    if ((pHScrollBar != nullptr) && pHScrollBar->IsValid()) {
        int32_t nScrollSizeRight = nScrollSize;
        int32_t nScrollSizeLeft = nScrollSize;
        if ((pVScrollBar != nullptr) && pVScrollBar->IsValid() && pVScrollBar->IsVisible()) {
            if (m_pRichEdit->IsVScrollBarAtLeft()) {
                nScrollSizeLeft += pVScrollBar->GetFixedWidth().GetInt32();
            }
            else {
                nScrollSizeRight += pVScrollBar->GetFixedWidth().GetInt32();
            }
        }
        if ((pt.x < rcRichEdit.right) && ((rcRichEdit.right - pt.x) < nScrollSizeRight)) {
            m_pRichEdit->LineRight();
        }
        else if ((pt.x > rcRichEdit.left) && ((pt.x - rcRichEdit.left) < nScrollSizeLeft)) {
            m_pRichEdit->LineLeft();
        }
    }
    if ((pVScrollBar != nullptr) && pVScrollBar->IsValid()) {
        int32_t nScrollSizeTop = nScrollSize;
        int32_t nScrollSizeBottom = nScrollSize;
        if ((pHScrollBar != nullptr) && pHScrollBar->IsValid() && pHScrollBar->IsVisible()) {
            nScrollSizeBottom += pHScrollBar->GetFixedWidth().GetInt32();
        }
        if ((pt.y < rcRichEdit.bottom) && ((rcRichEdit.bottom - pt.y) < nScrollSizeBottom)) {
            m_pRichEdit->LineDown();
        }
        else if ((pt.y > rcRichEdit.top) && ((pt.y - rcRichEdit.top) < nScrollSizeTop)) {
            m_pRichEdit->LineUp();
        }
    }
}

} // namespace ui
