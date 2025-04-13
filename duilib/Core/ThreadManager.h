#ifndef UI_CORE_THREAD_MANAGER_H_
#define UI_CORE_THREAD_MANAGER_H_

#include "duilib/Core/FrameworkThread.h"
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

    /** 向线程发送一个任务，立即执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    */
    bool PostTask(int32_t nThreadIdentifier, const StdClosure& task);

    /** 向线程发送一个任务，延迟执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    * @param [in] nDelayMs 延迟的时间（单位：毫秒）
    */
    bool PostDelayedTask(int32_t nThreadIdentifier, const StdClosure& task, int32_t nDelayMs);

    /** 向线程发送一个任务，可定时重复执行
    * @param [in] nThreadIdentifier 线程标识ID
    * @param [in] task 任务回调函数
    * @param [in] nIntervalMs 间隔的时间（单位：毫秒）
    * @param [in] nTimes 重复的次数，如果为-1表示一直执行
    */
    bool PostRepeatedTask(int32_t nThreadIdentifier, const StdClosure& task,
                          int32_t nIntervalMs, int32_t nTimes = -1);

    /** 关闭线程管理器，释放资源
    */
    void Clear();

private:
    /** 线程的信息
    */
    struct ThreadInfo
    {
        //线程接口
        FrameworkThread* m_pThread = nullptr;

        //线程的WeakFlag
        std::weak_ptr<WeakFlag> m_threadFlag;
    };

private:
    /** 获取线程信息
    */
    bool GetThreadInfo(int32_t nThreadIdentifier, ThreadInfo& threadInfo) const;

    /** 线程信息映射表
    */
    std::map<int32_t, ThreadInfo> m_threadsMap;

    /** 多线程同步锁
    */
    mutable std::mutex m_threadMutex;
};

}
#endif //UI_CORE_THREAD_MANAGER_H_
