#include "AnimationPlayer.h"
#include "duilib/Animation/EasingFunctions.h"
#include "duilib/Core/GlobalManager.h"

#define AP_NO_VALUE -1

namespace ui 
{
AnimationPlayer::AnimationPlayer():
    m_animationType(AnimationType::kAnimationNone),
    m_playCallback(nullptr),
    m_completeCallback(nullptr)
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
    m_pEasingFunctions = std::make_unique<EasingFunctions>(m_startValue, m_endValue, frameCount, m_easingFunctionType);
    if (bContinueMode && (m_frameIndex >= 0) && (m_frameIndex <= frameCount)) {
        //继续上次的开始播放
        if (bOldReversePlay != m_bReversePlay) {
            m_frameIndex = frameCount - m_frameIndex;
        }
        m_currentValue = m_pEasingFunctions->GetEasingValue(m_frameIndex);
    }
    else {
        //重新开始
        m_frameIndex = 0;
        m_currentValue = m_startValue;
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
    ++m_frameIndex;
    if (m_frameIndex > m_pEasingFunctions->GetFrameCount()) {
        m_frameIndex = m_pEasingFunctions->GetFrameCount();
    }
    int32_t newCurrentValue = m_pEasingFunctions->GetEasingValue(m_frameIndex);
    if (m_playCallback) {
        if (newCurrentValue != m_currentValue) {
            m_playCallback(newCurrentValue);
        }
    }
    m_currentValue = newCurrentValue;
    if (m_frameIndex == m_pEasingFunctions->GetFrameCount()) {
        //播放完成
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
    m_weakFlagOwner.Cancel();
    m_bPlaying = false;
    m_currentValue = m_endValue;
    if (m_completeCallback) {
        m_completeCallback();
    }
}

} //namespace ui
