#include "Matrix_Skia.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkMatrix.h"
#include "SkiaHeaderEnd.h"

namespace ui {

Matrix_Skia::Matrix_Skia()
{
    m_matrix = std::make_unique<SkMatrix>();
}

void Matrix_Skia::Translate(float offsetX, float offsetY)
{
    m_matrix->setTranslate((SkScalar)offsetX, (SkScalar)offsetY);
}

void Matrix_Skia::Scale(float scaleX, float scaleY)
{
    m_matrix->setScale((SkScalar)scaleX, (SkScalar)scaleY);
}

void Matrix_Skia::Scale(float scaleX, float scaleY, float px, float py)
{
    m_matrix->setScale((SkScalar)scaleX, (SkScalar)scaleY, (SkScalar)px, (SkScalar)py);
}

void Matrix_Skia::Rotate(float angle)
{
    m_matrix->setRotate((SkScalar)angle);
}

void Matrix_Skia::RotateAt(float angle, float px, float py)
{
    m_matrix->setRotate(angle, (SkScalar)px, (SkScalar)py);
}

void Matrix_Skia::Skew(float kx, float ky)
{
    m_matrix->setSkew((SkScalar)kx, (SkScalar)ky);
}

void Matrix_Skia::Skew(float kx, float ky, float px, float py)
{
    m_matrix->setSkew((SkScalar)kx, (SkScalar)ky, (SkScalar)px, (SkScalar)py);
}

} // namespace ui
