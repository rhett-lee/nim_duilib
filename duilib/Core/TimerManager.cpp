#include "TimerManager.h"
#include "duilib/Core/GlobalManager.h"
#include <Mmsystem.h>

namespace ui 
{

#define WM_USER_DEFINED_TIMER	(WM_USER + 9999)
#define TIMER_INTERVAL	16
#define TIMER_PRECISION	1

TimerManager::TimerManager() : 
	m_hMessageWnd(nullptr),
	m_aTimers(),
	m_timeFrequency(),
	m_bMinPause(true),
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

bool TimerManager::AddCancelableTimer(const std::weak_ptr<nbase::WeakFlag>& weakFlag, const TIMERINFO::TimerCallback& callback, UINT uElapse, int iRepeatTime)
{
	ASSERT(uElapse > 0);

	TIMERINFO pTimer;
	pTimer.timerCallback = callback;
	pTimer.uPause = uElapse * m_timeFrequency.QuadPart / 1000;
	QueryPerformanceCounter(&pTimer.dwTimeEnd);
	pTimer.dwTimeEnd.QuadPart += pTimer.uPause;
	pTimer.iRepeatTime = iRepeatTime;
	pTimer.weakFlag = weakFlag;
	m_aTimers.push(pTimer);
	
	if (m_nTimerId == 0 || m_bMinPause == false) {
		timeKillEvent(m_nTimerId);
		m_nTimerId = ::timeSetEvent(TIMER_INTERVAL, TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);

		ASSERT(m_nTimerId);
		m_bMinPause = true;
	}

	return true;
}

void TimerManager::Poll()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	while (!m_aTimers.empty()) {
		LONGLONG detaTime = m_aTimers.top().dwTimeEnd.QuadPart - currentTime.QuadPart;
		if (detaTime <= 0) {
			TIMERINFO it = m_aTimers.top();
			m_aTimers.pop();

			if (!it.weakFlag.expired()) {
				it.timerCallback();
				bool rePush = false;
				if (it.iRepeatTime > 0) {
					it.iRepeatTime--;
					if (it.iRepeatTime > 0) {
						rePush = true;
					}
				}
				else if (it.iRepeatTime == REPEAT_FOREVER) {
					rePush = true;
				}

				if (rePush) {
					TIMERINFO rePushTimerInfo = it;
					rePushTimerInfo.dwTimeEnd.QuadPart = currentTime.QuadPart + it.uPause;
					m_aTimers.push(rePushTimerInfo);
				}
			}
		}
		else if (detaTime > 0 && detaTime < m_timeFrequency.QuadPart) {
			if (!m_bMinPause) {
				timeKillEvent(m_nTimerId);
				m_nTimerId = ::timeSetEvent(TIMER_INTERVAL, TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);
				ASSERT(m_nTimerId);
				m_bMinPause = true;
			}

			break;
		}
		else {
			double newDetaTime = double(detaTime) * 1000 / m_timeFrequency.QuadPart;
			timeKillEvent(m_nTimerId);
			m_nTimerId = ::timeSetEvent(int(newDetaTime + 2 * TIMER_PRECISION), TIMER_PRECISION, &TimerManager::TimeCallback, NULL, TIME_PERIODIC);
			ASSERT(m_nTimerId);
			m_bMinPause = false;
			break;
		}
	}

	if (m_aTimers.empty()) {
		::timeKillEvent(m_nTimerId);
		m_nTimerId = 0;
	}
}

void TimerManager::Clear()
{
	if (m_nTimerId != 0) {
		::timeKillEvent(m_nTimerId);
		m_nTimerId = 0;
	}
	if (m_hMessageWnd != nullptr) {
		::DestroyWindow(m_hMessageWnd);
		m_hMessageWnd = nullptr;
	}	
	while (!m_aTimers.empty()) {
		m_aTimers.pop();
	}
	m_bMinPause = false;
}

}