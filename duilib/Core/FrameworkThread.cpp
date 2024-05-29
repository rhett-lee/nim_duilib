#include "FrameworkThread.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{
#ifdef DUILIB_PLATFORM_WIN
    #define WM_USER_DEFINED_MSG    (WM_USER + 9998)
#endif

FrameworkThread::FrameworkThread(const DString& threadName, int32_t nThreadIdentifier):
    m_bThreadUI(false),
    m_bRunning(false),
    m_threadName(threadName),
    m_nThreadIdentifier(nThreadIdentifier),
    m_nNextTaskId(1)
{
#ifdef DUILIB_PLATFORM_WIN
    m_hMessageWnd = nullptr;
#endif
}

FrameworkThread::~FrameworkThread()
{
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
    }
#ifdef DUILIB_PLATFORM_WIN
    if (m_hMessageWnd != nullptr) {
        ::DestroyWindow(m_hMessageWnd);
        m_hMessageWnd = nullptr;
    }
#endif
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        Stop();
    }
}

bool FrameworkThread::RunOnCurrentThreadWithLoop()
{
    ASSERT(!m_bRunning);
    if (m_bRunning) {
        return false;
    }
    m_bRunning = true;
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().RegisterThread(m_nThreadIdentifier, this);
    }
    m_nThisThreadId = std::this_thread::get_id();    
    m_bThreadUI = true;

    //创建消息窗口
#ifdef DUILIB_PLATFORM_WIN
    auto hinst = ::GetModuleHandle(NULL);
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndMsgProc;
    wc.hInstance = hinst;
    wc.lpszClassName = _T("duilib_FrameworkThread");
    ::RegisterClassExW(&wc);
    m_hMessageWnd = ::CreateWindowW(_T("duilib_FrameworkThread"), 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hinst, 0);
    ASSERT(m_hMessageWnd != nullptr);
    ::SetWindowLongPtr(m_hMessageWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
#endif

    OnInit();

    MSG msg = {0, };
    while (::GetMessage(&msg, 0, 0, 0) > 0) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    OnCleanup();
    if (m_nThreadIdentifier != kThreadNone) {
        GlobalManager::Instance().Thread().UnregisterThread(m_nThreadIdentifier);
    }
    m_bThreadUI = false;    
#ifdef DUILIB_PLATFORM_WIN
    if (m_hMessageWnd != nullptr) {
        ::DestroyWindow(m_hMessageWnd);
        m_hMessageWnd = nullptr;
    }
#endif
    m_bRunning = false;
    return true;
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

size_t FrameworkThread::PostTask(const StdClosure& task)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    size_t nTaskId = m_nNextTaskId++;
    TaskInfo& taskInfo = m_taskMap[nTaskId];
    taskInfo.m_taskType = TaskType::kTask;
    taskInfo.m_task = task;
    taskInfo.m_nIntervalMs = 0;
    taskInfo.m_nTimes = 1;
    taskInfo.m_nTaskId = nTaskId;
    taskInfo.m_startTime = std::chrono::steady_clock::now();
    taskInfo.m_nTotalExecTimes = 0;

    bool bAdded = NotifyExecTask(nTaskId);
    ASSERT_UNUSED_VARIABLE(bAdded);
    return nTaskId;
}

size_t FrameworkThread::PostDelayedTask(const StdClosure& task, int32_t nDelayMs)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    size_t nTaskId = m_nNextTaskId++;
    TaskInfo& taskInfo = m_taskMap[nTaskId];
    taskInfo.m_taskType = TaskType::kDelayedTask;
    taskInfo.m_task = task;
    taskInfo.m_nIntervalMs = nDelayMs;
    taskInfo.m_nTimes = 1;
    taskInfo.m_nTaskId = nTaskId;
    taskInfo.m_startTime = std::chrono::steady_clock::now();
    taskInfo.m_nTotalExecTimes = 0;

    bool bAdded = false;
    if (nDelayMs > 0) {
        //生成一个定时器，用来触发任务执行(只执行1次)
        auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId);
        bAdded = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nDelayMs, 1);
    }
    else {
        bAdded = NotifyExecTask(nTaskId);
    }
    ASSERT_UNUSED_VARIABLE(bAdded);
    return nTaskId;
}

size_t FrameworkThread::PostRepeatedTask(const StdClosure& task, int32_t nIntervalMs, int32_t nTimes)
{
    ASSERT((task != nullptr) && (nIntervalMs > 0) && (nTimes != 0));
    if ((task == nullptr) || (nIntervalMs <= 0) || (nTimes == 0)) {
        return 0;
    }
    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    size_t nTaskId = m_nNextTaskId++;
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
    auto timerCallback = UiBind(&FrameworkThread::NotifyExecTask, this, nTaskId);
    size_t nTimerId = GlobalManager::Instance().Timer().AddTimer(GetWeakFlag(), timerCallback, nIntervalMs, nTimes);
    ASSERT_UNUSED_VARIABLE(nTimerId > 0);
    return nTaskId;
}

bool FrameworkThread::CancelTask(size_t nTaskId)
{
    bool bDeleted = false;
    std::lock_guard<std::mutex> threadGuard(m_taskMutex);
    auto iter = m_taskMap.find(nTaskId);
    if (iter != m_taskMap.end()) {
        m_taskMap.erase(nTaskId);
        bDeleted = true;
    }
    return bDeleted;
}

bool FrameworkThread::NotifyExecTask(size_t nTaskId)
{
    if (IsUIThread()) {
        //UI线程
#ifdef DUILIB_PLATFORM_WIN
        ASSERT(m_hMessageWnd != nullptr);
        if (m_hMessageWnd == nullptr) {
            return false;
        }
        return ::PostMessage(m_hMessageWnd, WM_USER_DEFINED_MSG, nTaskId, 0) != FALSE;
#else
        ASSERT(false);
#endif
    }
    else {
        //后台工作线程
        std::lock_guard<std::mutex> threadGuard(m_penddingTaskMutex);
        m_penddingTaskIds.push_back(nTaskId);
        m_cv.notify_one();
        return true;
    }
}

void FrameworkThread::ExecTask(size_t nTaskId)
{
    ASSERT(std::this_thread::get_id() == m_nThisThreadId);
    StdClosure task;
    {
        std::lock_guard<std::mutex> threadGuard(m_taskMutex);
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
        task();
    }
}

#ifdef DUILIB_PLATFORM_WIN

LRESULT FrameworkThread::WndMsgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_USER_DEFINED_MSG) {
        FrameworkThread* pThis = reinterpret_cast<FrameworkThread*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (pThis != nullptr) {
            pThis->ExecTask((size_t)wparam);
        }        
        return 1;
    }
    return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

#endif

void FrameworkThread::WorkerThreadProc()
{
    ASSERT(std::this_thread::get_id() == m_nThisThreadId);
    while (m_bRunning) {
        std::unique_lock lk(m_penddingTaskMutex);
        m_cv.wait(lk);
        std::vector<size_t> penddingTaskIds;
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
}

void FrameworkThread::OnInit()
{
}

void FrameworkThread::OnCleanup()
{
}



}//namespace ui 
