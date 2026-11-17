// 图片缩放结果缓存的性能基准
//
// 用途：量化 DrawSkiaImage 的"按目标尺寸缓存缩放结果 + 1:1绘制"优化效果，
//       优化前后用同一份代码对比（缓存可通过 DrawSkiaImage::EnableImageScaleCache 开关）。
//
// 运行要求：
//   1) 使用 Release 配置构建与运行（Debug 的断言与未优化代码会影响绝对值）；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources），用于初始化渲染工厂。
//
// 用法：
//   bench_image_scale_cache [--iterations N] [--resources <资源目录>]
//   默认：iterations = 1000。
//
// 场景说明：
//   static   ：同一张大图按固定目标尺寸重复绘制（典型：静态图片/缩略图重复绘制）
//   animation：64 张不同内容的大图轮换绘制（典型：动图首轮播放，每帧内容都不同）
//   small    ：小图（源图片小于缓存阈值 64x64）按固定目标尺寸重复绘制
//
// 指标说明：
//   us/call  单次 DrawImage 的平均耗时（微秒）
//   speedup  相对"缓存关闭"的加速比

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/RenderSkia/DrawSkiaImage.h"
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

/** 基准结果 */
struct BenchResult
{
    double m_usPerCall = 0.0;
    double m_totalMs = 0.0;
};

/** 执行一个基准场景（先预热，再统计） */
BenchResult RunBench(const std::function<void()>& run, int nIterations)
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
    return result;
}

/** 输出表格的标题行 */
void PrintHeader()
{
    ::printf("%-56s %11s %10s %10s\n", "scenario", "us/call", "total(ms)", "speedup");
    ::printf("%-56s %11s %10s %10s\n", "------------------------------------------",
             "-----------", "----------", "----------");
}

/** 输出一行基准结果 */
void PrintRow(const char* name, const BenchResult& result, double fSpeedup)
{
    if (fSpeedup > 0.0) {
        ::printf("%-56s %11.3f %10.3f %9.2fx\n", name, result.m_usPerCall, result.m_totalMs, fSpeedup);
    }
    else {
        ::printf("%-56s %11.3f %10.3f %10s\n", name, result.m_usPerCall, result.m_totalMs, "-");
    }
}

/** 创建测试位图，并填充有规律的内容 */
std::unique_ptr<ui::IBitmap> CreatePatternBitmap(ui::IRenderFactory* pRenderFactory,
                                                 uint32_t nWidth, uint32_t nHeight, uint32_t nSeed)
{
    std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap(nWidth, nHeight, ui::AlphaType::kPremul));
    if ((pBitmap == nullptr) || !pBitmap->IsValid()) {
        return nullptr;
    }
    uint32_t* pBits = (uint32_t*)pBitmap->LockPixelBits();
    if (pBits == nullptr) {
        return nullptr;
    }
    for (uint32_t y = 0; y < nHeight; ++y) {
        for (uint32_t x = 0; x < nWidth; ++x) {
            const uint32_t nRed = (uint32_t)((x * 3 + y * 5 + nSeed * 11) & 0xFF);
            const uint32_t nGreen = (uint32_t)((x * 7 + y * 13 + nSeed * 17) & 0xFF);
            const uint32_t nBlue = (uint32_t)((x * 29 + y * 31 + nSeed * 37) & 0xFF);
            pBits[(size_t)y * (size_t)nWidth + (size_t)x] = 0xFF000000u | (nRed << 16) | (nGreen << 8) | nBlue;
        }
    }
    pBitmap->UnLockPixelBits();
    return pBitmap;
}

/** 绘制一轮并返回耗时（同一场景重复多次，取最优值以降低调度噪声的影响） */
BenchResult BenchDrawImage(ui::IRender* pRender, const std::vector<ui::IBitmap*>& bitmaps,
                           const ui::UiRect& rcDest, int nIterations, bool bCacheEnabled)
{
    ui::DrawSkiaImage::EnableImageScaleCache(bCacheEnabled);
    if (!bCacheEnabled) {
        ui::DrawSkiaImage::ClearImageScaleCache();
    }
    size_t nIndex = 0;
    const size_t nCount = bitmaps.size();
    BenchResult best;
    for (int nRepeat = 0; nRepeat < 3; ++nRepeat) {
        const BenchResult result = RunBench([&]() {
            pRender->DrawImage(bitmaps[nIndex], rcDest);
            nIndex = (nIndex + 1) % nCount;
        }, nIterations);
        if ((nRepeat == 0) || (result.m_usPerCall < best.m_usPerCall)) {
            best = result;
        }
    }
    return best;
}

} // namespace

int main(int argc, char* argv[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    int nIterations = 1000;
    DString resourcePath = _T(DUILIB_BENCH_RESOURCE_DIR);
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

    std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(1024, 768);
    if (pRender == nullptr) {
        ::printf("[error] CreateOffscreenRender failed\n");
        return 1;
    }

    //静态大图：1024x768 缩放到 170x130 / 400x300
    const uint32_t nSrcWidth = 1024;
    const uint32_t nSrcHeight = 768;
    std::unique_ptr<ui::IBitmap> pStaticBitmap = CreatePatternBitmap(pRenderFactory, nSrcWidth, nSrcHeight, 1);
    if (pStaticBitmap == nullptr) {
        ::printf("[error] CreatePatternBitmap failed\n");
        return 1;
    }

    //动图帧：64 张 512x384 的不同内容图片（每帧内容不同，模拟首轮播放）
    std::vector<std::unique_ptr<ui::IBitmap>> animationBitmaps;
    for (uint32_t nFrame = 0; nFrame < 64; ++nFrame) {
        std::unique_ptr<ui::IBitmap> pFrame = CreatePatternBitmap(pRenderFactory, 512, 384, 100 + nFrame);
        if (pFrame == nullptr) {
            ::printf("[error] CreatePatternBitmap(frame) failed\n");
            return 1;
        }
        animationBitmaps.push_back(std::move(pFrame));
    }
    std::vector<ui::IBitmap*> animationBitmapPtrs;
    for (const std::unique_ptr<ui::IBitmap>& pFrame : animationBitmaps) {
        animationBitmapPtrs.push_back(pFrame.get());
    }
    std::vector<ui::IBitmap*> staticBitmapPtrs = { pStaticBitmap.get() };

    //小图：32x32 缩放到 16x16（低于缓存阈值，走原有绘制路径）
    std::unique_ptr<ui::IBitmap> pSmallBitmap = CreatePatternBitmap(pRenderFactory, 32, 32, 7);
    if (pSmallBitmap == nullptr) {
        ::printf("[error] CreatePatternBitmap(small) failed\n");
        return 1;
    }
    std::vector<ui::IBitmap*> smallBitmapPtrs = { pSmallBitmap.get() };

    ::printf("bench_image_scale_cache: iterations=%d, backend=%d\n",
             nIterations, (int)pRender->GetRenderBackendType());

    struct Scenario
    {
        const char* m_name;
        std::vector<ui::IBitmap*>* m_pBitmaps;
        ui::UiRect m_rcDest;
    };
    std::vector<Scenario> scenarios = {
        { "static 1024x768 -> 170x130", &staticBitmapPtrs, ui::UiRect(0, 0, 170, 130) },
        { "static 1024x768 -> 400x300", &staticBitmapPtrs, ui::UiRect(0, 0, 400, 300) },
        { "animation 64 frames 512x384 -> 200x150", &animationBitmapPtrs, ui::UiRect(0, 0, 200, 150) },
        { "small 32x32 -> 16x16 (below threshold)", &smallBitmapPtrs, ui::UiRect(0, 0, 16, 16) },
    };

    PrintHeader();
    for (const Scenario& scenario : scenarios) {
        const BenchResult off = BenchDrawImage(pRender.get(), *scenario.m_pBitmaps, scenario.m_rcDest, nIterations, false);
        const BenchResult on = BenchDrawImage(pRender.get(), *scenario.m_pBitmaps, scenario.m_rcDest, nIterations, true);
        const double fSpeedup = (on.m_usPerCall > 0.0) ? (off.m_usPerCall / on.m_usPerCall) : 0.0;
        char nameOff[128] = { 0 };
        char nameOn[128] = { 0 };
        ::snprintf(nameOff, sizeof(nameOff), "%s [cache off]", scenario.m_name);
        ::snprintf(nameOn, sizeof(nameOn), "%s [cache on ]", scenario.m_name);
        PrintRow(nameOff, off, -1.0);
        PrintRow(nameOn, on, fSpeedup);
    }

    ::printf("\ncache entries=%u, hits=%u\n",
             (uint32_t)ui::DrawSkiaImage::GetImageScaleCacheCount(),
             (uint32_t)ui::DrawSkiaImage::GetImageScaleCacheHitCount());

    ui::DrawSkiaImage::ClearImageScaleCache();
    ui::DrawSkiaImage::EnableImageScaleCache(true);
    return 0;
}
