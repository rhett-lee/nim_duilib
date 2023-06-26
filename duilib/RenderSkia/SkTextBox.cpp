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
                        const SkFont& font, const SkPaint& paint, SkScalar margin,
                        size_t* trailing = nullptr)
{
    size_t lengthBreak = SkTextBox::breakText(text, stop - text, textEncoding, font, paint, margin);
    
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
        //分行逻辑：按Word分行，保证一个英文单词或者一个完整的数字不被分行显示
        bool currIsLineBreaker = SkUTF_IsLineBreaker(uni);
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
                                  const SkFont& font, const SkPaint& paint, SkScalar width)
{
    const char* stop = text + len;
    int         count = 0;

    if (width > 0)
    {
        do {
            count += 1;
            text += linebreak(text, stop, textEncoding, font, paint, width);
        } while (text < stop);
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////////

SkTextBox::SkTextBox()
{
    fBox.setEmpty();
    fSpacingMul = SK_Scalar1;
    fSpacingAdd = 0;
    fMode = kLineBreak_Mode;
    fSpacingAlign = kStart_SpacingAlign;
    fTextAlign = kLeft_Align;

    fText = nullptr;
    fLen = 0;    
    fTextEncoding = SkTextEncoding::kUTF8;

    fPaint = nullptr;
    fFont = nullptr;
}

void SkTextBox::setMode(Mode mode)
{
    SkASSERT((unsigned)mode < kModeCount);
    fMode = SkToU8(mode);
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

SkScalar SkTextBox::visit(Visitor& visitor, 
                          const char text[], size_t len, SkTextEncoding textEncoding,
                          const SkFont& font, const SkPaint& paint) const {
    SkScalar marginWidth = fBox.width();

    if (marginWidth <= 0 || len == 0) {
        return fBox.top();
    }

    const char* textStop = text + len;

    SkScalar scaledSpacing = 0;
    SkScalar height = 0;
    SkScalar fontHeight = 0;
    SkFontMetrics metrics;

    SkScalar x = fBox.fLeft;
    SkScalar y = fBox.fTop;
    fontHeight = font.getMetrics(&metrics);
    scaledSpacing = fontHeight * fSpacingMul + fSpacingAdd;
    height = fBox.height();

    //  compute Y position for first line
    {
        SkScalar textHeight = fontHeight;

        if (fMode == kLineBreak_Mode && fSpacingAlign != kStart_SpacingAlign) {
            int count = SkTextLineBreaker::CountLines(text, textStop - text, textEncoding,
                                                      font, paint, marginWidth);
            SkASSERT(count > 0);
            textHeight += scaledSpacing * (count - 1);
        }

        switch (fSpacingAlign) {
        case kStart_SpacingAlign:
            y = 0;
            break;
        case kCenter_SpacingAlign:
            y = SkScalarHalf(height - textHeight);
            break;
        default:
            SkASSERT(fSpacingAlign == kEnd_SpacingAlign);
            y = height - textHeight;
            break;
        }
        y += fBox.fTop - metrics.fAscent;
    }

    for (;;) {
        size_t trailing = 0;
        len = linebreak(text, textStop, textEncoding, font, paint, marginWidth, &trailing);
        if (y + metrics.fDescent + metrics.fLeading > 0) {

            if (fTextAlign == kLeft_Align) {
                //横向：左对齐
                x = fBox.fLeft;
            }
            else {
                //右对齐或者中对齐
                SkScalar textWidth = font.measureText(text,
                                                      len - trailing,
                                                      textEncoding,
                                                      nullptr,
                                                      &paint);
                if (fTextAlign == kCenter_Align) {
                    //横向：中对齐
                    x = fBox.fLeft + (marginWidth / 2) - textWidth / 2;
                    if (x < fBox.fLeft) {
                        x = fBox.fLeft;
                    }
                }
                else {
                    //横向，右对齐
                    x = fBox.fRight - textWidth;
                    if (x < fBox.fLeft) {
                        x = fBox.fLeft;
                    }
                }
            }
            visitor(text, len - trailing, textEncoding, x, y, font, paint);
        }
        text += len;
        if (text >= textStop) {
            break;
        }
        y += scaledSpacing;
        if (y + metrics.fAscent >= fBox.fBottom) {
            break;
        }
    }
    return y + metrics.fDescent + metrics.fLeading;
}

///////////////////////////////////////////////////////////////////////////////

class CanvasVisitor : public SkTextBox::Visitor {
    SkCanvas* fCanvas;
public:
    CanvasVisitor(SkCanvas* canvas) : fCanvas(canvas) {}

    void operator()(const char text[], size_t length, SkTextEncoding textEncoding, 
                    SkScalar x, SkScalar y,
                    const SkFont& font, const SkPaint& paint) override {
        fCanvas->drawSimpleText(text, length, textEncoding, x, y, font, paint);
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

    CanvasVisitor sink(canvas);
    this->visit(sink, text, len, textEncoding, font, paint);
}

void SkTextBox::draw(SkCanvas* canvas) {
    this->draw(canvas, fText, fLen, fTextEncoding, *fFont, *fPaint);
}

int SkTextBox::countLines() const {
    return SkTextLineBreaker::CountLines(fText, fLen, fTextEncoding, *fFont, *fPaint, fBox.width());
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
                    const SkFont& font, const SkPaint& /*paint*/) override {
        const int count = font.countText(text, length, textEncoding);
        SkTextBlobBuilder::RunBuffer runBuffer = fBuilder.allocRun(font, count, x, y);
        font.textToGlyphs(text, length, textEncoding, runBuffer.glyphs, count);
    }
};

sk_sp<SkTextBlob> SkTextBox::snapshotTextBlob(SkScalar* computedBottom) const {
    TextBlobVisitor visitor;
    SkScalar newB = this->visit(visitor, fText, fLen, fTextEncoding, *fFont, *fPaint);
    if (computedBottom) {
        *computedBottom = newB;
    }
    return visitor.fBuilder.make();
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
    glyphs.resize(byteLength, { 0, });
    int glyphsCount = font.textToGlyphs(text, byteLength, textEncoding, glyphs.data(), (int)glyphs.size());
    if (glyphsCount <= 0) {
        if (measuredWidth != nullptr) {
            *measuredWidth = width;
        }
        return byteLength;
    }
    SkASSERT(glyphsCount <= glyphs.size());
    glyphs.resize(glyphsCount);

    //计算每个glyphs对应的字符个数
    std::vector<size_t> glyphChars;
    glyphChars.resize(glyphs.size(), 1);
    if (textEncoding == SkTextEncoding::kUTF8) {
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

    SkASSERT(glyphChars.size() == glyphs.size());
    if (glyphChars.size() != glyphs.size()) {
        if (measuredWidth != nullptr) {
            *measuredWidth = width;
        }
        return byteLength;
    }
    std::vector<SkScalar> glyphWidths;
    glyphWidths.resize(glyphs.size(), 0);
    font.getWidthsBounds(glyphs.data(), (int)glyphs.size(), glyphWidths.data(), nullptr, &paint);

    //每个字符的字节数
    int charBytes = 1;
    if (textEncoding == SkTextEncoding::kUTF16) {
        charBytes = 2;
    }
    else if (textEncoding == SkTextEncoding::kUTF32) {
        charBytes = 4;
    }

    size_t breakByteLength = 0;//单位是字节
    SkScalar totalWidth = 0;
    for (size_t i = 0; i < glyphWidths.size(); ++i) {        
        if ((totalWidth + glyphWidths[i]) > maxWidth) {
            if (measuredWidth != nullptr) {
                *measuredWidth = totalWidth;
            }
            for (size_t index = 0; index < i; ++index) {
                //计算字符个数
                breakByteLength += (glyphChars[index] * charBytes);
            }
            break;
        }
        totalWidth += glyphWidths[i];
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
