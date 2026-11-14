#include "Matrix_Skia.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkMatrix.h"
#include "SkiaHeaderEnd.h"

namespace ui {

Matrix_Skia::Matrix_Skia()
{
    m_matrix = std::make_unique<SkMatrix>();
}

Matrix_Skia::~Matrix_Skia()
{
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

void Matrix_Skia::Reset()
{
    m_matrix->setIdentity();
}

void Matrix_Skia::Concat(const IMatrix* pMatrix)
{
    ASSERT(pMatrix != nullptr);
    if (pMatrix == nullptr) {
        return;
    }
    const Matrix_Skia* pSkiaMatrix = dynamic_cast<const Matrix_Skia*>(pMatrix);
    ASSERT(pSkiaMatrix != nullptr);
    if (pSkiaMatrix == nullptr) {
        return;
    }
    //先执行当前变换，再执行 pMatrix 的变换
    m_matrix->postConcat(*pSkiaMatrix->GetMatrix());
}

bool Matrix_Skia::Invert()
{
    SkMatrix inverted;
    if (!m_matrix->invert(&inverted)) {
        return false;
    }
    *m_matrix = inverted;
    return true;
}

bool Matrix_Skia::IsIdentity() const
{
    return m_matrix->isIdentity();
}

UiPointF Matrix_Skia::MapPoint(const UiPointF& pt) const
{
    SkPoint skPoint = SkPoint::Make((SkScalar)pt.x, (SkScalar)pt.y);
    skPoint = m_matrix->mapPoint(skPoint);
    return UiPointF(skPoint.fX, skPoint.fY);
}

} // namespace ui
