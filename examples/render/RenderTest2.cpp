#include "RenderTest2.h"

namespace ui {

RenderTest2::RenderTest2()
{

}

RenderTest2::~RenderTest2()
{

}

void RenderTest2::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    __super::AlphaPaint(pRender, rcPaint);
}

void RenderTest2::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
}

void RenderTest2::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

} //end of namespace ui