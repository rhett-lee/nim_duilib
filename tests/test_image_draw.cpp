// 图片绘制（rcPaint 可绘制区域）相关接口的单元测试
//
// 测试对象：ui::IRender::DrawImage 的 ImageDrawParam::rcPaint 参数
//  1. 传 UiRect() 表示不限制，由渲染接口使用当前裁剪区域兜底；
//  2. rcPaint 与目标区域不相交时，跳过绘制；
//  3. rcPaint 与目标区域部分相交时，只绘制相交部分。
// 另外覆盖 ui::ImageInfo::GetFrame 的动画帧对象复用：
//  帧号与目标尺寸均未变化时复用上一帧对象，变化时重新读取。
//
// 本测试使用离屏渲染，不创建窗口；需要 duilib 的资源目录（编译期宏
// DUILIB_TEST_RESOURCE_DIR，默认指向源码树的 bin/resources）。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ImageManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageLoadParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const char* message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}
#define CHECK(expr) Check((expr), #expr)

/** 读取单个像素的颜色值（读取失败时返回false） */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

/** 创建纯色位图（ARGB格式，不透明） */
std::unique_ptr<ui::IBitmap> CreateSolidBitmap(ui::IRenderFactory* pRenderFactory, uint32_t color)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap());
    if (pBitmap == nullptr) {
        return nullptr;
    }
    const uint32_t nWidth = 10;
    const uint32_t nHeight = 10;
    std::vector<uint32_t> pixels((size_t)nWidth * nHeight, color);
    if (!pBitmap->Init(nWidth, nHeight, pixels.data(), 1.0f, ui::AlphaType::kPremul)) {
        return nullptr;
    }
    return pBitmap;
}

void TestImagePaintRect(ui::IRender* pRender, ui::IRenderFactory* pRenderFactory)
{
    if ((pRender == nullptr) || (pRenderFactory == nullptr)) {
        Check(false, "IRender/IRenderFactory is null");
        return;
    }
    CHECK(pRender->Resize(100, 100));

    //纯红图片
    std::unique_ptr<ui::IBitmap> pBitmap = CreateSolidBitmap(pRenderFactory, 0xFFFF0000u);
    CHECK(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    const ui::UiRect rcDest(10, 10, 20, 20);
    const ui::UiRect rcSource(0, 0, 10, 10);

    //基准色：背景白色，图片绘制后的颜色
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    uint32_t nWhiteColor = 0;
    CHECK(ReadPixel(pRender, 5, 5, nWhiteColor));
    pRender->DrawImage(pBitmap.get(), rcDest, rcSource);
    uint32_t nImageColor = 0;
    CHECK(ReadPixel(pRender, 15, 15, nImageColor));
    CHECK(nImageColor != nWhiteColor);

    //用例1：rcPaint 传 UiRect()（不传）表示不限制，正常绘制
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    pRender->DrawImage(pBitmap.get(), rcDest, rcSource);
    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender, 15, 15, nColor));
    CHECK(nColor == nImageColor);
    CHECK(ReadPixel(pRender, 5, 5, nColor));
    CHECK(nColor == nWhiteColor);

    //用例2：rcPaint 与目标区域不相交时，跳过绘制
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    {
        ui::ImageDrawParam drawParam;
        drawParam.rcPaint = ui::UiRect(50, 50, 60, 60);
        pRender->DrawImage(pBitmap.get(), rcDest, rcSource, drawParam);
    }
    CHECK(ReadPixel(pRender, 15, 15, nColor));
    CHECK(nColor == nWhiteColor);

    //用例3：rcPaint 与目标区域部分相交时，正常发起绘制；
    //      实际可见区域由裁剪区域决定（真实绘制流程中裁剪区域即 rcPaint），
    //      rcPaint 只用于跳过完全不可见的绘制，不承担裁剪职责
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    const int32_t nClipState = pRender->ClipRect(ui::UiRect(0, 0, 15, 15));
    {
        ui::ImageDrawParam drawParam;
        drawParam.rcPaint = ui::UiRect(0, 0, 15, 15);
        pRender->DrawImage(pBitmap.get(), rcDest, rcSource, drawParam);
    }
    pRender->RestoreClip(nClipState);
    CHECK(ReadPixel(pRender, 12, 12, nColor));
    CHECK(nColor == nImageColor);
    CHECK(ReadPixel(pRender, 18, 18, nColor));
    CHECK(nColor == nWhiteColor);

    //用例4：整图绘制接口（不传源区域），正常绘制
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    pRender->DrawImage(pBitmap.get(), rcDest);
    CHECK(ReadPixel(pRender, 15, 15, nColor));
    CHECK(nColor == nImageColor);

    //用例5：指定变换矩阵时，rcPaint 不参与可见性判断，按矩阵绘制
    pRender->Clear(ui::UiColor(0xFFFFFFFFu));
    std::unique_ptr<ui::IMatrix> pMatrix(pRenderFactory->CreateMatrix());
    CHECK(pMatrix != nullptr);
    if (pMatrix != nullptr) {
        pMatrix->Translate(50, 50);
        ui::ImageDrawParam drawParam;
        drawParam.pMatrix = pMatrix.get();
        drawParam.rcPaint = ui::UiRect(50, 50, 60, 60);
        pRender->DrawImage(pBitmap.get(), rcDest, rcSource, drawParam);
        CHECK(ReadPixel(pRender, 65, 65, nColor));
        CHECK(nColor == nImageColor);
    }
}

/** 动画帧对象复用（ImageInfo::GetFrame）：
 *  帧号与目标尺寸均未变化时复用上一帧对象，避免重复读取帧数据与创建对象；
 *  帧号或目标尺寸变化时重新读取，返回的帧数据保持正确。
 */
void TestAnimationFrameReuse()
{
    const DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    const DString imagePath = resourcePath + _T("/themes/default/render/gif_test.gif");

    ui::ImageLoadParam loadParam;
    loadParam.SetImageDpiScaleEnabled(false);
    loadParam.SetLoadDpiScale(100);
    ui::ImageLoadPath loadPath;
    loadPath.m_imageFullPath = ui::FilePath(imagePath);
    loadPath.m_pathType = ui::ImageLoadPathType::kLocalPath;
    loadParam.SetImageLoadPath(loadPath);

    bool bFromCache = false;
    const std::shared_ptr<ui::ImageInfo> pImageInfo =
        ui::GlobalManager::Instance().Image().GetImage(loadParam, nullptr, bFromCache);
    Check(pImageInfo != nullptr, "GetImage() is null");
    if (pImageInfo == nullptr) {
        return;
    }
    Check(pImageInfo->IsMultiFrameImage(), "gif_test.gif is not a multi-frame image");
    Check(pImageInfo->GetFrameCount() >= 2, "gif_test.gif frame count < 2");
    if (!pImageInfo->IsMultiFrameImage() || (pImageInfo->GetFrameCount() < 2)) {
        return;
    }

    const ui::UiSize szFrameSize(64, 64);
    const ui::UiSize szOtherSize(32, 32);

    //用例1：首次读取，返回有效的帧数据
    const ui::AnimationFramePtr pFrame0 = pImageInfo->GetFrame(0, szFrameSize);
    Check(pFrame0 != nullptr, "GetFrame(0) is null");
    if (pFrame0 == nullptr) {
        return;
    }
    Check(!pFrame0->m_bDataPending, "frame 0 is pending");
    Check(pFrame0->m_pBitmap != nullptr, "frame 0 bitmap is null");
    Check(pFrame0->m_nFrameIndex == 0, "frame 0 index is invalid");

    //用例2：帧号与目标尺寸均未变化，复用同一个帧对象
    const ui::AnimationFramePtr pFrame0Again = pImageInfo->GetFrame(0, szFrameSize);
    CHECK(pFrame0Again.get() == pFrame0.get());
    CHECK(pFrame0Again->m_pBitmap.get() == pFrame0->m_pBitmap.get());

    //用例3：帧号变化，返回新的帧对象，并且帧号正确
    const ui::AnimationFramePtr pFrame1 = pImageInfo->GetFrame(1, szFrameSize);
    Check(pFrame1 != nullptr, "GetFrame(1) is null");
    if (pFrame1 != nullptr) {
        Check(pFrame1.get() != pFrame0.get(), "frame 1 reuses frame 0 object");
        Check(pFrame1->m_nFrameIndex == 1, "frame 1 index is invalid");
        Check(pFrame1->m_pBitmap != nullptr, "frame 1 bitmap is null");
    }

    //用例4：帧号相同但目标尺寸变化，返回新的帧对象
    const ui::AnimationFramePtr pFrame1Small = pImageInfo->GetFrame(1, szOtherSize);
    Check(pFrame1Small != nullptr, "GetFrame(1, small size) is null");
    if ((pFrame1Small != nullptr) && (pFrame1 != nullptr)) {
        Check(pFrame1Small.get() != pFrame1.get(), "frame cache ignores dest size");
    }

    //用例5：回到之前的帧号+尺寸组合，缓存只有一帧，需要重新读取且帧数据正确
    const ui::AnimationFramePtr pFrame0Reload = pImageInfo->GetFrame(0, szFrameSize);
    Check(pFrame0Reload != nullptr, "GetFrame(0) reload is null");
    if (pFrame0Reload != nullptr) {
        Check(pFrame0Reload.get() != pFrame0.get(), "reload should not reuse stale object");
        Check(pFrame0Reload->m_nFrameIndex == 0, "reload frame index is invalid");
        Check(pFrame0Reload->m_pBitmap != nullptr, "reload frame bitmap is null");
    }

    //用例6：重新设置图片数据后，之前缓存的帧对象必须失效，不能返回旧的帧数据
    {
        const ui::AnimationFramePtr pFrameBefore = pImageInfo->GetFrame(0, szFrameSize);
        Check(pFrameBefore != nullptr, "GetFrame(0) before SetImageData is null");
        std::shared_ptr<ui::IImage> pImageData = pImageInfo->GetImageData();
        Check(pImageData != nullptr, "GetImageData() is null");
        if (pImageData != nullptr) {
            Check(pImageInfo->SetImageData(loadParam, pImageData, false, 100), "SetImageData() failed");
            const ui::AnimationFramePtr pFrameAfter = pImageInfo->GetFrame(0, szFrameSize);
            Check(pFrameAfter != nullptr, "GetFrame(0) after SetImageData is null");
            if ((pFrameBefore != nullptr) && (pFrameAfter != nullptr)) {
                Check(pFrameAfter.get() != pFrameBefore.get(), "cached frame is not invalidated by SetImageData");
                Check(pFrameAfter->m_nFrameIndex == 0, "frame index after SetImageData is invalid");
                Check(pFrameAfter->m_pBitmap != nullptr, "frame bitmap after SetImageData is null");
            }
        }
    }
}

}//namespace

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
        Check(pRenderFactory != nullptr, "GetRenderFactory() is null");
        if (pRenderFactory != nullptr) {
            //离屏渲染对象（不创建窗口，使用内存位图作为绘制目标）
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(0, 0);
            Check(pRender != nullptr, "CreateOffscreenRender() is null");
            if (pRender != nullptr) {
                TestImagePaintRect(pRender.get(), pRenderFactory);
            }
        }

        //动画帧对象复用（不依赖渲染对象）
        TestAnimationFrameReuse();
    }

    std::cout << "image_draw: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "image_draw: FAILED" << std::endl;
        return 1;
    }
    std::cout << "image_draw: PASSED" << std::endl;
    return 0;
}
