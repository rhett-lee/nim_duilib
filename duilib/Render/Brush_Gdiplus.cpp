#include "Brush_Gdiplus.h"
#include "duilib/Image/GdiPlusDefs.h"

namespace ui {

Brush_Gdiplus::Brush_Gdiplus(UiColor color)
	: IBrush(color)
{
	brush_.reset(new Gdiplus::SolidBrush(color.GetARGB()));
}

Brush_Gdiplus::Brush_Gdiplus(const Brush_Gdiplus& r)
	: IBrush(r)
{
	if (r.brush_)
		brush_.reset(r.brush_->Clone());
}

ui::IBrush* Brush_Gdiplus::Clone()
{
	return new Brush_Gdiplus(*this);
}

Gdiplus::Brush* Brush_Gdiplus::GetBrush() const
{ 
	return brush_.get();
}

} // namespace ui
