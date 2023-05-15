#include "Brush.h"
#include "duilib/Core/GdiPlusDefs.h"

namespace ui {

Brush_Gdiplus::Brush_Gdiplus(UiColor color)
	: IBrush(color)
{
	brush_.reset(new Gdiplus::SolidBrush(color.GetARGB()));
}

Brush_Gdiplus::Brush_Gdiplus(HBITMAP bitmap)
	: IBrush(bitmap)
{
	Gdiplus::Bitmap image(bitmap, NULL);
	bitmap_brush_.reset(new Gdiplus::TextureBrush(&image));
}

Brush_Gdiplus::Brush_Gdiplus(const Brush_Gdiplus& r)
	: IBrush(r)
{
	if (r.brush_)
		brush_.reset(r.brush_->Clone());

	if (r.bitmap_brush_)
		bitmap_brush_.reset(r.bitmap_brush_->Clone());
}

ui::IBrush* Brush_Gdiplus::Clone()
{
	return new Brush_Gdiplus(*this);
}

Gdiplus::Brush* Brush_Gdiplus::GetBrush() const
{ 
	return brush_ ? brush_.get() : bitmap_brush_.get(); 
}

} // namespace ui
