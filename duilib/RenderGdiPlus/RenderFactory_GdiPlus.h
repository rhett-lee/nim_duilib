#ifndef UI_RENDER_GDIPLUS_RENDER_FACTORY_H_
#define UI_RENDER_GDIPLUS_RENDER_FACTORY_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API RenderFactory_GdiPlus : public IRenderFactory
{
public:
	virtual ui::IPen* CreatePen(UiColor color, int width = 1) override;
	virtual ui::IBrush* CreateBrush(UiColor corlor) override;
	virtual ui::IPath* CreatePath() override;
	virtual ui::IBitmap* CreateBitmap() override;
	virtual ui::IRender* CreateRender() override;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_RENDER_FACTORY_H_
