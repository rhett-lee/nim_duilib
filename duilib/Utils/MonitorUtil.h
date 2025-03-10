#ifndef UI_UTILS_MONITOR_UTIL_H_
#define UI_UTILS_MONITOR_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
class WindowBase;

/** 显示器操作的辅助类
*/
class UILIB_API MonitorUtil
{
public:
    /** 获取窗口所在显示器的DPI值
    * @param [in] pWindowBase 窗口
    * @return 返回该窗口的DPI值, 如果失败返回0
    */
    static uint32_t GetWindowDpi(const WindowBase* pWindowBase);

    /** 获取主显示器DPI
    * @return 返回 DPI值
    */
    static uint32_t GetPrimaryMonitorDPI();

};

} // namespace ui

#endif // UI_UTILS_MONITOR_UTIL_H_
