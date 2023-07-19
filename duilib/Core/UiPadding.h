#ifndef UI_CORE_UIPADDING_H_
#define UI_CORE_UIPADDING_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <cstdint>

namespace ui 
{
/** 内边距的数据结构封装（不是矩形，没有宽高的概念）
*/
class UILIB_API UiPadding
{
public:
	UiPadding()
	{
		left = top = right = bottom = 0;
	}
	UiPadding(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom)
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
	bool Equals(const UiPadding& r) const
	{
		return left == r.left && top == r.top &&
			   right == r.right && bottom == r.bottom;
	}

public:
	/** 矩形左侧的内边距
	*/
	int32_t left;

	/** 矩形上面的内边距
	*/
	int32_t top;

	/** 矩形右侧的内边距
	*/
	int32_t right;

	/** 矩形下面的内边距
	*/
	int32_t bottom;
};

} // namespace ui

#endif // UI_CORE_UIPADDING_H_
