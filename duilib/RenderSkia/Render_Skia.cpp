#include "Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/SkTextBox.h"

#include "duilib/Utils/DpiManager.h"
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

Render_Skia::Render_Skia()
	: m_bTransparent(false),
	m_pSkCanvas(nullptr),
	m_hDC(nullptr),
	m_saveCount(0)
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
	std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_Skia>();
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

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, uint8_t alpha)
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

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding)
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

void Render_Skia::SetClip(const UiRect& rc)
{
	SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
	SkRect rcSk = SkRect::Make(rcSkI);
	rcSk.offset(*m_pSkPointOrg);
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->save();
		m_pSkCanvas->clipRect(rcSk, true);
	}
}

void Render_Skia::SetRoundClip(const UiRect& rc, int width, int height)
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
		m_pSkCanvas->clipRegion(rgn);
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

void Render_Skia::DrawImage(const UiRect& rcPaint,
							   IBitmap* pBitmap, 						       
						       const UiRect& rcImageDest, 
							   UiRect rcImageSource, 
						       UiRect rcImageCorners,
						       bool bBitmapDpiScaled,
						       uint8_t uFade,
						       bool xtiled, 
						       bool ytiled, 
						       bool fullxtiled, 
						       bool fullytiled, 
						       int nTiledMargin)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	UiRect rcTestTemp;
	if (!::IntersectRect(&rcTestTemp, &rcImageDest, &rcPaint)) {
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

	//对边角值进行容错处理
	if ((rcImageCorners.left < 0)  ||
		(rcImageCorners.top < 0)   ||
		(rcImageCorners.right < 0) ||
		(rcImageCorners.bottom < 0)) {
		rcImageCorners.left = 0;
		rcImageCorners.top = 0;
		rcImageCorners.right = 0;
		rcImageCorners.bottom = 0;
	}

	UiRect rcTemp;
	UiRect rcSource;
	UiRect rcDest;
	UiRect rcDpiCorner = rcImageCorners;
	DpiManager::GetInstance()->ScaleRect(rcDpiCorner);
	// 如果源位图已经按照DPI缩放过，那么对应的rcImageCorners也需要缩放
	if (bBitmapDpiScaled) {
		rcImageCorners = rcDpiCorner;
	}
	// 如果源位图已经按照DPI缩放过，那么对应的rcImageSource也需要缩放
	if ((rcImageSource.left < 0)  ||
		(rcImageSource.top < 0)   ||
		(rcImageSource.right < 0) ||
		(rcImageSource.bottom < 0)) {
		//如果是无效值，则重置为整个图片大小
		rcImageSource.left = 0;
		rcImageSource.top = 0;
		rcImageSource.right = pBitmap->GetWidth();
		rcImageSource.bottom = pBitmap->GetHeight();
	}
	else if (bBitmapDpiScaled) {
		//如果外部设置此值，做DPI自适应处理
		DpiManager::GetInstance()->ScaleRect(rcImageSource);
	}
	//图片源容错处理
	if (rcImageSource.left < 0) {
		rcImageSource.left = 0;
	}
	if (rcImageSource.top < 0) {
		rcImageSource.top = 0;
	}
	if (rcImageSource.right > (LONG)pBitmap->GetWidth()) {
		rcImageSource.right = pBitmap->GetWidth();
	}
	if (rcImageSource.bottom > (LONG)pBitmap->GetHeight()) {
		rcImageSource.bottom = pBitmap->GetHeight();
	}
	bool bAlphaChannel = pBitmap->IsAlphaBitmap();

	SkPaint skPaint = *m_pSkPaint;
	if (uFade != 0xFF) {
		skPaint.setAlpha(uFade);
	}
	if (m_bTransparent || bAlphaChannel || (uFade != 0xFF)) {
		skPaint.setBlendMode(SkBlendMode::kSrcOver);
	}

	// middle
	rcDest.left = rcImageDest.left + rcDpiCorner.left;
	rcDest.top = rcImageDest.top + rcDpiCorner.top;
	rcDest.right = rcImageDest.right - rcDpiCorner.right;
	rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;

	rcSource.left = rcImageSource.left + rcImageCorners.left;
	rcSource.top = rcImageSource.top + rcImageCorners.top;
	rcSource.right = rcImageSource.right - rcImageCorners.right;
	rcSource.bottom = rcImageSource.bottom - rcImageCorners.bottom;
	if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
		if (!xtiled && !ytiled) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
		else if (xtiled && ytiled) {
			const LONG imageDrawWidth = rcImageSource.right - rcImageSource.left - rcImageCorners.left - rcImageCorners.right;
			const LONG imageDrawHeight = rcImageSource.bottom - rcImageSource.top - rcImageCorners.top - rcImageCorners.bottom;
			int iTimesX = (rcDest.right - rcDest.left) / (imageDrawWidth + nTiledMargin);
			if (!fullxtiled) {
				if ((rcDest.right - rcDest.left) % (imageDrawWidth + nTiledMargin) > 0) {
					iTimesX += 1;
				}
			}
			int iTimesY = (rcDest.bottom - rcDest.top) / (imageDrawHeight + nTiledMargin);
			if (!fullytiled) {
				if ((rcDest.bottom - rcDest.top) % (imageDrawHeight + nTiledMargin) > 0) {
					iTimesY += 1;
				}
			}
			for (int j = 0; j < iTimesY; ++j) {
				LONG lDestTop = rcDest.top + j * imageDrawHeight + j * nTiledMargin;
				LONG lDestBottom = lDestTop + imageDrawHeight;
				LONG lDrawHeight = imageDrawHeight;
				if (lDestBottom > rcDest.bottom) {
					lDrawHeight -= lDestBottom - rcDest.bottom;
					lDestBottom = rcDest.bottom;
				}
				for (int i = 0; i < iTimesX; ++i) {
					LONG lDestLeft = rcDest.left + i * imageDrawWidth + i * nTiledMargin;
					LONG lDestRight = lDestLeft + imageDrawWidth;
					LONG lDrawWidth = imageDrawWidth;
					if (lDestRight > rcDest.right) {
						lDrawWidth -= (lDestRight - rcDest.right);
						lDestRight = rcDest.right;
					}

					rcSource.left = rcImageSource.left + rcImageCorners.left;
					rcSource.top = rcImageSource.top + rcImageCorners.top;
					rcSource.right = rcSource.left + lDrawWidth;
					rcSource.bottom = rcSource.top + lDrawHeight;

					UiRect rcDestTemp;
					rcDestTemp.left = lDestLeft;
					rcDestTemp.right = lDestRight;
					rcDestTemp.top = lDestTop;
					rcDestTemp.bottom = lDestBottom;
					DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcSource, skPaint);
				}
			}
		}
		else if (xtiled) { // supp
			const LONG imageDrawWidth = rcImageSource.right - rcImageSource.left - rcImageCorners.left - rcImageCorners.right;
			int iTimes = iTimes = (rcDest.right - rcDest.left) / (imageDrawWidth + nTiledMargin);
			if (!fullxtiled) {
				if ((rcDest.right - rcDest.left) % (imageDrawWidth + nTiledMargin) > 0) {
					iTimes += 1;
				}
			}

			for (int i = 0; i < iTimes; ++i) {
				LONG lDestLeft = rcDest.left + i * imageDrawWidth + i * nTiledMargin;
				LONG lDestRight = lDestLeft + imageDrawWidth;
				LONG lDrawWidth = imageDrawWidth;
				if (lDestRight > rcDest.right) {
					lDrawWidth -= (lDestRight - rcDest.right);
					lDestRight = rcDest.right;
				}

				//源区域：如果设置了边角，则仅包含中间区域
				rcSource.left = rcImageSource.left + rcImageCorners.left;
				rcSource.top = rcImageSource.top + rcImageCorners.top;
				rcSource.right = rcSource.left + lDrawWidth;
				rcSource.bottom = rcImageSource.bottom - rcImageCorners.bottom;

				UiRect rcDestTemp = rcDest;
				rcDestTemp.top = rcDest.top;
				rcDestTemp.bottom = rcDest.top + rcSource.GetHeight();
				rcDestTemp.left = lDestLeft;
				rcDestTemp.right = lDestRight;

				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcSource, skPaint);
			}
		}
		else { // ytiled
			const LONG imageDrawHeight = rcImageSource.bottom - rcImageSource.top - rcImageCorners.top - rcImageCorners.bottom;
			int iTimes = (rcDest.bottom - rcDest.top) / (imageDrawHeight + nTiledMargin);
			if (!fullytiled) {
				if ((rcDest.bottom - rcDest.top) % (imageDrawHeight + nTiledMargin) > 0) {
					iTimes += 1;
				}
			}

			UiRect rcDestTemp;
			rcDestTemp.left = rcDest.left;			

			for (int i = 0; i < iTimes; ++i) {
				LONG lDestTop = rcDest.top + i * imageDrawHeight + i*nTiledMargin;
				LONG lDestBottom = lDestTop + imageDrawHeight;
				LONG lDrawHeight = imageDrawHeight;
				if (lDestBottom > rcDest.bottom) {
					lDrawHeight -= lDestBottom - rcDest.bottom;
					lDestBottom = rcDest.bottom;
				}

				rcSource.left = rcImageSource.left + rcImageCorners.left;
				rcSource.top = rcImageSource.top + rcImageCorners.top;
				rcSource.right = rcImageSource.right - rcImageCorners.right;
				rcSource.bottom = rcSource.top + lDrawHeight;

				rcDestTemp.right = rcDest.left + rcSource.GetWidth();
				rcDestTemp.top = lDestTop;
				rcDestTemp.bottom = lDestBottom;
				
				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skImage, rcSource, skPaint);
			}
		}
	}

	// left-top
	if (rcImageCorners.left > 0 && rcImageCorners.top > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.left + rcImageCorners.left;
		rcSource.bottom = rcImageSource.top + rcImageCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// top
	if (rcImageCorners.top > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left + rcImageCorners.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right - rcImageCorners.right;
		rcSource.bottom = rcImageSource.top + rcImageCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// right-top
	if (rcImageCorners.right > 0 && rcImageCorners.top > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.right - rcImageCorners.right;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.top + rcImageCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// left
	if (rcImageCorners.left > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top + rcImageCorners.top;
		rcSource.right = rcImageSource.left + rcImageCorners.left;
		rcSource.bottom = rcImageSource.bottom - rcImageCorners.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// right
	if (rcImageCorners.right > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.right - rcImageCorners.right;
		rcSource.top = rcImageSource.top + rcImageCorners.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom - rcImageCorners.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// left-bottom
	if (rcImageCorners.left > 0 && rcImageCorners.bottom > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.bottom - rcImageCorners.bottom;
		rcSource.right = rcImageSource.left + rcImageCorners.left;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// bottom
	if (rcImageCorners.bottom > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left + rcImageCorners.left;
		rcSource.top = rcImageSource.bottom - rcImageCorners.bottom;
		rcSource.right = rcImageSource.right - rcImageCorners.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
	// right-bottom
	if (rcImageCorners.right > 0 && rcImageCorners.bottom > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.right - rcImageCorners.right;
		rcSource.top = rcImageSource.bottom - rcImageCorners.bottom;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skImage, rcSource, skPaint);
		}
	}
}

void Render_Skia::DrawImageRect(const UiRect& rcPaint,
								IBitmap* pBitmap,
								const UiRect& rcImageDest,
								UiRect rcImageSource,
								bool bBitmapDpiScaled,
								uint8_t uFade,
								IMatrix* pMatrix)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	if (pMatrix == nullptr) {
		//仅在没有Matrix的情况下判断裁剪区域，
		//因为有Matrix时，实际绘制区域并不是rcImageDest，而是变换过后的位置，需要调整判断方法
		UiRect rcTestTemp;
		if (!::IntersectRect(&rcTestTemp, &rcImageDest, &rcPaint)) {
			return;
		}
	}

	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return;
	}
	// 如果源位图已经按照DPI缩放过，那么对应的rcImageSource也需要缩放
	if ((rcImageSource.left < 0) ||
		(rcImageSource.top < 0) ||
		(rcImageSource.right < 0) ||
		(rcImageSource.bottom < 0)) {
		//如果是无效值，则重置为整个图片大小
		rcImageSource.left = 0;
		rcImageSource.top = 0;
		rcImageSource.right = pBitmap->GetWidth();
		rcImageSource.bottom = pBitmap->GetHeight();
	}
	else if (bBitmapDpiScaled) {
		//如果外部设置此值，做DPI自适应处理
		DpiManager::GetInstance()->ScaleRect(rcImageSource);
	}
	//图片源容错处理
	if (rcImageSource.left < 0) {
		rcImageSource.left = 0;
	}
	if (rcImageSource.top < 0) {
		rcImageSource.top = 0;
	}
	if (rcImageSource.right > (LONG)pBitmap->GetWidth()) {
		rcImageSource.right = pBitmap->GetWidth();
	}
	if (rcImageSource.bottom > (LONG)pBitmap->GetHeight()) {
		rcImageSource.bottom = pBitmap->GetHeight();
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
	DrawFunction(m_pSkCanvas, rcImageDest, *m_pSkPointOrg, skImage, rcImageSource, skPaint);
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

void Render_Skia::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int nWidth)
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

void Render_Skia::DrawRect(const UiRect& rc, UiColor penColor, int nWidth)
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
	switch (dashStyle) {
	case IPen::kDashStyleSolid:
	{
		SkScalar intervals[] = { 1.0f, 0.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDash:
	{
		SkScalar intervals[] = { 5.0f,5.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDot:
	{
		SkScalar intervals[] = { 1.0f,4.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 2, 0.0f);
		break;
	}
	case IPen::kDashStyleDashDot:
	{
		SkScalar intervals[] = { 4.0f,1.0f,1.0f,1.0f };
		skPathEffect = SkDashPathEffect::Make(intervals, 4, 0.0f);
		break;
	}
	case IPen::kDashStyleDashDotDot:
	{
		SkScalar intervals[] = { 4.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
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

/** 生成Skia字体信息
*/
sk_sp<SkTypeface> MakeSkFont(IFont* pFont)
{
	ASSERT(pFont != nullptr);
	if (pFont == nullptr){
		return nullptr;
	}
	SkFontStyle fontStyle = SkFontStyle(pFont->FontWeight(), pFont->FontWidth(), pFont->IsItalic() ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant);
	std::string fontName; //UTF8编码的字体名称
	StringHelper::UnicodeToMBCS(pFont->FontName(), fontName, CP_UTF8);
	return SkTypeface::MakeFromName(fontName.c_str(), fontStyle);
}

void Render_Skia::DrawString(const UiRect& rc, 
							 const std::wstring& strText,
	                         UiColor dwTextColor, 
	                         const std::wstring& strFontId, 
	                         uint32_t uFormat, 
	                         uint8_t uFade /*= 255*/)
{
	PerformanceStat statPerformance(L"Render_Skia::MeasureString");
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
	
	//字体设置
	IFont* pFont = GlobalManager::GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return;
	}

	SkFont skFont;
	skFont.setTypeface(MakeSkFont(pFont));
	skFont.setSize(SkIntToScalar(std::abs(pFont->FontSize())));
	skFont.setEdging(SkFont::Edging::kSubpixelAntiAlias);
	
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
		           skFont,
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

	//字体设置
	IFont* pFont = GlobalManager::GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return UiRect();
	}	

	SkFont skFont;
	skFont.setTypeface(MakeSkFont(pFont));
	skFont.setSize(SkIntToScalar(std::abs(pFont->FontSize())));
	skFont.setEdging(SkFont::Edging::kAntiAlias);

	//绘制属性设置
	SkPaint skPaint = *m_pSkPaint;

	bool isSingleLineMode = false;
	if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
		isSingleLineMode = true;
	}
		
	//计算行高
	SkFontMetrics fontMetrics;
	SkScalar fontHeight = skFont.getMetrics(&fontMetrics);

	if (isSingleLineMode || (width <= 0)) {
		//单行模式, 或者没有限制宽度
		SkScalar textWidth = skFont.measureText(strText.c_str(),
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
													  skFont,
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
	srcRc.setXYWH((SkScalar)destRc.left, (SkScalar)destRc.top, (SkScalar)destRc.GetWidth(), (SkScalar)destRc.GetHeight());

	//原始区域，做裁剪用
	SkRect excludeRc;
	excludeRc.setXYWH((SkScalar)rc.left, (SkScalar)rc.top, (SkScalar)rc.GetWidth(), (SkScalar)rc.GetHeight());

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
