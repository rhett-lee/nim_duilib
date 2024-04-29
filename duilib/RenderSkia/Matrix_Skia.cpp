#include "Matrix_Skia.h"

#pragma warning (push)
#pragma warning (disable: 4244)
#include "include/core/SkMatrix.h"
#pragma warning (pop)

namespace ui {

Matrix_Skia::Matrix_Skia()
{
	m_matrix = std::make_unique<SkMatrix>();
}

void Matrix_Skia::Translate(int offsetX, int offsetY)
{
	m_matrix->setTranslate((SkScalar)offsetX, (SkScalar)offsetY);
}

void Matrix_Skia::Scale(float scaleX, float scaleY)
{
	m_matrix->setScale((SkScalar)scaleX, (SkScalar)scaleY);
}

void Matrix_Skia::Rotate(float angle)
{
	m_matrix->setRotate((SkScalar)angle);
}

void Matrix_Skia::RotateAt(float angle, const UiPoint& center)
{
	m_matrix->setRotate(angle, (SkScalar)center.x, (SkScalar)center.y);
}

} // namespace ui
