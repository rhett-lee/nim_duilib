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

    //画直线
    int sep = DpiScaledInt(10);
    for (LONG topValue = rect.top; topValue <= rect.bottom; topValue += sep) {
        pRender->DrawLine(UiPoint(rect.left, topValue), UiPoint(rect.right, topValue), UiColor(UiColors::DarkCyan), DpiScaledInt(4));
    }
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawLine", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //画一个正六边形
    UiRect hexagonRect = rect;
    hexagonRect.Offset(UiPoint(rect.Width() + 10, 0));
    DrawRegularHexagon3(pRender, hexagonRect.Center(), rect.Width() / 2, UiColor(UiColors::White), 2, UiColor(UiColors::Olive));

    //画一个正六边形
    hexagonRect.Offset(UiPoint(rect.Width() + 10, 0));
    UiPointF centerF((float)hexagonRect.CenterX(), (float)hexagonRect.CenterY());
    DrawRegularHexagon(pRender, centerF, rect.Width() / 2, UiColor(UiColors::White), 2, UiColor(UiColors::SandyBrown));

    //用正六边形拼接一个复杂图形
    hexagonRect.Offset(UiPoint(rect.Width() + 10, 0));
    DrawColorMap(pRender, hexagonRect);

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
    pRender->DrawRect(rect, UiColor(UiColors::Fuchsia), DpiScaledInt(2));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawRect", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //填充矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColors::Brown));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRect", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColors::Brown), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRect Alpha", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

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
    UiSize roundSize(12, 12);
    GlobalManager::Instance().Dpi().ScaleSize(roundSize);
    pRender->DrawRoundRect(rect, roundSize, UiColor(0xffC63535), DpiScaledInt(2));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawRoundRect", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //填充圆角矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColors::Blue));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRoundRect", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColors::Blue), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillRoundRect Alpha", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //画圆形/填充圆形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    int32_t radius = std::min(rect.Width(), rect.Height()) / 2;//圆的半径
    pRender->DrawCircle(rect.Center(), radius, UiColor(UiColors::Blue), 2);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawCircle", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillCircle(rect.Center(), radius, UiColor(UiColors::CadetBlue), 255);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillCircle", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillCircle(rect.Center(), radius, UiColor(UiColors::CadetBlue), 96);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillCircle Alpha", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);
    
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
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if (pen && path) {
            const UiRect& rc = rect;
            path->AddArc(UiRect((INT)rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
            path->AddLine(rc.left + roundSize.cx / 2, (INT)rc.top, rc.right - roundSize.cx / 2, rc.top);
            path->AddArc(UiRect(rc.right - roundSize.cx, (INT)rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
            path->AddLine((INT)rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
            path->AddLine(rc.right - roundSize.cx / 2, (INT)rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
            path->AddArc(UiRect((INT)rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
            path->AddLine((INT)rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
            path->Close();

            pRender->DrawPath(path.get(), pen.get());
        }
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawPath", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //用FillPath画圆角四边形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(UiColor(UiColors::Red)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        if (pen && brush && path) {
            const UiRect& rc = rect;

            path->AddArc(UiRect((INT)rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
            path->AddLine(rc.left + roundSize.cx / 2, (INT)rc.top, rc.right - roundSize.cx / 2, rc.top);                      
            path->AddArc(UiRect(rc.right - roundSize.cx, (INT)rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
            path->AddLine((INT)rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
            path->AddLine(rc.right - roundSize.cx / 2, (INT)rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
            path->AddArc(UiRect(rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
            path->AddLine((INT)rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
            path->Close();

            pRender->DrawPath(path.get(), pen.get());
            pRender->FillPath(path.get(), brush.get());
        }
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"FillPath", UiColor(UiColors::Blue), L"system_14", TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2;
    rect.right = rect.left + nSize;
    int nBlurRadius = DpiScaledInt(6);
    int nSpreadRadius = DpiScaledInt(2);
    pRender->DrawBoxShadow(rect, roundSize, UiPoint(0, 0), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawBoxShadow", UiColor(UiColors::Blue), L"system_12", TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2 + 40;
    rect.right = rect.left + nSize;
    nBlurRadius = DpiScaledInt(6);
    nSpreadRadius = DpiScaledInt(4);
    pRender->DrawBoxShadow(rect, UiSize(0, 0), UiPoint(0, 0), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawBoxShadow", UiColor(UiColors::Blue), L"system_12", TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2 + 40;
    rect.right = rect.left + nSize;
    nBlurRadius = DpiScaledInt(4);
    nSpreadRadius = DpiScaledInt(4);
    pRender->DrawBoxShadow(rect, UiSize(0, 0), UiPoint(8, 8), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, L"DrawBoxShadow", UiColor(UiColors::Blue), L"system_12", TEXT_CENTER);
}

void RenderTest3::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

int RenderTest3::DpiScaledInt(int iValue)
{
    return GlobalManager::Instance().Dpi().GetScaleInt(iValue);
}

void RenderTest3::DrawColorMap(IRender* pRender, const UiRect& rect)
{
    int32_t radius = static_cast<int32_t>(rect.Width() / 13 / 2 / std::cos(30 / 57.2957795f)); //半径
    const float distance = radius * std::cos(30 / 57.2957795f); //中心点到边的垂直距离

    UiPointF firstCenterPt = UiPointF((float)rect.CenterX(), (float)rect.CenterY()); //矩形中心点坐标
    firstCenterPt.x = firstCenterPt.x - distance * 2 * 6 * std::sin(30 / 57.2957795f); //第一个六边形中心点X坐标
    firstCenterPt.y = firstCenterPt.y - distance * 2 * 6 * std::cos(30 / 57.2957795f); //第一个六边形中心点Y坐标

    for (int32_t y = 0; y < 13; ++y) { //共计13行
        int32_t count = 0;
        if (y < 7) {
            count = 7 + y;
        }
        else {
            count = 7 + (13 - y - 1);
        }
        for (int32_t x = 0; x < count; ++x) {
            UiPointF centerPt = firstCenterPt;
            if (y < 7) {
                centerPt.x += distance * 2 * x - distance * y;
            }
            else {
                centerPt.x += distance * 2 * x - distance * (13 - y - 1);
            }
            centerPt.y += radius * 1.5f * y;
            DrawRegularHexagon(pRender, centerPt, radius, UiColor(UiColors::Blue), 1, UiColor(UiColors::Salmon));
        }
    }
}

bool RenderTest3::DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius,
                                     const UiColor& penColor, int32_t penWidth, const UiColor& brushColor)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    ASSERT(radius > 0); //多边形的半径
    if (radius <= 0) {
        return false;
    }

    const int32_t count = 6; //多边形的边数
    //正多边形上任意一个顶点的坐标为： x = r * cos(θ) y = r * sin(θ) 
    std::vector<UiPointF> polygonPoints;
    for (int32_t i = 0; i < count; ++i) {
        int32_t degree = i * 60 + 30;// +30是为了使顶点在中心点的最上方
        float radian = degree / 57.2957795f;
        float x = radius * std::cos(radian) + 0.5f;
        float y = radius * std::sin(radian) + 0.5f;
        polygonPoints.push_back(UiPointF(centerPt.x + x, centerPt.y + y));
    }

    std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
    path->AddPolygon(polygonPoints.data(), (int32_t)polygonPoints.size());
    path->Close();

    bool bRet = false;
    if (brushColor.GetARGB() != 0) {
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
        pRender->FillPath(path.get(), brush.get());
        bRet = true;
    }
    if ((penColor.GetARGB() != 0) && (penWidth > 0)) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(penColor, penWidth));
        pRender->DrawPath(path.get(), pen.get());
        bRet = true;
    }
    return bRet;
}

bool RenderTest3::DrawRegularHexagon3(IRender* pRender, const UiPoint& centerPt, int32_t radius,
                                      const UiColor& penColor, int32_t penWidth, const UiColor& brushColor)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }

    bool bRet = false;
    const int count = 6;
    UiPoint oldWindowOrg = pRender->SetWindowOrg(UiPoint(centerPt.x, centerPt.y));

    //开始绘制多边形，并为每个区块上色
    for (int i = 0; i < count; ++i)
    {
        //设中心点到边的垂线与半径的夹角为degree=(360/count)/2即：
        float degree = 180.0f / count;

        float radian = degree / 57.2957795f;
        int32_t width = static_cast<int32_t>(radius * std::sin(radian));
        int32_t height = static_cast<int32_t>(radius * std::cos(radian));

        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        //绘制该三角区块
        path->AddLine(0, 0, -width, -height);
        path->AddLine(-width, -height, width, -height);
        path->AddLine(width, -height, 0, 0);

        std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
        if (spMatrix != nullptr) {
            float angle = 2 * degree * i;
            spMatrix->RotateAt(angle, UiPoint(0, 0));
            path->Transform(spMatrix.get());
        }        
        if (brushColor.GetARGB() != 0) {
            std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(brushColor));
            pRender->FillPath(path.get(), brush.get());
            bRet = true;
        }
        if ((penColor.GetARGB() != 0) && (penWidth > 0)) {
            std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(penColor, penWidth));
            pRender->DrawPath(path.get(), pen.get());
            bRet = true;
        }
    }
    pRender->SetWindowOrg(oldWindowOrg);
    return bRet;
}

} //end of namespace ui