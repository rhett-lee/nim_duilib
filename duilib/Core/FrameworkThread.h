#ifndef UI_CORE_FRAMEWORK_THREAD_H_
#define UI_CORE_FRAMEWORK_THREAD_H_

#include "duilib/Core/ThreadMessage.h"
#include "duilib/Core/Callback.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <map>

namespace ui 
{
/** 线程标识ID(此为预定义，也可以自定义，以满足项目需要)
*/
enum ThreadIdentifier
{
    kThreadNone     = -1,   //无线程标识ID
    kThreadUI       = 0,    //UI线程
    kThreadWorker   = 1,    //工作线程(duilib库内部用该线程加载多帧图片)
    kThreadMisc     = 2,    //杂事线程
    kThreadUser     = 3     //用户自定义线程的起始ID，如果创建更多线程时，可在此ID后递增ID
};

/** 框架线程
*/
class UILIB_API FrameworkThread : public virtual SupportWeakCallback
{
public:
    /** 使用线程名称和线程识别ID构造线程对象
    * @param [in] threadName 线程名称
    * @param [in] nThreadIdentifier 线程标识ID，跨线程通信时需要用到此值
    */
    FrameworkThread(const DString& threadName, int32_t nThreadIdentifier);
    virtual ~FrameworkThread() override;
    FrameworkThread(const FrameworkThread&) = delete;
    FrameworkThread& operator = (const FrameworkThread&) = delete;

public:
    /** 执行UI主线程的消息循环（阻塞直到消息循环退出）
    */
    bool RunMessageLoop();

    /** 启动子线程
    */
    bool Start();

    /** 停止子线程（异步停止）
    */
    bool Stop();

    /** 是否正在运行中
    */
    bool IsRunning() const;

    /** 是否为UI线程(即使用RunMessageLoop函数运行的线程, 内部有消息循环)
    */
    bool IsUIThread() const;

public:
    /** 获取当前线程的线程ID(与操作系统的线程ID意义相同)
    */
    std::thread::id GetThreadId() const;

    /** 将线程ID转换为字符串
    */
    static DString ThreadIdToString(const std::thread::id& threadId);

    /** 获取当前线程标识符，线程唯一标识，用于线程间通信使用(线程构造时初始化的值)
    */
    int32_t GetThreadIdentifier() const;

    /** 返回线程名称(线程构造时初始化的值)
    */
    const DString& GetThreadName() const;

public:
    /** 向线程发送一个任务，立即执行
    * @param [in] task 任务回调函数
    * @param [in] unlockClosure 用于释放外层锁的函数（用于避免死锁）
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostTask(const StdClosure& task, const StdClosure& unlockClosure = nullptr);

    /** 向线程发送一个任务，延迟执行
    * @param [in] task 任务回调函数
    * @param [in] nDelayMs 延迟的时间（单位：毫秒）
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostDelayedTask(const StdClosure& task, int32_t nDelayMs);

    /** 向线程发送一个任务，可定时重复执行
    * @param [in] task 任务回调函数
    * @param [in] nIntervalMs 间隔的时间（单位：毫秒）
    * @param [in] nTimes 重复的次数，如果为-1表示一直执行
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostRepeatedTask(const StdClosure& task, int32_t nIntervalMs, int32_t nTimes = -1);

    /** 取消一个任务
    * @param [in] nTaskId 任务ID，即上面的PostXXX函数的返回值
    * @return 取消成功返回true，否则返回false
    */
    bool CancelTask(size_t nTaskId);

protected:
    /** 运行前初始化，在进入消息循环前调用
    */
    virtual void OnInit();

    /** 运行消息循环，子类可以重写该函数，自己实现消息循环
    */
    virtual void OnRunMessageLoop();

    /** 退出时清理，在退出消息循环后调用
    */
    virtual void OnCleanup();

private:
    /** 主线程完成初始化
    */
    void OnMainThreadInited();

    /** 主线程消息循环退出
    */
    void OnMainThreadExit();

    /** 后台线程的线程函数
    */
    void WorkerThreadProc();

    /** 通知执行一个任务
    */
    bool NotifyExecTask(size_t nTaskId,
                        const StdClosure& unlockClosure1 = nullptr,
                        const StdClosure& unlockClosure2 = nullptr);

    /** 执行任务
    */
    void ExecTask(size_t nTaskId);

    /** 消息函数
    */
    void OnTaskMessage(uint32_t msgId, WPARAM wParam, LPARAM lParam);

    /** 获取下一个任务ID
    */
    size_t GetNextTaskId() const;

private:
    /** 任务类型
    */
    enum class TaskType
    {
        kTask,          //普通任务，立即执行
        kDelayedTask,   //延迟执行的任务
        kRepeatedTask   //按一定间隔，重复执行的任务
    };

    /** 任务信息
    */
    struct TaskInfo
    {
        TaskType m_taskType = TaskType::kTask;  //任务类型
        StdClosure m_task;                      //任务回调函数
        int32_t m_nIntervalMs = 0;              //任务执行的事件间隔
        int32_t m_nTimes = 0;                   //任务重复执行的次数，如果为-1表示一直执行

        size_t m_nTaskId = 0;                   //任务ID（递增）
        std::chrono::steady_clock::time_point m_startTime;     //开始时间（任务放入队列的时间）
        std::chrono::steady_clock::time_point m_lastExecTime;  //任务上次执行的时间
        int32_t m_nTotalExecTimes = 0;          //任务总计执行的次数
    };

    /** 任务信息映射表
    */
    typedef std::map<size_t, TaskInfo> TaskMap;
    TaskMap m_taskMap;

    /** 任务数据多线程同步锁
    */
    mutable std::mutex m_taskMutex;

private:
    /** 线程名称
    */
    DString m_threadName;

    /** 线程标识ID，跨线程通信时需要用到此值
    */
    int32_t m_nThreadIdentifier;

    /** 当前线程的线程ID
    */
    std::atomic<std::thread::id> m_nThisThreadId;

    /** 是否为UI线程
    */
    bool m_bThreadUI;

    /** 是否正在运行中
    */
    volatile bool m_bRunning;

private:
    /** 后台线程
    */
    std::unique_ptr<std::thread> m_pWorkerThread;

    /** 线程的事件通知机制
    */
    std::condition_variable m_cv;

    /** 等待执行的任务ID
    */
    std::vector<size_t> m_penddingTaskIds;

    /** 等待任务ID容器锁
    */
    std::mutex m_penddingTaskMutex;

    /** 与主线程通信的机制
    */
    ThreadMessage m_threadMsg;
};

}
#endif //UI_CORE_FRAMEWORK_THREAD_H_
