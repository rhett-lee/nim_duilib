#include "Brush_Skia.h"

namespace ui {

Brush_Skia::Brush_Skia(UiColor color):
	m_color(color)
{
}

Brush_Skia::Brush_Skia(const Brush_Skia& r):
	m_color(r.m_color)
{
}

ui::IBrush* Brush_Skia::Clone()
{
	return new Brush_Skia(*this);
}

UiColor Brush_Skia::GetColor() const
{
	return m_color;
}

} // namespace ui
