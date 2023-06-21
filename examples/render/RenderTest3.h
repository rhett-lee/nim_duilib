#pragma once

// duilib
#include "duilib/duilib.h"

namespace ui {

class RenderTest3 : public ui::Control
{
public:
	RenderTest3();
	virtual ~RenderTest3();

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

private:
	int DpiScaledInt(int iValue);
};

} //end of namespace ui
