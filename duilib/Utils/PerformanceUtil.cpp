#include "PerformanceUtil.h"

namespace ui 
{

PerformanceUtil::PerformanceUtil()
{
}

PerformanceUtil::~PerformanceUtil()
{
    for (const auto& iter : m_stat) {
        if (iter.second.totalCount == 0) {
            continue;
        }
        wchar_t szBuf[256] = { 0 };
        swprintf_s(szBuf, L"%s(%d): %d ms, average: %d ms, max: %d ms\n", 
                   iter.first.c_str(),
                   (int)iter.second.totalCount, 
                   (int)(iter.second.totalTimes.count() / 1000),
                   (int)(iter.second.totalTimes.count() / 1000 / iter.second.totalCount),
                   (int)(iter.second.maxTime.count() / 1000));
        ::OutputDebugString(szBuf);
    }
}

PerformanceUtil& PerformanceUtil::Instance()
{
    static PerformanceUtil self;
    return self;
}

void PerformanceUtil::BeginStat(const std::wstring& name)
{
    ASSERT(!name.empty());
    TStat& stat = m_stat[name];
    stat.startTime = std::chrono::high_resolution_clock::now();
    stat.hasStarted = true;
}

void PerformanceUtil::EndStat(const std::wstring& name)
{
    ASSERT(!name.empty());
    TStat& stat = m_stat[name];
    ASSERT(stat.hasStarted);
    if (!stat.hasStarted) {
        return;
    }
    stat.endTime = std::chrono::high_resolution_clock::now();
    stat.totalCount += 1;
    auto thisTime = std::chrono::duration_cast<std::chrono::microseconds>(stat.endTime - stat.startTime);
    stat.totalTimes += thisTime;
    stat.hasStarted = false;
    stat.maxTime = std::max(stat.maxTime, thisTime);
}

}
