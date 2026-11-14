// IRender 接口的单元测试（补充覆盖）
//
// 本文件补齐 test_render_shape / test_render_ext / test_image_draw / test_text_draw
// 未覆盖到的接口，覆盖范围：
//  1. RA-01 ~ RA-06  坐标系：SetWindowOrg / OffsetWindowOrg / GetWindowOrg
//  2. RA-07 ~ RA-14  变换：IMatrix 全部变换、ConcatMatrix、SetTransform
//  3. RA-15 ~ RA-25  裁剪：SaveClip / RestoreClip / ClipRect(交集/差集) /
//                    ClipRoundRect / ClipPath / GetClipInfo / IsClipEmpty
//  4. RA-26 ~ RA-29  图层：SaveLayer / RestoreLayer
//  5. RA-30 ~ RA-38  位块传输：BitBlt / StretchBlt / AlphaBlend 及越界裁剪
//  6. RA-39 ~ RA-56  图形绘制：DrawLine / DrawRect / FillRect / DrawRoundRect /
//                    FillRoundRect / DrawArc / FillArc / DrawCircle / FillCircle /
//                    DrawEllipse / FillEllipse / DrawPolygon / FillPolygon /
//                    DrawPath / FillPath / FillPath(渐变) / DrawBoxShadow
//  7. RA-57 ~ RA-60  Alpha 处理：ClearAlpha / RestoreAlpha
//  8. RA-61 ~ RA-67  像素操作：ClearRect / ReadPixels / WritePixels（两个重载）/ IsEmpty
//  9. RA-68 ~ RA-70  快照与克隆：MakeImageSnapshot(rc) 的非法参数、Clone 独立性
// 10. RA-71 ~ RA-85  IPen / IBrush / IPath / IMatrix / IBitmap 属性与 Clone
//
// 全部使用离屏渲染，不创建窗口。

#if defined(_WIN32)
#include <crtdbg.h>
#include <windows.h>
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/StringConvert.h"

#include <cmath>
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

/** 输出当前测试小节的标题，便于定位失败用例（与失败信息使用同一输出流，保证顺序一致） */
void Section(const char* name)
{
    std::cerr << "--- " << name << std::endl;
}

/** 常用颜色值（ARGB格式） */
const uint32_t kWhiteColor = 0xFFFFFFFFu;
const uint32_t kBlackColor = 0xFF000000u;
const uint32_t kRedColor   = 0xFFFF0000u;
const uint32_t kGreenColor = 0xFF00FF00u;
const uint32_t kBlueColor  = 0xFF0000FFu;

/** 读取Render中单个像素的颜色值（ARGB格式） */
bool ReadPixel(ui::IRender* pRender, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if (pRender == nullptr) {
        return false;
    }
    return pRender->ReadPixels(ui::UiRect(x, y, x + 1, y + 1), &color, sizeof(color));
}

/** 判断两个颜色是否接近（用于抗锯齿/渐变导致的轻微误差），不匹配时输出实际颜色值便于定位 */
bool ColorNear(uint32_t color, uint32_t expected, int32_t tolerance = 6)
{
    for (int32_t i = 0; i < 32; i += 8) {
        const int32_t a = (int32_t)((color >> i) & 0xFF);
        const int32_t b = (int32_t)((expected >> i) & 0xFF);
        if (std::abs(a - b) > tolerance) {
            char buf[128] = { 0 };
            std::snprintf(buf, sizeof(buf), "[COLOR] actual=0x%08X, expected=0x%08X", color, expected);
            std::cerr << buf << std::endl;
            return false;
        }
    }
    return true;
}

/** 创建离屏渲染对象 */
std::unique_ptr<ui::IRender> CreateRender(ui::IRenderFactory* pRenderFactory, int32_t w, int32_t h)
{
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    return pRenderFactory->CreateOffscreenRender(w, h);
}

/** 创建指定尺寸、指定纯色的离屏渲染对象（用于位块传输的源） */
std::unique_ptr<ui::IRender> CreateSolidRender(ui::IRenderFactory* pRenderFactory,
                                               int32_t w, int32_t h, uint32_t color)
{
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, w, h);
    if (pRender != nullptr) {
        pRender->Clear(ui::UiColor(color));
    }
    return pRender;
}

// ---------------------------------------------------------------------------
// 1. 坐标系（RA-01 ~ RA-06）
// ---------------------------------------------------------------------------
void TestWindowOrg(ui::IRenderFactory* pRenderFactory)
{
    Section("TestWindowOrg");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 60, 60);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //RA-01 默认原点为(0,0)
    CHECK(pRender->GetWindowOrg().x == 0);
    CHECK(pRender->GetWindowOrg().y == 0);

    //RA-02 SetWindowOrg：把指定点映射为视区原点，返回旧原点
    ui::UiPoint oldOrg = pRender->SetWindowOrg(ui::UiPoint(10, 20));
    CHECK(oldOrg.x == 0);
    CHECK(oldOrg.y == 0);
    CHECK(pRender->GetWindowOrg().x == 10);
    CHECK(pRender->GetWindowOrg().y == 20);

    //RA-03 视区原点语义：原点(10,20)表示"客户区坐标(10,20)映射到设备坐标(0,0)"
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(10.0f, 20.0f, 15.0f, 25.0f), ui::UiColor(kRedColor));
    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender.get(), 12, 22, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 30, 25, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-04 OffsetWindowOrg：原点从(10,20)增加(5,-10)后变成(15,10)
    oldOrg = pRender->OffsetWindowOrg(ui::UiPoint(5, -10));
    CHECK(oldOrg.x == 10);
    CHECK(oldOrg.y == 20);
    CHECK(pRender->GetWindowOrg().x == 15);
    CHECK(pRender->GetWindowOrg().y == 10);

    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(15.0f, 10.0f, 20.0f, 15.0f), ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 17, 12, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //RA-05 恢复原点后，绘制坐标与像素坐标一致
    pRender->SetWindowOrg(ui::UiPoint(0, 0));
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(1.0f, 1.0f, 6.0f, 6.0f), ui::UiColor(kGreenColor));
    CHECK(ReadPixel(pRender.get(), 3, 3, nColor));
    CHECK(ColorNear(nColor, kGreenColor));

    //RA-06 原点偏移不影响裁剪区域判断（裁剪区域按客户区坐标换算到设备坐标）
    pRender->Clear(ui::UiColor(kWhiteColor));
    const int32_t nState = pRender->ClipRect(ui::UiRect(0, 0, 30, 60));
    CHECK(nState >= 0);
    pRender->OffsetWindowOrg(ui::UiPoint(20, 0));
    //客户区(20,0)-(50,10)映射到设备坐标(0,0)-(30,10)，恰好被裁剪区域(0,0,30,60)裁到x<30
    pRender->FillRect(ui::UiRectF(20.0f, 0.0f, 50.0f, 10.0f), ui::UiColor(kRedColor));
    pRender->RestoreClip(nState);
    CHECK(ReadPixel(pRender.get(), 25, 5, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 55, 5, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    pRender->SetWindowOrg(ui::UiPoint(0, 0));
}

// ---------------------------------------------------------------------------
// 2. 变换：IMatrix 与 ConcatMatrix / SetTransform（RA-07 ~ RA-14）
// ---------------------------------------------------------------------------
void TestMatrix(ui::IRenderFactory* pRenderFactory)
{
    Section("TestMatrix");
    CHECK(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    //RA-07 单位矩阵
    std::unique_ptr<ui::IMatrix> pMatrix(pRenderFactory->CreateMatrix());
    CHECK(pMatrix != nullptr);
    if (pMatrix == nullptr) {
        return;
    }
    CHECK(pMatrix->IsIdentity());
    ui::UiPointF pt = pMatrix->MapPoint(ui::UiPointF(3.0f, 4.0f));
    CHECK(pt.x == 3.0f);
    CHECK(pt.y == 4.0f);

    //RA-08 平移
    pMatrix->Translate(10.0f, 20.0f);
    CHECK(!pMatrix->IsIdentity());
    pt = pMatrix->MapPoint(ui::UiPointF(3.0f, 4.0f));
    CHECK(pt.x == 13.0f);
    CHECK(pt.y == 24.0f);

    //RA-09 逆变换可还原
    CHECK(pMatrix->Invert());
    pt = pMatrix->MapPoint(ui::UiPointF(13.0f, 24.0f));
    CHECK(std::fabs(pt.x - 3.0f) < 0.001f);
    CHECK(std::fabs(pt.y - 4.0f) < 0.001f);

    //RA-10 缩放（带中心点）
    pMatrix->Reset();
    pMatrix->Scale(2.0f, 2.0f, 10.0f, 10.0f);
    pt = pMatrix->MapPoint(ui::UiPointF(10.0f, 10.0f));
    CHECK(std::fabs(pt.x - 10.0f) < 0.001f);
    pt = pMatrix->MapPoint(ui::UiPointF(20.0f, 10.0f));
    CHECK(std::fabs(pt.x - 30.0f) < 0.001f);

    //RA-11 旋转 90 度
    pMatrix->Reset();
    pMatrix->RotateAt(90.0f, 0.0f, 0.0f);
    pt = pMatrix->MapPoint(ui::UiPointF(1.0f, 0.0f));
    CHECK(std::fabs(pt.x) < 0.001f);
    CHECK(std::fabs(pt.y - 1.0f) < 0.001f);

    //RA-12 倾斜（Skew）：参数是错切因子（不是角度），x' = x + kx * y
    pMatrix->Reset();
    pMatrix->Skew(2.0f, 0.0f);
    pt = pMatrix->MapPoint(ui::UiPointF(0.0f, 1.0f));
    CHECK(std::fabs(pt.x - 2.0f) < 0.01f);
    CHECK(std::fabs(pt.y - 1.0f) < 0.01f);

    //RA-13 Concat：矩阵合并
    pMatrix->Reset();
    std::unique_ptr<ui::IMatrix> pMatrix2(pRenderFactory->CreateMatrix());
    CHECK(pMatrix2 != nullptr);
    if (pMatrix2 != nullptr) {
        pMatrix->Translate(10.0f, 0.0f);
        pMatrix2->Scale(2.0f, 2.0f);
        pMatrix->Concat(pMatrix2.get());
        pt = pMatrix->MapPoint(ui::UiPointF(1.0f, 1.0f));
        //先平移(10,0)，再缩放2倍
        CHECK(std::fabs(pt.x - 22.0f) < 0.001f);
        CHECK(std::fabs(pt.y - 2.0f) < 0.001f);
    }

    //RA-14 IRender 的 ConcatMatrix / SetTransform
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 60, 60);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    pRender->Clear(ui::UiColor(kWhiteColor));

    std::unique_ptr<ui::IMatrix> pTranslate(pRenderFactory->CreateMatrix());
    pTranslate->Translate(20.0f, 0.0f);
    pRender->ConcatMatrix(pTranslate.get());
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 10.0f, 10.0f), ui::UiColor(kRedColor));
    uint32_t nColor = 0;
    CHECK(ReadPixel(pRender.get(), 25, 5, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 5, 5, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //SetTransform 替换当前变换（此处传nullptr恢复单位矩阵）
    pRender->SetTransform(nullptr);
    pRender->FillRect(ui::UiRectF(30.0f, 30.0f, 40.0f, 40.0f), ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 35, 35, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //SetTransform(nullptr) 后 ConcatMatrix(nullptr) 不应改变任何状态
    pRender->ConcatMatrix(nullptr);
    pRender->FillRect(ui::UiRectF(0.0f, 50.0f, 10.0f, 60.0f), ui::UiColor(kGreenColor));
    CHECK(ReadPixel(pRender.get(), 5, 55, nColor));
    CHECK(ColorNear(nColor, kGreenColor));
}

// ---------------------------------------------------------------------------
// 3. 裁剪（RA-15 ~ RA-25）
// ---------------------------------------------------------------------------
void TestClip(ui::IRenderFactory* pRenderFactory)
{
    Section("TestClip");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 80, 80);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;

    //RA-15 初始状态没有裁剪区域
    CHECK(!pRender->IsClipEmpty());
    {
        std::vector<ui::UiRect> clipRects;
        const ui::RenderClipType clipType = pRender->GetClipInfo(clipRects);
        CHECK(clipType != ui::RenderClipType::kRegion);
    }

    //RA-16 SaveClip / RestoreClip 恢复裁剪状态
    pRender->Clear(ui::UiColor(kWhiteColor));
    int32_t nSaveState = -1;
    pRender->SaveClip(nSaveState);
    CHECK(nSaveState >= 0);
    const int32_t nClipState = pRender->ClipRect(ui::UiRect(0, 0, 20, 20));
    CHECK(nClipState >= 0);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 40.0f, 40.0f), ui::UiColor(kRedColor));
    pRender->RestoreClip(nSaveState);
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-17 裁剪区域信息（矩形）
    {
        std::vector<ui::UiRect> clipRects;
        const int32_t nState = pRender->ClipRect(ui::UiRect(10, 10, 50, 50));
        CHECK(pRender->GetClipInfo(clipRects) == ui::RenderClipType::kRect);
        CHECK(!clipRects.empty());
        if (!clipRects.empty()) {
            CHECK(clipRects[0].left == 10);
            CHECK(clipRects[0].top == 10);
            CHECK(clipRects[0].right == 50);
            CHECK(clipRects[0].bottom == 50);
        }
        CHECK(!pRender->IsClipEmpty());
        pRender->RestoreClip(nState);
    }

    //RA-18 裁剪区域与画布无交集时为空裁剪
    {
        const int32_t nState = pRender->ClipRect(ui::UiRect(200, 200, 300, 300));
        CHECK(pRender->IsClipEmpty());
        pRender->RestoreClip(nState);
        CHECK(!pRender->IsClipEmpty());
    }

    //RA-19 ClipRect(bIntersect=false)：从裁剪区域中减去指定矩形
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        const int32_t nState = pRender->ClipRect(ui::UiRect(0, 0, 40, 40));
        const int32_t nState2 = pRender->ClipRect(ui::UiRect(10, 10, 20, 20), false);
        pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 40.0f, 40.0f), ui::UiColor(kBlueColor));
        pRender->RestoreClip(nState2);
        pRender->RestoreClip(nState);
    }
    CHECK(ReadPixel(pRender.get(), 5, 5, nColor));
    CHECK(ColorNear(nColor, kBlueColor));
    CHECK(ReadPixel(pRender.get(), 15, 15, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-20 ClipRoundRect：圆角外的内容被裁剪
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        const int32_t nState = pRender->ClipRoundRect(ui::UiRect(20, 20, 60, 60), 16.0f, 16.0f);
        CHECK(nState >= 0);
        pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 80.0f, 80.0f), ui::UiColor(kGreenColor));
        pRender->RestoreClip(nState);
    }
    CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
    CHECK(ColorNear(nColor, kGreenColor));
    CHECK(ReadPixel(pRender.get(), 21, 21, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-21 ClipPath：只保留路径内的内容
    pRender->Clear(ui::UiColor(kWhiteColor));
    std::unique_ptr<ui::IPath> pPath(pRenderFactory->CreatePath());
    CHECK(pPath != nullptr);
    if (pPath != nullptr) {
        pPath->AddEllipse(ui::UiRect(10, 10, 70, 70));
        const int32_t nState = pRender->ClipPath(pPath.get());
        CHECK(nState >= 0);
        pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 80.0f, 80.0f), ui::UiColor(kRedColor));
        pRender->RestoreClip(nState);
        CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
        CHECK(ColorNear(nColor, kRedColor));
        CHECK(ReadPixel(pRender.get(), 12, 12, nColor));
        CHECK(ColorNear(nColor, kWhiteColor));
    }

    //RA-22 ClipPath 传入nullptr：返回错误标志 -1
    CHECK(pRender->ClipPath(nullptr) == -1);

    //RA-23 非法裁剪矩形（空矩形）：返回有效标志，且裁剪为空
    {
        const int32_t nState = pRender->ClipRect(ui::UiRect(10, 10, 10, 10));
        CHECK(nState >= 0);
        CHECK(pRender->IsClipEmpty());
        pRender->RestoreClip(nState);
    }

    //RA-24 多层裁剪叠加：交集生效
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        const int32_t nState1 = pRender->ClipRect(ui::UiRect(0, 0, 40, 40));
        const int32_t nState2 = pRender->ClipRect(ui::UiRect(20, 20, 60, 60));
        pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 80.0f, 80.0f), ui::UiColor(kBlueColor));
        pRender->RestoreClip(nState2);
        pRender->RestoreClip(nState1);
    }
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK(ColorNear(nColor, kBlueColor));
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-25 裁剪恢复后可以正常绘制整个画布
    pRender->Clear(ui::UiColor(kBlackColor));
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 80.0f, 80.0f), ui::UiColor(kWhiteColor));
    CHECK(ReadPixel(pRender.get(), 79, 79, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
}

// ---------------------------------------------------------------------------
// 4. 图层（RA-26 ~ RA-29）
// ---------------------------------------------------------------------------
void TestLayer(ui::IRenderFactory* pRenderFactory)
{
    Section("TestLayer");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 60, 60);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;

    //RA-26 图层整体透明度：白色背景上绘制不透明红色图层，alpha=128 得到混合色
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->SaveLayer(ui::UiRect(0, 0, 60, 60), 128);
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 50.0f, 50.0f), ui::UiColor(kRedColor));
    pRender->RestoreLayer();
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK((nColor & 0x00FF0000u) != 0);//仍为偏红色
    CHECK((nColor & 0x0000FF00u) != 0);//绿色通道被白色背景提亮
    CHECK(nColor != kRedColor);
    CHECK(nColor != kWhiteColor);

    //RA-27 图层区域外不绘制
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->SaveLayer(ui::UiRect(0, 0, 20, 20), 255);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 60.0f, 60.0f), ui::UiColor(kBlueColor));
    pRender->RestoreLayer();
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kBlueColor));
    CHECK(ReadPixel(pRender.get(), 40, 40, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-28 空矩形表示整个画布
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->SaveLayer(ui::UiRect(), 255);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 60.0f, 60.0f), ui::UiColor(kGreenColor));
    pRender->RestoreLayer();
    CHECK(ReadPixel(pRender.get(), 59, 59, nColor));
    CHECK(ColorNear(nColor, kGreenColor));

    //RA-29 多个图层嵌套使用
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->SaveLayer(ui::UiRect(0, 0, 60, 60), 255);
    pRender->SaveLayer(ui::UiRect(0, 0, 30, 30), 255);
    pRender->FillRect(ui::UiRectF(0.0f, 0.0f, 60.0f, 60.0f), ui::UiColor(kRedColor));
    pRender->RestoreLayer();
    pRender->RestoreLayer();
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 45, 45, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
}

// ---------------------------------------------------------------------------
// 5. 位块传输（RA-30 ~ RA-38）
// ---------------------------------------------------------------------------
void TestBlt(ui::IRenderFactory* pRenderFactory)
{
    Section("TestBlt");
    std::unique_ptr<ui::IRender> pSrc = CreateSolidRender(pRenderFactory, 20, 20, kRedColor);
    std::unique_ptr<ui::IRender> pDst = CreateRender(pRenderFactory, 60, 60);
    CHECK(pSrc != nullptr);
    CHECK(pDst != nullptr);
    if ((pSrc == nullptr) || (pDst == nullptr)) {
        return;
    }
    uint32_t nColor = 0;

    //RA-30 BitBlt 基本复制
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->BitBlt(10, 10, 20, 20, pSrc.get(), 0, 0, ui::RopMode::kSrcCopy));
    CHECK(ReadPixel(pDst.get(), 20, 20, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pDst.get(), 5, 5, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-31 BitBlt 越界裁剪（目标区域部分在画布外）
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->BitBlt(-10, -10, 20, 20, pSrc.get(), 0, 0, ui::RopMode::kSrcCopy));
    CHECK(ReadPixel(pDst.get(), 2, 2, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pDst.get(), 15, 15, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-32 BitBlt 源区域越界：源区域的负坐标部分被裁剪，其余部分按偏移绘制
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->BitBlt(0, 0, 20, 20, pSrc.get(), -10, -10, ui::RopMode::kSrcCopy));
    CHECK(ReadPixel(pDst.get(), 2, 2, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    CHECK(ReadPixel(pDst.get(), 12, 12, nColor));
    CHECK(ColorNear(nColor, kRedColor));

    //RA-34 BitBlt 尺寸为0时返回失败
    CHECK(!pDst->BitBlt(0, 0, 0, 10, pSrc.get(), 0, 0, ui::RopMode::kSrcCopy));

    //RA-35 StretchBlt 拉伸复制
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->StretchBlt(0, 0, 40, 40, pSrc.get(), 0, 0, 20, 20, ui::RopMode::kSrcCopy));
    CHECK(ReadPixel(pDst.get(), 20, 20, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pDst.get(), 39, 39, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pDst.get(), 45, 45, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-36 StretchBlt 源区域超出范围时返回失败（源区域无效）
    CHECK(!pDst->StretchBlt(0, 0, 10, 10, pSrc.get(), 0, 0, 0, 0, ui::RopMode::kSrcCopy));

    //RA-37 AlphaBlend 半透明混合
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->AlphaBlend(0, 0, 20, 20, pSrc.get(), 0, 0, 20, 20, 128));
    CHECK(ReadPixel(pDst.get(), 10, 10, nColor));
    CHECK((nColor & 0x00FF0000u) != 0);
    CHECK((nColor & 0x0000FF00u) != 0);
    CHECK(nColor != kRedColor);

    //RA-38 AlphaBlend alpha=255 等价于不透明复制
    pDst->Clear(ui::UiColor(kWhiteColor));
    CHECK(pDst->AlphaBlend(0, 0, 20, 20, pSrc.get(), 0, 0, 20, 20, 255));
    CHECK(ReadPixel(pDst.get(), 10, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
}

// ---------------------------------------------------------------------------
// 6. 图形绘制（RA-39 ~ RA-56）
// ---------------------------------------------------------------------------
void TestShapes(ui::IRenderFactory* pRenderFactory)
{
    Section("TestShapes");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 100, 100);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;

    //RA-39 DrawLine
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawLine(ui::UiPointF(2.0f, 50.0f), ui::UiPointF(98.0f, 50.0f), ui::UiColor(kRedColor), 2.0f);
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 50, 20, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-40 DrawLine 传 IPen
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kBlueColor), 3.0f));
        CHECK(pPen != nullptr);
        if (pPen != nullptr) {
            pRender->DrawLine(ui::UiPointF(10.0f, 10.0f), ui::UiPointF(90.0f, 90.0f), pPen.get());
            CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
            CHECK(ColorNear(nColor, kBlueColor));
        }
    }

    //RA-41 DrawRect
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), ui::UiColor(kRedColor), 2.0f);
    CHECK(ReadPixel(pRender.get(), 50, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-42 DrawRect(bLineInRect=true)：线条绘制在矩形内部
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), ui::UiColor(kBlueColor), 4.0f, true);
    CHECK(ReadPixel(pRender.get(), 50, 12, nColor));
    CHECK(ColorNear(nColor, kBlueColor));
    CHECK(ReadPixel(pRender.get(), 50, 8, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-43 DrawRect 传 IPen
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kGreenColor), 2.0f));
        CHECK(pPen != nullptr);
        if (pPen != nullptr) {
            pRender->DrawRect(ui::UiRectF(20.0f, 20.0f, 80.0f, 80.0f), pPen.get());
            CHECK(ReadPixel(pRender.get(), 50, 20, nColor));
            CHECK(ColorNear(nColor, kGreenColor));
        }
    }

    //RA-44 FillRect
    const uint32_t kBrownColor = 0xFF8B4513u;
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 50.0f, 50.0f), ui::UiColor(kBrownColor));
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK(ColorNear(nColor, kBrownColor));

    //RA-45 FillRect 渐变（左->右）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 50.0f), ui::UiColor(kRedColor), ui::UiColor(kBlueColor), 1);
    uint32_t nLeft = 0;
    uint32_t nRight = 0;
    CHECK(ReadPixel(pRender.get(), 15, 30, nLeft));
    CHECK(ReadPixel(pRender.get(), 85, 30, nRight));
    CHECK((nLeft & 0x000000FFu) < (nRight & 0x000000FFu));//右侧蓝色分量更大
    CHECK((nLeft & 0x00FF0000u) > (nRight & 0x00FF0000u));//左侧红色分量更大

    //RA-46 FillRect 带整体透明度
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRect(ui::UiRectF(10.0f, 10.0f, 50.0f, 50.0f), ui::UiColor(kRedColor), 128);
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK(nColor != kRedColor);
    CHECK((nColor & 0x0000FF00u) != 0);

    //RA-47 DrawRoundRect
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawRoundRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 20.0f, 20.0f, ui::UiColor(kRedColor), 2.0f);
    CHECK(ReadPixel(pRender.get(), 50, 10, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 11, 11, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-48 DrawRoundRect 传 IPen
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kBlueColor), 2.0f));
        if (pPen != nullptr) {
            pRender->DrawRoundRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 20.0f, 20.0f, pPen.get());
            CHECK(ReadPixel(pRender.get(), 50, 10, nColor));
            CHECK(ColorNear(nColor, kBlueColor));
        }
    }

    //RA-49 FillRoundRect
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRoundRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 20.0f, 20.0f, ui::UiColor(kGreenColor));
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kGreenColor));
    CHECK(ReadPixel(pRender.get(), 11, 11, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-50 FillRoundRect 渐变
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->FillRoundRect(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 20.0f, 20.0f,
                           ui::UiColor(kRedColor), ui::UiColor(kBlueColor), 1);
    CHECK(ReadPixel(pRender.get(), 15, 50, nLeft));
    CHECK(ReadPixel(pRender.get(), 85, 50, nRight));
    CHECK(nLeft != nRight);

    //RA-51 DrawArc / FillArc
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kRedColor), 2.0f));
        if (pPen != nullptr) {
            pRender->DrawArc(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 0.0f, 90.0f, true, pPen.get());
            CHECK(ReadPixel(pRender.get(), 35, 35, nColor));
            CHECK(ColorNear(nColor, kWhiteColor));//只画扇形边框，区域内部不填充
        }
    }
    pRender->FillArc(ui::UiRectF(10.0f, 10.0f, 90.0f, 90.0f), 0.0f, 90.0f, true, ui::UiColor(kBlueColor));
    CHECK(ReadPixel(pRender.get(), 60, 60, nColor));
    CHECK(ColorNear(nColor, kBlueColor));

    //RA-52 DrawCircle / FillCircle
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawCircle(ui::UiPointF(50.0f, 50.0f), 30.0f, ui::UiColor(kRedColor), 2.0f);
    CHECK(ReadPixel(pRender.get(), 50, 20, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));
    pRender->FillCircle(ui::UiPointF(50.0f, 50.0f), 10.0f, ui::UiColor(kGreenColor));
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kGreenColor));

    //RA-53 DrawEllipse / FillEllipse
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawEllipse(ui::UiRectF(10.0f, 30.0f, 90.0f, 70.0f), ui::UiColor(kBlueColor), 2.0f);
    CHECK(ReadPixel(pRender.get(), 50, 30, nColor));
    CHECK(ColorNear(nColor, kBlueColor));
    pRender->FillEllipse(ui::UiRectF(20.0f, 40.0f, 40.0f, 60.0f), ui::UiColor(kRedColor));
    CHECK(ReadPixel(pRender.get(), 30, 50, nColor));
    CHECK(ColorNear(nColor, kRedColor));

    //RA-54 DrawPolygon / FillPolygon
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        const ui::UiPointF points[3] = { ui::UiPointF(50.0f, 10.0f),
                                         ui::UiPointF(90.0f, 80.0f),
                                         ui::UiPointF(10.0f, 80.0f) };
        pRender->DrawPolygon(points, 3, ui::UiColor(kRedColor), 2.0f);
        //顶点附近的线条受抗锯齿影响，扫描一小片区域确认识别到边框颜色
        bool bFoundEdge = false;
        for (int32_t y = 8; y <= 14 && !bFoundEdge; ++y) {
            for (int32_t x = 45; x <= 55; ++x) {
                if (ReadPixel(pRender.get(), x, y, nColor) && (nColor != kWhiteColor)) {
                    bFoundEdge = true;
                    break;
                }
            }
        }
        CHECK(bFoundEdge);
        pRender->FillPolygon(points, 3, ui::UiColor(kGreenColor));
        CHECK(ReadPixel(pRender.get(), 50, 60, nColor));
        CHECK(ColorNear(nColor, kGreenColor));
    }

    //RA-55 DrawPath / FillPath（纯色画刷）
    pRender->Clear(ui::UiColor(kWhiteColor));
    std::unique_ptr<ui::IPath> pPath(pRenderFactory->CreatePath());
    CHECK(pPath != nullptr);
    if (pPath != nullptr) {
        pPath->AddRect(ui::UiRect(20, 20, 80, 80));
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kRedColor), 2.0f));
        std::unique_ptr<ui::IBrush> pBrush(pRenderFactory->CreateBrush(ui::UiColor(kBlueColor)));
        CHECK(pPen != nullptr);
        CHECK(pBrush != nullptr);
        if (pPen != nullptr) {
            pRender->DrawPath(pPath.get(), pPen.get());
            CHECK(ReadPixel(pRender.get(), 50, 20, nColor));
            CHECK(ColorNear(nColor, kRedColor));
        }
        if (pBrush != nullptr) {
            pRender->FillPath(pPath.get(), pBrush.get());
            CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
            CHECK(ColorNear(nColor, kBlueColor));
        }

        //RA-56 FillPath 渐变重载
        pRender->Clear(ui::UiColor(kWhiteColor));
        pRender->FillPath(pPath.get(), ui::UiRectF(20.0f, 20.0f, 80.0f, 80.0f),
                          ui::UiColor(kRedColor), ui::UiColor(kBlueColor), 1);
        CHECK(ReadPixel(pRender.get(), 25, 50, nLeft));
        CHECK(ReadPixel(pRender.get(), 75, 50, nRight));
        CHECK(nLeft != nRight);
    }

    //RA-100 用圆弧与直线构成圆角矩形路径，填充内部区域
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPath> pRoundPath(pRenderFactory->CreatePath());
        CHECK(pRoundPath != nullptr);
        if (pRoundPath != nullptr) {
            const int32_t nRound = 20;
            pRoundPath->AddArc(ui::UiRect(20, 20, 20 + nRound, 20 + nRound), 180.0f, 90.0f);
            pRoundPath->AddLine((float)(20 + nRound / 2), 20.0f, (float)(80 - nRound / 2), 20.0f);
            pRoundPath->AddArc(ui::UiRect(80 - nRound, 20, 80, 20 + nRound), 270.0f, 90.0f);
            pRoundPath->AddLine(80.0f, (float)(20 + nRound / 2), 80.0f, (float)(80 - nRound / 2));
            pRoundPath->AddArc(ui::UiRect(80 - nRound, 80 - nRound, 80, 80), 0.0f, 90.0f);
            pRoundPath->AddLine((float)(80 - nRound / 2), 80.0f, (float)(20 + nRound / 2), 80.0f);
            pRoundPath->AddArc(ui::UiRect(20, 80 - nRound, 20 + nRound, 80), 90.0f, 90.0f);
            pRoundPath->AddLine(20.0f, (float)(80 - nRound / 2), 20.0f, (float)(20 + nRound / 2));
            pRoundPath->Close();

            std::unique_ptr<ui::IBrush> pRoundBrush(pRenderFactory->CreateBrush(ui::UiColor(kRedColor)));
            CHECK(pRoundBrush != nullptr);
            if (pRoundBrush != nullptr) {
                pRender->FillPath(pRoundPath.get(), pRoundBrush.get());
                //内部区域被填充
                CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
                CHECK(ColorNear(nColor, kRedColor));
                //四个角位于圆角外侧，未被填充
                CHECK(ReadPixel(pRender.get(), 22, 22, nColor));
                CHECK(ColorNear(nColor, kWhiteColor));
            }
        }
    }

    //RA-101 圆角半径为奇数时，圆弧端点与连线端点存在半个像素的差异，填充仍然有效
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IPath> pRoundPath(pRenderFactory->CreatePath());
        CHECK(pRoundPath != nullptr);
        if (pRoundPath != nullptr) {
            const int32_t nRound = 15;              //奇数半径：半圆角为 7.5
            const float fHalfRound = nRound / 2.0f;
            pRoundPath->AddArc(ui::UiRect(20, 20, 20 + nRound, 20 + nRound), 180.0f, 90.0f);
            pRoundPath->AddLine(20 + fHalfRound, 20.0f, 80 - fHalfRound, 20.0f);
            pRoundPath->AddArc(ui::UiRect(80 - nRound, 20, 80, 20 + nRound), 270.0f, 90.0f);
            pRoundPath->AddLine(80.0f, 20 + fHalfRound, 80.0f, 80 - fHalfRound);
            pRoundPath->AddArc(ui::UiRect(80 - nRound, 80 - nRound, 80, 80), 0.0f, 90.0f);
            pRoundPath->AddLine(80 - fHalfRound, 80.0f, 20 + fHalfRound, 80.0f);
            pRoundPath->AddArc(ui::UiRect(20, 80 - nRound, 20 + nRound, 80), 90.0f, 90.0f);
            pRoundPath->AddLine(20.0f, 80 - fHalfRound, 20.0f, 20 + fHalfRound);
            pRoundPath->Close();

            std::unique_ptr<ui::IBrush> pRoundBrush(pRenderFactory->CreateBrush(ui::UiColor(kRedColor)));
            CHECK(pRoundBrush != nullptr);
            if (pRoundBrush != nullptr) {
                pRender->FillPath(pRoundPath.get(), pRoundBrush.get());
                //内部区域被填充，且四个角位于圆角外侧，未被填充
                CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
                CHECK(ColorNear(nColor, kRedColor));
                CHECK(ReadPixel(pRender.get(), 90, 50, nColor));
                CHECK(ColorNear(nColor, kWhiteColor));
            }
        }
    }

    //RA-57 DrawBoxShadow：阴影绘制在矩形外部
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->DrawBoxShadow(ui::UiRect(30, 30, 70, 70), ui::UiSize(0, 0), ui::UiPoint(0, 0),
                           6, 4, ui::UiColor(kBlackColor), 255);
    CHECK(ReadPixel(pRender.get(), 50, 50, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));//矩形内部不绘制阴影
    bool bHasShadow = false;
    for (int32_t y = 25; y < 30; ++y) {
        for (int32_t x = 40; x < 60; ++x) {
            if (ReadPixel(pRender.get(), x, y, nColor) && (nColor != kWhiteColor)) {
                bHasShadow = true;
            }
        }
    }
    CHECK(bHasShadow);
}

// ---------------------------------------------------------------------------
// 7. Alpha 处理（RA-58 ~ RA-61）
// ---------------------------------------------------------------------------
void TestAlpha(ui::IRenderFactory* pRenderFactory)
{
    Section("TestAlpha");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 40, 40);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;

    //RA-58 ClearAlpha 把指定区域的Alpha设为0，同时清除图像数据（RGB 也置为 0）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->ClearAlpha(ui::UiRect(0, 0, 20, 20), 0);
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK((nColor >> 24) == 0);
    CHECK(ColorNear(nColor, 0x00000000));
    CHECK(ReadPixel(pRender.get(), 30, 30, nColor));
    CHECK((nColor >> 24) == 255);

    //RA-59 RestoreAlpha 恢复为指定值
    pRender->RestoreAlpha(ui::UiRect(0, 0, 20, 20), ui::UiPadding(), 255);
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK((nColor >> 24) == 255);

    //RA-60 ClearAlpha 设为半透明，再用 RestoreAlpha 恢复
    pRender->ClearAlpha(ui::UiRect(0, 0, 40, 40), 128);
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK((nColor >> 24) == 128);
    pRender->RestoreAlpha(ui::UiRect(0, 0, 40, 40));
    CHECK(ReadPixel(pRender.get(), 10, 10, nColor));
    CHECK((nColor >> 24) == 255);

    //RA-61 阴影边距参数参与区域扩展（不应影响矩形内部的结果）
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->ClearAlpha(ui::UiRect(10, 10, 30, 30), 0);
    pRender->RestoreAlpha(ui::UiRect(10, 10, 30, 30), ui::UiPadding(2, 2, 2, 2), 255);
    CHECK(ReadPixel(pRender.get(), 20, 20, nColor));
    CHECK((nColor >> 24) == 255);
}

// ---------------------------------------------------------------------------
// 8. 像素操作（RA-62 ~ RA-69）
// ---------------------------------------------------------------------------
void TestPixels(ui::IRenderFactory* pRenderFactory)
{
    Section("TestPixels");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 40, 40);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    uint32_t nColor = 0;

    //RA-62 ClearRect 只清除指定区域
    pRender->Clear(ui::UiColor(kWhiteColor));
    pRender->ClearRect(ui::UiRect(10, 10, 20, 20), ui::UiColor(kRedColor));
    CHECK(ReadPixel(pRender.get(), 15, 15, nColor));
    CHECK(ColorNear(nColor, kRedColor));
    CHECK(ReadPixel(pRender.get(), 25, 25, nColor));
    CHECK(ColorNear(nColor, kWhiteColor));

    //RA-63 ReadPixels 缓冲区长度满足要求时读取成功
    uint32_t pixels[4] = { 0, 0, 0, 0 };
    CHECK(pRender->ReadPixels(ui::UiRect(0, 0, 2, 2), pixels, sizeof(pixels)));
    CHECK(ColorNear(pixels[0], kWhiteColor));

    //RA-64 ReadPixels 超出画布范围时返回失败
    std::vector<uint32_t> bigPixels(20 * 20, 0);
    CHECK(!pRender->ReadPixels(ui::UiRect(30, 30, 50, 50), bigPixels.data(), bigPixels.size() * sizeof(uint32_t)));
    CHECK(!pRender->ReadPixels(ui::UiRect(-5, 0, 5, 5), bigPixels.data(), bigPixels.size() * sizeof(uint32_t)));

    //RA-65 WritePixels 基本写入
    {
        const int32_t nWidth = 4;
        const int32_t nHeight = 4;
        std::vector<uint32_t> srcPixels((size_t)nWidth * nHeight, kBlueColor);
        pRender->Clear(ui::UiColor(kWhiteColor));
        CHECK(pRender->WritePixels(srcPixels.data(), srcPixels.size() * sizeof(uint32_t), ui::UiRect(4, 4, 8, 8)));
        CHECK(ReadPixel(pRender.get(), 5, 5, nColor));
        CHECK(ColorNear(nColor, kBlueColor));
        CHECK(ReadPixel(pRender.get(), 20, 20, nColor));
        CHECK(ColorNear(nColor, kWhiteColor));

        //RA-66 目标区域超出画布时返回失败
        CHECK(!pRender->WritePixels(srcPixels.data(), srcPixels.size() * sizeof(uint32_t), ui::UiRect(38, 38, 42, 42)));

        //RA-68 WritePixels 带 rcPaint：只写入脏区域
        const int32_t nWidth2 = 8;
        const int32_t nHeight2 = 8;
        std::vector<uint32_t> srcPixels2((size_t)nWidth2 * nHeight2, kGreenColor);
        pRender->Clear(ui::UiColor(kWhiteColor));
        CHECK(pRender->WritePixels(srcPixels2.data(), srcPixels2.size() * sizeof(uint32_t),
                                    ui::UiRect(0, 0, 8, 8), ui::UiRect(0, 0, 4, 4)));
        CHECK(ReadPixel(pRender.get(), 2, 2, nColor));
        CHECK(ColorNear(nColor, kGreenColor));
        CHECK(ReadPixel(pRender.get(), 6, 6, nColor));
        CHECK(ColorNear(nColor, kWhiteColor));
    }

    //RA-69 IsEmpty：未初始化的离屏对象为空，Resize 后不为空
    {
        std::unique_ptr<ui::IRender> pEmpty = CreateRender(pRenderFactory, 0, 0);
        CHECK(pEmpty != nullptr);
        if (pEmpty != nullptr) {
            CHECK(pEmpty->IsEmpty());
            CHECK(pEmpty->Resize(10, 10));
            CHECK(!pEmpty->IsEmpty());
            CHECK(pEmpty->GetWidth() == 10);
            CHECK(pEmpty->GetHeight() == 10);
        }
    }
}

// ---------------------------------------------------------------------------
// 9. 快照与克隆（RA-70 ~ RA-73）
// ---------------------------------------------------------------------------
void TestSnapshotAndClone(ui::IRenderFactory* pRenderFactory)
{
    Section("TestSnapshotAndClone");
    std::unique_ptr<ui::IRender> pRender = CreateRender(pRenderFactory, 40, 40);
    CHECK(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //RA-70 MakeImageSnapshot(rc)：区域完全在画布外时返回nullptr；超出画布的部分被裁剪
    pRender->Clear(ui::UiColor(kWhiteColor));
    {
        std::unique_ptr<ui::IBitmap> pOutside(pRender->MakeImageSnapshot(ui::UiRect(50, 50, 60, 60)));
        CHECK(pOutside == nullptr);
        std::unique_ptr<ui::IBitmap> pEmpty(pRender->MakeImageSnapshot(ui::UiRect()));
        CHECK(pEmpty == nullptr);

        std::unique_ptr<ui::IBitmap> pClip(pRender->MakeImageSnapshot(ui::UiRect(30, 30, 50, 50)));
        CHECK(pClip != nullptr);
        if (pClip != nullptr) {
            CHECK(pClip->GetWidth() == 10);
            CHECK(pClip->GetHeight() == 10);
        }
    }

    //RA-71 MakeImageSnapshot(rc)：有效区域返回对应尺寸的位图，且内容正确
    pRender->ClearRect(ui::UiRect(10, 10, 20, 20), ui::UiColor(kRedColor));
    {
        std::unique_ptr<ui::IBitmap> pBitmap(pRender->MakeImageSnapshot(ui::UiRect(10, 10, 20, 20)));
        CHECK(pBitmap != nullptr);
        if (pBitmap != nullptr) {
            CHECK(pBitmap->GetWidth() == 10);
            CHECK(pBitmap->GetHeight() == 10);
            const void* pBits = pBitmap->LockPixelBits();
            CHECK(pBits != nullptr);
            if (pBits != nullptr) {
                const uint32_t* pLine = (const uint32_t*)pBits;
                CHECK(ColorNear(pLine[0], kRedColor));
                CHECK(ColorNear(pLine[5 * 10 + 5], kRedColor));
            }
            pBitmap->UnLockPixelBits();
        }
    }

    //RA-72 Clone：克隆对象与原对象相互独立
    pRender->Clear(ui::UiColor(kGreenColor));
    {
        std::unique_ptr<ui::IRender> pClone = pRender->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->GetWidth() == pRender->GetWidth());
            CHECK(pClone->GetHeight() == pRender->GetHeight());
            uint32_t nColor = 0;
            CHECK(ReadPixel(pClone.get(), 5, 5, nColor));
            CHECK(ColorNear(nColor, kGreenColor));

            //修改克隆对象，原对象不受影响
            pClone->Clear(ui::UiColor(kBlueColor));
            CHECK(ReadPixel(pRender.get(), 5, 5, nColor));
            CHECK(ColorNear(nColor, kGreenColor));
            CHECK(ReadPixel(pClone.get(), 5, 5, nColor));
            CHECK(ColorNear(nColor, kBlueColor));
        }
    }

    //RA-73 Clone 后对克隆对象Resize，原对象尺寸不变
    {
        std::unique_ptr<ui::IRender> pClone = pRender->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->Resize(80, 20));
            CHECK(pClone->GetWidth() == 80);
            CHECK(pRender->GetWidth() == 40);
        }
    }
}

// ---------------------------------------------------------------------------
// 10. IPen / IBrush / IPath / IBitmap（RA-74 ~ RA-95）
// ---------------------------------------------------------------------------
void TestPen(ui::IRenderFactory* pRenderFactory)
{
    Section("TestPen");
    std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kRedColor), 2.5f));
    CHECK(pPen != nullptr);
    if (pPen == nullptr) {
        return;
    }

    //RA-74 宽度与颜色
    CHECK(pPen->GetWidth() == 2.5f);
    CHECK(pPen->GetColor().GetARGB() == kRedColor);
    pPen->SetWidth(4.0f);
    CHECK(pPen->GetWidth() == 4.0f);
    pPen->SetColor(ui::UiColor(kBlueColor));
    CHECK(pPen->GetColor().GetARGB() == kBlueColor);

    //RA-75 笔帽样式
    pPen->SetStartCap(ui::IPen::kRound_Cap);
    CHECK(pPen->GetStartCap() == ui::IPen::kRound_Cap);
    pPen->SetEndCap(ui::IPen::kSquare_Cap);
    CHECK(pPen->GetEndCap() == ui::IPen::kSquare_Cap);
    pPen->SetDashCap(ui::IPen::kRound_Cap);
    CHECK(pPen->GetDashCap() == ui::IPen::kRound_Cap);

    //RA-76 联接样式
    pPen->SetLineJoin(ui::IPen::kRound_Join);
    CHECK(pPen->GetLineJoin() == ui::IPen::kRound_Join);
    pPen->SetLineJoin(ui::IPen::kBevel_Join);
    CHECK(pPen->GetLineJoin() == ui::IPen::kBevel_Join);

    //RA-77 虚线样式
    pPen->SetDashStyle(ui::IPen::kDashStyleDashDot);
    CHECK(pPen->GetDashStyle() == ui::IPen::kDashStyleDashDot);

    //RA-78 自定义虚线样式
    const float intervals[4] = { 4.0f, 2.0f, 1.0f, 2.0f };
    pPen->SetDashPattern(intervals, 4);
    CHECK(pPen->HasDashPattern());
    //传0表示清除自定义样式
    pPen->SetDashPattern(nullptr, 0);
    CHECK(!pPen->HasDashPattern());

    //RA-79 抗锯齿与斜接限制
    pPen->SetAntiAlias(false);
    CHECK(!pPen->IsAntiAlias());
    pPen->SetAntiAlias(true);
    CHECK(pPen->IsAntiAlias());
    pPen->SetMiterLimit(8.0f);
    CHECK(pPen->GetMiterLimit() == 8.0f);

    //RA-80 Clone 复制全部属性
    pPen->SetWidth(3.0f);
    pPen->SetColor(ui::UiColor(kGreenColor));
    pPen->SetDashStyle(ui::IPen::kDashStyleDot);
    {
        std::unique_ptr<ui::IPen> pClone = pPen->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->GetWidth() == 3.0f);
            CHECK(pClone->GetColor().GetARGB() == kGreenColor);
            CHECK(pClone->GetDashStyle() == ui::IPen::kDashStyleDot);
            CHECK(pClone->GetStartCap() == pPen->GetStartCap());
            CHECK(pClone->GetLineJoin() == pPen->GetLineJoin());
            CHECK(pClone->GetMiterLimit() == pPen->GetMiterLimit());

            //修改克隆对象不影响原对象
            pClone->SetWidth(9.0f);
            CHECK(pPen->GetWidth() == 3.0f);
        }
    }
}

void TestBrush(ui::IRenderFactory* pRenderFactory)
{
    Section("TestBrush");
    std::unique_ptr<ui::IBrush> pBrush(pRenderFactory->CreateBrush(ui::UiColor(kRedColor)));
    CHECK(pBrush != nullptr);
    if (pBrush == nullptr) {
        return;
    }

    //RA-81 纯色画刷
    CHECK(pBrush->GetBrushType() == ui::IBrush::BrushType::kSolid);
    CHECK(pBrush->GetColor().GetARGB() == kRedColor);

    //RA-82 线性渐变画刷
    pBrush->SetLinearGradient(ui::UiColor(kBlueColor), ui::UiPointF(0.0f, 0.0f), ui::UiPointF(10.0f, 0.0f));
    CHECK(pBrush->GetBrushType() == ui::IBrush::BrushType::kLinearGradient);
    CHECK(pBrush->GetColor().GetARGB() == kRedColor);//起始颜色不变

    //RA-83 径向渐变画刷
    pBrush->SetRadialGradient(ui::UiColor(kGreenColor), ui::UiPointF(5.0f, 5.0f), 10.0f);
    CHECK(pBrush->GetBrushType() == ui::IBrush::BrushType::kRadialGradient);

    //RA-84 SetSolid 恢复为纯色画刷
    pBrush->SetSolid(ui::UiColor(kBlackColor));
    CHECK(pBrush->GetBrushType() == ui::IBrush::BrushType::kSolid);
    CHECK(pBrush->GetColor().GetARGB() == kBlackColor);

    //RA-85 Clone 独立性
    {
        std::unique_ptr<ui::IBrush> pClone = pBrush->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->GetBrushType() == ui::IBrush::BrushType::kSolid);
            CHECK(pClone->GetColor().GetARGB() == kBlackColor);
            pClone->SetSolid(ui::UiColor(kWhiteColor));
            CHECK(pBrush->GetColor().GetARGB() == kBlackColor);
        }
    }
}

void TestPath(ui::IRenderFactory* pRenderFactory)
{
    Section("TestPath");
    std::unique_ptr<ui::IPath> pPath(pRenderFactory->CreatePath());
    CHECK(pPath != nullptr);
    if (pPath == nullptr) {
        return;
    }

    //RA-86 填充类型
    CHECK(pPath->GetFillType() == ui::IPath::FillType::kWinding);
    pPath->SetFillType(ui::IPath::FillType::kEvenOdd);
    CHECK(pPath->GetFillType() == ui::IPath::FillType::kEvenOdd);

    //RA-87 AddLine / AddLines
    pPath->AddLine(0, 0, 10, 10);
    pPath->AddLine(0.0f, 10.0f, 10.0f, 0.0f);
    ui::UiPoint points[3] = { ui::UiPoint(0, 0), ui::UiPoint(10, 0), ui::UiPoint(10, 10) };
    pPath->AddLines(points, 3);
    ui::UiPointF pointsF[3] = { ui::UiPointF(0.0f, 0.0f), ui::UiPointF(10.0f, 0.0f), ui::UiPointF(10.0f, 10.0f) };
    pPath->AddLines(pointsF, 3);

    //RA-88 AddBezier / AddBeziers（AddBeziers 的点数必须满足 count == 1 + 3n）
    pPath->AddBezier(0, 0, 5, 10, 10, 10, 15, 0);
    pPath->AddBezier(0.0f, 0.0f, 5.0f, 10.0f, 10.0f, 10.0f, 15.0f, 0.0f);
    ui::UiPoint bezierPoints[7] = { ui::UiPoint(0, 0), ui::UiPoint(5, 10), ui::UiPoint(10, 10),
                                    ui::UiPoint(15, 0), ui::UiPoint(20, -10), ui::UiPoint(25, 0),
                                    ui::UiPoint(30, 10) };
    pPath->AddBeziers(bezierPoints, 7);
    ui::UiPointF bezierPointsF[7] = { ui::UiPointF(0.0f, 0.0f), ui::UiPointF(5.0f, 10.0f), ui::UiPointF(10.0f, 10.0f),
                                      ui::UiPointF(15.0f, 0.0f), ui::UiPointF(20.0f, -10.0f), ui::UiPointF(25.0f, 0.0f),
                                      ui::UiPointF(30.0f, 10.0f) };
    pPath->AddBeziers(bezierPointsF, 7);

    //RA-89 AddRect / AddEllipse / AddArc / AddPolygon
    pPath->AddRect(ui::UiRect(20, 20, 40, 40));
    pPath->AddRect(ui::UiRectF(20.0f, 20.0f, 40.0f, 40.0f));
    pPath->AddEllipse(ui::UiRect(50, 50, 70, 70));
    pPath->AddEllipse(ui::UiRectF(50.0f, 50.0f, 70.0f, 70.0f));
    pPath->AddArc(ui::UiRect(80, 80, 100, 100), 0.0f, 90.0f);
    pPath->AddArc(ui::UiRectF(80.0f, 80.0f, 100.0f, 100.0f), 0.0f, 90.0f);
    ui::UiPoint polygon[4] = { ui::UiPoint(0, 0), ui::UiPoint(10, 0), ui::UiPoint(10, 10), ui::UiPoint(0, 10) };
    pPath->AddPolygon(polygon, 4);

    //RA-90 GetBounds：非空路径的边界有效
    {
        ui::UiRect rcBounds = pPath->GetBounds(nullptr);
        CHECK(rcBounds.Width() > 0);
        CHECK(rcBounds.Height() > 0);
    }

    //RA-91 Transform：路径被矩阵变换后边界随之变化
    {
        ui::UiRect rcBefore = pPath->GetBounds(nullptr);
        std::unique_ptr<ui::IMatrix> pMatrix(pRenderFactory->CreateMatrix());
        CHECK(pMatrix != nullptr);
        if (pMatrix != nullptr) {
            pMatrix->Translate(100.0f, 0.0f);
            pPath->Transform(pMatrix.get());
            ui::UiRect rcAfter = pPath->GetBounds(nullptr);
            CHECK(rcAfter.left > rcBefore.left);
            CHECK(std::abs(rcAfter.Width() - rcBefore.Width()) <= 1);
        }
    }

    //RA-92 Close：闭合路径后仍可获取有效边界
    pPath->Close();
    CHECK(pPath->GetBounds(nullptr).Width() > 0);

    //RA-93 Clone：克隆路径独立
    {
        std::unique_ptr<ui::IPath> pClone = pPath->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->GetFillType() == ui::IPath::FillType::kEvenOdd);
            CHECK(pClone->GetBounds(nullptr).Width() == pPath->GetBounds(nullptr).Width());
            pClone->Reset();
            CHECK(pClone->GetBounds(nullptr).Width() == 0);
            CHECK(pPath->GetBounds(nullptr).Width() > 0);
        }
    }

    //RA-94 Reset：清空路径
    pPath->Reset();
    CHECK(pPath->GetBounds(nullptr).Width() == 0);
    CHECK(pPath->GetBounds(nullptr).Height() == 0);

    //RA-95 GetBounds 传入画笔时包含线宽
    {
        pPath->AddRect(ui::UiRect(10, 10, 20, 20));
        std::unique_ptr<ui::IPen> pPen(pRenderFactory->CreatePen(ui::UiColor(kRedColor), 6.0f));
        CHECK(pPen != nullptr);
        if (pPen != nullptr) {
            const ui::UiRect rcNoPen = pPath->GetBounds(nullptr);
            const ui::UiRect rcWithPen = pPath->GetBounds(pPen.get());
            CHECK(rcWithPen.left <= rcNoPen.left);
            CHECK(rcWithPen.top <= rcNoPen.top);
        }
    }
}

void TestBitmap(ui::IRenderFactory* pRenderFactory)
{
    Section("TestBitmap");
    //RA-96 从像素数据初始化
    std::unique_ptr<ui::IBitmap> pBitmap(pRenderFactory->CreateBitmap());
    CHECK(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    const uint32_t nWidth = 4;
    const uint32_t nHeight = 2;
    std::vector<uint32_t> pixels(nWidth * nHeight, kRedColor);
    CHECK(pBitmap->Init(nWidth, nHeight, pixels.data(), 1.0f, ui::AlphaType::kOpaque));
    CHECK(pBitmap->IsValid());
    CHECK(pBitmap->GetWidth() == nWidth);
    CHECK(pBitmap->GetHeight() == nHeight);
    CHECK(pBitmap->GetRowBytes() == (uint32_t)(nWidth * sizeof(uint32_t)));
    CHECK(pBitmap->GetAlphaType() == ui::AlphaType::kOpaque);
    CHECK(pBitmap->GetImageSizeScale() == 1.0f);

    //RA-97 锁定像素数据后可读写
    {
        void* pBits = pBitmap->LockPixelBits();
        CHECK(pBits != nullptr);
        if (pBits != nullptr) {
            uint32_t* pLine = (uint32_t*)pBits;
            CHECK(ColorNear(pLine[0], kRedColor));
            pLine[0] = kBlueColor;
        }
        pBitmap->UnLockPixelBits();
        const void* pConstBits = pBitmap->LockPixelBits();
        CHECK(pConstBits != nullptr);
        if (pConstBits != nullptr) {
            CHECK(ColorNear(((const uint32_t*)pConstBits)[0], kBlueColor));
        }
        pBitmap->UnLockPixelBits();
    }

    //RA-98 Clone：复制位图数据，且相互独立
    {
        std::unique_ptr<ui::IBitmap> pClone = pBitmap->Clone();
        CHECK(pClone != nullptr);
        if (pClone != nullptr) {
            CHECK(pClone->GetWidth() == nWidth);
            CHECK(pClone->GetHeight() == nHeight);
            const uint32_t* pCloneBits = (const uint32_t*)pClone->LockPixelBits();
            CHECK(pCloneBits != nullptr);
            if (pCloneBits != nullptr) {
                CHECK(ColorNear(pCloneBits[0], kBlueColor));
            }
            pClone->UnLockPixelBits();
        }
    }

    //RA-99 Init 传入nullptr：创建尺寸有效的空位图
    {
        std::unique_ptr<ui::IBitmap> pEmpty(pRenderFactory->CreateBitmap());
        CHECK(pEmpty != nullptr);
        if (pEmpty != nullptr) {
            CHECK(pEmpty->Init(6, 8, nullptr));
            CHECK(pEmpty->IsValid());
            CHECK(pEmpty->GetWidth() == 6);
            CHECK(pEmpty->GetHeight() == 8);
        }
    }
}

} //namespace

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
            TestWindowOrg(pRenderFactory);
            TestMatrix(pRenderFactory);
            TestClip(pRenderFactory);
            TestLayer(pRenderFactory);
            TestBlt(pRenderFactory);
            TestShapes(pRenderFactory);
            TestAlpha(pRenderFactory);
            TestPixels(pRenderFactory);
            TestSnapshotAndClone(pRenderFactory);
            TestPen(pRenderFactory);
            TestBrush(pRenderFactory);
            TestPath(pRenderFactory);
            TestBitmap(pRenderFactory);
        }
    }

    std::cout << "render_api: " << g_checkCount << " checks, "
              << g_failedCount << " failed" << std::endl;
    if (g_failedCount != 0) {
        std::cerr << "render_api: FAILED" << std::endl;
        return 1;
    }
    std::cout << "render_api: PASSED" << std::endl;
    return 0;
}
