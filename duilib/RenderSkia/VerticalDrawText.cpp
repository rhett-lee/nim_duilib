#include "VerticalDrawText.h"
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

VerticalDrawText::VerticalDrawText(SkCanvas* pSkCanvas, SkPaint* pSkPaint, SkPoint* pSkPointOrg) :
    m_pSkCanvas(pSkCanvas),
    m_pSkPaint(pSkPaint),
    m_pSkPointOrg(pSkPointOrg)
{
}


UTF16String VerticalDrawText::GetDrawStringUTF16(const DString& strText, bool bSingleLineMode) const
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

bool VerticalDrawText::NeedRotateForVertical(DUTF16Char ch) const
{
    // 拉丁字母、数字、通用符号：需要旋转
    if (
        // 大写字母
        (ch >= 0x41 && ch <= 0x5A) ||
        // 小写字母
        (ch >= 0x61 && ch <= 0x7A) ||
        // 数字
        (ch >= 0x30 && ch <= 0x39) ||
        // 基础标点符号（!@#$%^&*等）
        (ch >= 0x21 && ch <= 0x2F) ||
        (ch >= 0x3A && ch <= 0x40) ||
        (ch >= 0x5B && ch <= 0x60) ||
        (ch >= 0x7B && ch <= 0x7E) ||
        // 半角标点
        (ch >= 0xFF01 && ch <= 0xFF5E) ||
        // 其他符号（如箭头、数学符号等）
        (ch >= 0x2190 && ch <= 0x21FF) ||
        (ch >= 0x2200 && ch <= 0x22FF)
        ) {
        return true; // 需要旋转90度
    }

    // 其他字符：默认不旋转
    return false;
}

/** 纵向绘制文本的字符属性
*/
struct TVerticalChar
{
    DUTF16Char ch;
    bool bNewLine;  //是否为换行符
    bool bRotate90; //该字符绘制时，需要旋转90度显示
    SkSize size;    //字符绘制后的宽度和高度
    SkRect bounds;  //字符绘制后的边界信息
};

bool VerticalDrawText::CalculateTextCharBounds(const UTF16String& textUTF16, const SkFont* pSkFont, const SkPaint* skPaint,
                                               bool bUseFontHeight, float fFontHeight, bool bRotate90ForAscii,
                                               std::vector<TVerticalChar>& charRects) const
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

    TVerticalChar verticalChar;
    for (DUTF16Char ch : textUTF16) {
        verticalChar.ch = ch;
        if (ch == L'\n') {
            //换行符
            verticalChar.bNewLine = true;
            verticalChar.bRotate90 = false;
            verticalChar.size = SkSize();
            verticalChar.bounds = SkRect();
            charRects.push_back(verticalChar);
        }
        else {
            verticalChar.bNewLine = false;
            verticalChar.bRotate90 = false;
            SkScalar fTextWidth = pSkFont->measureText(&ch,
                                                       sizeof(DUTF16Char),
                                                       SkTextEncoding::kUTF16,
                                                       &verticalChar.bounds,//斜体字时，这个宽度包含了外延的宽度
                                                       skPaint);
            if ((verticalChar.bounds.width() <= 0) || (verticalChar.bounds.height() <= 0)) {
                //空格或者不可见字符(按小写字母确定显示区域)
                ch = 'a';
                fTextWidth = pSkFont->measureText(&ch,
                                                  sizeof(DUTF16Char),
                                                  SkTextEncoding::kUTF16,
                                                  &verticalChar.bounds,//斜体字时，这个宽度包含了外延的宽度
                                                  skPaint);
            }
            if (bUseFontHeight) {
                //用字体高度作为字的高度，所有字都等高
                verticalChar.size = SkSize::Make(std::max(fTextWidth, verticalChar.bounds.width()), (SkScalar)fFontHeight);
            }
            else {
                //用字体的实际高度，作为字的高度
                verticalChar.size = SkSize::Make(std::max(fTextWidth, verticalChar.bounds.width()), verticalChar.bounds.height()); // 用实际高度
            }
            if (bRotate90ForAscii && NeedRotateForVertical(verticalChar.ch)) {
                verticalChar.bRotate90 = true;
                std::swap(verticalChar.size.fHeight, verticalChar.size.fWidth);
            }
            charRects.push_back(verticalChar);
        }
    }
    ASSERT(charRects.size() == textUTF16.size());
    return (charRects.size() == textUTF16.size());
}

SkRect VerticalDrawText::CalculateVerticalTextBounds(const std::vector<TVerticalChar>& charRects, int32_t height, bool bSingleLineMode,
                                                     float fSpacingMul, float fSpacingAdd, float fWordVerticalSpacing,
                                                     float fDefaultCharWidth, float fDefaultCharHeight,
                                                     std::vector<std::vector<int32_t>>* pColumnRows,
                                                     std::vector<float>* pColumnWidths,
                                                     std::vector<float>* pColumnHeights) const
{
    if (charRects.empty()) {
        return SkRect::MakeEmpty();
    }
    if (height <= 0) {
        height = INT32_MAX; // 无限高处理
    }
    if (fSpacingMul <= 0.01f) {
        fSpacingMul = 1.0f;
    }
    if (fSpacingAdd < 0) {
        fSpacingAdd = 0;
    }
    if (fWordVerticalSpacing < 0) {
        fWordVerticalSpacing = 0;
    }

    std::vector<std::vector<int32_t>> columnRows;   // 每列每行字符在charRects容器中对应的下标值
    std::vector<float> columnHeights;  //返回每列的列高

    float currentY = 0.0f;       // 当前行的Y起始位置（从上到下绘制）
    float maxY = 0.0f;           // 纵坐标的Y值最大值

    // 行列计数（从0开始，与vector索引一致）
    int32_t currentColumn = 0;   // 当前列索引（0-based）
 
    bool bNextRow = false; // 标记是否换行
    const int32_t nCharCount = (int32_t)charRects.size();
    for (int32_t nCharIndex = 0; nCharIndex < nCharCount; ++nCharIndex) {
        const TVerticalChar& verticalChar = charRects[nCharIndex];

        // 处理换行
        if (!bSingleLineMode && (verticalChar.bNewLine || bNextRow)) {
            // 换行到左侧新列
            currentY = 0.0f;
            currentColumn++;            // 列索引+1
            columnRows.emplace_back();  // 新增一列
            columnHeights.emplace_back(fDefaultCharHeight); //列高，新增1列

            bNextRow = false;
            if (verticalChar.bNewLine) {
                continue;
            }
        }

        // 计算字符实际绘制位置
        float charTop = currentY;
        float charBottom = charTop + verticalChar.size.height();

        // 记录当前字符的行列索引
        while (columnRows.size() <= (size_t)currentColumn) {
            columnRows.emplace_back(); // 不足则自动新增一列
        }
        // 将当前字符索引加入当前列的当前行
        columnRows[currentColumn].push_back(nCharIndex);

        // 记录当前的列高
        while (columnHeights.size() <= (size_t)currentColumn) {
            columnHeights.emplace_back(fDefaultCharHeight); // 不足则自动新增一列
        }
        float& currentColumnHeight = columnHeights[currentColumn];
        currentColumnHeight = std::max(currentColumnHeight, charBottom);

        // 记录字符纵坐标的最大值
        maxY = std::max(maxY, charBottom);

        // 计算下一个字符的Y位置（当前字符底部）
        float nextY = charBottom;

        // 增加字间距（纵向）
        nextY += fWordVerticalSpacing;

        // 标记是否换行
        if (!bSingleLineMode) {
            bNextRow = nextY > height ? true : false;
            if (!bNextRow && (nCharIndex < (nCharCount - 1))) {
                // 判断下一个字是否能够绘制出来, 如果无法绘制出来，则换行
                const TVerticalChar& nextVerticalChar = charRects[nCharIndex + 1];
                if (nextVerticalChar.size.height() > 0) {
                    if ((nextY + (nextVerticalChar.size.height() * 0.75f)) > height) {
                        bNextRow = true;
                    }
                }
            }
        }
        if (!bNextRow) {
            // 不换行，更新Y位置和行索引
            currentY = nextY;
        }
    }

    //无有效数据
    if (columnRows.empty()) {
        return SkRect::MakeEmpty();
    }

    //计算每列的宽度
    std::vector<float> columnWidths;  //返回每列的列宽
    columnWidths.resize(columnRows.size());

    const size_t nColumnCount = columnRows.size();
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        const std::vector<int32_t>& row = columnRows[nColumnIndex];
        SkScalar columnWidth = 0;
        const size_t nRowCount = row.size();
        for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
            const size_t nCharIndex = (size_t)row[nRowIndex];
            ASSERT(nCharIndex < charRects.size());
            if (nCharIndex < charRects.size()) {
                //绘制一个字           
                const TVerticalChar& verticalChar = charRects[nCharIndex];
                SkScalar nWidthDiff = 0;
                if (verticalChar.bounds.fLeft < 0) {
                    //斜体字左侧溢出
                    nWidthDiff += -verticalChar.bounds.fLeft;
                }
                if (verticalChar.bounds.fRight > verticalChar.size.width()) {
                    //斜体字右侧溢出
                    nWidthDiff += verticalChar.bounds.fRight - verticalChar.size.width();
                }
                columnWidth = std::max(columnWidth, verticalChar.size.width() + nWidthDiff);
            }
        }
        columnWidths[nColumnIndex] = (float)columnWidth;
    }

    //检测只有一个换行符的空列
    const float fMinValue = 0.001f;
    float lastColumnWidth = 0;
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        float& columnWidth = columnWidths[nColumnIndex];
        if (columnWidth < fMinValue) {
            if (lastColumnWidth > fMinValue) {
                columnWidth = lastColumnWidth;
            }
            else {
                for (size_t j = nColumnIndex + 1; j < nColumnCount; ++j) {
                    if (columnWidths[j] > fMinValue) {
                        columnWidth = columnWidths[j];
                        break;
                    }
                }
            }
        }
        else {
            lastColumnWidth = columnWidth;
        }
    }

    //如果都是空列，则重新计算(兜底处理)
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        float& columnWidth = columnWidths[nColumnIndex];
        if (columnWidth < fMinValue) {
            columnWidth = fDefaultCharWidth;
        }
    }

    // 计算总宽度
    SkScalar maxX = 0;
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        float fWidth = columnWidths[nColumnIndex];
        if (nColumnIndex != 0) {
            fWidth = fWidth * fSpacingMul + fSpacingAdd;
        }
        maxX += fWidth;
    }
    maxY = std::max(maxY, fDefaultCharHeight); //只有换行符，无其他字符时，需要使用默认的字符高度

    if (pColumnRows != nullptr) {
        pColumnRows->swap(columnRows);
    }
    if (pColumnWidths != nullptr) {
        pColumnWidths->swap(columnWidths);
    }
    if (pColumnHeights != nullptr) {
        pColumnHeights->swap(columnHeights);
    }
    return SkRect::MakeWH(maxX, maxY);
}

float VerticalDrawText::CalculateDefaultCharWidth(const SkFont* pSkFont, const SkPaint* skPaint) const
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

UiRect VerticalDrawText::MeasureString(const DString& strText, const MeasureStringParam& measureParam)
{
    PerformanceStat statPerformance(_T("VerticalDrawText::MeasureString"));
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

    // 计算字体高度
    SkFontMetrics fontMetrics;
    const SkScalar fFontHeight = pSkFont->getMetrics(&fontMetrics);
    ASSERT(fFontHeight > 0);
    if (fFontHeight <= 0) {
        return UiRect();
    }

    //绘制文本始终使用UTF16编码
    const UTF16String textUTF16 = GetDrawStringUTF16(strText, bSingleLineMode);

    std::vector<TVerticalChar> charRects;
    if (!CalculateTextCharBounds(textUTF16, pSkFont, &skPaint, measureParam.bUseFontHeight, (float)fFontHeight, measureParam.bRotate90ForAscii, charRects)) {
        return UiRect();
    }
    ASSERT(charRects.size() == textUTF16.size());
    if (charRects.size() != textUTF16.size()) {
        return UiRect();
    }

    //默认字符的宽度
    float fDefaultCharWidth = CalculateDefaultCharWidth(pSkFont, &skPaint);
    SkRect skTextBounds = CalculateVerticalTextBounds(charRects, measureParam.rectSize, bSingleLineMode,
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

void VerticalDrawText::DrawString(const DString& strText, const DrawStringParam& drawParam)
{
    // 备注：纵向文本绘制不支持以下功能
    // 1. 文本风格：DrawStringFormat::TEXT_PATH_ELLIPSIS 不支持，按DrawStringFormat::TEXT_END_ELLIPSIS处理
    PerformanceStat statPerformance(_T("VerticalDrawText::DrawString"));
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

    //绘制区域
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

    // 列间距设置
    float fSpacingMul = drawParam.fSpacingMul;
    float fSpacingAdd = drawParam.fSpacingAdd;
    float fWordVerticalSpacing = drawParam.fWordSpacing;
    if (fSpacingMul <= 0.01f) {
        fSpacingMul = 1.0f;
    }
    if (fSpacingAdd < 0) {
        fSpacingAdd = 0;
    }
    if (fWordVerticalSpacing < 0) {
        fWordVerticalSpacing = 0;
    }

    //绘制文本始终使用UTF16编码
    const UTF16String textUTF16 = GetDrawStringUTF16(strText, bSingleLineMode);

    std::vector<TVerticalChar> charRects;
    if (!CalculateTextCharBounds(textUTF16, pSkFont, &skPaint, drawParam.bUseFontHeight, (float)fFontHeight, drawParam.bRotate90ForAscii, charRects)) {
        return;
    }
    ASSERT(charRects.size() == textUTF16.size());
    if (charRects.size() != textUTF16.size()) {
        return;
    }

    // 默认字符宽度
    float fDefaultCharWidth = CalculateDefaultCharWidth(pSkFont, &skPaint);

    std::vector<std::vector<int32_t>> columnRows;
    std::vector<float> columnWidths;
    std::vector<float> columnHeights;
    SkRect skTextBounds = CalculateVerticalTextBounds(charRects, drawParam.textRect.Height(), bSingleLineMode,
                                                      fSpacingMul, fSpacingAdd, fWordVerticalSpacing,
                                                      fDefaultCharWidth, fFontHeight,
                                                      &columnRows, &columnWidths, &columnHeights);

    ASSERT(columnRows.size() == columnWidths.size());
    if (columnRows.size() != columnWidths.size()) {
        return;
    }

    //记录每个字符的绘制位置，后续还需要处理对齐方式
    struct TDrawCharPos
    {
        DUTF16Char ch = 0;          //字符
        bool bRotate90 = false;     //是否需要旋转90度绘制
        bool bDrew = false;         //该字符是否完成绘制（用于绘制删除线和下划线）
        int32_t nColumnIndex = 0;   //列序号
        int32_t nRowIndex = 0;      //行序号
        SkScalar xPos = 0;          //绘制时的X坐标
        SkScalar yPos = 0;          //绘制时的Y坐标
        int32_t chHeight = 0;       //字符高度（用于绘制删除线和下划线）
    };
    std::vector<TDrawCharPos> drawCharPos;
    drawCharPos.reserve(charRects.size());

    //每一列有几个字符
    std::vector<int32_t> columnRowCount;
    columnRowCount.resize(columnRows.size(), 0);

    //开始绘制文字（按列，逐字绘制）
    SkScalar xPos = rcSkDest.right();
    const size_t nColumnCount = columnRows.size();
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        float fColumnWidth = columnWidths[nColumnIndex]; //列宽（已经预计算）
        SkScalar yPos = rcSkDest.top();
        const std::vector<int32_t>& row = columnRows[nColumnIndex];
        const size_t nRowCount = row.size();
        columnRowCount[nColumnIndex] = (int32_t)nRowCount; //记录每一列中有几个字符
        for (size_t nRowIndex = 0; nRowIndex < nRowCount; ++nRowIndex) {
            const size_t nCharIndex = (size_t)row[nRowIndex];
            ASSERT(nCharIndex < charRects.size());
            if (nCharIndex >= charRects.size()) {
                continue;
            }
            //绘制一个字
            TDrawCharPos& charPos = drawCharPos.emplace_back(); //新增一个字符
            const TVerticalChar& verticalChar = charRects[nCharIndex];
                
            //记录该字符的绘制位置，处理对齐方式以后再绘制
            charPos.ch = verticalChar.ch;
            charPos.nColumnIndex = (int32_t)nColumnIndex;
            charPos.nRowIndex = (int32_t)nRowIndex;
            charPos.bRotate90 = verticalChar.bRotate90;
            charPos.chHeight = (int32_t)verticalChar.size.height();

            if (charPos.bRotate90) { // 该字符需要旋转90度绘制                    
                charPos.xPos = xPos + fontMetrics.fAscent / fFontHeight * fColumnWidth;
                charPos.yPos = yPos;
            }
            else { //该字符正常绘制

                //该字实际占用的宽度（含溢出）
                SkScalar fCharWidth = verticalChar.size.width();
                SkScalar nWidthDiff = 0;
                if (verticalChar.bounds.fLeft < 0) {
                    //斜体字左侧溢出
                    nWidthDiff += -verticalChar.bounds.fLeft;
                }
                if (verticalChar.bounds.fRight > fCharWidth) {
                    //斜体字右侧溢出
                    nWidthDiff += verticalChar.bounds.fRight - fCharWidth;
                }
                fCharWidth += nWidthDiff;

                //字在同一列的对齐方式: 三选一(居中对齐最适合目前的逻辑，因为计算字符宽度的时候，考虑增加了溢出的fLeft和fRight值)
                //charPos.xPos = xPos - fCharWidth;                     //字在该列中靠右对齐
                //charPos.xPos = xPos - fColumnWidth;                   //靠左对齐
                charPos.xPos = xPos - (fColumnWidth + fCharWidth) / 2;  //居中对齐

                if (verticalChar.bounds.fLeft < 0) {
                    charPos.xPos += -verticalChar.bounds.fLeft; //字体的最左侧在基点左方，需要平移基点
                }
                if (drawParam.bUseFontHeight) {
                    //所有字体等高
                    charPos.yPos = yPos - fontMetrics.fAscent;
                }
                else {
                    //用实际高度
                    charPos.yPos = yPos - verticalChar.bounds.top();
                }
            }

            //下一个字符的纵坐标更新(带纵向字间距)
            yPos += (verticalChar.size.height() + fWordVerticalSpacing); //字体高度使用评估时确定的高度
        }

        //下一列的坐标，带行间距
        xPos -= (fColumnWidth * fSpacingMul + fSpacingAdd);
    }

    //水平方向对齐：所有绘制内容整体对齐
    int32_t textBoundWidth = SkScalarTruncToInt(skTextBounds.width() + 0.5f);
    if (textBoundWidth < drawParam.textRect.Width()) {
        int32_t nOffsetX = 0;
        if (drawParam.uFormat & DrawStringFormat::TEXT_HCENTER) {
            //横向对齐：居中对齐(左移)
            nOffsetX = -(drawParam.textRect.Width() - textBoundWidth) / 2;
        }
        else if (drawParam.uFormat & DrawStringFormat::TEXT_LEFT) {
            //横向对齐：左对齐(左移)
            nOffsetX = -(drawParam.textRect.Width() - textBoundWidth);
        }
        else {
            //横向对齐：右对齐(默认，不处理)
            nOffsetX = 0;
        }
        if (nOffsetX != 0) {
            for (TDrawCharPos& charPos : drawCharPos) {
                charPos.xPos += nOffsetX;
            }
        }
    }

    //垂直方向对齐：按每列的占用的实际区域对齐
    std::vector<float> columnOffsets;    
    if (drawParam.uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        columnOffsets.resize(columnHeights.size(), 0.0f);
        int32_t textRectHeight = drawParam.textRect.Height();
        for (size_t nColumnIndex = 0; nColumnIndex < columnHeights.size(); ++nColumnIndex) {
            if (columnHeights[nColumnIndex] < textRectHeight) {
                columnOffsets[nColumnIndex] = (textRectHeight - columnHeights[nColumnIndex]) / 2;
            }
        }
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：靠下对齐
        columnOffsets.resize(columnHeights.size(), 0.0f);
        int32_t textRectHeight = drawParam.textRect.Height();
        for (size_t nColumnIndex = 0; nColumnIndex < columnHeights.size(); ++nColumnIndex) {
            if (columnHeights[nColumnIndex] < textRectHeight) {
                columnOffsets[nColumnIndex] = (textRectHeight - columnHeights[nColumnIndex]);
            }
        }
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_VJUSTIFY) {
        //纵向对齐：两端对齐
        ASSERT(columnRowCount.size() == columnHeights.size());
        if (!columnRowCount.empty() && columnRowCount.size() == columnHeights.size()) {
            std::vector<std::vector<float>> columnRowOffsets; //每个字的偏移
            columnRowOffsets.resize(columnHeights.size());

            int32_t textRectHeight = drawParam.textRect.Height();
            for (size_t nColumnIndex = 0; nColumnIndex < columnHeights.size(); ++nColumnIndex) {
                const int32_t nRowCount = columnRowCount[nColumnIndex];
                // 计算每个字的偏移
                columnRowOffsets[nColumnIndex].resize(nRowCount, 0.0f);

                if (nRowCount < 2) {
                    //该列的字数少于2，不需要处理两端对齐
                    continue;
                }

                if (columnHeights[nColumnIndex] < textRectHeight) {
                    float fOffsetTotal = (textRectHeight - columnHeights[nColumnIndex]); //总的偏移
                    float fOffset = fOffsetTotal / (nRowCount - 1); //每个字符的偏移量

                    for (int32_t nRow = 0; nRow < nRowCount; ++nRow) {
                        columnRowOffsets[nColumnIndex][nRow] = fOffset * nRow;
                    }
                }
            }

            //为每个字符设置纵向偏移
            const int32_t nOffsetColumnCount = (int32_t)columnRowOffsets.size();
            for (TDrawCharPos& charPos : drawCharPos) {
                ASSERT(charPos.nColumnIndex < nOffsetColumnCount);
                if (charPos.nColumnIndex < nOffsetColumnCount) {
                    const std::vector<float>& rowOffsets = columnRowOffsets[charPos.nColumnIndex];
                    ASSERT(charPos.nRowIndex < (int32_t)rowOffsets.size());
                    if (charPos.nRowIndex < (int32_t)rowOffsets.size()) {
                        charPos.yPos += rowOffsets[charPos.nRowIndex];
                    }
                }
            }
        }
    }
    else {
        //纵向对齐：靠上对齐(默认，不处理)
    }
    if (columnOffsets.size() == columnHeights.size()) {
        const size_t nOffsetCount = columnOffsets.size();
        for (TDrawCharPos& charPos : drawCharPos) {
            if (charPos.nColumnIndex < (int32_t)nOffsetCount) {
                charPos.yPos += columnOffsets[charPos.nColumnIndex];
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
        if (bSingleLineMode && charPos.nColumnIndex != 0) {
            //单行模式，只绘制一列
            break;
        }

        if (bNeedEllipsis && (charIndex < (drawCharCount - 1))) {// 是否绘制"..."的业务逻辑
            //判断下一个字符是否越界（纵向）
            float fColumnWidth = 0;
            if (charPos.nColumnIndex < (int32_t)columnWidths.size()) {
                fColumnWidth = columnWidths[charPos.nColumnIndex]; //列宽
            }
            bool bNeedDrawEllipsis = false;
            const TDrawCharPos& nextCharPos = drawCharPos[charIndex + 1];
            if (nextCharPos.nColumnIndex == charPos.nColumnIndex) {
                SkScalar yPos = nextCharPos.yPos;
                if (nextCharPos.bRotate90) {
                    yPos += nextCharPos.chHeight;
                }                
                if ((yPos > (SkScalar)rcSkDest.fBottom) &&
                    (bSingleLineMode || (charPos.xPos - fColumnWidth) < (SkScalar)rcSkDest.fLeft)) {
                    //下个字符无法显示了
                    bNeedDrawEllipsis = true;
                }
            }
            else if (!bSingleLineMode && (charPos.xPos - fColumnWidth) < (SkScalar)rcSkDest.fLeft) {
                //左侧其他列的数据无法显示了
                bNeedDrawEllipsis = true;
            }
            if (bNeedDrawEllipsis) {
                //字符越界，绘制 "..."
                std::string ellipsis = "...";
                skCanvas->save();
                skCanvas->translate(charPos.xPos, charPos.bRotate90 ? charPos.yPos : charPos.yPos - charPos.chHeight / 2);
                skCanvas->rotate(90);
                skCanvas->drawSimpleText(ellipsis.data(), ellipsis.size(), SkTextEncoding::kUTF8,
                                         0, -fColumnWidth / 5,
                                         *pSkFont, skPaint);
                skCanvas->restore();

                //越界后不再绘制
                break;
            }
        }

        if (!skTextRect.contains(charPos.xPos, charPos.yPos)) {
            //当前字符不在文本显示区域，不绘制
            continue;
        }
        charPos.bDrew = true;
        if (charPos.bRotate90) {
            //该字符需要旋转90度绘制
            int saveCount = skCanvas->save();
            skCanvas->translate(charPos.xPos, charPos.yPos);
            skCanvas->rotate(90);
            skCanvas->drawSimpleText(&charPos.ch, sizeof(charPos.ch), SkTextEncoding::kUTF16,
                                     0, 0,
                                     *pSkFont, skPaint);
            skCanvas->restoreToCount(saveCount);
        }
        else {
            skCanvas->drawSimpleText(&charPos.ch, sizeof(charPos.ch), SkTextEncoding::kUTF16,
                                     charPos.xPos, charPos.yPos,
                                     *pSkFont, skPaint);
        }
    }

    //绘制下划线/删除线
    if ((drawParam.pFont->IsUnderline() || drawParam.pFont->IsStrikeOut()) && !columnWidths.empty()) {
        struct TextColumnInfo
        {
            UiPoint ptStart;
            UiPoint ptEnd;
            bool bStartRotate90 = false;
            bool bEndRotate90 = false;
            bool bStartFlag = false;
            bool bEndFlag = false;
            int32_t chStartHeight = 0;
            int32_t chEndHeight = 0;
        };
        std::vector<TextColumnInfo> textColumnInfo;
        textColumnInfo.resize(columnWidths.size());
        for (const TDrawCharPos& charPos : drawCharPos) {
            if (!charPos.bDrew) {
                continue;
            }
            ASSERT(charPos.nColumnIndex < (int32_t)textColumnInfo.size());
            if (charPos.nColumnIndex >= (int32_t)textColumnInfo.size()) {
                continue;
            }
            TextColumnInfo& textColumn = textColumnInfo[charPos.nColumnIndex];
            if (!textColumn.bStartFlag) {
                textColumn.bStartRotate90 = charPos.bRotate90;
                textColumn.bStartFlag = true;
                textColumn.ptStart.x = (int32_t)charPos.xPos;
                textColumn.ptStart.y = (int32_t)(charPos.yPos + 0.5f);
                textColumn.chStartHeight = charPos.chHeight;
            }
            else {
                textColumn.bEndRotate90 = charPos.bRotate90;
                textColumn.bEndFlag = true;
                textColumn.ptEnd.x = (int32_t)charPos.xPos;
                textColumn.ptEnd.y = (int32_t)charPos.yPos;
                textColumn.chEndHeight = charPos.chHeight;
            }
        }

        // 绘制线条
        for (size_t nColumnIndex = 0; nColumnIndex < textColumnInfo.size(); ++nColumnIndex) {
            const TextColumnInfo& textColumn = textColumnInfo[nColumnIndex];
            if (!textColumn.bStartFlag && !textColumn.bEndFlag) {
                continue;
            }

            // Default fraction of the text size to use for a strike-through or underline.
            static constexpr SkScalar kLineThicknessFactor = (SK_Scalar1 / 18);

            SkScalar top = (SkScalar)textColumn.ptStart.y;
            SkScalar x_scalar = SkIntToScalar(textColumn.ptStart.x);
            if (textColumn.bEndFlag) {
                x_scalar = (SkIntToScalar(textColumn.ptStart.x) + SkIntToScalar(textColumn.ptEnd.x)) / 2;
            }
            if (!textColumn.bStartRotate90) {
                top -= textColumn.chStartHeight;
            }
            else {
                top += fFontHeight * 1 / 9;
            }
            if (top <= rcSkDest.fTop) {
                top = rcSkDest.fTop + fFontHeight * 1 / 9;
            }

            SkScalar height = (SkScalar)textColumn.chStartHeight;
            if (textColumn.bEndFlag) {
                height = (SkScalar)(textColumn.ptEnd.y - textColumn.ptStart.y + textColumn.chEndHeight * 4 / 5);
                if (textColumn.bStartRotate90) {
                    height -= fFontHeight * 4 / 5;
                }
            }

            if (drawParam.pFont->IsStrikeOut()) {
                //删除线
                const SkScalar width = pSkFont->getSize() * kLineThicknessFactor;

                const float fColumnWidth = columnWidths[nColumnIndex];
                const SkScalar xOffset = fColumnWidth * 2 / 5;

                const SkRect r = SkRect::MakeLTRB(x_scalar + xOffset, top, x_scalar + xOffset + width, top + height);
                skCanvas->drawRect(r, skPaint);

            }
            if (drawParam.pFont->IsUnderline()) {
                //下划线
                const SkScalar width = pSkFont->getSize() * kLineThicknessFactor * 1.5f;

                const SkRect r = SkRect::MakeLTRB(x_scalar, top, x_scalar + width, top + height);
                skCanvas->drawRect(r, skPaint);
            }            
        }
    }
}

} // namespace ui
