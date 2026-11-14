// RichEdit2 纯文本内存占用基准
//
// 用途：量化 RichEdit2（自绘编辑框）在"设置纯文本 -> 首次排版 -> 完成绘制"整个流程中
//       随文本规模增长的内存占用，用于评估"每增加 1MB 纯文本，内存增加多少"。
//
// 测量方式：
//   1) 每个文本规模在独立进程中运行（避免不同规模的堆复用互相污染）；
//   2) 使用 Windows GetProcessMemoryInfo 读取进程私有提交内存（PrivateUsage）与工作集；
//   3) 分阶段采样：空白基线 -> 生成调用方文本 -> SetText 后 -> 首次绘制后 -> 稳态绘制后；
//   4) 所有文本使用同一份"中文+英文"行模板重复拼接，保证字符集一致（字形缓存为常量）。
//
// 运行要求：
//   1) 必须使用 Release 配置构建与运行（Debug 的断言与未优化代码会影响绝对值）；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources）；
//   3) 使用离屏渲染 + 轻量窗口桩对象驱动 RichEdit2::Paint，不需要真实窗口。
//
// 用法：
//   bench_richedit2_memory [--mb N] [--chars-per-line N] [--paints N] [--resources <资源目录>]
//   默认：--mb 4（1 MB 按 1 MiB = 1048576 字节的 UTF-16 文本计算，即 524288 个字符）、
//         --chars-per-line 64（含行尾换行符）、--paints 3（首次绘制之外的稳态绘制次数）。
//
// 输出：
//   [stage] 行：各阶段的私有提交内存/工作集（字节），供外部汇总与回归分析；
//   [result] 行：本规模的关键增量（MB）与文本规模信息。

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
#include <psapi.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
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

/** 进程内存快照（字节）
*/
struct MemSnapshot
{
    /** 私有提交内存（等同任务管理器的"提交大小"中的私有部分，包含堆、字体与Skia内部缓存） */
    unsigned long long m_privateBytes = 0;

    /** 工作集（常驻物理内存） */
    unsigned long long m_workingSet = 0;

    /** 峰值工作集（用于观察排版/绘制过程中的瞬时峰值） */
    unsigned long long m_peakWorkingSet = 0;

    /** 页面文件占用（含共享部分，仅作参考） */
    unsigned long long m_pagefileBytes = 0;
};

/** 读取当前进程的内存快照
*/
MemSnapshot GetMemSnapshot()
{
    MemSnapshot snapshot;
#ifdef DUILIB_BUILD_FOR_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc = {};
    pmc.cb = sizeof(pmc);
    if (::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        snapshot.m_privateBytes = (unsigned long long)pmc.PrivateUsage;
        snapshot.m_workingSet = (unsigned long long)pmc.WorkingSetSize;
        snapshot.m_peakWorkingSet = (unsigned long long)pmc.PeakWorkingSetSize;
        snapshot.m_pagefileBytes = (unsigned long long)pmc.PagefileUsage;
    }
#endif
    return snapshot;
}

/** 把字节数换算为 MB（1 MB = 1 MiB = 1048576 字节），便于阅读
*/
double ToMB(unsigned long long nBytes)
{
    return (double)nBytes / (1024.0 * 1024.0);
}

/** 把宽字符串转成 UTF-8（用于控制台输出）
*/
std::string ToUTF8(const DString& strText)
{
    return ui::StringConvert::UTF16ToUTF8(strText.c_str(), strText.length());
}

/** 构造"中文+英文"混合的行模板（不含换行符）
*/
const DStringW& GetLinePattern()
{
    static const DStringW linePattern =
        L"Duilib RichEdit2 纯文本内存评估：简体中文与 English mixed text 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ。";
    return linePattern;
}

/** 构造纯文本：每行固定 nCharsPerLine 个字符（含行尾的换行符）
* @param [in] nTotalChars 目标字符总数（UTF-16 编码单元）
* @param [in] nCharsPerLine 每行字符数（含换行符），必须 >= 2
* @return 返回构造好的文本，实际长度是 nCharsPerLine 的整数倍且不超过 nTotalChars
*/
DStringW MakePlainText(size_t nTotalChars, int32_t nCharsPerLine)
{
    if (nCharsPerLine < 2) {
        nCharsPerLine = 2;
    }
    const DStringW& linePattern = GetLinePattern();
    DStringW lineText;
    lineText.reserve((size_t)nCharsPerLine);
    while (lineText.size() < (size_t)(nCharsPerLine - 1)) {
        const size_t nRemain = (size_t)(nCharsPerLine - 1) - lineText.size();
        const size_t nCopy = (nRemain < linePattern.size()) ? nRemain : linePattern.size();
        lineText.append(linePattern.c_str(), nCopy);
    }
    lineText += L'\n';

    const size_t nLineCount = nTotalChars / (size_t)nCharsPerLine;
    DStringW strText;
    strText.reserve(nLineCount * (size_t)nCharsPerLine);
    for (size_t nIndex = 0; nIndex < nLineCount; ++nIndex) {
        strText += lineText;
    }
    return strText;
}

/** 输出一个阶段的内存快照
*/
void PrintStage(const char* stageName, const MemSnapshot& snapshot)
{
    ::printf("[stage] %s,%llu,%llu,%llu,%llu\n",
        stageName,
        snapshot.m_privateBytes,
        snapshot.m_workingSet,
        snapshot.m_peakWorkingSet,
        snapshot.m_pagefileBytes);
}

/** 输出一个增量（字节），同时给出 MB 值
*/
void PrintDelta(const char* deltaName, unsigned long long nBaseBytes, unsigned long long nNowBytes)
{
    const unsigned long long nDelta = (nNowBytes >= nBaseBytes) ? (nNowBytes - nBaseBytes) : 0;
    ::printf("[delta] %s,%llu,%.3f\n", deltaName, nDelta, ToMB(nDelta));
}

/** 统计文本按 UTF-8 编码后的字节数（不分配额外的大缓冲区）
*/
unsigned long long CalcUTF8Bytes(const DStringW& strText)
{
    unsigned long long nBytes = 0;
    const size_t nCount = strText.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const uint32_t ch = (uint32_t)strText[nIndex];
        if (ch < 0x80) {
            nBytes += 1;
        }
        else if (ch < 0x800) {
            nBytes += 2;
        }
        else if ((ch >= 0xD800) && (ch <= 0xDBFF) && ((nIndex + 1) < nCount)) {
            //代理对：按4字节计算，并跳过低位代理
            nBytes += 4;
            ++nIndex;
        }
        else {
            nBytes += 3;
        }
    }
    return nBytes;
}

} // namespace

int main(int argc, char* argv[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    double fTextMB = 4.0;
    int32_t nCharsPerLine = 64;
    int32_t nSteadyPaints = 3;
    DString resourcePath = _T(DUILIB_BENCH_RESOURCE_DIR);
    for (int i = 1; i + 1 < argc; ++i) {
        if (::strcmp(argv[i], "--mb") == 0) {
            fTextMB = ::atof(argv[i + 1]);
        }
        else if (::strcmp(argv[i], "--chars-per-line") == 0) {
            nCharsPerLine = ::atoi(argv[i + 1]);
        }
        else if (::strcmp(argv[i], "--paints") == 0) {
            nSteadyPaints = ::atoi(argv[i + 1]);
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
    if (fTextMB < 0.01) {
        fTextMB = 0.01;
    }
    if (nSteadyPaints < 0) {
        nSteadyPaints = 0;
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

    //创建离屏渲染对象（尺寸与渲染缓存固定，不随文本规模变化）
    const int32_t nRenderWidth = 800;
    const int32_t nRenderHeight = 600;
    std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(0, 0);
    if ((pRender == nullptr) || !pRender->Resize(nRenderWidth, nRenderHeight)) {
        ::printf("[error] create render failed\n");
        return 1;
    }
    pRender->Clear(ui::UiColor(0xFFFFFFFF));

    //设置回退字体列表（与产品默认配置一致，保证中文/符号可正常测量）
    ui::IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    DString fallbackFontNames;
    const std::vector<DString> fallbackCandidates = { _T("Microsoft YaHei"), _T("SimSun") };
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

    //窗口桩对象：只提供离屏渲染对象给控件使用
    BenchWindow benchWindow(pRender.get());

    //创建编辑框控件（配置与产品中的多行编辑框一致）
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

    ::printf("==== RichEdit2 plain text memory benchmark ====\n");
#ifdef _DEBUG
    ::printf("build config      : Debug\n");
#else
    ::printf("build config      : Release\n");
#endif
    ::printf("render size       : %d x %d (raster, offscreen)\n", nRenderWidth, nRenderHeight);
    ::printf("primary font      : Microsoft YaHei, 16px\n");
    ::printf("fallback fonts    : %s\n", ToUTF8(fallbackFontNames).c_str());
    ::printf("text size         : %.2f MB (UTF-16, 1 MB = 1048576 bytes = 524288 chars)\n", fTextMB);
    ::printf("chars per line    : %d (including '\\n')\n", nCharsPerLine);
    ::printf("steady paints     : %d (after the first paint)\n\n", nSteadyPaints);

    //预热：用同一份字符集的小文本完成一次排版与绘制，
    //使字体对象、Skia字形缓存等"与文本规模无关"的内存提前分配，后续增量只反映文本相关部分
    {
        const DStringW warmupText = MakePlainText((size_t)nCharsPerLine * 200, nCharsPerLine);
        richEdit.SetText(warmupText);
        richEdit.Paint(pRender.get(), rcControl);
        richEdit.SetText(DStringW());
        richEdit.Paint(pRender.get(), rcControl);
        richEdit.Paint(pRender.get(), rcControl);
    }

    //阶段0：空白基线（控件已创建、字体已加载、渲染缓存已分配）
    const MemSnapshot memEmpty = GetMemSnapshot();
    PrintStage("empty", memEmpty);

    //阶段1：生成"调用方持有"的纯文本（这部分内存不属于控件内部占用，单独统计）
    const size_t nTotalChars = (size_t)(fTextMB * 1024.0 * 1024.0 / (double)sizeof(DStringW::value_type));
    const DStringW strText = MakePlainText(nTotalChars, nCharsPerLine);
    const unsigned long long nTextBytes = (unsigned long long)strText.size() * sizeof(DStringW::value_type);
    const MemSnapshot memTextCreated = GetMemSnapshot();
    PrintStage("text_created", memTextCreated);

    //阶段2：SetText 之后（控件复制文本，尚未排版）
    richEdit.SetText(strText);
    const MemSnapshot memAfterSetText = GetMemSnapshot();
    PrintStage("after_settext", memAfterSetText);

    //阶段3：首次绘制之后（完成全量排版、建立绘制缓存并绘制可见区域）
    richEdit.SetScrollPos(ui::UiSize64(0, 0));
    richEdit.Paint(pRender.get(), rcControl);
    const MemSnapshot memAfterFirstPaint = GetMemSnapshot();
    PrintStage("after_first_paint", memAfterFirstPaint);

    //阶段4：稳态绘制之后（缓存有效，重复绘制不应再有与文本规模相关的内存增长）
    for (int32_t nIndex = 0; nIndex < nSteadyPaints; ++nIndex) {
        richEdit.Paint(pRender.get(), rcControl);
    }
    const MemSnapshot memAfterSteadyPaints = GetMemSnapshot();
    PrintStage("after_steady_paints", memAfterSteadyPaints);

    //输出结果摘要（文本规模 + 各阶段增量）
    const unsigned long long nUtf8Bytes = CalcUTF8Bytes(strText);
    ::printf("\n[root] mb,%.3f,chars,%llu,lines,%llu,utf16_bytes,%llu,utf8_bytes,%llu\n",
        fTextMB, (unsigned long long)strText.size(),
        (unsigned long long)(strText.size() / (size_t)nCharsPerLine), nTextBytes, nUtf8Bytes);
    PrintDelta("control_text_copy", memTextCreated.m_privateBytes, memAfterSetText.m_privateBytes);
    PrintDelta("layout_and_first_paint", memAfterSetText.m_privateBytes, memAfterFirstPaint.m_privateBytes);
    PrintDelta("steady_paints", memAfterFirstPaint.m_privateBytes, memAfterSteadyPaints.m_privateBytes);
    PrintDelta("control_total_excluding_caller_text", memTextCreated.m_privateBytes, memAfterSteadyPaints.m_privateBytes);
    PrintDelta("control_total_including_caller_text", memEmpty.m_privateBytes, memAfterSteadyPaints.m_privateBytes);
    PrintDelta("caller_text_buffer", memEmpty.m_privateBytes, memTextCreated.m_privateBytes);
    ::printf("[peak] working_set_peak,%llu,%.3f\n",
        memAfterSteadyPaints.m_peakWorkingSet, ToMB(memAfterSteadyPaints.m_peakWorkingSet));

    //关键换算：每字符内存、相对 UTF-16 文本的放大倍数、相对 UTF-8 文本的放大倍数
    const unsigned long long nControlBytes =
        (memAfterSteadyPaints.m_privateBytes >= memTextCreated.m_privateBytes) ?
        (memAfterSteadyPaints.m_privateBytes - memTextCreated.m_privateBytes) : 0;
    if (!strText.empty()) {
        ::printf("[ratio] per_char_bytes,%.2f\n", (double)nControlBytes / (double)strText.size());
        ::printf("[ratio] control_vs_utf16_text,%.3f\n",
            (nTextBytes > 0) ? ((double)nControlBytes / (double)nTextBytes) : 0.0);
        ::printf("[ratio] control_vs_utf8_text,%.3f\n",
            (nUtf8Bytes > 0) ? ((double)nControlBytes / (double)nUtf8Bytes) : 0.0);
    }
    ::printf("[size] RichTextCharInfo,%zu,RichTextData,%zu,RichTextRowInfo,%zu,RichTextLineInfo,%zu\n",
        sizeof(ui::RichTextCharInfo), sizeof(ui::RichTextData),
        sizeof(ui::RichTextRowInfo), sizeof(ui::RichTextLineInfo));

    richEdit.SetWindow(nullptr);
    return 0;
}
