// duilib/Core 尺寸数值类型与工具函数的单元测试
//
// 覆盖 UiFixedInt（Int32 / Stretch / Auto）、UiEstInt（Int32 / Stretch）、
// 以及 UiTypes.h 中的转换辅助函数（MakeEstInt/MakeEstSize/MakeSize、
// CalcStretchValue、CEILF、IsFloatEqual、Truncate* 等）。
// 全部为纯逻辑代码，不创建窗口。

#include "duilib/Core/UiTypes.h"
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

/** UiFixedInt 的三种类型与取值规则 */
void TestUiFixedInt()
{
    //默认值：整型 0
    ui::UiFixedInt value;
    CHECK(value.IsInt32());
    CHECK(!value.IsStretch());
    CHECK(!value.IsAuto());
    CHECK(value.IsValid());
    CHECK_INT(value.GetInt32(), 0);
    CHECK_INT(value.GetStretchPercentValue(), 0);

    //整型值
    ui::UiFixedInt fixedValue(120);
    CHECK(fixedValue.IsInt32());
    CHECK_INT(fixedValue.GetInt32(), 120);
    CHECK(fixedValue == ui::UiFixedInt::MakeInt(120));
    CHECK(fixedValue != ui::UiFixedInt::MakeInt(121));

    //负值通过构造函数进入，IsValid 为 false，GetInt32 返回 0
    ui::UiFixedInt negative(-5);
    CHECK(!negative.IsValid());
    CHECK_INT(negative.GetInt32(), 0);

    //SetInt32
    ui::UiFixedInt setValue;
    setValue.SetInt32(33);
    CHECK(setValue.IsInt32());
    CHECK_INT(setValue.GetInt32(), 33);

    //拉伸：默认 100%
    ui::UiFixedInt stretch = ui::UiFixedInt::MakeStretch();
    CHECK(stretch.IsStretch());
    CHECK(stretch.IsValid());
    CHECK_INT(stretch.GetStretchPercentValue(), 100);
    CHECK_INT(stretch.GetInt32(), 0);

    //拉伸：指定百分比
    ui::UiFixedInt stretch25 = ui::UiFixedInt::MakeStretch(25);
    CHECK(stretch25.IsStretch());
    CHECK_INT(stretch25.GetStretchPercentValue(), 25);
    CHECK(stretch25 == ui::UiFixedInt::MakeStretch(25));
    CHECK(stretch25 != ui::UiFixedInt::MakeStretch(26));

    //非法百分比会被修正为 100%
    ui::UiFixedInt stretchInvalid;
    stretchInvalid.SetStretch(0);
    CHECK_INT(stretchInvalid.GetStretchPercentValue(), 100);
    stretchInvalid.SetStretch(101);
    CHECK_INT(stretchInvalid.GetStretchPercentValue(), 100);
    stretchInvalid.SetStretch(-10);
    CHECK_INT(stretchInvalid.GetStretchPercentValue(), 100);
    stretchInvalid.SetStretch(100);
    CHECK_INT(stretchInvalid.GetStretchPercentValue(), 100);

    //Auto 类型
    ui::UiFixedInt autoValue = ui::UiFixedInt::MakeAuto();
    CHECK(autoValue.IsAuto());
    CHECK(!autoValue.IsInt32());
    CHECK(autoValue.IsValid());
    CHECK_INT(autoValue.GetInt32(), 0);
    CHECK_INT(autoValue.GetStretchPercentValue(), 0);
    CHECK(autoValue == ui::UiFixedInt::MakeAuto());
    CHECK(autoValue != ui::UiFixedInt::MakeInt(0));
    CHECK(autoValue != ui::UiFixedInt::MakeStretch());

    //不同类型之间不相等
    CHECK(fixedValue != stretch);
    CHECK(stretch != autoValue);
    CHECK(!(fixedValue == stretch));
}

/** UiEstInt 的整型/拉伸取值规则 */
void TestUiEstInt()
{
    ui::UiEstInt value;
    CHECK(value.IsInt32());
    CHECK(!value.IsStretch());
    CHECK(value.IsValid());
    CHECK_INT(value.GetInt32(), 0);
    CHECK_INT(value.GetStretchPercentValue(), 0);

    ui::UiEstInt fixedValue(200);
    CHECK(fixedValue.IsInt32());
    CHECK_INT(fixedValue.GetInt32(), 200);

    ui::UiEstInt negative(-1);
    CHECK(!negative.IsValid());
    CHECK_INT(negative.GetInt32(), 0);

    ui::UiEstInt setValue;
    setValue.SetInt32(16);
    CHECK_INT(setValue.GetInt32(), 16);

    //拉伸：默认 100%
    ui::UiEstInt stretch = ui::UiEstInt::MakeStretch();
    CHECK(stretch.IsStretch());
    CHECK(stretch.IsValid());
    CHECK_INT(stretch.GetStretchPercentValue(), 100);
    CHECK_INT(stretch.GetInt32(), 0);

    ui::UiEstInt stretch40 = ui::UiEstInt::MakeStretch(40);
    CHECK_INT(stretch40.GetStretchPercentValue(), 40);
    CHECK(stretch40 == ui::UiEstInt::MakeStretch(40));
    CHECK(stretch40 != ui::UiEstInt::MakeStretch(41));

    //不同类型之间不相等
    CHECK(fixedValue != stretch);
    CHECK(fixedValue == ui::UiEstInt(200));
    CHECK(stretch != stretch40);
}

/** UiFixedInt -> UiEstInt / UiEstSize 的转换 */
void TestEstConversions()
{
    //Int32
    ui::UiEstInt estInt = ui::MakeEstInt(ui::UiFixedInt::MakeInt(64));
    CHECK(estInt.IsInt32());
    CHECK_INT(estInt.GetInt32(), 64);

    //Stretch
    ui::UiEstInt estStretch = ui::MakeEstInt(ui::UiFixedInt::MakeStretch(75));
    CHECK(estStretch.IsStretch());
    CHECK_INT(estStretch.GetStretchPercentValue(), 75);

    //Auto：转换为整型 0
    ui::UiEstInt estAuto = ui::MakeEstInt(ui::UiFixedInt::MakeAuto());
    CHECK(estAuto.IsInt32());
    CHECK_INT(estAuto.GetInt32(), 0);

    //UiFixedSize -> UiEstSize
    ui::UiFixedSize fixedSize;
    fixedSize.cx = ui::UiFixedInt::MakeInt(100);
    fixedSize.cy = ui::UiFixedInt::MakeStretch(50);
    ui::UiEstSize estSize = ui::MakeEstSize(fixedSize);
    CHECK_INT(estSize.cx.GetInt32(), 100);
    CHECK(estSize.cy.IsStretch());
    CHECK_INT(estSize.cy.GetStretchPercentValue(), 50);

    //UiSize -> UiEstSize -> UiSize
    ui::UiEstSize fromSize = ui::MakeEstSize(ui::UiSize(300, 400));
    CHECK_INT(fromSize.cx.GetInt32(), 300);
    CHECK_INT(fromSize.cy.GetInt32(), 400);
    ui::UiSize size = ui::MakeSize(fromSize);
    CHECK(size == ui::UiSize(300, 400));
}

/** 百分比长度计算 */
void TestCalcStretchValue()
{
    //UiEstInt 版本
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(50), 200), 100);
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(25), 100), 25);
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(33), 100), 33);  //四舍五入
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(1), 100), 1);
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(), 123), 123);   //100% 返回原值
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt(40), 123), 123);              //非拉伸返回原值
    CHECK_INT(ui::CalcStretchValue(ui::UiEstInt::MakeStretch(50), 0), 0);

    //UiFixedInt 版本
    CHECK_INT(ui::CalcStretchValue(ui::UiFixedInt::MakeStretch(50), 200), 100);
    CHECK_INT(ui::CalcStretchValue(ui::UiFixedInt::MakeStretch(10), 55), 6);  //5.5 四舍五入为 6
    CHECK_INT(ui::CalcStretchValue(ui::UiFixedInt::MakeStretch(), 77), 77);
    CHECK_INT(ui::CalcStretchValue(ui::UiFixedInt::MakeAuto(), 77), 77);
    CHECK_INT(ui::CalcStretchValue(ui::UiFixedInt::MakeInt(20), 77), 77);
}

/** 其它数值辅助函数 */
void TestNumericHelpers()
{
    //CEILF：正数向上取整，负数向负无穷取整
    CHECK(ui::CEILF(1.2f) == 2.0f);
    CHECK(ui::CEILF(1.0f) == 1.0f);
    CHECK(ui::CEILF(0.0f) == 0.0f);
    CHECK(ui::CEILF(-1.2f) == -2.0f);
    CHECK(ui::CEILF(-1.0f) == -1.0f);

    //浮点比较
    CHECK(ui::IsFloatEqual(1.0f, 1.0f));
    CHECK(ui::IsFloatEqual(1.0f, 1.000001f));
    CHECK(!ui::IsFloatEqual(1.0f, 1.001f));

    //Truncate* 在合法范围内应保持数值不变
    CHECK_INT(ui::TruncateToInt32(12345), 12345);
    CHECK_INT(ui::TruncateToInt32(INT32_MAX), INT32_MAX);
    CHECK_INT(ui::TruncateToInt32(INT32_MIN), INT32_MIN);
    CHECK_INT(ui::TruncateToUInt16((uint32_t)100), 100);
    CHECK_INT(ui::TruncateToUInt16((uint32_t)UINT16_MAX), UINT16_MAX);
    CHECK_INT(ui::TruncateToUInt16((int32_t)100), 100);
    CHECK_INT(ui::TruncateToInt16(INT16_MAX), INT16_MAX);
    CHECK_INT(ui::TruncateToInt16(0), 0);
    CHECK_INT(ui::TruncateToInt8(INT8_MIN), INT8_MIN);
    CHECK_INT(ui::TruncateToInt8(INT8_MAX), INT8_MAX);
    CHECK_INT(ui::TruncateToUInt8(0), 0);
    CHECK_INT(ui::TruncateToUInt8(UINT8_MAX), UINT8_MAX);
}

} // namespace

int main()
{
    TestUiFixedInt();
    TestUiEstInt();
    TestEstConversions();
    TestCalcStretchValue();
    TestNumericHelpers();

    std::cout << "ui int types tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
