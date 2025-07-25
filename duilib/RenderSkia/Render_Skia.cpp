#include "Render_Skia.h"
#include "SkUtils.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"
#include "duilib/RenderSkia/DrawSkiaImage.h"
#include "duilib/Render/BitmapAlpha.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/PerformanceUtil.h"
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
#include "include/core/SkRegion.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"

#include "SkiaHeaderEnd.h"

#include <unordered_set>
#include <unordered_map>

namespace ui {

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
    if (m_pSkPointOrg) {
        delete m_pSkPointOrg;
        m_pSkPointOrg = nullptr;
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

void Render_Skia::SetRoundClip(const UiRect& rc, float rx, float ry, bool bIntersect)
{
    SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
    SkRect rcSk = SkRect::Make(rcSkI);

    SkPath skPath;
    skPath.addRoundRect(rcSk, rx, ry);
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
                            bool fullxtiled, bool fullytiled, int32_t nTiledMargin,
                            bool bWindowShadowMode)
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
    if (!bWindowShadowMode && UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
        //绘制中间部分
        if (!xtiled && !ytiled) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
                    DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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

                DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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

                DrawSkiaImage::DrawImage(skCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
            }
        }
    }

    // left-top
    if (rcSourceCorners.left > 0 && rcSourceCorners.top > 0) {
        //左上角
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // top
    if (rcSourceCorners.top > 0) {
        //边框：上
        rcDrawDest.left = rcDest.left + rcDestCorners.left;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.right - rcDestCorners.right;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.left + rcSourceCorners.left;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.right - rcSourceCorners.right;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right-top
    if (rcSourceCorners.right > 0 && rcSourceCorners.top > 0) {
        //右上角
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.top;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.top + rcDestCorners.top;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.top;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.top + rcSourceCorners.top;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // left
    if (rcSourceCorners.left > 0) {
        //边框：左
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.top + rcDestCorners.top;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.top + rcSourceCorners.top;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right
    if (rcSourceCorners.right > 0) {
        //边框：右
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.top + rcDestCorners.top;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.bottom - rcDestCorners.bottom;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.top + rcSourceCorners.top;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.bottom - rcSourceCorners.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // left-bottom
    if (rcSourceCorners.left > 0 && rcSourceCorners.bottom > 0) {
        //左下角
        rcDrawDest.left = rcDest.left;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.left + rcDestCorners.left;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.left;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.left + rcSourceCorners.left;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // bottom
    if (rcSourceCorners.bottom > 0) {
        //边框：下
        rcDrawDest.left = rcDest.left + rcDestCorners.left;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.right - rcDestCorners.right;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.left + rcSourceCorners.left;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.right - rcSourceCorners.right;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
    // right-bottom
    if (rcSourceCorners.right > 0 && rcSourceCorners.bottom > 0) {
        //右下角
        rcDrawDest.left = rcDest.right - rcDestCorners.right;
        rcDrawDest.top = rcDest.bottom - rcDestCorners.bottom;
        rcDrawDest.right = rcDest.right;
        rcDrawDest.bottom = rcDest.bottom;

        rcDrawSource.left = rcSource.right - rcSourceCorners.right;
        rcDrawSource.top = rcSource.bottom - rcSourceCorners.bottom;
        rcDrawSource.right = rcSource.right;
        rcDrawSource.bottom = rcSource.bottom;
        if (UiRect::Intersect(rcTemp, rcPaint, rcDrawDest)) {
            DrawSkiaImage::DrawImage(skCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
        }
    }
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
                            const UiRect& rcDest, const UiRect& rcSource,
                            uint8_t uFade, bool xtiled, bool ytiled,
                            bool fullxtiled, bool fullytiled, int32_t nTiledMargin,
                            bool bWindowShadowMode)
{
    UiRect rcDestCorners;
    UiRect rcSourceCorners;
    return DrawImage(rcPaint, pBitmap, 
                     rcDest, rcDestCorners,
                     rcSource, rcSourceCorners,
                     uFade, xtiled, ytiled,
                     fullxtiled, fullytiled, nTiledMargin, bWindowShadowMode);
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

void Render_Skia::FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    FillRect(rcF, dwColor, uFade);
}

void Render_Skia::FillRect(const UiRectF& rc, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::InitGradientColor(SkPaint& skPaint, const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction) const
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    InitGradientColor(skPaint, rcF, dwColor, dwColor2, nColor2Direction);
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

    SkColor colors[2];
    colors[0] = SkColorSetRGB(dwColor.GetR(), dwColor.GetG(), dwColor.GetB());
    colors[1] = SkColorSetRGB(dwColor2.GetR(), dwColor2.GetG(), dwColor2.GetB());

    sk_sp<SkShader> shader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    skPaint.setShader(shader);
}

void Render_Skia::FillRect(const UiRect& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    FillRect(rcF, dwColor, dwColor2, nColor2Direction, uFade);
}

void Render_Skia::FillRect(const UiRectF& rc, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
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
    SkRect rcSkDest = SkRect::MakeLTRB(rc.left, rc.top, rc.right, rc.bottom);
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
    DrawLine(pt1, pt2, penColor, (float)nWidth);
}

void Render_Skia::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStyle(SkPaint::kStroke_Style);
    skPaint.setStrokeWidth(SkScalar(fWidth));

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
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRect(rcF, penColor, nWidth, bLineInRect);
}

void Render_Skia::DrawRect(const UiRectF& rc, UiColor penColor, int32_t nWidth, bool bLineInRect)
{
    DrawRect(rc, penColor, (float)nWidth, bLineInRect);
}

void Render_Skia::DrawRect(const UiRect& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRect(rcF, penColor, fWidth, bLineInRect);
}

void Render_Skia::DrawRect(const UiRectF& rc, UiColor penColor, float fWidth, bool bLineInRect)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::DrawRect(const UiRect& rc, IPen* pen, bool bLineInRect)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRect(rcF, pen, bLineInRect);
}

void Render_Skia::DrawRect(const UiRectF& rc, IPen* pen, bool bLineInRect)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, int32_t nWidth)
{
    DrawRoundRect(rc, rx, ry, penColor, (float)nWidth);
}

void Render_Skia::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, int32_t nWidth)
{
    DrawRoundRect(rc, rx, ry, penColor, (float)nWidth);
}

void Render_Skia::DrawRoundRect(const UiRect& rc, float rx, float ry, UiColor penColor, float fWidth)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRoundRect(rcF, rx, ry, penColor, fWidth);
}

void Render_Skia::DrawRoundRect(const UiRectF& rc, float rx, float ry, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::DrawRoundRect(const UiRect& rc, float rx, float ry, IPen* pen)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    DrawRoundRect(rcF, rx, ry, pen);
}

void Render_Skia::DrawRoundRect(const UiRectF& rc, float rx, float ry, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, uint8_t uFade)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    FillRoundRect(rcF, rx, ry, dwColor, uFade);
}

void Render_Skia::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
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

void Render_Skia::FillRoundRect(const UiRect& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    UiRectF rcF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom);
    FillRoundRect(rcF, rx, ry, dwColor, dwColor2, nColor2Direction, uFade);
}

void Render_Skia::FillRoundRect(const UiRectF& rc, float rx, float ry, UiColor dwColor, UiColor dwColor2, int8_t nColor2Direction, uint8_t uFade)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    if (dwColor2.IsEmpty()) {
        return FillRoundRect(rc, rx, ry, dwColor, uFade);
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

void Render_Skia::DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, int32_t nWidth)
{
    DrawCircle(centerPt, radius, penColor, (float)nWidth);
}

void Render_Skia::DrawCircle(const UiPoint& centerPt, int32_t radius, UiColor penColor, float fWidth)
{
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
    skPaint.setStrokeWidth(SkIntToScalar(fWidth));
    skPaint.setStyle(SkPaint::kStroke_Style);

    SkPoint rcSkPoint = SkPoint::Make(SkIntToScalar(centerPt.x), SkIntToScalar(centerPt.y));
    rcSkPoint.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY);

    SkCanvas* skCanvas = GetSkCanvas();
    ASSERT(skCanvas != nullptr);
    if (skCanvas != nullptr) {
        skCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, SkIntToScalar(radius), skPaint);
    }
}

void Render_Skia::DrawCircle(const UiPoint& centerPt, int32_t radius, IPen* pen)
{
    ASSERT(pen != nullptr);
    if (pen == nullptr) {
        return;
    }
    ASSERT((GetWidth() > 0) && (GetHeight() > 0));
    SkPaint skPaint = *m_pSkPaint;
    SetPaintByPen(skPaint, pen);

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
        SkColor colors[2] = {0};
        colors[0] = pen->GetColor().GetARGB();
        colors[1] = gradientColor->GetARGB();
        
        SkIRect rcGradientDestI = { gradientRect->left, gradientRect->top, gradientRect->right, gradientRect->bottom };
        SkRect rcGradientDest = SkRect::Make(rcGradientDestI);
        rcGradientDest.offset(*m_pSkPointOrg);

        SkPoint pts[2] = { {0, 0}, {0, 0} };
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
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDash:
    {
        float fValue = GetScaleFloat(5.0f) * fRatio;
        SkScalar intervals[] = { fValue * 1.0f, fValue * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue1 * 1.0f, fValue4 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
        break;
    }
    case IPen::kDashStyleDashDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue4 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f };
        skPathEffect = SkDashPathEffect::Make(intervals, 4, 0.0f);
        break;
    }
    case IPen::kDashStyleDashDotDot:
    {
        float fValue1 = GetScaleFloat(1.0f) * fRatio;
        float fValue4 = GetScaleFloat(4.0f) * fRatio;
        SkScalar intervals[] = { fValue4 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f, fValue1 * 1.0f };
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

void Render_Skia::DrawString(const UiRect& textRect,
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
    SkIRect rcSkDestI = { textRect.left, textRect.top, textRect.right, textRect.bottom };
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

void Render_Skia::MeasureRichText(const UiRect& textRect,
                                  const UiSize& szScrollOffset,
                                  IRenderFactory* pRenderFactory,
                                  const std::vector<RichTextData>& richTextData,
                                  std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::MeasureRichText"));
    InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, nullptr, nullptr, pRichTextRects);
}

void Render_Skia::MeasureRichText2(const UiRect& textRect,
                                   const UiSize& szScrollOffset,
                                   IRenderFactory* pRenderFactory,
                                   const std::vector<RichTextData>& richTextData,
                                   RichTextLineInfoParam* pLineInfoParam,
                                   std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::MeasureRichText2"));
    InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, nullptr, pRichTextRects);
}

void Render_Skia::MeasureRichText3(const UiRect& textRect,
                                   const UiSize& szScrollOffset,
                                   IRenderFactory* pRenderFactory, 
                                   const std::vector<RichTextData>& richTextData,
                                   RichTextLineInfoParam* pLineInfoParam,
                                   std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,
                                   std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::MeasureRichText3"));
    InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, pLineInfoParam, &spDrawRichTextCache, pRichTextRects);
}

void Render_Skia::DrawRichText(const UiRect& textRect,
                               const UiSize& szScrollOffset,
                               IRenderFactory* pRenderFactory,
                               const std::vector<RichTextData>& richTextData,
                               uint8_t uFade,
                               std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::DrawRichText"));
    InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, uFade, false, nullptr, nullptr, pRichTextRects);
}

//待绘制的文本
struct TPendingDrawRichText: public NVRefCount<TPendingDrawRichText>
{
    //在richTextData中的索引号
    uint32_t m_nDataIndex = 0;

    //物理行号
    uint32_t m_nLineNumber = 0;

    //行号(是绘制后的逻辑行号，当自动换行的时候，一个物理行显示为多个逻辑行，物理行号是指按文本中的换行符划分的行号）
    uint32_t m_nRowIndex = 0;

    //待绘制文本
    std::wstring_view m_textView;

    //绘制目标区域
    UiRect m_destRect;

    //Font对象
    std::shared_ptr<IFont> m_spFont;

    /** 文字颜色
    */
    UiColor m_textColor;

    /** 背景颜色
    */
    UiColor m_bgColor;

    /** 绘制文字的属性(包含文本对齐方式等属性，参见 enum DrawStringFormat)
    */
    uint16_t m_textStyle = 0;
};

/** 绘制缓存
*/
class DrawRichTextCache
{
public:
    /** 原始参数
    */
    UiRect m_textRect;
    std::vector<RichTextData> m_richTextData;

    SkTextEncoding m_textEncoding = SkTextEncoding::kUTF16;
    size_t m_textCharSize = sizeof(DStringW::value_type);

    /** 生成好的待绘制的数据
    */
    std::vector<SharePtr<TPendingDrawRichText>> m_pendingTextData;
};

bool Render_Skia::CreateDrawRichTextCache(const UiRect& textRect,
                                          const UiSize& szScrollOffset,
                                          IRenderFactory* pRenderFactory,
                                          const std::vector<RichTextData>& richTextData,
                                          std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    PerformanceStat statPerformance(_T("Render_Skia::CreateDrawRichTextCache"));
    spDrawRichTextCache.reset();
    InternalDrawRichText(textRect, szScrollOffset, pRenderFactory, richTextData, 255, true, nullptr, &spDrawRichTextCache, nullptr);
    return spDrawRichTextCache != nullptr;
}

bool Render_Skia::IsValidDrawRichTextCache(const UiRect& textRect,
                                           const std::vector<RichTextData>& richTextData,
                                           const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache)
{
    if (spDrawRichTextCache == nullptr) {
        return false;
    }
    if ((spDrawRichTextCache->m_textRect.Width() != textRect.Width()) ||
        (spDrawRichTextCache->m_textRect.Height() != textRect.Height())) {
        //矩形大小发生变化，不能使用缓存(位置变化时，可以使用缓存)
        return false;
    }
    if (spDrawRichTextCache->m_richTextData.size() != richTextData.size()) {
        return false;
    }
    bool bValid = true;
    const size_t nCount = richTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const RichTextData& textData = richTextData[nIndex];
        const RichTextData& textDataCache = spDrawRichTextCache->m_richTextData[nIndex];
        if (textData.m_textView.data() != textDataCache.m_textView.data()) {
            bValid = false;
        }
        else if (textData.m_textView.size() != textDataCache.m_textView.size()) {
            bValid = false;
        }
        else if (textData.m_textColor != textDataCache.m_textColor) {
            bValid = false;
        }
        else if (textData.m_bgColor != textDataCache.m_bgColor) {
            bValid = false;
        }
        else if (textData.m_pFontInfo != textDataCache.m_pFontInfo) {
            if ((textData.m_pFontInfo == nullptr) || (textDataCache.m_pFontInfo == nullptr)) {
                bValid = false;
            }
            else if (*textData.m_pFontInfo != *textDataCache.m_pFontInfo) {
                bValid = false;
            }
        }
        else if (textData.m_fRowSpacingMul != textDataCache.m_fRowSpacingMul) {
            bValid = false;
        }
        else if (textData.m_textStyle != textDataCache.m_textStyle) {
            bValid = false;
        }

        if (!bValid) {
            break;
        }
    }
    return bValid;
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
    PerformanceStat statPerformance(_T("Render_Skia::UpdateDrawRichTextCache"));
    ASSERT(spOldDrawRichTextCache != nullptr);
    if (spOldDrawRichTextCache == nullptr) {
        return false;
    }

    ASSERT(!modifiedLines.empty() || !deletedLines.empty());

    if (!modifiedLines.empty()) {
        if (nStartLine != modifiedLines[0]) {
            ASSERT(modifiedLines.empty() || (nStartLine == modifiedLines[0]));
            return false;
        }        
    }
    else if (!deletedLines.empty()) {
        if (nStartLine != deletedLines[0]) {
            ASSERT(deletedLines.empty() || (nStartLine == deletedLines[0]));
            return false;
        }
    }

    //删除的行数据，对应移除
    DrawRichTextCache& oldData = *spOldDrawRichTextCache;
    oldData.m_richTextData.swap(richTextDataNew);

    if (!deletedLines.empty()) {
        std::unordered_set<uint32_t> deletedLineSet;
        for (size_t nLine : deletedLines) {
            deletedLineSet.insert((uint32_t)nLine);
        }
        const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
        for (int32_t nIndex = nCount - 1; nIndex >= 0; --nIndex) {
            const TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
            if (deletedLineSet.find(pendingData.m_nLineNumber) != deletedLineSet.end()) {
                oldData.m_pendingTextData.erase(oldData.m_pendingTextData.begin() + nIndex);
            }
        }
    }

    //更新缓存开始的元素下标值    
    size_t nUpdateCacheStartIndex = (size_t)-1;
    //是否将修改的内容追加到最后了
    bool bAppendUpdateAtEnd = false;
    if (spUpdateDrawRichTextCache != nullptr) {
        DrawRichTextCache& updateData = *spUpdateDrawRichTextCache;
        if (!updateData.m_pendingTextData.empty()) {//容器可能为空（当本行为空行时为空）
            ASSERT(updateData.m_textRect == oldData.m_textRect);
            if (updateData.m_textRect != oldData.m_textRect) {
                return false;
            }
            ASSERT(updateData.m_textEncoding == oldData.m_textEncoding);
            if (updateData.m_textEncoding != oldData.m_textEncoding) {
                return false;
            }
            ASSERT(updateData.m_textCharSize == oldData.m_textCharSize);
            if (updateData.m_textCharSize != oldData.m_textCharSize) {
                return false;
            }
            //将新的绘制缓存，合并到原绘制缓存中
            const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
            for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
                const TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
                if (pendingData.m_nLineNumber > nStartLine) {
                    oldData.m_pendingTextData.insert(oldData.m_pendingTextData.begin() + nIndex, updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());
                    nUpdateCacheStartIndex = nIndex + updateData.m_pendingTextData.size();
                    break;
                }
            }
            if (nUpdateCacheStartIndex == (size_t)-1) {
                //追加在最后, 行号（物理行号，逻辑行号）无需更新，因为增量绘制的时候已经设置了正确的值
                bAppendUpdateAtEnd = true;
                oldData.m_pendingTextData.insert(oldData.m_pendingTextData.end(), updateData.m_pendingTextData.begin(), updateData.m_pendingTextData.end());                
            }
        }
    }

    int32_t nLineNumberDiff = (int32_t)modifiedLines.size() - (int32_t)deletedLines.size();
    int32_t nRowIndexDiff = (int32_t)nModifiedRows - (int32_t)nDeletedRows;
    bool bUpdateIndex = (nLineNumberDiff != 0) || (nRowIndexDiff != 0);

    //修正物理行号，逻辑行号，本行的绘制目标区域值
    const size_t nRowRectCount = rowRectTopList.size();
    bool bUpdateLineRows = false;
    const int32_t nCount = (int32_t)oldData.m_pendingTextData.size();
    for (int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        TPendingDrawRichText& pendingData = *oldData.m_pendingTextData[nIndex];
        if (!bUpdateLineRows && bUpdateIndex) {
            if ((nUpdateCacheStartIndex != (size_t)-1)) {
                //更新行号(有修改，并且修改点不再最后)
                if (nIndex >= (int32_t)nUpdateCacheStartIndex) {
                    bUpdateLineRows = true;
                }
            }
            else if (!bAppendUpdateAtEnd) {
                //无修改，只有删除；或者新修改的行只是空行的情况
                if (pendingData.m_nLineNumber >= nStartLine) {
                    bUpdateLineRows = true;
                }
            }
        }
        if (bUpdateLineRows) {
            if (nLineNumberDiff > 0) {
                pendingData.m_nLineNumber += (uint32_t)nLineNumberDiff;
            }
            else if (nLineNumberDiff < 0) {
                pendingData.m_nLineNumber -= (uint32_t)-nLineNumberDiff;
            }
            if (nRowIndexDiff > 0) {
                pendingData.m_nRowIndex += (uint32_t)nRowIndexDiff;
            }
            else if (nRowIndexDiff < 0) {
                pendingData.m_nRowIndex -= (uint32_t)-nRowIndexDiff;
            }
        }

        if (pendingData.m_nLineNumber >= nStartLine) {
            //对应数据的索引下标更新为无效值，后续不再使用
            pendingData.m_nDataIndex = (uint32_t)-1;

            //更新本行的绘制目标区域
            ASSERT(pendingData.m_nRowIndex < nRowRectCount);
            if (pendingData.m_nRowIndex < nRowRectCount) {
                pendingData.m_destRect.bottom = rowRectTopList[pendingData.m_nRowIndex] + pendingData.m_destRect.Height();
                pendingData.m_destRect.top = rowRectTopList[pendingData.m_nRowIndex];
            }
        }
    }
    return true;
}

bool Render_Skia::IsDrawRichTextCacheEqual(const DrawRichTextCache& first, const DrawRichTextCache& second) const
{
    ASSERT(first.m_textRect == second.m_textRect);
    if (first.m_textRect != second.m_textRect) {
        return false;
    }

    ASSERT(first.m_textEncoding == second.m_textEncoding);
    if (first.m_textEncoding != second.m_textEncoding) {
        return false;
    }

    ASSERT(first.m_textCharSize == second.m_textCharSize);
    if (first.m_textCharSize != second.m_textCharSize) {
        return false;
    }

    ASSERT(first.m_richTextData.size() == second.m_richTextData.size());
    if (first.m_richTextData.size() != second.m_richTextData.size()) {
        return false;
    }
    const size_t nDataCount = first.m_richTextData.size();
    for (size_t nIndex = 0; nIndex < nDataCount; ++nIndex) {
        const RichTextData& v1 = first.m_richTextData[nIndex];
        const RichTextData& v2 = second.m_richTextData[nIndex];
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT((v1.m_pFontInfo != nullptr) && (v2.m_pFontInfo != nullptr));
        if ((v1.m_pFontInfo == nullptr) || (v2.m_pFontInfo == nullptr)) {
            return false;
        }
        ASSERT(*v1.m_pFontInfo == *v2.m_pFontInfo);
        if (*v1.m_pFontInfo != *v2.m_pFontInfo) {
            return false;
        }
        ASSERT(v1.m_fRowSpacingMul == v2.m_fRowSpacingMul);
        if (v1.m_fRowSpacingMul != v2.m_fRowSpacingMul) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }

    ASSERT(first.m_pendingTextData.size() == second.m_pendingTextData.size());
    if (first.m_pendingTextData.size() != second.m_pendingTextData.size()) {
        return false;
    }

    const size_t nCount = first.m_pendingTextData.size();
    for (size_t nIndex = 0; nIndex < nCount; ++nIndex) {
        const TPendingDrawRichText& v1 = *first.m_pendingTextData[nIndex];
        const TPendingDrawRichText& v2 = *second.m_pendingTextData[nIndex];

        //m_nDataIndex 此值不需要比较
        ASSERT(v1.m_nLineNumber == v2.m_nLineNumber);
        if (v1.m_nLineNumber != v2.m_nLineNumber) {
            return false;
        }
        ASSERT(v1.m_nRowIndex == v2.m_nRowIndex);
        if (v1.m_nRowIndex != v2.m_nRowIndex) {
            return false;
        }
        ASSERT(v1.m_textView == v2.m_textView);
        if (v1.m_textView != v2.m_textView) {
            return false;
        }
        ASSERT(v1.m_destRect == v2.m_destRect);
        if (v1.m_destRect != v2.m_destRect) {
            return false;
        }

        ASSERT((v1.m_spFont != nullptr) && (v2.m_spFont != nullptr));
        if ((v1.m_spFont == nullptr) || (v2.m_spFont == nullptr)) {
            return false;
        }
        ASSERT(v1.m_spFont->FontName() == v2.m_spFont->FontName());
        if (v1.m_spFont->FontName() != v2.m_spFont->FontName()) {
            return false;
        }
        ASSERT(v1.m_spFont->FontSize() == v2.m_spFont->FontSize());
        if (v1.m_spFont->FontSize() != v2.m_spFont->FontSize()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsBold() == v2.m_spFont->IsBold());
        if (v1.m_spFont->IsBold() != v2.m_spFont->IsBold()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsUnderline() == v2.m_spFont->IsUnderline());
        if (v1.m_spFont->IsUnderline() != v2.m_spFont->IsUnderline()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsItalic() == v2.m_spFont->IsItalic());
        if (v1.m_spFont->IsItalic() != v2.m_spFont->IsItalic()) {
            return false;
        }
        ASSERT(v1.m_spFont->IsStrikeOut() == v2.m_spFont->IsStrikeOut());
        if (v1.m_spFont->IsStrikeOut() != v2.m_spFont->IsStrikeOut()) {
            return false;
        }
        
        ASSERT(v1.m_textColor == v2.m_textColor);
        if (v1.m_textColor != v2.m_textColor) {
            return false;
        }
        ASSERT(v1.m_bgColor == v2.m_bgColor);
        if (v1.m_bgColor != v2.m_bgColor) {
            return false;
        }
        ASSERT(v1.m_textStyle == v2.m_textStyle);
        if (v1.m_textStyle != v2.m_textStyle) {
            return false;
        }
    }
    return true;
}

void Render_Skia::DrawRichTextCacheData(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache,                                       
                                        const UiRect& rcNewTextRect,
                                        const UiSize& szNewScrollOffset,
                                        const std::vector<int32_t>& rowXOffset,
                                        uint8_t uFade,
                                        std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::DrawRichTextCacheData"));
    ASSERT(spDrawRichTextCache != nullptr);
    if (spDrawRichTextCache == nullptr) {
        return;
    }

    const SkTextEncoding textEncoding = spDrawRichTextCache->m_textEncoding;
    const size_t textCharSize = spDrawRichTextCache->m_textCharSize;

    const std::vector<SharePtr<TPendingDrawRichText>>& pendingTextData = spDrawRichTextCache->m_pendingTextData;

    UiRect rcTemp;
    UiRect rcDestRect;
    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(spDrawRichTextCache->m_richTextData.size());
    }

    UiColor textColor;
    for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
        const TPendingDrawRichText& textData = *spTextData;
        //通过缓存绘制的时候，不能使用textData.m_nDataIndex值，此值再增量绘制的情况下是不正确的
        
        //执行绘制        
        rcDestRect = textData.m_destRect;
        rcDestRect.Offset(rcNewTextRect.left, rcNewTextRect.top);
        rcDestRect.Offset(-szNewScrollOffset.cx, -szNewScrollOffset.cy);
        if (!rowXOffset.empty()) {
            ASSERT(textData.m_nRowIndex < rowXOffset.size());
            if ((textData.m_nRowIndex < rowXOffset.size()) && (rowXOffset[textData.m_nRowIndex] > 0)) {
                rcDestRect.Offset(rowXOffset[textData.m_nRowIndex], 0);
            }
        }

        //记录绘制区域
        if (pRichTextRects != nullptr) {
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(rcDestRect);
        }

        if (!UiRect::Intersect(rcTemp, rcDestRect, rcNewTextRect)) {
            continue;
        }

        //绘制文字的背景色
        if (!textData.m_bgColor.IsEmpty()) {
            FillRect(rcDestRect, textData.m_bgColor, uFade);
        }

        //设置文本颜色
        if (textColor != textData.m_textColor) {
            const UiColor& color = textData.m_textColor;
            skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
            textColor = textData.m_textColor;
        }

        //绘制文字
        if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
            //绘制TAB键
            const char* text = (const char*)" ";
            const size_t len = 1; //字节数
            DrawTextString(rcDestRect, text, len, SkTextEncoding::kUTF8,
                            textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                            skPaint, textData.m_spFont.get());
        }
        else {
            const char* text = (const char*)textData.m_textView.data();
            size_t len = textData.m_textView.size() * textCharSize; //字节数
            DrawTextString(rcDestRect, text, len, textEncoding,
                           textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                           skPaint, textData.m_spFont.get());
        }
    }
}

void Render_Skia::InternalDrawRichText(const UiRect& rcTextRect,
                                       const UiSize& szScrollOffset,
                                       IRenderFactory* pRenderFactory, 
                                       const std::vector<RichTextData>& richTextData,
                                       uint8_t uFade,
                                       bool bMeasureOnly,
                                       RichTextLineInfoParam* pLineInfoParam,
                                       std::shared_ptr<DrawRichTextCache>* pDrawRichTextCache,
                                       std::vector<std::vector<UiRect>>* pRichTextRects)
{
    PerformanceStat statPerformance(_T("Render_Skia::InternalDrawRichText"));
    //内部使用string_view实现，避免字符串复制影响性能
    if (rcTextRect.IsEmpty()) {
        return;
    }
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    //绘制区域：绘制区域的坐标以 (rcTextRect.left,rcTextRect.top)作为(0,0)点
    UiRect rcDrawRect = rcTextRect;
    rcDrawRect.Offset(-szScrollOffset.cx, -szScrollOffset.cy);

    if ((pLineInfoParam != nullptr) || (pDrawRichTextCache != nullptr)) {
        ASSERT(bMeasureOnly);
        if (!bMeasureOnly) {
            return;
        }
        //使用(0,0)坐标点，作为估算的(0,0)点
        rcDrawRect.Offset(-rcDrawRect.left, -rcDrawRect.top);
        ASSERT(rcDrawRect.left == 0);
        ASSERT(rcDrawRect.top == 0);
    }

    //文本编码：固定为UTF16 或者 UTF32
    constexpr const SkTextEncoding textEncoding = (sizeof(DStringW::value_type) == sizeof(uint32_t)) ? SkTextEncoding::kUTF32 : SkTextEncoding::kUTF16;
    constexpr const size_t textCharSize = sizeof(DStringW::value_type);

    //当绘制超过目标矩形边界时，是否继续绘制
    const bool bBreakWhenOutOfRect = !bMeasureOnly && (pDrawRichTextCache == nullptr);

    std::vector<SharePtr<TPendingDrawRichText>> pendingTextData;
    pendingTextData.reserve(richTextData.size());

    const int32_t nTextRectRightMax = (int32_t)rcTextRect.right;   //绘制区域的最右侧
    const int32_t nTextRectBottomMax = (int32_t)rcTextRect.bottom; //绘制区域的最底端

    SkScalar xPos = (SkScalar)rcDrawRect.left;  //水平坐标：字符绘制的时候，是按浮点型坐标，每个字符所占的宽度是浮点型的，不能对齐到像素
    int32_t yPos = rcDrawRect.top;              //垂直坐标，对齐到像素，所以用整型
    int32_t nRowHeight = 0;   //行高（本行中，所有字符绘制高度的最大值，对齐到像素）
    uint32_t nLineNumber = 0; //物理行号
    uint32_t nRowIndex = 0;   //逻辑行号

    std::unordered_map<uint32_t, uint32_t> rowHeightMap;  //每行的实际行高表

    //字体缓存(由于创建字体比较耗时，所以尽量复用相同的对象)
    SharePtr<UiFontEx> lastFont;
    std::shared_ptr<IFont> spLastSkiaFont;

    if (pLineInfoParam != nullptr) {
        //设置起始行号
        nLineNumber = (uint32_t)pLineInfoParam->m_nStartLineIndex;
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        if (pLineInfoParam->m_pLineInfoList == nullptr) {
            return;
        }
        ASSERT(nLineNumber < pLineInfoParam->m_pLineInfoList->size());
        if (nLineNumber >= pLineInfoParam->m_pLineInfoList->size()) {
            return;
        }
        //起始的逻辑行号
        nRowIndex = pLineInfoParam->m_nStartRowIndex;
    }

    //绘制属性
    SkPaint skPaint = *m_pSkPaint;
    if (uFade != 0xFF) {
        //透明度
        skPaint.setAlpha(uFade);
    }
    UiColor textColor;

    std::vector<SkGlyphID> glyphs;      //内部临时变量，为提升执行速度，在外部声明变量
    std::vector<uint8_t> glyphChars;    //内部临时变量，为提升执行速度，在外部声明变量
    std::vector<SkScalar> glyphWidths;  //内部临时变量，为提升执行速度，在外部声明变量

    std::vector<uint8_t> glyphCharList;   //每个字由几个字符构成
    std::vector<SkScalar> glyphWidthList; //每个字符的宽度

    //按换行符进行文本切分
    std::vector<std::wstring_view> lineTextViewList;

    //分行时文本切分的内部临时变量，为提升执行速度，在外部声明变量
    std::vector<uint32_t> lineSeprators;

    //是否正在绘制TAB键（按4个字符对齐）
    bool bDrawTabChar = false;

    //本行（逻辑行）已经绘制了多少个字符（不含回车和换行）
    size_t nRowCharCount = 0;

    for (size_t index = 0; index < richTextData.size(); ++index) {
        const RichTextData& textData = richTextData[index];
        if (textData.m_textView.empty()) {
            continue;
        }

        //设置文本颜色
        if (textColor != textData.m_textColor) {
            const UiColor& color = textData.m_textColor;
            skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
            textColor = textData.m_textColor;
        }
        
        std::shared_ptr<IFont> spSkiaFont;
        if ((spLastSkiaFont != nullptr) &&
            (textData.m_pFontInfo != nullptr) && (lastFont != nullptr) &&
            ((textData.m_pFontInfo == lastFont) || (*textData.m_pFontInfo == *lastFont))) {
            //复用缓存中的字体对象
            spSkiaFont = spLastSkiaFont;
        }
        else {
            spSkiaFont.reset(pRenderFactory->CreateIFont());
            ASSERT(spSkiaFont != nullptr);
            if (spSkiaFont == nullptr) {
                continue;
            }
            ASSERT(textData.m_pFontInfo != nullptr);
            if (textData.m_pFontInfo == nullptr) {
                continue;
            }
            if (!spSkiaFont->InitFont(*textData.m_pFontInfo)) {
                spSkiaFont.reset();
                continue;
            }
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

        if (spLastSkiaFont != spSkiaFont) {
            spLastSkiaFont = spSkiaFont;
            lastFont = textData.m_pFontInfo;
        }

        const SkFont& skFont = *pSkFont;
        SkFontMetrics metrics;
        SkScalar fFontHeight = skFont.getMetrics(&metrics);     //字体高度，换行时使用
        fFontHeight *= textData.m_fRowSpacingMul;               //运用行间距
        const int32_t nFontHeight = SkScalarCeilToInt(fFontHeight);   //行高对齐到像素
        nRowHeight = std::max(nRowHeight, nFontHeight);
        if (nRowHeight <= 0) {
            continue;
        }
        const uint32_t uTextStyle = textData.m_textStyle;
        const bool bSingleLineMode = (uTextStyle & DrawStringFormat::TEXT_SINGLELINE) ? true : false; //是否为单行模式，单行模式下，不换行
        const bool bWordWrap = bSingleLineMode ? false : ((uTextStyle & DrawStringFormat::TEXT_WORD_WRAP) ? true : false);
        bool bBreakAll = false;//标记是否终止

        //按换行符进行文本切分
        lineTextViewList.clear();
        SplitLines(textData.m_textView, lineSeprators, lineTextViewList);

        //物理行内的逻辑行号(每个物理行中，从0开始编号)
        uint32_t nLineTextRowIndex = 0;

        for (const std::wstring_view& lineTextView : lineTextViewList) {
            bDrawTabChar = false;
            if (lineTextView.size() == 1) {              
                if (lineTextView[0] == L'\r') {
                    //处理回车
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }
                    continue; //忽略回车
                }
                if (lineTextView[0] == L'\t') {
                    //处理TAB键
                    bDrawTabChar = true;
                }
                else if (lineTextView[0] == L'\n') {
                    //处理换行符
                    if (pLineInfoParam != nullptr) {
                        OnDrawUnicodeChar(pLineInfoParam, lineTextView[0], 1, 2, nLineNumber, nLineTextRowIndex, xPos, yPos, 0, nRowHeight);
                    }

                    //换行：执行换行操作(物理换行)
                    if (!bSingleLineMode) {
                        xPos = (SkScalar)rcDrawRect.left;
                        ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                        yPos += nRowHeight;
                        rowHeightMap[nRowIndex] = nRowHeight;
                        nRowHeight = nFontHeight;
                        ++nRowIndex;
                        nRowCharCount = 0;
                        ++nLineTextRowIndex;
                        ++nLineNumber;
                    }
                    continue; //处理下一行
                }
            }

            //绘制的文本下标开始值
            const size_t textCount = lineTextView.size();
            size_t textStartIndex = 0;
            while (textStartIndex < textCount) {
                //估算文本绘制区域                
                size_t byteLength = (textCount - textStartIndex) * textCharSize;                
                SkScalar maxWidth = SkIntToScalar(rcDrawRect.right) - xPos;//可用宽度
                if (!bWordWrap || bSingleLineMode) {
                    //不自动换行 或者 单行模式
                    maxWidth = SK_FloatInfinity;
                }
                ASSERT(maxWidth > 0);
                SkScalar textMeasuredWidth = 0;  //当前要绘制的文本，估算的所需宽度
                SkScalar textMeasuredHeight = 0; //当前要绘制的文本，估算的所需高度

                glyphCharList.clear();
                glyphWidthList.clear();

                //评估每个字符的矩形范围
                std::vector<uint8_t>* pGlyphCharList = &glyphCharList;
                std::vector<SkScalar>* pGlyphWidthList = &glyphWidthList;

                size_t nDrawLength = 0;
                if (bDrawTabChar) {
                    ASSERT(textCount == 1);
                    //绘制TAB键, 按4个字符对齐
                    const DStringW blank = L"    ";
                    size_t nBlankCount = nRowCharCount % blank.size();
                    nBlankCount = blank.size() - nBlankCount;
                    nDrawLength = SkTextBox::breakText(blank.c_str(),
                                                       nBlankCount * sizeof(DStringW::value_type), textEncoding,
                                                       skFont, skPaint,
                                                       maxWidth, &textMeasuredWidth, &textMeasuredHeight,
                                                       glyphs, glyphChars, glyphWidths,
                                                       pGlyphCharList, pGlyphWidthList);
                    if (nDrawLength > 0) {
                        nDrawLength = textCount * sizeof(DStringW::value_type);
                        if (glyphs.empty()) {
                            glyphs.resize(1);
                            glyphChars.resize(1);
                            glyphChars[0] = 1;
                            glyphWidths.resize(1, textMeasuredWidth);
                        }
                        pGlyphCharList->resize(1);
                        (*pGlyphCharList)[0] = 1;
                        pGlyphWidthList->resize(1);
                        (*pGlyphWidthList)[0] = textMeasuredWidth;
                    }
                }
                else {
                    //breakText函数执行时间占比约30%
                    nDrawLength = SkTextBox::breakText(lineTextView.data() + textStartIndex,
                                                       byteLength, textEncoding,
                                                       skFont, skPaint,
                                                       maxWidth, &textMeasuredWidth, &textMeasuredHeight,
                                                       glyphs, glyphChars, glyphWidths,
                                                       pGlyphCharList, pGlyphWidthList);
                }
                
                if (nDrawLength == 0) {
                    if (!bWordWrap || bSingleLineMode || (SkScalarTruncToInt(maxWidth) == rcDrawRect.Width())) {
                        //出错了(不能换行，或者换行后依然不够)
                        bBreakAll = true;
                        break;
                    }
                }
                else {
                    SharePtr<TPendingDrawRichText> spTextData(new TPendingDrawRichText);
                    spTextData->m_nDataIndex = (uint32_t)index;
                    spTextData->m_nLineNumber = nLineNumber;
                    spTextData->m_nRowIndex = nRowIndex;
                    spTextData->m_textView = std::wstring_view(lineTextView.data() + textStartIndex, nDrawLength / textCharSize);
                    spTextData->m_spFont = spSkiaFont;

                    spTextData->m_bgColor = textData.m_bgColor;
                    spTextData->m_textColor = textData.m_textColor;
                    spTextData->m_textStyle = textData.m_textStyle;

                    //绘制文字所需的矩形区域
                    spTextData->m_destRect.left = SkScalarTruncToInt(xPos); //左值：直接截断，如果有小数部分，直接去掉小数即可

                    SkScalar fRight = xPos + textMeasuredWidth;             //右值：如果有小数，则需要增加1个像素
                    spTextData->m_destRect.right = SkScalarCeilToInt(fRight);
                    spTextData->m_destRect.top = yPos;
                    spTextData->m_destRect.bottom = yPos + SkScalarCeilToInt(textMeasuredHeight); //记录字符的真实高度
                    pendingTextData.emplace_back(std::move(spTextData));

                    if (pLineInfoParam != nullptr) {
                        //评估每个字符的矩形范围
                        ASSERT(!glyphCharList.empty());
                        ASSERT(glyphCharList.size() == glyphWidthList.size());
                        if (glyphCharList.size() == glyphWidthList.size()) {
                            const size_t glyphCount = glyphCharList.size();
                            SkScalar glyphWidth = 0;
                            uint8_t glyphCharCount = 0;
                            SkScalar glyphLeft = (SkScalar)SkScalarTruncToInt(xPos);
                            for (size_t glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
                                glyphWidth = glyphWidthList[glyphIndex];//字符宽度
                                glyphCharCount = glyphCharList[glyphIndex];  //该字占几个字符（UTF16编码，可能是1或者2）
                                ASSERT((glyphCharCount == 1) || (glyphCharCount == 2));
                                OnDrawUnicodeChar(pLineInfoParam, 0, glyphCharCount, glyphCount, nLineNumber, nLineTextRowIndex, glyphLeft, yPos, glyphWidth, nRowHeight);
                                glyphLeft += glyphWidth;
                            }
                        }
                    }

                    //统计本逻辑行已经绘制了多少个字符
                    nRowCharCount += glyphs.size();
                }

                bool bNextRow = false; //是否需要换行的标志
                if (nDrawLength < byteLength) {
                    //宽度不足，需要换行
                    if (!bSingleLineMode) {
                        bNextRow = true;
                    }                    
                    textStartIndex += nDrawLength / textCharSize;
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                }
                else {
                    //当前行可容纳文本绘制
                    textStartIndex = textCount;//标记，结束循环
                    ASSERT(((int64_t)xPos + (int64_t)textMeasuredWidth) < INT32_MAX);
                    xPos += textMeasuredWidth;
                    if (xPos >= nTextRectRightMax) {
                        //X坐标右侧已经超出目标矩形的范围
                        if (bBreakWhenOutOfRect && bSingleLineMode) {
                            //单行模式，终止绘制
                            bBreakAll = true;
                            break;
                        }
                        else if (bWordWrap && !bSingleLineMode) {
                            //在自动换行的情况下，换行
                            bNextRow = true;
                        }
                    }
                }

                if (bNextRow) {
                    //换行：执行换行操作（逻辑换行，对nLineNumber不增加）
                    xPos = (SkScalar)rcDrawRect.left;
                    ASSERT(((int64_t)yPos + (int64_t)nRowHeight) < INT32_MAX);
                    yPos += nRowHeight;
                    rowHeightMap[nRowIndex] = nRowHeight;
                    nRowHeight = nFontHeight;
                    ++nRowIndex;
                    nRowCharCount = 0;
                    ++nLineTextRowIndex;

                    if (bBreakWhenOutOfRect && (yPos >= nTextRectBottomMax)) {
                        //Y坐标底部已经超出目标矩形的范围，终止绘制
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

    //记录最后一行的行高
    rowHeightMap[nRowIndex] = nRowHeight;

    //更新每行的行高(只有提前确定行高，才能正确绘制纵向对齐的文本)
    for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
        TPendingDrawRichText& textData = *spTextData;
        auto iter = rowHeightMap.find(textData.m_nRowIndex);
        ASSERT(iter != rowHeightMap.end());
        if (iter != rowHeightMap.end()) {
            textData.m_destRect.bottom = textData.m_destRect.top + iter->second;
        }
    }

    if (pRichTextRects != nullptr) {
        pRichTextRects->clear();
        pRichTextRects->resize(richTextData.size());
        for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
            const TPendingDrawRichText& textData = *spTextData;
            //保存绘制的目标区域，同一个文本，可能会有多个区域（换行时）
            ASSERT(textData.m_nDataIndex < pRichTextRects->size());
            std::vector<UiRect>& textRects = (*pRichTextRects)[textData.m_nDataIndex];
            textRects.push_back(textData.m_destRect); 
        }
    }

    if (pDrawRichTextCache != nullptr) {
        //生成绘制缓存，但不执行绘制
        std::shared_ptr<DrawRichTextCache> spDrawRichTextCache = std::make_shared<DrawRichTextCache>();
        *pDrawRichTextCache = spDrawRichTextCache;
        spDrawRichTextCache->m_richTextData = richTextData;
        spDrawRichTextCache->m_textRect = rcTextRect;

        spDrawRichTextCache->m_textEncoding = textEncoding;
        spDrawRichTextCache->m_textCharSize = textCharSize;

        spDrawRichTextCache->m_pendingTextData.swap(pendingTextData);
    }
    else if (!bMeasureOnly) {
        UiRect rcTemp;
        for (const SharePtr<TPendingDrawRichText>& spTextData : pendingTextData) {
            const TPendingDrawRichText& textData = *spTextData;
            //执行绘制            
            const UiRect& rcDestRect = textData.m_destRect;
            if (!UiRect::Intersect(rcTemp, rcDestRect, rcTextRect)) {
                continue;
            }

            //绘制文字的背景色
            FillRect(rcDestRect, textData.m_bgColor, uFade);

            if (textColor != textData.m_textColor) {
                const UiColor& color = textData.m_textColor;
                skPaint.setARGB(color.GetA(), color.GetR(), color.GetG(), color.GetB());
                textColor = textData.m_textColor;
            }

            //绘制文字
            if ((textData.m_textView.size() == 1) && (textData.m_textView[0] == L'\t')) {
                //绘制TAB键
                const char* text = (const char*)" ";
                const size_t len = 1; //字节数
                DrawTextString(rcDestRect, text, len, SkTextEncoding::kUTF8,
                               textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                               skPaint, textData.m_spFont.get());
            }
            else {
                const char* text = (const char*)textData.m_textView.data();
                const size_t len = textData.m_textView.size() * textCharSize; //字节数
                DrawTextString(rcDestRect, text, len, textEncoding,
                               textData.m_textStyle | DrawStringFormat::TEXT_SINGLELINE,
                               skPaint, textData.m_spFont.get());
            }
        }
    }
}

void Render_Skia::OnDrawUnicodeChar(RichTextLineInfoParam* pLineInfoParam,
                                    DStringW::value_type ch, uint8_t glyphChars, size_t glyphCount,
                                    size_t nLineTextIndex, uint32_t nLineTextRowIndex,
                                    float xPos, int32_t yPos, float glyphWidth, int32_t nRowHeight)
{
    //该函数执行频率非常高，每个字符都会调用一次，性能敏感    
    if (pLineInfoParam == nullptr) {
        ASSERT(pLineInfoParam != nullptr);
        return;
    }    
    if (pLineInfoParam->m_pLineInfoList == nullptr) {
        ASSERT(pLineInfoParam->m_pLineInfoList != nullptr);
        return;
    }
    size_t nIndex = nLineTextIndex; //外部已经加上了pLineInfoParam->m_nStartLineIndex的值    
    if (nIndex >= pLineInfoParam->m_pLineInfoList->size()) {
        ASSERT(nIndex < pLineInfoParam->m_pLineInfoList->size());
        return;
    }
    if ((*pLineInfoParam->m_pLineInfoList)[nIndex] == nullptr) {
        ASSERT((*pLineInfoParam->m_pLineInfoList)[nIndex] != nullptr);
        return;
    }

    RichTextLineInfo& lineInfo = *(*pLineInfoParam->m_pLineInfoList)[nIndex];
    bool bFound = (nLineTextRowIndex < lineInfo.m_rowInfo.size()) ? true : false;
    if (!bFound) {        
        if (nLineTextRowIndex != lineInfo.m_rowInfo.size()) {
            ASSERT(nLineTextRowIndex == lineInfo.m_rowInfo.size());
            return;
        }
        lineInfo.m_rowInfo.push_back(RichTextRowInfoPtr(new RichTextRowInfo));        
    }    
    if (lineInfo.m_rowInfo[nLineTextRowIndex] == nullptr) {
        ASSERT(lineInfo.m_rowInfo[nLineTextRowIndex] != nullptr);
        return;
    }
    RichTextRowInfo& rowInfo = *lineInfo.m_rowInfo[nLineTextRowIndex];
    if (!bFound) {
        //该行的第一个字符
        rowInfo.m_rowRect.left = xPos;
        rowInfo.m_rowRect.right = xPos + glyphWidth;
        rowInfo.m_rowRect.top = (SkScalar)yPos;
        rowInfo.m_rowRect.bottom = rowInfo.m_rowRect.top + nRowHeight;
        ASSERT(nRowHeight > 0);

        rowInfo.m_charInfo.reserve(glyphCount + 2);
    }
    else {
        rowInfo.m_rowRect.right += glyphWidth;
        ASSERT(nRowHeight == (int32_t)rowInfo.m_rowRect.Height());
    }

    RichTextCharInfo charInfo;
    charInfo.SetCharWidth(glyphWidth);    
    if (ch == '\r') {
        //回车
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo.AddCharFlag(RichTextCharFlag::kIsReturn);
        charInfo.SetCharWidth(0);
    }
    else if (ch == '\n') {
        //换行
        charInfo.SetCharFlag(0);
        charInfo.AddCharFlag(RichTextCharFlag::kIsNewLine);
        charInfo.SetCharWidth(0);
    }

    rowInfo.m_charInfo.emplace_back(std::move(charInfo));

    if (glyphChars == 2) {
        RichTextCharInfo charInfo2;
        charInfo2.SetCharWidth(0);
        charInfo2.SetCharFlag(0);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsIgnoredChar);
        charInfo2.AddCharFlag(RichTextCharFlag::kIsLowSurrogate);
        rowInfo.m_charInfo.emplace_back(std::move(charInfo2));
    }
    ASSERT((glyphChars == 1) || (glyphChars == 2));
}

void Render_Skia::SplitLines(const std::wstring_view& lineText, std::vector<uint32_t>& lineSeprators, std::vector<std::wstring_view>& lineTextViewList)
{
    if (lineText.empty()) {
        return;
    }
    lineSeprators.clear();
    const uint32_t nTextLen = (uint32_t)lineText.size();
    lineSeprators.reserve(nTextLen/100);
    for (uint32_t nTextIndex = 0; nTextIndex < nTextLen; ++nTextIndex) {
        const std::wstring_view::value_type& ch = lineText[nTextIndex];
        if ((ch == L'\t') || (ch == L'\r') || (ch == L'\n')) {
            lineSeprators.push_back(nTextIndex);
        }
    }
    if (lineSeprators.empty()) {
        //没有换行分隔符，单行
        lineTextViewList.emplace_back(std::wstring_view(lineText.data(), lineText.size()));
    }
    else {
        //有换行分隔符，切分为多行, 并保留换行符
        size_t nLastIndex = 0;
        size_t nCurrentIndex = 0;
        size_t nCharCount = 0;
        const size_t nLineSepCount = lineSeprators.size();
        lineTextViewList.reserve(nLineSepCount * 2 + 1);
        for (size_t nLine = 0; nLine < nLineSepCount; ++nLine) {
            if (nLine == 0) {
                //首行
                nLastIndex = 0;
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data(), nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }
            else {
                //中间行
                nLastIndex = lineSeprators[nLine - 1];
                nCurrentIndex = lineSeprators[nLine];
                ASSERT(nCurrentIndex > nLastIndex);
                ASSERT(nCurrentIndex < lineText.size());
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
                lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nCurrentIndex, 1));
            }

            if (nLine == (nLineSepCount - 1)) {
                //末行: 将最后一行数据添加进来
                nLastIndex = lineSeprators[nLine];
                nCurrentIndex = lineText.size();
                ASSERT(nCurrentIndex > nLastIndex);
                nCharCount = nCurrentIndex - nLastIndex - 1;
                if (nCharCount > 0) {
                    lineTextViewList.emplace_back(std::wstring_view(lineText.data() + nLastIndex + 1, nCharCount));
                }
            }
        }
    }
}

size_t Render_Skia::GetUTF16CharCount(const DStringW::value_type* srcPtr, size_t textStartIndex) const
{
    if (srcPtr != nullptr) {
        ASSERT(sizeof(uint16_t) == sizeof(DStringW::value_type));
        const uint16_t* src = (const uint16_t*)(srcPtr + textStartIndex);
        ASSERT(!SkUTF16_IsLowSurrogate(*src));
        if (SkUTF16_IsHighSurrogate(*src)) {
            ASSERT(SkUTF16_IsLowSurrogate(*(src + 1)));
            return 2;
        }
    }
    return 1;
}

void Render_Skia::DrawTextString(const UiRect& textRect, const DString& strText, uint32_t uFormat,
                                 const SkPaint& skPaint, IFont* pFont) const
{
    ASSERT(!strText.empty());
    if (strText.empty()) {
        return;
    }
    const char* text = (const char*)strText.c_str();
    size_t len = strText.size() * sizeof(DString::value_type);
    SkTextEncoding textEncoding = GetTextEncoding();
    DrawTextString(textRect, text, len, textEncoding, uFormat, skPaint, pFont);
}

void Render_Skia::DrawTextString(const UiRect& textRect,
                                 const char* text, size_t len, SkTextEncoding textEncoding,
                                 uint32_t uFormat, const SkPaint& skPaint, IFont* pFont) const
{
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
    const SkFont* pSkFont = pSkiaFont->GetFontHandle();
    ASSERT(pSkFont != nullptr);
    if (pSkFont == nullptr) {
        return;
    }

    //绘制区域
    SkIRect rcSkDestI = { textRect.left, textRect.top, textRect.right, textRect.bottom };
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
    skTextBox.draw(skCanvas, text, len, textEncoding, *pSkFont, skPaint);
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
