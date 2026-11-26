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
class DUILIB_API DpiManager
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
    * @param [in] fDisplayScale 界面缩放比，1.0f表示无缩放
    * @param [in] fPixelDensity 窗口像素密度，1.0f表示无缩放（仅SDL实现时有效）
    */
    void SetDisplayScale(float fDisplayScale, float fPixelDensity);

    /** 比较DPI缩放比与现在的值相比是否变化
    * @param [in] fDisplayScale 目标界面缩放比
    * @param [in] fPixelDensity 目标窗口像素密度
    * @return 返回true表示缩放比发生变化，返回false表示未变化
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

    /** 获取按当前界面缩放比缩放后的整数
    */
    int32_t GetScaleInt(int32_t nValue) const;
    uint32_t GetScaleInt(uint32_t nValue) const;

    /** 将按旧界面缩放比计算的整数，换算到当前界面缩放比下
    * @param [in] nValue 按旧缩放比计算的数值
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的数值
    */
    int32_t GetScaleInt(int32_t nValue, uint32_t nOldScaleFactor) const;
    uint32_t GetScaleInt(uint32_t nValue, uint32_t nOldScaleFactor) const;

    /** 获取按当前界面缩放比缩放后的浮点值
    */
    float GetScaleFloat(int32_t nValue) const;
    float GetScaleFloat(uint32_t nValue) const;
    float GetScaleFloat(float fValue) const;

    /** 将按旧界面缩放比计算的浮点值，换算到当前界面缩放比下
    * @param [in] fValue 按旧缩放比计算的数值
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的数值
    */
    float GetScaleFloat(float fValue, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiSize
    */
    void ScaleSize(UiSize& size) const;

    /** 获取按当前界面缩放比缩放后的UiSize
    */
    UiSize GetScaleSize(UiSize size) const;

    /** 将按旧界面缩放比计算的UiSize，换算到当前界面缩放比下
    * @param [in] size 按旧缩放比计算的尺寸
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的尺寸
    */
    UiSize GetScaleSize(UiSize size, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiPoint
    */
    void ScalePoint(UiPoint& point) const;

    /** 将按旧界面缩放比计算的UiPoint，换算到当前界面缩放比下
    * @param [in] point 按旧缩放比计算的坐标点
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的坐标点
    */
    UiPoint GetScalePoint(UiPoint point, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放UiRect
    */
    void ScaleRect(UiRect& rect) const;
    void ScalePadding(UiPadding& padding) const;
    void ScaleMargin(UiMargin& margin) const;

    /** 将按旧界面缩放比计算的矩形，换算到当前界面缩放比下
    * @param [in] rect 按旧缩放比计算的矩形
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的矩形
    */
    UiRect GetScaleRect(UiRect rect, uint32_t nOldScaleFactor) const;

    /** 将按旧界面缩放比计算的内边距，换算到当前界面缩放比下
    * @param [in] padding 按旧缩放比计算的内边距
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的内边距
    */
    UiPadding GetScalePadding(UiPadding padding, uint32_t nOldScaleFactor) const;

    /** 将按旧界面缩放比计算的外边距，换算到当前界面缩放比下
    * @param [in] margin 按旧缩放比计算的外边距
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的外边距
    */
    UiMargin GetScaleMargin(UiMargin margin, uint32_t nOldScaleFactor) const;

public:
    /// 高分屏支持相关API接口

    /** 根据界面缩放比来缩放窗口大小相关数值（同时处理窗口像素密度）
    */
    void ScaleWindowSize(int32_t& windowSize) const;

    /** 将窗口大小还原为缩放前的数值（同时处理窗口像素密度）
    */
    void UnscaleWindowSize(int32_t& windowSize) const;

    /** 获取按当前界面缩放比缩放后的窗口大小（同时处理窗口像素密度）
    */
    int32_t GetScaleWindowSize(int32_t windowSize) const;

    /** 获取还原后的窗口大小（同时处理窗口像素密度）
    */
    int32_t GetUnscaleWindowSize(int32_t windowSize) const;

    /** 将按旧界面缩放比计算的窗口大小，换算到当前界面缩放比下
    * @param [in] windowSize 按旧缩放比计算的窗口大小
    * @param [in] nOldScaleFactor 旧的界面缩放百分比
    * @return 返回换算到当前缩放比后的窗口大小
    */
    int32_t GetScaleWindowSize(int32_t windowSize, uint32_t nOldScaleFactor) const;

    /** 根据界面缩放比来缩放窗口大小（UiSize 版本）
    */
    void ScaleWindowSize(UiSize& windowSize) const;

    /** 将窗口大小还原为缩放前的数值（UiSize 版本）
    */
    void UnscaleWindowSize(UiSize& windowSize) const;

    /** 获取按当前界面缩放比缩放后的窗口大小（UiSize 版本）
    */
    UiSize GetScaleWindowSize(UiSize windowSize) const;

    /** 获取还原后的窗口大小（UiSize 版本）
    */
    UiSize GetUnscaleWindowSize(UiSize windowSize) const;

    //将客户区尺寸转换为窗口尺寸(仅换算大小：除以窗口像素密度)
    void ClientSizeToWindowSize(int32_t& pt) const;
    void ClientSizeToWindowSize(UiPoint& pt) const;
    void ClientSizeToWindowSize(UiSize& size) const;
    void ClientSizeToWindowSize(UiRect& rc) const;
    void ClientSizeToWindowSize(UiPadding& padding) const;
    void ClientSizeToWindowSize(UiMargin& margin) const;

    /** 将客户区尺寸（物理像素）转换为DIP尺寸（同时处理像素密度与界面缩放比）
    */
    int32_t GetDipSizeFromClientSize(int32_t clientSize) const;
    UiSize GetDipSizeFromClientSize(UiSize clientSize) const;

    //将窗口尺寸转换为客户区尺寸(仅换算大小：乘以窗口像素密度)
    void WindowSizeToClientSize(int32_t& pt) const;
    void WindowSizeToClientSize(UiPoint& pt) const;
    void WindowSizeToClientSize(UiSize& size) const;
    void WindowSizeToClientSize(UiRect& rc) const;
    void WindowSizeToClientSize(UiPadding& padding) const;
    void WindowSizeToClientSize(UiMargin& margin) const;

public:
    /** 将已经做过DPI缩放的整数还原为原数值（即恢复到缩放比为1.0f条件下的原值）
    */
    void UnscaleInt(int32_t& nValue) const;
    void UnscaleInt(uint32_t& nValue) const;

    /** 获取还原后的整数（恢复到缩放比为1.0f条件下的原值）
    */
    int32_t GetUnscaleInt(int32_t nValue) const;
    uint32_t GetUnscaleInt(uint32_t nValue) const;

public:
    /** MulDiv 函数封装
    * @param [in] nNumber 被乘数
    * @param [in] nNumerator 乘数
    * @param [in] nDenominator 除数
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
