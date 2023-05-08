#include "bitmap_control.h"

using namespace ui;

BitmapControl::BitmapControl()
{
	bitmap_ = NULL;
}

void BitmapControl::Paint(ui::IRenderContext* pRender, const ui::UiRect& rcPaint)
{
	if (!::IntersectRect(&m_rcPaint, &rcPaint, &GetRect())) return;
	__super::Paint(pRender, rcPaint);

	if (NULL == bitmap_)
		return;

	HDC hCloneDC = ::CreateCompatibleDC(pRender->GetDC());
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, bitmap_);

	pRender->AlphaBlend(GetRect().left, GetRect().top, GetRect().right - GetRect().left, GetRect().bottom - GetRect().top, hCloneDC,
		0, 0, GetRect().right - GetRect().left, GetRect().bottom - GetRect().top);

	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void BitmapControl::SetBitmapImage(HBITMAP bitmap)
{
	bitmap_ = bitmap;
}
