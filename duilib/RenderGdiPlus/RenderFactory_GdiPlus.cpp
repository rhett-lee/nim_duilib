#include "RenderFactory_GdiPlus.h"
#include "duilib/RenderGdiPlus/Font_GDI.h"
#include "duilib/RenderGdiPlus/Pen_GdiPlus.h"
#include "duilib/RenderGdiPlus/Brush_GdiPlus.h"
#include "duilib/RenderGdiPlus/Path_GdiPlus.h"
#include "duilib/RenderGdiPlus/Matrix_Gdiplus.h"
#include "duilib/RenderGdiPlus/Render_GdiPlus.h"
#include "duilib/RenderGdiPlus/Bitmap_GDI.h"

namespace ui {

IFont* RenderFactory_GdiPlus::CreateIFont()
{
	return new Font_GDI();
}

IPen* RenderFactory_GdiPlus::CreatePen(UiColor color, int width /*= 1*/)
{
	return new Pen_GdiPlus(color, width);
}

IBrush* RenderFactory_GdiPlus::CreateBrush(UiColor color)
{
	return new Brush_Gdiplus(color);
}

IPath* RenderFactory_GdiPlus::CreatePath()
{
	return new Path_Gdiplus();
}

IMatrix* RenderFactory_GdiPlus::CreateMatrix()
{
	return new Matrix_Gdiplus();
}

IBitmap* RenderFactory_GdiPlus::CreateBitmap()
{
	return new Bitmap_GDI();
}

IRender* RenderFactory_GdiPlus::CreateRender()
{
	return new Render_GdiPlus();
}

} // namespace ui
