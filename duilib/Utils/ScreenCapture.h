#ifndef UI_UTILS_SCREEN_CAPTURE_H_
#define UI_UTILS_SCREEN_CAPTURE_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/Window.h"
#include <memory>

namespace ui
{
/** 屏幕截图
*/
class UILIB_API ScreenCapture
{
public:
    /** 抓一张窗口所在屏幕的截图
    * @param [in] pWindow 窗口
    */
    static std::shared_ptr<IBitmap> CaptureBitmap(const Window* pWindow);
};

} // namespace ui

#endif // UI_UTILS_SCREEN_CAPTURE_H_
