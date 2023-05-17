#ifndef UI_CORE_RENDER_FACTORY_H_
#define UI_CORE_RENDER_FACTORY_H_
#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API RenderFactory_GdiPlus : public IRenderFactory
{
public:
	virtual ui::IPen* CreatePen(UiColor color, int width = 1) override;
	virtual ui::IBrush* CreateBrush(UiColor corlor) override;
	virtual ui::IBrush* CreateBrush(HBITMAP bitmap) override;
	virtual ui::IPath* CreatePath() override;
	virtual ui::IBitmap* CreateBitmap() override;
	virtual ui::IRenderContext* CreateRenderContext() override;
};

} // namespace ui

#endif // UI_CORE_RENDER_FACTORY_H_
