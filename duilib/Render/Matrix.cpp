#include "Matrix.h"
#include "duilib/Image/GdiPlusDefs.h"

namespace ui {

Matrix_Gdiplus::Matrix_Gdiplus()
{
	matrix_ = std::make_unique<Gdiplus::Matrix>();
}

void Matrix_Gdiplus::Translate(int offsetX, int offsetY)
{
	matrix_->Translate(static_cast<Gdiplus::REAL>(offsetX), static_cast<Gdiplus::REAL>(offsetY));
}

void Matrix_Gdiplus::Scale(int scaleX, int scaleY)
{
	matrix_->Scale(static_cast<Gdiplus::REAL>(scaleX), static_cast<Gdiplus::REAL>(scaleY));
}

void Matrix_Gdiplus::Rotate(float angle)
{
	matrix_->Rotate(angle);
}

void Matrix_Gdiplus::RotateAt(float angle, const UiPoint& center)
{
	matrix_->RotateAt(angle, Gdiplus::PointF(static_cast<Gdiplus::REAL>(center.x), static_cast<Gdiplus::REAL>(center.y)));
}

Gdiplus::Matrix* Matrix_Gdiplus::GetMatrix() const
{ 
	return matrix_.get(); 
}

} // namespace ui
