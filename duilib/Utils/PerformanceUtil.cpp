#include "PerformanceUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/LogUtil.h"

#if defined DUILIB_BUILD_FOR_WIN
namespace ui
{
/** Windows平台高精度计时辅助函数（基于 QueryPerformanceCounter）
*   使用 QPC 原始 tick 值进行累积, 仅在最终输出时转换为微秒,
*   避免每次测量时调用 duration_cast 造成的整数截断误差, 计时精度最高.
*/
namespace PerformanceUtilHelperInternal
{
    /** 获取 QPC 频率(每秒钟的 tick 数), 仅首次调用时查询系统
    */
    static int64_t GetQPCFrequency()
    {
        static int64_t s_qpcFrequency = 0;
        if (s_qpcFrequency == 0) {
            LARGE_INTEGER freq;
            QueryPerformanceFrequency(&freq);
            s_qpcFrequency = freq.QuadPart;
        }
        return s_qpcFrequency;
    }

    /** 获取当前 QPC tick 值
    */
    static int64_t GetQPCTick()
    {
        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);
        return tick.QuadPart;
    }

    /** 将 QPC tick 差值转换为微秒(保证最高精度)
    *   使用整数除法先拆分为"整秒部分 + 余数部分"再做转换, 避免大数值乘法溢出.
    */
    static int64_t QPCTickToMicroseconds(int64_t tickDiff)
    {
        if (tickDiff <= 0) {
            return 0;
        }
        const int64_t qpcFreq = GetQPCFrequency();
        if (qpcFreq <= 0) {
            return 0;
        }
        // 整秒部分: tickDiff / qpcFreq
        const int64_t wholeSeconds = tickDiff / qpcFreq;
        // 余数部分(不足一秒的 tick): tickDiff % qpcFreq
        const int64_t remainderTicks = tickDiff - wholeSeconds * qpcFreq;
        // 余数部分转换为微秒(整型除法, 不会溢出)
        const int64_t remainderMicroseconds = (remainderTicks * 1000000) / qpcFreq;
        return wholeSeconds * 1000000 + remainderMicroseconds;
    }
}
}
#endif

namespace ui 
{
PerformanceUtil::PerformanceUtil(const DString& statName) :
    m_statName(statName),
    m_nameHash(0)
{
    if (!m_statName.empty()) {
        PerformanceUtilHelper::Instance().AddStat(m_statName);
        m_nameHash = std::hash<DString>{}(m_statName);
        PerformanceUtilHelper::Instance().BeginStat(m_nameHash);
    }
}
PerformanceUtil::~PerformanceUtil()
{
    if (m_nameHash != 0) {
        PerformanceUtilHelper::Instance().EndStat(m_nameHash);
    }
}

void PerformanceUtil::EndStat()
{
    if (m_nameHash != 0) {
        PerformanceUtilHelper::Instance().EndStat(m_nameHash);
        m_nameHash = 0;
    }
}

////////////////////////////////////////////////////////////////////

PerformanceUtilFast::PerformanceUtilFast(size_t nameHash) :
    m_nameHash(nameHash)
{
    if (m_nameHash != 0) {
        PerformanceUtilHelper::Instance().BeginStat(m_nameHash);
    }
}
PerformanceUtilFast::~PerformanceUtilFast()
{
    if (m_nameHash != 0) {
        PerformanceUtilHelper::Instance().EndStat(m_nameHash);
    }
}

void PerformanceUtilFast::EndStat()
{
    if (m_nameHash != 0) {
        PerformanceUtilHelper::Instance().EndStat(m_nameHash);
        m_nameHash = 0;
    }
}

////////////////////////////////////////////////////////////////////

PerformanceUtilHelper::PerformanceUtilHelper():
    m_nStatIndex(0)
{
}

PerformanceUtilHelper::~PerformanceUtilHelper()
{
#if DUILIB_PERFORMANCE_STAT_ENABLED
    std::vector<TStat> statList;
    for (const auto& iter : m_stat) {
        if (iter.second.totalCount == 0) {
            continue;
        }
        statList.push_back(iter.second);
    }
    //按名称/添加顺序排序
    if (!statList.empty()) {
        std::sort(statList.begin(), statList.end(), [](const TStat& l, const TStat& r) {
            return l.m_name < r.m_name;
            });
    }
    for (const TStat& stat : statList) {
#if defined DUILIB_BUILD_FOR_WIN
        //Windows平台: 仅在这里把累积的 QPC tick 转换为微秒(只转换一次, 避免累积误差)
        const int64_t totalMicroseconds = PerformanceUtilHelperInternal::QPCTickToMicroseconds(stat.totalTicks);
        const int64_t maxMicroseconds = PerformanceUtilHelperInternal::QPCTickToMicroseconds(stat.maxTicks);
        const int64_t totalMs = totalMicroseconds / 1000;
        const int32_t totalMsInt32 = (int32_t)totalMs;
        const float totalMsFloat = (float)totalMsInt32 / stat.totalCount;
        DString log = StringUtil::Printf(_T("%s(%d): %d ms, average: %.03f ms, max: %d ms"),
                                        stat.m_name.c_str(),                 //统计名称
                                        (int32_t)stat.totalCount,            //统计总次数
                                        (int32_t)(totalMs),                  //总耗时(ms)
                                        totalMsFloat,                        //平均耗时(ms)
                                        (int32_t)(maxMicroseconds / 1000)    //最大耗时(ms)
                                        );
#else
        auto totalMs = stat.totalTimes.count() / 1000;
        int32_t totalMsInt32 = (int32_t)totalMs;
        float totalMsFloat = (float)totalMsInt32 / stat.totalCount;
        DString log = StringUtil::Printf(_T("%s(%d): %d ms, average: %.03f ms, max: %d ms"),
                                        stat.m_name.c_str(),                 //统计名称
                                        (int32_t)stat.totalCount,    //统计总次数
                                        (int32_t)(stat.totalTimes.count() / 1000), //总耗时(ms)
                                        totalMsFloat, //平均耗时(ms)
                                        (int32_t)(stat.maxTime.count() / 1000) //最大耗时(ms)
                                        );
#endif
        LogUtil::OutputLine(log);
    }
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED
}

PerformanceUtilHelper& PerformanceUtilHelper::Instance()
{
    static PerformanceUtilHelper self;
    return self;
}

void PerformanceUtilHelper::BeginStat(const DString& name)
{
    ASSERT(!name.empty());
    size_t nameHash = std::hash<DString>{}(name);
    TStat& stat = m_stat[nameHash];
    stat.m_name = name;
    BeginStat(nameHash);
}

void PerformanceUtilHelper::EndStat(const DString& name)
{
    ASSERT(!name.empty());
    size_t nameHash = std::hash<DString>{}(name);
    EndStat(nameHash);
}

void PerformanceUtilHelper::AddStat(const DString& name)
{
    ASSERT(!name.empty());
    size_t nameHash = std::hash<DString>{}(name);
    TStat& stat = m_stat[nameHash];
    ASSERT(stat.m_name.empty() || (stat.m_name == name));
    stat.m_name = name;
    stat.nStatIndex = ++m_nStatIndex;
}

void PerformanceUtilHelper::BeginStat(size_t nameHash)
{
    TStat& stat = m_stat[nameHash];
#if defined DUILIB_BUILD_FOR_WIN
    //Windows平台: 使用 QueryPerformanceCounter 获取高精度开始时间
    //关键改进: 将开始 tick 压栈, 而不是覆盖 stat.startTick, 这样即使出现嵌套调用
    //          (例如 AlphaPaint 在 Alpha 分支中触发了对自身的递归或回调), 也不会丢失
    //          外层调用的开始时间, 从而避免外层时间被算小的问题
    const int64_t startTick = PerformanceUtilHelperInternal::GetQPCTick();
    stat.tickStack.push_back(startTick);
    if (stat.tickStack.size() == 1) {
        //最外层调用, 记录到 stat.startTick(供调试/扩展使用)
        stat.startTick = startTick;
    }
#else
    stat.startTime = std::chrono::steady_clock::now();
#endif
    ASSERT(stat.nStartRefCount >= 0);
    stat.nStartRefCount++;
}

void PerformanceUtilHelper::EndStat(size_t nameHash)
{
#if defined DUILIB_BUILD_FOR_WIN
    //Windows平台: 使用 QueryPerformanceCounter 获取高精度结束时间
    const int64_t endTick = PerformanceUtilHelperInternal::GetQPCTick();
#else
    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
#endif
    TStat& stat = m_stat[nameHash];
    ASSERT(stat.nStartRefCount > 0);
    if (stat.nStartRefCount <= 0) {
        return;
    }
    stat.totalCount += 1;
#if defined DUILIB_BUILD_FOR_WIN
    //Windows平台: 从栈顶弹出对应的开始 tick 计算本次耗时, 这样无论嵌套多少层都能正确计时
    if (stat.tickStack.empty()) {
        //理论上不会到这里(因为 nStartRefCount > 0 表示至少有一次 BeginStat, 必然有压栈)
        //但作为防御性编程, 防止栈下溢
        return;
    }
    const int64_t thisStartTick = stat.tickStack.back();
    stat.tickStack.pop_back();
    if (stat.tickStack.empty()) {
        //最外层调用结束, 记录到 stat.endTick(供调试/扩展使用)
        stat.endTick = endTick;
    }
    int64_t thisTick = endTick - thisStartTick;
    if (thisTick < 0) {
        thisTick = 0;
    }
    stat.totalTicks += thisTick;
    if (thisTick > stat.maxTicks) {
        stat.maxTicks = thisTick;
    }
#else
    stat.endTime = endTime;
    auto thisTime = std::chrono::duration_cast<std::chrono::microseconds>(stat.endTime - stat.startTime);
    stat.totalTimes += thisTime;
    stat.maxTime = (std::max)(stat.maxTime, thisTime);
#endif
    stat.nStartRefCount--;
}

}
