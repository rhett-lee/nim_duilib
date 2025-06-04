#ifndef UI_CEF_CONTROL_CEF_MANAGER_MACOS_H_
#define UI_CEF_CONTROL_CEF_MANAGER_MACOS_H_

#include "duilib/CEFControl/CefManager.h"

#ifdef DUILIB_BUILD_FOR_MACOS

namespace ui
{
/** CEF组件管理器（MacOS实现）
 */
class CefManager_MacOS : public CefManager
{
    friend class CefManager;
    typedef CefManager BaseClass;

protected:
    CefManager_MacOS();
    CefManager_MacOS(const CefManager_MacOS&) = delete;
    CefManager_MacOS& operator=(const CefManager_MacOS&) = delete;
protected:
    virtual ~CefManager_MacOS() override;

public:
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

    /** 当前CEF是否运行在多线程消息循环状态(Windows/Linux平台支持，但MacOS不支持)
    */
    virtual bool IsMultiThreadedMessageLoop() const override;

};

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_MACOS)

#endif //UI_CEF_CONTROL_CEF_MANAGER_MACOS_H_
