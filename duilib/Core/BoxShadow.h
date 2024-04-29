#ifndef UI_UTILS_BOXSHADOW_H_
#define UI_UTILS_BOXSHADOW_H_

#pragma once

#include "duilib/Core/UiPoint.h"
#include <string>

namespace ui {

/** 控件阴影属性类
*/
class UILIB_API BoxShadow
{
public:
	BoxShadow();

	/**
	 * @brief 设置阴影属性
	 * @param[in] strBoxShadow 要设置的属性,如 "color='black' offset='1,1' blur_radius='2' spread_radius='2'"
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

	/** 模糊半径，值越大，模糊面积越大，阴影就越大越淡, 如果为0，此时阴影边缘锐利，无模糊效果，不能为负值。
	 */
	int m_nBlurRadius;

	/** 扩展半径，即模糊区域距离rc矩形边缘多少个像素。取正值时，阴影扩大；取负值时，阴影收缩。
	 */
	int m_nSpreadRadius;
};

} // namespace ui

#endif // !UI_UTILS_BOXSHADOW_H_

