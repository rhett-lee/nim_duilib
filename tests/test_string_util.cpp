// duilib/Utils/StringUtil 的单元测试
//
// 覆盖字符串替换、大小写转换、裁剪、分割、比较、数值与字符串互转、
// 字符串复制（含超长截断）以及格式化输出（含超过栈缓冲区长度的场景）。
// 全部为纯逻辑，不创建窗口、不加载资源。

#include "duilib/Utils/StringUtil.h"
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

/** 将宽字符串转换为UTF-8（仅用于构造失败时的输出信息） */
std::string ToUtf8(const std::wstring& text)
{
    std::string result;
    for (wchar_t ch : text) {
        if (ch < 0x80) {
            result.push_back(static_cast<char>(ch));
        }
        else {
            result.push_back('?');
        }
    }
    return result;
}

#define CHECK_WSTR(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected) + ": expected \"" + \
               ToUtf8(expected) + "\", actual \"" + ToUtf8(actual) + "\""; \
    })

void TestReplaceAll()
{
    std::string text = "hello world";
    CHECK_INT(ui::StringUtil::ReplaceAll("o", "0", text), 2);
    CHECK(text == "hell0 w0rld");

    // 替换后长度不变
    text = "a.b.c";
    CHECK_INT(ui::StringUtil::ReplaceAll(".", "/", text), 2);
    CHECK(text == "a/b/c");

    // 替换后长度变长（走扩容分支）
    text = "a-a-a";
    CHECK_INT(ui::StringUtil::ReplaceAll("a", "abc", text), 3);
    Check(text == "abc-abc-abc", [&]() { return "text=[" + text + "]"; });

    // 替换后长度变短
    text = "aaaa";
    CHECK_INT(ui::StringUtil::ReplaceAll("aa", "a", text), 2);
    CHECK(text == "aa");

    // 未找到、空查找串、空输出
    text = "abc";
    CHECK_INT(ui::StringUtil::ReplaceAll("z", "y", text), 0);
    CHECK(text == "abc");
    CHECK_INT(ui::StringUtil::ReplaceAll("", "y", text), 0);
    std::string emptyText;
    CHECK_INT(ui::StringUtil::ReplaceAll("a", "b", emptyText), 0);
}

void TestCaseConvert()
{
    std::wstring text = L"AbC-def";
    ui::StringUtil::LowerString(text);
    CHECK_WSTR(text, L"abc-def");
    ui::StringUtil::UpperString(text);
    CHECK_WSTR(text, L"ABC-DEF");

    CHECK_WSTR(ui::StringUtil::MakeLowerString(std::wstring(L"AbC")), L"abc");
    CHECK_WSTR(ui::StringUtil::MakeUpperString(std::wstring(L"AbC")), L"ABC");
    CHECK(ui::StringUtil::MakeLowerString(std::string("AbC")) == "abc");
    CHECK(ui::StringUtil::MakeUpperString(std::string("AbC")) == "ABC");

    // 非ASCII字符不应被修改
    CHECK_WSTR(ui::StringUtil::MakeLowerString(std::wstring(L"中A")), L"中a");
}

void TestTrim()
{
    CHECK(ui::StringUtil::TrimLeft(" \t abc") == "abc");
    CHECK(ui::StringUtil::TrimRight("abc \r\n") == "abc");
    CHECK(ui::StringUtil::Trim("  abc  ") == "abc");
    CHECK(ui::StringUtil::Trim("   ") == "");
    CHECK(ui::StringUtil::Trim("abc") == "abc");

    std::wstring text = L"\t 中文 \r\n";
    ui::StringUtil::Trim(text);
    CHECK_WSTR(text, L"中文");

    std::wstring left = L"  x";
    ui::StringUtil::TrimLeft(left);
    CHECK_WSTR(left, L"x");
    std::wstring right = L"x  ";
    ui::StringUtil::TrimRight(right);
    CHECK_WSTR(right, L"x");
}

void TestSplit()
{
    std::vector<std::string> tokens = ui::StringUtil::Split("a,b,,c", ",");
    CHECK_INT((int)tokens.size(), 3);
    if (tokens.size() == 3) {
        CHECK(tokens[0] == "a");
        CHECK(tokens[1] == "b");
        CHECK(tokens[2] == "c");
    }

    // 分隔符集合中的任意字符都作为分隔符
    tokens = ui::StringUtil::Split("a;b|c", ";|");
    CHECK_INT((int)tokens.size(), 3);

    // 空字符串与全分隔符都返回空列表
    CHECK(ui::StringUtil::Split(std::string(), ",").empty());
    CHECK(ui::StringUtil::Split(",,,", ",").empty());

    std::vector<std::wstring> wideTokens = ui::StringUtil::Split(L"x y z", L" ");
    CHECK_INT((int)wideTokens.size(), 3);

    // keepEmpty = true：保留连续分隔符及首尾分隔符产生的空项
    std::vector<std::string> keepAll = ui::StringUtil::Split("a,,b,", ",", true);
    CHECK_INT((int)keepAll.size(), 4);
    if (keepAll.size() == 4) {
        CHECK(keepAll[0] == "a");
        CHECK(keepAll[1].empty());
        CHECK(keepAll[2] == "b");
        CHECK(keepAll[3].empty());
    }
    CHECK_INT((int)ui::StringUtil::Split(",", ",", true).size(), 2);
    CHECK_INT((int)ui::StringUtil::Split(std::string(), ",", true).size(), 1);
    CHECK(ui::StringUtil::Split(std::string(), ",", true)[0].empty());
    CHECK_INT((int)ui::StringUtil::Split(std::wstring(L"a,,b"), std::wstring(L","), true).size(), 3);

    // 空分隔符：整体作为一个字段（与原实现一致）
    std::vector<std::string> whole = ui::StringUtil::Split("abc", "");
    CHECK_INT((int)whole.size(), 1);
    CHECK(whole[0] == "abc");

    // 内嵌'\0'：只处理其前面的内容（与原strtok实现一致）
    const std::string withNul("a\0b", 3);
    std::vector<std::string> nulTokens = ui::StringUtil::Split(withNul, ",");
    CHECK_INT((int)nulTokens.size(), 1);
    CHECK(nulTokens[0] == "a");
}

void TestCompare()
{
    CHECK(ui::StringUtil::IsEqualNoCase(std::wstring(L"AbC"), std::wstring(L"aBc")));
    CHECK(ui::StringUtil::IsEqualNoCase(L"abc", std::wstring(L"ABC")));
    CHECK(ui::StringUtil::IsEqualNoCase(std::wstring(L"abc"), L"ABC"));
    CHECK(ui::StringUtil::IsEqualNoCase(std::string("AbC"), std::string("aBc")));
    CHECK(!ui::StringUtil::IsEqualNoCase(std::wstring(L"abc"), std::wstring(L"abcd")));
    CHECK(!ui::StringUtil::IsEqualNoCase(L"abc", L"abd"));

    // 空指针语义：都为nullptr相等；只有一个为nullptr时不相等
    CHECK(ui::StringUtil::IsEqualNoCase(static_cast<const wchar_t*>(nullptr),
                                        static_cast<const wchar_t*>(nullptr)));
    CHECK(!ui::StringUtil::IsEqualNoCase(static_cast<const wchar_t*>(nullptr), L"abc"));
    CHECK(!ui::StringUtil::IsEqualNoCase(L"abc", static_cast<const wchar_t*>(nullptr)));
    CHECK(!ui::StringUtil::IsEqualNoCase(static_cast<const char*>(nullptr), "abc"));
    CHECK(!ui::StringUtil::IsEqualNoCase("abc", static_cast<const char*>(nullptr)));

    CHECK_INT(ui::StringUtil::StringCompare(std::wstring(L"abc"), std::wstring(L"abc")), 0);
    CHECK(ui::StringUtil::StringCompare(std::wstring(L"abd"), std::wstring(L"abc")) > 0);
    CHECK(ui::StringUtil::StringCompare(std::wstring(L"abc"), std::wstring(L"abd")) < 0);
    CHECK(ui::StringUtil::StringICompare(std::wstring(L"ABC"), std::wstring(L"abc")) == 0);
    CHECK(ui::StringUtil::StringCompare(std::string("a"), std::string("b")) < 0);

    // 空指针比较：nullptr 小于非空字符串
    CHECK_INT(ui::StringUtil::StringCompare(static_cast<const wchar_t*>(nullptr),
                                            static_cast<const wchar_t*>(nullptr)), 0);
    CHECK(ui::StringUtil::StringCompare(static_cast<const wchar_t*>(nullptr), L"a") < 0);
    CHECK(ui::StringUtil::StringCompare(L"a", static_cast<const wchar_t*>(nullptr)) > 0);
}

void TestNumberConvert()
{
    CHECK_WSTR(ui::StringUtil::UInt64ToStringW(0), L"0");
    CHECK_WSTR(ui::StringUtil::UInt64ToStringW(18446744073709551615ULL), L"18446744073709551615");
    CHECK_WSTR(ui::StringUtil::UInt32ToStringW(4294967295U), L"4294967295");
    CHECK(ui::StringUtil::UInt64ToStringA(123456789ULL) == "123456789");
    CHECK_WSTR(ui::StringUtil::Int64ToStringW(-123), L"-123");
    CHECK_WSTR(ui::StringUtil::Int64ToStringW(INT64_MIN), L"-9223372036854775808");
    CHECK(ui::StringUtil::Int32ToStringA(-42) == "-42");

    CHECK_INT(ui::StringUtil::StringToInt32(std::wstring(L"42x")), 42);
    CHECK_INT(ui::StringUtil::StringToInt32(std::string("-7")), -7);
    CHECK_INT(ui::StringUtil::StringToInt32(std::string("abc")), 0);
    CHECK(ui::StringUtil::StringToInt64(std::wstring(L"-9223372036854775808")) == INT64_MIN);
    CHECK(ui::StringUtil::StringToInt64(std::string("1234567890123")) == 1234567890123LL);

    const double value = ui::StringUtil::StringToDouble(std::string("3.25"));
    CHECK(value > 3.24 && value < 3.26);
    CHECK(ui::StringUtil::StringToFloat("1.5") > 1.49f);

    // 多进制解析
    wchar_t* pEnd = nullptr;
    const wchar_t* hexText = L"0x1F";
    CHECK_INT(ui::StringUtil::StringToUInt32(hexText, &pEnd, 0), 31);
}

void TestCopy()
{
    // 源字符串超长时应截断，并保证以'\0'结尾
    wchar_t wideDest[4] = { L'x', L'x', L'x', L'x' };
    ui::StringUtil::StringCopy(wideDest, 4, L"abcdef");
    CHECK_WSTR(std::wstring(wideDest), L"abc");

    wchar_t wideShort[8] = { 0 };
    ui::StringUtil::StringCopy(wideShort, 8, L"abc");
    CHECK_WSTR(std::wstring(wideShort), L"abc");

    char dest[4] = { 'x', 'x', 'x', 'x' };
    ui::StringUtil::StringCopy(dest, 4, "abcdef");
    CHECK(std::string(dest) == "abc");

    // StringNCopy 只拷贝指定数量的字符
    wchar_t nDest[8] = { 0 };
    ui::StringUtil::StringNCopy(nDest, 8, L"abcdefgh", 3);
    CHECK_WSTR(std::wstring(nDest), L"abc");

    char nDestA[4] = { 0 };
    ui::StringUtil::StringNCopy(nDestA, 4, "abcdefgh", 8);
    CHECK(std::string(nDestA) == "abc");

    CHECK_INT((int)ui::StringUtil::StringLen(static_cast<const wchar_t*>(nullptr)), 0);
    CHECK_INT((int)ui::StringUtil::StringLen(L"abc"), 3);
    CHECK_INT((int)ui::StringUtil::StringLen("abcd"), 4);

    // 目标为空、源为空时应安全返回
    CHECK_INT(ui::StringUtil::StringCopy(static_cast<wchar_t*>(nullptr), 0, L"a"), 0);
    wchar_t emptyDest[2] = { L'x', L'x' };
    CHECK_INT(ui::StringUtil::StringCopy(emptyDest, 2, L""), 0);
    CHECK_WSTR(std::wstring(emptyDest), L"");
}

void TestFormat()
{
    std::cout << "[STEP] Format short" << std::endl;
    CHECK_WSTR(ui::StringUtil::Printf(L"%d-%s", 5, L"x"), L"5-x");
    CHECK(ui::StringUtil::Printf("%s/%d", "abc", 7) == "abc/7");

    // 超过栈缓冲区(1024字节)的格式化结果应完整返回
    std::cout << "[STEP] Format long wide" << std::endl;
    const std::wstring longText(2000, L'a');
    const std::wstring result = ui::StringUtil::Printf(L"%s", longText.c_str());
    CHECK_INT((int)result.size(), 2000);
    CHECK(result == longText);

    std::cout << "[STEP] Format long ansi" << std::endl;
    const std::string longTextA(2000, 'a');
    const std::string resultA = ui::StringUtil::Printf("%s", longTextA.c_str());
    CHECK_INT((int)resultA.size(), 2000);

    // 空格式串
    std::cout << "[STEP] Format empty" << std::endl;
    CHECK_WSTR(ui::StringUtil::Printf(L"%s", L""), L"");
}

void TestIsValueTrue()
{
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("true"))));
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("TRUE"))));
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("1"))));
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("yes"))));
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("on"))));
    CHECK(ui::StringUtil::IsValueTrue(DString(_T(" 1 "))));
    CHECK(!ui::StringUtil::IsValueTrue(DString(_T("false"))));
    CHECK(!ui::StringUtil::IsValueTrue(DString(_T("off"))));
    CHECK(!ui::StringUtil::IsValueTrue(DString()));
}

void TestP0Fixes()
{
    // Trim 系列的空指针安全性
    CHECK(ui::StringUtil::Trim(static_cast<const char*>(nullptr)).empty());
    CHECK(ui::StringUtil::TrimLeft(static_cast<const char*>(nullptr)).empty());
    CHECK(ui::StringUtil::TrimRight(static_cast<const char*>(nullptr)).empty());
    CHECK(ui::StringUtil::Trim(static_cast<const wchar_t*>(nullptr)).empty());

    // IsEqualNoCase：nullptr 等价于空字符串
    CHECK(ui::StringUtil::IsEqualNoCase(static_cast<const wchar_t*>(nullptr), std::wstring()));
    CHECK(ui::StringUtil::IsEqualNoCase(std::wstring(), static_cast<const wchar_t*>(nullptr)));
    CHECK(ui::StringUtil::IsEqualNoCase(static_cast<const char*>(nullptr), std::string()));
    CHECK(ui::StringUtil::IsEqualNoCase(std::string(), static_cast<const char*>(nullptr)));
    CHECK(!ui::StringUtil::IsEqualNoCase(static_cast<const wchar_t*>(nullptr), std::wstring(L"a")));

    // 数值解析的成功标志：仅接受完整合法的数值
    bool ok = false;
    const int32_t int32Value = ui::StringUtil::StringToInt32(std::wstring(L" 42 "), &ok);
    CHECK_INT(int32Value, 42);
    CHECK(ok);

    ok = true;
    const int32_t badInt32 = ui::StringUtil::StringToInt32(std::string("42x"), &ok);
    CHECK_INT(badInt32, 0);
    CHECK(!ok);

    ok = true;
    ui::StringUtil::StringToInt32(std::wstring(), &ok);
    CHECK(!ok);

    ok = false;
    const int64_t int64Value = ui::StringUtil::StringToInt64(std::string("9223372036854775807"), &ok);
    CHECK_INT(int64Value, INT64_MAX);
    CHECK(ok);

    ok = true;
    ui::StringUtil::StringToInt64(std::string("9223372036854775808"), &ok);
    CHECK(!ok);

    ok = false;
    const double doubleValue = ui::StringUtil::StringToDouble(std::wstring(L" 3.5 "), &ok);
    CHECK(ok);
    CHECK(doubleValue > 3.49 && doubleValue < 3.51);

    ok = true;
    ui::StringUtil::StringToDouble(std::string("3.5x"), &ok);
    CHECK(!ok);

    // StringToDouble 的 pEndPtr 版本
    char* pEnd = nullptr;
    const double parsedDouble = ui::StringUtil::StringToDouble("1.25abc", &pEnd);
    CHECK(parsedDouble > 1.24 && parsedDouble < 1.26);
    CHECK((pEnd != nullptr) && (*pEnd == 'a'));

    // StringToBool
    CHECK(ui::StringUtil::StringToBool(std::wstring(L" Yes ")));
    CHECK(!ui::StringUtil::StringToBool(std::string("OFF")));
    CHECK(ui::StringUtil::StringToBool(std::string("unknown"), true));
    CHECK(ui::StringUtil::StringToBool(std::wstring(), true));
    CHECK(!ui::StringUtil::StringToBool(std::wstring(), false));
}

void TestStartsWithEndsWith()
{
    CHECK(ui::StringUtil::StartsWith(std::wstring(L"abcdef"), std::wstring(L"abc")));
    CHECK(!ui::StringUtil::StartsWith(std::wstring(L"abcdef"), std::wstring(L"abd")));
    CHECK(ui::StringUtil::StartsWith(std::wstring(L"abc"), std::wstring()));
    CHECK(ui::StringUtil::StartsWithNoCase(std::string("ABCdef"), std::string("abc")));
    CHECK(!ui::StringUtil::StartsWithNoCase(std::string("ABC"), std::string("abcd")));

    CHECK(ui::StringUtil::EndsWith(std::wstring(L"abcdef"), std::wstring(L"def")));
    CHECK(!ui::StringUtil::EndsWith(std::wstring(L"abcdef"), std::wstring(L"de")));
    CHECK(ui::StringUtil::EndsWithNoCase(std::string("abcDEF"), std::string("def")));
    CHECK(ui::StringUtil::EndsWith(std::wstring(L"abc"), std::wstring()));

    CHECK(ui::StringUtil::ContainsNoCase(std::string("Hello World"), std::string("WORLD")));
    CHECK(!ui::StringUtil::ContainsNoCase(std::wstring(L"Hello"), std::wstring(L"xyz")));
    CHECK(ui::StringUtil::ContainsNoCase(std::wstring(L"abc"), std::wstring()));
}

void TestEmptyAndWhiteSpace()
{
    CHECK(ui::StringUtil::IsNullOrEmpty(static_cast<const char*>(nullptr)));
    CHECK(ui::StringUtil::IsNullOrEmpty(std::wstring()));
    CHECK(!ui::StringUtil::IsNullOrEmpty(std::string(" ")));
    CHECK(ui::StringUtil::IsNullOrWhiteSpace(std::string(" \t\r\n")));
    CHECK(ui::StringUtil::IsNullOrWhiteSpace(static_cast<const wchar_t*>(nullptr)));
    CHECK(!ui::StringUtil::IsNullOrWhiteSpace(std::wstring(L" x ")));
}

void TestSplitJoinAndLines()
{
    // SplitByString：多字符分隔符与空项控制
    std::vector<std::string> parts = ui::StringUtil::SplitByString(std::string("a||b||||c"), std::string("||"), false);
    CHECK_INT((int)parts.size(), 3);
    if (parts.size() == 3) {
        CHECK(parts[0] == "a");
        CHECK(parts[1] == "b");
        CHECK(parts[2] == "c");
    }

    parts = ui::StringUtil::SplitByString(std::string("a,b,,"), std::string(","), true);
    CHECK_INT((int)parts.size(), 4);
    if (parts.size() == 4) {
        CHECK(parts[3].empty());
    }

    parts = ui::StringUtil::SplitByString(std::string("abc"), std::string(), false);
    CHECK_INT((int)parts.size(), 1);

    // Join
    std::vector<std::wstring> wideParts;
    wideParts.push_back(L"a");
    wideParts.push_back(L"b");
    wideParts.push_back(L"c");
    CHECK_WSTR(ui::StringUtil::Join(wideParts, std::wstring(L"-")), L"a-b-c");
    CHECK(ui::StringUtil::Join(std::vector<std::string>(), std::string(",")).empty());

    // SplitLines：支持 \r\n、\n、\r，且不保留末尾换行产生的空行
    std::vector<std::wstring> lines = ui::StringUtil::SplitLines(std::wstring(L"a\r\nb\nc\rd\n"));
    CHECK_INT((int)lines.size(), 4);
    if (lines.size() == 4) {
        CHECK_WSTR(lines[0], L"a");
        CHECK_WSTR(lines[1], L"b");
        CHECK_WSTR(lines[2], L"c");
        CHECK_WSTR(lines[3], L"d");
    }

    lines = ui::StringUtil::SplitLines(std::wstring(L"a\n\nb"));
    CHECK_INT((int)lines.size(), 3);
    if (lines.size() == 3) {
        CHECK(lines[1].empty());
    }
    CHECK(ui::StringUtil::SplitLines(std::wstring()).empty());
}

void TestMakeReplaceAll()
{
    CHECK_WSTR(ui::StringUtil::MakeReplaceAll(std::wstring(L"a-b-a"), std::wstring(L"a"), std::wstring(L"xy")), L"xy-b-xy");
    CHECK(ui::StringUtil::MakeReplaceAll(std::string("abc"), std::string("z"), std::string("y")) == "abc");
    CHECK(ui::StringUtil::MakeReplaceAll(std::string("abc"), std::string(), std::string("y")) == "abc");
    CHECK_WSTR(ui::StringUtil::MakeReplaceAllNoCase(std::wstring(L"AbcABC"), std::wstring(L"aBc"), std::wstring(L"x")), L"xx");
    CHECK(ui::StringUtil::MakeReplaceAllNoCase(std::string("aAa"), std::string("A"), std::string("b")) == "bbb");
}

void TestBoolAndDoubleToString()
{
    CHECK(ui::StringUtil::BoolToString(true) == DString(_T("true")));
    CHECK(ui::StringUtil::BoolToString(false) == DString(_T("false")));

    CHECK(ui::StringUtil::DoubleToString(3.14159, 2) == DString(_T("3.14")));
    CHECK(ui::StringUtil::DoubleToString(3.14159, 0) == DString(_T("3")));
    CHECK(ui::StringUtil::DoubleToString(-1.5, 1) == DString(_T("-1.5")));

    // 精度越界时按边界值处理：负精度按0位小数，超过17位按17位小数
    CHECK(ui::StringUtil::DoubleToString(1.5, -1) == DString(_T("2")));
    CHECK(ui::StringUtil::DoubleToString(1.0, 100) == DString(_T("1.00000000000000000")));
}

void TestUnicodeWhiteSpace()
{
    // 宽字符版本的 Trim 需要处理全角空格(U+3000)与不换行空格(U+00A0)
    std::wstring text = L"\u3000 abc \u00A0";
    ui::StringUtil::Trim(text);
    CHECK_WSTR(text, L"abc");

    CHECK(ui::StringUtil::IsNullOrWhiteSpace(std::wstring(L"\u3000\u00A0")));
    CHECK(!ui::StringUtil::IsNullOrWhiteSpace(std::wstring(L"\u3000x\u00A0")));
}

} // namespace

int main()
{
    TestReplaceAll();
    TestCaseConvert();
    TestTrim();
    TestSplit();
    TestCompare();
    TestNumberConvert();
    TestCopy();
    TestFormat();
    TestIsValueTrue();
    TestP0Fixes();
    TestStartsWithEndsWith();
    TestEmptyAndWhiteSpace();
    TestSplitJoinAndLines();
    TestMakeReplaceAll();
    TestBoolAndDoubleToString();
    TestUnicodeWhiteSpace();

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}
