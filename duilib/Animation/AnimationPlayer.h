#ifndef UI_ANIMATION_ANIMATIONPLAYER_H_
#define UI_ANIMATION_ANIMATIONPLAYER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "base/callback/callback.h"
#include <functional>

namespace ui 
{

typedef std::function<void (int64_t)> PlayCallback;
typedef std::function<void (void)> CompleteCallback;

class UILIB_API AnimationPlayerBase : public virtual nbase::SupportWeakCallback
{
public:
	AnimationPlayerBase();
	virtual ~AnimationPlayerBase() = default;

	AnimationPlayerBase(const AnimationPlayerBase& r) = delete;
	AnimationPlayerBase& operator=(const AnimationPlayerBase& r) = delete;

public:
	void SetAnimationType(int type) { m_animationType = type; };
	int GetAnimationType() const { return m_animationType; };
	
	void SetStartValue(int64_t startValue) { m_startValue = startValue; }
	int64_t GetStartValue() { return m_startValue; }

	void SetEndValue(int64_t endValue) { m_endValue = endValue; }
	int64_t GetEndValue() { return m_endValue; }

	void SetTotalMillSeconds(int totalMillSeconds) { m_totalMillSeconds = totalMillSeconds; }
	bool IsPlaying() { return m_bPlaying; }
	bool IsFirstRun() { return m_bFirstRun; }

	void SetCallback(const PlayCallback& callback) { m_playCallback = callback; }
	void SetCompleteCallback(const CompleteCallback& callback) { m_completeCallback = callback; }

	/** 停止并清理资源
	*/
	void Clear();
	void Reset();
	virtual void Init();
	virtual void Start();
	virtual void Stop();
	virtual void Continue();
	virtual void ReverseContinue();
	virtual void StartTimer();

	virtual int64_t GetCurrentValue() = 0;

private:
	void Play();
	void ReverseAllValue();
	void Complete();

protected:
	int m_animationType;

	//起始值
	int64_t m_startValue;

	//结束值
	int64_t m_endValue;

	//当前值
	int64_t m_currentValue;

	int64_t m_totalMillSeconds;
	double m_palyedMillSeconds;
	int64_t m_elapseMillSeconds;

	bool m_bFirstRun;
	bool m_bPlaying;
	bool m_reverseStart;

	PlayCallback m_playCallback;
	CompleteCallback m_completeCallback;
		
	LARGE_INTEGER m_currentTime;
	LARGE_INTEGER m_timeFrequency;
	
	nbase::WeakCallbackFlag m_weakFlagOwner;
};


class UILIB_API AnimationPlayer : 
	public AnimationPlayerBase
{
public:
	AnimationPlayer();
	virtual ~AnimationPlayer() = default;
	AnimationPlayer(const AnimationPlayer& r) = delete;
	AnimationPlayer& operator=(const AnimationPlayer& r) = delete;

public:
	virtual void Init() override;
	virtual void StartTimer() override;
	virtual int64_t GetCurrentValue() override;

	void InitFactor();
	virtual void SetSpeedUpfactorA(double factorA) { m_speedUpfactorA = factorA; }
	virtual void SetSpeedDownfactorA(double factorA) { m_speedDownfactorA = factorA; }

	void SetMaxTotalMillSeconds(int maxTotalMillSeconds) { m_maxTotalMillSeconds = maxTotalMillSeconds; }
	void SetLinearSpeed(double linearSpeed) { m_linearSpeed = linearSpeed; }
	void SetSpeedUpRatio(double speedUpRatio) { m_speedUpRatio = speedUpRatio; }
	void SetSpeedDownRatio(double speedDownRatio) { m_speedDownRatio = speedDownRatio; }

private:
	double m_speedUpRatio;
	double m_speedDownRatio;
	double m_speedUpMillSeconds;

	double m_linearSpeed;
	double m_linerMillSeconds;

	double m_speedDownMillSeconds;
	
	double m_speedUpfactorA;
	double m_speedDownfactorA;
	double m_speedDownfactorB;

	int m_maxTotalMillSeconds;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONPLAYER_H_
