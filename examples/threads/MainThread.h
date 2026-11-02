#ifndef EXAMPLES_MAIN_THREAD_H_
#define EXAMPLES_MAIN_THREAD_H_

// duilib
#include "duilib/duilib.h"
#include <atomic>
#include <thread>
#include <vector>

class WorkerThread;
class MainForm;

/** 主线程(UI线程)
*/
class MainThread : public ui::FrameworkThread
{
public:
    /** 线程池中允许的最大线程个数
    */
    static const int32_t kMaxPoolThreadCount = 8;

    /** 线程池的默认线程个数
    */
    static const int32_t kDefaultPoolThreadCount = 4;

    /** 线程状态信息，用于在界面上展示
    */
    struct ThreadInfo
    {
        /** 线程标识符(线程间通信使用)
        */
        int32_t nThreadIdentifier = 0;

        /** 线程名称
        */
        DString sThreadName;

        /** 操作系统线程ID
        */
        DString sSystemThreadId;

        /** 线程是否在运行中
        */
        bool bRunning = false;

        /** 该线程已执行的任务次数
        */
        int32_t nTaskCount = 0;
    };

    /** 启动子线程的结果
    */
    enum class StartResult
    {
        /** 已按指定规模启动（新建或重启） */
        kStarted,

        /** 线程池已经是该规模，未做任何改动 */
        kAlreadyRunning
    };

public:
    MainThread();
    virtual ~MainThread() override;

public:
    /** 启动子线程
    * @param [in] nThreadCount 线程池中的线程个数，取值范围是 [1, kMaxPoolThreadCount]
    * @return 返回启动结果：如果线程池已经是该规模，则不做任何改动
    */
    StartResult StartThreads(int32_t nThreadCount);

    /** 停止子线程
    * @param [in] bLogLifecycle true表示各个线程输出OnCleanup日志；
    *                            false表示静默停止（重启线程池时使用，避免新旧线程的日志交错）
    * @param [in] bAsync true表示异步停止：界面线程立即返回，等后台线程真正退出后再回调界面；
    *                    false表示同步停止（重建线程池时必须同步，否则新旧线程会使用相同的标识符）
    */
    void StopThreads(bool bLogLifecycle = true, bool bAsync = false);

    /** 是否正在异步停止线程池的过程中
    */
    bool IsStopping() const;

    /** 获取线程池中的工作线程个数
    */
    int32_t GetPoolThreadCount() const;

    /** 获取线程池中各个线程的状态信息
    * @param [out] threadInfo 返回线程状态信息，按照线程标识符升序排列
    */
    void GetThreadInfo(std::vector<ThreadInfo>& threadInfo) const;

    /** 记录某个线程执行了一个任务（由任务函数在子线程中调用）
    * @param [in] nThreadIdentifier 线程标识符
    */
    void OnThreadTaskExecuted(int32_t nThreadIdentifier);

    /** 输出日志，可以在任何线程中调用，最终在主线程中执行
    * @param [in] log 日志内容
    */
    void PrintLog(const DString& log);

private:
    /** 运行前初始化，在进入消息循环前调用，如果初始化失败则不进入消息循环
    * @return 初始化成功返回true，初始化失败返回false
    */
    virtual bool OnInit() override;

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup() override;

    /** 异步停止完成后的回调（在界面线程中执行）
    */
    void OnAsyncStopFinished();

private:
    /** 主界面的接口
    */
    ui::ControlPtrT<MainForm> m_pMainForm;

    /** 用户自定义线程池: 线程标识符为 ui::kThreadUser + vector的元素索引号
    */
    std::vector<std::shared_ptr<WorkerThread>> m_threadPools;

    /** 异步停止时使用的辅助线程：在工作线程上执行join，避免阻塞界面线程
    */
    std::thread m_stopThread;

    /** 是否正在异步停止线程池的过程中
    */
    std::atomic<bool> m_bStopping;

    /** 主线程是否正在退出（退出过程中不再向界面线程投递回调）
    */
    std::atomic<bool> m_bExiting;
};

#endif // EXAMPLES_MAIN_THREAD_H_
