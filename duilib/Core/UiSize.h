#ifndef UI_CORE_UISIZE_H_
#define UI_CORE_UISIZE_H_

#pragma once

#include "duilib/Core/UiSize64.h"

namespace ui 
{

/** Size的封装
*/
class UILIB_API UiSize
{
public:
	UiSize()
	{
		cx = cy = 0;
	}

	UiSize(const UiSize& src)
	{
		cx = src.cx;
		cy = src.cy;
	}

	UiSize(int32_t cxValue, int32_t cyValue)
	{
		cx = cxValue;
		cy = cyValue;
	}

	/** 设置新的宽度和高度
	*/
	void SetWidthHeight(int32_t cxValue, int32_t cyValue)
	{
		cx = cxValue;
		cy = cyValue;
	}

	/** 设置新的宽度
	*/
	void SetWidth(int32_t cxValue) { cx = cxValue; }

	/** 设置新的高度
	*/
	void SetHeight(int32_t cyValue) { cy = cyValue; }

	/** 返回宽度
	*/
	constexpr int32_t Width() const { return cx; }

	/** 返回高度
	*/
	constexpr int32_t Height() const { return cy; }

	/** 检查宽度和高度值是否为有效值，并修正
	*/
	void Validate()
	{
		if (cx < 0) {
			cx = 0;
		}
		if (cy < 0) {
			cy = 0;
		}
	}

	/** 高度和宽度是否为有效值
	*/
	bool IsValid() const { return cx >= 0 && cy >= 0; }

	/** 如果cx和cy都是0，返回true
	*/
	bool IsEmpty() const { return cx == 0 && cy == 0; }

	/** cx和cy都设置为0
	*/
	void Clear() 
	{ 
		cx = 0; 
		cy = 0; 
	}

	/** 判断是否与另外一个Size相同
	*/
	bool Equals(int32_t cxValue, int32_t cyValue) const
	{
		return cx == cxValue && cy == cyValue;
	}

	/** 判断是否与另外一个Size相同
	*/
	bool Equals(const UiSize& dst) const
	{
		return cx == dst.cx && cy == dst.cy;
	}

	/** 判断两个值是否相等
	*/
	friend bool operator == (const UiSize& a, const UiSize& b)
	{
		return a.Equals(b);
	}

	/** 判断两个值是否不相等
	*/
	friend bool operator != (const UiSize& a, const UiSize& b)
	{
		return !a.Equals(b);
	}

public:
    /** 宽度
    */
    int32_t cx;

    /** 高度
    */
    int32_t cy;
};

} // namespace ui

#endif // UI_CORE_UISIZE_H_
