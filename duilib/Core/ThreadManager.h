#ifndef UI_CORE_THREAD_MANAGER_H_
#define UI_CORE_THREAD_MANAGER_H_

#include "duilib/Core/FrameworkThread.h"
#include "duilib/Core/ControlPtrT.h"
#include <map>

namespace ui 
{
/** 线程管理器，用以支持线程间通信
*/
class UILIB_API ThreadManager
{
public:
    ThreadManager();
    ~ThreadManager();
    ThreadManager(const ThreadManager&) = delete;
    ThreadManager& operator = (const ThreadManager&) = delete;

public:
    /** 注册一个线程到管理器（只有注册后才能用于线程间通信）
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] pThread 线程的接口
    */
    bool RegisterThread(int32_t nThreadIdentifier, FrameworkThread* pThread);

    /** 判断是否包含指定标识符的线程
    * @param [in] nThreadIdentifier 线程标识ID
    */
    bool HasThread(int32_t nThreadIdentifier) const;

    /** 从管理器中取消注册一个线程
    * @param [in] nThreadIdentifier 线程标识ID
    */
    bool UnregisterThread(int32_t nThreadIdentifier);

    /** 获取当前线程的线程标识ID
    * @return 成功返回线程标识ID，失败则返回kThreadNone(值为-1)
    */
    int32_t GetCurrentThreadIdentifier() const;

public:
    /** 向线程发送一个任务，立即执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostTask(int32_t nThreadIdentifier, const StdClosure& task);

    /** 向线程发送一个任务，延迟执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    * @param [in] nDelayMs 延迟的时间（单位：毫秒）
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostDelayedTask(int32_t nThreadIdentifier, const StdClosure& task, int32_t nDelayMs);

    /** 向线程发送一个任务，可定时重复执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    * @param [in] nIntervalMs 间隔的时间（单位：毫秒）
    * @param [in] nTimes 重复的次数，如果为-1表示一直执行
    * @return 成功返回任务ID(大于0)，如果失败则返回0
    */
    size_t PostRepeatedTask(int32_t nThreadIdentifier, const StdClosure& task,
                            int32_t nIntervalMs, int32_t nTimes = -1);

    /** 取消一个任务
    * @param [in] nTaskId 任务ID，即上面的PostXXX函数的返回值
    */
    bool CancelTask(size_t nTaskId);

    /** 获取下一个任务ID (线程安全)
    * @return 返回下一个任务ID，任务ID编号从1开始
    */
    size_t GetNextTaskId();

public:
    /** 关闭线程管理器，释放资源
    */
    void Clear();

public:
    /** 主线程是否已经退出
    */
    bool IsMainThreadExit() const;

    /** 当主线程退出时，调用该函数设置主线程退出标志
    */
    void SetMainThreadExit();

private:
    /** 线程对象的智能指针
    */
    typedef ControlPtrT<FrameworkThread> FrameworkThreadPtr;

    /** 线程信息映射表
    */
    std::map<int32_t, FrameworkThreadPtr> m_threadsMap;

    /** 多线程同步锁
    */
    mutable std::mutex m_threadMutex;

    /** 下一个任务ID
    */
    std::atomic<size_t> m_nNextTaskId;

    /** 主线程是否已经退出
    */
    std::atomic<bool> m_bMainThreadExit;
};

}
#endif //UI_CORE_THREAD_MANAGER_H_
