#include "HorizontalDrawText.h"
#include "duilib/RenderSkia/Font_Skia.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/PerformanceUtil.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "SkiaHeaderEnd.h"

namespace ui {

HorizontalDrawText::HorizontalDrawText(SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg) :
    m_pSkCanvas(pSkCanvas),
    m_pSkPaint(pSkPaint),
    m_pSkPointOrg(pSkPointOrg)
{
}

UTF16String HorizontalDrawText::GetDrawStringUTF16(const DString& strText, bool bSingleLineMode) const
{
    DString text = strText;
    StringUtil::ReplaceAll(_T("\r\n"), _T("\n"), text);
    StringUtil::ReplaceAll(_T("\r"), _T(""), text);
    StringUtil::ReplaceAll(_T("\t"), _T(" "), text);
    if (bSingleLineMode) {
        StringUtil::ReplaceAll(_T("\n"), _T(" "), text);
    }
#if defined DUILIB_UNICODE && defined WCHAR_T_IS_UTF16
    return text;
#else
    std::string textUTF8 = StringConvert::TToUTF8(text);
    return StringConvert::UTF8ToUTF16(textUTF8.c_str(), textUTF8.size());
#endif
}

/** 横向绘制文本的字符属性
*/
struct THorizontalChar
{
    DUTF16Char ch;
    bool bNewLine;  //是否为换行符
    SkSize size;    //字符绘制后的宽度和高度
    SkRect bounds;  //字符绘制后的边界信息
};

bool HorizontalDrawText::CalculateTextCharBounds(const UTF16String& textUTF16, const SkFont* pSkFont, const SkPaint* skPaint,
                                                 float fFontHeight, std::vector<THorizontalChar>& charRects) const
{
    if (textUTF16.empty()) {
        return false;
    }
    ASSERT(fFontHeight > 0);
    if (fFontHeight <= 0) {
        return false;
    }
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return false;
    }
    ASSERT(skPaint != nullptr);
    if (skPaint == nullptr) {
        return false;
    }
    //每个字符绘制所占的矩形范围
    charRects.clear();
    charRects.reserve(textUTF16.size());

    THorizontalChar horizontalChar;
    for (DUTF16Char ch : textUTF16) {
        horizontalChar.ch = ch;
        if (ch == L'\n') {
            //换行符
            horizontalChar.bNewLine = true;
            horizontalChar.size = SkSize();
            horizontalChar.bounds = SkRect();
            charRects.push_back(horizontalChar);
        }
        else {
            horizontalChar.bNewLine = false;
            SkScalar fTextWidth = pSkFont->measureText(&ch,
                                                       sizeof(DUTF16Char),
                                                       SkTextEncoding::kUTF16,
                                                       &horizontalChar.bounds,//斜体字时，这个宽度包含了外延的宽度
                                                       skPaint);
            if ((horizontalChar.bounds.width() <= 0) || (horizontalChar.bounds.height() <= 0)) {
                //空格或者不可见字符(按小写字母确定显示区域)
                ch = 'a';
                fTextWidth = pSkFont->measureText(&ch,
                                                  sizeof(DUTF16Char),
                                                  SkTextEncoding::kUTF16,
                                                  &horizontalChar.bounds,//斜体字时，这个宽度包含了外延的宽度
                                                  skPaint);
            }

            //用字体高度作为字的高度，所有字都等高
            horizontalChar.size = SkSize::Make(std::max(fTextWidth, horizontalChar.bounds.width()), (SkScalar)fFontHeight);
            charRects.push_back(horizontalChar);
        }
    }
    return (charRects.size() == textUTF16.size());
}

SkRect HorizontalDrawText::CalculateHorizontalTextBounds(const std::vector<THorizontalChar>& charRects, int32_t width, bool bSingleLineMode,
                                                         float fSpacingMul, float fSpacingAdd, float fWordHorizontalSpacing,
                                                         float fDefaultCharWidth, float fDefaultCharHeight,
                                                         std::vector<std::vector<int32_t>>* pRowColumns,
                                                         std::vector<float>* pRowHeights,
                                                         std::vector<float>* pRowWidths) const
{
    if (charRects.empty()) {
        return SkRect::MakeEmpty();
    }
    if (width <= 0) {
        width = INT32_MAX; // 无限宽处理
    }
    if (fSpacingMul <= 0.01f) {
        fSpacingMul = 1.0f;
    }
    if (fSpacingAdd < 0) {
        fSpacingAdd = 0;
    }
    if (fWordHorizontalSpacing < 0) {
        fWordHorizontalSpacing = 0;
    }

    std::vector<std::vector<int32_t>> rowColumns;   // 每行每列字符在charRects中的索引
    std::vector<float> rowWidths;  // 每行宽度

    float currentX = 0.0f;       // 当前X起始位置（从左到右）
    float maxX = 0.0f;           // X最大值

    // 行列计数
    int32_t currentRow = 0;      // 当前行索引
    int32_t currentColumn = 0;   // 当前列索引

    bool bNextLine = false; // 标记是否换行
    const int32_t nCharCount = (int32_t)charRects.size();
    for (int32_t nCharIndex = 0; nCharIndex < nCharCount; ++nCharIndex) {
        const THorizontalChar& horizontalChar = charRects[nCharIndex];

        // 处理换行
        if (!bSingleLineMode && (horizontalChar.bNewLine || bNextLine)) {
            currentX = 0.0f;
            currentRow++;             // 行索引+1
            rowColumns.emplace_back();  // 新增一行
            rowWidths.emplace_back(fDefaultCharWidth); // 行宽
            currentColumn = 0;        // 新行从第0列开始

            bNextLine = false;
            if (horizontalChar.bNewLine) {
                continue;
            }
        }

        // 计算字符位置
        float charLeft = currentX;
        float charRight = charLeft + horizontalChar.size.width();

        // 记录字符行列索引
        while (rowColumns.size() <= (size_t)currentRow) {
            rowColumns.emplace_back(); // 不足则自动新增一行
        }
        // 将当前字符索引加入当前行的当前列
        rowColumns[currentRow].push_back(nCharIndex);

        // 记录行宽
        while (rowWidths.size() <= (size_t)currentRow) {
            rowWidths.emplace_back(fDefaultCharWidth); // 不足则自动新增一行
        }
        float& currentRowWidth = rowWidths[currentRow];
        currentRowWidth = std::max(currentRowWidth, charRight);

        // 更新X最大值
        maxX = std::max(maxX, charRight);

        // 计算下一个字符X位置
        float nextX = charRight;

        // 增加字间距（横向）
        nextX += fWordHorizontalSpacing;

        // 判断是否需要换行
        if (!bSingleLineMode) {
            bNextLine = nextX > width ? true : false;
            if (!bNextLine && (nCharIndex < (nCharCount - 1))) {
                const THorizontalChar& nextHorizontalChar = charRects[nCharIndex + 1];
                if (nextHorizontalChar.size.width() > 0) {
                    if ((nextX + (nextHorizontalChar.size.width() * 0.75f)) > width) {
                        bNextLine = true;
                    }
                }
            }
        }

        if (!bNextLine) {
            // 不换行，更新X位置和列索引
            currentX = nextX;
            currentColumn++;
        }
    }

    //无有效数据
    if (rowColumns.empty()) {
        return SkRect::MakeEmpty();
    }

    // 计算每行高度
    std::vector<float> rowHeights;
    rowHeights.resize(rowColumns.size());

    const size_t nRowCount = rowColumns.size();
    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        const std::vector<int32_t>& row = rowColumns[nRowIndex];
        SkScalar rowHeight = 0;
        const size_t nColumnCount = row.size();
        for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
            const size_t nCharIndex = (size_t)row[nColumnIndex];
            ASSERT(nCharIndex < charRects.size());
            if (nCharIndex < charRects.size()) {
                //绘制一个字           
                const THorizontalChar& horizontalChar = charRects[nCharIndex];
                rowHeight = std::max(rowHeight, horizontalChar.size.height());
            }
        }
        rowHeights[nRowIndex] = (float)rowHeight;
    }

    //检测只有一个换行符的空行
    const float fMinValue = 0.001f;
    float lastRowHeight = 0;
    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        float& rowHeight = rowHeights[nRowIndex];
        if (rowHeight < fMinValue) {
            if (lastRowHeight > fMinValue) {
                rowHeight = lastRowHeight;
            }
            else {
                for (size_t j = nRowIndex + 1; j < nRowCount; ++j) {
                    if (rowHeights[j] > fMinValue) {
                        rowHeight = rowHeights[j];
                        break;
                    }
                }
            }
        }
        else {
            lastRowHeight = rowHeight;
        }
    }

    // 如果都是空行，则重新计算(兜底处理)
    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        if (rowHeights[nRowIndex] < fMinValue) {
            rowHeights[nRowIndex] = fDefaultCharHeight;
        }
    }

    // 计算总高度
    SkScalar maxY = 0;
    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        float fHeight = rowHeights[nRowIndex];
        if (nRowIndex != 0) {
            fHeight = fHeight * fSpacingMul + fSpacingAdd;
        }
        maxY += fHeight;
    }
    maxX = std::max(maxX, fDefaultCharWidth); //只有换行符，无其他字符时，需要使用默认的字符宽度

    if (pRowColumns != nullptr) {
        pRowColumns->swap(rowColumns);
    }
    if (pRowHeights != nullptr) {
        pRowHeights->swap(rowHeights);
    }
    if (pRowWidths != nullptr) {
        pRowWidths->swap(rowWidths);
    }
    return SkRect::MakeWH(maxX, maxY);
}

float HorizontalDrawText::CalculateDefaultCharWidth(const SkFont* pSkFont, const SkPaint* skPaint) const
{
    if ((pSkFont == nullptr) || (skPaint == nullptr)) {
        return 0;
    }
    DUTF16Char ch = L'W';
    SkRect bounds;
    SkScalar fCharWidth = pSkFont->measureText(&ch,
                                               sizeof(DUTF16Char),
                                               SkTextEncoding::kUTF16,
                                               &bounds,//斜体字时，这个宽度包含了外延的宽度
                                               skPaint);

    SkScalar nWidthDiff = 0;
    if (bounds.fLeft < 0) {
        //斜体字左侧溢出
        nWidthDiff += -bounds.fLeft;
    }
    if (bounds.fRight > fCharWidth) {
        //斜体字右侧溢出
        nWidthDiff += bounds.fRight - fCharWidth;
    }
    fCharWidth += nWidthDiff;
    return fCharWidth;
}

UiRect HorizontalDrawText::MeasureString(const DString& strText, const MeasureStringParam& measureParam)
{
    PerformanceStat statPerformance(_T("HorizontalDrawText::MeasureString"));
    ASSERT((m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return UiRect();
    }
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return UiRect();
    }
    ASSERT(measureParam.pFont != nullptr);
    if (measureParam.pFont == nullptr) {
        return UiRect();
    }

    //获取字体接口
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(measureParam.pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return UiRect();
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return UiRect();
    }

    //绘制属性设置
    SkPaint skPaint = *m_pSkPaint;

    bool bSingleLineMode = (measureParam.uFormat & DrawStringFormat::TEXT_SINGLELINE) != 0;

    // 计算字体高度（作为字行高）
    SkFontMetrics fontMetrics;
    const SkScalar fFontHeight = pSkFont->getMetrics(&fontMetrics);
    ASSERT(fFontHeight > 0);
    if (fFontHeight <= 0) {
        return UiRect();
    }

    //绘制文本始终使用UTF16编码
    const UTF16String textUTF16 = GetDrawStringUTF16(strText, bSingleLineMode);

    std::vector<THorizontalChar> charRects;
    if (!CalculateTextCharBounds(textUTF16, pSkFont, &skPaint, (float)fFontHeight, charRects)) {
        return UiRect();
    }
    ASSERT(charRects.size() == textUTF16.size());
    if (charRects.size() != textUTF16.size()) {
        return UiRect();
    }

    //默认字符的宽度
    float fDefaultCharWidth = CalculateDefaultCharWidth(pSkFont, &skPaint);
    SkRect skTextBounds = CalculateHorizontalTextBounds(charRects, measureParam.rectSize, bSingleLineMode,
                                                        measureParam.fSpacingMul, measureParam.fSpacingAdd,
                                                        measureParam.fWordSpacing,
                                                        fDefaultCharWidth, fFontHeight,
                                                        nullptr, nullptr, nullptr);

    int32_t nTextWidth = SkScalarTruncToInt(skTextBounds.width() + 0.5f);
    if (skTextBounds.width() > nTextWidth) {
        nTextWidth += 1;
    }
    int32_t nTextHeight = SkScalarTruncToInt(skTextBounds.height() + 0.5f);
    if (skTextBounds.height() > nTextHeight) {
        nTextHeight += 1;
    }
    return UiRect(0, 0, nTextWidth, nTextHeight);
}

void HorizontalDrawText::DrawString(const DString& strText, const DrawStringParam& drawParam)
{
    // 备注：横向文本绘制不支持以下功能
    // 1. 文本风格：DrawStringFormat::TEXT_PATH_ELLIPSIS 不支持，按DrawStringFormat::TEXT_END_ELLIPSIS处理
    PerformanceStat statPerformance(_T("HorizontalDrawText::DrawString"));
    ASSERT((m_pSkCanvas != nullptr) && (m_pSkPaint != nullptr) && (m_pSkPointOrg != nullptr));
    if ((m_pSkCanvas == nullptr) || (m_pSkPaint == nullptr) || (m_pSkPointOrg == nullptr)) {
        return;
    }

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    ASSERT(!drawParam.textRect.IsEmpty());
    if (drawParam.textRect.IsEmpty()) {
        return;
    }

    ASSERT(drawParam.pFont != nullptr);
    if (drawParam.pFont == nullptr) {
        return;
    }

    SkCanvas* skCanvas = m_pSkCanvas;

    //获取字体接口    
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(drawParam.pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return;
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    // 设置绘制属性
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(drawParam.dwTextColor.GetA(), drawParam.dwTextColor.GetR(),
                    drawParam.dwTextColor.GetG(), drawParam.dwTextColor.GetB());
    if (drawParam.uFade != 0xFF) {
        skPaint.setAlpha(drawParam.uFade);
    }

    // 绘制区域
    SkIRect rcSkDestI = { drawParam.textRect.left, drawParam.textRect.top,
                          drawParam.textRect.right, drawParam.textRect.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    bool bSingleLineMode = (drawParam.uFormat & DrawStringFormat::TEXT_SINGLELINE) != 0;

    // 计算字体高度
    SkFontMetrics fontMetrics;
    const SkScalar fFontHeight = pSkFont->getMetrics(&fontMetrics);
    ASSERT(fFontHeight > 0);
    if (fFontHeight <= 0) {
        return;
    }

    // 行间距设置
    float fSpacingMul = drawParam.fSpacingMul;
    float fSpacingAdd = drawParam.fSpacingAdd;
    float fWordHorizontalSpacing = drawParam.fWordSpacing;
    if (fSpacingMul <= 0.01f) {
        fSpacingMul = 1.0f;
    }
    if (fSpacingAdd < 0) {
        fSpacingAdd = 0;
    }
    if (fWordHorizontalSpacing < 0) {
        fWordHorizontalSpacing = 0;
    }

    //绘制文本始终使用UTF16编码
    const UTF16String textUTF16 = GetDrawStringUTF16(strText, bSingleLineMode);

    std::vector<THorizontalChar> charRects;
    if (!CalculateTextCharBounds(textUTF16, pSkFont, &skPaint, (float)fFontHeight, charRects)) {
        return;
    }
    ASSERT(charRects.size() == textUTF16.size());
    if (charRects.size() != textUTF16.size()) {
        return;
    }

    // 默认字符宽度
    float fDefaultCharWidth = CalculateDefaultCharWidth(pSkFont, &skPaint);

    std::vector<std::vector<int32_t>> rowColumns;
    std::vector<float> rowHeights;
    std::vector<float> rowWidths;
    SkRect skTextBounds = CalculateHorizontalTextBounds(charRects, drawParam.textRect.Width(), bSingleLineMode,
                                                        fSpacingMul, fSpacingAdd, fWordHorizontalSpacing,
                                                        fDefaultCharWidth, fFontHeight,
                                                        &rowColumns, &rowHeights, &rowWidths);

    ASSERT(rowColumns.size() == rowHeights.size());
    if (rowColumns.size() != rowHeights.size()) {
        return;
    }

    //记录每个字符的绘制位置，后续还需要处理对齐方式
    struct TDrawCharPos
    {
        DUTF16Char ch = 0;          //字符
        int32_t nRowIndex = 0;      //行序号
        int32_t nColumnIndex = 0;   //列序号
        SkScalar xPos = 0;          //绘制时的X坐标
        SkScalar yPos = 0;          //绘制时的Y坐标
        int32_t chWidth = 0;        //字符宽度（用于绘制删除线和下划线）
        bool bDrew = false;         //该字符是否完成绘制（用于绘制删除线和下划线）
    };
    std::vector<TDrawCharPos> drawCharPos;
    drawCharPos.reserve(charRects.size());

    // 每行字符数
    std::vector<int32_t> rowColumnCount;
    rowColumnCount.resize(rowColumns.size(), 0);

    // 计算绘制位置（按行绘制）
    SkScalar yPos = rcSkDest.top();
    const size_t nRowCount = rowColumns.size();
    for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
        float fRowHeight = rowHeights[nRowIndex]; //行高（已经预计算）
        SkScalar xPos = rcSkDest.left();
        const std::vector<int32_t>& column = rowColumns[nRowIndex];
        const size_t nColCount = column.size();
        rowColumnCount[nRowIndex] = (int32_t)nColCount;//记录每一行中有几个字符

        for (size_t nColIndex = 0; nColIndex < nColCount; ++nColIndex) {
            const size_t nCharIndex = (size_t)column[nColIndex];
            ASSERT(nCharIndex < charRects.size());
            if (nCharIndex >= charRects.size()) {
                continue;
            }
            //绘制一个字
            TDrawCharPos& charPos = drawCharPos.emplace_back(); //新增一个字符
            const THorizontalChar& horizontalChar = charRects[nCharIndex];

            //记录该字符的绘制位置，处理对齐方式以后再绘制
            charPos.ch = horizontalChar.ch;
            charPos.nRowIndex = (int32_t)nRowIndex;
            charPos.nColumnIndex = (int32_t)nColIndex;
            charPos.chWidth = (int32_t)horizontalChar.size.width();

            // 水平位置
            charPos.xPos = xPos + (horizontalChar.bounds.left() < 0 ? -horizontalChar.bounds.left() : 0);

            // 垂直位置, 所有字体等高
            charPos.yPos = yPos - fontMetrics.fAscent;

            // 更新下一个字符X位置
            xPos += (horizontalChar.size.width() + fWordHorizontalSpacing);
        }

        // 更新下一行Y位置
        yPos += (fRowHeight * fSpacingMul + fSpacingAdd);
    }

    // 垂直方向对齐：所有绘制内容整体对齐
    int32_t textBoundHeight = SkScalarTruncToInt(skTextBounds.height() + 0.5f);
    if (textBoundHeight < drawParam.textRect.Height()) {
        int32_t nOffsetY = 0;
        if (drawParam.uFormat & DrawStringFormat::TEXT_VCENTER) {
            // 纵向对齐：居中对齐(下移)
            nOffsetY = (drawParam.textRect.Height() - textBoundHeight) / 2;
        }
        else if (drawParam.uFormat & DrawStringFormat::TEXT_BOTTOM) {
            // 纵向对齐：靠下对齐(下移)
            nOffsetY = (drawParam.textRect.Height() - textBoundHeight);
        }
        else {
            //纵向对齐：靠上对齐(默认，不处理)
            nOffsetY = 0;
        }
        if (nOffsetY != 0) {
            for (TDrawCharPos& charPos : drawCharPos) {
                charPos.yPos += nOffsetY;
            }
        }
    }

    //水平方向对齐：按每列的占用的实际区域对齐
    std::vector<float> rowOffsets;
    if (drawParam.uFormat & DrawStringFormat::TEXT_HCENTER) {
        //水平对齐：居中对齐
        rowOffsets.resize(rowWidths.size(), 0.0f);
        int32_t textRectWidth = drawParam.textRect.Width();
        for (size_t nRowIndex = 0; nRowIndex < rowWidths.size(); ++nRowIndex) {
            if (rowWidths[nRowIndex] < textRectWidth) {
                rowOffsets[nRowIndex] = (textRectWidth - rowWidths[nRowIndex]) / 2;
            }
        }
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_RIGHT) {
        //水平对齐：靠右对齐
        rowOffsets.resize(rowWidths.size(), 0.0f);
        int32_t textRectWidth = drawParam.textRect.Width();
        for (size_t nRowIndex = 0; nRowIndex < rowWidths.size(); ++nRowIndex) {
            if (rowWidths[nRowIndex] < textRectWidth) {
                rowOffsets[nRowIndex] = (textRectWidth - rowWidths[nRowIndex]);
            }
        }
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_HJUSTIFY) {
        //水平对齐：两端对齐
        ASSERT(rowColumnCount.size() == rowWidths.size());
        if (!rowColumnCount.empty() && rowColumnCount.size() == rowWidths.size()) {
            std::vector<std::vector<float>> rowColOffsets; //每个字的偏移
            rowColOffsets.resize(rowWidths.size());

            int32_t textRectWidth = drawParam.textRect.Width();
            for (size_t nRowIndex = 0; nRowIndex < rowWidths.size(); ++nRowIndex) {
                const int32_t nColCount = rowColumnCount[nRowIndex];
                // 计算每个字的偏移
                rowColOffsets[nRowIndex].resize(nColCount, 0.0f);

                if (nColCount < 2) {
                    //该行的字数少于2，不需要处理两端对齐
                    continue;
                }

                if (rowWidths[nRowIndex] < textRectWidth) {
                    float fOffsetTotal = (textRectWidth - rowWidths[nRowIndex]); //总的偏移
                    float fOffset = fOffsetTotal / (nColCount - 1); //每个字符的偏移量

                    for (int32_t nCol = 0; nCol < nColCount; ++nCol) {
                        rowColOffsets[nRowIndex][nCol] = fOffset * nCol;
                    }
                }
            }

            //为每个字符设置横向偏移
            const int32_t nOffsetRowCount = (int32_t)rowColOffsets.size();
            for (TDrawCharPos& charPos : drawCharPos) {
                ASSERT(charPos.nRowIndex < nOffsetRowCount);
                if (charPos.nRowIndex < nOffsetRowCount) {
                    const std::vector<float>& colOffsets = rowColOffsets[charPos.nRowIndex];
                    ASSERT(charPos.nColumnIndex < (int32_t)colOffsets.size());
                    if (charPos.nColumnIndex < (int32_t)colOffsets.size()) {
                        charPos.xPos += colOffsets[charPos.nColumnIndex];
                    }
                }
            }
        }
    }
    else {
        //水平对齐：靠左对齐(默认，不处理)
    }

    if (rowOffsets.size() == rowWidths.size()) {
        const size_t nOffsetCount = rowOffsets.size();
        for (TDrawCharPos& charPos : drawCharPos) {
            if (charPos.nRowIndex < nOffsetCount) {
                charPos.xPos += rowOffsets[charPos.nRowIndex];
            }
        }
    }

    //绘制每个字符
    
    //将范围扩大些，避免误判导致字无法绘制出来
    SkRect skTextRect = rcSkDest;
    skTextRect.fLeft -= fFontHeight;
    skTextRect.fRight += fFontHeight;
    skTextRect.fTop -= fFontHeight;
    skTextRect.fBottom += fFontHeight;

    bool bNeedEllipsis = false;
    if ((drawParam.uFormat & TEXT_PATH_ELLIPSIS) || (drawParam.uFormat & TEXT_END_ELLIPSIS)) {
        //如果绘制区域不足，在结尾加"..."，省略部分文字(不支持TEXT_PATH_ELLIPSIS，按TEXT_END_ELLIPSIS处理)
        bNeedEllipsis = true;
    }

    //析构时，自动还原剪辑区域
    struct AutoClipInfo
    {
        bool bCliped = false;
        int drawSaveCount = 0;
        SkCanvas* skCanvas = nullptr;

        ~AutoClipInfo()
        {
            if (bCliped && (skCanvas != nullptr)) {
                skCanvas->restoreToCount(drawSaveCount);
            }
        }
    };

    //开始绘制
    AutoClipInfo autoClip;
    if (!(drawParam.uFormat & TEXT_NOCLIP)) {
        //设置剪辑区域
        autoClip.bCliped = true;
        autoClip.drawSaveCount = skCanvas->save();
        autoClip.skCanvas = skCanvas;
        skCanvas->clipRect(rcSkDest, true);
    }

    // 绘制字符
    const size_t drawCharCount = drawCharPos.size();
    for (size_t charIndex = 0; charIndex < drawCharCount; ++charIndex) {
        TDrawCharPos& charPos = drawCharPos[charIndex];
        if (bSingleLineMode && charPos.nRowIndex != 0) {
            //单行模式，只绘制一行
            break;
        }

        if (bNeedEllipsis && (charIndex < (drawCharCount - 1))) {// 是否绘制"..."的业务逻辑
            //判断下一个字符是否越界（横向）
            float fRowHeight = 0;
            if (charPos.nRowIndex < rowHeights.size()) {
                fRowHeight = rowHeights[charPos.nRowIndex]; //行高
            }
            bool bNeedDrawEllipsis = false;
            const TDrawCharPos& nextCharPos = drawCharPos[charIndex + 1];
            if (nextCharPos.nRowIndex == charPos.nRowIndex) {
                SkScalar xPos = nextCharPos.xPos;
                xPos += nextCharPos.chWidth;
                if ((xPos > (SkScalar)rcSkDest.fRight) &&
                    (bSingleLineMode || (charPos.yPos + fRowHeight) > (SkScalar)rcSkDest.fBottom)) {
                    //下个字符无法显示了
                    bNeedDrawEllipsis = true;
                }
            }
            else if (!bSingleLineMode && (charPos.yPos + fRowHeight) > (SkScalar)rcSkDest.fBottom) {
                //下方其他列的数据无法显示了
                bNeedDrawEllipsis = true;
            }

            if (bNeedDrawEllipsis) {
                //字符越界，绘制 "..."
                std::string ellipsis = "...";
                skCanvas->drawSimpleText(ellipsis.data(), ellipsis.size(), SkTextEncoding::kUTF8,
                                         charPos.xPos, charPos.yPos,
                                         *pSkFont, skPaint);
                //越界后不再绘制
                break;
            }
        }

        if (!skTextRect.contains(charPos.xPos, charPos.yPos)) {
            //当前字符不在文本显示区域，不绘制
            continue;
        }

        charPos.bDrew = true;
        skCanvas->drawSimpleText(&charPos.ch, sizeof(charPos.ch), SkTextEncoding::kUTF16,
                                 charPos.xPos, charPos.yPos,
                                 *pSkFont, skPaint);
    }

    // 绘制下划线/删除线
    if ((drawParam.pFont->IsUnderline() || drawParam.pFont->IsStrikeOut()) && !rowHeights.empty()) {
        struct TextRowInfo
        {
            UiPoint ptStart;
            UiPoint ptEnd;
            bool bStartFlag = false;
            bool bEndFlag = false;
            int32_t chStartWidth = 0;
            int32_t chEndWidth = 0;
        };
        std::vector<TextRowInfo> textRowInfo;
        textRowInfo.resize(rowHeights.size());

        for (const TDrawCharPos& charPos : drawCharPos) {
            if (!charPos.bDrew) {
                continue;
            }
            ASSERT(charPos.nRowIndex <= (int32_t)textRowInfo.size());
            if (charPos.nRowIndex >= (int32_t)textRowInfo.size()) {
                continue;
            }
            TextRowInfo& textRow = textRowInfo[charPos.nRowIndex];
            if (!textRow.bStartFlag) {
                textRow.bStartFlag = true;
                textRow.ptStart.x = (int32_t)charPos.xPos;
                textRow.ptStart.y = (int32_t)(charPos.yPos + 0.5f);
                textRow.chStartWidth = charPos.chWidth;
            }
            else {
                textRow.bEndFlag = true;
                textRow.ptEnd.x = (int32_t)charPos.xPos;
                textRow.ptEnd.y = (int32_t)charPos.yPos;
                textRow.chEndWidth = charPos.chWidth;
            }
        }

        // 绘制线条
        for (size_t nRowIndex = 0; nRowIndex < textRowInfo.size(); ++nRowIndex) {
            const TextRowInfo& textRow = textRowInfo[nRowIndex];
            if (!textRow.bStartFlag && !textRow.bEndFlag) {
                continue;
            }
            SkScalar left = (SkScalar)textRow.ptStart.x;
            SkScalar right = left + (SkScalar)textRow.chStartWidth;
            if (textRow.bEndFlag) {
                right = (SkScalar)textRow.ptEnd.x + (SkScalar)textRow.chEndWidth;
            }
            SkScalar width = right - left;

            SkScalar x = (SkScalar)textRow.ptStart.x;
            SkScalar y = (SkScalar)textRow.ptStart.y;
            if (textRow.bEndFlag) {
                y = ((SkScalar)textRow.ptStart.y + (SkScalar)textRow.ptEnd.y) / 2 ;
            }

            // Default fraction of the text size to use for a strike-through or underline.
            static constexpr SkScalar kLineThicknessFactor = (SK_Scalar1 / 18);
            // Fraction of the text size to raise the center of a strike-through line above
            // the baseline.
            const SkScalar kStrikeThroughOffset = (SK_Scalar1 * 65 / 252);
            // Fraction of the text size to lower an underline below the baseline.
            const SkScalar kUnderlineOffset = (SK_Scalar1 / 9);

            if (drawParam.pFont->IsStrikeOut()) {
                //绘制删除线
                SkScalar thickness_factor = kLineThicknessFactor;
                const SkScalar text_size = pSkFont->getSize();
                const SkScalar height = text_size * thickness_factor;
                const SkScalar top = y - text_size * kStrikeThroughOffset - height / 2;
                SkScalar x_scalar = SkIntToScalar(x);
                const SkRect r = SkRect::MakeLTRB(x_scalar, top, x_scalar + width, top + height);
                skCanvas->drawRect(r, skPaint);
            }
            if (drawParam.pFont->IsUnderline()) {
                //绘制下划线
                SkScalar thickness_factor = 1.5;
                SkScalar x_scalar = SkIntToScalar(x);
                const SkScalar text_size = pSkFont->getSize();
                SkRect r = SkRect::MakeLTRB(
                    x_scalar, y + text_size * kUnderlineOffset, x_scalar + width,
                    y + (text_size *
                        (kUnderlineOffset +
                            (thickness_factor * kLineThicknessFactor))));
                skCanvas->drawRect(r, skPaint);
            }
        }
    }
}

} // namespace ui
