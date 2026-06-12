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
    m_bSupportIdle(false),
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
    // 关键修复：必须保证 worker 线程在任何路径下都被 join。
    // 场景：worker 线程进入 WorkerThreadProc 后，若 OnInit() 失败，
    //       会自行设置 m_bRunning=false 并 return，此时线程已退出但未 join。
    // 旧实现仅在 m_bRunning==true 时调用 Stop()，导致 unique_ptr<std::thread>
    //       析构时线程仍处于 joinable 状态，触发 std::terminate() 整个进程崩溃。
    if (m_pWorkerThread != nullptr && m_pWorkerThread->joinable()) {
        if (m_bRunning.load(std::memory_order_acquire)) {
            // 正常运行中 -> 走正常停止流程（Stop 内部会 join + reset）
            Stop();
        }
        else {
            // worker 已自行退出（如 OnInit 失败）-> 仅 join + reset 即可
            m_pWorkerThread->join();
            m_pWorkerThread.reset();
        }
    }
}

bool FrameworkThread::RunMessageLoop(bool bSupportIdle)
{
    ASSERT(m_nThreadIdentifier == kThreadUI);
    ASSERT(!m_bRunning.load(std::memory_order_acquire));
    if (m_bRunning.load(std::memory_order_acquire)) {
        return false;
    }
    m_bRunning.store(true, std::memory_order_release);
    m_bSupportIdle = bSupportIdle;
    if (!OnInit()) {
        //初始化失败，直接返回
        m_bRunning.store(false, std::memory_order_release);
        return false;
    }
    OnRunMessageLoop();
    OnCleanup();
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
        m_nThreadIdentifier = kThreadNone;
    }
    m_bThreadUI = false;
    m_threadMsg.Clear();
    m_bRunning.store(false, std::memory_order_release);
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
    ASSERT(!m_bRunning.load(std::memory_order_acquire));
    if (m_bRunning.load(std::memory_order_acquire)) {
        return false;
    }
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().RegisterThread(m_nThreadIdentifier, this);
    }
    m_bRunning.store(true, std::memory_order_release);
    m_bThreadUI = false;
    m_pWorkerThread = std::make_unique<std::thread>(&FrameworkThread::WorkerThreadProc, this);
    // 注意：m_nThisThreadId 由 WorkerThreadProc 进入后第一时间写入，
    //       这里不再重复赋值，避免与 worker 自身的赋值产生竞态。
    return true;
}

bool FrameworkThread::Stop()
{
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
        //与 RunMessageLoop 保持对称：Unregister 后置为 kThreadNone，
        //避免析构函数中重复 UnregisterThread（依赖底层实现是否幂等）。
        m_nThreadIdentifier = kThreadNone;
    }
    ASSERT(!IsUIThread());
    if (m_pWorkerThread != nullptr) {
        //停止线程
        // 关键修复：
        // 1) 必须在 m_penddingTaskMutex 锁内将 m_bRunning 置为 false，
        //    保证 worker 线程在 wait 唤醒后通过谓词检查能立即看到新的状态；
        // 2) m_cv.notify_all() 必须在锁释放之后调用，避免"未到达等待态的 notify"信号丢失问题；
        // 3) 使用 std::atomic 的 release 内存序与 worker 的 acquire load 配对，确保 happens-before。
        {
            std::lock_guard<std::mutex> lk(m_penddingTaskMutex);
            m_bRunning.store(false, std::memory_order_release);
        }
        m_cv.notify_all();
        m_pWorkerThread->join();
        m_pWorkerThread.reset();
    }
    else {
        m_bRunning.store(false, std::memory_order_release);
    }
    return true;
}

bool FrameworkThread::IsRunning() const
{
    return m_bRunning.load(std::memory_order_acquire);
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
        return 0;
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
    taskInfo.m_nTimerId = 0;

    if (nDelayMs < 1) {
        nDelayMs = 1;
    }
    //生成一个定时器，用来触发任务执行(只执行1次)
    auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId, nullptr, nullptr);
    taskInfo.m_nTimerId = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nDelayMs, 1);
    ASSERT_UNUSED_VARIABLE(taskInfo.m_nTimerId > 0);
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
    taskInfo.m_nTimerId = 0;

    if (nTimes < 0) {
        nTimes = -1;
    }
    //生成一个定时器，用来触发任务执行
    auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId, nullptr, nullptr);
    taskInfo.m_nTimerId = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nIntervalMs, nTimes);
    ASSERT_UNUSED_VARIABLE(taskInfo.m_nTimerId > 0);
    return nTaskId;
}

bool FrameworkThread::CancelTask(size_t nTaskId)
{
    bool bDeleted = false;
    size_t nTimerIdToRemove = 0;
    {
        ScopedLock threadGuard(m_taskMutex);
        if (!m_taskMap.empty()) {
            auto iter = m_taskMap.find(nTaskId);
            if (iter != m_taskMap.end()) {
                // 记录关联的定时器 ID，在锁外调用 RemoveTimer
                // （避免在持锁状态下进入 TimerManager 的内部锁，引起不必要的锁顺序耦合）
                nTimerIdToRemove = iter->second.m_nTimerId;
                m_taskMap.erase(iter);
                bDeleted = true;
            }
        }
    }
    if (bDeleted && nTimerIdToRemove != 0) {
        // 同步取消底层定时器，避免定时器空转触发 NotifyExecTask 空回调
        GlobalManager::Instance().Timer().RemoveTimer(nTimerIdToRemove);
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

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        //优先处理延迟发送的消息
        std::vector<size_t> winTaskIds;
        {
            ScopedLock threadGuard(m_winTaskMutex);
            winTaskIds.swap(m_winTaskIds);
        }
        if (!winTaskIds.empty()) {
            auto iter = winTaskIds.begin();
            while (iter != winTaskIds.end()) {
                bool bRet = m_threadMsg.PostMsg(WM_USER_DEFINED_MSG, *iter, 0, nullptr);
                if (bRet) {
                    iter = winTaskIds.erase(iter);
                }
                else {
                    ++iter;
                }
            }
        }
#endif

        uint32_t nErrorCode = 0;
        bool bRet = m_threadMsg.PostMsg(WM_USER_DEFINED_MSG, nTaskId, 0, &nErrorCode);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        if (!bRet) {
            if (nErrorCode == ERROR_NOT_ENOUGH_QUOTA) {
                if (!GlobalManager::Instance().IsInUIThread()) { //在子线程中执行
                    //将外层的锁释放，避免SDL底层的锁反向调用产生死锁
                    if (unlockClosure1) {
                        unlockClosure1();
                    }
                    if (unlockClosure2) {
                        unlockClosure2();
                    }
                    //在程序启动时，如果在子线程向主线程Post消息，会遇到此错误
                    for (int32_t i = 0; i < 200; ++i) {
                        ::Sleep(50);
                        if (!IsRunning()) {
                            break;
                        }
                        bRet = m_threadMsg.PostMsg(WM_USER_DEFINED_MSG, nTaskId, 0, &nErrorCode);
                        if (bRet || (nErrorCode != ERROR_NOT_ENOUGH_QUOTA)) {
                            break;
                        }
                    }
                }

                if (!bRet) { //发送失败时，只能延迟发送消息
                    winTaskIds.push_back(nTaskId);
                    bRet = true;
                }
                if (!winTaskIds.empty()) {
                    ScopedLock threadGuard(m_winTaskMutex);
                    for (size_t nPenddingTaskId : winTaskIds) {
                        m_winTaskIds.push_back(nPenddingTaskId);
                    }                    
                }
            }
            ASSERT_UNUSED_VARIABLE(bRet);
        }
#else
        ASSERT_UNUSED_VARIABLE(bRet);
#endif
        return bRet;
    }
    else {
        //后台工作线程
        // 关键修复：Stop() 之后停止接收新任务，避免 m_penddingTaskIds 残留。
        // worker 线程已经退出（或即将退出）的情况下，这里推入的任务不会被消费。
        if (!m_bRunning.load(std::memory_order_acquire)) {
            return false;
        }
        ScopedLock threadGuard(m_penddingTaskMutex);
        m_penddingTaskIds.push_back(nTaskId);
        // 锁外通知：先释放锁再 notify，避免"未到达等待态的 notify"信号丢失问题
        threadGuard.Unlock();
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
    if (!OnInit()) {
        //初始化失败，退出线程
        m_bRunning.store(false, std::memory_order_release);
        OnCleanup();
        return;
    }
    // 关键修复：使用带谓词的 wait，处理 spurious wakeup 和 Stop() 期间的"信号丢失"竞态
    // 谓词条件：有任务 OR 线程应停止
    // 同时将 penddingTaskIds 提取到循环外，避免每轮重新分配 vector
    std::vector<size_t> penddingTaskIds;
    while (m_bRunning.load(std::memory_order_acquire)) {
        std::unique_lock lk(m_penddingTaskMutex);
        // wait(lk, pred) 等价于:
        //   while (!pred()) {
        //       lk.unlock();
        //       block_on(m_cv);
        //       lk.lock();
        //   }
        // 即谓词会在每次唤醒（包括 spurious wakeup）后重新检查，
        // 保证即使 Stop() 中的 notify_all 因竞态丢失，worker 也能在重新获取锁后看到 m_bRunning=false
        if (m_penddingTaskIds.empty()) {
            m_cv.wait(lk, [this] {
                return !m_penddingTaskIds.empty() || !m_bRunning.load(std::memory_order_acquire);
                });
        }
        if (!m_penddingTaskIds.empty()) {
            penddingTaskIds.swap(m_penddingTaskIds);
        }
        lk.unlock();

        for (size_t nTaskId : penddingTaskIds) {
            if (!m_bRunning.load(std::memory_order_acquire)) {
                break;
            }
            ExecTask(nTaskId);
        }
        penddingTaskIds.clear();
    }
    m_bRunning.store(false, std::memory_order_release);
    OnCleanup();
}

bool FrameworkThread::OnInit()
{
    return true;
}

void FrameworkThread::OnRunMessageLoop()
{
#if defined (DUILIB_BUILD_FOR_SDL)
    MessageLoop_SDL msgLoop;
    MessageLoop_SDL::CheckInitSDL();
#elif defined (DUILIB_BUILD_FOR_WIN)
    MessageLoop_Windows msgLoop;
#else
    ASSERT(0);
    return;
#endif

    OnMainThreadInited();
    if (m_bSupportIdle) {
        //支持Idle函数
        msgLoop.Run([this]() {
            return OnMessageLoopIdle();
            });
    }
    else {
        //不支持Idle函数
        msgLoop.Run(nullptr);
    }
    OnMainThreadExit();
}

void FrameworkThread::OnCleanup()
{
}

void FrameworkThread::OnMessageLoopIdle()
{
}

}//namespace ui 
