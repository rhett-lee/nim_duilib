// duilib/Core/DpiManager 的单元测试
//
// DpiManager 的初始化接口依赖全局管理器与窗口系统，
// 这里只覆盖构造后的默认状态、无缩放（100%）下的数值换算、
// 缩放变化判断与静态 MulDiv 工具函数，可在命令行直接运行。

#include "duilib/Core/DpiManager.h"
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

/** 构造后的默认状态：100% 缩放、非用户自定义 DPI */
void TestDefaults()
{
    ui::DpiManager dpi;
    CHECK_INT(dpi.GetDisplayScaleFactor(), 100);
    CHECK(dpi.GetDisplayScale() == 1.0f);
    CHECK(!dpi.IsDisplayScaled());
    CHECK(!dpi.IsUserDefinedDpi());
    CHECK(!dpi.HasPixelDensity());
    CHECK(dpi.GetPixelDensity() == 1.0f);
    CHECK(dpi.CheckDisplayScaleFactor(100));
}

/** 缩放变化判断 */
void TestDisplayScaleChanged()
{
    ui::DpiManager dpi;
    CHECK(!dpi.IsDisplayScaleChanged(1.0f, 1.0f));
    CHECK(dpi.IsDisplayScaleChanged(1.25f, 1.0f));
    CHECK(dpi.IsDisplayScaleChanged(1.5f, 1.0f));
    CHECK(dpi.IsDisplayScaleChanged(2.0f, 1.0f));

    //无效的像素密度会按 1.0f 处理，不产生变化
    CHECK(!dpi.IsDisplayScaleChanged(1.0f, 0.5f));
    CHECK(!dpi.IsDisplayScaleChanged(1.0f, 0.0f));
}

/** 无缩放时，所有尺寸换算应保持原值 */
void TestScaleAt100Percent()
{
    ui::DpiManager dpi;

    //整数
    int32_t nValue = 42;
    dpi.ScaleInt(nValue);
    CHECK_INT(nValue, 42);
    CHECK_INT(dpi.GetScaleInt(42), 42);
    dpi.UnscaleInt(nValue);
    CHECK_INT(nValue, 42);
    CHECK_INT(dpi.GetUnscaleInt(42), 42);

    uint32_t uValue = 77;
    dpi.ScaleInt(uValue);
    CHECK_INT(uValue, 77);
    CHECK_INT(dpi.GetScaleInt((uint32_t)77), 77);

    //UiSize
    ui::UiSize size(10, 20);
    dpi.ScaleSize(size);
    CHECK(size == ui::UiSize(10, 20));
    CHECK(dpi.GetScaleSize(ui::UiSize(30, 40)) == ui::UiSize(30, 40));

    //UiPoint
    ui::UiPoint point(3, 4);
    dpi.ScalePoint(point);
    CHECK(point == ui::UiPoint(3, 4));

    //UiRect
    ui::UiRect rect(1, 2, 3, 4);
    dpi.ScaleRect(rect);
    CHECK(rect == ui::UiRect(1, 2, 3, 4));

    //UiPadding / UiMargin
    ui::UiPadding padding(1, 2, 3, 4);
    dpi.ScalePadding(padding);
    CHECK(padding == ui::UiPadding(1, 2, 3, 4));
    ui::UiMargin margin(5, 6, 7, 8);
    dpi.ScaleMargin(margin);
    CHECK(margin == ui::UiMargin(5, 6, 7, 8));

    //窗口尺寸相关换算
    CHECK_INT(dpi.GetScaleWindowSize(100), 100);
    CHECK_INT(dpi.GetUnscaleWindowSize(100), 100);
    CHECK(dpi.GetScaleWindowSize(ui::UiSize(10, 20)) == ui::UiSize(10, 20));
    CHECK(dpi.GetUnscaleWindowSize(ui::UiSize(10, 20)) == ui::UiSize(10, 20));
}

/** MulDiv：四舍五入、负数、0 分母与无符号版本 */
void TestMulDiv()
{
    //常规四舍五入
    CHECK_INT(ui::DpiManager::MulDiv(100, 125, 100), 125);
    CHECK_INT(ui::DpiManager::MulDiv(5, 2, 3), 3);       //3.33 -> 3
    CHECK_INT(ui::DpiManager::MulDiv(7, 3, 2), 11);      //10.5 -> 11
    CHECK_INT(ui::DpiManager::MulDiv(1, 1, 2), 1);       //0.5 -> 1
    CHECK_INT(ui::DpiManager::MulDiv(1, 1, 3), 0);       //0.33 -> 0
    CHECK_INT(ui::DpiManager::MulDiv(10, 0, 1), 0);

    //负数：std::round 远离 0 取整
    CHECK_INT(ui::DpiManager::MulDiv(-3, 125, 100), -4); //-3.75 -> -4
    CHECK_INT(ui::DpiManager::MulDiv(-1, 1, 2), -1);     //-0.5 -> -1
    CHECK_INT(ui::DpiManager::MulDiv(-10, 0, 1), 0);

    //分母为 0：有符号版本返回 -1
    CHECK_INT(ui::DpiManager::MulDiv(10, 10, 0), -1);

    //大数值不溢出（内部使用 64 位中间结果）
    CHECK_INT(ui::DpiManager::MulDiv(INT32_MAX, 1, 1), INT32_MAX);
    CHECK_INT(ui::DpiManager::MulDiv(1000000, 300, 100), 3000000);

    //无符号版本
    CHECK_INT(ui::DpiManager::MulDiv((uint32_t)100, (uint32_t)125, (uint32_t)100), 125u);
    CHECK_INT(ui::DpiManager::MulDiv((uint32_t)1, (uint32_t)1, (uint32_t)2), 1u);
    CHECK_INT(ui::DpiManager::MulDiv((uint32_t)7, (uint32_t)3, (uint32_t)2), 11u);
    //无符号版本分母为 0 时返回原值
    CHECK_INT(ui::DpiManager::MulDiv((uint32_t)10, (uint32_t)10, (uint32_t)0), 10u);
}

} // namespace

int main()
{
    TestDefaults();
    TestDisplayScaleChanged();
    TestScaleAt100Percent();
    TestMulDiv();

    std::cout << "dpi manager tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
