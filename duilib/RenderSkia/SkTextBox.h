/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef UI_RENDER_SKIA_SK_TEXT_BOX_H_
#define UI_RENDER_SKIA_SK_TEXT_BOX_H_

#include "include/core/SkCanvas.h"

//该文件原始文件的出处：skia/chrome_67/include/utils/SkTextBox.h
//基于原始文件，有修改，以兼容最新版本的skia代码（2023-06-25）
//原始文件从chrome 68以后就删除了。

namespace ui
{

/** \class SkTextBox

    SkTextBox is a helper class for drawing 1 or more lines of text
    within a rectangle. The textbox is positioned and clipped by its Frame.
    The Margin rectangle controls where the text is drawn relative to
    the Frame. Line-breaks occur inside the Margin rectangle.

    Spacing is a linear equation used to compute the distance between lines
    of text. Spacing consists of two scalars: mul and add, and the spacing
    between lines is computed as: spacing = font.getSize() * mul + add
*/
class SkTextBox 
{
public:
    SkTextBox();

    //原始函数：SkPaint::breakText 这个函数在最新的Skia代码里面已经删除了，这个是自己实现的版本。
    /** Returns the bytes of text that fit within maxWidth.
        @param text           character codes or glyph indices to be measured
        @param byteLength     number of bytes of text to measure
        @param textEncoding   text encoding, SkTextEncoding::kUTF8 or kUTF16 or kUTF32
        @param maxWidth       advance limit; text is measured while advance is less than maxWidth
        @param measuredWidth  returns the width of the text less than or equal to maxWidth
        @return               bytes of text that fit, always less than or equal to length
    */
    static size_t breakText(const void* text, size_t byteLength, SkTextEncoding textEncoding,
                            const SkFont& font, const SkPaint& paint,
                            SkScalar maxWidth, SkScalar* measuredWidth = nullptr);

public:
    //换行模式
    enum Mode {
        kOneLine_Mode,   //Char Break模式，按字符分行
        kLineBreak_Mode, //Word Break模式，按单词分行（默认）

        kModeCount
    };
    Mode    getMode() const { return (Mode)fMode; }
    void    setMode(Mode);

    //纵向对齐方式
    enum SpacingAlign {
        kStart_SpacingAlign,  //上对齐，相当于：top 对齐(默认)
        kCenter_SpacingAlign, //中对齐，相当于：vcener 对齐
        kEnd_SpacingAlign,    //下对齐，相当于：bottom 对齐

        kSpacingAlignCount
    };
    SpacingAlign getSpacingAlign() const { return (SpacingAlign)fSpacingAlign; }
    void         setSpacingAlign(SpacingAlign);

    //横向对齐方式
    enum TextAlign {
        kLeft_Align,        //左对齐(默认)
        kCenter_Align,      //中对齐
        kRight_Align,       //右对齐

        kAlignCount
    };
    TextAlign getTextAlign() const { return (TextAlign)fTextAlign; }
    void      setTextAlign(TextAlign);

    void getBox(SkRect*) const;
    void setBox(const SkRect&);
    void setBox(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom);

    //行间距：mul为行间距的倍数，add 为增加多少
    //设置后，实际的行间距为：fontHeight * mul + add;
    void getSpacing(SkScalar* mul, SkScalar* add) const;
    void setSpacing(SkScalar mul, SkScalar add);

    void draw(SkCanvas*, 
              const char text[], size_t len, SkTextEncoding, 
              const SkFont&, const SkPaint&);

    void setText(const char text[], size_t len, SkTextEncoding, 
                 const SkFont&, const SkPaint&);
    void draw(SkCanvas*);
    int  countLines() const;
    SkScalar getTextHeight() const;

    sk_sp<SkTextBlob> snapshotTextBlob(SkScalar* computedBottom) const;

    class Visitor {
    public:
        virtual ~Visitor() {}
        virtual void operator()(const char*, size_t, SkTextEncoding, 
                                SkScalar x, SkScalar y, 
                                const SkFont&, const SkPaint&) = 0;
    };

private:
    SkRect      fBox;
    SkScalar    fSpacingMul, fSpacingAdd;
    uint8_t     fMode, fSpacingAlign, fTextAlign;
    const char* fText;
    size_t      fLen;
    SkTextEncoding fTextEncoding;
    const SkPaint* fPaint;
    const SkFont* fFont;

    SkScalar visit(Visitor&, 
                   const char text[], size_t len, SkTextEncoding, 
                   const SkFont&, const SkPaint&) const;
};

class SkTextLineBreaker {
public:
    static int CountLines(const char text[], size_t len, SkTextEncoding, 
                          const SkFont&, const SkPaint&, 
                          SkScalar width);
};

} //namespace ui

#endif //UI_RENDER_SKIA_SK_TEXT_BOX_H_
