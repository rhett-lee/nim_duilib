#include "RenderTest3.h"
#include "duilib/Utils/BitmapHelper_Windows.h"

namespace ui {

RenderTest3::RenderTest3(ui::Window* pWindow):
    ui::Control(pWindow)
{
}

RenderTest3::~RenderTest3()
{
}

void RenderTest3::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::AlphaPaint(pRender, rcPaint);
}

void RenderTest3::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);

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

    //画直线
    const int sep = DpiScaledInt(10);
    for (int32_t topValue = rect.top; topValue <= rect.bottom; topValue += sep) {
        pRender->DrawLine(UiPoint(rect.left, topValue), UiPoint(rect.right, topValue), UiColor(UiColors::DarkCyan), DpiScaledInt(4));
    }
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawLine"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //画各种线形的线
    rect.Offset(UiPoint(rect.Width() + 10, 0));
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(UiColors::CornflowerBlue), DpiScaledInt(2)));
        int32_t style = 0;
        for (int32_t topValue = rect.top; topValue <= rect.bottom; topValue += sep) {
            if (style == 0) {
                pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleSolid);
            }
            else if (style == 1) {
                pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDash);
            }
            else if (style == 2) {
                pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDot);
            }
            else if (style == 3) {
                pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDot);
            }
            else if (style == 4) {
                pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDotDot);
            }
            pRender->DrawLine(UiPoint(rect.left, topValue), UiPoint(rect.right, topValue), pen.get());
            ++style;
            if (style > 4) {
                style = 0;
            }
        }
    }


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
    pRender->DrawString(textRect, _T("DrawRect"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //画各种线形的矩形
    for (size_t style = 0; style < 5; ++style) {
        rect.left = rect.right + marginLeft;
        rect.right = rect.left + nSize;

        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(UiColors::CornflowerBlue), DpiScaledInt(2)));
        if (style == 0) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleSolid);
        }
        else if (style == 1) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDash);
        }
        else if (style == 2) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDot);
        }
        else if (style == 3) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDot);
        }
        else if (style == 4) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDotDot);
        }
        pRender->DrawRect(rect, pen.get());
    }
    

    //填充矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColors::Brown));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillRect"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRect(rect, UiColor(UiColors::Brown), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillRect Alpha"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

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

    //圆角大小，必须是偶数，否则使用路径的时候，不能闭合
    UiSize roundSize(12, 12);
    Dpi().ScaleSize(roundSize);
    roundSize.cx = (roundSize.cx / 2) * 2;
    roundSize.cy = (roundSize.cy / 2) * 2;

    pRender->DrawRoundRect(rect, roundSize, UiColor(0xffC63535), DpiScaledInt(2));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawRoundRect"), UiColor(UiColors::Blue), GetIFontById(_T("system_12")), TEXT_CENTER);

    //画各种线形的圆角矩形
    for (size_t style = 0; style < 5; ++style) {
        rect.left = rect.right + marginLeft;
        rect.right = rect.left + nSize;

        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(UiColors::CornflowerBlue), DpiScaledInt(2)));
        if (style == 0) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleSolid);
        }
        else if (style == 1) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDash);
        }
        else if (style == 2) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDot);
        }
        else if (style == 3) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDot);
        }
        else if (style == 4) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDotDot);
        }
        pRender->DrawRoundRect(rect, roundSize, pen.get());
    }

    //填充圆角矩形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColors::Blue));
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillRoundRect"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillRoundRect(rect, roundSize, UiColor(UiColors::Blue), 128);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillRoundRect Alpha"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //换行
    currentBottom = textRect.bottom;//记录当前的bottom值
    rect = GetRect();
    rect.left += marginLeft;
    rect.right = rect.left;
    rect.top = currentBottom + marginTop;
    rect.bottom = rect.top + nSize;

    //画圆形/填充圆形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    int32_t radius = std::min(rect.Width(), rect.Height()) / 2;//圆的半径
    pRender->DrawCircle(rect.Center(), radius, UiColor(UiColors::Blue), 2);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawCircle"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //画各种线形的圆形
    for (size_t style = 0; style < 5; ++style) {
        rect.left = rect.right + marginLeft;
        rect.right = rect.left + nSize;

        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(UiColors::CornflowerBlue), DpiScaledInt(2)));
        if (style == 0) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleSolid);
        }
        else if (style == 1) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDash);
        }
        else if (style == 2) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDot);
        }
        else if (style == 3) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDot);
        }
        else if (style == 4) {
            pen->SetDashStyle(ui::IPen::DashStyle::kDashStyleDashDotDot);
        }
        pRender->DrawCircle(rect.Center(), radius, pen.get());
    }

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillCircle(rect.Center(), radius, UiColor(UiColors::CadetBlue), 255);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillCircle"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    pRender->FillCircle(rect.Center(), radius, UiColor(UiColors::CadetBlue), 96);
    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillCircle Alpha"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);
    
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
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if (pen && path) {
            const UiRect& rc = rect;
            path->AddArc(UiRect(rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
            path->AddLine(rc.left + roundSize.cx / 2, rc.top, rc.right - roundSize.cx / 2, rc.top);
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
            path->AddLine(rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
            path->AddLine(rc.right - roundSize.cx / 2, rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
            path->AddArc(UiRect(rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
            path->AddLine(rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
            path->Close();

            pRender->DrawPath(path.get(), pen.get());
        }
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawPath"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //用FillPath画圆角四边形
    rect.left = rect.right + marginLeft;
    rect.right = rect.left + nSize;
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(UiColor(0xff006DD9), DpiScaledInt(2)));
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(UiColor(UiColors::Red)));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());

        if (pen && brush && path) {
            const UiRect& rc = rect;

            path->AddArc(UiRect(rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
            path->AddLine(rc.left + roundSize.cx / 2, rc.top, rc.right - roundSize.cx / 2, rc.top);                      
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
            path->AddLine(rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
            path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
            path->AddLine(rc.right - roundSize.cx / 2, rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
            path->AddArc(UiRect(rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
            path->AddLine(rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
            path->Close();

            pRender->DrawPath(path.get(), pen.get());
            pRender->FillPath(path.get(), brush.get());
        }
    }

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("FillPath"), UiColor(UiColors::Blue), GetIFontById(_T("system_14")), TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2;
    rect.right = rect.left + nSize;
    int nBlurRadius = DpiScaledInt(6);
    int nSpreadRadius = DpiScaledInt(2);
    pRender->DrawBoxShadow(rect, roundSize, UiPoint(0, 0), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawBoxShadow"), UiColor(UiColors::Blue), GetIFontById(_T("system_12")), TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2 + 40;
    rect.right = rect.left + nSize;
    nBlurRadius = DpiScaledInt(6);
    nSpreadRadius = DpiScaledInt(4);
    pRender->DrawBoxShadow(rect, UiSize(0, 0), UiPoint(0, 0), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawBoxShadow"), UiColor(UiColors::Blue), GetIFontById(_T("system_12")), TEXT_CENTER);

    //DrawBoxShadow
    rect.left = rect.right + marginLeft * 2 + 40;
    rect.right = rect.left + nSize;
    nBlurRadius = DpiScaledInt(4);
    nSpreadRadius = DpiScaledInt(4);
    pRender->DrawBoxShadow(rect, UiSize(0, 0), UiPoint(8, 8), nBlurRadius, nSpreadRadius, UiColor(0xffC63535));

    textRect = rect;
    textRect.top = rect.bottom;
    textRect.bottom = textRect.top + nTextLineHeight;
    pRender->DrawString(textRect, _T("DrawBoxShadow"), UiColor(UiColors::Blue), GetIFontById(_T("system_12")), TEXT_CENTER);
}

void RenderTest3::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::PaintChild(pRender, rcPaint);
}

int RenderTest3::DpiScaledInt(int iValue)
{
    return Dpi().GetScaleInt(iValue);
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
