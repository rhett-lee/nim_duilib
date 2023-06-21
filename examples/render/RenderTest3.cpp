#include "RenderTest3.h"
#include "duilib/RenderGdiPlus/BitmapHelper.h"

namespace ui {

RenderTest3::RenderTest3()
{

}

RenderTest3::~RenderTest3()
{

}

void RenderTest3::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    __super::AlphaPaint(pRender, rcPaint);
}

void RenderTest3::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
    int marginLeft = 8;
    int marginTop = 4;
    DpiManager::GetInstance()->ScaleInt(marginLeft);
    DpiManager::GetInstance()->ScaleInt(marginTop);

    UiRect rect = GetRect();
    UiRect textRect = rect;
    rect.left += marginLeft;
    rect.top += marginTop;

    int nSize = 110;
    int nTextLineHeight = 40;
    DpiManager::GetInstance()->ScaleInt(nSize);
    DpiManager::GetInstance()->ScaleInt(nTextLineHeight);

    rect.right = rect.left + nSize;
    rect.bottom = rect.top + nSize;
    int currentBottom = rect.bottom;//记录当前的bottom值

    //画直线
    int sep = DpiScaledInt(10);
    for (LONG topValue = rect.top; topValue <= rect.bottom; topValue += sep) {
        pRender->DrawLine(UiPoint(rect.left, topValue), UiPoint(rect.right, topValue), UiColor(UiColor::DarkCyan), DpiScaledInt(4));
    }
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawLine", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //换行
    currentBottom = textRect.bottom;//记录当前的bottom值
    rect = GetRect();
    rect.left += marginLeft;
    rect.right = rect.left;
    rect.top = currentBottom + marginTop;
    rect.bottom = rect.top + nSize;
    
    //画矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->DrawRect(rect, UiColor(UiColor::Fuchsia), DpiScaledInt(2));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawRect", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //填充矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColor::Brown));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRect", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColor::Brown), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRect Alpha", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //换行
    currentBottom = textRect.bottom;//记录当前的bottom值
    rect = GetRect();
    rect.left += marginLeft;
    rect.right = rect.left;
    rect.top = currentBottom + marginTop;
    rect.bottom = rect.top + nSize;

    //画圆角矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    UiSize roundSize(24, 24);
    DpiManager::GetInstance()->ScaleSize(roundSize);
    pRender->DrawRoundRect(rect, roundSize, UiColor(0xffC63535), DpiScaledInt(2));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawRoundRect", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //填充圆角矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColor::Blue));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRoundRect", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColor::Blue), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRoundRect Alpha", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //换行
    currentBottom = textRect.bottom;//记录当前的bottom值
    rect = GetRect();
    rect.left += marginLeft;
    rect.right = rect.left;
    rect.top = currentBottom + marginTop;
    rect.bottom = rect.top + nSize;

    //用DrawPath画圆角四边形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if (pen && path) {
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
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawPath", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //用FillPath画圆角四边形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(UiColor(0x23006DD9)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        if (pen && brush && path) {
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
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillPath", UiColor(UiColor::Blue), L"system_14", TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2;
    rect.right = rect.left + nSize;
    pRender->DrawBoxShadow(rect, roundSize, UiPoint(0, 0), 4, 4, UiColor(0xffC63535), true);
}

void RenderTest3::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

int RenderTest3::DpiScaledInt(int iValue)
{
    return DpiManager::GetInstance()->GetScaledInt(iValue);
}

} //end of namespace ui