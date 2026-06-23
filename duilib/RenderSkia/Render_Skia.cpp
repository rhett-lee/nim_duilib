#include "Render_Skia.h"
#include "duilib/RenderSkia/VerticalDrawText.h"
#include "duilib/RenderSkia/HorizontalDrawText.h"
#include "duilib/RenderSkia/DrawRichText.h"
#include "duilib/RenderSkia/DrawSkiaText.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"
#include "duilib/RenderSkia/DrawSkiaImage.h"
#include "duilib/Render/BitmapAlpha.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/SharePtr.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRegion.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradient.h"
#include "include/effects/SkImageFilters.h"

#include "SkiaHeaderEnd.h"

#include <unordered_set>
#include <unordered_map>
#include <cstdint>

namespace ui {

Render_Skia::Render_Skia():
    m_saveCount(0)
{
    m_pSkPointOrg = std::make_unique<SkPoint>();
    m_pSkPointOrg->iset(0, 0);
    m_pSkPaint = std::make_unique<SkPaint>();
    m_pSkPaint->setAntiAlias(true);
    m_pSkPaint->setDither(true);
}

Render_Skia::~Render_Skia()
{
}

RenderType Render_Skia::GetRenderType() const
{
    return RenderType::kRenderType_Skia;
}

SkPoint& Render_Skia::GetPointOrg() const
{
    return *m_pSkPointOrg;
}

IRenderDpiPtr Render_Skia::GetRenderDpi() const
{
    return m_spRenderDpi;
}

void* Render_Skia::GetPixelBits() const
{
    void* pPixelBits = nullptr;
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        SkPixmap pixmap;
        if (skCanvas->peekPixels(&pixmap)) {
            pPixelBits = pixmap.writable_addr();
        }
    }
    if (pPixelBits == nullptr) {
        ASSERT(GetRenderBackendType() != RenderBackendType::kRaster_BackendType);
    }
    return pPixelBits;
}

void Render_Skia::Clear(const UiColor& uiColor)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        const int32_t nWidth = GetWidth();
        const int32_t nHeight = GetHeight();
        if (nWidth > 0 && nHeight > 0) {
            // 防止 nWidth * nHeight 溢出 int32_t
            if ((size_t)nWidth <= SIZE_MAX / (size_t)nHeight) {
                const size_t totalPixels = (size_t)nWidth * (size_t)nHeight;
                if (totalPixels <= (SIZE_MAX / sizeof(uint32_t))) {
                    const size_t totalBytes = totalPixels * sizeof(uint32_t);
                    uint32_t nARGB = uiColor.GetARGB();
                    if (nARGB == 0) {
                        // nARGB == 0 时，使用 memset 高效清零
                        ::memset(pPixelBits, 0, totalBytes);
                    }
                    else {
                        // 非零填充：使用 size_t 索引避免溢出
                        uint32_t* pBits = (uint32_t*)pPixelBits;
                        uint32_t* pEnd = pBits + totalPixels;
                        while (pBits < pEnd) {
                            *pBits++ = nARGB;
                        }
                    }
                }
            }
        }
    }
}

void Render_Skia::ClearRect(const UiRect& rcDirty, const UiColor& uiColor)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        const int32_t nWidth = GetWidth();
        const int32_t nHeight = GetHeight();
        if (nWidth > 0 && nHeight > 0) {
            const int32_t nLeft = std::max((int32_t)rcDirty.left, 0);
            const int32_t nTop = std::max((int32_t)rcDirty.top, 0);
            const int32_t nRight = std::min((int32_t)rcDirty.right, nWidth);
            const int32_t nBottom = std::min((int32_t)rcDirty.bottom, nHeight);
            if ((nRight > nLeft) && (nBottom > nTop)) {
                // 计算每行字节数（先做 size_t 提升再相乘，避免 nWidth*4 溢出）
                const size_t bytesPerRow = (size_t)nWidth * sizeof(uint32_t);
                const uint32_t nARGB = uiColor.GetARGB();
                // 优化：使用单层循环 + 指针递增，避免 i*nWidth+j 溢出
                uint8_t* pBase = (uint8_t*)pPixelBits;
                for (int32_t i = nTop; i < nBottom; ++i) {
                    uint32_t* pLineStart = (uint32_t*)(pBase + (size_t)i * bytesPerRow) + nLeft;
                    uint32_t* pLineEnd = pLineStart + (nRight - nLeft);
                    uint32_t* pCur = pLineStart;
                    while (pCur < pLineEnd) {
                        *pCur++ = nARGB;
                    }
                }
            }
        }
    }
}

IBitmap* Render_Skia::MakeImageSnapshot()
{
    int32_t nWidth = GetWidth();
    int32_t nHeight = GetHeight();
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return nullptr;
    }
    void* pPixelBits = GetPixelBits();
    if (pPixelBits == nullptr) {
        return nullptr;
    }
    Bitmap_Skia* pBitmap = new Bitmap_Skia;
    if (!pBitmap->Init(nWidth, nHeight, pPixelBits)) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

void Render_Skia::ClearAlpha(const UiRect& rcDirty, uint8_t alpha)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        BitmapAlpha bitmapAlpha((uint8_t*)pPixelBits, GetWidth(), GetHeight(), sizeof(uint32_t));
        bitmapAlpha.ClearAlpha(rcDirty, alpha);
    }
}

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        BitmapAlpha bitmapAlpha((uint8_t*)pPixelBits, GetWidth(), GetHeight(), sizeof(uint32_t));
        bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding, alpha);
    }
}

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        BitmapAlpha bitmapAlpha((uint8_t*)pPixelBits, GetWidth(), GetHeight(), sizeof(uint32_t));
        bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding);
    }
}

UiPoint Render_Skia::OffsetWindowOrg(UiPoint ptOffset)
{
    UiPoint ptOldWindowOrg = { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY) };
    SkPoint ptOff = { SkIntToScalar(ptOffset.x), SkIntToScalar(ptOffset.y) };
    //Skia内部是用的SetViewportOrgEx逻辑(与SetWindowOrgEx相反)，所以这里要符号取反
    m_pSkPointOrg->offset(-ptOff.fX, -ptOff.fY);
    return ptOldWindowOrg;
}

UiPoint Render_Skia::SetWindowOrg(UiPoint ptOffset)
{
    UiPoint ptOldWindowOrg = { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY)};
    m_pSkPointOrg->fX = SkIntToScalar(ptOffset.x);
    m_pSkPointOrg->fY = SkIntToScalar(ptOffset.y);
    return ptOldWindowOrg;
}

UiPoint Render_Skia::GetWindowOrg() const
{
    return UiPoint { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY) };
}

void Render_Skia::SaveClip(int32_t& nState)
{
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        m_saveCount = skCanvas->save();
        nState = m_saveCount;
    }
}

void Render_Skia::RestoreClip(int32_t nState)
{
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    ASSERT(m_saveCount == nState);
    if (m_saveCount != nState) {
        return;
    }
    if (skCanvas != nullptr) {
        skCanvas->restoreToCount(nState);
    }
}

int32_t Render_Skia::SetClip(const UiRect& rc, bool bIntersect)
{
    // 空矩形（rc.right <= rc.left）跳过：避免无意义的 save+clip 调用
    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        return -1;
    }
    SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSk = SkRect::Make(rcSkI);
    rcSk.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        // 同步更新 m_saveCount，使其与 Skia 内部 save 栈保持一致
        m_saveCount = skCanvas->save();
        if (bIntersect) {
            skCanvas->clipRect(rcSk, SkClipOp::kIntersect, true);
        }
        else {
            skCanvas->clipRect(rcSk, SkClipOp::kDifference, true);
        }
        return m_saveCount;
    }
    return -1;
}

int32_t Render_Skia::SetRoundClip(const UiRect& rc, float rx, float ry, bool bIntersect)
{
    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        return -1;
    }
    SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSk = SkRect::Make(rcSkI);

    SkPath skPath = SkPath::RRect(rcSk, rx, ry);
    SkRegion clip;
    clip.setRect(rcSkI);
    SkRegion rgn;
    rgn.setPath(skPath, clip);
    rgn.translate((int)m_pSkPointOrg->fX, (int)m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        // 同步更新 m_saveCount，使其与 Skia 内部 save 栈保持一致
        m_saveCount = skCanvas->save();
        if (bIntersect) {
            skCanvas->clipRegion(rgn, SkClipOp::kIntersect);
        }
        else {
            skCanvas->clipRegion(rgn, SkClipOp::kDifference);
        }
        return m_saveCount;
    }
    return -1;
}

void Render_Skia::ClearClip(int32_t nState)
{
    ASSERT(nState >= 0);
    if (nState < 0) {
        return;
    }
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        // 主动恢复一次 save 状态，使 m_saveCount 与 Skia 内部状态保持一致
        ASSERT(m_saveCount == nState);
        if (m_saveCount != nState) {
            return;
        }
        if (m_saveCount > 0) {
            skCanvas->restoreToCount(m_saveCount);
            m_saveCount = skCanvas->getSaveCount() - 1;
        }
    }
}

void Render_Skia::SetRopMode(SkPaint& skPaint, RopMode rop) const
{
    switch (rop) {
    case RopMode::kSrcCopy:
        skPaint.setBlendMode(SkBlendMode::kSrc);
        break;
    default:
        ASSERT(!"no impl!");
        break;
    }
}

bool Render_Skia::BitBlt(int32_t x, int32_t y, int32_t cx, int32_t cy, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, RopMode rop)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr) {
        return false;
    }
    // 防止负值或零尺寸导致 Skia::drawImageRect 内部 UB
    if (cx <= 0 || cy <= 0 || xSrc < 0 || ySrc < 0) {
        return false;
    }

    Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
    ASSERT(pSkiaRender != nullptr);
    if (pSkiaRender == nullptr) {
        return false;
    }
    SkSurface* skSurface = pSkiaRender->GetSkSurface();
    ASSERT(skSurface != nullptr);
    if (skSurface == nullptr) {
        return false;
    }
    sk_sp<SkImage> skSrcImage = skSurface->makeImageSnapshot();
    ASSERT(skSrcImage != nullptr);
    if (skSrcImage == nullptr) {
        return false;
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    SetRopMode(skPaint, rop);

    SkIRect rcSkDestI = SkIRect::MakeXYWH(x, y, cx, cy);
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkIRect rcSkSrcI = SkIRect::MakeXYWH(xSrc, ySrc, cx, cy);
    SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
        return true;
    }
    return false;
}

bool Render_Skia::StretchBlt(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, RopMode rop)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr) {
        return false;
    }
    // 防止负值或零尺寸导致 Skia::drawImageRect 内部 UB
    if (widthDest <= 0 || heightDest <= 0 || widthSrc <= 0 || heightSrc <= 0 || xSrc < 0 || ySrc < 0) {
        return false;
    }

    Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
    ASSERT(pSkiaRender != nullptr);
    if (pSkiaRender == nullptr) {
        return false;
    }
    SkSurface* skSurface = pSkiaRender->GetSkSurface();
    ASSERT(skSurface != nullptr);
    if (skSurface == nullptr) {
        return false;
    }
    sk_sp<SkImage> skSrcImage = skSurface->makeImageSnapshot();
    ASSERT(skSrcImage != nullptr);
    if (skSrcImage == nullptr) {
        return false;
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    SetRopMode(skPaint, rop);

    SkIRect rcSkDestI = SkIRect::MakeXYWH(xDest, yDest, widthDest, heightDest);
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkIRect rcSkSrcI = SkIRect::MakeXYWH(xSrc, ySrc, widthSrc, heightSrc);
    SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
        return true;
    }
    return false;
}

bool Render_Skia::AlphaBlend(int32_t xDest, int32_t yDest, int32_t widthDest, int32_t heightDest, IRender* pSrcRender, int32_t xSrc, int32_t ySrc, int32_t widthSrc, int32_t heightSrc, uint8_t alpha)
{
    // 1. 参数验证: 与 StretchBlt/BitBlt 保持一致, 提前过滤无效调用, 避免 Skia 内部 UB
    //    (xSrc/ySrc 不能为负; width*/height* 必须为正)
    if (widthDest <= 0 || heightDest <= 0 || widthSrc <= 0 || heightSrc <= 0 || xSrc < 0 || ySrc < 0) {
        return false;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(pSrcRender != nullptr);
    if (pSrcRender == nullptr) {
        return false;
    }

    Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
    ASSERT(pSkiaRender != nullptr);
    if (pSkiaRender == nullptr) {
        return false;
    }
    SkSurface* skSurface = pSkiaRender->GetSkSurface();
    ASSERT(skSurface != nullptr);
    if (skSurface == nullptr) {
        return false;
    }

    // 2. 早期获取目标画布: GetSkCanvas() 是虚函数, 在 makeImageSnapshot() 之前调用,
    //    一旦画布无效可以快速失败, 避免后续相对昂贵的快照操作(GPU 同步或像素拷贝)
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return false;
    }

    // 3. 源图像快照: 该操作是函数中相对昂贵的部分(可能涉及 GPU 同步或像素拷贝)
    sk_sp<SkImage> skSrcImage = skSurface->makeImageSnapshot();
    ASSERT(skSrcImage != nullptr);
    if (skSrcImage == nullptr) {
        return false;
    }

    // 4. 构造 SkPaint: 继承 m_pSkPaint 的属性(颜色/滤镜等), 仅修改 Style 和 Alpha
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    if (alpha != 0xFF) {
        skPaint.setAlpha(alpha);
    }

    // 5. 直接构造 SkRect: 消除 SkIRect 中间对象和 SkRect::Make/offset 多次调用,
    //    减少内存访问与函数调用开销
    const SkScalar orgX = m_pSkPointOrg->fX;
    const SkScalar orgY = m_pSkPointOrg->fY;
    SkRect rcSkDest = SkRect::MakeXYWH((SkScalar)xDest + orgX,
                                       (SkScalar)yDest + orgY,
                                       (SkScalar)widthDest,
                                       (SkScalar)heightDest);
    SkRect rcSkSrc = SkRect::MakeXYWH((SkScalar)xSrc,
                                      (SkScalar)ySrc,
                                      (SkScalar)widthSrc,
                                      (SkScalar)heightSrc);

    // 6. 执行绘制: 使用 kFast_SrcRectConstraint 走 Skia 快速路径(本函数已校验源矩形合法性)
    skCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
    return true;
}

/** 计算平铺绘制图片时，应该循环绘制多少次（横向或者纵向绘制）
* @param [in] nAvailableSpace 总的绘制可用空间（宽度或者高度）
* @param [in] nImageSize 图片的大小（宽度或者高度）
* @param [in] nTiledMargin 连续两次绘制时的图片间距，最左侧（或者上侧）无间距
* @param [in] bFullyTiled true表示每次绘制图片必须保证图片能够完整显示，如果不能完整显示则不绘制，false表示不必保证图片完整绘制，运行绘制一部分
*/
static int32_t CalcDrawImageTimes(int32_t nAvailableSpace, int32_t nImageSize, int32_t nTiledMargin, bool bFullyTiled)
{
    // 实现思路
    //    边界条件处理：当可用空间小于等于 0、图片大小小于等于 0 时，无法绘制，返回 0。
    //    单次绘制判断：计算第一张图片是否能绘制（考虑是否需要完整显示）。若不能绘制，直接返回 0。
    //    多次绘制计算：若第一张图片可以绘制，计算剩余空间。对于剩余空间，每次绘制需要考虑图片大小和间距（从第二次绘制开始有间距），根据是否需要完整显示来确定总绘制次数。

    // 边界条件：可用空间或图片大小无效时，无法绘制
    if (nAvailableSpace <= 0 || nImageSize <= 0) {
        return 0;
    }

    // 第一张图片所需空间（无需间距）
    int32_t firstImageRequired = bFullyTiled ? nImageSize : 1; // 非完整绘制时，1像素即可绘制
    if (nAvailableSpace < firstImageRequired) {
        return 0; // 第一张图片都无法绘制
    }

    // 已绘制1次，剩余空间减去第一张图片的占用
    int32_t remainingSpace = nAvailableSpace - nImageSize;
    int32_t drawTimes = 1;

    // 剩余空间不足时，直接返回1
    if (remainingSpace <= 0) {
        return drawTimes;
    }

    // 后续每次绘制的前置间距（从第二次开始需要间距）
    while (true) {
        // 先扣除间距（若间距不足，无法继续绘制）
        if (remainingSpace < nTiledMargin) {
            break;
        }
        remainingSpace -= nTiledMargin;

        // 检查剩余空间是否能支持一次绘制（无论是否完整）
        if (remainingSpace <= 0) {
            break;
        }

        // 非完整绘制：只要剩余空间>0，就可以绘制一次（哪怕只显示1像素）
        // 完整绘制：需要剩余空间 >= 图片大小
        if ((!bFullyTiled) || (remainingSpace >= nImageSize)) {
            drawTimes++;
            remainingSpace -= nImageSize; // 扣除图片占用的空间（无论是否完整显示）
        }
        else {
            break; // 完整绘制模式下，剩余空间不足，无法继续
        }
    }

    return drawTimes;
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                            const UiRect& rcDest, const UiRect& rcDestCorners,
                            const UiRect& rcSource, const UiRect& rcSourceCorners,
                            uint8_t uFade,
                            const TiledDrawParam* pTiledDrawParam,    
                            bool bWindowShadowMode)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    UiRect rcTestTemp;
    if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
        return;
    }
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("Render_Skia::DrawImage");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }
    Bitmap_Skia* skiaBitmap = dynamic_cast<Bitmap_Skia*>(pBitmap);
    ASSERT(skiaBitmap != nullptr);
    if (skiaBitmap == nullptr) {
        return;
    }
    
    const SkBitmap& skSrcBitmap = skiaBitmap->GetSkBitmap();
    SkPixmap skSrcPixmap;
    sk_sp<SkImage> skImage;
    if (skSrcBitmap.peekPixels(&skSrcPixmap)) {
        skImage = SkImages::RasterFromPixmap(skSrcPixmap, nullptr, nullptr);
    }
    if (skImage == nullptr) {
        skImage = skSrcBitmap.asImage();
    }

    UiRect rcTemp;
    UiRect rcDrawSource;
    UiRect rcDrawDest;

    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    // 默认值就是kSrcOver
    skPaint.setBlendMode(SkBlendMode::kSrcOver);

    // 平铺绘制相关参数
    bool bTiledX = false;
    bool bTiledY = false;
    bool bFullTiledX = false;
    bool bFullTiledY = false;
    int32_t nTiledMarginX = 0;
    int32_t nTiledMarginY = 0;
    UiPadding rcTiledPadding;
    if (pTiledDrawParam != nullptr) {
        bTiledX = pTiledDrawParam->m_bTiledX;
        bTiledY = pTiledDrawParam->m_bTiledY;
        bFullTiledX = pTiledDrawParam->m_bFullTiledX;
        bFullTiledY = pTiledDrawParam->m_bFullTiledY;
        nTiledMarginX = pTiledDrawParam->m_nTiledMarginX;
        nTiledMarginY = pTiledDrawParam->m_nTiledMarginY;
        rcTiledPadding = pTiledDrawParam->m_rcTiledPadding;
    }

    // 绘制目标中间区域的矩形范围
    rcDrawDest.left = rcDest.left + rcDestCorners.left;
    rcDrawDest.top = rcDest.top + rcDestCorners.top;
    rcDrawDest.right = rcDest.right - rcDestCorners.right;
    rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

    if (bTiledX || bTiledY) {
        //中间部分，剪去平铺绘制的内边距
        rcDrawDest.Deflate(rcTiledPadding);
    }

    // 绘制图片的源区域（中间部分）
    rcDrawSource.left = rcSource.left + rcSourceCorners.left;
    rcDrawSource.top = rcSource.top + rcSourceCorners.top;
    rcDrawSource.right = rcSource.right - rcSourceCorners.right;
    rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;

    if (rcDestCorners.IsZero()) {
        //不是九宫格绘制时，禁用，避免图片无法绘出
        bWindowShadowMode = false;
    }

    if (!bWindowShadowMode && UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
        //绘制中间部分
        if (!bTiledX && !bTiledY) {
            //拉伸的方式绘制
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
        else if (bTiledX && bTiledY) {
            //平铺：横向和纵向均平铺绘制
            const int32_t nImageWidth = rcSource.Width() - rcSourceCorners.left - rcSourceCorners.right;
            const int32_t nAvailableSpaceX = rcDrawDest.Width();
            const int32_t iTimesX = CalcDrawImageTimes(nAvailableSpaceX, nImageWidth, nTiledMarginX, bFullTiledX);

            const int32_t nImageHeight = rcSource.Height() - rcSourceCorners.top - rcSourceCorners.bottom;
            const int32_t nAvailableSpaceY = rcDrawDest.Height();
            const int32_t iTimesY = CalcDrawImageTimes(nAvailableSpaceY, nImageHeight, nTiledMarginY, bFullTiledY);

            int32_t nPosY = rcDrawDest.top;
            for (int32_t j = 0; j < iTimesY; ++j) {
                int32_t nPosX = rcDrawDest.left;
                if (j > 0) {
                    nPosY += nTiledMarginY;
                }
                int32_t lDestTop = nPosY;
                int32_t lDestBottom = lDestTop + nImageHeight;
                int32_t lDrawHeight = nImageHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= (lDestBottom - rcDrawDest.bottom);
                    lDestBottom = rcDrawDest.bottom;
                }
                for (int32_t i = 0; i < iTimesX; ++i) {
                    if (i > 0) {
                        nPosX += nTiledMarginX;
                    }

                    int32_t lDestLeft = nPosX;
                    int32_t lDestRight = lDestLeft + nImageWidth;
                    int32_t lDrawWidth = nImageWidth;
                    if (lDestRight > rcDrawDest.right) {
                        lDrawWidth -= (lDestRight - rcDrawDest.right);
                        lDestRight = rcDrawDest.right;
                    }

                    rcDrawSource.left = rcSource.left + rcSourceCorners.left;
                    rcDrawSource.top = rcSource.top + rcSourceCorners.top;
                    rcDrawSource.right = rcDrawSource.left + lDrawWidth;
                    rcDrawSource.bottom = rcDrawSource.top + lDrawHeight;

                    UiRect rcDestTemp;
                    rcDestTemp.left = lDestLeft;
                    rcDestTemp.right = lDestRight;
                    rcDestTemp.top = lDestTop;
                    rcDestTemp.bottom = lDestBottom;
                    DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);

                    nPosX += lDrawWidth;
                }
                nPosY += lDrawHeight;
            }
        }
        else if (bTiledX) {
            //只有横向平铺绘制，纵向则拉伸绘制
            const int32_t nImageWidth = rcSource.Width() - rcSourceCorners.left - rcSourceCorners.right;
            const int32_t nAvailableSpaceX = rcDrawDest.Width();
            const int32_t iTimesX = CalcDrawImageTimes(nAvailableSpaceX, nImageWidth, nTiledMarginX, bFullTiledX);

            int32_t nPosX = rcDrawDest.left;
            for (int32_t i = 0; i < iTimesX; ++i) {
                if (i > 0) {
                    nPosX += nTiledMarginX;
                }
                int32_t lDestLeft = nPosX;
                int32_t lDestRight = lDestLeft + nImageWidth;
                int32_t lDrawWidth = nImageWidth;
                if (lDestRight > rcDrawDest.right) {
                    lDrawWidth -= (lDestRight - rcDrawDest.right);
                    lDestRight = rcDrawDest.right;
                }

                //源区域：如果设置了边角，则仅包含中间区域
                rcDrawSource.left = rcSource.left + rcSourceCorners.left;
                rcDrawSource.top = rcSource.top + rcSourceCorners.top;
                rcDrawSource.right = rcDrawSource.left + lDrawWidth;
                rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;

                UiRect rcDestTemp;
                rcDestTemp.top = rcDrawDest.top;
                rcDestTemp.bottom = rcDrawDest.bottom;
                rcDestTemp.left = lDestLeft;
                rcDestTemp.right = lDestRight;

                DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);

                nPosX += lDrawWidth;
            }
        }
        else { // bTiledY
            //只有纵向平铺绘制，横向则拉伸绘制
            const int32_t nImageHeight = rcSource.Height() - rcSourceCorners.top - rcSourceCorners.bottom;
            const int32_t nAvailableSpaceY = rcDrawDest.Height();
            const int32_t iTimesY = CalcDrawImageTimes(nAvailableSpaceY, nImageHeight, nTiledMarginY, bFullTiledY);

            int32_t nPosY = rcDrawDest.top;
            for (int32_t i = 0; i < iTimesY; ++i) {
                if (i > 0) {
                    nPosY += nTiledMarginY;
                }

                int32_t lDestTop = nPosY;
                int32_t lDestBottom = lDestTop + nImageHeight;
                int32_t lDrawHeight = nImageHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= (lDestBottom - rcDrawDest.bottom);
                    lDestBottom = rcDrawDest.bottom;
                }

                rcDrawSource.left = rcSource.left + rcSourceCorners.left;
                rcDrawSource.top = rcSource.top + rcSourceCorners.top;
                rcDrawSource.right = rcSource.right - rcSourceCorners.right;
                rcDrawSource.bottom = rcDrawSource.top + lDrawHeight;

                UiRect rcDestTemp;
                rcDestTemp.left = rcDrawDest.left;
                rcDestTemp.right = rcDrawDest.right;
                rcDestTemp.top = lDestTop;
                rcDestTemp.bottom = lDestBottom;

                DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
                nPosY += lDrawHeight;
            }
        }
    }

    // 通用绘制九宫格四角/四边的辅助 Lambda
    // 设置目标矩形和源矩形后，与绘制区域求交并执行绘制
    auto DrawCorner = [&](int32_t srcL, int32_t srcT, int32_t srcR, int32_t srcB,
                          int32_t dstL, int32_t dstT, int32_t dstR, int32_t dstB) {
        rcDrawSource = { srcL, srcT, srcR, srcB };
        rcDrawDest = { dstL, dstT, dstR, dstB };
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    };

    // left-top
    if (rcSourceCorners.left > 0 && rcSourceCorners.top > 0) {
        //左上角
        DrawCorner(
            rcSource.left, rcSource.top,
            rcSource.left + rcSourceCorners.left, rcSource.top + rcSourceCorners.top,
            rcDest.left, rcDest.top,
            rcDest.left + rcDestCorners.left, rcDest.top + rcDestCorners.top);
    }
    // top
    if (rcSourceCorners.top > 0) {
        //边框：上
        DrawCorner(
            rcSource.left + rcSourceCorners.left, rcSource.top,
            rcSource.right - rcSourceCorners.right, rcSource.top + rcSourceCorners.top,
            rcDest.left + rcDestCorners.left, rcDest.top,
            rcDest.right - rcDestCorners.right, rcDest.top + rcDestCorners.top);
    }
    // right-top
    if (rcSourceCorners.right > 0 && rcSourceCorners.top > 0) {
        //右上角
        DrawCorner(
            rcSource.right - rcSourceCorners.right, rcSource.top,
            rcSource.right, rcSource.top + rcSourceCorners.top,
            rcDest.right - rcDestCorners.right, rcDest.top,
            rcDest.right, rcDest.top + rcDestCorners.top);
    }
    // left
    if (rcSourceCorners.left > 0) {
        //边框：左
        DrawCorner(
            rcSource.left, rcSource.top + rcSourceCorners.top,
            rcSource.left + rcSourceCorners.left, rcSource.bottom - rcSourceCorners.bottom,
            rcDest.left, rcDest.top + rcDestCorners.top,
            rcDest.left + rcDestCorners.left, rcDest.bottom - rcDestCorners.bottom);
    }
    // right
    if (rcSourceCorners.right > 0) {
        //边框：右
        DrawCorner(
            rcSource.right - rcSourceCorners.right, rcSource.top + rcSourceCorners.top,
            rcSource.right, rcSource.bottom - rcSourceCorners.bottom,
            rcDest.right - rcDestCorners.right, rcDest.top + rcDestCorners.top,
            rcDest.right, rcDest.bottom - rcDestCorners.bottom);
    }
    // left-bottom
    if (rcSourceCorners.left > 0 && rcSourceCorners.bottom > 0) {
        //左下角
        DrawCorner(
            rcSource.left, rcSource.bottom - rcSourceCorners.bottom,
            rcSource.left + rcSourceCorners.left, rcSource.bottom,
            rcDest.left, rcDest.bottom - rcDestCorners.bottom,
            rcDest.left + rcDestCorners.left, rcDest.bottom);
    }
    // bottom
    if (rcSourceCorners.bottom > 0) {
        //边框：下
        DrawCorner(
            rcSource.left + rcSourceCorners.left, rcSource.bottom - rcSourceCorners.bottom,
            rcSource.right - rcSourceCorners.right, rcSource.bottom,
            rcDest.left + rcDestCorners.left, rcDest.bottom - rcDestCorners.bottom,
            rcDest.right - rcDestCorners.right, rcDest.bottom);
    }
    // right-bottom
    if (rcSourceCorners.right > 0 && rcSourceCorners.bottom > 0) {
        //右下角
        DrawCorner(
            rcSource.right - rcSourceCorners.right, rcSource.bottom - rcSourceCorners.bottom,
            rcSource.right, rcSource.bottom,
            rcDest.right - rcDestCorners.right, rcDest.bottom - rcDestCorners.bottom,
            rcDest.right, rcDest.bottom);
    }
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                            const UiRect& rcDest, const UiRect& rcSource,
                            uint8_t uFade,
                            const TiledDrawParam* pTiledDrawParam,
                            bool bWindowShadowMode)
{
    UiRect rcDestCorners;
    UiRect rcSourceCorners;
    return DrawImage(rcPaint, pBitmap, 
                     rcDest, rcDestCorners,
                     rcSource, rcSourceCorners,
                     uFade, pTiledDrawParam, bWindowShadowMode);
}

void Render_Skia::DrawImageRect(const UiRect& rcPaint, IBitmap* pBitmap,
                                const UiRect& rcDest, const UiRect& rcSource,
                                uint8_t uFade, IMatrix* pMatrix)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (pMatrix == nullptr) {
        //仅在没有Matrix的情况下判断裁剪区域，
        //因为有Matrix时，实际绘制区域并不是rcDest，而是变换过后的位置，需要调整判断方法
        UiRect rcTestTemp;
        if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
            return;
        }
    }

    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }

    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    //默认值就是kSrcOver
    skPaint.setBlendMode(SkBlendMode::kSrcOver);

    Bitmap_Skia* skiaBitmap = dynamic_cast<Bitmap_Skia*>(pBitmap);
    ASSERT(skiaBitmap != nullptr);
    if (skiaBitmap == nullptr) {
        return;
    }
    const SkBitmap& skSrcBitmap = skiaBitmap->GetSkBitmap();
    SkPixmap skSrcPixmap;
    sk_sp<SkImage> skImage;
    if (skSrcBitmap.peekPixels(&skSrcPixmap)) {
        skImage = SkImages::RasterFromPixmap(skSrcPixmap, nullptr, nullptr);
    }
    if (skImage == nullptr) {
        skImage = skSrcBitmap.asImage();
    }

    bool isMatrixSet = false;
    if (pMatrix != nullptr) {
        Matrix_Skia* pSkMatrix = dynamic_cast<Matrix_Skia*>(pMatrix);
        if (pSkMatrix != nullptr) {
            SkM44 skM44(*pSkMatrix->GetMatrix());
            skCanvas->setMatrix(skM44);
            isMatrixSet = true;
        }
    }
    DrawSkiaImage::DrawImage(skCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
    if (isMatrixSet) {
        skCanvas->resetMatrix();
    }
}

void Render_Skia::FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    // 空矩形跳过：避免无效的 Skia drawRect 调用（含 NaN/Inf 退化为空矩形）
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::InitGradientColor(SkPaint& skPaint, const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const
{
    if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
        nColor2Direction = 1;
    }
    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    SkPoint pts[2] = { {0, 0}, {0, 0} };
    pts[0].set(rcSkDest.fLeft, rcSkDest.fTop);
    if (nColor2Direction == 2) {
        //上->下
        pts[1].set(rcSkDest.fLeft, rcSkDest.fBottom);
    }
    else if (nColor2Direction == 3) {
        //左上->右下
        pts[1].set(rcSkDest.fRight, rcSkDest.fBottom);
    }
    else if (nColor2Direction == 4) {
        //右上->左下
        pts[0].set(rcSkDest.fRight, rcSkDest.fTop);
        pts[1].set(rcSkDest.fLeft, rcSkDest.fBottom);
    }
    else {
        //左->右
        pts[1].set(rcSkDest.fRight, rcSkDest.fTop);
    }

    SkColor4f colors[2];
    colors[0] = SkColor4f::FromColor(SkColorSetARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB()));
    colors[1] = SkColor4f::FromColor(SkColorSetARGB(dwColor2.GetA(), dwColor2.GetR(), dwColor2.GetG(), dwColor2.GetB()));
    SkGradient::Colors skColors(SkSpan<const SkColor4f>(colors, 2), SkTileMode::kClamp);
    sk_sp<SkShader> shader(SkShaders::LinearGradient(pts, SkGradient(skColors, SkGradient::Interpolation())));
    skPaint.setShader(shader);
}

void Render_Skia::FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (dwColor2.IsEmpty()) {
        return FillRect(rc, dwColor, uFade);
    }
    if (rc.IsEmpty()) {
        return;
    }

    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    // 退化检查：端点重合（pt1==pt2）跳过 drawLine 调用
    if (pt1.Equals(pt2)) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setStrokeWidth(SkScalar(fWidth));

    SkPoint skPt1 = SkPoint::Make(pt1.x, pt1.y);
    skPt1.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkPoint skPt2 = SkPoint::Make(pt2.x, pt2.y);
    skPt2.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawLine(skPt1, skPt2, skPaint);
    }
}

void Render_Skia::DrawLine(const UiPointF& pt1, const UiPointF& pt2, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

    SkPoint skPt1 = SkPoint::Make(pt1.x, pt1.y);
    skPt1.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkPoint skPt2 = SkPoint::Make(pt2.x, pt2.y);
    skPt2.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawLine(skPt1, skPt2, skPaint);
    }
}

void Render_Skia::DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    // 空矩形跳过：bLineInRect 模式还会再减去半线宽，可能让 fRight < fLeft
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(fWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    if (bLineInRect) {
        //确保画的线，都在矩形范围内
        SkScalar fHalfStrokeWidth = skPaint.getStrokeWidth() / 2;
        rcSkDest.fLeft += fHalfStrokeWidth;
        rcSkDest.fRight -= fHalfStrokeWidth;
        rcSkDest.fTop += fHalfStrokeWidth;
        rcSkDest.fBottom -= fHalfStrokeWidth;        
    }
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    if (bLineInRect) {
        //确保画的线，都在矩形范围内
        SkScalar fHalfStrokeWidth = skPaint.getStrokeWidth() / 2;
        rcSkDest.fLeft += fHalfStrokeWidth;
        rcSkDest.fRight -= fHalfStrokeWidth;
        rcSkDest.fTop += fHalfStrokeWidth;
        rcSkDest.fBottom -= fHalfStrokeWidth;
    }
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(fWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, rx, ry, skPaint);
    }
}

void Render_Skia::DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, rx, ry, skPaint);
    }
}

void Render_Skia::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (rc.IsEmpty()) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, rx, ry, skPaint);
    }
}

void Render_Skia::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (dwColor2.IsEmpty()) {
        return FillRoundRect(rc, rx, ry, dwColor, uFade);
    }
    if (rc.IsEmpty()) {
        return;
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
    rcSkDest.offset(*m_pSkPointOrg);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, rx, ry, skPaint);
    }
}

void Render_Skia::DrawCircle(const UiPointF& centerPt, float radius, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    // 退化检查：半径 <= 0 或 NaN 时跳过；使用 !(radius > 0) 同时排除负数和 NaN
    if (!(radius > 0.0f)) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(fWidth);
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkPoint rcSkPoint = SkPoint::Make(centerPt.x, centerPt.y);
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, radius, skPaint);
    }
}

void Render_Skia::DrawCircle(const UiPointF& centerPt, float radius, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (!(radius > 0.0f)) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

    SkPoint rcSkPoint = SkPoint::Make(centerPt.x, centerPt.y);
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, radius, skPaint);
    }
}

void Render_Skia::FillCircle(const UiPointF& centerPt, float radius, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (!(radius > 0.0f)) {
        return;
    }
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkPoint rcSkPoint = SkPoint::Make(centerPt.x, centerPt.y);
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, radius, skPaint);
    }
}

void Render_Skia::DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool useCenter, 
                          const IPen* pen, 
                          UiColor* gradientColor,
                          const UiRect* gradientRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    SkPaint paint = *m_pSkPaint;
    SetPaintByPen(paint, pen);

    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect ovalRect = SkRect::Make(rcSkDestI);
    ovalRect.offset(*m_pSkPointOrg);

    if((gradientColor != nullptr) && (gradientRect != nullptr)) {
        UiColor dwColor = pen->GetColor();
        SkColor4f colors[2];        
        colors[0] = SkColor4f::FromColor(SkColorSetARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB()));
        colors[1] = SkColor4f::FromColor(SkColorSetARGB(gradientColor->GetA(), gradientColor->GetR(), gradientColor->GetG(), gradientColor->GetB()));
        
        SkIRect rcGradientDestI = { gradientRect->left, gradientRect->top, gradientRect->right, gradientRect->bottom };
        SkRect rcGradientDest = SkRect::Make(rcGradientDestI);
        rcGradientDest.offset(*m_pSkPointOrg);

        SkPoint pts[2] = { {0, 0}, {0, 0} };
        pts[0].set(rcGradientDest.fLeft, rcGradientDest.fTop);
        pts[1].set(rcGradientDest.fRight, rcGradientDest.fBottom);

        SkGradient::Colors skColors(SkSpan<const SkColor4f>(colors), SkTileMode::kClamp);
        sk_sp<SkShader> shaderA(SkShaders::LinearGradient(pts, SkGradient(skColors, SkGradient::Interpolation())));
        paint.setShader(shaderA);
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawArc(ovalRect, startAngle, sweepAngle, useCenter, paint);
    }
}

void Render_Skia::DrawPath(const IPath* path, const IPen* pen)
{
    ASSERT(path != nullptr);
    ASSERT(pen != nullptr);
    if ((path == nullptr) || (pen == nullptr)) {
        return;
    }
    const Path_Skia* pSkiaPath = dynamic_cast<const Path_Skia*>(path);
    ASSERT(pSkiaPath != nullptr);
    if (pSkiaPath == nullptr) {
        return;
    }

    SkPaint paint = *m_pSkPaint;
    SetPaintByPen(paint, pen);

    SkPathBuilder skPathBuilder = *pSkiaPath->GetSkPathBuilder();
    skPathBuilder.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
    SkPath skPath = skPathBuilder.snapshot();
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, paint);
    }
}

float Render_Skia::GetScaleFloat(float fValue) const
{
    if (m_spRenderDpi != nullptr) {
        return m_spRenderDpi->GetScaleFloat(fValue);
    }
    return fValue;
}

void Render_Skia::SetPaintByPen(SkPaint& skPaint, const IPen* pen)
{
    if (pen == nullptr) {
        return;
    }
    skPaint.setColor(pen->GetColor().GetARGB());

    sk_sp<SkPathEffect> skPathEffect;
    IPen::DashStyle dashStyle = pen->GetDashStyle();
    //线宽的倍数
    float fRatio = pen->GetWidth() / GetScaleFloat(1.0f);
    switch (dashStyle) {
    case IPen::kDashStyleSolid:
    {
        float fValue = GetScaleFloat(1.0f) * fRatio;
        SkScalar intervals[] = { fValue * 1.0f, 0.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 2), 0.0f);
        break;
    }
    case IPen::kDashStyleDash:
    {
        float fValue = GetScaleFloat(5.0f) * fRatio;
        SkScalar intervals[] = { fValue * 1.0f, fValue * 1.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 2), 0.0f);
        break;
    }
    case IPen::kDashStyleDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue1 * 1.0f, fValue4 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 2), 0.0f);
        break;
    }
    case IPen::kDashStyleDashDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue4 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 4), 0.0f);
        break;
    }
    case IPen::kDashStyleDashDotDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue4 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 6), 0.0f);
        break;
    }
    default:
    {
        SkScalar intervals[] = { 1.0f, 0.0f };
        skPathEffect = SkDashPathEffect::Make(SkSpan<const SkScalar>(intervals, 2), 0.0f);
        break;
    }
    }
    skPaint.setPathEffect(skPathEffect);

    SkPaint::Cap cap = SkPaint::Cap::kDefault_Cap;
    switch (pen->GetDashCap()) {
    case IPen::kButt_Cap:
        cap = SkPaint::Cap::kButt_Cap;
        break;
    case IPen::kRound_Cap:
        cap = SkPaint::Cap::kRound_Cap;
        break;
    case IPen::kSquare_Cap:
        cap = SkPaint::Cap::kSquare_Cap;
        break;
    default:
        break;
    }

    skPaint.setStrokeCap(cap);

    SkPaint::Join join = SkPaint::Join::kDefault_Join;
    switch (pen->GetLineJoin()) {
    case IPen::LineJoin::kMiter_Join:
        join = SkPaint::Join::kMiter_Join;
        break;
    case IPen::LineJoin::kBevel_Join:
        join = SkPaint::Join::kBevel_Join;
        break;
    case IPen::LineJoin::kRound_Join:
        join = SkPaint::Join::kRound_Join;
        break;
    default:
        break;
    }

    skPaint.setStrokeJoin(join);
    skPaint.setStyle(SkPaint::kStroke_Style);
    if (skPaint.isAntiAlias()) {
        skPaint.setStrokeWidth((SkScalar)pen->GetWidth() - 0.5f);
    }
    else {
        skPaint.setStrokeWidth((SkScalar)pen->GetWidth());
    }
}

void Render_Skia::FillPath(const IPath* path, const IBrush* brush)
{
    ASSERT(path != nullptr);
    ASSERT(brush != nullptr);
    if ((path == nullptr) || (brush == nullptr)) {
        return;
    }
    const Path_Skia* pSkiaPath = dynamic_cast<const Path_Skia*>(path);
    ASSERT(pSkiaPath != nullptr);
    if (pSkiaPath == nullptr) {
        return;
    }

    SkPaint paint = *m_pSkPaint;
    paint.setColor(brush->GetColor().GetARGB());
    paint.setStyle(SkPaint::kFill_Style);

    SkPathBuilder skPathBuilder = *pSkiaPath->GetSkPathBuilder();
    skPathBuilder.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
    SkPath skPath = skPathBuilder.snapshot();

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, paint);
    }
}

void Render_Skia::FillPath(const IPath* path, const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction)
{
    ASSERT(path != nullptr);
    if (path == nullptr){
        return;
    }
    const Path_Skia* pSkiaPath = dynamic_cast<const Path_Skia*>(path);
    ASSERT(pSkiaPath != nullptr);
    if (pSkiaPath == nullptr) {
        return;
    }
    if (rc.IsEmpty()) {
        return;
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);
    
    SkPathBuilder skPathBuilder = *pSkiaPath->GetSkPathBuilder();
    skPathBuilder.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
    SkPath skPath = skPathBuilder.snapshot();

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, skPaint);
    }
}

void Render_Skia::DrawString(const DString& strText, const DrawStringParam& drawParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        //这种情况是窗口大小为0的情况，返回，不加断言
        return;
    }
    if (drawParam.uFormat & TEXT_VERTICAL) {
        //纵向绘制文本
        VerticalDrawText drawTextUtil(GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
        return drawTextUtil.DrawString(strText, drawParam);
    }
    else if ((drawParam.uFormat & TEXT_HJUSTIFY) || (drawParam.fWordSpacing > 0.0001f)) {
        //当横向文本，对齐方式设置为两端对齐时，或者设置了字间距时，使用该实现方案（因为修改SkTextBox的实现比较困难，维护难度高）
        HorizontalDrawText drawTextUtil(GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
        return drawTextUtil.DrawString(strText, drawParam);
    }

#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("Render_Skia::DrawString");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    //ASSERT(!drawParam.textRect.IsEmpty());
    if (drawParam.textRect.IsEmpty()) {
        return;
    }
    ASSERT(drawParam.pFont != nullptr);
    if (drawParam.pFont == nullptr) {
        return;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }
    //文本编码
    const SkTextEncoding textEncoding = GetTextEncoding();
    
    //获取字体接口    
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(drawParam.pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return;
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    //绘制属性设置
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(drawParam.dwTextColor.GetA(),
                    drawParam.dwTextColor.GetR(),
                    drawParam.dwTextColor.GetG(),
                    drawParam.dwTextColor.GetB());
    if (drawParam.uFade != 0xFF) {
        skPaint.setAlpha(drawParam.uFade);
    }

    //绘制区域
    SkIRect rcSkDestI = { drawParam.textRect.left, drawParam.textRect.top,
                          drawParam.textRect.right, drawParam.textRect.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    //设置绘制属性
    SkTextBox skTextBox;
    skTextBox.SetBox(rcSkDest);
    if (drawParam.uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        //单行文本
        skTextBox.SetLineMode(TextBoxLineMode::kOneLine_Mode);
    }

    //设置行间距
    skTextBox.SetSpacing(drawParam.fSpacingMul, drawParam.fSpacingAdd);

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = false;
    if (drawParam.uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
        bEndEllipsis = true;
    }
    skTextBox.SetEndEllipsis(bEndEllipsis);

    bool bPathEllipsis = false;
    if (drawParam.uFormat & DrawStringFormat::TEXT_PATH_ELLIPSIS) {
        bPathEllipsis = true;
    }
    skTextBox.SetPathEllipsis(bPathEllipsis);

    //绘制文字时，不使用裁剪区域（可能会导致文字绘制超出边界）
    if (drawParam.uFormat & DrawStringFormat::TEXT_NOCLIP) {
        skTextBox.SetClipBox(false);
    }
    //删除线
    skTextBox.SetStrikeOut(drawParam.pFont->IsStrikeOut());
    //下划线
    skTextBox.SetUnderline(drawParam.pFont->IsUnderline());

    if (drawParam.uFormat & DrawStringFormat::TEXT_HCENTER) {
        //横向对齐：居中对齐
        skTextBox.SetTextAlign(SkTextBox::kCenter_Align);
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_RIGHT) {
        //横向对齐：右对齐
        skTextBox.SetTextAlign(SkTextBox::kRight_Align);
    }
    else {
        //横向对齐：左对齐
        skTextBox.SetTextAlign(SkTextBox::kLeft_Align);
    }

    if (drawParam.uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        skTextBox.SetSpacingAlign(SkTextBox::kCenter_SpacingAlign);
    }
    else if (drawParam.uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：下对齐
        skTextBox.SetSpacingAlign(SkTextBox::kEnd_SpacingAlign);
    }
    else {
        //纵向对齐：上对齐
        skTextBox.SetSpacingAlign(SkTextBox::kStart_SpacingAlign);
    }

    FallbackFontCreator fallbackFontCreator = [this, drawParam](SkUnichar unicodeChar, SkGlyphID* glyphId) {
        return DrawSkiaText::CreateFallbackFont(drawParam.pFont, unicodeChar, glyphId);
        };

    SkiaTextData textData((const char*)strText.c_str(), strText.size() * sizeof(DString::value_type), textEncoding);
    skTextBox.Draw(skCanvas, textData, *pSkFont, skPaint, fallbackFontCreator);
}

UiRect Render_Skia::MeasureString(const DString& strText, const MeasureStringParam& measureParam)
{
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        //这种情况是窗口大小为0的情况，返回空，不加断言
        return UiRect();
    }
    if (measureParam.uFormat & TEXT_VERTICAL) {
        //纵向绘制文本
        VerticalDrawText drawTextUtil(GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
        return drawTextUtil.MeasureString(strText, measureParam);
    }
    else if ((measureParam.uFormat & TEXT_HJUSTIFY) || (measureParam.fWordSpacing > 0.0001f)) {
        //当横向文本，对齐方式设置为两端对齐时，或者设置了字间距时，使用该实现方案（因为修改SkTextBox的实现比较困难，维护难度高）
        HorizontalDrawText drawTextUtil(GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
        return drawTextUtil.MeasureString(strText, measureParam);
    }

#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("Render_Skia::MeasureString");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    ASSERT(!strText.empty());
    if (strText.empty()) {
        return UiRect();
    }
    ASSERT(measureParam.pFont != nullptr);
    if (measureParam.pFont == nullptr) {
        return UiRect();
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return UiRect();
    }

    FallbackFontCreator fallbackFontCreator = [this, measureParam](SkUnichar unicodeChar, SkGlyphID* glyphId) {
        return DrawSkiaText::CreateFallbackFont(measureParam.pFont, unicodeChar, glyphId);
        };

    //获取字体接口
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(measureParam.pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return UiRect();
    }
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return UiRect();
    }

    //绘制属性设置
    SkPaint skPaint = *m_pSkPaint;

    bool bSingleLineMode = false;
    if (measureParam.uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        bSingleLineMode = true;
    }
        
    //计算行高
    SkFontMetrics fontMetrics;
    SkScalar fontHeight = pSkFont->getMetrics(&fontMetrics);

    MeasureTextTempData measureTempData;  //内部临时变量，为提升执行速度，在外部声明变量

    if (bSingleLineMode) {
        //单行模式
        SkRect bounds; //斜体字时，这个宽度包含了外延的宽度
        SkScalar textWidth = DrawSkiaText::MeasureText(*pSkFont, strText.c_str(),
                                                       strText.size() * sizeof(DString::value_type), GetTextEncoding(),
                                                       &bounds, &skPaint, fallbackFontCreator, measureTempData);
        textWidth = std::max(textWidth, bounds.width());
        int textIWidth = SkScalarTruncToInt(textWidth + 0.5f);
        if (textWidth > textIWidth) {
            textIWidth += 1;
        }
        if (textIWidth <= 0) {
            return UiRect();
        }
        UiRect rc;
        rc.left = 0;
        rc.right = textIWidth;
        rc.top = 0;
        rc.bottom = SkScalarTruncToInt(fontHeight + 0.5f);
        if (fontHeight > rc.bottom) {
            rc.bottom += 1;
        }
        return rc;
    }
    else {
        //多行模式
        int32_t nRectWidth = measureParam.rectSize;
        if (nRectWidth <= 0) {
            nRectWidth = INT32_MAX;
        }
        std::vector<size_t> lineLenList; //每行文本数据的长度（字节）
        int32_t lineCount = DrawSkiaText::CountLines((const char*)strText.c_str(),
                                                     strText.size() * sizeof(DString::value_type),
                                                     GetTextEncoding(),
                                                     *pSkFont, fallbackFontCreator,
                                                     skPaint,
                                                     SkScalar(nRectWidth),
                                                     TextBoxLineMode::kWordBreak_Mode,
                                                     &lineLenList);
        //计算所需宽度
        int32_t textWidth = 0;
        ASSERT((int32_t)lineLenList.size() == lineCount);
        if (!lineLenList.empty()) {
            std::vector<DString> lineTextList; //每行的文本
            size_t nTextPos = 0;
            for (size_t len : lineLenList) {
                ASSERT((len % sizeof(DString::value_type)) == 0);
                size_t nTextLen = len / sizeof(DString::value_type);
                lineTextList.push_back(strText.substr(nTextPos, nTextLen));
                nTextPos += nTextLen;
            }
            for (const DString& lineText : lineTextList) {
                //按单行评估每行文本，取最大宽度
                SkRect bounds; //斜体字时，这个宽度包含了外延的宽度
                SkScalar lineTextLen = DrawSkiaText::MeasureText(*pSkFont, lineText.c_str(),
                                                                 lineText.size() * sizeof(DString::value_type),
                                                                 GetTextEncoding(), &bounds, &skPaint, fallbackFontCreator,
                                                                 measureTempData);
                lineTextLen = std::max(lineTextLen, bounds.width());
                int32_t lineTextIWidth = SkScalarTruncToInt(lineTextLen + 0.5f);
                if (lineTextLen > lineTextIWidth) {
                    lineTextIWidth += 1;
                }
                textWidth = std::max(textWidth, lineTextIWidth);
            }
        }
        float spacingMul = 1.0f;//行间距倍数，暂不支持设置
        SkScalar scaledSpacing = fontHeight * spacingMul;
        SkScalar textHeight = fontHeight;
        if (lineCount > 0) {
            textHeight += scaledSpacing * (lineCount - 1);
        }
        UiRect rc;
        rc.left = 0;
        rc.right = textWidth;
        rc.top = 0;
        rc.bottom = SkScalarTruncToInt(textHeight + 0.5f);
        if (textHeight > rc.bottom) {
            rc.bottom += 1;
        }
        return rc;
    }
}

void Render_Skia::MeasureRichText(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
     ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    drawRichText.InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, nullptr, nullptr, pRichTextRects);
}

void Render_Skia::MeasureRichText2(const UiRect& textRect,
                                   const UiSize& szScrollOffset,
                                   IRenderFactory* pRenderFactory,
                                   const std::vector<RichTextData>& richTextData,
                                   RichTextLineInfoParam* pLineInfoParam,
                                   std::vector<std::vector<UiRect>>* pRichTextRects)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    drawRichText.InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, nullptr, pRichTextRects);
}

void Render_Skia::MeasureRichText3(const UiRect& textRect,
                                   const UiSize& szScrollOffset,
                                   IRenderFactory* pRenderFactory, 
                                   const std::vector<RichTextData>& richTextData,
                                   RichTextLineInfoParam* pLineInfoParam,
                                   std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                   std::vector<std::vector<UiRect>>* pRichTextRects)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    drawRichText.InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, &spDrawRichTextCache, pRichTextRects);
}

void Render_Skia::DrawRichText(const UiRect& textRect,
                               const UiSize& szScrollOffset,
                               IRenderFactory* pRenderFactory,
                               const std::vector<RichTextData>& richTextData,
                               uint8_t uFade,
                               std::vector<std::vector<UiRect>>* pRichTextRects)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    drawRichText.InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, uFade, false, nullptr, nullptr, pRichTextRects);
}

bool Render_Skia::CreateDrawRichTextCache(const UiRect& textRect,
                                          const UiSize& szScrollOffset,
                                          IRenderFactory* pRenderFactory,
                                          const std::vector<RichTextData>& richTextData,
                                          std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    spDrawRichTextCache.reset();
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    drawRichText.InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, nullptr, &spDrawRichTextCache, nullptr);
    return spDrawRichTextCache != nullptr;
}

bool Render_Skia::IsValidDrawRichTextCache(const UiRect& textRect,
                                           const std::vector<RichTextData>& richTextData,
                                           const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    return drawRichText.IsValidDrawRichTextCache(textRect, richTextData, spDrawRichTextCache);
}

bool Render_Skia::UpdateDrawRichTextCache(std::shared_ptr<DrawRichTextCache>& spOldDrawRichTextCache,
                                          const std::shared_ptr<DrawRichTextCache>& spUpdateDrawRichTextCache,
                                          std::vector<RichTextData>& richTextDataNew,
                                          size_t nStartLine,
                                          const std::vector<size_t>& modifiedLines,
                                          size_t nModifiedRows,
                                          const std::vector<size_t>& deletedLines,
                                          size_t nDeletedRows,
                                          const std::vector<int32_t>& rowRectTopList)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    return drawRichText.UpdateDrawRichTextCache(spOldDrawRichTextCache,
                                                spUpdateDrawRichTextCache,
                                                richTextDataNew,
                                                nStartLine,
                                                modifiedLines,
                                                nModifiedRows,
                                                deletedLines,
                                                nDeletedRows,
                                                rowRectTopList);
}

bool Render_Skia::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    ui::DrawRichText drawRichText(const_cast<Render_Skia*>(this), GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    return drawRichText.IsDrawRichTextCacheEqual(first, second);
}

void Render_Skia::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,                                       
                                        const UiRect& rcNewTextRect,
                                        const UiSize& szNewScrollOffset,
                                        const std::vector<int32_t>& rowXOffset,
                                        uint8_t uFade,
                                        std::vector<std::vector<UiRect>>* pRichTextRects)
{
    ui::DrawRichText drawRichText(this, GetSkCanvas(), m_pSkPaint.get(), m_pSkPointOrg.get());
    return drawRichText.DrawRichTextCacheData(spDrawRichTextCache,
                                              rcNewTextRect,
                                              szNewScrollOffset,
                                              rowXOffset,
                                              uFade,
                                              pRichTextRects);
}

void Render_Skia::DrawBoxShadow(const UiRect& rc,
                                const UiSize& roundSize, 
                                const UiPoint& cpOffset, 
                                int32_t nBlurRadius, 
                                int32_t nSpreadRadius,
                                UiColor dwColor,
                                uint8_t uAlpha)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(dwColor.GetARGB() != 0);
    if (nBlurRadius < 0) {
        nBlurRadius = 0;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }

    //阴影的扩撒区域
    ui::UiRect destRc = rc;    
    destRc.left -= nSpreadRadius;
    destRc.top -= nSpreadRadius;
    destRc.right += nSpreadRadius;
    destRc.bottom += nSpreadRadius;

    SkRect srcRc;
    srcRc.setXYWH((SkScalar)destRc.left, (SkScalar)destRc.top, (SkScalar)destRc.Width(), (SkScalar)destRc.Height());

    //原始区域，做裁剪用
    SkRect excludeRc;
    excludeRc.setXYWH((SkScalar)rc.left, (SkScalar)rc.top, (SkScalar)rc.Width(), (SkScalar)rc.Height());

    SkPathBuilder shadowPath;
    shadowPath.addRRect(SkRRect::MakeRectXY(srcRc, (SkScalar)roundSize.cx, (SkScalar)roundSize.cy));

    SkPathBuilder excludePath;
    excludePath.addRRect(SkRRect::MakeRectXY(excludeRc, (SkScalar)roundSize.cx, (SkScalar)roundSize.cy));

    SkPaint paint = *m_pSkPaint;
    paint.setColor(dwColor.GetARGB());
    paint.setStyle(SkPaint::kFill_Style);

    SkAutoCanvasRestore autoCanvasRestore(skCanvas, true);

    //裁剪中间区域
    excludePath.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);
    skCanvas->clipPath(excludePath.snapshot(), SkClipOp::kDifference);

    shadowPath.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    //设置绘制阴影的属性
    const SkScalar sigmaX = (SkScalar)nBlurRadius;
    const SkScalar sigmaY = (SkScalar)nBlurRadius;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setColor(dwColor.GetARGB());

    paint.setImageFilter(SkImageFilters::Blur(sigmaX, sigmaY, SkTileMode::kDecal, nullptr));

    //设置绘制阴影的偏移量
    const SkScalar offsetX = (SkScalar)cpOffset.x;
    const SkScalar offsetY = (SkScalar)cpOffset.y;
    SkMatrix mat;
    mat.postTranslate(offsetX, offsetY);
    shadowPath.transform(mat);
    if (uAlpha != 0xFF) {
        paint.setAlpha(uAlpha);
    }
    skCanvas->drawPath(shadowPath.snapshot(), paint);
}

bool Render_Skia::ReadPixels(const UiRect& rc, void* dstPixels, size_t dstPixelsLen)
{
    ASSERT(dstPixels != nullptr);
    if (dstPixels == nullptr) {
        return false;
    }
    ASSERT(!rc.IsEmpty());
    if (rc.IsEmpty()) {
        return false;
    }
    ASSERT(dstPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (dstPixelsLen < (rc.Width() * rc.Height() * sizeof(uint32_t))) {
        return false;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return false;
    }

    SkBitmap skBitmap;
    skBitmap.setInfo(SkImageInfo::Make(rc.Width(), rc.Height(), SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType));
    skBitmap.setPixels(dstPixels);
    bool bRet = skCanvas->readPixels(skBitmap, rc.left + (int32_t)m_pSkPointOrg->fX, rc.top + (int32_t)m_pSkPointOrg->fY);
    ASSERT_UNUSED_VARIABLE(bRet);
    return bRet;
}

bool Render_Skia::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc)
{
    ASSERT(srcPixels != nullptr);
    if (srcPixels == nullptr) {
        return false;
    }
    ASSERT(!rc.IsEmpty());
    if (rc.IsEmpty()) {
        return false;
    }
    ASSERT(srcPixelsLen == (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (srcPixelsLen != (rc.Width() * rc.Height() * sizeof(uint32_t))) {
        return false;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return false;
    }

    SkBitmap skBitmap;
    skBitmap.setInfo(SkImageInfo::Make(rc.Width(), rc.Height(), SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType));
    skBitmap.setPixels(srcPixels);

    bool bRet = skCanvas->writePixels(skBitmap, rc.left + (int32_t)m_pSkPointOrg->fX, rc.top + (int32_t)m_pSkPointOrg->fY);
    ASSERT_UNUSED_VARIABLE(bRet);
    return bRet;
}

bool Render_Skia::WritePixels(void* srcPixels, size_t srcPixelsLen, const UiRect& rc, const UiRect& rcPaint)
{
    if (rc == rcPaint) {
        return WritePixels(srcPixels, srcPixelsLen, rc);
    }
    ASSERT(srcPixels != nullptr);
    if (srcPixels == nullptr) {
        return false;
    }
    ASSERT(!rc.IsEmpty());
    if (rc.IsEmpty()) {
        return false;
    }
    ASSERT(srcPixelsLen == (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (srcPixelsLen != (rc.Width() * rc.Height() * sizeof(uint32_t))) {
        return false;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return false;
    }

    UiRect updateRect = rc;
    updateRect.Intersect(rcPaint);
    ASSERT(!updateRect.IsEmpty());
    if (updateRect.IsEmpty()) {
        return false;
    }

    int32_t destX = updateRect.left;
    int32_t destY = updateRect.top;
    updateRect.Offset(-rc.left, -rc.top);

    
    SkImageInfo skImageInfo = SkImageInfo::Make(rc.Width(), rc.Height(), SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType);
    SkBitmap skBitmap;
    skBitmap.setInfo(skImageInfo);
    skBitmap.setPixels(srcPixels);

    SkBitmap dstDirtyBitmap;
    SkIRect dstRect = SkIRect::MakeXYWH(updateRect.left, updateRect.top, updateRect.Width(), updateRect.Height());
    bool bRet = skBitmap.extractSubset(&dstDirtyBitmap, dstRect);
    ASSERT(bRet);
    if(bRet) {
        bRet = skCanvas->writePixels(dstDirtyBitmap, destX + (int32_t)m_pSkPointOrg->fX, destY + (int32_t)m_pSkPointOrg->fY);
        ASSERT_UNUSED_VARIABLE(bRet);
    }
    return bRet;    
}

RenderClipType Render_Skia::GetClipInfo(std::vector<UiRect>& clipRects)
{
    RenderClipType clipType = RenderClipType::kEmpty;
    clipRects.clear();

    SkCanvas* skCanvas = GetSkCanvas();
    if (skCanvas != nullptr) {
        if (skCanvas->isClipEmpty()) {
            clipType = RenderClipType::kEmpty;
        }
        else if (skCanvas->isClipRect()) {
            clipType = RenderClipType::kRect;
            SkRect rcClip;
            if (skCanvas->getLocalClipBounds(&rcClip)) {
                UiRect rect = { (int32_t)rcClip.left(), (int32_t)rcClip.top(), (int32_t)rcClip.right(), (int32_t)rcClip.bottom() };
                rect.Deflate(1, 1); //注意需要向内缩小一个象素（Skia在设置Clip的时候，会放大一个像素）
                clipRects.push_back(rect);
            }
        }
        else {
            clipType = RenderClipType::kRegion;
            SkRegion rgn;
            skCanvas->temporary_internal_getRgnClip(&rgn);
            SkRegion::Iterator it(rgn);
            for (; !it.done(); it.next()) {
                SkIRect skrc = it.rect();
                UiRect rect = { skrc.fLeft, skrc.fTop, skrc.fRight, skrc.fBottom };
                rect.Deflate(1, 1); //注意需要向内缩小一个象素（Skia在设置Clip的时候，会放大一个像素）
                clipRects.push_back(rect);
            }
        }
    }
    if (clipRects.empty()) {
        clipType = RenderClipType::kEmpty;
    }
    else {
        //将坐标转换为客户区坐标
        for (UiRect& rc : clipRects) {
            rc.Offset(-(int32_t)m_pSkPointOrg->x(), -(int32_t)m_pSkPointOrg->y());
        }
    }
    return clipType;
}

bool Render_Skia::IsClipEmpty() const
{
    SkCanvas* skCanvas = GetSkCanvas();
    if ((skCanvas != nullptr) && (skCanvas->isClipEmpty())) {
        return true;
    }
    return false;
}

bool Render_Skia::IsEmpty() const
{
    SkCanvas* skCanvas = GetSkCanvas();
    return (skCanvas != nullptr) && (GetWidth() > 0) && (GetHeight() > 0);
}

void Render_Skia::SetRenderDpi(const IRenderDpiPtr& spRenderDpi)
{
    m_spRenderDpi = spRenderDpi;
}

SkTextEncoding Render_Skia::GetTextEncoding() const
{
    constexpr const size_t nValueLen = sizeof(DString::value_type);
    if constexpr (nValueLen == 1) {
        return SkTextEncoding::kUTF8;
    }
    else if constexpr (nValueLen == 2) {
        return SkTextEncoding::kUTF16;
    }
    else if constexpr (nValueLen == 4) {
        return SkTextEncoding::kUTF32;
    }
    else {
#ifdef DUILIB_UNICODE
        return SkTextEncoding::kUTF16;
#else
        return SkTextEncoding::kUTF8;
#endif
    }
}

} // namespace ui
