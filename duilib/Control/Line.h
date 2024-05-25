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
	explicit Line(Window* pWindow);

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/** DPI发生变化，更新控件大小和布局
	* @param [in] nOldDpiScale 旧的DPI缩放百分比
	* @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
	*/
	virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

	/** 设置水平还是垂直(true表示垂直)
	*/
	void SetLineVertical(bool bVertical);

	/** 获取水平还是垂直
	*/
	bool IsLineVertical() const;

	/** 设置线条的宽度
	* @param [in] lineWidth 线条宽度
	* @param [in] bNeedDpiScale 是否支持DPI缩放
	*/
	void SetLineWidth(int32_t lineWidth, bool bNeedDpiScale);

	/** 获取线条的宽度
	*/
	int32_t GetLineWidth() const;

	/** 设置线条颜色
	*/
	void SetLineColor(const std::wstring& lineColor);

	/** 获取线条颜色
	*/
	std::wstring GetLineColor() const;

	/** 设置线条类型
	*/
	void SetLineDashStyle(const std::wstring& dashStyle);

	/** 获取线条类型
	*/
	std::wstring GetLineDashStyle() const;

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
	UiString m_lineColor;

	/** 线条宽度
	*/
	int32_t m_lineWidth;

	/** 线条类型
	*/
	int32_t m_dashStyle;
};

}//namespace ui

#endif //UI_CONTROL_LINE_H_
