#include "RenderFactory_GdiPlus.h"
#include "duilib/RenderGdiPlus/Pen_GdiPlus.h"
#include "duilib/RenderGdiPlus/Brush_GdiPlus.h"
#include "duilib/RenderGdiPlus/Path_GdiPlus.h"
#include "duilib/RenderGdiPlus/Matrix_Gdiplus.h"
#include "duilib/RenderGdiPlus/Render_GdiPlus.h"
#include "duilib/RenderGdiPlus/Bitmap_GDI.h"

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

IMatrix* RenderFactory_GdiPlus::CreateMatrix()
{
	return new Matrix_Gdiplus();
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
