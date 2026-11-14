#include "Pen_Skia.h"

#include <algorithm>

namespace ui {

Pen_Skia::Pen_Skia(UiColor color, float fWidth):
    m_color(color),
    m_fWidth(fWidth),
    m_startCap(kButt_Cap),
    m_dashCap(kButt_Cap),
    m_endCap(kButt_Cap),
    m_lineJoin(kMiter_Join),
    m_dashStyle(kDashStyleSolid),
    m_bAntiAlias(true),
    m_fMiterLimit(4.0f)
{
}

Pen_Skia::Pen_Skia(const Pen_Skia& r)
{
    m_color = r.m_color;
    m_fWidth = r.m_fWidth;
    m_startCap = r.m_startCap;
    m_dashCap = r.m_dashCap;
    m_endCap = r.m_endCap;
    m_lineJoin = r.m_lineJoin;
    m_dashStyle = r.m_dashStyle;
    m_dashPattern = r.m_dashPattern;
    m_bAntiAlias = r.m_bAntiAlias;
    m_fMiterLimit = r.m_fMiterLimit;
}

std::unique_ptr<IPen> Pen_Skia::Clone() const
{
    return std::make_unique<Pen_Skia>(*this);
}

void Pen_Skia::SetWidth(float fWidth)
{
    m_fWidth = fWidth;
}

float Pen_Skia::GetWidth() const
{
    return m_fWidth;
}

void Pen_Skia::SetColor(UiColor color)
{
    m_color = color;
}

UiColor Pen_Skia::GetColor() const
{
    return m_color;
}

void Pen_Skia::SetStartCap(LineCap cap)
{
    m_startCap = cap;
}

IPen::LineCap Pen_Skia::GetStartCap() const
{
    return m_startCap;
}

void Pen_Skia::SetEndCap(LineCap cap)
{
    m_endCap = cap;
}

IPen::LineCap Pen_Skia::GetEndCap() const
{
    return m_endCap;
}

void Pen_Skia::SetDashCap(LineCap cap)
{
    m_dashCap = cap;
}

IPen::LineCap Pen_Skia::GetDashCap() const
{
    return m_dashCap;
}

void Pen_Skia::SetLineJoin(LineJoin join)
{
    m_lineJoin = join;
}

IPen::LineJoin Pen_Skia::GetLineJoin() const
{
    return m_lineJoin;
}

void Pen_Skia::SetDashStyle(DashStyle style)
{
    m_dashStyle = style;
    //切换预设样式时，清除自定义虚线样式（以最后一次设置为准）
    m_dashPattern.clear();
}

IPen::DashStyle Pen_Skia::GetDashStyle() const
{
    return m_dashStyle;
}

void Pen_Skia::SetDashPattern(const float* intervals, int32_t count)
{
    m_dashPattern.clear();
    if ((intervals == nullptr) || (count <= 0) || ((count % 2) != 0)) {
        //传0或非法参数表示清除自定义虚线样式
        return;
    }
    m_dashPattern.assign(intervals, intervals + count);
}

bool Pen_Skia::HasDashPattern() const
{
    return !m_dashPattern.empty();
}

void Pen_Skia::SetAntiAlias(bool bAntiAlias)
{
    m_bAntiAlias = bAntiAlias;
}

bool Pen_Skia::IsAntiAlias() const
{
    return m_bAntiAlias;
}

void Pen_Skia::SetMiterLimit(float fMiterLimit)
{
    //斜接限制的合法取值为大于等于1
    m_fMiterLimit = std::max(fMiterLimit, 1.0f);
}

float Pen_Skia::GetMiterLimit() const
{
    return m_fMiterLimit;
}

const std::vector<float>& Pen_Skia::GetDashPattern() const
{
    return m_dashPattern;
}

} // namespace ui
