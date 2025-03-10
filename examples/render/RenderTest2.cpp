#include "RenderTest2.h"

namespace ui {

RenderTest2::RenderTest2(ui::Window* pWindow):
    ui::Control(pWindow)
{
}

RenderTest2::~RenderTest2()
{
}

void RenderTest2::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::AlphaPaint(pRender, rcPaint);
}

void RenderTest2::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
}

void RenderTest2::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::PaintChild(pRender, rcPaint);
}

} //end of namespace ui
