#include "Pen_GdiPlus.h"
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"

namespace ui {

Pen_GdiPlus::Pen_GdiPlus(UiColor color, int width /*= 1*/)
{
	m_color = color;
	m_pen.reset(new Gdiplus::Pen(color.GetARGB(), static_cast<Gdiplus::REAL>(width)));
}

Pen_GdiPlus::Pen_GdiPlus(const Pen_GdiPlus& r)
{
	m_color = r.m_color;
	m_pen.reset(r.m_pen->Clone());
}

ui::IPen* Pen_GdiPlus::Clone() const
{
	return new Pen_GdiPlus(*this);
}

void Pen_GdiPlus::SetWidth(int width)
{
	m_pen->SetWidth(static_cast<Gdiplus::REAL>(width));
}

int Pen_GdiPlus::GetWidth() const
{
	return (int)m_pen->GetWidth();
}

void Pen_GdiPlus::SetColor(UiColor color)
{
	m_color = color;
	m_pen->SetColor(color.GetARGB());
}

UiColor Pen_GdiPlus::GetColor() const
{
	return m_color;
}

void Pen_GdiPlus::SetStartCap(LineCap cap)
{
	m_pen->SetStartCap((Gdiplus::LineCap)cap);
}

void Pen_GdiPlus::SetEndCap(LineCap cap)
{
	m_pen->SetEndCap((Gdiplus::LineCap)cap);
}

void Pen_GdiPlus::SetDashCap(LineCap cap)
{
	m_pen->SetDashCap((Gdiplus::DashCap)cap);
}

IPen::LineCap Pen_GdiPlus::GetStartCap() const
{
	return (IPen::LineCap)m_pen->GetStartCap();
}

IPen::LineCap Pen_GdiPlus::GetEndCap() const
{
	return (IPen::LineCap)m_pen->GetEndCap();
}

IPen::LineCap Pen_GdiPlus::GetDashCap() const
{
	return (IPen::LineCap)m_pen->GetDashCap();
}

void Pen_GdiPlus::SetLineJoin(LineJoin join)
{
	m_pen->SetLineJoin((Gdiplus::LineJoin)join);
}

IPen::LineJoin Pen_GdiPlus::GetLineJoin() const
{
	return (IPen::LineJoin)m_pen->GetLineJoin();
}

void Pen_GdiPlus::SetDashStyle(DashStyle style)
{
	m_pen->SetDashStyle((Gdiplus::DashStyle)style);
}

IPen::DashStyle Pen_GdiPlus::GetDashStyle() const
{
	return (IPen::DashStyle)m_pen->GetDashStyle();
}

Gdiplus::Pen* Pen_GdiPlus::GetPen() const
{ 
	return m_pen.get();
};

} // namespace ui
