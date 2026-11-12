// RichText 控件的单元测试
//
// 测试对象：ui::RichText（RichTextT<Control>）及其派生类型 RichTextHBox/RichTextVBox。
// 本测试不创建窗口、不加载主题资源，只验证与界面无关的属性、文本解析与序列化逻辑，
// 因此可以在命令行（含 CI）直接运行。
//
// 覆盖内容：
//  1. 默认属性值（对齐方式、行间距、内边距、字体、换行、透明度、文本选择等）；
//  2. text_align / textalign / font / text_color / text_padding / 行间距 / 段间距 /
//     word_wrap / trim_policy / enable_text_select 等属性的解析；
//  3. 格式化文本的设置、追加、清空与 ToString 序列化（纯文本、粗体、超链接、根标签）；
//  4. text 属性的花括号替换（{ } -> < >）；
//  5. TrimText 的不同裁剪策略；
//  6. 链接与文本选择相关接口在未布局时的安全行为；
//  7. 空文本/无窗口时的尺寸估算。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Control/RichText.h"

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

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 判断字符串中是否包含指定的子串 */
bool Contains(const DString& text, const DString& subText)
{
    return text.find(subText) != DString::npos;
}

/** 判断内边距是否与指定值相等 */
bool IsPaddingEqual(const ui::UiPadding& padding, int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    return (padding.left == left) && (padding.top == top) && (padding.right == right) && (padding.bottom == bottom);
}

/** 默认属性值 */
void TestDefaults()
{
    ui::RichText richText(nullptr);
    CHECK(richText.GetType() == DUI_CTR_RICHTEXT);
    CHECK(richText.GetRichTextImpl() != nullptr);

    //默认水平靠左、垂直居中（与Label保持一致）
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignLeft);
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignCenter);

    CHECK(richText.IsWordWrap());
    CHECK(richText.GetRowSpacingMul() == 1.0f);
    CHECK(richText.GetRowSpacingAdd() == 0.0f);
    CHECK(IsPaddingEqual(richText.GetTextPadding(), 0, 0, 0, 0));
    CHECK(richText.GetFontId().empty());
    CHECK(richText.GetTextColor().empty());

    CHECK(richText.IsEnableRedraw());
    CHECK(richText.GetPaintAlpha() == 255);
    CHECK(richText.IsEnableTextSelect());

    CHECK(!richText.HasSelection());
    CHECK(richText.GetSelectedText().empty());
    CHECK(richText.GetHoveredLinkIndex() == -1);
    CHECK(richText.GetFocusedLinkIndex() == -1);
    CHECK(richText.GetRichTextImpl()->GetParagraphSpacing() == 0);

    //空文本的序列化结果
    CHECK(richText.ToString() == _T("<RichText>\r\n</RichText>"));

    //无窗口时无法估算尺寸，返回0
    const ui::UiSize size = richText.EstimateText(ui::UiSize(100, 100));
    CHECK((size.cx == 0) && (size.cy == 0));

    //链接数量与查询接口的默认行为
    CHECK(richText.GetLinkCount() == 0);
    CHECK(richText.GetLinkUrl(0).empty());
    CHECK(richText.GetLinkTitle(0).empty());
    ui::UiRect rcLink;
    CHECK(!richText.GetLinkRect(0, rcLink));

    //派生类型
    ui::RichTextHBox richTextHBox(nullptr);
    CHECK(richTextHBox.GetType() == DUI_CTR_RICHTEXT_HBOX);
    ui::RichTextVBox richTextVBox(nullptr);
    CHECK(richTextVBox.GetType() == DUI_CTR_RICHTEXT_VBOX);
    ui::RichTextBox richTextBox(nullptr);
    CHECK(richTextBox.GetType() == DUI_CTR_RICHTEXT_BOX);
}

/** text_align 与 textalign 属性 */
void TestTextAlign()
{
    ui::RichText richText(nullptr);

    richText.SetAttribute(_T("text_align"), _T("left"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignLeft);

    richText.SetAttribute(_T("text_align"), _T("center"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);

    richText.SetAttribute(_T("text_align"), _T("right"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignRight);

    //垂直对齐方式
    richText.SetAttribute(_T("text_align"), _T("top"));
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignTop);
    richText.SetAttribute(_T("text_align"), _T("vcenter"));
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignCenter);
    richText.SetAttribute(_T("text_align"), _T("bottom"));
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignBottom);

    //水平与垂直同时设置
    richText.SetAttribute(_T("text_align"), _T("right,bottom"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignRight);
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignBottom);

    richText.SetAttribute(_T("text_align"), _T("hcenter,top"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignTop);

    //justify：两端对齐（不属于枚举值，仅验证不改变水平枚举值）
    richText.SetAttribute(_T("text_align"), _T("justify"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);

    //空字符串不改变现有对齐方式
    richText.SetAttribute(_T("text_align"), _T(""));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);

    //textalign兼容别名
    richText.SetAttribute(_T("textalign"), _T("center"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);
    richText.SetAttribute(_T("textalign"), _T("end"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignRight);
    richText.SetAttribute(_T("textalign"), _T("start"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignLeft);
    richText.SetAttribute(_T("textalign"), _T("justify"));
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignLeft);

    //SetTextVAlignType/SetTextHAlignType
    richText.SetTextHAlignType(ui::HorAlignType::kAlignCenter);
    CHECK(richText.GetHAlignType() == ui::HorAlignType::kAlignCenter);
    richText.SetTextVAlignType(ui::VerAlignType::kAlignTop);
    CHECK(richText.GetVAlignType() == ui::VerAlignType::kAlignTop);
}

/** 字体、颜色、内边距、行间距、段间距等属性 */
void TestAttributes()
{
    ui::RichText richText(nullptr);

    richText.SetFontId(_T("font_test"));
    CHECK(richText.GetFontId() == _T("font_test"));
    richText.SetAttribute(_T("font"), _T("font_attr"));
    CHECK(richText.GetFontId() == _T("font_attr"));

    richText.SetTextColor(_T("#FF102030"));
    CHECK(richText.GetTextColor() == _T("#FF102030"));
    richText.SetAttribute(_T("text_color"), _T("#FF203040"));
    CHECK(richText.GetTextColor() == _T("#FF203040"));
    richText.SetAttribute(_T("normal_text_color"), _T("#FF304050"));
    CHECK(richText.GetTextColor() == _T("#FF304050"));

    //内边距（DPI为100%时保持不变）
    richText.SetAttribute(_T("text_padding"), _T("2,4,6,8"));
    CHECK(IsPaddingEqual(richText.GetTextPadding(), 2, 4, 6, 8));
    richText.SetAttribute(_T("textpadding"), _T("1,1,1,1"));
    CHECK(IsPaddingEqual(richText.GetTextPadding(), 1, 1, 1, 1));
    richText.SetTextPadding(ui::UiPadding(3, 3, 3, 3), false);
    CHECK(IsPaddingEqual(richText.GetTextPadding(), 3, 3, 3, 3));

    //行间距：小于等于0.01的倍数被修正为1.0
    richText.SetRowSpacingMul(2.0f);
    CHECK(richText.GetRowSpacingMul() == 2.0f);
    richText.SetAttribute(_T("row_spacing_mul"), _T("1.5"));
    CHECK(richText.GetRowSpacingMul() == 1.5f);
    richText.SetRowSpacingMul(0.0f);
    CHECK(richText.GetRowSpacingMul() == 1.0f);

    richText.SetRowSpacingAdd(6.0f);
    CHECK(richText.GetRowSpacingAdd() == 6.0f);
    richText.SetAttribute(_T("row_spacing_add"), _T("4"));
    CHECK(richText.GetRowSpacingAdd() == 4.0f);
    richText.SetRowSpacingAdd(-5.0f);
    CHECK(richText.GetRowSpacingAdd() == 0.0f);

    //段间距：负数被修正为0
    richText.SetAttribute(_T("paragraph_spacing"), _T("12"));
    CHECK(richText.GetRichTextImpl()->GetParagraphSpacing() == 12);
    richText.SetAttribute(_T("paragraph_spacing"), _T("-3"));
    CHECK(richText.GetRichTextImpl()->GetParagraphSpacing() == 0);

    //自动换行
    CHECK(richText.IsWordWrap());
    richText.SetAttribute(_T("word_wrap"), _T("false"));
    CHECK(!richText.IsWordWrap());
    richText.SetWordWrap(true);
    CHECK(richText.IsWordWrap());

    //文本选择开关
    CHECK(richText.IsEnableTextSelect());
    richText.SetAttribute(_T("enable_text_select"), _T("false"));
    CHECK(!richText.IsEnableTextSelect());
    richText.SetAttribute(_T("text_select"), _T("true"));
    CHECK(richText.IsEnableTextSelect());
    richText.SetEnableTextSelect(false);
    CHECK(!richText.IsEnableTextSelect());

    //重绘开关与绘制透明度
    richText.SetEnableRedraw(false);
    CHECK(!richText.IsEnableRedraw());
    richText.SetEnableRedraw(true);
    CHECK(richText.IsEnableRedraw());

    richText.SetPaintAlpha(128);
    CHECK(richText.GetPaintAlpha() == 128);
    richText.SetPaintAlpha(0);
    CHECK(richText.GetPaintAlpha() == 0);

    //链接颜色设置接口（仅验证可安全调用）
    richText.SetLinkColor(_T("#FF111111"), _T("#FF222222"), _T("#FF333333"), _T("#FF444444"));
    CHECK(richText.GetLinkCount() == 0);

    //字间距/首行缩进/最大行数/溢出方式等属性（无对应读取接口，验证可安全设置）
    richText.SetAttribute(_T("letter_spacing"), _T("2"));
    richText.SetAttribute(_T("letter_spacing"), _T("-1"));
    richText.SetAttribute(_T("text_indent"), _T("8"));
    richText.SetAttribute(_T("max_lines"), _T("3"));
    richText.SetAttribute(_T("text_overflow"), _T("clip"));
    richText.SetAttribute(_T("selection_bg_color"), _T("#FFABCDEF"));
    richText.SetAttribute(_T("default_link_font_color"), _T("#FF123456"));
    richText.SetAttribute(_T("hovered_link_font_color"), _T("#FF234567"));
    richText.SetAttribute(_T("pressed_link_font_color"), _T("#FF345678"));
    richText.SetAttribute(_T("visited_link_font_color"), _T("#FF456789"));
    richText.SetAttribute(_T("link_font_underline"), _T("false"));
    CHECK(richText.GetLinkCount() == 0);
}

/** 格式化文本的设置、追加、清空与序列化 */
void TestTextContent()
{
    //纯文本
    ui::RichText richText(nullptr);
    CHECK(richText.SetText(_T("Hello")));
    DString text = richText.ToString();
    CHECK(Contains(text, _T("Hello")));

    //单个标签
    CHECK(richText.SetText(_T("<b>bold</b>")));
    text = richText.ToString();
    CHECK(Contains(text, _T("<b>")));
    CHECK(Contains(text, _T("</b>")));
    CHECK(Contains(text, _T("bold")));

    //带根标签的文本
    CHECK(richText.SetText(_T("<RichText><u>underline</u></RichText>")));
    text = richText.ToString();
    CHECK(Contains(text, _T("<u>")));
    CHECK(Contains(text, _T("underline")));

    //超级链接（URL与title）
    CHECK(richText.SetText(_T("<a href=\"https://www.example.com\" title=\"Example\">link</a>")));
    text = richText.ToString();
    CHECK(Contains(text, _T("href=\"https://www.example.com\"")));
    CHECK(Contains(text, _T("title=\"Example\"")));
    CHECK(Contains(text, _T("link")));

    //空文本：清空全部内容
    CHECK(richText.SetText(_T("")));
    CHECK(richText.ToString() == _T("<RichText>\r\n</RichText>"));

    //追加文本（不清空原内容）
    CHECK(richText.SetText(_T("first")));
    CHECK(richText.AppendText(_T("<i>second</i>")));
    text = richText.ToString();
    CHECK(Contains(text, _T("first")));
    CHECK(Contains(text, _T("<i>")));
    CHECK(Contains(text, _T("second")));

    //追加空文本不改变内容
    CHECK(richText.AppendText(_T("")));
    CHECK(Contains(richText.ToString(), _T("first")));

    //追加文本片段（无子节点的节点序列化为自闭合标签）
    ui::RichTextSlice slice;
    slice.m_nodeName = _T("s");
    richText.AppendTextSlice(slice, true);
    CHECK(Contains(richText.ToString(), _T("<s/>")));

    //追加带子节点的文本片段
    ui::RichTextSlice parentSlice;
    parentSlice.m_nodeName = _T("u");
    ui::RichTextSlice childSlice;
    childSlice.m_text = L"child";
    parentSlice.m_children.push_back(childSlice);
    richText.AppendTextSlice(std::move(parentSlice), true);
    text = richText.ToString();
    CHECK(Contains(text, _T("<u>")));
    CHECK(Contains(text, _T("child")));

    //清空
    richText.Clear();
    CHECK(richText.ToString() == _T("<RichText>\r\n</RichText>"));

    //设置空文本ID不改变内容
    CHECK(richText.SetTextId(_T("")));
    CHECK(richText.ToString() == _T("<RichText>\r\n</RichText>"));
}

/** text 属性的花括号替换 */
void TestReplaceBrace()
{
    ui::RichText richText(nullptr);

    //默认允许替换，且替换后是合法标签时按富文本解析
    richText.SetAttribute(_T("text"), _T("{b}bold{/b}"));
    DString text = richText.ToString();
    CHECK(Contains(text, _T("<b>")));
    CHECK(Contains(text, _T("</b>")));
    CHECK(Contains(text, _T("bold")));

    //替换后不是合法标签时，按普通文本处理（内容不丢失）
    richText.SetAttribute(_T("text"), _T("{xyz}text{/xyz}"));
    text = richText.ToString();
    CHECK(Contains(text, _T("{xyz}text{/xyz}")));

    //关闭替换开关后，花括号内容原样保留
    richText.SetAttribute(_T("replace_brace"), _T("false"));
    richText.SetAttribute(_T("text"), _T("{i}italic{/i}"));
    text = richText.ToString();
    CHECK(Contains(text, _T("{i}italic{/i}")));

    //开启替换开关后恢复替换
    richText.SetAttribute(_T("replace_brace"), _T("true"));
    richText.SetAttribute(_T("text"), _T("{i}italic{/i}"));
    text = richText.ToString();
    CHECK(Contains(text, _T("<i>")));
    CHECK(Contains(text, _T("italic")));
}

/** TrimText 的裁剪策略 */
void TestTrimText()
{
    ui::RichText richText(nullptr);

    //默认策略（kAll）：首尾各保留一个空格，词间空格原样保留
    CHECK(richText.TrimText(_T("  a  b  ")) == _T(" a  b "));

    //none：原样返回
    richText.SetAttribute(_T("trim_policy"), _T("none"));
    CHECK(richText.TrimText(_T("  a  b  ")) == _T("  a  b  "));

    //keep_one：连续空格压缩为一个
    richText.SetAttribute(_T("trim_policy"), _T("keep_one"));
    CHECK(richText.TrimText(_T("  a  b  ")) == _T(" a b "));

    //未知策略按默认策略处理
    richText.SetAttribute(_T("trim_policy"), _T("unknown"));
    CHECK(richText.TrimText(_T("  a  b  ")) == _T(" a  b "));

    //裁剪策略影响解析后的文本内容
    richText.SetAttribute(_T("trim_policy"), _T("none"));
    CHECK(richText.SetText(_T("  a  b  ")));
    CHECK(Contains(richText.ToString(), _T("  a  b  ")));

    richText.SetAttribute(_T("trim_policy"), _T("keep_one"));
    CHECK(richText.SetText(_T("  a  b  ")));
    CHECK(Contains(richText.ToString(), _T(" a b ")));

    //空字符串与引用版本接口
    DString emptyText;
    CHECK(richText.TrimText(emptyText).empty());
}

/** 链接与文本选择接口在未布局时的安全行为 */
void TestSelectionAndLinks()
{
    ui::RichText richText(nullptr);

    CHECK(!richText.HasSelection());
    CHECK(richText.GetSelectedText().empty());
    richText.ClearSelection();
    CHECK(!richText.HasSelection());
    CHECK(!richText.CopySelectedText());

    //未解析（未布局）时，链接数量为0
    CHECK(richText.SetText(_T("<a href=\"https://www.example.com\">link</a>")));
    CHECK(richText.GetLinkCount() == 0);
    CHECK(richText.GetLinkUrl(0).empty());
    CHECK(richText.GetLinkTitle(0).empty());
    ui::UiRect rcLink;
    CHECK(!richText.GetLinkRect(0, rcLink));

    //非空文本在无窗口时同样无法估算，返回0
    const ui::UiSize size = richText.EstimateText(ui::UiSize(200, 100));
    CHECK((size.cx == 0) && (size.cy == 0));
}

}//namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    // 本测试不启动全局资源（GlobalManager::Startup），部分内部断言仅用于提示
    // "未初始化/非UI线程"，这里让断言只输出到调试器，避免弹出对话框导致测试挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

    std::cout << "[run] TestDefaults" << std::endl;
    TestDefaults();
    std::cout << "[run] TestTextAlign" << std::endl;
    TestTextAlign();
    std::cout << "[run] TestAttributes" << std::endl;
    TestAttributes();
    std::cout << "[run] TestTextContent" << std::endl;
    TestTextContent();
    std::cout << "[run] TestReplaceBrace" << std::endl;
    TestReplaceBrace();
    std::cout << "[run] TestTrimText" << std::endl;
    TestTrimText();
    std::cout << "[run] TestSelectionAndLinks" << std::endl;
    TestSelectionAndLinks();

    std::cout << "rich text tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
