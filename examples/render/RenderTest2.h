#pragma once

// duilib
#include "duilib/duilib.h"

namespace ui {

class RenderTest2 : public ui::Control
{
public:
	RenderTest2();
	virtual ~RenderTest2();

public:
	/**
	* @brief 待补充
	* @param[in] 待补充
	* @return 待补充
	*/
	virtual void AlphaPaint(IRender* pRender, const UiRect& rcPaint);

	/**
	* @brief 绘制控件的入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void Paint(IRender* pRender, const UiRect& rcPaint);

	/**
	* @brief 绘制控件子项入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void PaintChild(IRender* pRender, const UiRect& rcPaint);

};

} //end of namespace ui
