#include "PerformanceUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/LogUtil.h"

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
        DString log = StringUtil::Printf(_T("%s(%d): %d ms, average: %d ms, max: %d ms"), 
                                        iter.first.c_str(),
                                        (int32_t)iter.second.totalCount, 
                                        (int32_t)(iter.second.totalTimes.count() / 1000),
                                        (int32_t)(iter.second.totalTimes.count() / 1000 / iter.second.totalCount),
                                        (int32_t)(iter.second.maxTime.count() / 1000));
        LogUtil::OutputLine(log);
    }
}

PerformanceUtil& PerformanceUtil::Instance()
{
    static PerformanceUtil self;
    return self;
}

void PerformanceUtil::BeginStat(const DString& name)
{
    ASSERT(!name.empty());
    TStat& stat = m_stat[name];
    stat.startTime = std::chrono::steady_clock::now();
    ASSERT(stat.nStartRefCount >= 0);
    stat.nStartRefCount++;
}

void PerformanceUtil::EndStat(const DString& name)
{
    ASSERT(!name.empty());
    TStat& stat = m_stat[name];
    ASSERT(stat.nStartRefCount > 0);
    if (stat.nStartRefCount <= 0) {
        return;
    }
    stat.endTime = std::chrono::steady_clock::now();
    stat.totalCount += 1;
    auto thisTime = std::chrono::duration_cast<std::chrono::microseconds>(stat.endTime - stat.startTime);
    stat.totalTimes += thisTime;
    stat.nStartRefCount--;
    stat.maxTime = (std::max)(stat.maxTime, thisTime);
}

}
