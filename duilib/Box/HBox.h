#ifndef UI_CORE_HBOX_H_
#define UI_CORE_HBOX_H_

#pragma once

#include "duilib/Core/Box.h"

namespace ui
{

class UILIB_API HLayout : public Layout
{
public:
	HLayout();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual UiSize64 ArrangeChild(const std::vector<Control*>& items, UiRect rc) override;
	virtual UiSize EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable) override;
};

class UILIB_API HBox : public Box
{
public:
	HBox();

	virtual std::wstring GetType() const override;
};

}
#endif // UI_CORE_HBOX_H_
