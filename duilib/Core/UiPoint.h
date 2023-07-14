#ifndef UI_CORE_UIPOINT_H_
#define UI_CORE_UIPOINT_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>

namespace ui 
{

/** 点的封装
*/
class UILIB_API UiPoint
{
public:
	UiPoint()
	{
		x = y = 0;
	}

	UiPoint(int32_t xValue, int32_t yValue)
	{
		x = xValue;
		y = yValue;
	}

	/** 设置新的X和Y值
	*/
	void SetXY(int32_t xValue, int32_t yValue)
	{
		x = xValue;
		y = yValue;
	}

	/** 设置新的X值
	*/
	void SetX(int32_t xValue) { x = xValue; }

	/** 设置新的Y值
	*/
	void SetY(int32_t yValue) { y = yValue;	}

	/** 偏移操作
	*/
	void Offset(int32_t offsetX, int32_t offsetY)
	{
		x += offsetX;
		y += offsetY;
	}

	/** 偏移操作
	*/
	void Offset(const UiPoint& offsetPoint)
	{
		x += offsetPoint.x;
		y += offsetPoint.y;
	}

	/** 返回x值
	*/
	constexpr int32_t X() const { return x; }

	/** 返回y值
	*/
	constexpr int32_t Y() const { return y; }

	/** 如果x和y都是0，返回true
	*/
	bool IsZero() const 
	{ 
		return x == 0 && y == 0; 
	}

	/** 判断是否与另外一个点相同
	*/
	bool Equals(int32_t xValue, int32_t yValue) const 
	{
		return x == xValue && y == yValue;
	}

	/** 判断是否与另外一个点相同
	*/
	bool Equals(const UiPoint& dst) const
	{
		return x == dst.x && y == dst.y;
	}

	/** 判断两个值是否相等
	*/
	friend bool operator == (const UiPoint& a, const UiPoint& b)
	{
		return a.Equals(b);
	}

	/** 判断两个值是否不相等
	*/
	friend bool operator != (const UiPoint& a, const UiPoint& b)
	{
		return !a.Equals(b);
	}

public:
	/** X轴坐标
	*/
	int32_t x;

	/** Y轴坐标
	*/
	int32_t y;
};

} // namespace ui

#endif // UI_CORE_UIPOINT_H_
