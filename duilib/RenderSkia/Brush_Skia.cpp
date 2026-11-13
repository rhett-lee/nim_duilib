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

std::unique_ptr<IBrush> Brush_Skia::Clone() const
{
    return std::make_unique<Brush_Skia>(*this);
}

UiColor Brush_Skia::GetColor() const
{
    return m_color;
}

} // namespace ui
