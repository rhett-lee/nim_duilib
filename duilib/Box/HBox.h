#ifndef UI_CORE_HBOX_H_
#define UI_CORE_HBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/HLayout.h"

namespace ui
{

/** 水平布局的Box
*/
class UILIB_API HBox : public Box
{
public:
    explicit HBox(Window* pWindow) :
        Box(pWindow, new HLayout())
    {
    }

    virtual DString GetType() const override { return DUI_CTR_HBOX; }
};

}
#endif // UI_CORE_HBOX_H_
