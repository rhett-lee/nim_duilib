#include "duilib/Core/StateColorMap2.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Animation/AnimationManager.h"

namespace ui 
{
StateColorMap2::StateColorMap2(Control* pControl):
    m_pControl(pControl),
    m_fStateColorMinWidth(0.0f),
    m_fStateColorMinHeight(0.0f)
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
        if (m_pControl->IsAnimationPlayerPlaying(AnimationType::kAnimationHot)) {
            if ((stateType == kControlStateNormal || stateType == kControlStateHot) && HasStateColor(kControlStateHot)) {
                const uint8_t nHotAlpha = m_pControl->GetHotAlpha();
                //先绘制默认的颜色
                DString strColor = GetStateColor(kControlStateNormal);
                if (!strColor.empty()) {
                    DoPaintStateColor(pRender, rcPaint, kControlStateNormal, m_pControl->GetUiColor(strColor), 255 - nHotAlpha);
                }

                //绘制Hot状态的颜色（半透明）
                DoPaintStateColor(pRender, rcPaint, kControlStateHot, m_pControl->GetUiColor(GetStateColor(kControlStateHot)), nHotAlpha);
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

void StateColorMap2::DoPaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType, UiColor colorValue, uint8_t nAlpha) const
{
    if ((pRender == nullptr) || rcPaint.IsEmpty() || (nAlpha == 0) || colorValue.IsEmpty()) {
        return;
    }
    UiRect rcStatePaint = rcPaint;
    const UiMargin rcMargin = GetStateColorMargin(stateType);
    if (!rcMargin.IsEmpty()) {
        rcStatePaint.Deflate(rcMargin);//剪去外边距        
    }

    constexpr float fMinSize = 0.1f;
    const float fConfigStateColorMinWidth = !rcMargin.IsEmpty() ? m_fStateColorMinWidth : 0.0f;
    const float fConfigStateColorMinHeight = !rcMargin.IsEmpty() ? m_fStateColorMinHeight : 0.0f;
    if ((fConfigStateColorMinWidth < fMinSize) && (fConfigStateColorMinHeight < fMinSize)) {
        //未设置最小宽度和最小高度限制
        if (rcStatePaint.IsEmpty()) {
            return;
        }
    }
    else if (fConfigStateColorMinWidth < fMinSize) {
        //未设置最小宽度限制
        if (rcStatePaint.Width() <= 0) {
            return;
        }
    }
    else if (fConfigStateColorMinHeight < fMinSize) {
        //未设置最小高度限制
        if (rcStatePaint.Height() <= 0) {
            return;
        }
    }

    UiRectF rcStatePaintF = UiRectF::MakeFromRect(rcStatePaint);
    if (fConfigStateColorMinWidth >= fMinSize) {
        //设置了最小宽度限制
        float fStateColorMinWidth = (m_pControl != nullptr) ? m_pControl->Dpi().GetScaleFloat(fConfigStateColorMinWidth) : fConfigStateColorMinWidth;
        fStateColorMinWidth = std::min(fStateColorMinWidth, (float)rcPaint.Width());
        if (rcStatePaintF.Width() < fStateColorMinWidth) {
            float diff = fStateColorMinWidth - rcStatePaintF.Width();
            rcStatePaintF.left -= diff / 2;
            rcStatePaintF.right += diff / 2;
        }
    }
    if (fConfigStateColorMinHeight >= fMinSize) {
        //设置了最小高度限制
        float fStateColorMinHeight = (m_pControl != nullptr) ? m_pControl->Dpi().GetScaleFloat(fConfigStateColorMinHeight) : fConfigStateColorMinHeight;
        fStateColorMinHeight = std::min(fStateColorMinHeight, (float)rcPaint.Height());
        if (rcStatePaintF.Height() < fStateColorMinHeight) {
            float diff = fStateColorMinHeight - rcStatePaintF.Height();
            rcStatePaintF.top -= diff / 2;
            rcStatePaintF.bottom += diff / 2;
        }
    }

    UiSize szRound = GetStateColorRound(stateType);
    if (!szRound.IsEmpty()) {
        //圆角矩形
        pRender->FillRoundRect(rcStatePaintF, (float)szRound.cx, (float)szRound.cy, colorValue, nAlpha);
    }
    else {
        //直角矩形
        pRender->FillRect(rcStatePaintF, colorValue, nAlpha);
    }
}

void StateColorMap2::SetStateColorMinWidth(float fMinWidth)
{
    m_fStateColorMinWidth = fMinWidth;
}

void StateColorMap2::SetStateColorMinHeight(float fMinHeight)
{
    m_fStateColorMinHeight = fMinHeight;
}

float StateColorMap2::GetStateColorMinWidth() const
{
    return m_fStateColorMinWidth;
}

float StateColorMap2::GetStateColorMinHeight() const
{
    return m_fStateColorMinHeight;
}

} // namespace ui
