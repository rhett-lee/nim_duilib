#include "RichEditData.h"
#include "duilib/Utils/PerformanceUtil.h"
#include <unordered_set>

namespace ui
{
RichEditData::RichEditData(IRichTextData* pRichTextData):
    m_pRichTextData(pRichTextData),
    m_hAlignType(HorAlignType::kHorAlignLeft),
    m_vAlignType(VerAlignType::kVerAlignTop),
    m_bSingleLineMode(false),
    m_pRender(nullptr),
    m_pRenderFactory(nullptr),
    m_bCacheDirty(true),
    m_nUndoLimit(64)
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

void RichEditData::SetHAlignType(HorAlignType hAlignType)
{
    if (m_hAlignType != hAlignType) {
        m_hAlignType = hAlignType;
        SetCacheDirty(true);
    }
}

void RichEditData::SetVAlignType(VerAlignType vAlignType)
{
    if (m_vAlignType != vAlignType) {
        m_vAlignType = vAlignType;
        SetCacheDirty(true);
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
    ASSERT(m_pRichTextData != nullptr);
    if (m_pRichTextData == nullptr) {
        return rect;
    }

    UiRect rcDrawRect = m_pRichTextData->GetRichTextDrawRect();
    ASSERT(rcAvailable.Width() == rcAvailable.Width());
    if (rcAvailable.Width() == rcAvailable.Width()) {
        //检查并计算字符位置
        CheckCalcTextRects();

        UiRectF rowRects;
        for (RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
            ASSERT(pLineInfo != nullptr);
            const size_t nRowCount = pLineInfo->m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                const UiRectF& rowRect = pLineInfo->m_rowInfo[nRow]->m_rowRect;
                rowRects.Union(rowRect);
            }
        }
        rect.left = (int32_t)rowRects.left;
        rect.right = (int32_t)(std::ceilf(rowRects.right));
        rect.top = (int32_t)rowRects.top;
        rect.bottom = (int32_t)(std::ceilf(rowRects.bottom));       
    }
    else {
        //重新估算
        std::vector<std::wstring_view> textView;
        GetTextView(textView);
        if (!textView.empty()) {
            std::vector<RichTextData> richTextDataList;
            if (m_pRichTextData->GetRichTextForDraw(textView, richTextDataList)) {                
                std::vector<std::vector<UiRect>> richTextRects;
                m_pRender->MeasureRichText(rcAvailable, UiSize(), m_pRenderFactory, richTextDataList, &richTextRects);
                for (const std::vector<UiRect>& data : richTextRects) {
                    for (const UiRect& textRect : data) {
                        rect.Union(textRect);
                    }
                }
            }
        }
    }
    return rect;
}

void RichEditData::CheckCalcTextRects()
{
    SetTextDrawRect(m_pRichTextData->GetRichTextDrawRect(), true);
    if (m_bCacheDirty) {
        CalcTextRects();
        SetCacheDirty(false);
    }
}

void RichEditData::CalcTextRects()
{
    PerformanceStat statPerformance(_T("RichEditData::CalcTextRects"));
    //清空所有行的缓存数据
    for (RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
        ASSERT(pLineInfo != nullptr);
        pLineInfo->m_rowInfo.clear();
    }

    ASSERT(m_pRender != nullptr);
    if (m_pRender == nullptr) {
        return;
    }
    ASSERT(m_pRenderFactory != nullptr);
    if (m_pRenderFactory == nullptr) {
        return;
    }
    ASSERT(m_pRichTextData != nullptr);
    if (m_pRichTextData == nullptr) {
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

    UiRect rcDrawText = m_pRichTextData->GetRichTextDrawRect();    
    if (rcDrawText.IsEmpty()) {
        return;
    }
    //估算的时候，滚动条位置始终为(0,0)
    UiSize szScrollOffset;
    RichTextLineInfoParam lineInfoParam;
    lineInfoParam.m_nStartLineIndex = 0;
    lineInfoParam.m_nStartRowIndex = 0;
    lineInfoParam.m_pLineInfoList = &m_lineTextInfo;

    //绘制所有数据，清空行数据信息
    std::vector<RichTextData> richTextDataList;
    m_pRichTextData->GetRichTextForDraw(textView, richTextDataList);
    if (richTextDataList.empty()) {
        return;
    }
    m_spDrawRichTextCache.reset();
    m_pRender->MeasureRichText3(rcDrawText, szScrollOffset, m_pRenderFactory, richTextDataList, &lineInfoParam, m_spDrawRichTextCache, nullptr);
    SetTextDrawRect(rcDrawText, false);
}

void RichEditData::CalcTextRects(size_t nStartLine,
                                 const std::vector<size_t>& modifiedLines,
                                 const std::vector<size_t>& deletedLines,
                                 size_t nDeletedRows)
{
    PerformanceStat statPerformance(_T("RichEditData::CalcTextRects2"));
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
    ASSERT(nStartLine < m_lineTextInfo.size());
    if (nStartLine >= m_lineTextInfo.size()) {
        return;
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
    ASSERT(m_pRichTextData != nullptr);
    if (m_pRichTextData == nullptr) {
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

    UiRect rcDrawText = m_pRichTextData->GetRichTextDrawRect();
    if (rcDrawText.IsEmpty()) {
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

    //当前最新的待绘制数据
    std::vector<RichTextData> richTextDataListAll;
    if (m_spDrawRichTextCache != nullptr) {
        //比较绘制缓存是否失效，如果失效则清空
        m_pRichTextData->GetRichTextForDraw(textView, richTextDataListAll);
    }
    //修改后的文本，重新生成的绘制缓存
    std::shared_ptr<DrawRichTextCache> spDrawRichTextCacheUpdated;


    size_t nModifiedRows = 0;//修改后的文本，计算后切分为几行（逻辑行）
    if (!modifiedLines.empty()) {
        //有修改的行，重新计算行数据
        std::vector<RichTextData> richTextDataListModified;
        m_pRichTextData->GetRichTextForDraw(textView, richTextDataListModified, nStartLine, modifiedLines);
        if (richTextDataListModified.empty()) {
            return;
        }        
        m_pRender->MeasureRichText3(rcDrawText, szScrollOffset, m_pRenderFactory, richTextDataListModified, &lineInfoParam, spDrawRichTextCacheUpdated, nullptr);
        std::unordered_set<uint32_t> modifiedLineSet;
        for (size_t nLine : modifiedLines) {
            modifiedLineSet.insert((uint32_t)nLine);
        }
        const size_t nLineCount = m_lineTextInfo.size();
        for (size_t nLine = 0; nLine < nLineCount; ++nLine) {
            if (modifiedLineSet.find(nLine) != modifiedLineSet.end()) {
                const RichTextLineInfo& lineInfo = *m_lineTextInfo[nLine];
                nModifiedRows += (uint32_t)lineInfo.m_rowInfo.size();
            }
        }
    }

    //绘制后，增量绘制后的行高数据           
    UpdateRowInfo(nStartLine);

    //更新绘制缓存
    if (m_spDrawRichTextCache != nullptr) {
        std::unordered_map<uint32_t, int32_t> rowTopMap;
        uint32_t nRowIndex = 0;
        const size_t nLineCount = m_lineTextInfo.size();
        for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
            const RichTextLineInfo& lineInfo = *m_lineTextInfo[nLineIndex];
            const size_t nRowCount = lineInfo.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                rowTopMap[nRowIndex] = (int32_t)lineInfo.m_rowInfo[nRow]->m_rowRect.top;
                ++nRowIndex;
            }
        }

        if (!m_pRender->UpdateDrawRichTextCache(m_spDrawRichTextCache, spDrawRichTextCacheUpdated, richTextDataListAll,
                                                nStartLine, modifiedLines, nModifiedRows, deletedLines, nDeletedRows, rowTopMap)) {
            m_spDrawRichTextCache.reset();
        }
        //该数据已经交还给缓存，不能再使用
        richTextDataListAll.clear();
    }
    
#ifdef _DEBUG
    //比较与完整绘制时是否一致
    if (nStartLine != (size_t)-1) {
        std::vector<std::wstring_view> textView2;
        RichTextLineInfoList lineTextInfoList;
        for (RichTextLineInfoPtr& pLineInfo : m_lineTextInfo) {
            ASSERT(pLineInfo != nullptr);
            RichTextLineInfoPtr spLineInfo(new RichTextLineInfo);
            spLineInfo->m_nLineTextLen = pLineInfo->m_nLineTextLen;
            spLineInfo->m_lineText = pLineInfo->m_lineText;
            textView2.push_back(std::wstring_view(spLineInfo->m_lineText.c_str(), spLineInfo->m_nLineTextLen));
            lineTextInfoList.push_back(spLineInfo);
        }
        std::vector<RichTextData> richTextDataList2;
        m_pRichTextData->GetRichTextForDraw(textView2, richTextDataList2);

        std::shared_ptr<DrawRichTextCache> spDrawRichTextCacheNew;

        RichTextLineInfoParam lineInfoParam2;
        lineInfoParam2.m_pLineInfoList = &lineTextInfoList;
        lineInfoParam2.m_nStartLineIndex = 0;
        lineInfoParam2.m_nStartRowIndex = 0;
        m_pRender->MeasureRichText3(rcDrawText, szScrollOffset, m_pRenderFactory, richTextDataList2, &lineInfoParam2, spDrawRichTextCacheNew, nullptr);

        //比较数据的一致性，增量绘制的结果，应该与完整绘制的结果相同
        ASSERT(lineTextInfoList.size() == m_lineTextInfo.size());
        const size_t nDataCount = lineTextInfoList.size();
        for (size_t nDataIndex = 0; nDataIndex < nDataCount; ++nDataIndex) {
            const RichTextLineInfo& infoOld = *m_lineTextInfo[nDataIndex];
            const RichTextLineInfo& infoNew = *lineTextInfoList[nDataIndex];
            ASSERT(infoOld.m_lineText == infoNew.m_lineText);
            ASSERT(infoOld.m_nLineTextLen == infoNew.m_nLineTextLen);
            ASSERT(infoOld.m_rowInfo.size() == infoNew.m_rowInfo.size());

            const size_t nRowCount = infoOld.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                const RichTextRowInfo& rowOld = *infoOld.m_rowInfo[nRow];
                const RichTextRowInfo& rowNew = *infoNew.m_rowInfo[nRow];
                ASSERT(rowOld.m_rowRect == rowNew.m_rowRect);
                ASSERT(rowOld.m_charInfo.size() == rowNew.m_charInfo.size());
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
}

bool RichEditData::SetText(const DStringW& text)
{
    PerformanceStat statPerformance(_T("RichEditData::SetText"));
    if (text.empty()) {
        Clear();
        return true;
    }
    std::vector<std::wstring_view> lineTextViewList;
    std::wstring_view textView = text;
    if (m_bSingleLineMode) {
        //单行文本模式
        lineTextViewList.push_back(textView);
    }
    else {
        //多行文本模式
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
                if (std::wstring_view(m_lineTextInfo[nIndex]->m_lineText.c_str(), m_lineTextInfo[nIndex]->m_nLineTextLen) != lineTextViewList[nIndex]) {
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
            lineTextInfo.resize(nLineCount);
            for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
                const std::wstring_view& lineTextView = lineTextViewList[nIndex];
                RichTextLineInfoPtr& lineText = lineTextInfo[nIndex];
                lineText.reset(new RichTextLineInfo);
                lineText->m_lineText = lineTextView; //文本数据复制一份，保存起来
                lineText->m_nLineTextLen = (uint32_t)lineTextView.size();
                ASSERT(lineText->m_nLineTextLen > 0);
            }
        }
        m_lineTextInfo.swap(lineTextInfo);
        SetCacheDirty(true);
        ClearUndoList();
    }
    return bTextChanged;
}

void RichEditData::SplitLines(const std::wstring_view& textView, std::vector<std::wstring_view>& lineTextViewList)
{
    if (textView.empty()) {
        return;
    }
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
        ASSERT(lineText.m_nLineTextLen > 0);
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
        ASSERT(lineText.m_nLineTextLen > 0);
        nTextLen += lineText.m_nLineTextLen;
    }
    return nTextLen;
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
    ASSERT((nStartChar >= 0) && (nEndChar >= 0) && (nEndChar >= nStartChar));
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar > nEndChar)) {
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
        ASSERT(lineText.m_nLineTextLen > 0);
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
    }
    if ((nStartLine != nNotFound) && (nEndLine != nNotFound) &&
        (nStartCharLineOffset != nNotFound) && (nEndCharLineOffset != nNotFound)) {
        ASSERT(nEndLine >= nStartLine);
        return true;
    }
    return false;
}

bool RichEditData::ReplaceText(int32_t nStartChar, int32_t nEndChar, const DStringW& text, bool bCanUndo)
{
    PerformanceStat statPerformance(_T("RichEditData::ReplaceText"));
    ASSERT((nStartChar >= 0) && (nEndChar >= 0) && (nEndChar >= nStartChar));
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar > nEndChar)) {
        return false;
    }

    //检查并计算字符位置
    CheckCalcTextRects();

    constexpr const size_t nNotFound = (size_t)-1;
    size_t nStartLine = nNotFound;              //起始行
    size_t nEndLine = nNotFound;                //结束行
    size_t nStartCharLineOffset = nNotFound;    //在起始行中，开始字符的偏移量
    size_t nEndCharLineOffset = nNotFound;      //在结束行中，结束字符的偏移量
    if (!FindLineTextPos(nStartChar, nEndChar, nStartLine, nEndLine, nStartCharLineOffset, nEndCharLineOffset)) {
        return false;
    }

    DStringW oldText; //旧文本内容
    const int32_t nNewStartChar = nStartChar; //新文本的起始下标值
    const int32_t nNewEndChar = nStartChar + (int32_t)text.size(); //新文本的结束下标值

    //是否需要记录撤销操作
    if (m_nUndoLimit <= 0) {
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
        const RichTextLineInfo& lineText = *m_lineTextInfo[nStartLine];
        std::wstring_view textView(lineText.m_lineText.c_str(), lineText.m_nLineTextLen);
        startLineTextView = textView.substr(0, nStartCharLineOffset); //保留到行首的文本
        endLineTextView = textView.substr(nEndCharLineOffset);        //保留到行尾的文本
    }
    else if (nEndLine > nStartLine) {
        //在不同行
        for (size_t nIndex = nStartLine; nIndex <= nEndLine; ++nIndex) {
            const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
            std::wstring_view textView(lineText.m_lineText.c_str(), lineText.m_nLineTextLen);
            if (nIndex == nStartLine) {
                //首行，保留到行首的文本
                startLineTextView = textView.substr(0, nStartCharLineOffset);
            }
            else if (nIndex == nEndLine) {
                //末行，保留到行尾的文本
                endLineTextView = textView.substr(nEndCharLineOffset);
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
    //删除了几行
    size_t nDeletedRows = 0;
    //倒序删除
    if (!deletedLines.empty()) {
        int32_t nDelIndex = (int32_t)deletedLines.size() - 1;
        for (; nDelIndex >= 0; --nDelIndex) {
            if (deletedLines[nDelIndex] < m_lineTextInfo.size()) {
                nDeletedRows += m_lineTextInfo[deletedLines[nDelIndex]]->m_rowInfo.size();
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
            ASSERT(textView.back() == L'\n');
            //插入新行
            RichTextLineInfoPtr lineTextInfo(new RichTextLineInfo);
            lineTextInfo->m_lineText = textView.data();
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
        CalcTextRects(nStartLine, modifiedLines, deletedLines, nDeletedRows);
    }
    if (bCanUndo) {
        //生成撤销列表
        AddToUndoList(nStartChar, text, oldText);
    }
    else {
        ClearUndoList();
    }   
    return true;
}

DStringW RichEditData::GetTextRange(int32_t nStartChar, int32_t nEndChar)
{
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar >= nEndChar)) {
        return DStringW();
    }
    //检查并计算字符位置
    CheckCalcTextRects();

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
        DStringW newText = lineText.m_lineText.c_str();
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
            newText = lineText.m_lineText.c_str();
            if (nIndex == nStartLine) {
                //首行, 选择到行尾
                selText = newText.substr(nStartCharLineOffset);
            }
            else if (nIndex == nEndLine) {
                //末行，选择到行首
                newText = lineText.m_lineText.c_str();
                if (nEndCharLineOffset > 0) {
                    selText += newText.substr(0, nEndCharLineOffset);
                }                    
            }
            else {
                //中间行
                selText += lineText.m_lineText.c_str();
            }
        }
    }
    return selText;
}

bool RichEditData::HasTextRange(int32_t nStartChar, int32_t nEndChar)
{
    if ((nStartChar < 0) || (nEndChar < 0) || (nStartChar >= nEndChar)) {
        return false;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

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

bool RichEditData::CanUndo() const
{
    //TODO
    return 0;
}

bool RichEditData::Undo()
{
    //TODO
    return 0;
}

void RichEditData::SetUndoLimit(int32_t nUndoLimit)
{
    if (nUndoLimit < 0) {
        nUndoLimit = 0;
    }
    if (m_nUndoLimit != nUndoLimit) {
        m_nUndoLimit = nUndoLimit;
        //TODO
    }
}

int32_t RichEditData::GetUndoLimit() const
{
    return m_nUndoLimit;
}

bool RichEditData::CanRedo() const
{
    //TODO
    return 0;
}

bool RichEditData::Redo()
{
    //TODO
    return 0;
}

void RichEditData::Clear()
{
    RichTextLineInfoList lineTextInfo;
    m_lineTextInfo.swap(lineTextInfo);
    m_spDrawRichTextCache.reset();
    SetCacheDirty(true);
}

int32_t RichEditData::GetRowCount() const
{
    //TODO
    return 0;
}

DString RichEditData::GetRowText(int32_t nRowIndex, int32_t nMaxLength) const
{
    //TODO
    return DString();
}

int32_t RichEditData::RowIndex(int32_t nRowIndex) const
{
    //TODO
    return 0;
}

int32_t RichEditData::RowLength(int32_t nRowIndex) const
{
    //TODO
    return 0;
}

int32_t RichEditData::RowFromChar(int32_t nCharIndex) const
{
    //TODO
    return 0;
}

bool RichEditData::GetCharLineRowIndex(int32_t nCharIndex, size_t& nLineNumber, size_t& nLineRowIndex, size_t& nStartCharRowOffset) const
{
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
        ASSERT(lineTextInfo.m_nLineTextLen > 0);
        nTextLen += lineTextInfo.m_nLineTextLen;
        if (nCharIndex < nTextLen) {
            const size_t nStartBaseLen = nTextLen - lineTextInfo.m_nLineTextLen;
            const size_t nStartLineOffset = (size_t)nCharIndex - nStartBaseLen;
            ASSERT(nStartLineOffset < lineTextInfo.m_nLineTextLen);
            //定位在本物理分行中，再定位在哪个逻辑分行中
            size_t nRowTextLen = 0;
            const size_t nRowCount = lineTextInfo.m_rowInfo.size();
            for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
                ASSERT(lineTextInfo.m_rowInfo[nRow] != nullptr);
                const RichTextRowInfo& rowInfo = *lineTextInfo.m_rowInfo[nRow];
                nRowTextLen += rowInfo.m_charInfo.size();
                if (nStartLineOffset < nRowTextLen) {
                    //定位在本逻辑分行中
                    const size_t nStartCharBaseLen = nRowTextLen - rowInfo.m_charInfo.size();
                    bFound = true;
                    nStartCharRowOffset = (size_t)nStartLineOffset - nStartCharBaseLen;                    
                    nLineNumber = nLineIndex;
                    nLineRowIndex = nRow;
                    break;
                }
            }
            break;
        }
    }
    return bFound;
}

RichTextRowInfoPtr RichEditData::GetRowInfoFromPoint(const UiPoint& pt) const
{
    RichTextRowInfoPtr spRowInfo;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            ASSERT(lineTextInfo.m_rowInfo[nRow] != nullptr);
            const RichTextRowInfo& rowInfo = *lineTextInfo.m_rowInfo[nRow];
            const UiRectF& rowRect = rowInfo.m_rowRect;
            if ((pt.y >= rowRect.top) && (pt.y < rowRect.bottom)) {
                spRowInfo = lineTextInfo.m_rowInfo[nRow];
                break;
            }
        }
        if (spRowInfo != nullptr) {
            break;
        }
    }
    return spRowInfo;
}

RichTextRowInfoPtr RichEditData::GetCharRowInfo(int32_t nCharIndex, size_t& nStartCharRowOffset) const
{
    size_t nLineNumber = 0;
    size_t nLineRowIndex = 0;
    RichTextRowInfoPtr spRowInfo;
    if (GetCharLineRowIndex(nCharIndex, nLineNumber, nLineRowIndex, nStartCharRowOffset)) {
        const RichTextLineInfo& lineTextInfo = *m_lineTextInfo[nLineNumber];
        spRowInfo = lineTextInfo.m_rowInfo[nLineRowIndex];
    }
    return spRowInfo;
}

RichTextRowInfoPtr RichEditData::GetFirstRowInfo() const
{
    RichTextRowInfoPtr spRowInfo;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    if (!lineTextInfoList.empty()) {
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[0];
        ASSERT(!lineTextInfo.m_rowInfo.empty());
        if (!lineTextInfo.m_rowInfo.empty()) {
            spRowInfo = lineTextInfo.m_rowInfo[0];
        }
    }
    return spRowInfo;
}

RichTextRowInfoPtr RichEditData::GetLastRowInfo() const
{
    RichTextRowInfoPtr spRowInfo;
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    if (nLineCount != 0) {
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineCount - 1];
        ASSERT(!lineTextInfo.m_rowInfo.empty());
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        if (nRowCount != 0) {
            spRowInfo = lineTextInfo.m_rowInfo[nRowCount - 1];
        }
    }
    return spRowInfo;
}

size_t RichEditData::GetRowInfoStartIndex(const RichTextRowInfoPtr& spRowInfo) const
{
    size_t nStartIndex = (size_t)-1;
    size_t nTextLen = 0; //文本总长度
    const RichTextLineInfoList& lineTextInfoList = m_lineTextInfo;
    const size_t nLineCount = lineTextInfoList.size();
    for (size_t nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex) {
        ASSERT(lineTextInfoList[nLineIndex] != nullptr);
        const RichTextLineInfo& lineTextInfo = *lineTextInfoList[nLineIndex];
        ASSERT(lineTextInfo.m_nLineTextLen > 0);

        size_t nRowTextLen = 0;
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            ASSERT(lineTextInfo.m_rowInfo[nRow] != nullptr);
            const RichTextRowInfo& rowInfo = *lineTextInfo.m_rowInfo[nRow];            
            if (lineTextInfo.m_rowInfo[nRow] == spRowInfo) {
                //找到此行
                nStartIndex = nTextLen + nRowTextLen;
                break;
            }
            nRowTextLen += rowInfo.m_charInfo.size();
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
        ASSERT(nLineRowCount > 0);
        for (size_t nLineRowIndex = 0; nLineRowIndex < nLineRowCount; ++nLineRowIndex) {
            ASSERT(lineTextInfoList[nLineIndex]->m_rowInfo[nLineRowIndex] != nullptr);
            if (lineTextInfoList[nLineIndex]->m_rowInfo[nLineRowIndex] == nullptr) {
                continue;
            }
            UiRectF& rowRect = lineTextInfoList[nLineIndex]->m_rowInfo[nLineRowIndex]->m_rowRect;
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

UiPoint RichEditData::CaretPosFromChar(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiPoint cursorPos;
    if (nCharIndex < 0) {
        cursorPos.x = 0;
        cursorPos.y = 0;
    }
    else {
        size_t nStartCharRowOffset = 0;
        RichTextRowInfoPtr spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
        if (spRowInfo != nullptr) {
            const RichTextRowInfo& rowInfo = *spRowInfo;
            const size_t nIndexOffset = nStartCharRowOffset;
            float xPos = rowInfo.m_rowRect.left;//左上角坐标
            cursorPos.y = (int32_t)rowInfo.m_rowRect.top;
            for (size_t i = 0; i < nIndexOffset; ++i) {
                const RichTextCharInfo& charInfo = rowInfo.m_charInfo[i];
                xPos += charInfo.CharWidth();
            }
            cursorPos.x = (int32_t)xPos;
        }
        
        else {
            //取最后一个字符的右上角坐标
            spRowInfo = GetLastRowInfo();
            if (spRowInfo != nullptr) {
                const RichTextRowInfo& rowInfo = *spRowInfo;
                float xPos = rowInfo.m_rowRect.left;
                cursorPos.y = (int32_t)rowInfo.m_rowRect.top;
                const size_t nCharCount = rowInfo.m_charInfo.size();
                for (size_t i = 0; i < nCharCount; ++i) {
                    const RichTextCharInfo& charInfo = rowInfo.m_charInfo[i];
                    xPos += charInfo.CharWidth();//右上角坐标
                }
                cursorPos.x = (int32_t)std::ceilf(xPos);
            }
        }
    }

    //转换为外部坐标
    ConvertToExternal(cursorPos);
    return cursorPos;
}

UiPoint RichEditData::PosFromChar(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiPoint pt;
    if (nCharIndex < 0) {
        pt.x = 0;
        pt.y = 0;
    }
    else {     
        size_t nStartCharRowOffset = 0;
        RichTextRowInfoPtr spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);
        if (spRowInfo != nullptr) {
            const RichTextRowInfo& rowInfo = *spRowInfo;
            const size_t nIndexOffset = nStartCharRowOffset;
            float xPos = rowInfo.m_rowRect.left;//左上角坐标
            pt.y = (int32_t)rowInfo.m_rowRect.top;
            for (size_t i = 0; i <= nIndexOffset; ++i) {
                const RichTextCharInfo& charInfo = rowInfo.m_charInfo[i];
                xPos += charInfo.CharWidth();
            }
            pt.x = (int32_t)xPos;
        }
        else {
            //取最后一个字符的左上角坐标
            spRowInfo = GetLastRowInfo();
            if (spRowInfo != nullptr) {
                const RichTextRowInfo& rowInfo = *spRowInfo;
                float xPos = rowInfo.m_rowRect.left;
                pt.y = (int32_t)rowInfo.m_rowRect.top;
                if (!rowInfo.m_charInfo.empty()) {
                    const size_t nIndexOffset = rowInfo.m_charInfo.size() - 1;
                    for (size_t i = 0; i < nIndexOffset; ++i) {
                        const RichTextCharInfo& charInfo = rowInfo.m_charInfo[i];
                        xPos += charInfo.CharWidth();//左上角坐标
                    }
                }
                pt.x = (int32_t)xPos;
            }
        }
    }

    //转换为外部坐标
    ConvertToExternal(pt);
    return pt;
}

int32_t RichEditData::CharFromPos(UiPoint pt)
{
    const int32_t nTextLength = (int32_t)GetTextLength();
    if (nTextLength == 0) {
        return 0;
    }

    //检查并计算字符位置
    CheckCalcTextRects();

    //转换为内部坐标
    ConvertToInternal(pt);

    //横向按字符边界对齐，纵向按行高对齐
    int32_t nCharPosIndex = -1;
    RichTextRowInfoPtr spDestRow;
    RichTextRowInfoPtr spFirstRow = GetFirstRowInfo();
    if (spFirstRow != nullptr) {
        const UiRectF& rowRect = spFirstRow->m_rowRect;
        if (pt.y < rowRect.top) {
            //该点在区域上方，定位到第一行
            spDestRow = spFirstRow;
        }
    }
    if (spDestRow == nullptr) {
        RichTextRowInfoPtr spLastRow = GetLastRowInfo();
        const UiRectF& rowRect = spLastRow->m_rowRect;
        if (pt.y >= rowRect.bottom) {
            //该点在区域下方，定位到最后一行
            spDestRow = spLastRow;
        }
    }

    if (spDestRow == nullptr) {
        spDestRow = GetRowInfoFromPoint(pt);        
    }
    ASSERT(spDestRow != nullptr);
    if (spDestRow != nullptr) {
        const RichTextRowInfo& rowInfo = *spDestRow;
        const size_t nCharCount = rowInfo.m_charInfo.size();
        ASSERT(!rowInfo.m_charInfo.empty());

        if (pt.x <= rowInfo.m_rowRect.left) {
            //该点在本行的左侧，指向本行的首字符
            nCharPosIndex = (int32_t)GetRowInfoStartIndex(spDestRow);
        }
        else if (pt.x >= rowInfo.m_rowRect.right) {
            //该点在本行的右侧，指向本行的尾字符
            if ((nCharCount >= 2) && rowInfo.m_charInfo[nCharCount - 1].IsNewLine() && rowInfo.m_charInfo[nCharCount - 2].IsReturn()){
                //该行以回车+换行结尾: 指向回车字符
                nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.size() - 2);
            }
            else {
                nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.size() - 1);
            }
        }
        else if ((nCharCount == 2) && rowInfo.m_charInfo[nCharCount - 1].IsNewLine() && rowInfo.m_charInfo[nCharCount - 2].IsReturn()) {
            //本行为空行，只有一个回车+换行: 指向回车字符
            nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + rowInfo.m_charInfo.size() - 2);
        }
        else if (nCharCount == 1) {
            //该行只有一个字符
            nCharPosIndex = (int32_t)GetRowInfoStartIndex(spDestRow);
        }
        else {
            float xRowPos = rowInfo.m_rowRect.left;
            const size_t nCount = rowInfo.m_charInfo.size();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                const RichTextCharInfo& charInfo = rowInfo.m_charInfo[nIndex];
                if (charInfo.IsIgnoredChar()) {
                    continue;
                }
                if ((pt.x >= xRowPos) && (pt.x < (xRowPos + charInfo.CharWidth()))) {
                    if (pt.x <= (xRowPos + charInfo.CharWidth() / 2)) {
                        //如果X坐标小于等于中心点，取当前字符
                        nCharPosIndex = (int32_t)(GetRowInfoStartIndex(spDestRow) + nIndex);
                    }
                    else {
                        //如果X坐标大于中心点，则取下一个字符
                        for (size_t i = nIndex + 1; i < nCount; ++i) {
                            const RichTextCharInfo& nextCharInfo = rowInfo.m_charInfo[i];
                            if (nextCharInfo.IsLowSurrogate()) {
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
                xRowPos += charInfo.CharWidth();
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

#define SkUTF16_IsHighSurrogate(c)  (((c) & 0xFC00) == 0xD800)
#define SkUTF16_IsLowSurrogate(c)   (((c) & 0xFC00) == 0xDC00)

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

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        ASSERT(lineText.m_nLineTextLen > 0);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找
            size_t i = nStartCharLineOffset + 1;
            while ( i < lineText.m_nLineTextLen) {
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.c_str() + i);
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

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        ASSERT(lineText.m_nLineTextLen > 0);
        nTextLen += lineText.m_nLineTextLen;
        if (nCharIndex < (int32_t)nTextLen) {
            const size_t nStartCharBaseLen = nTextLen - lineText.m_nLineTextLen;
            const size_t nStartCharLineOffset = (size_t)nCharIndex - nStartCharBaseLen;
            ASSERT(nStartCharLineOffset < lineText.m_nLineTextLen);
            //在本行中寻找
            int32_t i = (int32_t)nStartCharLineOffset - 1;
            while (i >= 0) {
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.c_str() + i);
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
            }
            if ((nNewCharIndex == nCharIndex) && (i <= 0) && (nIndex >= 1)) {
                //已经在行首，跳到前一行的最后一个字符
                const RichTextLineInfo& prevLineText = *m_lineTextInfo[nIndex - 1];
                ASSERT(prevLineText.m_nLineTextLen > 0);
                if (prevLineText.m_nLineTextLen > 1) {
                    ASSERT(prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 1] == L'\n');
                    nNewCharIndex = nCharIndex - 2; //跳过最后一个'\n'字符
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

bool RichEditData::IsSeperatorChar(wchar_t ch) const
{
    static const DStringW sep = L"`~!@#$%^&*()-=+\t[]{}|\\;:'\"\r\n,<.>/?·！￥…、，。《》？“”；：‘’（）【】";
    return sep.find(ch) != DStringW::npos;
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

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        ASSERT(lineText.m_nLineTextLen > 0);
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
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.c_str() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    i += 2;//跳过该双字节字符
                }
                else {
                    i += 1;//跳过该字符
                }
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

    int32_t nNewCharIndex = nCharIndex;
    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        ASSERT(lineText.m_nLineTextLen > 0);
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

                if (i > 0) {
                    const uint16_t* src = (const uint16_t*)(lineText.m_lineText.c_str() + i);
                    if (SkUTF16_IsLowSurrogate(*src)) {
                        i -= 1;//跳过低代理字符
                    }
                }

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
                ASSERT(prevLineText.m_nLineTextLen > 0);
                if (prevLineText.m_nLineTextLen > 1) {
                    ASSERT(prevLineText.m_lineText.data()[prevLineText.m_nLineTextLen - 1] == L'\n');
                    nNewCharIndex = nCharIndex - 2; //跳过最后一个'\n'字符
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

    size_t nTextLen = 0; //文本总长度
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        const RichTextLineInfo& lineText = *m_lineTextInfo[nIndex];
        ASSERT(lineText.m_nLineTextLen > 0);
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
                const uint16_t* src = (const uint16_t*)(lineText.m_lineText.c_str() + i);
                if (SkUTF16_IsHighSurrogate(*src)) {
                    ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
                    i += 2;//跳过该双字节字符
                }
                else {
                    i += 1;//跳过该字符
                }
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

UiRect RichEditData::GetCharRowRect(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    size_t nStartCharRowOffset = 0;
    RichTextRowInfoPtr spRowInfo = GetCharRowInfo(nCharIndex, nStartCharRowOffset);

    UiRect rowRect;    
    if (spRowInfo != nullptr) {
        const RichTextRowInfo& rowInfo = *spRowInfo;
        const UiRectF& rowRectF = rowInfo.m_rowRect;
        UiRect rc = m_pRichTextData->GetRichTextDrawRect();
        rowRect.left = 0;
        rowRect.right = rc.Width();
        rowRect.top = (int32_t)rowRectF.top;
        rowRect.bottom = (int32_t)std::ceilf(rowRectF.bottom);
    }
    
    //转换为外部坐标
    ConvertToExternal(rowRect);
    return rowRect;
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
        ASSERT(lineTextInfo.m_nLineTextLen > 0);
        nRowTextLen = 0;
        const size_t nRowCount = lineTextInfo.m_rowInfo.size();
        for (size_t nRow = 0; nRow < nRowCount; ++nRow) {
            ASSERT(lineTextInfo.m_rowInfo[nRow] != nullptr);

            nRowStartCharIndex = nTextLen + nRowTextLen;
            const RichTextRowInfo& rowInfo = *lineTextInfo.m_rowInfo[nRow];

            nEndRowIndex = (int32_t)(nRowStartCharIndex + rowInfo.m_charInfo.size());
            bool bFirstLine = (nStartChar >= (int32_t)nRowStartCharIndex) && (nStartChar < nEndRowIndex);
            bool bLastLine = (nEndChar >= (int32_t)nRowStartCharIndex) && (nEndChar < nEndRowIndex);

            if (bFirstLine && bLastLine) {
                //首行和尾行是同一行
                UiRectF rowRectF = rowInfo.m_rowRect;
                rowRectF.right = rowRectF.left;
                const size_t nStartCharIndex = (size_t)nStartChar - nRowStartCharIndex;
                const size_t nEndCharIndex = (size_t)nEndChar - nRowStartCharIndex;
                for (size_t i = 0; i < nEndCharIndex; ++i) {
                    if (rowInfo.m_charInfo[i].IsIgnoredChar() || rowInfo.m_charInfo[i].IsNewLine()) {
                        continue;
                    }
                    if (i < nStartCharIndex) {
                        rowRectF.left += rowInfo.m_charInfo[i].CharWidth();
                        rowRectF.right = rowRectF.left;
                    }
                    else {
                        rowRectF.right += rowInfo.m_charInfo[i].CharWidth();
                    }
                }
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    destRowRect.Union(rowRectF);
                }
                bEnd = true;
                break;
            }
            else if (bFirstLine) {
                //首行: 选择到行尾
                nStartRowIndex = nCurrentRowIndex;
                UiRectF rowRectF = rowInfo.m_rowRect;
                rowRectF.right = rowRectF.left;
                const size_t nStartCharIndex = (size_t)nStartChar - nRowStartCharIndex;
                const size_t nEndCharIndex = rowInfo.m_charInfo.size();
                for (size_t i = 0; i < nEndCharIndex; ++i) {
                    if (rowInfo.m_charInfo[i].IsIgnoredChar() || rowInfo.m_charInfo[i].IsNewLine()) {
                        continue;
                    }
                    if (i < nStartCharIndex) {
                        rowRectF.left += rowInfo.m_charInfo[i].CharWidth();
                        rowRectF.right = rowRectF.left;
                    }
                    else {
                        rowRectF.right += rowInfo.m_charInfo[i].CharWidth();
                    }
                }
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    destRowRect.Union(rowRectF);
                }
            }
            else if (bLastLine) {
                //尾行：选择到行首
                UiRectF rowRectF = rowInfo.m_rowRect;
                rowRectF.right = rowRectF.left;
                const size_t nEndCharIndex = (size_t)nEndChar - nRowStartCharIndex;
                for (size_t i = 0; i < nEndCharIndex; ++i) {
                    if (rowInfo.m_charInfo[i].IsIgnoredChar() || rowInfo.m_charInfo[i].IsNewLine()) {
                        continue;
                    }
                    rowRectF.right += rowInfo.m_charInfo[i].CharWidth();
                }
                UiRectF& destRowRect = rowTextRectFs[nCurrentRowIndex];
                if (destRowRect.IsZero()) {
                    destRowRect = rowRectF;
                }
                else {
                    destRowRect.Union(rowRectF);
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
                    destRowRect.Union(rowRectF);
                }
            }            

            nRowTextLen += rowInfo.m_charInfo.size();
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
    UiRect rc = m_pRichTextData->GetRichTextDrawRect();
    pt.Offset(rc.left, rc.top);
    pt.Offset(-m_szScrollOffset.cx, -m_szScrollOffset.cy);
    return pt;
}

const UiRect& RichEditData::ConvertToExternal(UiRect& rect) const
{
    UiRect rc = m_pRichTextData->GetRichTextDrawRect();
    rect.Offset(rc.left, rc.top);
    rect.Offset(-m_szScrollOffset.cx, -m_szScrollOffset.cy);
    return rect;
}

const UiRectF& RichEditData::ConvertToExternal(UiRectF& rect) const
{
    UiRect rc = m_pRichTextData->GetRichTextDrawRect();
    rect.Offset((float)rc.left, (float)rc.top);
    rect.Offset(-(float)m_szScrollOffset.cx, -(float)m_szScrollOffset.cy);
    return rect;
}

const UiPoint& RichEditData::ConvertToInternal(UiPoint& pt) const
{
    UiRect rc = m_pRichTextData->GetRichTextDrawRect();
    pt.Offset(-rc.left, -rc.top);
    pt.Offset(m_szScrollOffset.cx, m_szScrollOffset.cy);
    return pt;
}

const UiRect& RichEditData::ConvertToInternal(UiRect& rect) const
{
    UiRect rc = m_pRichTextData->GetRichTextDrawRect();
    rect.Offset(-rc.left, -rc.top);
    rect.Offset(m_szScrollOffset.cx, m_szScrollOffset.cy);
    return rect;
}

void RichEditData::SetCacheDirty(bool bDirty)
{
    m_bCacheDirty = bDirty;
}

void RichEditData::ClearUndoList()
{

}

void RichEditData::AddToUndoList(int32_t nStartChar, const DStringW& newText, const DStringW& oldText)
{
    //还原操作为:
    //ReplaceText(nStartChar, nStartChar + newText.size(), oldText);
}

void RichEditData::SetDrawRichTextCache(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    m_spDrawRichTextCache = spDrawRichTextCache;
}

const std::shared_ptr<DrawRichTextCache>& RichEditData::GetDrawRichTextCache() const
{
    return m_spDrawRichTextCache;
}

void RichEditData::ClearDrawRichTextCache()
{
    m_spDrawRichTextCache.reset();
}

} //namespace ui
