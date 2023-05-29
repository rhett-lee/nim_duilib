#ifndef UI_UTILS_BOXSHADOW_H_
#define UI_UTILS_BOXSHADOW_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>

namespace ui {

// 控件阴影属性类
// 待优化：
// 1.实现真正高斯模糊
//   参考资料：
//           https://www.cnblogs.com/mfbzr/p/14154156.html
//           https://stackoverflow.com/questions/42314939/diagonal-shadow-with-gdi
//           https://blog.csdn.net/aoshilang2249/article/details/45153375
//			 https://codersblock.com/blog/creating-glow-effects-with-css/
//   算法:
//           http://blog.ivank.net/fastest-gaussian-blur.html
//           https://software.intel.com/content/www/us/en/develop/articles/iir-gaussian-blur-filter-implementation-using-intel-advanced-vector-extensions.html
//           https://software.intel.com/content/dam/develop/public/us/en/downloads/gaussian_blur_0311.cpp
// 2.阴影应提前绘制好并进行缓存，而不是实时绘制
class UILIB_API BoxShadow 
{
public:
	BoxShadow();
	~BoxShadow();

	/**
	 * @brief 设置阴影属性
	 * @param[in] strBoxShadow 要设置的属性,如 "color='black' offset='1,1' blursize='2'"
	 * @return 无
	 */
	void SetBoxShadowString(const std::wstring& strBoxShadow);

	/**
	 * @brief 是否有阴影
	 * @return 是或否
	 */
	bool HasShadow() const;

	// 阴影属性
	// 暂时仅仅支持 color offset blurSize
	// https://codersblock.com/blog/creating-glow-effects-with-css/

	/** 阴影的颜色值
	*/
	std::wstring m_strColor;

	/** 设置阴影偏移量（offset-x 和 offset-y）
	 *                 <offset-x> 设置水平偏移量，如果是负值则阴影位于矩形左边。 
	 *                 <offset-y> 设置垂直偏移量，如果是负值则阴影位于矩形上面。
	 */
	UiPoint m_cpOffset;

	/** 模糊半径，半径必须介于 0 到 255 的范围内。
	 *  值越大，模糊面积越大，阴影就越大越淡。 不能为负值。默认为0，此时阴影边缘锐利。
	 */
	int m_nBlurRadius;

	/** 扩展半径，即模糊区域距离rc矩形边缘多少个像素。
	 *   取正值时，阴影扩大；取负值时，阴影收缩。默认为0，此时阴影与元素同样大。
	 */
	int m_nSpreadRadius;

	/** 当为true的时候表示阴影在矩形边框外面，为false的时候表示阴影在边框矩形内部
	*/
	bool m_bExclude;
};

} // namespace ui

#endif // !UI_UTILS_BOXSHADOW_H_

