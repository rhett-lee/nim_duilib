#include "Pen_Skia.h"

namespace ui {

Pen_Skia::Pen_Skia(UiColor color, int width /*= 1*/):
	m_color(color),
	m_width(width),
	m_startCap(kButt_Cap),
	m_dashCap(kButt_Cap),
	m_endCap(kButt_Cap),
	m_lineJoin(kMiter_Join),
	m_dashStyle(kDashStyleSolid)
{
}

Pen_Skia::Pen_Skia(const Pen_Skia& r)
{
	m_color = r.m_color;
	m_width = r.m_width;
	m_startCap = r.m_startCap;
	m_dashCap = r.m_dashCap;
	m_endCap = r.m_endCap;
	m_lineJoin = r.m_lineJoin;
	m_dashStyle = r.m_dashStyle;
}

ui::IPen* Pen_Skia::Clone() const
{
	return new Pen_Skia(*this);
}

void Pen_Skia::SetWidth(int width)
{
	m_width = width;
}

int Pen_Skia::GetWidth() const
{
	return m_width;
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
}

IPen::DashStyle Pen_Skia::GetDashStyle() const
{
	return m_dashStyle;
}

} // namespace ui
