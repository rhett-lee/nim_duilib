#include "ClickThrough.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_MACOS

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse)
{
    if (pWindow == nullptr) {
        return false;
    }
    
    return false;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_MACOS
