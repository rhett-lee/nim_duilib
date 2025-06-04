#ifndef UI_CEF_CONTROL_CEF_MANAGER_WINDOWS_H_
#define UI_CEF_CONTROL_CEF_MANAGER_WINDOWS_H_

#include "duilib/CEFControl/CefManager.h"
#include <memory>

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
//进程单例控制（CEF109）
class ProcessSingleton;

/** CEF组件管理器（Windows实现）
 */
class CefManager_Windows : public CefManager
{
    friend class CefManager;
    typedef CefManager BaseClass;

protected:
    CefManager_Windows();
    CefManager_Windows(const CefManager_Windows&) = delete;
    CefManager_Windows& operator=(const CefManager_Windows&) = delete;
protected:
    virtual ~CefManager_Windows() override;

public:
    /** 获取CEF模块运行库文件所在路径
    */
    virtual DString GetCefMoudlePath() const override;

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
                            OnCefSettingsEvent callback) override;

    /** 清理cef组件
    */
    virtual void UnInitialize() override;

public:
    /** 绑定一个回调函数用于监听Browser进程启动事件（仅在Windows + CEF109使用，其他情况不需要设置）
    * @param [in] callback 一个回调函数，参考 OnAlreadyRunningAppRelaunchEvent 声明
    */
    virtual void SetAlreadyRunningAppRelaunch(const OnAlreadyRunningAppRelaunchEvent& callback) override;

    /** 获取监听Browser进程启动事件的回调函数
    */
    virtual OnAlreadyRunningAppRelaunchEvent GetAlreadyRunningAppRelaunch() const override;

private:
    /** 把cef dll文件的位置添加到程序的"path"环境变量中, 这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件
    */
    void AddCefDllToPath();

#if CEF_VERSION_MAJOR <= 109
    /** 浏览器单例控制回调函数
    */
    static void OnBrowserAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList);
#endif

private:
    /** 另外一个Browser进程启动时的回调函数
    */
    OnAlreadyRunningAppRelaunchEvent m_pfnAlreadyRunningAppRelaunch;

    //进程单例控制（CEF 109）
    std::unique_ptr<ProcessSingleton> m_pProcessSingleton;
};

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_WIN)

#endif //UI_CEF_CONTROL_CEF_MANAGER_WINDOWS_H_
