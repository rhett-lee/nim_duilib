#include "duilib/Core/StateColorMap.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Animation/AnimationManager.h"

namespace ui 
{
StateColorMap::StateColorMap() :
    m_pControl(nullptr),
    m_stateColorMap()
{
}

void StateColorMap::SetControl(Control* control)
{
    m_pControl = control;
}

bool StateColorMap::HasHotColor() const
{
    return m_stateColorMap.find(kControlStateHot) != m_stateColorMap.end();
}

bool StateColorMap::HasStateColors() const
{
    return (m_stateColorMap.find(kControlStateNormal) != m_stateColorMap.end()) ||
           (m_stateColorMap.find(kControlStateHot) != m_stateColorMap.end()) ||
           (m_stateColorMap.find(kControlStatePushed) != m_stateColorMap.end()) ||
           (m_stateColorMap.find(kControlStateDisabled) != m_stateColorMap.end());
}

bool StateColorMap::HasStateColor(ControlStateType stateType) const
{
    return m_stateColorMap.find(stateType) != m_stateColorMap.end();
}

DString StateColorMap::GetStateColor(ControlStateType stateType) const
{
    auto iter = m_stateColorMap.find(stateType);
    if (iter != m_stateColorMap.end()) {
        return iter->second.c_str();
    }
    return DString();
}

void StateColorMap::SetStateColor(ControlStateType stateType, const DString& color)
{
    if (!color.empty()) {
        m_stateColorMap[stateType] = color;
    }
    else {
        //确保颜色值不是空字符串
        auto iter = m_stateColorMap.find(stateType);
        if (iter != m_stateColorMap.end()) {
            m_stateColorMap.erase(iter);
        }
    }
}

void StateColorMap::PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (m_pControl != nullptr) {
        bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot) != nullptr;
        int32_t nHotAlpha = m_pControl->GetHotAlpha();
        if (bFadeHot) {
            if ((stateType == kControlStateNormal || stateType == kControlStateHot) && HasStateColor(kControlStateHot)) {
                DString strColor = GetStateColor(kControlStateNormal);
                if (!strColor.empty()) {
                    pRender->FillRect(rcPaint, m_pControl->GetUiColor(strColor));
                }
                if (nHotAlpha > 0) {
                    pRender->FillRect(rcPaint, m_pControl->GetUiColor(GetStateColor(kControlStateHot)), static_cast<uint8_t>(nHotAlpha));
                }
                return;
            }
        }
    }

    if (stateType == kControlStatePushed && !HasStateColor(kControlStatePushed)) {
        stateType = kControlStateHot;
    }
    if (stateType == kControlStateHot && !HasStateColor(kControlStateHot)) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && !HasStateColor(kControlStateDisabled)) {
        stateType = kControlStateNormal;
    }
    DString strColor = GetStateColor(stateType);
    if (!strColor.empty()) {
        UiColor color = m_pControl ? m_pControl->GetUiColor(strColor) :
                                     GlobalManager::Instance().Color().GetColor(strColor);
        pRender->FillRect(rcPaint, color);
    }
}
} // namespace ui
