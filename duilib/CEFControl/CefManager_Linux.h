#ifndef UI_CEF_CONTROL_CEF_MANAGER_LINUX_H_
#define UI_CEF_CONTROL_CEF_MANAGER_LINUX_H_

#include "duilib/CEFControl/CefManager.h"

#ifdef DUILIB_BUILD_FOR_LINUX

namespace ui
{
/** CEF组件管理器（Linux实现）
 */
class CefManager_Linux : public CefManager
{
    friend class CefManager;
    typedef CefManager BaseClass;

protected:
    CefManager_Linux();
    CefManager_Linux(const CefManager_Linux&) = delete;
    CefManager_Linux& operator=(const CefManager_Linux&) = delete;
protected:
    virtual ~CefManager_Linux() override;

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

};

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX)

#endif //UI_CEF_CONTROL_CEF_MANAGER_LINUX_H_
