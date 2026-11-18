// 动画帧对象复用（ImageInfo::GetFrame）的性能基准
//
// 用途：量化"帧号 + 目标尺寸未变化时复用上一帧对象"的优化效果，
//       优化前后使用同一份代码、同一份图片资源对比。
//
// 运行要求：
//   1) 使用 Release 配置构建与运行；
//   2) 需要 duilib 的资源目录（默认使用源码树的 bin/resources）。
//
// 用法：
//   bench_animation_frame_reuse [--iterations N] [--resources <资源目录>] [--image <GIF路径>]
//   默认：iterations = 200000，image = themes/default/render/gif_test.gif
//
// 场景说明：
//   same-frame   ：同一帧、同一目标尺寸重复读取（典型：动画帧未变化时的重复绘制）
//   next-frame   ：逐帧轮换读取（典型：动画播放，缓存必然不命中，用于确认无性能回退）
//   two-sizes    ：同一帧、两个目标尺寸交替读取（典型：多个控件共用同一图片且尺寸不同）
//
// 指标说明：
//   us/call  单次 ImageInfo::GetFrame 的平均耗时（微秒）

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ImageManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageLoadParam.h"
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

/** 执行一个基准场景（先预热，再统计），重复多次取最优值以降低调度噪声的影响 */
BenchResult RunBench(const std::function<void()>& run, int nIterations)
{
    const int nWarmup = nIterations / 10 + 1;
    for (int i = 0; i < nWarmup; ++i) {
        run();
    }
    BenchResult best;
    for (int nRepeat = 0; nRepeat < 3; ++nRepeat) {
        const auto startTime = std::chrono::steady_clock::now();
        for (int i = 0; i < nIterations; ++i) {
            run();
        }
        const auto endTime = std::chrono::steady_clock::now();
        BenchResult result;
        result.m_totalMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        result.m_usPerCall = result.m_totalMs * 1000.0 / (double)nIterations;
        if ((nRepeat == 0) || (result.m_usPerCall < best.m_usPerCall)) {
            best = result;
        }
    }
    return best;
}

void PrintRow(const char* name, const BenchResult& result)
{
    ::printf("%-44s %11.3f %10.3f\n", name, result.m_usPerCall, result.m_totalMs);
}

} // namespace

int main(int argc, char* argv[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    int nIterations = 200000;
    DString resourcePath = _T(DUILIB_BENCH_RESOURCE_DIR);
    DString imagePath = _T("themes/default/render/gif_test.gif");
    for (int i = 1; i + 1 < argc; ++i) {
        if (::strcmp(argv[i], "--iterations") == 0) {
            nIterations = ::atoi(argv[i + 1]);
        }
        else if (::strcmp(argv[i], "--resources") == 0) {
            const int nLength = ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, nullptr, 0);
            if (nLength > 0) {
                std::vector<wchar_t> buffer((size_t)nLength);
                ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, buffer.data(), nLength);
                resourcePath = buffer.data();
            }
        }
        else if (::strcmp(argv[i], "--image") == 0) {
            const int nLength = ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, nullptr, 0);
            if (nLength > 0) {
                std::vector<wchar_t> buffer((size_t)nLength);
                ::MultiByteToWideChar(CP_ACP, 0, argv[i + 1], -1, buffer.data(), nLength);
                imagePath = buffer.data();
            }
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

    //加载多帧图片（GIF），使用绝对路径
    ui::FilePath fullImagePath(resourcePath);
    fullImagePath.JoinFilePath(ui::FilePath(imagePath));
    ui::ImageLoadParam loadParam;
    loadParam.SetImageDpiScaleEnabled(false);
    loadParam.SetLoadDpiScale(100);
    ui::ImageLoadPath loadPath;
    loadPath.m_imageFullPath = fullImagePath;
    loadPath.m_pathType = ui::ImageLoadPathType::kLocalPath;
    loadParam.SetImageLoadPath(loadPath);

    bool bFromCache = false;
    const std::shared_ptr<ui::ImageInfo> pImageInfo =
        ui::GlobalManager::Instance().Image().GetImage(loadParam, nullptr, bFromCache);
    if ((pImageInfo == nullptr) || !pImageInfo->IsMultiFrameImage() || (pImageInfo->GetFrameCount() < 2)) {
        ::printf("[error] failed to load multi-frame image: %s\n",
                 ui::StringConvert::UTF16ToUTF8(fullImagePath.NativePath().c_str(),
                                                fullImagePath.NativePath().size()).c_str());
        return 1;
    }

    const uint32_t nFrameCount = pImageInfo->GetFrameCount();
    const ui::UiSize szFrameSize(96, 96);
    const ui::UiSize szOtherSize(64, 64);

    ::printf("bench_animation_frame_reuse: image=%s, frames=%u, iterations=%d\n",
             ui::StringConvert::UTF16ToUTF8(imagePath.c_str(), imagePath.size()).c_str(),
             nFrameCount, nIterations);
    ::printf("%-44s %11s %10s\n", "scenario", "us/call", "total(ms)");
    ::printf("%-44s %11s %10s\n", "--------------------------------------------",
             "-----------", "----------");

    //场景1：同一帧、同一目标尺寸重复读取
    {
        const BenchResult result = RunBench([&]() {
            (void)pImageInfo->GetFrame(0, szFrameSize);
        }, nIterations);
        PrintRow("same-frame (frame 0, 96x96)", result);
    }

    //场景2：逐帧轮换读取（缓存不会命中）
    {
        uint32_t nFrameIndex = 0;
        const BenchResult result = RunBench([&]() {
            (void)pImageInfo->GetFrame(nFrameIndex, szFrameSize);
            nFrameIndex = (nFrameIndex + 1) % nFrameCount;
        }, nIterations);
        PrintRow("next-frame (rotate all frames, 96x96)", result);
    }

    //场景3：同一帧、两个目标尺寸交替读取（缓存不会命中）
    {
        bool bUseFirstSize = true;
        const BenchResult result = RunBench([&]() {
            (void)pImageInfo->GetFrame(0, bUseFirstSize ? szFrameSize : szOtherSize);
            bUseFirstSize = !bUseFirstSize;
        }, nIterations);
        PrintRow("two-sizes (frame 0, 96x96/64x64 alternate)", result);
    }

    return 0;
}
