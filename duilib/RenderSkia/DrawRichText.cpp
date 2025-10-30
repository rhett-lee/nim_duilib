#include "DrawRichText.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/PerformanceUtil.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"

#include "SkiaHeaderEnd.h"

#include <unordered_set>

namespace ui {

//待绘制的文本
struct TPendingDrawRichText : public NVRefCount<TPendingDrawRichText>
{
    //在richTextData中的索引号
    uint32_t m_nDataIndex = 0;

    //物理行号
    uint32_t m_nLineNumber = 0;

    //行号(是绘制后的逻辑行号，当自动换行的时候，一个物理行显示为多个逻辑行，物理行号是指按文本中的换行符划分的行号）
    uint32_t m_nRowIndex = 0;

    //待绘制文本
    std::wstring_view m_textView;

    //绘制目标区域
    UiRect m_destRect;

    //Font对象
    std::shared_ptr<IFont> m_spFont;

    /** 文字颜色
    */
    UiColor m_textColor;

    /** 背景颜色
    */
    UiColor m_bgColor;

    /** 绘制文字的属性(包含文本对齐方式等属性，参见 enum DrawStringFormat)
    */
    uint16_t m_textStyle = 0;
};

/** 绘制缓存
*/
class DrawRichTextCache
{
public:
    /** 原始参数
    */
    UiRect m_textRect;
    std::vector<RichTextData> m_richTextData;

    SkTextEncoding m_textEncoding = SkTextEncoding::kUTF16;
    size_t m_textCharSize = sizeof(DStringW::value_type);

    /** 生成好的待绘制的数据
    */
    std::vector<SharePtr<TPendingDrawRichText>> m_pendingTextData;
};

DrawRichText::DrawRichText(IRender* pRender, SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg) :
    m_pRender(pRender),
    m_pSkCanvas(pSkCanvas),
    m_pSkPaint(pSkPaint),
    m_pSkPointOrg(pSkPointOrg)
{
}

void DrawRichText::InternalDrawRichText(const UiRect& rcTextRect,
                                        const UiSize& szScrollOffset,
                                        IRenderFactory* pRenderFactory, 
                                        const std::vector<RichTextData>& richTextData,
                                        uint8_t uFade,
                                        bool bMeasureOnly,
                                        RichTextLineInfoParam* pLineInfoParam,
                                        std::shared_ptr<DrawRichTextCache>* pDrawRichTextCache,
                                        std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("DrawRichText::InternalDrawRichText"));
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return;
    }

    //内部使用string_view实现，避免字符串复制影响性能
    if (rcTextRect.IsEmpty()) {
        return;
    }
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    //绘制区域：绘制区域的坐标以 (rcTextRect.left,rcTextRect.top)作为(0,0)点
    UiRect rcDrawRect = rcTextRect;
    rcDrawRect.Offset(-szScrollOffset.cx, -szScrollOffset.cy);

    if ((pLineInfoParam != nullptr) || (pDrawRichTextCache != nullptr)) {
        ASSERT(bMeasureOnly);
        if (!bMeasureOnly) {
            return;
        }
        //使用(0,0)坐标点，作为估算的(0,0)点
        rcDrawRect.Offset(-rcDrawRect.left, -rcDrawRect.top);
        ASSERT(rcDrawRect.left == 0);
        ASSERT(rcDrawRect.top == 0);
    }

    //文本编码：固定为UTF16 或者 UTF32
    constexpr const SkTextEncoding textEncoding = (sizeof(DStringW::value_type) == sizeof(uint32_t)) ? SkTextEncoding::kUTF32 : SkTextEncoding::kUTF16;
    constexpr const size_t textCharSize = sizeof(DStringW::value_type);

    //当绘制超过目标矩形边界时，是否继续绘制
    const bool bBreakWhenOutOfRect = !bMeasureOnly && (pDrawRichTextCache == nullptr);

    std::vector<SharePtr<TPendingDrawRichText>> pendingTextData;
    pendingTextData.reserve(richTextData.size());

    const int32_t nTextRectRightMax = (int32_t)rcTextRect.right;   //绘制区域的最右侧
    const int32_t nTextRectBottomMax = (int32_t)rcTextRect.bottom; //绘制区域的最底端

    SkScalar xPos = (SkScalar)rcDrawRect.left;  //水平坐标：字符绘制的时候，是按浮点型坐标，每个字符所占的宽度是浮点型的，不能对齐到像素
    int32_t yPos = rcDrawRect.top;              //垂直坐标，对齐到像素，所以用整型
    int32_t nRowHeight = 0;   //行高（本行中，所有字符绘制高度的最大值，对齐到像素）
    uint32_t nLineNumber = 0; //物理行号
    uint32_t nRowIndex = 0;   //逻辑行号

    std::unordered_map<uint32_t, uint32_t> rowHeightMap;  //每行的实际行高表

    //字体缓存(由于创建字体比较耗时，所以尽量复用相同的对象)
    SharePtr<UiFontEx> lastFont;
    std::shared_ptr<IFont> spLastSkiaFont;

    if (pLineInfoParam != nullptr) {
        //设置起始行号
        nLineNumber = (uint32_t)pLineInfoParam->m_nStartLineIndex;
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        if (pLineInfoParam->m_pLineInfoList == nullptr) {
            return;
        }
        ASSERT(nLineNumber < pLineInfoParam->m_pLineInfoList->size());
        if (nLineNumber >= pLineInfoParam->m_pLineInfoList->size()) {
            return;
        }
        //起始的逻辑行号
        nRowIndex = pLineInfoParam->m_nStartRowIndex;
    }

    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }
    UiColor textColor;

    std::vector<SkGlyphID> glyphs;      //内部临时变量，为提升执行速度，在外部声明变量
    std::vector<uint8_t> glyphChars;    //内部临时变量，为提升执行速度，在外部声明变量
    std::vector<SkScalar> glyphWidths;  //内部临时变量，为提升执行速度，在外部声明变量

    std::vector<uint8_t> glyphCharList;   //每个字由几个字符构成
    std::vector<SkScalar> glyphWidthList; //每个字符的宽度

    //按换行符进行文本切分
    std::vector<std::wstring_view> lineTextViewList;

    //分行时文本切分的内部临时变量，为提升执行速度，在外部声明变量
    std::vector<uint32_t> lineSeprators;

    //是否正在绘制TAB键（按4个字符对齐）
    bool bDrawTabChar = false;

    //本行（逻辑行）已经绘制了多少个字符（不含回车和换行）
    size_t nRowCharCount = 0;

    for (size_t index = 0; index < richTextData.size(); ++index) {
        const RichTextData& textData = richTextData[index];
        if (textData.m_textView.empty()) {
            continue;
        }

        //设置文本颜色
        if (textColor != textData.m_textColor) {
            const UiColor& color = textData.m_textColor;
            skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
            textColor = textData.m_textColor;
        }
        
        std::shared_ptr<IFont> spSkiaFont;
        if ((spLastSkiaFont != nullptr) &&
            (textData.m_pFontInfo != nullptr) && (lastFont != nullptr) &&
            ((textData.m_pFontInfo == lastFont) || (*textData.m_pFontInfo == *lastFont))) {
            //复用缓存中的字体对象
            spSkiaFont = spLastSkiaFont;
        }
        else {
            spSkiaFont.reset(pRenderFactory->CreateIFont());
            ASSERT(spSkiaFont != nullptr);
            if (spSkiaFont == nullptr) {
                continue;
            }
            ASSERT(textData.m_pFontInfo != nullptr);
            if (textData.m_pFontInfo == nullptr) {
                continue;
            }
            if (!spSkiaFont->InitFont(*textData.m_pFontInfo)) {
                spSkiaFont.reset();
                continue;
            }
        }

        Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(spSkiaFont.get());
        ASSERT(pSkiaFont != nullptr);
        if (pSkiaFont == nullptr) {
            continue;
        }
        const SkFont* pSkFont = pSkiaFont->GetFontHandle();
        ASSERT(pSkFont != nullptr);
        if (pSkFont == nullptr) {
            continue;
        }

        if (spLastSkiaFont != spSkiaFont) {
            spLastSkiaFont = spSkiaFont;
            lastFont = textData.m_pFontInfo;
        }

        const SkFont& skFont = *pSkFont;
        SkFontMetrics metrics;
        SkScalar fFontHeight = skFont.getMetrics(&metrics);     //字体高度，换行时使用
        fFontHeight *= textData.m_fRowSpacingMul;               //运用行间距
        const int32_t nFontHeight = SkScalarCeilToInt(fFontHeight);   //行高对齐到像素
        nRowHeight = std::max(nRowHeight, nFontHeight);
        if (nRowHeight <= 0) {
            continue;
        }
        const uint32_t uTextStyle = textData.m_textStyle;
        const bool bSingleLineMode = (uTextStyle & DrawStringFormat::TEXT_SINGLELINE) ? true : false; //是否为单行模式，单行模式下，不换行
        const bool bWordWrap = bSingleLineMode ? false : ((uTextStyle & DrawStringFormat::TEXT_WORD_WRAP) ? true : false);
        bool bBreakAll = false;//标记是否终止

        //按换行符进行文本切分
        lineTextViewList.clear();
        SplitLines(textData.m_textView, lineSeprators, lineTextViewList);

        //物理行内的逻辑行号(每个物理行中，从0开始编号)
        uint32_t nLineTextRowIndex = 0;

        for (const std::wstring_view& lineTextView : lineTextViewList) {
            bDrawTabChar = false;
            if (lineTextView.size() == 1) {              
                if (lineTextView[0] == L'\r') {
                    //处理回车
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }
                    continue; //忽略回车
                }
                if (lineTextView[0] == L'\t') {
                    //处理TAB键
                    bDrawTabChar = true;
                }
                else if (lineTextView[0] == L'\n') {
                    //处理换行符
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }

                    //换行：执行换行操作(物理换行)
                    if (!bSingleLineMode) {
                        xPos = (SkScalar)rcDrawRect.left;
                        ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                        yPos += nRowHeight;
                        rowHeightMap[nRowIndex] = nRowHeight;
                        nRowHeight = nFontHeight;
                        ++nRowIndex;
                        nRowCharCount = 0;
                        ++nLineTextRowIndex;
                        ++nLineNumber;
                    }
                    continue; //处理下一行
                }
            }

            //绘制的文本下标开始值
            const size_t textCount = lineTextView.size();
            size_t textStartIndex = 0;
            while (textStartIndex < textCount) {
                //估算文本绘制区域                
                size_t byteLength = (textCount - textStartIndex) * textCharSize;                
                SkScalar maxWidth = SkIntToScalar(rcDrawRect.right) - xPos;//可用宽度
                if (!bWordWrap || bSingleLineMode) {
                    //不自动换行 或者 单行模式
                    maxWidth = SK_FloatInfinity;
                }
                ASSERT(maxWidth > 0);
                SkScalar textMeasuredWidth = 0;  //当前要绘制的文本，估算的所需宽度
                SkScalar textMeasuredHeight = 0; //当前要绘制的文本，估算的所需高度

                glyphCharList.clear();
                glyphWidthList.clear();

                //评估每个字符的矩形范围
                std::vector<uint8_t>* pGlyphCharList = &glyphCharList;
                std::vector<SkScalar>* pGlyphWidthList = &glyphWidthList;

                size_t nDrawLength = 0;
                if (bDrawTabChar) {
                    ASSERT(textCount == 1);
                    //绘制TAB键, 按4个字符对齐
                    const DStringW blank = L"    ";
                    size_t nBlankCount = nRowCharCount % blank.size();
                    nBlankCount = blank.size() - nBlankCount;
                    nDrawLength = SkTextBox::breakText(blank.c_str(),
                                                       nBlankCount * sizeof(DStringW::value_type), textEncoding,
                                                       skFont, skPaint,
                                                       maxWidth, &textMeasuredWidth, &textMeasuredHeight,
                                                       glyphs, glyphChars, glyphWidths,
                                                       pGlyphCharList, pGlyphWidthList);
                    if (nDrawLength > 0) {
                        nDrawLength = textCount * sizeof(DStringW::value_type);
                        if (glyphs.empty()) {
                            glyphs.resize(1);
                            glyphChars.resize(1);
                            glyphChars[0] = 1;
                            glyphWidths.resize(1, textMeasuredWidth);
                        }
                        pGlyphCharList->resize(1);
                        (*pGlyphCharList)[0] = 1;
                        pGlyphWidthList->resize(1);
                        (*pGlyphWidthList)[0] = textMeasuredWidth;
                    }
                }
                else {
                    //breakText函数执行时间占比约30%
                    nDrawLength = SkTextBox::breakText(lineTextView.data() + textStartIndex,
                                                       byteLength, textEncoding,
                                                       skFont, skPaint,
                                                       maxWidth, &textMeasuredWidth, &textMeasuredHeight,
                                                       glyphs, glyphChars, glyphWidths,
                                                       pGlyphCharList, pGlyphWidthList);
                }
                
                if (nDrawLength == 0) {
                    if (!bWordWrap || bSingleLineMode || (SkScalarTruncToInt(maxWidth) == rcDrawRect.Width())) {
                        //出错了(不能换行，或者换行后依然不够)
                        bBreakAll = true;
                        break;
                    }
                }
                else {
                    SharePtr<TPendingDrawRichText> spTextData(new TPendingDrawRichText);
                    spTextData->m_nDataIndex = (uint32_t)index;
                    spTextData->m_nLineNumber = nLineNumber;
                    spTextData->m_nRowIndex = nRowIndex;
                    spTextData->m_textView = std::wstring_view(lineTextView.data() + textStartIndex, nDrawLength / textCharSize);
                    spTextData->m_spFont = spSkiaFont;

                    spTextData->m_bgColor = textData.m_bgColor;
                    spTextData->m_textColor = textData.m_textColor;
                    spTextData->m_textStyle = textData.m_textStyle;

                    //绘制文字所需的矩形区域
                    spTextData->m_destRect.left = SkScalarTruncToInt(xPos); //左值：直接截断，如果有小数部分，直接去掉小数即可

                    SkScalar fRight = xPos + textMeasuredWidth;             //右值：如果有小数，则需要增加1个像素
                    spTextData->m_destRect.right = SkScalarCeilToInt(fRight);
                    spTextData->m_destRect.top = yPos;
                    spTextData->m_destRect.bottom = yPos + SkScalarCeilToInt(textMeasuredHeight); //记录字符的真实高度
                    pendingTextData.emplace_back(std::move(spTextData));

                    if (pLineInfoParam != nullptr) {
                        //评估每个字符的矩形范围
                        ASSERT(!glyphCharList.empty());
                        ASSERT(glyphCharList.size() == glyphWidthList.size());
                        if (glyphCharList.size() == glyphWidthList.size()) {
                            const size_t glyphCount = glyphCharList.size();
                            SkScalar glyphWidth = 0;
                            uint8_t glyphCharCount = 0;
                            SkScalar glyphLeft = (SkScalar)SkScalarTruncToInt(xPos);
                            for (size_t glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
                                glyphWidth = glyphWidthList[glyphIndex];//字符宽度
                                glyphCharCount = glyphCharList[glyphIndex];  //该字占几个字符（UTF16编码，可能是1或者2）
                                ASSERT((glyphCharCount == 1) || (glyphCharCount == 2));
                                OnDrawUnicodeChar(pLineInfoParam, 0, glyphCharCount, glyphCount, nLineNumber, nLineTextRowIndex, glyphLeft, yPos, glyphWidth, nRowHeight);
                                glyphLeft += glyphWidth;
                            }
                        }
                    }

                    //统计本逻辑行已经绘制了多少个字符
                    nRowCharCount += glyphs.size();
                }

                bool bNextRow = false; //是否需要换行的标志
                if (nDrawLength < byteLength) {
                    //宽度不足，需要换行
                    if (!bSingleLineMode) {
                        bNextRow = true;
                    }                    
                    textStartIndex += nDrawLength / textCharSize;
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                }
                else {
                    //当前行可容纳文本绘制
                    textStartIndex = textCount;//标记，结束循环
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                    if (xPos >= nTextRectRightMax) {
                        //X坐标右侧已经超出目标矩形的范围
                        if (bBreakWhenOutOfRect && bSingleLineMode) {
                            //单行模式，终止绘制
                            bBreakAll = true;
                            break;
                        }
                        else if (bWordWrap && !bSingleLineMode) {
                            //在自动换行的情况下，换行
                            bNextRow = true;
                        }
                    }
                }

                if (bNextRow) {
                    //换行：执行换行操作（逻辑换行，对nLineNumber不增加）
                    xPos = (SkScalar)rcDrawRect.left;
                    ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                    yPos += nRowHeight;
                    rowHeightMap[nRowIndex] = nRowHeight;
                    nRowHeight = nFontHeight;
                    ++nRowIndex;
                    nRowCharCount = 0;
                    ++nLineTextRowIndex;

                    if (bBreakWhenOutOfRect && (yPos >= nTextRectBottomMax)) {
                        //Y坐标底部已经超出目标矩形的范围，终止绘制
                        bBreakAll = true;
                        break;
                    }
                }
            }
            if (bBreakAll) {
                break;
            }
        }
        if (bBreakAll) {
            break;
        }
    }

    //记录最后一行的行高
    rowHeightMap[nRowIndex] = nRowHeight;

    //更新每行的行高(只有提前确定行高，才能正确绘制纵向对齐的文本)
    for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
        TPendingDrawRichText& textData = *spTextData;
        auto iter = rowHeightMap.find(textData.m_nRowIndex);
        ASSERT(iter != rowHeightMap.end());
        if (iter != rowHeightMap.end()) {
            textData.m_destRect.bottom = textData.m_destRect.top + iter->second;
        }
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(richTextData.size());
        for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
            const TPendingDrawRichText& textData = *spTextData;
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(textData.m_destRect); 
        }
    }

    if (pDrawRichTextCache != nullptr) {
        //生成绘制缓存，但不执行绘制
        std::shared_ptr<DrawRichTextCache> spDrawRichTextCache = std::make_shared<DrawRichTextCache>();
        *pDrawRichTextCache = spDrawRichTextCache;
        spDrawRichTextCache->m_richTextData = richTextData;
        spDrawRichTextCache->m_textRect = rcTextRect;

        spDrawRichTextCache->m_textEncoding = textEncoding;
        spDrawRichTextCache->m_textCharSize = textCharSize;

        spDrawRichTextCache->m_pendingTextData.swap(pendingTextData);
    }
    else if (!bMeasureOnly) {
        UiRect rcTemp;
        for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
            const TPendingDrawRichText& textData = *spTextData;
            //执行绘制            
            const UiRect& rcDestRect = textData.m_destRect;
            if (!UiRect::Intersect(rcTemp, rcDestRect, rcTextRect)) {
                continue;
            }

            //绘制文字的背景色
            m_pRender->FillRect(rcDestRect, textData.m_bgColor, uFade);

            if (textColor != textData.m_textColor) {
                const UiColor& color = textData.m_textColor;
                skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
                textColor = textData.m_textColor;
            }

            //绘制文字
            if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
                //绘制TAB键
                const char* text = (const char*)" ";
                const size_t len = 1; //字节数
                DrawTextString(rcDestRect, text, len, SkTextEncoding::kUTF8,
                               textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                               skPaint, textData.m_spFont.get());
            }
            else {
                const char* text = (const char*)textData.m_textView.data();
                const size_t len = textData.m_textView.size() * textCharSize; //字节数
                DrawTextString(rcDestRect, text, len, textEncoding,
                               textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                               skPaint, textData.m_spFont.get());
            }
        }
    }
}

void DrawRichText::SplitLines(const std::wstring_view& lineText, std::vector<uint32_t>& lineSeprators, std::vector<std::wstring_view>& lineTextViewList)
{
    if (lineText.empty()) {
        return;
    }
    lineSeprators.clear();
    const uint32_t nTextLen = (uint32_t)lineText.size();
    lineSeprators.reserve(nTextLen/100);
    for (uint32_t nTextIndex = 0; nTextIndex < nTextLen; ++nTextIndex) {
        const std::wstring_view::value_type& ch = lineText[nTextIndex];
        if ((ch == L'\t') || (ch == L'\r') || (ch == L'\n')) {
            lineSeprators.push_back(nTextIndex);
        }
    }
    if (lineSeprators.empty()) {
        //没有换行分隔符，单行
        lineTextViewList.emplace_back(std::wstring_view(lineText.data(), lineText.size()));
    }
    else {
        //有换行分隔符，切分为多行, 并保留换行符
        size_t nLastIndex = 0;
        size_t nCurrentIndex = 0;
        size_t nCharCount = 0;
        const size_t nLineSepCount = lineSeprators.size();
        lineTextViewList.reserve(nLineSepCount * 2 + 1);
        for (size_t nLine = 0; nLine < nLineSepCount; ++nLine) {
            if (nLine == 0) {
                //首行
                nLastIndex = 0;
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data(), nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }
            else {
                //中间行
                nLastIndex = lineSeprators[nLine - 1];
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex > nLastIndex);
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }

            if (nLine == (nLineSepCount - 1)) {
                //末行: 将最后一行数据添加进来
                nLastIndex = lineSeprators[nLine];
                nCurrentIndex = lineText.size();
                ASSERT(nCurrentIndex > nLastIndex);
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
            }
        }
    }
}

void DrawRichText::OnDrawUnicodeChar(RichTextLineInfoParam* pLineInfoParam,
                                     DStringW::value_type ch, uint8_t glyphChars, size_t glyphCount,
                                     size_t nLineTextIndex, uint32_t nLineTextRowIndex,
                                     float xPos, int32_t yPos, float glyphWidth, int32_t nRowHeight)
{
    //该函数执行频率非常高，每个字符都会调用一次，性能敏感    
    if (pLineInfoParam == nullptr) {
        ASSERT(pLineInfoParam != nullptr);
        return;
    }    
    if (pLineInfoParam->m_pLineInfoList == nullptr) {
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        return;
    }
    size_t nIndex = nLineTextIndex; //外部已经加上了pLineInfoParam->m_nStartLineIndex的值    
    if (nIndex >= pLineInfoParam->m_pLineInfoList->size()) {
        ASSERT(nIndex < pLineInfoParam->m_pLineInfoList->size());
        return;
    }
    if ((*pLineInfoParam->m_pLineInfoList)[nIndex] == nullptr) {
        ASSERT((*pLineInfoParam->m_pLineInfoList)[nIndex] != nullptr);
        return;
    }

    RichTextLineInfo& lineInfo = *(*pLineInfoParam->m_pLineInfoList)[nIndex];
    bool bFound = (nLineTextRowIndex < lineInfo.m_rowInfo.size()) ? true : false;
    if (!bFound) {        
        if (nLineTextRowIndex != lineInfo.m_rowInfo.size()) {
            ASSERT(nLineTextRowIndex == lineInfo.m_rowInfo.size());
            return;
        }
        lineInfo.m_rowInfo.push_back(RichTextRowInfoPtr(new RichTextRowInfo));        
    }    
    if (lineInfo.m_rowInfo[nLineTextRowIndex] == nullptr) {
        ASSERT(lineInfo.m_rowInfo[nLineTextRowIndex] != nullptr);
        return;
    }
    RichTextRowInfo& rowInfo = *lineInfo.m_rowInfo[nLineTextRowIndex];
    if (!bFound) {
        //该行的第一个字符
        rowInfo.m_rowRect.left = xPos;
        rowInfo.m_rowRect.right = xPos + glyphWidth;
        rowInfo.m_rowRect.top = (SkScalar)yPos;
        rowInfo.m_rowRect.bottom = rowInfo.m_rowRect.top + nRowHeight;
        ASSERT(nRowHeight > 0);

        rowInfo.m_charInfo.reserve(glyphCount + 2);
    }
    else {
        rowInfo.m_rowRect.right += glyphWidth;
        ASSERT(nRowHeight == (int32_t)rowInfo.m_rowRect.Height());
    }

    RichTextCharInfo charInfo;
    charInfo.SetCharWidth(glyphWidth);    
    if (ch == '\r') {
        //回车
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo.AddCharFlag(RichTextCharFlag::kIsReturn);
        charInfo.SetCharWidth(0);
    }
    else if (ch == '\n') {
        //换行
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsNewLine);
        charInfo.SetCharWidth(0);
    }

    rowInfo.m_charInfo.emplace_back(std::move(charInfo));

    if (glyphChars == 2) {
        RichTextCharInfo charInfo2;
        charInfo2.SetCharWidth(0);
        charInfo2.SetCharFlag(0);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsLowSurrogate);
        rowInfo.m_charInfo.emplace_back(std::move(charInfo2));
    }
    ASSERT((glyphChars == 1) || (glyphChars == 2));
}

bool DrawRichText::IsValidDrawRichTextCache(const UiRect& textRect,
                                            const std::vector<RichTextData>& richTextData,
                                            const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }
    if (spDrawRichTextCache == nullptr) {
        return false;
    }
    if ((spDrawRichTextCache->m_textRect.Width() != textRect.Width()) ||
        (spDrawRichTextCache->m_textRect.Height() != textRect.Height())) {
        //矩形大小发生变化，不能使用缓存(位置变化时，可以使用缓存)
        return false;
    }
    if (spDrawRichTextCache->m_richTextData.size() != richTextData.size()) {
        return false;
    }
    bool bValid = true;
    const size_t nCount = richTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const RichTextData& textData = richTextData[nIndex];
        const RichTextData& textDataCache = spDrawRichTextCache->m_richTextData[nIndex];
        if (textData.m_textView.data() != textDataCache.m_textView.data()) {
            bValid = false;
        }
        else if (textData.m_textView.size() != textDataCache.m_textView.size()) {
            bValid = false;
        }
        else if (textData.m_textColor != textDataCache.m_textColor) {
            bValid = false;
        }
        else if (textData.m_bgColor != textDataCache.m_bgColor) {
            bValid = false;
        }
        else if (textData.m_pFontInfo != textDataCache.m_pFontInfo) {
            if ((textData.m_pFontInfo == nullptr) || (textDataCache.m_pFontInfo == nullptr)) {
                bValid = false;
            }
            else if (*textData.m_pFontInfo != *textDataCache.m_pFontInfo) {
                bValid = false;
            }
        }
        else if (textData.m_fRowSpacingMul != textDataCache.m_fRowSpacingMul) {
            bValid = false;
        }
        else if (textData.m_textStyle != textDataCache.m_textStyle) {
            bValid = false;
        }

        if (!bValid) {
            break;
        }
    }
    return bValid;
}

bool DrawRichText::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                           const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                           std::vector<RichTextData>& richTextDataNew,
                                           size_t nStartLine,
                                           const std::vector<size_t>& modifiedLines,
                                           size_t nModifiedRows,
                                           const std::vector<size_t>& deletedLines,
                                           size_t nDeletedRows,
                                           const std::vector<int32_t>& rowRectTopList)
{
    PerformanceStat statPerformance(_T("DrawRichText::UpdateDrawRichTextCache"));
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }
    ASSERT(spOldDrawRichTextCache != nullptr);
    if (spOldDrawRichTextCache == nullptr) {
        return false;
    }

    ASSERT(!modifiedLines.empty() || !deletedLines.empty());

    if (!modifiedLines.empty()) {
        if (nStartLine != modifiedLines[0]) {
            ASSERT(modifiedLines.empty() || (nStartLine == modifiedLines[0]));
            return false;
        }        
    }
    else if (!deletedLines.empty()) {
        if (nStartLine != deletedLines[0]) {
            ASSERT(deletedLines.empty() || (nStartLine == deletedLines[0]));
            return false;
        }
    }

    //删除的行数据，对应移除
    DrawRichTextCache& oldData = *spOldDrawRichTextCache;
    oldData.m_richTextData.swap(richTextDataNew);

    if (!deletedLines.empty()) {
        std::unordered_set<uint32_t> deletedLineSet;
        for (size_t nLine : deletedLines) {
            deletedLineSet.insert((uint32_t)nLine);
        }
        const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
        for (int32_t nIndex = nCount - 1; nIndex >= 0; --nIndex) {
            const TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
            if (deletedLineSet.find(pendingData.m_nLineNumber) != deletedLineSet.end()) {
                oldData.m_pendingTextData.erase(oldData.m_pendingTextData.begin() + nIndex);
            }
        }
    }

    //更新缓存开始的元素下标值    
    size_t nUpdateCacheStartIndex = (size_t)-1;
    //是否将修改的内容追加到最后了
    bool bAppendUpdateAtEnd = false;
    if (spUpdateDrawRichTextCache != nullptr) {
        DrawRichTextCache& updateData = *spUpdateDrawRichTextCache;
        if (!updateData.m_pendingTextData.empty()) {//容器可能为空（当本行为空行时为空）
            ASSERT(updateData.m_textRect == oldData.m_textRect);
            if (updateData.m_textRect != oldData.m_textRect) {
                return false;
            }
            ASSERT(updateData.m_textEncoding == oldData.m_textEncoding);
            if (updateData.m_textEncoding != oldData.m_textEncoding) {
                return false;
            }
            ASSERT(updateData.m_textCharSize == oldData.m_textCharSize);
            if (updateData.m_textCharSize != oldData.m_textCharSize) {
                return false;
            }
            //将新的绘制缓存，合并到原绘制缓存中
            const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
            for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
                const TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
                if (pendingData.m_nLineNumber > nStartLine) {
                    oldData.m_pendingTextData.insert(oldData.m_pendingTextData.begin() + nIndex, updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());
                    nUpdateCacheStartIndex = nIndex + updateData.m_pendingTextData.size();
                    break;
                }
            }
            if (nUpdateCacheStartIndex == (size_t)-1) {
                //追加在最后, 行号（物理行号，逻辑行号）无需更新，因为增量绘制的时候已经设置了正确的值
                bAppendUpdateAtEnd = true;
                oldData.m_pendingTextData.insert(oldData.m_pendingTextData.end(), updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());                
            }
        }
    }

    int32_t nLineNumberDiff = (int32_t)modifiedLines.size() - (int32_t)deletedLines.size();
    int32_t nRowIndexDiff = (int32_t)nModifiedRows - (int32_t)nDeletedRows;
    bool bUpdateIndex = (nLineNumberDiff != 0) || (nRowIndexDiff != 0);

    //修正物理行号，逻辑行号，本行的绘制目标区域值
    const size_t nRowRectCount = rowRectTopList.size();
    bool bUpdateLineRows = false;
    const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
        if (!bUpdateLineRows && bUpdateIndex) {
            if ((nUpdateCacheStartIndex != (size_t)-1)) {
                //更新行号(有修改，并且修改点不再最后)
                if (nIndex >= (int32_t)nUpdateCacheStartIndex) {
                    bUpdateLineRows = true;
                }
            }
            else if (!bAppendUpdateAtEnd) {
                //无修改，只有删除；或者新修改的行只是空行的情况
                if (pendingData.m_nLineNumber >= nStartLine) {
                    bUpdateLineRows = true;
                }
            }
        }
        if (bUpdateLineRows) {
            if (nLineNumberDiff > 0) {
                pendingData.m_nLineNumber += (uint32_t)nLineNumberDiff;
            }
            else if (nLineNumberDiff < 0) {
                pendingData.m_nLineNumber -= (uint32_t)-nLineNumberDiff;
            }
            if (nRowIndexDiff > 0) {
                pendingData.m_nRowIndex += (uint32_t)nRowIndexDiff;
            }
            else if (nRowIndexDiff < 0) {
                pendingData.m_nRowIndex -= (uint32_t)-nRowIndexDiff;
            }
        }

        if (pendingData.m_nLineNumber >= nStartLine) {
            //对应数据的索引下标更新为无效值，后续不再使用
            pendingData.m_nDataIndex = (uint32_t)-1;

            //更新本行的绘制目标区域
            ASSERT(pendingData.m_nRowIndex < nRowRectCount);
            if (pendingData.m_nRowIndex < nRowRectCount) {
                pendingData.m_destRect.bottom = rowRectTopList[pendingData.m_nRowIndex] + pendingData.m_destRect.Height();
                pendingData.m_destRect.top = rowRectTopList[pendingData.m_nRowIndex];
            }
        }
    }
    return true;
}

bool DrawRichText::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return false;
    }

    ASSERT(first.m_textRect == second.m_textRect);
    if (first.m_textRect != second.m_textRect) {
        return false;
    }

    ASSERT(first.m_textEncoding == second.m_textEncoding);
    if (first.m_textEncoding != second.m_textEncoding) {
        return false;
    }

    ASSERT(first.m_textCharSize == second.m_textCharSize);
    if (first.m_textCharSize != second.m_textCharSize) {
        return false;
    }

    ASSERT(first.m_richTextData.size() == second.m_richTextData.size());
    if (first.m_richTextData.size() != second.m_richTextData.size()) {
        return false;
    }
    const size_t nDataCount = first.m_richTextData.size();
    for (size_t nIndex = 0; nIndex < nDataCount; ++nIndex) {
        const RichTextData& v1 = first.m_richTextData[nIndex];
        const RichTextData& v2 = second.m_richTextData[nIndex];
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT((v1.m_pFontInfo != nullptr) && (v2.m_pFontInfo != nullptr));
        if ((v1.m_pFontInfo == nullptr) || (v2.m_pFontInfo == nullptr)) {
            return false;
        }
        ASSERT(*v1.m_pFontInfo == *v2.m_pFontInfo);
        if (*v1.m_pFontInfo != *v2.m_pFontInfo) {
            return false;
        }
        ASSERT(v1.m_fRowSpacingMul == v2.m_fRowSpacingMul);
        if (v1.m_fRowSpacingMul != v2.m_fRowSpacingMul) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }

    ASSERT(first.m_pendingTextData.size() == second.m_pendingTextData.size());
    if (first.m_pendingTextData.size() != second.m_pendingTextData.size()) {
        return false;
    }

    const size_t nCount = first.m_pendingTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const TPendingDrawRichText& v1 = *first.m_pendingTextData[nIndex];
        const TPendingDrawRichText& v2 = *second.m_pendingTextData[nIndex];

        //m_nDataIndex 此值不需要比较
        ASSERT(v1.m_nLineNumber == v2.m_nLineNumber);
        if (v1.m_nLineNumber != v2.m_nLineNumber) {
            return false;
        }
        ASSERT(v1.m_nRowIndex == v2.m_nRowIndex);
        if (v1.m_nRowIndex != v2.m_nRowIndex) {
            return false;
        }
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_destRect == v2.m_destRect);
        if (v1.m_destRect != v2.m_destRect) {
            return false;
        }

        ASSERT((v1.m_spFont != nullptr) && (v2.m_spFont != nullptr));
        if ((v1.m_spFont == nullptr) || (v2.m_spFont == nullptr)) {
            return false;
        }
        ASSERT(v1.m_spFont->FontName() == v2.m_spFont->FontName());
        if (v1.m_spFont->FontName() != v2.m_spFont->FontName()) {
            return false;
        }
        ASSERT(v1.m_spFont->FontSize() == v2.m_spFont->FontSize());
        if (v1.m_spFont->FontSize() != v2.m_spFont->FontSize()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsBold() == v2.m_spFont->IsBold());
        if (v1.m_spFont->IsBold() != v2.m_spFont->IsBold()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsUnderline() == v2.m_spFont->IsUnderline());
        if (v1.m_spFont->IsUnderline() != v2.m_spFont->IsUnderline()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsItalic() == v2.m_spFont->IsItalic());
        if (v1.m_spFont->IsItalic() != v2.m_spFont->IsItalic()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsStrikeOut() == v2.m_spFont->IsStrikeOut());
        if (v1.m_spFont->IsStrikeOut() != v2.m_spFont->IsStrikeOut()) {
            return false;
        }
        
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }
    return true;
}

void DrawRichText::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,                                       
                                         const UiRect& rcNewTextRect,
                                         const UiSize& szNewScrollOffset,
                                         const std::vector<int32_t>& rowXOffset,
                                         uint8_t uFade,
                                         std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("DrawRichText::DrawRichTextCacheData"));
    ASSERT((m_pRender != nullptr) && (m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pRender == nullptr) || (m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return;
    }

    ASSERT(spDrawRichTextCache != nullptr);
    if (spDrawRichTextCache == nullptr) {
        return;
    }

    const SkTextEncoding textEncoding = spDrawRichTextCache->m_textEncoding;
    const size_t textCharSize = spDrawRichTextCache->m_textCharSize;

    const std::vector<SharePtr<TPendingDrawRichText>>& pendingTextData = spDrawRichTextCache->m_pendingTextData;

    UiRect rcTemp;
    UiRect rcDestRect;
    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(spDrawRichTextCache->m_richTextData.size());
    }

    UiColor textColor;
    for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
        const TPendingDrawRichText& textData = *spTextData;
        //通过缓存绘制的时候，不能使用textData.m_nDataIndex值，此值再增量绘制的情况下是不正确的
        
        //执行绘制        
        rcDestRect = textData.m_destRect;
        rcDestRect.Offset(rcNewTextRect.left, rcNewTextRect.top);
        rcDestRect.Offset(-szNewScrollOffset.cx, -szNewScrollOffset.cy);
        if (!rowXOffset.empty()) {
            ASSERT(textData.m_nRowIndex < rowXOffset.size());
            if ((textData.m_nRowIndex < rowXOffset.size()) && (rowXOffset[textData.m_nRowIndex] > 0)) {
                rcDestRect.Offset(rowXOffset[textData.m_nRowIndex], 0);
            }
        }

        //记录绘制区域
        if (pRichTextRects != nullptr) {
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(rcDestRect);
        }

        if (!UiRect::Intersect(rcTemp, rcDestRect, rcNewTextRect)) {
            continue;
        }

        //绘制文字的背景色
        if (!textData.m_bgColor.IsEmpty()) {
            m_pRender->FillRect(rcDestRect, textData.m_bgColor, uFade);
        }

        //设置文本颜色
        if (textColor != textData.m_textColor) {
            const UiColor& color = textData.m_textColor;
            skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
            textColor = textData.m_textColor;
        }

        //绘制文字
        if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
            //绘制TAB键
            const char* text = (const char*)" ";
            const size_t len = 1; //字节数
            DrawTextString(rcDestRect, text, len, SkTextEncoding::kUTF8,
                            textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                            skPaint, textData.m_spFont.get());
        }
        else {
            const char* text = (const char*)textData.m_textView.data();
            size_t len = textData.m_textView.size() * textCharSize; //字节数
            DrawTextString(rcDestRect, text, len, textEncoding,
                           textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                           skPaint, textData.m_spFont.get());
        }
    }
}

void DrawRichText::DrawTextString(const UiRect& textRect, const DString& strText, uint32_t uFormat,
                                 const SkPaint& skPaint, IFont* pFont) const
{
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    const char* text = (const char*)strText.c_str();
    size_t len = strText.size() * sizeof(DString::value_type);
    SkTextEncoding textEncoding = GetTextEncoding();
    DrawTextString(textRect, text, len, textEncoding, uFormat, skPaint, pFont);
}

void DrawRichText::DrawTextString(const UiRect& textRect,
                                  const char* text, size_t len, SkTextEncoding textEncoding,
                                  uint32_t uFormat, const SkPaint& skPaint, IFont* pFont) const
{
    SkCanvas* skCanvas = m_pSkCanvas;
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return;
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    //绘制区域
    SkIRect rcSkDestI = { textRect.left, textRect.top, textRect.right, textRect.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    //设置绘制属性
    SkTextBox skTextBox;
    skTextBox.setBox(rcSkDest);
    if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        //单行文本
        skTextBox.setLineMode(SkTextBox::kOneLine_Mode);
    }

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
        bEndEllipsis = true;
    }
    skTextBox.setEndEllipsis(bEndEllipsis);

    bool bPathEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_PATH_ELLIPSIS) {
        bPathEllipsis = true;
    }
    skTextBox.setPathEllipsis(bPathEllipsis);

    //绘制文字时，不使用裁剪区域（可能会导致文字绘制超出边界）
    if (uFormat & DrawStringFormat::TEXT_NOCLIP) {
        skTextBox.setClipBox(false);
    }
    //删除线
    skTextBox.setStrikeOut(pSkiaFont->IsStrikeOut());
    //下划线
    skTextBox.setUnderline(pSkiaFont->IsUnderline());

    if (uFormat & DrawStringFormat::TEXT_HCENTER) {
        //横向对齐：居中对齐
        skTextBox.setTextAlign(SkTextBox::kCenter_Align);
    }
    else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
        //横向对齐：右对齐
        skTextBox.setTextAlign(SkTextBox::kRight_Align);
    }
    else {
        //横向对齐：左对齐
        skTextBox.setTextAlign(SkTextBox::kLeft_Align);
    }

    if (uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        skTextBox.setSpacingAlign(SkTextBox::kCenter_SpacingAlign);
    }
    else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：下对齐
        skTextBox.setSpacingAlign(SkTextBox::kEnd_SpacingAlign);
    }
    else {
        //纵向对齐：上对齐
        skTextBox.setSpacingAlign(SkTextBox::kStart_SpacingAlign);
    }
    skTextBox.draw(skCanvas, text, len, textEncoding, *pSkFont, skPaint);
}

SkTextEncoding DrawRichText::GetTextEncoding() const
{
    constexpr const size_t nValueLen = sizeof(DString::value_type);
    if constexpr (nValueLen == 1) {
        return SkTextEncoding::kUTF8;
    }
    else if constexpr (nValueLen == 2) {
        return SkTextEncoding::kUTF16;
    }
    else if constexpr (nValueLen == 4) {
        return SkTextEncoding::kUTF32;
    }
    else {
#ifdef DUILIB_UNICODE
        return SkTextEncoding::kUTF16;
#else
        return SkTextEncoding::kUTF8;
#endif
    }
}


} //namespace ui {
