#include "RenderFactory_Skia.h"
#include "duilib/RenderSkia/Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Brush_Skia.h"
#include "duilib/RenderSkia/Pen_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"

namespace ui {

ui::IPen* RenderFactory_Skia::CreatePen(UiColor color, int width /*= 1*/)
{
	return new Pen_Skia(color, width);
}

ui::IBrush* RenderFactory_Skia::CreateBrush(UiColor color)
{
	return new Brush_Skia(color);
}

ui::IPath* RenderFactory_Skia::CreatePath()
{
	return new Path_Skia();
}

IMatrix* RenderFactory_Skia::CreateMatrix()
{
	return new Matrix_Skia();
}

ui::IBitmap* RenderFactory_Skia::CreateBitmap()
{
	return new Bitmap_Skia();
}

ui::IRender* RenderFactory_Skia::CreateRender()
{
	return new Render_Skia();
}

} // namespace ui
