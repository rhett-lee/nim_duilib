#include "Render_GdiPlus.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/RenderGdiPlus/Pen_Gdiplus.h"
#include "duilib/RenderGdiPlus/Path_Gdiplus.h"
#include "duilib/RenderGdiPlus/Brush_Gdiplus.h"
#include "duilib/RenderGdiPlus/Matrix_Gdiplus.h"
#include "duilib/RenderGdiPlus/Bitmap_GDI.h"
#include "duilib/RenderGdiPlus/Font_GDI.h"
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"
#include "duilib/Render/BitmapAlpha.h"

namespace ui {

static inline void DrawFunction(HDC hDC, bool bTransparent, UiRect rcDest, HDC hdcSrc, UiRect rcSrc, bool bAlphaChannel, int uFade)
{
	if (bTransparent || bAlphaChannel || uFade < 255 
		|| (rcSrc.Width() == rcDest.Width() && rcSrc.Height() == rcDest.Height())) {
		BLENDFUNCTION ftn = { AC_SRC_OVER, 0, static_cast<uint8_t>(uFade), AC_SRC_ALPHA };
		::AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), ftn);
	}
	else {
		::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.Width(), rcSrc.Height(), SRCCOPY);
	}
}

Render_GdiPlus::Render_GdiPlus()
	: m_hDC(nullptr)
	, m_saveDC(0)
	, m_bTransparent(false)
	, m_hOldBitmap(nullptr)
	, m_hBitmap(nullptr)
	, m_pPiexl(nullptr)
	, m_nWidth(0)
	, m_nHeight(0)
{
	HDC hDC = ::GetDC(NULL);
	m_hDC = ::CreateCompatibleDC(hDC);
	::ReleaseDC(NULL, hDC);
	ASSERT(m_hDC != nullptr);
}

Render_GdiPlus::~Render_GdiPlus()
{
	if (m_hOldBitmap != nullptr) {
		::SelectObject(m_hDC, m_hOldBitmap);
		m_hOldBitmap = nullptr;
	}
	if (m_hDC != nullptr) {
		::DeleteDC(m_hDC);
		m_hDC = nullptr;
	}
	if (m_hBitmap != nullptr) {
		::DeleteObject(m_hBitmap);
		m_hBitmap = nullptr;
	}
}

RenderType Render_GdiPlus::GetRenderType() const
{
	return RenderType::kRenderType_GdiPlus;
}

HDC Render_GdiPlus::GetDC()
{
	return m_hDC;
}

void Render_GdiPlus::ReleaseDC(HDC hdc)
{
	ASSERT_UNUSED_VARIABLE(hdc == m_hDC);
}

bool Render_GdiPlus::Resize(int width, int height)
{
	ASSERT((width > 0) && (height > 0));
	if ((width <= 0) || (height <= 0)) {
		return false;
	}

	ASSERT(m_hDC != nullptr);
	if ((m_nWidth == width) && (m_nHeight == height)) {
		return true;
	}

	if (m_hOldBitmap != nullptr) {
		::SelectObject(m_hDC, m_hOldBitmap);
		m_hOldBitmap = nullptr;
	}
	if (m_hBitmap != nullptr) {
		::DeleteObject(m_hBitmap);
		m_hBitmap = nullptr;
	}
	m_pPiexl = nullptr;
	m_nWidth = width;
	m_nHeight = height;
	m_hBitmap = Bitmap_GDI::CreateBitmap(width, height, true, (LPVOID*)&m_pPiexl);
	ASSERT(m_pPiexl != nullptr);
	if (m_pPiexl == nullptr) {
		if (m_hBitmap != nullptr) {
			::DeleteObject(m_hBitmap);
			m_hBitmap = nullptr;
		}
	}
	if (m_hBitmap == nullptr) {
		m_pPiexl = nullptr;
		m_nWidth = 0;
		m_nHeight = 0;
		return false;
	}	
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, m_hBitmap);
	return true;
}

void Render_GdiPlus::Clear()
{
	//将位图数据清零
	if ((m_pPiexl != nullptr) && (m_nWidth > 0) && (m_nHeight > 0)) {
		::memset(m_pPiexl, 0, m_nWidth * m_nHeight * 4);
	}
}

std::unique_ptr<ui::IRender> Render_GdiPlus::Clone()
{
	std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_GdiPlus>();
	pClone->Resize(GetWidth(), GetHeight());
	pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this, 0, 0, RopMode::kSrcCopy);
	return pClone;
}

IBitmap* Render_GdiPlus::DetachBitmap()
{
	ASSERT(m_hOldBitmap != nullptr);
	ASSERT(GetHeight() > 0 && GetWidth() > 0);
	if (m_hOldBitmap != nullptr) {
		::SelectObject(m_hDC, m_hOldBitmap);
		m_hOldBitmap = nullptr;
	}
	if (m_hBitmap == nullptr) {
		return nullptr;
	}
	IBitmap* pBitmap = nullptr;	
	HBITMAP hBitmap = m_hBitmap;
	if (hBitmap != nullptr) {
		pBitmap = new Bitmap_GDI(hBitmap, true);
	}
	m_hBitmap = nullptr;
	m_pPiexl = nullptr;
	m_nWidth = 0;
	m_nHeight = 0;
	return pBitmap;
}

int Render_GdiPlus::GetWidth()
{
	return m_nWidth;
}

int Render_GdiPlus::GetHeight()
{
	return m_nHeight;
}

void Render_GdiPlus::ClearAlpha(const UiRect& rcDirty, uint8_t alpha)
{
	HBITMAP hBitmap = m_hBitmap;
	ASSERT(hBitmap != nullptr);
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.ClearAlpha(rcDirty, alpha);
	}
}

void Render_GdiPlus::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha)
{
	HBITMAP hBitmap = m_hBitmap;
	ASSERT(hBitmap != nullptr);
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding, alpha);
	}
}

void Render_GdiPlus::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding /*= UiRect()*/)
{
	HBITMAP hBitmap = m_hBitmap;
	ASSERT(hBitmap != nullptr);
	if (hBitmap != nullptr) {
		BITMAP bm = { 0 };
		::GetObject(hBitmap, sizeof(bm), &bm);
		BitmapAlpha bitmapAlpha((uint8_t*)bm.bmBits, bm.bmWidth, bm.bmHeight, bm.bmBitsPixel / 8);
		bitmapAlpha.RestoreAlpha(rcDirty, rcShadowPadding);
	}
}

bool Render_GdiPlus::IsRenderTransparent() const
{
	return m_bTransparent;
}

bool Render_GdiPlus::SetRenderTransparent(bool bTransparent)
{
	bool oldValue = m_bTransparent;
	m_bTransparent = bTransparent;
	return oldValue;
}

UiPoint Render_GdiPlus::OffsetWindowOrg(UiPoint ptOffset)
{
	POINT pt = {0, 0};
	::GetWindowOrgEx(m_hDC, &pt);
	UiPoint ptOldWindowOrg(pt.x, pt.y);
	ptOffset.Offset(ptOldWindowOrg.x, ptOldWindowOrg.y);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return ptOldWindowOrg;
}

UiPoint Render_GdiPlus::SetWindowOrg(UiPoint ptOffset)
{
	POINT pt = { 0, 0 };
	::GetWindowOrgEx(m_hDC, &pt);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return UiPoint(pt.x, pt.y);
}

UiPoint Render_GdiPlus::GetWindowOrg() const
{
	POINT pt = {0, 0};
	::GetWindowOrgEx(m_hDC, &pt);
	return UiPoint(pt.x, pt.y);
}

void Render_GdiPlus::SaveClip(int& nState)
{
	m_saveDC = ::SaveDC(m_hDC);
	nState = m_saveDC;
}

void Render_GdiPlus::RestoreClip(int nState)
{
	ASSERT(m_saveDC == nState);
	if (nState != m_saveDC) {
		return;
	}
	if (m_saveDC != 0) {
		::RestoreDC(m_hDC, m_saveDC);
		m_saveDC = 0;
	}
	else {
		::RestoreDC(m_hDC, -1);
	}
}

void Render_GdiPlus::SetClip(const UiRect& rc, bool bIntersect)
{
	HDC hDC = m_hDC;
	UiRect rcItem = rc;

	POINT ptWinOrg = {0, 0};
	::GetWindowOrgEx(hDC, &ptWinOrg);	
	rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);
	RECT itemRect;
	itemRect.left = rcItem.left;
	itemRect.top = rcItem.top;
	itemRect.right = rcItem.right;
	itemRect.bottom = rcItem.bottom;
	HRGN hRgn = ::CreateRectRgnIndirect(&itemRect);
	::SaveDC(hDC);
	::ExtSelectClipRgn(hDC, hRgn, bIntersect ? RGN_AND : RGN_DIFF);
	::DeleteObject(hRgn);
}

void Render_GdiPlus::SetRoundClip(const UiRect& rc, int width, int height, bool bIntersect)
{
	HDC hDC = m_hDC;
	UiRect rcItem = rc;

	POINT ptWinOrg = {0, 0};
	::GetWindowOrgEx(hDC, &ptWinOrg);
	rcItem.Offset(-ptWinOrg.x, -ptWinOrg.y);

	HRGN hRgn = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, width, height);
	::SaveDC(hDC);
	::ExtSelectClipRgn(hDC, hRgn, bIntersect ? RGN_AND : RGN_DIFF);
	::DeleteObject(hRgn);
}

void Render_GdiPlus::ClearClip()
{
	HDC hDC = m_hDC;
	ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
	::RestoreDC(hDC, -1);
}

DWORD Render_GdiPlus::GetRopMode(RopMode rop) const
{
	DWORD ropMode = SRCCOPY;
	switch (rop) {
	case RopMode::kSrcCopy:
		ropMode = SRCCOPY;
		break;
	case RopMode::kDstInvert:
		ropMode = DSTINVERT;
		break;
	case RopMode::kSrcInvert:
		ropMode = SRCINVERT;
		break;
	case RopMode::kSrcAnd:
		ropMode = SRCAND;
		break;
	default:
		break;
	}
	return ropMode;
}

bool Render_GdiPlus::BitBlt(int x, int y, int cx, int cy, IBitmap* pSrcBitmap, int xSrc, int ySrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcBitmap != nullptr);
	if (pSrcBitmap == nullptr) {
		return false;
	}
	Bitmap_GDI* gdiBitmap = dynamic_cast<Bitmap_GDI*>(pSrcBitmap);
	ASSERT(gdiBitmap != nullptr);
	if (gdiBitmap == nullptr) {
		return false;
	}
	HBITMAP hBitmap = gdiBitmap->GetHBitmap();
	ASSERT(hBitmap != nullptr);
	if (hBitmap == nullptr) {
		return false;
	}
	ASSERT(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);

	HDC hCloneDC = ::CreateCompatibleDC(m_hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	bool bResult = ::BitBlt(m_hDC, x, y, cx, cy, hCloneDC, xSrc, ySrc, GetRopMode(rop)) != FALSE;
	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
	return bResult;
}

bool Render_GdiPlus::BitBlt(int x, int y, int cx, int cy, IRender* pSrcRender, int xSrc, int ySrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	bool bResult = false;
	if (pSrcRender != nullptr) {
		HDC hdcSrc = pSrcRender->GetDC();
		ASSERT(hdcSrc != nullptr);
		bResult = ::BitBlt(m_hDC, x, y, cx, cy, hdcSrc, xSrc, ySrc, GetRopMode(rop)) != FALSE;
		pSrcRender->ReleaseDC(hdcSrc);
	}
	return bResult;
}

bool Render_GdiPlus::StretchBlt(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, RopMode rop)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	bool bResult = false;
	if (pSrcRender != nullptr) {
		HDC hdcSrc = pSrcRender->GetDC();
		ASSERT(hdcSrc != nullptr);
		int stretchBltMode = ::SetStretchBltMode(m_hDC, HALFTONE);
		bResult = ::StretchBlt(m_hDC, xDest, yDest, widthDest, heightDest,
							   hdcSrc, xSrc, ySrc, widthSrc, heightSrc, GetRopMode(rop)) != FALSE;
		::SetStretchBltMode(m_hDC, stretchBltMode);
		pSrcRender->ReleaseDC(hdcSrc);
	}
	return bResult;
}

bool Render_GdiPlus::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, IRender* pSrcRender, int xSrc, int ySrc, int widthSrc, int heightSrc, uint8_t alpha)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(pSrcRender != nullptr);
	bool bResult = false;
	if (pSrcRender != nullptr) {
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
		HDC hdcSrc = pSrcRender->GetDC();
		ASSERT(hdcSrc != nullptr);
		bResult =::AlphaBlend(m_hDC, xDest, yDest, widthDest, heightDest, hdcSrc, xSrc, ySrc, widthSrc, heightSrc, bf) != FALSE;
		pSrcRender->ReleaseDC(hdcSrc);
	}
	return bResult;
}

void Render_GdiPlus::DrawImage(const UiRect& rcPaint,
							   IBitmap* pBitmap,
							   const UiRect& rcDest,
							   const UiRect& rcDestCorners,
							   const UiRect& rcSource,
							   const UiRect& rcSourceCorners,
							   uint8_t uFade,
							   bool xtiled,
							   bool ytiled,
							   bool fullxtiled,
							   bool fullytiled,
							   int nTiledMargin)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	UiRect rcTestTemp;
	if (!UiRect::Intersect(rcTestTemp, rcDest, rcPaint)) {
		return;
	}

	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return;
	}
	Bitmap_GDI* gdiBitmap = dynamic_cast<Bitmap_GDI*>(pBitmap);
	ASSERT(gdiBitmap != nullptr);
	if (gdiBitmap == nullptr) {
		return;
	}
	HBITMAP hBitmap = gdiBitmap->GetHBitmap();
	ASSERT(hBitmap != nullptr);
	if (hBitmap == nullptr) {
		return;
	}

	ASSERT(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);

	HDC hCloneDC = ::CreateCompatibleDC(m_hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	int stretchBltMode = ::SetStretchBltMode(m_hDC, HALFTONE);

	UiRect rcTemp;
	UiRect rcDrawSource;
	UiRect rcDrawDest;

	bool bAlphaChannel = pBitmap->IsAlphaBitmap();

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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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

					DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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

				DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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

				DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
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
			DrawFunction(m_hDC, m_bTransparent, rcDrawDest, hCloneDC, rcDrawSource, bAlphaChannel, uFade);
		}
	}

	::SetStretchBltMode(m_hDC, stretchBltMode);
	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void Render_GdiPlus::DrawImage(const UiRect& rcPaint, IBitmap* pBitmap,
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

void Render_GdiPlus::DrawImageRect(const UiRect& rcPaint,
								   IBitmap* pBitmap,
								   const UiRect& rcDest,
								   const UiRect& rcSource,
							       uint8_t uFade,
								   IMatrix* pMatrix)
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

	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	if (pMatrix != nullptr) {
		Matrix_Gdiplus* pGdiplusMatrix = dynamic_cast<Matrix_Gdiplus*>(pMatrix);
		if (pGdiplusMatrix != nullptr) {
			graphics.SetTransform(pGdiplusMatrix->GetMatrix());
		}
	}
		
	Gdiplus::RectF rectf;
	rectf.X = static_cast<Gdiplus::REAL>(rcDest.left);
	rectf.Y = static_cast<Gdiplus::REAL>(rcDest.top);
	rectf.Width = static_cast<Gdiplus::REAL>(rcDest.Width());
	rectf.Height = static_cast<Gdiplus::REAL>(rcDest.Height());

	Gdiplus::REAL srcx = (Gdiplus::REAL)rcSource.left;
	Gdiplus::REAL srcy = (Gdiplus::REAL)rcSource.top;
	Gdiplus::REAL srcWidth = (Gdiplus::REAL)rcSource.Width();
	Gdiplus::REAL srcHeight = (Gdiplus::REAL)rcSource.Height();
	Gdiplus::Unit srcUnit = Gdiplus::UnitPixel;

	void* data = pBitmap->LockPixelBits();
	Gdiplus::Bitmap gdiplusBitmap(pBitmap->GetWidth(),
							      pBitmap->GetHeight(), 
		                          pBitmap->GetWidth() * 4,
		                          PixelFormat32bppARGB,
		                          (BYTE*)data);
	if (uFade < 255) {
		//设置了透明度
		float alpha = (float)uFade / 255;
		Gdiplus::ColorMatrix cm = { 1,0,0,0,0,
									0,1,0,0,0,
									0,0,1,0,0,
									0,0,0,alpha,0,
									0,0,0,0,1 };
		Gdiplus::ImageAttributes imageAttr;
		imageAttr.SetColorMatrix(&cm);
		graphics.DrawImage(&gdiplusBitmap, rectf, srcx, srcy, srcWidth, srcHeight, srcUnit, &imageAttr);
	}
	else {
		graphics.DrawImage(&gdiplusBitmap, rectf, srcx, srcy, srcWidth, srcHeight, srcUnit);
	}	
	pBitmap->UnLockPixelBits();
}

void Render_GdiPlus::FillRect(const UiRect& rc, UiColor dwColor, uint8_t uFade)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	UiColor::ARGB dwNewColor = dwColor.GetARGB();
	if (uFade < 255) {
		//在原来颜色值的透明度基础上，在做一次透明度计算（uFade是在原来基础上在设置透明度）
		int alpha = dwColor.GetARGB() >> 24;
		dwNewColor = dwColor.GetARGB() & 0xffffff;
		alpha = static_cast<int>(static_cast<double>(alpha) * static_cast<double>(uFade) / 255.0);
		dwNewColor += alpha << 24;
	}

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Color color(dwNewColor);
	Gdiplus::SolidBrush brush(color);
	Gdiplus::RectF rcFill(static_cast<Gdiplus::REAL>(rc.left), static_cast<Gdiplus::REAL>(rc.top), 
		                  static_cast<Gdiplus::REAL>(rc.Width()), static_cast<Gdiplus::REAL>(rc.Height()));
	graphics.FillRectangle(&brush, rcFill);
}

void Render_GdiPlus::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int32_t nWidth)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);
	graphics.DrawLine(&pen, Gdiplus::Point(pt1.x, pt1.y), Gdiplus::Point(pt2.x, pt2.y));
}

void Render_GdiPlus::DrawLine(const UiPointF& pt1, const UiPointF& pt2, UiColor penColor, float fWidth)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)fWidth);
	graphics.DrawLine(&pen, Gdiplus::PointF(pt1.x, pt1.y), Gdiplus::PointF(pt2.x, pt2.y));
}

void Render_GdiPlus::DrawLine(const UiPoint& pt1, const UiPoint& pt2, IPen* pen)
{
	ASSERT(pen != nullptr);
	if (pen == nullptr) {
		return;
	}
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	graphics.DrawLine(((Pen_GdiPlus*)pen)->GetPen(), Gdiplus::Point(pt1.x, pt1.y), Gdiplus::Point(pt2.x, pt2.y));
}

void Render_GdiPlus::DrawRect(const UiRect& rc, UiColor penColor, int32_t nWidth, bool /*bLineInRect*/)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);
	graphics.DrawRectangle(&pen, rc.left, rc.top, rc.Width(), rc.Height());
}

void Render_GdiPlus::DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);

	Gdiplus::GraphicsPath pPath;
	pPath.AddArc((INT)rc.left, rc.top, roundSize.cx, roundSize.cy, 180, 90);
	pPath.AddLine(rc.left + roundSize.cx, (INT)rc.top, rc.right - roundSize.cx, rc.top);
	pPath.AddArc(rc.right - roundSize.cx, (INT)rc.top, roundSize.cx, roundSize.cy, 270, 90);
	pPath.AddLine((INT)rc.right, rc.top + roundSize.cy, rc.right, rc.bottom - roundSize.cy);
	pPath.AddArc(rc.right - roundSize.cx, rc.bottom - roundSize.cy, (INT)roundSize.cx, roundSize.cy, 0, 90);
	pPath.AddLine(rc.right - roundSize.cx, (INT)rc.bottom, rc.left + roundSize.cx, rc.bottom);
	pPath.AddArc((INT)rc.left, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 90, 90);
	pPath.AddLine((INT)rc.left, rc.bottom - roundSize.cy, rc.left, rc.top + roundSize.cy);
	pPath.CloseFigure();

	graphics.DrawPath(&pen, &pPath);
}

void Render_GdiPlus::FillRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor dwColor, uint8_t uFade)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	UiColor::ARGB dwNewColor = dwColor.GetARGB();
	if (uFade < 255) {
        //在原来颜色值的透明度基础上，在做一次透明度计算（uFade是在原来基础上在设置透明度）
		int alpha = dwColor.GetARGB() >> 24;
		dwNewColor = dwColor.GetARGB() & 0xffffff;
		alpha = static_cast<int>(static_cast<double>(alpha) * static_cast<double>(uFade) / 255.0);
		dwNewColor += alpha << 24;
	}
	Gdiplus::Color color(dwNewColor);
	Gdiplus::SolidBrush brush(color);

	Gdiplus::GraphicsPath pPath;
	pPath.AddArc((INT)rc.left, rc.top, roundSize.cx, roundSize.cy, 180, 90);
	pPath.AddLine(rc.left + roundSize.cx, (INT)rc.top, rc.right - roundSize.cx, rc.top);
	pPath.AddArc(rc.right - roundSize.cx, (INT)rc.top, roundSize.cx, roundSize.cy, 270, 90);
	pPath.AddLine((INT)rc.right, rc.top + roundSize.cy, rc.right, rc.bottom - roundSize.cy);
	pPath.AddArc(rc.right - roundSize.cx, rc.bottom - roundSize.cy, (INT)roundSize.cx, roundSize.cy, 0, 90);
	pPath.AddLine(rc.right - roundSize.cx, (INT)rc.bottom, rc.left + roundSize.cx, rc.bottom);
	pPath.AddArc((INT)rc.left, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 90, 90);
	pPath.AddLine((INT)rc.left, rc.bottom - roundSize.cy, rc.left, rc.top + roundSize.cy);
	pPath.CloseFigure();

	graphics.FillPath(&brush, &pPath);
}

void Render_GdiPlus::DrawCircle(const UiPoint& /*centerPt*/, int32_t /*radius*/, UiColor /*penColor*/, int /*nWidth*/)
{
	ASSERT(!"No Impl!");
}

void Render_GdiPlus::FillCircle(const UiPoint& /*centerPt*/, int32_t /*radius*/, UiColor /*dwColor*/, uint8_t /*uFade*/)
{
	ASSERT(!"No Impl!");
}

void Render_GdiPlus::DrawArc(const UiRect& rc, float startAngle, float sweepAngle, bool /*useCenter*/, 
							 const IPen* pen,
	                         UiColor* gradientColor, const UiRect* gradientRect)
{
	ASSERT(pen != nullptr);
	if (pen == nullptr) {
		return;
	}
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	Gdiplus::RectF rect((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
	
	if ((gradientColor == nullptr) || (gradientRect == nullptr)){
		graphics.DrawArc(((Pen_GdiPlus*)pen)->GetPen(), rect, startAngle, sweepAngle);
	}
	else {
		Gdiplus::RectF gradientRectF((Gdiplus::REAL)gradientRect->left, 
								     (Gdiplus::REAL)gradientRect->top,
								     (Gdiplus::REAL)(gradientRect->right - gradientRect->left),
									 (Gdiplus::REAL)(gradientRect->bottom - gradientRect->top));
		Gdiplus::LinearGradientBrush lgbrush(gradientRectF, pen->GetColor().GetARGB(), gradientColor->GetARGB(), Gdiplus::LinearGradientModeVertical);

		Gdiplus::REAL factors[4] = { 0.0f, 0.4f, 0.6f, 1.0f };
		Gdiplus::REAL positions[4] = { 0.0f, 0.2f, 0.8f, 1.0f };
		lgbrush.SetBlend(factors, positions, 4);

		Gdiplus::Pen fgPen(&lgbrush, static_cast<Gdiplus::REAL>(pen->GetWidth()));
		graphics.DrawArc(&fgPen, rect, startAngle, sweepAngle);
	}
}

void Render_GdiPlus::DrawPath(const IPath* path, const IPen* pen)
{
	ASSERT(path != nullptr);
	ASSERT(pen != nullptr);
	if ((path == nullptr) || (pen == nullptr)) {
		return;
	}
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.DrawPath(((Pen_GdiPlus*)pen)->GetPen(), ((Path_Gdiplus*)path)->GetPath());
}

void Render_GdiPlus::FillPath(const IPath* path, const IBrush* brush)
{
	ASSERT(path != nullptr);
	ASSERT(brush != nullptr);
	if ((path == nullptr) || (brush == nullptr)) {
		return;
	}
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	graphics.FillPath(((Brush_Gdiplus*)brush)->GetBrush(), ((Path_Gdiplus*)path)->GetPath());
}

void Render_GdiPlus::DrawString(const UiRect& rc, const std::wstring& strText,
	                            UiColor dwTextColor, const std::wstring& strFontId, uint32_t uFormat, uint8_t uFade /*= 255*/)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));
	ASSERT(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) {
		return;
	}

	IFont* pFont = GlobalManager::Instance().Font().GetIFont(strFontId);
	Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(pFont);
	ASSERT(pGdiFont != nullptr);
	if (pGdiFont == nullptr) {
		return;
	}
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, pGdiFont->GetFontHandle());

	Gdiplus::RectF rcPaint((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
	int alpha = dwTextColor.GetARGB() >> 24;
	uFade = static_cast<uint8_t>(uFade * static_cast<double>(alpha) / 255);
	if (uFade == 255) {
		uFade = 254;
	}
	Gdiplus::SolidBrush tBrush(Gdiplus::Color(uFade, dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB()));

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	if ((uFormat & TEXT_END_ELLIPSIS) != 0) {
		stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
	}
	if ((uFormat & TEXT_PATH_ELLIPSIS) != 0) {
		stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
	}

	int formatFlags = Gdiplus::StringFormatFlagsLineLimit;
	if ((uFormat & TEXT_NOCLIP) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoClip;
	}
	if ((uFormat & TEXT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if ((uFormat & TEXT_CENTER) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uFormat & TEXT_RIGHT) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
	}
	else {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
	}

	if ((uFormat & TEXT_VCENTER) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uFormat & TEXT_BOTTOM) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentFar);
	}
	else {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
	}

	// try to fix font issue
#if 1
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
#else
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
#endif
	graphics.DrawString(strText.c_str(), (int)strText.length(), &font, rcPaint, &stringFormat, &tBrush);
}

UiRect Render_GdiPlus::MeasureString(const std::wstring& strText, const std::wstring& strFontId,
								     uint32_t uFormat, int width /*= DUI_NOSET_VALUE*/)
{
	IFont* pFont = GlobalManager::Instance().Font().GetIFont(strFontId);
	Font_GDI* pGdiFont = dynamic_cast<Font_GDI*>(pFont);
	ASSERT(pGdiFont != nullptr);
	if (pGdiFont == nullptr) {
		return UiRect();
	}
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, pGdiFont->GetFontHandle());
	Gdiplus::RectF bounds;

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	int formatFlags = 0;
	if ((uFormat & TEXT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if (width == DUI_NOSET_VALUE) {
		graphics.MeasureString(strText.c_str(), (int)strText.length(), &font, Gdiplus::PointF(), &stringFormat, &bounds);
	}
	else {
		Gdiplus::REAL height = 0;
		if ((uFormat & TEXT_SINGLELINE) != 0) {
			Gdiplus::RectF rcEmpty((Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)0);
			graphics.MeasureString(L"测试", 2, &font, rcEmpty, &stringFormat, &bounds);
			height = bounds.Height;
		}
		Gdiplus::RectF rcText((Gdiplus::REAL)0, (Gdiplus::REAL)0, (Gdiplus::REAL)width, height);
		graphics.MeasureString(strText.c_str(), (int)strText.length(), &font, rcText, &stringFormat, &bounds);
	}

	UiRect rc(int(bounds.GetLeft()), int(bounds.GetTop()), int(bounds.GetRight() + 1), int(bounds.GetBottom() + 1));
	return rc;
}

void Render_GdiPlus::DrawRichText(const UiRect& /*rc*/,
								  std::vector<RichTextData>& /*richTextData*/,
								  uint32_t /*uFormat*/,
								  bool /*bMeasureOnly*/,
								  uint8_t /*uFade*/)
{
	ASSERT(!"No Impl!");
}

void Render_GdiPlus::DrawBoxShadow(const UiRect& rc, 
								   const UiSize& roundSize, 
						     	   const UiPoint& cpOffset, 
								   int nBlurRadius, 
								   int nSpreadRadius,
								   UiColor dwColor)
{
	ASSERT((GetWidth() > 0) && (GetHeight() > 0));

	ASSERT(dwColor.GetARGB() != 0);

	ui::UiRect destRc = rc;
	destRc.Offset(cpOffset);
	destRc.left -= nSpreadRadius;
	destRc.top -= nSpreadRadius;
	destRc.right += nSpreadRadius;
	destRc.bottom += nSpreadRadius;

	Gdiplus::RectF srcRc(0.0f, 0.0f, (Gdiplus::REAL)destRc.Width(), (Gdiplus::REAL)destRc.Height());
	Gdiplus::RectF excludeRc(std::abs(nSpreadRadius) * 1.0f,
							 std::abs(nSpreadRadius) * 1.0f,
		                     srcRc.Width - std::abs(nSpreadRadius) * 2.0f,
		                     srcRc.Height - std::abs(nSpreadRadius) * 2.0f);

	Gdiplus::GraphicsPath shadowPath;
	Gdiplus::GraphicsPath excludePath;

	auto add_roundcorner_path = [](Gdiplus::GraphicsPath& path, const Gdiplus::RectF& rect, const UiSize& roundSize) {
		path.AddArc(rect.GetLeft(), rect.GetTop(), (Gdiplus::REAL)roundSize.cx, (Gdiplus::REAL)roundSize.cy, 180, 90);
		path.AddLine(rect.GetLeft() + (Gdiplus::REAL)roundSize.cx, rect.GetTop(), rect.GetRight() - (Gdiplus::REAL)roundSize.cx, rect.GetTop());
		path.AddArc(rect.GetRight() - (Gdiplus::REAL)roundSize.cx, rect.GetTop(), (Gdiplus::REAL)roundSize.cx, (Gdiplus::REAL)roundSize.cy, 270, 90);
		path.AddLine(rect.GetRight(), rect.GetTop() + (Gdiplus::REAL)roundSize.cy, rect.GetRight(), rect.GetBottom() - (Gdiplus::REAL)roundSize.cy);
		path.AddArc(rect.GetRight() - (Gdiplus::REAL)roundSize.cx, rect.GetBottom() - (Gdiplus::REAL)roundSize.cy, (Gdiplus::REAL)roundSize.cx, (Gdiplus::REAL)roundSize.cy, 0, 90);
		path.AddLine(rect.GetRight() - (Gdiplus::REAL)roundSize.cx, rect.GetBottom(), rect.GetLeft() + (Gdiplus::REAL)roundSize.cx, rect.GetBottom());
		path.AddArc(rect.GetLeft(), rect.GetBottom() - (Gdiplus::REAL)roundSize.cy, (Gdiplus::REAL)roundSize.cx, (Gdiplus::REAL)roundSize.cy, 90, 90);
		path.AddLine(rect.GetLeft(), rect.GetBottom() - (Gdiplus::REAL)roundSize.cy, rect.GetLeft(), rect.GetTop() + (Gdiplus::REAL)roundSize.cy);
		path.CloseFigure();
	};

	add_roundcorner_path(shadowPath, srcRc, roundSize);	
	add_roundcorner_path(excludePath, excludeRc, roundSize);
		
	
	Gdiplus::PathGradientBrush gradientPathBrush(&shadowPath);
	gradientPathBrush.SetWrapMode(Gdiplus::WrapMode::WrapModeClamp);

	Gdiplus::Color colors[] = { Gdiplus::Color::Transparent,Gdiplus::Color(dwColor.GetARGB()) ,Gdiplus::Color(dwColor.GetARGB()) };
	Gdiplus::REAL pos[] = { 0.0f,0.7f,1.0f };

	gradientPathBrush.SetInterpolationColors(colors, pos, 3);

	Gdiplus::Bitmap tempBitmap(static_cast<INT>(srcRc.Width), static_cast<INT>(srcRc.Height));
	Gdiplus::Graphics tempRender(&tempBitmap);

	Gdiplus::Region excludeRegion(&excludePath);
	tempRender.ExcludeClip(&excludeRegion);
	tempRender.FillPath(&gradientPathBrush, &shadowPath);

	// blur effect
	Gdiplus::BlurParams blurParams;
	blurParams.expandEdge = false;
	blurParams.radius = static_cast<float>(nBlurRadius);

	Gdiplus::Blur blurEffect;
	blurEffect.SetParameters(&blurParams);

	RECT rcBlurEffect{ 2 * nSpreadRadius, 2 * nSpreadRadius,static_cast<LONG>(srcRc.Width) - 2 * nSpreadRadius,static_cast<LONG>(srcRc.Height) - 2 * nSpreadRadius };
	tempBitmap.ApplyEffect(&blurEffect, &rcBlurEffect);

	Gdiplus::Graphics graphics(m_hDC);
	graphics.DrawImage(&tempBitmap,
					   Gdiplus::RectF(static_cast<Gdiplus::REAL>(destRc.left), static_cast<Gdiplus::REAL>(destRc.top), 
									  static_cast<Gdiplus::REAL>(destRc.Width()), static_cast<Gdiplus::REAL>(destRc.Height())),
					   0, 0, srcRc.Width, srcRc.Height,
		               Gdiplus::UnitPixel);
}

} // namespace ui
