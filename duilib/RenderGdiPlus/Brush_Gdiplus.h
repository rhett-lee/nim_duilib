#ifndef UI_RENDER_GDIPLUS_BRUSH_H_
#define UI_RENDER_GDIPLUS_BRUSH_H_

#pragma once


#include "duilib/Render/IRender.h"

namespace Gdiplus
{
	class Brush;
}

namespace ui 
{

class UILIB_API Brush_Gdiplus : public IBrush
{
public:
	explicit Brush_Gdiplus(UiColor color);
	Brush_Gdiplus(const Brush_Gdiplus& r);
	Brush_Gdiplus& operator=(const Brush_Gdiplus& r) = delete;

	virtual IBrush* Clone() override;
	virtual UiColor GetColor() const override { return color_; };

	Gdiplus::Brush* GetBrush() const;
private:
	UiColor color_;
	std::unique_ptr<Gdiplus::Brush> brush_;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_BRUSH_H_
