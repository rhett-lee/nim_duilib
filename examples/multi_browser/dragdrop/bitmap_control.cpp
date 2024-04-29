#include "bitmap_control.h"

using namespace ui;

BitmapControl::BitmapControl()
{
	bitmap_ = NULL;
}

void BitmapControl::Paint(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
	UiRect paintRect = GetPaintRect();
	if (!UiRect::Intersect(paintRect, rcPaint, GetRect())) {
		return;
	}
	SetPaintRect(paintRect);

	__super::Paint(pRender, rcPaint);

	if (NULL == bitmap_)
		return;

	std::unique_ptr<IRender> render;
	std::unique_ptr<IBitmap> bitmap;
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory != nullptr) {
		render.reset(pRenderFactory->CreateRender());
		bitmap.reset(pRenderFactory->CreateBitmap());
	}
	ASSERT(render != nullptr);
	ASSERT(bitmap != nullptr);
	if (!bitmap || !render) {
		return;
	}

	BITMAP bm = { 0 };
	::GetObject(bitmap_, sizeof(bm), &bm);
	//ASSERT((bm.bmBits != nullptr) && (bm.bmBitsPixel == 32) && (bm.bmHeight > 0) && (bm.bmWidth > 0));
	if ((bm.bmBits != nullptr) && (bm.bmBitsPixel == 32) && (bm.bmHeight > 0) && (bm.bmWidth > 0)) {
		const uint32_t imageDataSize = bm.bmHeight * bm.bmWidth * 4;
		if (render->Resize(bm.bmWidth, bm.bmHeight)) {
			bitmap->Init(bm.bmWidth, bm.bmHeight, true, bm.bmBits);
			render->DrawImage(UiRect(0, 0, bm.bmWidth, bm.bmHeight),
				              bitmap.get(),
				              UiRect(0, 0, bm.bmWidth, bm.bmHeight),
				              UiRect(0, 0, 0, 0),
				              UiRect(0, 0, bm.bmWidth, bm.bmHeight),
				              UiRect(0, 0, 0, 0));
		}
		else {
			return;
		}
	}
	else {
		return;
	}

	UiRect rect = GetRect();
	pRender->AlphaBlend(rect.left, rect.top, rect.Width(), rect.Height(),
						render.get(),
						0, 0, rect.Width(), rect.Height());
}

void BitmapControl::SetBitmapImage(HBITMAP bitmap)
{
	bitmap_ = bitmap;
}
