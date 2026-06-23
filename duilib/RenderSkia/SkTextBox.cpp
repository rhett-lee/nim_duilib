/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBox.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkSpan.h"
#include "SkiaHeaderEnd.h"

#include "duilib/RenderSkia/DrawSkiaText.h"

//该文件原始文件的出处：skia/chrome_67/src/utils/SkTextBox.cpp
//基于原始文件，有修改，以兼容最新版本的skia代码（2023-06-25）
//原始文件从chrome 68以后就删除了。

namespace ui
{

SkTextBox::SkTextBox()
{
    m_clipBox = true;
    m_box.setEmpty();
    m_spacingMul = SK_Scalar1;
    m_spacingAdd = 0;
    m_lineMode = kWordBreak_Mode;
    m_spacingAlign = kStart_SpacingAlign;
    m_textAlign = kLeft_Align;

    m_endEllipsis = false;
    m_pathEllipsis = false;
    m_underline = false;
    m_strikeOut = false;

    m_pPaint = nullptr;
    m_pFont = nullptr;
    m_fallbackFontCreator = nullptr;
}

void SkTextBox::SetLineMode(TextBoxLineMode lineMode)
{
    ASSERT((unsigned)lineMode < kModeCount);
    m_lineMode = lineMode;
}

void SkTextBox::SetSpacingAlign(SpacingAlign align)
{
    ASSERT((unsigned)align < kSpacingAlignCount);
    m_spacingAlign = SkToU8(align);
}

void SkTextBox::SetTextAlign(TextAlign align)
{
    ASSERT((unsigned)align < kAlignCount);
    m_textAlign = SkToU8(align);
}

void SkTextBox::SetEndEllipsis(bool bEndEllipsis)
{
    m_endEllipsis = bEndEllipsis;
}

void SkTextBox::SetPathEllipsis(bool bPathEllipsis)
{
    m_pathEllipsis = bPathEllipsis;
}

void SkTextBox::SetUnderline(bool bUnderline)
{
    m_underline = bUnderline;
}

void SkTextBox::SetStrikeOut(bool bStrikeOut)
{
    m_strikeOut = bStrikeOut;
}

void SkTextBox::GetBox(SkRect* pBox) const
{
    if (pBox) {
        *pBox = m_box;
    }
}

void SkTextBox::SetBox(const SkRect& box)
{
    m_box = box;
}

void SkTextBox::SetBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    m_box.setLTRB(left, top, right, bottom);
}

void SkTextBox::SetClipBox(bool bClipBox)
{
    m_clipBox = bClipBox;
}

void SkTextBox::GetSpacing(SkScalar* pMul, SkScalar* pAdd) const
{
    if (pMul) {
        *pMul = m_spacingMul;
    }
    if (pAdd) {
        *pAdd = m_spacingAdd;
    }
}

void SkTextBox::SetSpacing(SkScalar mul, SkScalar add)
{
    m_spacingMul = mul;
    m_spacingAdd = add;
}

SkScalar SkTextBox::Visit(Visitor& visitor) const
{
    if (m_textData.IsEmpty() || m_pFont == nullptr || m_pPaint == nullptr) {
        ASSERT(!m_textData.IsEmpty());
        ASSERT(m_pFont != nullptr);
        ASSERT(m_pPaint != nullptr);
        return 0;
    }
    //该值不强制检查，允许为nullptr
    //if (m_fallbackFontCreator == nullptr) {
    //    ASSERT(m_fallbackFontCreator != nullptr);
    //    return 0;
    //}

    MeasureTextTempData measureTempData;  //内部临时变量，为提升执行速度，在外部声明变量

    const char* pText = static_cast<const char*>(m_textData.GetText());
    const size_t nByteLen = m_textData.GetByteLength();
    SkTextEncoding textEncoding = m_textData.GetTextEncoding();
    const SkFont& font = *m_pFont;
    const SkPaint& paint = *m_pPaint;
    TextBoxLineMode lineMode = m_lineMode;
    SkRect boxRect = m_box;
    SkScalar spacingMul = m_spacingMul;
    SkScalar spacingAdd = m_spacingAdd;
    uint8_t spacingAlign = m_spacingAlign;
    uint8_t textAlign = m_textAlign;
    FallbackFontCreator fallbackFontCreator = m_fallbackFontCreator;

    SkScalar marginWidth = boxRect.width();
    if (marginWidth <= 0 || nByteLen == 0) {
        return boxRect.fTop;
    }

    const char* textStop = pText + nByteLen;

    SkScalar fontHeight = 0;
    SkFontMetrics metrics;
    fontHeight = font.getMetrics(&metrics);
    SkScalar scaledSpacing = fontHeight * spacingMul + spacingAdd;
    SkScalar height = boxRect.height();

    SkScalar x = boxRect.fLeft;
    SkScalar y = boxRect.fTop;

    //计算第一行文字的Y坐标位置（处理纵向对齐）
    {
        SkScalar textHeight = fontHeight;

        if (spacingAlign != kStart_SpacingAlign) {
            int32_t nCount = DrawSkiaText::CountLines(pText, textStop - pText, textEncoding,
                                                     font, fallbackFontCreator, paint, marginWidth, lineMode);
            if (nCount > 0) {
                textHeight += scaledSpacing * (nCount - 1);
            }
        }

        switch (spacingAlign) {
        case kStart_SpacingAlign:
            y = 0;
            break;
        case kCenter_SpacingAlign:
            y = SkScalarHalf(height - textHeight);
            if (y < 0) {
                //如果居中对齐绘制区域不足，那么按照可显示的文字进行居中对齐
                y = SkScalarHalf(height - fontHeight);
            }
            break;
        case kEnd_SpacingAlign:
            y = height - textHeight;
            break;
        default:
            ASSERT(false);
            y = 0;
            break;
        }
        y += boxRect.fTop - metrics.fAscent;
    }

    //遍历每一行文字并调用visitor回调
    for (;;) {
        size_t trailing = 0;
        const size_t nLineByteLen = DrawSkiaText::Linebreak(pText, textStop, textEncoding,
                                                            font, fallbackFontCreator, paint,
                                                            marginWidth, lineMode,
                                                            measureTempData, &trailing);
        if (y + metrics.fDescent + metrics.fLeading > 0) {
            //计算当前行的水平对齐位置
            if (textAlign == kLeft_Align) {
                //横向：左对齐
                x = boxRect.fLeft;
            }
            else {
                //右对齐或者中对齐
                SkScalar textWidth = DrawSkiaText::MeasureText(font, pText,
                                                               nLineByteLen - trailing,
                                                               textEncoding,
                                                               nullptr, &paint, fallbackFontCreator, measureTempData);
                if (textAlign == kCenter_Align) {
                    //横向：中对齐
                    x = boxRect.fLeft + SkScalarHalf(marginWidth - textWidth);
                    if (x < boxRect.fLeft) {
                        x = boxRect.fLeft;
                    }
                }
                else {
                    //横向，右对齐
                    x = boxRect.fRight - textWidth;
                    if (x < boxRect.fLeft) {
                        x = boxRect.fLeft;
                    }
                }
            }
            //后续还有没有待绘制文本
            bool bHasMoreText = (pText + nLineByteLen) < textStop;
            //当前是否为最后一行
            bool bIsLastLine = (y + scaledSpacing + metrics.fAscent / 2) >= boxRect.fBottom;

            SkiaTextData lineTextData(pText, nLineByteLen - trailing, textEncoding);
            visitor(lineTextData, x, y, font, paint, bHasMoreText, bIsLastLine, fallbackFontCreator, measureTempData);
        }
        pText += nLineByteLen;
        if (pText >= textStop) {
            break;
        }
        y += scaledSpacing;
        if ((y + metrics.fAscent / 2) >= boxRect.fBottom) {
            break;
        }
    }
    return y + metrics.fDescent + metrics.fLeading;
}

/** 计算给定编码格式下一个字符的字节数
 * @param [in] textEncoding 文本编码格式
 * @return 字符字节数（UTF8:1, UTF16:2, UTF32:4）
 */
static inline size_t GetCharBytes(SkTextEncoding textEncoding)
{
    switch (textEncoding) {
    case SkTextEncoding::kUTF8:
        return 1;
    case SkTextEncoding::kUTF16:
        return 2;
    case SkTextEncoding::kUTF32:
        return 4;
    default:
        ASSERT(false);
        return 1;
    }
}

template<typename T>
static bool EllipsisTextUTF(const char pText[], size_t nByteLen, SkTextEncoding textEncoding,
                            bool bEndEllipsis, bool bPathEllipsis,
                            const SkFont& font, FallbackFontCreator fallbackFontCreator,
                            const SkPaint& paint,
                            SkScalar destWidth,
                            const char** pTextOut,
                            size_t& nByteLenOut,
                            T& stringUtf,
                            const T& ellipsisStr,
                            const T& pathSep,
                            MeasureTextTempData& measureTempData)
{
    if ((pText == nullptr) || (nByteLen == 0)) {
        return false;
    }
    if (destWidth <= 0) {
        return false;
    }

    size_t charBytes = GetCharBytes(textEncoding);
    SkScalar ellipsisWidth = DrawSkiaText::MeasureText(font, ellipsisStr.c_str(), ellipsisStr.size() * charBytes, textEncoding, nullptr, &paint, fallbackFontCreator, measureTempData);
    if (ellipsisWidth >= destWidth) {
        return false;
    }

    T pathEnd;
    T string;
    string.assign((const typename T::value_type*)pText, nByteLen / charBytes);

    SkScalar pathEndWidth = 0;
    if (bPathEllipsis) {
        //查找路径分隔符（支持反斜杠和正斜杠）
        size_t pos = string.find_last_of(pathSep);
        if (pos != T::npos && pos > 0) {
            pathEnd = string.substr(pos);
            pathEndWidth = DrawSkiaText::MeasureText(font, pathEnd.c_str(), pathEnd.size() * charBytes, textEncoding, nullptr, &paint, fallbackFontCreator, measureTempData);
            if ((pathEndWidth + ellipsisWidth) > destWidth) {
                //宽度不足以显示路径的最后一段文字
                pathEnd.clear();
                pathEndWidth = 0;
            }
            else {
                string = string.substr(0, pos);
            }
        }
    }

    if (!bEndEllipsis && pathEnd.empty()) {
        return false;
    }

    SkScalar leftWidth = destWidth - ellipsisWidth - pathEndWidth;
    if (leftWidth <= 1) {
        return false;
    }

    size_t textByteLen = DrawSkiaText::BreakText(string.c_str(), string.size() * charBytes, textEncoding,
                                                 font, fallbackFontCreator, paint, leftWidth, nullptr, measureTempData);
    textByteLen /= charBytes;
    if ((textByteLen > 0) && (textByteLen <= string.size())) {
        string.erase(textByteLen);
        string += ellipsisStr;
        string += pathEnd;
        stringUtf = string;
        if (pTextOut) {
            *pTextOut = reinterpret_cast<const char*>(stringUtf.c_str());
        }
        nByteLenOut = stringUtf.size() * charBytes;
        return true;
    }
    return false;
}

static bool EllipsisText(const char pText[], size_t nByteLen, SkTextEncoding textEncoding,
                         std::string& stringUtf8,
                         std::u16string& stringUtf16,
                         std::u32string& stringUtf32,
                         bool bEndEllipsis, bool bPathEllipsis,
                         const SkFont& font, FallbackFontCreator fallbackFontCreator, const SkPaint& paint,
                         SkScalar destWidth,
                         const char** pTextOut, size_t& nByteLenOut, MeasureTextTempData& measureTempData)
{
    //路径分隔符同时支持反斜杠和正斜杠，以适应不同操作系统的路径格式
    switch (textEncoding) {
    case SkTextEncoding::kUTF8:
        return EllipsisTextUTF<std::string>(pText, nByteLen, SkTextEncoding::kUTF8,
                                            bEndEllipsis, bPathEllipsis,
                                            font, fallbackFontCreator, paint,
                                            destWidth,
                                            pTextOut, nByteLenOut,
                                            stringUtf8,
                                            "...",
                                            "\\/",
                                            measureTempData);
    case SkTextEncoding::kUTF16:
        return EllipsisTextUTF<std::u16string>(pText, nByteLen, SkTextEncoding::kUTF16,
                                            bEndEllipsis, bPathEllipsis,
                                            font, fallbackFontCreator, paint,
                                            destWidth,
                                            pTextOut, nByteLenOut,
                                            stringUtf16,
                                            u"...",
                                            u"\\/",
                                            measureTempData);
    case SkTextEncoding::kUTF32:
        return EllipsisTextUTF<std::u32string>(pText, nByteLen, SkTextEncoding::kUTF32,
                                            bEndEllipsis, bPathEllipsis,
                                            font, fallbackFontCreator, paint,
                                            destWidth,
                                            pTextOut, nByteLenOut,
                                            stringUtf32,
                                            U"...",
                                            U"\\/",
                                            measureTempData);
    default:
        ASSERT(false);
        return false;
    }
}

/** 绘制删除线或下划线
 * @param [in] pSkCanvas 绘制画布
 * @param [in] x 起始X坐标
 * @param [in] y 基线Y坐标
 * @param [in] width 线条宽度
 * @param [in] textSize 字体大小
 * @param [in] lineOffset 线条相对于基线的偏移因子（正值表示上方，负值表示下方）
 * @param [in] thicknessFactor 线条厚度因子（相对于字体大小）
 * @param [in] paint 绘制属性
 */
static void DrawTextLine(SkCanvas* pSkCanvas,
                         SkScalar x, SkScalar y, SkScalar width, SkScalar textSize,
                         SkScalar lineOffset, SkScalar thicknessFactor,
                         const SkPaint& paint)
{
    SkScalar thickness = textSize * thicknessFactor;
    SkScalar top = y + textSize * lineOffset - thickness / 2;
    SkRect r = SkRect::MakeLTRB(x, top, x + width, top + thickness);
    pSkCanvas->drawRect(r, paint);
}

/** 绘制带删除线/下划线的文本
 * @param [in] pTextBox 文本框对象
 * @param [in] pSkCanvas 绘制画布
 * @param [in] textData 文本数据
 * @param [in] x 绘制X坐标
 * @param [in] y 绘制Y坐标（基线位置）
 * @param [in] font 字体
 * @param [in] paint 绘制属性
 * @param [in] bHasMoreText 是否还有更多文本
 * @param [in] bIsLastLine 是否是最后一行
 * @param [in] fallbackFontCreator 回退字体管理器
 */
static void TextBox_DrawText(SkTextBox* pTextBox,
                             SkCanvas* pSkCanvas,
                             const SkiaTextData& textData,
                             SkScalar x, SkScalar y,
                             const SkFont& font, const SkPaint& paint,
                             bool bHasMoreText, bool bIsLastLine,
                             FallbackFontCreator fallbackFontCreator,
                             MeasureTextTempData& measureTempData)
{
    ASSERT(pTextBox != nullptr);
    ASSERT(pSkCanvas != nullptr);
    if (pTextBox == nullptr || pSkCanvas == nullptr) {
        return;
    }
    //绘制一行文字
    SkRect boxRect;
    pTextBox->GetBox(&boxRect);

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = pTextBox->GetEndEllipsis();
    //绘制区域不足时，自动在绘制省略号代替文本
    //如果字符串包含反斜杠 (\\) 字符，在最后一个反斜杠之后保留尽可能多的文本。
    bool bPathEllipsis = pTextBox->GetPathEllipsis();
    //字体属性：下划线
    bool bUnderline = pTextBox->GetUnderline();
    //字体属性：删除线
    bool bStrikeOut = pTextBox->GetStrikeOut();
    //单行模式
    bool isSingleLine = pTextBox->GetLineMode() == TextBoxLineMode::kOneLine_Mode;

    if (!bEndEllipsis && !bPathEllipsis && !bUnderline && !bStrikeOut) {
        DrawSkiaText::DrawSimpleText(pSkCanvas, textData, x, y, font, paint, fallbackFontCreator);
        return;
    }

    bool needEllipsis = false;
    if (bEndEllipsis || bPathEllipsis) {
        if (isSingleLine) {
            //单行模式
            SkScalar textWidth = DrawSkiaText::MeasureText(font, textData, nullptr, &paint, fallbackFontCreator, measureTempData);
            if ((x + textWidth) > boxRect.fRight) {
                //文字超出边界，需要增加"..."替代无法显示的文字
                needEllipsis = true;
            }
        }
        else {
            //多行模式
            if (bEndEllipsis && bIsLastLine && bHasMoreText) {
                //文字超出边界，需要增加"..."替代无法显示的文字
                needEllipsis = true;
            }
        }
    }

    if (!needEllipsis && !bUnderline && !bStrikeOut) {
        DrawSkiaText::DrawSimpleText(pSkCanvas, textData, x, y, font, paint, fallbackFontCreator);
        return;
    }

    std::string stringUtf8;
    std::u16string stringUtf16;
    std::u32string stringUtf32;
    SkiaTextData drawTextData = textData;

    if (needEllipsis) {
        const char* pTextOut = nullptr;
        size_t nLengthOut = 0;
        if (EllipsisText(static_cast<const char*>(textData.GetText()), textData.GetByteLength(), textData.GetTextEncoding(),
                         stringUtf8, stringUtf16, stringUtf32,
                         bEndEllipsis, bPathEllipsis,
                         font, fallbackFontCreator, paint,
                         boxRect.fRight - x,
                         &pTextOut, nLengthOut, measureTempData)) {
            drawTextData.SetText(pTextOut, nLengthOut, textData.GetTextEncoding());
        }
    }
    //绘制文本
    DrawSkiaText::DrawSimpleText(pSkCanvas, drawTextData, x, y, font, paint, fallbackFontCreator);

    if (bUnderline || bStrikeOut) {
        SkScalar width = DrawSkiaText::MeasureText(font, drawTextData, nullptr, &paint, fallbackFontCreator, measureTempData);
        SkScalar textSize = font.getSize();

        if (bStrikeOut) {
            //绘制删除线
            DrawTextLine(pSkCanvas, x, y, width, textSize,
                         -SkTextBox::kStrikeThroughOffset, SkTextBox::kLineThicknessFactor, paint);
        }
        if (bUnderline) {
            //绘制下划线
            DrawTextLine(pSkCanvas, x, y, width, textSize,
                         SkTextBox::kUnderlineOffset,
                         SkTextBox::kLineThicknessFactor * SkTextBox::kUnderlineThicknessFactor, paint);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

class CanvasVisitor : public SkTextBox::Visitor {
    SkCanvas* m_pCanvas;
    SkTextBox* m_pTextBox;
public:
    CanvasVisitor(SkCanvas* pCanvas, SkTextBox* pTextBox) :
        m_pCanvas(pCanvas),
        m_pTextBox(pTextBox) {
    }

    void operator()(const SkiaTextData& textData,
                    SkScalar x, SkScalar y,
                    const SkFont& font, const SkPaint& paint,
                    bool bHasMoreText, bool bIsLastLine,
                    FallbackFontCreator fallbackFontCreator,
                    MeasureTextTempData& measureTempData) override
    {
        //调用单独封装的函数绘制文字，便于扩展
        TextBox_DrawText(m_pTextBox,
                         m_pCanvas,
                         textData,
                         x, y,
                         font, paint,
                         bHasMoreText, bIsLastLine,
                         fallbackFontCreator, measureTempData);
    }
};

void SkTextBox::SetText(const SkiaTextData& textData,
                        const SkFont& font, const SkPaint& paint,
                        FallbackFontCreator fallbackFontCreator)
{
    m_textData = textData;
    m_pPaint = &paint;
    m_pFont = &font;
    m_fallbackFontCreator = fallbackFontCreator;
}

void SkTextBox::Draw(SkCanvas* pSkCanvas,
                     const SkiaTextData& textData,
                     const SkFont& font, const SkPaint& paint,
                     FallbackFontCreator fallbackFontCreator)
{
    SetText(textData, font, paint, fallbackFontCreator);
    Draw(pSkCanvas);
}

void SkTextBox::Draw(SkCanvas* pSkCanvas)
{
    ASSERT(pSkCanvas != nullptr);
    if (pSkCanvas == nullptr) {
        return;
    }

    if (m_textData.IsEmpty()) {
        return;
    }

    if (m_pFont == nullptr || m_pPaint == nullptr) {
        ASSERT(m_pFont != nullptr);
        ASSERT(m_pPaint != nullptr);
        return;
    }

    int saveCount = 0;
    if (m_clipBox) {
        saveCount = pSkCanvas->save();
        pSkCanvas->clipRect(m_box, true);
    }

    CanvasVisitor sink(pSkCanvas, this);
    this->Visit(sink);

    if (m_clipBox) {
        pSkCanvas->restoreToCount(saveCount);
    }
}

int32_t SkTextBox::CountLines() const
{
    if (m_textData.IsEmpty() || m_pFont == nullptr || m_pPaint == nullptr) {
        return 0;
    }
    return DrawSkiaText::CountLines(m_textData,
                                    *m_pFont, m_fallbackFontCreator, *m_pPaint, m_box.width(),
                                    m_lineMode);
}

SkScalar SkTextBox::GetTextHeight() const
{
    if (m_pFont == nullptr) {
        return 0;
    }
    SkScalar spacing = m_pFont->getSize() * m_spacingMul + m_spacingAdd;
    int32_t nLines = this->CountLines();
    return nLines * spacing;
}

}//namespace ui
