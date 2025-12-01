#include "duilib/Core/StateColorMap.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Animation/AnimationManager.h"

namespace ui 
{
StateColorMap::StateColorMap(Control* pControl):
    m_pControl(pControl)
{
    ASSERT(kControlStateCount > 0);
    m_stateColors.resize(kControlStateCount);
}

void StateColorMap::SetStateColor(ControlStateType stateType, const DString& color)
{
    size_t nIndex = (size_t)stateType;
    ASSERT(nIndex < m_stateColors.size());
    if (nIndex < m_stateColors.size()) {
        m_stateColors[nIndex] = color;
    }
}

bool StateColorMap::HasHotColor() const
{
    return HasStateColor(kControlStateHot);
}

bool StateColorMap::HasStateColors() const
{
    for (const UiString& color : m_stateColors) {
        if (!color.empty()) {
            return true;
        }
    }
    return false;
}

bool StateColorMap::HasStateColor(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        return !m_stateColors[nIndex].empty();
    }
    return false;
}

DString StateColorMap::GetStateColor(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        return m_stateColors[nIndex].c_str();
    }
    return DString();
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
