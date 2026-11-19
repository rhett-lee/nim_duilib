// duilib/Core/UiString.h 与 UiFont 的单元测试
//
// UiString 是控件内部使用的紧凑字符串（内部持有 new[] 缓冲），
// 这里覆盖其构造、拷贝/移动、赋值、比较与清空行为，
// 并验证 string_view 构造不会在内嵌 '\0' 处被截断。
// UiFont 覆盖默认值与比较运算。两者都不依赖窗口环境。

#include "duilib/Core/UiString.h"
#include "duilib/Core/UiFont.h"
#include <functional>
#include <iostream>
#include <string>
#include <utility>

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

/** 宽字符版本的构造与空值语义 */
void TestUiStringWBasics()
{
    ui::UiStringW str;
    CHECK(str.empty());
    CHECK(std::wstring(str.c_str()).empty());

    //从 nullptr 构造：应保持空
    ui::UiStringW nullStr((const wchar_t*)nullptr);
    CHECK(nullStr.empty());

    //从字符串常量 / std::wstring / string_view 构造
    ui::UiStringW fromLiteral(L"hello");
    CHECK(!fromLiteral.empty());
    CHECK(fromLiteral == L"hello");
    CHECK(!(fromLiteral != L"hello"));
    CHECK(fromLiteral.equals(std::wstring(L"hello")));

    std::wstring source(L"world");
    ui::UiStringW fromString(source);
    CHECK(fromString == source);

    std::wstring_view view(L"view");
    ui::UiStringW fromView(view);
    CHECK(fromView == L"view");

    //清空后可再次赋值
    fromView.clear();
    CHECK(fromView.empty());
    fromView = L"again";
    CHECK(fromView == L"again");
}

/** 拷贝构造为深拷贝，源对象销毁/清空不影响副本 */
void TestUiStringWCopy()
{
    std::wstring value(L"deep copy");
    ui::UiStringW src(value);
    ui::UiStringW copy(src);
    CHECK(copy == value);

    src.clear();
    CHECK(src.empty());
    CHECK(copy == value);

    //拷贝赋值
    ui::UiStringW assigned;
    assigned = copy;
    CHECK(assigned == value);

    //自赋值
    assigned = assigned;
    CHECK(assigned == value);

    //空值之间的拷贝与比较
    ui::UiStringW empty1;
    ui::UiStringW empty2(empty1);
    CHECK(empty1 == empty2);
    CHECK(empty1 == L"");
    CHECK(empty1 == std::wstring());
    CHECK(empty1 == std::wstring_view());
    CHECK(!(empty1 != empty2));

    //非空与空值不相等
    CHECK(empty1 != assigned);
    CHECK(assigned != empty1);
}

/** 移动构造/移动赋值：接管内存，源对象变为空 */
void TestUiStringWMove()
{
    ui::UiStringW src(L"move me");
    ui::UiStringW moved(std::move(src));
    CHECK(moved == L"move me");
    CHECK(src.empty());

    ui::UiStringW target;
    target = std::move(moved);
    CHECK(target == L"move me");
    CHECK(moved.empty());

    //自移动赋值不应破坏自身内容
    target = std::move(target);
    CHECK(target == L"move me");
}

/** 各类赋值运算符 */
void TestUiStringWAssign()
{
    ui::UiStringW str;

    str = std::wstring(L"string");
    CHECK(str == L"string");

    str = std::wstring_view(L"view");
    CHECK(str == L"view");

    str = L"literal";
    CHECK(str == L"literal");

    str = (const wchar_t*)nullptr;
    CHECK(str.empty());

    //赋值为空字符串后，再赋非空值
    str = L"";
    CHECK(str.empty());
    str = L"value";
    CHECK(str == L"value");

    //相互比较
    ui::UiStringW other(L"value");
    CHECK(str == other);
    CHECK(str.equals(other));
    CHECK(other == str);
    CHECK(str.equals(L"value"));
    CHECK(str.equals(std::wstring_view(L"value")));
}

/** string_view 中含内嵌 '\0' 时：类内部按 C 字符串语义存储，
 *  复制到 '\0' 为止；同时不能越界读取 string_view 之外的内存 */
void TestEmbeddedNull()
{
    const wchar_t raw[] = { L'a', L'b', L'\0', L'c', L'd' };
    std::wstring_view view(raw, 5);
    ui::UiStringW str(view);
    CHECK(str.c_str()[0] == L'a');
    CHECK(str.c_str()[1] == L'b');
    CHECK(str.c_str()[2] == L'\0');
    CHECK(str == L"ab");

    //赋值路径同样按 C 字符串语义处理
    ui::UiStringW assigned;
    assigned = view;
    CHECK(assigned.c_str()[0] == L'a');
    CHECK(assigned.c_str()[1] == L'b');
    CHECK(assigned.c_str()[2] == L'\0');
    CHECK(assigned == L"ab");

    //view 自身没有 null 结尾时，也不应越界读取
    std::wstring_view unterminated(L"xyz", 3);
    ui::UiStringW fromUnterminated(unterminated);
    CHECK(fromUnterminated == L"xyz");
}

/** ANSI 版本的基本行为 */
void TestUiStringA()
{
    ui::UiStringA str;
    CHECK(str.empty());
    CHECK(std::string(str.c_str()).empty());

    ui::UiStringA fromLiteral("hello");
    CHECK(fromLiteral == "hello");
    CHECK(fromLiteral != "world");

    std::string value("abc");
    ui::UiStringA copy(fromLiteral);
    fromLiteral.clear();
    CHECK(copy == "hello"); //深拷贝：源清空不影响副本

    copy = value;
    CHECK(copy == "abc");

    ui::UiStringA moved(std::move(copy));
    CHECK(moved == "abc");
    CHECK(copy.empty());

    const char raw[] = { 'x', 'y', '\0', 'z' };
    ui::UiStringA embedded(std::string_view(raw, 4));
    CHECK(embedded.c_str()[0] == 'x');
    CHECK(embedded.c_str()[1] == 'y');
    CHECK(embedded.c_str()[2] == '\0');
    CHECK(embedded == "xy");
}

/** UiString 使用 DString 类型，验证与平台字符集一致 */
void TestUiStringDefaultType()
{
    ui::UiString str;
    CHECK(str.empty());

    ui::UiString fromLiteral(_T("default"));
    CHECK(!fromLiteral.empty());
    CHECK(fromLiteral == _T("default"));
    CHECK(fromLiteral == std::basic_string<DString::value_type>(_T("default")));

    fromLiteral.clear();
    CHECK(fromLiteral.empty());
    CHECK(fromLiteral == _T(""));
}

/** UiFont 的默认值与比较运算 */
void TestUiFont()
{
    ui::UiFont font;
    CHECK(font.m_fontName.empty());
    CHECK(font.m_fontSize == 0);
    CHECK(!font.m_bBold);
    CHECK(!font.m_bUnderline);
    CHECK(!font.m_bItalic);
    CHECK(!font.m_bStrikeOut);

    ui::UiFont font2;
    CHECK(font == font2);
    CHECK(!(font != font2));

    font2.m_fontName = ui::UiString(_T("Arial"));
    font2.m_fontSize = 14;
    font2.m_bBold = true;
    CHECK(font != font2);
    CHECK(font2 == font2);

    ui::UiFont font3(font2);
    CHECK(font3 == font2);

    font3.m_bItalic = true;
    CHECK(font3 != font2);

    font3.m_bItalic = false;
    font3.m_bUnderline = true;
    CHECK(font3 != font2);

    font3.m_bUnderline = false;
    font3.m_bStrikeOut = true;
    CHECK(font3 != font2);
}

} // namespace

int main()
{
    TestUiStringWBasics();
    TestUiStringWCopy();
    TestUiStringWMove();
    TestUiStringWAssign();
    TestEmbeddedNull();
    TestUiStringA();
    TestUiStringDefaultType();
    TestUiFont();

    std::cout << "ui string tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
