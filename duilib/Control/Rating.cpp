#include "Rating.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <cmath>

namespace ui
{

Rating::Rating(Window* pWindow) :
    BaseClass(pWindow),
    m_fValue(0),
    m_nMax(5),
    m_bAllowHalf(false),
    m_bReadOnly(false),
    m_nStarSize(20),
    m_nStarGap(4),
    m_starImages(),
    m_starStateImage(),
    m_bDrag(false),
    m_fHoverValue(-1),
    m_nHoverStar(-1),
    m_nPressedStar(-1),
    m_bMuteEvent(false)
{
    //星图需要关联控件接口，以便解析相对路径与缓存管理
    for (StateImage& stateImage : m_starImages) {
        stateImage.SetControl(this);
    }
    m_starStateImage.SetControl(this);
}

Rating::~Rating()
{
}

DString Rating::GetType() const
{
    return DUI_CTR_RATING;
}

void Rating::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("value")) {
        SetValueNoEvent(StringUtil::StringToDouble(strValue));
    }
    else if ((strName == _T("max")) || (strName == _T("star_count")) || (strName == _T("starcount"))) {
        SetMax(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("allow_half")) || (strName == _T("allowhalf"))) {
        SetAllowHalf(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("read_only")) || (strName == _T("readonly"))) {
        SetReadOnly(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("star_size")) || (strName == _T("starsize"))) {
        SetStarSize(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("star_gap")) || (strName == _T("stargap"))) {
        SetStarGap(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("star_filled_image")) || (strName == _T("star_full_image")) || (strName == _T("starfilledimage"))) {
        SetStarImage(StarImageType::kFilled, strValue);
    }
    else if ((strName == _T("star_half_image")) || (strName == _T("starhalfimage"))) {
        SetStarImage(StarImageType::kHalf, strValue);
    }
    else if ((strName == _T("star_empty_image")) || (strName == _T("staremptyimage"))) {
        SetStarImage(StarImageType::kEmpty, strValue);
    }
    else if ((strName == _T("star_hovered_image")) || (strName == _T("star_hover_image")) || (strName == _T("starhoveredimage"))) {
        SetStarStateImage(kControlStateHovered, strValue);
    }
    else if ((strName == _T("star_pressed_image")) || (strName == _T("star_pushed_image")) || (strName == _T("starpressedimage"))) {
        SetStarStateImage(kControlStatePressed, strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

double Rating::NormalizeValue(double fValue) const
{
    const double fMax = static_cast<double>(m_nMax);
    if (fValue < 0) {
        fValue = 0;
    }
    if (fValue > fMax) {
        fValue = fMax;
    }
    //按步长对齐（半星 0.5，否则 1）
    const double fStep = m_bAllowHalf ? 0.5 : 1.0;
    fValue = std::floor(fValue / fStep + 0.5) * fStep;
    if (fValue < 0) {
        fValue = 0;
    }
    if (fValue > fMax) {
        fValue = fMax;
    }
    return fValue;
}

void Rating::SetValue(double fValue)
{
    const double fNewValue = NormalizeValue(fValue);
    const double fOldValue = m_fValue;
    if (fNewValue == fOldValue) {
        return;
    }
    m_fValue = fNewValue;
    Invalidate();
    EmitValueChanged(fOldValue);
}

void Rating::SetValueNoEvent(double fValue)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    SetValue(fValue);
    m_bMuteEvent = bOldMute;
}

void Rating::SetMax(int32_t nMax)
{
    if (nMax < 0) {
        nMax = 0;
    }
    if (nMax > 100) {
        nMax = 100;
    }
    if (m_nMax == nMax) {
        return;
    }
    m_nMax = nMax;
    //调小 max 时同步钳位 value
    const double fOldValue = m_fValue;
    const double fNewValue = NormalizeValue(m_fValue);
    if (fNewValue != fOldValue) {
        m_fValue = fNewValue;
        Invalidate();
        EmitValueChanged(fOldValue);
    }
    else {
        Invalidate();
    }
}

void Rating::SetAllowHalf(bool bAllow)
{
    if (m_bAllowHalf == bAllow) {
        return;
    }
    m_bAllowHalf = bAllow;
    //步长变化后重新对齐当前值
    const double fOldValue = m_fValue;
    const double fNewValue = NormalizeValue(m_fValue);
    if (fNewValue != fOldValue) {
        m_fValue = fNewValue;
        Invalidate();
        EmitValueChanged(fOldValue);
    }
    else {
        Invalidate();
    }
}

void Rating::SetReadOnly(bool bReadOnly)
{
    if (m_bReadOnly != bReadOnly) {
        m_bReadOnly = bReadOnly;
        if (m_bReadOnly) {
            m_fHoverValue = -1;
            m_nHoverStar = -1;
            m_nPressedStar = -1;
        }
        Invalidate();
    }
}

void Rating::SetStarSize(int32_t nSize)
{
    if (nSize < 8) {
        nSize = 8;
    }
    if (nSize > 128) {
        nSize = 128;
    }
    if (m_nStarSize != nSize) {
        m_nStarSize = nSize;
        Invalidate();
    }
}

void Rating::SetStarGap(int32_t nGap)
{
    if (nGap < 0) {
        nGap = 0;
    }
    if (nGap > 64) {
        nGap = 64;
    }
    if (m_nStarGap != nGap) {
        m_nStarGap = nGap;
        Invalidate();
    }
}

void Rating::SetStarImage(StarImageType type, const DString& strImage)
{
    const int32_t nIndex = static_cast<int>(type);
    if ((nIndex < 0) || (nIndex >= static_cast<int>(m_starImages.size()))) {
        return;
    }
    m_starImages[nIndex].SetImageString(kControlStateNormal, strImage, Dpi());
    Invalidate();
}

DString Rating::GetStarImage(StarImageType type) const
{
    const int32_t nIndex = static_cast<int>(type);
    if ((nIndex < 0) || (nIndex >= static_cast<int>(m_starImages.size()))) {
        return DString();
    }
    return m_starImages[nIndex].GetImageString(kControlStateNormal);
}

void Rating::SetStarStateImage(ControlStateType stateType, const DString& strImage)
{
    m_starStateImage.SetImageString(stateType, strImage, Dpi());
    Invalidate();
}

DString Rating::GetStarStateImage(ControlStateType stateType) const
{
    return m_starStateImage.GetImageString(stateType);
}

Image* Rating::GetStarImagePtr(StarImageType type) const
{
    const int32_t nIndex = static_cast<int>(type);
    if ((nIndex < 0) || (nIndex >= static_cast<int>(m_starImages.size()))) {
        return nullptr;
    }
    return m_starImages[nIndex].GetStateImage(kControlStateNormal);
}

Image* Rating::GetStarStateImagePtr(ControlStateType stateType) const
{
    return m_starStateImage.GetStateImage(stateType);
}

void Rating::EmitValueChanged(double fOldValue)
{
    if (m_bMuteEvent) {
        return;
    }
    //评分值通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    RatingValue value;
    value.value = m_fValue;
    value.oldValue = fOldValue;
    SendEvent(kEventValueChanged, 0, 0, static_cast<void*>(&value));
}

void Rating::PaintStarImage(IRender* pRender, Image* pImage, const DString& strModify, int32_t nFade) const
{
    if (pImage == nullptr) {
        return;
    }
    if (nFade == DUI_NOSET_VALUE) {
        PaintImage(pRender, pImage, strModify);
    }
    else {
        PaintImage(pRender, pImage, strModify, nFade);
    }
}

void Rating::PaintStarStateIcon(IRender* pRender, int32_t nIndex, int32_t nLeft, int32_t nTop, int32_t nSize)
{
    //仅对可交互的星星绘制悬停/按下态图标
    if ((pRender == nullptr) || !IsEnabled() || !IsMouseEnabled() || m_bReadOnly) {
        return;
    }
    if (!m_starStateImage.HasImage()) {
        return;
    }

    ControlStateType stateType = kControlStateNormal;
    if (m_nPressedStar == nIndex) {
        stateType = kControlStatePressed;
    }
    else if (m_nHoverStar == nIndex) {
        stateType = kControlStateHovered;
    }
    else {
        return;
    }

    Image* pImage = m_starStateImage.GetStateImage(stateType);
    if ((pImage == nullptr) && (stateType == kControlStatePressed)) {
        //按下态未配置图片时，回退使用悬停态图片
        pImage = m_starStateImage.GetStateImage(kControlStateHovered);
    }
    if (pImage == nullptr) {
        return;
    }

    //图标以星为中心，比星形略大（形成外圈光晕/描边效果）；目标区域为控件相对坐标
    const int32_t nIconSize = nSize + Dpi().GetScaleInt(8);
    const UiRect rcCtrl = GetRect();
    const int32_t nIconLeft = nLeft - (nIconSize - nSize) / 2 - rcCtrl.left;
    const int32_t nIconTop = nTop - (nIconSize - nSize) / 2 - rcCtrl.top;
    const DString strModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"),
                                                 nIconLeft, nIconTop, nIconLeft + nIconSize, nIconTop + nIconSize);
    PaintImage(pRender, pImage, strModify);
}

void Rating::PaintStateImages(IRender* pRender)
{
    BaseClass::PaintStateImages(pRender);
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if ((rc.Width() <= 0) || (rc.Height() <= 0) || (m_nMax <= 0)) {
        return;
    }

    const int32_t nSize = Dpi().GetScaleInt(m_nStarSize);
    const int32_t nGap = Dpi().GetScaleInt(m_nStarGap);
    if (nSize <= 0) {
        return;
    }
    const int32_t nStep = nSize + nGap;
    const int32_t nTop = rc.top + (rc.Height() - nSize) / 2;
    const UiRect rcCtrl = GetRect();

    //显示的评分值：非只读且处于悬停时，优先显示预览值
    double fDisplayValue = m_fValue;
    if (IsEnabled() && !m_bReadOnly && (m_fHoverValue >= 0)) {
        fDisplayValue = m_fHoverValue;
    }

    //禁用态整体降低透明度
    const int32_t nFade = IsEnabled() ? DUI_NOSET_VALUE : static_cast<int32_t>(GetAlpha() * 3 / 5);

    for (int32_t i = 0; i < m_nMax; ++i) {
        const int32_t nLeft = rc.left + i * nStep;
        const int32_t nStarRight = nLeft + nSize;

        //先绘制悬停/按下态图标（在星形下层）
        PaintStarStateIcon(pRender, i, nLeft, nTop, nSize);

        double fRatio = fDisplayValue - static_cast<double>(i);
        if (fRatio < 0) {
            fRatio = 0;
        }
        if (fRatio > 1) {
            fRatio = 1;
        }

        //目标矩形为控件相对坐标
        const DString strModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"),
                                                     nLeft - rcCtrl.left, nTop - rcCtrl.top,
                                                     nStarRight - rcCtrl.left, nTop + nSize - rcCtrl.top);

        if (fRatio >= 1) {
            //实心星
            PaintStarImage(pRender, GetStarImagePtr(StarImageType::kFilled), strModify, nFade);
        }
        else if (fRatio <= 0) {
            //空心星
            PaintStarImage(pRender, GetStarImagePtr(StarImageType::kEmpty), strModify, nFade);
        }
        else {
            //半星：配置了半星图片直接用；否则对实心星做左半裁剪绘制（图片裁剪，非自绘）
            Image* pHalfImage = GetStarImagePtr(StarImageType::kHalf);
            if (pHalfImage != nullptr) {
                PaintStarImage(pRender, pHalfImage, strModify, nFade);
            }
            else {
                Image* pFillImage = GetStarImagePtr(StarImageType::kFilled);
                if (pFillImage != nullptr) {
                    //裁剪区域使用窗口坐标：只显示星星左半边
                    const UiRect rcClip(nLeft, nTop, nLeft + nSize / 2, nTop + nSize);
                    AutoClip clip(pRender, rcClip);
                    PaintStarImage(pRender, pFillImage, strModify, nFade);
                }
            }
        }
    }
}

int32_t Rating::HitTestStar(const UiRect& rc, const UiPoint& ptMouse, bool* pHalf) const
{
    if (m_nMax <= 0) {
        return -1;
    }
    const int32_t nSize = Dpi().GetScaleInt(m_nStarSize);
    const int32_t nGap = Dpi().GetScaleInt(m_nStarGap);
    const int32_t nStep = nSize + nGap;
    if ((nSize <= 0) || (nStep <= 0)) {
        return -1;
    }
    const int32_t nRelX = ptMouse.x - rc.left;
    if (nRelX < 0) {
        return -1;
    }
    int32_t nIndex = nRelX / nStep;
    if (nIndex >= m_nMax) {
        return -1;
    }
    if (pHalf != nullptr) {
        const int32_t nOffsetInStar = nRelX - nIndex * nStep;
        *pHalf = (nOffsetInStar >= nSize / 2);
    }
    return nIndex;
}

double Rating::ValueFromHit(int32_t nIndex, bool bHalf) const
{
    double fValue = static_cast<double>(nIndex + 1);
    //半星模式下，落在左半边计 index + 0.5
    if (m_bAllowHalf && !bHalf) {
        fValue = static_cast<double>(nIndex) + 0.5;
    }
    return fValue;
}

bool Rating::ButtonDown(const EventArgs& msg)
{
    if (!IsEnabled() || m_bReadOnly) {
        return BaseClass::ButtonDown(msg);
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    bool bHalf = false;
    const int32_t nHit = HitTestStar(rc, msg.ptMouse, &bHalf);
    if (nHit >= 0) {
        //进入拖动打分，记录按下态，并获取鼠标焦点使按住期间持续收到鼠标消息
        m_bDrag = true;
        m_nPressedStar = nHit;
        SetMouseFocused(true);
        SetValue(ValueFromHit(nHit, bHalf));
        Invalidate();
        return true;
    }
    return BaseClass::ButtonDown(msg);
}

bool Rating::ButtonUp(const EventArgs& msg)
{
    if (m_bDrag) {
        m_bDrag = false;
        if (IsMouseFocused()) {
            SetMouseFocused(false);
        }
    }
    if (m_nPressedStar >= 0) {
        m_nPressedStar = -1;
        Invalidate();
    }
    return BaseClass::ButtonUp(msg);
}

bool Rating::MouseMove(const EventArgs& msg)
{
    if (!IsEnabled() || m_bReadOnly) {
        return BaseClass::MouseMove(msg);
    }
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    bool bHalf = false;
    const int32_t nHit = HitTestStar(rc, msg.ptMouse, &bHalf);
    const double fHitValue = (nHit >= 0) ? ValueFromHit(nHit, bHalf) : -1;

    bool bRedraw = false;
    if (m_nHoverStar != nHit) {
        m_nHoverStar = nHit;
        bRedraw = true;
    }
    if (m_fHoverValue != fHitValue) {
        m_fHoverValue = fHitValue;
        bRedraw = true;
    }
    if (m_bDrag && IsMouseFocused() && (nHit >= 0)) {
        //拖动中连续打分
        SetValue(fHitValue);
    }
    if (bRedraw) {
        Invalidate();
    }
    return BaseClass::MouseMove(msg);
}

bool Rating::MouseLeave(const EventArgs& msg)
{
    if ((m_fHoverValue >= 0) || (m_nHoverStar >= 0) || (m_nPressedStar >= 0)) {
        m_fHoverValue = -1;
        m_nHoverStar = -1;
        m_nPressedStar = -1;
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

void Rating::ClearImageCache()
{
    BaseClass::ClearImageCache();
    for (StateImage& stateImage : m_starImages) {
        stateImage.ClearImageCache();
    }
    m_starStateImage.ClearImageCache();
}

} // namespace ui
