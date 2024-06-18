#ifndef UI_CORE_VBOX_H_
#define UI_CORE_VBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/VLayout.h"

namespace ui
{

/** 垂直布局的Box
*/
class UILIB_API VBox : public Box
{
public:
    explicit VBox(Window* pWindow) :
        Box(pWindow, new VLayout())
    {
    }

    virtual DString GetType() const override { return DUI_CTR_VBOX; }
};

}
#endif // UI_CORE_VBOX_H_
