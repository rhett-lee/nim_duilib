// Calendar 每周禁用星期（disabled_weekdays）的单元测试
//
// Calendar::SetDisabledWeekdays 的属性解析逻辑集中在 CalendarWeekdayUtil 中，
// 本测试只覆盖纯逻辑接口，不创建窗口/控件对象，可在任意平台直接运行。

#include "duilib/Control/CalendarWeekdayUtil.h"
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

/** 检查掩码只禁用了期望的星期（0=周日 ... 6=周六） */
void CheckDisabledWeekdays(uint32_t weekdayMask, const bool expected[7])
{
    for (int32_t weekday = 0; weekday < 7; ++weekday) {
        const bool actual = ui::IsCalendarWeekdayDisabled(weekdayMask, weekday);
        Check(actual == expected[weekday],
              [weekday, expected, actual]() {
                  return "weekday " + std::to_string(weekday) +
                         ": expected " + (expected[weekday] ? "disabled" : "enabled") +
                         ", actual " + (actual ? "disabled" : "enabled");
              });
    }
}

/** 空配置：不应禁用任何星期 */
void TestEmpty()
{
    const bool disabled[7] = {false, false, false, false, false, false, false};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("")), disabled);
}

/** 完整星期名："saturday,sunday"（周末） */
void TestFullNames()
{
    const bool disabled[7] = {true, false, false, false, false, false, true};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("saturday,sunday")), disabled);
}

/** 缩写、大小写与多种分隔符 */
void TestAliasesAndSeparators()
{
    const bool disabled[7] = {true, false, false, false, false, false, true};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("SAT;SUN")), disabled);
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("  saturday | sunday  ")), disabled);
}

/** 数字写法：0=周日, 6=周六 */
void TestNumericValues()
{
    const bool disabled[7] = {true, false, false, false, false, false, true};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("0,6")), disabled);

    const bool monday[7] = {false, true, false, false, false, false, false};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("1")), monday);
}

/** 非法项被忽略，不能影响合法项 */
void TestInvalidTokensIgnored()
{
    const bool disabled[7] = {false, true, false, false, false, false, false};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("monday,notaday,9,-1,")), disabled);
}

/** 重复项不应产生额外影响 */
void TestDuplicates()
{
    const bool disabled[7] = {true, false, false, false, false, false, false};
    CheckDisabledWeekdays(ui::ParseDisabledWeekdays(_T("sunday,sun,0,SUN")), disabled);
}

/** 星期取值越界时视为未禁用 */
void TestInvalidWeekdayIndex()
{
    const uint32_t mask = ui::ParseDisabledWeekdays(_T("saturday,sunday"));
    CHECK(!ui::IsCalendarWeekdayDisabled(mask, -1));
    CHECK(!ui::IsCalendarWeekdayDisabled(mask, 7));
    CHECK(ui::IsCalendarWeekdayDisabled(mask, 6));
}

} // namespace

int main()
{
    TestEmpty();
    TestFullNames();
    TestAliasesAndSeparators();
    TestNumericValues();
    TestInvalidTokensIgnored();
    TestDuplicates();
    TestInvalidWeekdayIndex();

    std::cout << "calendar disabled_weekdays tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
