#include "Line.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

Line::Line(Window* pWindow):
    Control(pWindow),
    m_bLineVertical(false),
    m_dashStyle((int8_t)IPen::kDashStyleDashDot),
    m_fLineWidth(0)
{
    SetLineWidth(1.0f, true);
}

DString Line::GetType() const { return DUI_CTR_LINE; }

void Line::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("vertical")) {
        SetLineVertical(strValue == _T("true"));
    }
    else if (strName == _T("line_color")) {
        SetLineColor(strValue);
    }
    else if (strName == _T("line_width")) {
        if (!strValue.empty()) {
            SetLineWidth((float)StringUtil::StringToInt32(strValue), true);
        }
    }
    else if (strName == _T("dash_style")) {
        SetLineDashStyle(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Line::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    float fValue = GetLineWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetLineWidth(fValue, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Line::SetLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (fLineWidth < 0) {
        fLineWidth = 1.0f;
    }
    if (bNeedDpiScale) {
        fLineWidth = Dpi().GetScaleFloat(fLineWidth);
    }
    if (m_fLineWidth != fLineWidth) {
        m_fLineWidth = fLineWidth;
        Invalidate();
    }
}

float Line::GetLineWidth() const
{
    return m_fLineWidth;
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

void Line::SetLineColor(const DString& lineColor)
{
    if (m_lineColor != lineColor) {
        m_lineColor = lineColor;
        Invalidate();
    }    
}

DString Line::GetLineColor() const
{
    return m_lineColor.c_str();
}

void Line::SetLineDashStyle(const DString& dashStyle)
{
    int32_t oldDashStyle = m_dashStyle;
    if (dashStyle == _T("solid")) {
        m_dashStyle = IPen::kDashStyleSolid;
    }
    else if (dashStyle == _T("dash")) {
        m_dashStyle = IPen::kDashStyleDash;
    }
    else if (dashStyle == _T("dot")) {
        m_dashStyle = IPen::kDashStyleDot;
    }
    else if (dashStyle == _T("dash_dot")) {
        m_dashStyle = IPen::kDashStyleDashDot;
    }
    else if (dashStyle == _T("dash_dot_dot")) {
        m_dashStyle = IPen::kDashStyleDashDotDot;
    }
    else {
        m_dashStyle = IPen::kDashStyleDashDot;
    }
    if (oldDashStyle != m_dashStyle) {
        Invalidate();
    }
}

DString Line::GetLineDashStyle() const
{
    if (m_dashStyle == IPen::kDashStyleSolid) {
        return _T("solid");
    }
    else if (m_dashStyle == IPen::kDashStyleDash) {
        return _T("dash");
    }
    else if (m_dashStyle == IPen::kDashStyleDot) {
        return _T("dot");
    }
    else if (m_dashStyle == IPen::kDashStyleDashDot) {
        return _T("dash_dot");
    }
    else if (m_dashStyle == IPen::kDashStyleDashDotDot) {
        return _T("dash_dot_dot");
    }
    else {
        return _T("dash_dot");
    }
}

void Line::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }

    DString sLineColor = m_lineColor.c_str();
    if (sLineColor.empty()) {
        sLineColor = GlobalManager::Instance().Color().GetDefaultTextColor();
    }
    UiColor lineColor = GetUiColor(sLineColor);
    float fLineWidth = GetLineWidth();
    if (fLineWidth <= 0) {
        fLineWidth = this->Dpi().GetScaleFloat(1);
    }
    IPen* pLinePen = pRenderFactory->CreatePen(lineColor, fLineWidth);
    ASSERT(pLinePen != nullptr);
    if (pLinePen == nullptr) {
        return;
    }
    std::unique_ptr<IPen> spLinePen(pLinePen);
    ASSERT(spLinePen != nullptr);
    if (spLinePen == nullptr) {
        return;
    }

    pLinePen->SetDashStyle(static_cast<IPen::DashStyle>(m_dashStyle));

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if (!m_bLineVertical) {
        //水平
        UiPoint pt1(rc.left, rc.CenterY());
        UiPoint pt2(rc.right, rc.CenterY());
        pRender->DrawLine(pt1, pt2, pLinePen);
    }
    else {
        //垂直
        UiPoint pt1(rc.CenterX(), rc.top);
        UiPoint pt2(rc.CenterX(), rc.bottom);
        pRender->DrawLine(pt1, pt2, pLinePen);
    }
}

}//namespace ui

