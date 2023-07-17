#ifndef UI_CORE_UISIZE64_H_
#define UI_CORE_UISIZE64_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>

namespace ui 
{

/** Size64的封装
*/
class UILIB_API UiSize64
{
public:
	UiSize64()
	{
		cx = cy = 0;
	}

	UiSize64(const UiSize64& src)
	{
		cx = src.cx;
		cy = src.cy;
	}

	UiSize64(int64_t cxValue, int64_t cyValue)
	{
		cx = cxValue;
		cy = cyValue;
	}

	/** 设置新的宽度和高度
	*/
	void SetWidthHeight(int64_t cxValue, int64_t cyValue)
	{
		cx = cxValue;
		cy = cyValue;
	}

	/** 设置新的宽度
	*/
	void SetWidth(int64_t cxValue) { cx = cxValue; }

	/** 设置新的高度
	*/
	void SetHeight(int64_t cyValue) { cy = cyValue; }

	/** 返回宽度
	*/
	constexpr int64_t Width() const { return cx; }

	/** 返回高度
	*/
	constexpr int64_t Height() const { return cy; }

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
	bool Equals(int64_t cxValue, int64_t cyValue) const
	{
		return cx == cxValue && cy == cyValue;
	}

	/** 判断是否与另外一个Size相同
	*/
	bool Equals(const UiSize64& dst) const
	{
		return cx == dst.cx && cy == dst.cy;
	}

	/** 判断两个值是否相等
	*/
	friend bool operator == (const UiSize64& a, const UiSize64& b)
	{
		return a.Equals(b);
	}

	/** 判断两个值是否不相等
	*/
	friend bool operator != (const UiSize64& a, const UiSize64& b)
	{
		return !a.Equals(b);
	}

public:
	/** 宽度
	*/
	int64_t cx;

	/** 高度
	*/
	int64_t cy;
};

} // namespace ui

#endif // UI_CORE_UISIZE_H_
