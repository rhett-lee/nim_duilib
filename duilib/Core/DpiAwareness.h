#ifndef UI_CORE_DPI_AWARENESS_H_
#define UI_CORE_DPI_AWARENESS_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** DPI感知模式的定义（参照Windows平台的DPI感知功能定义，功能定义同时也兼容其他平台，其他平台最终只有支持DPI感知和不支持DPI感知两种结果）
 *  对于Linux/macOS/FreeBSD平台：
 *  1. 设置为 kDpiUnaware/kSystemDpiAware 具有相同的结果：都是不支持DPI感知，高DPI时，界面不会跟随DPI等比例放大，会出现模糊现象
 *  2. 设置为 kFromManifest/kPerMonitorDpiAware/kPerMonitorDpiAware_V2 具有相同的结果：支持DPI感知，高DPI时，界面会等比例放大，界面显示效果清晰
 */
enum class UILIB_API DpiAwarenessMode
{
    /** 设置根据平台设置不同的DPI感知模式：
    *   Windows平台：从可执行程序exe文件的manifest配置中读取，不需要使用代码设置，如果exe的manifest未配置，则不支持DPI感知
    *   Linux/macOS/FreeBSD平台：默认按支持DPI感知处理，会被设置为kPerMonitorDpiAware_V2
    */
    kFromManifest = -1,

    /** 无法感知
     *  应用程序的 DPI 视图：所有显示器均为 96 DPI，界面显示比例为100%
     *  屏幕的DPI更改时的行为：位图拉伸（模糊）
     */
    kDpiUnaware = 0,

    /** 系统感知
     *  应用程序的 DPI 视图：所有显示器均为 96 DPI，界面显示比例为100%
     *  屏幕的DPI更改时的行为：位图拉伸（模糊））
     */
    kSystemDpiAware = 1,

    /** 每显示器感知
     *  Window平台：引入的Windows版本：8.1
     *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
     *  屏幕的DPI更改时的行为：界面按屏幕DPI等比例放大，界面效果清晰
     */
    kPerMonitorDpiAware = 2,

    /** 每显示器 V2 感知
     *  Window平台：引入的Windows版本：Windows 10 创意者更新 (1703)
     *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
     *  屏幕的DPI更改时的行为：界面按屏幕DPI等比例放大，界面效果清晰
     */
    kPerMonitorDpiAware_V2 = 3
};

/** 界面显示比例的最小值
*/
#define DUILIB_DISPLAY_SCALE_MIN    (0.60f)

/** 界面显示比例的最大值
*/
#define DUILIB_DISPLAY_SCALE_MAX    (5.00f)

/** DPI感知功能初始化参数
*/
class UILIB_API DpiInitParam
{
public:
    /** 参数设置的DPI感知参数模式
    */
    DpiAwarenessMode m_dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware_V2;

    /** 界面显示比例: 1.0f表示原值，显示比例为100%，2.0f表示放大到原来的200%
    */
    float m_fDisplayScale = 1.0f;

    /** 是否支持界面像素比例(该值只在开启SDL时有效, 且只在支持高分屏的系统环境中生效)
     *  在macOS/Wayland桌面环境中：true表示支持高分屏，false表示不支持高分屏
     *  在Windows/X11桌面环境中：该参数无效，会被忽略
     */
    bool m_bEnablePixelDensity = true;
};

/** DPI感知功能的接口
*/
class UILIB_API DpiAwareness
{
public:
    DpiAwareness();
    ~DpiAwareness();
    DpiAwareness(const DpiAwareness&) = delete;
    DpiAwareness& operator = (const DpiAwareness&) = delete;

public:
    /** 初始化DPI感知模式和DPI值（该函数只可调用一次，后续调用被忽略）
    * @param [in] initParam 初始化参数，详见参数说明
    */
    bool InitDpiAwareness(DpiAwarenessMode dpiAwarenessMode);

    /** 获取当前进程的DPI感知模式
    *   说明：该函数的返回值与InitDpiAwareness传入的值可能不相等
    */
    DpiAwarenessMode GetDpiAwareness() const;

public:
    /** 设置进程的DPI感知模式
       （DPI感知模式只允许设置一次，设置成功后不允许修改；如果在可执行程序的manifest配置中设置了感知模式，也无法更改）
    * @param [in] dpiAwarenessMode 期望的DPI感知模式，总体策略是逐步降级设置
    *             依次为：kPerMonitorDpiAware_V2 -> kPerMonitorDpiAware -> kSystemDpiAware
    * @return 返回实际的进程DPI感知模式
    */
    DpiAwarenessMode SetDpiAwareness(DpiAwarenessMode dpiAwarenessMode);

private:
    /** 外部设置的值
    */
    DpiAwarenessMode m_dpiAwarenessMode;
};
}
#endif //UI_CORE_DPI_AWARENESS_H_
