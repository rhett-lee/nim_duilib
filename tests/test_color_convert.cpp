// duilib/Control/ColorConvert 的单元测试
//
// 覆盖 HSV/HSL 与 RGB 的互转、参数校验、取色缓冲区生成函数
// （GetRGB/HSV_*/HSL_*）的写入数量与端点取值，全部为纯计算，不创建窗口。

#include "duilib/Control/ColorConvert.h"
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

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

#define CHECK_NEAR(actual, expected, eps) \
    Check(std::fabs((double)(actual) - (double)(expected)) <= (eps), [&]() { \
        return std::string(#actual " ≈ " #expected) + ": expected " + \
               std::to_string((double)(expected)) + ", actual " + \
               std::to_string((double)(actual)); \
    })

constexpr double kEps = 1e-6;

/** RGB 八位分量的颜色值（与 GetRGB 一致：低字节为 B，其次 G，再次 R） */
uint32_t MakeBgr(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint32_t>(b) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(r) << 16);
}

/** 颜色值（与 ui::MakeColorRef 一致：低字节为 R，其次 G，再次 B） */
uint32_t RgbValue(uint8_t r, uint8_t g, uint8_t b)
{
    return ui::MakeColorRef(r, g, b);
}

/** RGB -> HSV 的常用取值 */
void TestRGB2HSV()
{
    double h = 0, s = 0, v = 0;
    CHECK_INT(ui::ColorConvert::RGB2HSV(1.0, 0.0, 0.0, &h, &s, &v), 0);
    CHECK_NEAR(h, 0.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(v, 1.0, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSV(0.0, 1.0, 0.0, &h, &s, &v), 0);
    CHECK_NEAR(h, 120.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(v, 1.0, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSV(0.0, 0.0, 1.0, &h, &s, &v), 0);
    CHECK_NEAR(h, 240.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(v, 1.0, kEps);

    //黑色
    CHECK_INT(ui::ColorConvert::RGB2HSV(0.0, 0.0, 0.0, &h, &s, &v), 0);
    CHECK_NEAR(h, 0.0, kEps);
    CHECK_NEAR(s, 0.0, kEps);
    CHECK_NEAR(v, 0.0, kEps);

    //灰色：饱和度为 0
    CHECK_INT(ui::ColorConvert::RGB2HSV(0.5, 0.5, 0.5, &h, &s, &v), 0);
    CHECK_NEAR(h, 0.0, kEps);
    CHECK_NEAR(s, 0.0, kEps);
    CHECK_NEAR(v, 0.5, kEps);

    //越界与空指针
    CHECK_INT(ui::ColorConvert::RGB2HSV(-0.1, 0.0, 0.0, &h, &s, &v), -1);
    CHECK_INT(ui::ColorConvert::RGB2HSV(0.0, 0.0, 1.1, &h, &s, &v), -1);
    CHECK_INT(ui::ColorConvert::RGB2HSV(0.0, 0.0, 0.0, nullptr, &s, &v), -1);
}

/** HSV -> RGB 的常用取值与边界 */
void TestHSV2RGB()
{
    uint8_t r = 0, g = 0, b = 0;
    CHECK_INT(ui::ColorConvert::HSV2RGB(0.0, 1.0, 1.0, r, g, b), 0);
    CHECK_INT(r, 255);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    CHECK_INT(ui::ColorConvert::HSV2RGB(120.0, 1.0, 1.0, r, g, b), 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 255);
    CHECK_INT(b, 0);

    CHECK_INT(ui::ColorConvert::HSV2RGB(240.0, 1.0, 1.0, r, g, b), 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 255);

    //hue = 360 与 hue = 0 等价
    uint8_t r360 = 0, g360 = 0, b360 = 0;
    CHECK_INT(ui::ColorConvert::HSV2RGB(360.0, 1.0, 1.0, r360, g360, b360), 0);
    CHECK_INT(r360, 255);
    CHECK_INT(g360, 0);
    CHECK_INT(b360, 0);

    //饱和度为 0 时为灰度
    CHECK_INT(ui::ColorConvert::HSV2RGB(200.0, 0.0, 0.5, r, g, b), 0);
    CHECK_INT(r, 127);
    CHECK_INT(g, 127);
    CHECK_INT(b, 127);

    //参数校验
    CHECK_INT(ui::ColorConvert::HSV2RGB(-1.0, 1.0, 1.0, r, g, b), -1);
    CHECK_INT(ui::ColorConvert::HSV2RGB(361.0, 1.0, 1.0, r, g, b), -1);
    CHECK_INT(ui::ColorConvert::HSV2RGB(0.0, 1.1, 1.0, r, g, b), -1);
    CHECK_INT(ui::ColorConvert::HSV2RGB(0.0, 1.0, -0.1, r, g, b), -1);
}

/** RGB -> HSL 的常用取值 */
void TestRGB2HSL()
{
    double h = 0, s = 0, l = 0;
    CHECK_INT(ui::ColorConvert::RGB2HSL(1.0, 0.0, 0.0, &h, &s, &l), 0);
    CHECK_NEAR(h, 0.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(l, 0.5, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSL(0.0, 1.0, 0.0, &h, &s, &l), 0);
    CHECK_NEAR(h, 120.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(l, 0.5, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSL(0.0, 0.0, 1.0, &h, &s, &l), 0);
    CHECK_NEAR(h, 240.0, kEps);
    CHECK_NEAR(s, 1.0, kEps);
    CHECK_NEAR(l, 0.5, kEps);

    //白色、黑色、灰色
    CHECK_INT(ui::ColorConvert::RGB2HSL(1.0, 1.0, 1.0, &h, &s, &l), 0);
    CHECK_NEAR(h, 0.0, kEps);
    CHECK_NEAR(s, 0.0, kEps);
    CHECK_NEAR(l, 1.0, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSL(0.0, 0.0, 0.0, &h, &s, &l), 0);
    CHECK_NEAR(l, 0.0, kEps);

    CHECK_INT(ui::ColorConvert::RGB2HSL(0.5, 0.5, 0.5, &h, &s, &l), 0);
    CHECK_NEAR(s, 0.0, kEps);
    CHECK_NEAR(l, 0.5, kEps);

    //越界
    CHECK_INT(ui::ColorConvert::RGB2HSL(1.2, 0.0, 0.0, &h, &s, &l), -1);
    CHECK_INT(ui::ColorConvert::RGB2HSL(0.0, 0.0, 0.0, &h, nullptr, &l), -1);
}

/** HSL -> RGB 的常用取值与边界 */
void TestHSL2RGB()
{
    uint8_t r = 0, g = 0, b = 0;
    CHECK_INT(ui::ColorConvert::HSL2RGB(0.0, 1.0, 0.5, r, g, b), 0);
    CHECK_INT(r, 255);
    CHECK_INT(g, 0);
    CHECK_INT(b, 0);

    CHECK_INT(ui::ColorConvert::HSL2RGB(120.0, 1.0, 0.5, r, g, b), 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 255);
    CHECK_INT(b, 0);

    CHECK_INT(ui::ColorConvert::HSL2RGB(240.0, 1.0, 0.5, r, g, b), 0);
    CHECK_INT(r, 0);
    CHECK_INT(g, 0);
    CHECK_INT(b, 255);

    //饱和度为 0 时为灰度
    CHECK_INT(ui::ColorConvert::HSL2RGB(100.0, 0.0, 0.5, r, g, b), 0);
    CHECK_INT(r, 127);
    CHECK_INT(g, 127);
    CHECK_INT(b, 127);

    //参数校验
    CHECK_INT(ui::ColorConvert::HSL2RGB(-1.0, 1.0, 0.5, r, g, b), -1);
    CHECK_INT(ui::ColorConvert::HSL2RGB(0.0, 1.0, 1.5, r, g, b), -1);
}

/** HSV/HSL 往返转换（HSL 的 hue 会被截断为整度，容差放宽） */
void TestRoundTrip()
{
    double h = 0, s = 0, v = 0;
    CHECK_INT(ui::ColorConvert::RGB2HSV(0.2, 0.6, 0.9, &h, &s, &v), 0);
    uint8_t r = 0, g = 0, b = 0;
    CHECK_INT(ui::ColorConvert::HSV2RGB(h, s, v, r, g, b), 0);
    CHECK_INT(r, 51);
    CHECK_INT(g, 153);
    CHECK_INT(b, 229);

    double h2 = 0, s2 = 0, l2 = 0;
    CHECK_INT(ui::ColorConvert::RGB2HSL(0.2, 0.6, 0.9, &h2, &s2, &l2), 0);
    uint8_t r2 = 0, g2 = 0, b2 = 0;
    CHECK_INT(ui::ColorConvert::HSL2RGB(h2, s2, l2, r2, g2, b2), 0);
    CHECK(std::abs((int)r2 - 51) <= 2);
    CHECK(std::abs((int)g2 - 153) <= 2);
    CHECK(std::abs((int)b2 - 229) <= 2);
}

/** GetRGB：写入数量与端点取值（端点按 DIB 的 BGR 顺序） */
void TestGetRGB()
{
    constexpr uint32_t kSentinel = 0xA5A5A5A5u;
    std::vector<uint32_t> buffer(8, kSentinel);
    ui::ColorConvert::GetRGB(buffer.data(), 2, RgbValue(10, 20, 30), RgbValue(200, 210, 220));
    CHECK_INT(buffer[0], MakeBgr(10, 20, 30));
    CHECK_INT(buffer[1], MakeBgr(200, 210, 220));
    CHECK_INT(buffer[2], kSentinel);

    //samples == 1：只写一个点且不能除零
    std::vector<uint32_t> single(2, kSentinel);
    ui::ColorConvert::GetRGB(single.data(), 1, RgbValue(1, 2, 3), RgbValue(4, 5, 6));
    CHECK_INT(single[0], MakeBgr(1, 2, 3));
    CHECK_INT(single[1], kSentinel);

    //非法参数不写入
    std::vector<uint32_t> none(2, kSentinel);
    ui::ColorConvert::GetRGB(none.data(), 0, RgbValue(1, 2, 3), RgbValue(4, 5, 6));
    CHECK_INT(none[0], kSentinel);
    ui::ColorConvert::GetRGB(nullptr, 4, RgbValue(1, 2, 3), RgbValue(4, 5, 6));
}

/** 检查颜色（BGR 打包）各分量与期望值的偏差在容差内 */
void CheckColorNear(uint32_t actual, uint8_t r, uint8_t g, uint8_t b, int32_t tolerance)
{
    const int32_t aR = static_cast<int32_t>((actual >> 16) & 0xFF);
    const int32_t aG = static_cast<int32_t>((actual >> 8) & 0xFF);
    const int32_t aB = static_cast<int32_t>(actual & 0xFF);
    Check(std::abs(aR - static_cast<int32_t>(r)) <= tolerance,
          [actual, r, g, b]() {
              return "red channel of " + std::to_string(actual) + ", expected " + std::to_string(r);
          });
    Check(std::abs(aG - static_cast<int32_t>(g)) <= tolerance,
          [actual, r, g, b]() {
              return "green channel of " + std::to_string(actual) + ", expected " + std::to_string(g);
          });
    Check(std::abs(aB - static_cast<int32_t>(b)) <= tolerance,
          [actual, r, g, b]() {
              return "blue channel of " + std::to_string(actual) + ", expected " + std::to_string(b);
          });
}

/** HSV/HSL 取色条：写入像素数量必须等于 samples，且起点/终点颜色正确 */
void TestGradientBuffers()
{
    constexpr int32_t kSamples = 60;
    constexpr uint32_t kSentinel = 0xA5A5A5A5u;
    std::vector<uint32_t> buffer(static_cast<size_t>(kSamples) + 2, kSentinel);

    ui::ColorConvert::HSV_HUE(buffer.data(), kSamples, 1.0, 1.0);
    CHECK_INT(buffer[kSamples], kSentinel);      //不能越界写
    CHECK_INT(buffer[0], MakeBgr(255, 0, 0));    //起点为红色
    CheckColorNear(buffer[kSamples - 1], 255, 0, 0, 10); //终点接近 360°，仍接近红色

    std::fill(buffer.begin(), buffer.end(), kSentinel);
    ui::ColorConvert::HSV_SAT(buffer.data(), kSamples, 0.0, 1.0);
    CHECK_INT(buffer[kSamples], kSentinel);
    CHECK_INT(buffer[0], MakeBgr(255, 255, 255));  //饱和度为 0 时从白色开始
    CheckColorNear(buffer[kSamples - 1], 255, 0, 0, 10);

    std::fill(buffer.begin(), buffer.end(), kSentinel);
    ui::ColorConvert::HSV_VAL(buffer.data(), kSamples, 0.0, 1.0);
    CHECK_INT(buffer[kSamples], kSentinel);
    CHECK_INT(buffer[0], MakeBgr(0, 0, 0));      //亮度为 0 时从黑色开始
    CheckColorNear(buffer[kSamples - 1], 255, 0, 0, 10);

    std::fill(buffer.begin(), buffer.end(), kSentinel);
    ui::ColorConvert::HSL_HUE(buffer.data(), kSamples, 1.0, 0.5);
    CHECK_INT(buffer[kSamples], kSentinel);
    CHECK_INT(buffer[0], MakeBgr(255, 0, 0));
    CheckColorNear(buffer[kSamples - 1], 255, 0, 0, 10);

    std::fill(buffer.begin(), buffer.end(), kSentinel);
    ui::ColorConvert::HSL_SAT(buffer.data(), kSamples, 0.0, 0.5);
    CHECK_INT(buffer[kSamples], kSentinel);
    CHECK_INT(buffer[0], MakeBgr(127, 127, 127));
    CheckColorNear(buffer[kSamples - 1], 255, 0, 0, 10);

    std::fill(buffer.begin(), buffer.end(), kSentinel);
    ui::ColorConvert::HSL_LIG(buffer.data(), kSamples, 0.0, 1.0);
    CHECK_INT(buffer[kSamples], kSentinel);
    CHECK_INT(buffer[0], MakeBgr(0, 0, 0));
    CheckColorNear(buffer[kSamples - 1], 255, 255, 255, 10);

    //samples == 1 时只写一个像素，不能除零或死循环
    std::vector<uint32_t> single(2, kSentinel);
    ui::ColorConvert::HSV_HUE(single.data(), 1, 1.0, 1.0);
    CHECK(single[0] != kSentinel);
    CHECK_INT(single[1], kSentinel);

    //samples 非法时不应写入
    std::vector<uint32_t> none(4, kSentinel);
    ui::ColorConvert::HSV_HUE(none.data(), 0, 1.0, 1.0);
    ui::ColorConvert::HSV_SAT(none.data(), -1, 0.0, 1.0);
    ui::ColorConvert::HSL_LIG(none.data(), 0, 0.0, 1.0);
    for (uint32_t value : none) {
        CHECK_INT(value, kSentinel);
    }
}

} // namespace

int main()
{
    TestRGB2HSV();
    TestHSV2RGB();
    TestRGB2HSL();
    TestHSL2RGB();
    TestRoundTrip();
    TestGetRGB();
    TestGradientBuffers();

    std::cout << "color_convert tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
