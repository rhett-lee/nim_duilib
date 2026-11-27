#ifndef UI_UTILS_SCREEN_CAPTURE_H_
#define UI_UTILS_SCREEN_CAPTURE_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/Window.h"
#include <memory>

namespace ui
{
/** 屏幕截图
 *  抓取窗口所在显示器的整屏内容（不是窗口自身的区域），并保存为位图对象；
 *  抓屏失败时返回空指针，调用方需自行判空。
 */
class DUILIB_API ScreenCapture
{
public:
    /** 抓一张窗口所在屏幕的截图
    * @param [in] pWindow 窗口，不可为nullptr；为nullptr时直接返回nullptr
    * @return 返回抓取到的屏幕位图；参数非法、渲染工厂不可用或系统调用失败时返回nullptr
    */
    static std::shared_ptr<IBitmap> CaptureBitmap(const Window* pWindow);
};

} // namespace ui

#endif // UI_UTILS_SCREEN_CAPTURE_H_
