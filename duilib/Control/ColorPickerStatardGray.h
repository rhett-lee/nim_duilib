#ifndef UI_CONTROL_COLORPICKER_STANDARD_GRAY_H_
#define UI_CONTROL_COLORPICKER_STANDARD_GRAY_H_

#pragma once

#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 颜色选择器：标准颜色，灰色
*/
class ColorPickerStatardGray : public Control
{
public:
	ColorPickerStatardGray();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

	/** 选择一个颜色
	*/
	void SelectColor(const UiColor& color);

	/**
	* @brief 绘制控件的入口函数
	* @param[in] pRender 指定绘制区域
	* @param[in] rcPaint 指定绘制坐标
	* @return 无
	*/
	virtual void Paint(IRender* pRender, const UiRect& rcPaint);

	/** 计算控件大小(宽和高)
		如果设置了图片并设置 width 或 height 任意一项为 auto，将根据图片大小和文本大小来计算最终大小
	 *  @param [in] szAvailable 可用大小，不包含外边距
	 *  @return 控件的估算大小，包含内边距(Box)，不包含外边距
	 */
	virtual UiEstSize EstimateSize(UiSize szAvailable) override;

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

	/** 鼠标移动
	*/
	virtual bool MouseMove(const EventArgs& msg) override;

	/** 鼠标左键按下
	*/
	virtual bool ButtonDown(const EventArgs& msg) override;

	/** 获取当前鼠标所在点的颜色信息
	*/
	bool GetColorInfo(const UiPoint& ptMouse, UiColor& ptColor) const;

	/** 计算两点之间的距离
	*/
	float GetPointsDistance(const UiPointF& pt1, const UiPointF& pt2) const;

	/** 初始化颜色表
	*/
	void InitColorMap();

	/** 监听选择颜色的事件
	* @param[in] callback 选择颜色变化时的回调函数
	*            参数说明:
						wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
						lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
	*/
	void AttachSelectColor(const EventCallback& callback) { AttachEvent(kEventSelectColor, callback); }

private:
	/** 颜色表
	*/
	struct ColorInfo
	{
		//颜色值
		UiColor color;

		//该颜色的正六边形中心点坐标
		UiPointF centerPt;

		/** 正六边形的半径
		*/
		int32_t m_radius;
	};
	std::vector<ColorInfo> m_colorMap;

	/** 当前选择了哪种颜色
	*/
	UiColor m_selectedColor;
};

}//namespace ui

#endif //UI_CONTROL_COLORPICKER_STANDARD_GRAY_H_

