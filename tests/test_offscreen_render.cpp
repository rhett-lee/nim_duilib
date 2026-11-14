//
// 离屏渲染（IRenderFactory::CreateOffscreenRender）单元测试
//
// 覆盖设计文档 docs/misc/IRender职责拆分与离屏渲染设计方案.md 中的用例：
//  A. 基础创建与尺寸        OS-01 ~ OS-06
//  B. 绘制与像素正确性      OS-07 ~ OS-12
//  C. 快照                  OS-13 ~ OS-17
//  D. Clone                 OS-18 ~ OS-20
//  E. 位图直绘              OS-21 ~ OS-22
//  F. 多后端                OS-23 ~ OS-28（GPU 不可用时打印 SKIP）
//  G. 线程与生命周期        OS-29 ~ OS-31
//  H. 迁移回归              OS-32 ~ OS-33（需窗口，打印 SKIP）
//  I. 位图导出              OS-35 ~ OS-36
//
// 本测试不创建窗口；未初始化尺寸的对象允许被安全使用。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/UiFont.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;
int32_t g_skipCount = 0;

void Check(bool condition, const char* message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}
#define CHECK(expr) Check((expr), #expr)

void Skip(const char* message)
{
    ++g_skipCount;
    std::cout << "[SKIP] " << message << std::endl;
}

const uint32_t kWhiteColor = 0xFFFFFFFFu;
const uint32_t kBlackColor = 0xFF000000u;
const uint32_t kRedColor   = 0xFFFF0000u;
const uint32_t kGreenColor = 0xFF00FF00u;
const uint32_t kBlueColor  = 0xFF0000FFu;

/** 读取Render中单个像素的颜色值 */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

/** 读取IBitmap中单个像素的颜色值 */
bool ReadBitmapPixel(ui::IBitmap* pBitmap, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if ((pBitmap == nullptr) || !pBitmap->IsValid()) {
        return false;
    }
    if ((x < 0) || (y < 0) || (x >= (int32_t)pBitmap->GetWidth()) || (y >= (int32_t)pBitmap->GetHeight())) {
        return false;
    }
    const void* pBits = pBitmap->LockPixelBits();
    if (pBits == nullptr) {
        return false;
    }
    const uint32_t* pLine = (const uint32_t*)((const uint8_t*)pBits + (size_t)y * pBitmap->GetRowBytes());
    color = pLine[x];
    pBitmap->UnLockPixelBits();
    return true;
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

// ---------------------------------------------------------------------------
// A. 基础创建与尺寸（OS-01 ~ OS-06）
// ---------------------------------------------------------------------------
void TestCreateAndResize(ui::IRenderFactory* pRenderFactory)
{
    //OS-01 正常创建
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(200, 100);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            CHECK(pRender->GetWidth() == 200);
            CHECK(pRender->GetHeight() == 100);
            CHECK(pRender->GetRenderBackendType() == ui::RenderBackendType::kRaster_BackendType);
        }
    }

    //OS-02 0x0 懒创建
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(0, 0);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            CHECK(pRender->GetWidth() == 0);
            CHECK(pRender->GetHeight() == 0);
            CHECK(pRender->Resize(64, 64));
            CHECK(pRender->GetWidth() == 64);
            CHECK(pRender->GetHeight() == 64);
        }
    }

    //OS-03 重新Resize（旧内容不保留）
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(32, 32);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            CHECK(pRender->Resize(64, 48));
            CHECK(pRender->GetWidth() == 64);
            CHECK(pRender->GetHeight() == 48);
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 0, 0, nColor));
            CHECK(nColor != kRedColor);
        }
    }

    //OS-04 非法尺寸
    {
        std::unique_ptr<ui::IRender> pNull = pRenderFactory->CreateOffscreenRender(0, 100);
        CHECK(pNull == nullptr);

        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(32, 32);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            CHECK(!pRender->Resize(-1, 10));
            CHECK(!pRender->Resize(10, 0));
            CHECK(pRender->GetWidth() == 32);
            CHECK(pRender->GetHeight() == 32);
        }
    }

    //OS-05 超大尺寸
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(20000, 20000);
        CHECK(pRender == nullptr);
    }

    //OS-06 未初始化就绘制
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(0, 0);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 10.0f, 10.0f), ui::UiColor(kBlueColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            CHECK(pBitmap == nullptr);
        }
    }
}

// ---------------------------------------------------------------------------
// B. 绘制与像素正确性（OS-07 ~ OS-12）
// ---------------------------------------------------------------------------
void TestRasterDraw(ui::IRenderFactory* pRenderFactory, ui::IFont* pFont)
{
    //OS-07 Clear
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 30);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 0, 0, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pRender.get(), 39, 0, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pRender.get(), 0, 29, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pRender.get(), 20, 15, nColor) && (nColor == kRedColor));
        }
    }

    //OS-08 图元位置 + OS-09 坐标原点
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 40);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kWhiteColor));
            pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 20.0f, 20.0f), ui::UiColor(kBlueColor));
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 15, 15, nColor) && (nColor == kBlueColor));
            CHECK(ReadPixel(pRender.get(), 5, 5, nColor) && (nColor == kWhiteColor));

            //OS-09：在(0,0)画1x1像素，验证左上角为坐标原点
            pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 1.0f, 1.0f), ui::UiColor(kRedColor));
            CHECK(ReadPixel(pRender.get(), 0, 0, nColor) && (nColor == kRedColor));
        }
    }

    //OS-10 裁剪
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(100, 100);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kWhiteColor));
            int32_t nState = pRender->ClipRect(ui::UiRect(0, 0, 50, 50));
            CHECK(nState >= 0);
            pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 100.0f, 100.0f), ui::UiColor(kRedColor));
            pRender->RestoreClip(nState);
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 10, 10, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pRender.get(), 80, 80, nColor) && (nColor == kWhiteColor));
        }
    }

    //OS-11 图层合成
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 40);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kWhiteColor));
            pRender->SaveLayer(ui::UiRect(0, 0, 40, 40), 128);
            pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 40.0f, 40.0f), ui::UiColor(kBlueColor));
            pRender->RestoreLayer();
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 20, 20, nColor));
            const uint32_t nR = (nColor >> 16) & 0xFF;
            const uint32_t nG = (nColor >> 8) & 0xFF;
            const uint32_t nB = nColor & 0xFF;
            CHECK((nR >= 0x70) && (nR <= 0x90));
            CHECK((nG >= 0x70) && (nG <= 0x90));
            CHECK(nB == 0xFF);
        }
    }

    //OS-12 文本
    if (pFont == nullptr) {
        Skip("OS-12: test font is not available");
    }
    else {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(200, 60);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            ui::UiRect rcMeasure = pRender->MeasureString(_T("Hello"), pFont, 0);
            CHECK(rcMeasure.Width() > 0);
            CHECK(rcMeasure.Height() > 0);

            pRender->Clear(ui::UiColor(kBlackColor));
            ui::DrawStringParam drawParam;
            drawParam.textRect = ui::UiRect(0, 0, 200, 60);
            drawParam.dwTextColor = ui::UiColor(kRedColor);
            drawParam.pFont = pFont;
            pRender->DrawString(_T("Hello"), drawParam);

            //统计非背景像素的横向范围
            int32_t nMinX = 200;
            int32_t nMaxX = -1;
            for (int32_t y = 0; y < 60; ++y) {
                for (int32_t x = 0; x < 200; ++x) {
                    uint32_t nColor = 0;
                    if (ReadPixel(pRender.get(), x, y, nColor) && (nColor != kBlackColor)) {
                        nMinX = std::min(nMinX, x);
                        nMaxX = std::max(nMaxX, x);
                    }
                }
            }
            CHECK(nMaxX >= nMinX);
            if (nMaxX >= nMinX) {
                const int32_t nDrawWidth = nMaxX - nMinX + 1;
                //绘制宽度不应超过测量宽度过多（允许抗锯齿带来的少量差异）
                CHECK(nDrawWidth <= rcMeasure.Width() + 2);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// C. 快照（OS-13 ~ OS-17）
// ---------------------------------------------------------------------------
void TestSnapshot(ui::IRenderFactory* pRenderFactory)
{
    //OS-13 整图快照
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 30);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);
            if (pBitmap != nullptr) {
                CHECK(pBitmap->GetWidth() == 40);
                CHECK(pBitmap->GetHeight() == 30);
                uint32_t nColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 0, 0, nColor) && (nColor == kRedColor));
                CHECK(ReadBitmapPixel(pBitmap.get(), 20, 15, nColor) && (nColor == kRedColor));
            }
        }
    }

    //OS-14 局部快照
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 40);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kWhiteColor));
            pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 30.0f, 30.0f), ui::UiColor(kRedColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot(ui::UiRect(10, 10, 30, 30)));
            CHECK(pBitmap != nullptr);
            if (pBitmap != nullptr) {
                CHECK(pBitmap->GetWidth() == 20);
                CHECK(pBitmap->GetHeight() == 20);
                uint32_t nColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 0, 0, nColor) && (nColor == kRedColor));
            }
        }
    }

    //OS-15 越界局部快照
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 40);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kWhiteColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot(ui::UiRect(-10, -10, 20, 20)));
            CHECK(pBitmap != nullptr);
            if (pBitmap != nullptr) {
                CHECK(pBitmap->GetWidth() == 20);
                CHECK(pBitmap->GetHeight() == 20);
            }
            std::unique_ptr<ui::IBitmap> pOutBitmap(pRender->MakeImageSnapshot(ui::UiRect(100, 100, 200, 200)));
            CHECK(pOutBitmap == nullptr);
        }
    }

    //OS-16 快照独立性
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(20, 20);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);
            pRender->Clear(ui::UiColor(kGreenColor));
            if (pBitmap != nullptr) {
                uint32_t nColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 10, 10, nColor) && (nColor == kRedColor));
            }
        }
    }

    //OS-17 快照用于绘制
    {
        std::unique_ptr<ui::IRender> pSrc = pRenderFactory->CreateOffscreenRender(20, 20);
        std::unique_ptr<ui::IRender> pDst = pRenderFactory->CreateOffscreenRender(40, 40);
        CHECK(pSrc != nullptr);
        CHECK(pDst != nullptr);
        if ((pSrc != nullptr) && (pDst != nullptr)) {
            pSrc->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pSrc->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);
            pDst->Clear(ui::UiColor(kWhiteColor));
            if (pBitmap != nullptr) {
                pDst->DrawImage(pBitmap.get(), ui::UiRect(5, 5, 25, 25));
            }
            uint32_t nColor = 0;
            CHECK(ReadPixel(pDst.get(), 4, 4, nColor) && (nColor == kWhiteColor));
            CHECK(ReadPixel(pDst.get(), 5, 5, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pDst.get(), 24, 24, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pDst.get(), 25, 25, nColor) && (nColor == kWhiteColor));
        }
    }
}

// ---------------------------------------------------------------------------
// D. Clone（OS-18 ~ OS-20）
// ---------------------------------------------------------------------------
void TestClone(ui::IRenderFactory* pRenderFactory)
{
    //OS-18 Clone基本
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(30, 20);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IRender> pClone(pRender->Clone());
            CHECK(pClone != nullptr);
            if (pClone != nullptr) {
                CHECK(pClone->GetWidth() == 30);
                CHECK(pClone->GetHeight() == 20);
                CHECK(pClone->GetRenderBackendType() == pRender->GetRenderBackendType());
                uint32_t nColor = 0;
                CHECK(ReadPixel(pClone.get(), 10, 10, nColor) && (nColor == kRedColor));
            }
        }
    }

    //OS-19 Clone独立性
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(20, 20);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IRender> pClone(pRender->Clone());
            CHECK(pClone != nullptr);
            pRender->Clear(ui::UiColor(kGreenColor));
            if (pClone != nullptr) {
                uint32_t nColor = 0;
                CHECK(ReadPixel(pClone.get(), 10, 10, nColor) && (nColor == kRedColor));
            }
        }
    }

    //OS-20 Clone属性（采样质量）
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(16, 16);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->SetImageSamplingQuality(ui::ImageSamplingQuality::kLinear);
            std::unique_ptr<ui::IRender> pClone(pRender->Clone());
            CHECK(pClone != nullptr);
            if (pClone != nullptr) {
                CHECK(pClone->GetImageSamplingQuality() == ui::ImageSamplingQuality::kLinear);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// E. 位图直绘（OS-21 ~ OS-22）
// ---------------------------------------------------------------------------
void TestBitmapRender(ui::IRenderFactory* pRenderFactory)
{
    //OS-21 直接绘制到IBitmap（零拷贝）
    {
        std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap(32, 24, ui::AlphaType::kPremul));
        CHECK(pBitmap != nullptr);
        if (pBitmap != nullptr) {
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateBitmapRender(pBitmap.get());
            CHECK(pRender != nullptr);
            if (pRender != nullptr) {
                CHECK(pRender->GetWidth() == 32);
                CHECK(pRender->GetHeight() == 24);
                CHECK(pRender->GetRenderBackendType() == ui::RenderBackendType::kRaster_BackendType);
                pRender->Clear(ui::UiColor(kBlueColor));
                //零拷贝：不经过快照，直接从位图读取像素
                uint32_t nColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 16, 12, nColor) && (nColor == kBlueColor));
                //位图尺寸不可调整
                CHECK(!pRender->Resize(64, 64));
                CHECK(pRender->Resize(32, 24));
            }
        }
    }

    //OS-22 无效位图时返回nullptr
    {
        std::unique_ptr<ui::IBitmap> pEmptyBitmap(pRenderFactory->CreateBitmap());
        CHECK(pEmptyBitmap != nullptr);
        if (pEmptyBitmap != nullptr) {
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateBitmapRender(pEmptyBitmap.get());
            CHECK(pRender == nullptr);
        }
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateBitmapRender(nullptr);
        CHECK(pRender == nullptr);
    }
}

// ---------------------------------------------------------------------------
// I. 位图导出（OS-35 ~ OS-36）
// ---------------------------------------------------------------------------
void TestBitmapExport(ui::IRenderFactory* pRenderFactory)
{
    //OS-35 导出BMP文件
    {
        std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap(8, 4, ui::AlphaType::kOpaque));
        CHECK(pBitmap != nullptr);
        if (pBitmap != nullptr) {
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateBitmapRender(pBitmap.get());
            CHECK(pRender != nullptr);
            if (pRender != nullptr) {
                pRender->Clear(ui::UiColor(kRedColor));
            }
            const ui::FilePath bmpFilePath(_T("offscreen_render_test.bmp"));
            CHECK(ui::ImageUtil::SaveBitmapToBmpFile(pBitmap.get(), bmpFilePath));
            CHECK(bmpFilePath.IsExistsFile());

            std::vector<uint8_t> fileData;
            CHECK(ui::FileUtil::ReadFileData(bmpFilePath, fileData));
            if (fileData.size() >= 58) {
                //文件头与信息头校验
                CHECK(fileData[0] == 'B');
                CHECK(fileData[1] == 'M');
                const uint16_t nBitCount = (uint16_t)(fileData[28] | (fileData[29] << 8));
                CHECK(nBitCount == 32);
                //BMP自下而上存储，首像素为左下角红色（内存字节序：B G R A）
                CHECK(fileData[54] == 0x00);
                CHECK(fileData[55] == 0x00);
                CHECK(fileData[56] == 0xFF);
                CHECK(fileData[57] == 0xFF);
            }
            std::error_code ec;
            std::filesystem::remove(bmpFilePath.NativePath(), ec);
        }
    }

    //OS-36 无效位图导出返回false
    {
        const ui::FilePath bmpFilePath(_T("offscreen_render_invalid.bmp"));
        CHECK(!ui::ImageUtil::SaveBitmapToBmpFile(nullptr, bmpFilePath));
        CHECK(!bmpFilePath.IsExistsFile());
    }
}

// ---------------------------------------------------------------------------
// F. 多后端（OS-23 ~ OS-28）
// ---------------------------------------------------------------------------
void TestBackends(ui::IRenderFactory* pRenderFactory)
{
    //OS-23 能力查询：Raster恒为true
    CHECK(pRenderFactory->IsOffscreenBackendSupported(ui::RenderBackendType::kRaster_BackendType));

    //OS-24 GPU能力查询（返回平台实际能力）
    const bool bGpuSupported = pRenderFactory->IsOffscreenBackendSupported(ui::RenderBackendType::kNativeGL_BackendType);
    std::cout << "[INFO] GPU offscreen support: " << (bGpuSupported ? "true" : "false") << std::endl;

    if (!bGpuSupported) {
        //OS-26 GPU不可用时，显式请求返回nullptr，不静默回退
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(32, 32, nullptr, ui::RenderBackendType::kNativeGL_BackendType);
        CHECK(pRender == nullptr);
        Skip("OS-25/OS-27/OS-28: GPU offscreen backend is not supported on this platform");
        return;
    }

    //OS-25 GPU离屏冒烟
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(40, 40, nullptr, ui::RenderBackendType::kNativeGL_BackendType);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            CHECK(pRender->GetRenderBackendType() == ui::RenderBackendType::kNativeGL_BackendType);
            pRender->Clear(ui::UiColor(kRedColor));
            pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 20.0f, 20.0f), ui::UiColor(kBlueColor));
            uint32_t nColor = 0;
            CHECK(ReadPixel(pRender.get(), 0, 0, nColor) && (nColor == kRedColor));
            CHECK(ReadPixel(pRender.get(), 15, 15, nColor) && (nColor == kBlueColor));

            //OS-27 GPU快照回读
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);
            if (pBitmap != nullptr) {
                CHECK(pBitmap->GetWidth() == 40);
                CHECK(pBitmap->GetHeight() == 40);
                uint32_t nBitmapColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 15, 15, nBitmapColor));
                CHECK(nBitmapColor == kBlueColor);
            }
            //OS-28：在创建线程析构（离开作用域时自动析构）
        }
    }
}

// ---------------------------------------------------------------------------
// G. 线程与生命周期（OS-29 ~ OS-31）
// ---------------------------------------------------------------------------
void TestThreadAndLifetime(ui::IRenderFactory* pRenderFactory)
{
    //OS-29 子线程Raster
    {
        bool bOk = false;
        std::thread worker([pRenderFactory, &bOk]() {
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(24, 24);
            if (pRender == nullptr) {
                return;
            }
            pRender->Clear(ui::UiColor(kGreenColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            uint32_t nColor = 0;
            bOk = (pBitmap != nullptr) && ReadBitmapPixel(pBitmap.get(), 12, 12, nColor) && (nColor == kGreenColor);
        });
        worker.join();
        CHECK(bOk);
    }

    //OS-30 重复创建销毁
    {
        bool bOk = true;
        for (int32_t i = 0; i < 200; ++i) {
            std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(16, 16);
            if (pRender == nullptr) {
                bOk = false;
                break;
            }
            pRender->Clear(ui::UiColor(kBlueColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            if (pBitmap == nullptr) {
                bOk = false;
                break;
            }
        }
        CHECK(bOk);
    }

    //OS-31 快照跨线程
    {
        std::unique_ptr<ui::IRender> pRender = pRenderFactory->CreateOffscreenRender(16, 16);
        CHECK(pRender != nullptr);
        if (pRender != nullptr) {
            pRender->Clear(ui::UiColor(kRedColor));
            std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);

            int32_t nSuccessCount = 0;
            std::thread worker([&pBitmap, &nSuccessCount]() {
                //pBitmap的所有权仍在主线程，这里只读取内容
                for (int32_t i = 0; i < 100; ++i) {
                    uint32_t nColor = 0;
                    if (ReadBitmapPixel(pBitmap.get(), 8, 8, nColor) && (nColor == kRedColor)) {
                        ++nSuccessCount;
                    }
                }
            });
            worker.join();
            CHECK(nSuccessCount == 100);
        }
    }
}

// ---------------------------------------------------------------------------
// H. 迁移回归（OS-32 ~ OS-33）
// ---------------------------------------------------------------------------
void TestCompatEntry(ui::IRenderFactory* /*pRenderFactory*/)
{
    //OS-32/OS-33 需要创建窗口控件，本测试不创建窗口，改为在集成环境中验证
    Skip("OS-32: Control::CreateTempRender migration requires a window");
    Skip("OS-33: ControlDragable migration requires a window");
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
            std::unique_ptr<ui::IFont> pFont = CreateTestFont(pRenderFactory);

            TestCreateAndResize(pRenderFactory);
            TestRasterDraw(pRenderFactory, pFont.get());
            TestSnapshot(pRenderFactory);
            TestClone(pRenderFactory);
            TestBitmapRender(pRenderFactory);
            TestBackends(pRenderFactory);
            TestThreadAndLifetime(pRenderFactory);
            TestCompatEntry(pRenderFactory);
            TestBitmapExport(pRenderFactory);
        }
    }

    std::cout << "offscreen_render: " << g_checkCount << " checks, "
              << g_failedCount << " failed, " << g_skipCount << " skipped" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "offscreen_render: FAILED" << std::endl;
        return 1;
    }
    std::cout << "offscreen_render: PASSED" << std::endl;
    return 0;
}
