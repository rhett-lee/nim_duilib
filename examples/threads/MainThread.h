#ifndef EXAMPLES_MAIN_THREAD_H_
#define EXAMPLES_MAIN_THREAD_H_

// duilib
#include "duilib/duilib.h"

class WorkerThread;
class MainForm;

/** 主线程(UI线程)
*/
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;

public:
    /** 启动子线程
    */
    void StartThreads();

    /** 停止子线程
    */
    void StopThreads();

    /** 获取线程池中的工作线程个数
    */
    int32_t GetPoolThreadCount() const;

    /** 输出日志，可以在任何线程中调用，最终在主线程中执行
    * @param [in] log 日志内容
    */
    void PrintLog(const DString& log);

private:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

private:
    /** 主界面的接口
    */
    MainForm* m_pMainForm;

    /** 主界面的接口的生命周期
    */
    std::weak_ptr<ui::WeakFlag> m_mainFormFlag;

    /** 工作线程: 线程标识符为 ui::kThreadWorker
    */
    std::unique_ptr<WorkerThread> m_workerThread;

    /** 杂事线程: 线程标识符为 ui::kThreadMisc
    */
    std::unique_ptr<WorkerThread> m_miscThread;

    /** 用户自定义线程池: 线程标识符为 ui::kThreadUser + vector的元素索引号
    */
    std::vector<std::shared_ptr<WorkerThread>> m_threadPools;
};

#endif // EXAMPLES_MAIN_THREAD_H_
