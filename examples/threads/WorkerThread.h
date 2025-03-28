//WorkerThread.h
#ifndef EXAMPLES_WORKDER_THREAD_H_
#define EXAMPLES_WORKDER_THREAD_H_

// duilib
#include "duilib/duilib.h"

class MainForm;

/** 工作线程
*/
class WorkerThread : public ui::FrameworkThread
{
public:
    /** 线程构造函数
    @param [in] nThreadIdentifier 线程标识符，线程间通信时需要使用该线程标识符，所以需要保证唯一
    */
    explicit WorkerThread(int32_t nThreadIdentifier);
    virtual ~WorkerThread() override;

public:
    /** 设置主界面的接口，用于与主界面交互
    */
    void SetMainForm(MainForm* pMainForm);

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

    /** 输出日志
    */
    void PrintLog(const DString& log);

private:
    /** 主界面的接口
    */
    ui::ControlPtr<MainForm> m_pMainForm;

};

#endif // EXAMPLES_WORKDER_THREAD_H_
