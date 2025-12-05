#ifndef UI_CORE_DPI_MANAGER_H_
#define UI_CORE_DPI_MANAGER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/DpiAwareness.h"

#if defined DUILIB_BUILD_FOR_WIN && defined DUILIB_BUILD_FOR_SDL
    //定义测试专用宏（可以在Windows环境下，模拟高分屏，进行功能测试）
    //#define DUILIB_HDPI_TEST_PIXEL_DENSITY (1.5f)
#endif

namespace ui
{
class WindowBase;

/** DPI感知功能的接口（支持高分屏）
*/
class UILIB_API DpiManager
{
public:
    DpiManager();
    ~DpiManager();
    DpiManager(const DpiManager&) = delete;
    DpiManager& operator = (const DpiManager&) = delete;

public:
    /** 初始化DPI感知模式和DPI值（该函数只可调用一次，后续调用被忽略）
     *   该函数在进程启动时用于初始化全局DPI管理器
     * @param [in] dpiInitParam 初始化参数，详见参数说明
     */
    void InitDpiAwareness(const DpiInitParam& dpiInitParam);

    /** 获取进程的DPI感知模式
     * 该属性为进程内属性，程序启动后设置一次，然后不可更改
     * @return 返回当前进程的DPI感知模式
     */
    DpiAwarenessMode GetDpiAwareness() const;

    /** 当前配置是否支持DPI感知，支持DPI感知的时候，同一个窗口在不同屏幕中，按所在屏幕的DPI显示，界面清晰
     *  该属性为进程内属性，程序启动后设置一次，然后不可更改
     *  实际是否支持DPI感知，需要组合判断：(!IsUserDefinedDpi() && IsDpiAware()) 是否为true
     * @return 返回true表示支持DPI感知，返回false标志不支持DPI感知
     */
    bool IsDpiAware() const;

public:
    /** 设置窗口的DPI缩放比，初始化DPI管理器
     @param [in] pWindow 窗口的接口，如果为nullptr，则读取系统配置的DPI值
    */
    void SetDisplayScaleForWindow(const WindowBase* pWindow);

    /** 设置窗口的DPI缩放比，初始化DPI管理器
    */
    void SetDisplayScale(float fDisplayScale, float fPixelDensity);

    /** 比较DPI缩放比与现在的值相比是否变化
    */
    bool IsDisplayScaleChanged(float fDisplayScale, float fPixelDensity) const;

    /** 是否为用户自定义的DPI
    * @return true表示用户自定义DPI, 不支持DPI感知；false表示系统管理DPI，可支持DPI感知
    */
    bool IsUserDefinedDpi() const;

    /** 设置是否为用户自定义的DPI，实际是否支持DPI感知，需要组合判断：(!IsUserDefinedDpi() && IsDpiAware()) 是否为true
    * @param [in] bUserDefinedDpi true表示用户自定义DPI, 不支持DPI感知；false表示系统管理DPI，可支持DPI感知
    */
    void SetUserDefinedDpi(bool bUserDefinedDpi);

public:
    /** 当前界面是否有DPI缩放
    * @return 如果当前界面缩放百分比为100返回false，否则返回true
    */
    bool IsDisplayScaled() const;

    /** 获取当前界面缩放百分比因子（举例：100代表缩放百分比为100%，无缩放）
    * @return 缩放比，比如：如果返回缩放比为125，代表界面缩放百分比为125%，其对应的DPI值是120
    */
    uint32_t GetDisplayScaleFactor() const;

    /** 获取当前界面缩放缩放比（举例：1.0代表缩放百分比为100%，无缩放）
    */
    float GetDisplayScale() const;

    /** 校验当前界面缩放百分比因子与目标缩放比是否一致，如果不一致进行断言报错
    */
    bool CheckDisplayScaleFactor(uint32_t nCheckScaleFactor) const;

public:
    /** 是否支持窗口像素密度值（仅在使用SDL实现时支持）
    */
    bool IsPixelDensityEnabled() const;

    /** 是否包含有效的窗口像素密度值
    */
    bool HasPixelDensity() const;

    /** 获取窗口像素密度, 1.0f表示无缩放
    */
    float GetPixelDensity() const;

public:
    /** 根据界面缩放比来缩放整数
    */
    void ScaleInt(int32_t& nValue) const;
    void ScaleInt(uint32_t& nValue) const;
    int32_t GetScaleInt(int32_t nValue) const;
    uint32_t GetScaleInt(uint32_t nValue) const;

    int32_t GetScaleInt(int32_t nValue, uint32_t nOldScaleFactor) const;
    uint32_t GetScaleInt(uint32_t nValue, uint32_t nOldScaleFactor) const;

    float GetScaleFloat(int32_t nValue) const;
    float GetScaleFloat(uint32_t nValue) const;
    float GetScaleFloat(float fValue) const;

    float GetScaleFloat(float fValue, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiSize
    */
    void ScaleSize(UiSize& size) const;
    UiSize GetScaleSize(UiSize size) const;

    UiSize GetScaleSize(UiSize size, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiPoint
    */
    void ScalePoint(UiPoint& point) const;

    UiPoint GetScalePoint(UiPoint point, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiRect
    */
    void ScaleRect(UiRect& rect) const;
    void ScalePadding(UiPadding& padding) const;
    void ScaleMargin(UiMargin& margin) const;

    UiRect GetScaleRect(UiRect rect, uint32_t nOldScaleFactor) const;
    UiPadding GetScalePadding(UiPadding padding, uint32_t nOldScaleFactor) const;
    UiMargin GetScaleMargin(UiMargin margin, uint32_t nOldScaleFactor) const;

public:
    /// 高分屏支持相关API接口

    /** 根据界面缩放比来缩放窗口大小相关数值
    */
    void ScaleWindowSize(int32_t& windowSize) const;
    void UnscaleWindowSize(int32_t& windowSize) const;

    int32_t GetScaleWindowSize(int32_t windowSize) const;
    int32_t GetUnscaleWindowSize(int32_t windowSize) const;

    int32_t GetScaleWindowSize(int32_t windowSize, uint32_t nOldScaleFactor) const;

    void ScaleWindowSize(UiSize& windowSize) const;
    void UnscaleWindowSize(UiSize& windowSize) const;

    UiSize GetScaleWindowSize(UiSize windowSize) const;
    UiSize GetUnscaleWindowSize(UiSize windowSize) const;

    //将客户区坐标转换为屏幕坐标(仅转换大小)
    void ClientSizeToWindowSize(int32_t& pt) const;
    void ClientSizeToWindowSize(UiPoint& pt) const;
    void ClientSizeToWindowSize(UiSize& size) const;
    void ClientSizeToWindowSize(UiRect& rc) const;
    void ClientSizeToWindowSize(UiPadding& padding) const;
    void ClientSizeToWindowSize(UiMargin& margin) const;

    //将屏幕坐标转换为客户区坐标(仅转换大小)
    void WindowSizeToClientSize(int32_t& pt) const;
    void WindowSizeToClientSize(UiPoint& pt) const;
    void WindowSizeToClientSize(UiSize& size) const;
    void WindowSizeToClientSize(UiRect& rc) const;
    void WindowSizeToClientSize(UiPadding& padding) const;
    void WindowSizeToClientSize(UiMargin& margin) const;

public:
    /** 将已经做过DPI缩放的数值还原为原数值（即恢复到缩放比为1.0f条件下的原值）
    */
    void UnscaleInt(int32_t& nValue) const;
    void UnscaleInt(uint32_t& nValue) const;
    int32_t GetUnscaleInt(int32_t nValue) const;
    uint32_t GetUnscaleInt(uint32_t nValue) const;

public:
    /** MulDiv 函数封装
    * @return 如果nDenominator为0，返回-1；否则返回nNumber * nNumerator / nDenominator的运算结果, 可确保运算中间结果不越界，并且计算结果有四舍五入
    */
    static int32_t MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator);
    static uint32_t MulDiv(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator);

private:
    /** 是否已经初始化过
    */
    bool m_bDpiInited;

    /** DPI值是否为用户自定义，如果为用户自定义，则不再支持DPI感知
    */
    bool m_bUserDefinedDpi;

    /** 是否支持界面像素比例(该值只在开启SDL时有效, 且只在支持高分屏的系统环境中生效)
     *  在macOS/Wayland桌面环境中：true表示支持高分屏，false表示不支持高分屏
     *  在Windows/X11桌面环境中：该参数无效
     */
    bool m_bEnablePixelDensity;

    /** 当前进程的DPI感知模式
    */
    DpiAwarenessMode m_dpiAwarenessMode;

private:
    /** DPI缩放因子，100表示无缩放
    */
    uint32_t m_nScaleFactor;

    /** 窗口像素密度, 1.0f表示无缩放
    */
    float m_fPixelDensity;
};
}
#endif //UI_CORE_DPI_MANAGER_H_
