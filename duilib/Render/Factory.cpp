#include "Factory.h"
#include "duilib/Render/Bitmap.h"
#include "duilib/Render/Pen.h"
#include "duilib/Render/Brush.h"
#include "duilib/Render/Path.h"
#include "duilib/Render/Render.h"

namespace ui {

ui::IPen* RenderFactory_GdiPlus::CreatePen(UiColor color, int width /*= 1*/)
{
	return new Pen_GdiPlus(color, width);
}

ui::IBrush* RenderFactory_GdiPlus::CreateBrush(UiColor color)
{
	return new Brush_Gdiplus(color);
}

ui::IBrush* RenderFactory_GdiPlus::CreateBrush(HBITMAP bitmap)
{
	return new Brush_Gdiplus(bitmap);
}

ui::IPath* RenderFactory_GdiPlus::CreatePath()
{
	return new Path_Gdiplus();
}

ui::IBitmap* RenderFactory_GdiPlus::CreateBitmap()
{
	return new GdiBitmap();
}

ui::IRenderContext* RenderFactory_GdiPlus::CreateRenderContext()
{
	return new RenderContext_GdiPlus();
}

} // namespace ui
