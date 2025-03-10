#ifndef UI_CORE_DPI_AWARENESS_H_
#define UI_CORE_DPI_AWARENESS_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** DPI感知模式
*/
enum class UILIB_API DpiAwarenessMode
{
    /** 无法感知
     *    应用程序的 DPI 视图：所有显示器均为 96 DPI
     *      DPI 更改时的行为：位图拉伸（模糊）
     */
    kDpiUnaware = 0,

    /** 系统
     *  引入的Windows版本：Vista
     *  应用程序的 DPI 视图：所有显示器均为 96 DPI
     *    DPI 更改时的行为：位图拉伸（模糊））
     */
    kSystemDpiAware = 1,

    /** 每显示器
     *  引入的Windows版本：8.1
     *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
     *    DPI 更改时的行为：通知顶级窗口 DPI 更改，没有任何 UI 元素的 DPI 缩放。
     */
    kPerMonitorDpiAware = 2,

    /** 每显示器 V2
     *  引入的Windows版本：Windows 10 创意者更新 (1703)
     *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
     *    DPI 更改时的行为：通知顶级窗口 DPI 更改，自动 DPI 缩放：非工作区、常用控件中的主题绘制位图 (comctl32 V6)、对话框 (CreateDialog)。
     */
    kPerMonitorDpiAware_V2 = 3
};

/** DPI感知功能初始化参数
*/
class UILIB_API DpiInitParam
{
public:
    DpiInitParam();

    enum class DpiAwarenessFlag
    {
        kFromManifest,  //从可执行程序的manifest配置中读取，不需要再次设置
        kFromUserDefine //通过参数设置（即：DpiInitParam::m_dpiAwarenessMode 成员变量）
                        //如果可执行程序的manifest配置已经设置了DPI感知类型，则是无法修改的。
    };

    /** DPI感知参数类型（默认是通过 m_dpiAwarenessMode 值设置的）
    */
    DpiAwarenessFlag m_dpiAwarenessFlag;

    /** 参数设置的DPI感知参数模式(仅当 m_dpiAwarenessFlag 为 kFromUserDefine时有效)
    */
    DpiAwarenessMode m_dpiAwarenessMode;

    /** 参数设置的DPI值（如果设置了有效值，则按设置的值作为启动时的DPI值；
                       如果未设置（值为0时），则读取系统配置的DPI值）
    */
    uint32_t m_uDPI;
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
    bool InitDpiAwareness(const DpiInitParam& initParam);

    /** 设置进程的DPI感知模式
       （DPI感知模式只允许设置一次，设置成功后不允许修改；如果在可执行程序的manifest配置中设置了感知模式，也无法更改）
    * @param [in] dpiAwarenessMode 期望的DPI感知模式，总体策略是逐步降级设置
    *             依次为：kPerMonitorDpiAware_V2 -> kPerMonitorDpiAware -> kSystemDpiAware
    * @return 返回实际的进程DPI感知模式
    */
    DpiAwarenessMode SetDpiAwareness(DpiAwarenessMode dpiAwarenessMode);

    /** 获取当前进程的DPI感知模式
    */
    DpiAwarenessMode GetDpiAwareness() const;

private:
    /** 外部设置的值
    */
    DpiAwarenessMode m_dpiAwarenessMode;
};
}
#endif //UI_CORE_DPI_AWARENESS_H_
