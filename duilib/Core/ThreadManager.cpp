#include "ThreadManager.h"

namespace ui 
{
ThreadManager::ThreadManager()
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

    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    ASSERT(iter == m_threadsMap.end());
    if (iter != m_threadsMap.end()) {
        return false;
    }
    ThreadInfo& threadInfo = m_threadsMap[nThreadIdentifier];
    threadInfo.m_pThread = pThread;
    threadInfo.m_threadFlag = pThread->GetWeakFlag();
    return true;
}

bool ThreadManager::HasThread(int32_t nThreadIdentifier) const
{
    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    return iter != m_threadsMap.end();
}

bool ThreadManager::UnregisterThread(int32_t nThreadIdentifier)
{
    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
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
    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
    for (auto iter = m_threadsMap.begin(); iter != m_threadsMap.end(); ++iter) {
        const ThreadInfo& threadInfo = iter->second;
        if (currentThreadId == threadInfo.m_pThread->GetThreadId()) {
            nThreadIdentifier = iter->first;
            break;
        }
    }
    return nThreadIdentifier;
}

bool ThreadManager::GetThreadInfo(int32_t nThreadIdentifier, ThreadInfo& threadInfo) const
{
    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
    auto iter = m_threadsMap.find(nThreadIdentifier);
    if (iter != m_threadsMap.end()) {
        threadInfo = iter->second;
    }
    else {
        threadInfo.m_pThread = nullptr;
        threadInfo.m_threadFlag.reset();
    }
    return !threadInfo.m_threadFlag.expired() && (threadInfo.m_pThread != nullptr);
}

bool ThreadManager::PostTask(int32_t nThreadIdentifier, const StdClosure& task)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return false;
    }
    ThreadInfo threadInfo;
    if (!GetThreadInfo(nThreadIdentifier, threadInfo)) {
        ASSERT(!"ThreadManager::PostTask failed!");
        return false;
    }
    return threadInfo.m_pThread->PostTask(task);
}

bool ThreadManager::PostDelayedTask(int32_t nThreadIdentifier, const StdClosure& task, int32_t nDelayMs)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return false;
    }
    ThreadInfo threadInfo;
    if (!GetThreadInfo(nThreadIdentifier, threadInfo)) {
        ASSERT(!"ThreadManager::PostDelayedTask failed!");
        return false;
    }
    return threadInfo.m_pThread->PostDelayedTask(task, nDelayMs);
}

bool ThreadManager::PostRepeatedTask(int32_t nThreadIdentifier, const StdClosure& task,
                                     int32_t nIntervalMs, int32_t nTimes)
{
    ASSERT(task != nullptr);
    if (task == nullptr) {
        return false;
    }
    ThreadInfo threadInfo;
    if (!GetThreadInfo(nThreadIdentifier, threadInfo)) {
        ASSERT(!"ThreadManager::PostRepeatedTask failed!");
        return false;
    }
    return threadInfo.m_pThread->PostRepeatedTask(task, nIntervalMs, nTimes);
}

void ThreadManager::Clear()
{
    std::lock_guard<std::mutex> threadGuard(m_threadMutex);
    m_threadsMap.clear();
}

}//namespace ui 
