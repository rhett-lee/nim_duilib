// Render 目录的单元测试
//
// 测试对象：duilib/Render 下的基础设施代码：
//  1. RichTextAttrTable：字体/颜色/内联图片索引表的去重、无效索引、跨表比较、Clear/Release/Swap；
//  2. RichTextDataList：文本片段列表的增删、交换与属性表联动；
//  3. RichTextCharLayout：行内字符排版数据的存储、x 偏移/宽度/属性标志查询、
//     超宽行（> 8191 像素）自动切换为 32 位偏移后的数据一致性、异常输入（负数/NaN）处理；
//  4. RichTextLineText / RichTextLineInfo：共享文本的偏移访问、独立存储与行文本视图；
//  5. AutoClip：矩形裁剪、圆角裁剪、嵌套裁剪、bClip=false 空操作，以及析构时裁剪状态的恢复；
//  6. CanPresentBackBufferOnly：后备缓冲复用条件的真值表。
//
// 说明：BitmapAlpha 的 Alpha 保存/恢复逻辑已由 test_bitmap_alpha.cpp 覆盖，本文件不重复测试。
//       AutoClip 的测试使用离屏渲染，不创建窗口。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiColor.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/UiString.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
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

/** 输出当前测试小节的标题，便于定位失败用例 */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

/** 浮点数比较：字符 x 偏移按 1/8 像素四舍五入存储，单个值的误差不超过 1/16 像素 */
bool FloatNear(float actual, float expected)
{
    return std::fabs(actual - expected) <= 0.07f;
}

void CheckFloat(float actual, float expected, const char* expression)
{
    ++g_checkCount;
    if (!FloatNear(actual, expected)) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << expression << ": actual=" << actual
                  << ", expected=" << expected << std::endl;
    }
}

#define CHECK_FLOAT(actual, expected) CheckFloat((actual), (expected), #actual " == " #expected)

/** 常用颜色值（ARGB 格式） */
const uint32_t kWhiteColor = 0xFFFFFFFFu;
const uint32_t kRedColor   = 0xFFFF0000u;
const uint32_t kGreenColor = 0xFF00FF00u;
const uint32_t kBlueColor  = 0xFF0000FFu;

/** 读取 Render 中单个像素的颜色值（ARGB 格式） */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

/** 判断两个颜色是否接近（允许抗锯齿带来的轻微误差） */
bool ColorNear(uint32_t color, uint32_t expected, int32_t tolerance = 6)
{
    for (int32_t i = 0; i < 32; i += 8) {
        const int32_t a = (int32_t)((color >> i) & 0xFF);
        const int32_t b = (int32_t)((expected >> i) & 0xFF);
        if (std::abs(a - b) > tolerance) {
            char buf[128] = { 0 };
            std::snprintf(buf, sizeof(buf), "[COLOR] actual=0x%08X, expected=0x%08X", color, expected);
            std::cerr << buf << std::endl;
            return false;
        }
    }
    return true;
}

/** 创建离屏渲染对象 */
std::unique_ptr<ui::IRender> CreateRender(ui::IRenderFactory* pRenderFactory, int32_t w, int32_t h)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    return pRenderFactory->CreateOffscreenRender((uint32_t)w, (uint32_t)h);
}

/** 设置字体属性（UiFontEx 的引用计数基类禁止拷贝，因此通过输出参数构造） */
void MakeFont(ui::UiFontEx& font, const wchar_t* pName, int32_t nSize, bool bBold)
{
    font.m_fontName = pName;
    font.m_fontSize = nSize;
    font.m_bBold = bBold;
}

// ---------------------------------------------------------------------------
// 1. RichTextAttrTable（RT-01 ~ RT-20）
// ---------------------------------------------------------------------------
void TestRichTextAttrTable()
{
    Section("TestRichTextAttrTable");
    ui::UiFontEx fontA;
    MakeFont(fontA, L"Arial", 12, true);
    ui::UiFontEx fontB;
    MakeFont(fontB, L"Arial", 12, false);

    ui::RichTextAttrTable attrs;

    //RT-01 空表：无效索引返回空值
    CHECK(attrs.GetFontCount() == 0);
    CHECK(attrs.GetFont(0) == nullptr);
    CHECK(attrs.GetFont(1) == nullptr);
    CHECK(attrs.GetColor(0).IsEmpty());
    CHECK(attrs.GetColor(1).IsEmpty());
    CHECK(attrs.GetImage(0).m_pImage == nullptr);
    CHECK(attrs.GetImage(1).m_pImage == nullptr);

    //RT-02 添加空字体返回0，不占用索引
    CHECK(attrs.AddFont(nullptr) == 0);
    CHECK(attrs.GetFontCount() == 0);
    CHECK(attrs.AddColor(ui::UiColor()) == 0);
    CHECK(attrs.AddImage(nullptr, 10, 10) == 0);

    //RT-03 添加字体：索引从1开始
    CHECK(attrs.AddFont(&fontA) == 1);
    CHECK(attrs.AddFont(&fontB) == 2);
    CHECK(attrs.GetFontCount() == 2);

    //RT-04 相同字体去重
    CHECK(attrs.AddFont(&fontA) == 1);
    CHECK(attrs.GetFontCount() == 2);

    //RT-05 获取字体属性
    const ui::UiFontEx* pFont1 = attrs.GetFont(1);
    CHECK(pFont1 != nullptr);
    if (pFont1 != nullptr) {
        CHECK(*pFont1 == fontA);
        CHECK(pFont1->m_fontName == L"Arial");
        CHECK(pFont1->m_fontSize == 12);
        CHECK(pFont1->m_bBold);
    }
    CHECK(attrs.GetFont(3) == nullptr);

    //RT-06 颜色去重与获取
    const ui::UiColor red = ui::UiColor(kRedColor);
    const ui::UiColor blue = ui::UiColor(kBlueColor);
    CHECK(attrs.AddColor(red) == 1);
    CHECK(attrs.AddColor(red) == 1);
    CHECK(attrs.AddColor(blue) == 2);
    CHECK(attrs.GetColor(1) == red);
    CHECK(attrs.GetColor(2) == blue);
    CHECK(attrs.GetColor(3).IsEmpty());

    //RT-07 跨表比较：索引不同但内容相同
    ui::RichTextAttrTable other;
    CHECK(other.AddFont(&fontB) == 1);
    CHECK(other.AddFont(&fontA) == 2);
    CHECK(ui::RichTextAttrTable::IsFontEqual(attrs, 1, other, 2));
    CHECK(!ui::RichTextAttrTable::IsFontEqual(attrs, 1, other, 1));
    CHECK(ui::RichTextAttrTable::IsFontEqual(attrs, 0, other, 0));
    CHECK(!ui::RichTextAttrTable::IsFontEqual(attrs, 1, other, 0));

    //RT-08 跨表比较颜色
    CHECK(other.AddColor(blue) == 1);
    CHECK(other.AddColor(red) == 2);
    CHECK(ui::RichTextAttrTable::IsColorEqual(attrs, 1, other, 2));
    CHECK(!ui::RichTextAttrTable::IsColorEqual(attrs, 1, other, 1));

    //RT-09 Swap 交换两表的数据
    ui::RichTextAttrTable swapTable;
    swapTable.AddFont(&fontB);
    swapTable.AddColor(blue);
    swapTable.Swap(other);
    CHECK(swapTable.GetFontCount() == 2);
    CHECK(other.GetFontCount() == 1);
    const ui::UiFontEx* pSwapFont = swapTable.GetFont(1);
    CHECK(pSwapFont != nullptr);
    if (pSwapFont != nullptr) {
        CHECK(*pSwapFont == fontB);
    }

    //RT-10 Clear 清空数据
    attrs.Clear();
    CHECK(attrs.GetFontCount() == 0);
    CHECK(attrs.GetFont(1) == nullptr);
    CHECK(attrs.GetColor(1).IsEmpty());

    //RT-11 Release 释放容量后仍可正常使用
    other.Release();
    CHECK(other.GetFontCount() == 0);
    CHECK(other.AddFont(&fontA) == 1);
    CHECK(other.GetFontCount() == 1);
}

// ---------------------------------------------------------------------------
// 2. RichTextAttrTable 的内联图片（RT-21 ~ RT-26）
//    需要真实的 IBitmap，因此依赖离屏渲染
// ---------------------------------------------------------------------------
void TestRichTextAttrTableImages(ui::IRenderFactory* pRenderFactory)
{
    Section("TestRichTextAttrTableImages");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 8, 8);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    std::shared_ptr<ui::IBitmap> spImage1(pRender->MakeImageSnapshot());
    std::shared_ptr<ui::IBitmap> spImage2(pRender->MakeImageSnapshot());
    CHECK(spImage1 != nullptr);
    CHECK(spImage2 != nullptr);
    if ((spImage1 == nullptr) || (spImage2 == nullptr)) {
        return;
    }

    ui::RichTextAttrTable attrs;

    //RT-21 添加图片：索引从1开始，相同图片+相同显示大小去重
    CHECK(attrs.AddImage(spImage1, 8, 8) == 1);
    CHECK(attrs.AddImage(spImage1, 8, 8) == 1);

    //RT-22 同一图片但显示大小不同：视为不同图片
    CHECK(attrs.AddImage(spImage1, 4, 4) == 2);

    //RT-23 不同图片：新增索引
    CHECK(attrs.AddImage(spImage2, 8, 8) == 3);

    //RT-24 获取图片信息
    const ui::RichTextImageInfo& image1 = attrs.GetImage(1);
    CHECK(image1.m_pImage == spImage1);
    CHECK(image1.m_nWidth == 8);
    CHECK(image1.m_nHeight == 8);
    CHECK(attrs.GetImage(4).m_pImage == nullptr);

    //RT-25 跨表比较图片
    ui::RichTextAttrTable other;
    CHECK(other.AddImage(spImage2, 8, 8) == 1);
    CHECK(other.AddImage(spImage1, 8, 8) == 2);
    CHECK(ui::RichTextAttrTable::IsImageEqual(attrs, 3, other, 1));
    CHECK(ui::RichTextAttrTable::IsImageEqual(attrs, 1, other, 2));
    CHECK(!ui::RichTextAttrTable::IsImageEqual(attrs, 1, other, 1));
    CHECK(!ui::RichTextAttrTable::IsImageEqual(attrs, 1, attrs, 2));

    //RT-26 清空后图片索引失效
    attrs.Release();
    CHECK(attrs.GetImage(1).m_pImage == nullptr);
}

// ---------------------------------------------------------------------------
// 3. RichTextDataList（RT-31 ~ RT-36）
// ---------------------------------------------------------------------------
void TestRichTextDataList()
{
    Section("TestRichTextDataList");
    const DStringW text = L"Hello, RichText!";

    ui::RichTextDataList dataList;
    CHECK(dataList.IsEmpty());
    CHECK(dataList.Size() == 0);

    ui::RichTextData data;
    data.m_textView = std::wstring_view(text.c_str(), 5);
    data.m_nFontIndex = 1;
    data.m_nTextColorIndex = 2;
    data.m_fLetterSpacing = 0.5f;
    dataList.PushBack(data);

    CHECK(!dataList.IsEmpty());
    CHECK(dataList.Size() == 1);
    CHECK(dataList[0].m_textView == L"Hello");
    CHECK(dataList[0].m_nFontIndex == 1);
    CHECK(dataList[0].m_nTextColorIndex == 2);
    CHECK_FLOAT(dataList[0].m_fLetterSpacing, 0.5f);

    //RT-33 Reserve 不改变数据个数
    dataList.Reserve(16);
    CHECK(dataList.Size() == 1);

    //RT-34 通过属性表管理字体/颜色索引
    ui::UiFontEx font;
    MakeFont(font, L"Arial", 14, false);
    CHECK(dataList.m_attrs.AddFont(&font) == 1);
    CHECK(dataList.m_attrs.AddColor(ui::UiColor(kBlueColor)) == 1);

    //RT-35 Swap：文本片段与属性表一起交换
    ui::RichTextDataList emptyList;
    dataList.Swap(emptyList);
    CHECK(dataList.IsEmpty());
    CHECK(emptyList.Size() == 1);
    CHECK(emptyList.m_attrs.GetFontCount() == 1);

    //RT-36 Clear 与 Release 都清空数据
    emptyList.Clear();
    CHECK(emptyList.IsEmpty());
    CHECK(emptyList.m_attrs.GetFontCount() == 0);

    dataList.PushBack(data);
    dataList.Release();
    CHECK(dataList.IsEmpty());
    CHECK(dataList.m_attrs.GetFontCount() == 0);
}

// ---------------------------------------------------------------------------
// 4. RichTextCharLayout（RT-41 ~ RT-60）
// ---------------------------------------------------------------------------
void TestRichTextCharLayout()
{
    Section("TestRichTextCharLayout");
    ui::RichTextCharLayout layout;

    //RT-41 空对象
    CHECK(layout.CharCount() == 0);
    CHECK(layout.Empty());
    CHECK(!layout.IsWideRow());
    CHECK_FLOAT(layout.CharX(0), 0.0f);
    CHECK_FLOAT(layout.CharWidth(0), 0.0f);
    CHECK(layout.CharFlag(0) == 0);
    CHECK(!layout.IsReturn(0));

    //RT-42 常规行：偏移、宽度与行宽
    layout.PushBack(0.0f, 10.5f, 0);
    layout.PushBack(10.5f, 9.75f, (uint8_t)(ui::RichTextCharFlag::kIsLowSurrogate |
                                            ui::RichTextCharFlag::kIsIgnoredChar));
    layout.PushBack(20.25f, 5.0f, (uint8_t)(ui::RichTextCharFlag::kIsReturn |
                                            ui::RichTextCharFlag::kIsNewLine));
    CHECK(layout.CharCount() == 3);
    CHECK(!layout.Empty());
    CHECK(!layout.IsWideRow());
    CHECK_FLOAT(layout.CharX(0), 0.0f);
    CHECK_FLOAT(layout.CharX(1), 10.5f);
    CHECK_FLOAT(layout.CharX(2), 20.25f);

    //RT-43 CharX(CharCount()) 返回行宽（最后一个字符的右边界）
    CHECK_FLOAT(layout.CharX(3), 25.25f);
    CHECK_FLOAT(layout.CharX(100), 25.25f);

    //RT-44 字符宽度
    CHECK_FLOAT(layout.CharWidth(0), 10.5f);
    CHECK_FLOAT(layout.CharWidth(1), 9.75f);
    CHECK_FLOAT(layout.CharWidth(2), 5.0f);
    CHECK_FLOAT(layout.CharWidth(3), 0.0f);

    //RT-45 字符属性标志
    CHECK(layout.CharFlag(0) == 0);
    CHECK(layout.CharFlag(1) == (uint8_t)(ui::RichTextCharFlag::kIsLowSurrogate |
                                          ui::RichTextCharFlag::kIsIgnoredChar));
    CHECK(layout.CharFlag(2) == (uint8_t)(ui::RichTextCharFlag::kIsReturn |
                                          ui::RichTextCharFlag::kIsNewLine));
    CHECK(layout.CharFlag(3) == 0);
    CHECK(layout.IsLowSurrogate(1));
    CHECK(layout.IsIgnoredChar(1));
    CHECK(!layout.IsIgnoredChar(0));
    CHECK(layout.IsReturn(2));
    CHECK(layout.IsNewLine(2));
    CHECK(!layout.IsNewLine(1));

    //RT-46 拷贝后的内容比较
    ui::RichTextCharLayout copy(layout);
    CHECK(copy == layout);
    copy.PushBack(30.0f, 1.0f, 0);
    CHECK(copy != layout);

    //RT-47 Clear 重置状态（包括超宽行标志）
    copy.Clear();
    CHECK(copy.Empty());
    CHECK(copy.CharCount() == 0);
    CHECK(!copy.IsWideRow());
    CHECK_FLOAT(copy.CharX(0), 0.0f);
    CHECK(copy.CharFlag(0) == 0);

    //RT-48 异常输入：负数与 NaN 均按 0 处理
    ui::RichTextCharLayout invalid;
    invalid.PushBack(-12.5f, 3.0f, 0);
    invalid.PushBack(std::numeric_limits<float>::quiet_NaN(), 3.0f, 0);
    CHECK_FLOAT(invalid.CharX(0), 0.0f);
    CHECK_FLOAT(invalid.CharX(1), 0.0f);

    //RT-49 超宽行：偏移超过 0xFFFF/8 像素后自动切换为 32 位存储
    ui::RichTextCharLayout wide;
    wide.PushBack(0.0f, 9000.0f, 0);
    CHECK(!wide.IsWideRow());
    wide.PushBack(9000.0f, 5.0f, 0);
    CHECK(wide.IsWideRow());
    wide.PushBack(9005.0f, 5.0f, (uint8_t)ui::RichTextCharFlag::kIsNewLine);
    CHECK(wide.CharCount() == 3);

    //RT-50 切换为超宽行后，之前保存的偏移仍然正确
    CHECK_FLOAT(wide.CharX(0), 0.0f);
    CHECK_FLOAT(wide.CharX(1), 9000.0f);
    CHECK_FLOAT(wide.CharX(2), 9005.0f);
    CHECK_FLOAT(wide.CharX(3), 9010.0f);
    CHECK_FLOAT(wide.CharWidth(1), 5.0f);
    CHECK(wide.IsNewLine(2));
    CHECK(wide.CharFlag(1) == 0);

    //RT-51 超宽行继续追加字符
    wide.PushBack(9010.0f, 7.0f, 0);
    CHECK(wide.CharCount() == 4);
    CHECK_FLOAT(wide.CharX(3), 9010.0f);
    CHECK_FLOAT(wide.CharWidth(3), 7.0f);
    CHECK_FLOAT(wide.CharX(4), 9017.0f);

    //RT-52 边界值：偏移刚好等于可表示上限（0xFFFF/8 像素）时仍使用常规存储
    ui::RichTextCharLayout boundary;
    const float fMaxCompactOffset = (float)ui::RichTextCharLayout::kOffsetMaxValue /
                                    (float)ui::RichTextCharLayout::kOffsetUnit;
    boundary.PushBack(fMaxCompactOffset, 1.0f, 0);
    CHECK(!boundary.IsWideRow());
    CHECK_FLOAT(boundary.CharX(0), fMaxCompactOffset);

    //RT-53 Reserve 不改变已有数据
    ui::RichTextCharLayout reserved;
    reserved.Reserve(64);
    CHECK(reserved.CharCount() == 0);
    reserved.PushBack(3.0f, 4.0f, 0);
    CHECK_FLOAT(reserved.CharX(0), 3.0f);
}

// ---------------------------------------------------------------------------
// 5. RichTextLineText / RichTextLineInfo（RT-61 ~ RT-68）
// ---------------------------------------------------------------------------
void TestRichTextLineText()
{
    Section("TestRichTextLineText");
    const std::shared_ptr<const ui::UiStringW> spText =
        std::make_shared<const ui::UiStringW>(ui::UiStringW(L"hello world"));

    //RT-61 默认构造：没有共享文本，data() 返回空字符串
    ui::RichTextLineText line;
    CHECK(line.data() != nullptr);
    CHECK(line.data()[0] == L'\0');

    //RT-62 共享整篇文本并指定起始偏移：data() 指向行首，行文本不保证以'\0'结尾
    line.SetShareText(spText, 6);
    CHECK(line.data()[0] == L'w');
    CHECK(std::wstring(line.data(), 5) == L"world");

    //RT-63 偏移等于文本长度时，指向字符串结尾的'\0'（长度为0的行）
    ui::RichTextLineText endLine;
    endLine.SetShareText(spText, 11);
    CHECK(endLine.data()[0] == L'\0');

    //RT-64 独立存储文本（复制内容），与共享文本互不影响
    ui::RichTextLineText copyLine;
    copyLine = std::wstring_view(L"independent");
    CHECK(copyLine.data() != line.data());
    CHECK(std::wstring(copyLine.data(), 11) == L"independent");

    //RT-65 拷贝构造共享同一份文本存储
    ui::RichTextLineText sharedLine(line);
    CHECK(sharedLine.data() == line.data());

    //RT-66 RichTextLineInfo：按长度截取本行文本
    ui::RichTextLineInfo lineInfo;
    lineInfo.m_lineText.SetShareText(spText, 6);
    lineInfo.m_nLineTextLen = 5;
    CHECK(lineInfo.GetLineTextView() == L"world");

    //RT-67 长度为0时返回空视图
    lineInfo.m_nLineTextLen = 0;
    CHECK(lineInfo.GetLineTextView().empty());

    //RT-68 独立存储文本后，行文本视图使用独立存储
    ui::RichTextLineInfo copyInfo;
    copyInfo.m_lineText = std::wstring_view(L"abc");
    copyInfo.m_nLineTextLen = 3;
    CHECK(copyInfo.GetLineTextView() == L"abc");
}

// ---------------------------------------------------------------------------
// 6. AutoClip（AC-01 ~ AC-08）
// ---------------------------------------------------------------------------
void TestAutoClip(ui::IRenderFactory* pRenderFactory)
{
    Section("TestAutoClip");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 80, 80);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;
    const ui::UiRectF rcFull(0.0f, 0.0f, 80.0f, 80.0f);

    //AC-01 矩形裁剪：作用域内只绘制裁剪区域，析构后恢复
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip clip(pRender.get(), ui::UiRect(0, 0, 40, 40));
        pRender->FillRect(rcFull, ui::UiColor(kRedColor));
    }
    CHECK(ReadPixel(pRender.get(), 20, 20, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 60, 60, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //AC-02 作用域结束后裁剪区域已恢复：可以绘制整个画布
    pRender->FillRect(rcFull, ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 60, 60, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //AC-03 bClip=false：不设置裁剪，作用域内外绘制范围一致
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip clip(pRender.get(), ui::UiRect(0, 0, 10, 10), false);
        pRender->FillRect(rcFull, ui::UiColor(kRedColor));
    }
    CHECK(ReadPixel(pRender.get(), 70, 70, nColor));
    CHECK(ColorNear(nColor, kRedColor));

    //AC-04 嵌套裁剪：内层析构后外层裁剪仍然有效
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip outer(pRender.get(), ui::UiRect(0, 0, 60, 60));
        {
            ui::AutoClip inner(pRender.get(), ui::UiRect(0, 0, 30, 30));
            pRender->FillRect(rcFull, ui::UiColor(kRedColor));
        }
        CHECK(ReadPixel(pRender.get(), 15, 15, nColor));
        CHECK(ColorNear(nColor, kRedColor));
        CHECK(ReadPixel(pRender.get(), 45, 45, nColor));
        CHECK(ColorNear(nColor, kWhiteColor));

        pRender->FillRect(rcFull, ui::UiColor(kGreenColor));
        CHECK(ReadPixel(pRender.get(), 15, 15, nColor));
        CHECK(ColorNear(nColor, kGreenColor));
        CHECK(ReadPixel(pRender.get(), 45, 45, nColor));
        CHECK(ColorNear(nColor, kGreenColor));
        CHECK(ReadPixel(pRender.get(), 70, 70, nColor));
        CHECK(ColorNear(nColor, kWhiteColor));
    }
    //外层析构后裁剪完全恢复
    pRender->FillRect(rcFull, ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 70, 70, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //AC-05 空矩形：产生空裁剪区域，不绘制任何内容，析构后恢复
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip clip(pRender.get(), ui::UiRect(10, 10, 10, 10));
        CHECK(pRender->IsClipEmpty());
        pRender->FillRect(rcFull, ui::UiColor(kRedColor));
    }
    CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    pRender->FillRect(rcFull, ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //AC-06 圆角矩形裁剪：圆角外不绘制，析构后恢复
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip clip(pRender.get(), ui::UiRect(0, 0, 80, 80), 30.0f, 30.0f);
        pRender->FillRect(rcFull, ui::UiColor(kRedColor));
    }
    CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 0, 0, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    pRender->FillRect(rcFull, ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 0, 0, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //AC-07 pRender 为 nullptr 且 bClip=false：空操作，不崩溃
    {
        ui::AutoClip clip(nullptr, ui::UiRect(0, 0, 10, 10), false);
        (void)clip;
    }

    //AC-08 同一渲染对象上多次顺序使用：每次都能正确恢复
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        ui::AutoClip clip1(pRender.get(), ui::UiRect(0, 0, 20, 20));
        pRender->FillRect(rcFull, ui::UiColor(kRedColor));
    }
    {
        ui::AutoClip clip2(pRender.get(), ui::UiRect(40, 40, 80, 80));
        pRender->FillRect(rcFull, ui::UiColor(kGreenColor));
    }
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 60, 60, nColor));
    CHECK(ColorNear(nColor, kGreenColor));
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
}

// ---------------------------------------------------------------------------
// 7. CanPresentBackBufferOnly（RT-91 ~ RT-94）
// ---------------------------------------------------------------------------
void TestCanPresentBackBufferOnly()
{
    Section("TestCanPresentBackBufferOnly");

    //RT-91 后备缓冲有效且内容未变化、无脏区域：可以只提交后备缓冲
    CHECK(ui::CanPresentBackBufferOnly(true, false, false));

    //RT-92 后备缓冲无效：不能只提交后备缓冲
    CHECK(!ui::CanPresentBackBufferOnly(false, false, false));

    //RT-93 界面内容发生变化：需要重新绘制
    CHECK(!ui::CanPresentBackBufferOnly(true, true, false));
    CHECK(!ui::CanPresentBackBufferOnly(true, false, true));

    //RT-94 内容变化且有脏区域
    CHECK(!ui::CanPresentBackBufferOnly(true, true, true));
}

} // namespace

int main(int /*argc*/, char* /*argv*/[])
{
    //关闭输出缓冲，便于程序异常时也能看到已产生的输出
    ::setvbuf(stdout, nullptr, _IONBF, 0);
#if defined(_WIN32) && defined(_DEBUG)
    //把断言输出重定向到标准错误，避免在无界面环境下弹出断言对话框导致进程挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif
    //测试不创建窗口，关闭全局动画避免启动定时器
    ui::GlobalManager::Instance().SetAnimationEnabled(false);

    //纯逻辑部分：不依赖渲染后端
    TestRichTextAttrTable();
    TestRichTextDataList();
    TestRichTextCharLayout();
    TestRichTextLineText();
    TestCanPresentBackBufferOnly();

    //依赖离屏渲染的部分
    const DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        std::cerr << "[FAIL] GlobalManager::Startup failed, resource path: "
                  << ui::StringConvert::UTF16ToUTF8(resourcePath.c_str(), resourcePath.size()) << std::endl;
        ++g_failedCount;
    }
    else {
        struct GlobalShutdown
        {
            ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
        } shutdownGuard;

        ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
        CHECK(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            TestRichTextAttrTableImages(pRenderFactory);
            TestAutoClip(pRenderFactory);
        }
    }

    std::cout << "render_core: " << g_checkCount << " checks, " << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "render_core: FAILED" << std::endl;
        return 1;
    }
    std::cout << "render_core: PASSED" << std::endl;
    return 0;
}
