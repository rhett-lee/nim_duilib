#include "FrameworkThread.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/WindowMessage.h"
#include "duilib/Core/ScopedLock.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Core/MessageLoop_SDL.h"
    #include <SDL3/SDL.h>
#elif defined (DUILIB_BUILD_FOR_WIN)
    #include "duilib/Core/MessageLoop_Windows.h"
#endif

#include <sstream>

/** 用户自定义消息
*/
#if defined (DUILIB_BUILD_FOR_SDL)
    #define WM_USER_DEFINED_MSG     (SDL_EVENT_USER + 1)
#else
    #define WM_USER_DEFINED_MSG     (kWM_USER + 568)
#endif

namespace ui 
{
FrameworkThread::FrameworkThread(const DString& threadName, int32_t nThreadIdentifier):
    m_bThreadUI(false),
    m_bRunning(false),
    m_threadName(threadName),
    m_nThreadIdentifier(nThreadIdentifier)
{
    if (m_nThreadIdentifier == kThreadUI) {
        //主线程在构造时，完成必要的初始化
        GlobalManager::Instance().Thread().RegisterThread(m_nThreadIdentifier, this);
        m_nThisThreadId = std::this_thread::get_id();
        m_bThreadUI = true;

#ifdef DUILIB_BUILD_FOR_SDL
        MessageLoop_SDL::CheckInitSDL();
#endif
        //初始化与主线程通信的机制
        m_threadMsg.Initialize(GlobalManager::Instance().GetPlatformData());
        m_threadMsg.SetMessageCallback(WM_USER_DEFINED_MSG, UiBind(&FrameworkThread::OnTaskMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
}

FrameworkThread::~FrameworkThread()
{
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
    }
    m_threadMsg.Clear();
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        Stop();
    }
}

bool FrameworkThread::RunMessageLoop()
{
    ASSERT(m_nThreadIdentifier == kThreadUI);
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        return false;
    }
    m_bRunning = true;
    OnInit();
    OnRunMessageLoop();
    OnCleanup();
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
        m_nThreadIdentifier = kThreadNone;
    }
    m_bThreadUI = false;
    m_threadMsg.Clear();
    m_bRunning = false;
    return true;
}

void FrameworkThread::OnMainThreadInited()
{
}

void FrameworkThread::OnMainThreadExit()
{
    GlobalManager::Instance().Thread().SetMainThreadExit();
}

bool FrameworkThread::Start()
{
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        return false;
    }
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().RegisterThread(m_nThreadIdentifier, this);
    }
    m_bRunning = true;
    m_bThreadUI = false;
    m_pWorkerThread = std::make_unique<std::thread>(&FrameworkThread::WorkerThreadProc, this);
    m_nThisThreadId = m_pWorkerThread->get_id();
    return true;
}

bool FrameworkThread::Stop()
{
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
    }
    ASSERT(!IsUIThread());
    if (m_pWorkerThread != nullptr) {
        //停止线程
        m_bRunning = false;
        m_cv.notify_one();
        m_pWorkerThread->join();
        m_pWorkerThread.reset();
    }
    else {
        m_bRunning = false;
    }
    return true;
}

bool FrameworkThread::IsRunning() const
{
    return m_bRunning;
}

bool FrameworkThread::IsUIThread() const
{
    return m_bThreadUI;
}

std::thread::id FrameworkThread::GetThreadId() const
{
    return m_nThisThreadId;
}

DString FrameworkThread::ThreadIdToString(const std::thread::id& threadId)
{
    // 转为字符串
#ifdef DUILIB_UNICODE    
    std::wstringstream ss;
    ss << threadId;
    std::wstring thread_id_str = ss.str();
    return thread_id_str;
#else
    std::stringstream ss;
    ss << threadId;
    std::string thread_id_str = ss.str();
    return thread_id_str;
#endif
}

int32_t FrameworkThread::GetThreadIdentifier() const
{
    return m_nThreadIdentifier;
}

const DString& FrameworkThread::GetThreadName() const
{
    return m_threadName;
}

size_t FrameworkThread::GetNextTaskId() const
{
    //使用全局任务ID，确保在进程中，此任务ID是唯一的
    return GlobalManager::Instance().Thread().GetNextTaskId();
}

size_t FrameworkThread::PostTask(const StdClosure& task, const StdClosure& unlockClosure)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return false;
    }
    ScopedLock threadGuard(m_taskMutex);
    size_t nTaskId = GetNextTaskId();
    TaskInfo& taskInfo = m_taskMap[nTaskId];
    taskInfo.m_taskType = TaskType::kTask;
    taskInfo.m_task = task;
    taskInfo.m_nIntervalMs = 0;
    taskInfo.m_nTimes = 1;
    taskInfo.m_nTaskId = nTaskId;
    taskInfo.m_startTime = std::chrono::steady_clock::now();
    taskInfo.m_nTotalExecTimes = 0;

    StdClosure unlockClosure1 = [&threadGuard]() {
            threadGuard.Unlock();
        };
    bool bAdded = NotifyExecTask(nTaskId, unlockClosure1, unlockClosure);
    ASSERT_UNUSED_VARIABLE(bAdded);
    return nTaskId;
}

size_t FrameworkThread::PostDelayedTask(const StdClosure& task, int32_t nDelayMs)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return 0;
    }
    ScopedLock threadGuard(m_taskMutex);
    size_t nTaskId = GetNextTaskId();
    TaskInfo& taskInfo = m_taskMap[nTaskId];
    taskInfo.m_taskType = TaskType::kDelayedTask;
    taskInfo.m_task = task;
    taskInfo.m_nIntervalMs = nDelayMs;
    taskInfo.m_nTimes = 1;
    taskInfo.m_nTaskId = nTaskId;
    taskInfo.m_startTime = std::chrono::steady_clock::now();
    taskInfo.m_nTotalExecTimes = 0;

    if (nDelayMs < 1) {
        nDelayMs = 1;
    }
    //生成一个定时器，用来触发任务执行(只执行1次)
    auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId, nullptr, nullptr);
    bool bAdded = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nDelayMs, 1);
    ASSERT_UNUSED_VARIABLE(bAdded);
    return nTaskId;
}

size_t FrameworkThread::PostRepeatedTask(const StdClosure& task, int32_t nIntervalMs, int32_t nTimes)
{
    ASSERT((task != nullptr) && (nIntervalMs > 0) && (nTimes != 0));
    if ((task == nullptr) || (nIntervalMs <= 0) || (nTimes == 0)) {
        return 0;
    }
    ScopedLock threadGuard(m_taskMutex);
    size_t nTaskId = GetNextTaskId();
    TaskInfo& taskInfo = m_taskMap[nTaskId];
    taskInfo.m_taskType = TaskType::kRepeatedTask;
    taskInfo.m_task = task;
    taskInfo.m_nIntervalMs = nIntervalMs;
    taskInfo.m_nTimes = nTimes;
    taskInfo.m_nTaskId = nTaskId;
    taskInfo.m_startTime = std::chrono::steady_clock::now();
    taskInfo.m_nTotalExecTimes = 0;

    if (nTimes < 0) {
        nTimes = -1;
    }
    //生成一个定时器，用来触发任务执行(只执行1次)
    auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId, nullptr, nullptr);
    size_t nTimerId = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nIntervalMs, nTimes);
    ASSERT_UNUSED_VARIABLE(nTimerId > 0);
    return nTaskId;
}

bool FrameworkThread::CancelTask(size_t nTaskId)
{
    bool bDeleted = false;
    ScopedLock threadGuard(m_taskMutex);
    if (!m_taskMap.empty()) {
        auto iter = m_taskMap.find(nTaskId);
        if (iter != m_taskMap.end()) {
            m_taskMap.erase(nTaskId);
            bDeleted = true;
        }
    }
    return bDeleted;
}

bool FrameworkThread::NotifyExecTask(size_t nTaskId,
                                     const StdClosure& unlockClosure1,
                                     const StdClosure& unlockClosure2)
{
    if (IsUIThread()) {
        //UI线程: 异步执行
#ifdef DUILIB_BUILD_FOR_SDL
        //将外层的锁释放，避免SDL底层的锁反向调用产生死锁
        if (unlockClosure1) {
            unlockClosure1();
        }
        if (unlockClosure2) {            
            unlockClosure2();
        }
#else
        UNUSED_VARIABLE(unlockClosure1);
        UNUSED_VARIABLE(unlockClosure2);
#endif
        return m_threadMsg.PostMsg(WM_USER_DEFINED_MSG, nTaskId, 0);
    }
    else {
        //后台工作线程
        ScopedLock threadGuard(m_penddingTaskMutex);
        m_penddingTaskIds.push_back(nTaskId);
        m_cv.notify_all();
        return true;
    }    
}

void FrameworkThread::ExecTask(size_t nTaskId)
{
    ASSERT(std::this_thread::get_id() == m_nThisThreadId);
    StdClosure task;
    {
        ScopedLock threadGuard(m_taskMutex);
        auto iter = m_taskMap.find(nTaskId);
        if (iter != m_taskMap.end()) {
            TaskInfo& taskInfo = iter->second;
            if (taskInfo.m_task != nullptr) {
                if (taskInfo.m_taskType == TaskType::kTask) {
                    //只执行一次
                    task = taskInfo.m_task;
                    m_taskMap.erase(iter);
                }
                else if (taskInfo.m_taskType == TaskType::kDelayedTask) {
                    //只执行一次
                    task = taskInfo.m_task;
                    m_taskMap.erase(iter);
                }
                else if (taskInfo.m_taskType == TaskType::kRepeatedTask) {
                    //定时执行
                    task = taskInfo.m_task;
                    taskInfo.m_nTotalExecTimes++;
                    taskInfo.m_lastExecTime = std::chrono::steady_clock::now();
                    if ((taskInfo.m_nTimes >= 0) && (taskInfo.m_nTotalExecTimes >= taskInfo.m_nTimes)) {
                        //已经执行完成
                        m_taskMap.erase(iter);
                    }
                }
            }
        }
    }
    if (task != nullptr) {
        //执行该任务，在不加锁的状态执行，避免死锁
        task();
    }
}

void FrameworkThread::OnTaskMessage(uint32_t msgId, WPARAM wParam, LPARAM /*lParam*/)
{
    ASSERT(msgId == WM_USER_DEFINED_MSG);
    if (msgId == WM_USER_DEFINED_MSG) {
        ExecTask((size_t)wParam);
    }
}

void FrameworkThread::WorkerThreadProc()
{
    m_nThisThreadId = std::this_thread::get_id();
    OnInit();
    while (m_bRunning) {        
        std::unique_lock lk(m_penddingTaskMutex);
        std::vector<size_t> penddingTaskIds;
        if (m_penddingTaskIds.empty()) {
            m_cv.wait(lk);
        }              
        if (!m_penddingTaskIds.empty()) {
            penddingTaskIds.swap(m_penddingTaskIds);
        }
        lk.unlock();

        for (size_t nTaskId : penddingTaskIds) {
            if (!m_bRunning) {
                break;
            }
            ExecTask(nTaskId);
        }
    }
    m_bRunning = false;
    OnCleanup();
}

void FrameworkThread::OnInit()
{
}

void FrameworkThread::OnRunMessageLoop()
{
#if defined (DUILIB_BUILD_FOR_SDL)
    MessageLoop_SDL msgLoop;
    MessageLoop_SDL::CheckInitSDL();
    OnMainThreadInited();    
    msgLoop.Run();
    OnMainThreadExit();
#elif defined (DUILIB_BUILD_FOR_WIN)
    MessageLoop_Windows msgLoop;
    OnMainThreadInited();
    msgLoop.Run();
    OnMainThreadExit();
#else
    ASSERT(0);
#endif
}

void FrameworkThread::OnCleanup()
{
}

}//namespace ui 
