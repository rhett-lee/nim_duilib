#include "ScreenCapture.h"
#include "ScreenCapture_X11.h"
#include "ScreenCapture_Wayland.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
//Linux/FreeBSD OS

namespace ui
{
std::shared_ptr<IBitmap> ScreenCapture::CaptureBitmap(const ui::Window* pWindow)
{
    if (ScreenCapture_Wayland::IsWaylandEnvironment()) {
        // Wayland环境
        return ScreenCapture_Wayland::CaptureBitmap(pWindow);
    }
    else {
        // X11环境
        return ScreenCapture_X11::CaptureBitmap(pWindow);
    }    
}

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
