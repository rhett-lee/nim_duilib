#include "RenderTest1.h"

#if defined (DUILIB_BUILD_FOR_WIN)
    #include "duilib/Utils/BitmapHelper_Windows.h"
#elif defined(DUILIB_BUILD_FOR_SDL)
    #include "duilib/Utils/BitmapHelper_SDL.h"
#endif

namespace ui {

RenderTest1::RenderTest1(ui::Window* pWindow):
    ui::Control(pWindow)
{
}

RenderTest1::~RenderTest1()
{
}

void RenderTest1::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }

    int32_t marginLeft = 8;
    int32_t marginTop = 4;
    Dpi().ScaleInt(marginLeft);
    Dpi().ScaleInt(marginTop);

    UiRect rect = GetRect();
    rect.left += marginLeft;
    rect.top += marginTop;

    //首次绘制时，创建图片对象
    if (m_pImage == nullptr) {
        m_pImage = std::make_unique<Image>();
        m_pImage->SetImageString(_T("file='autumn.png' async_load='false'"), Dpi());
    }
    //每次绘制时检查DPI是否变化：DPI变化时重新加载图片，避免图片和平铺格子的尺寸错误
    LoadImageInfo(*m_pImage);
    const Image& image = *m_pImage;
    std::shared_ptr<ImageInfo> pImageInfo = image.GetImageInfo();
    if (pImageInfo == nullptr) {
        return;
    }
    const int32_t nImageWidth = pImageInfo->GetWidth();
    const int32_t nImageHeight = pImageInfo->GetHeight();
    if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
        return;
    }

    rect.right = rect.left + nImageWidth;
    rect.bottom = rect.top + nImageHeight;
    int32_t currentBottom = rect.bottom;//记录当前的bottom值

    UiRect rcSourceCorner;
    UiRect rcImageSource(0, 0, nImageWidth, nImageHeight);

    //1. 正常绘制图片
    std::shared_ptr<IBitmap> pBitmap = image.GetCurrentBitmap(false, rect, rcImageSource, rcSourceCorner, nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    pRender->DrawImage(rcPaint, pBitmap.get(), rect, UiRect(), rcImageSource, UiRect());

    //2. 半透明绘制图片
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nImageWidth;
    pBitmap = image.GetCurrentBitmap(false, rect, rcImageSource, rcSourceCorner, nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    pRender->DrawImage(rcPaint, pBitmap.get(), rect, UiRect(), rcImageSource, UiRect(), 100);

    //3. 缩小绘制
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nImageWidth / 2;
    rect.bottom = rect.top + nImageHeight / 2;
    pBitmap = image.GetCurrentBitmap(false, rect, rcImageSource, rcSourceCorner, nullptr);
    if (pBitmap == nullptr) {
        return;
    }
    pRender->DrawImage(rcPaint, pBitmap.get(), rect, UiRect(), rcImageSource, UiRect());

    //4. BitBlt/StretchBlt/AlphaBlend三个绘制函数，需要先把图片绘制到一个Render对象中
    std::unique_ptr<IRender> pSrcRender(BitmapHelper::CreateRenderObject(pBitmap.get()));
    if (pSrcRender == nullptr) {
        return;
    }

    //BitBlt
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nImageWidth;
    rect.bottom = rect.top + nImageHeight;
    pRender->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender.get(), 0, 0, RopMode::kSrcCopy);

    //StretchBlt
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nImageWidth / 2;
    rect.bottom = rect.top + nImageHeight / 2;
    pRender->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender.get(), 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), RopMode::kSrcCopy);

    //AlphaBlend
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nImageWidth;
    rect.bottom = rect.top + nImageHeight;
    pRender->AlphaBlend(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender.get(), 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), 96);

    currentBottom = rect.bottom;//记录当前的bottom值

    //换行
    rect = GetRect();
    rect.left += marginLeft;
    rect.top = currentBottom + marginTop;
    //限制平铺绘制区域的高度，避免平铺图案占满整个控件
    rect.bottom = std::min(rect.bottom, rect.top + nImageHeight * 2);
    if (rect.bottom <= rect.top) {
        return;
    }

    //5. 平铺绘制, 使用小图
    int32_t nTileSize = Dpi().GetScaleInt(32);
    std::unique_ptr<IBitmap> pTiledBitmap = ui::ImageUtil::ResizeImageBitmap(pBitmap.get(), nTileSize, nTileSize);
    if (pTiledBitmap == nullptr) {
        return;
    }

    UiRect rect1 = rect;
    UiRect rect2 = rect;
    UiRect rect3 = rect;
    UiRect rect4 = rect;
    rect1.right = rect1.left + rect.Width() / 2;
    rect1.bottom = rect1.top + rect.Height() / 2;

    rect2.left = rect1.right + Dpi().GetScaleInt(2);
    rect2.right = rect.right - Dpi().GetScaleInt(4);
    rect2.top = rect1.top;
    rect2.bottom = rect1.bottom;

    rect3.left = rect1.left;
    rect3.right = rect1.right;
    rect3.top = rect1.bottom + Dpi().GetScaleInt(2);
    rect3.bottom = rect.bottom - Dpi().GetScaleInt(4);

    rect4.left = rect3.right + Dpi().GetScaleInt(2);
    rect4.top = rect3.top;
    rect4.right = rect.right - Dpi().GetScaleInt(4);
    rect4.bottom = rect3.bottom;

    rcImageSource.left = 0;
    rcImageSource.top = 0;
    rcImageSource.right = pTiledBitmap->GetWidth();
    rcImageSource.bottom = pTiledBitmap->GetHeight();

    UiRect rcCorners;
    TiledDrawParam tiledParam;

    //平铺绘制
    tiledParam.m_bTiledX = true;
    tiledParam.m_bTiledY = true;
    pRender->DrawImage(rcPaint, pTiledBitmap.get(), rect1, rcCorners, rcImageSource, rcCorners, 255, &tiledParam);

    //平铺绘制（指定边距）
    tiledParam.m_nTiledMarginX = Dpi().GetScaleInt(2);
    tiledParam.m_nTiledMarginY = Dpi().GetScaleInt(2);
    pRender->DrawImage(rcPaint, pTiledBitmap.get(), rect2, rcCorners, rcImageSource, rcCorners, 255, &tiledParam);

    //九宫格绘制
    tiledParam = TiledDrawParam();
    tiledParam.m_bTiledX = true;
    tiledParam.m_bTiledY = true;
    int32_t nCornerSize = Dpi().GetScaleInt(6);
    rcCorners = UiRect(nCornerSize, nCornerSize, nCornerSize, nCornerSize);
    pRender->DrawImage(rcPaint, pTiledBitmap.get(), rect3, rcCorners, rcImageSource, rcCorners, 255, &tiledParam);

    //九宫格绘制（指定内边距）
    int32_t nPaddingSize = Dpi().GetScaleInt(2);
    tiledParam.m_nTiledMarginX = Dpi().GetScaleInt(2);
    tiledParam.m_nTiledMarginY = Dpi().GetScaleInt(2);
    tiledParam.m_rcTiledPadding = UiPadding(nPaddingSize, nPaddingSize, nPaddingSize, nPaddingSize);
    pRender->DrawImage(rcPaint, pTiledBitmap.get(), rect4, rcCorners, rcImageSource, rcCorners, 255, &tiledParam);
}

} //end of namespace ui
