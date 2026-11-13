// 文字绘制相关类的单元测试
//
// 测试对象：
//  1. ui::DrawSkiaText：文本归一化、单字符/整段文字测量、断行、布局与字形运行结果构建；
//  2. ui::TextGlyphRunList：run 的构建、空 run 丢弃、宽度累计；
//  3. ui::SkiaTextData：按编码枚举字符与转 UTF32；
//  4. ui::HorizontalDrawText / ui::VerticalDrawText：通过 ui::IRender 验证测量与绘制；
//  5. ui::DrawRichText：不换行文本超宽时的省略号截断与绘制缓存路径。
//
// 本测试使用离屏渲染，不创建窗口；需要 duilib 的资源目录（编译期宏
// DUILIB_TEST_RESOURCE_DIR，默认指向源码树的 bin/resources）。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/FontManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Render/IRender.h"
#include "duilib/RenderSkia/DrawSkiaText.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkiaTextData.h"
#include "duilib/RenderSkia/TextGlyphRun.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;
int32_t g_skipCount = 0;

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

/** 输出一条跳过信息（环境缺少指定字体时使用） */
void Skip(const char* reason)
{
    ++g_skipCount;
    std::cout << "[SKIP] " << reason << std::endl;
}

/** 浮点数近似相等 */
bool Near(float first, float second, float epsilon = 0.01f)
{
    return std::fabs(first - second) <= epsilon;
}

/** 构造字体回退函数（与绘制路径内部使用的实现一致） */
ui::FallbackFontCreator MakeFallbackCreator(ui::IFont* pFont)
{
    return [pFont](SkUnichar unicodeChar, SkGlyphID* glyphId) {
        return ui::DrawSkiaText::CreateFallbackFont(pFont, unicodeChar, glyphId);
        };
}

/** 字体相关的测试上下文（由 main 初始化） */
struct FontContext
{
    ui::IFont* pFont = nullptr;          //主字体，不含 CJK 字形（如 Arial）
    const SkFont* pSkFont = nullptr;     //主字体的Skia句柄
    const SkFont* pSecondSkFont = nullptr; //第二个Skia字体（用于构建多run场景）
    SkPaint paint;                       //绘制属性
    bool bCjkFallback = false;           //是否存在可用的CJK回退字体
};

/** 把 IFont 的当前属性转换为 UiFont */
ui::UiFont MakeUiFont(const ui::IFont* pFont)
{
    ui::UiFont fontInfo;
    fontInfo.m_fontName = pFont->FontName();
    fontInfo.m_fontSize = pFont->FontSize();
    fontInfo.m_bUnderline = pFont->IsUnderline();
    fontInfo.m_bStrikeOut = pFont->IsStrikeOut();
    fontInfo.m_bItalic = pFont->IsItalic();
    fontInfo.m_bBold = pFont->IsBold();
    return fontInfo;
}

/** 读取指定区域的像素，统计非白色像素个数（读取失败时返回-1） */
int32_t CountNonWhitePixels(ui::IRender* pRender, const ui::UiRect& rcRead)
{
    if ((pRender == nullptr) || (rcRead.Width() <= 0) || (rcRead.Height() <= 0)) {
        return -1;
    }
    std::vector<uint32_t> pixels((size_t)rcRead.Width() * (size_t)rcRead.Height());
    if (!pRender->ReadPixels(rcRead, pixels.data(), pixels.size() * sizeof(uint32_t))) {
        return -1;
    }
    int32_t nCount = 0;
    for (uint32_t pixel : pixels) {
        if (pixel != 0xFFFFFFFFu) {
            ++nCount;
        }
    }
    return nCount;
}

/** 读取整幅画面，统计非白色像素个数（绘制结果为空时返回0或-1） */
int32_t CountRenderedPixels(ui::IRender* pRender)
{
    if ((pRender == nullptr) || (pRender->GetWidth() <= 0) || (pRender->GetHeight() <= 0)) {
        return -1;
    }
    return CountNonWhitePixels(pRender, ui::UiRect(0, 0, pRender->GetWidth(), pRender->GetHeight()));
}

//////////////////////////////////////////////////////////////////////////
// 1. 文本归一化（不依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestUnicodeNormalize()
{
    //多行模式："\r\n" -> "\n"，单独的"\r"删除，"\t" -> 空格
    UTF32String textUTF32;
    ui::DrawSkiaText::GetDrawStringUTF32(_T("a\r\nb\rc\td"), false, textUTF32);
    CHECK(textUTF32 == UTF32String(U"a\nbc d"));

    //单行模式：所有换行符（含"\r\n"）都转换为空格
    ui::DrawSkiaText::GetDrawStringUTF32(_T("a\r\nb\rc\td\n"), true, textUTF32);
    CHECK(textUTF32 == UTF32String(U"a bc d "));

    //单行模式下只有"\r\n"时，也必须转换为空格（不能残留换行符）
    ui::DrawSkiaText::GetDrawStringUTF32(_T("x\r\ny"), true, textUTF32);
    CHECK(textUTF32 == UTF32String(U"x y"));

    //空字符串与无需处理的字符串
    ui::DrawSkiaText::GetDrawStringUTF32(_T(""), false, textUTF32);
    CHECK(textUTF32.empty());
    ui::DrawSkiaText::GetDrawStringUTF32(_T("plain"), false, textUTF32);
    CHECK(textUTF32 == UTF32String(U"plain"));

    //按编码转换：UTF8、UTF16、UTF32
    const std::string utf8Text = "\xE4\xB8\xAD" "A"; //"中A"
    UTF32String expectUTF32(U"\u4E2D" U"A");
    textUTF32 = ui::DrawSkiaText::GetDrawStringUTF32(utf8Text.data(), utf8Text.size(), SkTextEncoding::kUTF8);
    CHECK(textUTF32 == expectUTF32);

    const std::basic_string<DUTF16Char> utf16Text =
        ui::StringConvert::UTF8ToUTF16(utf8Text.c_str(), utf8Text.size());
    textUTF32 = ui::DrawSkiaText::GetDrawStringUTF32(utf16Text.data(), utf16Text.size() * sizeof(DUTF16Char),
                                                     SkTextEncoding::kUTF16);
    CHECK(textUTF32 == expectUTF32);

    textUTF32 = ui::DrawSkiaText::GetDrawStringUTF32(expectUTF32.data(),
                                                     expectUTF32.size() * sizeof(DUTF32Char),
                                                     SkTextEncoding::kUTF32);
    CHECK(textUTF32 == expectUTF32);

    //异常参数
    CHECK(ui::DrawSkiaText::GetDrawStringUTF32(nullptr, 0, SkTextEncoding::kUTF8).empty());
    CHECK(ui::DrawSkiaText::GetDrawStringUTF32(utf8Text.data(), 0, SkTextEncoding::kUTF8).empty());
}

//////////////////////////////////////////////////////////////////////////
// 2. SkiaTextData 的字符枚举与转UTF32（不依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestSkiaTextData()
{
    const std::string utf8Text = "A\xE4\xB8\xAD" "B"; //"A中B"，共5字节
    ui::SkiaTextData textData(utf8Text.data(), utf8Text.size(), SkTextEncoding::kUTF8);
    CHECK(!textData.IsEmpty());
    CHECK(textData.GetByteLength() == utf8Text.size());

    std::vector<uint32_t> chars;
    std::vector<size_t> charBytes;
    const bool bEnumOk = textData.EnumChars([&](SkUnichar unicodeChar, size_t charByteLength) {
        chars.push_back((uint32_t)unicodeChar);
        charBytes.push_back(charByteLength);
        return true;
        });
    CHECK(bEnumOk);
    CHECK(chars.size() == 3);
    CHECK(charBytes.size() == 3);
    if (chars.size() == 3) {
        CHECK(chars[0] == (uint32_t)U'A');
        CHECK(chars[1] == 0x4E2Du);
        CHECK(chars[2] == (uint32_t)U'B');
        CHECK(charBytes[1] == 3);
    }
    const UTF32String textUTF32 = textData.ToUTF32String();
    CHECK(textUTF32 == UTF32String(U"A\u4E2D" U"B"));

    //空文本
    ui::SkiaTextData emptyData;
    CHECK(emptyData.IsEmpty());
    CHECK(emptyData.ToUTF32String().empty());
}

//////////////////////////////////////////////////////////////////////////
// 3. TextGlyphRunList（不依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestTextGlyphRunList()
{
    ui::TextGlyphRunList glyphRuns;
    CHECK(glyphRuns.IsEmpty());
    CHECK(glyphRuns.GetTotalWidth() == 0.0f);

    //游标方式：BeginRun -> AddGlyph -> EndRun
    glyphRuns.BeginRun(nullptr, nullptr, 10.0f, 20.0f);
    glyphRuns.AddGlyph(11, 7.0f);
    glyphRuns.AddGlyph(12, 8.0f);
    glyphRuns.EndRun();
    CHECK(glyphRuns.GetRunCount() == 1);
    CHECK(Near(glyphRuns.GetTotalWidth(), 15.0f));
    if (glyphRuns.GetRunCount() == 1) {
        const ui::TextGlyphRun& run = glyphRuns.GetRun(0);
        CHECK(run.m_nCount == 2);
        CHECK(Near(run.m_x, 10.0f));
        CHECK(Near(run.m_y, 20.0f));
        CHECK(run.m_pGlyphs != nullptr);
        CHECK(run.m_pAdvances != nullptr);
        if ((run.m_pGlyphs != nullptr) && (run.m_pAdvances != nullptr) && (run.m_nCount == 2)) {
            CHECK(run.m_pGlyphs[0] == 11);
            CHECK(run.m_pGlyphs[1] == 12);
            CHECK(Near(run.m_pAdvances[0], 7.0f));
            CHECK(Near(run.m_pAdvances[1], 8.0f));
        }
    }

    //没有字形的run会被丢弃
    glyphRuns.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    glyphRuns.EndRun();
    CHECK(glyphRuns.GetRunCount() == 1);

    //未调用EndRun时，BeginRun会自动结束上一个run
    glyphRuns.BeginRun(nullptr, nullptr, 0.0f, 0.0f);
    glyphRuns.AddGlyph(21, 5.0f);
    glyphRuns.BeginRun(nullptr, nullptr, 5.0f, 0.0f);
    glyphRuns.AddGlyph(22, 6.0f);
    glyphRuns.EndRun();
    CHECK(glyphRuns.GetRunCount() == 3);
    CHECK(Near(glyphRuns.GetTotalWidth(), 26.0f));

    //整段方式：AddRun（数据会被拷贝）
    {
        const SkGlyphID glyphs[] = { 31, 32, 33 };
        const SkScalar advances[] = { 1.0f, 2.0f, 3.0f };
        glyphRuns.Clear();
        glyphRuns.AddRun(nullptr, nullptr, glyphs, advances, 3, 100.0f, 50.0f);
        CHECK(glyphRuns.GetRunCount() == 1);
        CHECK(Near(glyphRuns.GetTotalWidth(), 6.0f));
        if (glyphRuns.GetRunCount() == 1) {
            const ui::TextGlyphRun& run = glyphRuns.GetRun(0);
            CHECK(run.m_nCount == 3);
            if ((run.m_pGlyphs != nullptr) && (run.m_nCount == 3)) {
                CHECK(run.m_pGlyphs[2] == 33);
                CHECK((run.m_pAdvances != nullptr) && Near(run.m_pAdvances[2], 3.0f));
            }
        }
    }

    //Clear 后无有效run，但对象可继续复用
    glyphRuns.Clear();
    CHECK(glyphRuns.IsEmpty());
    CHECK(glyphRuns.GetTotalWidth() == 0.0f);
}

//////////////////////////////////////////////////////////////////////////
// 4. 单字符测量（依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestMeasureTextChar(const FontContext& ctx)
{
    //主字体支持ASCII字符：命中主字体，字形ID不为0
    SkRect bounds;
    SkGlyphID glyphId = 0;
    const SkFont* pHitSkFont = nullptr;
    SkScalar fWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, U'A', &bounds, &ctx.paint,
                                                        ctx.pFont, true, &pHitSkFont, &glyphId);
    CHECK(fWidth > 0.0f);
    CHECK(glyphId != 0);
    CHECK(pHitSkFont == ctx.pSkFont);
    CHECK(bounds.width() > 0.0f);
    CHECK(bounds.height() > 0.0f);

    //不支持字体回退时：无法绘制的字符宽度为0、字形ID为0、边界为空
    const DUTF32Char unsupportedChar = 0x10FFFD; //Unicode私有区补充字符，系统中通常没有字形
    CHECK(!ctx.pFont->IsUnicodeCharSupported(unsupportedChar));
    SkRect unsupportedBounds(1.0f, 2.0f, 3.0f, 4.0f);
    glyphId = 0x1234;
    pHitSkFont = nullptr;
    fWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, unsupportedChar, &unsupportedBounds, &ctx.paint,
                                               static_cast<const ui::IFont*>(nullptr), false,
                                               &pHitSkFont, &glyphId);
    CHECK(fWidth == 0.0f);
    CHECK(glyphId == 0);
    CHECK(pHitSkFont == ctx.pSkFont);
    CHECK(unsupportedBounds.isEmpty());

    //估算失败时使用默认字符（'A'）的宽度
    SkRect defaultBounds;
    const SkScalar fDefaultWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, unsupportedChar, &defaultBounds,
                                                                     &ctx.paint, static_cast<const ui::IFont*>(nullptr),
                                                                     true);
    SkRect aBounds;
    const SkScalar fAWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, U'A', &aBounds, &ctx.paint,
                                                               static_cast<const ui::IFont*>(nullptr), true);
    CHECK(fDefaultWidth > 0.0f);
    CHECK(Near(fDefaultWidth, fAWidth));

    //带字体回退：CJK字符应命中回退字体，且回退字形ID不为0
    if (ctx.bCjkFallback && !ctx.pFont->IsUnicodeCharSupported(0x5B57)) {
        SkRect cjkBounds;
        SkGlyphID cjkGlyphId = 0;
        const SkFont* pCjkHitFont = nullptr;
        const SkScalar fCjkWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, 0x5B57, &cjkBounds, &ctx.paint,
                                                                     ctx.pFont, true, &pCjkHitFont, &cjkGlyphId);
        CHECK(fCjkWidth > 0.0f);
        CHECK(cjkGlyphId != 0);
        CHECK(pCjkHitFont != nullptr);
        CHECK(cjkBounds.width() > 0.0f);
    }
    else {
        Skip("没有可用的CJK回退字体，跳过CJK回退字形测量用例");
    }

    //零宽字形（变体选择符 U+FE0F）：只要存在字形，就不能被判定为“无法绘制”
    SkGlyphID vsGlyphId = ctx.pSkFont->unicharToGlyph(0xFE0F);
    if (vsGlyphId == 0) {
        SkGlyphID fallbackVsGlyphId = 0;
        const SkFont* pVsFont = ui::DrawSkiaText::CreateFallbackFont(ctx.pFont, 0xFE0F, &fallbackVsGlyphId);
        if ((pVsFont != nullptr) && (fallbackVsGlyphId != 0)) {
            vsGlyphId = fallbackVsGlyphId;
        }
    }
    if (vsGlyphId != 0) {
        SkGlyphID measuredGlyphId = 0;
        const SkFont* pVsHitFont = nullptr;
        SkRect vsBounds;
        ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, 0xFE0F, &vsBounds, &ctx.paint,
                                          ctx.pFont, true, &pVsHitFont, &measuredGlyphId);
        CHECK(measuredGlyphId != 0);
    }
    else {
        Skip("没有字体提供变体选择符 U+FE0F 的字形，跳过零宽字形用例");
    }
}

//////////////////////////////////////////////////////////////////////////
// 5. 整段文本测量（依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestMeasureText(const FontContext& ctx)
{
    ui::MeasureTextTempData tempData;
    const std::string asciiText = "abc";

    //有回退支持与无回退支持的测量结果应一致（ASCII文本不触发回退）
    const SkScalar fWidthWithFallback = ui::DrawSkiaText::MeasureText(*ctx.pSkFont, asciiText.data(), asciiText.size(),
                                                                      SkTextEncoding::kUTF8, nullptr, &ctx.paint,
                                                                      ctx.pFont, tempData);
    const SkScalar fWidthNoFallback = ui::DrawSkiaText::MeasureText(*ctx.pSkFont, asciiText.data(), asciiText.size(),
                                                                    SkTextEncoding::kUTF8, nullptr, &ctx.paint,
                                                                    static_cast<const ui::IFont*>(nullptr), tempData);
    CHECK(fWidthWithFallback > 0.0f);
    CHECK(Near(fWidthWithFallback, fWidthNoFallback, 0.05f));

    //整段测量结果应等于逐字符测量之和
    SkScalar fSum = 0;
    for (char ch : asciiText) {
        fSum += ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, (DUTF32Char)(unsigned char)ch, nullptr,
                                                  &ctx.paint, ctx.pFont, true);
    }
    CHECK(Near(fWidthWithFallback, fSum, 0.05f));

    //混合文本：CJK字符由回退字体测量，宽度应大于纯ASCII部分的宽度
    const std::string asciiPart = "A";
    const std::string mixedUTF8 = "A\xE4\xB8\xAD" "B"; //"A中B"
    const SkScalar fAsciiWidth = ui::DrawSkiaText::MeasureText(*ctx.pSkFont, asciiPart.data(), asciiPart.size(),
                                                               SkTextEncoding::kUTF8, nullptr, &ctx.paint,
                                                               ctx.pFont, tempData);
    const SkScalar fMixedWidth = ui::DrawSkiaText::MeasureText(*ctx.pSkFont, mixedUTF8.data(), mixedUTF8.size(),
                                                               SkTextEncoding::kUTF8, nullptr, &ctx.paint,
                                                               ctx.pFont, tempData);
    CHECK(fMixedWidth > fAsciiWidth);
    if (ctx.bCjkFallback && !ctx.pFont->IsUnicodeCharSupported(0x5B57)) {
        SkScalar fCharSum = 0;
        const UTF32String mixedUTF32(U"A\u4E2D" U"B");
        for (DUTF32Char ch : mixedUTF32) {
            fCharSum += ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, ch, nullptr, &ctx.paint, ctx.pFont, true);
        }
        CHECK(Near(fMixedWidth, fCharSum, 0.05f));
    }
}

//////////////////////////////////////////////////////////////////////////
// 6. 断行（依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestBreakText(const FontContext& ctx)
{
    ui::MeasureTextTempData tempData;
    const ui::FallbackFontCreator fallbackFontCreator = MakeFallbackCreator(ctx.pFont);
    const std::string asciiText = "hello";

    //宽度足够：整段文本都可绘制
    {
        ui::BreakTextTempData breakTextData;
        breakTextData.m_bNeedGlyphFonts = true;
        SkScalar measuredWidth = -1;
        const size_t nBytes = ui::DrawSkiaText::BreakText(asciiText.data(), asciiText.size(), SkTextEncoding::kUTF8,
                                                          *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                          10000.0f, &measuredWidth, tempData, &breakTextData);
        CHECK(nBytes == asciiText.size());
        CHECK(measuredWidth > 0.0f);
        CHECK(breakTextData.glyphIDs.size() == asciiText.size());
        CHECK(breakTextData.glyphChars.size() == asciiText.size());
        CHECK(breakTextData.glyphWidths.size() == asciiText.size());
        CHECK(breakTextData.glyphFonts.size() == asciiText.size());

        size_t nSumBytes = 0;
        SkScalar fSumWidth = 0;
        const size_t nCount = breakTextData.glyphWidths.size();
        for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
            nSumBytes += breakTextData.glyphChars[nIndex];
            fSumWidth += breakTextData.glyphWidths[nIndex];
            CHECK(breakTextData.glyphIDs[nIndex] != 0);
            CHECK(breakTextData.glyphFonts[nIndex] == ctx.pSkFont);
        }
        CHECK(nSumBytes == nBytes);
        CHECK(Near(fSumWidth, measuredWidth));
    }

    //宽度不足：返回部分文本，且测量宽度不超过限制（至少保留一个字符）
    {
        const SkScalar fCharWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, U'h', nullptr, &ctx.paint,
                                                                      ctx.pFont, true);
        const SkScalar maxWidth = fCharWidth * 2.0f + 0.1f;
        ui::BreakTextTempData breakTextData;
        SkScalar measuredWidth = -1;
        const size_t nBytes = ui::DrawSkiaText::BreakText(asciiText.data(), asciiText.size(), SkTextEncoding::kUTF8,
                                                          *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                          maxWidth, &measuredWidth, tempData, &breakTextData);
        CHECK(nBytes > 0);
        CHECK(nBytes < asciiText.size());
        CHECK(measuredWidth <= maxWidth + 0.01f);
        CHECK(!breakTextData.glyphIDs.empty());
        CHECK(breakTextData.glyphIDs.size() == breakTextData.glyphChars.size());
        CHECK(breakTextData.glyphIDs.size() == breakTextData.glyphWidths.size());
    }

    //非法参数
    {
        SkScalar measuredWidth = 1.0f;
        CHECK(ui::DrawSkiaText::BreakText(nullptr, 10, SkTextEncoding::kUTF8, *ctx.pSkFont, fallbackFontCreator,
                                          ctx.paint, 100.0f, &measuredWidth, tempData) == 0);
        CHECK(measuredWidth == 0.0f);
        CHECK(ui::DrawSkiaText::BreakText(asciiText.data(), asciiText.size(), SkTextEncoding::kUTF8, *ctx.pSkFont,
                                          fallbackFontCreator, ctx.paint, 0.0f, &measuredWidth, tempData) == 0);
    }

    //回退字形：字形ID写回、命中字体记录为回退字体，且宽度与测量结果一致
    if (ctx.bCjkFallback && !ctx.pFont->IsUnicodeCharSupported(0x5B57)) {
        const std::string mixedUTF8 = "A\xE4\xB8\xAD" "B"; //"A中B"
        ui::BreakTextTempData breakTextData;
        breakTextData.m_bNeedGlyphFonts = true;
        SkScalar measuredWidth = 0;
        const size_t nBytes = ui::DrawSkiaText::BreakText(mixedUTF8.data(), mixedUTF8.size(), SkTextEncoding::kUTF8,
                                                          *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                          10000.0f, &measuredWidth, tempData, &breakTextData);
        CHECK(nBytes == mixedUTF8.size());
        CHECK(breakTextData.glyphIDs.size() == 3);
        CHECK(breakTextData.glyphFonts.size() == 3);
        if (breakTextData.glyphIDs.size() == 3) {
            CHECK(breakTextData.glyphFonts[0] == ctx.pSkFont);
            CHECK(breakTextData.glyphIDs[1] != 0);
            CHECK(breakTextData.glyphFonts[1] != ctx.pSkFont);
            CHECK(breakTextData.glyphWidths[1] > 0.0f);
        }
        SkScalar fSumWidth = 0;
        for (SkScalar fGlyphWidth : breakTextData.glyphWidths) {
            fSumWidth += fGlyphWidth;
        }
        CHECK(Near(fSumWidth, measuredWidth));
    }
    else {
        Skip("没有可用的CJK回退字体，跳过断行回退字形用例");
    }
}

//////////////////////////////////////////////////////////////////////////
// 7. 使用布局结果构建字形运行（依赖字体）
//////////////////////////////////////////////////////////////////////////

void TestBuildTextRuns(const FontContext& ctx)
{
    //空数据
    {
        ui::TextGlyphRunList glyphRuns;
        SkScalar fWidth = 123.0f;
        ui::DrawSkiaText::BuildTextRuns(nullptr, nullptr, nullptr, 0, glyphRuns, fWidth);
        CHECK(glyphRuns.IsEmpty());
        CHECK(fWidth == 0.0f);
    }

    //按字体分组：连续命中同一字体的字形合并为一个run
    {
        const SkFont* pFontA = ctx.pSkFont;
        const SkFont* pFontB = (ctx.pSecondSkFont != nullptr) ? ctx.pSecondSkFont : ctx.pSkFont;
        const SkGlyphID glyphIds[] = { 1, 2, 3, 4 };
        const SkScalar advances[] = { 1.0f, 2.0f, 3.0f, 4.0f };
        const SkFont* glyphFonts[] = { pFontA, pFontA, pFontB, pFontB };
        ui::TextGlyphRunList glyphRuns;
        SkScalar fWidth = 0;
        ui::DrawSkiaText::BuildTextRuns(glyphIds, advances, glyphFonts, 4, glyphRuns, fWidth);
        CHECK(Near(fWidth, 10.0f));
        CHECK(glyphRuns.GetRunCount() == 2);
        if (glyphRuns.GetRunCount() == 2) {
            CHECK(glyphRuns.GetRun(0).m_nCount == 2);
            CHECK(glyphRuns.GetRun(0).m_pSkFont == pFontA);
            CHECK(Near(glyphRuns.GetRun(0).m_x, 0.0f));
            CHECK(glyphRuns.GetRun(1).m_nCount == 2);
            CHECK(glyphRuns.GetRun(1).m_pSkFont == pFontB);
            CHECK(Near(glyphRuns.GetRun(1).m_x, 3.0f));
        }
    }

    //占位方框（字形ID为0）之后强制结束当前run，后续字形不能混入该run
    {
        const SkGlyphID glyphIds[] = { 5, 0, 6 };
        const SkScalar advances[] = { 1.0f, 2.0f, 3.0f };
        const SkFont* glyphFonts[] = { ctx.pSkFont, ctx.pSkFont, ctx.pSkFont };
        ui::TextGlyphRunList glyphRuns;
        SkScalar fWidth = 0;
        ui::DrawSkiaText::BuildTextRuns(glyphIds, advances, glyphFonts, 3, glyphRuns, fWidth);
        CHECK(Near(fWidth, 6.0f));
        CHECK(glyphRuns.GetRunCount() == 2);
        size_t nGlyphCount = 0;
        for (size_t nRun = 0; nRun < glyphRuns.GetRunCount(); ++nRun) {
            nGlyphCount += glyphRuns.GetRun(nRun).m_nCount;
        }
        CHECK(nGlyphCount == 3);
        if (glyphRuns.GetRunCount() == 2) {
            CHECK(glyphRuns.GetRun(0).m_nCount == 2);
            CHECK(glyphRuns.GetRun(1).m_nCount == 1);
        }
    }

    //字体为空的字形会被跳过，且不累计宽度
    {
        const SkGlyphID glyphIds[] = { 7, 8, 9 };
        const SkScalar advances[] = { 1.0f, 2.0f, 3.0f };
        const SkFont* glyphFonts[] = { ctx.pSkFont, nullptr, ctx.pSkFont };
        ui::TextGlyphRunList glyphRuns;
        SkScalar fWidth = 0;
        ui::DrawSkiaText::BuildTextRuns(glyphIds, advances, glyphFonts, 3, glyphRuns, fWidth);
        CHECK(Near(fWidth, 4.0f));
        size_t nGlyphCount = 0;
        for (size_t nRun = 0; nRun < glyphRuns.GetRunCount(); ++nRun) {
            nGlyphCount += glyphRuns.GetRun(nRun).m_nCount;
        }
        CHECK(nGlyphCount == 2);
    }
}

//////////////////////////////////////////////////////////////////////////
// 8. 文本布局（断行 + 行宽）
//////////////////////////////////////////////////////////////////////////

void TestLayoutText(const FontContext& ctx)
{
    const ui::FallbackFontCreator fallbackFontCreator = MakeFallbackCreator(ctx.pFont);
    const std::string text = "hello world";
    const SkScalar fCharWidth = ui::DrawSkiaText::MeasureTextChar(*ctx.pSkFont, U'h', nullptr, &ctx.paint,
                                                                  ctx.pFont, true);

    //宽度足够：只有一行，行宽与整段测量结果一致
    {
        ui::TextGlyphRunList glyphRuns;
        std::vector<ui::TextLineLayout> lineLayouts;
        const int32_t nLineCount = ui::DrawSkiaText::LayoutText(text.data(), text.size(), SkTextEncoding::kUTF8,
                                                                *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                                10000.0f, ui::TextBoxLineMode::kWordBreak_Mode,
                                                                glyphRuns, lineLayouts, 0, true);
        CHECK(nLineCount == 1);
        CHECK(lineLayouts.size() == 1);
        if (lineLayouts.size() == 1) {
            CHECK(lineLayouts[0].m_nByteLength == text.size());
            CHECK(lineLayouts[0].m_nTrailing == 0);
            CHECK(lineLayouts[0].m_fWidth > 0.0f);

            ui::MeasureTextTempData tempData;
            const SkScalar fMeasureWidth = ui::DrawSkiaText::MeasureText(*ctx.pSkFont, text.data(), text.size(),
                                                                         SkTextEncoding::kUTF8, nullptr, &ctx.paint,
                                                                         fallbackFontCreator, tempData);
            CHECK(Near(lineLayouts[0].m_fWidth, fMeasureWidth, 0.05f));
        }
        CHECK(!glyphRuns.IsEmpty());
    }

    //宽度不足：多行，每行宽度不超过限制，且各行字节数之和等于文本长度
    {
        const SkScalar maxWidth = fCharWidth * 5.0f;
        ui::TextGlyphRunList glyphRuns;
        std::vector<ui::TextLineLayout> lineLayouts;
        const int32_t nLineCount = ui::DrawSkiaText::LayoutText(text.data(), text.size(), SkTextEncoding::kUTF8,
                                                                *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                                maxWidth, ui::TextBoxLineMode::kWordBreak_Mode,
                                                                glyphRuns, lineLayouts, 0, true);
        CHECK(nLineCount >= 2);
        size_t nSumBytes = 0;
        for (const ui::TextLineLayout& lineLayout : lineLayouts) {
            nSumBytes += lineLayout.m_nByteLength;
            CHECK(lineLayout.m_fWidth <= maxWidth + 0.01f);
            CHECK(lineLayout.m_nRunCount > 0);
        }
        CHECK(nSumBytes == text.size());

        //回归用例：按行构建run时，不能清空前面行已生成的run，各行run范围必须连续
        size_t nExpectRunStart = 0;
        for (const ui::TextLineLayout& lineLayout : lineLayouts) {
            CHECK(lineLayout.m_nRunStart == nExpectRunStart);
            nExpectRunStart += lineLayout.m_nRunCount;
        }
        CHECK(nExpectRunStart == glyphRuns.GetRunCount());

        //不构建run时，行宽必须与构建run时一致
        ui::TextGlyphRunList glyphRuns2;
        std::vector<ui::TextLineLayout> lineLayouts2;
        const int32_t nLineCount2 = ui::DrawSkiaText::LayoutText(text.data(), text.size(), SkTextEncoding::kUTF8,
                                                                 *ctx.pSkFont, fallbackFontCreator, ctx.paint,
                                                                 maxWidth, ui::TextBoxLineMode::kWordBreak_Mode,
                                                                 glyphRuns2, lineLayouts2, 0, false);
        CHECK(nLineCount2 == nLineCount);
        CHECK(lineLayouts2.size() == lineLayouts.size());
        CHECK(glyphRuns2.IsEmpty());
        const size_t nCompareCount = std::min(lineLayouts.size(), lineLayouts2.size());
        for (size_t nIndex = 0; nIndex < nCompareCount; ++nIndex) {
            CHECK(lineLayouts[nIndex].m_nByteLength == lineLayouts2[nIndex].m_nByteLength);
            CHECK(Near(lineLayouts[nIndex].m_fWidth, lineLayouts2[nIndex].m_fWidth, 0.05f));
        }
    }

    //显式换行符：换行符计入行字节数，但不绘制（计入trailing）
    {
        const std::string newLineText = "a\nb";
        ui::TextGlyphRunList glyphRuns;
        std::vector<ui::TextLineLayout> lineLayouts;
        const int32_t nLineCount = ui::DrawSkiaText::LayoutText(newLineText.data(), newLineText.size(),
                                                                SkTextEncoding::kUTF8, *ctx.pSkFont,
                                                                fallbackFontCreator, ctx.paint, 10000.0f,
                                                                ui::TextBoxLineMode::kWordBreak_Mode,
                                                                glyphRuns, lineLayouts, 0, true);
        CHECK(nLineCount == 2);
        if (lineLayouts.size() == 2) {
            CHECK(lineLayouts[0].m_nByteLength == 2);
            CHECK(lineLayouts[0].m_nTrailing == 1);
            CHECK(lineLayouts[1].m_nByteLength == 1);
            CHECK(lineLayouts[1].m_nTrailing == 0);
        }
    }

    //单行模式：即使超出宽度，也只产出一行
    {
        const std::string longText = "hello world hello world";
        ui::TextGlyphRunList glyphRuns;
        std::vector<ui::TextLineLayout> lineLayouts;
        const int32_t nLineCount = ui::DrawSkiaText::LayoutText(longText.data(), longText.size(),
                                                                SkTextEncoding::kUTF8, *ctx.pSkFont,
                                                                fallbackFontCreator, ctx.paint, fCharWidth * 2.0f,
                                                                ui::TextBoxLineMode::kOneLine_Mode,
                                                                glyphRuns, lineLayouts, 0, true);
        CHECK(nLineCount == 1);
        if (lineLayouts.size() == 1) {
            CHECK(lineLayouts[0].m_nByteLength == longText.size());
        }
    }

    //非法参数
    {
        ui::TextGlyphRunList glyphRuns;
        std::vector<ui::TextLineLayout> lineLayouts;
        CHECK(ui::DrawSkiaText::LayoutText(nullptr, 10, SkTextEncoding::kUTF8, *ctx.pSkFont, fallbackFontCreator,
                                           ctx.paint, 100.0f, ui::TextBoxLineMode::kWordBreak_Mode,
                                           glyphRuns, lineLayouts, 0, true) == 0);
        CHECK(lineLayouts.empty());
    }
}

//////////////////////////////////////////////////////////////////////////
// 9. 横向/纵向文本的测量与绘制（经 IRender）
//////////////////////////////////////////////////////////////////////////

void TestHorizontalDrawText(ui::IRender* pRender, const FontContext& ctx)
{
    //默认横向路径（SkTextBox）的多行测量
    {
        ui::MeasureStringParam measureParam;
        measureParam.rectSize = 100;
        measureParam.pFont = ctx.pFont;
        measureParam.uFormat = ui::DrawStringFormat::TEXT_LEFT;
        const ui::UiRect rcText = pRender->MeasureString(_T("duilib font fallback text measure 多行测量"), measureParam);
        CHECK(rcText.Width() > 0);
        CHECK(rcText.Height() > 0);
        CHECK(rcText.Width() <= 100);
    }

    //多行文本绘制：每一行都必须有可见像素
    //（回归用例：按行构建字形run时，不能清空前面行已经生成的run，否则只有第一行能正常绘制）
    {
        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        ui::DrawStringParam drawParam;
        drawParam.textRect = ui::UiRect(0, 0, 120, 120);
        drawParam.dwTextColor = ui::UiColor(0xFF000000u);
        drawParam.pFont = ctx.pFont;
        drawParam.uFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_TOP;
        pRender->DrawString(_T("duilib text rendering multi line 多行文本绘制测试内容"), drawParam);
        CHECK(CountNonWhitePixels(pRender, ui::UiRect(0, 0, 120, 40)) > 0);
        CHECK(CountNonWhitePixels(pRender, ui::UiRect(0, 50, 120, 120)) > 0);
    }

    //两端对齐路径（HorizontalDrawText）：测量与绘制
    {
        ui::MeasureStringParam measureParam;
        measureParam.rectSize = 120;
        measureParam.pFont = ctx.pFont;
        measureParam.uFormat = ui::DrawStringFormat::TEXT_HJUSTIFY | ui::DrawStringFormat::TEXT_TOP;
        const ui::UiRect rcText = pRender->MeasureString(_T("duilib font fallback text justify 两端对齐"), measureParam);
        CHECK(rcText.Width() > 0);
        CHECK(rcText.Height() > 0);
        CHECK(rcText.Width() <= 120);

        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        ui::DrawStringParam drawParam;
        drawParam.textRect = ui::UiRect(0, 0, 120, 160);
        drawParam.dwTextColor = ui::UiColor(0xFF000000u);
        drawParam.pFont = ctx.pFont;
        drawParam.uFormat = ui::DrawStringFormat::TEXT_HJUSTIFY | ui::DrawStringFormat::TEXT_TOP;
        pRender->DrawString(_T("duilib font fallback text justify 两端对齐"), drawParam);
        CHECK(CountRenderedPixels(pRender) > 0);
    }

    //字间距路径（HorizontalDrawText）
    {
        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        ui::DrawStringParam drawParam;
        drawParam.textRect = ui::UiRect(0, 0, 300, 120);
        drawParam.dwTextColor = ui::UiColor(0xFF000000u);
        drawParam.pFont = ctx.pFont;
        drawParam.uFormat = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_TOP;
        drawParam.fWordSpacing = 2.0f;
        pRender->DrawString(_T("word spacing 字间距 绘制"), drawParam);
        CHECK(CountRenderedPixels(pRender) > 0);
    }
}

void TestVerticalDrawText(ui::IRender* pRender, const FontContext& ctx)
{
    //纵向测量：高度不超过限制，宽度与字体高度相关
    ui::MeasureStringParam measureParam;
    measureParam.rectSize = 200;
    measureParam.pFont = ctx.pFont;
    measureParam.uFormat = ui::DrawStringFormat::TEXT_VERTICAL | ui::DrawStringFormat::TEXT_LEFT;
    const ui::UiRect rcText = pRender->MeasureString(_T("duilib vertical text 竖排文本测量"), measureParam);
    CHECK(rcText.Width() > 0);
    CHECK(rcText.Height() > 0);
    CHECK(rcText.Height() <= 200);

    //纵向绘制：应产生可见像素
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    ui::DrawStringParam drawParam;
    drawParam.textRect = ui::UiRect(0, 0, 120, 200);
    drawParam.dwTextColor = ui::UiColor(0xFF000000u);
    drawParam.pFont = ctx.pFont;
    drawParam.uFormat = ui::DrawStringFormat::TEXT_VERTICAL | ui::DrawStringFormat::TEXT_LEFT |
                        ui::DrawStringFormat::TEXT_TOP;
    pRender->DrawString(_T("duilib vertical text 竖排文本绘制"), drawParam);
    CHECK(CountRenderedPixels(pRender) > 0);

    //单行纵向模式：只绘制一列
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    drawParam.uFormat |= ui::DrawStringFormat::TEXT_SINGLELINE;
    pRender->DrawString(_T("duilib vertical text 竖排文本绘制"), drawParam);
    const int32_t nSingleLinePixels = CountRenderedPixels(pRender);
    CHECK(nSingleLinePixels > 0);
}

//////////////////////////////////////////////////////////////////////////
// 10. 富文本路径：省略号截断与绘制缓存（经 IRender）
//////////////////////////////////////////////////////////////////////////

void TestRichText(ui::IRender* pRender, ui::IRenderFactory* pRenderFactory, const FontContext& ctx)
{
    //构造长文本（不自动换行，超出宽度时按省略号截断）
    const DString longText = _T("duilib rich text 富文本省略号 ellipsis 截断 字体回退 fallback 绘制 内容很长");
    ui::RichTextData richTextData;
    richTextData.m_textView = longText;
    richTextData.m_textColor = ui::UiColor(0xFF202020u);
    ui::SharePtr<ui::UiFontEx> pFontEx(new ui::UiFontEx);
    pFontEx->CopyFrom(MakeUiFont(ctx.pFont));
    richTextData.m_pFontInfo = pFontEx;
    richTextData.m_textStyle = ui::DrawStringFormat::TEXT_LEFT | ui::DrawStringFormat::TEXT_TOP;
    richTextData.m_bTextOverflowEllipsis = true;

    std::vector<ui::RichTextData> richTextDataList;
    richTextDataList.push_back(richTextData);

    const ui::UiRect rcText(0, 0, 160, 40);
    const ui::UiSize szScrollOffset(0, 0);

    //绘制缓存路径：不换行文本超宽时为省略号预留宽度，绘制区域不能超出文本矩形
    {
        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        std::shared_ptr<ui::DrawRichTextCache> spCache;
        const bool bCreated = pRender->CreateDrawRichTextCache(rcText, szScrollOffset, pRenderFactory,
                                                               richTextDataList, spCache);
        CHECK(bCreated);
        CHECK(spCache != nullptr);

        std::vector<std::vector<ui::UiRect>> textRects;
        textRects.resize(richTextDataList.size());
        const std::vector<int32_t> rowXOffset;
        pRender->DrawRichTextCacheData(spCache, rcText, szScrollOffset, rowXOffset, 255, &textRects);
        CHECK(textRects.size() == 1);
        if (textRects.size() == 1) {
            CHECK(!textRects[0].empty());
            for (const ui::UiRect& rcDraw : textRects[0]) {
                CHECK(rcDraw.left >= rcText.left);
                CHECK(rcDraw.right <= rcText.right + 1);
            }
        }
        CHECK(CountRenderedPixels(pRender) > 0);
    }

    //自动换行 + 字间距：绘制缓存路径同样要产生可见文字
    {
        ui::RichTextData wrapData = richTextData;
        wrapData.m_textStyle = ui::DrawStringFormat::TEXT_WORD_WRAP | ui::DrawStringFormat::TEXT_LEFT |
                               ui::DrawStringFormat::TEXT_TOP;
        wrapData.m_fLetterSpacing = 2.0f;
        std::vector<ui::RichTextData> wrapDataList;
        wrapDataList.push_back(wrapData);

        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        std::shared_ptr<ui::DrawRichTextCache> spCache;
        const bool bCreated = pRender->CreateDrawRichTextCache(rcText, szScrollOffset, pRenderFactory,
                                                               wrapDataList, spCache);
        CHECK(bCreated);

        std::vector<std::vector<ui::UiRect>> textRects;
        textRects.resize(wrapDataList.size());
        const std::vector<int32_t> rowXOffset;
        pRender->DrawRichTextCacheData(spCache, rcText, szScrollOffset, rowXOffset, 255, &textRects);
        for (const ui::UiRect& rcDraw : textRects[0]) {
            CHECK(rcDraw.right <= rcText.right + 1);
        }
        CHECK(CountRenderedPixels(pRender) > 0);
    }

    //普通自动换行（不加字间距）：多行文本的每一行都必须有可见像素
    //（回归点：富文本按行构建run时，前面行的run不能被清空，否则只有最后一行可见）
    {
        ui::RichTextData wrapData = richTextData;
        wrapData.m_textStyle = ui::DrawStringFormat::TEXT_WORD_WRAP | ui::DrawStringFormat::TEXT_LEFT |
                               ui::DrawStringFormat::TEXT_TOP;
        std::vector<ui::RichTextData> wrapDataList;
        wrapDataList.push_back(wrapData);

        //使用较窄的绘制区域，保证文本会换行成多行
        const ui::UiRect rcWrapText(0, 0, 90, 90);
        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        std::shared_ptr<ui::DrawRichTextCache> spCache;
        const bool bCreated = pRender->CreateDrawRichTextCache(rcWrapText, szScrollOffset, pRenderFactory,
                                                               wrapDataList, spCache);
        CHECK(bCreated);

        std::vector<std::vector<ui::UiRect>> textRects;
        textRects.resize(wrapDataList.size());
        const std::vector<int32_t> rowXOffset;
        pRender->DrawRichTextCacheData(spCache, rcWrapText, szScrollOffset, rowXOffset, 255, &textRects);
        CHECK(!textRects[0].empty());
        if (!textRects[0].empty()) {
            //绘制区域应分布在至少两行（每行有一个绘制矩形）
            int32_t nFirstTop = textRects[0][0].top;
            bool bHasOtherRow = false;
            for (const ui::UiRect& rcDraw : textRects[0]) {
                if (rcDraw.top != nFirstTop) {
                    bHasOtherRow = true;
                    break;
                }
            }
            CHECK(bHasOtherRow);
        }
        CHECK(CountNonWhitePixels(pRender, ui::UiRect(0, 0, 90, 24)) > 0);
        CHECK(CountNonWhitePixels(pRender, ui::UiRect(0, 24, 90, 48)) > 0);
    }

    //直接绘制路径（不使用缓存）：不崩溃且产生可见文字
    {
        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        pRender->DrawRichText(rcText, szScrollOffset, pRenderFactory, richTextDataList, 255, nullptr);
        CHECK(CountRenderedPixels(pRender) > 0);
    }

    //空文本：不绘制
    {
        std::vector<ui::RichTextData> emptyDataList;
        ui::RichTextData emptyData = richTextData;
        emptyData.m_textView = std::wstring_view();
        emptyDataList.push_back(emptyData);

        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
        pRender->DrawRichText(rcText, szScrollOffset, pRenderFactory, emptyDataList, 255, nullptr);
        CHECK(CountRenderedPixels(pRender) == 0);
    }
}

} // namespace

int main(int argc, char* argv[])
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

    DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    for (int nIndex = 1; nIndex + 1 < argc; ++nIndex) {
        if (::strcmp(argv[nIndex], "--resources") == 0) {
#ifdef _WIN32
            //命令行参数为本地编码，这里按 ANSI 转成宽字符
            const int nLength = ::MultiByteToWideChar(CP_ACP, 0, argv[nIndex + 1], -1, nullptr, 0);
            if (nLength > 0) {
                std::vector<wchar_t> buffer((size_t)nLength);
                ::MultiByteToWideChar(CP_ACP, 0, argv[nIndex + 1], -1, buffer.data(), nLength);
                resourcePath = buffer.data();
            }
#else
            resourcePath = argv[nIndex + 1];
#endif
        }
    }

    //不依赖字体的用例：始终执行
    TestUnicodeNormalize();
    TestSkiaTextData();
    TestTextGlyphRunList();

    //初始化全局资源（主题、字体、渲染工厂、字体回退管理器）
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        std::cerr << "[FAIL] GlobalManager::Startup failed, resource path: "
                  << ui::StringConvert::UTF16ToUTF8(resourcePath.c_str(), resourcePath.size()) << std::endl;
        ++g_failedCount;
    }
    else
    {
        struct GlobalShutdown
        {
            ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
        } shutdownGuard;

        ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
        CHECK(pRenderFactory != nullptr);
        if (pRenderFactory != nullptr) {
            //设置回退字体（只保留系统中存在的字体）
            ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
            DString fallbackFontNames;
            bool bHasCjkFallbackFont = false;
            const std::vector<DString> cjkFontNames = { _T("Microsoft YaHei"), _T("SimSun") };
            const std::vector<DString> fallbackCandidates = { _T("Microsoft YaHei"), _T("SimSun"),
                                                              _T("Segoe UI Emoji") };
            for (const DString& fontName : fallbackCandidates) {
                if (!pFontMgr->HasFontName(fontName)) {
                    continue;
                }
                if (!fallbackFontNames.empty()) {
                    fallbackFontNames += _T(",");
                }
                fallbackFontNames += fontName;
                if (std::find(cjkFontNames.begin(), cjkFontNames.end(), fontName) != cjkFontNames.end()) {
                    bHasCjkFallbackFont = true;
                }
            }
            ui::GlobalManager::Instance().Font().SetFallbackFontFamilyNames(fallbackFontNames);

            //创建主字体（优先选择不含CJK字形的字体，保证CJK字符必须走回退字体）
            std::unique_ptr<ui::IFont> pMainFont;
            bool bMainFontSupportCjk = true;
            const std::vector<DString> fontCandidates = { _T("Arial"), _T("Segoe UI"), _T("Tahoma"),
                                                          _T("DejaVu Sans"), _T("Microsoft YaHei") };
            for (const DString& fontName : fontCandidates) {
                if (!pFontMgr->HasFontName(fontName)) {
                    continue;
                }
                ui::IFont* pFont = pRenderFactory->CreateIFont();
                if (pFont == nullptr) {
                    continue;
                }
                ui::UiFont fontInfo;
                fontInfo.m_fontName = fontName;
                fontInfo.m_fontSize = 16;
                if (!pFont->InitFont(fontInfo)) {
                    delete pFont;
                    continue;
                }
                const bool bSupportCjk = pFont->IsUnicodeCharSupported(0x5B57);
                if ((pMainFont == nullptr) || !bSupportCjk) {
                    //释放上一个候选字体，采用当前字体
                    pMainFont.reset(pFont);
                    bMainFontSupportCjk = bSupportCjk;
                    if (!bSupportCjk) {
                        //找到一个不含CJK字形的字体，优先使用
                        break;
                    }
                }
                else {
                    delete pFont;
                }
            }
            CHECK(pMainFont != nullptr);

            //创建第二个字体（用于构建多run场景，字体句柄必须不同）
            std::unique_ptr<ui::IFont> pSecondFont;
            {
                ui::IFont* pFont = pRenderFactory->CreateIFont();
                if (pFont != nullptr) {
                    ui::UiFont fontInfo;
                    fontInfo.m_fontName = (pMainFont != nullptr) ? pMainFont->FontName() : _T("Arial");
                    fontInfo.m_fontSize = 16;
                    fontInfo.m_bBold = true;
                    if (pFont->InitFont(fontInfo)) {
                        pSecondFont.reset(pFont);
                    }
                    else {
                        delete pFont;
                    }
                }
            }

            if (pMainFont != nullptr) {
                ui::Font_Skia* pFontSkia = dynamic_cast<ui::Font_Skia*>(pMainFont.get());
                CHECK(pFontSkia != nullptr);
                if (pFontSkia != nullptr) {
                    const SkFont* pSkFont = pFontSkia->GetFontHandle();
                    CHECK(pSkFont != nullptr);

                    ui::Font_Skia* pSecondFontSkia = dynamic_cast<ui::Font_Skia*>(pSecondFont.get());
                    const SkFont* pSecondSkFont = (pSecondFontSkia != nullptr) ? pSecondFontSkia->GetFontHandle()
                                                                               : nullptr;

                    FontContext ctx;
                    ctx.pFont = pMainFont.get();
                    ctx.pSkFont = pSkFont;
                    ctx.pSecondSkFont = pSecondSkFont;
                    ctx.paint.setAntiAlias(true);
                    ctx.bCjkFallback = (!bMainFontSupportCjk && bHasCjkFallbackFont);

                    //离屏渲染对象（不创建窗口，使用内存位图作为绘制目标）
                    std::unique_ptr<ui::IRender> pRender(pRenderFactory->CreateRender(nullptr, nullptr));
                    CHECK(pRender != nullptr);
                    if (pRender != nullptr) {
                        CHECK(pRender->Resize(400, 300));
                        pRender->Clear(ui::UiColor(0xFFFFFFFFu));
                    }

                    TestMeasureTextChar(ctx);
                    TestMeasureText(ctx);
                    TestBreakText(ctx);
                    TestBuildTextRuns(ctx);
                    TestLayoutText(ctx);
                    if (pRender != nullptr) {
                        TestHorizontalDrawText(pRender.get(), ctx);
                        TestVerticalDrawText(pRender.get(), ctx);
                        TestRichText(pRender.get(), pRenderFactory, ctx);
                    }
                }
            }
        }
    }

    //输出测试结果
    std::cout << "text_draw: " << g_checkCount << " checks, "
              << g_failedCount << " failed, " << g_skipCount << " skipped" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "text_draw: FAILED" << std::endl;
        return 1;
    }
    std::cout << "text_draw: PASSED" << std::endl;
    return 0;
}
