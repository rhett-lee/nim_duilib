#ifndef UI_UTILS_SCREEN_CAPTURE_WAYLAND_H_
#define UI_UTILS_SCREEN_CAPTURE_WAYLAND_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/Window.h"
#include <memory>

namespace ui
{
/** 屏幕截图（Linux Wayland的实现）
 *  Wayland环境下的屏幕抓取尚未实现，CaptureBitmap仅做环境检测并返回nullptr；
 *  同时提供Wayland桌面环境的检测接口（仅Linux/FreeBSD平台编译）。
 */
class DUILIB_API ScreenCapture_Wayland
{
public:
    /** 抓一张窗口所在屏幕的截图
    * @param [in] pWindow 窗口，不可为nullptr
    * @return 当前实现始终返回nullptr（Wayland抓屏未实现；pWindow为nullptr或非Wayland环境时也返回nullptr）
    */
    static std::shared_ptr<IBitmap> CaptureBitmap(const Window* pWindow);
        
    /** 检查当前是否为Wayland桌面环境
    * @return 当前为Wayland桌面环境时返回true，否则返回false；
    *         通过环境变量WAYLAND_DISPLAY或XDG_SESSION_TYPE判断
    */
    static bool IsWaylandEnvironment();
};

} // namespace ui

#endif // UI_UTILS_SCREEN_CAPTURE_WAYLAND_H_
