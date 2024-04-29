#ifndef UI_RENDER_SKIA_BRUSH_H_
#define UI_RENDER_SKIA_BRUSH_H_

#pragma once

#include "duilib/Render/IRender.h"

class SkBitmap;

namespace ui 
{

class UILIB_API Brush_Skia : public IBrush
{
public:
	explicit Brush_Skia(UiColor color);
	Brush_Skia(const Brush_Skia& r);
	Brush_Skia& operator=(const Brush_Skia& r) = delete;

	virtual IBrush* Clone() override;
	virtual UiColor GetColor() const override;

private:
	UiColor m_color;
};

} // namespace ui

#endif // UI_RENDER_SKIA_BRUSH_H_
