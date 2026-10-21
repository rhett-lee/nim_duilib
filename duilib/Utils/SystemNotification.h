#ifndef UI_UTILS_SYSTEM_NOTIFICATION_H_
#define UI_UTILS_SYSTEM_NOTIFICATION_H_

#include "duilib/duilib_defs.h"

namespace ui
{

/** 系统通知的优先级
*/
enum class SystemNotificationPriority
{
    kLow = -1,      //低优先级
    kNormal = 0,    //普通优先级
    kHigh = 1,      //高优先级
    kCritical = 2,  //最高优先级（可能覆盖系统的"勿扰模式"）
};

/** 系统原生通知
*
* 与 ToastWnd 相互独立：ToastWnd 是应用内自绘的提示框，本类走操作系统的通知通道。
*
* 平台实现说明：
* 1. 使用 SDL 构建时：通过 SDL 的系统通知 API 实现，支持 Windows 10+、macOS 10.14+、
*    Linux/FreeBSD（依赖 org.freedesktop.Notifications 通知守护进程）。
*    macOS 要求程序运行在 *.app 包内，否则显示失败。
* 2. 不使用 SDL 的 Windows 构建：通过 Shell_NotifyIcon（托盘气泡通知）实现，支持 Windows 7 及以上。
*    显示期间会创建一个临时托盘图标，通知超时或被点击/移除后自动清理。
* 3. 通知的最终显示样式与时长由系统策略决定；本类不提供通知点击回调。
*
* 注意：Windows 原生实现依赖窗口消息，需要在 UI 线程（有消息循环的线程）调用。
*/
class DUILIB_API SystemNotification
{
public:
    /** 当前平台是否支持系统通知
    * @return 支持返回true，否则返回false
    */
    static bool IsSupported();

    /** 请求系统通知权限（部分平台首次显示通知前需要，重复调用无副作用）
    * @return 请求已成功发出返回true；平台无此机制或不受支持时返回false
    */
    static bool RequestPermission();

    /** 显示系统通知
    * @param [in] title 通知标题（不能为空）
    * @param [in] content 通知内容
    * @param [in] priority 通知优先级（仅 SDL 实现生效）
    * @param [in] timeoutMs 显示时长（毫秒，仅 Windows 原生实现生效，默认5000）
    * @return 通知ID（大于0），失败返回0
    */
    static uint32_t Show(const DString& title, const DString& content,
                         SystemNotificationPriority priority = SystemNotificationPriority::kNormal,
                         uint32_t timeoutMs = 5000);

    /** 移除已显示的通知
    * @param [in] notificationId Show 返回的通知ID
    * @return 移除成功返回true；通知不存在或平台不支持移除时返回false
    */
    static bool Remove(uint32_t notificationId);
};

} //namespace ui

#endif // UI_UTILS_SYSTEM_NOTIFICATION_H_
