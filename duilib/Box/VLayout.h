#ifndef UI_BOX_VLAYOUT_H_
#define UI_BOX_VLAYOUT_H_

#pragma once

#include "duilib/Box/Layout.h"

namespace ui 
{

/** 垂直布局
*/
class UILIB_API VLayout : public Layout
{
public:
	VLayout();
	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
};

} // namespace ui

#endif // UI_BOX_VLAYOUT_H_
