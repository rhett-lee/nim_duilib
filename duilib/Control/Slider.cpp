#include "Slider.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{

Slider::Slider(Window* pWindow) :
    Progress(pWindow),
    m_szThumb(0, 0),
    m_nStep(1),
    m_sImageModify(),
    m_thumbStateImage(),
    m_rcProgressBarPadding()
{
    SetTextStyle(TEXT_SINGLELINE | TEXT_CENTER, false);
}

DString Slider::GetType() const { return DUI_CTR_SLIDER; }

UiRect Slider::GetProgressPos()
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    double fValue = GetValue();

    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > nMax) {
        fValue = nMax;
    }
    if (fValue < nMin) {
        fValue = nMin;
    }

    UiSize szThumb = GetThumbSize();
    UiRect rc;
    if (IsHorizontal()) {
        rc.right = int((fValue - nMin) * (GetRect().right - GetRect().left - szThumb.cx) / (nMax - nMin) + szThumb.cx / 2 + 0.5);
        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        rc.top = int((nMax - fValue) * (GetRect().bottom - GetRect().top - szThumb.cy) / (nMax - nMin) + szThumb.cy / 2 + 0.5);
        rc.right = GetRect().right - GetRect().left;
        rc.bottom = GetRect().bottom - GetRect().top;
    }

    return rc;
}

void Slider::HandleEvent(const EventArgs& msg)
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
    if (msg.eventType == kEventMouseButtonDown || msg.eventType == kEventMouseDoubleClick) {
        UiPoint newPtMouse(msg.ptMouse);
        newPtMouse.Offset(GetScrollOffsetInScrollBox());
        UiRect rcThumb = GetThumbRect();
        if (rcThumb.ContainsPt(newPtMouse)) {
            SetMouseFocused(true);
        }
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        UiSize szThumb = GetThumbSize();
        double oldValue = GetValue();
        if(IsMouseFocused()) {
            SetMouseFocused(false);
        }
        const int32_t nMin = GetMinValue();
        const int32_t nMax = GetMaxValue();
        if (IsHorizontal()) {
            if (msg.ptMouse.x >= GetRect().right - szThumb.cx / 2) {
                SetValue(nMax);
            }
            else if (msg.ptMouse.x <= GetRect().left + szThumb.cx / 2) {
                SetValue(nMin);
            }
            else {
                double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - szThumb.cx / 2)) / (GetRect().right - GetRect().left - szThumb.cx);
                SetValue(newValue);
            }
        }
        else {
            if (msg.ptMouse.y >= GetRect().bottom - szThumb.cy / 2) {
                SetValue(nMin);
            }
            else if (msg.ptMouse.y <= GetRect().top + szThumb.cy / 2) {
                SetValue(nMax);
            }
            else {
                double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - szThumb.cy);
                SetValue(newValue);
            }
        }
        SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
        Invalidate();
        return;
    }
    if (msg.eventType == kEventMouseWheel) {
        double oldValue = GetValue();
        int32_t detaValue = msg.eventData;
        if (detaValue > 0) {
            SetValue(GetValue() + GetChangeStep());
            SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
            return;
        }
        else {
            SetValue(GetValue() - GetChangeStep());
            SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
            return;
        }
    }
    if (msg.eventType == kEventMouseMove) {
        UiSize szThumb = GetThumbSize();
        const int32_t nMin = GetMinValue();
        const int32_t nMax = GetMaxValue();
        if (IsMouseFocused()) {
            double oldValue = GetValue();
            if (IsHorizontal()) {
                if (msg.ptMouse.x >= GetRect().right - szThumb.cx / 2) {
                    SetValue(nMax);
                }
                else if (msg.ptMouse.x <= GetRect().left + szThumb.cx / 2) {
                    SetValue(nMin);
                }
                else {
                    double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - szThumb.cx / 2)) / (GetRect().right - GetRect().left - szThumb.cx);
                    SetValue(newValue);
                }
            }
            else {
                if (msg.ptMouse.y >= GetRect().bottom - szThumb.cy / 2) {
                    SetValue(nMin);
                }
                else if (msg.ptMouse.y <= GetRect().top + szThumb.cy / 2) {
                    SetValue(nMax);
                }
                else {
                    double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - szThumb.cy);
                    SetValue(newValue);
                }
            }
            SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
            Invalidate();
        }
        return;
    }

    Progress::HandleEvent(msg);
}

void Slider::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("step")) {
        SetChangeStep(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("thumb_normal_image")) || (strName == _T("thumbnormalimage"))) {
        SetThumbStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == _T("thumb_hot_image")) || (strName == _T("thumbhotimage"))) {
        SetThumbStateImage(kControlStateHot, strValue);
    }
    else if ((strName == _T("thumb_pushed_image")) || (strName == _T("thumbpushedimage"))) {
        SetThumbStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == _T("thumb_disabled_image")) || (strName == _T("thumbdisabledimage"))) {
        SetThumbStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("thumb_size")) || (strName == _T("thumbsize"))) {
        UiSize szXY;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szXY);
        SetThumbSize(szXY, true);
    }
    else if ((strName == _T("progress_bar_padding")) || (strName == _T("progressbarpadding"))) {
        UiPadding rcPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
        SetProgressBarPadding(rcPadding, true);
    }
    else {
        Progress::SetAttribute(strName, strValue);
    }
}

void Slider::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    UiSize szThumbSize = GetThumbSize();
    szThumbSize = Dpi().GetScaleSize(szThumbSize, nOldDpiScale);
    SetThumbSize(szThumbSize, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Slider::PaintBkColor(IRender* pRender)
{
    UiRect rc = GetRect();
    const UiPadding& padding = m_rcProgressBarPadding;
    rc.Deflate(padding);
    SetRect(rc);

    UiRect painttRect = GetPaintRect();
    painttRect.Deflate(padding);
    SetPaintRect(painttRect);

    Control::PaintBkColor(pRender);

    painttRect = GetPaintRect();
    painttRect.Inflate(padding);
    SetPaintRect(painttRect);

    rc = GetRect();
    rc.Inflate(padding);
    SetRect(rc);
}

void Slider::PaintStateImages(IRender* pRender)
{
    UiRect rc = GetRect();
    const UiPadding& padding = m_rcProgressBarPadding;
    rc.Deflate(padding);
    SetRect(rc);

    Progress::PaintStateImages(pRender);

    rc = GetRect();
    rc.Inflate(padding.left, padding.top, padding.right, padding.bottom);
    SetRect(rc);

    UiRect rcThumb = GetThumbRect();
    rcThumb.left -= GetRect().left;
    rcThumb.top -= GetRect().top;
    rcThumb.right -= GetRect().left;
    rcThumb.bottom -= GetRect().top;

    if (IsMouseFocused()) {
        m_sImageModify.clear();
        m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStatePushed), m_sImageModify.c_str())) {
            return;
        }
    }
    else if (GetState() == kControlStateHot) {
        m_sImageModify.clear();
        m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateHot), m_sImageModify.c_str())) {
            return;
        }
    }

    m_sImageModify.clear();
    m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
    PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateNormal), m_sImageModify.c_str());
}

void Slider::ClearImageCache()
{
    BaseClass::ClearImageCache();
    m_thumbStateImage.ClearImageCache();
}

int Slider::GetChangeStep()
{
    return m_nStep;
}

void Slider::SetChangeStep(int step)
{
    m_nStep = step;
}

void Slider::SetThumbSize(UiSize szXY, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScaleSize(szXY);
    }    
    m_szThumb = szXY;
}

UiSize Slider::GetThumbSize() const
{
    UiSize szThumb = m_szThumb;
    if (szThumb.IsEmpty()) {
        szThumb = {10, 10};
        Dpi().ScaleSize(szThumb);
    }
    return szThumb;
}

UiRect Slider::GetThumbRect() const
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    double fValue = GetValue();

    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > nMax) {
        fValue = nMax;
    }
    if (fValue < nMin) {
        fValue = nMin;
    }

    UiSize szThumb = GetThumbSize();
    if(IsHorizontal()) {
        int32_t left = int32_t(GetRect().left + (GetRect().right - GetRect().left - szThumb.cx) * (fValue - nMin) / (nMax - nMin));
        int32_t top = (GetRect().bottom + GetRect().top - szThumb.cy) / 2;
        return UiRect(left, top, left + szThumb.cx, top + szThumb.cy);
    }
    else {
        int32_t left = (GetRect().right + GetRect().left - szThumb.cx) / 2;
        int32_t top = int32_t(GetRect().bottom - szThumb.cy - (GetRect().bottom - GetRect().top - szThumb.cy) * (fValue - nMin) / (nMax - nMin));
        return UiRect(left, top, left + szThumb.cx, top + szThumb.cy);
    }
}

DString Slider::GetThumbStateImage(ControlStateType stateType) const
{
    return m_thumbStateImage.GetImageString(stateType);
}

void Slider::SetThumbStateImage(ControlStateType stateType, const DString& pStrImage)
{
    m_thumbStateImage.SetImageString(stateType, pStrImage, Dpi());
    Invalidate();
}

const UiPadding& Slider::GetProgressBarPadding() const
{
    return m_rcProgressBarPadding;
}

void Slider::SetProgressBarPadding(UiPadding padding, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        Dpi().ScalePadding(padding);
    }
    if (!m_rcProgressBarPadding.Equals(padding)) {
        m_rcProgressBarPadding = padding;
        RelayoutOrRedraw();
    }    
}

}
