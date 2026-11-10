#include "AnimationManager.h"
#include "AnimationScheduler.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/ControlPtrT.h"
#include <vector>

namespace ui 
{
namespace
{
/** 显示/隐藏时，需要触发的动画类型列表
*   函数内静态常量：只初始化一次，且初始化过程是线程安全的
*/
const std::vector<AnimationType>& GetAppearAnimationList()
{
    static const std::vector<AnimationType> s_animationList =
    {
        AnimationType::kAnimationAlpha,
        AnimationType::kAnimationWidth,
        AnimationType::kAnimationHeight,
        AnimationType::kAnimationSize,
        AnimationType::kAnimationInoutXFromLeft,
        AnimationType::kAnimationInoutXFromRight,
        AnimationType::kAnimationInoutYFromTop,
        AnimationType::kAnimationInoutYFromBottom
    };
    return s_animationList;
}
} // namespace

AnimationManager::AnimationManager(Control* pControl) :
    m_pControl(pControl),
    m_frameIntervalMillSeconds(-1),
    m_totalMillSeconds(-1),
    m_easingFunctionType(EasingFunctionType::EaseInOutCubic),
    m_easingFunctionTypeIn(EasingFunctionType::EaseOutCubic),
    m_easingFunctionTypeOut(EasingFunctionType::EaseInCubic),
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

AnimationPlayer* AnimationManager::SetFadeAlpha(bool bFadeVisible, uint8_t nEndAlpha)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    const AnimationType animationType = AnimationType::kAnimationAlpha;
    if (bFadeVisible) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue((int32_t)nEndAlpha);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int32_t nNewValue) {
                if (pControl != nullptr) {
                    if (nNewValue < 0) {
                        nNewValue = 0;
                    }
                    if (nNewValue > 255) {
                        nNewValue = 255;
                    }
                    pControl->SetAlpha(TruncateToUInt8(nNewValue));
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

AnimationPlayer* AnimationManager::SetFadeWidth(bool bFadeWidth)
{
    AnimationPlayer* pAnimationPlayer = nullptr;
    int32_t cx = 0;
    if (bFadeWidth) {
        // 使用一个超大尺寸去探测控件"不受限"时的自然宽度
        // 该常量定义在 AnimationManager.h 中
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(kAnimationEstimateMaxSize, kAnimationEstimateMaxSize));
        cx = estSize.cx.GetInt32();
        ASSERT(cx > 0);
    }
    const AnimationType animationType = AnimationType::kAnimationWidth;
    if (bFadeWidth && (cx > 0)) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cx);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int32_t nNewValue) {
                if (pControl != nullptr) {
                    if (nNewValue < 0) {
                        nNewValue = 0;
                    }
                    //由动画调度器在本帧统一重排（同一帧内多个尺寸动画只重排一次）
                    pControl->SetFixedWidth(UiFixedInt(nNewValue), false, false);
                    AnimationScheduler::Instance().RequestArrange(pControl);
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
        // 使用一个超大尺寸去探测控件"不受限"时的自然高度
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(kAnimationEstimateMaxSize, kAnimationEstimateMaxSize));
        cy = estSize.cy.GetInt32();
        ASSERT(cy > 0);
    }
    const AnimationType animationType = AnimationType::kAnimationHeight;
    if (bFadeHeight && (cy > 0)) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(cy);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int32_t nNewValue) {
                if (pControl != nullptr) {
                    if (nNewValue < 0) {
                        nNewValue = 0;
                    }
                    //由动画调度器在本帧统一重排（同一帧内多个尺寸动画只重排一次）
                    pControl->SetFixedHeight(UiFixedInt(nNewValue), false, false);
                    AnimationScheduler::Instance().RequestArrange(pControl);
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
        // 使用一个超大尺寸去探测控件"不受限"时的自然尺寸
        // 该常量定义在 AnimationManager.h 中
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(kAnimationEstimateMaxSize, kAnimationEstimateMaxSize));
        cx = estSize.cx.GetInt32();
        cy = estSize.cy.GetInt32();
        ASSERT(cy > 0);
        ASSERT(cx > 0);
    }
    const AnimationType animationType = AnimationType::kAnimationSize;
    if (bFadeSize && (cx > 0) && (cy > 0)) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(100);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl, cx, cy](int32_t nNewValue) {
            if (pControl != nullptr) {
                if (nNewValue > 0) {
                    int32_t cxNow = TruncateToInt32((int64_t)cx * nNewValue / 100);
                    int32_t cyNow = TruncateToInt32((int64_t)cy * nNewValue / 100);
                    //宽高必须同时更新，先设置尺寸（不重排），由动画调度器在本帧统一重排一次
                    pControl->SetFixedWidth(UiFixedInt(cxNow), false, false);
                    pControl->SetFixedHeight(UiFixedInt(cyNow), false, false);
                    AnimationScheduler::Instance().RequestArrange(pControl);
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
        // 使用一个超大尺寸去探测控件"不受限"时的自然宽度
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(kAnimationEstimateMaxSize, kAnimationEstimateMaxSize));
        cx = estSize.cx.GetInt32();
        if (cx <= 0) {
            cx = 100;
        }
    }
    const AnimationType animationType = bIsFromRight ? AnimationType::kAnimationInoutXFromRight : AnimationType::kAnimationInoutXFromLeft;
    if (bFade) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetEndValue(0);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int32_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetRenderOffsetX(nNewValue);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);

        if (bIsFromRight) {
            pAnimationPlayer->SetStartValue(-cx);            
        }
        else {
            pAnimationPlayer->SetStartValue(cx);
        }
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
        // 使用一个超大尺寸去探测控件"不受限"时的自然高度
        UiEstSize estSize = m_pControl->EstimateSize(UiSize(kAnimationEstimateMaxSize, kAnimationEstimateMaxSize));
        cy = estSize.cy.GetInt32();
        if (cy <= 0) {
            cy = 100;
        }
    }
    const AnimationType animationType = bIsFromBottom ? AnimationType::kAnimationInoutYFromBottom : AnimationType::kAnimationInoutYFromTop;
    if (bFade) {
        pAnimationPlayer = CreateAnimationPlayer(animationType);
        pAnimationPlayer->SetEndValue(0);
        ControlPtr pControl(m_pControl);
        AnimationPlayCallback playCallback = [pControl](int32_t nNewValue) {
                if (pControl != nullptr) {
                    pControl->SetRenderOffsetY(nNewValue);
                }
            };
        pAnimationPlayer->SetPlayCallback(playCallback);

        if (bIsFromBottom) {
            pAnimationPlayer->SetStartValue(-cy);
        }
        else {
            pAnimationPlayer->SetStartValue(cy);
        }
        m_animationMap[animationType].reset(pAnimationPlayer);
    }
    else{
        m_animationMap.erase(animationType);
    }

    return pAnimationPlayer;
}

void AnimationManager::UpdateInoutStartValue()
{
    if (m_pControl == nullptr) {
        return;
    }
    //控件的实际布局尺寸（控件尺寸可能是 stretch/auto 的，配置动画时拿到的尺寸并不准确）
    const int32_t cx = m_pControl->GetRect().Width();
    const int32_t cy = m_pControl->GetRect().Height();

    //(动画类型, 滑动起始值)
    const AnimationType inoutTypes[] =
    {
        AnimationType::kAnimationInoutXFromLeft,
        AnimationType::kAnimationInoutXFromRight,
        AnimationType::kAnimationInoutYFromTop,
        AnimationType::kAnimationInoutYFromBottom
    };
    for (AnimationType animationType : inoutTypes) {
        AnimationPlayer* pAnimationPlayer = GetAnimationPlayer(animationType);
        if ((pAnimationPlayer == nullptr) || pAnimationPlayer->IsPlaying()) {
            //播放中的动画不做修改，避免打断当前播放
            continue;
        }
        int32_t nStartValue = 0;
        switch (animationType) {
        case AnimationType::kAnimationInoutXFromLeft:
            nStartValue = cx;
            break;
        case AnimationType::kAnimationInoutXFromRight:
            nStartValue = -cx;
            break;
        case AnimationType::kAnimationInoutYFromTop:
            nStartValue = cy;
            break;
        case AnimationType::kAnimationInoutYFromBottom:
            nStartValue = -cy;
            break;
        default:
            continue;
        }
        if (nStartValue == 0) {
            //控件还没有布局出实际尺寸，保留配置动画时的估算值
            continue;
        }
        //先恢复到正向播放状态（反向播放时，起始值保存在结束值中，在此交换回来）
        pAnimationPlayer->Stop();
        pAnimationPlayer->SetStartValue(nStartValue);
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

    //按控件的实际布局尺寸修正进出场动画的滑动起始值
    UpdateInoutStartValue();

    //可见属性是否发生变化
    const bool bVisibleChanged = !bOldVisibleInited || (bOldVisible != bNewVisible);
    for (AnimationType animationType : GetAppearAnimationList()) {
        AnimationPlayer* pAnimationPlayer = GetAnimationPlayer(animationType);
        if (pAnimationPlayer == nullptr) {
            continue;
        }
        pAnimationPlayer->SetCompleteCallback(AnimationCompleteCallback());
        //进场动画使用进场缓动曲线
        pAnimationPlayer->SetEasingFunctionType(m_easingFunctionTypeIn);
        if (bVisibleChanged) {
            //可见属性发生变化
            if (pAnimationPlayer->IsPlaying()) {
                pAnimationPlayer->Continue();
            }
            else {
                pAnimationPlayer->Start();
            }
        }
        else {
            //可见属性未发生变化
            if (pAnimationPlayer->IsPlaying()) {
                pAnimationPlayer->Continue();
            }
            else {
                pAnimationPlayer->Stop();
            }
        }
    }

    m_bControlVisibleInited = true;
    m_bControlVisible = true;
}

void AnimationManager::Disappear()
{
    ASSERT(m_pControl != nullptr);
    if (m_pControl == nullptr) {
        return;
    }
    //仅当可见属性发生变化时，才触发控件动画
    const bool bOldVisible = m_bControlVisible;
    const bool bOldVisibleInited = m_bControlVisibleInited;
    const bool bNewVisible = false;

    bool handled = false;
    AnimationCompleteCallback completeCallback = UiBind(&Control::SetVisible, m_pControl, false);

    //按控件的实际布局尺寸修正进出场动画的滑动起始值
    UpdateInoutStartValue();

    //可见属性是否发生变化
    const bool bVisibleChanged = !bOldVisibleInited || (bOldVisible != bNewVisible);
    for (AnimationType animationType : GetAppearAnimationList()) {
        AnimationPlayer* pAnimationPlayer = GetAnimationPlayer(animationType);
        if (pAnimationPlayer == nullptr) {
            continue;
        }
        pAnimationPlayer->SetCompleteCallback(completeCallback);
        //出场动画使用出场缓动曲线
        pAnimationPlayer->SetEasingFunctionType(m_easingFunctionTypeOut);
        if (bVisibleChanged) {
            //可见属性发生变化
            if (pAnimationPlayer->IsPlaying()) {
                pAnimationPlayer->ReverseContinue();
            }
            else {
                pAnimationPlayer->ReverseStart();
            }
        }
        else {
            //可见属性未发生变化
            if (pAnimationPlayer->IsPlaying()) {
                pAnimationPlayer->ReverseContinue();
            }
            else {
                pAnimationPlayer->Stop();
            }
        }
        handled = true;
    }

    if (!handled) {
        m_pControl->SetVisible(false);
    }
    m_bControlVisibleInited = true;
    m_bControlVisible = false;
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

AnimationPlayer* AnimationManager::CreateAnimationPlayer(AnimationType animationType) const
{
    AnimationPlayer* pAnimationPlayer = new AnimationPlayer();
    pAnimationPlayer->SetAnimationType(animationType);
    pAnimationPlayer->SetFrameIntervalMillSeconds(GetFrameIntervalMillSeconds());
    pAnimationPlayer->SetTotalMillSeconds(GetTotalMillSeconds());
    pAnimationPlayer->SetEasingFunctionType(GetEasingFunctionType());
    return pAnimationPlayer;
}

void AnimationManager::SetFrameIntervalMillSeconds(int32_t frameIntervalMillSeconds)
{
    m_frameIntervalMillSeconds = frameIntervalMillSeconds;
    for (auto& iter : m_animationMap) {
        if (iter.second != nullptr) {
            iter.second->SetFrameIntervalMillSeconds(frameIntervalMillSeconds);
        }
    }
}

int32_t AnimationManager::GetFrameIntervalMillSeconds() const
{
    return m_frameIntervalMillSeconds;
}

void AnimationManager::SetTotalMillSeconds(int32_t totalMillSeconds)
{
    m_totalMillSeconds = totalMillSeconds;
    for (auto& iter : m_animationMap) {
        if (iter.second != nullptr) {
            iter.second->SetTotalMillSeconds(totalMillSeconds);
        }
    }
}

int32_t AnimationManager::GetTotalMillSeconds() const
{
    return m_totalMillSeconds;
}

void AnimationManager::SetEasingFunctionType(EasingFunctionType easingFunctionType)
{
    SetEasingFunctionType(easingFunctionType, easingFunctionType);
}

void AnimationManager::SetEasingFunctionType(EasingFunctionType easingInType, EasingFunctionType easingOutType)
{
    m_easingFunctionType = easingInType;
    m_easingFunctionTypeIn = easingInType;
    m_easingFunctionTypeOut = easingOutType;
    for (auto& iter : m_animationMap) {
        if (iter.second != nullptr) {
            iter.second->SetEasingFunctionType(easingInType);
        }
    }
}

EasingFunctionType AnimationManager::GetEasingFunctionType() const
{
    return m_easingFunctionType;
}

}
