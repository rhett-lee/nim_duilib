#ifndef UI_CEF_CONTROL_CEF_MANAGER_H_
#define UI_CEF_CONTROL_CEF_MANAGER_H_

#include "duilib/Core/Callback.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_app.h"
    #include "include/cef_version.h"
#pragma warning (pop)

#include <vector>

namespace ui
{
    /** Browser进程单例控制的回调函数
     * (1) 由于一个Browser进程，需要启动好多个Render子进程，资源开销比较大，所以有必要控制保证只有一个Browser进程)
     * (2) CEF 133版本及以上版本，自身有单例控制功能，只要实现接口就可以了；但CEF 109版本无此功能，允许同时存在多个Browser进程
     * @param [in] argumentList 重新启动的Browser进程的启动参数列表
     */
    typedef std::function<void (const std::vector<DString>& argumentList)> OnAlreadyRunningAppRelaunchEvent;

    //进程单例控制（CEF109）
    class ProcessSingleton;

    //窗口类型
    class Window;

/** 管理Cef组件的初始化、销毁、消息循环
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
class CefManager : public virtual ui::SupportWeakCallback
{
public:
    CefManager();
    CefManager(const CefManager&) = delete;
    CefManager& operator=(const CefManager&) = delete;
private:
    ~CefManager();
public:
    /** 单例对象
    */
    static CefManager* GetInstance();

public:
    /** 把cef dll文件的位置添加到程序的"path"环境变量中,这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件
    */
    void AddCefDllToPath();

    /** 初始化cef组件
    * @param [in] app_data_dir 应用路径名称
    * @param [in] settings Cef全部配置
    * @param [in] bEnableOffScreenRendering 是否开启离屏渲染
    * @param [in] appName 产品名称标识符（保证唯一性，用于控制Browser进程单例）
    * @param [in] argc 程序启动时的参数个数(仅Linux环境使用)
    * @param [in] argv 程序启动时的参数列表(仅Linux环境使用)
    * @return bool true 继续运行，false 应该结束程序
    */
#ifdef DUILIB_BUILD_FOR_WIN
    bool Initialize(const DString& app_data_dir,
                    CefSettings& settings,
                    bool bEnableOffScreenRendering,
                    const DString& appName);
#else
    bool Initialize(const DString& app_data_dir,
                    CefSettings& settings,
                    bool bEnableOffScreenRendering,
                    int argc,
                    char** argv);
#endif

    /** 绑定一个回调函数用于监听Browser进程启动事件
    * @param [in] callback 一个回调函数，参考 OnAlreadyRunningAppRelaunchEvent 声明
    */
    void SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& callback);

    /** 获取监听Browser进程启动事件的回调函数
    */
    OnAlreadyRunningAppRelaunchEvent GetAlreadyRunningAppRelaunch() const;

    /** 清理cef组件
    */
    void UnInitialize();

    /** 是否开启离屏渲染
    * @return bool true 开启，false 不开启
    */
    bool IsEnableOffScreenRendering() const;

    // 记录浏览器对象的数量
    void AddBrowserCount();
    void SubBrowserCount();
    int32_t GetBrowserCount();

    /** 窗口关闭时，处理该窗口下的所有Browser控件，在窗口关闭前退出
    */
    void ProcessWindowCloseEvent(Window* pWindow);

    /** 在Cef浏览器对象销毁后发送 WM_QUIT 消息，退出主进程的消息循环
    */
    void PostQuitMessage(int32_t nExitCode);

private:
    /** 设置cef配置信息
    * @param [in] app_data_dir 应用路径名称(UTF16或者UTF8编码)
    * @param [out] settings cef配置类
    */
    void GetCefSetting(const DString& app_data_dir, CefSettings& settings);

#if CEF_VERSION_MAJOR <= 109
    /** 浏览器单例控制回调函数
    */
    static void OnBrowserAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList);
#endif

private:
    //浏览器控件的计数器
    int32_t m_browserCount;

    //是否启用离屏渲染
    bool m_bEnableOffScreenRendering;

    //另外一个Browser进程启动时的回调函数
    OnAlreadyRunningAppRelaunchEvent m_pfnAlreadyRunningAppRelaunch;

#ifdef DUILIB_BUILD_FOR_WIN
    //进程单例控制（CEF 109）
    std::unique_ptr<ProcessSingleton> m_pProcessSingleton;
#endif
};
}

#endif //UI_CEF_CONTROL_CEF_MANAGER_H_
