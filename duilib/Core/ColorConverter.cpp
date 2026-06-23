#include "duilib/Core/ColorConverter.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace ui
{

bool ColorConverter::ParseHexColor(const std::string& colorStr, uint8_t& alpha, uint8_t& r, uint8_t& g, uint8_t& b)
{
    if (colorStr.empty() || colorStr.length() != 9 || colorStr[0] != '#') {
        return false;
    }

    try {
        alpha = static_cast<uint8_t>(std::stoul(colorStr.substr(1, 2), nullptr, 16));
        r = static_cast<uint8_t>(std::stoul(colorStr.substr(3, 2), nullptr, 16));
        g = static_cast<uint8_t>(std::stoul(colorStr.substr(5, 2), nullptr, 16));
        b = static_cast<uint8_t>(std::stoul(colorStr.substr(7, 2), nullptr, 16));
        return true;
    }
    catch (...) {
        return false;
    }
}

std::string ColorConverter::RGBToHex(uint8_t alpha, uint8_t r, uint8_t g, uint8_t b)
{
    std::ostringstream oss;
    oss << "#" << std::uppercase << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(alpha)
        << std::setw(2) << static_cast<int>(r)
        << std::setw(2) << static_cast<int>(g)
        << std::setw(2) << static_cast<int>(b);
    return oss.str();
}

void ColorConverter::RGBToHSL(uint8_t r, uint8_t g, uint8_t b, double& h, double& s, double& l)
{
    // 归一化到 0~1
    double rf = r / 255.0;
    double gf = g / 255.0;
    double bf = b / 255.0;

    double maxVal = std::max({rf, gf, bf});
    double minVal = std::min({rf, gf, bf});
    double delta = maxVal - minVal;

    // 明度
    l = (maxVal + minVal) / 2.0;

    // 饱和度
    if (delta < 1e-10) {
        s = 0.0;
        h = 0.0;
        return;
    }

    s = (l <= 0.5) ? (delta / (maxVal + minVal)) : (delta / (2.0 - maxVal - minVal));

    // 色相
    if (maxVal == rf) {
        h = 60.0 * (fmod(((gf - bf) / delta), 6.0));
    }
    else if (maxVal == gf) {
        h = 60.0 * (((bf - rf) / delta) + 2.0);
    }
    else {
        h = 60.0 * (((rf - gf) / delta) + 4.0);
    }

    if (h < 0.0) {
        h += 360.0;
    }
}

void ColorConverter::HSLToRGB(double h, double s, double l, uint8_t& r, uint8_t& g, uint8_t& b)
{
    // 饱和度和明度限制在 [0, 1]
    s = std::max(0.0, std::min(1.0, s));
    l = std::max(0.0, std::min(1.0, l));

    // 色相归一化到 [0, 360)
    h = fmod(h, 360.0);
    if (h < 0.0) {
        h += 360.0;
    }

    if (s < 1e-10) {
        // 灰度
        uint8_t gray = static_cast<uint8_t>(std::round(l * 255.0));
        r = g = b = gray;
        return;
    }

    double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - std::abs(fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c / 2.0;

    double r1, g1, b1;
    if (h < 60.0) {
        r1 = c; g1 = x; b1 = 0;
    }
    else if (h < 120.0) {
        r1 = x; g1 = c; b1 = 0;
    }
    else if (h < 180.0) {
        r1 = 0; g1 = c; b1 = x;
    }
    else if (h < 240.0) {
        r1 = 0; g1 = x; b1 = c;
    }
    else if (h < 300.0) {
        r1 = x; g1 = 0; b1 = c;
    }
    else {
        r1 = c; g1 = 0; b1 = x;
    }

    r = static_cast<uint8_t>(std::round((r1 + m) * 255.0));
    g = static_cast<uint8_t>(std::round((g1 + m) * 255.0));
    b = static_cast<uint8_t>(std::round((b1 + m) * 255.0));
}

double ColorConverter::GetRelativeLuminance(uint8_t r, uint8_t g, uint8_t b)
{
    double rs = r / 255.0;
    double gs = g / 255.0;
    double bs = b / 255.0;

    rs = (rs > 0.03928) ? rs : rs / 12.92;
    gs = (gs > 0.03928) ? gs : gs / 12.92;
    bs = (bs > 0.03928) ? bs : bs / 12.92;

    return 0.2126 * rs + 0.7152 * gs + 0.0722 * bs;
}

double ColorConverter::CalculateContrastRatio(const std::string& color1, const std::string& color2)
{
    uint8_t a1, r1, g1, b1;
    uint8_t a2, r2, g2, b2;

    if (!ParseHexColor(color1, a1, r1, g1, b1) || !ParseHexColor(color2, a2, r2, g2, b2)) {
        return 0.0;
    }

    double l1 = GetRelativeLuminance(r1, g1, b1);
    double l2 = GetRelativeLuminance(r2, g2, b2);

    double lighter = std::max(l1, l2);
    double darker = std::min(l1, l2);

    return (lighter + 0.05) / (darker + 0.05);
}

bool ColorConverter::RGBToOKLCH(uint8_t r, uint8_t g, uint8_t b, double& L, double& C, double& H)
{
    double red = r / 255.0;
    double green = g / 255.0;
    double blue = b / 255.0;

    return RGBToOKLCH(red, green, blue, &L, &C, &H) == 0;
}

int ColorConverter::RGBToOKLCH(double red, double green, double blue, double* L, double* C, double* H)
{
    if ((L == nullptr) || (C == nullptr) || (H == nullptr)) {
        return -1;
    }
    if (red < 0.0 || red > 1.0 || green < 0.0 || green > 1.0 || blue < 0.0 || blue > 1.0) {
        return -1;
    }

    double r_lin = InverseSRGBTransferFunction(red);
    double g_lin = InverseSRGBTransferFunction(green);
    double b_lin = InverseSRGBTransferFunction(blue);

    double l_ = 0.4122214708 * r_lin + 0.5363325363 * g_lin + 0.0514459929 * b_lin;
    double m_ = 0.2119034982 * r_lin + 0.6806995451 * g_lin + 0.1073969566 * b_lin;
    double s_ = 0.0883024619 * r_lin + 0.2817188376 * g_lin + 0.6299787005 * b_lin;

    double l = Cbrt(l_);
    double m = Cbrt(m_);
    double s = Cbrt(s_);

    *L = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
    double a = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
    double b_c = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;

    *C = std::sqrt(a * a + b_c * b_c);
    *H = std::atan2(b_c, a) * 180.0 / PI;
    if (*H < 0.0) {
        *H += 360.0;
    }

    return 0;
}

std::string ColorConverter::OKLCHToARGB(double L, double C, double H, uint8_t alpha)
{
    uint8_t r, g, b;
    OKLCHToRGB(L, C, H, r, g, b);
    return RGBToHex(alpha, r, g, b);
}

int ColorConverter::OKLCHToRGB(double L, double C, double H, double* red, double* green, double* blue)
{
    if ((red == nullptr) || (green == nullptr) || (blue == nullptr)) {
        return -1;
    }
    // Clamp L to valid range [0, 1]
    if (L < 0.0) L = 0.0;
    if (L > 1.0) L = 1.0;
    // C should be non-negative, use absolute value if negative
    if (C < 0.0) C = 0.0;
    // Normalize H to [0, 360)
    while (H < 0.0) H += 360.0;
    while (H >= 360.0) H -= 360.0;

    double a = C * std::cos(H * PI / 180.0);
    double b = C * std::sin(H * PI / 180.0);

    double l_ = L + 0.3963377774 * a + 0.2158037573 * b;
    double m_ = L - 0.1055613458 * a - 0.0638541728 * b;
    double s_ = L - 0.0894841775 * a - 1.2914855480 * b;

    double l = l_ * l_ * l_;
    double m = m_ * m_ * m_;
    double s = s_ * s_ * s_;

    *red = +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    *green = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    *blue = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    *red = sRGBTransferFunction(std::max(0.0, std::min(1.0, *red)));
    *green = sRGBTransferFunction(std::max(0.0, std::min(1.0, *green)));
    *blue = sRGBTransferFunction(std::max(0.0, std::min(1.0, *blue)));

    return 0;
}

int ColorConverter::OKLCHToRGB(double L, double C, double H, uint8_t& red, uint8_t& green, uint8_t& blue)
{
    double r = 0, g = 0, b = 0;
    if (OKLCHToRGB(L, C, H, &r, &g, &b) != 0) {
        return -1;
    }
    red = static_cast<uint8_t>(255.0 * r);
    green = static_cast<uint8_t>(255.0 * g);
    blue = static_cast<uint8_t>(255.0 * b);
    return 0;
}

double ColorConverter::sRGBTransferFunction(double x)
{
    if (x <= 0.0031308) {
        return 12.92 * x;
    }
    return 1.055 * std::pow(x, 1.0 / 2.4) - 0.055;
}

double ColorConverter::InverseSRGBTransferFunction(double x)
{
    if (x <= 0.04045) {
        return x / 12.92;
    }
    return std::pow((x + 0.055) / 1.055, 2.4);
}

double ColorConverter::Cbrt(double x)
{
    return std::pow(x, 1.0 / 3.0);
}

}  // namespace ui
