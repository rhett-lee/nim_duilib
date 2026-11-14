// RichEdit2 文本绘制性能基准
//
// 用途：为 RichEdit2 自绘编辑框的文本绘制流程提供可重复的量化基线，
//       覆盖"含简体中文+英文、不含 emoji"与"含大量 emoji（触发字体回退）"两类文本，
//       以及"缓存有效的稳态绘制"、"文本变化后的重新排版绘制"、"滚动绘制"三种典型场景。
//
// 运行要求：
//   1) 必须使用 Release 配置构建与运行（Debug 的断言与未优化代码会影响绝对值）；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources）；
//   3) 使用离屏渲染 + 轻量窗口桩对象驱动 RichEdit2::Paint，不需要真实窗口。
//
// 用法：
//   bench_richedit2 [--iterations N] [--resources <资源目录>]
//   默认：iterations = 200，资源目录由编译期宏 DUILIB_BENCH_RESOURCE_DIR 指定。

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/RichEdit2.h"
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

/** 轻量窗口桩对象：只提供离屏渲染对象，避免基准对真实窗口的依赖
*/
class BenchWindow : public ui::Window
{
public:
    explicit BenchWindow(ui::IRender* pRender) : m_pBenchRender(pRender) {}
    virtual ui::IRender* GetRender() const override { return m_pBenchRender; }
private:
    ui::IRender* m_pBenchRender;
};

/** 基准用的 RichEdit2：把 protected 的 Paint 暴露出来，便于直接驱动绘制流程
*/
class BenchRichEdit2 : public ui::RichEdit2
{
public:
    explicit BenchRichEdit2(ui::Window* pWindow) : ui::RichEdit2(pWindow) {}
    using ui::RichEdit2::Paint;
};

/** 基准测试的单个场景
*/
struct BenchCase
{
    /** 场景标识（英文，便于跨版本对比） */
    std::string m_name;

    /** 场景说明 */
    std::string m_desc;

    /** 文档字符数（UTF-16 编码单元） */
    size_t m_nCharCount = 0;

    /** 预热次数 */
    int m_nWarmup = 20;

    /** 统计次数 */
    int m_nIterations = 200;

    /** 场景开始前的准备动作（例如设置文本、完成首次排版） */
    std::function<void()> m_setup;

    /** 执行一帧绘制 */
    std::function<void()> m_run;
};

/** 单个场景的计时结果
*/
struct BenchResult
{
    double m_totalMs = 0.0;
    double m_usPerFrame = 0.0;
    double m_usPerKChar = 0.0;
};

/** 执行一个场景，返回计时结果
*/
BenchResult RunCase(const BenchCase& benchCase)
{
    if (benchCase.m_setup) {
        benchCase.m_setup();
    }
    for (int i = 0; i < benchCase.m_nWarmup; ++i) {
        benchCase.m_run();
    }
    const auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < benchCase.m_nIterations; ++i) {
        benchCase.m_run();
    }
    const auto endTime = std::chrono::steady_clock::now();

    BenchResult result;
    result.m_totalMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.m_usPerFrame = result.m_totalMs * 1000.0 / (double)benchCase.m_nIterations;
    if (benchCase.m_nCharCount > 0) {
        result.m_usPerKChar = result.m_usPerFrame * 1000.0 / (double)benchCase.m_nCharCount;
    }
    return result;
}

/** 构造一行中文+英文字符（不含 emoji）
*/
const DStringW& GetZhEnLine()
{
    static const DStringW lineText =
        L"Duilib 文本绘制性能评估：简体中文与 English mixed text 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return lineText;
}

/** 构造一行包含大量 emoji 的文本（emoji 为增补平面字符，需字体回退）
*/
const DStringW& GetEmojiSuffix()
{
    static const DStringW emojiText =
        L"😀😁😂🤣😃😄😅😆😉😊😋😎😍😘🥰😗😙😚🙂🤗🤩🤔🤨😐😑😶🙄😏😣😥😮🤐😯😪😫😴😌😛😜😝🤤😒😓😔😕🙃🤑😲🙁😖😞😟😤😢😭😦😧😨😩🤯😬😰😱🥵🥶😳🤪😵🥴😠😡🤬😷🤒🤕🤢🤮🤧😇🥳🥺🤠🤡🤥🤫🤭🧐🤓";
    return emojiText;
}

/** 构造测试文本：每行中文+英文，可选追加大量 emoji
*/
DStringW MakeText(int nLines, bool bEmoji)
{
    const DStringW& lineText = GetZhEnLine();
    const DStringW& emojiText = GetEmojiSuffix();
    DStringW strText;
    strText.reserve((size_t)nLines * (lineText.size() + (bEmoji ? emojiText.size() + 2 : 1)));
    for (int i = 0; i < nLines; ++i) {
        if (!strText.empty()) {
            strText += L'\n';
        }
        strText += lineText;
        if (bEmoji) {
            strText += L' ';
            strText += emojiText;
        }
    }
    return strText;
}

/** 把宽字符串转成 UTF-8（用于控制台输出）
*/
std::string ToUTF8(const DString& strText)
{
    return ui::StringConvert::UTF16ToUTF8(strText.c_str(), strText.length());
}

/** 创建主字体（用于统计 emoji 的回退字符比例）
*/
ui::IFont* CreateFontForCheck(ui::IRenderFactory* pRenderFactory, const DString& fontName)
{
    ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    if (!pFontMgr->HasFontName(fontName)) {
        return nullptr;
    }
    ui::IFont* pFont = pRenderFactory->CreateIFont();
    if (pFont == nullptr) {
        return nullptr;
    }
    ui::UiFont fontInfo;
    fontInfo.m_fontName = fontName;
    fontInfo.m_fontSize = 16;
    if (!pFont->InitFont(fontInfo)) {
        delete pFont;
        return nullptr;
    }
    return pFont;
}

/** 计算文本中"当前字体不支持、需要回退字体渲染"的字符比例
*/
double CalcFallbackRatio(ui::IFont* pFont, const DStringW& strText)
{
    if (pFont == nullptr) {
        return 0.0;
    }
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
    return (nTotalCount == 0) ? 0.0 : (double)nFallbackCount / (double)nTotalCount;
}

} // namespace

int main(int argc, char* argv[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    int nIterations = 200;
    bool bEnableFallback = true;
    DString resourcePath = _T(DUILIB_BENCH_RESOURCE_DIR);
    for (int i = 1; i < argc; ++i) {
        if (::strcmp(argv[i], "--no-fallback") == 0) {
            //关闭字体回退（用于量化字体回退本身的耗时，emoji将显示为缺省字形）
            bEnableFallback = false;
        }
    }
    for (int i = 1; i + 1 < argc; ++i) {
        if (::strcmp(argv[i], "--iterations") == 0) {
            nIterations = ::atoi(argv[i + 1]);
        }
        else if (::strcmp(argv[i], "--resources") == 0) {
#ifdef DUILIB_BUILD_FOR_WIN
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
    struct GlobalShutdown
    {
        ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
    } shutdownGuard;

    ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory == nullptr) {
        ::printf("[error] GetRenderFactory failed\n");
        return 1;
    }

    //创建离屏渲染对象
    const int32_t nRenderWidth = 800;
    const int32_t nRenderHeight = 600;
    std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(0, 0);
    if ((pRender == nullptr) || !pRender->Resize(nRenderWidth, nRenderHeight)) {
        ::printf("[error] create render failed\n");
        return 1;
    }
    pRender->Clear(ui::UiColor(0xFFFFFFFF));

    //设置回退字体列表（emoji 需要 "Segoe UI Emoji" 才能显示）
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
    ui::GlobalManager::Instance().Font().SetFallbackFontFamilyNames(bEnableFallback ? fallbackFontNames : DString());

    //窗口桩对象：只提供离屏渲染对象给控件使用
    BenchWindow benchWindow(pRender.get());

    //基准文本：小文档（约一屏）、大文档（远大于一屏）与超大文档（用于观察每帧 O(行数) 的开销）
    const int nSmallLines = 15;
    const int nLargeLines = 400;
    const int nXLargeLines = 2000;
    const DStringW strSmallZhEn = MakeText(nSmallLines, false);
    const DStringW strLargeZhEn = MakeText(nLargeLines, false);
    const DStringW strXLargeZhEn = MakeText(nXLargeLines, false);
    const DStringW strSmallEmoji = MakeText(nSmallLines, true);
    const DStringW strLargeEmoji = MakeText(nLargeLines, true);
    const DStringW strXLargeEmoji = MakeText(nXLargeLines, true);

    //输出运行环境与输入说明
    std::unique_ptr<ui::IFont> pCheckFont(CreateFontForCheck(pRenderFactory, _T("Microsoft YaHei")));
    const double fallbackRatioNoEmoji = CalcFallbackRatio(pCheckFont.get(), strSmallZhEn);
    const double fallbackRatioEmoji = CalcFallbackRatio(pCheckFont.get(), strSmallEmoji);
    ::printf("==== RichEdit2 paint benchmark ====\n");
#ifdef _DEBUG
    ::printf("build config      : Debug\n");
#else
    ::printf("build config      : Release\n");
#endif
    ::printf("render size       : %d x %d (raster, offscreen)\n", nRenderWidth, nRenderHeight);
    ::printf("primary font      : Microsoft YaHei, 16px\n");
    ::printf("fallback fonts    : %s\n", bEnableFallback ? ToUTF8(fallbackFontNames).c_str() : "(disabled by --no-fallback)");
    ::printf("text (no emoji)   : small %d lines / %d chars, fallback ratio %.1f%%\n",
        nSmallLines, (int)strSmallZhEn.length(), fallbackRatioNoEmoji * 100.0);
    ::printf("text (emoji)      : small %d lines / %d chars, fallback ratio %.1f%%\n",
        nSmallLines, (int)strSmallEmoji.length(), fallbackRatioEmoji * 100.0);
    ::printf("text (no emoji)   : large %d lines / %d chars\n", nLargeLines, (int)strLargeZhEn.length());
    ::printf("text (emoji)      : large %d lines / %d chars\n", nLargeLines, (int)strLargeEmoji.length());
    ::printf("text (no emoji)   : xlarge %d lines / %d chars\n", nXLargeLines, (int)strXLargeZhEn.length());
    ::printf("text (emoji)      : xlarge %d lines / %d chars\n", nXLargeLines, (int)strXLargeEmoji.length());
    ::printf("iterations        : %d (steady), %d (relayout/scroll)\n\n",
        nIterations, (nIterations / 4) > 0 ? (nIterations / 4) : 1);

    //创建编辑框控件
    BenchRichEdit2 richEdit(&benchWindow);
    richEdit.SetWindow(&benchWindow);
    richEdit.Init();

    ui::UiFont fontInfo;
    fontInfo.m_fontName = _T("Microsoft YaHei");
    fontInfo.m_fontSize = 16;
    richEdit.SetFontInfo(fontInfo);
    richEdit.SetTextColor(_T("#FF202020"));
    richEdit.SetTextPadding(ui::UiPadding(4, 4, 4, 4), false);
    richEdit.SetRowSpacingMul(1.0f);
    richEdit.SetMultiLine(true);
    richEdit.SetWordWrap(true);

    const ui::UiRect rcControl(0, 0, nRenderWidth, nRenderHeight);
    richEdit.SetPos(rcControl);

    const int nRelayoutIterations = (nIterations / 4) > 0 ? (nIterations / 4) : 1;
    std::vector<BenchCase> benchCaseList;

    //辅助：构造"稳态绘制"场景（文本不变，绘制缓存有效）
    auto addSteadyCase = [&](const std::string& name, const std::string& desc,
                             const DStringW& text, bool bEmoji) {
        BenchCase benchCase;
        benchCase.m_name = name;
        benchCase.m_desc = desc;
        benchCase.m_nCharCount = text.length();
        benchCase.m_nIterations = nIterations;
        benchCase.m_setup = [&richEdit, &pRender, &rcControl, text]() {
            richEdit.SetText(text);
            richEdit.SetScrollPos(ui::UiSize64(0, 0));
            richEdit.Paint(pRender.get(), rcControl); //完成首次排版并建立绘制缓存
        };
        benchCase.m_run = [&richEdit, &pRender, &rcControl]() {
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCaseList.push_back(benchCase);
        (void)bEmoji;
    };

    //辅助：构造"文本变化后重新排版绘制"场景（每次内容变化，触发重新排版与缓存重建）
    auto addRelayoutCase = [&](const std::string& name, const std::string& desc,
                               const DStringW& text, bool bEmoji) {
        DStringW textA = text;
        DStringW textB = text;
        DStringW tail = bEmoji ? DStringW(L" 😀") : DStringW(L" x");
        textB += tail;
        //按值捕获文本与切换标志，避免场景执行时引用已销毁的局部变量
        bool bToggle = false;
        BenchCase benchCase;
        benchCase.m_name = name;
        benchCase.m_desc = desc;
        benchCase.m_nCharCount = textA.length();
        benchCase.m_nWarmup = 4;
        benchCase.m_nIterations = nRelayoutIterations;
        benchCase.m_setup = [&richEdit, &pRender, &rcControl, textA]() {
            richEdit.SetText(textA);
            richEdit.SetScrollPos(ui::UiSize64(0, 0));
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCase.m_run = [&richEdit, &pRender, &rcControl, textA, textB, bToggle]() mutable {
            bToggle = !bToggle;
            richEdit.SetText(bToggle ? textB : textA);
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCaseList.push_back(benchCase);
    };

    //辅助：构造"滚动绘制"场景（缓存有效，仅改变滚动位置）
    auto addScrollCase = [&](const std::string& name, const std::string& desc,
                             const DStringW& text) {
        int32_t nOffset = 0;
        BenchCase benchCase;
        benchCase.m_name = name;
        benchCase.m_desc = desc;
        benchCase.m_nCharCount = text.length();
        benchCase.m_nIterations = nIterations;
        benchCase.m_setup = [&richEdit, &pRender, &rcControl, text]() {
            richEdit.SetText(text);
            richEdit.SetScrollPos(ui::UiSize64(0, 0));
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCase.m_run = [&richEdit, &pRender, &rcControl, nOffset]() mutable {
            nOffset = (nOffset + 7) % 300;
            richEdit.SetScrollPos(ui::UiSize64(0, nOffset));
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCaseList.push_back(benchCase);
    };

    //辅助：构造"输入字符（增量排版）绘制"场景（每次在文档末尾插入一个字符）
    auto addTypingCase = [&](const std::string& name, const std::string& desc,
                             const DStringW& text) {
        BenchCase benchCase;
        benchCase.m_name = name;
        benchCase.m_desc = desc;
        benchCase.m_nCharCount = text.length();
        benchCase.m_nIterations = nIterations;
        benchCase.m_setup = [&richEdit, &pRender, &rcControl, text]() {
            richEdit.SetText(text);
            richEdit.SetScrollPos(ui::UiSize64(0, 0));
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCase.m_run = [&richEdit, &pRender, &rcControl]() {
            //在文本末尾插入一个字符，模拟用户输入（多行模式下走增量排版路径）
            richEdit.InsertText(richEdit.GetTextLength(), DStringW(L"a"), false);
            richEdit.Paint(pRender.get(), rcControl);
        };
        benchCaseList.push_back(benchCase);
    };

    addSteadyCase("steady_empty", "稳态绘制：空文本（控件绘制基础开销）", DStringW(), false);
    addSteadyCase("steady_zhen_small", "稳态绘制：中文+英文（小文档）", strSmallZhEn, false);
    addSteadyCase("steady_zhen_large", "稳态绘制：中文+英文（大文档）", strLargeZhEn, false);
    addSteadyCase("steady_zhen_xlarge", "稳态绘制：中文+英文（超大文档）", strXLargeZhEn, false);
    addSteadyCase("steady_emoji_small", "稳态绘制：中文+英文+大量emoji（小文档）", strSmallEmoji, true);
    addSteadyCase("steady_emoji_large", "稳态绘制：中文+英文+大量emoji（大文档）", strLargeEmoji, true);
    addSteadyCase("steady_emoji_xlarge", "稳态绘制：中文+英文+大量emoji（超大文档）", strXLargeEmoji, true);
    addScrollCase("scroll_zhen_large", "滚动绘制：中文+英文（大文档）", strLargeZhEn);
    addScrollCase("scroll_emoji_large", "滚动绘制：中文+英文+大量emoji（大文档）", strLargeEmoji);
    addTypingCase("typing_zhen_large", "输入字符：中文+英文（大文档，增量排版）", strLargeZhEn);
    addTypingCase("typing_emoji_large", "输入字符：中文+英文+大量emoji（大文档，增量排版）", strLargeEmoji);
    addRelayoutCase("relayout_zhen_small", "重新排版：中文+英文（小文档）", strSmallZhEn, false);
    addRelayoutCase("relayout_zhen_large", "重新排版：中文+英文（大文档）", strLargeZhEn, false);
    addRelayoutCase("relayout_emoji_small", "重新排版：中文+英文+大量emoji（小文档）", strSmallEmoji, true);
    addRelayoutCase("relayout_emoji_large", "重新排版：中文+英文+大量emoji（大文档）", strLargeEmoji, true);

    //输出排版尺寸签名：用于验证优化前后排版结果完全一致（宽度计算不能有偏差）
    ::printf("[layout signature] required size of each text (used to verify layout correctness)\n");
    {
        const std::vector<std::pair<std::string, DStringW> > signatureTextList = {
            { "zhen_small", strSmallZhEn },
            { "zhen_large", strLargeZhEn },
            { "emoji_small", strSmallEmoji },
            { "emoji_large", strLargeEmoji }
        };
        for (const auto& item : signatureTextList) {
            richEdit.SetText(item.second);
            const ui::UiSize64 requiredSize = richEdit.CalcRequiredSize(rcControl, false);
            ::printf("  %-12s: %d x %d\n", item.first.c_str(), (int)requiredSize.cx, (int)requiredSize.cy);
        }
    }
    ::printf("\n");

    //依次执行并输出结果
    ::printf("| 场景 | 说明 | 字符数 | 总耗时(ms) | 每帧(us) | 每千字符(us) |\n");
    ::printf("| --- | --- | --- | --- | --- | --- |\n");
    for (const BenchCase& benchCase : benchCaseList) {
        const BenchResult result = RunCase(benchCase);
        ::printf("| %s | %s | %d | %.2f | %.2f | %.2f |\n",
            benchCase.m_name.c_str(),
            benchCase.m_desc.c_str(),
            (int)benchCase.m_nCharCount,
            result.m_totalMs,
            result.m_usPerFrame,
            result.m_usPerKChar);
    }
    ::printf("\n");

    richEdit.SetWindow(nullptr);
    return 0;
}
