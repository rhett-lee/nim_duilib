// 常用图形绘制接口的单元测试
//
// 测试对象（ui::IRender）：
//  1. FillEllipse / DrawEllipse：椭圆填充与描边；
//  2. FillArc：扇形填充；
//  3. FillPolygon / DrawPolygon：多边形填充与描边；
//  4. ClipPath：非矩形（多边形）裁剪；
//  5. SetImageSamplingQuality / GetImageSamplingQuality：采样质量设置。
//
// 本测试使用离屏渲染，不创建窗口；需要 duilib 的资源目录（编译期宏
// DUILIB_TEST_RESOURCE_DIR，默认指向源码树的 bin/resources）。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cstdio>
#include <iostream>
#include <memory>

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

const uint32_t kWhiteColor = 0xFFFFFFFFu;
const uint32_t kRedColor = 0xFFFF0000u;
const uint32_t kBlueColor = 0xFF0000FFu;

/** 读取单个像素的颜色值（读取失败时返回false） */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

void TestEllipse(ui::IRender* pRender)
{
    CHECK(pRender->Resize(100, 100));

    //填充椭圆
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillEllipse(ui::UiRectF(20, 20, 80, 80), ui::UiColor(kRedColor));
    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender, 50, 50, nColor));
    CHECK(nColor == kRedColor);
    CHECK(ReadPixel(pRender, 21, 21, nColor));
    CHECK(nColor == kWhiteColor);

    //绘制椭圆边框（线宽4）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawEllipse(ui::UiRectF(20, 20, 80, 80), ui::UiColor(kBlueColor), 4.0f);
    CHECK(ReadPixel(pRender, 50, 20, nColor));
    CHECK(nColor == kBlueColor);
    CHECK(ReadPixel(pRender, 50, 50, nColor));
    CHECK(nColor == kWhiteColor);
}

void TestArc(ui::IRender* pRender)
{
    CHECK(pRender->Resize(100, 100));

    //左上象限的扇形（Skia的角度以x轴正方向为0度，顺时针为正）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillArc(ui::UiRectF(20, 20, 80, 80), 0.0f, 90.0f, true, ui::UiColor(kRedColor));
    uint32_t nColor = 0;
    //右下象限在扇形内
    CHECK(ReadPixel(pRender, 70, 70, nColor));
    CHECK(nColor == kRedColor);
    //左下象限在扇形外
    CHECK(ReadPixel(pRender, 30, 70, nColor));
    CHECK(nColor == kWhiteColor);
}

void TestPolygon(ui::IRender* pRender)
{
    CHECK(pRender->Resize(100, 100));

    const ui::UiPointF points[3] = { {50, 10}, {90, 90}, {10, 90} };

    //填充三角形
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillPolygon(points, 3, ui::UiColor(kRedColor));
    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender, 50, 80, nColor));
    CHECK(nColor == kRedColor);
    CHECK(ReadPixel(pRender, 10, 10, nColor));
    CHECK(nColor == kWhiteColor);

    //绘制三角形边框（线宽4）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawPolygon(points, 3, ui::UiColor(kBlueColor), 4.0f);
    CHECK(ReadPixel(pRender, 50, 12, nColor));
    CHECK(nColor == kBlueColor);
    CHECK(ReadPixel(pRender, 50, 50, nColor));
    CHECK(nColor == kWhiteColor);
}

void TestClipPath(ui::IRender* pRender, ui::IRenderFactory* pRenderFactory)
{
    CHECK(pRender->Resize(100, 100));

    std::unique_ptr<ui::IPath> pPath(pRenderFactory->CreatePath());
    CHECK(pPath != nullptr);
    if (pPath == nullptr) {
        return;
    }
    const ui::UiPointF points[3] = { {50, 10}, {90, 90}, {10, 90} };
    pPath->AddPolygon(points, 3);

    pRender->Clear(ui::UiColor(kWhiteColor));
    const int32_t nClipState = pRender->ClipPath(pPath.get());
    CHECK(nClipState >= 0);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 100.0f, 100.0f), ui::UiColor(kRedColor));
    pRender->RestoreClip(nClipState);

    uint32_t nColor = 0;
    //三角形内部被填充
    CHECK(ReadPixel(pRender, 50, 80, nColor));
    CHECK(nColor == kRedColor);
    //三角形外部未被填充
    CHECK(ReadPixel(pRender, 10, 10, nColor));
    CHECK(nColor == kWhiteColor);
}

void TestImageSamplingQuality(ui::IRender* pRender)
{
    CHECK(pRender->GetImageSamplingQuality() == ui::ImageSamplingQuality::kNearest);

    pRender->SetImageSamplingQuality(ui::ImageSamplingQuality::kLinear);
    CHECK(pRender->GetImageSamplingQuality() == ui::ImageSamplingQuality::kLinear);

    pRender->SetImageSamplingQuality(ui::ImageSamplingQuality::kCubic);
    CHECK(pRender->GetImageSamplingQuality() == ui::ImageSamplingQuality::kCubic);

    pRender->SetImageSamplingQuality(ui::ImageSamplingQuality::kNearest);
    CHECK(pRender->GetImageSamplingQuality() == ui::ImageSamplingQuality::kNearest);
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
            std::unique_ptr<ui::IRender> pRender(pRenderFactory->CreateRender(nullptr, nullptr));
            Check(pRender != nullptr, "CreateRender() is null");
            if (pRender != nullptr) {
                TestEllipse(pRender.get());
                TestArc(pRender.get());
                TestPolygon(pRender.get());
                TestClipPath(pRender.get(), pRenderFactory);
                TestImageSamplingQuality(pRender.get());
            }
        }
    }

    std::cout << "render_shape: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "render_shape: FAILED" << std::endl;
        return 1;
    }
    std::cout << "render_shape: PASSED" << std::endl;
    return 0;
}
