// 文本绘制性能基准（覆盖全部文本测量/绘制路径）
//
// 用途：为"字体回退性能优化"提供可重复的量化基线，优化前后用同一份代码对比。
//
// 运行要求：
//   1) 必须使用 Release 配置构建与运行（Debug 的断言与未优化代码会影响绝对值）；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources），用于初始化主题与字体；
//   3) 基准文本中约 1/3 的字符由回退字体渲染（程序会校验并打印该比例，要求 >= 20%）。
//
// 用法：
//   bench_text_render [--iterations N] [--resources <资源目录>]
//   默认：iterations = 100，资源目录由编译期宏 DUILIB_BENCH_RESOURCE_DIR 指定。
//
// 输出的表格可直接粘贴到 docs/misc/字体回退性能优化分析.md 的性能基线章节。

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include <windows.h>
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace {

/** 基准测试的单个场景
*/
struct BenchCase
{
    /** 场景标识（英文，便于跨版本对比） */
    std::string m_name;

    /** 对应的绘制路径说明 */
    std::string m_path;

    /** 每帧处理的字符数（UTF-16 编码单元） */
    size_t m_nCharCount = 0;

    /** 执行一帧（与控件 OnPaint 的行为保持一致：测量 + 绘制） */
    std::function<void()> m_run;
};

/** 单个场景的计时结果
*/
struct BenchResult
{
    /** 总耗时（毫秒） */
    double m_totalMs = 0.0;

    /** 每帧耗时（微秒） */
    double m_usPerFrame = 0.0;

    /** 每千字符耗时（微秒） */
    double m_usPerKChar = 0.0;
};

/** 执行一个场景，返回计时结果
* @param [in] benchCase 场景定义
* @param [in] nWarmup 预热次数（不计入统计，用于填充字形与回退缓存）
* @param [in] nIterations 统计次数
*/
BenchResult RunCase(const BenchCase& benchCase, int nWarmup, int nIterations)
{
    for (int i = 0; i < nWarmup; ++i) {
        benchCase.m_run();
    }
    const auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < nIterations; ++i) {
        benchCase.m_run();
    }
    const auto endTime = std::chrono::steady_clock::now();

    BenchResult result;
    result.m_totalMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.m_usPerFrame = result.m_totalMs * 1000.0 / (double)nIterations;
    if (benchCase.m_nCharCount > 0) {
        result.m_usPerKChar = result.m_usPerFrame * 1000.0 / (double)benchCase.m_nCharCount;
    }
    return result;
}

/** 用 IFont 的当前属性构造 UiFont
*/
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

/** 创建主字体（优先选择"不含 CJK 字形"的字体，保证基准文本中的 CJK 字符必须走回退字体）
*/
ui::IFont* CreatePrimaryFont(ui::IRenderFactory* pRenderFactory)
{
    ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    std::vector<DString> fontNames = { _T("Arial"), _T("Segoe UI"), _T("Tahoma"), _T("Microsoft YaHei") };

    ui::IFont* pRetFont = nullptr;
    for (const DString& fontName : fontNames) {
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
        //ASCII 字符必须支持，CJK 字符必须不支持（否则无法保证回退字体参与绘制）
        if (pFont->IsUnicodeCharSupported((uint32_t)L'A') &&
            !pFont->IsUnicodeCharSupported((uint32_t)L'\u5B57')) {
            pRetFont = pFont;
            break;
        }
        delete pFont;
    }
    return pRetFont;
}

/** 计算文本中"当前字体不支持、需要回退字体渲染"的字符比例
*/
double CalcFallbackRatio(ui::IFont* pFont, const DString& strText)
{
    size_t nTotalCount = 0;
    size_t nFallbackCount = 0;
    for (wchar_t ch : strText) {
        if ((ch == L'\r') || (ch == L'\n')) {
            continue;
        }
        ++nTotalCount;
        if (!pFont->IsUnicodeCharSupported((uint32_t)ch)) {
            ++nFallbackCount;
        }
    }
    if (nTotalCount == 0) {
        return 0.0;
    }
    return (double)nFallbackCount / (double)nTotalCount;
}

/** 构造基准文本：ASCII 与 CJK 交替，CJK 占比约 1/3
*/
DString MakeMixedText(int nLines)
{
    const DString lineText =
        _T("duilib render 字体回退 text 生僻字符 glyph 绘制路径 fallback 性能评估 benchmark ");
    DString strText;
    for (int i = 0; i < nLines; ++i) {
        if (!strText.empty()) {
            strText += _T("\n");
        }
        strText += lineText;
    }
    return strText;
}

/** 把宽字符串转成 UTF-8（用于控制台输出）
*/
std::string ToUTF8(const DString& strText)
{
    return ui::StringConvert::UTF16ToUTF8(strText.c_str(), strText.length());
}

} // namespace

int main(int argc, char* argv[])
{
    //关闭输出缓冲，便于在基准程序异常时也能看到已产生的输出
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    int nIterations = 100;
    DString resourcePath = _T(DUILIB_BENCH_RESOURCE_DIR);
    for (int i = 1; i + 1 < argc; ++i) {
        if (::strcmp(argv[i], "--iterations") == 0) {
            nIterations = ::atoi(argv[i + 1]);
        }
        else if (::strcmp(argv[i], "--resources") == 0) {
#ifdef DUILIB_BUILD_FOR_WIN
            //命令行参数为本地编码，这里按 ANSI 转成宽字符
            const int nLength = ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, nullptr, 0);
            if (nLength > 0) {
                std::vector<wchar_t> buffer((size_t)nLength);
                ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, buffer.data(), nLength);
                resourcePath = buffer.data();
            }
#else
            resourcePath = argv[i + 1];
#endif
        }
    }
    if (nIterations < 1) {
        nIterations = 1;
    }

    //初始化全局资源（主题、字体、渲染工厂、字体回退管理器）
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        ::printf("[error] GlobalManager::Startup failed, resource path: %s\n", ToUTF8(resourcePath).c_str());
        return 1;
    }
    //注册退出清理
    struct GlobalShutdown
    {
        ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
    } shutdownGuard;

    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        ::printf("[error] GetRenderFactory failed\n");
        return 1;
    }

    //创建离屏渲染对象（不创建窗口，直接使用内存位图作为绘制目标）
    const int32_t nRenderWidth = 800;
    const int32_t nRenderHeight = 600;
    std::unique_ptr<ui::IRender> pRender(pRenderFactory->CreateRender(nullptr, nullptr));
    if ((pRender == nullptr) || !pRender->Resize(nRenderWidth, nRenderHeight)) {
        ::printf("[error] create render failed\n");
        return 1;
    }
    pRender->Clear(ui::UiColor(0xFFFFFFFF));

    //设置回退字体列表（只保留系统中存在的字体）
    ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    DString fallbackFontNames;
    const std::vector<DString> fallbackCandidates = { _T("Microsoft YaHei"), _T("SimSun"), _T("Segoe UI Emoji") };
    for (const DString& fontName : fallbackCandidates) {
        if (!pFontMgr->HasFontName(fontName)) {
            continue;
        }
        if (!fallbackFontNames.empty()) {
            fallbackFontNames += _T(",");
        }
        fallbackFontNames += fontName;
    }
    ui::GlobalManager::Instance().Font().SetFallbackFontFamilyNames(fallbackFontNames);

    //创建主字体（不含 CJK 字形，确保基准文本中的 CJK 字符必须走回退字体）
    std::unique_ptr<ui::IFont> pFont(CreatePrimaryFont(pRenderFactory));
    if (pFont == nullptr) {
        ::printf("[error] create primary font failed\n");
        return 1;
    }

    //基准文本
    const DString strSingleLine = MakeMixedText(1);
    const DString strMultiLine = MakeMixedText(6);
    const double fallbackRatio = CalcFallbackRatio(pFont.get(), strMultiLine);

    //输出运行环境与输入说明
    ::printf("==== text render benchmark ====\n");
#ifdef _DEBUG
    ::printf("build config      : Debug\n");
#else
    ::printf("build config      : Release\n");
#endif
    ::printf("render size       : %d x %d (raster, offscreen)\n", nRenderWidth, nRenderHeight);
    ::printf("primary font      : %s, 16px\n", ToUTF8(pFont->FontName()).c_str());
    ::printf("fallback fonts    : %s\n", ToUTF8(fallbackFontNames).c_str());
    ::printf("text length       : single line %d chars, multi line %d chars\n",
        (int)strSingleLine.length(), (int)strMultiLine.length());
    ::printf("fallback ratio    : %.1f%% (require >= 20%%)\n", fallbackRatio * 100.0);
    ::printf("iterations        : %d (warmup 20)\n\n", nIterations);
    if (fallbackRatio < 0.20) {
        ::printf("[error] fallback ratio is lower than 20%%, benchmark is invalid\n");
        return 2;
    }

    //富文本内容（与普通文本使用相同的字符集，部分段落为粗体）
    const DString strRichSegment1 = _T("duilib render 字体回退 text ");
    const DString strRichSegment2 = _T("生僻字符 glyph 绘制路径 ");
    const DString strRichSegment3 = _T("fallback 性能评估 benchmark 生僻字符 ");
    const DString strRichSegment4 = _T("render 字体回退 text 绘制路径 ");
    ui::SharePtr<ui::UiFontEx> pNormalFontEx(new ui::UiFontEx);
    pNormalFontEx->CopyFrom(MakeUiFont(pFont.get()));
    ui::SharePtr<ui::UiFontEx> pBoldFontEx(new ui::UiFontEx);
    pBoldFontEx->CopyFrom(MakeUiFont(pFont.get()));
    pBoldFontEx->m_bBold = true;

    const ui::UiColor textColor(0xFF202020u);
    std::vector<ui::RichTextData> richTextData;
    richTextData.resize(4);
    richTextData[0].m_textView = strRichSegment1;
    richTextData[0].m_textColor = textColor;
    richTextData[0].m_pFontInfo = pNormalFontEx;
    richTextData[1].m_textView = strRichSegment2;
    richTextData[1].m_textColor = textColor;
    richTextData[1].m_pFontInfo = pBoldFontEx;
    richTextData[2].m_textView = strRichSegment3;
    richTextData[2].m_textColor = textColor;
    richTextData[2].m_pFontInfo = pNormalFontEx;
    richTextData[3].m_textView = strRichSegment4;
    richTextData[3].m_textColor = textColor;
    richTextData[3].m_pFontInfo = pBoldFontEx;

    //带字间距的富文本（覆盖富文本的字间距分支）
    std::vector<ui::RichTextData> richTextDataSpacing = richTextData;
    for (ui::RichTextData& richText : richTextDataSpacing) {
        richText.m_fLetterSpacing = 2.0f;
    }

    const ui::UiRect rcSingleLine(10, 10, 780, 40);
    const ui::UiRect rcMultiLine(10, 10, 610, 500);
    const ui::UiRect rcEllipsis(10, 10, 260, 40);
    const ui::UiRect rcVertical(10, 10, 250, 580);
    const ui::UiRect rcRichText(10, 10, 610, 500);
    const ui::UiSize szScrollOffset(0, 0);
    const std::vector<int32_t> rowXOffset;
    const size_t nRichCharCount = strRichSegment1.length() + strRichSegment2.length() +
        strRichSegment3.length() + strRichSegment4.length();

    //构造各绘制路径的场景
    std::vector<BenchCase> benchCaseList;

    //P1：默认横向路径（SkTextBox），单行测量
    {
        BenchCase benchCase;
        benchCase.m_name = "single_line_measure";
        benchCase.m_path = "P1 默认横向-单行测量";
        benchCase.m_nCharCount = strSingleLine.length();
        benchCase.m_run = [&]() {
            ui::MeasureStringParam measureParam;
            measureParam.rectSize = rcSingleLine.Width();
            measureParam.pFont = pFont.get();
            measureParam.uFormat = ui::TEXT_LEFT | ui::TEXT_SINGLELINE;
            pRender->MeasureString(strSingleLine, measureParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P1：默认横向路径，单行绘制
    {
        BenchCase benchCase;
        benchCase.m_name = "single_line_draw";
        benchCase.m_path = "P1 默认横向-单行绘制";
        benchCase.m_nCharCount = strSingleLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcSingleLine;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_LEFT | ui::TEXT_SINGLELINE;
            pRender->DrawString(strSingleLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P7：多行测量（CountLines + 逐行 MeasureText）
    {
        BenchCase benchCase;
        benchCase.m_name = "multi_line_measure";
        benchCase.m_path = "P7 多行文本测量";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::MeasureStringParam measureParam;
            measureParam.rectSize = rcMultiLine.Width();
            measureParam.pFont = pFont.get();
            measureParam.uFormat = ui::TEXT_LEFT;
            pRender->MeasureString(strMultiLine, measureParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P1：默认横向多行绘制
    {
        BenchCase benchCase;
        benchCase.m_name = "multi_line_draw";
        benchCase.m_path = "P1 默认横向-多行绘制";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcMultiLine;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_LEFT | ui::TEXT_TOP;
            pRender->DrawString(strMultiLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P1：水平/垂直居中（触发 CountLines + 逐行 Linebreak + 逐行 MeasureText）
    {
        BenchCase benchCase;
        benchCase.m_name = "hcenter_vcenter_draw";
        benchCase.m_path = "P1 默认横向-居中绘制";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcMultiLine;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_HCENTER | ui::TEXT_VCENTER;
            pRender->DrawString(strMultiLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P1：省略号（宽度不足，触发 MeasureText + EllipsisText）
    {
        BenchCase benchCase;
        benchCase.m_name = "ellipsis_draw";
        benchCase.m_path = "P1 默认横向-省略号";
        benchCase.m_nCharCount = strSingleLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcEllipsis;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_LEFT | ui::TEXT_TOP | ui::TEXT_SINGLELINE | ui::TEXT_END_ELLIPSIS;
            pRender->DrawString(strSingleLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P2：两端对齐（HorizontalDrawText，逐字符测量 + 逐字符绘制）
    {
        BenchCase benchCase;
        benchCase.m_name = "justify_draw";
        benchCase.m_path = "P2 两端对齐绘制";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcMultiLine;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_HJUSTIFY | ui::TEXT_TOP;
            pRender->DrawString(strMultiLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P2：字间距（HorizontalDrawText，逐字符测量 + 逐字符绘制）
    {
        BenchCase benchCase;
        benchCase.m_name = "word_spacing_draw";
        benchCase.m_path = "P2 字间距绘制";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcMultiLine;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_LEFT | ui::TEXT_TOP;
            drawParam.fWordSpacing = 2.0f;
            pRender->DrawString(strMultiLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P3：纵向绘制（VerticalDrawText，逐字符测量 + 逐字符绘制 + 旋转）
    {
        BenchCase benchCase;
        benchCase.m_name = "vertical_draw";
        benchCase.m_path = "P3 纵向绘制";
        benchCase.m_nCharCount = strMultiLine.length();
        benchCase.m_run = [&]() {
            ui::DrawStringParam drawParam;
            drawParam.textRect = rcVertical;
            drawParam.dwTextColor = textColor;
            drawParam.pFont = pFont.get();
            drawParam.uFormat = ui::TEXT_VERTICAL | ui::TEXT_LEFT | ui::TEXT_TOP;
            pRender->DrawString(strMultiLine, drawParam);
        };
        benchCaseList.push_back(benchCase);
    }

    //P4：富文本布局（MeasureRichText）
    {
        BenchCase benchCase;
        benchCase.m_name = "rich_text_measure";
        benchCase.m_path = "P4 富文本布局";
        benchCase.m_nCharCount = nRichCharCount;
        benchCase.m_run = [&]() {
            pRender->MeasureRichText(rcRichText, szScrollOffset, pRenderFactory, richTextData, nullptr);
        };
        benchCaseList.push_back(benchCase);
    }

    //P4/P5：富文本直接绘制（无绘制缓存）
    {
        BenchCase benchCase;
        benchCase.m_name = "rich_text_draw";
        benchCase.m_path = "P4 富文本直接绘制";
        benchCase.m_nCharCount = nRichCharCount;
        benchCase.m_run = [&]() {
            pRender->DrawRichText(rcRichText, szScrollOffset, pRenderFactory, richTextData, 255, nullptr);
        };
        benchCaseList.push_back(benchCase);
    }

    //P5：富文本创建绘制缓存（每次重建布局）
    {
        BenchCase benchCase;
        benchCase.m_name = "rich_text_cache_create";
        benchCase.m_path = "P5 富文本缓存创建";
        benchCase.m_nCharCount = nRichCharCount;
        benchCase.m_run = [&]() {
            std::shared_ptr<ui::DrawRichTextCache> spDrawRichTextCache;
            pRender->CreateDrawRichTextCache(rcRichText, szScrollOffset, pRenderFactory,
                richTextData, spDrawRichTextCache);
        };
        benchCaseList.push_back(benchCase);
    }

    //P5：富文本使用已有缓存绘制（缓存有效时的稳态开销）
    {
        std::shared_ptr<ui::DrawRichTextCache> spDrawRichTextCache;
        pRender->CreateDrawRichTextCache(rcRichText, szScrollOffset, pRenderFactory,
            richTextData, spDrawRichTextCache);
        BenchCase benchCase;
        benchCase.m_name = "rich_text_cached_draw";
        benchCase.m_path = "P5 富文本缓存绘制";
        benchCase.m_nCharCount = nRichCharCount;
        //缓存对象按值捕获，其生命周期由基准用例自身持有
        benchCase.m_run = [&, spDrawRichTextCache]() {
            pRender->DrawRichTextCacheData(spDrawRichTextCache, rcRichText, szScrollOffset,
                rowXOffset, 255, nullptr);
        };
        benchCaseList.push_back(benchCase);
    }

    //P6：富文本字间距分支（逐字符测量 + 逐字符绘制）
    {
        BenchCase benchCase;
        benchCase.m_name = "rich_text_letter_spacing";
        benchCase.m_path = "P6 富文本字间距绘制";
        benchCase.m_nCharCount = nRichCharCount;
        benchCase.m_run = [&]() {
            pRender->DrawRichText(rcRichText, szScrollOffset, pRenderFactory,
                richTextDataSpacing, 255, nullptr);
        };
        benchCaseList.push_back(benchCase);
    }

    //依次执行并输出结果
    ::printf("| 场景 | 绘制路径 | 字符数 | 总耗时(ms) | 每帧(us) | 每千字符(us) |\n");
    ::printf("| --- | --- | --- | --- | --- | --- |\n");
    for (const BenchCase& benchCase : benchCaseList) {
        const BenchResult result = RunCase(benchCase, 20, nIterations);
        ::printf("| %s | %s | %d | %.2f | %.2f | %.2f |\n",
            benchCase.m_name.c_str(),
            benchCase.m_path.c_str(),
            (int)benchCase.m_nCharCount,
            result.m_totalMs,
            result.m_usPerFrame,
            result.m_usPerKChar);
    }
    ::printf("\n");
    return 0;
}
