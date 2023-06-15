#ifndef UI_RENDER_GDIPLUS_CLIP_H_
#define UI_RENDER_GDIPLUS_CLIP_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API Clip_GDI : public IClip
{
public:
	Clip_GDI();
    ~Clip_GDI();

	virtual void CreateClip(HDC hDC, UiRect rc) override;
	virtual void CreateRoundClip(HDC hDC, UiRect rc, int width, int height) override;
	virtual void ClearClip(HDC hDC) override;
};


} // namespace ui

#endif // UI_RENDER_GDIPLUS_CLIP_H_
