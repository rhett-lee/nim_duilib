#ifndef UI_CORE_TIMER_MANAGER_H_
#define UI_CORE_TIMER_MANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "base/callback/callback.h"
#include <queue>

namespace ui 
{

/** 定时器回调函数原型：void FunctionName();
*/
typedef std::function<void()> TimerCallback;
class TimerInfo;

/** 定时器管理器
*/
class TimerManager
{
public:
	TimerManager();
	~TimerManager();
	TimerManager(const TimerManager&) = delete;
	TimerManager& operator = (const TimerManager&) = delete;

public:
	const static int REPEAT_FOREVER = -1;

	/** 添加一个可取消的定时器
	* @param [in] weakFlag 定时器取消机制，如果weakFlag.expired()为true表示定时器已经取消，不会在继续派发定时器回调
	* @param [in] callback 定时器回调函数
	* @param [in] uElapse 定时器触发时间间隔，单位为毫秒
	* @param [in] iRepeatTime 定时器回调次数限制，如果为 -1 表示不停重复回调
	*/
	bool AddCancelableTimer(const std::weak_ptr<nbase::WeakFlag>& weakFlag,
		                    const TimerCallback& callback,
		                    uint32_t uElapse, 
		                    int32_t iRepeatTime);

	/** 关闭定时器管理器，释放资源
	 */
	void Clear();

private:
	/** 消息窗口函数
	*/
	static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	/** 定时器回调函数
	*/
	static void CALLBACK TimeCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	/** 定时器触发，进行定时器事件回调派发
	*/
	void Poll();

	/** 停止定时器
	*/
	void KillTimerEvent();

private:
	/** 消息窗口句柄，用于在UI线程中派发定时器事件
	*/
	HWND m_hMessageWnd;

	/** 所有注册的定时器
	*/
	std::priority_queue<TimerInfo> m_aTimers;

	/** 性能计数器频率(每秒的时钟滴答数)
	*/
	LARGE_INTEGER m_timeFrequency;

	/** 当前系统定时器的间隔是否为最小间隔
	*/
	bool m_bMinInterval;

	/** 定时器ID
	*/
	uint32_t m_nTimerId;
};

} // namespace ui

#endif // UI_CORE_TIMER_MANAGER_H_
