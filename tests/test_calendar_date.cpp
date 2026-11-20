// duilib/Control/Calendar 日期工具函数的单元测试
//
// 覆盖 Calendar 的静态日期工具：日期比较、范围判断、日期字符串解析/格式化、
// time_t 与 struct tm 的互转。这些函数不依赖窗口和渲染，可直接运行。

#include "duilib/Control/Calendar.h"
#include <ctime>
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

/** 构造一个日期（tm_year 为相对 1900 的年份） */
struct tm MakeDate(int32_t year, int32_t month, int32_t day,
                   int32_t hour = 0, int32_t minute = 0, int32_t second = 0)
{
    struct tm date = {0, };
    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_hour = hour;
    date.tm_min = minute;
    date.tm_sec = second;
    return date;
}

/** 同一天的判断只比较年月日 */
void TestIsSameDay()
{
    CHECK(ui::Calendar::IsSameDay(MakeDate(2024, 2, 29, 0, 0, 0),
                                  MakeDate(2024, 2, 29, 23, 59, 59)));
    CHECK(!ui::Calendar::IsSameDay(MakeDate(2024, 2, 28, 12, 0, 0),
                                   MakeDate(2024, 2, 29, 12, 0, 0)));
    CHECK(!ui::Calendar::IsSameDay(MakeDate(2023, 2, 28), MakeDate(2024, 2, 28)));
    CHECK(!ui::Calendar::IsSameDay(MakeDate(2024, 1, 31), MakeDate(2024, 2, 1)));
}

/** 日期先后比较 */
void TestIsDateLess()
{
    CHECK(ui::Calendar::IsDateLess(MakeDate(2023, 12, 31), MakeDate(2024, 1, 1)));
    CHECK(ui::Calendar::IsDateLess(MakeDate(2024, 1, 31), MakeDate(2024, 2, 1)));
    CHECK(ui::Calendar::IsDateLess(MakeDate(2024, 2, 28), MakeDate(2024, 2, 29)));
    CHECK(!ui::Calendar::IsDateLess(MakeDate(2024, 2, 29), MakeDate(2024, 2, 29)));
    CHECK(!ui::Calendar::IsDateLess(MakeDate(2024, 3, 1), MakeDate(2024, 2, 29)));
    //仅时分秒不同时，视为同一天
    CHECK(!ui::Calendar::IsDateLess(MakeDate(2024, 6, 1, 23, 59, 59),
                                    MakeDate(2024, 6, 1, 0, 0, 0)));
}

/** 日期范围判断（闭区间） */
void TestIsDateInRange()
{
    const struct tm start = MakeDate(2024, 1, 1);
    const struct tm end = MakeDate(2024, 12, 31);

    CHECK(ui::Calendar::IsDateInRange(MakeDate(2024, 1, 1), start, end));
    CHECK(ui::Calendar::IsDateInRange(MakeDate(2024, 12, 31), start, end));
    CHECK(ui::Calendar::IsDateInRange(MakeDate(2024, 7, 15), start, end));
    CHECK(!ui::Calendar::IsDateInRange(MakeDate(2023, 12, 31), start, end));
    CHECK(!ui::Calendar::IsDateInRange(MakeDate(2025, 1, 1), start, end));

    //起止日期相同，仅当天在范围内
    CHECK(ui::Calendar::IsDateInRange(MakeDate(2024, 5, 20, 18, 0, 0),
                                      MakeDate(2024, 5, 20),
                                      MakeDate(2024, 5, 20)));
    CHECK(!ui::Calendar::IsDateInRange(MakeDate(2024, 5, 21), MakeDate(2024, 5, 20), MakeDate(2024, 5, 20)));
}

/** time_t 与 struct tm 互转（代码固定使用当天 12:00，避免时区边界问题） */
void TestTimeRoundTrip()
{
    const int32_t dates[][3] = {
        {2024, 2, 29},
        {2000, 1, 1},
        {2038, 1, 19},
        {1999, 12, 31},
    };
    for (const auto& item : dates) {
        const struct tm source = MakeDate(item[0], item[1], item[2]);
        const time_t value = ui::Calendar::DateToTimeT(source);
        CHECK(value != static_cast<time_t>(-1));
        const struct tm result = ui::Calendar::TimeTToDate(value);
        Check(ui::Calendar::IsSameDay(source, result), [&item]() {
            return "round trip failed for " + std::to_string(item[0]) + "-" +
                   std::to_string(item[1]) + "-" + std::to_string(item[2]);
        });
    }
}

/** 日期字符串解析 */
void TestParseDateString()
{
    struct tm date = {0, };
    CHECK(ui::Calendar::ParseDateString(_T("2024-02-29"), date));
    CHECK(date.tm_year == 124);
    CHECK(date.tm_mon == 1);
    CHECK(date.tm_mday == 29);

    //支持斜杠分隔，且不要求前导零
    CHECK(ui::Calendar::ParseDateString(_T("2024/3/5"), date));
    CHECK(date.tm_year == 124);
    CHECK(date.tm_mon == 2);
    CHECK(date.tm_mday == 5);

    //边界年份
    CHECK(ui::Calendar::ParseDateString(_T("1900-01-01"), date));
    CHECK(date.tm_year == 0);
    CHECK(ui::Calendar::ParseDateString(_T("9999-12-31"), date));
    CHECK(date.tm_year == 8099);

    //非法输入不改变既有值
    const struct tm unchanged = MakeDate(2020, 6, 15);
    struct tm target = unchanged;
    CHECK(!ui::Calendar::ParseDateString(_T(""), target));
    CHECK(!ui::Calendar::ParseDateString(_T("abc"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("2024-1"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("2024-13-01"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("2024-00-10"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("2024-06-00"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("2024-06-32"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("1899-12-31"), target));
    CHECK(!ui::Calendar::ParseDateString(_T("10000-01-01"), target));
    CHECK(ui::Calendar::IsSameDay(target, unchanged));
}

/** 日期格式化 */
void TestFormatDateString()
{
    CHECK(ui::Calendar::FormatDateString(MakeDate(2024, 2, 29)) == _T("2024-02-29"));
    CHECK(ui::Calendar::FormatDateString(MakeDate(2024, 1, 1)) == _T("2024-01-01"));
    CHECK(ui::Calendar::FormatDateString(MakeDate(9999, 12, 31)) == _T("9999-12-31"));

    //解析与格式化往返一致
    struct tm date = {0, };
    CHECK(ui::Calendar::ParseDateString(_T("2024/7/8"), date));
    CHECK(ui::Calendar::FormatDateString(date) == _T("2024-07-08"));
}

} // namespace

int main()
{
    TestIsSameDay();
    TestIsDateLess();
    TestIsDateInRange();
    TestTimeRoundTrip();
    TestParseDateString();
    TestFormatDateString();

    std::cout << "calendar_date tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
