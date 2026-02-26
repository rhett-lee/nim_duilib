#include "AnimationManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui 
{

AnimationManager::AnimationManager(Control* pControl) :
    m_pControl(pControl)
{
}

bool AnimationManager::HasAnimationPlayer(AnimationType animationType) const
{
    auto it = m_animationMap.find(animationType);
    return (it != m_animationMap.end());
}

AnimationPlayer* AnimationManager::GetAnimationPlayer(AnimationType animationType) const
{
    auto it = m_animationMap.find(animationType);
    if (it != m_animationMap.end()) {
        return it->second.get();
    }
    else {
        return nullptr;
    }
}

AnimationPlayer* AnimationManager::SetFadeHot(bool bFadeHot)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    if (bFadeHot) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(AnimationType::kAnimationHot);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(255);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetHotAlpha(TruncateToUInt8(TruncateToInt32(nNewValue)));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[AnimationType::kAnimationHot].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(AnimationType::kAnimationHot);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeAlpha(bool bFadeVisible, uint8_t nEndAlpha)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    if (bFadeVisible) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(AnimationType::kAnimationAlpha);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue((int64_t)nEndAlpha);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetAlpha(TruncateToUInt8(TruncateToInt32(nNewValue)));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[AnimationType::kAnimationAlpha].reset(pAnimationPlayer);
        m_pControl->SetAlpha(0);
    }
    else {
        m_animationMap.erase(AnimationType::kAnimationAlpha);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeWidth(bool bFadeWidth)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cx = 0;
    if (bFadeWidth) {
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
        cx = estSize.cx.GetInt32();
        ASSERT(cx > 0);
    }
    if (bFadeWidth && (cx > 0)) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(AnimationType::kAnimationWidth);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cx);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetFixedWidth(UiFixedInt(TruncateToInt32(nNewValue)), true, false);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[AnimationType::kAnimationWidth].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(AnimationType::kAnimationWidth);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeHeight(bool bFadeHeight)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cy = 0;
    if (bFadeHeight) {
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
        cy = estSize.cy.GetInt32();
        ASSERT(cy > 0);
    }
    if (bFadeHeight && (cy > 0)) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(AnimationType::kAnimationHeight);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cy);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetFixedHeight(UiFixedInt(TruncateToInt32(nNewValue)), true, false);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[AnimationType::kAnimationHeight].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(AnimationType::kAnimationHeight);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeInOutX(bool bFade, bool bIsFromRight)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cx = 0;
    if (bFade) {
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
        cx = estSize.cx.GetInt32();
        if (cx <= 0) {
            cx = 100;
        }
    }
    if (bFade) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetEndValue(0);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetRenderOffsetX(TruncateToInt32(nNewValue));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);

        if (bIsFromRight) {
            pAnimationPlayer->SetStartValue(-cx);
            pAnimationPlayer->SetAnimationType(AnimationType::kAnimationInoutXFromRight);
            m_animationMap[AnimationType::kAnimationInoutXFromRight].reset(pAnimationPlayer);
        }
        else {
            pAnimationPlayer->SetStartValue(cx);
            pAnimationPlayer->SetAnimationType(AnimationType::kAnimationInoutXFromLeft);
            m_animationMap[AnimationType::kAnimationInoutXFromLeft].reset(pAnimationPlayer);
        }
    }
    else{
        if (bIsFromRight) {
            m_animationMap.erase(AnimationType::kAnimationInoutXFromRight);
        }
        else {
            m_animationMap.erase(AnimationType::kAnimationInoutXFromLeft);
        }
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeInOutY(bool bFade, bool bIsFromBottom)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cy = 0;
    if (bFade) {
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
        cy = estSize.cy.GetInt32();
        if (cy <= 0) {
            cy = 100;
        }
    }
    if (bFade) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetEndValue(0);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetRenderOffsetY(TruncateToInt32(nNewValue));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);

        if (bIsFromBottom) {
            pAnimationPlayer->SetStartValue(-cy);
            pAnimationPlayer->SetAnimationType(AnimationType::kAnimationInoutYFromBottom);
            m_animationMap[AnimationType::kAnimationInoutYFromBottom].reset(pAnimationPlayer);
        }
        else {
            pAnimationPlayer->SetStartValue(cy);
            pAnimationPlayer->SetAnimationType(AnimationType::kAnimationInoutYFromTop);
            m_animationMap[AnimationType::kAnimationInoutYFromTop].reset(pAnimationPlayer);
        }
    }
    else{
        if (bIsFromBottom) {
            m_animationMap.erase(AnimationType::kAnimationInoutYFromBottom);
        }
        else {
            m_animationMap.erase(AnimationType::kAnimationInoutYFromTop);
        }
    }

    return pAnimationPlayer;
}

void AnimationManager::Appear()
{
    ASSERT(m_pControl != nullptr);
    if (m_pControl != nullptr) {
        m_pControl->SetVisible(true);
    }    
    if (HasAnimationPlayer(AnimationType::kAnimationAlpha)) {
        m_animationMap[AnimationType::kAnimationAlpha]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationAlpha]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationWidth)) {
        m_animationMap[AnimationType::kAnimationWidth]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationWidth]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationHeight)) {
        m_animationMap[AnimationType::kAnimationHeight]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationHeight]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutXFromLeft)) {
        m_animationMap[AnimationType::kAnimationInoutXFromLeft]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationInoutXFromLeft]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutXFromRight)) {
        m_animationMap[AnimationType::kAnimationInoutXFromRight]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationInoutXFromRight]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutYFromTop)) {
        m_animationMap[AnimationType::kAnimationInoutYFromTop]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationInoutYFromTop]->Continue();
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutYFromBottom)) {
        m_animationMap[AnimationType::kAnimationInoutYFromBottom]->SetCompleteCallback(AnimationCompleteCallback());
        m_animationMap[AnimationType::kAnimationInoutYFromBottom]->Continue();
    }
}

void AnimationManager::Disappear()
{
    bool handled = false;
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return;
    }

    AnimationCompleteCallback completeCallback = UiBind(&Control::SetVisible, m_pControl, false);
    if (HasAnimationPlayer(AnimationType::kAnimationAlpha)) {
        m_animationMap[AnimationType::kAnimationAlpha]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationAlpha]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationWidth)) {
        m_animationMap[AnimationType::kAnimationWidth]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationWidth]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationHeight)) {
        m_animationMap[AnimationType::kAnimationHeight]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationHeight]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutXFromLeft)) {
        m_animationMap[AnimationType::kAnimationInoutXFromLeft]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationInoutXFromLeft]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutXFromRight)) {
        m_animationMap[AnimationType::kAnimationInoutXFromRight]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationInoutXFromRight]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutYFromTop)) {
        m_animationMap[AnimationType::kAnimationInoutYFromTop]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationInoutYFromTop]->ReverseContinue();
        handled = true;
    }
    if (HasAnimationPlayer(AnimationType::kAnimationInoutYFromBottom)) {
        m_animationMap[AnimationType::kAnimationInoutYFromBottom]->SetCompleteCallback(completeCallback);
        m_animationMap[AnimationType::kAnimationInoutYFromBottom]->ReverseContinue();
        handled = true;
    }

    if (!handled) {
        m_pControl->SetVisible(false);
    }
}

void AnimationManager::MouseEnter()
{
    AnimationPlayer* pPlayer = GetAnimationPlayer(AnimationType::kAnimationHot);
    if (pPlayer) {
        pPlayer->Continue();
    }
}

void AnimationManager::MouseLeave()
{
    AnimationPlayer* pPlayer = GetAnimationPlayer(AnimationType::kAnimationHot);
    if (pPlayer) {
        pPlayer->ReverseContinue();
    }
}

void AnimationManager::Clear(Control* control)
{
    if (m_pControl != nullptr) {
        ASSERT_UNUSED_VARIABLE(control == m_pControl);
    }

    for (auto& iter : m_animationMap) {
        if (iter.second != nullptr) {
            iter.second->Clear();
        }
    }

    m_pControl = nullptr;
    m_animationMap.clear();
}

}
