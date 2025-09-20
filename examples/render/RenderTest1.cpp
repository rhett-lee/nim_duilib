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

void RenderTest1::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::AlphaPaint(pRender, rcPaint);
}

void RenderTest1::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    int marginLeft = 8;
    int marginTop = 4;
    Dpi().ScaleInt(marginLeft);
    Dpi().ScaleInt(marginTop);

    UiRect rect = GetRect();
    UiRect textRect = rect;
    rect.left += marginLeft;
    rect.top += marginTop;

    int nSize = 110;
    int nTextLineHeight = 40;
    Dpi().ScaleInt(nSize);
    Dpi().ScaleInt(nTextLineHeight);

    rect.right = rect.left + nSize;
    rect.bottom = rect.top + nSize;
    int currentBottom = rect.bottom;//记录当前的bottom值
    
    //绘图相同接口
    Image image;
    image.SetImageString(_T("file='autumn.png' async_load='false'"), Dpi());
    LoadImageInfo(image);
    rect.right = rect.left + image.GetImageInfo()->GetWidth();
    rect.bottom = rect.top + image.GetImageInfo()->GetHeight();

    UiRect rcImageSource(0, 0, image.GetImageInfo()->GetWidth(), image.GetImageInfo()->GetHeight());
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap().get(), rect, UiRect(), rcImageSource, UiRect());

    //半透明绘制图片
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageInfo()->GetWidth();
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap().get(), rect, UiRect(), rcImageSource, UiRect(), 100);

    //缩小绘制
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageInfo()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageInfo()->GetHeight() / 2;
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap().get(), rect, UiRect(), rcImageSource, UiRect());

    //BitBlt/StretchBlt/AlphaBlend三个绘制函数
    IRender* pSrcRender = BitmapHelper::CreateRenderObject(image.GetCurrentBitmap().get());
    ASSERT(pSrcRender != nullptr);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageInfo()->GetWidth();
    rect.bottom = rect.top + image.GetImageInfo()->GetHeight();
    pRender->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender, 0, 0, RopMode::kSrcCopy);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageInfo()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageInfo()->GetHeight() / 2;
    pRender->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), pSrcRender, 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), RopMode::kSrcCopy);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + image.GetImageInfo()->GetWidth() ;
    rect.bottom = rect.top + image.GetImageInfo()->GetHeight() ;
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
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap().get(), rect, rcCorners, rcImageSource, rcCorners, 255, xtiled, ytiled, fullxtiled, fullytiled, nTiledMargin);
}

void RenderTest1::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::PaintChild(pRender, rcPaint);
}

} //end of namespace ui
