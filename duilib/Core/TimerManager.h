#ifndef UI_CORE_TIMER_MANAGER_H_
#define UI_CORE_TIMER_MANAGER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/ThreadMessage.h"
#include <queue>
#include <set>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ui 
{

/** 定时器回调函数原型：void FunctionName();
*/
typedef std::function<void()> TimerCallback;
class TimerInfo;

/** 定时器管理器
*/
class TimerManager: public SupportWeakCallback
{
public:
    TimerManager();
    virtual ~TimerManager() override;
    TimerManager(const TimerManager&) = delete;
    TimerManager& operator = (const TimerManager&) = delete;

public:
    /** 初始化
    * @param [in] platformData 平台相关数据（可选参数，如不填写则使用默认值：nullptr）
    * Windows平台：是资源所在模块句柄（HMODULE），如果为nullptr，则使用所在exe的句柄（可选参数）
    */
    void Initialize(void* platformData);

    /** 添加一个可取消的定时器
    * @param [in] weakFlag 定时器取消机制，如果weakFlag.expired()为true表示定时器已经取消，不会在继续派发定时器回调
    * @param [in] callback 定时器回调函数
    * @param [in] uElapseMs 定时器触发时间间隔，单位为毫秒
    * @param [in] iRepeatTime 定时器回调次数限制，如果为 -1 表示不停重复回调
    * @param [in] 成功返回定时器ID（其值大于0），失败则返回0
    */
    size_t AddTimer(const std::weak_ptr<WeakFlag>& weakFlag,
                    const TimerCallback& callback,
                    uint32_t uElapseMs,
                    int32_t iRepeatTime = -1);

    /** 删除一个定时器任务
    * @param [in] nTimerId 定时器任务ID，即AddTimer的返回值
    */
    void RemoveTimer(size_t nTimerId);

    /** 关闭定时器管理器，释放资源
     */
    void Clear();

private:
    /** 后台线程的线程函数
    */
    void WorkerThreadProc();

    /** 定时器触发，进行定时器事件回调派发
    */
    void Poll();

    /** 是否定时器任务已经被取消
    */
    bool IsTimerRemoved(size_t nTimerId) const;

    /** 移除已经取消的定时器任务ID
    */
    void ClearRemovedTimerId(size_t nTimerId);

private:
    /** 消息窗口函数
    */
    void OnTimerMessage(uint32_t msgId, WPARAM wParam, LPARAM lParam);

private:
    /** 所有注册的定时器
    */
    std::priority_queue<TimerInfo> m_aTimers;

    /** 下一个定时器任务ID
    */
    size_t m_nNextTimerId;

    /** 移除的定时器任务ID
    */
    std::set<size_t> m_removedTimerIds;

private:
    /** 是否正在运行中
    */
    volatile bool m_bRunning;

    /** 是否正在等待主线程处理定时器的回调事件
    */
    volatile bool m_bHasPenddingPoll;

    /** 后台线程
    */
    std::unique_ptr<std::thread> m_pWorkerThread;

    /** 线程的事件通知机制
    */
    std::condition_variable m_cv;

    /** 任务数据容器锁
    */
    std::mutex m_taskMutex;

    /** 线程间通信机制（与主线程）
    */
    ThreadMessage m_threadMsg;
};

} // namespace ui

#endif // UI_CORE_TIMER_MANAGER_H_
