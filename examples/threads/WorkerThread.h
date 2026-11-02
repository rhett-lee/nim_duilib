//WorkerThread.h
#ifndef EXAMPLES_WORKDER_THREAD_H_
#define EXAMPLES_WORKDER_THREAD_H_

// duilib
#include "duilib/duilib.h"
#include <atomic>

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

    /** 设置是否输出线程生命周期的日志（OnInit/OnCleanup）
    * @param [in] bLog true表示输出，false表示静默（用于重启线程池时避免日志交错）
    */
    void SetLogLifecycle(bool bLog);

    /** 累加已执行的任务次数（由任务函数在本线程中调用）
    */
    void IncreaseTaskCount();

    /** 获取已执行的任务次数
    */
    int32_t GetTaskCount() const;

    /** 生成线程名称：WorkerThread-1、WorkerThread-2 …（按线程池中的序号编号，便于在界面上区分）
    * @param [in] nThreadIdentifier 线程标识符
    */
    static DString MakeThreadName(int32_t nThreadIdentifier);

private:
    /** 运行前初始化，在进入消息循环前调用，如果初始化失败则不进入消息循环
    * @return 初始化成功返回true，初始化失败返回false
    */
    virtual bool OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

    /** 输出日志
    */
    void PrintLog(const DString& log);

private:
    /** 主界面的接口
    */
    ui::ControlPtrT<MainForm> m_pMainForm;

    /** 线程标识符（缓存构造时的值：Stop() 会先把标识符置为 kThreadNone，
    *   清理阶段的日志仍然需要使用原始标识符）
    */
    const int32_t m_nLocalThreadIdentifier;

    /** 是否输出线程生命周期的日志
    */
    std::atomic<bool> m_bLogLifecycle;

    /** 已执行的任务次数（在子线程中累加，在界面线程中读取）
    */
    std::atomic<int32_t> m_nTaskCount;
};

#endif // EXAMPLES_WORKDER_THREAD_H_
