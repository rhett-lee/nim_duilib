#include "TimerManager.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/LogUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/WindowMessage.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include <SDL3/SDL.h>
#endif

/** 自定义消息
*/
#if defined (DUILIB_BUILD_FOR_SDL)
    #define WM_USER_DEFINED_TIMER   (SDL_EVENT_USER + 2)
#else
    #define WM_USER_DEFINED_TIMER   (kWM_USER + 567)
#endif

namespace ui 
{

/** 定时器的数据
*/
class TimerInfo
{
public:
    TimerInfo(): 
        timerCallback(nullptr),
        uElapseMs(0),
        uRepeatTime(0),
        m_nTimerId(0)
    {
    }

    bool operator < (const TimerInfo& r) const {
        //排序条件：最先触发的排在前面
        return trigerTime > r.trigerTime;
    }

    //定时器ID
    size_t m_nTimerId;

    //定时器回调函数
    TimerCallback timerCallback;

    //取消定时器同步机制
    std::weak_ptr<WeakFlag> weakFlag;

    //定时器间隔：（单位：毫秒）
    uint32_t uElapseMs;

    //重复次数
    uint32_t uRepeatTime;

    //定时器的触发时间
    std::chrono::steady_clock::time_point trigerTime;
};

TimerManager::TimerManager():
    m_nNextTimerId(1),
    m_bRunning(false),
    m_bHasPenddingPoll(false)
{
}

TimerManager::~TimerManager()
{
    Clear();
}

void TimerManager::Initialize(void* platformData)
{
    m_threadMsg.Initialize(platformData);
    m_threadMsg.SetMessageCallback(WM_USER_DEFINED_TIMER, UiBind(&TimerManager::OnTimerMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void TimerManager::Clear()
{
    std::unique_lock<std::mutex> guard(m_taskMutex);
    m_threadMsg.Clear();
    while (!m_aTimers.empty()) {
        m_aTimers.pop();
    }
    m_removedTimerIds.clear();
    m_bRunning = false;
    if (m_pWorkerThread != nullptr) {
        m_cv.notify_one();
        guard.unlock();
        m_pWorkerThread->join();
        m_pWorkerThread = nullptr;
    }
}

size_t TimerManager::AddTimer(const std::weak_ptr<WeakFlag>& weakFlag, const TimerCallback& callback,
                              uint32_t uElapseMs, int32_t iRepeatTime)
{
    ASSERT((callback != nullptr) && (uElapseMs > 0) && (iRepeatTime != 0));
    if ((callback == nullptr) || (uElapseMs == 0) || (iRepeatTime == 0)) {
        return 0;
    }
    if (iRepeatTime < 0) {
        iRepeatTime = -1;
    }
    size_t nTimerId = m_nNextTimerId++;
    TimerInfo pTimer;
    pTimer.m_nTimerId = nTimerId;

    pTimer.timerCallback = callback;
    pTimer.uElapseMs = uElapseMs;
    pTimer.trigerTime = std::chrono::steady_clock::now();
    pTimer.trigerTime += std::chrono::milliseconds(uElapseMs); //计算出下次触发时间(当前时间 + 间隔的毫秒数)
    pTimer.uRepeatTime = static_cast<uint32_t>(iRepeatTime);
    pTimer.weakFlag = weakFlag;

    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    m_aTimers.push(pTimer);
    if (m_pWorkerThread == nullptr) {
        //启动线程
        m_bRunning = true;
        m_pWorkerThread = std::make_unique<std::thread>(&TimerManager::WorkerThreadProc, this);
    }
    ASSERT(m_bRunning);
    //唤醒工作线程，检查任务状态
    m_cv.notify_one();
    return nTimerId;
}

void TimerManager::RemoveTimer(size_t nTimerId)
{
    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    m_removedTimerIds.insert(nTimerId);
}

bool TimerManager::IsTimerRemoved(size_t nTimerId) const
{
    if (!m_removedTimerIds.empty()) {
        return m_removedTimerIds.find(nTimerId) != m_removedTimerIds.end();
    }
    else {
        return false;
    }
}

void TimerManager::ClearRemovedTimerId(size_t nTimerId)
{
    m_removedTimerIds.erase(nTimerId);
}

void TimerManager::OnTimerMessage(uint32_t msgId, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    ASSERT(msgId == WM_USER_DEFINED_TIMER);
    if (msgId == WM_USER_DEFINED_TIMER) {
        //LogUtil::OutputLine(StringUtil::Printf(_T("TimerManager::OnTimerMessage: received timer event")));
        m_threadMsg.RemoveDuplicateMsg(WM_USER_DEFINED_TIMER);
        Poll();
    }    
}

void TimerManager::Poll()
{
    //该函数在UI线程中调用
    std::unique_lock<std::mutex> taskGuard(m_taskMutex);
    while (!m_aTimers.empty()) {
        if (m_aTimers.top().weakFlag.expired() || IsTimerRemoved(m_aTimers.top().m_nTimerId)) {
            //删除已经失效或者取消的定时器
            ClearRemovedTimerId(m_aTimers.top().m_nTimerId);
            m_aTimers.pop();
        }
        else if (std::chrono::steady_clock::now() >= m_aTimers.top().trigerTime) {
            //队列顶的定时器：已经达到定时器触发条件
            TimerInfo timerTask = m_aTimers.top();
            m_aTimers.pop();

            if (!timerTask.weakFlag.expired()) {
                //调用定时器的回调函数
                taskGuard.unlock();
                timerTask.timerCallback();
                //LogUtil::OutputLine(StringUtil::Printf(_T("timerTask.timerCallback(): exec. TimerId: %u, ElapseMs: %u"), timerTask.m_nTimerId, timerTask.uElapseMs));
                taskGuard.lock();
            }
            if (timerTask.uRepeatTime > 0) {
                timerTask.uRepeatTime--;
            }
            if ((timerTask.uRepeatTime > 0) &&
                !timerTask.weakFlag.expired() &&
                !IsTimerRemoved(timerTask.m_nTimerId)) {
                //如果未达到触发次数限制，重新设置下次触发的时间
                timerTask.trigerTime = std::chrono::steady_clock::now();
                timerTask.trigerTime += std::chrono::milliseconds(timerTask.uElapseMs); //计算出下次触发时间(当前时间 + 间隔的毫秒数)
                m_aTimers.push(timerTask);
            }
            else {
                //执行已完成或者已经失效
                ClearRemovedTimerId(timerTask.m_nTimerId);
            }
        }
        else {
            //已经没有触发的定时器任务
            break;
        }
    }
    //唤醒工作线程，检查任务状态
    m_cv.notify_one();
    m_bHasPenddingPoll = false;
}

void TimerManager::WorkerThreadProc()
{
    while (m_bRunning) {
        std::unique_lock taskGuard(m_taskMutex);
        if (!m_bRunning) {
            break;
        }
        if (m_aTimers.empty()) {
            //为空，等待任务
            m_cv.wait(taskGuard);
            if (!m_bRunning) {
                break;
            }
        }
        else {
            //计算最早的任务，等待超时
            uint32_t nDetaTimeMs = 0;
            auto currentTime = std::chrono::steady_clock::now();
            if (m_aTimers.top().weakFlag.expired() || IsTimerRemoved(m_aTimers.top().m_nTimerId)) {
                //删除已经失效或者取消的定时器
                nDetaTimeMs = 0;
            }
            else if (currentTime >= m_aTimers.top().trigerTime) {
                //队列顶的定时器：已经达到定时器触发条件
                nDetaTimeMs = 0;
            }
            else {
                //已经没有触发的定时器任务
                auto nDiffTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_aTimers.top().trigerTime - currentTime);
                nDetaTimeMs = (uint32_t)nDiffTimeMs.count();
            }

            if (nDetaTimeMs > 0) {
                //延迟等待超时
                //LogUtil::OutputLine(StringUtil::Printf(_T("condition_variable: wait_for timer event(%u ms)"), nDetaTimeMs));
                //该函数精确度10ms左右
                //注意事项：发现gcc版本和glibc版本对wait_for都有问题（使用的时系统时间），gcc >=10 且 glibc >= 2.30 才会对程序行为没有影响。
                m_cv.wait_for(taskGuard, std::chrono::milliseconds(nDetaTimeMs));
            }
            //通知处理(发送到主线程执行, 此时不能加锁，避免出现死锁问题)
            m_bHasPenddingPoll = true;
            taskGuard.unlock();

            m_threadMsg.PostMsg(WM_USER_DEFINED_TIMER, 0, 0);
            taskGuard.lock();
            //LogUtil::OutputLine(StringUtil::Printf(_T("PostMessage: send timer event")));

            if (m_bRunning && m_bHasPenddingPoll) {
                m_cv.wait(taskGuard);
            }
        }        
    }
    m_bRunning = false;
}

}
