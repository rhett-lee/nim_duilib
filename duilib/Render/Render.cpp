#include "Render.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/Pen.h"
#include "duilib/Render/Path.h"
#include "duilib/Render/Brush.h"
#include "duilib/Image/GdiPlusDefs.h"

namespace ui {

static inline void DrawFunction(HDC hDC, bool bTransparent, UiRect rcDest, HDC hdcSrc, UiRect rcSrc, bool bAlphaChannel, int uFade)
{
	if (bTransparent || bAlphaChannel || uFade < 255 
		|| (rcSrc.GetWidth() == rcDest.GetWidth() && rcSrc.GetHeight() == rcDest.GetHeight())) {
		BLENDFUNCTION ftn = { AC_SRC_OVER, 0, static_cast<BYTE>(uFade), AC_SRC_ALPHA };
		::AlphaBlend(hDC, rcDest.left, rcDest.top, rcDest.GetWidth(), rcDest.GetHeight(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.GetWidth(), rcSrc.GetHeight(), ftn);
	}
	else {
		::StretchBlt(hDC, rcDest.left, rcDest.top, rcDest.GetWidth(), rcDest.GetHeight(),
			hdcSrc, rcSrc.left, rcSrc.top, rcSrc.GetWidth(), rcSrc.GetHeight(), SRCCOPY);
	}
}

RenderContext_GdiPlus::RenderContext_GdiPlus()
	: m_hDC(NULL)
	, m_saveDC(0)
	, m_hOldBitmap(NULL)
	, m_bTransparent(false)
{
	HDC hDC = ::GetDC(NULL);
	m_hDC = ::CreateCompatibleDC(hDC);
	::ReleaseDC(NULL, hDC);
	ASSERT(m_hDC);	
}

RenderContext_GdiPlus::~RenderContext_GdiPlus()
{
	if (m_hOldBitmap != NULL)
	{
		::SelectObject(m_hDC, m_hOldBitmap);
		m_hOldBitmap = NULL;
	}

	if (m_hDC != NULL)
	{
		::DeleteDC(m_hDC);
		m_hDC = NULL;
	}
}

HDC RenderContext_GdiPlus::GetDC()
{
	return m_hDC;
}

bool RenderContext_GdiPlus::Resize(int width, int height, bool flipBItmap)
{
    if (width <= 0)
        width = 1;
    if (height <= 0)
        height = 1;

	ASSERT(m_hDC);
	if (m_bitmap.GetWidth() == width && m_bitmap.GetHeight() == height)
		return false;

	if (m_hOldBitmap != NULL)
	{
		::SelectObject(m_hDC, m_hOldBitmap);
	}

	bool ret = m_bitmap.Init(m_hDC, width, height, flipBItmap);
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hDC, m_bitmap.GetBitmap());
	return ret;
}

void RenderContext_GdiPlus::Clear()
{
	ASSERT(m_hDC);
	m_bitmap.Clear();
}

std::unique_ptr<ui::IRenderContext> RenderContext_GdiPlus::Clone()
{
	std::unique_ptr<ui::IRenderContext> pClone = std::make_unique<ui::RenderContext_GdiPlus>();
	pClone->Resize(GetWidth(), GetHeight());
	pClone->BitBlt(0, 0, GetWidth(), GetHeight(), m_hDC);
	return pClone;
}

HBITMAP RenderContext_GdiPlus::DetachBitmap()
{
	ASSERT(m_hDC && m_hOldBitmap);
	ASSERT(m_bitmap.GetHeight() != 0 && m_bitmap.GetWidth() != 0);
	if (m_hOldBitmap == NULL)
		return NULL;

	::SelectObject(m_hDC, m_hOldBitmap);
	return m_bitmap.DetachBitmap();
}

BYTE* RenderContext_GdiPlus::GetBits()
{
	return m_bitmap.GetBits();
}

HBITMAP RenderContext_GdiPlus::GetBitmap()
{
	return m_bitmap.GetBitmap();
}

int RenderContext_GdiPlus::GetWidth()
{
	return m_bitmap.GetWidth();
}

int RenderContext_GdiPlus::GetHeight()
{
	return m_bitmap.GetHeight();
}

void RenderContext_GdiPlus::ClearAlpha(const UiRect& rcDirty, int alpha)
{
	m_bitmap.ClearAlpha(rcDirty, alpha);
}

void RenderContext_GdiPlus::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha)
{
	m_bitmap.RestoreAlpha(rcDirty, rcShadowPadding, alpha);
}

void RenderContext_GdiPlus::RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding /*= UiRect()*/)
{
	m_bitmap.RestoreAlpha(rcDirty, rcShadowPadding);
}

bool RenderContext_GdiPlus::IsRenderTransparent() const
{
	return m_bTransparent;
}

bool RenderContext_GdiPlus::SetRenderTransparent(bool bTransparent)
{
	bool oldValue = m_bTransparent;
	m_bTransparent = bTransparent;
	return oldValue;
}

void RenderContext_GdiPlus::Save()
{
	m_saveDC = SaveDC(m_hDC);
}

void RenderContext_GdiPlus::Restore()
{
	RestoreDC(m_hDC, m_saveDC);
}

UiPoint RenderContext_GdiPlus::OffsetWindowOrg(UiPoint ptOffset)
{
	UiPoint ptOldWindowOrg;
	GetWindowOrgEx(m_hDC, &ptOldWindowOrg);
	ptOffset.Offset(ptOldWindowOrg.x, ptOldWindowOrg.y);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return ptOldWindowOrg;
}

UiPoint RenderContext_GdiPlus::SetWindowOrg(UiPoint ptOffset)
{
	UiPoint ptOldWindowOrg;
	GetWindowOrgEx(m_hDC, &ptOldWindowOrg);
	::SetWindowOrgEx(m_hDC, ptOffset.x, ptOffset.y, NULL);
	return ptOldWindowOrg;
}

UiPoint RenderContext_GdiPlus::GetWindowOrg() const
{
	UiPoint ptWindowOrg;
	GetWindowOrgEx(m_hDC, &ptWindowOrg);
	return ptWindowOrg;
}

void RenderContext_GdiPlus::SetClip(const UiRect& rc)
{
	m_clip.CreateClip(m_hDC, rc);
}

void RenderContext_GdiPlus::SetRoundClip(const UiRect& rc, int width, int height)
{
	m_clip.CreateRoundClip(m_hDC, rc, width, height);
}

void RenderContext_GdiPlus::ClearClip()
{
	m_clip.ClearClip(m_hDC);
}

HRESULT RenderContext_GdiPlus::BitBlt(int x, int y, int cx, int cy, HDC hdcSrc, int xSrc /*= 0*/, int yScr /*= 0*/, DWORD rop /*= SRCCOPY*/)
{
	return ::BitBlt(m_hDC, x, y, cx, cy, hdcSrc, xSrc, yScr, rop);
}

bool RenderContext_GdiPlus::StretchBlt(int xDest, int yDest, int widthDest, int heightDest,	HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, DWORD rop /*= SRCCOPY*/)
{
	int stretchBltMode = ::SetStretchBltMode(m_hDC, HALFTONE);
	bool ret = (TRUE == ::StretchBlt(m_hDC, xDest, yDest, widthDest, heightDest,
		hdcSrc, xSrc, yScr, widthSrc, heightSrc, rop));
	::SetStretchBltMode(m_hDC, stretchBltMode);
	return ret;
}

bool RenderContext_GdiPlus::AlphaBlend(int xDest, int yDest, int widthDest, int heightDest, HDC hdcSrc, int xSrc, int yScr, int widthSrc, int heightSrc, BYTE uFade /*= 255*/)
{
	BLENDFUNCTION bf = { AC_SRC_OVER, 0, uFade, AC_SRC_ALPHA };
	return (TRUE == ::AlphaBlend(m_hDC, xDest, yDest, widthDest, heightDest, hdcSrc, xSrc, yScr, widthSrc, heightSrc, bf));
}

void RenderContext_GdiPlus::DrawImage(const UiRect& rcPaint, HBITMAP hBitmap, bool bAlphaChannel,
	const UiRect& rcImageDest, const UiRect& rcImageSource, UiRect rcCorners, bool bBitmapDpiScale, 
	BYTE uFade /*= 255*/, bool xtiled /*= false*/, bool ytiled /*= false*/,
	bool fullxtiled /*= true*/, bool fullytiled /*= true*/, int nTiledMargin /*=0*/)
{
	UiRect rcTestTemp;
	if (!::IntersectRect(&rcTestTemp, &rcImageDest, &rcPaint)) return;

	ASSERT(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);

	if (hBitmap == NULL) return;

	HDC hCloneDC = ::CreateCompatibleDC(m_hDC);
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	int stretchBltMode = ::SetStretchBltMode(m_hDC, HALFTONE);

	UiRect rcTemp;
	UiRect rcSource;
	UiRect rcDest;
	UiRect rcDpiCorner = rcCorners;
	DpiManager::GetInstance()->ScaleRect(rcDpiCorner);
	// 如果源位图已经按照DPI缩放过，那么对应的corner也缩放一下
	if (bBitmapDpiScale)
		rcCorners = rcDpiCorner;

	// middle
	rcDest.left = rcImageDest.left + rcDpiCorner.left;
	rcDest.top = rcImageDest.top + rcDpiCorner.top;
	rcDest.right = rcImageDest.right - rcDpiCorner.right;
	rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
	rcSource.left = rcImageSource.left + rcCorners.left;
	rcSource.top = rcImageSource.top + rcCorners.top;
	rcSource.right = rcImageSource.right - rcCorners.right;
	rcSource.bottom = rcImageSource.bottom - rcCorners.bottom;
	if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
		if (!xtiled && !ytiled) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
		else if (xtiled && ytiled) {
			const LONG imageDrawWidth = rcImageSource.right - rcImageSource.left - rcCorners.left - rcCorners.right;
			const LONG imageDrawHeight = rcImageSource.bottom - rcImageSource.top - rcCorners.top - rcCorners.bottom;
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

					rcSource.left = rcImageSource.left + rcCorners.left;
					rcSource.top = rcImageSource.top + rcCorners.top;
					rcSource.right = rcSource.left + lDrawWidth;
					rcSource.bottom = rcSource.top + lDrawHeight;

					UiRect rcDestTemp;
					rcDestTemp.left = lDestLeft;
					rcDestTemp.right = lDestRight;
					rcDestTemp.top = lDestTop;
					rcDestTemp.bottom = lDestBottom;
					
					DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcSource, bAlphaChannel, uFade);
				}
			}
		}
		else if (xtiled) { // supp
			const LONG imageDrawWidth = rcImageSource.right - rcImageSource.left - rcCorners.left - rcCorners.right;
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
				rcSource.left = rcImageSource.left + rcCorners.left;
				rcSource.top = rcImageSource.top + rcCorners.top;
				rcSource.right = rcSource.left + lDrawWidth;
				rcSource.bottom = rcImageSource.bottom - rcCorners.bottom;

				UiRect rcDestTemp = rcDest;
				rcDestTemp.top = rcDest.top;
				rcDestTemp.bottom = rcDest.top + rcSource.GetHeight();
				rcDestTemp.left = lDestLeft;
				rcDestTemp.right = lDestRight;

				DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcSource, bAlphaChannel, uFade);
			}
		}
		else { // ytiled
			const LONG imageDrawHeight = rcImageSource.bottom - rcImageSource.top - rcCorners.top - rcCorners.bottom;
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

				rcSource.left = rcImageSource.left + rcCorners.left;
				rcSource.top = rcImageSource.top + rcCorners.top;
				rcSource.right = rcImageSource.right - rcCorners.right;
				rcSource.bottom = rcSource.top + lDrawHeight;

				rcDestTemp.right = rcDest.left + rcSource.GetWidth();
				rcDestTemp.top = lDestTop;
				rcDestTemp.bottom = lDestBottom;
				
				DrawFunction(m_hDC, m_bTransparent, rcDestTemp, hCloneDC, rcSource, bAlphaChannel, uFade);
			}
		}
	}

	// left-top
	if (rcCorners.left > 0 && rcCorners.top > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.left + rcCorners.left;
		rcSource.bottom = rcImageSource.top + rcCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// top
	if (rcCorners.top > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.left + rcCorners.left;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right - rcCorners.right;
		rcSource.bottom = rcImageSource.top + rcCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right-top
	if (rcCorners.right > 0 && rcCorners.top > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.top + rcDpiCorner.top;
		rcSource.left = rcImageSource.right - rcCorners.right;
		rcSource.top = rcImageSource.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.top + rcCorners.top;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// left
	if (rcCorners.left > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.top + rcCorners.top;
		rcSource.right = rcImageSource.left + rcCorners.left;
		rcSource.bottom = rcImageSource.bottom - rcCorners.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right
	if (rcCorners.right > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.top + rcDpiCorner.top;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom - rcDpiCorner.bottom;
		rcSource.left = rcImageSource.right - rcCorners.right;
		rcSource.top = rcImageSource.top + rcCorners.top;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom - rcCorners.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// left-bottom
	if (rcCorners.left > 0 && rcCorners.bottom > 0) {
		rcDest.left = rcImageDest.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.left + rcDpiCorner.left;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left;
		rcSource.top = rcImageSource.bottom - rcCorners.bottom;
		rcSource.right = rcImageSource.left + rcCorners.left;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// bottom
	if (rcCorners.bottom > 0) {
		rcDest.left = rcImageDest.left + rcDpiCorner.left;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right - rcDpiCorner.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.left + rcCorners.left;
		rcSource.top = rcImageSource.bottom - rcCorners.bottom;
		rcSource.right = rcImageSource.right - rcCorners.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}
	// right-bottom
	if (rcCorners.right > 0 && rcCorners.bottom > 0) {
		rcDest.left = rcImageDest.right - rcDpiCorner.right;
		rcDest.top = rcImageDest.bottom - rcDpiCorner.bottom;
		rcDest.right = rcImageDest.right;
		rcDest.bottom = rcImageDest.bottom;
		rcSource.left = rcImageSource.right - rcCorners.right;
		rcSource.top = rcImageSource.bottom - rcCorners.bottom;
		rcSource.right = rcImageSource.right;
		rcSource.bottom = rcImageSource.bottom;
		if (::IntersectRect(&rcTemp, &rcPaint, &rcDest)) {
			DrawFunction(m_hDC, m_bTransparent, rcDest, hCloneDC, rcSource, bAlphaChannel, uFade);
		}
	}

	::SetStretchBltMode(m_hDC, stretchBltMode);
	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void RenderContext_GdiPlus::DrawColor(const UiRect& rc, UiColor dwColor, BYTE uFade)
{
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
		                  static_cast<Gdiplus::REAL>(rc.GetWidth()), static_cast<Gdiplus::REAL>(rc.GetHeight()));
	graphics.FillRectangle(&brush, rcFill);
}

void RenderContext_GdiPlus::DrawLine(const UiPoint& pt1, const UiPoint& pt2, UiColor penColor, int nWidth)
{
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);
	graphics.DrawLine(&pen, Gdiplus::Point(pt1.x, pt1.y), Gdiplus::Point(pt2.x, pt2.y));
}

void RenderContext_GdiPlus::DrawRect(const UiRect& rc, UiColor penColor, int nWidth)
{
	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);
	graphics.DrawRectangle(&pen, rc.left, rc.top, rc.GetWidth(), rc.GetHeight());
}

void RenderContext_GdiPlus::DrawRoundRect(const UiRect& rc, const UiSize& roundSize, UiColor penColor, int nWidth)
{
	Gdiplus::Graphics graphics(m_hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	Gdiplus::Pen pen(Gdiplus::Color(penColor.GetARGB()), (Gdiplus::REAL)nWidth);

	// 裁剪区域不能作画，导致边框有时不全，往里收缩一个像素
	// UiRect rcInflate = rc;
	// rcInflate.Inflate({ -1, -1, -1, -1 });

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

void RenderContext_GdiPlus::DrawPath(const IPath* path, const IPen* pen)
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

void RenderContext_GdiPlus::FillPath(const IPath* path, const IBrush* brush)
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

void RenderContext_GdiPlus::DrawText(const UiRect& rc, const std::wstring& strText, UiColor dwTextColor, const std::wstring& strFontId, UINT uStyle, BYTE uFade /*= 255*/, bool bLineLimit /*= false*/, bool bFillPath /*= false*/)
{
	ASSERT(::GetObjectType(m_hDC) == OBJ_DC || ::GetObjectType(m_hDC) == OBJ_MEMDC);
	if (strText.empty()) return;

	Gdiplus::InstalledFontCollection installedFontCollection;

	// How many font families are installed?
	int count = installedFontCollection.GetFamilyCount();
	if (count == 0) {

		::SetBkMode(m_hDC, TRANSPARENT);
		::SetTextColor(m_hDC, dwTextColor.ToCOLORREF());
		HFONT hOldFont = (HFONT)::SelectObject(m_hDC, GlobalManager::GetFont(strFontId));

		RECT rcGdi = { rc.left,rc.top,rc.right,rc.bottom };
		::DrawText(m_hDC, strText.c_str(), -1, &rcGdi, uStyle);
		::SelectObject(m_hDC, hOldFont);

		return;
	}

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, GlobalManager::GetFont(strFontId));

	Gdiplus::RectF rcPaint((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
	int alpha = dwTextColor.GetARGB() >> 24;
	uFade = static_cast<BYTE>(uFade * static_cast<double>(alpha) / 255);
	if (uFade == 255) {
		uFade = 254;
	}
	Gdiplus::SolidBrush tBrush(Gdiplus::Color(uFade, dwTextColor.GetR(), dwTextColor.GetG(), dwTextColor.GetB()));

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	if ((uStyle & DT_END_ELLIPSIS) != 0) {
		stringFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
	}

	int formatFlags = 0;
	if ((uStyle & DT_NOCLIP) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoClip;
	}
	if ((uStyle & DT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	if (bLineLimit) {
		formatFlags |= Gdiplus::StringFormatFlagsLineLimit;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if ((uStyle & DT_LEFT) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
	}
	else if ((uStyle & DT_CENTER) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uStyle & DT_RIGHT) != 0) {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentFar);
	}
	else {
		stringFormat.SetAlignment(Gdiplus::StringAlignmentNear);
	}

	if ((uStyle & DT_TOP) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentNear);
	}
	else if ((uStyle & DT_VCENTER) != 0) {
		stringFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	}
	else if ((uStyle & DT_BOTTOM) != 0) {
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
  if (bFillPath) {
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::GraphicsPath path;
    Gdiplus::FontFamily font_family;
    font.GetFamily(&font_family);
    path.AddString(strText.c_str(), (int)strText.length(), &font_family, font.GetStyle(),
      font.GetSize(), rcPaint, &stringFormat);
    graphics.FillPath(&tBrush, &path);
    return;
  }
	graphics.DrawString(strText.c_str(), (int)strText.length(), &font, rcPaint, &stringFormat, &tBrush);
}

void RenderContext_GdiPlus::DrawBoxShadow(const UiRect& rc, 
									  	 const UiSize& roundSize, 
										 const UiPoint& cpOffset, 
										 int nBlurRadius, 
										 int nBlurSize, 
										 int nSpreadSize, 
										 UiColor dwColor,
										 bool bExclude)
{
	(void)nSpreadSize;
#define USE_BLUR 1
#define USE_COLOR_MATRIX 0

	ASSERT(dwColor.GetARGB() != 0);

	ui::UiRect destRc = rc;
	destRc.Offset(cpOffset);
	destRc.left -= nBlurSize;
	destRc.top -= nBlurSize;
	destRc.right += nBlurSize;
	destRc.bottom += nBlurSize;

	Gdiplus::REAL scale = 1.0f;

	Gdiplus::RectF srcRc(0.0f, 0.0f, destRc.GetWidth()/ scale, destRc.GetHeight()/ scale);
	Gdiplus::RectF excludeRc(abs(nBlurSize) / scale,
		abs(nBlurSize) / scale,
		srcRc.Width - abs(nBlurSize) * 2.0f / scale,
		srcRc.Height - abs(nBlurSize) * 2.0f / scale);

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
	
	if (bExclude)
		add_roundcorner_path(excludePath, excludeRc, roundSize);
	
	Gdiplus::PathGradientBrush gradientPathBrush(&shadowPath);
	gradientPathBrush.SetWrapMode(Gdiplus::WrapMode::WrapModeClamp);

	Gdiplus::Color colors[] = { Gdiplus::Color::Transparent,Gdiplus::Color(dwColor.GetARGB()) ,Gdiplus::Color(dwColor.GetARGB()) };
	Gdiplus::REAL pos[] = { 0.0f,0.7f,1.0f };

	gradientPathBrush.SetInterpolationColors(colors, pos, 3);

	Gdiplus::Bitmap tempBitmap(static_cast<INT>(srcRc.Width), static_cast<INT>(srcRc.Height));
	Gdiplus::Graphics tempRender(&tempBitmap);

	if (bExclude) {
		Gdiplus::Region excludeRegion(&excludePath);
		tempRender.ExcludeClip(&excludeRegion);
	}

	tempRender.FillPath(&gradientPathBrush, &shadowPath);

#if USE_BLUR
	// blur effect
	Gdiplus::BlurParams blurParams;
	blurParams.expandEdge = false;
	blurParams.radius = static_cast<float>(nBlurRadius);

	Gdiplus::Blur blurEffect;
	blurEffect.SetParameters(&blurParams);

	RECT rcBlurEffect{ nBlurSize,nBlurSize,static_cast<LONG>(srcRc.Width) - 2 * nBlurSize,static_cast<LONG>(srcRc.Height) - 2 * nBlurSize };
	tempBitmap.ApplyEffect(&blurEffect, &rcBlurEffect);
#endif

#if USE_COLOR_MATRIX
	// color matrix
	//https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-using-a-color-matrix-to-transform-a-single-color-use
	//https://docs.microsoft.com/en-us/windows/win32/api/gdipluscolormatrix/ns-gdipluscolormatrix-colormatrix
	//A 5×5 color matrix is a homogeneous matrix for a 4 - space transformation.
	//The element in the fifth row and fifth column of a 5×5 homogeneous matrix must be 1, 
	//and all of the other elements in the fifth column must be 0. 
	//Color matrices are used to transform color vectors.
	//The first four components of a color vector hold the red, green, blue,
	//and alpha components(in that order) of a color.
	//The fifth component of a color vector is always 1.
	Gdiplus::ColorMatrix colorMatrix = {
	0.99215,0,0,0,0,
	0,0.24313,0,0,0,
	0,0,0.24705,0,0,
	0,0,0,0.53923,0,
	0,0,0,0,1
	};

	Gdiplus::ColorMatrixEffect colorEffect;
	colorEffect.SetParameters(&colorMatrix);
	tempBitmap.ApplyEffect(&colorEffect, NULL);
#endif


	Gdiplus::Graphics graphics(m_hDC);
	graphics.DrawImage(&tempBitmap,
		Gdiplus::RectF(static_cast<Gdiplus::REAL>(destRc.left), static_cast<Gdiplus::REAL>(destRc.top), 
			           static_cast<Gdiplus::REAL>(destRc.GetWidth()), static_cast<Gdiplus::REAL>(destRc.GetHeight())),
		0, 0, srcRc.Width, srcRc.Height,
		Gdiplus::UnitPixel);
}

ui::UiRect RenderContext_GdiPlus::MeasureText(const std::wstring& strText, const std::wstring& strFontId, UINT uStyle, int width /*= DUI_NOSET_VALUE*/)
{
	
	Gdiplus::InstalledFontCollection installedFontCollection;

	// How many font families are installed?
	int count = installedFontCollection.GetFamilyCount();
	if (count == 0) {
		::SetBkMode(m_hDC, TRANSPARENT);
		HFONT hOldFont = (HFONT)::SelectObject(m_hDC, GlobalManager::GetFont(strFontId));

		SIZE size = { 0 };
		GetTextExtentPoint32(m_hDC, strText.c_str(), (int)strText.size(), &size);
		::SelectObject(m_hDC, hOldFont);

		UiRect rc(int(0), int(0), int(size.cx + 1), int(size.cy + 1));
		return rc;
	}

	Gdiplus::Graphics graphics(m_hDC);
	Gdiplus::Font font(m_hDC, GlobalManager::GetFont(strFontId));
	Gdiplus::RectF bounds;

	Gdiplus::StringFormat stringFormat = Gdiplus::StringFormat::GenericTypographic();
	int formatFlags = 0;
	if ((uStyle & DT_SINGLELINE) != 0) {
		formatFlags |= Gdiplus::StringFormatFlagsNoWrap;
	}
	stringFormat.SetFormatFlags(formatFlags);

	if (width == DUI_NOSET_VALUE) {
		graphics.MeasureString(strText.c_str(), (int)strText.length(), &font, Gdiplus::PointF(), &stringFormat, &bounds);
	}
	else {
		Gdiplus::REAL height = 0;
		if ((uStyle & DT_SINGLELINE) != 0) {
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

} // namespace ui
