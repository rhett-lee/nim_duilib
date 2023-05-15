#ifndef UI_RENDER_COLOR_H_
#define UI_RENDER_COLOR_H_

#pragma once

#include "duilib/duilib_defs.h"

namespace ui 
{

/** 颜色值的封装(ARGB格式)
*/
class UILIB_API UiColor
{
public:

    //颜色值类型
    typedef DWORD ARGB;

    UiColor():
        Argb(0)
    {
    }

    UiColor(uint8_t r, uint8_t g, uint8_t b)
    {
        Argb = MakeARGB(255, r, g, b);
    }

    UiColor(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
    {
        Argb = MakeARGB(a, r, g, b);
    }

    explicit UiColor(ARGB argb)
    {
        Argb = argb;
    }

    uint8_t GetAlpha() const
    {
        return (uint8_t)(Argb >> AlphaShift);
    }

    uint8_t GetA() const
    {
        return GetAlpha();
    }

    uint8_t GetRed() const
    {
        return (uint8_t)(Argb >> RedShift);
    }

    uint8_t GetR() const
    {
        return GetRed();
    }

    uint8_t GetGreen() const
    {
        return (uint8_t)(Argb >> GreenShift);
    }

    uint8_t GetG() const
    {
        return GetGreen();
    }

    uint8_t GetBlue() const
    {
        return (uint8_t)(Argb >> BlueShift);
    }

    uint8_t GetB() const
    {
        return GetBlue();
    }

    ARGB GetARGB() const
    {
        return Argb;
    }

    void SetARGB(ARGB argb)
    {
        Argb = argb;
    }

    void SetFromCOLORREF(COLORREF rgb)
    {
        Argb = MakeARGB(255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    }

    COLORREF ToCOLORREF() const
    {
        return RGB(GetRed(), GetGreen(), GetBlue());
    }

    // Shift count and bit mask for A, R, G, B components
    enum
    {
        AlphaShift = 24,
        RedShift   = 16,
        GreenShift = 8,
        BlueShift  = 0
    };

    enum
    {
        AlphaMask = 0xff000000,
        RedMask   = 0x00ff0000,
        GreenMask = 0x0000ff00,
        BlueMask  = 0x000000ff
    };

    // Assemble A, R, G, B values into a 32-bit integer
    static ARGB MakeARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
    {
        return (((ARGB)(b) << BlueShift)  |
                ((ARGB)(g) << GreenShift) |
                ((ARGB)(r) << RedShift)   |
                ((ARGB)(a) << AlphaShift));
    }

private:
    //颜色值
    ARGB Argb;
};

} // namespace ui

#endif // UI_RENDER_CLIP_H_
