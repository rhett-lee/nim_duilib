#ifndef UI_RENDER_SKIA_PEN_H_
#define UI_RENDER_SKIA_PEN_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API Pen_Skia : public IPen
{
public:
	explicit Pen_Skia(UiColor color, int width = 1);
	Pen_Skia(const Pen_Skia& r);
	Pen_Skia& operator=(const Pen_Skia& r) = delete;

	virtual IPen* Clone() const override;

	virtual void SetWidth(int width) override;
	virtual int GetWidth() const override;
	virtual void SetColor(UiColor color) override;
	virtual UiColor GetColor() const override;

	virtual void SetStartCap(LineCap cap) override;
	virtual void SetEndCap(LineCap cap) override;
	virtual void SetDashCap(LineCap cap) override;
	virtual LineCap GetStartCap() const override;
	virtual LineCap GetEndCap() const override;
	virtual LineCap GetDashCap() const override;

	virtual void SetLineJoin(LineJoin join) override;
	virtual LineJoin GetLineJoin() const override;

	virtual void SetDashStyle(DashStyle style) override;
	virtual DashStyle GetDashStyle() const override;

	//Gdiplus::Pen* GetPen() const;

private:
	UiColor m_color;
	//std::unique_ptr<Gdiplus::Pen> pen_;
};

} // namespace ui

#endif // UI_RENDER_SKIA_PEN_H_
