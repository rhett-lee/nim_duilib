#include "Matrix_Gdiplus.h"
#include "duilib/RenderGdiPlus/GdiPlusDefs.h"

namespace ui {

Matrix_Gdiplus::Matrix_Gdiplus()
{
	m_matrix = std::make_unique<Gdiplus::Matrix>();
}

void Matrix_Gdiplus::Translate(int offsetX, int offsetY)
{
	m_matrix->Translate((Gdiplus::REAL)offsetX, (Gdiplus::REAL)offsetY);
}

void Matrix_Gdiplus::Scale(float scaleX, float scaleY)
{
	m_matrix->Scale((Gdiplus::REAL)scaleX, (Gdiplus::REAL)scaleY);
}

void Matrix_Gdiplus::Rotate(float angle)
{
	m_matrix->Rotate((Gdiplus::REAL)angle);
}

void Matrix_Gdiplus::RotateAt(float angle, const UiPoint& center)
{
	m_matrix->RotateAt(angle, Gdiplus::PointF((Gdiplus::REAL)center.x, (Gdiplus::REAL)center.y));
}

} // namespace ui
