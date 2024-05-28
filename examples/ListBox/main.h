#pragma once

#include "resource.h"

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
    virtual ~MainThread();

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;
};
