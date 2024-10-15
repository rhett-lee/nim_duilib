#include "Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"
#include "duilib/Render/BitmapAlpha.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PerformanceUtil.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201 4100)

#include "include/core/SkMatrix.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#pragma warning (pop)

namespace ui {

static inline void DrawFunction(SkCanvas* pSkCanvas, 
                                const UiRect& rcDest,
                                const SkPoint& skPointOrg,
                                const sk_sp<SkImage>& skImage,
                                const UiRect& rcSrc,
                                const SkPaint& skPaint)
{
    if (pSkCanvas == nullptr) {
        return;
    }
    SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(skPointOrg);

    SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
    SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

    pSkCanvas->drawImageRect(skImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
}

Render_Skia::Render_Skia():
    m_saveCount(0)
{
    m_pSkPointOrg = new SkPoint;
    m_pSkPointOrg->iset(0, 0);
    m_pSkPaint = new SkPaint;
    m_pSkPaint->setAntiAlias(true);
    m_pSkPaint->setDither(true);
}

Render_Skia::~Render_Skia()
{
    if (m_pSkPaint) {
        delete m_pSkPaint;
        m_pSkPaint = nullptr;
    }
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
        uint32_t nARGB = uiColor.GetARGB();
        if (nARGB == 0) {
            ::memset(pPixelBits, uiColor.GetARGB(), GetWidth() * GetHeight() * sizeof(uint32_t));
        }
        else {
            constexpr const int32_t nLeft = 0;
            constexpr const int32_t nTop = 0;
            const int32_t nRight = std::max(GetWidth(), 0);
            const int32_t nBottom = std::max(GetHeight(), 0);
            const int32_t nWidth = nRight - nLeft;
            for (int32_t i = nTop; i < nBottom; i++) {
                for (int32_t j = nLeft; j < nRight; j++) {
                    uint32_t* color = (uint32_t*)pPixelBits + (i * nWidth + j);
                    *color = nARGB;
                }
            }
        }
    }
}

void Render_Skia::ClearRect(const UiRect& rcDirty, const UiColor& uiColor)
{
    void* pPixelBits = GetPixelBits();
    if (pPixelBits != nullptr) {
        uint32_t nARGB = uiColor.GetARGB();
        const int32_t nLeft = std::max((int32_t)rcDirty.left, 0);
        const int32_t nTop = std::max((int32_t)rcDirty.top, 0);
        const int32_t nRight = std::min((int32_t)rcDirty.right, (int32_t)GetWidth());
        const int32_t nBottom = std::min((int32_t)rcDirty.bottom, (int32_t)GetHeight());
        const int32_t nWidth = nRight - nLeft;
        for (int32_t i = nTop; i < nBottom; i++) {
            for (int32_t j = nLeft; j < nRight; j++) {
                uint32_t* color = (uint32_t*)pPixelBits + (i * nWidth + j);
                *color = nARGB;
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
    if (!pBitmap->Init(nWidth, nHeight, true, pPixelBits)) {
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

void Render_Skia::SetClip(const UiRect& rc, bool bIntersect)
{
    SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSk = SkRect::Make(rcSkI);
    rcSk.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->save();
        if (bIntersect) {
            skCanvas->clipRect(rcSk, SkClipOp::kIntersect, true);
        }
        else {
            skCanvas->clipRect(rcSk, SkClipOp::kDifference, true);
        }
    }
}

void Render_Skia::SetRoundClip(const UiRect& rc, int32_t width, int32_t height, bool bIntersect)
{
    SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSk = SkRect::Make(rcSkI);

    SkPath skPath;
    skPath.addRoundRect(rcSk, SkScalar(width), SkScalar(height));
    SkRegion clip;
    clip.setRect(rcSkI);
    SkRegion rgn;
    rgn.setPath(skPath, clip);
    rgn.translate((int)m_pSkPointOrg->fX, (int)m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->save();
        if (bIntersect) {
            skCanvas->clipRegion(rgn, SkClipOp::kIntersect);
        }
        else {
            skCanvas->clipRegion(rgn, SkClipOp::kDifference);
        }
    }
}

void Render_Skia::ClearClip()
{
    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->restore();
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
    sk_sp<SkImage> skSrcImage = skSurface->makeImageSnapshot();
    ASSERT(skSrcImage != nullptr);
    if (skSrcImage == nullptr) {
        return false;
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    if (alpha != 0xFF) {
        skPaint.setAlpha(alpha);
    }

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

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                            const UiRect& rcDest, const UiRect& rcDestCorners,
                            const UiRect& rcSource, const UiRect& rcSourceCorners,
                            uint8_t uFade, bool xtiled, bool ytiled,
                            bool fullxtiled, bool fullytiled, int32_t nTiledMargin)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    UiRect rcTestTemp;
    if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
        return;
    }
    PerformanceStat statPerformance(_T("Render_Skia::DrawImage"));

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
    sk_sp<SkImage> skImage = skSrcBitmap.asImage();//这里是复制了一份位图数据的，有性能损耗

    UiRect rcTemp;
    UiRect rcDrawSource;
    UiRect rcDrawDest;

    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    //默认值就是kSrcOver
    skPaint.setBlendMode(SkBlendMode::kSrcOver);

    // middle
    rcDrawDest.left = rcDest.left + rcDestCorners.left;
    rcDrawDest.top = rcDest.top + rcDestCorners.top;
    rcDrawDest.right = rcDest.right - rcDestCorners.right;
    rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

    rcDrawSource.left = rcSource.left + rcSourceCorners.left;
    rcDrawSource.top = rcSource.top + rcSourceCorners.top;
    rcDrawSource.right = rcSource.right - rcSourceCorners.right;
    rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;
    if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
        if (!xtiled && !ytiled) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
        else if (xtiled && ytiled) {
            const int32_t imageDrawWidth = rcSource.right - rcSource.left - rcSourceCorners.left - rcSourceCorners.right;
            const int32_t imageDrawHeight = rcSource.bottom - rcSource.top - rcSourceCorners.top - rcSourceCorners.bottom;
            int iTimesX = (rcDrawDest.right - rcDrawDest.left) / (imageDrawWidth + nTiledMargin);
            if (!fullxtiled) {
                if ((rcDrawDest.right - rcDrawDest.left) % (imageDrawWidth + nTiledMargin) > 0) {
                    iTimesX += 1;
                }
            }
            int iTimesY = (rcDrawDest.bottom - rcDrawDest.top) / (imageDrawHeight + nTiledMargin);
            if (!fullytiled) {
                if ((rcDrawDest.bottom - rcDrawDest.top) % (imageDrawHeight + nTiledMargin) > 0) {
                    iTimesY += 1;
                }
            }
            for (int j = 0; j < iTimesY; ++j) {
                int32_t lDestTop = rcDrawDest.top + j * imageDrawHeight + j * nTiledMargin;
                int32_t lDestBottom = lDestTop + imageDrawHeight;
                int32_t lDrawHeight = imageDrawHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= lDestBottom - rcDrawDest.bottom;
                    lDestBottom = rcDrawDest.bottom;
                }
                for (int i = 0; i < iTimesX; ++i) {
                    int32_t lDestLeft = rcDrawDest.left + i * imageDrawWidth + i * nTiledMargin;
                    int32_t lDestRight = lDestLeft + imageDrawWidth;
                    int32_t lDrawWidth = imageDrawWidth;
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
                    DrawFunction(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
                }
            }
        }
        else if (xtiled) { // supp
            const int32_t imageDrawWidth = rcSource.right - rcSource.left - rcSourceCorners.left - rcSourceCorners.right;
            int iTimes = (rcDrawDest.right - rcDrawDest.left) / (imageDrawWidth + nTiledMargin);
            if (!fullxtiled) {
                if ((rcDrawDest.right - rcDrawDest.left) % (imageDrawWidth + nTiledMargin) > 0) {
                    iTimes += 1;
                }
            }

            for (int i = 0; i < iTimes; ++i) {
                int32_t lDestLeft = rcDrawDest.left + i * imageDrawWidth + i * nTiledMargin;
                int32_t lDestRight = lDestLeft + imageDrawWidth;
                int32_t lDrawWidth = imageDrawWidth;
                if (lDestRight > rcDrawDest.right) {
                    lDrawWidth -= (lDestRight - rcDrawDest.right);
                    lDestRight = rcDrawDest.right;
                }

                //源区域：如果设置了边角，则仅包含中间区域
                rcDrawSource.left = rcSource.left + rcSourceCorners.left;
                rcDrawSource.top = rcSource.top + rcSourceCorners.top;
                rcDrawSource.right = rcDrawSource.left + lDrawWidth;
                rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;

                UiRect rcDestTemp = rcDrawDest;
                rcDestTemp.top = rcDrawDest.top;
                rcDestTemp.bottom = rcDrawDest.top + rcDrawSource.Height();
                rcDestTemp.left = lDestLeft;
                rcDestTemp.right = lDestRight;

                DrawFunction(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
            }
        }
        else { // ytiled
            const int32_t imageDrawHeight = rcSource.bottom - rcSource.top - rcSourceCorners.top - rcSourceCorners.bottom;
            int iTimes = (rcDrawDest.bottom - rcDrawDest.top) / (imageDrawHeight + nTiledMargin);
            if (!fullytiled) {
                if ((rcDrawDest.bottom - rcDrawDest.top) % (imageDrawHeight + nTiledMargin) > 0) {
                    iTimes += 1;
                }
            }

            UiRect rcDestTemp;
            rcDestTemp.left = rcDrawDest.left;

            for (int i = 0; i < iTimes; ++i) {
                int32_t lDestTop = rcDrawDest.top + i * imageDrawHeight + i * nTiledMargin;
                int32_t lDestBottom = lDestTop + imageDrawHeight;
                int32_t lDrawHeight = imageDrawHeight;
                if (lDestBottom > rcDrawDest.bottom) {
                    lDrawHeight -= lDestBottom - rcDrawDest.bottom;
                    lDestBottom = rcDrawDest.bottom;
                }

                rcDrawSource.left = rcSource.left + rcSourceCorners.left;
                rcDrawSource.top = rcSource.top + rcSourceCorners.top;
                rcDrawSource.right = rcSource.right - rcSourceCorners.right;
                rcDrawSource.bottom = rcDrawSource.top + lDrawHeight;

                rcDestTemp.right = rcDrawDest.left + rcDrawSource.Width();
                rcDestTemp.top = lDestTop;
                rcDestTemp.bottom = lDestBottom;

                DrawFunction(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
            }
        }
    }

    // left-top
    if (rcSourceCorners.left > 0 && rcSourceCorners.top > 0) {
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // top
    if (rcSourceCorners.top > 0) {
        rcDrawDest.left = rcDest.left + rcDestCorners.left;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.right - rcDestCorners.right;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.left + rcSourceCorners.left;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.right - rcSourceCorners.right;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right-top
    if (rcSourceCorners.right > 0 && rcSourceCorners.top > 0) {
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // left
    if (rcSourceCorners.left > 0) {
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.top + rcDestCorners.top;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.top + rcSourceCorners.top;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right
    if (rcSourceCorners.right > 0) {
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.top + rcDestCorners.top;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.top + rcSourceCorners.top;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // left-bottom
    if (rcSourceCorners.left > 0 && rcSourceCorners.bottom > 0) {
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // bottom
    if (rcSourceCorners.bottom > 0) {
        rcDrawDest.left = rcDest.left + rcDestCorners.left;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.right - rcDestCorners.right;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.left + rcSourceCorners.left;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.right - rcSourceCorners.right;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right-bottom
    if (rcSourceCorners.right > 0 && rcSourceCorners.bottom > 0) {
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawFunction(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                            const UiRect& rcDest, const UiRect& rcSource,
                            uint8_t uFade, bool xtiled, bool ytiled,
                            bool fullxtiled, bool fullytiled, int32_t nTiledMargin)
{
    UiRect rcDestCorners;
    UiRect rcSourceCorners;
    return DrawImage(rcPaint, pBitmap, 
                     rcDest, rcDestCorners,
                     rcSource, rcSourceCorners,
                     uFade, xtiled, ytiled,
                     fullxtiled, fullytiled, nTiledMargin);
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
    sk_sp<SkImage> skImage = skSrcBitmap.asImage();//这里是复制了一份位图数据的，有性能损耗

    bool isMatrixSet = false;
    if (pMatrix != nullptr) {
        Matrix_Skia* pSkMatrix = dynamic_cast<Matrix_Skia*>(pMatrix);
        if (pSkMatrix != nullptr) {
            SkM44 skM44(*pSkMatrix->GetMatrix());
            skCanvas->setMatrix(skM44);
            isMatrixSet = true;
        }
    }
    DrawFunction(skCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
    if (isMatrixSet) {
        skCanvas->resetMatrix();
    }
}

void Render_Skia::FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::InitGradientColor(SkPaint& skPaint, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const
{
    if ((nColor2Direction != 2) && (nColor2Direction != 3) && (nColor2Direction != 4)) {
        nColor2Direction = 1;
    }
    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkPoint pts[2] = {0, };
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

    SkColor colors[2];
    colors[0] = SkColorSetRGB(dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    colors[1] = SkColorSetRGB(dwColor2.GetR(), dwColor2.GetG(), dwColor2.GetB());

    sk_sp<SkShader> shader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    skPaint.setShader(shader);
}

void Render_Skia::FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (dwColor2.IsEmpty()) {
        return FillRect(rc, dwColor, uFade);
    }

    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }
    skPaint.setStyle(SkPaint::kFill_Style);
    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRect(rcSkDest, skPaint);
    }
}

void Render_Skia::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setStrokeWidth(SkScalar(nWidth));

    SkPoint skPt1;
    skPt1.iset(pt1.x, pt1.y);
    skPt1.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkPoint skPt2;
    skPt2.iset(pt2.x, pt2.y);
    skPt2.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawLine(skPt1, skPt2, skPaint);
    }
}

void Render_Skia::DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

    SkPoint skPt1;
    skPt1.iset(pt1.x, pt1.y);
    skPt1.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkPoint skPt2;
    skPt2.iset(pt2.x, pt2.y);
    skPt2.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawLine(skPt1, skPt2, skPaint);
    }
}

void Render_Skia::DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool bLineInRect)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(nWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
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

void Render_Skia::DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int32_t nWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(nWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, SkIntToScalar(roundSize.cx), SkIntToScalar(roundSize.cy), skPaint);
    }
}

void Render_Skia::FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, SkIntToScalar(roundSize.cx), SkIntToScalar(roundSize.cy), skPaint);
    }
}

void Render_Skia::FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (dwColor2.IsEmpty()) {
        return FillRoundRect(rc, roundSize, dwColor, uFade);
    }

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawRoundRect(rcSkDest, SkIntToScalar(roundSize.cx), SkIntToScalar(roundSize.cy), skPaint);
    }
}

void Render_Skia::DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int nWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(nWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkPoint rcSkPoint = SkPoint::Make(SkIntToScalar(centerPt.x), SkIntToScalar(centerPt.y));
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, SkIntToScalar(radius), skPaint);
    }
}

void Render_Skia::FillCircle(const UiPoint& centerPt, int32_t radius, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(dwColor.GetA(), dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    skPaint.setStyle(SkPaint::kFill_Style);
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    SkPoint rcSkPoint = SkPoint::Make(SkIntToScalar(centerPt.x), SkIntToScalar(centerPt.y));
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, SkIntToScalar(radius), skPaint);
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
        SkColor colors[2] = {0, };
        colors[0] = pen->GetColor().GetARGB();
        colors[1] = gradientColor->GetARGB();
        
        SkIRect rcGradientDestI = { gradientRect->left, gradientRect->top, gradientRect->right, gradientRect->bottom };
        SkRect rcGradientDest = SkRect::Make(rcGradientDestI);
        rcGradientDest.offset(*m_pSkPointOrg);

        SkPoint pts[2] = {0, };
        pts[0].set(rcGradientDest.fLeft, rcGradientDest.fTop);
        pts[1].set(rcGradientDest.fRight, rcGradientDest.fBottom);
        
        sk_sp<SkShader> shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
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

    SkPath skPath;
    pSkiaPath->GetSkPath()->offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPath);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, paint);
    }
}

int32_t Render_Skia::GetScaleInt(int32_t iValue) const
{
    if (m_spRenderDpi != nullptr) {
        return m_spRenderDpi->GetScaleInt(iValue);
    }
    return iValue;
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
    int32_t nRatio = pen->GetWidth() / GetScaleInt(1);
    switch (dashStyle) {
    case IPen::kDashStyleSolid:
    {
        SkScalar intervals[] = { 1.0f, 0.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDash:
    {
        int32_t nValue = GetScaleInt(5) * nRatio;
        SkScalar intervals[] = { nValue * 1.0f, nValue * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDot:
    {
        int32_t nValue1 = GetScaleInt(1) * nRatio;
        int32_t nValue4 = GetScaleInt(4) * nRatio;
        SkScalar intervals[] = { nValue1 * 1.0f, nValue4 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDashDot:
    {
        int32_t nValue1 = GetScaleInt(1) * nRatio;
        int32_t nValue4 = GetScaleInt(4) * nRatio;
        SkScalar intervals[] = { nValue4 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 4, 0.0f);
        break;
    }
    case IPen::kDashStyleDashDotDot:
    {
        int32_t nValue1 = GetScaleInt(1) * nRatio;
        int32_t nValue4 = GetScaleInt(4) * nRatio;
        SkScalar intervals[] = { nValue4 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 6, 0.0f);
        break;
    }
    default:
    {
        SkScalar intervals[] = { 1.0f, 0.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
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

    SkPath skPath;
    pSkiaPath->GetSkPath()->offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPath);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, paint);
    }
}

void Render_Skia::FillPath(const IPath* path, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction)
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

    SkPaint skPaint = *m_pSkPaint;
    skPaint.setStyle(SkPaint::kFill_Style);

    InitGradientColor(skPaint, rc, dwColor, dwColor2, nColor2Direction);
    
    SkPath skPath;
    pSkiaPath->GetSkPath()->offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPath);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawPath(skPath, skPaint);
    }
}

void Render_Skia::DrawString(const UiRect& rc, 
                             const DString& strText,
                             UiColor dwTextColor, 
                             IFont* pFont, 
                             uint32_t uFormat, 
                             uint8_t uFade /*= 255*/)
{
    PerformanceStat statPerformance(_T("Render_Skia::DrawString"));
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
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
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
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
    skPaint.setARGB(dwTextColor.GetA(), dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB());
    if (uFade != 0xFF) {
        skPaint.setAlpha(uFade);
    }

    //绘制区域
    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    //设置绘制属性
    SkTextBox skTextBox;
    skTextBox.setBox(rcSkDest);
    if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        //单行文本
        skTextBox.setLineMode(SkTextBox::kOneLine_Mode);
    }

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
        bEndEllipsis = true;
    }
    skTextBox.setEndEllipsis(bEndEllipsis);

    bool bPathEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_PATH_ELLIPSIS) {
        bPathEllipsis = true;
    }
    skTextBox.setPathEllipsis(bPathEllipsis);

    //绘制文字时，不使用裁剪区域（可能会导致文字绘制超出边界）
    if (uFormat & DrawStringFormat::TEXT_NOCLIP) {
        skTextBox.setClipBox(false);
    }
    //删除线
    skTextBox.setStrikeOut(pFont->IsStrikeOut());
    //下划线
    skTextBox.setUnderline(pFont->IsUnderline());

    if (uFormat & DrawStringFormat::TEXT_CENTER) {
        //横向对齐：居中对齐
        skTextBox.setTextAlign(SkTextBox::kCenter_Align);
    }
    else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
        //横向对齐：右对齐
        skTextBox.setTextAlign(SkTextBox::kRight_Align);
    }
    else {
        //横向对齐：左对齐
        skTextBox.setTextAlign(SkTextBox::kLeft_Align);
    }

    if (uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        skTextBox.setSpacingAlign(SkTextBox::kCenter_SpacingAlign);
    }
    else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：下对齐
        skTextBox.setSpacingAlign(SkTextBox::kEnd_SpacingAlign);
    }
    else {
        //纵向对齐：上对齐
        skTextBox.setSpacingAlign(SkTextBox::kStart_SpacingAlign);
    }
    skTextBox.draw(skCanvas, 
                   (const char*)strText.c_str(), 
                   strText.size() * sizeof(DString::value_type),
                   textEncoding, 
                   *pSkFont,
                   skPaint);
}

UiRect Render_Skia::MeasureString(const DString& strText, 
                                  IFont* pFont, 
                                  uint32_t uFormat, 
                                  int width /*= DUI_NOSET_VALUE*/)
{
    PerformanceStat statPerformance(_T("Render_Skia::MeasureString"));
    if ((GetWidth() <= 0) || (GetHeight() <= 0)) {
        //这种情况是窗口大小为0的情况，返回空，不加断言
        return UiRect();
    }
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return UiRect();
    }
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return UiRect();
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return UiRect();
    }

    //获取字体接口
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
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

    bool isSingleLineMode = false;
    if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        isSingleLineMode = true;
    }
        
    //计算行高
    SkFontMetrics fontMetrics;
    SkScalar fontHeight = pSkFont->getMetrics(&fontMetrics);

    if (isSingleLineMode || (width <= 0)) {
        //单行模式, 或者没有限制宽度
        SkScalar textWidth = pSkFont->measureText(strText.c_str(),
                                                  strText.size() * sizeof(DString::value_type),
                                                  GetTextEncoding(),
                                                  nullptr,
                                                  &skPaint);
        int textIWidth = SkScalarTruncToInt(textWidth + 0.5f);
        if (textWidth > textIWidth) {
            textIWidth += 1;
        }
        if (textIWidth <= 0) {
            return UiRect();
        }
        UiRect rc;
        rc.left = 0;
        if (width <= 0) {
            rc.right = textIWidth;
        }
        else if (textIWidth < width) {
            rc.right = textIWidth;
        }
        else {
            //返回限制宽度
            rc.right = width;
        }
        rc.top = 0;
        rc.bottom = SkScalarTruncToInt(fontHeight + 0.5f);
        if (fontHeight > rc.bottom) {
            rc.bottom += 1;
        }
        return rc;
    }
    else {
        //多行模式，并且限制宽度width为有效值
        ASSERT(width > 0);
        int lineCount = SkTextLineBreaker::CountLines((const char*)strText.c_str(),
                                                      strText.size() * sizeof(DString::value_type),
                                                      GetTextEncoding(),
                                                      *pSkFont,
                                                      skPaint,
                                                      SkScalar(width),
                                                      SkTextBox::kWordBreak_Mode);

        float spacingMul = 1.0f;//行间距倍数，暂不支持设置
        SkScalar scaledSpacing = fontHeight * spacingMul;
        SkScalar textHeight = fontHeight;
        if (lineCount > 0) {
            textHeight += scaledSpacing * (lineCount - 1);
        }
        UiRect rc;
        rc.left = 0;
        rc.right = width;
        rc.top = 0;
        rc.bottom = SkScalarTruncToInt(textHeight + 0.5f);
        if (textHeight > rc.bottom) {
            rc.bottom += 1;
        }
        return rc;
    }
}

void Render_Skia::DrawRichText(const UiRect& rc,
                               IRenderFactory* pRenderFactory, 
                               std::vector<RichTextData>& richTextData,
                               uint32_t uFormat,
                               bool bMeasureOnly,
                               uint8_t uFade)
{
    PerformanceStat statPerformance(_T("Render_Skia::DrawRichText"));
    if (rc.IsEmpty()) {
        return;
    }
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    //待绘制的文本
    struct TPendingTextData
    {
        //在richTextData中的索引号
        size_t m_dataIndex = 0;

        //行号
        size_t m_nRows = 0;

        //待绘制文本
        DString m_text;

        //绘制目标区域
        UiRect m_destRect;

        //Paint对象
        SkPaint m_skPaint;

        //Font对象
        std::shared_ptr<IFont> m_spFont;

        //背景颜色
        UiColor m_bgColor;
    };

    std::vector<std::shared_ptr<TPendingTextData>> pendingTextData;

    int32_t xPos = rc.left;
    int32_t yPos = rc.top;
    int32_t nRowHeight = 0;
    size_t nRowIndex = 0;
    for (size_t index = 0; index < richTextData.size(); ++index) {
        const RichTextData& textData = richTextData[index];
        if (textData.m_text.empty()) {
            continue;
        }

        //文本颜色
        SkPaint skPaint = *m_pSkPaint;
        if (uFade != 0xFF) {
            //透明度
            skPaint.setAlpha(uFade);
        }
        const UiColor& color = textData.m_textColor;
        skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
        
        std::shared_ptr<IFont> spSkiaFont(pRenderFactory->CreateIFont());
        ASSERT(spSkiaFont != nullptr);
        if (spSkiaFont == nullptr) {
            continue;
        }
        if (!spSkiaFont->InitFont(textData.m_fontInfo)) {
            spSkiaFont.reset();
            continue;
        }
        Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(spSkiaFont.get());
        ASSERT(pSkiaFont != nullptr);
        if (pSkiaFont == nullptr) {
            continue;
        }

        const SkFont* pSkFont = pSkiaFont->GetFontHandle();
        ASSERT(pSkFont != nullptr);
        if (pSkFont == nullptr) {
            continue;
        }

        SkFont skFont = *pSkFont;
        SkFontMetrics metrics;
        SkScalar fFontHeight = skFont.getMetrics(&metrics); //字体高度，换行时使用
        fFontHeight *= textData.m_fRowSpacingMul;
        int32_t nFontHeight = SkScalarTruncToInt(fFontHeight);
        if ((fFontHeight - nFontHeight) > 0.01f) {
            nFontHeight += 1;
        }
        nRowHeight = std::max(nRowHeight, nFontHeight);
        if (nRowHeight <= 0) {
            continue;
        }

        DString textValue = textData.m_text.c_str();
        //统一换行标志
        StringUtil::ReplaceAll(_T("\r\n"), _T("\n"), textValue);
        StringUtil::ReplaceAll(_T("\r"), _T("\n"), textValue);
        if (textValue == _T("\n")) {
            //文本内容是分隔符，执行换行操作
            xPos = rc.left;
            yPos += nRowHeight;
            nRowHeight = 0;
            ++nRowIndex;

            if (yPos >= rc.bottom) {
                //绘制区域已满，终止绘制
                break;
            }
            continue;
        }

        bool bBreakAll = false;//标记是否终止

        //按换行符进行文本切分
        std::list<DString> textList = StringUtil::Split(textValue, _T("\n"));
        for (const DString& text : textList) {
            //绘制的文本下标开始值        
            const size_t textCount = text.size();
            size_t textStartIndex = 0;
            while (textStartIndex < textCount) {
                //估算文本绘制区域
                SkScalar maxWidth = SkIntToScalar(rc.right - xPos);//可用宽度
                ASSERT(maxWidth > 0);
                SkScalar measuredWidth = 0;
                size_t byteLength = (textCount - textStartIndex) * sizeof(DString::value_type);
                size_t nDrawLength = SkTextBox::breakText(text.c_str() + textStartIndex,
                                                          byteLength,
                                                          GetTextEncoding(),
                                                          skFont, skPaint,
                                                          maxWidth, &measuredWidth);

                //绘制文字所需宽度
                int32_t nTextWidth = SkScalarTruncToInt(measuredWidth);
                if ((measuredWidth - nTextWidth) > 0.01f) {
                    nTextWidth += 1;
                }

                std::shared_ptr<TPendingTextData> spTextData = std::make_shared<TPendingTextData>();
                spTextData->m_dataIndex = index;
                spTextData->m_nRows = nRowIndex;
                if (nDrawLength > 0) {
                    spTextData->m_text = text.substr(textStartIndex, nDrawLength / sizeof(DString::value_type));
                }
                spTextData->m_skPaint = skPaint;
                spTextData->m_spFont = spSkiaFont;
                spTextData->m_bgColor = textData.m_bgColor;

                //绘制文字所需的矩形区域
                spTextData->m_destRect.left = xPos;
                spTextData->m_destRect.right = xPos + nTextWidth;
                spTextData->m_destRect.top = yPos;
                spTextData->m_destRect.bottom = yPos + nRowHeight;

                bool bNextRow = false; //是否需要换行的标志
                if (nDrawLength < byteLength) {
                    //宽度不足，需要换行
                    bNextRow = true;
                    textStartIndex += nDrawLength / sizeof(DString::value_type);
                }
                else {
                    //当前行可容纳文本绘制
                    textStartIndex = textCount;//标记，结束循环

                    xPos += nTextWidth;
                    if (xPos >= rc.right) {
                        //换行
                        bNextRow = true;
                    }
                }

                if (nDrawLength > 0) {
                    pendingTextData.push_back(spTextData);
                }

                if (bNextRow) {
                    //执行换行操作
                    xPos = rc.left;
                    yPos += nRowHeight;
                    nRowHeight = nFontHeight;
                    ++nRowIndex;

                    if (yPos >= rc.bottom) {
                        //绘制区域已满，终止绘制
                        bBreakAll = true;
                        break;
                    }
                }
            }
            if (bBreakAll) {
                break;
            }
        }
        if (bBreakAll) {
            break;
        }
    }

    for (RichTextData& textData : richTextData) {
        textData.m_textRects.clear();
    }
    for (const std::shared_ptr<TPendingTextData>& spTextData : pendingTextData) {
        const TPendingTextData& textData = *spTextData;
        ASSERT(textData.m_dataIndex < richTextData.size());
        RichTextData& richText = richTextData[textData.m_dataIndex];
        richText.m_textRects.push_back(textData.m_destRect); //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）

        if (!bMeasureOnly) {
            //绘制文字的背景色
            FillRect(textData.m_destRect, textData.m_bgColor, uFade);

            //绘制文字
            DrawTextString(textData.m_destRect, textData.m_text, uFormat | DrawStringFormat::TEXT_SINGLELINE, textData.m_skPaint, textData.m_spFont.get());
        }
    }
}

void Render_Skia::DrawTextString(const UiRect& rc, const DString& strText, uint32_t uFormat,
                                 const SkPaint& skPaint, IFont* pFont) const
{
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas == nullptr) {
        return;
    }
    Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
    ASSERT(pSkiaFont != nullptr);
    if (pSkiaFont == nullptr) {
        return;
    }
    //文本编码
    SkTextEncoding textEncoding = GetTextEncoding();
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    //绘制区域
    SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(*m_pSkPointOrg);

    //设置绘制属性
    SkTextBox skTextBox;
    skTextBox.setBox(rcSkDest);
    if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
        //单行文本
        skTextBox.setLineMode(SkTextBox::kOneLine_Mode);
    }

    //绘制区域不足时，自动在末尾绘制省略号
    bool bEndEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
        bEndEllipsis = true;
    }
    skTextBox.setEndEllipsis(bEndEllipsis);

    bool bPathEllipsis = false;
    if (uFormat & DrawStringFormat::TEXT_PATH_ELLIPSIS) {
        bPathEllipsis = true;
    }
    skTextBox.setPathEllipsis(bPathEllipsis);

    //绘制文字时，不使用裁剪区域（可能会导致文字绘制超出边界）
    if (uFormat & DrawStringFormat::TEXT_NOCLIP) {
        skTextBox.setClipBox(false);
    }
    //删除线
    skTextBox.setStrikeOut(pSkiaFont->IsStrikeOut());
    //下划线
    skTextBox.setUnderline(pSkiaFont->IsUnderline());

    if (uFormat & DrawStringFormat::TEXT_CENTER) {
        //横向对齐：居中对齐
        skTextBox.setTextAlign(SkTextBox::kCenter_Align);
    }
    else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
        //横向对齐：右对齐
        skTextBox.setTextAlign(SkTextBox::kRight_Align);
    }
    else {
        //横向对齐：左对齐
        skTextBox.setTextAlign(SkTextBox::kLeft_Align);
    }

    if (uFormat & DrawStringFormat::TEXT_VCENTER) {
        //纵向对齐：居中对齐
        skTextBox.setSpacingAlign(SkTextBox::kCenter_SpacingAlign);
    }
    else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
        //纵向对齐：下对齐
        skTextBox.setSpacingAlign(SkTextBox::kEnd_SpacingAlign);
    }
    else {
        //纵向对齐：上对齐
        skTextBox.setSpacingAlign(SkTextBox::kStart_SpacingAlign);
    }
    skTextBox.draw(skCanvas,
                   (const char*)strText.c_str(),
                   strText.size() * sizeof(DString::value_type),
                   textEncoding,
                   *pSkFont,
                   skPaint);
}

void Render_Skia::DrawBoxShadow(const UiRect& rc,
                                const UiSize& roundSize, 
                                const UiPoint& cpOffset, 
                                int32_t nBlurRadius, 
                                int32_t nSpreadRadius,
                                UiColor dwColor)
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

    SkPath shadowPath;
    shadowPath.addRoundRect(srcRc, (SkScalar)roundSize.cx, (SkScalar)roundSize.cy);

    SkPath excludePath;    
    excludePath.addRoundRect(excludeRc, (SkScalar)roundSize.cx, (SkScalar)roundSize.cy);

    SkPaint paint = *m_pSkPaint;
    paint.setColor(dwColor.GetARGB());
    paint.setStyle(SkPaint::kFill_Style);

    SkAutoCanvasRestore autoCanvasRestore(skCanvas, true);

    //裁剪中间区域
    SkPath skPathExclude;
    excludePath.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPathExclude);
    skCanvas->clipPath(skPathExclude, SkClipOp::kDifference);

    SkPath skPath;
    shadowPath.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPath);

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
    skPath.transform(mat);

    skCanvas->drawPath(skPath, paint);
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
    ASSERT(srcPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (srcPixelsLen < (rc.Width() * rc.Height() * sizeof(uint32_t))) {
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
    ASSERT(srcPixelsLen >= (rc.Width() * rc.Height() * sizeof(uint32_t)));
    if (srcPixelsLen < (rc.Width() * rc.Height() * sizeof(uint32_t))) {
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
#ifdef DUILIB_UNICODE
    return SkTextEncoding::kUTF16;
#else
    return SkTextEncoding::kUTF8;
#endif
}

} // namespace ui
