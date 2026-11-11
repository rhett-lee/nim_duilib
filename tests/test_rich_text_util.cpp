// RichText 纯逻辑的单元测试
//
// RichTextImpl/WindowBuilder 中与界面无关的文本处理逻辑集中在 RichTextUtil 中，
// 本测试只覆盖这些纯函数，不创建窗口/控件对象，可在任意平台直接运行：
// XML 转义、对齐属性解析、标签校验、块级标签判断、链接地址归一化与提示判断、
// 空白裁剪策略、跨文本节点的空格处理、选中文本的换行符归一化。

#include "duilib/Control/RichTextUtil.h"

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

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 比较字符串是否相等，失败时打印用例名 */
void CheckText(const DString& actual, const DString& expected, const char* caseName)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << caseName << ": actual size=" << actual.size()
                  << ", expected size=" << expected.size() << std::endl;
    }
}

/** XML 文本/属性转义 */
void TestEscapeXml()
{
    CheckText(ui::EscapeRichTextXmlText(_T("")), _T(""), "escape text: empty");
    CheckText(ui::EscapeRichTextXmlText(_T("abc 中文")), _T("abc 中文"), "escape text: plain");
    CheckText(ui::EscapeRichTextXmlText(_T("a&b<c>d")), _T("a&amp;b&lt;c&gt;d"), "escape text: special");
    //文本内容不转义引号（只有属性值需要）
    CheckText(ui::EscapeRichTextXmlText(_T("\"x\"")), _T("\"x\""), "escape text: quote kept");

    CheckText(ui::EscapeRichTextXmlAttribute(_T("")), _T(""), "escape attr: empty");
    CheckText(ui::EscapeRichTextXmlAttribute(_T("a&b<c>d\"e\"")),
              _T("a&amp;b&lt;c&gt;d&quot;e&quot;"), "escape attr: special");
}

/** 逗号分隔的属性值匹配 */
void TestContainsAlignValue()
{
    Check(ui::ContainsRichTextAlignValue(_T("left"), _T("left")), "contains: single");
    Check(ui::ContainsRichTextAlignValue(_T("left,vcenter"), _T("vcenter")), "contains: second item");
    Check(ui::ContainsRichTextAlignValue(_T(" left , right "), _T("right")), "contains: trim item");
    Check(ui::ContainsRichTextAlignValue(_T("center"), _T("hcenter")) == false,
          "contains: center is not hcenter");
    Check(ui::ContainsRichTextAlignValue(_T(""), _T("left")) == false, "contains: empty text");
    Check(ui::ContainsRichTextAlignValue(_T("left"), _T("lef")) == false, "contains: partial match");
    Check(ui::ContainsRichTextAlignValue(_T("left"), nullptr) == false, "contains: null value");
}

/** text_align 属性解析 */
void TestParseTextAlign()
{
    ui::RichTextTextAlign alignValue = ui::ParseRichTextTextAlign(_T(""));
    Check((alignValue.m_nHAlign == -1) && (alignValue.m_nVAlign == -1) && !alignValue.m_bJustify,
          "text align: empty");

    alignValue = ui::ParseRichTextTextAlign(_T("left"));
    Check((alignValue.m_nHAlign == 0) && (alignValue.m_nVAlign == -1) && !alignValue.m_bJustify,
          "text align: left");

    //"center"是"hcenter"的别名
    alignValue = ui::ParseRichTextTextAlign(_T("center"));
    Check((alignValue.m_nHAlign == 1) && (alignValue.m_nVAlign == -1), "text align: center alias");

    alignValue = ui::ParseRichTextTextAlign(_T("right,bottom"));
    Check((alignValue.m_nHAlign == 2) && (alignValue.m_nVAlign == 2) && !alignValue.m_bJustify,
          "text align: right bottom");

    alignValue = ui::ParseRichTextTextAlign(_T("justify"));
    Check((alignValue.m_nHAlign == -1) && alignValue.m_bJustify, "text align: justify");

    //多个取值同时出现时，按 left > hcenter > right 的优先级取值
    alignValue = ui::ParseRichTextTextAlign(_T("right,left"));
    Check(alignValue.m_nHAlign == 0, "text align: left has priority");

    alignValue = ui::ParseRichTextTextAlign(_T("top,vcenter"));
    Check(alignValue.m_nVAlign == 0, "text align: top has priority");

    alignValue = ui::ParseRichTextTextAlign(_T(" vcenter , justify "));
    Check((alignValue.m_nVAlign == 1) && alignValue.m_bJustify, "text align: trim and justify");
}

/** 标签名称与标签集合的校验 */
void TestSupportedTags()
{
    Check(ui::IsSupportedRichTextTagName(_T("a")), "tag name: a");
    Check(ui::IsSupportedRichTextTagName(_T("B")), "tag name: uppercase");
    Check(ui::IsSupportedRichTextTagName(_T("h6")), "tag name: h6");
    Check(ui::IsSupportedRichTextTagName(_T("strike")), "tag name: strike");
    Check(ui::IsSupportedRichTextTagName(_T("div")) == false, "tag name: unsupported");
    Check(ui::IsSupportedRichTextTagName(_T("")) == false, "tag name: empty");
    Check(ui::IsSupportedRichTextTagName(_T("p ")) == false, "tag name: trailing space");

    Check(ui::IsSupportedRichTextTags(_T("")), "tags: empty text");
    Check(ui::IsSupportedRichTextTags(_T("plain text")), "tags: plain text");
    Check(ui::IsSupportedRichTextTags(_T("<b>bold</b>")), "tags: pair");
    Check(ui::IsSupportedRichTextTags(_T("<a href=\"https://x.com\">x</a>")), "tags: with attribute");
    Check(ui::IsSupportedRichTextTags(_T("<DIV>x</DIV>")) == false, "tags: unsupported");
    //只校验标签名称，不校验标签是否闭合
    Check(ui::IsSupportedRichTextTags(_T("<b>unclosed")), "tags: unclosed tag name is valid");
    Check(ui::IsSupportedRichTextTags(_T("<b>unclosed<")) == false, "tags: broken tag");
    Check(ui::IsSupportedRichTextTags(_T("a < b")) == false, "tags: less than sign");
}

/** 块级标签判断 */
void TestBlockTags()
{
    Check(ui::IsRichTextBlockTagName(_T("p")), "block: p");
    Check(ui::IsRichTextBlockTagName(_T("h1")), "block: h1");
    Check(ui::IsRichTextBlockTagName(_T("h6")), "block: h6");
    Check(ui::IsRichTextBlockTagName(_T("ul")), "block: ul");
    Check(ui::IsRichTextBlockTagName(_T("li")), "block: li");
    Check(ui::IsRichTextBlockTagName(_T("hr")), "block: hr");
    Check(ui::IsRichTextBlockTagName(_T("b")) == false, "block: b");
    Check(ui::IsRichTextBlockTagName(_T("")) == false, "block: empty");
}

/** 可见文本判断 */
void TestVisibleText()
{
    Check(ui::HasRichTextVisibleText(_T("")) == false, "visible: empty");
    Check(ui::HasRichTextVisibleText(_T(" \t\r\n")) == false, "visible: whitespace");
    Check(ui::HasRichTextVisibleText(_T(" a ")), "visible: text");
    Check(ui::HasRichTextVisibleText(_T("\n\t x")), "visible: text after whitespace");
}

/** 段落对齐属性解析 */
void TestHAlignValue()
{
    Check(ui::ParseRichTextHAlignValue(_T("left")) == 0, "h align: left");
    Check(ui::ParseRichTextHAlignValue(_T("start")) == 0, "h align: start");
    Check(ui::ParseRichTextHAlignValue(_T("center")) == 1, "h align: center");
    Check(ui::ParseRichTextHAlignValue(_T("right")) == 2, "h align: right");
    Check(ui::ParseRichTextHAlignValue(_T("end")) == 2, "h align: end");
    Check(ui::ParseRichTextHAlignValue(_T("justify")) == 3, "h align: justify");
    Check(ui::ParseRichTextHAlignValue(_T("")) == -1, "h align: empty");
    Check(ui::ParseRichTextHAlignValue(_T("middle")) == -1, "h align: unknown");
    Check(ui::ParseRichTextHAlignValue(_T(" center ")) == 1, "h align: trim");
}

/** 链接地址归一化 */
void TestNormalizeLinkUrl()
{
    CheckText(ui::NormalizeRichTextLinkUrl(_T("")), _T(""), "link url: empty");
    CheckText(ui::NormalizeRichTextLinkUrl(_T("https://www.Baidu.com/")), _T("baidu.com"),
              "link url: https with www");
    CheckText(ui::NormalizeRichTextLinkUrl(_T("http://a.com")), _T("a.com"), "link url: http");
    CheckText(ui::NormalizeRichTextLinkUrl(_T("www.x.com//")), _T("x.com"), "link url: trailing slashes");
    CheckText(ui::NormalizeRichTextLinkUrl(_T("  Baidu.COM  ")), _T("baidu.com"), "link url: trim and lower");
    CheckText(ui::NormalizeRichTextLinkUrl(_T("//")), _T(""), "link url: only slashes");
}

/** 链接提示是否需要的判断 */
void TestLinkToolTip()
{
    //显示文本与URL一致：不需要提示
    Check(ui::IsRichTextLinkUrlDifferentFromText(L"www.baidu.com", _T("https://www.baidu.com/")) == false,
          "tooltip: text equals url");
    Check(ui::IsRichTextLinkUrlDifferentFromText(L" http://A.com/ ", _T("a.com")) == false,
          "tooltip: text equals url after normalize");

    //显示文本与URL不同：需要提示
    Check(ui::IsRichTextLinkUrlDifferentFromText(L"Baidu", _T("www.baidu.com")),
          "tooltip: text differs from url");
    Check(ui::IsRichTextLinkUrlDifferentFromText(L"点击这里", _T("https://x.com/a")),
          "tooltip: chinese text");

    //任一为空：不需要提示
    Check(ui::IsRichTextLinkUrlDifferentFromText(L"", _T("www.baidu.com")) == false, "tooltip: empty text");
    Check(ui::IsRichTextLinkUrlDifferentFromText(L"text", _T("")) == false, "tooltip: empty url");
}

/** 单文本片段的空白裁剪 */
void TestTrimRichText()
{
    //不处理：原样返回
    CheckText(ui::TrimRichText(_T(" a  b "), ui::RichTextTrimPolicy::kNone), _T(" a  b "),
              "trim none: keep all");
    CheckText(ui::TrimRichText(_T("a\tb"), ui::RichTextTrimPolicy::kNone), _T("a\tb"),
              "trim none: keep tab");

    //kAll：词间空格原样保留，只保留首尾各一个空格（用于跨节点的空格拼接）
    CheckText(ui::TrimRichText(_T("  hello   world  "), ui::RichTextTrimPolicy::kAll),
              _T(" hello   world "), "trim all: inner spaces kept");
    CheckText(ui::TrimRichText(_T("\r\n\thello\r\n"), ui::RichTextTrimPolicy::kAll),
              _T(" hello "), "trim all: line breaks to spaces");
    CheckText(ui::TrimRichText(_T("   "), ui::RichTextTrimPolicy::kAll), _T(" "),
              "trim all: only spaces");
    CheckText(ui::TrimRichText(_T(""), ui::RichTextTrimPolicy::kAll), _T(""), "trim all: empty");
    CheckText(ui::TrimRichText(_T("hello"), ui::RichTextTrimPolicy::kAll), _T("hello"),
              "trim all: no space");

    //kKeepOne：连续空白压缩为一个空格
    CheckText(ui::TrimRichText(_T(" a  b "), ui::RichTextTrimPolicy::kKeepOne), _T(" a b "),
              "trim keep one: compress");
    CheckText(ui::TrimRichText(_T("a\t \tb"), ui::RichTextTrimPolicy::kKeepOne), _T("a b"),
              "trim keep one: tab compress");
}

/** 跨文本节点的空格处理 */
void TestTrimRichTextSlices()
{
    {
        //整段文本首尾的空格去掉，词间的空格保留
        std::vector<std::wstring_view> textViews = {L"  Hello ", L"world  "};
        ui::TrimRichTextSlices(textViews);
        CheckText(DString(textViews[0].data(), textViews[0].size()), _T("Hello "),
                  "slices: first keeps inner space");
        CheckText(DString(textViews[1].data(), textViews[1].size()), _T("world"),
                  "slices: last trims trailing");
    }
    {
        //中间的空白片段不产生多余空格
        std::vector<std::wstring_view> textViews = {L"  a  ", L"  b  ", L"  c  "};
        ui::TrimRichTextSlices(textViews);
        CheckText(DString(textViews[0].data(), textViews[0].size()), _T("a "), "slices: a");
        CheckText(DString(textViews[1].data(), textViews[1].size()), _T("b "), "slices: b");
        CheckText(DString(textViews[2].data(), textViews[2].size()), _T("c"), "slices: c");
    }
    {
        //换行片段之后的行首空格需要去掉
        std::vector<std::wstring_view> textViews = {L"Hello", L"\n", L"  World  "};
        ui::TrimRichTextSlices(textViews);
        CheckText(DString(textViews[0].data(), textViews[0].size()), _T("Hello"),
                  "slices: before newline");
        CheckText(DString(textViews[1].data(), textViews[1].size()), _T("\n"),
                  "slices: newline kept");
        CheckText(DString(textViews[2].data(), textViews[2].size()), _T("World"),
                  "slices: line start trimmed");
    }
    {
        //空片段不影响判断，且不能越界
        std::vector<std::wstring_view> textViews = {L"", L" x ", L"", L"", L" y ", L""};
        ui::TrimRichTextSlices(textViews);
        CheckText(DString(textViews[1].data(), textViews[1].size()), _T("x "),
                  "slices: empty items skipped 1");
        CheckText(DString(textViews[4].data(), textViews[4].size()), _T("y"),
                  "slices: empty items skipped 2");
        Check(textViews[0].empty() && textViews[2].empty() && textViews[3].empty() && textViews[5].empty(),
              "slices: empty items stay empty");
    }
    {
        //全部为空白时，结果全部为空
        std::vector<std::wstring_view> textViews = {L"   ", L"  "};
        ui::TrimRichTextSlices(textViews);
        Check(textViews[0].empty() && textViews[1].empty(), "slices: all whitespace");
    }
    {
        //空列表不崩溃
        std::vector<std::wstring_view> textViews;
        ui::TrimRichTextSlices(textViews);
        Check(textViews.empty(), "slices: empty list");
    }
}

/** 选中文本的换行符归一化 */
void TestNormalizeSelectedText()
{
    Check(ui::NormalizeRichTextSelectedText(L"abc") == L"abc", "selected: plain");
    Check(ui::NormalizeRichTextSelectedText(L"a\nb") == L"a\r\nb", "selected: lf to crlf");
    Check(ui::NormalizeRichTextSelectedText(L"a\r\nb") == L"a\r\nb", "selected: crlf kept");
    Check(ui::NormalizeRichTextSelectedText(L"a\rb") == L"ab", "selected: cr removed");
    Check(ui::NormalizeRichTextSelectedText(L"") == L"", "selected: empty");
    Check(ui::NormalizeRichTextSelectedText(L"\n\n") == L"\r\n\r\n", "selected: two lines");
}

}//namespace

int main()
{
    TestEscapeXml();
    TestContainsAlignValue();
    TestParseTextAlign();
    TestSupportedTags();
    TestBlockTags();
    TestVisibleText();
    TestHAlignValue();
    TestNormalizeLinkUrl();
    TestLinkToolTip();
    TestTrimRichText();
    TestTrimRichTextSlices();
    TestNormalizeSelectedText();

    std::cout << "rich text util tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
