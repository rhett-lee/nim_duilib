/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTextBox.h"
#include "SkUtils.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTextBlob.h"

//该文件原始文件的出处：skia/chrome_67/src/utils/SkTextBox.cpp
//基于原始文件，有修改，以兼容最新版本的skia代码（2023-06-25）
//原始文件从chrome 68以后就删除了。

namespace ui
{

/** 判断是否为空格、不可见字符
* @param [in] c 是 Unicode 字符
* 返回true表示可以在当前字符进行换行
* 返回false表示不可以在当前字符换行
*/
static inline bool SkUTF_IsWhiteSpace(int c)
{
    //ASCII值（c）小于32的时候（含控制字符等不可见字符、空格），返回true；否则返回false
    return !((c - 1) >> 5);
}

/** 判断在当前字符处是否可以分行
* @param [in] c 是 Unicode 字符
*/
static inline bool SkUTF_IsLineBreaker(int c)
{
    //在数字和字母上不分行（返回false），尽量不换行，确保数字和英文单词的完整性
    //非字母数字（返回true）均可以分行
    if ((c >= -1) && (c <= 255)) {
        //ASCII值（c）
        if (::isalnum(c)) {
            return false;
        }
    }
    return true;
}

static SkUnichar SkUTF_NextUnichar(const void** ptr, SkTextEncoding textEncoding)
{
    if (textEncoding == SkTextEncoding::kUTF16) {
        return SkUTF16_NextUnichar((const uint16_t**)ptr);
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        const uint32_t** srcPtr = (const uint32_t**)ptr;
        const uint32_t* src = *srcPtr;
        SkUnichar c = *src;
        *srcPtr = ++src;
        return c;
    }
    else {
        return SkUTF8_NextUnichar((const char**)ptr);
    }
}

static SkUnichar SkUTF_ToUnichar(const void* utf, SkTextEncoding textEncoding)
{
    if (textEncoding == SkTextEncoding::kUTF16) {
        const uint16_t* srcPtr = (const uint16_t*)utf;
        return SkUTF16_NextUnichar(&srcPtr);
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        const uint32_t* srcPtr = (const uint32_t*)utf;
        SkUnichar c = *srcPtr;
        return c;
    }
    else {
        return SkUTF8_ToUnichar((const char*)utf);
    }
}

static int SkUTF_CountUTFBytes(const void* utf, SkTextEncoding textEncoding)
{
    if (textEncoding == SkTextEncoding::kUTF16) {
        // 2 or 4
        int numChars = 1;
        const uint16_t* src = static_cast<const uint16_t*>(utf);
        unsigned c = *src++;
        if (SkUTF16_IsHighSurrogate(c)) {
            c = *src++;
            if (!SkUTF16_IsLowSurrogate(c)) {
                SkASSERT(false);
            }
            numChars = 2;
        }
        return numChars * 2;
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        //only 4
        return 4;
    }
    else {
        //1 or 2 or 3 or 4
        return SkUTF8_CountUTF8Bytes((const char*)utf);
    }
}

static size_t linebreak(const char text[], const char stop[], SkTextEncoding textEncoding,
                        const SkFont& font, const SkPaint& paint, 
                        SkScalar margin, SkTextBox::LineMode lineMode,
                        size_t* trailing = nullptr)
{
    size_t lengthBreak = stop - text;//单行模式
    if (lineMode != SkTextBox::kOneLine_Mode) {
        //多行模式
        lengthBreak = SkTextBox::breakText(text, stop - text, textEncoding, font, paint, margin);
    }
    
    //Check for white space or line breakers before the lengthBreak
    const char* start = text;
    const char* word_start = text;

    //标记是否可以分行
    bool prevIsLineBreaker = true;

    //如果设置trailing的值，这部分字符串在绘制的时候，会被忽略，不绘制
    if (trailing) {
        *trailing = 0;
    }

    while (text < stop) {
        const char* prevText = text;
        SkUnichar uni = SkUTF_NextUnichar((const void**)&text, textEncoding);

        //当前字符是否为空格（或非可见字符）
        bool currIsWhiteSpace = SkUTF_IsWhiteSpace(uni);

        //当前字符是否可以分行，分行条件：当前字符是空格（或非可见字符），或者不是字母/数字
        //Word分行逻辑：按Word分行，保证一个英文单词或者一个完整的数字不被分行显示
        bool currIsLineBreaker = SkUTF_IsLineBreaker(uni);
        if (lineMode == SkTextBox::kCharBreak_Mode) {
            //按字符分行，每个字符都可以分行
            currIsLineBreaker = true;
        }
        if(prevIsLineBreaker){
            //如果前面字符可以分行, 就执行前面一个字符
            word_start = prevText;
        }
        prevIsLineBreaker = currIsLineBreaker;

        if (text > start + lengthBreak) {
            if (currIsWhiteSpace) {
                // eat the rest of the whitespace
                while (text < stop && SkUTF_IsWhiteSpace(SkUTF_ToUnichar(text, textEncoding))) {
                    text += SkUTF_CountUTFBytes(text, textEncoding);
                }
                if (trailing) {
                    *trailing = text - prevText;
                }
            } else {
                // backup until a whitespace (or 1 char)
                if (word_start == start) {
                    if (prevText > start) {
                        text = prevText;
                    }
                } else {
                    text = word_start;
                }
            }
            break;
        }

        if ('\n' == uni) {
            size_t ret = text - start;
            size_t lineBreakSize = 1;
            if (text < stop) {
                uni = SkUTF_NextUnichar((const void**)&text, textEncoding);
                if ('\r' == uni) {
                    ret = text - start;
                    ++lineBreakSize;
                }
            }
            if (trailing) {
                *trailing = lineBreakSize;
            }
            return ret;
        }

        if ('\r' == uni) {
            size_t ret = text - start;
            size_t lineBreakSize = 1;
            if (text < stop) {
                uni = SkUTF_NextUnichar((const void**)&text, textEncoding);
                if ('\n' == uni) {
                    ret = text - start;
                    ++lineBreakSize;
                }
            }
            if (trailing) {
                *trailing = lineBreakSize;
            }
            return ret;
        }
    }

    return text - start;
}

int SkTextLineBreaker::CountLines(const char text[], size_t len, SkTextEncoding textEncoding, 
                                  const SkFont& font, const SkPaint& paint, 
                                  SkScalar width, SkTextBox::LineMode lineMode)
{
    const char* stop = text + len;
    int         count = 0;

    if (width > 0)
    {
        do {
            count += 1;
            text += linebreak(text, stop, textEncoding, font, paint, width, lineMode);
        } while (text < stop);
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////////

SkTextBox::SkTextBox()
{
    fClipBox = true;
    fBox.setEmpty();
    fSpacingMul = SK_Scalar1;
    fSpacingAdd = 0;
    fLineMode = kWordBreak_Mode;
    fSpacingAlign = kStart_SpacingAlign;
    fTextAlign = kLeft_Align;

    fEndEllipsis = false;
    fPathEllipsis = false;
    fUnderline = false;
    fStrikeOut = false;

    fText = nullptr;
    fLen = 0;    
    fTextEncoding = SkTextEncoding::kUTF8;

    fPaint = nullptr;
    fFont = nullptr;
}

void SkTextBox::setLineMode(LineMode mode)
{
    SkASSERT((unsigned)mode < kModeCount);
    fLineMode = mode;
}

void SkTextBox::setSpacingAlign(SpacingAlign align)
{
    SkASSERT((unsigned)align < kSpacingAlignCount);
    fSpacingAlign = SkToU8(align);
}

void SkTextBox::setTextAlign(TextAlign align)
{
    SkASSERT((unsigned)align < kAlignCount);
    fTextAlign = SkToU8(align);
}

void SkTextBox::setEndEllipsis(bool bEndEllipsis)
{
    fEndEllipsis = bEndEllipsis;
}

void SkTextBox::setPathEllipsis(bool bPathEllipsis)
{
    fPathEllipsis = bPathEllipsis;
}

void SkTextBox::setUnderline(bool bUnderline)
{
    fUnderline = bUnderline;
}

void SkTextBox::setStrikeOut(bool bStrikeOut)
{
    fStrikeOut = bStrikeOut;
}

void SkTextBox::getBox(SkRect* box) const
{
    if (box) {
        *box = fBox;
    }
}

void SkTextBox::setBox(const SkRect& box)
{
    fBox = box;
}

void SkTextBox::setBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    fBox.setLTRB(left, top, right, bottom);
}

void SkTextBox::setClipBox(bool bClipBox)
{
    fClipBox = bClipBox;
}

void SkTextBox::getSpacing(SkScalar* mul, SkScalar* add) const
{
    if (mul) {
        *mul = fSpacingMul;
    }
    if (add) {
        *add = fSpacingAdd;
    }
}

void SkTextBox::setSpacing(SkScalar mul, SkScalar add)
{
    fSpacingMul = mul;
    fSpacingAdd = add;
}

/////////////////////////////////////////////////////////////////////////////////////////////

SkScalar SkTextBox::visit(Visitor& visitor) const {
    const char* text = fText;
    size_t len = fLen;
    SkTextEncoding textEncoding = fTextEncoding;
    const SkFont& font = *fFont;
    const SkPaint& paint = *fPaint;
    LineMode lineMode = fLineMode;
    SkRect boxRect = fBox;
    SkScalar spacingMul = fSpacingMul;
    SkScalar spacingAdd = fSpacingAdd;
    uint8_t spacingAlign = fSpacingAlign;
    uint8_t textAlign = fTextAlign;

    SkScalar marginWidth = boxRect.width();

    if (marginWidth <= 0 || len == 0) {
        return boxRect.top();
    }

    const char* textStop = text + len;

    SkScalar scaledSpacing = 0;
    SkScalar height = 0;
    SkScalar fontHeight = 0;
    SkFontMetrics metrics;

    SkScalar x = boxRect.fLeft;
    SkScalar y = boxRect.fTop;
    fontHeight = font.getMetrics(&metrics);
    scaledSpacing = fontHeight * spacingMul + spacingAdd;
    height = boxRect.height();

    //  compute Y position for first line
    {
        SkScalar textHeight = fontHeight;

        if (spacingAlign != kStart_SpacingAlign) {
            int count = SkTextLineBreaker::CountLines(text, textStop - text, textEncoding,
                                                      font, paint, marginWidth, lineMode);
            SkASSERT(count > 0);
            textHeight += scaledSpacing * (count - 1);
        }

        switch (spacingAlign) {
        case kStart_SpacingAlign:
            y = 0;
            break;
        case kCenter_SpacingAlign:
            y = SkScalarHalf(height - textHeight);
            if (y < 0) {
                //如果居中对齐绘制区域不足，那么按照可显示的文字进行居中对齐
                y = static_cast<SkScalar>((((int)height % (int)fontHeight) / 2)) ;
            }
            break;
        default:
            SkASSERT(spacingAlign == kEnd_SpacingAlign);
            y = height - textHeight;
            break;
        }
        y += boxRect.fTop - metrics.fAscent;
    }

    for (;;) {
        size_t trailing = 0;
        len = linebreak(text, textStop, textEncoding, 
                        font, paint, 
                        marginWidth, lineMode,
                        &trailing);
        if (y + metrics.fDescent + metrics.fLeading > 0) {

            if (textAlign == kLeft_Align) {
                //横向：左对齐
                x = boxRect.fLeft;
            }
            else {
                //右对齐或者中对齐
                SkScalar textWidth = font.measureText(text,
                                                      len - trailing,
                                                      textEncoding,
                                                      nullptr,
                                                      &paint);
                if (textAlign == kCenter_Align) {
                    //横向：中对齐
                    x = boxRect.fLeft + (marginWidth / 2) - textWidth / 2;
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
            bool hasMoreText = (text + len) < textStop;
            //当前是否为最后一行
            bool isLastLine = (y + scaledSpacing + metrics.fAscent / 2) >= boxRect.fBottom;

            visitor(text, len - trailing, textEncoding, x, y, font, paint, hasMoreText, isLastLine);
        }
        text += len;
        if (text >= textStop) {
            break;
        }
        y += scaledSpacing;
        if ((y + metrics.fAscent / 2) >= boxRect.fBottom) {
            break;
        }
    }
    return y + metrics.fDescent + metrics.fLeading;
}

template<typename T>
static bool EllipsisTextUTF(const char text[], size_t length, SkTextEncoding textEncoding,
                            bool bEndEllipsis, bool bPathEllipsis,
                            const SkFont& font, const SkPaint& paint,
                            SkScalar destWidth,
                            const char** textOut, 
                            size_t& lengthOut,
                            T& string_utf,
                            const T& ellipsisStr /* "..." */,
                            const T& pathSep /* "/\\" */)
{
    if ((text == nullptr) || (length == 0)) {
        return false;
    }
    T pathEnd;
    T string;

    size_t charBytes = 1;
    if (textEncoding == SkTextEncoding::kUTF8) {
        charBytes = 1;
    }
    if (textEncoding == SkTextEncoding::kUTF16) {
        charBytes = 2;
    }
    if (textEncoding == SkTextEncoding::kUTF32) {
        charBytes = 4;
    }
    SkScalar ellipsisWidth = font.measureText(ellipsisStr.c_str(), ellipsisStr.size()* charBytes, textEncoding, nullptr, &paint);
    SkScalar pathEndWidth = 0;    
    string.assign((const T::value_type*)text, length / charBytes);
    if (bPathEllipsis) {
        int pos = (int)string.find_last_of(pathSep);
        if (pos > 0) {
            pathEnd = string.substr(pos);
            pathEndWidth = font.measureText(pathEnd.c_str(), pathEnd.size()* charBytes, textEncoding, nullptr, &paint);
            if ((pathEndWidth + ellipsisWidth) > destWidth) {
                //宽度不足以显示路径的最后一段文字
                pathEnd.clear();
            }
        }
        if (pathEnd.empty()) {
            bEndEllipsis = true;
            pathEndWidth = 0;
        }
        else {
            string = string.substr(0, pos);
        }
    }

    if (!bEndEllipsis && pathEnd.empty()) {
        return false;
    }

    SkScalar leftWidth = destWidth - ellipsisWidth - pathEndWidth;
    if (leftWidth <= 1) {
        return false;
    }

    size_t textLen = SkTextBox::breakText(string.c_str(), string.size() * charBytes, textEncoding, font, paint, leftWidth);
    textLen /= charBytes;
    if ((textLen > 0) && (textLen <= (string.size()))) {
        string = string.substr(0, textLen);
        string += ellipsisStr;
        string += pathEnd;
        string_utf = string.c_str();
        if (textOut) {
            *textOut = (const char*)string_utf.c_str();
        }
        lengthOut = string_utf.size() * charBytes;
        return true;
    }
    return false;
}

static bool EllipsisText(const char text[], size_t length, SkTextEncoding textEncoding,
                         std::string& string_utf8,
                         std::u16string& string_utf16,
                         std::u32string& string_utf32,
                         bool bEndEllipsis, bool bPathEllipsis,
                         const SkFont& font, const SkPaint& paint,
                         SkScalar destWidth,
                         const char** textOut, size_t& lengthOut)
{
    if (textEncoding == SkTextEncoding::kUTF8) {
        return EllipsisTextUTF<std::string>(text, length, SkTextEncoding::kUTF8,
                                            bEndEllipsis, bPathEllipsis,
                                            font, paint,
                                            destWidth,
                                            textOut, lengthOut,
                                            string_utf8,
                                            "...",
                                            "\\/");
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        return EllipsisTextUTF<std::u16string>(text, length, SkTextEncoding::kUTF16,
                                            bEndEllipsis, bPathEllipsis,
                                            font, paint,
                                            destWidth,
                                            textOut, lengthOut,
                                            string_utf16,
                                            u"...",
                                            u"\\/");
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        return EllipsisTextUTF<std::u32string>(text, length, SkTextEncoding::kUTF32,
                                            bEndEllipsis, bPathEllipsis,
                                            font, paint,
                                            destWidth,
                                            textOut, lengthOut,
                                            string_utf32,
                                            U"...",
                                            U"\\/");
    }
    return false;
}

static void TextBox_DrawText(SkTextBox* textBox, 
                             SkCanvas* canvas,
                             const char text[], size_t length, SkTextEncoding textEncoding, 
                             SkScalar x, SkScalar y,
                             const SkFont& font, const SkPaint& paint,
                             bool hasMoreText, bool isLastLine) 
{

    //绘制一行文字
    SkRect boxRect;
    textBox->getBox(&boxRect);

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = textBox->getEndEllipsis();
    //绘制区域不足时，自动在绘制省略号代替文本
    //如果字符串包含反斜杠 (\\) 字符，在最后一个反斜杠之后保留尽可能多的文本。
    bool bPathEllipsis = textBox->getPathEllipsis();
    //字体属性：下划线
    bool bUnderline = textBox->getUnderline();
    //字体属性：删除线
    bool bStrikeOut = textBox->getStrikeOut();
    //单行模式
    bool isSingleLine = textBox->getLineMode() == SkTextBox::kOneLine_Mode;

    if (!bEndEllipsis && !bPathEllipsis && !bUnderline && !bStrikeOut) {
        canvas->drawSimpleText(text, length, textEncoding, x, y, font, paint);
    }
    else {
        bool needEllipsis = false;
        if (bEndEllipsis || bPathEllipsis) {
            if (isSingleLine) {                
                //单行模式
                SkScalar textWidth = font.measureText(text, length, textEncoding, nullptr, &paint);
                if ((x + textWidth) > boxRect.fRight) {
                    //文字超出边界，需要增加"..."替代无法显示的文字
                    needEllipsis = true;
                }
            }
            else {
                //多行模式
                if (bEndEllipsis && isLastLine && hasMoreText) {
                    //文字超出边界，需要增加"..."替代无法显示的文字
                    needEllipsis = true;
                }
            }
        }
        if(!needEllipsis && !bUnderline && !bStrikeOut) {
            canvas->drawSimpleText(text, length, textEncoding, x, y, font, paint);
        }
        else {
            std::string string_utf8;
            std::u16string string_utf16;
            std::u32string string_utf32;
            if (needEllipsis) {
                const char* textOut = nullptr;
                size_t lengthOut = 0;
                if (EllipsisText(text, length, textEncoding,
                                 string_utf8, string_utf16, string_utf32,
                                 bEndEllipsis, bPathEllipsis,
                                 font, paint,
                                 boxRect.fRight - x,
                                 &textOut, lengthOut)) {
                    //修改text和length的值，但不改变textEncoding
                    SkASSERT(textOut != nullptr);
                    SkASSERT(lengthOut != 0);
                    text = textOut;
                    length = lengthOut;
                }
            }
            //绘制文本
            canvas->drawSimpleText(text, length, textEncoding, x, y, font, paint);
            if (bUnderline || bStrikeOut) {
                SkScalar width = font.measureText(text, length, textEncoding, nullptr, &paint);

                // Default fraction of the text size to use for a strike-through or underline.
                static constexpr SkScalar kLineThicknessFactor = (SK_Scalar1 / 18);
                // Fraction of the text size to raise the center of a strike-through line above
                // the baseline.
                const SkScalar kStrikeThroughOffset = (SK_Scalar1 * 65 / 252);
                // Fraction of the text size to lower an underline below the baseline.
                const SkScalar kUnderlineOffset = (SK_Scalar1 / 9);

                if (bStrikeOut) {
                    //绘制删除线
                    SkScalar thickness_factor = kLineThicknessFactor;
                    const SkScalar text_size = font.getSize();
                    const SkScalar height = text_size * thickness_factor;
                    const SkScalar top = y - text_size * kStrikeThroughOffset - height / 2;
                    SkScalar x_scalar = SkIntToScalar(x);
                    const SkRect r = SkRect::MakeLTRB(x_scalar, top, x_scalar + width, top + height);
                    canvas->drawRect(r, paint);
                }
                if (bUnderline) {
                    //绘制下划线
                    SkScalar thickness_factor = 1.5;
                    SkScalar x_scalar = SkIntToScalar(x);
                    const SkScalar text_size = font.getSize();
                    SkRect r = SkRect::MakeLTRB(
                                                x_scalar, y + text_size * kUnderlineOffset, x_scalar + width,
                                                y + (text_size *
                                                    (kUnderlineOffset +
                                                    (thickness_factor * kLineThicknessFactor))));
                    canvas->drawRect(r, paint);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

class CanvasVisitor : public SkTextBox::Visitor {
    SkCanvas* fCanvas;
    SkTextBox* fTextBox;
public:
    CanvasVisitor(SkCanvas* canvas, SkTextBox* textBox): 
         fCanvas(canvas)
        ,fTextBox(textBox) {
    }

    void operator()(const char text[], size_t length, SkTextEncoding textEncoding, 
                    SkScalar x, SkScalar y,
                    const SkFont& font, const SkPaint& paint,
                    bool hasMoreText, bool isLastLine) override 
    {
        //调用单独封装的函数绘制文字，便于扩展
        TextBox_DrawText(fTextBox,
                         fCanvas,
                         text, length, textEncoding,
                         x, y,
                         font, paint,
                         hasMoreText, isLastLine);
    }
};

void SkTextBox::setText(const char text[], size_t len, SkTextEncoding textEncoding, 
                        const SkFont& font, const SkPaint& paint) {
    fText = text;
    fLen = len;
    fTextEncoding = textEncoding;
    fPaint = &paint;
    fFont = &font;
#ifdef _DEBUG
    //检查字符串序列是否正确
    if (textEncoding == SkTextEncoding::kUTF8) {
        SkASSERT(SkUTF8_CountUnichars(text, len) != -1);
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        SkASSERT(SkUTF16_CountUnichars(text, len) != -1);
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        SkASSERT(SkUTF32_CountUnichars(text, len) != -1);
    }
    else {
        SkASSERT(false);
    }
#endif
}

void SkTextBox::draw(SkCanvas* canvas, 
                     const char text[], size_t len, SkTextEncoding textEncoding, 
                     const SkFont& font, const SkPaint& paint) {
    setText(text, len, textEncoding, font, paint);
    draw(canvas);
}

void SkTextBox::draw(SkCanvas* canvas) {
    SkASSERT(canvas != nullptr);
    if (canvas == nullptr) {
        return;
    }

    SkASSERT(fText != nullptr);
    if (fText == nullptr) {
        return;
    }
    if (fLen == 0) {
        return;
    }
    SkASSERT((fTextEncoding == SkTextEncoding::kUTF8) ||
             (fTextEncoding == SkTextEncoding::kUTF16)||
             (fTextEncoding == SkTextEncoding::kUTF32));
    if ((fTextEncoding != SkTextEncoding::kUTF8)  &&
        (fTextEncoding != SkTextEncoding::kUTF16) &&
        (fTextEncoding != SkTextEncoding::kUTF32)) {
        return;
    }

    SkASSERT(fFont != nullptr);
    SkASSERT(fPaint != nullptr);
    if ((fPaint == nullptr) || (fFont == nullptr)) {
        return;
    }

#ifdef _DEBUG
    //检查字符串序列是否正确
    if (fTextEncoding == SkTextEncoding::kUTF8) {
        SkASSERT(SkUTF8_CountUnichars(fText, fLen) != -1);
    }
    else if (fTextEncoding == SkTextEncoding::kUTF16) {
        SkASSERT(SkUTF16_CountUnichars(fText, fLen) != -1);
    }
    else if (fTextEncoding == SkTextEncoding::kUTF32) {
        SkASSERT(SkUTF32_CountUnichars(fText, fLen) != -1);
    }
#endif

    int saveCount = 0;
    if (fClipBox) {
        saveCount = canvas->save();
        canvas->clipRect(fBox, true);
    }
    CanvasVisitor sink(canvas, this);
    this->visit(sink);
    if (fClipBox) {
        canvas->restoreToCount(saveCount);
    }
}

int SkTextBox::countLines() const {
    return SkTextLineBreaker::CountLines(fText, fLen, fTextEncoding, 
                                         *fFont, *fPaint, fBox.width(),
                                         fLineMode);
}

SkScalar SkTextBox::getTextHeight() const {
    SkScalar spacing = fFont->getSize() * fSpacingMul + fSpacingAdd;
    return this->countLines() * spacing;
}

///////////////////////////////////////////////////////////////////////////////

class TextBlobVisitor : public SkTextBox::Visitor {
public:
    SkTextBlobBuilder fBuilder;

    void operator()(const char text[], size_t length, SkTextEncoding textEncoding, 
                    SkScalar x, SkScalar y,
                    const SkFont& font, const SkPaint& /*paint*/,
                    bool /*hasMoreText*/, bool /*isLastLine*/) override {
        const int count = font.countText(text, length, textEncoding);
        SkTextBlobBuilder::RunBuffer runBuffer = fBuilder.allocRun(font, count, x, y);
        font.textToGlyphs(text, length, textEncoding, runBuffer.glyphs, count);
    }
};

sk_sp<SkTextBlob> SkTextBox::snapshotTextBlob(SkScalar* computedBottom) const {
    TextBlobVisitor visitor;
    SkScalar newB = this->visit(visitor);
    if (computedBottom) {
        *computedBottom = newB;
    }
    return visitor.fBuilder.make();
}

bool SkTextBox::TextToGlyphs(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                             const SkFont& font,
                             std::vector<SkGlyphID>& glyphs,
                             std::vector<size_t>& glyphChars,
                             size_t& charBytes)
{
    glyphs.resize(byteLength, { 0, });
    int glyphsCount = font.textToGlyphs(text, byteLength, textEncoding, glyphs.data(), (int)glyphs.size());
    if (glyphsCount <= 0) {
        return false;
    }
    SkASSERT(glyphsCount <= glyphs.size());
    glyphs.resize(glyphsCount);

    glyphChars.resize(glyphs.size(), 1);
    if (textEncoding == SkTextEncoding::kUTF8) {
        charBytes = 1;
        const char* utf8 = static_cast<const char*>(text);
        int count = 0;
        const char* stop = utf8 + byteLength;
        while (utf8 < stop) {
            size_t numChars = 1;
            int type = SkUTF8_ByteType(*(const uint8_t*)utf8);
            SkASSERT(type >= -1 && type <= 4);
            if (!SkUTF8_TypeIsValidLeadingByte(type) || utf8 + type > stop) {
                // Sequence extends beyond end.
                glyphChars.clear();
                SkASSERT(glyphChars.size() == glyphs.size());
                break;
            }
            while (type-- > 1) {
                ++numChars;
                ++utf8;
                if (!SkUTF8_ByteIsContinuation(*(const uint8_t*)utf8)) {
                    glyphChars.clear();
                    SkASSERT(glyphChars.size() == glyphs.size());
                    break;
                }
            }
            if (count < glyphChars.size()) {
                glyphChars[count] = numChars;
            }
            else {
                glyphChars.clear();
                SkASSERT(glyphChars.size() == glyphs.size());
                break;
            }
            ++utf8;
            ++count;
        }
    }
    else if (textEncoding == SkTextEncoding::kUTF16) {
        charBytes = 2;
        const uint16_t* src = static_cast<const uint16_t*>(text);
        const uint16_t* stop = src + (byteLength >> 1);
        int count = 0;
        while (src < stop) {
            size_t numChars = 1;
            unsigned c = *src++;
            SkASSERT(!SkUTF16_IsLowSurrogate(c));
            if (SkUTF16_IsHighSurrogate(c)) {
                if (src >= stop) {
                    glyphChars.clear();
                    SkASSERT(glyphChars.size() == glyphs.size());
                    break;
                }
                c = *src++;
                if (!SkUTF16_IsLowSurrogate(c)) {
                    glyphChars.clear();
                    SkASSERT(glyphChars.size() == glyphs.size());
                    break;
                }
                numChars = 2;
            }
            if (count < glyphChars.size()) {
                glyphChars[count] = numChars;
            }
            else {
                glyphChars.clear();
                SkASSERT(glyphChars.size() == glyphs.size());
                break;
            }
            count += 1;
        }
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        charBytes = 4;
    }
    else {
        SkASSERT(false);
        return false;
    }
    SkASSERT(!glyphChars.empty());
    SkASSERT(glyphChars.size() == glyphs.size());
    if (glyphChars.empty() || (glyphChars.size() != glyphs.size())) {
        return false;
    }
    return true;
}

size_t SkTextBox::breakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                            const SkFont& font, const SkPaint& paint,
                            SkScalar maxWidth, SkScalar* measuredWidth)
{
    if ((maxWidth <= 0) || (byteLength == 0)){
        if (measuredWidth != nullptr) {
            *measuredWidth = 0;
        }
        return 0;
    }
    SkScalar width = font.measureText(text, byteLength, textEncoding);
    if (width <= maxWidth) {
        if (measuredWidth != nullptr) {
            *measuredWidth = width;
        }
        return byteLength;
    }

    std::vector<SkGlyphID> glyphs;
    //计算每个glyphs对应的字符个数
    std::vector<size_t> glyphChars;
    //每个字符的字节数
    size_t charBytes = 1;

    if (!TextToGlyphs(text, byteLength, textEncoding, font, glyphs, glyphChars, charBytes)) {
        if (measuredWidth != nullptr) {
            *measuredWidth = width;
        }
        return byteLength;
    }

    std::vector<SkScalar> glyphWidths;
    glyphWidths.resize(glyphs.size(), 0);
    font.getWidthsBounds(glyphs.data(), (int)glyphs.size(), glyphWidths.data(), nullptr, &paint);

    size_t breakByteLength = 0;//单位是字节
    SkScalar totalWidth = 0;
    for (size_t i = 0; i < glyphWidths.size(); ++i) {        
        if ((totalWidth + glyphWidths[i]) > maxWidth) {
            for (size_t index = 0; index < i; ++index) {
                //计算字符个数
                breakByteLength += (glyphChars[index] * charBytes);
            }
            break;
        }
        totalWidth += glyphWidths[i];        
    }
    if (measuredWidth != nullptr) {
        *measuredWidth = totalWidth;
    }
    SkASSERT(breakByteLength <= byteLength);
    if (breakByteLength > byteLength) {
        breakByteLength = byteLength;
    }
    return breakByteLength;
}

}//namespace ui

////以下为breakText的测试函数，用于回归测试
//static void test_breakText(const void* text, size_t byteLength, SkTextEncoding textEncoding)
//{
//    SkFont font;
//    font.setTypeface(SkTypeface::MakeFromName("Microsoft Yahei", SkFontStyle()));
//    font.setSize(48.0f);
//
//    SkPaint paint;
//    SkScalar width = font.measureText(text, byteLength, textEncoding);
//
//    SkScalar mm = 0;
//    SkScalar nn = 0;
//    for (SkScalar w = 0; w <= width; w += SK_Scalar1) {
//        SkScalar m;
//        size_t n = SkTextBox::breakText(text, byteLength, textEncoding, font, paint, w, &m);
//
//        SkASSERT(n <= byteLength);
//        SkASSERT(m <= width);
//
//        if (n == 0) {
//            SkASSERT(m == 0);
//        }
//        else {
//            // now assert that we're monotonic
//            if (n == nn) {
//                SkASSERT(m == mm);
//            }
//            else {
//                SkASSERT(n > nn);
//                SkASSERT(m > mm);
//            }
//        }
//        nn = SkIntToScalar(n);
//        mm = m;
//    }
//
//    SkDEBUGCODE(size_t length2 = ) SkTextBox::breakText(text, byteLength, textEncoding, font, paint, width, &mm);
//    SkASSERT(length2 == byteLength);
//    SkASSERT(mm == width);
//}
//
//void TestBreakText()
//{
//    std::string textUTF8 = u8"UTF8: ssdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj 中文字符串";
//    test_breakText(textUTF8.c_str(), textUTF8.size(), SkTextEncoding::kUTF8);
//
//    std::u16string textUTF16 = u"UTF16: sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj 中文字符串\xD852\xDF62\xD83D\xDC69";
//    test_breakText(textUTF16.c_str(), textUTF16.size() * 2, SkTextEncoding::kUTF16);
//
//    std::u32string textUTF32 = U"UTF32: sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj 中文字符串";
//    test_breakText(textUTF32.c_str(), textUTF32.size() * 4, SkTextEncoding::kUTF32);
//}
