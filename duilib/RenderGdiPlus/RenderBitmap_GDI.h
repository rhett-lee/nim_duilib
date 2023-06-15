#ifndef UI_RENDER_GDIPLUS_RENDER_BITMAP_GDI_H_
#define UI_RENDER_GDIPLUS_RENDER_BITMAP_GDI_H_

#pragma once

#include "duilib/duilib_defs.h"

namespace ui
{

/** GDIäÖÈ¾ÒýÇæµÄ¸¨ÖúÀà
*/
class RenderBitmap_GDI
{
public:
	RenderBitmap_GDI();
	~RenderBitmap_GDI();

	bool Init(HDC hSrcDC, int width, int height);
	void Clear();
	HBITMAP DetachBitmap();

	HBITMAP GetHBitmap();
	int	GetWidth();
	int GetHeight();

	void ClearAlpha(const UiRect& rcDirty, int alpha);
	void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding, int alpha);
	void RestoreAlpha(const UiRect& rcDirty, const UiRect& rcShadowPadding);
private:
	void CleanUp();
	HBITMAP CreateDIBBitmap(HDC hdc, int width, int height, LPVOID* pBits);
private:
	HBITMAP	m_hBitmap;
	BYTE	*m_pPiexl;

	int		m_nWidth;
	int		m_nHeight;
};
} // namespace ui

#endif // UI_RENDER_GDIPLUS_RENDER_BITMAP_GDI_H_