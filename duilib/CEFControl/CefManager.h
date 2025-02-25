/**@brief 管理Cef组件的初始化、销毁、消息循环
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */
#ifndef UI_CEF_CONTROL_CEF_MANAGER_H_
#define UI_CEF_CONTROL_CEF_MANAGER_H_

#include "duilib/Core/Callback.h"
#include "duilib/CEFControl/internal/handler/drag/osr_dragdrop_win.h" //TODO: 待改进，不开放该头文件

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_app.h"
#pragma warning (pop)

namespace ui
{
/** @class CefManager
 * @brief Cef组件初始化和销毁
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
    /**
    * 把cef dll文件的位置添加到程序的"path"环境变量中,这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件
    * @return void    无返回值
    */
    void AddCefDllToPath();

    /**
    * 初始化cef组件
    * @param[in] app_data_dir 应用路径名称
    * @param[in] settings Cef全部配置
    * @param[in] is_enable_offset_render 是否开启离屏渲染
    * @return bool true 继续运行，false 应该结束程序
    */
    bool Initialize(const DString& app_data_dir, CefSettings &settings, bool is_enable_offset_render = true);

    /**
    * 清理cef组件
    * @return void    无返回值
    */
    void UnInitialize();

    /**
    * 是否开启离屏渲染
    * @return bool true 开启，false 不开启
    */
    bool IsEnableOffsetRender() const;

    // 记录浏览器对象的数量
    void AddBrowserCount();
    void SubBrowserCount();
    int    GetBrowserCount();

    // 在Cef浏览器对象销毁后发送WM_QUIT消息
    void PostQuitMessage(int nExitCode);

    // 获取某个窗口对应的DropTarget，用于浏览器控件的拖动功能
    client::DropTargetHandle GetDropTarget(HWND hwnd);
private:
    /**
    * 设置cef配置信息
    * @param[in] app_data_dir 应用路径名称(UTF16或者UTF8编码)，见nim::Client::Init的说明
    * @param[out] settings cef配置类
    * @return void    无返回值
    */
    void GetCefSetting(const DString& app_data_dir, CefSettings &settings);

private:
    int browser_count_;
    bool is_enable_offset_render_;

    std::map<HWND, std::weak_ptr<client::DropTargetWin>> map_drag_target_reference_; // 各个DropTarget的弱引用，一个窗口对应一个DropTarget，这个DropTarget可以给多个BorwserHandler使用
};
}

#endif //UI_CEF_CONTROL_CEF_MANAGER_H_
