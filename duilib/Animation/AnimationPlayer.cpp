#include "AnimationPlayer.h"
#include "duilib/Core/GlobalManager.h"

#define AP_NO_VALUE -1

namespace ui 
{

static bool IsZeroValue(double value) {
	return std::abs(value - 0) < 0.00001;
}

AnimationPlayerBase::AnimationPlayerBase(): 
	m_bFirstRun(true),
	m_animationType(AnimationType::kAnimationNone),
	m_playCallback(nullptr),
	m_completeCallback(nullptr)
{
	Init();
}

void AnimationPlayerBase::Reset()
{
	m_weakFlagOwner.Cancel();
	Init();
}

void AnimationPlayerBase::Clear()
{
	m_weakFlagOwner.Cancel();
	m_playCallback = nullptr;
	m_completeCallback = nullptr;
}

void AnimationPlayerBase::Init()
{
	m_startValue = 0;
	m_endValue = 0;
	m_currentValue = 0;
	m_totalMillSeconds = AP_NO_VALUE;
	m_palyedMillSeconds = 0;
	m_elapseMillSeconds = 0;
	m_reverseStart = false;
	m_bPlaying = false;
	m_startTime = std::chrono::high_resolution_clock::now();
}

void AnimationPlayerBase::Start()
{
	m_weakFlagOwner.Cancel();
	m_palyedMillSeconds = 0;
	m_reverseStart = false;
	StartTimer();
}

void AnimationPlayerBase::Stop()
{
	m_weakFlagOwner.Cancel();
}

void AnimationPlayerBase::Continue()
{
	m_weakFlagOwner.Cancel();
	if (m_reverseStart) {
		ReverseAllValue();
	}	

	m_reverseStart = false;
	StartTimer();
	m_bFirstRun = false;
}

void AnimationPlayerBase::ReverseContinue()
{
	m_weakFlagOwner.Cancel();
	if (!m_reverseStart) {
		ReverseAllValue();
	}		

	if (m_bFirstRun) {
		m_palyedMillSeconds = 0;
	}		

	m_reverseStart = true;
	StartTimer();
	m_bFirstRun = false;
}

void AnimationPlayerBase::StartTimer()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_bPlaying = true;
	if (m_endValue - m_startValue == 0) {
		Complete();
		return;
	}

	m_elapseMillSeconds = m_totalMillSeconds / std::abs(m_endValue - m_startValue);
	if (m_elapseMillSeconds == 0) {
		m_elapseMillSeconds = 1;
	}

	Play();
	auto playCallback = nbase::Bind(&AnimationPlayerBase::Play, this);
	ASSERT(m_elapseMillSeconds <= INT32_MAX);
	GlobalManager::Instance().Timer().AddCancelableTimer(m_weakFlagOwner.GetWeakFlag(), playCallback, (uint32_t)m_elapseMillSeconds, TimerManager::REPEAT_FOREVER);
}

void AnimationPlayerBase::Play()
{
	std::chrono::steady_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	auto thisTime = endTime - m_startTime; //播放耗时：微秒(千分之一毫秒)
	m_palyedMillSeconds += (thisTime.count() / 1000); //累计到已播放时间（毫秒）
	m_startTime = std::chrono::high_resolution_clock::now();

	int64_t newCurrentValue = GetCurrentValue();
    if (m_playCallback) {
        if (( (m_endValue > m_startValue) && (newCurrentValue >= m_endValue) ) ||
			( (m_endValue < m_startValue) && (newCurrentValue <= m_endValue) ) ) {
			//播放完成
            newCurrentValue = m_endValue;
            m_playCallback(newCurrentValue);
            Complete();
        }
        else {
            if (newCurrentValue != m_currentValue) {
                m_playCallback(newCurrentValue);
            }
        }
    }
    else {
        ASSERT(FALSE);
    }

	m_currentValue = newCurrentValue;
}

void AnimationPlayerBase::ReverseAllValue()
{
	std::swap(m_startValue, m_endValue);
	m_currentValue = m_startValue;
	m_palyedMillSeconds = m_totalMillSeconds - m_palyedMillSeconds;
	if (m_palyedMillSeconds < 0) {
		m_palyedMillSeconds = 0;
	}
}

void AnimationPlayerBase::Complete()
{
	if (m_completeCallback) {
		m_completeCallback();
	}		

	m_weakFlagOwner.Cancel();
	m_bPlaying = false;
}

AnimationPlayer::AnimationPlayer()
{
	Init();
}

void AnimationPlayer::Init()
{
	__super::Init();
	m_speedUpRatio = 0;
	m_speedDownRatio = 0;
	m_speedDownMillSeconds = 0;
	m_linerMillSeconds = 0;
	m_linearSpeed = 0;
	m_speedUpfactorA = 0;
	m_speedDownfactorA = 0;
	m_speedDownfactorB = 0;
	m_maxTotalMillSeconds = 1000000;
}

void AnimationPlayer::StartTimer()
{
	InitFactor();
	__super::StartTimer();
}

int64_t AnimationPlayer::GetCurrentValue() const
{
	if (m_palyedMillSeconds >= m_totalMillSeconds) {
		return m_endValue;
	}

	int64_t detaValue = 0;
	if (m_palyedMillSeconds <= m_speedUpMillSeconds) {
		detaValue = int64_t(m_speedUpfactorA * m_palyedMillSeconds * m_palyedMillSeconds);
	}
	else if (m_palyedMillSeconds <= (m_speedUpMillSeconds + m_linerMillSeconds)) {
		double linerTime = m_palyedMillSeconds - m_speedUpMillSeconds;
		detaValue = int64_t(m_speedUpfactorA * m_speedUpMillSeconds * m_speedUpMillSeconds + m_linearSpeed * linerTime);
	}
	else if (m_palyedMillSeconds <= m_totalMillSeconds) {
		double speedDownTime = m_palyedMillSeconds - m_speedUpMillSeconds - m_linerMillSeconds;
		detaValue = int64_t(m_speedUpfactorA * m_speedUpMillSeconds * m_speedUpMillSeconds + m_linearSpeed * m_linerMillSeconds
			+ m_speedDownfactorA * speedDownTime * speedDownTime + m_speedDownfactorB * speedDownTime);
	}
	else {
		ASSERT(FALSE); 
	}

	int64_t currentValue = 0;
	if (m_endValue > m_startValue) {
		currentValue = m_startValue + detaValue;
	}
	else {
		currentValue = m_startValue - detaValue;
	}
	
	return currentValue;
}

void AnimationPlayer::InitFactor()
{
	int64_t s = std::abs(m_endValue - m_startValue);
	if (m_speedUpRatio == 0 && m_speedDownRatio == 0) {	//liner
		ASSERT(m_totalMillSeconds == AP_NO_VALUE && !IsZeroValue(m_linearSpeed) || 
			   m_totalMillSeconds != AP_NO_VALUE && IsZeroValue(m_linearSpeed));
		if (m_totalMillSeconds == AP_NO_VALUE) {
			m_totalMillSeconds = static_cast<int64_t>(s / m_linearSpeed);
		}
		else {
			m_linearSpeed = 1.0 * s / m_totalMillSeconds;
		}
		m_linerMillSeconds = static_cast<double>(m_totalMillSeconds);
	}
	else {
		if (m_totalMillSeconds != AP_NO_VALUE) {
			if (!IsZeroValue(m_speedUpRatio)) {
				m_speedUpfactorA = s / ((m_speedUpRatio*m_speedUpRatio + (1 - m_speedUpRatio - m_speedDownRatio)*2*m_speedUpRatio + m_speedUpRatio*m_speedDownRatio)
					*m_totalMillSeconds*m_totalMillSeconds);
			}
			else if (m_speedDownRatio != 0) {
				m_speedDownfactorA = -s / ((m_speedDownRatio*m_speedDownRatio + (1 - m_speedUpRatio - m_speedDownRatio)*2*m_speedDownRatio + m_speedUpRatio*m_speedDownRatio)
					*m_totalMillSeconds*m_totalMillSeconds);
			}
			else {
				ASSERT(FALSE);
			}

		}
		double tmpValue = 0;
		if (!IsZeroValue(m_speedUpfactorA) && !IsZeroValue(m_speedUpRatio)) {
			tmpValue = m_speedUpfactorA * m_speedUpRatio;
			if (!IsZeroValue(m_speedDownRatio)) {
				m_speedDownfactorA = -m_speedUpfactorA * m_speedUpRatio / m_speedDownRatio;
			}
		}
		else if (!IsZeroValue(m_speedDownfactorA) && !IsZeroValue(m_speedDownRatio)) {
			tmpValue = -m_speedDownfactorA * m_speedDownRatio;
			if (!IsZeroValue(m_speedUpRatio)) {
				m_speedUpfactorA = -m_speedDownfactorA * m_speedDownRatio / m_speedUpRatio;
			}
		}
		else {
			ASSERT(FALSE);
		}
		if (m_totalMillSeconds == AP_NO_VALUE) {
			m_totalMillSeconds = int(std::sqrt(s / (m_speedUpfactorA*m_speedUpRatio*m_speedUpRatio + (1 - m_speedUpRatio - m_speedDownRatio) * 2 * tmpValue
				- m_speedDownfactorA*m_speedDownRatio*m_speedDownRatio) ));
			if (m_totalMillSeconds > m_maxTotalMillSeconds) {
				m_totalMillSeconds = m_maxTotalMillSeconds;
				InitFactor();
				return;
			}
		}

		m_linearSpeed = 2 * tmpValue * m_totalMillSeconds;
		m_speedDownfactorB = m_linearSpeed;

		m_speedUpMillSeconds = m_totalMillSeconds * m_speedUpRatio;
		m_speedDownMillSeconds = m_totalMillSeconds * m_speedDownRatio;
		m_linerMillSeconds = m_totalMillSeconds - m_speedUpMillSeconds - m_speedDownMillSeconds;
	}
}

}