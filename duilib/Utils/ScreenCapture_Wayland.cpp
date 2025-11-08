#include "ScreenCapture_Wayland.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
//Linux/FreeBSD OS

#include <cstdlib> // 用于 getenv

namespace ui
{
bool ScreenCapture_Wayland::IsWaylandEnvironment()
{
    // Wayland环境下通常会设置以下环境变量
    const char* waylandDisplay = getenv("WAYLAND_DISPLAY");
    const char* xdgSessionType = getenv("XDG_SESSION_TYPE");
    
    return (waylandDisplay != nullptr && *waylandDisplay != '\0') ||
           (xdgSessionType != nullptr && strcmp(xdgSessionType, "wayland") == 0);
}

std::shared_ptr<IBitmap> ScreenCapture_Wayland::CaptureBitmap(const ui::Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }

    // 获取原生窗口指针
    const NativeWindow* pNativeWnd = pWindow->NativeWnd();
    if (pNativeWnd == nullptr) {
        return nullptr;
    }
    
    // 提前检测Wayland环境，直接返回失败
    if (!IsWaylandEnvironment()) {
        return nullptr;
    }
    
    return nullptr;
}

} // namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
