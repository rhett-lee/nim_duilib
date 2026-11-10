#include "AnimationScheduler.h"
#include "AnimationPlayer.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include <chrono>
#include <functional>

namespace ui 
{

namespace
{
    /** 动画刷新的时间间隔（毫秒），约60帧/秒
    */
    const uint32_t kAnimationTimerIntervalMs = 16;
}

AnimationScheduler& AnimationScheduler::Instance()
{
    static AnimationScheduler s_animationScheduler;
    return s_animationScheduler;
}

AnimationScheduler::AnimationScheduler() = default;

AnimationScheduler::~AnimationScheduler()
{
    m_timerFlag.Cancel();
    m_players.clear();
    m_arrangeList.clear();
}

int64_t AnimationScheduler::GetTimeMillSeconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void AnimationScheduler::Register(AnimationPlayer* pPlayer, const std::weak_ptr<WeakFlag>& weakFlag)
{
    if (pPlayer == nullptr) {
        return;
    }
    for (TPlayerItem& playerItem : m_players) {
        if (playerItem.pPlayer == pPlayer) {
            playerItem.weakFlag = weakFlag;
            return;
        }
    }
    TPlayerItem playerItem;
    playerItem.pPlayer = pPlayer;
    playerItem.weakFlag = weakFlag;
    m_players.push_back(playerItem);

    if (!m_bTimerStarted) {
        m_bTimerStarted = true;
        TimerCallback timerCallback = std::bind(&AnimationScheduler::OnTimer, this);
        GlobalManager::Instance().Timer().AddTimer(m_timerFlag.GetWeakFlag(), timerCallback, kAnimationTimerIntervalMs);
    }
}

void AnimationScheduler::Unregister(AnimationPlayer* pPlayer)
{
    if (pPlayer == nullptr) {
        return;
    }
    for (auto iter = m_players.begin(); iter != m_players.end(); ++iter) {
        if (iter->pPlayer == pPlayer) {
            m_players.erase(iter);
            break;
        }
    }
    if (m_players.empty() && m_bTimerStarted) {
        //没有正在播放的动画，停止定时器，避免空转
        m_bTimerStarted = false;
        m_timerFlag.Cancel();
    }
}

void AnimationScheduler::RequestArrange(const ControlPtr& pControl)
{
    if (pControl == nullptr) {
        return;
    }
    for (const ControlPtr& arrangeControl : m_arrangeList) {
        if (arrangeControl == pControl) {
            //同一帧内已经请求过，去重
            return;
        }
    }
    m_arrangeList.push_back(pControl);
    if (!m_bInTick) {
        //不在动画刷新过程中，立即重排
        FlushArrange();
    }
}

void AnimationScheduler::OnTimer()
{
    const int64_t nNowMillSeconds = GetTimeMillSeconds();

    m_bInTick = true;
    Tick(nNowMillSeconds);
    m_bInTick = false;

    //动画刷新结束后，统一执行本帧内累计的控件重排（每帧每个控件只重排一次）
    FlushArrange();
}

void AnimationScheduler::Tick(int64_t nNowMillSeconds)
{
    if (m_players.empty()) {
        return;
    }
    //先移除已经失效的动画（动画播放器已销毁）
    for (auto iter = m_players.begin(); iter != m_players.end(); ) {
        if (iter->weakFlag.expired() || (iter->pPlayer == nullptr)) {
            iter = m_players.erase(iter);
        }
        else {
            ++iter;
        }
    }
    if (m_players.empty()) {
        return;
    }

    //复制一份列表后再遍历：动画的回调中可能注册/注销其他动画，甚至销毁控件
    const std::vector<TPlayerItem> tickList = m_players;
    for (const TPlayerItem& playerItem : tickList) {
        if ((playerItem.pPlayer != nullptr) && !playerItem.weakFlag.expired()) {
            playerItem.pPlayer->OnTick(nNowMillSeconds);
        }
    }
}

void AnimationScheduler::FlushArrange()
{
    if (m_arrangeList.empty()) {
        return;
    }
    const std::vector<ControlPtr> arrangeList = m_arrangeList;
    m_arrangeList.clear();
    for (const ControlPtr& pControl : arrangeList) {
        Control* pRawControl = pControl.get();
        if (pRawControl != nullptr) {
            pRawControl->ArrangeAncestor();
        }
    }
}

} // namespace ui
