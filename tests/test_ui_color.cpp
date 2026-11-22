// duilib/Core 颜色相关的单元测试
//
// 覆盖 UiColor 的 ARGB 分量访问、COLORREF 互转，
// StandardColorMap 的 Hex 解析/格式化与标准颜色名查询，
// 以及 ColorMap 的增删查接口。不依赖窗口与渲染环境。

#include "duilib/Core/UiColor.h"
#include "duilib/Core/ColorMap.h"
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

/** UiColor 的构造与分量访问 */
void TestUiColorBasics()
{
    ui::UiColor color;
    CHECK(color.IsEmpty());
    CHECK_INT(color.GetARGB(), 0);
    CHECK_INT(color.GetAlpha(), 0);
    CHECK_INT(color.GetRed(), 0);
    CHECK_INT(color.GetGreen(), 0);
    CHECK_INT(color.GetBlue(), 0);

    //RGB 构造：Alpha 固定为 255
    ui::UiColor rgb(0x11, 0x22, 0x33);
    CHECK(!rgb.IsEmpty());
    CHECK_INT(rgb.GetA(), 0xFF);
    CHECK_INT(rgb.GetR(), 0x11);
    CHECK_INT(rgb.GetG(), 0x22);
    CHECK_INT(rgb.GetB(), 0x33);
    CHECK_INT(rgb.GetARGB(), 0xFF112233u);

    //ARGB 构造
    ui::UiColor argb(0x80, 0x11, 0x22, 0x33);
    CHECK_INT(argb.GetAlpha(), 0x80);
    CHECK_INT(argb.GetARGB(), 0x80112233u);
    CHECK_INT(ui::UiColor::MakeARGB(0x80, 0x11, 0x22, 0x33), 0x80112233u);

    ui::UiColor fromArgb((ui::UiColor::ARGB)0x80112233u);
    CHECK(fromArgb == argb);
    CHECK(fromArgb != rgb);

    ui::UiColor fromInt(static_cast<int32_t>(0x80112233u));
    CHECK_INT(fromInt.GetARGB(), 0x80112233u);

    //SetARGB
    ui::UiColor changed;
    changed.SetARGB(0x7F010203u);
    CHECK_INT(changed.GetA(), 0x7F);
    CHECK_INT(changed.GetR(), 0x01);
    CHECK_INT(changed.GetG(), 0x02);
    CHECK_INT(changed.GetB(), 0x03);

    //相等判断
    CHECK(changed == ui::UiColor(0x7F, 1, 2, 3));
    CHECK(changed.Equals(ui::UiColor(0x7F, 1, 2, 3)));
    CHECK(changed != ui::UiColor(0x7E, 1, 2, 3));
}

/** COLORREF 互转：不透明颜色直接转换，半透明颜色按主题风格混合 */
void TestUiColorColorref()
{
    ui::UiColor color;
    color.SetFromCOLORREF(ui::MakeColorRef(0x11, 0x22, 0x33));
    CHECK_INT(color.GetA(), 0xFF);
    CHECK_INT(color.GetR(), 0x11);
    CHECK_INT(color.GetG(), 0x22);
    CHECK_INT(color.GetB(), 0x33);

    //不透明：与主题无关，直接返回 RGB
    CHECK_INT(color.ToCOLORREF(false), ui::MakeColorRef(0x11, 0x22, 0x33));
    CHECK_INT(color.ToCOLORREF(true), ui::MakeColorRef(0x11, 0x22, 0x33));

    //半透明白色：深色主题下按 Alpha 衰减，浅色主题下与白色背景混合
    ui::UiColor halfWhite(0x80, 0xFF, 0xFF, 0xFF);
    CHECK_INT(halfWhite.ToCOLORREF(true), ui::MakeColorRef(0x80, 0x80, 0x80));
    CHECK_INT(halfWhite.ToCOLORREF(false), ui::MakeColorRef(0xFF, 0xFF, 0xFF));

    //全透明：深色主题为黑色，浅色主题为白色
    ui::UiColor transparent(0x00, 0xFF, 0xFF, 0xFF);
    CHECK_INT(transparent.ToCOLORREF(true), ui::MakeColorRef(0x00, 0x00, 0x00));
    CHECK_INT(transparent.ToCOLORREF(false), ui::MakeColorRef(0xFF, 0xFF, 0xFF));

    //半透明黑色
    ui::UiColor halfBlack(0x40, 0x00, 0x00, 0x00);
    CHECK_INT(halfBlack.ToCOLORREF(true), ui::MakeColorRef(0x00, 0x00, 0x00));
    CHECK_INT(halfBlack.ToCOLORREF(false), ui::MakeColorRef(0xBF, 0xBF, 0xBF));
}

/** StandardColorMap：Hex 字符串与 UiColor 的互转 */
void TestStandardColorHex()
{
    ui::UiColor color = ui::StandardColorMap::HexToColor(_T("#FF112233"));
    CHECK_INT(color.GetA(), 0xFF);
    CHECK_INT(color.GetR(), 0x11);
    CHECK_INT(color.GetG(), 0x22);
    CHECK_INT(color.GetB(), 0x33);

    //#RRGGBB 自动补全 Alpha
    ui::UiColor shortColor = ui::StandardColorMap::HexToColor(_T("#112233"));
    CHECK(shortColor == color);

    //小写十六进制
    ui::UiColor lowerColor = ui::StandardColorMap::HexToColor(_T("#ff112233"));
    CHECK(lowerColor == color);

    //含 Alpha 的格式
    ui::UiColor alphaColor = ui::StandardColorMap::HexToColor(_T("#80112233"));
    CHECK_INT(alphaColor.GetA(), 0x80);

    //格式化
    CHECK(ui::StandardColorMap::ColorToHex(color) == _T("#FF112233"));
    CHECK(ui::StandardColorMap::ColorToHex(alphaColor) == _T("#80112233"));
    CHECK(ui::StandardColorMap::ColorToHex(ui::UiColor()).empty());

    //往返转换
    ui::UiColor roundTrip = ui::StandardColorMap::HexToColor(
        ui::StandardColorMap::ColorToHex(alphaColor));
    CHECK(roundTrip == alphaColor);
}

/** StandardColorMap：标准颜色名的查询（大小写不敏感） */
void TestStandardColorNames()
{
    const ui::StandardColorMap& colors = ui::StandardColorMap::Instance();
    CHECK_INT(colors.GetColor(_T("Black")).GetARGB(), ui::UiColors::Black);
    CHECK_INT(colors.GetColor(_T("white")).GetARGB(), ui::UiColors::White);
    CHECK_INT(colors.GetColor(_T("ReD")).GetARGB(), ui::UiColors::Red);
    CHECK(colors.GetColor(_T("NotAColorName")).IsEmpty());
    CHECK(colors.GetColor(_T("")).IsEmpty());
}

/** ColorMap：颜色表的增删查 */
void TestColorMap()
{
    ui::ColorMap colorMap;
    CHECK(colorMap.GetColor(_T("bkcolor")).IsEmpty());
    CHECK(colorMap.GetColor(_T("")).IsEmpty());

    //直接添加 UiColor
    colorMap.AddColor(_T("bkcolor"), ui::UiColor(0xFF, 0x11, 0x22, 0x33));
    CHECK_INT(colorMap.GetColor(_T("bkcolor")).GetARGB(), 0xFF112233u);
    CHECK(colorMap.GetColor(_T("BKCOLOR")).IsEmpty()); //名称区分大小写

    //用 Hex 字符串添加
    colorMap.AddColor(_T("textcolor"), _T("#80445566"));
    CHECK_INT(colorMap.GetColor(_T("textcolor")).GetARGB(), 0x80445566u);

    //用标准颜色名添加
    colorMap.AddColor(_T("named"), _T("Black"));
    CHECK_INT(colorMap.GetColor(_T("named")).GetARGB(), 0xFF000000u);

    //删除单个颜色
    colorMap.RemoveColor(_T("textcolor"));
    CHECK(colorMap.GetColor(_T("textcolor")).IsEmpty());
    CHECK_INT(colorMap.GetColor(_T("bkcolor")).GetARGB(), 0xFF112233u);

    //删除不存在的颜色：不应影响其他项
    colorMap.RemoveColor(_T("not-exist"));
    CHECK_INT(colorMap.GetColor(_T("bkcolor")).GetARGB(), 0xFF112233u);

    //全部清空
    colorMap.RemoveAllColors();
    CHECK(colorMap.GetColor(_T("bkcolor")).IsEmpty());
    CHECK(colorMap.GetColor(_T("named")).IsEmpty());
}

} // namespace

int main()
{
    TestUiColorBasics();
    TestUiColorColorref();
    TestStandardColorHex();
    TestStandardColorNames();
    TestColorMap();

    std::cout << "ui color tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
