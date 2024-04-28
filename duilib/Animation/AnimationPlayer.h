#ifndef UI_ANIMATION_ANIMATIONPLAYER_H_
#define UI_ANIMATION_ANIMATIONPLAYER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "base/callback/callback.h"
#include <functional>
#include <chrono>

namespace ui 
{

typedef std::function<void (int64_t)> PlayCallback;		//播放回调函数
typedef std::function<void (void)> CompleteCallback;	//播放完成回调函数

/** 动画播放器的基类接口
*/
class UILIB_API AnimationPlayerBase : public virtual nbase::SupportWeakCallback
{
public:
	AnimationPlayerBase();
	virtual ~AnimationPlayerBase() = default;

	AnimationPlayerBase(const AnimationPlayerBase& r) = delete;
	AnimationPlayerBase& operator=(const AnimationPlayerBase& r) = delete;

public:
	/** 设置动画类型
	*/
	void SetAnimationType(AnimationType type) { m_animationType = type; };

	/** 获取动画类型
	*/
	AnimationType GetAnimationType() const { return m_animationType; };
	
	/** 设置播放起始值
	*/
	void SetStartValue(int64_t startValue) { m_startValue = startValue; }

	/** 获取播放起始值
	*/
	int64_t GetStartValue() const { return m_startValue; }

	/** 设置播放结束值
	*/
	void SetEndValue(int64_t endValue) { m_endValue = endValue; }

	/** 获取播放结束值
	*/
	int64_t GetEndValue() const { return m_endValue; }

	/** 设置总的播放时间（毫秒）
	*/
	void SetTotalMillSeconds(int64_t totalMillSeconds) { m_totalMillSeconds = totalMillSeconds; }

	/** 获取总的播放时间（毫秒）
	*/
	int64_t GetTotalMillSeconds() const { return m_totalMillSeconds; }

	/** 设置播放回调函数
	*/
	void SetCallback(const PlayCallback& callback) { m_playCallback = callback; }

	/** 设置播放完成回调函数
	*/
	void SetCompleteCallback(const CompleteCallback& callback) { m_completeCallback = callback; }

	/** 停止并清理资源
	*/
	void Clear();

	/** 重置状态
	*/
	void Reset();

	/** 是否正在播放中
	*/
	bool IsPlaying() const { return m_bPlaying; }

	/** 是否第一次播放
	*/
	bool IsFirstRun() const { return m_bFirstRun; }

	/** 初始化
	*/
	virtual void Init();

	/** 动画开始
	*/
	virtual void Start();

	/** 动画结束
	*/
	virtual void Stop();

	/** 动画继续（从起始值 到 结束值）
	*/
	virtual void Continue();

	/** 动画反向继续（从结束值 到 起始值，反向动画）
	*/
	virtual void ReverseContinue();

	/** 启动动画定时器
	*/
	virtual void StartTimer();

	/** 获取动画当前值
	*/
	virtual int64_t GetCurrentValue() const = 0;

private:
	/** 播放一次动画（在定时器中触发调用）
	*/
	void Play();

	/** 交换起始值和结束值
	*/
	void ReverseAllValue();

	/** 完成播放动画，并触发播放完成回调函数
	*/
	void Complete();

protected:
	/** 动画类型
	*/
	AnimationType m_animationType;

	/** 起始值
	*/
	int64_t m_startValue;

	/** 结束值
	*/
	int64_t m_endValue;

	/** 当前值
	*/
	int64_t m_currentValue;

	/** 播放总的时间（毫秒）
	*/
	int64_t m_totalMillSeconds;

	/** 动画已经播放的时间（毫秒）
	*/
	int64_t m_palyedMillSeconds;

	/** 动画播放的时间间隔，即定时器的触发时间间隔（毫秒）
	*/
	int64_t m_elapseMillSeconds;

	/** 是否第一次播放
	*/
	bool m_bFirstRun;

	/** 是否正在播放中
	*/
	bool m_bPlaying;

	/** 是否正在反向播放
	*/
	bool m_reverseStart;

	/** 播放回调函数
	*/
	PlayCallback m_playCallback;

	/** 播放完成回调函数
	*/
	CompleteCallback m_completeCallback;
	
	/** 播放的开始时间戳
	*/
	std::chrono::steady_clock::time_point m_startTime;
	
	/** 定时器终止标志
	*/
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
	/** 初始化
	*/
	virtual void Init() override;

	/** 启动动画定时器
	*/
	virtual void StartTimer() override;

	/** 获取动画当前值
	*/
	virtual int64_t GetCurrentValue() const override;

	/** 设置加速因子A
	*/
	virtual void SetSpeedUpfactorA(double factorA) { m_speedUpfactorA = factorA; }

	/** 设置减速因子A
	*/
	virtual void SetSpeedDownfactorA(double factorA) { m_speedDownfactorA = factorA; }

	/** 设置最大播放时间（毫秒）
	*/
	void SetMaxTotalMillSeconds(int64_t maxTotalMillSeconds) { m_maxTotalMillSeconds = maxTotalMillSeconds; }

	/** 设置线性速度（值/毫秒）
	*/
	void SetLinearSpeed(double linearSpeed) { m_linearSpeed = linearSpeed; }

	/** 设置加速比率
	*/
	void SetSpeedUpRatio(double speedUpRatio) { m_speedUpRatio = speedUpRatio; }

	/** 设置减速比率
	*/
	void SetSpeedDownRatio(double speedDownRatio) { m_speedDownRatio = speedDownRatio; }

private:
	/** 初始化动画因子数据
	*/
	void InitFactor();

private:
	/** 加速比率
	*/
	double m_speedUpRatio;

	/** 加速动画总时间（毫秒）
	*/
	double m_speedUpMillSeconds;

	/** 减速比率
	*/
	double m_speedDownRatio;

	/** 减速动画总时间（毫秒）
	*/
	double m_speedDownMillSeconds;

	/** 线性速度（值/毫秒）
	*/
	double m_linearSpeed;

	/** 线性加速动画总时间（毫秒）
	*/
	double m_linerMillSeconds;
	
	/** 加速因子A
	*/
	double m_speedUpfactorA;

	/** 减速因子A
	*/
	double m_speedDownfactorA;

	/** 减速因子B
	*/
	double m_speedDownfactorB;

	/** 最大播放时间（毫秒）
	*/
	int64_t m_maxTotalMillSeconds;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONPLAYER_H_
