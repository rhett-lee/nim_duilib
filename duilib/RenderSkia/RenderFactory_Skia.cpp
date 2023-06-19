#include "RenderFactory_Skia.h"
#include "duilib/RenderSkia/Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"

namespace ui {

ui::IPen* RenderFactory_Skia::CreatePen(UiColor color, int width /*= 1*/)
{
	//ASSERT(!"RenderFactory_Skia::CreatePen is not impl!");
	return nullptr;
}

ui::IBrush* RenderFactory_Skia::CreateBrush(UiColor color)
{
	//ASSERT(!"RenderFactory_Skia::CreateBrush is not impl!");
	return nullptr;
}

ui::IPath* RenderFactory_Skia::CreatePath()
{
	//ASSERT(!"RenderFactory_Skia::CreatePath is not impl!");
	return nullptr;
}

ui::IBitmap* RenderFactory_Skia::CreateBitmap()
{
	//ASSERT(!"RenderFactory_Skia::CreateBitmap is not impl!");
	return new Bitmap_Skia();
}

ui::IRender* RenderFactory_Skia::CreateRender()
{
	return new Render_Skia();
}

} // namespace ui
