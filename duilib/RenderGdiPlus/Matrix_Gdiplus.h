#ifndef UI_RENDER_GDIPLUS_MATRIX_H_
#define UI_RENDER_GDIPLUS_MATRIX_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace Gdiplus
{
	class Matrix;
}

namespace ui 
{

class UILIB_API Matrix_Gdiplus : public IMatrix
{
public:
	Matrix_Gdiplus();

	virtual void Translate(int offsetX, int offsetY) override;
	virtual void Scale(float scaleX, float scaleY) override;
	virtual void Rotate(float angle) override;
	virtual void RotateAt(float angle, const UiPoint& center) override;

	Gdiplus::Matrix* GetMatrix() { return m_matrix.get(); }

private:
	std::unique_ptr<Gdiplus::Matrix> m_matrix;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_MATRIX_H_
