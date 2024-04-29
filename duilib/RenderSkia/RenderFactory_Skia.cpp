#include "RenderFactory_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/Render_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Brush_Skia.h"
#include "duilib/RenderSkia/Pen_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"

#pragma warning (push)
#pragma warning (disable: 4244)

#include "include/core/SkFontMgr.h"
#include "include/ports/SkTypeface_win.h"

#pragma warning (pop)

namespace ui {

RenderFactory_Skia::RenderFactory_Skia():
	m_pSkFontMgr(nullptr)
{
}

RenderFactory_Skia::~RenderFactory_Skia()
{
	if (m_pSkFontMgr != nullptr) {
		m_pSkFontMgr->unref();
	}
}

IFont* RenderFactory_Skia::CreateIFont()
{
	return new Font_Skia(this);
}

IPen* RenderFactory_Skia::CreatePen(UiColor color, int width /*= 1*/)
{
	return new Pen_Skia(color, width);
}

IBrush* RenderFactory_Skia::CreateBrush(UiColor color)
{
	return new Brush_Skia(color);
}

IPath* RenderFactory_Skia::CreatePath()
{
	return new Path_Skia();
}

IMatrix* RenderFactory_Skia::CreateMatrix()
{
	return new Matrix_Skia();
}

IBitmap* RenderFactory_Skia::CreateBitmap()
{
	return new Bitmap_Skia();
}

IRender* RenderFactory_Skia::CreateRender()
{
	return new Render_Skia(this);
}

SkFontMgr* RenderFactory_Skia::GetSkFontMgr()
{
	if (m_pSkFontMgr == nullptr) {
		sk_sp<SkFontMgr> spSkFontMgr = SkFontMgr_New_DirectWrite();
		ASSERT(spSkFontMgr != nullptr);
		m_pSkFontMgr = spSkFontMgr.get();
		m_pSkFontMgr->ref();
		spSkFontMgr.reset(nullptr);
	}
	return m_pSkFontMgr;
}

} // namespace ui
