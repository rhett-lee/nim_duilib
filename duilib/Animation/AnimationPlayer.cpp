#include "AnimationPlayer.h"
#include "AnimationScheduler.h"
#include "duilib/Animation/EasingFunctions.h"
#include <cmath>
#include <limits>

#define AP_NO_VALUE -1

namespace ui 
{
AnimationPlayer::AnimationPlayer():
    m_playCallback(nullptr),
    m_completeCallback(nullptr),
    m_animationType(AnimationType::kAnimationNone)
{
    Init();
}

AnimationPlayer::~AnimationPlayer()
{
    AnimationScheduler::Instance().Unregister(this);
}

void AnimationPlayer::Clear()
{
    AnimationScheduler::Instance().Unregister(this);
    m_weakFlagOwner.Cancel();
    m_bPlaying = false;
    m_playCallback = nullptr;
    m_completeCallback = nullptr;
    m_pEasingFunctions.reset();
    m_pSpringSimulation.reset();
    m_pInertiaSimulation.reset();
    m_playProgress = 0.0;
}

void AnimationPlayer::Init()
{
    m_startValue = 0;
    m_endValue = 0;
    m_currentValue = 0;
    m_totalMillSeconds = -1;
    m_frameIntervalMillSeconds = -1;
    m_bReversePlay = false;
    m_bPlaying = false;
    m_frameIndex = 0;
    m_playProgress = 0.0;
    m_playTotalMillSeconds = 0;
    m_startTimeMillSeconds = 0;
    m_pEasingFunctions.reset();
    m_easingFunctionType = EasingFunctionType::EaseInOutCubic;

    //物理动画的默认参数（0/<0 表示使用 PhysicsAnimation 的全局默认值）
    m_physicsType = AnimationPhysicsType::kNone;
    m_springStiffness = 0.0;
    m_springDamping = 0.0;
    m_springMass = 0.0;
    m_inertiaVelocity = 0.0;
    m_inertiaFriction = -1.0;
    m_inertiaMinValue = std::numeric_limits<int32_t>::min();
    m_inertiaMaxValue = std::numeric_limits<int32_t>::max();
    m_pSpringSimulation.reset();
    m_pInertiaSimulation.reset();
    m_physicsLastTimeMillSeconds = 0;
}

void AnimationPlayer::SetSpringParameters(double stiffness, double damping, double mass)
{
    m_springStiffness = (stiffness > 0.0) ? stiffness : 0.0;
    m_springDamping = (damping > 0.0) ? damping : 0.0;
    m_springMass = (mass > 0.0) ? mass : 0.0;
}

void AnimationPlayer::SetInertiaParameters(double initialVelocity, double friction)
{
    m_inertiaVelocity = initialVelocity;
    m_inertiaFriction = (friction > 0.0) ? friction : -1.0;
}

void AnimationPlayer::SetInertiaValueRange(int32_t minValue, int32_t maxValue)
{
    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
    }
    m_inertiaMinValue = minValue;
    m_inertiaMaxValue = maxValue;
}

bool AnimationPlayer::IsPhysicsEnabled() const
{
    return (m_physicsType != AnimationPhysicsType::kNone) && PhysicsAnimation::IsEnabled();
}

void AnimationPlayer::SetEndValue(int32_t endValue)
{
    if (m_pSpringSimulation != nullptr) {
        //弹簧动画播放中：直接修改目标值，从当前状态平滑过渡，不会回跳
        m_endValue = endValue;
        m_pSpringSimulation->SetTarget(static_cast<double>(endValue));
        return;
    }
    if (!m_bPlaying || (m_pEasingFunctions == nullptr) || m_bReversePlay) {
        //未在正向播放中，直接更新结束值
        m_endValue = endValue;
        return;
    }
    if (endValue == m_endValue) {
        return;
    }
    Retarget(endValue);
}

void AnimationPlayer::Retarget(int32_t nNewEndValue)
{
    //计算当前的真实播放进度
    double nProgress = 1.0;
    if (m_playTotalMillSeconds > 0) {
        const double nElapsedMs = static_cast<double>(AnimationScheduler::GetTimeMillSeconds() - m_startTimeMillSeconds);
        nProgress = nElapsedMs / static_cast<double>(m_playTotalMillSeconds);
    }
    if (nProgress < 0.0) {
        nProgress = 0.0;
    }
    if (nProgress > 1.0) {
        nProgress = 1.0;
    }

    int32_t nFrameIntervalMs = m_frameIntervalMillSeconds;
    if (nFrameIntervalMs <= 0) {
        nFrameIntervalMs = 1000 / 60;
    }
    //使用剩余的播放时间播放到新的结束值
    int32_t nRemainMillSeconds = static_cast<int32_t>(std::llround((1.0 - nProgress) * static_cast<double>(m_playTotalMillSeconds)));
    if (nRemainMillSeconds <= nFrameIntervalMs) {
        nRemainMillSeconds = nFrameIntervalMs;
    }
    int32_t nFrameCount = nRemainMillSeconds / nFrameIntervalMs;
    if (nFrameCount < 1) {
        nFrameCount = 1;
    }

    //以当前播放值作为新起点，避免数值回跳
    m_startValue = m_currentValue;
    m_endValue = nNewEndValue;
    m_playTotalMillSeconds = nRemainMillSeconds;
    m_playProgress = 0.0;
    m_frameIndex = 0;
    m_startTimeMillSeconds = AnimationScheduler::GetTimeMillSeconds();
    m_pEasingFunctions = std::make_unique<EasingFunctions>(m_startValue, m_endValue, nFrameCount, m_easingFunctionType);
}

void AnimationPlayer::Start()
{
    m_weakFlagOwner.Cancel();
    if (m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = false;
    }
    StartTimer(false, false);
}

void AnimationPlayer::Stop()
{
    AnimationScheduler::Instance().Unregister(this);
    m_weakFlagOwner.Cancel();
    if (m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = false;
    }
    m_pEasingFunctions.reset();
    m_pSpringSimulation.reset();
    m_pInertiaSimulation.reset();
    m_frameIndex = 0;
    m_playProgress = 0.0;
    m_currentValue = m_startValue;
    m_bPlaying = false;
}

void AnimationPlayer::Continue()
{
    const bool bContinueMode = IsPlaying();
    const bool bOldReversePlay = m_bReversePlay;
    m_weakFlagOwner.Cancel();
    if (m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = false;
    }    
    StartTimer(bContinueMode, bOldReversePlay);
}

void AnimationPlayer::ReverseStart()
{
    m_weakFlagOwner.Cancel();
    if (!m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = true;
    }
    StartTimer(false, false);
}

void AnimationPlayer::ReverseContinue()
{
    const bool bContinueMode = IsPlaying();
    const bool bOldReversePlay = m_bReversePlay;
    m_weakFlagOwner.Cancel();
    if (!m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = true;
    }    
    StartTimer(bContinueMode, bOldReversePlay);
}

void AnimationPlayer::StartTimer(bool bContinueMode, bool bOldReversePlay)
{
    if (IsPhysicsEnabled()) {
        //物理动画：惯性动画没有明确的目标值，起止值相同也仍然需要按初速度滑行
        StartPhysics();
        return;
    }
    if (m_endValue == m_startValue) {
        Complete();
        return;
    }

    int32_t timerIntervalMs = m_frameIntervalMillSeconds;
    if (timerIntervalMs <= 0) {
        timerIntervalMs = 1000 / 60;//默认按每秒60帧播放
    }

    int32_t totalMillSeconds = m_totalMillSeconds;
    if (totalMillSeconds <= 0) {
        totalMillSeconds = 180; //默认按动画总时常为180毫秒播放
    }

    //计算帧数
    int32_t frameCount = totalMillSeconds / timerIntervalMs;
    if (frameCount < 1) {
        frameCount = 1;
    }

    //检查是否应继续操作（不重新开始播放，而是继续操作）
    if (m_pEasingFunctions == nullptr) {
        bContinueMode = false;
    }
    if (bContinueMode && (m_pEasingFunctions != nullptr)) {
        int32_t nStartValue = m_startValue;
        int32_t nEndValue = m_endValue;
        if (bOldReversePlay != m_bReversePlay) {
            std::swap(nStartValue, nEndValue);
        }
        if ((m_pEasingFunctions->GetFrameCount() != frameCount) ||
            (m_pEasingFunctions->GetStartValue() != nStartValue) ||
            (m_pEasingFunctions->GetEndValue() != nEndValue)) {
            //重要的参数已经变化，重新开始
            bContinueMode = false;
        }
    }
    m_bPlaying = true;
    m_playTotalMillSeconds = totalMillSeconds;
    m_pEasingFunctions = std::make_unique<EasingFunctions>(m_startValue, m_endValue, frameCount, m_easingFunctionType);
    if (bContinueMode) {
        //继续上次的开始播放
        double nProgress = m_playProgress;
        if (bOldReversePlay != m_bReversePlay) {
            //切换播放方向时，进度取反（使用浮点进度，避免整数帧取整导致的数值跳变）
            nProgress = 1.0 - nProgress;
        }
        if (nProgress < 0.0) {
            nProgress = 0.0;
        }
        if (nProgress > 1.0) {
            nProgress = 1.0;
        }
        m_playProgress = nProgress;
        m_frameIndex = static_cast<int32_t>(std::round(nProgress * static_cast<double>(frameCount)));
        if (m_frameIndex > frameCount) {
            m_frameIndex = frameCount;
        }
        m_currentValue = m_pEasingFunctions->GetEasingValue(nProgress);
        //按已播放的进度反推播放起点时间，保证剩余时长与真实时间一致
        m_startTimeMillSeconds = AnimationScheduler::GetTimeMillSeconds() - static_cast<int64_t>(nProgress * totalMillSeconds);
    }
    else {
        //重新开始
        m_frameIndex = 0;
        m_playProgress = 0.0;
        m_currentValue = m_startValue;
        m_startTimeMillSeconds = AnimationScheduler::GetTimeMillSeconds();
    }
    //注册到动画调度器，由全局唯一的定时器统一驱动（同一帧内所有动画使用相同的时间基准）
    AnimationScheduler::Instance().Register(this, m_weakFlagOwner.GetWeakFlag());

    //首次调用，初始化当前的值（避免延迟调用导致的错误，比如设置控件大小、位置时，必须做初始化，否则会出现异常）
    if (m_playCallback) {
        m_playCallback(m_currentValue);
    }
}

void AnimationPlayer::StartPhysics()
{
    m_bPlaying = true;
    m_frameIndex = 0;
    m_playProgress = 0.0;
    m_currentValue = m_startValue;
    m_playTotalMillSeconds = m_totalMillSeconds;
    if (m_playTotalMillSeconds <= 0) {
        //物理动画没有固定时长，这里只作为兜底上限（正常情况下会先自然静止）
        m_playTotalMillSeconds = 3000;
    }
    m_startTimeMillSeconds = AnimationScheduler::GetTimeMillSeconds();
    m_physicsLastTimeMillSeconds = m_startTimeMillSeconds;
    m_pEasingFunctions.reset();
    m_pSpringSimulation.reset();
    m_pInertiaSimulation.reset();

    if (m_physicsType == AnimationPhysicsType::kSpring) {
        m_pSpringSimulation = std::make_unique<SpringSimulation>();
        if ((m_springStiffness > 0.0) && (m_springDamping > 0.0) && (m_springMass > 0.0)) {
            m_pSpringSimulation->SetParameters(m_springStiffness, m_springDamping, m_springMass);
        }
        else {
            double stiffness = 0.0;
            double damping = 0.0;
            double mass = 0.0;
            PhysicsAnimation::GetDefaultSpringParameters(stiffness, damping, mass);
            m_pSpringSimulation->SetParameters(stiffness, damping, mass);
        }
        m_pSpringSimulation->Reset(static_cast<double>(m_startValue), static_cast<double>(m_endValue));
    }
    else if (m_physicsType == AnimationPhysicsType::kInertia) {
        double friction = m_inertiaFriction;
        double minVelocity = 0.0;
        double defaultFriction = 0.0;
        PhysicsAnimation::GetDefaultInertiaParameters(defaultFriction, minVelocity);
        if (friction <= 0.0) {
            friction = defaultFriction;
        }
        m_pInertiaSimulation = std::make_unique<InertiaSimulation>();
        m_pInertiaSimulation->SetParameters(friction, minVelocity, m_playTotalMillSeconds);
        m_pInertiaSimulation->SetValueRange(static_cast<double>(m_inertiaMinValue),
                                            static_cast<double>(m_inertiaMaxValue));
        m_pInertiaSimulation->Reset(static_cast<double>(m_startValue), m_inertiaVelocity);
    }

    if (!HasPhysicsSimulation()) {
        //没有可用的物理积分器，按普通动画完成处理
        Complete();
        return;
    }
    //注册到动画调度器，由全局唯一的定时器统一驱动
    AnimationScheduler::Instance().Register(this, m_weakFlagOwner.GetWeakFlag());

    //首次调用，初始化当前的值
    if (m_playCallback) {
        m_playCallback(m_currentValue);
    }
}

void AnimationPlayer::OnTickPhysics(int64_t nNowMillSeconds)
{
    double deltaSeconds = static_cast<double>(nNowMillSeconds - m_physicsLastTimeMillSeconds) / 1000.0;
    m_physicsLastTimeMillSeconds = nNowMillSeconds;
    if (deltaSeconds <= 0.0) {
        return;
    }
    //UI线程卡顿时限制单帧步进，避免物理积分产生过大的跳变
    if (deltaSeconds > 0.1) {
        deltaSeconds = 0.1;
    }

    double dValue = static_cast<double>(m_currentValue);
    bool bFinished = false;
    if (m_pSpringSimulation != nullptr) {
        bFinished = !m_pSpringSimulation->Advance(deltaSeconds, dValue);
    }
    else if (m_pInertiaSimulation != nullptr) {
        bFinished = !m_pInertiaSimulation->Advance(deltaSeconds, dValue);
    }
    const int32_t newCurrentValue = static_cast<int32_t>(std::llround(dValue));

    //回调函数的副本与对象存活标志：回调中可能销毁 AnimationPlayer（控件被删除），
    //回调返回后不能直接访问任何成员变量，只能通过本地的存活标志判断对象是否仍然有效
    const std::weak_ptr<WeakFlag> aliveFlag = m_weakFlagOwner.GetWeakFlag();
    const int64_t nStartTimeBefore = m_startTimeMillSeconds;
    SpringSimulation* pSpringBefore = m_pSpringSimulation.get();
    InertiaSimulation* pInertiaBefore = m_pInertiaSimulation.get();
    AnimationPlayCallback playCallback = m_playCallback;
    if (playCallback && (newCurrentValue != m_currentValue)) {
        playCallback(newCurrentValue);
        if (aliveFlag.expired()) {
            //回调中动画已被清理、停止或者对象已被销毁，不再访问成员变量
            return;
        }
        if ((m_pSpringSimulation.get() != pSpringBefore) ||
            (m_pInertiaSimulation.get() != pInertiaBefore) ||
            (m_startTimeMillSeconds != nStartTimeBefore)) {
            //回调中重新启动了动画，本次计算结果作废
            return;
        }
    }
    if (!HasPhysicsSimulation()) {
        AnimationScheduler::Instance().Unregister(this);
        m_weakFlagOwner.Cancel();
        return;
    }
    m_currentValue = newCurrentValue;
    //记录近似进度（物理动画没有固定时长，仅供外部查询参考）
    m_playProgress = (m_playTotalMillSeconds > 0) ?
        (static_cast<double>(nNowMillSeconds - m_startTimeMillSeconds) / static_cast<double>(m_playTotalMillSeconds)) : 0.0;
    if (m_playProgress > 1.0) {
        m_playProgress = 1.0;
    }
    if (bFinished) {
        //弹簧动画结束时对齐到目标值；惯性动画保留停止时的实际值（如滚动到边界）
        Complete(m_pSpringSimulation != nullptr);
    }
}

void AnimationPlayer::OnTick(int64_t nNowMillSeconds)
{
    if (HasPhysicsSimulation()) {
        OnTickPhysics(nNowMillSeconds);
        return;
    }
    if (m_pEasingFunctions == nullptr) {
        AnimationScheduler::Instance().Unregister(this);
        return;
    }
    const int32_t nFrameCount = m_pEasingFunctions->GetFrameCount();
    //按真实时间计算播放进度：定时器触发延迟或UI线程卡顿时，直接跳到当前应有的进度，
    //保证动画的总时长准确（原来的实现每次只把帧序号+1，卡顿会让动画整体变慢）
    double nProgress = 1.0;
    if (m_playTotalMillSeconds > 0) {
        const double nElapsedMs = static_cast<double>(nNowMillSeconds - m_startTimeMillSeconds);
        nProgress = nElapsedMs / static_cast<double>(m_playTotalMillSeconds);
    }
    if (nProgress < 0.0) {
        nProgress = 0.0;
    }
    if (nProgress > 1.0) {
        nProgress = 1.0;
    }
    //记录当前进度（回调中可能触发反向/继续播放，需要用到该进度做平滑衔接）
    m_playProgress = nProgress;
    //同步帧序号，供反向播放/继续播放时反推进度使用
    m_frameIndex = static_cast<int32_t>(std::round(nProgress * static_cast<double>(nFrameCount)));
    if (m_frameIndex > nFrameCount) {
        m_frameIndex = nFrameCount;
    }
    const int32_t newCurrentValue = m_pEasingFunctions->GetEasingValue(nProgress);

    // 回调函数的副本与对象存活标志：回调中可能销毁 AnimationPlayer（控件被删除），
    // 回调返回后不能直接访问任何成员变量，只能通过本地的存活标志判断对象是否仍然有效
    const std::weak_ptr<WeakFlag> aliveFlag = m_weakFlagOwner.GetWeakFlag();
    //记录本次播放的状态标记：回调中可能重启动画（继续/反向播放），
    //此时本帧计算出的进度已经作废，不能再按 nProgress 判定播放完成
    const int64_t nStartTimeBefore = m_startTimeMillSeconds;
    const EasingFunctions* pEasingBefore = m_pEasingFunctions.get();
    AnimationPlayCallback playCallback = m_playCallback;
    if (playCallback && (newCurrentValue != m_currentValue)) {
        playCallback(newCurrentValue);
        if (aliveFlag.expired()) {
            // 回调中动画已被清理、停止或者对象已被销毁，不再访问成员变量
            return;
        }
        if ((m_pEasingFunctions.get() != pEasingBefore) || (m_startTimeMillSeconds != nStartTimeBefore)) {
            //回调中重新启动了动画，本次进度作废，等下一帧按新的起始时间重新计算
            return;
        }
    }
    if (m_pEasingFunctions == nullptr) {
        AnimationScheduler::Instance().Unregister(this);
        m_weakFlagOwner.Cancel();
        return;
    }
    m_currentValue = newCurrentValue;
    if (nProgress >= 1.0) {
        //播放完成（按进度判断，确保结束时取到精确的结束值）
        Complete();
    }
}

void AnimationPlayer::ReverseAllValue()
{
    std::swap(m_startValue, m_endValue);
    m_currentValue = m_startValue;
}

void AnimationPlayer::Complete(bool bSnapToEnd)
{
    // 先缓存回调，避免回调中 AnimationPlayer 被销毁后的悬空调用
    AnimationCompleteCallback completeCallback = m_completeCallback;
    if (bSnapToEnd) {
        m_currentValue = m_endValue;
    }
    m_playProgress = 1.0;
    m_bPlaying = false;
    // 先释放 easing 函数与物理积分器资源，让回调中如需重启也能正确初始化
    m_pEasingFunctions.reset();
    m_pSpringSimulation.reset();
    m_pInertiaSimulation.reset();
    AnimationScheduler::Instance().Unregister(this);
    m_weakFlagOwner.Cancel();
    if (completeCallback) {
        completeCallback();
    }
}

} //namespace ui
