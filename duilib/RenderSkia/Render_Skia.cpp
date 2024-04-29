#include "Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"

#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/BitmapAlpha.h"
#include "duilib/Utils/PerformanceUtil.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201)

#include "include/core/SkMatrix.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkCanvas.h"
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
	SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(skPointOrg);

	SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
	SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

	pSkCanvas->drawImageRect(skImage, rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
}

Render_Skia::Render_Skia(IRenderFactory* pRenderFactory)
	: m_bTransparent(false),
	m_pSkCanvas(nullptr),
	m_hDC(nullptr),
	m_saveCount(0),
	m_pRenderFactory(pRenderFactory)
{
	m_pSkPointOrg = new SkPoint;
	m_pSkPointOrg->iset(0, 0);
	m_pSkPaint = new SkPaint;
	m_pSkPaint->setAntiAlias(true);
	m_pSkPaint->setDither(true);
	m_pSkPaint->setStyle(SkPaint::kStrokeAndFill_Style);
}

Render_Skia::~Render_Skia()
{
	m_pBitmapSkia.reset();
	if (m_pSkPointOrg) {
		delete m_pSkPointOrg;
		m_pSkPointOrg = nullptr;
	}
	if (m_pSkCanvas) {
		delete m_pSkCanvas;
		m_pSkCanvas = nullptr;
	}
	if (m_pSkPaint) {
		delete m_pSkPaint;
		m_pSkPaint = nullptr;
	}
	if (m_hDC != nullptr) {
		::DeleteDC(m_hDC);
		m_hDC = nullptr;
	}
}

RenderType Render_Skia::GetRenderType() const
{
	return RenderType::kRenderType_Skia;
}

bool Render_Skia::Resize(int width, int height)
{
	ASSERT((width > 0) && (height > 0));
	if ((width <= 0) || (height <= 0)) {
		return false;
	}
	if ((GetWidth() == width) && (GetHeight() == height)) {
		return true;
	}

	if (m_hDC != nullptr) {
		::DeleteDC(m_hDC);
		m_hDC = nullptr;
	}
	m_pBitmapSkia = std::make_unique<Bitmap_Skia>();
	if (!m_pBitmapSkia->Init(width, height, true, nullptr)) {
		ASSERT(!"init failed!");
		return false;
	}
	if (m_pSkCanvas != nullptr) {
		delete m_pSkCanvas;
		m_pSkCanvas = nullptr;
	}
	m_pSkCanvas = new SkCanvas(m_pBitmapSkia->GetSkBitmap());
	return true;
}

void Render_Skia::Clear()
{
	if (m_pBitmapSkia != nullptr) {
		void* pPixelBits = m_pBitmapSkia->LockPixelBits();
		if (pPixelBits != nullptr) {
			::memset(pPixelBits, 0, GetWidth() * GetHeight() * 4);
			m_pBitmapSkia->UnLockPixelBits();
		}
	}
}

std::unique_ptr<ui::IRender> Render_Skia::Clone()
{
	std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_Skia>(m_pRenderFactory);
	pClone->Resize(GetWidth(), GetHeight());
	pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this, 0, 0, RopMode::kSrcCopy);
	return pClone;
}

IBitmap* Render_Skia::DetachBitmap()
{
	if (m_hDC != nullptr) {
		::DeleteDC(m_hDC);
		m_hDC = nullptr;
	}
	IBitmap* pBitmap = nullptr;
	HBITMAP hBitmap = m_pBitmapSkia->DetachHBitmap();
	if (hBitmap != nullptr) {
		pBitmap = new Bitmap_Skia(hBitmap, true);
	}
	if (m_pSkCanvas != nullptr) {
		delete m_pSkCanvas;
		m_pSkCanvas = nullptr;
	}
	m_pBitmapSkia.reset();
	return pBitmap;
}

int Render_Skia::GetWidth()
{
	if (m_pBitmapSkia != nullptr) {
		return m_pBitmapSkia->GetWidth();
	}
	return 0;
}

int Render_Skia::GetHeight()
{
	if (m_pBitmapSkia != nullptr) {
		return m_pBitmapSkia->GetHeight();
	}
	return 0;
}

void Render_Skia::ClearAlpha(const UiRect& rcDirty, uint8_t alpha)
{
	HBITMAP hBitmap = nullptr;
	if (m_pBitmapSkia != nullptr) {
		hBitmap = m_pBitmapSkia->GetHBitmap();
	}
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.ClearAlpha(rcDirty, alpha);
	}
}

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha)
{
	HBITMAP hBitmap = nullptr;
	if (m_pBitmapSkia != nullptr) {
		hBitmap = m_pBitmapSkia->GetHBitmap();
	}
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding, alpha);
	}
}

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding)
{
	HBITMAP hBitmap = nullptr;
	if (m_pBitmapSkia != nullptr) {
		hBitmap = m_pBitmapSkia->GetHBitmap();
	}
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding);
	}
}

bool Render_Skia::IsRenderTransparent() const
{
	return m_bTransparent;
}

bool Render_Skia::SetRenderTransparent(bool bTransparent)
{
	bool oldValue = m_bTransparent;
	m_bTransparent = bTransparent;
	return oldValue;
}

UiPoint Render_Skia::OffsetWindowOrg(UiPoint ptOffset)
{
	UiPoint ptOldWindowOrg = { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY) };
	SkPoint ptOff = { SkIntToScalar(ptOffset.x), SkIntToScalar(ptOffset.y) };
	//Gdiplus版本使用的是SetWindowOrgEx，而Skia内部是用的SetViewportOrgEx逻辑，所以这里要符号取反
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

void Render_Skia::SaveClip(int& nState)
{
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_saveCount = m_pSkCanvas->save();
		nState = m_saveCount;
	}
}

void Render_Skia::RestoreClip(int nState)
{
	ASSERT(m_pSkCanvas != nullptr);
	ASSERT(m_saveCount == nState);
	if (m_saveCount != nState) {
		return;
	}
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->restoreToCount(nState);
	}
}

void Render_Skia::SetClip(const UiRect& rc, bool bIntersect)
{
	SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
	SkRect rcSk = SkRect::Make(rcSkI);
	rcSk.offset(*m_pSkPointOrg);
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->save();
		if (bIntersect) {
			m_pSkCanvas->clipRect(rcSk, SkClipOp::kIntersect, true);
		}
		else {
			m_pSkCanvas->clipRect(rcSk, SkClipOp::kDifference, true);
		}
	}
}

void Render_Skia::SetRoundClip(const UiRect& rc, int width, int height, bool bIntersect)
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
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->save();
		if (bIntersect) {
			m_pSkCanvas->clipRegion(rgn, SkClipOp::kIntersect);
		}
		else {
			m_pSkCanvas->clipRegion(rgn, SkClipOp::kDifference);
		}
	}
}

void Render_Skia::ClearClip()
{
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->restore();
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

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, Bitmap_Skia* pSrcBitmap, int xSrc, int ySrc, RopMode rop)
{
	Bitmap_Skia* skiaBitmap = pSrcBitmap;
	ASSERT(skiaBitmap != nullptr);
	if (skiaBitmap == nullptr) {
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawImageRect(skiaBitmap->GetSkBitmap().asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
		return true;
	}
	return false;
}

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, IBitmap* pSrcBitmap, int xSrc, int ySrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcBitmap != nullptr);
	if (pSrcBitmap == nullptr) {
		return false;
	}
	Bitmap_Skia* skiaBitmap = dynamic_cast<Bitmap_Skia*>(pSrcBitmap);
	return BitBlt(x, y, cx, cy, skiaBitmap, xSrc, ySrc, rop);
}

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, IRender* pSrcRender, int xSrc, int ySrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}

	Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
	ASSERT(pSkiaRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}
	Bitmap_Skia* skiaBitmap = pSkiaRender->m_pBitmapSkia.get();
	return BitBlt(x, y, cx, cy, skiaBitmap, xSrc, ySrc, rop);
}

bool Render_Skia::StretchBlt(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}

	Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
	ASSERT(pSkiaRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}
	Bitmap_Skia* skiaBitmap = pSkiaRender->m_pBitmapSkia.get();
	ASSERT(skiaBitmap != nullptr);
	if (skiaBitmap == nullptr) {
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawImageRect(skiaBitmap->GetSkBitmap().asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
		return true;
	}
	return false;

}

bool Render_Skia::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, uint8_t alpha)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}

	Render_Skia* pSkiaRender = dynamic_cast<Render_Skia*>(pSrcRender);
	ASSERT(pSkiaRender != nullptr);
	if (pSrcRender == nullptr) {
		return false;
	}
	Bitmap_Skia* skiaBitmap = pSkiaRender->m_pBitmapSkia.get();
	ASSERT(skiaBitmap != nullptr);
	if (skiaBitmap == nullptr) {
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawImageRect(skiaBitmap->GetSkBitmap().asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
		return true;
	}
	return false;
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
							const UiRect& rcDest, const UiRect& rcDestCorners,
							const UiRect& rcSource, const UiRect& rcSourceCorners,
							uint8_t uFade, bool xtiled, bool ytiled,
							bool fullxtiled, bool fullytiled, int nTiledMargin)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	UiRect rcTestTemp;
	if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
		return;
	}
	PerformanceStat statPerformance(L"Render_Skia::DrawImage");

	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return;
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
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

	bool bAlphaChannel = pBitmap->IsAlphaBitmap();

	SkPaint skPaint = *m_pSkPaint;
	if (uFade != 0xFF) {
		skPaint.setAlpha(uFade);
	}
	if (m_bTransparent || bAlphaChannel || (uFade != 0xFF)) {
		skPaint.setBlendMode(SkBlendMode::kSrcOver);
	}

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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
		}
		else if (xtiled && ytiled) {
			const LONG imageDrawWidth = rcSource.right - rcSource.left - rcSourceCorners.left - rcSourceCorners.right;
			const LONG imageDrawHeight = rcSource.bottom - rcSource.top - rcSourceCorners.top - rcSourceCorners.bottom;
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
				LONG lDestTop = rcDrawDest.top + j * imageDrawHeight + j * nTiledMargin;
				LONG lDestBottom = lDestTop + imageDrawHeight;
				LONG lDrawHeight = imageDrawHeight;
				if (lDestBottom > rcDrawDest.bottom) {
					lDrawHeight -= lDestBottom - rcDrawDest.bottom;
					lDestBottom = rcDrawDest.bottom;
				}
				for (int i = 0; i < iTimesX; ++i) {
					LONG lDestLeft = rcDrawDest.left + i * imageDrawWidth + i * nTiledMargin;
					LONG lDestRight = lDestLeft + imageDrawWidth;
					LONG lDrawWidth = imageDrawWidth;
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
					DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
				}
			}
		}
		else if (xtiled) { // supp
			const LONG imageDrawWidth = rcSource.right - rcSource.left - rcSourceCorners.left - rcSourceCorners.right;
			int iTimes = iTimes = (rcDrawDest.right - rcDrawDest.left) / (imageDrawWidth + nTiledMargin);
			if (!fullxtiled) {
				if ((rcDrawDest.right - rcDrawDest.left) % (imageDrawWidth + nTiledMargin) > 0) {
					iTimes += 1;
				}
			}

			for (int i = 0; i < iTimes; ++i) {
				LONG lDestLeft = rcDrawDest.left + i * imageDrawWidth + i * nTiledMargin;
				LONG lDestRight = lDestLeft + imageDrawWidth;
				LONG lDrawWidth = imageDrawWidth;
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

				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
			}
		}
		else { // ytiled
			const LONG imageDrawHeight = rcSource.bottom - rcSource.top - rcSourceCorners.top - rcSourceCorners.bottom;
			int iTimes = (rcDrawDest.bottom - rcDrawDest.top) / (imageDrawHeight + nTiledMargin);
			if (!fullytiled) {
				if ((rcDrawDest.bottom - rcDrawDest.top) % (imageDrawHeight + nTiledMargin) > 0) {
					iTimes += 1;
				}
			}

			UiRect rcDestTemp;
			rcDestTemp.left = rcDrawDest.left;

			for (int i = 0; i < iTimes; ++i) {
				LONG lDestTop = rcDrawDest.top + i * imageDrawHeight + i * nTiledMargin;
				LONG lDestBottom = lDestTop + imageDrawHeight;
				LONG lDrawHeight = imageDrawHeight;
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

				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDrawDest, *m_pSkPointOrg, skImage, rcDrawSource, skPaint);
		}
	}
}

void Render_Skia::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
							const UiRect& rcDest, const UiRect& rcSource,
							uint8_t uFade, bool xtiled, bool ytiled,
							bool fullxtiled, bool fullytiled, int nTiledMargin)
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return;
	}

	bool bAlphaChannel = pBitmap->IsAlphaBitmap();
	SkPaint skPaint = *m_pSkPaint;
	if (uFade != 0xFF) {
		skPaint.setAlpha(uFade);
	}
	if (m_bTransparent || bAlphaChannel || (uFade != 0xFF)) {
		skPaint.setBlendMode(SkBlendMode::kSrcOver);
	}

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
			m_pSkCanvas->setMatrix(skM44);
			isMatrixSet = true;
		}
	}
	DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
	if (isMatrixSet) {
		m_pSkCanvas->resetMatrix();
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawRect(rcSkDest, skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawLine(skPt1, skPt2, skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawLine(skPt1, skPt2, skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawLine(skPt1, skPt2, skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawRect(rcSkDest, skPaint);
	}
}

void Render_Skia::DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	SkPaint skPaint = *m_pSkPaint;
	skPaint.setARGB(penColor.GetA(), penColor.GetR(), penColor.GetG(), penColor.GetB());
	skPaint.setStrokeWidth(SkIntToScalar(nWidth));
	skPaint.setStyle(SkPaint::kStroke_Style);

	SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(*m_pSkPointOrg);

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawRoundRect(rcSkDest, SkIntToScalar(roundSize.cx), SkIntToScalar(roundSize.cy), skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawRoundRect(rcSkDest, SkIntToScalar(roundSize.cx), SkIntToScalar(roundSize.cy), skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, SkIntToScalar(radius), skPaint);
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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawCircle(rcSkPoint.fX, rcSkPoint.fY, SkIntToScalar(radius), skPaint);
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
		SkColor colors[2];
		colors[0] = pen->GetColor().GetARGB();
		colors[1] = gradientColor->GetARGB();
		
		SkIRect rcGradientDestI = { gradientRect->left, gradientRect->top, gradientRect->right, gradientRect->bottom };
		SkRect rcGradientDest = SkRect::Make(rcGradientDestI);
		rcGradientDest.offset(*m_pSkPointOrg);

		SkPoint pts[2];
		pts[0].set(rcGradientDest.fLeft, rcGradientDest.fTop);
		pts[1].set(rcGradientDest.fRight, rcGradientDest.fBottom);
		
		sk_sp<SkShader> shaderA = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
		paint.setShader(shaderA);
	}

	m_pSkCanvas->drawArc(ovalRect, startAngle, sweepAngle, useCenter, paint);
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
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawPath(skPath, paint);
	}
}

void Render_Skia::SetPaintByPen(SkPaint& skPaint, const IPen* pen)
{
	skPaint.setColor(pen->GetColor().GetARGB());

	sk_sp<SkPathEffect> skPathEffect;
	IPen::DashStyle dashStyle = pen->GetDashStyle();
	//线宽的倍数
	int32_t nRatio = pen->GetWidth() / GlobalManager::Instance().Dpi().GetScaleInt(1);
	switch (dashStyle) {
	case IPen::kDashStyleSolid:
	{
		SkScalar intervals[] = { 1.0f, 0.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDash:
	{
		int32_t nValue = GlobalManager::Instance().Dpi().GetScaleInt(5) * nRatio;
		SkScalar intervals[] = { nValue * 1.0f, nValue * 1.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDot:
	{
		int32_t nValue1 = GlobalManager::Instance().Dpi().GetScaleInt(1) * nRatio;
		int32_t nValue4 = GlobalManager::Instance().Dpi().GetScaleInt(4) * nRatio;
		SkScalar intervals[] = { nValue1 * 1.0f, nValue4 * 1.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDashDot:
	{
		int32_t nValue1 = GlobalManager::Instance().Dpi().GetScaleInt(1) * nRatio;
		int32_t nValue4 = GlobalManager::Instance().Dpi().GetScaleInt(4) * nRatio;
		SkScalar intervals[] = { nValue4 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f, nValue1 * 1.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 4, 0.0f);
		break;
	}
	case IPen::kDashStyleDashDotDot:
	{
		int32_t nValue1 = GlobalManager::Instance().Dpi().GetScaleInt(1) * nRatio;
		int32_t nValue4 = GlobalManager::Instance().Dpi().GetScaleInt(4) * nRatio;
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

	SkPaint::Cap cap;
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
		cap = SkPaint::Cap::kDefault_Cap;
		break;
	}

	skPaint.setStrokeCap(cap);

	SkPaint::Join join;
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
		join = SkPaint::Join::kDefault_Join;
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
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawPath(skPath, paint);
	}
}

void Render_Skia::DrawString(const UiRect& rc, 
							 const std::wstring& strText,
	                         UiColor dwTextColor, 
	                         const std::wstring& strFontId, 
	                         uint32_t uFormat, 
	                         uint8_t uFade /*= 255*/)
{
	PerformanceStat statPerformance(L"Render_Skia::DrawString");
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(!strText.empty());
	if (strText.empty()) {
		return;
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return;
	}
	//文本编码
	SkTextEncoding textEncoding = SkTextEncoding::kUTF16;
	
	//获取字体接口
	IFont* pFont = GlobalManager::Instance().Font().GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return;
	}
	Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
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
	skTextBox.draw(m_pSkCanvas, 
		           (const char*)strText.c_str(), 
		           strText.size() * sizeof(std::wstring::value_type),
		           textEncoding, 
		           *pSkFont,
		           skPaint);
}

UiRect Render_Skia::MeasureString(const std::wstring& strText, 
								  const std::wstring& strFontId,
						          uint32_t uFormat, 
						    	  int width /*= DUI_NOSET_VALUE*/)
{
	PerformanceStat statPerformance(L"Render_Skia::MeasureString");
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(!strText.empty());
	if (strText.empty()) {
		return UiRect();
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return UiRect();
	}

	//获取字体接口
	IFont* pFont = GlobalManager::Instance().Font().GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return UiRect();
	}
	Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
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
												  strText.size() * sizeof(std::wstring::value_type),
												  SkTextEncoding::kUTF16,
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
													  strText.size() * sizeof(std::wstring::value_type),
													  SkTextEncoding::kUTF16,
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
							   std::vector<RichTextData>& richTextData,
							   uint32_t uFormat,
							   bool bMeasureOnly,
							   uint8_t uFade)
{
	PerformanceStat statPerformance(L"Render_Skia::DrawRichText");
	if (rc.IsEmpty()) {
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
		std::wstring m_text;

		//绘制目标区域
		UiRect m_destRect;

		//Paint对象
		SkPaint m_skPaint;

		//Font对象
		std::shared_ptr<Font_Skia> m_spSkiaFont;

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
		
		std::shared_ptr<Font_Skia> spSkiaFont = std::make_shared<Font_Skia>(m_pRenderFactory);
		if (!spSkiaFont->InitFont(textData.m_fontInfo)) {
			spSkiaFont.reset();
		}

		if (spSkiaFont == nullptr) {
			continue;
		}

		const SkFont* pSkFont = spSkiaFont->GetFontHandle();
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

		std::wstring textValue = textData.m_text.c_str();
		//统一换行标志
		StringHelper::ReplaceAll(L"\r\n", L"\n", textValue);
		StringHelper::ReplaceAll(L"\r", L"\n", textValue);
		if (textValue == L"\n") {
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
		std::list<std::wstring> textList = StringHelper::Split(textValue, L"\n");
		for (const std::wstring& text : textList) {
			//绘制的文本下标开始值		
			const size_t textCount = text.size();
			size_t textStartIndex = 0;
			while (textStartIndex < textCount) {
				//估算文本绘制区域
				SkScalar maxWidth = SkIntToScalar(rc.right - xPos);//可用宽度
				ASSERT(maxWidth > 0);
				SkScalar measuredWidth = 0;
				size_t byteLength = (textCount - textStartIndex) * sizeof(wchar_t);
				size_t nDrawLength = SkTextBox::breakText(text.c_str() + textStartIndex,
														  byteLength,
														  SkTextEncoding::kUTF16,
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
					spTextData->m_text = text.substr(textStartIndex, nDrawLength / sizeof(wchar_t));
				}
				spTextData->m_skPaint = skPaint;
				spTextData->m_spSkiaFont = spSkiaFont;
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
					textStartIndex += nDrawLength / sizeof(wchar_t);
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
			DrawTextString(textData.m_destRect, textData.m_text, uFormat | DrawStringFormat::TEXT_SINGLELINE, textData.m_skPaint, textData.m_spSkiaFont.get());
		}
	}
}

void Render_Skia::DrawTextString(const UiRect& rc, const std::wstring& strText, uint32_t uFormat,
							     const SkPaint& skPaint, IFont* pFont) const
{
	ASSERT(!strText.empty());
	if (strText.empty()) {
		return;
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return;
	}
	Font_Skia* pSkiaFont = dynamic_cast<Font_Skia*>(pFont);
	ASSERT(pSkiaFont != nullptr);
	if (pSkiaFont == nullptr) {
		return;
	}
	//文本编码
	SkTextEncoding textEncoding = SkTextEncoding::kUTF16;
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
	skTextBox.draw(m_pSkCanvas,
				   (const char*)strText.c_str(),
				   strText.size() * sizeof(std::wstring::value_type),
				   textEncoding,
				   *pSkFont,
				   skPaint);
}

void Render_Skia::DrawBoxShadow(const UiRect& rc,
								const UiSize& roundSize, 
						        const UiPoint& cpOffset, 
						 	    int nBlurRadius, 
								int nSpreadRadius,
								UiColor dwColor)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(dwColor.GetARGB() != 0);
	if (nBlurRadius < 0) {
		nBlurRadius = 0;
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

	SkAutoCanvasRestore autoCanvasRestore(m_pSkCanvas, true);

	//裁剪中间区域
	SkPath skPathExclude;
	excludePath.offset(m_pSkPointOrg->fX, m_pSkPointOrg->fY, &skPathExclude);
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->clipPath(skPathExclude, SkClipOp::kDifference);
	}

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

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawPath(skPath, paint);
	}
}

HDC Render_Skia::GetDC()
{
	if (m_hDC != nullptr) {
		return m_hDC;
	}
	if ((m_pBitmapSkia == nullptr) || (m_pSkCanvas == nullptr)) {
		return nullptr;
	}
	HBITMAP hBitmap = m_pBitmapSkia->GetHBitmap();
	if (hBitmap == nullptr) {
		return nullptr;
	}
	HDC hDeskDC = ::GetDC(NULL);
	HDC hGetDC = ::CreateCompatibleDC(hDeskDC);
	::ReleaseDC(NULL, hDeskDC);
	hDeskDC = nullptr;

	::SelectObject(hGetDC, hBitmap);

	if (m_pSkCanvas->isClipEmpty()) {
		::IntersectClipRect(hGetDC, 0, 0, 0, 0);
	}
	else if (m_pSkCanvas->isClipRect()) {
		SkRect rcClip;
		if (m_pSkCanvas->getLocalClipBounds(&rcClip)) {
			RECT rc = { (int)rcClip.left(),(int)rcClip.top(),(int)rcClip.right(),(int)rcClip.bottom() };
			::InflateRect(&rc, -1, -1); //注意需要向内缩小一个象素
			::IntersectClipRect(hGetDC, rc.left, rc.top, rc.right, rc.bottom);
		}
	}
	else
	{
		SkRegion rgn;
		m_pSkCanvas->temporary_internal_getRgnClip(&rgn);
		SkRegion::Iterator it(rgn);
		int nCount = 0;
		for (; !it.done(); it.next()) {
			++nCount;
		}
		it.rewind();

		int nSize = sizeof(RGNDATAHEADER) + nCount * sizeof(RECT);
		RGNDATA* rgnData = (RGNDATA*)::malloc(nSize);
		memset(rgnData, 0, nSize);
		rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
		rgnData->rdh.iType = RDH_RECTANGLES;
		rgnData->rdh.nCount = nCount;
		rgnData->rdh.rcBound.right = GetWidth();
		rgnData->rdh.rcBound.bottom = GetHeight();

		nCount = 0;
		LPRECT pRc = (LPRECT)rgnData->Buffer;
		for (; !it.done(); it.next()) {
			SkIRect skrc = it.rect();
			RECT rc = { skrc.fLeft,skrc.fTop,skrc.fRight,skrc.fBottom };
			pRc[nCount++] = rc;
		}

		HRGN hRgn = ::ExtCreateRegion(NULL, nSize, rgnData);
		::free(rgnData);
		::SelectClipRgn(hGetDC, hRgn);
		::DeleteObject(hRgn);
	}

	::SetGraphicsMode(hGetDC, GM_ADVANCED);
	::SetViewportOrgEx(hGetDC, (int)m_pSkPointOrg->x(), (int)m_pSkPointOrg->y(), NULL);

	struct IxForm
	{
		enum Index {
			kMScaleX = 0,
			kMSkewX,
			kMTransX,
			kMSkewY,
			kMScaleY,
			kMTransY,
			kMPersp0,
			kMPersp1,
			kMPersp2
		};
	};
	SkMatrix mtx = m_pSkCanvas->getTotalMatrix();
	XFORM xForm = { mtx.get(IxForm::kMScaleX),mtx.get(IxForm::kMSkewY),
		            mtx.get(IxForm::kMSkewX),mtx.get(IxForm::kMScaleY),
		            mtx.get(IxForm::kMTransX),mtx.get(IxForm::kMTransY) };
	::SetWorldTransform(hGetDC, &xForm);
	m_hDC = hGetDC;
	return hGetDC;
}

void Render_Skia::ReleaseDC(HDC hdc)
{
	if (hdc == m_hDC) {
		::DeleteDC(hdc);
		m_hDC = nullptr;
	}
}


} // namespace ui
