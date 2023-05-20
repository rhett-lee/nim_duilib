#include "Factory.h"
#include "duilib/Render/Pen.h"
#include "duilib/Render/Brush.h"
#include "duilib/Render/Path.h"
#include "duilib/Render/Render_GdiPlus.h"
#include "duilib/Render/Bitmap_GDI.h"

namespace ui {

ui::IPen* RenderFactory_GdiPlus::CreatePen(UiColor color, int width /*= 1*/)
{
	return new Pen_GdiPlus(color, width);
}

ui::IBrush* RenderFactory_GdiPlus::CreateBrush(UiColor color)
{
	return new Brush_Gdiplus(color);
}

ui::IPath* RenderFactory_GdiPlus::CreatePath()
{
	return new Path_Gdiplus();
}

ui::IBitmap* RenderFactory_GdiPlus::CreateBitmap()
{
	return new Bitmap_GDI();
}

ui::IRender* RenderFactory_GdiPlus::CreateRender()
{
	return new Render_GdiPlus();
}

} // namespace ui
