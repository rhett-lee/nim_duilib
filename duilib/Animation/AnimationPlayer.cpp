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

void AnimationPlayer::Reset()
{
    m_weakFlagOwner.Cancel();
    Init();
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
    m_reverseStart = false;
    m_bPlaying = false;
    m_frameIndex = 0;
    m_pEasingFunctions.reset();
    m_easingFunctionType = EasingFunctionType::EaseInOutCubic;
}

void AnimationPlayer::Start()
{
    m_weakFlagOwner.Cancel();
    if (m_reverseStart) {
        ReverseAllValue();
        m_reverseStart = false;
    }
    StartTimer();
}

void AnimationPlayer::Stop()
{
    m_weakFlagOwner.Cancel();
    m_bPlaying = false;
}

void AnimationPlayer::Continue()
{
    m_weakFlagOwner.Cancel();
    if (m_reverseStart) {
        ReverseAllValue();
        m_reverseStart = false;
    }
    StartTimer();
}

void AnimationPlayer::ReverseContinue()
{
    m_weakFlagOwner.Cancel();
    if (!m_reverseStart) {
        ReverseAllValue();
        m_reverseStart = true;
    }
    StartTimer();
}

void AnimationPlayer::StartTimer()
{
    if (m_endValue == m_startValue) {
        Complete();
        return;
    }

    int32_t timerIntervalMs = m_frameIntervalMillSeconds;
    if (timerIntervalMs <= 0) {
        timerIntervalMs = 1000 / 60;//默认按每秒60帧播放
    }

    int64_t totalMillSeconds = m_totalMillSeconds;
    if (totalMillSeconds <= 0) {
        totalMillSeconds = 180; //默认按动画总时常为180毫秒播放
    }

    //计算帧数
    int32_t frameCount = static_cast<int32_t>(totalMillSeconds / timerIntervalMs);
    if (frameCount < 1) {
        frameCount = 1;
    }
    m_bPlaying = true;
    m_frameIndex = 0;
    m_pEasingFunctions = std::make_unique<EasingFunctions>(m_startValue, m_endValue, frameCount, m_easingFunctionType);
    m_currentValue = m_startValue;

    auto playCallback = UiBind(&AnimationPlayer::Play, this);
    GlobalManager::Instance().Timer().AddTimer(m_weakFlagOwner.GetWeakFlag(), playCallback, (uint32_t)timerIntervalMs);
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
    int64_t newCurrentValue = m_pEasingFunctions->GetEasingValue(m_frameIndex);
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

}
