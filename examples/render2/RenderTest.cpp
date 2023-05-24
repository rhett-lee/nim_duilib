#include "RenderTest.h"

namespace ui {

RenderTest::RenderTest()
{

}

RenderTest::~RenderTest()
{

}

void RenderTest::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
    __super::AlphaPaint(pRender, rcPaint);
}

void RenderTest::Paint(IRender* pRender, const UiRect& rcPaint)
{
    const int marginSize = 8;
    UiRect rect = GetRect();
    rect.left += marginSize;
    rect.top += 4;

    int nSize = 50;
    rect.right = rect.left + nSize;
    rect.bottom = rect.top + nSize;
    int currentBottom = rect.bottom;//记录当前的bottom值


    //换行
    rect = GetRect();
    rect.left += marginSize;
    rect.top = currentBottom + marginSize;
}

void RenderTest::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
}

} //end of namespace ui