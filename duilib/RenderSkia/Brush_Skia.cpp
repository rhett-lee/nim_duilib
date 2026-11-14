#include "Brush_Skia.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkMatrix.h"
#include "include/effects/SkGradient.h"
#include "SkiaHeaderEnd.h"

namespace ui {

Brush_Skia::Brush_Skia(UiColor color):
    m_color(color),
    m_brushType(BrushType::kSolid),
    m_endColor(color),
    m_fRadius(0.0f)
{
}

Brush_Skia::Brush_Skia(const Brush_Skia& r):
    m_color(r.m_color),
    m_brushType(r.m_brushType),
    m_endColor(r.m_endColor),
    m_startPt(r.m_startPt),
    m_endPt(r.m_endPt),
    m_fRadius(r.m_fRadius)
{
}

std::unique_ptr<IBrush> Brush_Skia::Clone() const
{
    return std::make_unique<Brush_Skia>(*this);
}

IBrush::BrushType Brush_Skia::GetBrushType() const
{
    return m_brushType;
}

UiColor Brush_Skia::GetColor() const
{
    return m_color;
}

void Brush_Skia::SetSolid(UiColor color)
{
    m_color = color;
    m_endColor = color;
    m_brushType = BrushType::kSolid;
}

void Brush_Skia::SetLinearGradient(UiColor endColor, const UiPointF& startPt, const UiPointF& endPt)
{
    m_endColor = endColor;
    m_startPt = startPt;
    m_endPt = endPt;
    m_brushType = BrushType::kLinearGradient;
}

void Brush_Skia::SetRadialGradient(UiColor endColor, const UiPointF& center, float radius)
{
    m_endColor = endColor;
    m_startPt = center;
    m_fRadius = radius;
    m_brushType = BrushType::kRadialGradient;
}

void Brush_Skia::ApplyShader(SkPaint& skPaint, float offsetX, float offsetY) const
{
    if (m_brushType == BrushType::kSolid) {
        return;
    }
    SkColor4f colors[2];
    colors[0] = SkColor4f::FromColor(SkColorSetARGB(m_color.GetA(), m_color.GetR(), m_color.GetG(), m_color.GetB()));
    colors[1] = SkColor4f::FromColor(SkColorSetARGB(m_endColor.GetA(), m_endColor.GetR(), m_endColor.GetG(), m_endColor.GetB()));
    SkGradient::Colors skColors(SkSpan<const SkColor4f>(colors), SkTileMode::kClamp);
    SkGradient skGradient(skColors, SkGradient::Interpolation());

    //渐变坐标使用设备坐标，需要加上画布原点偏移
    const SkPoint startPt = SkPoint::Make((SkScalar)(m_startPt.x + offsetX), (SkScalar)(m_startPt.y + offsetY));
    if (m_brushType == BrushType::kLinearGradient) {
        SkPoint pts[2] = { startPt, SkPoint::Make((SkScalar)(m_endPt.x + offsetX), (SkScalar)(m_endPt.y + offsetY)) };
        skPaint.setShader(SkShaders::LinearGradient(pts, skGradient));
    }
    else if ((m_brushType == BrushType::kRadialGradient) && (m_fRadius > 0.0f)) {
        skPaint.setShader(SkShaders::RadialGradient(startPt, (SkScalar)m_fRadius, skGradient));
    }
}

} // namespace ui
