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
    std::list<std::string> tokens = ui::StringUtil::Split("a,b,,c", ",");
    CHECK_INT((int)tokens.size(), 3);
    if (tokens.size() == 3) {
        auto it = tokens.begin();
        CHECK(*it++ == "a");
        CHECK(*it++ == "b");
        CHECK(*it++ == "c");
    }

    // 分隔符集合中的任意字符都作为分隔符
    tokens = ui::StringUtil::Split("a;b|c", ";|");
    CHECK_INT((int)tokens.size(), 3);

    // 空字符串与全分隔符都返回空列表
    CHECK(ui::StringUtil::Split(std::string(), ",").empty());
    CHECK(ui::StringUtil::Split(",,,", ",").empty());

    std::list<std::wstring> wideTokens = ui::StringUtil::Split(L"x y z", L" ");
    CHECK_INT((int)wideTokens.size(), 3);
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
    CHECK(ui::StringUtil::IsValueTrue(DString(_T("1"))));
    CHECK(!ui::StringUtil::IsValueTrue(DString(_T("false"))));
    CHECK(!ui::StringUtil::IsValueTrue(DString(_T("TRUE"))));
    CHECK(!ui::StringUtil::IsValueTrue(DString()));
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

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}
