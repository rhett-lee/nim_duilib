// Label 控件的单元测试
//
// 测试对象：ui::Label（LabelTemplate<Control>）及其派生类型 LabelBox/LabelHBox/LabelVBox。
// 本测试不创建窗口、不加载主题资源，只验证与界面无关的属性、文本与样式逻辑，
// 因此可以在命令行（含 CI）直接运行。
//
// 覆盖内容：
//  1. 默认属性值（文本、样式、字体、间距、对齐、各种布尔开关）；
//  2. 文本/文本ID/UTF8 文本的设置与获取、是否相等的判断；
//  3. 换行符替换（"\\n" -> "\n"）；
//  4. text_align 属性（含 "center"/"hcenter" 歧义、hjustify/vjustify）的解析；
//  5. 单行/多行、省略号、auto_tooltip、字体、内边距、行间距、字间距、
//     纵向文本及相关开关、rich_text 属性；
//  6. 各状态文本颜色与 GetPaintStateTextColor 的降级规则；
//  7. SetTextStyle/SetDefaultTextStyle/GetValidTextStyle；
//  8. 空文本时的尺寸估算（不依赖渲染接口）。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Control/Label.h"

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

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 检查位标志是否存在 */
bool HasFlag(uint32_t value, uint32_t flag)
{
    return (value & flag) != 0;
}

/** 检查水平对齐方式（忽略其它标志位） */
uint32_t GetHAlign(uint32_t textStyle)
{
    return textStyle & ui::TEXT_HALIGN_ALL;
}

/** 检查垂直对齐方式（忽略其它标志位） */
uint32_t GetVAlign(uint32_t textStyle)
{
    return textStyle & ui::TEXT_VALIGN_ALL;
}

/** 默认属性值 */
void TestDefaults()
{
    ui::Label label(nullptr);
    CHECK(label.GetType() == DUI_CTR_LABEL);
    CHECK(label.GetText().empty());
    CHECK(label.GetTextId().empty());
    CHECK(label.GetUTF8Text().empty());
    CHECK(label.GetUTF8TextId().empty());
    CHECK(label.GetFontId().empty());
    CHECK(label.IsTextEquals(_T("")));
    CHECK(!label.IsRichText());
    CHECK(label.IsSingleLine());
    CHECK(!label.IsReplaceNewline());
    CHECK(!label.IsAutoToolTip());
    CHECK(!label.IsVerticalText());
    CHECK(label.IsUseFontHeight());
    CHECK(label.IsRotate90ForAscii());
    CHECK(!label.HasHoveredState());
    CHECK(label.GetToolTipText().empty());

    //默认文本样式：左对齐、垂直居中、结尾省略号、单行
    const uint32_t style = label.GetTextStyle();
    CHECK(GetHAlign(style) == ui::TEXT_LEFT);
    CHECK(GetVAlign(style) == ui::TEXT_VCENTER);
    CHECK(HasFlag(style, ui::TEXT_END_ELLIPSIS));
    CHECK(HasFlag(style, ui::TEXT_SINGLELINE));
    CHECK(!HasFlag(style, ui::TEXT_PATH_ELLIPSIS));
    CHECK(!HasFlag(style, ui::TEXT_VERTICAL));
    CHECK(!HasFlag(style, ui::TEXT_WORD_WRAP));

    const ui::UiPadding padding = label.GetTextPadding();
    CHECK((padding.left == 0) && (padding.top == 0) && (padding.right == 0) && (padding.bottom == 0));

    float mul = 0.0f;
    float add = 0.0f;
    label.GetLineSpacing(&mul, &add);
    CHECK(mul == 1.0f);
    CHECK(add == 0.0f);
    CHECK(label.GetWordSpacing() == 0.0f);

    //Label 默认尺寸为自动，LabelBox 默认尺寸为拉伸
    CHECK(label.GetFixedWidth().IsAuto());
    CHECK(label.GetFixedHeight().IsAuto());
    ui::LabelBox labelBox(nullptr);
    CHECK(labelBox.GetType() == DUI_CTR_LABELBOX);
    CHECK(labelBox.GetFixedWidth().IsStretch());
    CHECK(labelBox.GetFixedHeight().IsStretch());
    ui::LabelHBox labelHBox(nullptr);
    CHECK(labelHBox.GetType() == DUI_CTR_LABELHBOX);
    ui::LabelVBox labelVBox(nullptr);
    CHECK(labelVBox.GetType() == DUI_CTR_LABELVBOX);
}

/** 文本的设置/获取与比较 */
void TestText()
{
    ui::Label label(nullptr);
    label.SetText(_T("Hello"));
    CHECK(label.GetText() == _T("Hello"));
    CHECK(!label.GetText().empty());
    CHECK(label.IsTextEquals(_T("Hello")));
    CHECK(!label.IsTextEquals(_T("hello")));
    CHECK(!label.IsTextEquals(_T(" Hello")));
    CHECK(label.GetUTF8Text() == std::string("Hello"));

    //UTF8接口（含中文，验证编码来回转换一致）
    label.SetUTF8Text("中文内容");
    CHECK(label.GetText() == _T("中文内容"));
    CHECK(label.GetUTF8Text() == std::string("中文内容"));

    //清空文本
    label.SetText(_T(""));
    CHECK(label.GetText().empty());
    CHECK(label.IsTextEquals(_T("")));

    //文本ID（未加载语言资源时，只验证ID本身的保存与读取）
    label.SetTextId(_T("STRID_TEST_LABEL_TEXT"));
    CHECK(label.GetTextId() == _T("STRID_TEST_LABEL_TEXT"));
    CHECK(label.GetUTF8TextId() == std::string("STRID_TEST_LABEL_TEXT"));
    label.SetUTF8TextId("STRID_UTF8_ID");
    CHECK(label.GetTextId() == _T("STRID_UTF8_ID"));

    //设置字面文本后，文本ID仍然保留
    label.SetText(_T("content"));
    CHECK(label.GetText() == _T("content"));
    CHECK(label.GetTextId() == _T("STRID_UTF8_ID"));

    //替换换行符：文本中的"\\n"两个字符被替换成换行符
    ui::Label label2(nullptr);
    CHECK(!label2.IsReplaceNewline());
    label2.SetReplaceNewline(true);
    CHECK(label2.IsReplaceNewline());
    label2.SetText(_T("line1\\nline2"));
    CHECK(label2.GetText() == _T("line1\nline2"));
    label2.SetReplaceNewline(false);
    CHECK(label2.GetText() == _T("line1\\nline2"));
    CHECK(label2.IsTextEquals(_T("line1\\nline2")));
}

/** text_align 属性解析 */
void TestTextAlign()
{
    ui::Label label(nullptr);

    label.SetAttribute(_T("text_align"), _T("left"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_LEFT);

    label.SetAttribute(_T("text_align"), _T("right"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_RIGHT);

    label.SetAttribute(_T("text_align"), _T("hcenter"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_HCENTER);

    //"center"是"hcenter"的兼容别名
    label.SetAttribute(_T("text_align"), _T("left"));
    label.SetAttribute(_T("text_align"), _T("center"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_HCENTER);

    //"vcenter"不能被误判为"center"（水平对齐方式应保持为靠左）
    label.SetAttribute(_T("text_align"), _T("left"));
    label.SetAttribute(_T("text_align"), _T("vcenter"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_LEFT);
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_VCENTER);

    //"hjustify"为水平两端对齐
    label.SetAttribute(_T("text_align"), _T("hjustify"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_HJUSTIFY);

    //垂直对齐
    label.SetAttribute(_T("text_align"), _T("top"));
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_TOP);
    label.SetAttribute(_T("text_align"), _T("bottom"));
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_BOTTOM);
    label.SetAttribute(_T("text_align"), _T("vjustify"));
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_VJUSTIFY);

    //多个取值同时出现（水平 + 垂直）
    label.SetAttribute(_T("text_align"), _T("right,bottom"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_RIGHT);
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_BOTTOM);

    label.SetAttribute(_T("text_align"), _T("hcenter,top"));
    CHECK(GetHAlign(label.GetTextStyle()) == ui::TEXT_HCENTER);
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_TOP);

    //"top,vcenter"中"top"优先
    label.SetAttribute(_T("text_align"), _T("top,vcenter"));
    CHECK(GetVAlign(label.GetTextStyle()) == ui::TEXT_TOP);

    //空字符串不改变现有对齐方式
    const uint32_t styleBefore = label.GetTextStyle();
    label.SetAttribute(_T("text_align"), _T(""));
    CHECK(label.GetTextStyle() == styleBefore);
}

/** 单行/多行与省略号属性 */
void TestLineAndEllipsis()
{
    ui::Label label(nullptr);

    label.SetAttribute(_T("single_line"), _T("false"));
    CHECK(!label.IsSingleLine());
    CHECK(!HasFlag(label.GetTextStyle(), ui::TEXT_SINGLELINE));

    label.SetAttribute(_T("single_line"), _T("true"));
    CHECK(label.IsSingleLine());
    CHECK(HasFlag(label.GetTextStyle(), ui::TEXT_SINGLELINE));

    //multi_line与single_line的语义相反
    label.SetAttribute(_T("multi_line"), _T("true"));
    CHECK(!label.IsSingleLine());
    label.SetAttribute(_T("multi_line"), _T("false"));
    CHECK(label.IsSingleLine());

    //结尾省略号
    label.SetAttribute(_T("end_ellipsis"), _T("false"));
    CHECK(!HasFlag(label.GetTextStyle(), ui::TEXT_END_ELLIPSIS));
    label.SetAttribute(_T("endellipsis"), _T("true"));
    CHECK(HasFlag(label.GetTextStyle(), ui::TEXT_END_ELLIPSIS));

    //路径省略号
    label.SetAttribute(_T("path_ellipsis"), _T("true"));
    CHECK(HasFlag(label.GetTextStyle(), ui::TEXT_PATH_ELLIPSIS));
    label.SetAttribute(_T("pathellipsis"), _T("false"));
    CHECK(!HasFlag(label.GetTextStyle(), ui::TEXT_PATH_ELLIPSIS));

    //auto_tooltip
    CHECK(!label.IsAutoToolTip());
    label.SetAttribute(_T("auto_tooltip"), _T("true"));
    CHECK(label.IsAutoToolTip());
    label.SetAutoToolTip(false);
    CHECK(!label.IsAutoToolTip());
}

/** 字体、内边距、行间距与字间距 */
void TestFontAndSpacing()
{
    ui::Label label(nullptr);

    label.SetFontId(_T("font_test"));
    CHECK(label.GetFontId() == _T("font_test"));
    label.SetAttribute(_T("font"), _T("font_attr"));
    CHECK(label.GetFontId() == _T("font_attr"));

    //内边距（不做DPI缩放，便于精确校验）
    label.SetTextPadding(ui::UiPadding(1, 2, 3, 4), false);
    ui::UiPadding padding = label.GetTextPadding();
    CHECK((padding.left == 1) && (padding.top == 2) && (padding.right == 3) && (padding.bottom == 4));

    //DPI为100%时，属性设置的内边距保持不变
    label.SetAttribute(_T("text_padding"), _T("4,6,8,10"));
    padding = label.GetTextPadding();
    CHECK((padding.left == 4) && (padding.top == 6) && (padding.right == 8) && (padding.bottom == 10));

    //行间距：负数被修正为默认值
    label.SetLineSpacing(2.0f, 6.0f, false);
    float mul = 0.0f;
    float add = 0.0f;
    label.GetLineSpacing(&mul, &add);
    CHECK(mul == 2.0f);
    CHECK(add == 6.0f);

    label.SetAttribute(_T("spacing_mul"), _T("1.5"));
    label.GetLineSpacing(&mul, &add);
    CHECK(mul == 1.5f);
    CHECK(add == 6.0f);

    label.SetAttribute(_T("spacing_add"), _T("4"));
    label.GetLineSpacing(&mul, &add);
    CHECK(mul == 1.5f);
    CHECK(add == 4.0f);

    label.SetLineSpacing(-1.0f, -5.0f, false);
    label.GetLineSpacing(&mul, &add);
    CHECK(mul == 1.0f);
    CHECK(add == 0.0f);

    //字间距：负数被修正为0
    label.SetWordSpacing(3.0f, false);
    CHECK(label.GetWordSpacing() == 3.0f);
    label.SetAttribute(_T("word_spacing"), _T("2"));
    CHECK(label.GetWordSpacing() == 2.0f);
    label.SetWordSpacing(-2.0f, false);
    CHECK(label.GetWordSpacing() == 0.0f);
}

/** 纵向文本与RichText模式开关 */
void TestVerticalAndRichText()
{
    ui::Label label(nullptr);

    CHECK(!label.IsVerticalText());
    label.SetVerticalText(true);
    CHECK(label.IsVerticalText());
    CHECK(HasFlag(label.GetTextStyle(), ui::TEXT_VERTICAL));
    label.SetVerticalText(false);
    CHECK(!label.IsVerticalText());
    CHECK(!HasFlag(label.GetTextStyle(), ui::TEXT_VERTICAL));

    label.SetAttribute(_T("vertical_text"), _T("true"));
    CHECK(label.IsVerticalText());
    label.SetAttribute(_T("vertical_text"), _T("false"));
    CHECK(!label.IsVerticalText());

    //纵向文本相关开关
    CHECK(label.IsUseFontHeight());
    label.SetAttribute(_T("use_font_height"), _T("false"));
    CHECK(!label.IsUseFontHeight());
    label.SetUseFontHeight(true);
    CHECK(label.IsUseFontHeight());

    CHECK(label.IsRotate90ForAscii());
    label.SetAttribute(_T("ascii_rotate_90"), _T("false"));
    CHECK(!label.IsRotate90ForAscii());
    label.SetRotate90ForAscii(true);
    CHECK(label.IsRotate90ForAscii());

    //RichText模式
    CHECK(!label.IsRichText());
    label.SetAttribute(_T("rich_text"), _T("true"));
    CHECK(label.IsRichText());
    label.SetRichText(false);
    CHECK(!label.IsRichText());

    //RichText模式下设置文本内容不改变文本本身
    label.SetRichText(true);
    label.SetAttribute(_T("text"), _T("a<b>b</b>"));
    CHECK(label.GetText() == _T("a<b>b</b>"));
}

/** 各状态文本颜色与降级规则 */
void TestStateTextColor()
{
    ui::Label label(nullptr);
    CHECK(!label.HasHoveredState());

    //常规、悬停、按下、禁用状态颜色
    label.SetStateTextColor(ui::kControlStateNormal, _T("#FF102030"));
    CHECK(label.GetStateTextColor(ui::kControlStateNormal) == _T("#FF102030"));

    label.SetStateTextColor(ui::kControlStateHovered, _T("#FF405060"));
    CHECK(label.GetStateTextColor(ui::kControlStateHovered) == _T("#FF405060"));
    CHECK(label.HasHoveredState());

    label.SetStateTextColor(ui::kControlStatePressed, _T("#FF708090"));
    CHECK(label.GetStateTextColor(ui::kControlStatePressed) == _T("#FF708090"));

    label.SetStateTextColor(ui::kControlStateDisabled, _T("#FFA0B0C0"));
    CHECK(label.GetStateTextColor(ui::kControlStateDisabled) == _T("#FFA0B0C0"));

    //属性方式设置
    label.SetAttribute(_T("normal_text_color"), _T("#FF010203"));
    CHECK(label.GetStateTextColor(ui::kControlStateNormal) == _T("#FF010203"));
    label.SetAttribute(_T("hot_text_color"), _T("#FF040506"));
    CHECK(label.GetStateTextColor(ui::kControlStateHovered) == _T("#FF040506"));
    label.SetAttribute(_T("pushed_text_color"), _T("#FF070809"));
    CHECK(label.GetStateTextColor(ui::kControlStatePressed) == _T("#FF070809"));
    label.SetAttribute(_T("disabled_text_color"), _T("#FF0A0B0C"));
    CHECK(label.GetStateTextColor(ui::kControlStateDisabled) == _T("#FF0A0B0C"));

    //GetPaintStateTextColor：各状态均有效时，原样返回
    ui::ControlStateType stateType = ui::kControlStateNormal;
    CHECK(label.GetPaintStateTextColor(ui::kControlStatePressed, stateType) == _T("#FF070809"));
    CHECK(stateType == ui::kControlStatePressed);

    //按下颜色为空时，降级为悬停颜色
    label.SetStateTextColor(ui::kControlStatePressed, _T(""));
    stateType = ui::kControlStateNormal;
    CHECK(label.GetPaintStateTextColor(ui::kControlStatePressed, stateType) == _T("#FF040506"));
    CHECK(stateType == ui::kControlStateHovered);

    //悬停颜色为空时，降级为常规颜色
    label.SetStateTextColor(ui::kControlStateHovered, _T(""));
    stateType = ui::kControlStatePressed;
    CHECK(label.GetPaintStateTextColor(ui::kControlStateHovered, stateType) == _T("#FF010203"));
    CHECK(stateType == ui::kControlStateNormal);

    //未设置任何颜色时，返回常规颜色
    ui::Label label2(nullptr);
    const DString normalColor = label2.GetStateTextColor(ui::kControlStateNormal);
    stateType = ui::kControlStateDisabled;
    CHECK(label2.GetPaintStateTextColor(ui::kControlStateHovered, stateType) == normalColor);
    CHECK(stateType == ui::kControlStateNormal);
}

/** 文本样式的设置与恢复默认值 */
void TestTextStyle()
{
    ui::Label label(nullptr);

    //设置样式：水平居中、靠下、自动换行、纵向文本
    label.SetTextStyle(ui::TEXT_HCENTER | ui::TEXT_BOTTOM | ui::TEXT_WORD_WRAP | ui::TEXT_VERTICAL, false);
    uint32_t style = label.GetTextStyle();
    CHECK(GetHAlign(style) == ui::TEXT_HCENTER);
    CHECK(GetVAlign(style) == ui::TEXT_BOTTOM);
    CHECK(HasFlag(style, ui::TEXT_WORD_WRAP));
    CHECK(HasFlag(style, ui::TEXT_VERTICAL));
    //未指定单行标志时，自动变为多行模式
    CHECK(!label.IsSingleLine());
    CHECK(label.IsVerticalText());

    //恢复默认样式
    label.SetDefaultTextStyle(false);
    style = label.GetTextStyle();
    CHECK(GetHAlign(style) == ui::TEXT_LEFT);
    CHECK(GetVAlign(style) == ui::TEXT_VCENTER);
    CHECK(HasFlag(style, ui::TEXT_END_ELLIPSIS));
    CHECK(HasFlag(style, ui::TEXT_SINGLELINE));
    CHECK(label.IsSingleLine());
    CHECK(!label.IsVerticalText());

    //GetValidTextStyle：只保留支持的风格
    const uint32_t validStyle = ui::Label::GetValidTextStyle(
        ui::TEXT_RIGHT | ui::TEXT_TOP | ui::TEXT_SINGLELINE | ui::TEXT_END_ELLIPSIS |
        ui::TEXT_PATH_ELLIPSIS | ui::TEXT_NOCLIP | ui::TEXT_WORD_WRAP | ui::TEXT_VERTICAL);
    CHECK(GetHAlign(validStyle) == ui::TEXT_RIGHT);
    CHECK(GetVAlign(validStyle) == ui::TEXT_TOP);
    CHECK(HasFlag(validStyle, ui::TEXT_SINGLELINE));
    CHECK(HasFlag(validStyle, ui::TEXT_END_ELLIPSIS));
    CHECK(HasFlag(validStyle, ui::TEXT_PATH_ELLIPSIS));
    CHECK(HasFlag(validStyle, ui::TEXT_NOCLIP));
    CHECK(HasFlag(validStyle, ui::TEXT_WORD_WRAP));
    CHECK(HasFlag(validStyle, ui::TEXT_VERTICAL));

    //未指定对齐方式时，使用默认的靠左、靠上
    const uint32_t defaultStyle = ui::Label::GetValidTextStyle(0);
    CHECK(GetHAlign(defaultStyle) == ui::TEXT_LEFT);
    CHECK(GetVAlign(defaultStyle) == ui::TEXT_TOP);
    CHECK(!HasFlag(defaultStyle, ui::TEXT_SINGLELINE));
}

/** 空文本时的尺寸估算（不依赖渲染接口） */
void TestEstimateText()
{
    ui::Label label(nullptr);
    ui::UiSize size = label.EstimateText(ui::UiSize(100, 50));
    CHECK((size.cx == 0) && (size.cy == 0));

    size = label.EstimateTextWith(_T(""), ui::UiSize(100, 50));
    CHECK((size.cx == 0) && (size.cy == 0));

    //垂直文本、多行文本场景下，空文本同样返回0
    label.SetVerticalText(true);
    label.SetSingleLine(false);
    size = label.EstimateText(ui::UiSize(100, 50));
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
    std::cout << "[run] TestText" << std::endl;
    TestText();
    std::cout << "[run] TestTextAlign" << std::endl;
    TestTextAlign();
    std::cout << "[run] TestLineAndEllipsis" << std::endl;
    TestLineAndEllipsis();
    std::cout << "[run] TestFontAndSpacing" << std::endl;
    TestFontAndSpacing();
    std::cout << "[run] TestVerticalAndRichText" << std::endl;
    TestVerticalAndRichText();
    std::cout << "[run] TestStateTextColor" << std::endl;
    TestStateTextColor();
    std::cout << "[run] TestTextStyle" << std::endl;
    TestTextStyle();
    std::cout << "[run] TestEstimateText" << std::endl;
    TestEstimateText();

    std::cout << "label tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
