#ifndef UI_RENDER_GDIPLUS_PEN_H_
#define UI_RENDER_GDIPLUS_PEN_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace Gdiplus
{
	class Pen;
}

namespace ui 
{

class UILIB_API Pen_GdiPlus : public IPen
{
public:
	explicit Pen_GdiPlus(UiColor color, int width = 1);
	Pen_GdiPlus(const Pen_GdiPlus& r);
	Pen_GdiPlus& operator=(const Pen_GdiPlus& r) = delete;

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

	Gdiplus::Pen* GetPen() const;
private:
	UiColor m_color;
	std::unique_ptr<Gdiplus::Pen> m_pen;
};

} // namespace ui

#endif // UI_RENDER_GDIPLUS_PEN_H_
