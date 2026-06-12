#include "ThreadManager.h"
#include "duilib/Core/ScopedLock.h"

namespace ui 
{
ThreadManager::ThreadManager():
    m_nNextTaskId(1),
    m_bMainThreadExit(false)
{
}

ThreadManager::~ThreadManager()
{
    Clear();
}

bool ThreadManager::RegisterThread(int32_t nThreadIdentifier, FrameworkThread* pThread)
{
    ASSERT(nThreadIdentifier >= 0);
    ASSERT(pThread != nullptr);
    if (pThread == nullptr) {
        return false;
    }

    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    ASSERT(iter == m_threadsMap.end());
    if (iter != m_threadsMap.end()) {
        return false;
    }
    m_threadsMap[nThreadIdentifier] = pThread;
    return true;
}

bool ThreadManager::HasThread(int32_t nThreadIdentifier) const
{
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    return iter != m_threadsMap.end();
}

bool ThreadManager::UnregisterThread(int32_t nThreadIdentifier)
{
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter == m_threadsMap.end()) {
        return false;
    }
    else {
        m_threadsMap.erase(iter);
        return true;
    }
}

int32_t ThreadManager::GetCurrentThreadIdentifier() const
{
    int32_t nThreadIdentifier = kThreadNone;
    // 1. 先获取当前线程ID（不需要锁，thread::id 比较本身是原子的）
    std::thread::id currentThreadId = std::this_thread::get_id();
    // 2. 加锁保护 m_threadsMap 遍历
    // 注：map 元素较少（一般 < 10），持锁时间可接受
    // 优化方向：可使用 thread_local 缓存当前线程的 identifier 以减少查找
    ScopedLock threadGuard(m_threadMutex);
    for (auto iter = m_threadsMap.begin(); iter != m_threadsMap.end(); ++iter) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread == nullptr) {
            continue;
        }
        if (currentThreadId == spFrameworkThread->GetThreadId()) {
            nThreadIdentifier = iter->first;
            break;
        }
    }
    return nThreadIdentifier;
}

size_t ThreadManager::PostTask(int32_t nThreadIdentifier, const StdClosure& task)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return 0;
    }
    size_t nTaskId = 0;
    // 外层锁：保护 m_threadsMap，确保查找到的 FrameworkThread 不会被并发注销
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter != m_threadsMap.end()) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread != nullptr) {
            // 跨线程任务分发期间，需要在 FrameworkThread::NotifyExecTask 内部
            // 提前释放外层 ThreadManager 的互斥量与 FrameworkThread 自身的互斥量，
            // 以避免 UI 线程消息泵与子线程的锁顺序倒置造成死锁。
            // 此闭包由 FrameworkThread::NotifyExecTask 在调用方线程上回调，
            // 释放 ThreadManager::m_threadMutex；FrameworkThread 内部还会
            // 释放其自身的 m_taskMutex。两个 ScopedLock 的析构函数均已
            // 实现幂等 Unlock()，因此即便闭包提前解锁也不会重复释放。
            StdClosure unlockClosure = [&threadGuard]() {
                    threadGuard.Unlock();
                };
            nTaskId = spFrameworkThread->PostTask(task, unlockClosure);
        }
    }
    ASSERT(nTaskId != 0);
    return nTaskId;
}

size_t ThreadManager::PostDelayedTask(int32_t nThreadIdentifier, const StdClosure& task, int32_t nDelayMs)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return 0;
    }
    size_t nTaskId = 0;
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter != m_threadsMap.end()) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread != nullptr) {
            nTaskId = spFrameworkThread->PostDelayedTask(task, nDelayMs);
        }
    }
    ASSERT(nTaskId != 0);
    return nTaskId;
}

size_t ThreadManager::PostRepeatedTask(int32_t nThreadIdentifier, const StdClosure& task,
                                       int32_t nIntervalMs, int32_t nTimes)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return 0;
    }
    size_t nTaskId = 0;
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter != m_threadsMap.end()) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread != nullptr) {
            nTaskId = spFrameworkThread->PostRepeatedTask(task, nIntervalMs, nTimes);
        }
    }
    ASSERT(nTaskId != 0);
    return nTaskId;
}

bool ThreadManager::CancelTask(size_t nTaskId)
{
    bool bCancelTask = false;
    ScopedLock threadGuard(m_threadMutex);
    for (auto iter = m_threadsMap.begin(); iter != m_threadsMap.end(); ++iter) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread == nullptr) {
            continue;
        }
        if (spFrameworkThread->CancelTask(nTaskId)) {
            bCancelTask = true;
            break;
        }
    }
    return bCancelTask;
}

void ThreadManager::Clear()
{
    ScopedLock threadGuard(m_threadMutex);
    m_threadsMap.clear();
}

size_t ThreadManager::GetNextTaskId()
{
    size_t nNextTaskId = m_nNextTaskId++;
    return nNextTaskId;
}

bool ThreadManager::IsMainThreadExit() const
{
    return m_bMainThreadExit;
}

void ThreadManager::SetMainThreadExit()
{
    m_bMainThreadExit = true;
}

}//namespace ui 
