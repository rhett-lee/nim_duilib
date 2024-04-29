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
	/** 对整型值进行DPI自适应
	*/
	int DpiScaledInt(int iValue);

	/** 在一个矩形内绘制正六边形拼接的颜色地图, 尽量充满整个矩形
	*/
	void DrawColorMap(IRender* pRender, const UiRect& rect);

	/** 绘制一个正六边形(采用多边形顶点的方式进行路径填充)
	* @param [in] pRender 渲染接口
	* @param [in] centerPt 正六边形的中心点坐标
	* @param [in] radius 正六边形的半径，中心点距离顶点的距离（同时也是边长）
	* @param [in] penColor 画笔的颜色，如果为0，则不绘制边框
	* @param [in] penWidth 画笔的宽度，如果为0，则不绘制边框
	* @param [in] brushColor 画刷的颜色，如果为0，则填充颜色
	*/
	bool DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius, 
						    const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

	/** 绘制一个正六边形(采用三角形旋转的方式绘制)
	* @param [in] pRender 渲染接口
	* @param [in] centerPt 正六边形的中心点坐标
	* @param [in] radius 正六边形的半径，中心点距离顶点的距离（同时也是边长）
	* @param [in] penColor 画笔的颜色，如果为0，则不绘制边框
	* @param [in] penWidth 画笔的宽度，如果为0，则不绘制边框
	* @param [in] brushColor 画刷的颜色，如果为0，则填充颜色
	*/
	bool DrawRegularHexagon3(IRender* pRender, const UiPoint& centerPt, int32_t radius, 
						     const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

};

} //end of namespace ui
