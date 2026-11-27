#ifndef UI_UTILS_SCREEN_CAPTURE_X11_H_
#define UI_UTILS_SCREEN_CAPTURE_X11_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/Window.h"
#include <memory>

namespace ui
{
/** 屏幕截图（Linux X11的实现）
 *  通过X11抓取窗口所在显示器的整屏内容并转换为位图；仅Linux/FreeBSD平台编译。
 */
class DUILIB_API ScreenCapture_X11
{
public:
    /** 抓一张窗口所在屏幕的截图
    * @param [in] pWindow 窗口，不可为nullptr；为nullptr时直接返回nullptr
    * @return 返回抓取到的屏幕位图；参数非法、无法连接X服务器或抓取失败时返回nullptr
    */
    static std::shared_ptr<IBitmap> CaptureBitmap(const Window* pWindow);
};

} // namespace ui

#endif // UI_UTILS_SCREEN_CAPTURE_X11_H_
