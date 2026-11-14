// ClearAlpha / RestoreAlpha / Render Clear / Render ClearRect 的性能基准
//
// 用途：为这几个"高频调用"的函数提供可重复的量化基线，优化前后用同一份代码对比。
//
// 运行要求：
//   1) 必须使用 Release 配置构建与运行（Debug 的断言与未优化代码会影响绝对值）；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources），用于初始化渲染工厂。
//
// 用法：
//   bench_clear_alpha [--iterations N] [--resources <资源目录>]
//   默认：iterations = 200，资源目录由编译期宏 DUILIB_BENCH_RESOURCE_DIR 指定。
//
// 指标说明：
//   us/call  单次调用的平均耗时（微秒）
//   MB/s     估算的有效内存流量（写=1N，读+写=2N，N 为区域字节数）
//
// 输出的表格可直接粘贴到 docs/misc/ClearAlpha性能优化报告.md 的性能基线章节。

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/BitmapAlpha.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include <windows.h>
#endif

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace {

/** 64 字节对齐的像素缓冲区（模拟真实位图的内存对齐方式） */
struct AlignedBuffer
{
    std::vector<uint8_t> m_storage;
    uint8_t* m_pData = nullptr;

    explicit AlignedBuffer(size_t nBytes) : m_storage(nBytes + 64)
    {
        const uintptr_t nAddr = (uintptr_t)m_storage.data();
        m_pData = (uint8_t*)((nAddr + 63) & ~(uintptr_t)63);
    }
};

/** 基准结果 */
struct BenchResult
{
    double m_totalMs = 0.0;
    double m_usPerCall = 0.0;
    double m_mbPerSec = 0.0;
};

/** 执行一个基准场景
* @param [in] run 单次调用的闭包
* @param [in] nIterations 统计次数
* @param [in] nBytesPerCall 单次调用的有效内存流量（字节）
*/
BenchResult RunBench(const std::function<void()>& run, int nIterations, size_t nBytesPerCall)
{
    const int nWarmup = nIterations / 10 + 1;
    for (int i = 0; i < nWarmup; ++i) {
        run();
    }
    const auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < nIterations; ++i) {
        run();
    }
    const auto endTime = std::chrono::steady_clock::now();

    BenchResult result;
    result.m_totalMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    result.m_usPerCall = result.m_totalMs * 1000.0 / (double)nIterations;
    const double fTotalBytes = (double)nBytesPerCall * (double)nIterations;
    if (result.m_totalMs > 0.0) {
        result.m_mbPerSec = fTotalBytes / (result.m_totalMs / 1000.0) / (1024.0 * 1024.0);
    }
    return result;
}

/** 输出表格的标题行 */
void PrintHeader()
{
    ::printf("%-52s %11s %11s %10s\n", "scenario", "us/call", "MB/s", "total(ms)");
    ::printf("%-52s %11s %11s %10s\n", "--------------------------------------------",
             "-----------", "-----------", "----------");
}

/** 输出一行基准结果 */
void PrintRow(const char* name, const BenchResult& result)
{
    ::printf("%-52s %11.3f %11.1f %10.3f\n", name, result.m_usPerCall, result.m_mbPerSec, result.m_totalMs);
}

/** 填充像素：RGB 有规律，Alpha 在 0/1/2 之间交替（覆盖不同分支） */
void FillBitmapPattern(uint8_t* pPixel, int32_t nWidth, int32_t nHeight)
{
    uint32_t* pBits = (uint32_t*)pPixel;
    for (int32_t y = 0; y < nHeight; ++y) {
        for (int32_t x = 0; x < nWidth; ++x) {
            const uint32_t nAlpha = (uint32_t)((x + y) % 3);
            const uint32_t nRed = (uint32_t)((x * 13 + y * 7) & 0xFF);
            const uint32_t nGreen = (uint32_t)((x * 5 + y * 29) & 0xFF);
            const uint32_t nBlue = (uint32_t)((x * 17 + y * 3) & 0xFF);
            pBits[(size_t)y * (size_t)nWidth + (size_t)x] = (nAlpha << 24) | (nRed << 16) | (nGreen << 8) | nBlue;
        }
    }
}

/** 基准 BitmapAlpha 的几个函数
*/
void BenchBitmapAlpha(int nIterations)
{
    const int32_t nWidth = 1920;
    const int32_t nHeight = 1080;
    const size_t nBytes = (size_t)nWidth * (size_t)nHeight * sizeof(uint32_t);
    AlignedBuffer buffer(nBytes);
    FillBitmapPattern(buffer.m_pData, nWidth, nHeight);
    ui::BitmapAlpha bitmapAlpha(buffer.m_pData, nWidth, nHeight, 4);
    const ui::UiRect rcFull(0, 0, nWidth, nHeight);
    const ui::UiRect rcQuarter(nWidth / 4, nHeight / 4, nWidth * 3 / 4, nHeight * 3 / 4);
    const size_t nQuarterBytes = (size_t)rcQuarter.Width() * (size_t)rcQuarter.Height() * sizeof(uint32_t);
    const ui::UiPadding paddingNone;

    //ClearAlpha：整图清除（alpha = 0，常用路径，内部为单次 memset）
    PrintRow("BitmapAlpha::ClearAlpha full 1920x1080 alpha=0",
             RunBench([&]() { bitmapAlpha.ClearAlpha(rcFull, 0); }, nIterations, nBytes));

    //ClearAlpha：局部清除（alpha = 0，内部按行 memset）
    PrintRow("BitmapAlpha::ClearAlpha rect 960x540 alpha=0",
             RunBench([&]() { bitmapAlpha.ClearAlpha(rcQuarter, 0); }, nIterations, nQuarterBytes));

    //ClearAlpha：整图清除（alpha = 128，非零填充路径）
    PrintRow("BitmapAlpha::ClearAlpha full 1920x1080 alpha=128",
             RunBench([&]() { bitmapAlpha.ClearAlpha(rcFull, 128); }, nIterations, nBytes));

    //RestoreAlpha(alpha=0)：与 ClearAlpha 成对调用（真实绘制流程：先清除、再恢复）
    PrintRow("BitmapAlpha::Clear+Restore pair full alpha=0",
             RunBench([&]() {
                 bitmapAlpha.ClearAlpha(rcFull, 0);
                 bitmapAlpha.RestoreAlpha(rcFull, paddingNone, 0);
             }, nIterations, nBytes * 3));

    //RestoreAlpha(alpha=128)：与 ClearAlpha 成对调用
    PrintRow("BitmapAlpha::Clear+Restore pair full alpha=128",
             RunBench([&]() {
                 bitmapAlpha.ClearAlpha(rcFull, 128);
                 bitmapAlpha.RestoreAlpha(rcFull, paddingNone, 128);
             }, nIterations, nBytes * 3));

    //RestoreAlpha(无 alpha)：无条件置 255（幂等，可直接重复调用）
    PrintRow("BitmapAlpha::RestoreAlpha full 1920x1080 (=>255)",
             RunBench([&]() { bitmapAlpha.RestoreAlpha(rcFull, paddingNone); }, nIterations, nBytes * 2));

    //RestoreAlpha(无 alpha)：局部区域
    PrintRow("BitmapAlpha::RestoreAlpha rect 960x540 (=>255)",
             RunBench([&]() { bitmapAlpha.RestoreAlpha(rcQuarter, paddingNone); }, nIterations, nQuarterBytes * 2));

    //小块位图（512x512，主要受计算影响而非内存带宽影响）
    {
        const int32_t nSmallWidth = 512;
        const int32_t nSmallHeight = 512;
        const size_t nSmallBytes = (size_t)nSmallWidth * (size_t)nSmallHeight * sizeof(uint32_t);
        AlignedBuffer smallBuffer(nSmallBytes);
        FillBitmapPattern(smallBuffer.m_pData, nSmallWidth, nSmallHeight);
        ui::BitmapAlpha smallAlpha(smallBuffer.m_pData, nSmallWidth, nSmallHeight, 4);
        const ui::UiRect rcSmall(0, 0, nSmallWidth, nSmallHeight);
        PrintRow("BitmapAlpha::Clear+Restore pair 512x512 alpha=0",
                 RunBench([&]() {
                     smallAlpha.ClearAlpha(rcSmall, 0);
                     smallAlpha.RestoreAlpha(rcSmall, paddingNone, 0);
                 }, nIterations * 4, nSmallBytes * 3));
        PrintRow("BitmapAlpha::RestoreAlpha 512x512 (=>255)",
                 RunBench([&]() { smallAlpha.RestoreAlpha(rcSmall, paddingNone); }, nIterations * 4, nSmallBytes * 2));
    }
}

/** 基准 Render_Skia 的 Clear / ClearRect（通过 IRender 接口）
*/
void BenchRender(ui::IRenderFactory* pRenderFactory, int nIterations)
{
    const int32_t nWidth = 1920;
    const int32_t nHeight = 1080;
    const size_t nBytes = (size_t)nWidth * (size_t)nHeight * sizeof(uint32_t);
    std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(nWidth, nHeight);
    if ((pRender == nullptr) || pRender->IsEmpty()) {
        ::printf("[error] create offscreen render failed\n");
        return;
    }
    const ui::UiColor colorWhite(0xFFFFFFFFu);
    const ui::UiColor colorTransparent(0x00000000u);
    const ui::UiColor colorRed(0xFFFF0000u);
    const ui::UiRect rcFull(0, 0, nWidth, nHeight);
    const ui::UiRect rcQuarter(nWidth / 4, nHeight / 4, nWidth * 3 / 4, nHeight * 3 / 4);
    const size_t nQuarterBytes = (size_t)rcQuarter.Width() * (size_t)rcQuarter.Height() * sizeof(uint32_t);

    PrintRow("Render::Clear 1920x1080 opaque white",
             RunBench([&]() { pRender->Clear(colorWhite); }, nIterations, nBytes));

    PrintRow("Render::Clear 1920x1080 transparent (memset)",
             RunBench([&]() { pRender->Clear(colorTransparent); }, nIterations, nBytes));

    PrintRow("Render::ClearRect rect 960x540 opaque red",
             RunBench([&]() { pRender->ClearRect(rcQuarter, colorRed); }, nIterations, nQuarterBytes));

    PrintRow("Render::ClearRect whole canvas opaque red",
             RunBench([&]() { pRender->ClearRect(rcFull, colorRed); }, nIterations, nBytes));
}

} // namespace

int main(int argc, char* argv[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    int nIterations = 200;
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

    //初始化全局资源（主题、字体、渲染工厂）
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        ::printf("[error] GlobalManager::Startup failed, resource path: %s\n",
                 ui::StringConvert::UTF16ToUTF8(resourcePath.c_str(), resourcePath.size()).c_str());
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

    ::printf("bench_clear_alpha: iterations=%d\n", nIterations);
    PrintHeader();
    BenchBitmapAlpha(nIterations);
    BenchRender(pRenderFactory, nIterations);

    //读取少量像素，防止编译器把整个基准过程优化掉
    const ui::UiRect rcProbe(0, 0, 1, 1);
    uint32_t nProbe = 0;
    if (pRenderFactory != nullptr) {
        std::unique_ptr<ui::IRender> pProbe = pRenderFactory->CreateOffscreenRender(1, 1);
        if (pProbe != nullptr) {
            pProbe->Clear(ui::UiColor(0x11223344u));
            pProbe->ReadPixels(rcProbe, &nProbe, sizeof(nProbe));
        }
    }
    ::printf("probe=0x%08X\n", nProbe);
    return 0;
}
