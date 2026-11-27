#include "RichEditData.h"
#include "duilib/Utils/PerformanceUtil.h"
#include <unordered_set>
#include <algorithm>

namespace ui
{
RichEditData::RichEditData(IRichTextData* pRichTextData):
    m_pRichText(pRichTextData),
    m_hAlignType(HorAlignType::kAlignLeft),
    m_vAlignType(VerAlignType::kAlignTop),
    m_bSingleLineMode(true),
    m_pRender(nullptr),
    m_pRenderFactory(nullptr),
    m_bCacheDirty(true),
    m_bLastUndoCanMerge(false),
    m_nUndoLimit(64),
    m_bTextRectYOffsetUpdated(false),
    m_bTextRectXOffsetUpdated(false)
{
    ASSERT(pRichTextData != nullptr);

}

RichEditData::~RichEditData()
{
}

void RichEditData::SetRender(IRender* pRender)
{
    m_pRender = pRender;
}

void RichEditData::SetRenderFactory(IRenderFactory* pRenderFactory)
{
    m_pRenderFactory = pRenderFactory;
}

void RichEditData::SetTextDrawRect(const UiRect& rcTextDrawRect, bool bCheckDirty)
{
    if (m_rcTextDrawRect != rcTextDrawRect) {
        //当宽度发生变化时，需要重新计算文字的坐标
        if (bCheckDirty && (m_rcTextDrawRect.Width() != rcTextDrawRect.Width())) {
            SetCacheDirty(true);
        }
        m_rcTextDrawRect = rcTextDrawRect;
    }
}

void RichEditData::SetScrollOffset(const UiSize& szScrollOffset)
{
    //滚动条位置变化时，不需要重新计算
    m_szScrollOffset = szScrollOffset;
}

void RichEditData::SetTextHAlignType(HorAlignType hAlignType)
{
    if (m_hAlignType != hAlignType) {
        m_hAlignType = hAlignType;
        SetCacheDirty(true);
    }
}

HorAlignType RichEditData::GetHAlignType() const
{
    return m_hAlignType;
}

void RichEditData::SetTextVAlignType(VerAlignType vAlignType)
{
    if (m_vAlignType != vAlignType) {
        m_vAlignType = vAlignType;
        SetCacheDirty(true);
    }
}

VerAlignType RichEditData::GetVAlignType() const
{
    return m_vAlignType;
}

void RichEditData::UnionRect(UiRect& rect, const UiRect& r) const
{
    if ((r.left >= r.right) && (r.top >= r.bottom)) {
        //r是空矩形
        return;
    }
    if ((rect.left >= rect.right) && (rect.top >= rect.bottom)) {
        //自己是空矩形
        rect = r;
    }
    else {
        //按横向和纵向分别合并矩形
        if (r.right > r.left) {
            if (rect.right > rect.left) {
                //两个都不是空
                rect.left = (std::min)(rect.left, r.left);
                rect.top = (std::min)(rect.top, r.top);
            }
            else {
                //自己是空
                rect.left = r.left;
                rect.top  = r.top;
            }
        }
        if (r.bottom > r.top) {
            if (rect.bottom > rect.top) {
                //两个都不是空
                rect.right = (std::max)(rect.right, r.right);
                rect.bottom = (std::max)(rect.bottom, r.bottom);
            }
            else {
                //自己是空
                rect.right = r.right;
                rect.bottom = r.bottom;
            }
        }
    }
}

void RichEditData::UnionRectF(UiRectF& rect, const UiRectF& r) const
{
    if ((r.left >= r.right) && (r.top >= r.bottom)) {
        //r是空矩形
        return;
    }
    if ((rect.left >= rect.right) && (rect.top >= rect.bottom)) {
        //自己是空矩形
        rect = r;
    }
    else {
        //按横向和纵向分别合并矩形
        if (r.right > r.left) {
            if (rect.right > rect.left) {
                //两个都不是空
                rect.left = (std::min)(rect.left, r.left);
                rect.top = (std::min)(rect.top, r.top);
            }
            else {
                //自己是空
                rect.left = r.left;
                rect.top = r.top;
            }
        }
        if (r.bottom > r.top) {
            if (rect.bottom > rect.top) {
                //两个都不是空
                rect.right = (std::max)(rect.right, r.right);
                rect.bottom = (std::max)(rect.bottom, r.bottom);
            }
            else {
                //自己是空
                rect.right = r.right;
                rect.bottom = r.bottom;
            }
        }
    }
}

UiRect RichEditData::EstimateTextDisplayBounds(const UiRect& rcAvailable)
{
    UiRect rect;
    ASSERT(m_pRender != nullptr);
    if (m_pRender == nullptr) {
        return rect;
    }
    ASSERT(m_pRenderFactory != nullptr);
    if (m_pRenderFactory == nullptr) {
        return rect;
    }
    ASSERT(m_pRichText != nullptr);
    if (m_pRichText == nullptr) {
        return rect;
    }

    UiRect rcDrawRect = m_pRichText->GetRichTextDrawRect();
    if (rcAvailable.Width() == rcDrawRect.Width()) {
        //检查并计算字符位置
        CheckCalcTextRects();
        rect = GetTextRect();
    }
    else {
        //重新估算
        std::vector<std::wstring_view> textView;
        GetTextView(textView);
        if (!textView.empty()) {
            bool bFirst = true;
            RichTextDataList richTextDataList;
            if (m_pRichText->GetRichTextForDraw(textView, richTextDataList)) {
                std::vector<std::vector<UiRect>> richTextRects;
                m_pRender->MeasureRichText(rcAvailable, UiSize(), richTextDataList, nullptr, nullptr, &richTextRects);
                for (const std::vector<UiRect>& data : richTextRects) {
                    for (const UiRect& textRect : data) {
                        if (bFirst) {
                            //第一个
                            rect = textRect;
                            bFirst = false;
                        }
                        else {
                            UnionRect(rect, textRect);
                        }                        
                    }
                }
            }
        }
    }
    return rect;
}

void RichEditData::CalcCacheTextRects(UiRect& rcTextRect) const
{
    rcTextRect.Clear();
    UiRectF rowRects;
    bool bFirst = true;
    for (const RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
        ASSERT(pLineInfo != nullptr);
        const size_t nRowCount = pLineInfo->m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            const UiRectF& rowRect = pLineInfo->m_rowInfo[nRow].m_rowRect;
            if (bFirst) {
                rowRects = rowRect;
                bFirst = false;
            }
            else {
                UnionRectF(rowRects, rowRect);
            }
        }
    }
    rcTextRect.left = (int32_t)rowRects.left;
    rcTextRect.right = (int32_t)(ui::CEILF(rowRects.right));
    rcTextRect.top = (int32_t)rowRects.top;
    rcTextRect.bottom = (int32_t)(ui::CEILF(rowRects.bottom));
}

void RichEditData::UpdateRowTextOffsetY(RichTextLineInfoList& lineTextInfo, int32_t nOffsetY) const
{
    float fRowHeight = 0;
    float fLastRowBottom = 0;    
    bool bFirstRow = true;
    for (RichTextLineInfoPtr& pLineInfo : lineTextInfo) {
        ASSERT(pLineInfo != nullptr);
        const size_t nRowCount = pLineInfo->m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            UiRectF& rowRect = pLineInfo->m_rowInfo[nRow].m_rowRect;
            if (bFirstRow) {
                //第一行
                fRowHeight = rowRect.Height();
                rowRect.top = (float)nOffsetY;
                rowRect.bottom = rowRect.top + fRowHeight;
                fLastRowBottom = rowRect.bottom;
                bFirstRow = false;
            }
            else {
                fRowHeight = rowRect.Height();
                rowRect.top = fLastRowBottom;
                rowRect.bottom = rowRect.top + fRowHeight;
                fLastRowBottom = rowRect.bottom;
            }
        }
    }
}

void RichEditData::UpdateRowTextOffsetX(RichTextLineInfoList& lineTextInfo, HorAlignType hAlignType, std::vector<int32_t>& rowXOffset, bool& bTextRectXOffsetUpdated) const
{
    rowXOffset.clear();
    if (!bTextRectXOffsetUpdated && (hAlignType == HorAlignType::kAlignLeft)) {
        //无需更新
        return;
    }
    bTextRectXOffsetUpdated = false;
    const int32_t nDrawRectWidth = m_rcTextDrawRect.Width();//矩形总宽度
    for (RichTextLineInfoPtr& pLineInfo : lineTextInfo) {
        ASSERT(pLineInfo != nullptr);
        const size_t nRowCount = pLineInfo->m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            RichTextRowInfo& rowInfo = pLineInfo->m_rowInfo[nRow];
            UiRectF& rowRect = rowInfo.m_rowRect;
            if (rowInfo.m_xOffset > 0) {
                //恢复
                rowRect.Offset(-(float)rowInfo.m_xOffset, 0.0f);
                rowInfo.m_xOffset = 0;
            }
            if (rowRect.Width() < nDrawRectWidth) {
                if (hAlignType == HorAlignType::kAlignCenter) {
                    //居中对齐
                    float diff = nDrawRectWidth - rowRect.Width();
                    rowInfo.m_xOffset = (int32_t)(diff / 2);
                    if (rowInfo.m_xOffset > 0) {
                        rowRect.Offset((float)rowInfo.m_xOffset, 0.0f);
                        bTextRectXOffsetUpdated = true;
                    }
                }
                else if (hAlignType == HorAlignType::kAlignRight) {
                    //靠右对齐
                    float diff = nDrawRectWidth - rowRect.Width();
                    rowInfo.m_xOffset = (int32_t)diff;
                    if (rowInfo.m_xOffset > 0) {
                        rowRect.Offset((float)rowInfo.m_xOffset, 0.0f);
                        bTextRectXOffsetUpdated = true;
                    }
                }
            }
            ASSERT(rowInfo.m_xOffset >= 0);
            rowXOffset.push_back(rowInfo.m_xOffset);
        }
    }
    if (!bTextRectXOffsetUpdated) {
        std::vector<int32_t> temp;
        rowXOffset.swap(temp);
    }
}

int32_t RichEditData::GetTextRectOfssetY() const
{
    int32_t yOffset = 0;
    if (m_rcTextRect.Height() < m_rcTextDrawRect.Height()) {
        VerAlignType vAlignType = GetVAlignType();
        if (vAlignType == VerAlignType::kAlignCenter) {
            //居中对齐
            int32_t nDiff = m_rcTextDrawRect.Height() - m_rcTextRect.Height();
            yOffset = nDiff / 2;
        }
        else if (vAlignType == VerAlignType::kAlignBottom) {
            //底部对齐
            int32_t nDiff = m_rcTextDrawRect.Height() - m_rcTextRect.Height();
            yOffset = nDiff;
        }
    }    
    return yOffset;
}

const std::vector<int32_t>& RichEditData::GetTextRowXOffset() const
{
    return m_rowXOffset;
}

const UiRect& RichEditData::GetTextRect() const
{
    return m_rcTextRect;
}

void RichEditData::CheckCalcTextRects()
{
    SetTextDrawRect(m_pRichText->GetRichTextDrawRect(), true);
    if (m_bCacheDirty) {
        CalcTextRectsFull();
        SetCacheDirty(false);
        m_pRichText->OnTextRectsChanged();
    }
}

void RichEditData::CalcTextRectsFull()
{
    PerformanceUtil statPerformance(_T("RichEditData::CalcTextRectsFull"));
    //清空所有行的缓存数据
    RemoveEmptyLine(m_lineTextInfo);
    for (RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
        ASSERT(pLineInfo != nullptr);
        pLineInfo->m_rowInfo.clear();
    }
    m_rcTextRect.Clear();

    ASSERT(m_pRender != nullptr);
    if (m_pRender == nullptr) {
        return;
    }
    ASSERT(m_pRenderFactory != nullptr);
    if (m_pRenderFactory == nullptr) {
        return;
    }
    ASSERT(m_pRichText != nullptr);
    if (m_pRichText == nullptr) {
        return;
    }

    std::vector<std::wstring_view> textView;
    GetTextView(textView);
    if (textView.empty()) {
        return;
    }

    size_t nTextLen = 0;
    for (const std::wstring_view& text : textView) {
        nTextLen += text.size();
    }
    if (nTextLen == 0) {
        return;
    }

    UiRect rcDrawText = m_pRichText->GetRichTextDrawRect();
    if (rcDrawText.IsEmpty()) {
        std::vector<int32_t> temp;
        m_rowXOffset.swap(temp);
        m_bTextRectXOffsetUpdated = false;
        m_bTextRectYOffsetUpdated = false;
        ClearDrawRichTextCache();
        m_lineRowBeginIndex.clear();
        m_lineTop.clear();
        return;
    }
    //估算的时候，滚动条位置始终为(0,0)
    UiSize szScrollOffset;
    RichTextLineInfoParam lineInfoParam;
    lineInfoParam.m_nStartLineIndex = 0;
    lineInfoParam.m_nStartRowIndex = 0;
    lineInfoParam.m_pLineInfoList = &m_lineTextInfo;

    //绘制所有数据，清空行数据信息
    RichTextDataList richTextDataList;
    m_pRichText->GetRichTextForDraw(textView, richTextDataList);
    if (richTextDataList.IsEmpty()) {
        return;
    }
    m_spDrawRichTextCache.reset();
    if (m_pRichText->IsTextPasswordMode()) {
        //密码模式下，不使用绘制缓存
        ASSERT(richTextDataList.Size() == 1);
        if (richTextDataList.Size() == 1) {
            RichTextData& richTextData = richTextDataList[0];
            DStringW text = GetText();
            m_pRichText->ReplacePasswordChar(text);
            ASSERT(!text.empty());
            richTextData.m_textView = text;
            m_pRender->MeasureRichText(rcDrawText, szScrollOffset, richTextDataList, &lineInfoParam, nullptr, nullptr);
            richTextData.m_textView = std::wstring_view();
        }
        else {
            m_pRender->MeasureRichText(rcDrawText, szScrollOffset, richTextDataList, &lineInfoParam, nullptr, nullptr);
        }
    }
    else {
        //全量排版时不构建绘制缓存：绘制缓存按可视区域懒构建（见 RichEdit2::Paint）
        m_pRender->MeasureRichText(rcDrawText, szScrollOffset, richTextDataList, &lineInfoParam, nullptr, nullptr);
    }    
    SetTextDrawRect(rcDrawText, false);
    CalcCacheTextRects(m_rcTextRect);

    m_bTextRectYOffsetUpdated = false;    
    int32_t nOffsetY = GetTextRectOfssetY();
    if (nOffsetY > 0) {
        UpdateRowTextOffsetY(m_lineTextInfo, nOffsetY);
        m_bTextRectYOffsetUpdated = true;
    }

    UpdateRowTextOffsetX(m_lineTextInfo, GetHAlignType(), m_rowXOffset, m_bTextRectXOffsetUpdated);

    //检查并按需插入空行
    AppendEmptyLine(m_lineTextInfo);

    //更新物理行索引数据（行号范围与行顶部坐标），供绘制缓存按可视区域懒构建使用
    UpdateLineIndexData(0);
}

void RichEditData::AppendEmptyLine(RichTextLineInfoList& lineTextInfo) const
{
    if (IsSingleLineMode() || m_pRichText->IsTextPasswordMode() || m_pRichText->IsTextReadOnlyMode() || m_pRichText->IsTextDisabledMode()) {
        //只有多行编辑模式下，才插入空行
        return;
    }
    if (lineTextInfo.size() >= 1) {
        RichTextLineInfoPtr pLastLine = lineTextInfo[lineTextInfo.size() - 1];
        if ((pLastLine != nullptr) && (pLastLine->m_nLineTextLen > 0) && (pLastLine->m_lineText.data()[pLastLine->m_nLineTextLen - 1] == _T('\n'))) {
            //最后一行的最后一个字符是换行符，需要插入空行
            RichTextLineInfoPtr spNewTextLineInfo(new RichTextLineInfo);
            spNewTextLineInfo->m_nLineTextLen = 0;
            lineTextInfo.push_back(spNewTextLineInfo);
        }
    }
    if (lineTextInfo.size() >= 2) {
        RichTextLineInfoPtr pLastLine = lineTextInfo[lineTextInfo.size() - 1];
        RichTextLineInfoPtr pPreLine = lineTextInfo[lineTextInfo.size() - 2];
        if ((pLastLine != nullptr) && (pLastLine->m_nLineTextLen == 0) &&
            (pPreLine != nullptr) && !pPreLine->m_rowInfo.empty()) {
            pLastLine->m_rowInfo.clear();
            pLastLine->m_rowInfo.push_back(RichTextRowInfo());
            pLastLine->m_rowInfo[0].m_xOffset = pPreLine->m_rowInfo[0].m_xOffset;
            pLastLine->m_rowInfo[0].m_rowRect = pPreLine->m_rowInfo[0].m_rowRect;
            pLastLine->m_rowInfo[0].m_rowRect.Offset(0, pPreLine->m_rowInfo[0].m_rowRect.Height()); //高度与上一行相同
            pLastLine->m_rowInfo[0].m_rowRect.right = pLastLine->m_rowInfo[0].m_rowRect.left + m_pRichText->GetTextCaretWidth(); //宽度与光标宽度相同
            pLastLine->m_rowInfo[0].m_charInfo.PushBack(0.0f, 0.0f, 0);

            //按对齐方式，重新计算m_xOffset的值和left值
            HorAlignType hAlignType = m_hAlignType;
            const int32_t nDrawRectWidth = m_rcTextDrawRect.Width();//矩形总宽度
            RichTextRowInfo& rowInfo = pLastLine->m_rowInfo[0];
            UiRectF& rowRect = rowInfo.m_rowRect;
            if (rowInfo.m_xOffset > 0) {
                //恢复
                rowRect.Offset(-(float)rowInfo.m_xOffset, 0.0f);
                rowInfo.m_xOffset = 0;
            }
            if (rowRect.Width() < nDrawRectWidth) {
                if (hAlignType == HorAlignType::kAlignCenter) {
                    //居中对齐
                    float diff = nDrawRectWidth - rowRect.Width();
                    rowInfo.m_xOffset = (int32_t)(diff / 2);
                    if (rowInfo.m_xOffset > 0) {
                        rowRect.Offset((float)rowInfo.m_xOffset, 0.0f);
                    }
                }
                else if (hAlignType == HorAlignType::kAlignRight) {
                    //靠右对齐
                    float diff = nDrawRectWidth - rowRect.Width();
                    rowInfo.m_xOffset = (int32_t)diff;
                    if (rowInfo.m_xOffset > 0) {
                        rowRect.Offset((float)rowInfo.m_xOffset, 0.0f);
                    }
                }
            }
            ASSERT(rowInfo.m_xOffset >= 0);
        }
    }
}

void RichEditData::RemoveEmptyLine(RichTextLineInfoList& lineTextInfo) const
{
    if (lineTextInfo.size() >= 2) {
        RichTextLineInfoPtr pPreLine = lineTextInfo[lineTextInfo.size() - 2];
        if ((pPreLine != nullptr) && (pPreLine->m_nLineTextLen > 0) && (pPreLine->m_lineText.data()[pPreLine->m_nLineTextLen - 1] == _T('\n'))) {
            //最后一行的最后一个字符是换行符，需要删除最后的空行
            RichTextLineInfoPtr pLastLine = lineTextInfo[lineTextInfo.size() - 1];
            if ((pLastLine != nullptr) && (pLastLine->m_nLineTextLen == 0)) {
                lineTextInfo.pop_back();
            }
        }
    }
}

void RichEditData::CheckLineTextData(const RichTextLineInfoList& lineTextInfo, size_t nIndex) const
{
    ASSERT(nIndex < lineTextInfo.size());
    if (nIndex >= lineTextInfo.size()) {
        return;
    }
    const RichTextLineInfoPtr& spLineInfo = lineTextInfo[nIndex];
    ASSERT(spLineInfo != nullptr);
    if (spLineInfo == nullptr) {
        return;
    }
    ASSERT((spLineInfo->m_nLineTextLen > 0) || (nIndex == (lineTextInfo.size() - 1))); //仅最后一行允许为空
    if ((spLineInfo->m_nLineTextLen == 0) && (nIndex == (lineTextInfo.size() - 1))) {
        ASSERT(lineTextInfo.size() > 1);
        if (lineTextInfo.size() > 1) {
            const RichTextLineInfoPtr& spPreLineInfo = lineTextInfo[nIndex - 1];
            ASSERT(spPreLineInfo != nullptr);
            if (spPreLineInfo != nullptr) {
                ASSERT((spPreLineInfo->m_nLineTextLen > 0));
                ASSERT((spPreLineInfo->m_lineText.data()[spPreLineInfo->m_nLineTextLen - 1] == _T('\n')));
            }            
        }
    }
}

void RichEditData::CalcTextRectsPart(size_t nStartLine,
                                     const std::vector<size_t>& modifiedLines,
                                     const std::vector<size_t>& deletedLines)
{
    PerformanceUtil statPerformance(_T("RichEditData::CalcTextRectsPart"));
    ASSERT(!m_pRichText->IsTextPasswordMode());//密码模式下，不应使用该函数
    if (nStartLine != (size_t)-1) {
        ASSERT(!modifiedLines.empty() || !deletedLines.empty());
        if (!modifiedLines.empty()) {
            ASSERT(modifiedLines[0] == nStartLine);
            if (modifiedLines[0] != nStartLine) {
                nStartLine = (size_t)-1;
            }
        }
        else if (!deletedLines.empty()) {
            ASSERT(deletedLines[0] == nStartLine);
            if (deletedLines[0] != nStartLine) {
                nStartLine = (size_t)-1;
            }
        }
    }
    ASSERT(nStartLine != (size_t)-1);
    ASSERT(nStartLine <= m_lineTextInfo.size());
    if (nStartLine > m_lineTextInfo.size()) {
        return;
    }
    if (nStartLine == m_lineTextInfo.size()) {
        //删除最后一行的情况
        ASSERT(modifiedLines.empty());
    }

    //绘制变化的数据，清空相关的行数据信息
    if (!modifiedLines.empty()) {
        //有修改的行
        size_t nLineIndex = 0;
        const size_t nCount = modifiedLines.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            nLineIndex = modifiedLines[nIndex];
            ASSERT(nLineIndex < m_lineTextInfo.size());
            if (nLineIndex < m_lineTextInfo.size()) {
                RichTextLineInfoPtr& pLineInfo = m_lineTextInfo[nLineIndex];
                ASSERT(pLineInfo != nullptr);
                pLineInfo->m_rowInfo.clear();
            }
            else {
                //遇到数据错误
                return;
            }
        }
    }

    ASSERT(m_pRender != nullptr);
    if (m_pRender == nullptr) {
        return;
    }
    ASSERT(m_pRenderFactory != nullptr);
    if (m_pRenderFactory == nullptr) {
        return;
    }
    ASSERT(m_pRichText != nullptr);
    if (m_pRichText == nullptr) {
        return;
    }

    std::vector<std::wstring_view> textView;
    GetTextView(textView);
    if (textView.empty()) {
        return;
    }

    size_t nTextLen = 0;
    for (const std::wstring_view& text : textView) {
        nTextLen += text.size();
    }
    if (nTextLen == 0) {
        return;
    }

    UiRect rcDrawText = m_pRichText->GetRichTextDrawRect();
    if (rcDrawText.IsEmpty()) {
        std::vector<int32_t> temp;
        m_rowXOffset.swap(temp);
        m_bTextRectXOffsetUpdated = false;
        m_bTextRectYOffsetUpdated = false;
        ClearDrawRichTextCache();
        m_lineRowBeginIndex.clear();
        m_lineTop.clear();
        return;
    }

    //估算的时候，滚动条位置始终为(0,0)
    UiSize szScrollOffset;
    RichTextLineInfoParam lineInfoParam;
    lineInfoParam.m_nStartLineIndex = (uint32_t)nStartLine;
    lineInfoParam.m_nStartRowIndex = 0;
    lineInfoParam.m_pLineInfoList = &m_lineTextInfo;
    if (nStartLine > 0) {
        //计算起始的逻辑行号
        for (size_t nLine = 0; nLine < nStartLine; ++nLine) {
            const RichTextLineInfo& lineInfo = *m_lineTextInfo[nLine];
            lineInfoParam.m_nStartRowIndex += (uint32_t)lineInfo.m_rowInfo.size();
        }
    }

    //增量排版后，绘制缓存的处理：
    //绘制缓存按可视区域懒构建，只覆盖一部分物理行；当修改/删除的行都位于缓存覆盖的范围之后时，
    //缓存内的行内容、行号与行位置均不受影响，可以继续复用（避免每次输入都重建约 3 屏的缓存）；
    //否则丢弃缓存，由 RichEdit2::Paint 按新的可视区域重新懒构建。
    if ((m_spDrawRichTextCache == nullptr) || (nStartLine < (m_nDrawCacheFirstLine + m_nDrawCacheLineCount))) {
        ClearDrawRichTextCache();
    }

    if (!modifiedLines.empty()) {
        //有修改的行，重新计算行数据
        RichTextDataList richTextDataListModified;
        m_pRichText->GetRichTextForDraw(textView, richTextDataListModified, nStartLine, modifiedLines);
        if (richTextDataListModified.IsEmpty()) {
            return;
        }
        m_pRender->MeasureRichText(rcDrawText, szScrollOffset, richTextDataListModified, &lineInfoParam, nullptr, nullptr);
    }

    //绘制后，增量绘制后的行高数据
    if (nStartLine < m_lineTextInfo.size()) {
        UpdateRowInfo(nStartLine);
    }    
    if (m_bTextRectYOffsetUpdated) {
        UpdateRowTextOffsetY(m_lineTextInfo, 0);
    }
    UpdateRowTextOffsetX(m_lineTextInfo, HorAlignType::kAlignLeft, m_rowXOffset, m_bTextRectXOffsetUpdated);
    
    CalcCacheTextRects(m_rcTextRect);

    m_bTextRectYOffsetUpdated = false;
    const int32_t nOffsetY = GetTextRectOfssetY();
    if (nOffsetY > 0) {
        UpdateRowTextOffsetY(m_lineTextInfo, nOffsetY);
        m_bTextRectYOffsetUpdated = true;
    }
    UpdateRowTextOffsetX(m_lineTextInfo, GetHAlignType(), m_rowXOffset, m_bTextRectXOffsetUpdated);

    //更新物理行索引数据（从变化的行开始，后续行的行号与行顶部坐标可能变化）
    UpdateLineIndexData(nStartLine);

#ifdef _DEBUG
    //增量布局结果与完整布局的一致性校验：开销较大（相当于再做一次全量排版），
    //默认关闭；排查增量布局问题时，定义 DUILIB_RICHEDIT_DEBUG_FULL_LAYOUT_CHECK 开启
    #ifdef DUILIB_RICHEDIT_DEBUG_FULL_LAYOUT_CHECK
    //比较与完整绘制时是否一致
    if (nStartLine != (size_t)-1) {
        std::vector<std::wstring_view> textView2;
        RichTextLineInfoList lineTextInfoList;
        for (const RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
            ASSERT(pLineInfo != nullptr);
            if (pLineInfo->m_nLineTextLen == 0) {
                continue;
            }
            RichTextLineInfoPtr spLineInfo(new RichTextLineInfo);
            spLineInfo->m_nLineTextLen = pLineInfo->m_nLineTextLen;
            spLineInfo->m_lineText = pLineInfo->m_lineText;
            textView2.push_back(spLineInfo->GetLineTextView());
            lineTextInfoList.push_back(spLineInfo);
        }
        RichTextDataList richTextDataList2;
        m_pRichText->GetRichTextForDraw(textView2, richTextDataList2);

        std::shared_ptr<DrawRichTextCache> spDrawRichTextCacheNew;

        RichTextLineInfoParam lineInfoParam2;
        lineInfoParam2.m_pLineInfoList = &lineTextInfoList;
        lineInfoParam2.m_nStartLineIndex = 0;
        lineInfoParam2.m_nStartRowIndex = 0;
        m_pRender->MeasureRichText(rcDrawText, szScrollOffset, richTextDataList2, &lineInfoParam2, &spDrawRichTextCacheNew, nullptr);

        if (nOffsetY > 0) {
            UpdateRowTextOffsetY(lineTextInfoList, nOffsetY);
        }

        std::vector<int32_t> rowXOffset;
        bool bTextRectXOffsetUpdated = false;
        UpdateRowTextOffsetX(lineTextInfoList, GetHAlignType(), rowXOffset, bTextRectXOffsetUpdated);

        //比较数据的一致性，增量绘制的结果，应该与完整绘制的结果相同
        ASSERT(lineTextInfoList.size() == m_lineTextInfo.size());
        const size_t nDataCount = lineTextInfoList.size();
        for (size_t nDataIndex = 0; nDataIndex < nDataCount; ++nDataIndex) {
            const RichTextLineInfo& infoOld = *m_lineTextInfo[nDataIndex];
            const RichTextLineInfo& infoNew = *lineTextInfoList[nDataIndex];
            ASSERT(infoOld.GetLineTextView() == infoNew.GetLineTextView());
            ASSERT(infoOld.m_nLineTextLen == infoNew.m_nLineTextLen);
            ASSERT(infoOld.m_rowInfo.size() == infoNew.m_rowInfo.size());

            const size_t nRowCount = infoOld.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                const RichTextRowInfo& rowOld = infoOld.m_rowInfo[nRow];
                const RichTextRowInfo& rowNew = infoNew.m_rowInfo[nRow];
                ASSERT(rowOld.m_rowRect == rowNew.m_rowRect);
                ASSERT(rowOld.m_charInfo.CharCount() == rowNew.m_charInfo.CharCount());
                ASSERT(rowOld.m_charInfo == rowNew.m_charInfo);
            }
        }

        //比较绘制缓存的数据
        if ((m_spDrawRichTextCache != nullptr) && (spDrawRichTextCacheNew != nullptr)) {
            bool bRet = m_pRender->IsDrawRichTextCacheEqual(*m_spDrawRichTextCache, *spDrawRichTextCacheNew);
            ASSERT(bRet);
        }
    }
    #endif
#endif
}

bool RichEditData::SetText(const DStringW& text)
{
    PerformanceUtil statPerformance(_T("RichEditData::SetText"));
    if (text.empty()) {
        Clear();
        return true;
    }

    //清空追加的空行
    RemoveEmptyLine(m_lineTextInfo);

    //整篇文本的来源（需要截断或者含L'\0'字符时，使用临时字符串）
    const DStringW* pSourceText = &text;
    DStringW textLimit;
    DStringW validText;
    int32_t nLimitLength = m_pRichText->GetTextLimitLength();
    if ((nLimitLength > 0) && ((int32_t)text.size() > nLimitLength)){
        //截断字符串
        textLimit = text;
        if (text.find(L'\0') != DStringW::npos) {
            //如果包含L'\0'字符，需要截断处理
            textLimit = text.c_str();
        }
        TruncateLimitText(textLimit, nLimitLength);
        pSourceText = &textLimit;
    }
    else if (text.find(L'\0') != DStringW::npos) {
        //如果包含L'\0'字符，需要截断处理
        validText = text.c_str();
        pSourceText = &validText;
    }

    std::vector<std::wstring_view> lineTextViewList;
    {
        std::wstring_view textView = *pSourceText;
        SplitLines(textView, lineTextViewList);
    }
    
    const size_t nLineCount = lineTextViewList.size();
    bool bTextChanged = false;
    if (m_lineTextInfo.size() == lineTextViewList.size()) { //比较文本内容是否有变化
        //先比较字符串长度
        for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
            if (m_lineTextInfo[nIndex]->m_nLineTextLen != lineTextViewList[nIndex].size()) {
                bTextChanged = true;
                break;
            }
        }
        if (!bTextChanged) {
            //如果长度都一致，则比较字符串的内容
            for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
                if (m_lineTextInfo[nIndex]->GetLineTextView() != lineTextViewList[nIndex]) {
                    bTextChanged = true;
                    break;
                }
            }
        }
    }
    else {
        bTextChanged = true;
    }
    if (bTextChanged) {
        RichTextLineInfoList lineTextInfo;
        if (nLineCount > 0) {
            //整篇文本只保存一份，各行文本通过偏移量共享这篇文本的存储，避免逐行复制
            //(偏移量相对整篇文本的起始位置计算，这里的复制与来源文本的内容完全相同)
            std::shared_ptr<UiStringW> spText = std::make_shared<UiStringW>(*pSourceText);
            const DStringW::value_type* pSourceData = pSourceText->c_str();
            const size_t nTextSize = pSourceText->size();
            lineTextInfo.resize(nLineCount);
            for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
                const std::wstring_view& lineTextView = lineTextViewList[nIndex];
                ASSERT_UNUSED_VARIABLE((lineTextView.data() >= pSourceData) && ((lineTextView.data() + lineTextView.size()) <= (pSourceData + nTextSize)));
                RichTextLineInfoPtr& lineText = lineTextInfo[nIndex];
                lineText.reset(new RichTextLineInfo);
                lineText->m_lineText.SetShareText(spText, (uint32_t)(lineTextView.data() - pSourceData));
                lineText->m_nLineTextLen = (uint32_t)lineTextView.size();
                CheckLineTextData(lineTextInfo, nIndex);
            }
        }
        m_lineTextInfo.swap(lineTextInfo);
        SetCacheDirty(true);
        ClearUndoList();
    }
    else if (!m_bCacheDirty){
        //检查并追加空行
        AppendEmptyLine(m_lineTextInfo);
    }
    return bTextChanged;
}

void RichEditData::SplitLines(const std::wstring_view& textView, std::vector<std::wstring_view>& lineTextViewList) const
{
    if (textView.empty()) {
        return;
    }
    if (m_bSingleLineMode || m_pRichText->IsTextPasswordMode()) {
        //单行文本模式, 密码模式时，不分行
        lineTextViewList.push_back(textView);
        return;
    }
    //多行文本模式
    std::vector<size_t> lineSeprators;
    const size_t nTextLen = textView.size();
    for (size_t nTextIndex = 0; nTextIndex < nTextLen; ++nTextIndex) {
        if (textView[nTextIndex] == L'\n') {
            lineSeprators.push_back(nTextIndex);
        }
    }
    if (lineSeprators.empty()) {
        //没有换行分隔符，单行
        lineTextViewList.push_back(std::wstring_view(textView.data(), textView.size()));
    }
    else {
        //有换行分隔符，切分为多行, 并保留换行符
        size_t nLastIndex = 0;
        size_t nCurrentIndex = 0;
        size_t nCharCount = 0;
        const size_t nLineSepCount = lineSeprators.size();
        for (size_t nLine = 0; nLine < nLineSepCount; ++nLine) {
            if (nLine == 0) {
                //首行
                nLastIndex = 0;
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex < textView.size());
                nCharCount = nCurrentIndex - nLastIndex + 1;
                if (nCharCount > 0) {
                    lineTextViewList.push_back(std::wstring_view(textView.data(), nCharCount));
                }
            }
            else {
                //中间行
                nLastIndex = lineSeprators[nLine - 1];
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex > nLastIndex);
                ASSERT(nCurrentIndex < textView.size());
                nCharCount = nCurrentIndex - nLastIndex;
                if (nCharCount > 0) {
                    lineTextViewList.push_back(std::wstring_view(textView.data() + nLastIndex + 1, nCharCount));
                }
            }

            if (nLine == (nLineSepCount - 1)) {
                //末行: 将最后一行数据添加进来
                nLastIndex = lineSeprators[nLine];
                nCurrentIndex = textView.size() - 1;
                ASSERT(nCurrentIndex >= nLastIndex);
                nCharCount = nCurrentIndex - nLastIndex;
                if (nCharCount > 0) {
                    lineTextViewList.push_back(std::wstring_view(textView.data() + nLastIndex + 1, nCharCount));
                }
            }
        }
    }
}

void RichEditData::GetTextView(std::vector<std::wstring_view>& textView) const
{
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        if (lineText.m_nLineTextLen > 0) {
            textView.push_back(std::wstring_view(lineText.m_lineText.data(), lineText.m_nLineTextLen));
        }
    }
}

void RichEditData::GetTextView(std::vector<std::wstring_view>& textView, size_t nFirstLine, size_t nLineCount) const
{
    textView.clear();
    const size_t nTotalLineCount = m_lineTextInfo.size();
    if ((nFirstLine >= nTotalLineCount) || (nLineCount == 0)) {
        return;
    }
    size_t nEndLine = nFirstLine + nLineCount;
    if (nEndLine > nTotalLineCount) {
        nEndLine = nTotalLineCount;
    }
    textView.reserve(nEndLine - nFirstLine);
    for (size_t nIndex = nFirstLine; nIndex < nEndLine; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        if (lineText.m_nLineTextLen > 0) {
            textView.push_back(std::wstring_view(lineText.m_lineText.data(), lineText.m_nLineTextLen));
        }
    }
}

size_t RichEditData::GetTextLength() const
{
    size_t nTextLen = 0;
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
    }
    return nTextLen;
}

bool RichEditData::IsEmpty() const
{
    bool bEmpty = true;
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        if (lineText.m_nLineTextLen > 0) {
            bEmpty = false;
            break;
        }
    }
    return bEmpty;
}

DStringW RichEditData::GetText() const
{
    DStringW text;
    text.reserve(GetTextLength() + 2);
    std::vector<std::wstring_view> textViewList;
    GetTextView(textViewList);
    for (const std::wstring_view& textView : textViewList) {
        text += textView;
    }
    return text;
}

bool RichEditData::FindLineTextPos(int32_t nStartChar, int32_t nEndChar,
                                   size_t& nStartLine, size_t& nEndLine,
                                   size_t& nStartCharLineOffset, size_t& nEndCharLineOffset) const
{
    //该函数不需要计算字符的矩形范围
    ASSERT((nStartChar >= 0) && (nEndChar >= 0) && (nEndChar >= nStartChar));
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar > nEndChar)) {
        return false;
    }
    if (m_lineTextInfo.empty()) {
        //当前为空
        nStartLine = 0;
        nEndLine = 0;
        nStartCharLineOffset = 0;
        nEndCharLineOffset = 0;
        if ((nStartChar == 0) && (nEndChar == 0)) {
            return true;
        }
        return false;
    }

    constexpr const size_t nNotFound = (size_t)-1;
    nStartLine = nNotFound;                 //起始行
    nEndLine = nNotFound;                   //结束行
    size_t nStartCharBaseLen = nNotFound;   //在起始行之前的总长度
    size_t nEndCharBaseLen = nNotFound;     //在结束行之前的总长度
    nStartCharLineOffset = nNotFound;       //在起始行中，开始字符的偏移量
    nEndCharLineOffset = nNotFound;         //在结束行中，结束字符的偏移量
    size_t nTextLen = 0;                    //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if ((nStartChar < (int32_t)nTextLen) && (nStartLine == nNotFound)) {
            nStartLine = nIndex;
            nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            nStartCharLineOffset = (size_t)nStartChar - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
        }
        if ((nEndChar < (int32_t)nTextLen) && (nEndLine == nNotFound)) {
            nEndLine = nIndex;
            nEndCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            nEndCharLineOffset = (size_t)nEndChar - nEndCharBaseLen;
            ASSERT(nEndCharLineOffset < lineText.m_nLineTextLen);
        }
        if ((nStartLine != nNotFound) && (nEndLine != nNotFound)) {
            break;
        }
        if (nIndex == (nLineCount - 1)) {
            //最后一行
            if ((nStartChar == (int32_t)nTextLen) && (nStartLine == nNotFound)) {
                nStartLine = nIndex;
                nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
                nStartCharLineOffset = (size_t)nStartChar - nStartCharBaseLen;
                ASSERT(nStartCharLineOffset == lineText.m_nLineTextLen);
            }
            if ((nEndChar == (int32_t)nTextLen) && (nEndLine == nNotFound)) {
                nEndLine = nIndex;
                nEndCharBaseLen = nTextLen - lineText.m_nLineTextLen;
                nEndCharLineOffset = (size_t)nEndChar - nEndCharBaseLen;
                ASSERT(nEndCharLineOffset == lineText.m_nLineTextLen);
            }
        }
    }
    if ((nStartLine != nNotFound) && (nEndLine != nNotFound) &&
        (nStartCharLineOffset != nNotFound) && (nEndCharLineOffset != nNotFound)) {
        ASSERT(nEndLine >= nStartLine);
        return true;
    }
    return false;
}

bool RichEditData::ReplaceText(int32_t nStartChar, int32_t nEndChar, const DStringW& text, bool bCanUndo, bool bClearRedo, bool bMergeUndo)
{
    PerformanceUtil statPerformance(_T("RichEditData::ReplaceText"));
    ASSERT((nStartChar >= 0) && (nEndChar >= 0) && (nEndChar >= nStartChar));
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar > nEndChar)) {
        return false;
    }

    int32_t nLimitLength = m_pRichText->GetTextLimitLength();
    int32_t nTextLenDiff = (int32_t)text.size() - (nEndChar - nStartChar);
    if ((nTextLenDiff > 0) && (nLimitLength > 0)) {
        //字符串会变长，检查字符串长度是否超过限制
        int32_t nDestTextLen = (int32_t)GetTextLength() + nTextLenDiff;
        if (nDestTextLen > nLimitLength) {
            //超过限制长度：通知外部（用于触发文本超限事件），返回错误
            m_pRichText->OnTextLimitExceeded();
            return false;
        }
    }

    //检查并计算字符位置
    CheckCalcTextRects();

    //清空追加的空行
    RemoveEmptyLine(m_lineTextInfo);

    constexpr const size_t nNotFound = (size_t)-1;
    size_t nStartLine = nNotFound;              //起始行
    size_t nEndLine = nNotFound;                //结束行
    size_t nStartCharLineOffset = nNotFound;    //在起始行中，开始字符的偏移量
    size_t nEndCharLineOffset = nNotFound;      //在结束行中，结束字符的偏移量
    if (!FindLineTextPos(nStartChar, nEndChar, nStartLine, nEndLine, nStartCharLineOffset, nEndCharLineOffset)) {
        return false;
    }

    DStringW oldText; //旧文本内容

    //是否需要记录撤销操作
    if (m_nUndoLimit == 0) {
        bCanUndo = false;
    }
    if (bCanUndo && (nEndChar > nStartChar)) {
        oldText = GetTextRange(nStartChar, nEndChar);
    }
    //操作结果
    std::wstring_view startLineTextView; //起始行的剩余文本
    std::wstring_view endLineTextView;   //结束行的剩余文本
    if (nStartLine == nEndLine) {
        //在相同行
        if (nStartLine < m_lineTextInfo.size()) {
            const RichTextLineInfo& lineText = *m_lineTextInfo[nStartLine];
            std::wstring_view textView = lineText.GetLineTextView();
            startLineTextView = textView.substr(0, nStartCharLineOffset); //保留到行首的文本
            if (nEndCharLineOffset < textView.size()) {
                endLineTextView = textView.substr(nEndCharLineOffset);        //保留到行尾的文本
            }
        }
    }
    else if (nEndLine > nStartLine) {
        //在不同行
        ASSERT(nEndLine < m_lineTextInfo.size());
        if (nEndLine >= m_lineTextInfo.size()) {
            //错误
            return false;
        }
        for (size_t nIndex = nStartLine; nIndex <= nEndLine; ++nIndex) {
            const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
            std::wstring_view textView = lineText.GetLineTextView();
            if (nIndex == nStartLine) {
                //首行，保留到行首的文本
                startLineTextView = textView.substr(0, nStartCharLineOffset);
            }
            else if (nIndex == nEndLine) {
                //末行，保留到行尾的文本
                if (nEndCharLineOffset < textView.size()) {
                    endLineTextView = textView.substr(nEndCharLineOffset);
                }
            }
        }
    }

    //拼接变化后的新文本，重新分行
    DStringW newText;
    newText.reserve(startLineTextView.size() + text.size() + endLineTextView.size() + 1);
    newText = startLineTextView;
    newText += text;
    newText += endLineTextView;

    //待删除的行
    std::vector<size_t> deletedLines;
    for (size_t nIndex = nStartLine; nIndex <= nEndLine; ++nIndex) {
        deletedLines.push_back(nIndex);
    }
    //倒序删除
    if (!deletedLines.empty()) {
        int32_t nDelIndex = (int32_t)deletedLines.size() - 1;
        for (; nDelIndex >= 0; --nDelIndex) {
            if (deletedLines[nDelIndex] < m_lineTextInfo.size()) {
                m_lineTextInfo.erase(m_lineTextInfo.begin() + deletedLines[nDelIndex]);
            }
        }
    }

    std::wstring_view newTextView = newText;
    std::vector<std::wstring_view> lineTextViewList;
    SplitLines(newTextView, lineTextViewList);

    size_t nNewLineCount = 0;
    for (const std::wstring_view& textView : lineTextViewList) {
        if (!textView.empty()) {
            //插入新行
            RichTextLineInfoPtr lineTextInfo(new RichTextLineInfo);
            lineTextInfo->m_lineText = textView;
            lineTextInfo->m_nLineTextLen = (uint32_t)textView.size();
            m_lineTextInfo.insert(m_lineTextInfo.begin() + nStartLine + nNewLineCount, lineTextInfo);
            ++nNewLineCount;
        }
    }

    //文本有变化的行
    std::vector<size_t> modifiedLines;
    for (size_t nIndex = 0; nIndex < nNewLineCount; ++nIndex) {
        modifiedLines.push_back(nStartLine + nIndex);
    }

    if (!m_bCacheDirty && (!modifiedLines.empty() || !deletedLines.empty())) {
        //修改的行，需要重新计算(增量计算)
        if ((m_lineTextInfo.size() <= 1) || m_pRichText->IsTextPasswordMode()) {
            //单行模式、密码模式、文本为空时，完整绘制
            CalcTextRectsFull();
        }
        else {
            //多行模式时，使用增量绘制
            RemoveEmptyLine(m_lineTextInfo);
            CalcTextRectsPart(nStartLine, modifiedLines, deletedLines);
            AppendEmptyLine(m_lineTextInfo);
        }
    }
    if (bCanUndo) {
        //生成撤销列表
        AddToUndoList(nStartChar, text, oldText, bMergeUndo);
    }
    else if (bClearRedo){
        ClearUndoList();
    }
    return true;
}

DStringW RichEditData::GetTextRange(int32_t nStartChar, int32_t nEndChar) const
{
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar >= nEndChar)) {
        return DStringW();
    }

    constexpr const size_t nNotFound = (size_t)-1;
    size_t nStartLine = nNotFound;              //起始行
    size_t nEndLine = nNotFound;                //结束行
    size_t nStartCharLineOffset = nNotFound;    //在起始行中，开始字符的偏移量
    size_t nEndCharLineOffset = nNotFound;      //在结束行中，结束字符的偏移量
    if (!FindLineTextPos(nStartChar, nEndChar, nStartLine, nEndLine, nStartCharLineOffset, nEndCharLineOffset)) {
        return DStringW();
    }

    DStringW selText; //文本内容
    if (nStartLine == nEndLine) {
        //在相同行
        const RichTextLineInfo& lineText = *m_lineTextInfo[nStartLine];
        DStringW newText(lineText.GetLineTextView());
        if (nEndCharLineOffset > nStartCharLineOffset) {
            //有选择的文本
            size_t nCharCount = nEndCharLineOffset - nStartCharLineOffset;
            selText = newText.substr(nStartCharLineOffset, nCharCount);
        }
    }
    else if (nEndLine > nStartLine) {
        //在不同行
        DStringW newText;
        for (size_t nIndex = nStartLine; nIndex <= nEndLine; ++nIndex) {
            const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
            newText = lineText.GetLineTextView();
            if (nIndex == nStartLine) {
                //首行, 选择到行尾
                selText = newText.substr(nStartCharLineOffset);
            }
            else if (nIndex == nEndLine) {
                //末行，选择到行首
                newText = lineText.GetLineTextView();
                if (nEndCharLineOffset > 0) {
                    selText += newText.substr(0, nEndCharLineOffset);
                }                    
            }
            else {
                //中间行
                selText += lineText.GetLineTextView();
            }
        }
    }
    return selText;
}

bool RichEditData::HasTextRange(int32_t nStartChar, int32_t nEndChar) const
{
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar >= nEndChar)) {
        return false;
    }
    constexpr const size_t nNotFound = (size_t)-1;
    size_t nStartLine = nNotFound;              //起始行
    size_t nEndLine = nNotFound;                //结束行
    size_t nStartCharLineOffset = nNotFound;    //在起始行中，开始字符的偏移量
    size_t nEndCharLineOffset = nNotFound;      //在结束行中，结束字符的偏移量
    if (!FindLineTextPos(nStartChar, nEndChar, nStartLine, nEndLine, nStartCharLineOffset, nEndCharLineOffset)) {
        return false;
    }

    bool bHasText = false;
    if (nStartLine == nEndLine) {
        //在相同行
        bHasText = (nEndCharLineOffset > nStartCharLineOffset) ? true : false;
    }
    else if (nEndLine > nStartLine) {
        //在不同行
        bHasText = true;
    }
    return bHasText;
}

void RichEditData::SetSingleLineMode(bool bSingleLineMode)
{
    if (m_bSingleLineMode != bSingleLineMode) {
        m_bSingleLineMode = bSingleLineMode;
        SetCacheDirty(true);

        //切换单行模式和多行模式
        DStringW text = GetText();
        SetText(text);
    }
}

bool RichEditData::IsSingleLineMode() const
{
    return m_bSingleLineMode;
}

bool RichEditData::GetCharLineRowIndex(int32_t nCharIndex, size_t& nLineNumber, size_t& nLineRowIndex, size_t& nStartCharRowOffset) const
{
    ASSERT(!m_bCacheDirty);
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return false;
    }
    bool bFound = false;
    size_t nTextLen = 0; //文本总长度
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        CheckLineTextData(lineTextInfoList, nLineIndex);
        nTextLen += lineTextInfo.m_nLineTextLen;
        if ((size_t)nCharIndex < nTextLen) {
            const size_t nStartBaseLen = nTextLen - lineTextInfo.m_nLineTextLen;
            const size_t nStartLineOffset = (size_t)nCharIndex - nStartBaseLen;
            ASSERT(nStartLineOffset < lineTextInfo.m_nLineTextLen);
            //定位在本物理分行中，再定位在哪个逻辑分行中
            size_t nRowTextLen = 0;
            const size_t nRowCount = lineTextInfo.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                const RichTextRowInfo& rowInfo = lineTextInfo.m_rowInfo[nRow];
                nRowTextLen += rowInfo.m_charInfo.CharCount();
                if (nStartLineOffset < nRowTextLen) {
                    //定位在本逻辑分行中
                    const size_t nStartCharBaseLen = nRowTextLen - rowInfo.m_charInfo.CharCount();
                    bFound = true;
                    nStartCharRowOffset = (size_t)nStartLineOffset - nStartCharBaseLen;
                    nLineNumber = nLineIndex;
                    nLineRowIndex = nRow;
                    break;
                }
            }
            break;
        }
        else if (((size_t)nCharIndex == nTextLen) && (nLineIndex == (nLineCount - 1))) {
            //最后一行的最后一个字符之后的位置
            const size_t nRowCount = lineTextInfo.m_rowInfo.size();
            if (nRowCount > 0) {
                const RichTextRowInfo& rowInfo = lineTextInfo.m_rowInfo[nRowCount - 1];                
                nStartCharRowOffset = rowInfo.m_charInfo.CharCount();
                nLineNumber = nLineIndex;
                nLineRowIndex = nRowCount - 1;
                bFound = true;
                break;
            }
            else {
                //空行的情况
                nStartCharRowOffset = 0;
                nLineNumber = nLineIndex;
                nLineRowIndex = 0;
                bFound = true;
                break;
            }
        }
    }
    return bFound;
}

const RichTextRowInfo* RichEditData::GetRowInfoFromPoint(const UiPoint& pt) const
{
    ASSERT(!m_bCacheDirty);
    const RichTextRowInfo* pRowInfo = nullptr;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            const RichTextRowInfo& rowInfo = lineTextInfo.m_rowInfo[nRow];
            const UiRectF& rowRect = rowInfo.m_rowRect;
            if ((pt.y >= rowRect.top) && (pt.y < rowRect.bottom)) {
                pRowInfo = &lineTextInfo.m_rowInfo[nRow];
                break;
            }
        }
        if (pRowInfo != nullptr) {
            break;
        }
    }
    return pRowInfo;
}

const RichTextRowInfo* RichEditData::GetCharRowInfo(int32_t nCharIndex, size_t& nStartCharRowOffset) const
{
    ASSERT(!m_bCacheDirty);
    size_t nLineNumber = 0;
    size_t nLineRowIndex = 0;
    const RichTextRowInfo* pRowInfo = nullptr;
    if (GetCharLineRowIndex(nCharIndex, nLineNumber, nLineRowIndex, nStartCharRowOffset)) {
        if (nLineNumber < m_lineTextInfo.size()) {
            const RichTextLineInfo& lineTextInfo = *m_lineTextInfo[nLineNumber];
            if (nLineRowIndex < lineTextInfo.m_rowInfo.size()) {
                pRowInfo = &lineTextInfo.m_rowInfo[nLineRowIndex];
            }
        }
    }
    return pRowInfo;
}

const RichTextRowInfo* RichEditData::GetFirstRowInfo() const
{
    ASSERT(!m_bCacheDirty);
    const RichTextRowInfo* pRowInfo = nullptr;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    if (!lineTextInfoList.empty()) {
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[0];
        ASSERT(!lineTextInfo.m_rowInfo.empty());
        if (!lineTextInfo.m_rowInfo.empty()) {
            pRowInfo = &lineTextInfo.m_rowInfo[0];
        }
    }
    return pRowInfo;
}

const RichTextRowInfo* RichEditData::GetLastRowInfo() const
{
    ASSERT(!m_bCacheDirty);
    const RichTextRowInfo* pRowInfo = nullptr;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    if (nLineCount != 0) {
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineCount - 1];
        //ASSERT(!lineTextInfo.m_rowInfo.empty());
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        if (nRowCount != 0) {
            pRowInfo = &lineTextInfo.m_rowInfo[nRowCount - 1];
        }
    }
    return pRowInfo;
}

size_t RichEditData::GetRowInfoStartIndex(const RichTextRowInfo* pRowInfo) const
{
    ASSERT(!m_bCacheDirty);
    size_t nStartIndex = (size_t)-1;
    size_t nTextLen = 0; //文本总长度
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        CheckLineTextData(lineTextInfoList, nLineIndex);

        size_t nRowTextLen = 0;
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            const RichTextRowInfo& rowInfo = lineTextInfo.m_rowInfo[nRow];
            if (&lineTextInfo.m_rowInfo[nRow] == pRowInfo) {
                //找到此行
                nStartIndex = nTextLen + nRowTextLen;
                break;
            }
            nRowTextLen += rowInfo.m_charInfo.CharCount();
        }

        nTextLen += lineTextInfo.m_nLineTextLen;
        if (nStartIndex != (size_t)-1) {
            break;
        }
    }
    return nStartIndex;
}

void RichEditData::UpdateRowInfo(size_t nDrawStartLineIndex)
{
    RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    ASSERT(nDrawStartLineIndex < nLineCount);
    if (nDrawStartLineIndex >= nLineCount) {
        return;
    }
    size_t nLineIndex = nDrawStartLineIndex;
    if (nDrawStartLineIndex > 0) {
        nLineIndex -= 1;//从上一行开始，以获取上一行的bottom坐标值
    }
    float fLastRowHeight = 0.0f;   //本行的行高值
    float fLastBottomValue = 0.0f; //上一行的bottom值
    for (; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        if (lineTextInfoList[nLineIndex] == nullptr) {
            continue;
        }
        const size_t nLineRowCount = lineTextInfoList[nLineIndex]->m_rowInfo.size();
        ASSERT((nLineRowCount > 0) || (nLineIndex == (nLineCount - 1)));
        for (size_t nLineRowIndex = 0; nLineRowIndex < nLineRowCount; ++nLineRowIndex) {
            UiRectF& rowRect = lineTextInfoList[nLineIndex]->m_rowInfo[nLineRowIndex].m_rowRect;
            if (nLineIndex >= nDrawStartLineIndex) {
                //更新本行的纵向坐标值
                fLastRowHeight = rowRect.bottom - rowRect.top;
                rowRect.top = fLastBottomValue;
                rowRect.bottom = rowRect.top + fLastRowHeight;
            }
            fLastBottomValue = rowRect.bottom;
        }        
    }
}

UiPoint RichEditData::PosForEmptyText() const
{
    UiRect rcDrawRect = m_pRichText->GetRichTextDrawRect();
    HorAlignType hAlignType = GetHAlignType();
    VerAlignType vAlignType = GetVAlignType();
    UiPoint pt;
    if (hAlignType == HorAlignType::kAlignCenter) {
        pt.x = rcDrawRect.Width() / 2;
    }
    else if (hAlignType == HorAlignType::kAlignRight) {
        pt.x = rcDrawRect.Width();
    }
    else {
        pt.x = 0;
    }

    if (vAlignType == VerAlignType::kAlignCenter) {
        const int32_t nRowHeight = m_pRichText->GetTextRowHeight();
        if (rcDrawRect.Height() <= nRowHeight) {
            pt.y = 0;
        }
        else {
            pt.y = rcDrawRect.Height() / 2;
            if (nRowHeight > 0) {
                pt.y -= nRowHeight / 2;
                if (pt.y < 0) {
                    pt.y = 0;
                }
            }
        }
    }
    else if (vAlignType == VerAlignType::kAlignBottom) {
        const int32_t nRowHeight = m_pRichText->GetTextRowHeight();
        if (rcDrawRect.Height() <= nRowHeight) {
            pt.y = 0;
        }
        else {
            pt.y = rcDrawRect.Height();
            if (nRowHeight > 0) {
                pt.y -= nRowHeight;
                if (pt.y < 0) {
                    pt.y = 0;
                }
            }
        }
    }
    else {
        pt.y = 0;
    }
    return pt;
}

UiPoint RichEditData::CaretPosFromChar(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    if (m_rcTextDrawRect.IsEmpty()) {
        //绘制区域为空
        return UiPoint(m_rcTextDrawRect.left, m_rcTextDrawRect.top);
    }

    UiPoint cursorPos;
    if (m_lineTextInfo.empty()) {
        //空文本
        cursorPos = PosForEmptyText();
        if (GetHAlignType() == HorAlignType::kAlignRight) {
            cursorPos.x -= m_pRichText->GetTextCaretWidth();
        }
    }
    else if (nCharIndex < 0) {
        //无效参数
        cursorPos.x = 0;
        cursorPos.y = 0;
    }
    else {
        size_t nStartCharRowOffset = 0;
        const RichTextRowInfo* spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
        if (spRowInfo != nullptr) {
            const RichTextRowInfo& rowInfo = *spRowInfo;
            const size_t nIndexOffset = nStartCharRowOffset;
            cursorPos.y = (int32_t)rowInfo.m_rowRect.top;
            cursorPos.x = (int32_t)(rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nIndexOffset));
        }        
        else {
            //取最后一个字符的右上角坐标
            spRowInfo = GetLastRowInfo();
            if (spRowInfo != nullptr) {
                const RichTextRowInfo& rowInfo = *spRowInfo;
                cursorPos.y = (int32_t)rowInfo.m_rowRect.top;
                const size_t nCharCount = rowInfo.m_charInfo.CharCount();
                cursorPos.x = (int32_t)ui::CEILF(rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nCharCount));
            }
        }
    }

    //转换为外部坐标
    ConvertToExternal(cursorPos);
    return cursorPos;
}

UiRect RichEditData::GetCharRowRect(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiRect rowRect;
    if (m_lineTextInfo.empty()) {
        //空文本
        if (nCharIndex == 0) {
            const int32_t nRowHeight = m_pRichText->GetTextRowHeight();
            const UiRect rc = m_pRichText->GetRichTextDrawRect();
            UiPoint pt = PosForEmptyText();
            rowRect.left = 0;
            rowRect.right = rc.Width();
            rowRect.top = pt.y;
            rowRect.bottom = rowRect.top + nRowHeight;
        }
    }
    else {
        size_t nStartCharRowOffset = 0;
        const RichTextRowInfo* spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
        if (spRowInfo != nullptr) {
            const RichTextRowInfo& rowInfo = *spRowInfo;
            const UiRectF& rowRectF = rowInfo.m_rowRect;
            UiRect rc = m_pRichText->GetRichTextDrawRect();
            rowRect.left = 0;
            rowRect.right = std::max(rc.Width(), (int32_t)rowRectF.Width());
            rowRect.top = (int32_t)rowRectF.top;
            rowRect.bottom = (int32_t)ui::CEILF(rowRectF.bottom);
        }
    }

    //转换为外部坐标
    ConvertToExternal(rowRect);
    return rowRect;
}

UiPoint RichEditData::PosFromChar(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiPoint pt;
    if (m_lineTextInfo.empty()) {
        //空文本
        pt = PosForEmptyText();
    }
    else if (nCharIndex < 0) {
        //无效参数
        pt.x = 0;
        pt.y = 0;
    }
    else {     
        size_t nStartCharRowOffset = 0;
        const RichTextRowInfo* spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
        if (spRowInfo != nullptr) {
            const RichTextRowInfo& rowInfo = *spRowInfo;
            const size_t nIndexOffset = nStartCharRowOffset;
            pt.y = (int32_t)rowInfo.m_rowRect.top;
            pt.x = (int32_t)(rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nIndexOffset));
        }
        else {
            //取最后一个字符的左上角坐标
            spRowInfo = GetLastRowInfo();
            if (spRowInfo != nullptr) {
                const RichTextRowInfo& rowInfo = *spRowInfo;
                pt.y = (int32_t)rowInfo.m_rowRect.top;
                if (!rowInfo.m_charInfo.Empty()) {
                    const size_t nIndexOffset = rowInfo.m_charInfo.CharCount() - 1;
                    pt.x = (int32_t)(rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nIndexOffset));
                }
                else {
                    pt.x = (int32_t)rowInfo.m_rowRect.left;
                }
            }
        }
    }

    //转换为外部坐标
    ConvertToExternal(pt);
    return pt;
}

int32_t RichEditData::CharFromPos(UiPoint pt)
{
    bool bAtRowEnd = false;
    return CharFromPos(pt, bAtRowEnd);
}

int32_t RichEditData::CharFromPos(UiPoint pt, bool& bAtRowEnd)
{
    bAtRowEnd = false;
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength == 0) {
        return 0;
    }

    //检查并计算字符位置
    CheckCalcTextRects();

    if (m_rcTextDrawRect.IsEmpty()) {
        //文本显示区域为空
        return 0;
    }

    //转换为内部坐标
    ConvertToInternal(pt);

    //横向按字符边界对齐，纵向按行高对齐
    int32_t nCharPosIndex = -1;
    const RichTextRowInfo* spDestRow = nullptr;
    const RichTextRowInfo* spFirstRow = GetFirstRowInfo();
    if (spFirstRow != nullptr) {
        const UiRectF& rowRect = spFirstRow->m_rowRect;
        if (pt.y < rowRect.top) {
            //该点在区域上方，定位到第一行
            spDestRow = spFirstRow;
        }
    }
    if (spDestRow == nullptr) {
        const RichTextRowInfo* spLastRow = GetLastRowInfo();
        if (spLastRow != nullptr) {
            const UiRectF& rowRect = spLastRow->m_rowRect;
            if (pt.y >= rowRect.bottom) {
                //该点在区域下方，定位到最后一行
                spDestRow = spLastRow;
            }
        }        
    }

    if (spDestRow == nullptr) {
        spDestRow = GetRowInfoFromPoint(pt);        
    }
    ASSERT(spDestRow != nullptr);
    if (spDestRow != nullptr) {
        const RichTextRowInfo& rowInfo = *spDestRow;
        const size_t nCharCount = rowInfo.m_charInfo.CharCount();
        ASSERT(!rowInfo.m_charInfo.Empty());

        if (pt.x <= rowInfo.m_rowRect.left) {
            //该点在本行的左侧，指向本行的首字符
            nCharPosIndex = (int32_t)GetRowInfoStartIndex(spDestRow);
        }
        else if (pt.x >= rowInfo.m_rowRect.right) {
            //该点在本行内容的右侧，指向本行的尾字符
            bAtRowEnd = true;
            if ((nCharCount >= 2) && rowInfo.m_charInfo.IsNewLine(nCharCount - 1) && rowInfo.m_charInfo.IsReturn(nCharCount - 2)){
                //该行以回车+换行结尾: 指向回车字符
                nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.CharCount() - 2);
            }
            else if ((nCharCount >= 1) && rowInfo.m_charInfo.IsNewLine(nCharCount - 1)) {
                //该行以换行结尾: 指向换行字符
                nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.CharCount() - 1);
            }
            else {
                //本行结尾无回车和换行符，指向该字符后面
                nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.CharCount());
            }
        }
        else if ((nCharCount == 2) && rowInfo.m_charInfo.IsNewLine(nCharCount - 1) && rowInfo.m_charInfo.IsReturn(nCharCount - 2)) {
            //本行为空行，只有一个回车+换行: 指向回车字符
            nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.CharCount() - 2);
        }
        else if (nCharCount == 1) {
            //该行只有一个字符
            nCharPosIndex = (int32_t)GetRowInfoStartIndex(spDestRow);
        }
        else {
            const size_t nCount = rowInfo.m_charInfo.CharCount();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                if (rowInfo.m_charInfo.IsIgnoredChar(nIndex)) {
                    continue;
                }
                const float fCharX = rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nIndex);
                const float fCharWidth = rowInfo.m_charInfo.CharWidth(nIndex);
                if ((pt.x >= fCharX) && (pt.x < (fCharX + fCharWidth))) {
                    if (pt.x <= (fCharX + fCharWidth / 2)) {
                        //如果X坐标小于等于中心点，取当前字符
                        nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + nIndex);
                    }
                    else {
                        //如果X坐标大于中心点，则取下一个字符
                        for (size_t i = nIndex + 1; i < nCount; ++i) {
                            if (rowInfo.m_charInfo.IsLowSurrogate(i)) {
                                continue;
                            }
                            nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + i);
                            break;
                        }
                        if (nCharPosIndex == -1) {
                            nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + nIndex);
                        }
                    }
                    break;
                }
            }
        }
    }
    
    if (nCharPosIndex < 0) {
        nCharPosIndex = 0;
    }
    else if (nCharPosIndex > nTextLength) {
        nCharPosIndex = nTextLength;
    }
    return nCharPosIndex;
}

#if defined(WCHAR_T_IS_UTF16)
    #define SkUTF16_IsHighSurrogate(c)  (((c) & 0xFC00) == 0xD800)
    #define SkUTF16_IsLowSurrogate(c)   (((c) & 0xFC00) == 0xDC00)
#endif

int32_t RichEditData::GetNextValidCharIndex(const int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找
            size_t i = nStartCharLineOffset + 1;
            while ( i < lineText.m_nLineTextLen) {
#if defined(WCHAR_T_IS_UTF16)
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.data() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
                else if (SkUTF16_IsLowSurrogate(*src)) {
                    i += 1;//跳过该字符
                }
                else {
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
#else
                nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                break;
#endif
            }
            size_t nNewOffset = (size_t)nNewCharIndex - nStartCharBaseLen;
            if ((nNewOffset == (lineText.m_nLineTextLen - 1)) && (lineText.m_lineText.data()[nNewOffset] == L'\n')) {
                //如果已经指向换行符，那么跳到下一个字符(即避免从'\r'跳到'\n')
                if ((nNewOffset >= 1) && (lineText.m_lineText.data()[nNewOffset - 1] == L'\r')) {
                    nNewCharIndex += 1;
                }
            }
            break;
        }
    }
    if (nNewCharIndex == nCharIndex) {
        nNewCharIndex += 1;        
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

int32_t RichEditData::GetPrevValidCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength ;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找
            int32_t i = (int32_t)nStartCharLineOffset - 1;
            while (i >= 0) {
#if defined(WCHAR_T_IS_UTF16)
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.data() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
                else if (SkUTF16_IsLowSurrogate(*src)) {
                    i -= 1;//跳过该字符
                }
                else {
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
#else
                nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                break;
#endif
            }
            if ((nNewCharIndex == nCharIndex) && (i <= 0) && (nIndex >= 1)) {
                //已经在行首，跳到前一行的最后一个字符
                const RichTextLineInfo& prevLineText = *m_lineTextInfo[nIndex - 1];
                ASSERT(prevLineText.m_nLineTextLen > 0);
                CheckLineTextData(m_lineTextInfo, nIndex - 1);
                if (prevLineText.m_nLineTextLen > 1) {
                    ASSERT(prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 1] == L'\n');
                    if (prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 2] == L'\r') {
                        nNewCharIndex = nCharIndex - 2; //跳过"\r\n"两个字符
                    }
                    else {
                        nNewCharIndex = nCharIndex - 1; //只有'\n'时，只跳过这一个字符
                    }
                }
                else if (prevLineText.m_nLineTextLen == 1) {
                    nNewCharIndex = nCharIndex - 1; //指向此字符
                }
            }
            else {
                size_t nNewOffset = (size_t)nNewCharIndex - nStartCharBaseLen;
                if ((nNewOffset == (lineText.m_nLineTextLen - 1)) && (lineText.m_lineText.data()[nNewOffset] == L'\n')) {
                    //如果已经指向换行符，那么跳到前面的一个回车字符
                    if ((nNewOffset >= 1) && (lineText.m_lineText.data()[nNewOffset - 1] == L'\r')) {
                        nNewCharIndex -= 1;
                    }
                }
            }
            break;
        }
        else if ((nCharIndex == (int32_t)nTextLen) && (lineText.m_nLineTextLen > 0)) {
            //定位到所在的行
            bool bLastChar = false;
            if ((nIndex == (nLineCount - 2)) && (m_lineTextInfo[nLineCount - 1]->m_nLineTextLen == 0)) {
                //最后一行是空行的情况
                bLastChar = true;
            }
            else if (nIndex == (nLineCount - 1)) {
                //最后一行
                bLastChar = true;
            }
            if (bLastChar) {
                //定位到最后一个字符
                if (lineText.m_lineText.data()[lineText.m_nLineTextLen - 1] == _T('\n')) {
                    if (lineText.m_nLineTextLen >= 2) {
                        if (lineText.m_lineText.data()[lineText.m_nLineTextLen - 2] == _T('\r')) {
                            nNewCharIndex = nCharIndex - 2;
                        }
                        else {
                            nNewCharIndex = nCharIndex - 1;
                        }
                    }
                    else {
                        nNewCharIndex = nCharIndex - 1;
                    }
                }
                else {
                    nNewCharIndex = nCharIndex - 1;
                }
                if (nNewCharIndex != nCharIndex) {
                    break;
                }
            }
        }
    }
    if ((nNewCharIndex == nCharIndex) && (nNewCharIndex > 0)) {
        nNewCharIndex -= 1;
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

bool RichEditData::IsSeperatorChar(DStringW::value_type ch) const
{
    // Unicode 标准：标点 + 空白 + 控制符
    return iswpunct(ch) || iswspace(ch) || iswcntrl(ch);
}

int32_t RichEditData::GetNextValidWordIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找，直到找到一个分隔符（空格，标点符号等）
            size_t i = nStartCharLineOffset + 1;
            bool bFoundBlank = lineText.m_lineText.data()[nStartCharLineOffset] == L' ';
            while (i < lineText.m_nLineTextLen) {
                //如果是空格，则跳过连续所有空格
                while ((i < lineText.m_nLineTextLen) && lineText.m_lineText.data()[i] == L' ') {
                    bFoundBlank = true;
                    ++i;
                }
                if (i >= lineText.m_nLineTextLen) {
                    //已经到达行尾
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + lineText.m_nLineTextLen - 1);
                    break;
                }
                if (bFoundBlank) {
                    //有空格时，终止在空格后的字符
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
                if (IsSeperatorChar(lineText.m_lineText.data()[nStartCharLineOffset]) ||
                    IsSeperatorChar(lineText.m_lineText.data()[i])) {
                    //当前字符是分隔符，终止
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
#if defined(WCHAR_T_IS_UTF16)
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.data() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    i += 2;//跳过该双字节字符
                }
                else {
                    i += 1;//跳过该字符
                }
#else
                i += 1;//跳过该字符
#endif
            }
            size_t nNewOffset = (size_t)nNewCharIndex - nStartCharBaseLen;
            if ((nNewOffset == (lineText.m_nLineTextLen - 1)) && (lineText.m_lineText.data()[nNewOffset] == L'\n')) {
                //如果已经指向换行符，那么跳到下一个字符(即避免从'\r'跳到'\n')
                if ((nNewOffset >= 1) && (lineText.m_lineText.data()[nNewOffset - 1] == L'\r')) {
                    nNewCharIndex += 1;
                }
            }
            break;
        }
    }
    if (nNewCharIndex == nCharIndex) {
        nNewCharIndex += 1;
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

int32_t RichEditData::GetPrevValidWordIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找
            int32_t i = (int32_t)nStartCharLineOffset - 1;            
            bool bFoundBlank = lineText.m_lineText.data()[nStartCharLineOffset] == L' ';
            while (i >= 0) {
                //跳过连续的空格
                while ((i >= 0) && (lineText.m_lineText.data()[i] == L' ')) {
                    bFoundBlank = true;
                    i -= 1;//跳过该字符
                }

#if defined(WCHAR_T_IS_UTF16)
                if (i > 0) {
                    const uint16_t* src = (const uint16_t*)(lineText.m_lineText.data() + i);
                    if (SkUTF16_IsLowSurrogate(*src)) {
                        i -= 1;//跳过低代理字符
                    }
                }
#endif

                if (i <= 0) {
                    //已经到达行首
                    nNewCharIndex = (int32_t)nStartCharBaseLen;
                    break;
                }

                if (bFoundBlank) {
                    //有空格时，终止在空格前的字符，但不包含空格前的字符
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i + 1);
                    break;
                }

                if (IsSeperatorChar(lineText.m_lineText.data()[i])) {
                    //当前字符是分隔符，终止，但不包含分割字符本身
                    nNewCharIndex = (int32_t)(nStartCharBaseLen + i + 1);
                    break;
                }
            
                i -= 1;//跳过该字符
            }
            if ((nNewCharIndex == nCharIndex) && (i <= 0) && (nIndex >= 1)) {
                //已经在行首，跳到前一行的最后一个字符
                const RichTextLineInfo& prevLineText = *m_lineTextInfo[nIndex - 1];
                CheckLineTextData(m_lineTextInfo, nIndex - 1);
                if (prevLineText.m_nLineTextLen > 1) {
                    ASSERT(prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 1] == L'\n');
                    if (prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 2] == L'\r') {
                        nNewCharIndex = nCharIndex - 2; //跳过"\r\n"两个字符
                    }
                    else {
                        nNewCharIndex = nCharIndex - 1; //只有'\n'时，只跳过这一个字符
                    }
                }
                else if (prevLineText.m_nLineTextLen == 1) {
                    nNewCharIndex = nCharIndex - 1; //指向此字符
                }
            }
            else {
                size_t nNewOffset = (size_t)nNewCharIndex - nStartCharBaseLen;
                if ((nNewOffset == (lineText.m_nLineTextLen - 1)) && (lineText.m_lineText.data()[nNewOffset] == L'\n')) {
                    //如果已经指向换行符，那么跳到前面的一个回车字符
                    if ((nNewOffset >= 1) && (lineText.m_lineText.data()[nNewOffset - 1] == L'\r')) {
                        nNewCharIndex -= 1;
                    }
                }
            }
            break;
        }
    }
    if ((nNewCharIndex == nCharIndex) && (nNewCharIndex > 0)) {
        nNewCharIndex -= 1;
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

bool RichEditData::GetCurrentWordIndex(int32_t nCharIndex, int32_t& nWordStartIndex, int32_t& nWordEndIndex)
{
    nWordStartIndex = -1;
    nWordEndIndex = -1;
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return false;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return false;
    }
    if (nCharIndex > nTextLength) {
        return false;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);

            if (IsSeperatorChar(lineText.m_lineText.data()[nStartCharLineOffset])) {
                //当前字符是分隔符，选择此分隔符
                nWordStartIndex = (int32_t)(nStartCharBaseLen + nStartCharLineOffset);
                nWordEndIndex = (int32_t)(nStartCharBaseLen + nStartCharLineOffset + 1);
                break;
            }
            else if (lineText.m_lineText.data()[nStartCharLineOffset] == L' ') {
                //当前字符是空格符，选择连续的空格
                size_t i = nStartCharLineOffset + 1;
                while (i < lineText.m_nLineTextLen) {
                    if (lineText.m_lineText.data()[i] == L' ') {
                        ++i;
                        continue;
                    }
                    nWordEndIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
                int32_t j = (int32_t)nStartCharLineOffset - 1;
                while (j >= 0) {
                    if (lineText.m_lineText.data()[j] == L' ') {
                        --j;
                        continue;
                    }
                    nWordStartIndex = (int32_t)(nStartCharBaseLen + j + 1);
                    break;
                }
                if ((nWordEndIndex != -1)) {
                    if (j < 0) {
                        nWordStartIndex = (int32_t)nStartCharBaseLen;
                    }
                }
                if ((nWordStartIndex != -1) && (nWordEndIndex != -1)) {
                    break;
                }
            }

            //定位结束字符：向后，直到找到一个分隔符（空格，标点符号等）
            size_t i = nStartCharLineOffset + 1;
            while (i < lineText.m_nLineTextLen) {
                if (IsSeperatorChar(lineText.m_lineText.data()[nStartCharLineOffset]) ||
                    IsSeperatorChar(lineText.m_lineText.data()[i]) ||
                    (lineText.m_lineText.data()[nStartCharLineOffset] == L' ') ||
                    (lineText.m_lineText.data()[i] == L' ')) {
                    //当前字符是分隔符，终止
                    nWordEndIndex = (int32_t)(nStartCharBaseLen + i);
                    break;
                }
#if defined(WCHAR_T_IS_UTF16)
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.data() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    i += 2;//跳过该双字节字符
                }
                else {
                    i += 1;//跳过该字符
                }
#else
                i += 1;//跳过该字符
#endif
            }
            if (nWordEndIndex == -1) {
                nWordEndIndex = (int32_t)lineText.m_nLineTextLen;
            }

            //定位起始字符：向前，直到找到一个分隔符（空格，标点符号等）
            int32_t j = (int32_t)nStartCharLineOffset - 1;
            while (j >= 0) {
                if (IsSeperatorChar(lineText.m_lineText.data()[j]) || (lineText.m_lineText.data()[j] == ' ')) {
                    //当前字符是分隔符，终止，但不包含分割字符本身
                    nWordStartIndex = (int32_t)(nStartCharBaseLen + j + 1);
                    break;
                }
                j -= 1;//跳过该字符
            }
            if (nWordStartIndex == -1) {
                nWordStartIndex = (int32_t)nStartCharBaseLen;
            }
            break;
        }
    }
    return (nWordEndIndex > nWordStartIndex) && (nWordStartIndex >= 0) && (nWordEndIndex >= 0);
}

int32_t RichEditData::GetRowStartCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            //在本行中寻找
            nNewCharIndex = (int32_t)nTextLen - lineText.m_nLineTextLen;
            break;
        }
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

int32_t RichEditData::GetRowEndCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        return nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            //在本行中寻找
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            nNewCharIndex = (int32_t)(nTextLen - 1);
            const size_t nNewOffset = (size_t)nNewCharIndex - nStartCharBaseLen;
            const bool bHasLineBreak = (nNewOffset < lineText.m_nLineTextLen) &&
                                       ((lineText.m_lineText.data()[nNewOffset] == L'\n') ||
                                        (lineText.m_lineText.data()[nNewOffset] == L'\r'));
            if (!bHasLineBreak) {
                //最后一行没有换行符：行尾位置在最后一个字符之后，光标可以停在该位置
                nNewCharIndex += 1;
            }
            else if (lineText.m_lineText.data()[nNewOffset] == L'\n') {
                //如果已经指向换行符，那么跳到前面的回车符'\r'
                if ((nNewOffset >= 1) && (lineText.m_lineText.data()[nNewOffset - 1] == L'\r')) {
                    nNewCharIndex -= 1;
                }
            }
            break;
        }
    }
    if (nNewCharIndex < 0) {
        nNewCharIndex = 0;
    }
    if (nNewCharIndex > nTextLength) {
        nNewCharIndex = nTextLength;
    }
    return nNewCharIndex;
}

int32_t RichEditData::GetVisualRowStartCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        nCharIndex = nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    size_t nStartCharRowOffset = 0;
    const RichTextRowInfo* pRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
    if (pRowInfo == nullptr) {
        return GetRowStartCharIndex(nCharIndex);
    }
    const size_t nStartIndex = GetRowInfoStartIndex(pRowInfo);
    if (nStartIndex == (size_t)-1) {
        return GetRowStartCharIndex(nCharIndex);
    }
    return (int32_t)nStartIndex;
}

int32_t RichEditData::GetVisualRowEndCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    if (nCharIndex > nTextLength) {
        nCharIndex = nTextLength;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    size_t nStartCharRowOffset = 0;
    const RichTextRowInfo* pRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
    if (pRowInfo == nullptr) {
        return GetRowEndCharIndex(nCharIndex);
    }
    const size_t nStartIndex = GetRowInfoStartIndex(pRowInfo);
    if (nStartIndex == (size_t)-1) {
        return GetRowEndCharIndex(nCharIndex);
    }
    const size_t nCharCount = pRowInfo->m_charInfo.CharCount();
    int32_t nEndIndex = (int32_t)(nStartIndex + nCharCount);
    if ((nCharCount >= 1) && pRowInfo->m_charInfo.IsNewLine(nCharCount - 1)) {
        //本可视行以换行符结尾（硬换行）：光标停在换行符之前
        nEndIndex -= 1;
        if ((nCharCount >= 2) && pRowInfo->m_charInfo.IsReturn(nCharCount - 2)) {
            nEndIndex -= 1;
        }
    }
    if (nEndIndex < 0) {
        nEndIndex = 0;
    }
    if (nEndIndex > nTextLength) {
        nEndIndex = nTextLength;
    }
    return nEndIndex;
}

bool RichEditData::IsSoftWrapRowStartCharIndex(int32_t nCharIndex)
{
    const int32_t nTextLength = (int32_t)GetTextLength();
    if ((nCharIndex <= 0) || (nCharIndex >= nTextLength)) {
        return false;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    size_t nLineNumber = 0;
    size_t nLineRowIndex = 0;
    size_t nStartCharRowOffset = 0;
    if (!GetCharLineRowIndex(nCharIndex, nLineNumber, nLineRowIndex, nStartCharRowOffset)) {
        return false;
    }
    //在物理行中不是第一个逻辑行，且位于该逻辑行的行首
    return (nLineRowIndex > 0) && (nStartCharRowOffset == 0);
}

UiPoint RichEditData::CaretPosFromVisualRowEnd(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    if ((nCharIndex <= 0) || m_lineTextInfo.empty()) {
        return CaretPosFromChar(nCharIndex);
    }

    UiPoint cursorPos;
    size_t nStartCharRowOffset = 0;
    const RichTextRowInfo* pRowInfo = GetCharRowInfo(nCharIndex - 1, nStartCharRowOffset);
    if (pRowInfo == nullptr) {
        return CaretPosFromChar(nCharIndex);
    }
    const RichTextRowInfo& rowInfo = *pRowInfo;
    cursorPos.y = (int32_t)rowInfo.m_rowRect.top;
    cursorPos.x = (int32_t)ui::CEILF(rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(rowInfo.m_charInfo.CharCount()));

    //转换为外部坐标
    ConvertToExternal(cursorPos);
    return cursorPos;
}

int32_t RichEditData::GetCharWidthValue(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nCharWidth = 0;
    size_t nStartCharRowOffset = 0;
    const RichTextRowInfo* spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
    if (spRowInfo != nullptr) {
        const RichTextRowInfo& rowInfo = *spRowInfo;
        ASSERT(nStartCharRowOffset <= rowInfo.m_charInfo.CharCount());
        if (nStartCharRowOffset < rowInfo.m_charInfo.CharCount()) {
            nCharWidth = (int32_t)ui::CEILF(rowInfo.m_charInfo.CharWidth(nStartCharRowOffset));
        }
    }
    return nCharWidth;
}

void RichEditData::GetCharRangeRects(int32_t nStartChar, int32_t nEndChar, std::map<int32_t, UiRectF>& rowTextRectFs)
{
    rowTextRectFs.clear();
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength == 0) {
        return;
    }

    //检查并计算字符位置
    CheckCalcTextRects();

    if ((nStartChar < 0) || (nStartChar >= nTextLength) || (nEndChar <= nStartChar) || (nEndChar > nTextLength)) {
        return;
    }

    bool bEnd = false;
    int32_t nCurrentRowIndex = 0; //逻辑行号
    int32_t nEndRowIndex = -1;
    int32_t nStartRowIndex = -1;

    size_t nRowStartCharIndex = 0;//每行中起始字符的下标值
    size_t nTextLen = 0; //文本总长度
    size_t nRowTextLen = 0; //物理行中的逻辑行总长度
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        CheckLineTextData(lineTextInfoList, nLineIndex);
        nRowTextLen = 0;
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            nRowStartCharIndex = nTextLen + nRowTextLen;
            const RichTextRowInfo& rowInfo = lineTextInfo.m_rowInfo[nRow];

            nEndRowIndex = (int32_t)(nRowStartCharIndex + rowInfo.m_charInfo.CharCount());
            bool bFirstLine = (nStartChar >= (int32_t)nRowStartCharIndex) && (nStartChar < nEndRowIndex);
            bool bLastLine = (nEndChar >= (int32_t)nRowStartCharIndex) && (nEndChar < nEndRowIndex);

            if (bFirstLine && bLastLine) {
                //首行和尾行是同一行
                UiRectF rowRectF = rowInfo.m_rowRect;
                const size_t nStartCharIndex = (size_t)nStartChar - nRowStartCharIndex;
                const size_t nEndCharIndex = (size_t)nEndChar - nRowStartCharIndex;
                //非绘制字符和换行符的宽度为 0，因此可以直接用字符的 x 偏移量定位选区边界
                rowRectF.left = rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nStartCharIndex);
                rowRectF.right = rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nEndCharIndex);
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    UnionRectF(destRowRect, rowRectF);
                }
                bEnd = true;
                break;
            }
            else if (bFirstLine) {
                //首行: 选择到行尾
                nStartRowIndex = nCurrentRowIndex;
                UiRectF rowRectF = rowInfo.m_rowRect;
                const size_t nStartCharIndex = (size_t)nStartChar - nRowStartCharIndex;
                rowRectF.left = rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nStartCharIndex);
                rowRectF.right = rowInfo.m_rowRect.right;
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    UnionRectF(destRowRect, rowRectF);
                }
            }
            else if (bLastLine) {
                //尾行：选择到行首
                UiRectF rowRectF = rowInfo.m_rowRect;
                const size_t nEndCharIndex = (size_t)nEndChar - nRowStartCharIndex;
                rowRectF.right = rowInfo.m_rowRect.left + rowInfo.m_charInfo.CharX(nEndCharIndex);
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    UnionRectF(destRowRect, rowRectF);
                }
                bEnd = true;
                break;
            }
            else if ((nStartRowIndex >= 0) && (nCurrentRowIndex > (int32_t)nStartRowIndex)) {
                //中间行
                const UiRectF& rowRectF = rowInfo.m_rowRect;
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    UnionRectF(destRowRect, rowRectF);
                }
            }            

            nRowTextLen += rowInfo.m_charInfo.CharCount();
            ++nCurrentRowIndex; //逻辑行号递增
        }
        nTextLen += lineTextInfo.m_nLineTextLen;
        if (bEnd) {
            break;//已经结束
        }
    }

    //转换为外部坐标
    for (auto iter = rowTextRectFs.begin(); iter != rowTextRectFs.end(); ++iter) {
        UiRectF& rowRect = iter->second;
        ConvertToExternal(rowRect);
    }
}

const UiPoint& RichEditData::ConvertToExternal(UiPoint& pt) const
{
    UiRect rc = m_pRichText->GetRichTextDrawRect();
    pt.Offset(rc.left, rc.top);
    pt.Offset(-m_szScrollOffset.cx, -m_szScrollOffset.cy);
    return pt;
}

const UiRect& RichEditData::ConvertToExternal(UiRect& rect) const
{
    UiRect rc = m_pRichText->GetRichTextDrawRect();
    rect.Offset(rc.left, rc.top);
    rect.Offset(-m_szScrollOffset.cx, -m_szScrollOffset.cy);
    return rect;
}

const UiRectF& RichEditData::ConvertToExternal(UiRectF& rect) const
{
    UiRect rc = m_pRichText->GetRichTextDrawRect();
    rect.Offset((float)rc.left, (float)rc.top);
    rect.Offset(-(float)m_szScrollOffset.cx, -(float)m_szScrollOffset.cy);
    return rect;
}

const UiPoint& RichEditData::ConvertToInternal(UiPoint& pt) const
{
    UiRect rc = m_pRichText->GetRichTextDrawRect();
    pt.Offset(-rc.left, -rc.top);
    pt.Offset(m_szScrollOffset.cx, m_szScrollOffset.cy);
    return pt;
}

const UiRect& RichEditData::ConvertToInternal(UiRect& rect) const
{
    UiRect rc = m_pRichText->GetRichTextDrawRect();
    rect.Offset(-rc.left, -rc.top);
    rect.Offset(m_szScrollOffset.cx, m_szScrollOffset.cy);
    return rect;
}

void RichEditData::SetCacheDirty(bool bDirty)
{
    m_bCacheDirty = bDirty;
}

void RichEditData::SetDrawRichTextCache(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    m_spDrawRichTextCache = spDrawRichTextCache;
    if (m_spDrawRichTextCache == nullptr) {
        m_nDrawCacheFirstLine = 0;
        m_nDrawCacheLineCount = 0;
    }
}

const std::shared_ptr<DrawRichTextCache>& RichEditData::GetDrawRichTextCache() const
{
    return m_spDrawRichTextCache;
}

void RichEditData::ClearDrawRichTextCache()
{
    m_spDrawRichTextCache.reset();
    m_nDrawCacheFirstLine = 0;
    m_nDrawCacheLineCount = 0;
}

void RichEditData::SetDrawRichTextCacheRange(size_t nFirstLine, size_t nLineCount)
{
    m_nDrawCacheFirstLine = nFirstLine;
    m_nDrawCacheLineCount = nLineCount;
}

bool RichEditData::IsDrawRichTextCacheRangeMatched(size_t nFirstLine, size_t nLineCount) const
{
    return (m_spDrawRichTextCache != nullptr) &&
           (m_nDrawCacheFirstLine == nFirstLine) &&
           (m_nDrawCacheLineCount == nLineCount);
}

void RichEditData::UpdateLineIndexData(size_t nStartLine)
{
    const size_t nLineCount = m_lineTextInfo.size();

    //行数变化后，之前的索引数据可能不完整，需要从头重新计算
    if ((m_lineRowBeginIndex.size() != (nLineCount + 1)) || (m_lineTop.size() != nLineCount)) {
        nStartLine = 0;
    }
    m_lineRowBeginIndex.resize(nLineCount + 1);
    m_lineTop.resize(nLineCount);
    if (nStartLine > nLineCount) {
        nStartLine = nLineCount;
    }
    if (nStartLine == 0) {
        m_lineRowBeginIndex[0] = 0;
    }
    //行矩形中是否已经包含垂直对齐的偏移
    const int32_t nRowOffsetY = m_bTextRectYOffsetUpdated ? GetTextRectOfssetY() : 0;
    for (size_t nLine = nStartLine; nLine < nLineCount; ++nLine) {
        const RichTextLineInfo* pLineInfo = m_lineTextInfo[nLine].get();
        size_t nRowCount = 0;
        int32_t nLineTop = (nLine > 0) ? m_lineTop[nLine - 1] : 0;
        if (pLineInfo != nullptr) {
            nRowCount = pLineInfo->m_rowInfo.size();
            if (nRowCount > 0) {
                nLineTop = (int32_t)pLineInfo->m_rowInfo[0].m_rowRect.top - nRowOffsetY;
            }
        }
        m_lineTop[nLine] = nLineTop;
        m_lineRowBeginIndex[nLine + 1] = m_lineRowBeginIndex[nLine] + (uint32_t)nRowCount;
    }
}

bool RichEditData::GetVisibleLineRange(const UiRect& rcDrawText, const UiSize& szScrollOffset, RichTextVisibleRange& range) const
{
    range = RichTextVisibleRange();
    const size_t nLineCount = m_lineTextInfo.size();
    if ((nLineCount == 0) || (m_lineTop.size() != nLineCount) || (m_lineRowBeginIndex.size() != (nLineCount + 1))) {
        //行索引数据还未准备好
        return false;
    }
    const int32_t nViewHeight = (int32_t)rcDrawText.Height();
    if (nViewHeight <= 0) {
        return false;
    }

    //上下各预取一屏，滚动时避免频繁重建绘制缓存
    const int32_t nWindowTop = szScrollOffset.cy - nViewHeight;
    const int32_t nWindowBottom = szScrollOffset.cy + 2 * nViewHeight;

    //查找第一个顶部 >= 窗口顶部的行，其前一行可能跨越窗口顶部（需要包含）
    size_t nFirstLine = (size_t)(std::lower_bound(m_lineTop.begin(), m_lineTop.end(), nWindowTop) - m_lineTop.begin());
    if (nFirstLine > 0) {
        --nFirstLine;
    }
    //查找第一个顶部 > 窗口底部的行，其前一行是最后一行需要构建的行
    size_t nLastLine = (size_t)(std::upper_bound(m_lineTop.begin(), m_lineTop.end(), nWindowBottom) - m_lineTop.begin());
    if (nLastLine > 0) {
        --nLastLine;
    }
    if (nLastLine < nFirstLine) {
        nLastLine = nFirstLine;
    }
    if (nLastLine >= nLineCount) {
        nLastLine = nLineCount - 1;
    }

    range.m_nFirstLine = nFirstLine;
    range.m_nLastLine = nLastLine;
    range.m_nFirstRowIndex = m_lineRowBeginIndex[nFirstLine];
    range.m_nFirstLineTop = m_lineTop[nFirstLine];
    range.m_nTotalRowCount = m_lineRowBeginIndex[nLineCount];

    //构建范围的内容底部坐标
    if ((nLastLine + 1) < nLineCount) {
        range.m_nBottom = m_lineTop[nLastLine + 1];
    }
    else {
        const RichTextLineInfo* pLastLineInfo = m_lineTextInfo[nLineCount - 1].get();
        const int32_t nRowOffsetY = m_bTextRectYOffsetUpdated ? GetTextRectOfssetY() : 0;
        if ((pLastLineInfo != nullptr) && !pLastLineInfo->m_rowInfo.empty()) {
            range.m_nBottom = (int32_t)pLastLineInfo->m_rowInfo.back().m_rowRect.bottom - nRowOffsetY;
        }
        else {
            range.m_nBottom = m_lineTop[nLineCount - 1] + nViewHeight;
        }
    }
    if (range.m_nBottom <= range.m_nFirstLineTop) {
        range.m_nBottom = range.m_nFirstLineTop + nViewHeight;
    }
    return true;
}

void RichEditData::SetUndoLimit(uint32_t nUndoLimit)
{
    if (m_nUndoLimit != nUndoLimit) {
        m_nUndoLimit = nUndoLimit;
        m_redoList.clear();
        while (!m_undoList.empty() && (m_undoList.size() > m_nUndoLimit)) {
            m_undoList.pop_front();
        }
    }
}

uint32_t RichEditData::GetUndoLimit() const
{
    return m_nUndoLimit;
}

void RichEditData::ClearUndoList()
{
    m_undoList.clear();
    m_redoList.clear();
    m_bLastUndoCanMerge = false;
}

void RichEditData::EmptyUndoBuffer()
{
    ClearUndoList();
}

void RichEditData::AddToUndoList(int32_t nStartChar, const DStringW& newText, const DStringW& oldText, bool bMergeUndo)
{
    ASSERT(nStartChar >= 0);
    if (nStartChar < 0) {
        return;
    }
    if (m_nUndoLimit == 0) {
        //禁止功能
        return;
    }

    //连续输入或连续删除时，与上一条撤销记录合并，避免逐字符撤销
    if (bMergeUndo && m_bLastUndoCanMerge && !m_undoList.empty()) {
        TUndoData& lastUndoData = m_undoList.back();
        const int32_t nLastStartChar = lastUndoData.m_nStartChar;
        const int32_t nLastNewTextLen = (int32_t)lastUndoData.m_newText.size();
        if (lastUndoData.m_oldText.empty() && oldText.empty()) {
            //连续插入：新文本紧接在上次插入的文本之后
            if ((newText.find_first_of(L"\r\n") == DStringW::npos) &&
                (nStartChar == nLastStartChar + nLastNewTextLen)) {
                lastUndoData.m_newText += newText;
                m_redoList.clear();
                return;
            }
        }
        else if (newText.empty() && lastUndoData.m_newText.empty()) {
            //连续删除
            if (nStartChar + (int32_t)oldText.size() == nLastStartChar) {
                //向前删除(Backspace)：撤销记录向左扩展
                lastUndoData.m_nStartChar = nStartChar;
                lastUndoData.m_oldText = oldText + lastUndoData.m_oldText;
                m_redoList.clear();
                return;
            }
            if (nStartChar == nLastStartChar) {
                //向后删除(Delete)：撤销记录向右扩展
                lastUndoData.m_oldText += oldText;
                m_redoList.clear();
                return;
            }
        }
    }

    TUndoData undoData;
    undoData.m_nStartChar = nStartChar;
    undoData.m_newText = newText;
    undoData.m_oldText = oldText;

    while (!m_undoList.empty() && (m_undoList.size() >= m_nUndoLimit)) {
        m_undoList.pop_front();
    }

    //添加到Undo列表尾部
    m_undoList.emplace_back(std::move(undoData));

    //只有纯插入（不含换行）或纯删除的记录，才允许后续记录与之合并
    m_bLastUndoCanMerge = bMergeUndo &&
        ((oldText.empty() && !newText.empty() && (newText.find_first_of(L"\r\n") == DStringW::npos)) ||
         (newText.empty() && !oldText.empty()));

    //每次添加Undo后，清空Redo列表
    m_redoList.clear();
}

bool RichEditData::CanUndo() const
{
    return !m_undoList.empty();
}

bool RichEditData::Undo(int32_t& nEndCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    nEndCharIndex = -1;
    if (m_undoList.empty()) {
        return false;
    }

    //取出Undo列表尾部的数据，但先不修改列表：
    //只有文本替换成功后才迁移到Redo列表，避免替换失败时撤销链与文本不一致
    const TUndoData undoData = m_undoList.back();
    const int32_t nStartChar = undoData.m_nStartChar;
    const int32_t nOldEndChar = nStartChar + (int32_t)undoData.m_newText.size();
    if (!ReplaceText(nStartChar, nOldEndChar, undoData.m_oldText, false, false)) {
        return false;
    }

    //替换成功，迁移Undo列表到Redo列表
    m_undoList.pop_back();
    m_redoList.push_back(undoData);
    //撤销后的输入应开启新的撤销记录，不与撤销前的记录合并
    m_bLastUndoCanMerge = false;

    nEndCharIndex = nStartChar + (int32_t)undoData.m_oldText.size();
    return true;
}

bool RichEditData::CanRedo() const
{
    return !m_redoList.empty();
}

bool RichEditData::Redo(int32_t& nEndCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    nEndCharIndex = -1;
    if (m_redoList.empty()) {
        return false;
    }

    //取出Redo列表尾部的数据，但先不修改列表：
    //只有文本替换成功后才迁移到Undo列表，避免替换失败时撤销链与文本不一致
    const TUndoData redoData = m_redoList.back();
    const int32_t nStartChar = redoData.m_nStartChar;
    const int32_t nOldEndChar = nStartChar + (int32_t)redoData.m_oldText.size();
    if (!ReplaceText(nStartChar, nOldEndChar, redoData.m_newText, false, false)) {
        return false;
    }

    //替换成功，迁移Redo列表到Undo列表
    m_redoList.pop_back();
    m_undoList.push_back(redoData);
    //重做后的输入应开启新的撤销记录，不与重做前的记录合并
    m_bLastUndoCanMerge = false;

    nEndCharIndex = nStartChar + (int32_t)redoData.m_newText.size();
    return true;
}

void RichEditData::Clear()
{
    RichTextLineInfoList lineTextInfo;
    m_lineTextInfo.swap(lineTextInfo);
    ClearDrawRichTextCache();
    //释放行索引与行顶部坐标的容量（文本清空时归还内存）
    std::vector<uint32_t> lineRowBeginIndex;
    m_lineRowBeginIndex.swap(lineRowBeginIndex);
    std::vector<int32_t> lineTop;
    m_lineTop.swap(lineTop);
    m_rcTextRect.Clear();

    std::vector<int32_t> temp;
    m_rowXOffset.swap(temp);
    m_bTextRectXOffsetUpdated = false;
    m_bTextRectYOffsetUpdated = false;

    ClearUndoList();
    SetCacheDirty(false);
}

int32_t RichEditData::GetRowCount()
{
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nRowIndex = 0; //行号
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nRowIndex += (int32_t)lineText.m_rowInfo.size();
    }
    return nRowIndex;
}

DStringW RichEditData::GetRowText(int32_t nRowIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    DStringW rowText;
    bool bFound = false;
    int32_t nRows = 0; //逻辑行号
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        const size_t nRowCount = lineText.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            if (nRows == nRowIndex) {
                //定位到本行
                ASSERT(!lineText.m_rowInfo[nRow].m_charInfo.Empty());
                size_t nStartIndex = 0;
                for (size_t i = 0; i < nRow; ++i) {
                    nStartIndex += lineText.m_rowInfo[i].m_charInfo.CharCount();
                }
                if (!lineText.m_rowInfo[nRow].m_charInfo.Empty()) {
                    //说明：文本以换行符结尾时，末尾会补一个空行（见AppendEmptyLine），
                    //该空行的字符布局中包含1个仅用于定位光标的字符，但行文本长度为0，
                    //此时起始下标等于行文本长度，属于正常情况，不做起始下标小于文本长度的断言
                    ASSERT((lineText.m_nLineTextLen == 0) || (nStartIndex < lineText.m_nLineTextLen));
                    std::wstring_view lineView = lineText.GetLineTextView();
                    if (nStartIndex < lineView.size()) {
                        rowText = lineView.substr(nStartIndex, lineText.m_rowInfo[nRow].m_charInfo.CharCount());
                    }
                }
                bFound = true;
                break;
            }
            ++nRows;
        }
        if (bFound) {
            break;
        }
    }
    return rowText;
}

int32_t RichEditData::RowIndex(int32_t nRowIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nRowStartIndex = -1;
    bool bFound = false;
    int32_t nRows = 0; //逻辑行号
    int32_t nCharCount = 0; //字符总数
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        const size_t nRowCount = lineText.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            if (nRows == nRowIndex) {
                //定位到本行
                nRowStartIndex = nCharCount;
                bFound = true;
                break;
            }
            else {
                nCharCount += (int32_t)lineText.m_rowInfo[nRow].m_charInfo.CharCount();
            }
            ++nRows;
        }
        if (bFound) {
            break;
        }
    }
    return nRowStartIndex;
}

int32_t RichEditData::RowLength(int32_t nRowIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nRowLength = 0;
    bool bFound = false;
    int32_t nRows = 0; //逻辑行号
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        const size_t nRowCount = lineText.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            if (nRows == nRowIndex) {
                //获取到本行的字符长度
                nRowLength = (int32_t)lineText.m_rowInfo[nRow].m_charInfo.CharCount();
                bFound = true;
                break;
            }
            ++nRows;
        }
        if (bFound) {
            break;
        }
    }
    return nRowLength;
}

int32_t RichEditData::RowFromChar(int32_t nCharIndex)
{
    if (nCharIndex < 0) {
        //防御负值输入（LineFromChar(-1)的既有语义是返回第0行）
        return 0;
    }
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength < 1) {
        return 0;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    int32_t nRowIndex = 0; //逻辑行号
    size_t nTextLen = 0;   //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        CheckLineTextData(m_lineTextInfo, nIndex);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            //定位到行
            const size_t nStartBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartLineOffset = (size_t)nCharIndex - nStartBaseLen;
            ASSERT(nStartLineOffset < lineText.m_nLineTextLen);
            //定位在本物理分行中，再定位在哪个逻辑分行中
            size_t nRowTextLen = 0;
            const size_t nRowCount = lineText.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                const RichTextRowInfo& rowInfo = lineText.m_rowInfo[nRow];
                nRowTextLen += rowInfo.m_charInfo.CharCount();
                if (nStartLineOffset < nRowTextLen) {
                    //定位在本逻辑分行中
                    break;
                }
                else {
                    ++nRowIndex;
                }
            }
            break;
        }
        else {
            nRowIndex += (int32_t)lineText.m_rowInfo.size();
        }
    }
    return nRowIndex;
}

void RichEditData::TruncateLimitText(DStringW& text, int32_t nLimitLen) const
{
    if (nLimitLen <= 0) {
        return;
    }
    if ((int32_t)text.size() > nLimitLen) {
        DStringW::value_type ch = text.at(nLimitLen);
        text.resize((size_t)nLimitLen);
        if ((ch == L'\n') && (text.back() == L'\r')) {
            text.pop_back();
        }
    }
}

bool RichEditData::FindRichText(bool bMatchCase, bool bMatchWholeWord, bool bFindDown,
                                int32_t nFindStartChar, int32_t nFindEndChar,
                                const DStringW& findText,
                                int32_t& nFoundStartChar, int32_t& nFoundEndChar) const
{
    if (findText.empty() || (nFindStartChar == nFindEndChar) || (nFindStartChar < 0) || (nFindEndChar < 0)){
        return false;
    }
    const int32_t nTextLen = (int32_t)GetTextLength();
    if (nFindStartChar > nTextLen) {
        nFindStartChar = nTextLen;
    }
    if (nFindEndChar > nTextLen) {
        nFindEndChar = nTextLen;
    }

    int32_t nStartChar = nFindStartChar;
    int32_t nEndChar = nFindEndChar;
    if (!bFindDown) {
        nStartChar = nFindEndChar;
        nEndChar = nFindStartChar;
    }

    DStringW text = GetTextRange(nStartChar, nEndChar);
    DStringW findTextW = findText;
    if (!bMatchCase) {
        text = StringUtil::MakeLowerString(text);
        findTextW = StringUtil::MakeLowerString(findTextW);
    }

    size_t nPos = bFindDown ? text.find(findTextW) : text.rfind(findTextW);
    if (!bMatchWholeWord) {
        //不是全字匹配，查找一次即返回
        bool bFound = (nPos != DStringW::npos) ? true : false;
        if (bFound) {
            nFoundStartChar = nStartChar + (int32_t)nPos;
            nFoundEndChar = nFoundStartChar + (int32_t)findTextW.size();
        }
        return bFound;
    }

    //全字匹配
    bool bFound = false;
    while (nPos != DStringW::npos) {
        bFound = true;
        if (iswalnum(findTextW[0])) {
            if (nPos == 0) {
                //第一个字符
                int32_t nStartCharIndex = nStartChar + (int32_t)nPos;
                if (nStartCharIndex > 0) {
                    DStringW::value_type charBeforeStart = 0;
                    DStringW temp = GetTextRange(nStartCharIndex - 1, nStartCharIndex);
                    ASSERT(temp.size() == 1);
                    if (temp.size() == 1) {
                        charBeforeStart = temp[0];
                    }
                    if (iswalnum(charBeforeStart)) {
                        bFound = false;
                    }
                }
            }
            else {
                //不是第一个字符
                if (iswalnum(text[nPos - 1])) {
                    bFound = false;
                }
            }
        }
        if (iswalnum(findTextW[findTextW.size() - 1])) {
            if ((nPos + findTextW.size()) >= text.size()) {
                //最后一个字符
                int32_t nEndCharIndex = nStartChar + (int32_t)nPos + (int32_t)findTextW.size();
                if (nEndCharIndex < nTextLen) {
                    DStringW::value_type charAfterEnd = 0;
                    DStringW temp = GetTextRange(nEndCharIndex, nEndCharIndex + 1);
                    ASSERT(temp.size() == 1);
                    if (temp.size() == 1) {
                        charAfterEnd = temp[0];
                    }
                    if (iswalnum(charAfterEnd)) {
                        bFound = false;
                    }
                }
            }
            else {
                //不是最后一个字符
                if (iswalnum(text[nPos + findTextW.size()])) {
                    bFound = false;
                }
            }            
        }
        if(bFound) {
            nFoundStartChar = nStartChar + (int32_t)nPos;
            nFoundEndChar = nFoundStartChar + (int32_t)findTextW.size();
            break;
        }
        else {
            //继续查找
            if (!bFindDown && (nPos == 0)) {
                //已经查找到字符串起始位置，未匹配到
                break;
            }
            size_t nLastPos = nPos;
            nPos = bFindDown ? text.find(findTextW, nPos + 1) : text.rfind(findTextW, nPos - 1);
            ASSERT(nLastPos != nPos);
            if (nLastPos == nPos) {
                //避免出现死循环
                break;
            }
        }
    }
    return bFound;
}

} //namespace ui
