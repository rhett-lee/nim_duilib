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
    const int marginSize = 8;
    UiRect rect = GetRect();
    rect.left += marginSize;
    rect.top += 4;

    int nSize = 50;
    DpiManager::GetInstance()->ScaleInt(nSize);
    rect.right = rect.left + nSize;
    rect.bottom = rect.top + nSize;
    int currentBottom = rect.bottom;//记录当前的bottom值

    //绘制颜色
    pRender->DrawColor(rect, UiColor(0xFF000000));

    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    pRender->DrawColor(rect, UiColor(0xFF000000), 128);

    //画线
    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    pRender->DrawLine(UiPoint(rect.left, rect.top), UiPoint(rect.right, rect.bottom), UiColor(0xffC63535), 2);
    pRender->DrawLine(UiPoint(rect.right, rect.top), UiPoint(rect.left, rect.bottom), UiColor(0x23C63535), 4);

    //画四边形
    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    pRender->DrawRect(rect, UiColor(0xffC63535), 2);

    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    pRender->DrawRect(rect, UiColor(0x23C63535), 2);

    //画圆角四边形
    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    UiSize roundSize(12, 12);
    pRender->DrawRoundRect(rect, roundSize, UiColor(0xffC63535), 2);

    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    pRender->DrawRoundRect(rect, roundSize, UiColor(0x23C63535), 2);

    //用DrawPath画圆角四边形
    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;    
    IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), 2));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        const UiRect& rc = rect;
        path->AddArc((INT)rc.left, rc.top, roundSize.cx, roundSize.cy, 180, 90);
        path->AddLine(rc.left + roundSize.cx, (INT)rc.top, rc.right - roundSize.cx, rc.top);
        path->AddArc(rc.right - roundSize.cx, (INT)rc.top, roundSize.cx, roundSize.cy, 270, 90);
        path->AddLine((INT)rc.right, rc.top + roundSize.cy, rc.right, rc.bottom - roundSize.cy);
        path->AddArc(rc.right - roundSize.cx, rc.bottom - roundSize.cy, (INT)roundSize.cx, roundSize.cy, 0, 90);
        path->AddLine(rc.right - roundSize.cx, (INT)rc.bottom, rc.left + roundSize.cx, rc.bottom);
        path->AddArc((INT)rc.left, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 90, 90);
        path->AddLine((INT)rc.left, rc.bottom - roundSize.cy, rc.left, rc.top + roundSize.cy);
        path->CloseFigure();

        pRender->DrawPath(path.get(), pen.get());
    }

    //用FillPath画圆角四边形
    rect.left = rect.right + marginSize;
    rect.right = rect.left + nSize;
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), 2));
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(UiColor(0x23006DD9)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        const UiRect& rc = rect;
        path->AddArc((INT)rc.left, rc.top, roundSize.cx, roundSize.cy, 180, 90);
        path->AddLine(rc.left + roundSize.cx, (INT)rc.top, rc.right - roundSize.cx, rc.top);
        path->AddArc(rc.right - roundSize.cx, (INT)rc.top, roundSize.cx, roundSize.cy, 270, 90);
        path->AddLine((INT)rc.right, rc.top + roundSize.cy, rc.right, rc.bottom - roundSize.cy);
        path->AddArc(rc.right - roundSize.cx, rc.bottom - roundSize.cy, (INT)roundSize.cx, roundSize.cy, 0, 90);
        path->AddLine(rc.right - roundSize.cx, (INT)rc.bottom, rc.left + roundSize.cx, rc.bottom);
        path->AddArc((INT)rc.left, rc.bottom - roundSize.cy, roundSize.cx, roundSize.cy, 90, 90);
        path->AddLine((INT)rc.left, rc.bottom - roundSize.cy, rc.left, rc.top + roundSize.cy);
        path->CloseFigure();

        pRender->DrawPath(path.get(), pen.get());
        pRender->FillPath(path.get(), brush.get());
    }

    //DrawBoxShadow
    rect.left = rect.right + marginSize*2;
    rect.right = rect.left + nSize;
    pRender->DrawBoxShadow(rect, roundSize, UiPoint(0, 0), 4, 4, UiColor(0xffC63535), true);

    //绘制文字
    const UiRect oldRect = rect;
    std::wstring strText = L"绘制文字测试";
    UINT uFormat = TEXT_SINGLELINE | TEXT_VCENTER | TEXT_CENTER;
    UiRect textRect = pRender->MeasureString(strText, L"system_14", uFormat);
    rect.left = rect.right + marginSize;
    rect.right = rect.left + textRect.GetWidth();
    rect.bottom = rect.top + textRect.GetHeight();
    pRender->DrawRect(rect, UiColor(0x23C63535), 2);

    rect.top = rect.bottom + marginSize;
    rect.bottom = rect.top + textRect.GetHeight();

    pRender->DrawString(rect, strText, UiColor(0xFFC63535), L"system_14", uFormat);

    rect = oldRect;
    rect.left = rect.right + textRect.GetWidth() + marginSize * 2;
    rect.right = rect.left + textRect.GetWidth();
    uFormat = TEXT_SINGLELINE | TEXT_TOP | TEXT_CENTER;
    pRender->DrawString(rect, strText, UiColor(0xFFC63535), L"system_14", uFormat, 128);

    uFormat = TEXT_SINGLELINE | TEXT_BOTTOM | TEXT_CENTER;
    pRender->DrawString(rect, strText, UiColor(0xFFC63535), L"system_14", uFormat, 128);

    //换行
    rect = GetRect();
    rect.left += marginSize;    
    rect.top = currentBottom + marginSize;
    
    //绘图相同接口
    Image image;
    image.SetImageString(L"autumn.png");
    LoadImageData(image);
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    rect.bottom = rect.top + image.GetImageCache()->GetHeight();

    UiRect rcImageSource(0, 0, image.GetImageCache()->GetWidth(), image.GetImageCache()->GetHeight());
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, rcImageSource, UiRect());

    //半透明绘制图片
    rect.left = rect.right + marginSize;
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, rcImageSource, UiRect(), false, 100);

    //缩小绘制
    rect.left = rect.right + marginSize;
    rect.right = rect.left + image.GetImageCache()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() / 2;
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, rcImageSource, UiRect());

    //BitBlt/StretchBlt/AlphaBlend三个绘制函数
    IRender* pSrcRender = BitmapHelper::CreateRenderObject(image.GetCurrentBitmap());
    ASSERT(pSrcRender != nullptr);

    rect.left = rect.right + marginSize;
    rect.right = rect.left + image.GetImageCache()->GetWidth();
    rect.bottom = rect.top + image.GetImageCache()->GetHeight();
    pRender->BitBlt(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), pSrcRender, 0, 0, RopMode::kSrcCopy);

    rect.left = rect.right + marginSize;
    rect.right = rect.left + image.GetImageCache()->GetWidth() / 2;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() / 2;
    pRender->StretchBlt(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), pSrcRender, 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), RopMode::kSrcCopy);

    rect.left = rect.right + marginSize;
    rect.right = rect.left + image.GetImageCache()->GetWidth() ;
    rect.bottom = rect.top + image.GetImageCache()->GetHeight() ;
    pRender->AlphaBlend(rect.left, rect.top, rect.GetWidth() , rect.GetHeight() , pSrcRender, 0, 0, pSrcRender->GetWidth(), pSrcRender->GetHeight(), 96);

    delete pSrcRender;
    pSrcRender = nullptr;

    currentBottom = rect.bottom;//记录当前的bottom值

    //换行
    rect = GetRect();
    rect.left += marginSize;
    rect.top = currentBottom + marginSize;

    //平铺绘制
    rect.right = rcPaint.right;
    rect.bottom = rcPaint.bottom;

    bool xtiled = true;
    bool ytiled = true;
    bool fullxtiled = false;
    bool fullytiled = false;//完整平铺，如果控件不够绘制完整图片，就不会绘制。如果为false，则只要有空间就绘制，但此时图片只是绘制一部分的。
    int nTiledMargin = 0;
    UiRect rcCorners(48, 48, 48, 48);
    pRender->DrawImage(rcPaint, image.GetCurrentBitmap(), rect, rcImageSource, rcCorners, false, 255, xtiled, ytiled, fullxtiled, fullytiled, nTiledMargin);
}

void RenderTest1::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

} //end of namespace ui