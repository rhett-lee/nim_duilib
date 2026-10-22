#include "RangeSlider.h"
#include <algorithm>
#include <array>
#include <cmath>
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"

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
    m_bTooltipOpen(false),
    m_keepTooltipThumb(kThumbNone),
    m_nTooltipHideTaskId(0),
    m_bTooltipSuppressed(false),
    m_strTooltipFormat(),
    m_strTooltipFormatId(),
    m_strTooltipBkColor(_T("bg_tooltip")),
    m_strTooltipTextColor(_T("text_tooltip")),
    m_marks(),
    m_strMarkColor(_T("text_disabled")),
    m_strMarkLabelColor(_T("text_muted")),
    m_nMarkDotSize(6),
    m_nMarkLabelGap(6),
    m_bSnapToMarks(false),
    m_bIncluded(true),
    m_sImageModify()
{
    SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER, false);
}

RangeSlider::~RangeSlider()
{
    CancelTooltipHideTask();
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
    InvalidateExtras();
    if (m_fLowValue != fOldLow) {
        EmitRangeChanged();
    }
}

void RangeSlider::SetLowValue(double fValue)
{
    const double fOldLow = m_fLowValue;

    if (m_bSnapToMarks) {
        fValue = SnapValueToMark(fValue);
    }

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
    InvalidateExtras();
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

    if (m_bSnapToMarks) {
        fValue = SnapValueToMark(fValue);
    }

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
    InvalidateExtras();
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

void RangeSlider::SetTooltipOpen(bool bOpen)
{
    if (m_bTooltipOpen != bOpen) {
        m_bTooltipOpen = bOpen;
        InvalidateExtras();
    }
}

void RangeSlider::SetTooltipFormat(const DString& strFormat)
{
    if (m_strTooltipFormat != strFormat) {
        m_strTooltipFormat = strFormat;
        InvalidateExtras();
    }
}

void RangeSlider::SetTooltipFormatId(const DString& strFormatId)
{
    if (m_strTooltipFormatId != strFormatId) {
        m_strTooltipFormatId = strFormatId;
        InvalidateExtras();
    }
}

void RangeSlider::SetTooltipBkColor(const DString& strColor)
{
    if (m_strTooltipBkColor != strColor) {
        m_strTooltipBkColor = strColor;
        InvalidateExtras();
    }
}

void RangeSlider::SetTooltipTextColor(const DString& strColor)
{
    if (m_strTooltipTextColor != strColor) {
        m_strTooltipTextColor = strColor;
        InvalidateExtras();
    }
}

void RangeSlider::SetMarks(const std::vector<SliderMark>& marks)
{
    m_marks = marks;
    std::sort(m_marks.begin(), m_marks.end(), [](const SliderMark& a, const SliderMark& b) {
        return a.value < b.value;
    });
    InvalidateExtras();
}

void RangeSlider::SetMarks(const DString& strMarks)
{
    std::vector<SliderMark> marks;
    const std::list<DString> items = StringUtil::Split(strMarks, _T("|"));
    for (const DString& item : items) {
        DString strItem = item;
        StringUtil::Trim(strItem);
        if (strItem.empty()) {
            continue;
        }
        SliderMark mark;
        const size_t nValuePos = strItem.find(_T('='));
        DString strValue;
        DString strLabel;
        if (nValuePos == DString::npos) {
            //只有数值，没有标签
            strValue = strItem;
        }
        else {
            strValue = strItem.substr(0, nValuePos);
            strLabel = strItem.substr(nValuePos + 1);
        }
        StringUtil::Trim(strValue);
        StringUtil::Trim(strLabel);
        if (strValue.empty()) {
            continue;
        }
        mark.value = StringUtil::StringToDouble(strValue);

        //标签的颜色："label@color"
        const size_t nColorPos = strLabel.find(_T('@'));
        if (nColorPos != DString::npos) {
            mark.labelColor = strLabel.substr(nColorPos + 1);
            strLabel = strLabel.substr(0, nColorPos);
            StringUtil::Trim(mark.labelColor);
            StringUtil::Trim(strLabel);
        }
        mark.label = strLabel;
        marks.push_back(mark);
    }
    SetMarks(marks);
}

void RangeSlider::SetMarkColor(const DString& strColor)
{
    if (m_strMarkColor != strColor) {
        m_strMarkColor = strColor;
        InvalidateExtras();
    }
}

void RangeSlider::SetMarkLabelColor(const DString& strColor)
{
    if (m_strMarkLabelColor != strColor) {
        m_strMarkLabelColor = strColor;
        InvalidateExtras();
    }
}

void RangeSlider::SetMarkDotSize(int32_t nSize, bool bNeedDpiScale)
{
    if (nSize < 1) {
        nSize = 1;
    }
    if (bNeedDpiScale) {
        nSize = Dpi().GetScaleInt(nSize);
    }
    if (m_nMarkDotSize != nSize) {
        m_nMarkDotSize = nSize;
        InvalidateExtras();
    }
}

void RangeSlider::SetMarkLabelGap(int32_t nGap, bool bNeedDpiScale)
{
    if (nGap < 0) {
        nGap = 0;
    }
    if (bNeedDpiScale) {
        nGap = Dpi().GetScaleInt(nGap);
    }
    if (m_nMarkLabelGap != nGap) {
        m_nMarkLabelGap = nGap;
        InvalidateExtras();
    }
}

void RangeSlider::SetSnapToMarks(bool bSnap)
{
    if (m_bSnapToMarks == bSnap) {
        return;
    }
    m_bSnapToMarks = bSnap;
    if (!m_bSnapToMarks || m_marks.empty()) {
        InvalidateExtras();
        return;
    }

    //开启吸附后，当前值也吸附到最近的刻度
    const double fOldLow = GetLowValue();
    const double fOldHigh = GetHighValue();
    const bool bOldMute = m_bMuteRangeEvent;
    m_bMuteRangeEvent = true;
    SetLowValue(fOldLow);
    SetHighValue(fOldHigh);
    ClampLowValue();
    m_bMuteRangeEvent = bOldMute;
    InvalidateExtras();
    if ((GetLowValue() != fOldLow) || (GetHighValue() != fOldHigh)) {
        EmitRangeChanged();
    }
}

void RangeSlider::SetIncluded(bool bIncluded)
{
    if (m_bIncluded != bIncluded) {
        m_bIncluded = bIncluded;
        InvalidateExtras();
    }
}

void RangeSlider::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    if (!m_strTooltipFormatId.empty()) {
        //值气泡的文本来自多语言资源，需要刷新
        InvalidateExtras();
    }
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

double RangeSlider::SnapValueToMark(double fValue) const
{
    if (m_marks.empty()) {
        return fValue;
    }
    double fBestValue = m_marks[0].value;
    double fBestDistance = std::fabs(fValue - fBestValue);
    for (const SliderMark& mark : m_marks) {
        const double fDistance = std::fabs(fValue - mark.value);
        if (fDistance < fBestDistance) {
            fBestDistance = fDistance;
            fBestValue = mark.value;
        }
    }
    return fBestValue;
}

int32_t RangeSlider::MarkPosFromValue(double fValue) const
{
    const UiSize szThumb = GetThumbSize();
    const UiRect& rc = GetRect();
    int32_t nMin = GetMinValue();
    int32_t nMax = GetMaxValue();
    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    fValue = std::clamp(fValue, (double)nMin, (double)nMax);
    const double fRatio = (fValue - nMin) / (double)(nMax - nMin);
    if (IsHorizontal()) {
        //相对控件左上角的横向位置（刻度点圆心）
        return (int32_t)((double)(rc.Width() - szThumb.cx) * fRatio + 0.5) + szThumb.cx / 2;
    }
    //相对控件左上角的纵向位置（刻度点圆心）
    return (int32_t)((double)(rc.Height() - szThumb.cy) * (1.0 - fRatio) + 0.5) + szThumb.cy / 2;
}

DString RangeSlider::AutoFormatValue(double fValue) const
{
    if (std::fabs(fValue - std::round(fValue)) < 0.0001) {
        return StringUtil::Printf(_T("%lld"), (long long)std::llround(fValue));
    }
    DString strValue = StringUtil::Printf(_T("%.2f"), fValue);
    const size_t nPos = strValue.find(_T('.'));
    if (nPos != DString::npos) {
        while (!strValue.empty() && (strValue.back() == _T('0'))) {
            strValue.pop_back();
        }
        if (!strValue.empty() && (strValue.back() == _T('.'))) {
            strValue.pop_back();
        }
    }
    return strValue;
}

DString RangeSlider::GetTooltipText(double fValue) const
{
    DString strFormat;
    if (!m_strTooltipFormatId.empty()) {
        strFormat = GlobalManager::Instance().Lang().GetStringByID(m_strTooltipFormatId.c_str());
    }
    if (strFormat.empty()) {
        strFormat = m_strTooltipFormat;
    }
    if (!strFormat.empty()) {
        return StringUtil::Printf(strFormat.c_str(), fValue);
    }
    return AutoFormatValue(fValue);
}

void RangeSlider::PaintMarks(IRender* pRender)
{
    if ((pRender == nullptr) || m_marks.empty()) {
        return;
    }
    const UiRect& rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    const int32_t nDotSize = std::max(1, m_nMarkDotSize);
    const int32_t nGap = m_nMarkLabelGap;
    const UiColor dotColor = GetUiColor(m_strMarkColor);
    IFont* pFont = GetIFontById(GetFontId());
    const int32_t nFontHeight = (pFont != nullptr) ? pFont->FontSize() : Dpi().GetScaleInt(12);

    for (const SliderMark& mark : m_marks) {
        if ((mark.value < (double)nMin) || (mark.value > (double)nMax)) {
            //超出取值范围的刻度不绘制
            continue;
        }
        const int32_t nPos = MarkPosFromValue(mark.value);

        //刻度点
        if (dotColor.GetARGB() != 0) {
            UiPointF centerPt;
            if (IsHorizontal()) {
                centerPt = UiPointF((float)(rc.left + nPos), (float)((rc.top + rc.bottom) / 2));
            }
            else {
                centerPt = UiPointF((float)((rc.left + rc.right) / 2), (float)(rc.top + nPos));
            }
            pRender->FillCircle(centerPt, (float)nDotSize / 2.0f, dotColor, GetAlpha());
        }

        if (mark.label.empty()) {
            continue;
        }

        //刻度标签
        MeasureStringParam measureParam;
        measureParam.pFont = pFont;
        measureParam.uFormat = TEXT_SINGLELINE;
        const int32_t nTextWidth = pRender->MeasureString(mark.label, measureParam).Width();

        UiColor labelColor = GetUiColor(mark.labelColor.empty() ? m_strMarkLabelColor : mark.labelColor);
        if (labelColor.GetARGB() == 0) {
            labelColor = GetUiColor(m_strMarkLabelColor);
        }

        UiRect rcLabel;
        uint32_t uFormat = TEXT_SINGLELINE | TEXT_VCENTER | TEXT_HCENTER;
        if (IsHorizontal()) {
            //标签位于轨道下方
            int32_t nLeft = 0;
            const int32_t nCenter = rc.left + nPos;
            if (mark.value <= (double)nMin) {
                //最小值：左对齐，避免超出控件左侧
                nLeft = rc.left;
            }
            else if (mark.value >= (double)nMax) {
                //最大值：右对齐，避免超出控件右侧
                nLeft = rc.right - nTextWidth;
            }
            else {
                nLeft = nCenter - nTextWidth / 2;
                nLeft = std::clamp(nLeft, rc.left, std::max(rc.left, rc.right - nTextWidth));
            }
            rcLabel = UiRect(nLeft, rc.bottom + nGap, nLeft + nTextWidth, rc.bottom + nGap + nFontHeight);
        }
        else {
            //标签位于轨道右侧
            int32_t nTop = 0;
            const int32_t nCenter = rc.top + nPos;
            if (mark.value >= (double)nMax) {
                nTop = rc.top;
            }
            else if (mark.value <= (double)nMin) {
                nTop = rc.bottom - nFontHeight;
            }
            else {
                nTop = nCenter - nFontHeight / 2;
                nTop = std::clamp(nTop, rc.top, std::max(rc.top, rc.bottom - nFontHeight));
            }
            rcLabel = UiRect(rc.right + nGap, nTop, rc.right + nGap + nTextWidth, nTop + nFontHeight);
            uFormat = TEXT_SINGLELINE | TEXT_VCENTER | TEXT_LEFT;
        }

        DrawStringParam drawParam;
        drawParam.textRect = rcLabel;
        drawParam.dwTextColor = labelColor;
        drawParam.pFont = pFont;
        drawParam.uFormat = uFormat;
        drawParam.uFade = GetAlpha();
        pRender->DrawString(mark.label, drawParam);
    }
}

void RangeSlider::PaintTooltips(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    if (m_bTooltipOpen) {
        PaintTooltip(pRender, kThumbLow);
        PaintTooltip(pRender, kThumbHigh);
        return;
    }
    //未常显时：拖动中显示被拖动滑块的气泡；拖动结束后在延迟期内继续显示；否则显示鼠标悬停的滑块
    if (m_dragThumb != kThumbNone) {
        PaintTooltip(pRender, m_dragThumb);
    }
    else if (m_keepTooltipThumb != kThumbNone) {
        PaintTooltip(pRender, m_keepTooltipThumb);
    }
    else if (!m_bTooltipSuppressed && (m_hoverThumb != kThumbNone)) {
        PaintTooltip(pRender, m_hoverThumb);
    }
}

void RangeSlider::PaintTooltip(IRender* pRender, ThumbPart part)
{
    if ((pRender == nullptr) || (part == kThumbNone)) {
        return;
    }
    const double fValue = (part == kThumbLow) ? GetLowValue() : GetHighValue();
    const DString strText = GetTooltipText(fValue);
    if (strText.empty()) {
        return;
    }

    IFont* pFont = GetIFontById(GetFontId());
    MeasureStringParam measureParam;
    measureParam.pFont = pFont;
    measureParam.uFormat = TEXT_SINGLELINE;
    const UiRect rcMeasure = pRender->MeasureString(strText, measureParam);

    const int32_t nPaddingX = Dpi().GetScaleInt(8);
    const int32_t nPaddingY = Dpi().GetScaleInt(4);
    const int32_t nArrow = Dpi().GetScaleInt(5);
    const int32_t nGap = Dpi().GetScaleInt(4);
    const int32_t nRadius = Dpi().GetScaleInt(4);
    const int32_t nBubbleWidth = std::max(Dpi().GetScaleInt(20), rcMeasure.Width() + nPaddingX * 2);
    const int32_t nBubbleHeight = std::max(Dpi().GetScaleInt(20), rcMeasure.Height() + nPaddingY * 2);

    const UiRect rcThumb = (part == kThumbLow) ? GetLowThumbRect() : GetHighThumbRect();
    UiRect rcBubble;
    if (IsHorizontal()) {
        //水平：气泡居中显示在滑块上方，小三角朝下
        rcBubble.left = (rcThumb.left + rcThumb.right - nBubbleWidth) / 2;
        rcBubble.top = rcThumb.top - nArrow - nGap - nBubbleHeight;
        rcBubble.right = rcBubble.left + nBubbleWidth;
        rcBubble.bottom = rcBubble.top + nBubbleHeight;
    }
    else {
        //垂直：气泡居中显示在滑块左侧，小三角朝右（右侧留给刻度标签）
        rcBubble.right = rcThumb.left - nArrow - nGap;
        rcBubble.top = (rcThumb.top + rcThumb.bottom - nBubbleHeight) / 2;
        rcBubble.left = rcBubble.right - nBubbleWidth;
        rcBubble.bottom = rcBubble.top + nBubbleHeight;
    }

    const UiColor bkColor = GetUiColor(m_strTooltipBkColor);
    if (bkColor.GetARGB() != 0) {
        //气泡背景
        pRender->FillRoundRect(UiRectF::MakeFromRect(rcBubble), (float)nRadius, (float)nRadius, bkColor, GetAlpha());

        //小三角
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
            if (path != nullptr) {
                std::array<UiPointF, 3> pts;
                if (IsHorizontal()) {
                    const float fCenterX = (float)(rcBubble.left + rcBubble.right) / 2.0f;
                    pts = std::array<UiPointF, 3>{{
                        UiPointF(fCenterX - (float)nArrow, (float)rcBubble.bottom),
                        UiPointF(fCenterX + (float)nArrow, (float)rcBubble.bottom),
                        UiPointF(fCenterX, (float)rcBubble.bottom + (float)nArrow),
                    }};
                }
                else {
                    const float fCenterY = (float)(rcBubble.top + rcBubble.bottom) / 2.0f;
                    pts = std::array<UiPointF, 3>{{
                        UiPointF((float)rcBubble.right, fCenterY - (float)nArrow),
                        UiPointF((float)rcBubble.right, fCenterY + (float)nArrow),
                        UiPointF((float)rcBubble.right + (float)nArrow, fCenterY),
                    }};
                }
                path->AddPolygon(pts.data(), static_cast<int32_t>(pts.size()));
                path->Close();
                std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(bkColor));
                if (brush != nullptr) {
                    pRender->FillPath(path.get(), brush.get());
                }
            }
        }
    }

    //气泡文本
    UiColor textColor = GetUiColor(m_strTooltipTextColor);
    if (textColor.GetARGB() == 0) {
        textColor = UiColor(0xFFFFFFFF);
    }
    DrawStringParam drawParam;
    drawParam.textRect = rcBubble;
    drawParam.dwTextColor = textColor;
    drawParam.pFont = pFont;
    drawParam.uFormat = TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER;
    drawParam.uFade = GetAlpha();
    pRender->DrawString(strText, drawParam);
}

void RangeSlider::InvalidateExtras()
{
    if (!IsVisible()) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    //值气泡与刻度标签可能绘制到控件矩形之外，失效区域需要相应外扩
    IFont* pFont = GetIFontById(GetFontId());
    const int32_t nFontHeight = (pFont != nullptr) ? pFont->FontSize() : Dpi().GetScaleInt(12);
    //与 PaintTooltip 的几何参数保持一致：气泡高度 = 文本行高 + 上下内边距，另加小三角与滑块间距
    const int32_t nArrow = Dpi().GetScaleInt(5);
    const int32_t nGapToThumb = Dpi().GetScaleInt(4);
    const int32_t nBubbleHeight = std::max(Dpi().GetScaleInt(20), nFontHeight + Dpi().GetScaleInt(8));
    if (IsHorizontal()) {
        //上方：气泡 + 小三角 + 间距（额外留余量），下方：刻度标签
        rc.top -= nBubbleHeight + nArrow + nGapToThumb + Dpi().GetScaleInt(8);
        rc.bottom += nFontHeight + m_nMarkLabelGap + Dpi().GetScaleInt(4);
        //气泡以滑块为中心，可能超出控件左右边界
        rc.left -= Dpi().GetScaleInt(120);
        rc.right += Dpi().GetScaleInt(120);
    }
    else {
        //左侧：值气泡；右侧：刻度标签（宽度按经验值外扩）
        rc.left -= Dpi().GetScaleInt(120);
        rc.right += Dpi().GetScaleInt(160);
    }
    const UiPoint scrollBoxOffset = GetScrollOffsetInScrollBox();
    rc.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        pWindow->Invalidate(rc);
    }
}

void RangeSlider::CancelTooltipHideTask()
{
    if (m_nTooltipHideTaskId != 0) {
        GlobalManager::Instance().Thread().CancelTask(m_nTooltipHideTaskId);
        m_nTooltipHideTaskId = 0;
    }
}

void RangeSlider::KeepTooltip(ThumbPart part)
{
    CancelTooltipHideTask();
    m_bTooltipSuppressed = false;
    if (m_keepTooltipThumb != part) {
        m_keepTooltipThumb = part;
        InvalidateExtras();
    }
}

void RangeSlider::ScheduleTooltipHide()
{
    CancelTooltipHideTask();
    if (m_keepTooltipThumb == kThumbNone) {
        return;
    }
    //拖动结束后继续显示一段时间，再自动消失
    constexpr int32_t kTooltipHideDelayMs = 1000;
    m_nTooltipHideTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
        kThreadUI,
        [this]() {
            m_nTooltipHideTaskId = 0;
            m_keepTooltipThumb = kThumbNone;
            //抑制悬停显示，避免鼠标仍停在滑块上时气泡被立即重新绘制
            m_bTooltipSuppressed = true;
            InvalidateExtras();
        },
        kTooltipHideDelayMs);
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
            KeepTooltip(m_dragThumb);
            InvalidateExtras();
            return;
        }
        if (GetHighThumbRect().ContainsPt(newPtMouse)) {
            m_dragThumb = kThumbHigh;
            SetMouseFocused(true);
            KeepTooltip(m_dragThumb);
            InvalidateExtras();
            return;
        }
        //点击轨道：默认按下即跳值，并进入拖动状态（移动离点击位置更近的那个滑块）
        if (m_bClickToJump && IsEnabled() && IsMouseEnabled()) {
            const double fValue = ValueFromMousePt(newPtMouse);
            const double fDistLow = std::fabs(fValue - GetLowValue());
            const double fDistHigh = std::fabs(fValue - GetHighValue());
            //两个滑块重合时，优先选择能够向点击值移动的那个滑块，
            //避免一直选中被另一个滑块挡住、无法移动的滑块。
            const bool bCanMoveLow = (fValue <= GetHighValue() - m_fMinRange + 0.0001);
            const bool bCanMoveHigh = (fValue >= GetLowValue() + m_fMinRange - 0.0001);
            bool bDragLow = (fDistLow <= fDistHigh);
            if (bDragLow && !bCanMoveLow && bCanMoveHigh) {
                bDragLow = false;
            }
            else if (!bDragLow && !bCanMoveHigh && bCanMoveLow) {
                bDragLow = true;
            }
            if (bDragLow) {
                m_dragThumb = kThumbLow;
                SetLowValue(fValue);
            }
            else {
                m_dragThumb = kThumbHigh;
                SetHighValue(fValue);
            }
            SetMouseFocused(true);
            KeepTooltip(m_dragThumb);
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
                    if ((fValue > GetLowValue()) && (GetLowValue() + 0.0001 >= GetHighValue() - m_fMinRange)) {
                        SetHighValue(fValue);
                    }
                    else {
                        SetLowValue(fValue);
                    }
                }
                else if (m_dragThumb == kThumbHigh) {
                    if ((fValue < GetHighValue()) && (GetHighValue() - 0.0001 <= GetLowValue() + m_fMinRange)) {
                        SetLowValue(fValue);
                    }
                    else {
                        SetHighValue(fValue);
                    }
                }
            }
            m_dragThumb = kThumbNone;
            SetMouseFocused(false);
            //松开鼠标后气泡再保留一段时间，然后自动消失
            ScheduleTooltipHide();
            InvalidateExtras();
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
            InvalidateExtras();
        }

        if (IsMouseFocused()) {
            const double fValue = ValueFromMousePt(ptMouse);
            if (m_dragThumb == kThumbLow) {
                if ((fValue > GetLowValue()) && (GetLowValue() + 0.0001 >= GetHighValue() - m_fMinRange)) {
                    //下限被上限挡住：改为拖动上限，使两个滑块重合时也能继续向上（向右）滑动
                    m_dragThumb = kThumbHigh;
                    SetHighValue(fValue);
                }
                else {
                    SetLowValue(fValue);
                }
            }
            else if (m_dragThumb == kThumbHigh) {
                if ((fValue < GetHighValue()) && (GetHighValue() - 0.0001 <= GetLowValue() + m_fMinRange)) {
                    //上限被下限挡住：改为拖动下限，使两个滑块重合时也能继续向下（向左）滑动
                    m_dragThumb = kThumbLow;
                    SetLowValue(fValue);
                }
                else {
                    SetHighValue(fValue);
                }
            }
            KeepTooltip(m_dragThumb);
            return;
        }
        return;
    }

    if (msg.eventType == kEventMouseEnter) {
        //鼠标重新进入控件后，恢复"悬停显示气泡"的行为
        if (m_bTooltipSuppressed) {
            m_bTooltipSuppressed = false;
            InvalidateExtras();
        }
    }

    if (msg.eventType == kEventMouseLeave) {
        if (m_hoverThumb != kThumbNone) {
            m_hoverThumb = kThumbNone;
            m_bTooltipSuppressed = false;
            InvalidateExtras();
        }
        if (!IsMouseFocused()) {
            //鼠标离开且未在拖动：立即隐藏气泡
            CancelTooltipHideTask();
            m_keepTooltipThumb = kThumbNone;
            m_bTooltipSuppressed = false;
            InvalidateExtras();
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
    else if ((strName == _T("tooltip_open")) || (strName == _T("tooltipopen"))) {
        SetTooltipOpen(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("tooltip_format")) || (strName == _T("tooltipformat"))) {
        SetTooltipFormat(strValue);
    }
    else if ((strName == _T("tooltip_format_id")) || (strName == _T("tooltipformatid"))) {
        SetTooltipFormatId(strValue);
    }
    else if ((strName == _T("tooltip_bkcolor")) || (strName == _T("tooltip_bg_color")) ||
             (strName == _T("tooltipbkcolor")) || (strName == _T("tooltipbgcolor"))) {
        SetTooltipBkColor(strValue);
    }
    else if ((strName == _T("tooltip_text_color")) || (strName == _T("tooltiptextcolor"))) {
        SetTooltipTextColor(strValue);
    }
    else if (strName == _T("marks")) {
        SetMarks(strValue);
    }
    else if ((strName == _T("mark_color")) || (strName == _T("mark_dot_color")) ||
             (strName == _T("markcolor")) || (strName == _T("markdotcolor"))) {
        SetMarkColor(strValue);
    }
    else if ((strName == _T("mark_label_color")) || (strName == _T("marklabelcolor"))) {
        SetMarkLabelColor(strValue);
    }
    else if ((strName == _T("mark_dot_size")) || (strName == _T("markdotsize"))) {
        SetMarkDotSize(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("mark_label_gap")) || (strName == _T("marklabelgap"))) {
        SetMarkLabelGap(StringUtil::StringToInt32(strValue), true);
    }
    else if ((strName == _T("snap_to_marks")) || (strName == _T("snaptomarks"))) {
        SetSnapToMarks(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("included")) {
        SetIncluded(StringUtil::IsValueTrue(strValue));
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

    m_nMarkDotSize = Dpi().GetScaleInt(m_nMarkDotSize, nOldDpiScale);
    m_nMarkLabelGap = Dpi().GetScaleInt(m_nMarkLabelGap, nOldDpiScale);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void RangeSlider::PaintStateImages(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    //高亮区间（颜色未配置时回退使用 progress_color）
    const DString strRangeColor = m_strRangeColor.empty() ? GetProgressColor() : m_strRangeColor;
    const UiColor rangeColor = GetUiColor(strRangeColor);
    const UiRect& rc = GetRect();
    if (rangeColor.GetARGB() != 0) {
        const UiRect rcLow = CalcRangeRect(GetLowValue());
        const UiRect rcHigh = CalcRangeRect(GetHighValue());

        int32_t nRadius = 0;
        auto PaintBand = [&](int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom) {
            UiRect rcBand(nLeft, nTop, nRight, nBottom);
            if (rcBand.IsEmpty()) {
                return;
            }
            if (nRadius > 0) {
                const int32_t nMaxRadius = std::min(rcBand.Width(), rcBand.Height()) / 2;
                const int32_t nUseRadius = std::min(nRadius, nMaxRadius);
                pRender->FillRoundRect(UiRectF::MakeFromRect(rcBand), (float)nUseRadius, (float)nUseRadius, rangeColor, GetAlpha());
            }
            else {
                pRender->FillRect(UiRectF::MakeFromRect(rcBand), rangeColor, GetAlpha());
            }
        };

        nRadius = Dpi().GetScaleInt(m_nRangeRadius);
        if (IsHorizontal()) {
            if (m_bIncluded) {
                //高亮两个滑块之间
                PaintBand(rc.left + rcLow.right, rc.top, rc.left + rcHigh.right, rc.bottom);
            }
            else {
                //高亮滑块外侧的两段
                PaintBand(rc.left, rc.top, rc.left + rcLow.right, rc.bottom);
                PaintBand(rc.left + rcHigh.right, rc.top, rc.right, rc.bottom);
            }
        }
        else {
            if (m_bIncluded) {
                PaintBand(rc.left, rc.top + rcHigh.top, rc.right, rc.top + rcLow.top);
            }
            else {
                PaintBand(rc.left, rc.top, rc.right, rc.top + rcHigh.top);
                PaintBand(rc.left, rc.top + rcLow.top, rc.right, rc.bottom);
            }
        }
    }

    //刻度点与刻度标签
    PaintMarks(pRender);

    //两个滑块
    PaintThumb(pRender, kThumbLow);
    PaintThumb(pRender, kThumbHigh);

    //值气泡（绘制在最上层）
    PaintTooltips(pRender);
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
