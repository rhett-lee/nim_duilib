#ifndef UI_CEF_CONTROL_CEF_MANAGER_H_
#define UI_CEF_CONTROL_CEF_MANAGER_H_

#include "duilib/Core/Callback.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_app.h"
    #include "include/cef_version.h"
#pragma warning (pop)

namespace ui
{
/**@brief 管理Cef组件的初始化、销毁、消息循环
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

    /** 单例对象
    */
    static CefManager* GetInstance();

private:
    ~CefManager();
public:
    /** 把cef dll文件的位置添加到程序的"path"环境变量中,这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件
    */
    void AddCefDllToPath();

    /** 初始化cef组件
    * @param [in] app_data_dir 应用路径名称
    * @param [in] settings Cef全部配置
    * @param [in] bEnableOffScreenRendering 是否开启离屏渲染
    * @param [in] argc 程序启动时的参数个数(仅Linux环境使用)
    * @param [in] argv 程序启动时的参数列表(仅Linux环境使用)
    * @return bool true 继续运行，false 应该结束程序
    */
#ifdef DUILIB_BUILD_FOR_WIN
    bool Initialize(const DString& app_data_dir, CefSettings& settings, bool bEnableOffScreenRendering = true);
#else
    bool Initialize(const DString& app_data_dir,
                    CefSettings& settings,
                    bool bEnableOffScreenRendering,
                    int argc,
                    char** argv);
#endif

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

    // 在Cef浏览器对象销毁后发送WM_QUIT消息
    void PostQuitMessage(int32_t nExitCode);

private:
    /** 设置cef配置信息
    * @param [in] app_data_dir 应用路径名称(UTF16或者UTF8编码)
    * @param [out] settings cef配置类
    */
    void GetCefSetting(const DString& app_data_dir, CefSettings& settings);

private:
    //浏览器控件的计数器
    int32_t m_browserCount;

    //是否启用离屏渲染
    bool m_bEnableOffScreenRendering;
};
}

#endif //UI_CEF_CONTROL_CEF_MANAGER_H_
