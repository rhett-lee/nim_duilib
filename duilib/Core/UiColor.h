#ifndef UI_CORE_UICOLOR_H_
#define UI_CORE_UICOLOR_H_

#pragma once

#include "duilib/Core/UiColors.h"

namespace ui 
{

/** 颜色值的封装(ARGB格式)
*/
class UILIB_API UiColor
{
public:

    //颜色值类型
    typedef uint32_t ARGB;

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

    explicit UiColor(int32_t argb)
    {
        Argb = (ARGB)argb;
    }

    /** 获取颜色的Alpha值
    */
    uint8_t GetAlpha() const { return (uint8_t)(Argb >> AlphaShift); }

    /** 获取颜色的Alpha值
    */
    uint8_t GetA() const { return (uint8_t)(Argb >> AlphaShift); }

    /** 获取颜色的Red值
    */
    uint8_t GetRed() const { return (uint8_t)(Argb >> RedShift); }

    /** 获取颜色的Red值
    */
    uint8_t GetR() const { return (uint8_t)(Argb >> RedShift); }

    /** 获取颜色的Green值
    */
    uint8_t GetGreen() const { return (uint8_t)(Argb >> GreenShift); }

    /** 获取颜色的Green值
    */
    uint8_t GetG() const { return (uint8_t)(Argb >> GreenShift); }

    /** 获取颜色的Blue值
    */
    uint8_t GetBlue() const { return (uint8_t)(Argb >> BlueShift); }

    /** 获取颜色的Blue值
    */
    uint8_t GetB() const { return (uint8_t)(Argb >> BlueShift); }

    /** 获取颜色的ARGB值
    */
    ARGB GetARGB() const { return Argb; }

    /** 颜色值是否为空值
    */
    bool IsEmpty() const { return Argb == 0; }

    /** 设置颜色的ARGB值
    */
    void SetARGB(ARGB argb) { Argb = argb; }

    /** 从COLORREF颜色值设置颜色（COLORREF仅包含RGB值，不含A值）
    */
    void SetFromCOLORREF(uint32_t rgb)
    {
        Argb = MakeARGB(255, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    }

    /** 将ARGB颜色转换为COLORREF颜色值（COLORREF仅包含RGB值，不含A值）
    */
    uint32_t ToCOLORREF() const
    {
        return RGB(GetRed(), GetGreen(), GetBlue());
    }

    /** 判断是否与另外一个点相同
    */
    bool Equals(const UiColor& dst) const
    {
        return Argb == dst.Argb;
    }

    /** 判断两个值是否相等
    */
    friend bool operator == (const UiColor& a, const UiColor& b)
    {
        return a.Equals(b);
    }

    /** 判断两个值是否不相等
    */
    friend bool operator != (const UiColor& a, const UiColor& b)
    {
        return !a.Equals(b);
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

#endif // UI_CORE_UICOLOR_H_
