#include "Line.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

namespace ui
{

Line::Line():
    m_bLineVertical(false),
    m_dashStyle(IPen::kDashStyleDashDot)
{
    m_lineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
}

std::wstring Line::GetType() const { return DUI_CTR_LINE; }

void Line::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"vertical") {
        m_bLineVertical = (strValue == L"true");
    }
    else if (strName == L"line_color") {
        if (!strValue.empty()) {
            m_lineColor = GetUiColor(strValue);
        }
    }
    else if (strName == L"line_width") {
        if (!strValue.empty()) {
            m_lineWidth = GlobalManager::Instance().Dpi().GetScaleInt(_wtoi(strValue.c_str()));
            if (m_lineWidth <= 0) {
                m_lineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
            }
        }
    }
    else if (strName == L"dash_style") {
        if (strValue == L"solid") {
            m_dashStyle = IPen::kDashStyleSolid;
        }
        else if (strValue == L"dash") {
            m_dashStyle = IPen::kDashStyleDash;
        }
        else if (strValue == L"dot") {
            m_dashStyle = IPen::kDashStyleDot;
        }
        else if (strValue == L"dash_dot") {
            m_dashStyle = IPen::kDashStyleDashDot;
        }
        else if (strValue == L"dash_dot_dot") {
            m_dashStyle = IPen::kDashStyleDashDotDot;
        }
        else {
            m_dashStyle = IPen::kDashStyleDashDot;
        }
    }
    else {
        __super::SetAttribute(strName, strValue);
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

    if (m_lineColor.IsEmpty()) {
        std::wstring colorStr = GlobalManager::Instance().Color().GetDefaultTextColor();
        m_lineColor = GetUiColor(colorStr);
    }

    IPen* pLinePen = pRenderFactory->CreatePen(m_lineColor, m_lineWidth);
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

