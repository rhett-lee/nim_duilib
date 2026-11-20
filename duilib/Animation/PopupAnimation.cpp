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
    //目标 Alpha：如果上一次淡入还没播完（窗口被提前隐藏/关闭），窗口当前的 Alpha 只是
    //动画的中间值，甚至可能停在 0；此时改用上一次动画的结束值作为目标值，
    //否则再次显示时窗口会变成半透明甚至完全不可见
    const bool bHasPreviousPlayer = (pPlayer != nullptr);
    int32_t nTargetAlpha = 0;
    if (bHasPreviousPlayer) {
        nTargetAlpha = pPlayer->GetEndValue();
        pPlayer->Stop();
        pPlayer.reset();
    }
    if (pWindow == nullptr) {
        return;
    }
    if (!bHasPreviousPlayer) {
        nTargetAlpha = pWindow->GetLayeredWindowAlpha();
    }
    if (nTargetAlpha < 0) {
        nTargetAlpha = 0;
    }
    else if (nTargetAlpha > 255) {
        nTargetAlpha = 255;
    }

    const int32_t nTotalMillSeconds = GetTotalMillSeconds();
    if (!IsEnabled() || (nTotalMillSeconds <= 0) || !pWindow->IsLayeredWindow() || (nTargetAlpha <= 0)) {
        //不播放动画：直接设置为窗口的目标 Alpha（支持 XML 中的 alpha 属性）
        pWindow->SetLayeredWindowAlpha(nTargetAlpha);
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
