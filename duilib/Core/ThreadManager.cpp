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
    std::thread::id currentThreadId = std::this_thread::get_id();
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
    ScopedLock threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter != m_threadsMap.end()) {
        FrameworkThreadPtr spFrameworkThread = iter->second;
        if (spFrameworkThread != nullptr) {
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
