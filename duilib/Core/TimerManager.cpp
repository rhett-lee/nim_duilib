#include "TimerManager.h"
#include "duilib/Core/GlobalManager.h"
#include <Mmsystem.h>

namespace ui 
{

#define WM_USER_DEFINED_TIMER	(WM_USER + 9999)

/** 系统定时器触发时间间隔：单位为毫秒
*/
#define TIMER_INTERVAL	16

/** 定时器触发精度：单位为毫秒
*/
#define TIMER_PRECISION	1

/** 定时器的数据
*/
class TimerInfo
{
public:
	TimerInfo(): 
		timerCallback(nullptr),
		uPerformanceCount(0),
		uRepeatTime(0)
	{
		dwEndPerformanceCounter.QuadPart = 0;
	}

	bool operator < (const TimerInfo& timerInfo) const {
		//排序条件：最先触发的排在前面
		return dwEndPerformanceCounter.QuadPart > timerInfo.dwEndPerformanceCounter.QuadPart;
	}

	//定时器回调函数
	TimerCallback timerCallback;

	//定时器间隔：（单位：性能计数器的滴答次数）
	LONGLONG uPerformanceCount;

	//重复次数: -1 表示不停重复
	uint32_t uRepeatTime;

	//定时器结束时间: 性能计数器的期望值
	LARGE_INTEGER dwEndPerformanceCounter;

	//取消定时器同步机制
	std::weak_ptr<nbase::WeakFlag> weakFlag;
};

TimerManager::TimerManager() : 
	m_hMessageWnd(nullptr),
	m_aTimers(),
	m_bMinInterval(true),
	m_nTimerId(0)
{
	::QueryPerformanceFrequency(&m_timeFrequency); 
	auto hinst = ::GetModuleHandle(NULL);
	WNDCLASSEXW wc = {0};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProcThunk;
	wc.hInstance = hinst;
	wc.lpszClassName = L"UI_ANIMATION_TIMERMANAGER_H_";
	::RegisterClassExW(&wc);
	m_hMessageWnd = ::CreateWindowW(L"UI_ANIMATION_TIMERMANAGER_H_", 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hinst, 0);
}

TimerManager::~TimerManager()
{
	if (m_hMessageWnd != nullptr) {
		::DestroyWindow(m_hMessageWnd);
		m_hMessageWnd = nullptr;
	}
}

LRESULT TimerManager::WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_USER_DEFINED_TIMER) {
		//移除队列中多余的消息，避免队列中有大量无用的重复消息，导致无法处理鼠标键盘消息
		size_t msgCount = 0;
		MSG msg;
		while (::PeekMessage(&msg, hwnd, WM_USER_DEFINED_TIMER, WM_USER_DEFINED_TIMER, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				//检测到退出消息，重新放到消息队列中，避免进程退不出
				::PostQuitMessage(static_cast<int>(msg.wParam));
				return ::DefWindowProcW(hwnd, message, wparam, lparam);
			}
			ASSERT(msg.message == WM_USER_DEFINED_TIMER);
			++msgCount;
		}
//#ifdef _DEBUG
//		if (msgCount > 0) {
//			char str[256] = { 0 };
//			sprintf_s(str, "TimerManager::WndProcThunk eat msgs: %d\n", (int)msgCount);
//			::OutputDebugStringA(str);
//		}
//#endif
		GlobalManager::Instance().Timer().Poll();
		return 1;
	}
	return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

void TimerManager::TimeCallback(UINT /*uTimerID*/, UINT /*uMsg*/, DWORD_PTR /*dwUser*/, DWORD_PTR /*dw1*/, DWORD_PTR /*dw2*/)
{
	HWND hWnd = GlobalManager::Instance().Timer().m_hMessageWnd;
	::PostMessage(hWnd, WM_USER_DEFINED_TIMER, 0, 0);
}

bool TimerManager::AddCancelableTimer(const std::weak_ptr<nbase::WeakFlag>& weakFlag, 
									  const TimerCallback& callback, 
									  uint32_t uElapse, 
								      int32_t iRepeatTime)
{
	ASSERT(uElapse > 0);

	TimerInfo pTimer;
	pTimer.timerCallback = callback;
	//计算出需要滴答多少次
	pTimer.uPerformanceCount = uElapse * m_timeFrequency.QuadPart / 1000;
	::QueryPerformanceCounter(&pTimer.dwEndPerformanceCounter);
	//计算出下次触发时的时钟滴答数
	pTimer.dwEndPerformanceCounter.QuadPart += pTimer.uPerformanceCount;
	pTimer.uRepeatTime = static_cast<uint32_t>(iRepeatTime);
	pTimer.weakFlag = weakFlag;
	m_aTimers.push(pTimer);
	
	if ((m_nTimerId == 0) || !m_bMinInterval) {
		KillTimerEvent();
		m_nTimerId = ::timeSetEvent(TIMER_INTERVAL, TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);
		ASSERT(m_nTimerId != 0);
		m_bMinInterval = true;
	}

	return true;
}

void TimerManager::Poll()
{
	LARGE_INTEGER currentTime;
	::QueryPerformanceCounter(&currentTime);

	while (!m_aTimers.empty()) {
		//首先删除已经取消的定时器
		if (m_aTimers.top().weakFlag.expired()) {
			m_aTimers.pop();
			continue;
		}
		LONGLONG detaTime = m_aTimers.top().dwEndPerformanceCounter.QuadPart - currentTime.QuadPart;
		if (detaTime <= 0) {
			//队列顶的定时器：已经达到定时器触发条件
			TimerInfo it = m_aTimers.top();
			m_aTimers.pop();

			if (!it.weakFlag.expired()) {
				it.timerCallback();
				bool rePush = false;
				if (it.uRepeatTime > 0) {
					it.uRepeatTime--;
					if (it.uRepeatTime > 0) {
						rePush = true;
					}
				}
				if (rePush) {
					//如果未达到触发次数限制，重新设置下次触发的滴答数
					TimerInfo rePushTimerInfo = it;
					rePushTimerInfo.dwEndPerformanceCounter.QuadPart = currentTime.QuadPart + it.uPerformanceCount;
					m_aTimers.push(rePushTimerInfo);
				}
			}
		}
		else if ((detaTime > 0) && (detaTime < m_timeFrequency.QuadPart)) {
			//队列顶的定时器触发时间在1秒以内, 使用最小间隔的精确计时器
			if (!m_bMinInterval) {
				KillTimerEvent();
				m_nTimerId = ::timeSetEvent(TIMER_INTERVAL, TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);
				ASSERT(m_nTimerId != 0);
				m_bMinInterval = true;
			}
			break;
		}
		else {
			//队列顶的定时器触发时间在秒以后，放大定时器的触发间隔时间，调整到定时器触发后的2毫秒（优化性能，避免系统定时器频繁触发）
			double newDetaTime = double(detaTime) * 1000 / m_timeFrequency.QuadPart;
			KillTimerEvent();
			m_nTimerId = ::timeSetEvent(int(newDetaTime + 2 * TIMER_PRECISION), TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);
			ASSERT(m_nTimerId != 0);
			m_bMinInterval = false;
			break;
		}
	}

	if (m_aTimers.empty()) {
		KillTimerEvent();
	}
}

void TimerManager::KillTimerEvent()
{
	if (m_nTimerId != 0) {
		::timeKillEvent(m_nTimerId);
		m_nTimerId = 0;
	}
}

void TimerManager::Clear()
{
	KillTimerEvent();
	if (m_hMessageWnd != nullptr) {
		::DestroyWindow(m_hMessageWnd);
		m_hMessageWnd = nullptr;
	}	
	while (!m_aTimers.empty()) {
		m_aTimers.pop();
	}
	m_bMinInterval = true;
}

}