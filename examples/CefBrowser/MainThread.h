#ifndef EXAMPLES_MAIN_THREAD_H_
#define EXAMPLES_MAIN_THREAD_H_

// duilib
#include "duilib/duilib.h"

/** @class MainThread
* @brief 主线程（UI线程）类
* @copyright (c) 2015, NetEase Inc. All rights reserved
* @author towik
* @date 2015/1/1
*/
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;

    //开启DPI感知功能设置参数
    const ui::DpiInitParam& GetDpiInitParam() const;

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

    /** 控制主进程单例的回调函数
    */
    void OnAlreadyRunningAppRelaunch(const std::vector<DString>& argumentList);

private:
    //开启DPI感知功能设置参数
    ui::DpiInitParam m_dpiInitParam;
};
#endif //EXAMPLES_MAIN_THREAD_H_
