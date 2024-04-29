#ifndef UI_RENDER_SKIA_PEN_H_
#define UI_RENDER_SKIA_PEN_H_

#pragma once

#include "duilib/Render/IRender.h"

namespace ui 
{

class UILIB_API Pen_Skia : public IPen
{
public:
	explicit Pen_Skia(UiColor color, int width = 1);
	Pen_Skia(const Pen_Skia& r);
	Pen_Skia& operator=(const Pen_Skia& r) = delete;

	/** 设置画笔宽度
	*/
	virtual void SetWidth(int width) override;

	/** 获取画笔宽度
	*/
	virtual int GetWidth() const override;

	/** 设置画笔颜色
	*/
	virtual void SetColor(UiColor color) override;

	/** 获取画笔颜色
	*/
	virtual UiColor GetColor() const override;

	/** 设置线段起始的笔帽样式
	*/
	virtual void SetStartCap(LineCap cap) override;

	/** 获取线段起始的笔帽样式
	*/
	virtual LineCap GetStartCap() const override;

	/** 设置线段结束的笔帽样式
	*/
	virtual void SetEndCap(LineCap cap) override;

	/** 获取线段结束的笔帽样式
	*/
	virtual LineCap GetEndCap() const override;

	/** 设置短划线笔帽的样式
	*/
	virtual void SetDashCap(LineCap cap) override;

	/** 获取短划线笔帽的样式
	*/
	virtual LineCap GetDashCap() const override;

	/** 设置线段末尾使用的联接样式
	*/
	virtual void SetLineJoin(LineJoin join) override;

	/** 获取线段末尾使用的联接样式
	*/
	virtual LineJoin GetLineJoin() const override;

	/** 设置绘制的线条样式
	*/
	virtual void SetDashStyle(DashStyle style) override;

	/** 获取绘制的线条样式
	*/
	virtual DashStyle GetDashStyle() const override;

	/** 复制Pen对象
	*/
	virtual IPen* Clone() const override;

private:
	/** 画笔颜色
	*/
	UiColor m_color;

	/** 画笔宽度
	*/
	int m_width;

	/** 线段起始笔帽形状
	*/
	LineCap m_startCap;

	/** 短划线笔帽形状
	*/
	LineCap m_dashCap;

	/** 线段结束笔帽形状
	*/
	LineCap m_endCap;

	/** 线段末尾使用的联接样式，该线段与另一个线段相遇
	*/
	LineJoin m_lineJoin;

	/** 短划线样式
	*/
	DashStyle m_dashStyle;
};

} // namespace ui

#endif // UI_RENDER_SKIA_PEN_H_
