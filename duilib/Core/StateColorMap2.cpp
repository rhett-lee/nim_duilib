#include "duilib/Core/StateColorMap2.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Animation/AnimationManager.h"

namespace ui 
{
    StateColorMap2::StateColorMap2(Control* pControl):
    m_pControl(pControl)
{
    ASSERT(kControlStateCount > 0);
    m_stateColors.resize(kControlStateCount);
}

void StateColorMap2::SetStateColor(ControlStateType stateType, const DString& color)
{
    size_t nIndex = (size_t)stateType;
    ASSERT(nIndex < m_stateColors.size());
    if (nIndex < m_stateColors.size()) {
        m_stateColors[nIndex].m_colorStr = color;
    }
}

void StateColorMap2::SetStateColorMargin(ControlStateType stateType, const UiMargin& colorMargin)
{
    size_t nIndex = (size_t)stateType;
    ASSERT(nIndex < m_stateColors.size());
    if (nIndex < m_stateColors.size()) {
        UiMargin16& margin16 = m_stateColors[nIndex].m_colorMargin;
        margin16.left = ui::TruncateToInt16(colorMargin.left);
        margin16.top = ui::TruncateToInt16(colorMargin.top);
        margin16.right = ui::TruncateToInt16(colorMargin.right);
        margin16.bottom = ui::TruncateToInt16(colorMargin.bottom);
    }
}

void StateColorMap2::SetStateColorRound(ControlStateType stateType, const UiSize& colorRound)
{
    size_t nIndex = (size_t)stateType;
    ASSERT(nIndex < m_stateColors.size());
    if (nIndex < m_stateColors.size()) {
        UiSize16& size16 = m_stateColors[nIndex].m_colorRound;
        size16.cx = ui::TruncateToInt16(colorRound.cx);
        size16.cy = ui::TruncateToInt16(colorRound.cy);
    }
}

bool StateColorMap2::HasHotColor() const
{
    return HasStateColor(kControlStateHot);
}

bool StateColorMap2::HasStateColors() const
{
    for (const TColorProperty& color : m_stateColors) {
        if (!color.m_colorStr.empty()) {
            return true;
        }
    }
    return false;
}

bool StateColorMap2::HasStateColor(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        return !m_stateColors[nIndex].m_colorStr.empty();
    }
    return false;
}

DString StateColorMap2::GetStateColor(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        return m_stateColors[nIndex].m_colorStr.c_str();
    }
    return DString();
}

UiMargin StateColorMap2::GetStateColorMargin(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        const UiMargin16& margin16 = m_stateColors[nIndex].m_colorMargin;
        UiMargin rcMargin;
        rcMargin.left = margin16.left;
        rcMargin.top = margin16.top;
        rcMargin.right = margin16.right;
        rcMargin.bottom = margin16.bottom;
        return rcMargin;
    }
    return UiMargin();
}

UiSize StateColorMap2::GetStateColorRound(ControlStateType stateType) const
{
    size_t nIndex = (size_t)stateType;
    if (nIndex < m_stateColors.size()) {
        const UiSize16& size16 = m_stateColors[nIndex].m_colorRound;
        UiSize rcSize;
        rcSize.cx = size16.cx;
        rcSize.cy = size16.cy;
        return rcSize;
    }
    return UiSize();
}

void StateColorMap2::PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const
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
                    DoPaintStateColor(pRender, rcPaint, kControlStateNormal, m_pControl->GetUiColor(strColor));
                }
                if (nHotAlpha > 0) {
                    DoPaintStateColor(pRender, rcPaint, kControlStateHot, m_pControl->GetUiColor(GetStateColor(kControlStateHot)), nHotAlpha);
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
        DoPaintStateColor(pRender, rcPaint, stateType, color);
    }
}

void StateColorMap2::DoPaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType, UiColor colorValue, int32_t nAlpha) const
{
    if ((pRender == nullptr) || rcPaint.IsEmpty() || (nAlpha == 0) || colorValue.IsEmpty()) {
        return;
    }
    UiMargin rcMargin = GetStateColorMargin(stateType);
    if (!rcMargin.IsEmpty()) {
        rcPaint.Deflate(rcMargin);//剪去外边距
        if (rcPaint.IsEmpty()) {
            return;
        }
    }

    UiSize szRound = GetStateColorRound(stateType);
    if (!szRound.IsEmpty()) {
        //圆角矩形
        pRender->FillRoundRect(rcPaint, (float)szRound.cx, (float)szRound.cy, colorValue, static_cast<uint8_t>(nAlpha));
    }
    else {
        //直角矩形
        pRender->FillRect(rcPaint, colorValue, static_cast<uint8_t>(nAlpha));
    }
}

} // namespace ui
