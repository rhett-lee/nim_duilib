#ifndef UI_BOX_VLAYOUT_H_
#define UI_BOX_VLAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 垂直布局：各个子控件横向坐标的left都相同，垂直方向依次排列
*/
class UILIB_API VLayout : public Layout
{
public:
	VLayout();
	
	/** 调整内部所有控件的位置信息
	 * @param[in] items 控件列表
	 * @param[in] rc 当前容器位置信息, 外部调用时，不需要扣除内边距
	 * @return 返回排列后最终盒子的宽度和高度信息
	 */
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;

	/** 根据内部子控件大小估算容器自身大小，拉伸类型的子控件被忽略，不计入大小估算
	 * @param[in] items 子控件列表
	 * @param[in] szAvailable 子控件允许的最大宽度和高度
	 * @return 返回排列后最终布局的大小信息（宽度和高度），不包含拉伸类型的子控件大小
	 */
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
};

} // namespace ui

#endif // UI_BOX_VLAYOUT_H_
