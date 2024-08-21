#include "RichEditData.h"
#include "duilib/Utils/PerformanceUtil.h"

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

void RichEditData::SetTextDrawRect(const UiRect& rcTextDrawRect)
{
    if (m_rcTextDrawRect != rcTextDrawRect) {
        m_rcTextDrawRect = rcTextDrawRect;
        SetCacheDirty(true);
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

    std::vector<std::wstring_view> textView;
    GetTextView(textView);
    if (textView.empty()) {
        return rect;
    }

    std::vector<RichTextData> richTextDataList;
    if (m_pRichTextData->GetRichTextForDraw(textView, richTextDataList)) {
        bool bCacheAvailable = false;
        if (!m_estimateResult.m_rcEstimate.IsEmpty() &&
            (m_estimateResult.m_rcAvailable == rcAvailable) &&
            (m_estimateResult.m_richTextDataList.size() == richTextDataList.size())) {

            bool bValid = true;
            const size_t nCount = richTextDataList.size();
            for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
                const RichTextData& textData = richTextDataList[nIndex];
                const RichTextData& textDataCache = m_estimateResult.m_richTextDataList[nIndex];
                //只比较几个影响绘制结果矩形大小的成员变量
                if (textData.m_textView.data() != textDataCache.m_textView.data()) {
                    bValid = false;
                }
                else if (textData.m_textView.size() != textDataCache.m_textView.size()) {
                    bValid = false;
                }
                else if (textData.m_fontInfo != textDataCache.m_fontInfo) {
                    bValid = false;
                }
                else if (textData.m_fRowSpacingMul != textDataCache.m_fRowSpacingMul) {
                    bValid = false;
                }
                else if (textData.m_uTextStyle != textDataCache.m_uTextStyle) {
                    bValid = false;
                }

                if (!bValid) {
                    break;
                }
            }
            
            //参数未变，使用缓存的结果，以提高效率
            if (bValid) {
                rect = m_estimateResult.m_rcEstimate;
                bCacheAvailable = true;
            }            
        }

        if (!bCacheAvailable) {
            //重新估算
            m_pRender->MeasureRichText(rcAvailable, UiSize(), m_pRenderFactory, richTextDataList);
            for (const RichTextData& data : richTextDataList) {
                for (const UiRect& textRect : data.m_textRects) {
                    rect.Union(textRect);
                }
            }
            //保存估算结果
            m_estimateResult.m_rcEstimate = rect;
            m_estimateResult.m_rcAvailable = rcAvailable;
            m_estimateResult.m_richTextDataList.swap(richTextDataList);
        }        
    }
    return rect;
}

void RichEditData::CheckCalcTextRects()
{
    if (m_bCacheDirty) {
        CalcTextRects();
        SetCacheDirty(false);
    }
}

void RichEditData::CalcTextRects()
{
    m_textRects.clear();
    m_rowTextInfo.clear();
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

    std::vector<RichTextData> richTextDataList;
    m_pRichTextData->GetRichTextForDraw(textView, richTextDataList);
    if (richTextDataList.empty()) {
        return;
    }

    UiRect rcDrawText = m_pRichTextData->GetRichTextDrawRect();
    if (rcDrawText.IsEmpty()) {
        return;
    }
    //估算的时候，滚动条位置始终为(0,0)
    UiSize szScrollOffset;
    std::vector<MeasureCharRects> textRects;
    m_spDrawRichTextCache.reset();
    uint8_t nAlpha = m_pRichTextData->GetDrawAlpha();
    m_pRender->MeasureRichText3(rcDrawText, szScrollOffset, m_pRenderFactory, richTextDataList, nAlpha, &textRects, m_spDrawRichTextCache);
    ASSERT(textRects.size() == nTextLen);

    //更新每行的索引信息（逻辑行）
    UpdateRowTextInfo(rcDrawText, szScrollOffset, textRects);

    //更新到每行文本中（物理行）
    size_t nCharIndex = 0;
    const size_t nCharCount = textRects.size();
    const size_t nLineCount = m_lineTextInfo.size();
    for (size_t nLine = 0; nLine < nLineCount; ++nLine) {
        LineTextInfo& lineText = m_lineTextInfo[nLine];
        if (lineText.m_nLineTextLen == 0) {
            continue;
        }
        /*lineText.m_lineTextRects.resize(lineText.m_nLineTextLen);
        for (MeasureCharRects& charRect : lineText.m_lineTextRects) {
            ASSERT(nCharIndex < nCharCount);
            if (nCharIndex < nCharCount) {
                charRect = textRects[nCharIndex++];
            }
            else {
                break;
            }
        }*/
        if (nCharIndex >= nCharCount) {
            break;
        }
    }
    //ASSERT(nCharIndex == nCharCount);
    m_textRects.swap(textRects);

    //TEST
    size_t s00 = sizeof(MeasureCharRects);
    size_t s0 = sizeof(MeasureCharRects) * m_textRects.size() / 1024;
    size_t s1 = (sizeof(int32_t) + sizeof(RowTextInfo)) * m_rowTextInfo.size() / 1024 ;
    size_t s2 = sizeof(LineTextInfo) * m_lineTextInfo.size() / 1024;
    size_t s4 = 0;
    for (const auto& s : m_lineTextInfo) {
        s4 += s.m_nLineTextLen*sizeof(wchar_t);
    }
    s4 /= 1024;

    size_t s5 = s0 + s1 + s2 + s4;//81.6
    //TEST
}

void RichEditData::UpdateRowTextInfo(const UiRect& rcDrawText,
                                     const UiSize& szScrollOffset,
                                     const std::vector<MeasureCharRects>& textRects)
{
    m_rowTextInfo.clear();
    if (textRects.empty()) {
        return;
    }
    //计算每行的矩形区域
    const size_t nTextCount = textRects.size();
    for (size_t nTextIndex = 0; nTextIndex < nTextCount; ++nTextIndex) {
        const MeasureCharRects& charInfo = textRects[nTextIndex];
        RowTextInfo& rowTextInfo = m_rowTextInfo[charInfo.m_nRowIndex];
        if (!charInfo.m_bIgnoredChar && !charInfo.m_bNewLine) {
            ASSERT(!charInfo.m_charRect.IsEmpty());
            rowTextInfo.m_rowRect.Union(charInfo.m_charRect);
            ASSERT(!rowTextInfo.m_rowRect.IsEmpty());
        }
        else if (charInfo.m_bNewLine || charInfo.m_bReturn) {
            //回车或者换行符：只包含了top和bottom值
            if (rowTextInfo.m_rowRect.IsEmpty()) {
                rowTextInfo.m_rowRect = charInfo.m_charRect;
            }
        }
        if (rowTextInfo.m_nTextStart == (uint32_t)-1) {
            //该行的首个字符
            rowTextInfo.m_nTextStart = (uint32_t)nTextIndex;
        }
        rowTextInfo.m_nTextEnd = (uint32_t)nTextIndex;
    }    
}

bool RichEditData::SetText(const DStringW& text)
{
    PerformanceStat statPerformance(_T("RichEditData::SetText"));
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
            if (m_lineTextInfo[nIndex].m_nLineTextLen != lineTextViewList[nIndex].size()) {
                bTextChanged = true;
                break;
            }
        }
        if (!bTextChanged) {
            //如果长度都一致，则比较字符串的内容
            for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
                if (std::wstring_view(m_lineTextInfo[nIndex].m_lineText.c_str(), m_lineTextInfo[nIndex].m_nLineTextLen) != lineTextViewList[nIndex]) {
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
        std::vector<LineTextInfo> lineTextInfo;
        if (nLineCount > 0) {
            lineTextInfo.resize(nLineCount);
            for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
                const std::wstring_view& lineTextView = lineTextViewList[nIndex];
                LineTextInfo& lineText = lineTextInfo[nIndex];
                lineText.m_lineText = lineTextView; //文本数据复制一份，保存起来
                lineText.m_nLineTextLen = (uint32_t)lineTextView.size();
                ASSERT(lineText.m_nLineTextLen > 0);
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
        const LineTextInfo& lineText = m_lineTextInfo[nIndex];
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
        const LineTextInfo& lineText = m_lineTextInfo[nIndex];
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
        const LineTextInfo& lineText = m_lineTextInfo[nIndex];
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
    std::vector<size_t> deletedLines;   //待删除的行
    std::vector<size_t> modifiedLines;  //文本有变化的行

    if (nStartLine == nEndLine) {
        //在相同行
        bool bChanged = true;
        LineTextInfo& lineText = m_lineTextInfo[nStartLine];            
        DStringW newText = lineText.m_lineText.c_str();
        if (nStartCharLineOffset == nEndCharLineOffset) {
            //当前位置，插入新文本
            if (!text.empty()) {
                newText.insert(nStartCharLineOffset, text);
            }
            else {
                bChanged = false;
            }
        }
        else {
            //替换选择的文本
            size_t nCharCount = nEndCharLineOffset - nStartCharLineOffset;
            ASSERT(nCharCount > 0);
            newText.replace(nStartCharLineOffset, nCharCount, text);
        }
        if (bChanged) {
            if (!newText.empty()) {
                lineText.m_lineText = newText;
                lineText.m_nLineTextLen = (uint32_t)newText.size();
                //需要重新计算这一行
                modifiedLines.push_back(nStartLine);
            }
            else {
                //整行删除
                deletedLines.push_back(nStartLine);
            }
        }
    }
    else if (nEndLine > nStartLine){
        //在不同行
        DStringW startLineText;
        for (size_t nIndex = nStartLine; nIndex <= nEndLine; ++nIndex) {
            LineTextInfo& lineText = m_lineTextInfo[nIndex];
            if (nIndex == nStartLine) {
                //首行, 删除到行尾
                DStringW newText = lineText.m_lineText.c_str();
                ASSERT(nStartCharLineOffset < newText.size());
                if ((nStartCharLineOffset > 0) && (nStartCharLineOffset < newText.size())) {
                    newText.resize(nStartCharLineOffset); //截断旧文本
                }
                else if (nStartCharLineOffset == 0){
                    newText.clear(); //首行整行删除文本
                }
                startLineText.swap(newText);

                //将首行整行删除，然后与尾行合并
                deletedLines.push_back(nIndex);
            }
            else if (nIndex == nEndLine) {
                //末行，删除到行首
                DStringW newText = lineText.m_lineText.c_str();
                if ((nEndCharLineOffset > 0) && (nEndCharLineOffset < newText.size())) {
                    newText = newText.substr(nEndCharLineOffset); //删除到行首
                    newText.reserve(newText.size() + startLineText.size() + text.size() + 1);
                    newText = startLineText + text + newText; //合并首行、替换目标文本
                    lineText.m_lineText = newText;
                    lineText.m_nLineTextLen = (uint32_t)newText.size();
                    //需要重新计算这一行
                    modifiedLines.push_back(nIndex);
                }
                else if(nEndCharLineOffset == newText.size()) {
                    //整行删除尾行文本
                    if (!startLineText.empty() || !text.empty()) {
                        newText.reserve(startLineText.size() + text.size() + 1);
                        newText = startLineText + text; //合并首行、替换目标文本
                        lineText.m_lineText = newText;
                        lineText.m_nLineTextLen = (uint32_t)newText.size();
                        //需要重新计算这一行
                        modifiedLines.push_back(nIndex);
                    }
                    else {
                        deletedLines.push_back(nIndex);
                    }
                }
                else {
                    //无需删除本行数据
                    if (!startLineText.empty() || !text.empty()) {
                        newText.reserve(newText.size() + startLineText.size() + text.size() + 1);
                        newText = startLineText + text + newText; //合并首行、替换目标文本
                        lineText.m_lineText = newText;
                        lineText.m_nLineTextLen = (uint32_t)newText.size();
                        //需要重新计算这一行
                        modifiedLines.push_back(nIndex);
                    }
                }
            }
            else {
                //中间行
                deletedLines.push_back(nIndex);
            }
        }
    }

    //删除中间行(倒序删除)
    if (!deletedLines.empty()) {
        int32_t nDelIndex = (int32_t)deletedLines.size() - 1;
        for (; nDelIndex >= 0; --nDelIndex) {
            if (deletedLines[nDelIndex] < m_lineTextInfo.size()) {
                m_lineTextInfo.erase(m_lineTextInfo.begin() + deletedLines[nDelIndex]);
            }
        }
    }

    //修改的行，需要重新计算
    if (!modifiedLines.empty()) {
        std::vector<std::wstring_view> textView;
        size_t nLineCount = modifiedLines.size();
        for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
            const LineTextInfo& lineText = m_lineTextInfo[modifiedLines[nIndex]];
            ASSERT(lineText.m_nLineTextLen > 0);
            if (lineText.m_nLineTextLen > 0) {
                textView.push_back(std::wstring_view(lineText.m_lineText.data(), lineText.m_nLineTextLen));
            }
        }


    }

    SetCacheDirty(true);
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
        const LineTextInfo& lineText = m_lineTextInfo[nStartLine];
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
            const LineTextInfo& lineText = m_lineTextInfo[nIndex];
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
    m_textRects.clear();
    m_lineTextInfo.clear();
    m_rowTextInfo.clear();
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

UiPoint RichEditData::CaretPosFromChar(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiPoint cursorPos;
    const int32_t nTextRectCount = (int32_t)m_textRects.size();
    ASSERT((nCharIndex >= 0) && (nCharIndex <= nTextRectCount));
    if ((nCharIndex >= 0) && (nCharIndex < nTextRectCount)) {
        const MeasureCharRects& charInfo = m_textRects[nCharIndex];
        cursorPos.x = (int32_t)charInfo.m_charRect.left;//左上角坐标
        cursorPos.y = (int32_t)charInfo.m_charRect.top;
    }
    else if (nTextRectCount > 0) {
        const MeasureCharRects& charInfo = m_textRects[nTextRectCount - 1];
        cursorPos.x = (int32_t)std::ceilf(charInfo.m_charRect.right);//右上角坐标
        cursorPos.y = (int32_t)charInfo.m_charRect.top;
    }
    else {        
        cursorPos.x = 0;
        cursorPos.y = 0;
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
    const int32_t nTextRectCount = (int32_t)m_textRects.size();
    ASSERT((nCharIndex >= 0) && (nCharIndex <= nTextRectCount));
    if ((nCharIndex >= 0) && (nCharIndex < nTextRectCount)) {
        const MeasureCharRects& charInfo = m_textRects[nCharIndex];
        pt.x = (int32_t)charInfo.m_charRect.left; //左上角坐标
        pt.y = (int32_t)charInfo.m_charRect.top;
    }
    else if (nTextRectCount > 0) {
        //最后一个字符
        const MeasureCharRects& charInfo = m_textRects[nTextRectCount - 1];
        pt.x = (int32_t)charInfo.m_charRect.left; //左上角坐标
        pt.y = (int32_t)charInfo.m_charRect.top;
    }
    else {
        //其他情况，返回绘制区域左上角坐标
        pt.x = 0;
        pt.y = 0;
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
    //计算选择区域的行号
    size_t nRowStartIndex = (size_t)-1;
    size_t nRowEndIndex = (size_t)-1;
    UiRectF rowRectF;
    const std::map<int32_t, RowTextInfo>& rowTextInfo = m_rowTextInfo;
    for (auto iter = rowTextInfo.begin(); iter != rowTextInfo.end(); ++iter) {
        const UiRectF& rowRect = iter->second.m_rowRect;
        if ((pt.y >= rowRect.top) && (pt.y < rowRect.bottom)) {
            nRowStartIndex = iter->second.m_nTextStart;
            nRowEndIndex = iter->second.m_nTextEnd;
            rowRectF = rowRect;
        }
    }

    const size_t nTextRectCount = m_textRects.size();
    if ((nRowStartIndex < nTextRectCount) && (nRowEndIndex < nTextRectCount)) {
        //在该行中查找点的位置
        for (size_t nTextIndex = nRowStartIndex; nTextIndex <= nRowEndIndex; ++nTextIndex) {
            const MeasureCharRects& charInfo = m_textRects[nTextIndex];
            if (!charInfo.m_charRect.IsEmpty() && charInfo.m_charRect.ContainsPt((float)pt.x, (float)pt.y)) {
                if (pt.x > charInfo.m_charRect.CenterX()) {
                    //如果X坐标大于中心点，则取下一个字符
                    nCharPosIndex = (int32_t)GetNextValidCharIndex(m_textRects, nTextIndex);
                }
                else {
                    //如果X坐标小于等于中心点，取当前字符
                    nCharPosIndex = (int32_t)nTextIndex;
                }
                break;
            }
        }

        if (nCharPosIndex == -1) {
            if (((nRowEndIndex - nRowStartIndex) == 1) && m_textRects[nRowStartIndex].m_bReturn && m_textRects[nRowStartIndex].m_bNewLine) {
                //该行只有一个回车+换行: 指向回车字符
                nCharPosIndex = (int32_t)nRowStartIndex;
            }
            else if ((nRowStartIndex == nRowEndIndex) && (m_textRects[nRowStartIndex].m_bReturn || m_textRects[nRowStartIndex].m_bNewLine)) {
                //该行只有一个回车 或者 换行
                nCharPosIndex = (int32_t)nRowStartIndex;
            }
            else if (pt.x <= rowRectF.left) {
                //在行文本的左侧，取该行的首字符
                nCharPosIndex = (int32_t)nRowStartIndex;
            }
            else if (pt.x >= rowRectF.right) {
                //在行文本的右侧, 取该行的最后个字符（换行符）                
                if (((nRowEndIndex - 1) >= 0) && m_textRects[nRowEndIndex - 1].m_bReturn && m_textRects[nRowEndIndex].m_bNewLine) {
                    //如果结尾是'\r\n'，则指向'\r'
                    nCharPosIndex = (int32_t)nRowEndIndex - 1;
                }
                else {
                    nCharPosIndex = (int32_t)nRowEndIndex;
                }
            }
        }
    }
    else {
        ASSERT(0);
    }
    
    if (nCharPosIndex < 0) {
        nCharPosIndex = 0;
    }
    else if (nCharPosIndex > nTextLength) {
        nCharPosIndex = nTextLength;
    }
    return nCharPosIndex;
}

size_t RichEditData::GetNextValidCharIndex(const std::vector<MeasureCharRects>& textRects, size_t nCurrentIndex) const
{
    ASSERT(nCurrentIndex < textRects.size());
    if (nCurrentIndex >= textRects.size()) {
        return nCurrentIndex;
    }
    size_t nNextIndex = nCurrentIndex;
    ++nNextIndex;
    if (nNextIndex < textRects.size()) {
        if (textRects[nNextIndex - 1].m_bReturn && textRects[nNextIndex].m_bNewLine) {
            ++nNextIndex;
        }
    }
    while (nNextIndex < textRects.size()) {
        if (!textRects[nNextIndex].m_bIgnoredChar) {
            if (textRects[nNextIndex].m_bNewLine) {
                //当前字符是换行符, 如果字符是'\r\n'，应指向'\r'字符
                if ((nCurrentIndex != (nNextIndex - 1)) && textRects[nNextIndex - 1].m_bReturn) {
                    --nNextIndex;
                }
            }
            break;
        }
        ++nNextIndex;
    }
    if (nNextIndex < textRects.size()) {
        return nNextIndex;
    }
    return nCurrentIndex;
}

size_t RichEditData::GetPrevValidCharIndex(const std::vector<MeasureCharRects>& textRects, size_t nCurrentIndex) const
{
    ASSERT(nCurrentIndex < textRects.size());
    if (nCurrentIndex >= textRects.size()) {
        return nCurrentIndex;
    }
    int32_t nPrevIndex = (int32_t)nCurrentIndex;
    --nPrevIndex;
    while (nPrevIndex >= 0) {
        if (!textRects[nPrevIndex].m_bIgnoredChar) {
            if (textRects[nPrevIndex].m_bNewLine) {
                //当前字符是换行符, 如果字符是'\r\n'，应指向'\r'字符
                if ((nCurrentIndex != (nPrevIndex - 1)) && textRects[nPrevIndex - 1].m_bReturn) {
                    --nPrevIndex;
                }
            }
            break;
        }
        --nPrevIndex;
    }
    if (nPrevIndex >= 0) {
        return nPrevIndex;
    }
    return nCurrentIndex;
}

int32_t RichEditData::GetNextUnicodeCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    const std::vector<MeasureCharRects>& textRects = m_textRects;
    ASSERT(nCharIndex < textRects.size());
    if (nCharIndex >= textRects.size()) {
        return nCharIndex;
    }
    size_t nNextIndex = nCharIndex;
    ++nNextIndex;
    while (nNextIndex < textRects.size()) {
        if (!textRects[nNextIndex].m_bLowSurrogate) {
            break;
        }
        ++nNextIndex;
    }
    if (nNextIndex < textRects.size()) {
        return nNextIndex;
    }
    return nCharIndex;
}

int32_t RichEditData::GetPrevUnicodeCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    const std::vector<MeasureCharRects>& textRects = m_textRects;
    ASSERT(nCharIndex < textRects.size());
    if (nCharIndex >= textRects.size()) {
        return nCharIndex;
    }
    int32_t nPrevIndex = (int32_t)nCharIndex;
    --nPrevIndex;
    while (nPrevIndex >= 0) {
        if (!textRects[nPrevIndex].m_bLowSurrogate) {
            break;
        }
        --nPrevIndex;
    }
    if (nPrevIndex >= 0) {
        return nPrevIndex;
    }
    return nCharIndex;
}

int32_t RichEditData::GetNextValidCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    return (int32_t)GetNextValidCharIndex(m_textRects, (size_t)nCharIndex);
}

int32_t RichEditData::GetPrevValidCharIndex(int32_t nCharIndex)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    return (int32_t)GetPrevValidCharIndex(m_textRects, (size_t)nCharIndex);
}

int32_t RichEditData::GetNextValidCharIndexForDelete(int32_t nCharIndex, bool bMatchWord)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    if (bMatchWord) {
        //按单词选择：TODO

    }
    else {
        const std::vector<MeasureCharRects>& textRects = m_textRects;
        ASSERT(nCharIndex < textRects.size());
        if (nCharIndex >= textRects.size()) {
            return nCharIndex;
        }
        size_t nNextIndex = nCharIndex;
        ++nNextIndex;
        while (nNextIndex < textRects.size()) {
            if (!textRects[nNextIndex].m_bLowSurrogate) {
                if (textRects[nNextIndex].m_bNewLine && textRects[nCharIndex].m_bReturn && (nNextIndex == (nCharIndex + 1))) {
                    if (((nNextIndex + 1) < textRects.size())) {
                        //跳过回车/换行两个字符，指向换行符后面的一个字符，一次性的把'\r\n'删除
                        ++nNextIndex;
                        break;
                    }
                }
                break;
            }
            ++nNextIndex;
        }
        if (nNextIndex < textRects.size()) {
            return nNextIndex;
        }
    }
    return nCharIndex;
}

int32_t RichEditData::GetPrevValidCharIndexForDelete(int32_t nCharIndex, bool bMatchWord)
{
    ASSERT(nCharIndex >= 0);
    if (nCharIndex < 0) {
        return nCharIndex;
    }
    //检查并计算字符位置
    CheckCalcTextRects();

    if (bMatchWord) {
        //按单词选择：TODO

    }
    else {
        const std::vector<MeasureCharRects>& textRects = m_textRects;
        ASSERT(nCharIndex < textRects.size());
        if (nCharIndex >= textRects.size()) {
            return nCharIndex;
        }

        int32_t nPrevIndex = (int32_t)nCharIndex;
        --nPrevIndex;
        while (nPrevIndex >= 0) {
            if (!textRects[nPrevIndex].m_bIgnoredChar) {
                //如果当前字符指向'\n'，前面字符是'\r'，则将字符位置指向'\r'
                if (((nPrevIndex - 1) >= 0) && textRects[nPrevIndex].m_bNewLine && textRects[nPrevIndex - 1].m_bReturn) {
                    --nPrevIndex;
                }
                break;
            }
            --nPrevIndex;
        }
        if (nPrevIndex >= 0) {
            return nPrevIndex;
        }

    }
    return nCharIndex;
}

UiRect RichEditData::GetCharRowRect(int32_t nCharIndex)
{
    //检查并计算字符位置
    CheckCalcTextRects();

    UiRect rowRect;
    const int32_t nTextCount = (int32_t)m_textRects.size();
    if ((nCharIndex >= 0) && (nTextCount > 0) && (nCharIndex <= nTextCount)) {
        if (nCharIndex == nTextCount) {
            nCharIndex = nTextCount - 1;
        }
        const MeasureCharRects& charRect = m_textRects[nCharIndex];
        auto iter = m_rowTextInfo.find(charRect.m_nRowIndex);
        if (iter != m_rowTextInfo.end()) {
            const UiRectF& rowRectF = iter->second.m_rowRect;
            UiRect rc = m_pRichTextData->GetRichTextDrawRect();            
            rowRect.left = 0;
            rowRect.right = rc.Width();
            rowRect.top = (int32_t)rowRectF.top;
            rowRect.bottom = (int32_t)std::ceilf(rowRectF.bottom);
        }
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

    if ((nStartChar >= 0) && (nStartChar < nTextLength) && (nEndChar > nStartChar) && (nEndChar <= nTextLength)) {
        for (size_t nIndex = nStartChar; nIndex < nEndChar; ++nIndex) {
            const MeasureCharRects& charRect = m_textRects[nIndex];
            if (charRect.m_bIgnoredChar) {
                continue;
            }
            UiRectF& rowRect = rowTextRectFs[charRect.m_nRowIndex];
            if (charRect.m_bNewLine) {
                if (rowRect.IsZero()) {
                    rowRect = charRect.m_charRect;
                }
            }
            else {
                rowRect.Union(charRect.m_charRect);
            }
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
