#ifndef UI_BOX_TILEBOX_H_
#define UI_BOX_TILEBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/VTileLayout.h"
#include "duilib/Box/HTileLayout.h"

namespace ui
{

/** 瓦片布局的Box(纵向布局)
*/
class UILIB_API VTileBox : public Box
{
public:
    explicit VTileBox(Window* pWindow) :
        Box(pWindow, new VTileLayout())
    {
    }

    virtual DString GetType() const override { return DUI_CTR_VTILE_BOX; }
};

/** 瓦片布局的Box(水平布局)
*/
class UILIB_API HTileBox : public Box
{
public:
    explicit HTileBox(Window* pWindow) :
        Box(pWindow, new HTileLayout())
    {
    }

    virtual DString GetType() const override { return DUI_CTR_HTILE_BOX; }
};

}
#endif // UI_BOX_TILEBOX_H_
