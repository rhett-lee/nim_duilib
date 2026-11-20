// duilib/Control/TextDrawer 纯文本转换的单元测试
//
// ToPlainText 只跳过 "<...>" 形式的富文本标记，其余字符原样保留；
// 若 "<" 之后没有 ">"，则按普通字符处理。该函数为纯字符串处理，不依赖窗口。

#include "duilib/Control/TextDrawer.h"
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

void CheckPlainText(const DString& richText, const DString& expected)
{
    const DString actual = ui::TextDrawer::ToPlainText(richText);
    Check(actual == expected, [&richText, &expected, &actual]() {
        return "ToPlainText mismatch: input size " + std::to_string(richText.size()) +
               ", expected size " + std::to_string(expected.size()) +
               ", actual size " + std::to_string(actual.size());
    });
}

/** 无标记的文本原样返回 */
void TestPlainText()
{
    CheckPlainText(_T("hello"), _T("hello"));
    CheckPlainText(_T(""), _T(""));
    CheckPlainText(_T("中文 with English 123"), _T("中文 with English 123"));
}

/** 成对与嵌套的标记被去除 */
void TestTags()
{
    CheckPlainText(_T("<b>bold</b>"), _T("bold"));
    CheckPlainText(_T("<b><i>nested</i></b>"), _T("nested"));
    CheckPlainText(_T("<p align='left'>text</p>"), _T("text"));
    CheckPlainText(_T("<b></b>"), _T(""));
    CheckPlainText(_T("a<br/>b"), _T("ab"));
    CheckPlainText(_T("x<link url='http://a'>y</link>"), _T("xy"));
}

/** "<" 之后没有 ">" 时按普通字符处理 */
void TestUnclosedTag()
{
    CheckPlainText(_T("a < b"), _T("a < b"));
    CheckPlainText(_T("<b unclosed"), _T("<b unclosed"));
    CheckPlainText(_T("1 < 2"), _T("1 < 2"));
    CheckPlainText(_T("<"), _T("<"));
    CheckPlainText(_T("a<b"), _T("a<b"));
}

/** 多个标记之间的文本与空白保持原样 */
void TestMixedContent()
{
    CheckPlainText(_T("<b>a</b> b <i>c</i>"), _T("a b c"));
    CheckPlainText(_T("  <font color='red'>red</font>  "), _T("  red  "));
    //"<" 与 ">" 之间存在内容时按标记整体去除
    CheckPlainText(_T("1 < 2 > 3"), _T("1  3"));
}

} // namespace

int main()
{
    TestPlainText();
    TestTags();
    TestUnclosedTag();
    TestMixedContent();

    std::cout << "text_drawer tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
