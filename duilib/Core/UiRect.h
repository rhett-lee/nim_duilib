#ifndef UI_CORE_UIRECT_H_
#define UI_CORE_UIRECT_H_

#pragma once

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/UiSize.h"
#include "duilib/Core/UiPadding.h"
#include "duilib/Core/UiMargin.h"
#include <algorithm>

namespace ui 
{
/** 矩形区域的封装
*/
class UILIB_API UiRect
{
public:
	UiRect()
	{
		left = top = right = bottom = 0;
	}

	UiRect(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom)
	{
		left = nLeft;
		top = nTop;
		right = nRight;
		bottom = nBottom;
	}

	/** 获取left值
	*/
	constexpr int32_t Left() const { return left; }

	/** 获取right值
	*/
	constexpr int32_t Right() const { return right; }

	/** 获取top值
	*/
	constexpr int32_t Top() const { return top; }

	/** 获取bottom值
	*/
	constexpr int32_t Bottom() const { return bottom; }

	/** 获取宽度
	*/
	int32_t Width() const { return right - left; }

	/** 获取高度
	*/
	int32_t Height() const { return bottom - top; }

	/** 获取中心点的X轴坐标值
	*/
	int32_t CenterX() const { return (left + right) / 2; }

	/** 获取中心点的Y轴坐标值
	*/
	int32_t CenterY() const { return (top + bottom) / 2; }

	/** 获取矩形中心点坐标（X轴坐标和Y轴坐标）
	*/
	UiPoint Center() const { return { CenterX(), CenterY() }; }

	/** 清零
	*/
	void Clear()
	{
		left = top = right = bottom = 0;
	}

	/** 判断是否全为零
	*/
	bool IsZero() const
	{
		return (left == 0) && (right == 0) && (top == 0) && (bottom == 0);
	}

	/** 判断是否为空的矩形（宽和高不是有效值）
	*/
	bool IsEmpty() const
	{
		int32_t w = Width();
		int32_t h = Height();
		return (w <= 0 || h <= 0);
	}

	/** 检查宽度和高度值是否为有效值，并修正
	*/
	void Validate()
	{
		if (right < left) {
			right = left;
		}
		if (bottom < top) {
			bottom = top;
		}
	}

	/** 偏移操作
	*/
	void Offset(int32_t cx, int32_t cy)
	{
		left += cx;
		right += cx;
		top += cy;
		bottom += cy;
	}

	/** 偏移操作
	*/
	void Offset(const UiPoint& offset)
	{
		left += offset.X();
		right += offset.X();
		top += offset.Y();
		bottom += offset.Y();
	}

	/** 使矩形区域扩大（宽度增加2*dx，高度增加2*dy）
	*/
	void Inflate(int32_t dx, int32_t dy)
	{
		left -= dx;
		right += dx;
		top -= dy;
		bottom += dy;
	}

	/** 使矩形区域扩大
	* @param [in] margin 外边距的四边参数
	*/
	void Inflate(const UiMargin& margin)
	{
		left -= margin.left;
		right += margin.right;
		top -= margin.top;
		bottom += margin.bottom;
	}

	/** 使矩形区域扩大
	* @param [in] padding 内边距的四边参数
	*/
	void Inflate(const UiPadding& padding)
	{
		left -= padding.left;
		right += padding.right;
		top -= padding.top;
		bottom += padding.bottom;
	}

	/** 使矩形区域扩大
	* @param nLeft left扩大值
	* @param nTop top扩大值
	* @param nRight right扩大值
	* @param nBottom bottom扩大值
	*/
	void Inflate(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom)
	{
		left -= nLeft;
		right += nRight;
		top -= nTop;
		bottom += nBottom;
	}

	/** 使矩形区域缩小（宽度减少2*dx，高度减少2*dy）
	*/
	void Deflate(int32_t dx, int32_t dy)
	{
		left += dx;
		right -= dx;
		top += dy;
		bottom -= dy;
	}

	/** 使矩形区域缩小
	* @param padding 内边距的四边参数
	*/
	void Deflate(const UiPadding& padding)
	{
		left += padding.left;
		right -= padding.right;
		top += padding.top;
		bottom -= padding.bottom;
	}

	/** 使矩形区域缩小
	* @param margin 外边距的四边参数
	*/
	void Deflate(const UiMargin& margin)
	{
		left += margin.left;
		right -= margin.right;
		top += margin.top;
		bottom -= margin.bottom;
	}

	/** 使矩形区域缩小
	* @param nLeft left缩小值
	* @param nTop top缩小值
	* @param nRight right缩小值
	* @param nBottom bottom缩小值
	*/
	void Deflate(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom)
	{
		left += nLeft;
		right -= nRight;
		top += nTop;
		bottom -= nBottom;
	}

	/** 取两个矩形的并集
	* @param [in] r 另外一个矩形
	* @return 如果并集不是空矩形，返回true；如果并集是空矩形则返回false
	*/
	bool Union(const UiRect& r)
	{
		if (r.left >= r.right || r.top >= r.bottom) {
			//r是空矩形
			return !IsEmpty();
		}
		if (left >= right || top >= bottom) {
			//自己是空矩形
			*this = r;
		}
		else {
			//两个都不是空矩形
			left   = (std::min)(left, r.left);
			top    = (std::min)(top, r.top);
			right  = (std::max)(right, r.right);
			bottom = (std::max)(bottom, r.bottom);
		}
		return !IsEmpty();
	}

	/** 取两个矩形的交集
	* @param [in] r 另外一个矩形
	* @return 如果交集不是空矩形，返回true；如果交集是空矩形则返回false
	*/
	bool Intersect(const UiRect& r)
	{
		left = (std::max)(left, r.Left());
		top = (std::max)(top, r.Top());
		right = (std::min)(right, r.Right());
		bottom = (std::min)(bottom, r.Bottom());
		return !IsEmpty();
	}

	/** 取两个矩形(a,b)的交集, 将交集生成一个新的矩形c
	* @return 如果交集c不是空矩形，返回true；如果交集c是空矩形则返回false
	*/
	static bool Intersect(UiRect& c,const UiRect& a, const UiRect& b)
	{
		c.left = (std::max)(a.Left(), b.Left());
		c.top = (std::max)(a.Top(), b.Top());
		c.right = (std::min)(a.Right(), b.Right());
		c.bottom = (std::min)(a.Bottom(), b.Bottom());
		return !c.IsEmpty();
	}

	/** 判断一个点是否在矩形以内
	*/
	bool ContainsPt(const UiPoint& pt) const
	{
		return pt.X() >= left && pt.X() < right && pt.Y() >= top && pt.Y() < bottom;
	}

	/** 判断一个点是否在矩形以内
	*/
	bool ContainsPt(int32_t x, int32_t y) const
	{
		return x >= left && x < right && y >= top && y < bottom;
	}

	/** 判断是否包含另外一个矩形
	*/
	bool ContainsRect(const UiRect& r) const
	{
		return  !r.IsEmpty() && !this->IsEmpty() &&
			    left <= r.left && top <= r.top   &&
			    right >= r.right && bottom >= r.bottom;
	}

	/** 判断与另外一个矩形是否相同
	*/
	bool Equals(const UiRect& r) const
	{
		return left == r.left && top == r.top &&
			   right == r.right && bottom == r.bottom;
	}

	/** 判断两个值是否相等
	*/
	friend bool operator == (const UiRect& a, const UiRect& b)
	{
		return a.Equals(b);
	}

	/** 判断两个值是否不相等
	*/
	friend bool operator != (const UiRect& a, const UiRect& b)
	{
		return !a.Equals(b);
	}

public:
	/** 左上角X轴的坐标
	*/
	int32_t left;

	/** 左上角Y轴的坐标
	*/
	int32_t top;

	/** 右下角X轴的坐标
	*/
	int32_t right;

	/** 右下角Y轴的坐标
	*/
	int32_t bottom;
};

} // namespace ui

#endif // UI_CORE_UIPOINT_H_
