#include "AnimationManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui 
{
// 显示 / 隐藏的动画类型列表
std::vector<AnimationType> AnimationManager::s_animationList;

AnimationManager::AnimationManager(Control* pControl) :
    m_pControl(pControl),
    m_bControlVisible(false),
    m_bControlVisibleInited(false)
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
    const AnimationType animationType = AnimationType::kAnimationHot;
    if (bFadeHot) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(255);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    if (nNewValue < 0) {
                        nNewValue = 0;
                    }
                    if (nNewValue > 255) {
                        nNewValue = 255;
                    }
                    pControl->SetHotAlpha(TruncateToUInt8(TruncateToInt32(nNewValue)));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(animationType);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeAlpha(bool bFadeVisible, uint8_t nEndAlpha)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    const AnimationType animationType = AnimationType::kAnimationAlpha;
    if (bFadeVisible) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue((int64_t)nEndAlpha);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    if (nNewValue < 0) {
                        nNewValue = 0;
                    }
                    if (nNewValue > 255) {
                        nNewValue = 255;
                    }
                    pControl->SetAlpha(TruncateToUInt8(TruncateToInt32(nNewValue)));
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[animationType].reset(pAnimationPlayer);
        m_pControl->SetAlpha(0);
    }
    else {
        m_animationMap.erase(animationType);
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
    const AnimationType animationType = AnimationType::kAnimationWidth;
    if (bFadeWidth && (cx > 0)) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cx);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetFixedWidth(UiFixedInt(TruncateToInt32(nNewValue)), true, false);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(animationType);
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
    const AnimationType animationType = AnimationType::kAnimationHeight;
    if (bFadeHeight && (cy > 0)) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cy);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int64_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetFixedHeight(UiFixedInt(TruncateToInt32(nNewValue)), true, false);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(animationType);
    }

    return pAnimationPlayer;
}

AnimationPlayer* AnimationManager::SetFadeSize(bool bFadeSize)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cx = 0;
    int32_t cy = 0;
    if (bFadeSize) {
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(999999, 999999));
        cx = estSize.cx.GetInt32();
        cy = estSize.cy.GetInt32();
        ASSERT(cy > 0);
        ASSERT(cx > 0);
    }
    const AnimationType animationType = AnimationType::kAnimationHeight;
    if (bFadeSize && (cx > 0) && (cy > 0)) {
        pAnimationPlayer = new AnimationPlayer();
        pAnimationPlayer->SetAnimationType(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(100);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl, cx, cy](int64_t nNewValue) {
            if (pControl != nullptr) {
                if (nNewValue > 0) {
                    int32_t cxNow = TruncateToInt32(cx * nNewValue / 100);
                    int32_t cyNow = TruncateToInt32(cy * nNewValue / 100);
                    pControl->SetFixedWidth(UiFixedInt(cxNow), true, false);
                    pControl->SetFixedHeight(UiFixedInt(cyNow), true, false);
                }
            }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else {
        m_animationMap.erase(animationType);
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
    const AnimationType animationType = bIsFromRight ? AnimationType::kAnimationInoutXFromRight : AnimationType::kAnimationInoutXFromLeft;
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
        }
        else {
            pAnimationPlayer->SetStartValue(cx);
        }
        pAnimationPlayer->SetAnimationType(animationType);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else{
        m_animationMap.erase(animationType);
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
    const AnimationType animationType = bIsFromBottom ? AnimationType::kAnimationInoutYFromBottom : AnimationType::kAnimationInoutYFromTop;
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
        }
        else {
            pAnimationPlayer->SetStartValue(cy);
        }
        pAnimationPlayer->SetAnimationType(animationType);
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else{
        m_animationMap.erase(animationType);
    }

    return pAnimationPlayer;
}

void AnimationManager::InitAppearAnimationList(std::vector<AnimationType>& animationList) const
{
    if (animationList.empty()) {
        animationList.push_back(AnimationType::kAnimationAlpha);
        animationList.push_back(AnimationType::kAnimationWidth);
        animationList.push_back(AnimationType::kAnimationHeight);
        animationList.push_back(AnimationType::kAnimationSize);
        animationList.push_back(AnimationType::kAnimationInoutXFromLeft);
        animationList.push_back(AnimationType::kAnimationInoutXFromRight);
        animationList.push_back(AnimationType::kAnimationInoutYFromTop);
        animationList.push_back(AnimationType::kAnimationInoutYFromBottom);
    }
}

void AnimationManager::Appear()
{
    //仅当可见属性发生变化时，才触发控件动画
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return;
    }
    const bool bOldVisible = m_bControlVisible;
    const bool bOldVisibleInited = m_bControlVisibleInited;
    const bool bNewVisible = true;

    m_pControl->SetVisible(true);

    std::vector<AnimationType>& animationList = s_animationList;
    InitAppearAnimationList(animationList);
    if (!m_animationMap.empty()) {
        for (AnimationType animationType : animationList) {
            if (HasAnimationPlayer(animationType)) {
                AnimationPlayer* pAnimationPlayer = m_animationMap[animationType].get();
                pAnimationPlayer->SetCompleteCallback(AnimationCompleteCallback());
                if (bOldVisibleInited && (bOldVisible == bNewVisible)) {
                    //可见属性未发生变化
                    if (pAnimationPlayer->IsPlaying()) {
                        pAnimationPlayer->Continue();
                    }
                    else {
                        pAnimationPlayer->Stop();
                    }
                }
                else {
                    //可见属性发生变化
                    if (pAnimationPlayer->IsPlaying()) {
                        pAnimationPlayer->Continue();
                    }
                    else {
                        pAnimationPlayer->Start();
                    }
                }
            }
        }
    }

    m_bControlVisibleInited = true;
    m_bControlVisible = true;
}

void AnimationManager::Disappear()
{
    bool handled = false;
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return;
    }
    //仅当可见属性发生变化时，才触发控件动画
    const bool bOldVisible = m_bControlVisible;
    const bool bOldVisibleInited = m_bControlVisibleInited;
    const bool bNewVisible = false;

    AnimationCompleteCallback completeCallback = UiBind(&Control::SetVisible, m_pControl, false);
    std::vector<AnimationType>& animationList = s_animationList;
    InitAppearAnimationList(animationList);
    if (!m_animationMap.empty()) {
        for (AnimationType animationType : animationList) {
            if (HasAnimationPlayer(animationType)) {
                AnimationPlayer* pAnimationPlayer = m_animationMap[animationType].get();
                pAnimationPlayer->SetCompleteCallback(completeCallback);
                if (bOldVisibleInited && (bOldVisible == bNewVisible)) {
                    //可见属性未发生变化
                    if (pAnimationPlayer->IsPlaying()) {
                        pAnimationPlayer->ReverseContinue();
                    }
                    else {
                        pAnimationPlayer->Stop();
                    }
                }
                else {
                    //可见属性发生变化
                    if (pAnimationPlayer->IsPlaying()) {
                        pAnimationPlayer->ReverseContinue();
                    }
                    else {
                        pAnimationPlayer->ReverseStart();
                    }
                }
                handled = true;
            }
        }
    }
    
    if (!handled) {
        m_pControl->SetVisible(false);
    }
    m_bControlVisibleInited = true;
    m_bControlVisible = false;
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
