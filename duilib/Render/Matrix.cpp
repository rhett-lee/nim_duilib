#include "StdAfx.h"

namespace ui {

using namespace Gdiplus;

void Matrix_Gdiplus::Translate(int offsetX, int offsetY)
{
	matrix_.Translate(static_cast<Gdiplus::REAL>(offsetX), static_cast<Gdiplus::REAL>(offsetY));
}

void Matrix_Gdiplus::Scale(int scaleX, int scaleY)
{
	matrix_.Scale(static_cast<Gdiplus::REAL>(scaleX), static_cast<Gdiplus::REAL>(scaleY));
}

void Matrix_Gdiplus::Rotate(float angle)
{
	matrix_.Rotate(angle);
}

void Matrix_Gdiplus::RotateAt(float angle, const CPoint& center)
{
	matrix_.RotateAt(angle, PointF(static_cast<Gdiplus::REAL>(center.x), static_cast<Gdiplus::REAL>(center.y)));
}

} // namespace ui
