#include "Line.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{

Line::Line(Window* pWindow):
    Control(pWindow),
    m_bLineVertical(false),
    m_dashStyle(IPen::kDashStyleDashDot),
    m_lineWidth(0)
{
}

std::wstring Line::GetType() const { return DUI_CTR_LINE; }

void Line::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"vertical") {
        SetLineVertical(strValue == L"true");
    }
    else if (strName == L"line_color") {
        SetLineColor(strValue);
    }
    else if (strName == L"line_width") {
        if (!strValue.empty()) {
            SetLineWidth(_wtoi(strValue.c_str()), true);
        }
    }
    else if (strName == L"dash_style") {
        SetLineDashStyle(strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void Line::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    int32_t iValue = GetLineWidth();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetLineWidth(iValue, false);

    __super::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Line::SetLineWidth(int32_t lineWidth, bool bNeedDpiScale)
{
    if (lineWidth < 0) {
        lineWidth = 1;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(lineWidth);
    }
    if (m_lineWidth != lineWidth) {
        m_lineWidth = lineWidth;
        Invalidate();
    }
}

int32_t Line::GetLineWidth() const
{
    return m_lineWidth;
}

void Line::SetLineVertical(bool bVertical)
{
    if (m_bLineVertical != bVertical) {
        m_bLineVertical = bVertical;
        Invalidate();
    }    
}

bool Line::IsLineVertical() const
{
    return m_bLineVertical;
}

void Line::SetLineColor(const std::wstring& lineColor)
{
    if (m_lineColor != lineColor) {
        m_lineColor = lineColor;
        Invalidate();
    }    
}

std::wstring Line::GetLineColor() const
{
    return m_lineColor.c_str();
}

void Line::SetLineDashStyle(const std::wstring& dashStyle)
{
    int32_t oldDashStyle = m_dashStyle;
    if (dashStyle == L"solid") {
        m_dashStyle = IPen::kDashStyleSolid;
    }
    else if (dashStyle == L"dash") {
        m_dashStyle = IPen::kDashStyleDash;
    }
    else if (dashStyle == L"dot") {
        m_dashStyle = IPen::kDashStyleDot;
    }
    else if (dashStyle == L"dash_dot") {
        m_dashStyle = IPen::kDashStyleDashDot;
    }
    else if (dashStyle == L"dash_dot_dot") {
        m_dashStyle = IPen::kDashStyleDashDotDot;
    }
    else {
        m_dashStyle = IPen::kDashStyleDashDot;
    }
    if (oldDashStyle != m_dashStyle) {
        Invalidate();
    }
}

std::wstring Line::GetLineDashStyle() const
{
    if (m_dashStyle == IPen::kDashStyleSolid) {
        return L"solid";
    }
    else if (m_dashStyle == IPen::kDashStyleDash) {
        return L"dash";
    }
    else if (m_dashStyle == IPen::kDashStyleDot) {
        return L"dot";
    }
    else if (m_dashStyle == IPen::kDashStyleDashDot) {
        return L"dash_dot";
    }
    else if (m_dashStyle == IPen::kDashStyleDashDotDot) {
        return L"dash_dot_dot";
    }
    else {
        return L"dash_dot";
    }
}

void Line::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    std::wstring sLineColor = m_lineColor.c_str();
    if (sLineColor.empty()) {
        sLineColor = GlobalManager::Instance().Color().GetDefaultTextColor();
    }
    UiColor lineColor = GetUiColor(sLineColor);
    int32_t lineWidth = m_lineWidth;
    if (lineWidth <= 0) {
        lineWidth = this->Dpi().GetScaleInt(1);
    }
    IPen* pLinePen = pRenderFactory->CreatePen(lineColor, lineWidth);
    std::unique_ptr<IPen> spLinePen(pLinePen);
    ASSERT(spLinePen != nullptr);
    if (spLinePen == nullptr) {
        return;
    }

    pLinePen->SetDashStyle(static_cast<IPen::DashStyle>(m_dashStyle));

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (!m_bLineVertical) {
        //Ë®Æ½
        UiPoint pt1(rc.left, rc.CenterY());
        UiPoint pt2(rc.right, rc.CenterY());
        pRender->DrawLine(pt1, pt2, pLinePen);
    }
    else {
        //´¹Ö±
        UiPoint pt1(rc.CenterX(), rc.top);
        UiPoint pt2(rc.CenterX(), rc.bottom);
        pRender->DrawLine(pt1, pt2, pLinePen);
    }
}

}//namespace ui

