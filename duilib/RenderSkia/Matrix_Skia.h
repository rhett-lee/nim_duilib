#ifndef UI_RENDER_SKIA_MATRIX_H_
#define UI_RENDER_SKIA_MATRIX_H_

#include "duilib/Render/IRender.h"

class SkMatrix;

namespace ui 
{

class UILIB_API Matrix_Skia : public IMatrix
{
public:
    Matrix_Skia();

    virtual void Translate(float offsetX, float offsetY) override;

    virtual void Scale(float scaleX, float scaleY) override;
    virtual void Scale(float scaleX, float scaleY, float px, float py) override;

    virtual void Rotate(float angle) override;
    virtual void RotateAt(float angle, float px, float py) override;

    virtual void Skew(float kx, float ky) override;
    virtual void Skew(float kx, float ky, float px, float py) override;

    SkMatrix* GetMatrix() { return m_matrix.get(); }

private:
    std::unique_ptr<SkMatrix> m_matrix;
};

} // namespace ui

#endif // UI_RENDER_SKIA_MATRIX_H_
