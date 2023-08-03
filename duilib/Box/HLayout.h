#ifndef UI_BOX_HLAYOUT_H_
#define UI_BOX_HLAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 水平布局: 各个子控件纵向坐标的top都相同，水平方向依次排列
*/
class UILIB_API HLayout : public Layout
{
public:
	HLayout();

	/** 布局类型
	*/
	virtual LayoutType GetLayoutType() const override { return LayoutType::HLayout; }

	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息, 包含内边距，但不包含外边距
	 * @return 返回排列后最终盒子的宽度和高度信息，包含Owner Box的内边距，不包含外边距
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

	/** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
	 * @param[in] items 子控件列表
	 * @param [in] szAvailable 可用大小，包含分配给该控件的内边距，但不包含分配给控件的外边距
	 * @return 返回排列后最终布局的大小信息（宽度和高度）；
			   包含items中子控件的外边距，包含items中子控件的内边距；
			   包含Box控件本身的内边距；
			   不包含Box控件本身的外边距；
			   返回值中不包含拉伸类型的子控件大小。
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
};

} // namespace ui

#endif // UI_BOX_HLAYOUT_H_
