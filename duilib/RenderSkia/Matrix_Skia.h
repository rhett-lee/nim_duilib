#ifndef UI_RENDER_SKIA_MATRIX_H_
#define UI_RENDER_SKIA_MATRIX_H_

#pragma once

#include "duilib/Render/IRender.h"

class SkMatrix;

namespace ui 
{

class UILIB_API Matrix_Skia : public IMatrix
{
public:
	Matrix_Skia();

	virtual void Translate(int offsetX, int offsetY) override;
	virtual void Scale(float scaleX, float scaleY) override;
	virtual void Rotate(float angle) override;
	virtual void RotateAt(float angle, const UiPoint& center) override;

	SkMatrix* GetMatrix() { return m_matrix.get(); }

private:
	std::unique_ptr<SkMatrix> m_matrix;
};

} // namespace ui

#endif // UI_RENDER_SKIA_MATRIX_H_
