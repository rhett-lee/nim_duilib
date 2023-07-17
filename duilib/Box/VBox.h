#ifndef UI_CORE_VBOX_H_
#define UI_CORE_VBOX_H_

#pragma once

#include "duilib/Core/Box.h"
#include "duilib/Box/VLayout.h"

namespace ui
{

/** 垂直布局的Box
*/
class UILIB_API VBox : public Box
{
public:
	VBox() :
		Box(new VLayout())
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_VBOX; }
};

}
#endif // UI_CORE_VBOX_H_
