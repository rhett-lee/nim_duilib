#ifndef UI_CORE_HBOX_H_
#define UI_CORE_HBOX_H_

#include "duilib/Core/Box.h"
#include "duilib/Box/HLayout.h"
#include "duilib/Box/HFlowLayout.h"

namespace ui
{

/** 水平布局的Box(不换行)
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

/** 水平流式布局的Box(自动换行)
*/
class UILIB_API HFlowBox : public Box
{
public:
    explicit HFlowBox(Window* pWindow) :
        Box(pWindow, new HFlowLayout())
    {
    }

    virtual DString GetType() const override { return DUI_CTR_HFLOWBOX; }
};

}
#endif // UI_CORE_HBOX_H_
