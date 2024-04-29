#include "RenderTest1.h"
#include "duilib/RenderGdiPlus/BitmapHelper.h"

namespace ui {

RenderTest1::RenderTest1()
{

}

RenderTest1::~RenderTest1()
{

}

void RenderTest1::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    __super::AlphaPaint(pRender, rcPaint);
}

void RenderTest1::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
    int marginLeft = 8;
    int marginTop = 4;
    GlobalManager::Instance().Dpi().ScaleInt(marginLeft);
    GlobalManager::Instance().Dpi().ScaleInt(marginTop);

    UiRect rect = GetRect();
    UiRect textRect = rect;
    rect.left += marginLeft;
    rect.top += marginTop;

    int nSize = 110;
    int nTextLineHeight = 40;
    GlobalManager::Instance().Dpi().ScaleInt(nSize);
    GlobalManager::Instance().Dpi().ScaleInt(nTextLineHeight);

    rect.right = rect.left + nSize;
    rect.bottom = rect.top + nSize;
    int currentBottom = rect.bottom;//记录当前的bottom值
    
    //绘图相同接口
    Image image;
    image.SetImageString(L"autumn.png");
    LoadImageData(image);
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    rect.bottom = rect.top + image.GetImageCache()->GetHeight();

    UiRect rcImageSource(0, 0, image.GetImageCache()->GetWidth(), image.GetImageCache()->GetHeight());
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, UiRect(), rcImageSource, UiRect());

    //半透明绘制图片
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, UiRect(), rcImageSource, UiRect(), 100);

    //缩小绘制
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageCache()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() / 2;
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, UiRect(), rcImageSource, UiRect());

    //BitBlt/StretchBlt/AlphaBlend三个绘制函数
    IRender* pSrcRender = BitmapHelper::CreateRenderObject(image.GetCurrentBitmap());
    ASSERT(pSrcRender != nullptr);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    rect.bottom = rect.top + image.GetImageCache()->GetHeight();
    pRender->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender, 0, 0, RopMode::kSrcCopy);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageCache()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() / 2;
    pRender->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender, 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), RopMode::kSrcCopy);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageCache()->GetWidth() ;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() ;
    pRender->AlphaBlend(rect.left, rect.top, rect.Width() , rect.Height() , pSrcRender, 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), 96);

    delete pSrcRender;
    pSrcRender = nullptr;

    currentBottom = rect.bottom;//记录当前的bottom值

    //换行
    rect = GetRect();
    rect.left += marginLeft;
    rect.top = currentBottom + marginTop;

    //平铺绘制
    rect.right = rcPaint.right;
    rect.bottom = rcPaint.bottom;

    bool xtiled = true;
    bool ytiled = true;
    bool fullxtiled = false;
    bool fullytiled = false;//完整平铺，如果控件不够绘制完整图片，就不会绘制。如果为false，则只要有空间就绘制，但此时图片只是绘制一部分的。
    int nTiledMargin = 0;
    UiRect rcCorners(48, 48, 48, 48);
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, rcCorners, rcImageSource, rcCorners, 255, xtiled, ytiled, fullxtiled, fullytiled, nTiledMargin);
}

void RenderTest1::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

} //end of namespace ui