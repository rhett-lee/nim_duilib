#include "RenderTest3.h"
#include "duilib/RenderGdiPlus/BitmapHelper.h"

namespace ui {

RenderTest3::RenderTest3()
{

}

RenderTest3::~RenderTest3()
{

}

void RenderTest3::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    __super::AlphaPaint(pRender, rcPaint);
}

void RenderTest3::Paint(IRender* pRender, const UiRect& rcPaint)
{
    __super::Paint(pRender, rcPaint);
}

void RenderTest3::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

} //end of namespace ui