#include "RangeSlider.h"
#include <algorithm>
#include <cmath>
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

RangeSlider::RangeSlider(Window* pWindow) :
    Progress(pWindow),
    m_fLowValue(0),
    m_fMinRange(0),
    m_szThumb(0, 0),
    m_thumbStateImage(),
    m_highThumbStateImage(),
    m_strRangeColor(),
    m_nRangeRadius(0),
    m_bClickToJump(true),
    m_bMuteRangeEvent(false),
    m_bHighValueSet(false),
    m_dragThumb(kThumbNone),
    m_hoverThumb(kThumbNone),
    m_sImageModify()
{
    SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER, false);
}

DString RangeSlider::GetType() const { return DUI_CTR_RANGE_SLIDER; }

void RangeSlider::SetMinRange(double fMinRange)
{
    if (fMinRange < 0) {
        fMinRange = 0;
    }
    if (m_fMinRange == fMinRange) {
        return;
    }
    m_fMinRange = fMinRange;

    //最小间距变大后，下限可能越界，需要重新钳位
    const double fOldLow = m_fLowValue;
    ClampLowValue();
    Invalidate();
    if (m_fLowValue != fOldLow) {
        EmitRangeChanged();
    }
}

void RangeSlider::SetLowValue(double fValue)
{
    const double fOldLow = m_fLowValue;

    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    if (fValue < (double)nMin) {
        fValue = (double)nMin;
    }
    if (fValue > (double)nMax) {
        fValue = (double)nMax;
    }
    //下限不得超过上限（保留最小间距）
    //上限尚未被有效设置时，下限只受 [min, max] 约束，避免初始化阶段被默认上限值钳死
    double fLowMax = (double)nMax;
    if (m_bHighValueSet) {
        fLowMax = GetHighValue() - m_fMinRange;
        if (fLowMax < (double)nMin) {
            fLowMax = (double)nMin;
        }
    }
    if (fValue > fLowMax) {
        fValue = fLowMax;
    }

    if (fValue == fOldLow) {
        return;
    }
    m_fLowValue = fValue;
    Invalidate();
    EmitRangeChanged();
}

void RangeSlider::SetLowValueNoEvent(double fValue)
{
    const bool bOldMute = m_bMuteRangeEvent;
    m_bMuteRangeEvent = true;
    SetLowValue(fValue);
    m_bMuteRangeEvent = bOldMute;
}

void RangeSlider::SetHighValue(double fValue)
{
    const double fOldHigh = GetHighValue();

    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    double fHighMin = GetLowValue() + m_fMinRange;
    if (fHighMin < (double)nMin) {
        fHighMin = (double)nMin;
    }
    if (fValue < fHighMin) {
        fValue = fHighMin;
    }
    if (fValue > (double)nMax) {
        fValue = (double)nMax;
    }

    m_bHighValueSet = true;
    if (fValue == fOldHigh) {
        return;
    }
    //使用基类的静默赋值，抑制基类自身的 kEventValueChanged，由本控件统一派发范围事件
    Progress::SetValueNoEvent(fValue);
    Invalidate();
    EmitRangeChanged();
}

void RangeSlider::SetHighValueNoEvent(double fValue)
{
    const bool bOldMute = m_bMuteRangeEvent;
    m_bMuteRangeEvent = true;
    SetHighValue(fValue);
    m_bMuteRangeEvent = bOldMute;
}

void RangeSlider::SetRange(double fLowValue, double fHighValue)
{
    const double fOldLow = GetLowValue();
    const double fOldHigh = GetHighValue();

    //先设下限，再设上限（此时下限已就绪，可正确约束上限）
    const bool bOldMute = m_bMuteRangeEvent;
    m_bMuteRangeEvent = true;
    SetLowValue(fLowValue);
    SetHighValue(fHighValue);
    ClampLowValue();
    m_bMuteRangeEvent = bOldMute;

    if ((GetLowValue() != fOldLow) || (GetHighValue() != fOldHigh)) {
        EmitRangeChanged();
    }
}

void RangeSlider::SetRangeNoEvent(double fLowValue, double fHighValue)
{
    const bool bOldMute = m_bMuteRangeEvent;
    m_bMuteRangeEvent = true;
    SetRange(fLowValue, fHighValue);
    m_bMuteRangeEvent = bOldMute;
}

void RangeSlider::ClampLowValue()
{
    const double fMin = (double)GetMinValue();
    const double fMax = (double)GetMaxValue();

    double fLowMax = fMax;
    if (m_bHighValueSet) {
        fLowMax = GetHighValue() - m_fMinRange;
        if (fLowMax < fMin) {
            fLowMax = fMin;
        }
    }

    double fLow = m_fLowValue;
    if (fLow < fMin) {
        fLow = fMin;
    }
    if (fLow > fLowMax) {
        fLow = fLowMax;
    }
    if (fLow > fMax) {
        fLow = fMax;
    }
    m_fLowValue = fLow;
}

void RangeSlider::EmitRangeChanged()
{
    if (m_bMuteRangeEvent) {
        return;
    }
    //范围值通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    RangeValue range;
    range.lowValue = GetLowValue();
    range.highValue = GetHighValue();
    SendEvent(kEventValueChanged, (WPARAM)1, 0, static_cast<void*>(&range));
}

void RangeSlider::SetThumbSize(UiSize szXY, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleSize(szXY);
    }
    m_szThumb = szXY;
}

UiSize RangeSlider::GetThumbSize() const
{
    UiSize szThumb = m_szThumb;
    if (szThumb.IsEmpty()) {
        szThumb = { 10, 10 };
        Dpi().ScaleSize(szThumb);
    }
    return szThumb;
}

DString RangeSlider::GetThumbStateImage(ControlStateType stateType) const
{
    return m_thumbStateImage.GetImageString(stateType);
}

void RangeSlider::SetThumbStateImage(ControlStateType stateType, const DString& strImage)
{
    m_thumbStateImage.SetImageString(stateType, strImage, Dpi());
    Invalidate();
}

DString RangeSlider::GetHighThumbStateImage(ControlStateType stateType) const
{
    return m_highThumbStateImage.GetImageString(stateType);
}

void RangeSlider::SetHighThumbStateImage(ControlStateType stateType, const DString& strImage)
{
    m_highThumbStateImage.SetImageString(stateType, strImage, Dpi());
    Invalidate();
}

void RangeSlider::SetRangeColor(const DString& strColor)
{
    if (m_strRangeColor != strColor) {
        m_strRangeColor = strColor;
        Invalidate();
    }
}

void RangeSlider::SetRangeRadius(int32_t nRadius)
{
    if (nRadius < 0) {
        nRadius = 0;
    }
    if (m_nRangeRadius != nRadius) {
        m_nRangeRadius = nRadius;
        Invalidate();
    }
}

void RangeSlider::SetClickToJump(bool bEnable)
{
    m_bClickToJump = bEnable;
}

UiRect RangeSlider::ThumbRectFromValue(double fValue) const
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > (double)nMax) {
        fValue = (double)nMax;
    }
    if (fValue < (double)nMin) {
        fValue = (double)nMin;
    }

    const UiSize szThumb = GetThumbSize();
    const UiRect& rc = GetRect();
    if (IsHorizontal()) {
        int32_t left = (int32_t)(rc.left + (double)(rc.right - rc.left - szThumb.cx) * (fValue - nMin) / (nMax - nMin) + 0.5);
        int32_t top = (rc.bottom + rc.top - szThumb.cy) / 2;
        return UiRect(left, top, left + szThumb.cx, top + szThumb.cy);
    }
    else {
        int32_t left = (rc.right + rc.left - szThumb.cx) / 2;
        int32_t top = (int32_t)(rc.bottom - szThumb.cy - (double)(rc.bottom - rc.top - szThumb.cy) * (fValue - nMin) / (nMax - nMin) + 0.5);
        return UiRect(left, top, left + szThumb.cx, top + szThumb.cy);
    }
}

UiRect RangeSlider::CalcRangeRect(double fValue) const
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > (double)nMax) {
        fValue = (double)nMax;
    }
    if (fValue < (double)nMin) {
        fValue = (double)nMin;
    }
    double fRatio = (fValue - nMin) / (double)(nMax - nMin);
    fRatio = std::clamp(fRatio, 0.0, 1.0);

    UiRect rc;
    if (IsHorizontal()) {
        rc.right = (int32_t)std::ceil(fRatio * (GetRect().right - GetRect().left));
        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        rc.top = (int32_t)std::ceil((1.0 - fRatio) * (GetRect().bottom - GetRect().top));
        rc.bottom = GetRect().bottom - GetRect().top;
        rc.right = GetRect().right - GetRect().left;
    }
    return rc;
}

double RangeSlider::ValueFromMousePt(const UiPoint& ptMouse) const
{
    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    const UiSize szThumb = GetThumbSize();
    const UiRect& rc = GetRect();
    if (IsHorizontal()) {
        if (ptMouse.x >= rc.right - szThumb.cx / 2) {
            return (double)nMax;
        }
        if (ptMouse.x <= rc.left + szThumb.cx / 2) {
            return (double)nMin;
        }
        return nMin + (double)((nMax - nMin) * (ptMouse.x - rc.left - szThumb.cx / 2)) /
            (double)(rc.right - rc.left - szThumb.cx);
    }
    else {
        if (ptMouse.y >= rc.bottom - szThumb.cy / 2) {
            return (double)nMin;
        }
        if (ptMouse.y <= rc.top + szThumb.cy / 2) {
            return (double)nMax;
        }
        return nMin + (double)((nMax - nMin) * (rc.bottom - ptMouse.y - szThumb.cy / 2)) /
            (double)(rc.bottom - rc.top - szThumb.cy);
    }
}

void RangeSlider::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }

    if ((msg.eventType == kEventMouseButtonDown) || (msg.eventType == kEventMouseDoubleClick)) {
        UiPoint newPtMouse(msg.ptMouse);
        newPtMouse.Offset(GetScrollOffsetInScrollBox());
        if (GetLowThumbRect().ContainsPt(newPtMouse)) {
            m_dragThumb = kThumbLow;
            SetMouseFocused(true);
            return;
        }
        if (GetHighThumbRect().ContainsPt(newPtMouse)) {
            m_dragThumb = kThumbHigh;
            SetMouseFocused(true);
            return;
        }
        //点击轨道：默认按下即跳值，并进入拖动状态（移动离点击位置更近的那个滑块）
        if (m_bClickToJump && IsEnabled() && IsMouseEnabled()) {
            const double fValue = ValueFromMousePt(newPtMouse);
            const double fDistLow = std::fabs(fValue - GetLowValue());
            const double fDistHigh = std::fabs(fValue - GetHighValue());
            if (fDistLow <= fDistHigh) {
                m_dragThumb = kThumbLow;
                SetLowValue(fValue);
            }
            else {
                m_dragThumb = kThumbHigh;
                SetHighValue(fValue);
            }
            SetMouseFocused(true);
        }
        return;
    }

    if (msg.eventType == kEventMouseButtonUp) {
        if (IsMouseFocused()) {
            //点击跳转关闭时，松开鼠标才跳值
            if (!m_bClickToJump) {
                UiPoint ptMouse(msg.ptMouse);
                ptMouse.Offset(GetScrollOffsetInScrollBox());
                const double fValue = ValueFromMousePt(ptMouse);
                if (m_dragThumb == kThumbLow) {
                    SetLowValue(fValue);
                }
                else if (m_dragThumb == kThumbHigh) {
                    SetHighValue(fValue);
                }
            }
            m_dragThumb = kThumbNone;
            SetMouseFocused(false);
        }
        return;
    }

    if (msg.eventType == kEventMouseMove) {
        UiPoint ptMouse(msg.ptMouse);
        ptMouse.Offset(GetScrollOffsetInScrollBox());

        //更新鼠标悬停的滑块（用于绘制 hovered 状态图片）
        const ThumbPart oldHover = m_hoverThumb;
        if (GetLowThumbRect().ContainsPt(ptMouse)) {
            m_hoverThumb = kThumbLow;
        }
        else if (GetHighThumbRect().ContainsPt(ptMouse)) {
            m_hoverThumb = kThumbHigh;
        }
        else {
            m_hoverThumb = kThumbNone;
        }
        if (oldHover != m_hoverThumb) {
            Invalidate();
        }

        if (IsMouseFocused()) {
            const double fValue = ValueFromMousePt(ptMouse);
            if (m_dragThumb == kThumbLow) {
                SetLowValue(fValue);
            }
            else if (m_dragThumb == kThumbHigh) {
                SetHighValue(fValue);
            }
            return;
        }
        return;
    }

    if (msg.eventType == kEventMouseLeave) {
        if (m_hoverThumb != kThumbNone) {
            m_hoverThumb = kThumbNone;
            Invalidate();
        }
    }

    Progress::HandleEvent(msg);
}

void RangeSlider::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("low_value")) || (strName == _T("lowvalue"))) {
        SetLowValue(StringUtil::StringToDouble(strValue));
    }
    else if ((strName == _T("high_value")) || (strName == _T("highvalue"))) {
        SetHighValue(StringUtil::StringToDouble(strValue));
    }
    else if (strName == _T("value")) {
        //上限值（与单滑块控件语义保持一致）
        SetHighValue(StringUtil::StringToDouble(strValue));
    }
    else if ((strName == _T("min_range")) || (strName == _T("minrange"))) {
        SetMinRange(StringUtil::StringToDouble(strValue));
    }
    else if ((strName == _T("range_color")) || (strName == _T("rangecolor"))) {
        SetRangeColor(strValue);
    }
    else if ((strName == _T("range_radius")) || (strName == _T("rangeradius"))) {
        SetRangeRadius(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("click_to_jump")) || (strName == _T("clicktojump"))) {
        SetClickToJump(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("thumb_size")) || (strName == _T("thumbsize"))) {
        UiSize szXY;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szXY);
        SetThumbSize(szXY, true);
    }
    else if ((strName == _T("thumb_normal_image")) || (strName == _T("thumbnormalimage"))) {
        SetThumbStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == _T("thumb_hovered_image")) || (strName == _T("thumb_hot_image")) || (strName == _T("thumbhotimage"))) {
        SetThumbStateImage(kControlStateHovered, strValue);
    }
    else if ((strName == _T("thumb_pressed_image")) || (strName == _T("thumb_pushed_image")) || (strName == _T("thumbpushedimage"))) {
        SetThumbStateImage(kControlStatePressed, strValue);
    }
    else if ((strName == _T("thumb_disabled_image")) || (strName == _T("thumbdisabledimage"))) {
        SetThumbStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("high_thumb_normal_image")) || (strName == _T("highthumbnormalimage"))) {
        SetHighThumbStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == _T("high_thumb_hovered_image")) || (strName == _T("high_thumb_hot_image")) || (strName == _T("highthumbhotimage"))) {
        SetHighThumbStateImage(kControlStateHovered, strValue);
    }
    else if ((strName == _T("high_thumb_pressed_image")) || (strName == _T("high_thumb_pushed_image")) || (strName == _T("highthumbpushedimage"))) {
        SetHighThumbStateImage(kControlStatePressed, strValue);
    }
    else if ((strName == _T("high_thumb_disabled_image")) || (strName == _T("highthumbdisabledimage"))) {
        SetHighThumbStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("min")) || (strName == _T("max"))) {
        BaseClass::SetAttribute(strName, strValue);
        //边界变化后下限可能越界，需要重新钳位
        ClampLowValue();
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void RangeSlider::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiSize szThumbSize = GetThumbSize();
    szThumbSize = Dpi().GetScaleSize(szThumbSize, nOldDpiScale);
    SetThumbSize(szThumbSize, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void RangeSlider::PaintStateImages(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //高亮选中区间（颜色未配置时回退使用 progress_color）
    const DString strRangeColor = m_strRangeColor.empty() ? GetProgressColor() : m_strRangeColor;
    const UiColor rangeColor = GetUiColor(strRangeColor);
    if (rangeColor.GetARGB() != 0) {
        const UiRect rcLow = CalcRangeRect(GetLowValue());
        const UiRect rcHigh = CalcRangeRect(GetHighValue());
        const UiRect& rc = GetRect();
        UiRect rcBand;
        if (IsHorizontal()) {
            rcBand.left = rc.left + rcLow.right;
            rcBand.right = rc.left + rcHigh.right;
            rcBand.top = rc.top;
            rcBand.bottom = rc.bottom;
        }
        else {
            rcBand.left = rc.left;
            rcBand.right = rc.right;
            rcBand.top = rc.top + rcHigh.top;
            rcBand.bottom = rc.top + rcLow.top;
        }
        if (!rcBand.IsEmpty()) {
            int32_t nRadius = Dpi().GetScaleInt(m_nRangeRadius);
            if (nRadius > 0) {
                const int32_t nMaxRadius = std::min(rcBand.Width(), rcBand.Height()) / 2;
                if (nRadius > nMaxRadius) {
                    nRadius = nMaxRadius;
                }
            }
            if (nRadius > 0) {
                pRender->FillRoundRect(UiRectF::MakeFromRect(rcBand), (float)nRadius, (float)nRadius, rangeColor, GetAlpha());
            }
            else {
                pRender->FillRect(UiRectF::MakeFromRect(rcBand), rangeColor, GetAlpha());
            }
        }
    }

    //两个滑块
    PaintThumb(pRender, kThumbLow);
    PaintThumb(pRender, kThumbHigh);
}

void RangeSlider::PaintThumb(IRender* pRender, ThumbPart part)
{
    UiRect rcThumb = (part == kThumbLow) ? GetLowThumbRect() : GetHighThumbRect();
    rcThumb.left -= GetRect().left;
    rcThumb.top -= GetRect().top;
    rcThumb.right -= GetRect().left;
    rcThumb.bottom -= GetRect().top;

    //选择图片集合：上限滑块单独配置了图片时优先使用，否则回退使用下限滑块图片
    const StateImage* pStateImage = &m_thumbStateImage;
    if ((part == kThumbHigh) && m_highThumbStateImage.HasImage()) {
        pStateImage = &m_highThumbStateImage;
    }

    //状态优先级：禁用 > 拖动中(按下) > 悬停 > 普通
    ControlStateType stateType = kControlStateNormal;
    if (!IsEnabled()) {
        stateType = kControlStateDisabled;
    }
    else if ((m_dragThumb == part) && IsMouseFocused()) {
        stateType = kControlStatePressed;
    }
    else if ((GetState() == kControlStateHovered) && (m_hoverThumb == part)) {
        stateType = kControlStateHovered;
    }

    Image* pThumbImage = pStateImage->GetStateImage(stateType);
    if ((pThumbImage == nullptr) && (stateType != kControlStateNormal)) {
        //回退到普通状态图片
        pThumbImage = pStateImage->GetStateImage(kControlStateNormal);
    }
    if (pThumbImage == nullptr) {
        return;
    }

    m_sImageModify.clear();
    m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
    PaintImage(pRender, pThumbImage, m_sImageModify.c_str());
}

void RangeSlider::ClearImageCache()
{
    BaseClass::ClearImageCache();
    m_thumbStateImage.ClearImageCache();
    m_highThumbStateImage.ClearImageCache();
}

} // namespace ui
