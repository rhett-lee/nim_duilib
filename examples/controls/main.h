#ifndef EXAMPLES_MAIN_H_
#define EXAMPLES_MAIN_H_

// duilib
#include "duilib/duilib.h"

#include "resource.h"

/** @class 杂事线程
* @brief 杂事线程类
* @copyright (c) 2015, NetEase Inc. All rights reserved
* @date 2015/1/1
*/
class MiscThread : public ui::FrameworkThread
{
public:
    MiscThread();
    virtual ~MiscThread();

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;
};

/** @class MainThread
* @brief 主线程（UI线程）类
* @copyright (c) 2015, NetEase Inc. All rights reserved
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

private:
    /** 专门处理杂事的线程
    */
    std::unique_ptr<MiscThread> m_misc_thread;
};

#endif //EXAMPLES_MAIN_H_
