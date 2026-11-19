// duilib/Core/ColorConverter 的单元测试
//
// 覆盖 Hex/RGB 互转、RGB/HSL 互转、相对亮度与对比度、
// RGB/OKLCH 互转及参数钳制等纯计算逻辑，不依赖窗口与渲染环境。

#include "duilib/Core/ColorConverter.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <string>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

#define CHECK_INT(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected) + ": expected " + \
               std::to_string(static_cast<int64_t>(expected)) + ", actual " + \
               std::to_string(static_cast<int64_t>(actual)); \
    })

#define CHECK_FLOAT(actual, expected, eps) \
    Check(std::fabs((double)(actual) - (double)(expected)) <= (eps), [&]() { \
        return std::string(#actual " ≈ " #expected) + ": expected " + \
               std::to_string((double)(expected)) + ", actual " + \
               std::to_string((double)(actual)); \
    })

/** Hex 解析与格式化 */
void TestHex()
{
    uint8_t a = 0, r = 0, g = 0, b = 0;
    CHECK(ui::ColorConverter::ParseHexColor("#FF112233", a, r, g, b));
    CHECK_INT(a, 0xFF);
    CHECK_INT(r, 0x11);
    CHECK_INT(g, 0x22);
    CHECK_INT(b, 0x33);

    //小写
    CHECK(ui::ColorConverter::ParseHexColor("#ff112233", a, r, g, b));
    CHECK_INT(a, 0xFF);
    CHECK_INT(r, 0x11);

    //全 0
    CHECK(ui::ColorConverter::ParseHexColor("#00000000", a, r, g, b));
    CHECK_INT(a, 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    //非法格式：长度或前缀不符
    CHECK(!ui::ColorConverter::ParseHexColor("", a, r, g, b));
    CHECK(!ui::ColorConverter::ParseHexColor("#FF11223", a, r, g, b));
    CHECK(!ui::ColorConverter::ParseHexColor("#FF1122334", a, r, g, b));
    CHECK(!ui::ColorConverter::ParseHexColor("FF112233", a, r, g, b));

    //格式化：大写、两位补零
    CHECK(ui::ColorConverter::RGBToHex(0xFF, 0x11, 0x22, 0x33) == "#FF112233");
    CHECK(ui::ColorConverter::RGBToHex(0, 0, 0, 0) == "#00000000");
    CHECK(ui::ColorConverter::RGBToHex(1, 2, 3, 4) == "#01020304");

    //往返
    std::string hex = ui::ColorConverter::RGBToHex(0x80, 0x44, 0x55, 0x66);
    CHECK(ui::ColorConverter::ParseHexColor(hex, a, r, g, b));
    CHECK_INT(a, 0x80);
    CHECK_INT(r, 0x44);
    CHECK_INT(g, 0x55);
    CHECK_INT(b, 0x66);
}

/** RGB <-> HSL */
void TestHsl()
{
    double h = 0, s = 0, l = 0;

    ui::ColorConverter::RGBToHSL(255, 0, 0, h, s, l);
    CHECK_FLOAT(h, 0.0, 1e-9);
    CHECK_FLOAT(s, 1.0, 1e-9);
    CHECK_FLOAT(l, 0.5, 1e-9);

    ui::ColorConverter::RGBToHSL(0, 255, 0, h, s, l);
    CHECK_FLOAT(h, 120.0, 1e-9);
    CHECK_FLOAT(s, 1.0, 1e-9);
    CHECK_FLOAT(l, 0.5, 1e-9);

    ui::ColorConverter::RGBToHSL(0, 0, 255, h, s, l);
    CHECK_FLOAT(h, 240.0, 1e-9);
    CHECK_FLOAT(s, 1.0, 1e-9);
    CHECK_FLOAT(l, 0.5, 1e-9);

    ui::ColorConverter::RGBToHSL(255, 255, 0, h, s, l);
    CHECK_FLOAT(h, 60.0, 1e-9);

    //灰度：饱和度为 0
    ui::ColorConverter::RGBToHSL(255, 255, 255, h, s, l);
    CHECK_FLOAT(s, 0.0, 1e-9);
    CHECK_FLOAT(l, 1.0, 1e-9);

    ui::ColorConverter::RGBToHSL(0, 0, 0, h, s, l);
    CHECK_FLOAT(s, 0.0, 1e-9);
    CHECK_FLOAT(l, 0.0, 1e-9);

    ui::ColorConverter::RGBToHSL(128, 128, 128, h, s, l);
    CHECK_FLOAT(s, 0.0, 1e-9);
    CHECK_FLOAT(l, 128.0 / 255.0, 1e-9);

    //HSL -> RGB
    uint8_t r = 0, g = 0, b = 0;
    ui::ColorConverter::HSLToRGB(0.0, 1.0, 0.5, r, g, b);
    CHECK_INT(r, 255);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    ui::ColorConverter::HSLToRGB(120.0, 1.0, 0.5, r, g, b);
    CHECK_INT(r, 0);
    CHECK_INT(g, 255);
    CHECK_INT(b, 0);

    ui::ColorConverter::HSLToRGB(240.0, 1.0, 0.5, r, g, b);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 255);

    ui::ColorConverter::HSLToRGB(60.0, 1.0, 0.5, r, g, b);
    CHECK_INT(r, 255);
    CHECK_INT(g, 255);
    CHECK_INT(b, 0);

    //360 度等价于 0 度
    ui::ColorConverter::HSLToRGB(360.0, 1.0, 0.5, r, g, b);
    CHECK_INT(r, 255);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    //灰度：三通道相同
    ui::ColorConverter::HSLToRGB(0.0, 0.0, 0.5, r, g, b);
    CHECK_INT(r, 128);
    CHECK_INT(g, 128);
    CHECK_INT(b, 128);

    ui::ColorConverter::HSLToRGB(0.0, 0.0, 1.0, r, g, b);
    CHECK_INT(r, 255);
    CHECK_INT(g, 255);
    CHECK_INT(b, 255);

    ui::ColorConverter::HSLToRGB(0.0, 0.0, 0.0, r, g, b);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    //往返转换
    const uint8_t colors[][3] = {
        {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
        {17, 34, 51}, {128, 128, 128}, {200, 100, 50}, {1, 2, 3}
    };
    for (const auto& color : colors) {
        double hh = 0, ss = 0, ll = 0;
        ui::ColorConverter::RGBToHSL(color[0], color[1], color[2], hh, ss, ll);
        uint8_t rr = 0, gg = 0, bb = 0;
        ui::ColorConverter::HSLToRGB(hh, ss, ll, rr, gg, bb);
        CHECK_FLOAT(rr, color[0], 1.0);
        CHECK_FLOAT(gg, color[1], 1.0);
        CHECK_FLOAT(bb, color[2], 1.0);
    }
}

/** 相对亮度与对比度 */
void TestLuminanceAndContrast()
{
    CHECK_FLOAT(ui::ColorConverter::GetRelativeLuminance(0, 0, 0), 0.0, 1e-9);
    CHECK_FLOAT(ui::ColorConverter::GetRelativeLuminance(255, 255, 255), 1.0, 1e-6);
    CHECK_FLOAT(ui::ColorConverter::GetRelativeLuminance(255, 0, 0), 0.2126, 1e-4);
    CHECK_FLOAT(ui::ColorConverter::GetRelativeLuminance(0, 255, 0), 0.7152, 1e-4);
    CHECK_FLOAT(ui::ColorConverter::GetRelativeLuminance(0, 0, 255), 0.0722, 1e-4);

    //黑白对比度约为 21:1
    CHECK_FLOAT(ui::ColorConverter::CalculateContrastRatio("#FF000000", "#FFFFFFFF"), 21.0, 0.01);
    //参数顺序不影响结果
    CHECK_FLOAT(ui::ColorConverter::CalculateContrastRatio("#FFFFFFFF", "#FF000000"), 21.0, 0.01);
    //相同颜色对比度为 1
    CHECK_FLOAT(ui::ColorConverter::CalculateContrastRatio("#FF123456", "#FF123456"), 1.0, 1e-6);
    //非法输入返回 0
    CHECK_FLOAT(ui::ColorConverter::CalculateContrastRatio("", "#FFFFFFFF"), 0.0, 1e-9);
    CHECK_FLOAT(ui::ColorConverter::CalculateContrastRatio("#FF000000", "#XYZ"), 0.0, 1e-9);
}

/** RGB <-> OKLCH */
void TestOklch()
{
    double L = -1, C = -1, H = -1;

    //白色：L≈1，C≈0
    CHECK(ui::ColorConverter::RGBToOKLCH(255, 255, 255, L, C, H));
    CHECK_FLOAT(L, 1.0, 1e-3);
    CHECK_FLOAT(C, 0.0, 1e-3);

    //黑色：L≈0，C≈0
    CHECK(ui::ColorConverter::RGBToOKLCH(0, 0, 0, L, C, H));
    CHECK_FLOAT(L, 0.0, 1e-3);
    CHECK_FLOAT(C, 0.0, 1e-3);

    //纯色参考值
    CHECK(ui::ColorConverter::RGBToOKLCH(255, 0, 0, L, C, H));
    CHECK_FLOAT(L, 0.6280, 0.01);
    CHECK_FLOAT(C, 0.2577, 0.01);
    CHECK_FLOAT(H, 29.23, 1.0);

    CHECK(ui::ColorConverter::RGBToOKLCH(0, 255, 0, L, C, H));
    CHECK_FLOAT(L, 0.8664, 0.01);
    CHECK_FLOAT(C, 0.2948, 0.01);
    CHECK_FLOAT(H, 142.5, 1.0);

    CHECK(ui::ColorConverter::RGBToOKLCH(0, 0, 255, L, C, H));
    CHECK_FLOAT(L, 0.4523, 0.01);
    CHECK_FLOAT(C, 0.3132, 0.01);
    CHECK_FLOAT(H, 264.05, 1.0);

    //非法输入
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(1.5, 0.0, 0.0, &L, &C, &H), -1);
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(-0.1, 0.0, 0.0, &L, &C, &H), -1);
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(0.0, 1.1, 0.0, &L, &C, &H), -1);
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(0.5, 0.5, 0.5, nullptr, &C, &H), -1);
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(0.5, 0.5, 0.5, &L, nullptr, &H), -1);
    CHECK_INT(ui::ColorConverter::RGBToOKLCH(0.5, 0.5, 0.5, &L, &C, nullptr), -1);

    //OKLCH -> RGB（字节版本）
    uint8_t r = 0, g = 0, b = 0;
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(1.0, 0.0, 0.0, r, g, b), 0);
    CHECK_FLOAT(r, 255.0, 1.0);
    CHECK_FLOAT(g, 255.0, 1.0);
    CHECK_FLOAT(b, 255.0, 1.0);

    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.0, 0.0, 0.0, r, g, b), 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    //红色经 OKLCH 往返后仍为红色（允许 1 的量化误差）
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.62796, 0.25768, 29.234, r, g, b), 0);
    CHECK_FLOAT(r, 255.0, 1.0);
    CHECK_FLOAT(g, 0.0, 1.0);
    CHECK_FLOAT(b, 0.0, 1.0);

    //双精度版本：参数钳制与空指针
    double dr = -1, dg = -1, db = -1;
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(2.0, 0.0, 0.0, &dr, &dg, &db), 0); //L>1 钳制为 1
    CHECK_FLOAT(dr, 1.0, 1e-3);
    CHECK_FLOAT(dg, 1.0, 1e-3);
    CHECK_FLOAT(db, 1.0, 1e-3);

    CHECK_INT(ui::ColorConverter::OKLCHToRGB(-1.0, 0.0, 0.0, &dr, &dg, &db), 0); //L<0 钳制为 0
    CHECK_FLOAT(dr, 0.0, 1e-6);
    CHECK_FLOAT(dg, 0.0, 1e-6);
    CHECK_FLOAT(db, 0.0, 1e-6);

    //负色度按 0 处理（灰色）
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.5, -0.2, 0.0, &dr, &dg, &db), 0);
    CHECK_FLOAT(dr, dg, 1e-6);
    CHECK_FLOAT(dg, db, 1e-6);

    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.5, 0.1, 0.0, nullptr, &dg, &db), -1);
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.5, 0.1, 0.0, &dr, nullptr, &db), -1);
    CHECK_INT(ui::ColorConverter::OKLCHToRGB(0.5, 0.1, 0.0, &dr, &dg, nullptr), -1);

    //RGB -> OKLCH -> RGB 往返
    const uint8_t colors[][3] = {
        {255, 255, 255}, {0, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
        {17, 34, 51}, {128, 128, 128}, {200, 100, 50}
    };
    for (const auto& color : colors) {
        double ll = 0, cc = 0, hh = 0;
        CHECK(ui::ColorConverter::RGBToOKLCH(color[0], color[1], color[2], ll, cc, hh));
        uint8_t rr = 0, gg = 0, bb = 0;
        CHECK_INT(ui::ColorConverter::OKLCHToRGB(ll, cc, hh, rr, gg, bb), 0);
        CHECK_FLOAT(rr, color[0], 1.0);
        CHECK_FLOAT(gg, color[1], 1.0);
        CHECK_FLOAT(bb, color[2], 1.0);
    }

    //OKLCH -> ARGB 字符串（字节转换存在截断，颜色分量允许 1 的量化误差）
    uint8_t a = 0, rr = 0, gg = 0, bb = 0;
    std::string argbHex = ui::ColorConverter::OKLCHToARGB(1.0, 0.0, 0.0, 255);
    CHECK(ui::ColorConverter::ParseHexColor(argbHex, a, rr, gg, bb));
    CHECK_INT(a, 0xFF);
    CHECK_FLOAT(rr, 255.0, 1.0);
    CHECK_FLOAT(gg, 255.0, 1.0);
    CHECK_FLOAT(bb, 255.0, 1.0);

    argbHex = ui::ColorConverter::OKLCHToARGB(0.0, 0.0, 0.0, 255);
    CHECK(ui::ColorConverter::ParseHexColor(argbHex, a, rr, gg, bb));
    CHECK_INT(a, 0xFF);
    CHECK_INT(rr, 0);
    CHECK_INT(gg, 0);
    CHECK_INT(bb, 0);

    argbHex = ui::ColorConverter::OKLCHToARGB(1.0, 0.0, 0.0, 0x80);
    CHECK(ui::ColorConverter::ParseHexColor(argbHex, a, rr, gg, bb));
    CHECK_INT(a, 0x80);
}

} // namespace

int main()
{
    TestHex();
    TestHsl();
    TestLuminanceAndContrast();
    TestOklch();

    std::cout << "color converter tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
