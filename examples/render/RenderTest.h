#pragma once

// duilib
#include "duilib/duilib.h"

namespace ui {

class RenderTest : public ui::Control
{
public:
	RenderTest();
	virtual ~RenderTest();

public:
	/**
	* @brief 待补充
	* @param[in] 待补充
	* @return 待补充
	*/
	virtual void AlphaPaint(IRenderContext* pRender, const UiRect& rcPaint);

	/**
	* @brief 绘制控件的入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void Paint(IRenderContext* pRender, const UiRect& rcPaint);

	/**
	* @brief 绘制控件子项入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void PaintChild(IRenderContext* pRender, const UiRect& rcPaint);

};

} //end of namespace ui
