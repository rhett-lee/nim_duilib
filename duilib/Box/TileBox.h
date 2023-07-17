#ifndef UI_BOX_TILEBOX_H_
#define UI_BOX_TILEBOX_H_

#pragma once
#include "duilib/Core/Box.h"
#include "duilib/Box/TileLayout.h"

namespace ui
{

/** ÍßÆ¬²¼¾ÖµÄBox
*/
class UILIB_API TileBox : public Box
{
public:
	TileBox() :
		Box(new TileLayout())
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_TILEBOX; }
};

}
#endif // UI_BOX_TILEBOX_H_
