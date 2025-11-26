#ifndef UI_UTILS_MONITOR_UTIL_H_
#define UI_UTILS_MONITOR_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
class WindowBase;

/** 显示器显示比例辅助类
*/
class UILIB_API MonitorUtil
{
public:
    /** 获取窗口所在显示器的DPI缩放比例, 如果pWindowBase为nullptr, 则获取主显示屏的DPI缩放比
    * @param [in] pWindowBase 窗口
    * @param [out] fWindowPixelDensity 返回窗口的像素密度值
    * @return 返回该窗口的DPI缩放比例,1.0f表示显示比例为100%
    */
    static float GetWindowDisplayScale(const WindowBase* pWindowBase, float& fWindowPixelDensity);

    /** 获取主显示器的DPI缩放比例
    * @return 返回该主显示器的DPI缩放比例,1.0f表示显示比例为100%
    */
    static float GetPrimaryMonitorDisplayScale();

};

} // namespace ui

#endif // UI_UTILS_MONITOR_UTIL_H_
