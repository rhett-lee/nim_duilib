#include "AnimationPlayer.h"
#include "duilib/Animation/EasingFunctions.h"
#include "duilib/Core/GlobalManager.h"
#include <cmath>

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
}

void AnimationPlayer::Clear()
{
    m_weakFlagOwner.Cancel();
    m_bPlaying = false;
    m_playCallback = nullptr;
    m_completeCallback = nullptr;
    m_pEasingFunctions.reset();
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
    m_startTimePoint = std::chrono::steady_clock::time_point();
    m_pEasingFunctions.reset();
    m_easingFunctionType = EasingFunctionType::EaseInOutCubic;
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
    m_weakFlagOwner.Cancel();
    if (m_bReversePlay) {
        ReverseAllValue();
        m_bReversePlay = false;
    }
    m_pEasingFunctions.reset();
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
        m_startTimePoint = std::chrono::steady_clock::now() - std::chrono::milliseconds(static_cast<int64_t>(nProgress * totalMillSeconds));
    }
    else {
        //重新开始
        m_frameIndex = 0;
        m_playProgress = 0.0;
        m_currentValue = m_startValue;
        m_startTimePoint = std::chrono::steady_clock::now();
    }
    auto playCallback = UiBind(&AnimationPlayer::Play, this);
    GlobalManager::Instance().Timer().AddTimer(m_weakFlagOwner.GetWeakFlag(), playCallback, (uint32_t)timerIntervalMs);

    //首次调用，初始化当前的值（避免延迟调用导致的错误，比如设置控件大小、位置时，必须做初始化，否则会出现异常）
    if (m_playCallback) {
        m_playCallback(m_currentValue);
    }
}

void AnimationPlayer::Play()
{
    if (m_pEasingFunctions == nullptr) {
        m_weakFlagOwner.Cancel();
        return;
    }
    const int32_t nFrameCount = m_pEasingFunctions->GetFrameCount();
    //按真实时间计算播放进度：定时器触发延迟或UI线程卡顿时，直接跳到当前应有的进度，
    //保证动画的总时长准确（原来的实现每次只把帧序号+1，卡顿会让动画整体变慢）
    double nProgress = 1.0;
    if (m_playTotalMillSeconds > 0) {
        const double nElapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_startTimePoint).count();
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
    AnimationPlayCallback playCallback = m_playCallback;
    if (playCallback && (newCurrentValue != m_currentValue)) {
        playCallback(newCurrentValue);
        if (aliveFlag.expired()) {
            // 回调中动画已被清理、停止或者对象已被销毁，不再访问成员变量
            return;
        }
    }
    if (m_pEasingFunctions == nullptr) {
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

void AnimationPlayer::Complete()
{
    // 先缓存回调，避免回调中 AnimationPlayer 被销毁后的悬空调用
    AnimationCompleteCallback completeCallback = m_completeCallback;
    m_currentValue = m_endValue;
    m_playProgress = 1.0;
    m_bPlaying = false;
    // 先释放 easing 函数资源，让回调中如需重启也能正确初始化
    m_pEasingFunctions.reset();
    m_weakFlagOwner.Cancel();
    if (completeCallback) {
        completeCallback();
    }
}

} //namespace ui
