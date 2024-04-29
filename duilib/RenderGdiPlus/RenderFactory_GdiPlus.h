#ifndef UI_RENDER_GDIPLUS_RENDER_FACTORY_H_
#define UI_RENDER_GDIPLUS_RENDER_FACTORY_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API RenderFactory_GdiPlus : public IRenderFactory
{
public:
	virtual IFont* CreateIFont() override;
	virtual IPen* CreatePen(UiColor color, int width = 1) override;
	virtual IBrush* CreateBrush(UiColor corlor) override;
	virtual IPath* CreatePath() override;
	virtual IMatrix* CreateMatrix() override;
	virtual IBitmap* CreateBitmap() override;
	virtual IRender* CreateRender() override;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_RENDER_FACTORY_H_
