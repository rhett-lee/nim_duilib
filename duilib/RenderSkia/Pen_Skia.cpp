#include "Pen_Skia.h"

namespace ui {

Pen_Skia::Pen_Skia(UiColor color, int width /*= 1*/)
{
	m_color = color;
	//pen_.reset(new Gdiplus::Pen(color_.GetARGB(), static_cast<Gdiplus::REAL>(width)));
}

Pen_Skia::Pen_Skia(const Pen_Skia& r)
{
	m_color = r.m_color;
	//pen_.reset(r.pen_->Clone());
}

ui::IPen* Pen_Skia::Clone() const
{
	return new Pen_Skia(*this);
}

void Pen_Skia::SetWidth(int width)
{
	//pen_->SetWidth(static_cast<Gdiplus::REAL>(width));
}

int Pen_Skia::GetWidth() const
{
	return 0;
	//return (int)pen_->GetWidth();
}

void Pen_Skia::SetColor(UiColor color)
{
	m_color = color;
	//pen_->SetColor(color.GetARGB());
}

UiColor Pen_Skia::GetColor() const
{
	return m_color;
}

void Pen_Skia::SetStartCap(LineCap cap)
{
	//pen_->SetStartCap((Gdiplus::LineCap)cap);
}

void Pen_Skia::SetEndCap(LineCap cap)
{
	//pen_->SetEndCap((Gdiplus::LineCap)cap);
}

void Pen_Skia::SetDashCap(LineCap cap)
{
	//pen_->SetDashCap((Gdiplus::DashCap)cap);
}

IPen::LineCap Pen_Skia::GetStartCap() const
{
	return IPen::LineCap::LineCapFlat;
	//return (IPen::LineCap)pen_->GetStartCap();
}

IPen::LineCap Pen_Skia::GetEndCap() const
{
	return IPen::LineCap::LineCapFlat;
	//return (IPen::LineCap)pen_->GetEndCap();
}

IPen::LineCap Pen_Skia::GetDashCap() const
{
	return IPen::LineCap::LineCapFlat;
	//return (IPen::LineCap)pen_->GetDashCap();
}

void Pen_Skia::SetLineJoin(LineJoin join)
{
	//pen_->SetLineJoin((Gdiplus::LineJoin)join);
}

IPen::LineJoin Pen_Skia::GetLineJoin() const
{
	return IPen::LineJoin::LineJoinBevel;
	//return (IPen::LineJoin)pen_->GetLineJoin();
}

void Pen_Skia::SetDashStyle(DashStyle style)
{
	//pen_->SetDashStyle((Gdiplus::DashStyle)style);
}

IPen::DashStyle Pen_Skia::GetDashStyle() const
{
	return IPen::DashStyle::DashStyleDash;
	//return (IPen::DashStyle)pen_->GetDashStyle();
}
//
//Gdiplus::Pen* Pen_Skia::GetPen() const
//{ 
//	return pen_.get(); 
//};

} // namespace ui
