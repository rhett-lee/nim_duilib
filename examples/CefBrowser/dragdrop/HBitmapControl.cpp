#include "HBitmapControl.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

using namespace ui;

HBitmapControl::HBitmapControl(ui::Window* pWindow):
    ui::Control(pWindow),
    m_hBitmap(nullptr)
{
}

void HBitmapControl::Paint(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
    UiRect paintRect = GetPaintRect();
    if (!UiRect::Intersect(paintRect, rcPaint, GetRect())) {
        return;
    }
    SetPaintRect(paintRect);

    BaseClass::Paint(pRender, rcPaint);

    if (nullptr == m_hBitmap) {
        return;
    }

    std::unique_ptr<IRender> render;
    std::unique_ptr<IBitmap> bitmap;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        ASSERT(GetWindow() != nullptr);
        IRenderDpiPtr spRenderDpi;
        if (GetWindow() != nullptr) {
            spRenderDpi = GetWindow()->GetRenderDpi();
        }
        render.reset(pRenderFactory->CreateRender(spRenderDpi));
        bitmap.reset(pRenderFactory->CreateBitmap());
    }
    ASSERT(render != nullptr);
    ASSERT(bitmap != nullptr);
    if (!bitmap || !render) {
        return;
    }

    BITMAP bm = { 0 };
    ::GetObject(m_hBitmap, sizeof(bm), &bm);
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

void HBitmapControl::SetBitmapImage(HBITMAP bitmap)
{
    m_hBitmap = bitmap;
}

#endif //(DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
