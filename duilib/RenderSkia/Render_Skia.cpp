#include "Render_Skia.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/BitmapAlpha.h"

#include "duilib/RenderSkia/Bitmap_Skia.h"
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

#ifdef _DEBUG
#pragma comment (lib, "../../../../develop/skia/skia/out/Debug/skia.lib")
#else
#pragma comment (lib, "../../../../develop/skia/skia/out/StaticLLVM/skia.lib")
#endif
#pragma comment (lib, "opengl32.lib")

namespace ui {

static inline void DrawFunction(SkCanvas* pSkCanvas, 
	                            const UiRect& rcDest,
								const SkPoint& skPointOrg,
								const SkBitmap& skSrcBitmap,
								const UiRect& rcSrc,
	                            const SkPaint& skPaint)
{
	SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(skPointOrg);

	SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
	SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

	pSkCanvas->drawImageRect(skSrcBitmap.asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kStrict_SrcRectConstraint);
}

Render_Skia::Render_Skia()
	: m_bTransparent(false),
	m_pSkCanvas(nullptr),
	m_hDC(nullptr)
{
	m_pSkPointOrg = new SkPoint;
	m_pSkPointOrg->iset(0, 0);
	m_pSkPaint = new SkPaint;
	m_pSkPaint->setAntiAlias(true);
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
		Clear();
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

void Render_Skia::ClearAlpha(const UiRect& rcDirty, int alpha)
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

void Render_Skia::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha)
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

void Render_Skia::Save()
{
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->save();
	}
}

void Render_Skia::Restore()
{
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->restore();
	}
}

UiPoint Render_Skia::OffsetWindowOrg(UiPoint ptOffset)
{
	UiPoint ptOldWindowOrg = { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY) };
	SkPoint ptOff = { SkIntToScalar(ptOffset.x), SkIntToScalar(ptOffset.y) };
	m_pSkPointOrg->offset(ptOff.fX, ptOff.fY);
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
	UiPoint ptOldWindowOrg = { SkScalarTruncToInt(m_pSkPointOrg->fX), SkScalarTruncToInt(m_pSkPointOrg->fY) };
	return ptOldWindowOrg;
}

void Render_Skia::SetClip(const UiRect& rc)
{
	SkIRect rcSkI = { rc.left, rc.top, rc.right, rc.bottom };
	SkRect rcSk = SkRect::Make(rcSkI);
	rcSk.offset(*m_pSkPointOrg);
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->save();
		m_pSkCanvas->clipRect(rcSk, false);
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

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, Bitmap_Skia* pSrcBitmap, int xSrc, int yScr, RopMode rop)
{
	Bitmap_Skia* skiaBitmap = pSrcBitmap;
	ASSERT(skiaBitmap != nullptr);
	if (skiaBitmap == nullptr) {
		return false;
	}

	SkPaint skPaint = *m_pSkPaint;
	skPaint.setStyle(SkPaint::kFill_Style);
	SetRopMode(skPaint, rop);

	SkIRect rcSkDestI = { x, y, x + cx, y + cy };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(*m_pSkPointOrg);

	SkIRect rcSkSrcI = { xSrc, yScr, xSrc + cx, yScr + cy };
	SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawImageRect(skiaBitmap->GetSkBitmap().asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
		return true;
	}
	return false;
}

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, IBitmap* pSrcBitmap, int xSrc, int yScr, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcBitmap != nullptr);
	if (pSrcBitmap == nullptr) {
		return false;
	}
	Bitmap_Skia* skiaBitmap = dynamic_cast<Bitmap_Skia*>(pSrcBitmap);
	return BitBlt(x, y, cx, cy, skiaBitmap, xSrc, yScr, rop);
}

bool Render_Skia::BitBlt(int x, int y, int cx, int cy, IRender* pSrcRender, int xSrc, int yScr, RopMode rop)
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
	return BitBlt(x, y, cx, cy, skiaBitmap, xSrc, yScr, rop);
}

bool Render_Skia::StretchBlt(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int yScr, int widthSrc, int heightSrc, RopMode rop)
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

	SkIRect rcSkDestI = { xDest, yDest, xDest + widthDest, yDest + heightDest };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(*m_pSkPointOrg);

	SkIRect rcSkSrcI = { xSrc, yScr, xSrc + widthSrc, yScr + heightSrc };
	SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawImageRect(skiaBitmap->GetSkBitmap().asImage(), rcSkSrc, rcSkDest, SkSamplingOptions(), &skPaint, SkCanvas::kFast_SrcRectConstraint);
		return true;
	}
	return false;

}

bool Render_Skia::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int yScr, int widthSrc, int heightSrc, uint8_t alpha)
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

	SkIRect rcSkDestI = { xDest, yDest, xDest + widthDest, yDest + heightDest };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(*m_pSkPointOrg);

	SkIRect rcSkSrcI = { xSrc, yScr, xSrc + widthSrc, yScr + heightSrc };
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
		//skPaint.setBlender(SkBlender::Mode(SkBlendMode::kSrcOver));
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
					DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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

				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
				
				DrawFunction(m_pSkCanvas, rcDestTemp, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
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
			DrawFunction(m_pSkCanvas, rcDest, *m_pSkPointOrg, skSrcBitmap, rcSource, skPaint);
		}
	}
}

void Render_Skia::DrawColor(const UiRect& rc, UiColor dwColor, uint8_t uFade)
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

void Render_Skia::DrawPath(const IPath* path, const IPen* pen)
{
	return;
	ASSERT(path != nullptr);
	ASSERT(pen != nullptr);
	if ((path == nullptr) || (pen == nullptr)) {
		return;
	}

}

void Render_Skia::FillPath(const IPath* path, const IBrush* brush)
{

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
	//DrawColor(rc, dwTextColor, uFade);
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(!strText.empty());
	if (strText.empty()) {
		return;
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return;
	}

	IFont* pFont = GlobalManager::GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return;
	}

	SkFont skFont;
	skFont.setTypeface(MakeSkFont(pFont));
	skFont.setSize(SkIntToScalar(std::abs(pFont->FontSize())));
	skFont.setEdging(SkFont::Edging::kAntiAlias);

	SkTextEncoding textEncoding = SkTextEncoding::kUTF16;
	
	SkPaint skPaint = *m_pSkPaint;
	skPaint.setARGB(dwTextColor.GetA(), dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB());
	if (uFade != 0xFF) {
		skPaint.setAlpha(uFade);
	}

	SkIRect rcSkDestI = { rc.left, rc.top, rc.right, rc.bottom };
	SkRect rcSkDest = SkRect::Make(rcSkDestI);
	rcSkDest.offset(*m_pSkPointOrg);

	SkScalar textWidth = skFont.measureText(strText.c_str(), strText.size() * sizeof(std::wstring::value_type), textEncoding, nullptr);
	SkFontMetrics fontMetrics;
	skFont.getMetrics(&fontMetrics);

	SkScalar x = rcSkDest.fLeft;//默认值：左对齐
	SkScalar y = rcSkDest.fTop + std::fabs(fontMetrics.fAscent);//默认值：上对齐

	if (uFormat & DrawStringFormat::TEXT_SINGLELINE) {
		//单行文本， 暂不支持

	}
	if (uFormat & DrawStringFormat::TEXT_END_ELLIPSIS) {
		//暂不支持
	}
	if (uFormat & DrawStringFormat::TEXT_NOCLIP) {
		//不支持
	}

	if (uFormat & DrawStringFormat::TEXT_CENTER) {
		//横向对齐：居中对齐
		x = rcSkDest.fLeft + (rcSkDest.width() / 2) - textWidth / 2;
		if (x < rcSkDest.fLeft) {
			x = rcSkDest.fLeft;
		}
	}
	else if (uFormat & DrawStringFormat::TEXT_RIGHT) {
		//横向对齐：右对齐
		x = rcSkDest.fRight - textWidth;
		if (x < rcSkDest.fLeft) {
			x = rcSkDest.fLeft;
		}
	}
	else {
		//DrawStringFormat::TEXT_LEFT（这个是0，不能通过与操作判断结果）
		//横向对齐：左对齐
		x = rcSkDest.fLeft;
	}

	if (uFormat & DrawStringFormat::TEXT_VCENTER) {
		//纵向对齐：居中对齐
		y = rcSkDest.centerY() + (fontMetrics.fBottom - fontMetrics.fTop) / 2 - fontMetrics.fBottom;
	}
	else if (uFormat & DrawStringFormat::TEXT_BOTTOM) {
		//纵向对齐：下对齐
		y = rcSkDest.fBottom - fontMetrics.fDescent - fontMetrics.fLeading;
	}
	else {
		//DrawStringFormat::TEXT_TOP（这个是0，不能通过与操作判断结果）
		//纵向对齐：上对齐
		y = rcSkDest.fTop + std::fabs(fontMetrics.fAscent);
	}

	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas != nullptr) {
		m_pSkCanvas->drawSimpleText(strText.c_str(), 
								    strText.size() * sizeof(std::wstring::value_type), 
									textEncoding,
			                        x,
			                        y,
			                        skFont,
			                        skPaint);
	}
}

ui::UiRect Render_Skia::MeasureString(const std::wstring& strText, 
								      const std::wstring& strFontId,
								      uint32_t uFormat, 
									  int width /*= DUI_NOSET_VALUE*/)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(!strText.empty());
	UiRect rc;
	if (strText.empty()) {
		return rc;
	}
	ASSERT(m_pSkCanvas != nullptr);
	if (m_pSkCanvas == nullptr) {
		return rc;
	}

	IFont* pFont = GlobalManager::GetIFont(strFontId);
	ASSERT(pFont != nullptr);
	if (pFont == nullptr) {
		return rc;
	}	

	SkFont skFont;
	skFont.setTypeface(MakeSkFont(pFont));
	skFont.setSize(SkIntToScalar(std::abs(pFont->FontSize())));
	skFont.setEdging(SkFont::Edging::kAntiAlias);

	SkTextEncoding textEncoding = SkTextEncoding::kUTF16;
	SkScalar textWidth = skFont.measureText(strText.c_str(), strText.size()*sizeof(std::wstring::value_type), textEncoding, nullptr);
	rc.left = 0;
	rc.right = SkScalarTruncToInt(textWidth) + 1;

	SkFontMetrics fontMetrics;
	skFont.getMetrics(&fontMetrics);
	rc.top = 0;
	rc.bottom = SkScalarTruncToInt(fontMetrics.fBottom - fontMetrics.fTop) + 1;
	return rc;
}

void Render_Skia::DrawBoxShadow(const UiRect& rc,
								   const UiSize& roundSize, 
						     	   const UiPoint& cpOffset, 
								   int nBlurRadius, 
								   int nSpreadRadius,
								   UiColor dwColor,
								   bool bExclude)
{
	return;
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));

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
		//ASSERT(!"no impl!");
		/*SkRegion rgn = m_pSkCanvas->getTotalClip();
		SkRegion::Iterator it(rgn);
		int nCount = 0;
		for (; !it.done(); it.next())
		{
			nCount++;
		}
		it.rewind();

		int nSize = sizeof(RGNDATAHEADER) + nCount * sizeof(RECT);
		RGNDATA* rgnData = (RGNDATA*)malloc(nSize);
		memset(rgnData, 0, nSize);
		rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
		rgnData->rdh.iType = RDH_RECTANGLES;
		rgnData->rdh.nCount = nCount;
		rgnData->rdh.rcBound.right = m_curBmp->Width();
		rgnData->rdh.rcBound.bottom = m_curBmp->Height();

		nCount = 0;
		LPRECT pRc = (LPRECT)rgnData->Buffer;
		for (; !it.done(); it.next())
		{
			SkIRect skrc = it.rect();
			RECT rc = { skrc.fLeft,skrc.fTop,skrc.fRight,skrc.fBottom };
			pRc[nCount++] = rc;
		}

		HRGN hRgn = ExtCreateRegion(NULL, nSize, rgnData);
		free(rgnData);
		::SelectClipRgn(m_hGetDC, hRgn);
		DeleteObject(hRgn);*/
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
