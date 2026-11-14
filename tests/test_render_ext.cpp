// IRender 扩展接口（P1 文本/位图能力、P2 抽象能力）的单元测试
//
// 测试对象：
//  1. IFont：GetFontMetrics / MeasureTextWidth / GetFontInfo；
//  2. IBitmap：CreateBitmap(w,h) / IsValid / GetRowBytes / GetAlphaType /
//     GetImageSizeScale / LockPixelBits（只读）；
//  3. IRender：MeasureString 轻量重载、MakeImageSnapshot(rc)、
//     SaveLayer/RestoreLayer、SetBlendMode/GetBlendMode、SetTransform；
//  4. IMatrix：Reset / Concat / Invert / IsIdentity / MapPoint；
//  5. IPen：SetAntiAlias / SetMiterLimit / SetDashPattern；
//  6. IBrush：线性渐变填充。
//
// 本测试使用离屏渲染，不创建窗口；需要 duilib 的资源目录（编译期宏
// DUILIB_TEST_RESOURCE_DIR，默认指向源码树的 bin/resources）。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cmath>
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

/** 创建测试字体（初始化失败时返回nullptr） */
std::unique_ptr<ui::IFont> CreateTestFont(ui::IRenderFactory* pRenderFactory)
{
    std::unique_ptr<ui::IFont> pFont(pRenderFactory->CreateIFont());
    if (pFont == nullptr) {
        return nullptr;
    }
    ui::UiFont fontInfo;
    fontInfo.m_fontName = _T("Segoe UI");
    fontInfo.m_fontSize = 16;
    if (!pFont->InitFont(fontInfo)) {
        return nullptr;
    }
    return pFont;
}

void TestFont(const std::unique_ptr<ui::IFont>& pFont)
{
    CHECK(pFont != nullptr);
    if (pFont == nullptr) {
        return;
    }

    //字体度量
    ui::FontMetrics metrics;
    CHECK(pFont->GetFontMetrics(metrics));
    CHECK(metrics.fLineHeight > 0.0f);
    CHECK(metrics.fDescent > metrics.fAscent);

    //文本宽度
    CHECK(pFont->MeasureTextWidth(_T("Hello")) > 0.0f);
    CHECK(pFont->MeasureTextWidth(DString()) == 0.0f);
    CHECK(pFont->MeasureTextWidth(_T("Hello")) < pFont->MeasureTextWidth(_T("Hello World")));

    //字体信息
    ui::UiFont fontInfo;
    CHECK(pFont->GetFontInfo(fontInfo));
    CHECK(fontInfo.m_fontSize == 16);
    CHECK(!fontInfo.m_fontName.empty());
}

void TestBitmap(ui::IRenderFactory* pRenderFactory)
{
    std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap(8, 4, ui::AlphaType::kPremul));
    CHECK(pBitmap != nullptr);
    if (pBitmap != nullptr) {
        CHECK(pBitmap->IsValid());
        CHECK(pBitmap->GetSize().cx == 8);
        CHECK(pBitmap->GetSize().cy == 4);
        CHECK(pBitmap->GetRowBytes() == 8 * 4);
        CHECK(pBitmap->GetAlphaType() == ui::AlphaType::kPremul);
        CHECK(pBitmap->GetImageSizeScale() == 1.0f);
        const ui::IBitmap* pConstBitmap = pBitmap.get();
        CHECK(pConstBitmap->LockPixelBits() != nullptr);
    }

    //未初始化的位图不是有效位图
    std::unique_ptr<ui::IBitmap> pEmpty(pRenderFactory->CreateBitmap());
    CHECK(pEmpty != nullptr);
    if (pEmpty != nullptr) {
        CHECK(!pEmpty->IsValid());
        const ui::IBitmap* pConstEmpty = pEmpty.get();
        CHECK(pConstEmpty->LockPixelBits() == nullptr);
    }
}

void TestMatrix(ui::IRenderFactory* pRenderFactory)
{
    std::unique_ptr<ui::IMatrix> pMatrix(pRenderFactory->CreateMatrix());
    CHECK(pMatrix != nullptr);
    if (pMatrix == nullptr) {
        return;
    }

    CHECK(pMatrix->IsIdentity());
    pMatrix->Translate(10.0f, 20.0f);
    CHECK(!pMatrix->IsIdentity());

    ui::UiPointF pt = pMatrix->MapPoint(ui::UiPointF(1.0f, 2.0f));
    CHECK(pt.x == 11.0f);
    CHECK(pt.y == 22.0f);

    //求逆后可以还原坐标
    CHECK(pMatrix->Invert());
    pt = pMatrix->MapPoint(ui::UiPointF(11.0f, 22.0f));
    CHECK(std::fabs(pt.x - 1.0f) < 0.01f);
    CHECK(std::fabs(pt.y - 2.0f) < 0.01f);

    //重置为单位矩阵
    pMatrix->Reset();
    CHECK(pMatrix->IsIdentity());

    //矩阵叠加：先平移(10,0)，再叠加平移(0,5)
    pMatrix->Translate(10.0f, 0.0f);
    std::unique_ptr<ui::IMatrix> pMatrix2(pRenderFactory->CreateMatrix());
    CHECK(pMatrix2 != nullptr);
    if (pMatrix2 != nullptr) {
        pMatrix2->Translate(0.0f, 5.0f);
        pMatrix->Concat(pMatrix2.get());
        pt = pMatrix->MapPoint(ui::UiPointF(0.0f, 0.0f));
        CHECK(pt.x == 10.0f);
        CHECK(pt.y == 5.0f);
    }
}

void TestPen(ui::IRenderFactory* pRenderFactory)
{
    std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kRedColor), 2.0f));
    CHECK(pPen != nullptr);
    if (pPen == nullptr) {
        return;
    }

    //抗锯齿
    CHECK(pPen->IsAntiAlias());
    pPen->SetAntiAlias(false);
    CHECK(!pPen->IsAntiAlias());
    pPen->SetAntiAlias(true);
    CHECK(pPen->IsAntiAlias());

    //斜接限制
    pPen->SetMiterLimit(8.0f);
    CHECK(pPen->GetMiterLimit() == 8.0f);
    //非法值被修正为1
    pPen->SetMiterLimit(0.0f);
    CHECK(pPen->GetMiterLimit() == 1.0f);

    //自定义虚线样式
    const float intervals[2] = { 4.0f, 2.0f };
    pPen->SetDashPattern(intervals, 2);
    CHECK(pPen->HasDashPattern());
    //设置预设样式后，自定义样式被清除
    pPen->SetDashStyle(ui::IPen::kDashStyleDash);
    CHECK(!pPen->HasDashPattern());
}

void TestMeasureString(ui::IRender* pRender, const std::unique_ptr<ui::IFont>& pFont, bool bFontValid)
{
    CHECK(pRender->Resize(100, 100));
    if (!bFontValid) {
        return;
    }
    const ui::UiRect rcText = pRender->MeasureString(_T("Hello"), pFont.get());
    CHECK(rcText.Width() > 0);
    CHECK(rcText.Height() > 0);
}

void TestBlendMode(ui::IRender* pRender)
{
    CHECK(pRender->GetBlendMode() == ui::BlendMode::kSrcOver);
    pRender->SetBlendMode(ui::BlendMode::kMultiply);
    CHECK(pRender->GetBlendMode() == ui::BlendMode::kMultiply);
    pRender->SetBlendMode(ui::BlendMode::kSrcOver);
    CHECK(pRender->GetBlendMode() == ui::BlendMode::kSrcOver);
}

void TestTransform(ui::IRender* pRender, ui::IRenderFactory* pRenderFactory)
{
    CHECK(pRender->Resize(100, 100));
    pRender->Clear(ui::UiColor(kWhiteColor));

    std::unique_ptr<ui::IMatrix> pMatrix(pRenderFactory->CreateMatrix());
    CHECK(pMatrix != nullptr);
    if (pMatrix == nullptr) {
        return;
    }
    pMatrix->Translate(30.0f, 0.0f);
    pRender->SetTransform(pMatrix.get());
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 20.0f, 20.0f), ui::UiColor(kRedColor));
    pRender->SetTransform(nullptr);

    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender, 45, 15, nColor));
    CHECK(nColor == kRedColor);
    CHECK(ReadPixel(pRender, 15, 15, nColor));
    CHECK(nColor == kWhiteColor);
}

void TestSaveLayer(ui::IRender* pRender)
{
    CHECK(pRender->Resize(100, 100));
    pRender->Clear(ui::UiColor(kWhiteColor));

    //50%透明度的蓝色图层叠加在白色背景上
    pRender->SaveLayer(ui::UiRect(0, 0, 100, 100), 128);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 100.0f, 100.0f), ui::UiColor(kBlueColor));
    pRender->RestoreLayer();

    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender, 50, 50, nColor));
    const uint32_t nR = (nColor >> 16) & 0xFF;
    const uint32_t nG = (nColor >> 8) & 0xFF;
    const uint32_t nB = nColor & 0xFF;
    CHECK(nColor != kWhiteColor);
    CHECK(nColor != kBlueColor);
    CHECK(nB == 0xFF);
    CHECK((nR > 0x60) && (nR < 0xA0));
    CHECK((nG > 0x60) && (nG < 0xA0));
}

void TestSnapshotRect(ui::IRender* pRender)
{
    CHECK(pRender->Resize(100, 100));
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 20.0f, 20.0f), ui::UiColor(kRedColor));

    std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot(ui::UiRect(10, 10, 20, 20)));
    CHECK(pBitmap != nullptr);
    if (pBitmap != nullptr) {
        CHECK(pBitmap->GetSize().cx == 10);
        CHECK(pBitmap->GetSize().cy == 10);
        const uint32_t* pBits = (const uint32_t*)pBitmap->LockPixelBits();
        CHECK(pBits != nullptr);
        if (pBits != nullptr) {
            CHECK(pBits[0] == kRedColor);
        }
    }

    //无效区域返回nullptr
    CHECK(pRender->MakeImageSnapshot(ui::UiRect(200, 200, 220, 220)) == nullptr);
}

void TestBrushGradient(ui::IRender* pRender, ui::IRenderFactory* pRenderFactory)
{
    CHECK(pRender->Resize(100, 100));

    std::unique_ptr<ui::IBrush> pBrush(pRenderFactory->CreateBrush(ui::UiColor(kRedColor)));
    CHECK(pBrush != nullptr);
    if (pBrush == nullptr) {
        return;
    }
    pBrush->SetLinearGradient(ui::UiColor(kBlueColor), ui::UiPointF(0.0f, 0.0f), ui::UiPointF(100.0f, 0.0f));
    CHECK(pBrush->GetBrushType() == ui::IBrush::BrushType::kLinearGradient);

    std::unique_ptr<ui::IPath> pPath(pRenderFactory->CreatePath());
    CHECK(pPath != nullptr);
    if (pPath == nullptr) {
        return;
    }
    pPath->AddRect(ui::UiRect(0, 0, 100, 100));

    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillPath(pPath.get(), pBrush.get());

    uint32_t nColor = 0;
    //左侧接近起始颜色（红色）
    CHECK(ReadPixel(pRender, 3, 50, nColor));
    CHECK(((nColor >> 16) & 0xFF) > 0xC0);
    CHECK((nColor & 0xFF) < 0x40);
    //右侧接近结束颜色（蓝色）
    CHECK(ReadPixel(pRender, 96, 50, nColor));
    CHECK((nColor & 0xFF) > 0xC0);
    CHECK(((nColor >> 16) & 0xFF) < 0x40);
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
            std::unique_ptr<ui::IRender> pRender(pRenderFactory->CreateRender(nullptr, nullptr));
            Check(pRender != nullptr, "CreateRender() is null");
            if (pRender != nullptr) {
                std::unique_ptr<ui::IFont> pFont = CreateTestFont(pRenderFactory);
                const bool bFontValid = (pFont != nullptr);
                Check(bFontValid, "CreateTestFont() failed");

                TestFont(pFont);
                TestBitmap(pRenderFactory);
                TestMatrix(pRenderFactory);
                TestPen(pRenderFactory);
                TestMeasureString(pRender.get(), pFont, bFontValid);
                TestBlendMode(pRender.get());
                TestTransform(pRender.get(), pRenderFactory);
                TestSaveLayer(pRender.get());
                TestSnapshotRect(pRender.get());
                TestBrushGradient(pRender.get(), pRenderFactory);
            }
        }
    }

    std::cout << "render_ext: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "render_ext: FAILED" << std::endl;
        return 1;
    }
    std::cout << "render_ext: PASSED" << std::endl;
    return 0;
}
