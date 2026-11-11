#include "PopupAnimation.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Core/WindowBase.h"

namespace ui
{

namespace
{
    //弹出层动画总开关（默认开启；关闭后弹出层恢复为瞬时显示）
    bool g_bPopupAnimationEnabled = true;

    //淡入时长（毫秒）
    int32_t g_nPopupAnimationTotalMillSeconds = 120;

    //淡入使用的缓动函数
    EasingFunctionType g_popupAnimationEasing = EaseOutCubic;
}//namespace

void PopupAnimation::SetEnabled(bool bEnabled)
{
    g_bPopupAnimationEnabled = bEnabled;
}

bool PopupAnimation::IsEnabled()
{
    return g_bPopupAnimationEnabled;
}

void PopupAnimation::SetTotalMillSeconds(int32_t nTotalMillSeconds)
{
    g_nPopupAnimationTotalMillSeconds = nTotalMillSeconds;
}

int32_t PopupAnimation::GetTotalMillSeconds()
{
    return g_nPopupAnimationTotalMillSeconds;
}

void PopupAnimation::SetEasingFunctionType(EasingFunctionType easingFunctionType)
{
    g_popupAnimationEasing = easingFunctionType;
}

EasingFunctionType PopupAnimation::GetEasingFunctionType()
{
    return g_popupAnimationEasing;
}

void PopupAnimation::PlayFadeIn(WindowBase* pWindow, std::unique_ptr<AnimationPlayer>& pPlayer)
{
    if (pPlayer != nullptr) {
        pPlayer->Stop();
        pPlayer.reset();
    }
    if (pWindow == nullptr) {
        return;
    }

    const int32_t nTotalMillSeconds = GetTotalMillSeconds();
    const int32_t nTargetAlpha = pWindow->GetLayeredWindowAlpha();
    if (!IsEnabled() || (nTotalMillSeconds <= 0) || !pWindow->IsLayeredWindow() || (nTargetAlpha <= 0)) {
        //不播放动画：保持窗口原有的透明度（支持 XML 中的 alpha 属性）
        return;
    }

    //先把窗口 Alpha 置 0，避免显示时出现一帧全不透明的闪烁
    pWindow->SetLayeredWindowAlpha(0);

    std::weak_ptr<WeakFlag> weakFlag = pWindow->GetWeakFlag();
    std::unique_ptr<AnimationPlayer> pNewPlayer(new AnimationPlayer);
    pNewPlayer->SetAnimationType(AnimationType::kAnimationNone);
    pNewPlayer->SetStartValue(0);
    pNewPlayer->SetEndValue(nTargetAlpha);
    pNewPlayer->SetTotalMillSeconds(nTotalMillSeconds);
    pNewPlayer->SetFrameIntervalMillSeconds(16); //约 60 帧/秒
    pNewPlayer->SetEasingFunctionType(GetEasingFunctionType());
    pNewPlayer->SetPlayCallback([pWindow, weakFlag](int32_t nValue) {
        if (!weakFlag.expired()) {
            pWindow->SetLayeredWindowAlpha(nValue);
        }
    });
    pNewPlayer->SetCompleteCallback([pWindow, weakFlag, nTargetAlpha]() {
        if (!weakFlag.expired()) {
            pWindow->SetLayeredWindowAlpha(nTargetAlpha);
        }
    });
    pNewPlayer->Start();
    pPlayer = std::move(pNewPlayer);
}

} // namespace ui
