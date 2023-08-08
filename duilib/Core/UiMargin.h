#ifndef UI_CORE_UIMARGIN_H_
#define UI_CORE_UIMARGIN_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>

namespace ui 
{
/** 外边距的数据结构封装（不是矩形，没有宽高的概念）
*   外边距：Margin是不包含控件的矩形区域内的。
*/
class UILIB_API UiMargin
{
public:
	UiMargin()
	{
		left = top = right = bottom = 0;
	}
	UiMargin(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom)
	{
		left = nLeft;
		top = nTop;
		right = nRight;
		bottom = nBottom;
	}
	/** 清零
	*/
	void Clear()
	{
		left = top = right = bottom = 0;
	}
	/** 判断是否为空
	*/
	bool IsEmpty() const
	{
		return (left == 0) && (top == 0) && (right == 0) && (bottom == 0);
	}

	/** 检查各个值否为有效值（各个值不允许为赋值），并修正
	*/
	void Validate()
	{
		if (left < 0) {
			left = 0;
		}
		if (top < 0) {
			top = 0;
		}
		if (right < 0) {
			right = 0;
		}
		if (bottom < 0) {
			bottom = 0;
		}
	}

	/** 判断与另外一个是否相同
	*/
	bool Equals(const UiMargin& r) const
	{
		return left == r.left && top == r.top &&
			   right == r.right && bottom == r.bottom;
	}

public:
	/** 矩形左侧的外边距
	*/
	int32_t left;

	/** 矩形上面的外边距
	*/
	int32_t top;

	/** 矩形右侧的外边距
	*/
	int32_t right;

	/** 矩形下面的外边距
	*/
	int32_t bottom;
};

/** 16位无符号的版本，用于存储，节约空间
*/
class UILIB_API UiMargin16
{
public:
	UiMargin16()
	{
		left = top = right = bottom = 0;
	}
	UiMargin16(uint16_t nLeft, uint16_t nTop, uint16_t nRight, uint16_t nBottom)
	{
		left = nLeft;
		top = nTop;
		right = nRight;
		bottom = nBottom;
	}
public:
	/** 矩形左侧的外边距
	*/
	uint16_t left;

	/** 矩形上面的外边距
	*/
	uint16_t top;

	/** 矩形右侧的外边距
	*/
	uint16_t right;

	/** 矩形下面的外边距
	*/
	uint16_t bottom;
};

} // namespace ui

#endif // UI_CORE_UIMARGIN_H_
