#ifndef UI_BOX_LAYOUT_H_
#define UI_BOX_LAYOUT_H_

#pragma once

#include "duilib/Core/Control.h"
#include <vector>

namespace ui 
{
class Box;

/** 控件布局：各个子控件顶点坐标(left,top)都相同，各个控件堆叠排列（相当于都看成是Float控件）
*/
class UILIB_API Layout
{
public:
	Layout();
	Layout(const Layout& r) = delete;
	Layout& operator=(const Layout& r) = delete;
	virtual ~Layout() = default;

	/** 设置所有者
	 */
	void SetOwner(Box* pOwner);

	/** 设置浮动状态下的坐标信息
	 * @param[in] pControl 控件句柄
	 * @param[in] rcContainer 要设置的位置信息
	 * @return 返回控件最终的大小信息（宽度和高度）
	 */
	static UiSize64 SetFloatPos(Control* pControl, UiRect rcContainer);

	/** 设置布局属性
	 * @param[in] strName 要设置的属性名
	 * @param[in] strValue 要设置的属性值
	 * @return true 设置成功，false 属性不存在
	 */
	virtual bool SetAttribute(const std::wstring& strName, const std::wstring& strValue);

	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息
	 * @return 返回排列后最终盒子的宽度和高度信息
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc);

	/**
	 * @brief 根据内部子控件大小调整容器自身大小
	 * @param[in] items 控件列表
	 * @param[in] szAvailable 子控件允许的最大宽度
	 * @return 返回排列后最终布局的大小信息（宽度和高度）
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable);

public:
	/** 获取内边距
	 * @return 返回内边距四边的大小（Rect的四个参数分别代表四个边的内边距）
	 */
	const UiRect& GetPadding() const { return m_rcPadding; }

	/**
	 * @brief 设置内边距
	 * @param[in] rcPadding 内边距数据
	 * @param[in] bNeedDpiScale 是否根据 DPI 自适应，默认为 true
	 */
	void SetPadding(UiRect rcPadding, bool bNeedDpiScale);

	/** 获取子控件之间的额外边距（X轴方向）
	 * @return 返回额外间距的数值
	 */
	int32_t GetChildMarginX() const { return m_iChildMarginX; }

	/** 获取子控件之间的额外边距（Y轴方向）
	 * @return 返回额外间距的数值
	 */
	int32_t GetChildMarginY() const { return m_iChildMarginY; }

	/** 设置子控件之间的额外边距（X轴方向）
	 * @param[in] iMarginX 要设置的边距数值
	 */
	void SetChildMarginX(int32_t iMarginX);

	/** 设置子控件之间的额外边距（Y轴方向）
	 * @param[in] iMarginY 要设置的边距数值
	 */
	void SetChildMarginY(int32_t iMarginY);

	/** 设置子控件之间的额外边距（X轴方向和Y轴方向，均设置为同一个值）
	 * @param[in] iMargin 要设置的边距数值
	 */
	void SetChildMargin(int32_t iMargin);

	/** 获取除了内边距外的可用范围
	 * @return 返回可用范围位置信息
	 */
	UiRect GetInternalPos() const;

protected:

	//所属Box对象
	Box* m_pOwner;

private:
	//内边距四边的大小
	UiRect m_rcPadding;

	//子控件之间的额外边距: X 轴方向
	int32_t m_iChildMarginX;

	//子控件之间的额外边距: Y 轴方向
	int32_t m_iChildMarginY;
};

} // namespace ui

#endif // UI_BOX_LAYOUT_H_
