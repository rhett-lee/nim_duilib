#ifndef UI_UTILS_PERFORMANCE_UTIL_H_
#define UI_UTILS_PERFORMANCE_UTIL_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <string>
#include <map>
#include <chrono>

namespace ui 
{

/** 代码执行性能分析工具
*/
class UILIB_API PerformanceUtil
{
public:
	PerformanceUtil();
	~PerformanceUtil();

	/** 单例对象
	*/
	static PerformanceUtil& Instance();

	/** 代码开始执行，开始计时
	* @param [in] name 统计项的名称
	*/
	void BeginStat(const std::wstring& name);

	/** 代码结束执行，统计执行性能
	* @param [in] name 统计项的名称
	*/
	void EndStat(const std::wstring& name);
	
private:
	/** 记录每项统计的结果
	*/
	struct TStat
	{
		//是否有开始数据
		bool hasStarted = false;

		//开始时间
		std::chrono::steady_clock::time_point startTime;

		//结束时间
		std::chrono::steady_clock::time_point endTime;

		//代码执行总时间：微秒(千分之一毫秒)
		std::chrono::microseconds totalTimes = std::chrono::microseconds::zero();

		//统计总次数
		uint32_t totalCount = 0;

		/** 单次最大：：微秒(千分之一毫秒)
		*/
		std::chrono::microseconds maxTime = std::chrono::microseconds::zero();
	};

	std::map<std::wstring, TStat> m_stat;
};

class PerformanceStat
{
public:
	PerformanceStat(const std::wstring& statName):
		m_statName(statName)
	{
		PerformanceUtil::Instance().BeginStat(m_statName);
	}
	~PerformanceStat()
	{
		PerformanceUtil::Instance().EndStat(m_statName);
	}
private:
	std::wstring m_statName;
};

}

#endif // UI_UTILS_PERFORMANCE_UTIL_H_