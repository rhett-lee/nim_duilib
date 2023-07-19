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

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
};

} // namespace ui

#endif // UI_BOX_HLAYOUT_H_
