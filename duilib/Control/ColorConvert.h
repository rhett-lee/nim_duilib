/////////////////////////////////////////////////////////////////////////////////////
// color picker helper class by:																	   //
// codeproject.com/Articles/2577/Xguiplus-A-set-of-Photoshop-s-like-color-pickers  //
//																				   //
/////////////////////////////////////////////////////////////////////////////////////
#ifndef UI_CONTROL_COLOR_CONVERT_H_
#define UI_CONTROL_COLOR_CONVERT_H_

#pragma once

#include <math.h>
#include <stdint.h>

namespace ui
{

typedef unsigned long COLORREF;

/** 颜色类型（RGB/HSV/HSL）转换类
*/
class ColorConvert
{
public:
    // Convert from hsv to rgb value.
    static int HSV2RGB(double hue, double sat, double value, double* red, double* green, double* blue);

    // Convert from hsv to rgb value.
    static int HSV2RGB(double hue, double sat, double value, uint8_t& red, uint8_t& green, uint8_t& blue);

    // Convert from rgb to hsv value.
    static int RGB2HSV(double red, double green, double blue, double* hue, double* sat, double* value);

    // Convert from hsl to rgb value.
    static int HSL2RGB(double hue, double sat, double lightness, double* red, double* green, double* blue);

    // Convert from hsl to rgb value.
    static int HSL2RGB(double hue, double sat, double lightness, uint8_t& red, uint8_t& green, uint8_t& blue);

    // Convert from rgb to hsl value.
    static int RGB2HSL(double red, double green, double blue, double* hue, double* sat, double* lightness);

    // Get rgb value.
    static void GetRGB(uint32_t* buffer, int samples, COLORREF start, COLORREF end);

    // hsv from hue.
    static void HSV_HUE(uint32_t* buffer, int samples, double sat, double val);

    // hsv from sat.
    static void HSV_SAT(uint32_t* buffer, int samples, double hue, double val);

    // hsv value.
    static void HSV_VAL(uint32_t* buffer, int samples, double hue, double sat);

    // hsl from hue.
    static void HSL_HUE(uint32_t* buffer, int samples, double sat, double lightness);

    // hsl from sat.
    static void HSL_SAT(uint32_t* buffer, int samples, double hue, double lightness);

    // hsl lightness.
    static void HSL_LIG(uint32_t* buffer, int samples, double hue, double sat);
};

} //namespace ui

#endif //UI_CONTROL_COLOR_CONVERT_H_
