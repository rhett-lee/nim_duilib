#include "ColorConvert.h"
#include <cassert>
#include <algorithm>

namespace ui
{

#define GetRValue(rgb)      ((uint8_t)(rgb))
#define GetGValue(rgb)      ((uint8_t)(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)      ((uint8_t)((rgb)>>16))
#define RGB(r,g,b)          ((COLORREF)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))

#define	HSV_LOOP_STEPS(w)	((w) - 1)

#define	HSV_0()	RGB ((uint8_t) (coef1 >> int_extend),(uint8_t) (coef3 >> int_extend),(uint8_t) (val >> int_extend))

#define	HSV_HUE_ADV_0() coef1 += coef1_adv,coef3 += coef3_adv

#define	HSV_1()	RGB ((uint8_t) (coef1 >> int_extend),(uint8_t) (val >> int_extend),(uint8_t) (coef2 >> int_extend))

#define	HSV_HUE_ADV_1()	coef1 += coef1_adv,	coef2 += coef2_adv


#define	HSV_2()	RGB ((uint8_t) (coef3 >> int_extend),(uint8_t) (val >> int_extend),(uint8_t) (coef1 >> int_extend))

#define	HSV_HUE_ADV_2()		HSV_HUE_ADV_0()

#define	HSV_3()	RGB ((uint8_t) (val >> int_extend),(uint8_t) (coef2 >> int_extend),(uint8_t) (coef1 >> int_extend))

#define	HSV_HUE_ADV_3()		HSV_HUE_ADV_1()

#define	HSV_4()	RGB ((uint8_t) (val >> int_extend),(uint8_t) (coef1 >> int_extend),(uint8_t) (coef3 >> int_extend))

#define	HSV_HUE_ADV_4()		HSV_HUE_ADV_0()

#define	HSV_5()	 RGB (	(uint8_t) (coef2 >> int_extend),(uint8_t) (coef1 >> int_extend),(uint8_t) (val >> int_extend))

#define	HSV_HUE_ADV_5()		HSV_HUE_ADV_1()

// initialize for HSV colorspace in SAT mode, for HUE between 0 and 1 (0 and 60 deg)
#define	HSV_SAT_INIT_0()	coef3 = coef1,	coef3_adv = (int) ((val - coef3) / HSV_LOOP_STEPS (j))

// advance for HSV colorspace in SAT mode, for HUE between 0 and 1 (0 and 60 deg)
#define	HSV_SAT_ADV_0()	    coef3 += coef3_adv

#define	HSV_SAT_INIT_1()	coef2 = val, coef2_adv = (int) ((val * (1.0 - sat) - coef2) / HSV_LOOP_STEPS (j))

#define	HSV_SAT_ADV_1()	    coef2 += coef2_adv

#define	HSV_SAT_INIT_2()	HSV_SAT_INIT_0()
#define	HSV_SAT_ADV_2()		HSV_SAT_ADV_0()

#define	HSV_SAT_INIT_3()	HSV_SAT_INIT_1()
#define	HSV_SAT_ADV_3()		HSV_SAT_ADV_1()

#define	HSV_SAT_INIT_4()	HSV_SAT_INIT_0()
#define	HSV_SAT_ADV_4()		HSV_SAT_ADV_0()

#define	HSV_SAT_INIT_5()	HSV_SAT_INIT_1()
#define	HSV_SAT_ADV_5()		HSV_SAT_ADV_1()

// for HSV colorspace, VAL mode is calculate in a same manner as SAT mode
//	so all macroses simply maps over SAT mode macroses
#define	HSV_VAL_INIT_0()	HSV_SAT_INIT_0()
#define	HSV_VAL_ADV_0()		HSV_SAT_ADV_0()

#define	HSV_VAL_INIT_1()	HSV_SAT_INIT_1()
#define	HSV_VAL_ADV_1()		HSV_SAT_ADV_1()

#define	HSV_VAL_INIT_2()	HSV_SAT_INIT_2()
#define	HSV_VAL_ADV_2()		HSV_SAT_ADV_2()

#define	HSV_VAL_INIT_3()	HSV_SAT_INIT_3()
#define	HSV_VAL_ADV_3()		HSV_SAT_ADV_3()

#define	HSV_VAL_INIT_4()	HSV_SAT_INIT_4()
#define	HSV_VAL_ADV_4()		HSV_SAT_ADV_4()

const int	int_extend = 20;

static inline int scaled_red(COLORREF c)
{
    return (GetRValue(c) << int_extend);
}

static inline int scaled_green(COLORREF c)
{
    return (GetGValue(c) << int_extend);
}

static inline int scaled_blue(COLORREF c)
{
    return (GetBValue(c) << int_extend);
}

template <class T, class T1> void in_range(T& x, T1 start, T1 end)
{
    if (x < static_cast <T> (start)) x = static_cast <T> (start);
    if (x > static_cast <T> (end)) x = static_cast <T> (end);
}


int ColorConvert::HSV2RGB(double hue, double sat, double value, double* red, double* green, double* blue)
{
    double	frac, coef1, coef2, coef3;
    double	intp;
    // hsv values valid?
    if (sat < 0.0 || sat > 1.0 || value < 0.0 || value > 1.0) return (-1);
    if (hue < 0.0 || hue > 360.0) return (-1);

    // gray?
    if (sat == 0.0)
        *red = *green = *blue = value;
    else
    {
        // hue (chromatic) 360 == hue 0
        if (hue == 360.0) hue = 0;
        hue = hue / 60; 						// hue in [0, 6)
        frac = modf(hue, &intp);				// split hue to integer and fraction
        coef1 = value * (1 - sat);
        coef2 = value * (1 - sat * frac);
        coef3 = value * (1 - sat * (1 - frac));
        switch ((int)intp)
        {
        case 0:	*red = value; *green = coef3; *blue = coef1; break;
        case 1:	*red = coef2; *green = value; *blue = coef1; break;
        case 2:	*red = coef1; *green = value; *blue = coef3; break;
        case 3:	*red = coef1; *green = coef2; *blue = value; break;
        case 4:	*red = coef3; *green = coef1; *blue = value; break;
        case 5:	*red = value; *green = coef1; *blue = coef2; break;
        }
    }
    return (0);
}

int ColorConvert::HSV2RGB(double hue, double sat, double value, uint8_t& red, uint8_t& green, uint8_t& blue)
{
    double r = 0;
    double g = 0;
    double b = 0;
    // ceil (a - 0.5) = round (a) = floor (a + 0.5)
    if (HSV2RGB(hue, sat, value, &r, &g, &b) == 0) {
        red = static_cast<uint8_t>(ceil(r * 255 - 0.5));
        green = static_cast<uint8_t>(ceil(g * 255 - 0.5));
        blue = static_cast<uint8_t>(ceil(b * 255 - 0.5));
        return 0;
    }
    return -1;
}

int ColorConvert::RGB2HSV(double red, double green, double blue, double* hue, double* sat, double* value)
{
    double max, min, delta;

    max = std::max(red, std::max(green, blue));
    min = std::min(red, std::min(green, blue));

    // check the rgb values to see if valid
    if (min < 0.0 || max > 1.0) return (-1); 	// out of range

    *value = max;								// calculate the value v

    if (max > 0.0)
        *sat = (max - min) / max;
    else
        // red = green = blue = 0
        *sat = 0.0;

    if (*sat == 0.0)
        *hue = 0.0;
    else
    {
        delta = max - min;
        if (red == max)
            // between yellow and magenta
            *hue = (green - blue) / delta;
        else if (green == max)
            // between cyan and yellow
            *hue = 2 + (blue - red) / delta;
        else
            // between magenta and cyan
            *hue = 4 + (red - green) / delta;

        // hue to degrees
        *hue = *hue * 60;
        if (*hue < 0)
            // make hue > 0
            *hue = *hue + 360;
    }

    assert(*hue >= 0.0 && *hue <= 360.0);
    assert(*sat >= 0.0 && *sat <= 1.0);
    assert(*value >= 0.0 && *value <= 1.0);

    return (0);
}

// hsl helper function
static inline double HSL2RGBHelper(double v1, double v2, double vH)
{
    if (vH < 0) vH += 1;
    if (vH > 1) vH -= 1;
    if (6.0 * vH < 1) return v1 + (v2 - v1) * 6.0 * vH;
    if (2.0 * vH < 1) return v2;
    if (3.0 * vH < 2) return v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6.0;
    return (v1);
}

int ColorConvert::HSL2RGB(double hue, double sat, double lightness, double* red, double* green, double* blue)
{
    if (sat < 0.0 || sat > 1.0 || lightness < 0.0 || lightness > 1.0) return (-1);
    if (hue < 0.0 || hue > 360.0) return (-1);

    double H, S, L;
    double var_1, var_2;

    H = hue / 360.0f;
    S = sat;
    L = lightness;

    if (S == 0)							 //HSL values = 0 ÷ 1
    {
        *red = L;                   //RGB results = 0 ÷ 255
        *green = L;
        *blue = L;
    }
    else
    {
        if (L < 0.5) var_2 = L * (1 + S);
        else         var_2 = (L + S) - (S * L);

        var_1 = 2.0 * L - var_2;

        *red = HSL2RGBHelper(var_1, var_2, H + (1.0 / 3.0));
        *green = HSL2RGBHelper(var_1, var_2, H);
        *blue = HSL2RGBHelper(var_1, var_2, H - (1.0 / 3.0));
    }
    return 0;
}

int ColorConvert::HSL2RGB(double hue, double sat, double lightness, uint8_t& red, uint8_t& green, uint8_t& blue)
{
    double r = 0;
    double g = 0;
    double b = 0;
    if (HSL2RGB(hue, sat, lightness, &r, &g, &b) != 0) {
        return -1;
    }
    red = static_cast<uint8_t>(255.0 * r);
    green = static_cast<uint8_t>(255.0 * g);
    blue = static_cast<uint8_t>(255.0 * b);
    return 0;
}

int ColorConvert::RGB2HSL(double red, double green, double blue, double* hue, double* sat, double* lightness)
{
    double R, G, B, Max, Min, del_R, del_G, del_B, del_Max, H, S, L;
    R = red;       //Where RGB values = 0 ÷ 255
    G = green;
    B = blue;

    Min = std::min(R, std::min(G, B));    //Min. value of RGB
    Max = std::max(R, std::max(G, B));    //Max. value of RGB

    // check the rgb values to see if valid
    if (Min < 0.0 || Max > 1.0) return (-1); 	// out of range

    del_Max = Max - Min;        //Delta RGB value

    L = (Max + Min) / 2.0;
    H = 0;

    if (del_Max == 0) {         //This is a gray, no chroma...
        H = 0;                  //HSL results = 0 ÷ 1
        S = 0;
    }
    else {                      //Chromatic data...
        if (L < 0.5) S = del_Max / (Max + Min);
        else         S = del_Max / (2.0 - Max - Min);

        del_R = ((Max - R) / 6.0) + (del_Max / 2.0) / del_Max;
        del_G = ((Max - G) / 6.0) + (del_Max / 2.0) / del_Max;
        del_B = ((Max - B) / 6.0) + (del_Max / 2.0) / del_Max;

        if (R == Max) H = (del_B - del_G);
        else if (G == Max) H = (1.0 / 3.0) + del_R - del_B;
        else if (B == Max) H = (2.0 / 3.0) + del_G - del_R;

        if (H < 0)  H += 1;
        if (H > 1)  H -= 1;
    }
    *hue = (unsigned short)(H * 360.0);
    *sat = S;
    *lightness = L;

    assert(*hue >= 0.0 && *hue <= 360.0);
    assert(*sat >= 0.0 && *sat <= 1.0);
    assert(*lightness >= 0.0 && *lightness <= 1.0);
    return 0;
}

void ColorConvert::GetRGB(uint32_t* buffer, int samples, COLORREF start, COLORREF end)
{
    int	red, green, blue;
    int	red_adv, green_adv, blue_adv;

    red = scaled_red(start);
    green = scaled_green(start);
    blue = scaled_blue(start);
    red_adv = (scaled_red(end) - red) / (samples - 1);
    green_adv = (scaled_green(end) - green) / (samples - 1);
    blue_adv = (scaled_blue(end) - blue) / (samples - 1);

    while (samples--)
    {
        // set current pixel (in DIB bitmap format is BGR, not RGB!)
        *buffer++ = RGB(
            (uint8_t)(blue >> int_extend),
            (uint8_t)(green >> int_extend),
            (uint8_t)(red >> int_extend));
        // advance color values to the next pixel
        red += red_adv;
        green += green_adv;
        blue += blue_adv;
    }
}


void ColorConvert::HSV_HUE(uint32_t* buffer, int samples, double sat, double val_fp)
{
    // value, but as integer in [0, 255 << int_extend]
    int		val;

    // loop counter
    int		j;

    // coefficients and advances
    int		coef1, coef2, coef3;
    int		coef2_adv, coef3_adv;

    // current position and advance to the next one
    double	pos, pos_adv;

    //
    // hue increments in [0, 360); indirectly
    //	intp changes - 0, 1, 2, 3, 4, 5; indirectly (separate loops)
    //	frac increments in [0, 1) six times; indirectly (coefficients)
    // sat - const, in [0, 1]
    // val - const, in [0, (255 << int_extend)]
    //
    // coef1 => val * (1 - sat)              => const, = val * (1 - sat)
    // coef2 => val * (1 - sat * frac)       => changes from val to val * (1 - sat)
    // coef3 => val * (1 - sat * (1 - frac)) => changes from val * (1 - sat) to val
    //

    // constants
    val = (int)(val_fp * 255) << int_extend;
    coef1 = (int)(val * (1 - sat));

    // prepare
    pos = 0;
    pos_adv = (double)samples / 6.0;

    // hue in [0, 60)
    pos += pos_adv;
    j = (int)pos;
    HSV_SAT_INIT_0();
    while (j--) *buffer++ = HSV_0(), HSV_SAT_ADV_0();

    pos += pos_adv;
    j = (int)pos - (int)(1 * pos_adv);
    HSV_SAT_INIT_1();
    while (j--) *buffer++ = HSV_1(), HSV_SAT_ADV_1();

    pos += pos_adv;
    j = (int)pos - (int)(2 * pos_adv);
    HSV_SAT_INIT_2();
    while (j--) *buffer++ = HSV_2(), HSV_SAT_ADV_2();

    pos += pos_adv;
    j = (int)pos - (int)(3 * pos_adv);
    HSV_SAT_INIT_3();
    while (j--) *buffer++ = HSV_3(), HSV_SAT_ADV_3();

    pos += pos_adv;
    j = (int)pos - (int)(4 * pos_adv);
    HSV_SAT_INIT_4();
    while (j--) *buffer++ = HSV_4(), HSV_SAT_ADV_4();

    pos += (pos_adv + 0.1);	// + 0.1 because of floating-point math's rounding errors
    j = (int)pos - (int)(5 * pos_adv);
    HSV_SAT_INIT_5();
    while (j--) *buffer++ = HSV_5(), HSV_SAT_ADV_5();
}


void ColorConvert::HSV_SAT(uint32_t* buffer, int samples, double hue, double val_fp)
{
    // value, but as integer in [0, 255 << int_extend]
    int		val;

    // loop counter
    int		j;

    // coefficients and advances
    signed int		coef1, coef2, coef3;
    signed int		coef1_adv, coef2_adv, coef3_adv;

    double	intp, frac;

    //
    // hue - const, in [0, 360)
    //	intp - const in 0, 1, 2, 3, 4, 5
    //	frac - const in [0, 1)
    // sat - increments, in [0, 1]; indirectly (coefficients)
    // val - const, in [0, (255 << int_extend)]
    //
    // coef1 => val * (1 - sat)              => changes from val to 0
    // coef2 => val * (1 - sat * frac)       => changes from val to val * (1 - frac)
    // coef3 => val * (1 - sat * (1 - frac)) => changes from val to val * frac
    //

    // constants
    val = (int)(val_fp * 255) << int_extend;
    frac = modf(hue / 60.0, &intp);

    // prepare
    j = samples;

    coef1 = val;
    coef1_adv = -coef1 / samples;
    coef2 = val;
    coef2_adv = (int)((1 - frac) * val - coef2) / samples;
    coef3 = val;
    coef3_adv = (int)(frac * val - coef3) / samples;

    switch ((int)intp)
    {
    case	0:
        while (j--) *buffer++ = HSV_0(), HSV_HUE_ADV_0();
        break;
    case	1:
        while (j--) *buffer++ = HSV_1(), HSV_HUE_ADV_1();
        break;
    case	2:
        while (j--) *buffer++ = HSV_2(), HSV_HUE_ADV_2();
        break;
    case	3:
        while (j--) *buffer++ = HSV_3(), HSV_HUE_ADV_3();
        break;
    case	4:
        while (j--) *buffer++ = HSV_4(), HSV_HUE_ADV_4();
        break;
    case	5:
        while (j--) *buffer++ = HSV_5(), HSV_HUE_ADV_5();
        break;
    }
}

void ColorConvert::HSV_VAL(uint32_t* buffer, int samples, double hue, double sat)
{
    // loop counter
    int		j;

    // coefficients and advances
    signed int		coef1, coef2, coef3;
    signed int		coef1_adv, coef2_adv, coef3_adv;

    int		val, val_adv;
    int		val_max;

    double	intp, frac;

    //
    // hue - const, in [0, 360)
    //	intp - const in 0, 1, 2, 3, 4, 5
    //	frac - const in [0, 1)
    // sat - const, in [0, 1]
    // val - increments, in [0, (255 << int_extend)]; indirectly (coefficients)
    //
    // coef1 => val * (1 - sat)              => changes from 0 to val * (1 - sat)
    // coef2 => val * (1 - sat * frac)       => changes from 0 to val * (1 - sat * frac)
    // coef3 => val * (1 - sat * (1 - frac)) => changes from 0 to val * (1 - sat * (1 - frac))
    //

    // constants
    frac = modf(hue / 60.0, &intp);
    val_max = 255 << int_extend;

    // prepare
    j = samples;

    coef1 = 0;
    coef1_adv = (int)(val_max * (1 - sat)) / samples;
    coef2 = 0;
    coef2_adv = (int)(val_max * (1 - sat * frac)) / samples;
    coef3 = 0;
    coef3_adv = (int)(val_max * (1 - sat * (1 - frac))) / samples;
    val = 0;
    val_adv = val_max / samples;

    switch ((int)intp)
    {
    case	0:
        while (j--) *buffer++ = HSV_0(), HSV_HUE_ADV_0(), val += val_adv;
        break;
    case	1:
        while (j--) *buffer++ = HSV_1(), HSV_HUE_ADV_1(), val += val_adv;
        break;
    case	2:
        while (j--) *buffer++ = HSV_2(), HSV_HUE_ADV_2(), val += val_adv;
        break;
    case	3:
        while (j--) *buffer++ = HSV_3(), HSV_HUE_ADV_3(), val += val_adv;
        break;
    case	4:
        while (j--) *buffer++ = HSV_4(), HSV_HUE_ADV_4(), val += val_adv;
        break;
    case	5:
        while (j--) *buffer++ = HSV_5(), HSV_HUE_ADV_5(), val += val_adv;
        break;
    }
}

void ColorConvert::HSL_HUE(uint32_t* buffer, int samples, double sat, double lightness)
{
    if ((buffer == nullptr) || (samples < 1)) {
        return;
    }
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    double hue_step = 359.9 / samples;
    double hue = 0;

    while (samples--) {
        // set current pixel (in DIB bitmap format is BGR, not RGB!)
        if (samples == 0) {
            //最后一个，赋值为359.9
            hue = 359.9;
        }
        HSL2RGB(hue, sat, lightness, red, green, blue);
        *buffer++ = RGB(blue, green, red);

        hue += hue_step;
    }
}

void ColorConvert::HSL_SAT(uint32_t* buffer, int samples, double hue, double lightness)
{
    if ((buffer == nullptr) || (samples < 1)) {
        return;
    }
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    double sat_step = 1.0 / samples;
    double sat = 0;

    while (samples--) {
        // set current pixel (in DIB bitmap format is BGR, not RGB!)
        if (samples == 0) {
            //最后一个，赋值为1.0
            sat = 1.0;
        }
        HSL2RGB(hue, sat, lightness, red, green, blue);
        *buffer++ = RGB(blue, green, red);

        sat += sat_step;
    }
}

void ColorConvert::HSL_LIG(uint32_t* buffer, int samples, double hue, double sat)
{
    if ((buffer == nullptr) || (samples < 1)) {
        return;
    }
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    double lightness_step = 1.0 / samples;
    double lightness = 0;

    while (samples--) {
        // set current pixel (in DIB bitmap format is BGR, not RGB!)

        if (samples == 0) {
            //最后一个，赋值为1.0
            lightness = 1.0;
        }
        HSL2RGB(hue, sat, lightness, red, green, blue);
        *buffer++ = RGB(blue, green, red);

        lightness += lightness_step;
    }
}

} //namespace ui