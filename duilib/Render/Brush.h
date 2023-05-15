#ifndef UI_CORE_RENDER_BRUSH_H_
#define UI_CORE_RENDER_BRUSH_H_

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
	explicit Brush_Gdiplus(HBITMAP bitmap);
	Brush_Gdiplus(const Brush_Gdiplus& r);
	Brush_Gdiplus& operator=(const Brush_Gdiplus& r) = delete;

	virtual IBrush* Clone() override;

	Gdiplus::Brush* GetBrush() const;
private:
	std::unique_ptr<Gdiplus::Brush> brush_;
	std::unique_ptr<Gdiplus::Brush> bitmap_brush_;
};

} // namespace ui

#endif // UI_CORE_RENDER_BRUSH_H_
