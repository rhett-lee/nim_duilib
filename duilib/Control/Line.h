#ifndef UI_CONTROL_LINE_H_
#define UI_CONTROL_LINE_H_

#pragma once

#include "duilib/Core/Control.h"

namespace ui
{
/** 画线控件
*/
class Line: public Control
{
public:
	Line();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

protected:
	/** 绘制控件的入口函数
	* @param [in] pRender 指定绘制区域
	* @param [in] rcPaint 指定绘制坐标
	*/
	virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
	/** 水平还是垂直(true表示垂直)
	*/
	bool m_bLineVertical;

	/** 线条颜色
	*/
	UiColor m_lineColor;

	/** 线条宽度
	*/
	int32_t m_lineWidth;

	/** 线条类型
	*/
	int32_t m_dashStyle;
};

}//namespace ui

#endif //UI_CONTROL_LINE_H_
