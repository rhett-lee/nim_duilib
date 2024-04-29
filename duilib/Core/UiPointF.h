#ifndef UI_CORE_UIPOINTF_H_
#define UI_CORE_UIPOINTF_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>

namespace ui 
{

/** 点的封装
*/
class UILIB_API UiPointF
{
public:
	UiPointF()
	{
		x = y = 0;
	}

	UiPointF(float xValue, float yValue)
	{
		x = xValue;
		y = yValue;
	}

	/** 设置新的X和Y值
	*/
	void SetXY(float xValue, float yValue)
	{
		x = xValue;
		y = yValue;
	}

	/** 设置新的X值
	*/
	void SetX(float xValue) { x = xValue; }

	/** 设置新的Y值
	*/
	void SetY(float yValue) { y = yValue;	}

	/** 偏移操作
	*/
	void Offset(float offsetX, float offsetY)
	{
		x += offsetX;
		y += offsetY;
	}

	/** 偏移操作
	*/
	void Offset(const UiPointF& offsetPoint)
	{
		x += offsetPoint.x;
		y += offsetPoint.y;
	}

	/** 返回x值
	*/
	constexpr float X() const { return x; }

	/** 返回y值
	*/
	constexpr float Y() const { return y; }

	/** 如果x和y都是0，返回true
	*/
	bool IsZero() const 
	{ 
		return x == 0 && y == 0; 
	}

	/** 判断是否与另外一个点相同
	*/
	bool Equals(float xValue, float yValue) const
	{
		return x == xValue && y == yValue;
	}

	/** 判断是否与另外一个点相同
	*/
	bool Equals(const UiPointF& dst) const
	{
		return x == dst.x && y == dst.y;
	}

	/** 判断两个值是否相等
	*/
	friend bool operator == (const UiPointF& a, const UiPointF& b)
	{
		return a.Equals(b);
	}

	/** 判断两个值是否不相等
	*/
	friend bool operator != (const UiPointF& a, const UiPointF& b)
	{
		return !a.Equals(b);
	}

public:
	/** X轴坐标
	*/
	float x;

	/** Y轴坐标
	*/
	float y;
};

} // namespace ui

#endif // UI_CORE_UIPOINTF_H_
