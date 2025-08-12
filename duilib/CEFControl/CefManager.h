#ifndef UI_CEF_CONTROL_CEF_MANAGER_H_
#define UI_CEF_CONTROL_CEF_MANAGER_H_

#include "duilib/Core/Callback.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_app.h"
    #include "include/cef_version.h"
#pragma warning (pop)

#include <vector>
#include <atomic>
#include <chrono>

namespace ui
{
/** Browser进程单例控制的回调函数（仅在Windows平台 + CEF 109版本使用，其他情况下不使用）
 * (1) 由于一个Browser进程，需要启动好多个Render子进程，资源开销比较大，所以有必要控制保证只有一个Browser进程)
 * (2) CEF 133版本及以上版本，自身有单例控制功能，只要实现接口就可以了；但CEF 109版本无此功能，允许同时存在多个Browser进程
 * @param [in] argumentList 重新启动的Browser进程的启动参数列表
 */
typedef std::function<void (const std::vector<DString>& argumentList)> OnAlreadyRunningAppRelaunchEvent;

/** 设置CEF的初始化参数的回调，可以在回调函数中修改参数
 * @param [in] settings CEF模块的初始化参数
 */
typedef std::function<void (CefSettings& settings)> OnCefSettingsEvent;

//窗口类型
class Window;

/** 管理Cef组件的初始化、销毁、消息循环
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
class CefManager : public ui::SupportWeakCallback
{
public:
    CefManager();
    CefManager(const CefManager&) = delete;
    CefManager& operator=(const CefManager&) = delete;
protected:
    virtual ~CefManager();

public:
    /** 单例对象
    */
    static CefManager* GetInstance();

public:
    /** 设置CEF的网页缓存目录(如果不设置，则使用默认规则的缓存目录)
    *   可以是相对路径，如果设置了相对目录，则会在当前程序所在路径创建相应的相对目录
    *   默认规则："${程序目录}/cef_cache/${程序名称}"
    */
    virtual void SetCefCachePath(const DString& cefCachePath);

    /** 获取当前的CEF的网页缓存目录
    */
    virtual DString GetCefCachePath() const;

    /** 设置CEF模块运行库文件（libcef.dll）所在路径（仅限Windows平台，如果不设置，则使用默认规则的目录）
    *   可以是相对路径，如果设置了相对目录，则会在当前程序所在路径匹配相应的相对目录
    *   默认规则：
    *       Windows x64: "libcef_win\\x64"
    *       Windows x86: "libcef_win\\Win32"
    *       Windows x64: "libcef_win_109\\x64" (CEF 109)
    *       Windows x86: "libcef_win_109\\Win32" (CEF 109)
    */
    virtual void SetCefMoudlePath(const DString& cefMoudlePath);

    /** 获取CEF模块运行库文件所在路径
    */
    virtual DString GetCefMoudlePath() const;

    /** 设置默认语言，如果不设置的话，默认为简体中文（"zh-CN"）
    * @param [in] lang 语言字符串，比如"en-US"代表英文
    */
    virtual void SetCefLanguage(const DString& lang);

    /** 获取默认语言
    */
    virtual DString GetCefLanguage() const;

    /** 设置CEF的日志级别
    */
    virtual void SetLogSeverity(cef_log_severity_t log_severity);

    /** 获取CEF的日志级别
    */
    virtual cef_log_severity_t GetLogSeverity() const;

    /** 初始化cef组件
    * @param [in] bEnableOffScreenRendering 是否开启离屏渲染
    * @param [in] appName 产品名称标识符（必选参数，不可为空，用于生成CEF的缓存路径和控制进程唯一性）
    * @param [in] argc 程序启动时的参数个数(仅Linux环境使用，其他平台可传入0)
    * @param [in] argv 程序启动时的参数列表(仅Linux环境使用，其他平台可传入nullptr)
    * @param [in] callback CEF模块设置参数的回调函数，用于应用层修改初始化参数
    * @return bool true 继续运行，false 应该结束程序
    */
    virtual bool Initialize(bool bEnableOffScreenRendering,
                            const DString& appName,
                            int argc,
                            char** argv,
                            OnCefSettingsEvent callback);

    /** 清理cef组件
    */
    virtual void UnInitialize();

    /** 当前CEF是否运行在多线程消息循环状态(Windows/Linux平台支持，但MacOS不支持)
    */
    virtual bool IsMultiThreadedMessageLoop() const;

    /** 是否已经初始化
    */
    bool IsCefInited() const;

    /** 启动定时器，调用CEF消息处理函数：CefDoMessageLoopWork（仅限在IsMultiThreadedMessageLoop()为false时启用）
    *   如果未启用CEF的消息循环，则需要启动一个定时器保底（如果不做保底，运行不正常。OnScheduleMessagePumpWork函数的逻辑与期待不一致）
    */
    void ScheduleCefDoMessageLoopWork();

    /** 设置调用CEF消息处理函数定时器的时间间隔（毫秒）
    */
    void SetCefDoMessageLoopWorkDelayMs(int32_t nCefDoMessageLoopWorkDelayMs);

    /** 获取调用CEF消息处理函数定时器的时间间隔（毫秒）
    */
    int32_t GetCefDoMessageLoopWorkDelayMs() const;

public:
    /** 绑定一个回调函数用于监听Browser进程启动事件（仅在Windows + CEF109使用，其他情况不需要设置）
    * @param [in] callback 一个回调函数，参考 OnAlreadyRunningAppRelaunchEvent 声明
    */
    virtual void SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& callback);

    /** 获取监听Browser进程启动事件的回调函数
    */
    virtual OnAlreadyRunningAppRelaunchEvent GetAlreadyRunningAppRelaunch() const;

public:
    /** 是否开启离屏渲染(初始化后，不允许修改)
    * @return bool true 开启，false 不开启
    */
    bool IsEnableOffScreenRendering() const;

    /** 记录浏览器对象的数量（增加1）
    */
    void AddBrowserCount();

    /** 记录浏览器对象的数量（减少1）
    */
    void SubBrowserCount();

    /** 获取浏览器对象的数量
    */
    int32_t GetBrowserCount();

    /** 窗口关闭时，处理该窗口下的所有Browser控件，在窗口关闭前退出
    */
    void ProcessWindowCloseEvent(Window* pWindow);

    /** 在Cef浏览器对象销毁后发送QUIT消息，退出主进程的消息循环
    */
    void PostQuitMessage(int32_t nExitCode);

protected:
    /** 生成CEF配置数据
    */
    virtual void GetCefSetting(CefSettings& settings);

private:
    /** CEF的网页缓存目录(如果不设置，则使用默认规则的缓存目录)
    *   默认规则："${程序目录}/cef_cache/${程序名称}"
    */
    DString m_cefCachePath;

    /** 设置CEF模块运行库文件（libcef.dll）所在路径（仅限Windows平台，如果不设置，则使用默认规则的目录）
    */
    DString m_cefMoudlePath;

    /** 应用程序名称
    */
    DString m_appName;

    /** CEF的默认语言
    */
    DString m_lang;

    /** CEF配置的回调函数
    */
    OnCefSettingsEvent m_cefSettingCallback;

    /** CEF的日志级别
    */
    cef_log_severity_t m_logSeverity;

    /** 浏览器控件的计数器
    */
    std::atomic<int32_t> m_browserCount;

    /** 设置调用CEF消息处理函数定时器的时间间隔（毫秒）
    */
    int32_t m_nCefDoMessageLoopWorkDelayMs;

    /** 退出时的编码
    */
    int32_t m_nExitCode;

    /** 是否设置了CEF的网页缓存目录
    */
    bool m_bHasCefCachePath;

    /** 是否启用离屏渲染(初始化后，不允许修改)
    */
    bool m_bEnableOffScreenRendering;

    /** 是否已经初始化
    */
    bool m_bCefInit;

    /** CEF消息队列是否为空
    */
    bool m_bCefMessageLoopEmpty;

#ifdef DUILIB_BUILD_FOR_MACOS
    /** 开始退出的时间戳
    */
    std::chrono::steady_clock::time_point m_exitTime;

    /** 是否正在退出
    */
    bool m_bExiting;
#endif
};

} //namespace ui

#endif //UI_CEF_CONTROL_CEF_MANAGER_H_
