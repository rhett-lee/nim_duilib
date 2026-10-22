#include "Switch.h"
#include "duilib/Core/GlobalManager.h"
#include <algorithm>
#include <cmath>

namespace ui
{

Switch::Switch(Window* pWindow) :
    BaseClass(pWindow),
    m_nAnimationMs(160),
    m_nAnimValue(0),
    m_strTrackOffColor(_T("bg_switch_track_off")),
    m_strTrackOnColor(_T("bg_switch_track_on")),
    m_strThumbOffColor(_T("bg_switch_thumb_off")),
    m_strThumbOnColor(_T("bg_switch_thumb_on")),
    m_strTextOnColor(_T("bg_switch_thumb_on")),
    m_strTextOffColor(_T("text_default")),
    m_strTrackDisabledColor(_T("bg_switch_track_off")),
    m_strThumbDisabledColor(_T("bg_switch_thumb_on")),
    m_strLoadingColor(_T("color_accent")),
    m_nThumbPadding(0),
    m_bLoading(false),
    m_nLoadingAngle(0)
{
}

Switch::~Switch()
{
    m_loadingTimer.Cancel();
}

DString Switch::GetType() const
{
    return DUI_CTR_SWITCH;
}

void Switch::SetAttribute(const DString& strName, const DString& strValue2)
{
    //图片路径等属性中可能包含 ${VAR} 变量，先展开再解析
    DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("switch_animation_ms")) || (strName == _T("animation_ms"))) {
        SetAnimationDuration(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("track_off_color")) || (strName == _T("trackoffcolor"))) {
        SetTrackOffColor(strValue);
    }
    else if ((strName == _T("track_on_color")) || (strName == _T("trackoncolor"))) {
        SetTrackOnColor(strValue);
    }
    else if ((strName == _T("thumb_off_color")) || (strName == _T("thumboffcolor"))) {
        SetThumbOffColor(strValue);
    }
    else if ((strName == _T("thumb_on_color")) || (strName == _T("thumboncolor"))) {
        SetThumbOnColor(strValue);
    }
    else if ((strName == _T("track_off_image")) || (strName == _T("trackoffimage"))) {
        SetTrackOffImage(strValue);
    }
    else if ((strName == _T("track_on_image")) || (strName == _T("trackonimage"))) {
        SetTrackOnImage(strValue);
    }
    else if ((strName == _T("thumb_off_image")) || (strName == _T("thumboffimage"))) {
        SetThumbOffImage(strValue);
    }
    else if ((strName == _T("thumb_on_image")) || (strName == _T("thumbonimage"))) {
        SetThumbOnImage(strValue);
    }
    else if (strName == _T("thumb_padding")) {
        SetThumbPadding(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("switch_text_on")) || (strName == _T("switchtexton"))) {
        SetTextOn(strValue);
    }
    else if ((strName == _T("switch_text_off")) || (strName == _T("switchtextoff"))) {
        SetTextOff(strValue);
    }
    else if ((strName == _T("switch_text_on_color")) || (strName == _T("switchtextoncolor"))) {
        SetTextOnColor(strValue);
    }
    else if ((strName == _T("switch_text_off_color")) || (strName == _T("switchtextoffcolor"))) {
        SetTextOffColor(strValue);
    }
    else if ((strName == _T("switch_image_on")) || (strName == _T("switchimageon"))) {
        SetImageOn(strValue);
    }
    else if ((strName == _T("switch_image_off")) || (strName == _T("switchimageoff"))) {
        SetImageOff(strValue);
    }
    else if ((strName == _T("switch_loading")) || (strName == _T("switchloading"))) {
        SetLoading((strValue == _T("true")) || (strValue == _T("1")));
    }
    else if ((strName == _T("switch_loading_color")) || (strName == _T("switchloadingcolor"))) {
        SetLoadingColor(strValue);
    }
    else if ((strName == _T("track_disabled_color")) || (strName == _T("trackdisabledcolor"))) {
        SetTrackDisabledColor(strValue);
    }
    else if ((strName == _T("thumb_disabled_color")) || (strName == _T("thumbdisabledcolor"))) {
        SetThumbDisabledColor(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Switch::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    m_nThumbPadding = Dpi().GetScaleInt(m_nThumbPadding, nOldDpiScale);

    //图片属性中的尺寸需要按新DPI重新解析
    UpdateSwitchImage(m_pTrackOffImage, m_strTrackOffImage);
    UpdateSwitchImage(m_pTrackOnImage, m_strTrackOnImage);
    UpdateSwitchImage(m_pThumbOffImage, m_strThumbOffImage);
    UpdateSwitchImage(m_pThumbOnImage, m_strThumbOnImage);
    UpdateSwitchImage(m_pImageOn, m_strImageOn);
    UpdateSwitchImage(m_pImageOff, m_strImageOff);
}

void Switch::SetAnimationDuration(int32_t nMs)
{
    m_nAnimationMs = nMs;
    if (m_nAnimationMs < 0) {
        m_nAnimationMs = 0;
    }
}

void Switch::SetTrackOffColor(const DString& strColor)
{
    m_strTrackOffColor = strColor;
    Invalidate();
}

void Switch::SetTrackOnColor(const DString& strColor)
{
    m_strTrackOnColor = strColor;
    Invalidate();
}

void Switch::SetThumbOffColor(const DString& strColor)
{
    m_strThumbOffColor = strColor;
    Invalidate();
}

void Switch::SetThumbOnColor(const DString& strColor)
{
    m_strThumbOnColor = strColor;
    Invalidate();
}

void Switch::UpdateSwitchImage(std::unique_ptr<Image>& pImage, const DString& strImage)
{
    if (strImage.empty()) {
        pImage.reset();
        return;
    }
    if (pImage == nullptr) {
        pImage = std::make_unique<Image>();
        pImage->SetControl(this);
    }
    pImage->SetImageString(strImage, Dpi());
}

void Switch::SetTrackOffImage(const DString& strImage)
{
    m_strTrackOffImage = strImage;
    UpdateSwitchImage(m_pTrackOffImage, strImage);
    Invalidate();
}

void Switch::SetTrackOnImage(const DString& strImage)
{
    m_strTrackOnImage = strImage;
    UpdateSwitchImage(m_pTrackOnImage, strImage);
    Invalidate();
}

void Switch::SetThumbOffImage(const DString& strImage)
{
    m_strThumbOffImage = strImage;
    UpdateSwitchImage(m_pThumbOffImage, strImage);
    Invalidate();
}

void Switch::SetThumbOnImage(const DString& strImage)
{
    m_strThumbOnImage = strImage;
    UpdateSwitchImage(m_pThumbOnImage, strImage);
    Invalidate();
}

void Switch::SetThumbPadding(int32_t nPadding, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        nPadding = Dpi().GetScaleInt(nPadding);
    }
    if (nPadding < 0) {
        nPadding = 0;
    }
    m_nThumbPadding = nPadding;
    Invalidate();
}

void Switch::SetTextOn(const DString& strText)
{
    m_strTextOn = strText;
    Invalidate();
}

void Switch::SetTextOff(const DString& strText)
{
    m_strTextOff = strText;
    Invalidate();
}

void Switch::SetTextOnColor(const DString& strColor)
{
    m_strTextOnColor = strColor;
    Invalidate();
}

void Switch::SetTextOffColor(const DString& strColor)
{
    m_strTextOffColor = strColor;
    Invalidate();
}

void Switch::SetImageOn(const DString& strImage)
{
    m_strImageOn = strImage;
    UpdateSwitchImage(m_pImageOn, strImage);
    Invalidate();
}

void Switch::SetImageOff(const DString& strImage)
{
    m_strImageOff = strImage;
    UpdateSwitchImage(m_pImageOff, strImage);
    Invalidate();
}

void Switch::SetLoading(bool bLoading)
{
    if (m_bLoading == bLoading) {
        return;
    }
    m_bLoading = bLoading;
    m_nLoadingAngle = 0;

    if (m_bLoading) {
        auto playCallback = UiBind(&Switch::OnLoadingTimer, this);
        GlobalManager::Instance().Timer().AddTimer(m_loadingTimer.GetWeakFlag(), playCallback, 60);
    }
    else {
        m_loadingTimer.Cancel();
    }
    Invalidate();
}

void Switch::SetLoadingColor(const DString& strColor)
{
    m_strLoadingColor = strColor;
    Invalidate();
}

void Switch::SetTrackDisabledColor(const DString& strColor)
{
    m_strTrackDisabledColor = strColor;
    Invalidate();
}

void Switch::SetThumbDisabledColor(const DString& strColor)
{
    m_strThumbDisabledColor = strColor;
    Invalidate();
}

void Switch::OnLoadingTimer()
{
    if (!m_bLoading) {
        m_loadingTimer.Cancel();
        return;
    }
    m_nLoadingAngle = (m_nLoadingAngle + 30) % 360;
    Invalidate();
}

bool Switch::HasTrackImages() const
{
    return (m_pTrackOffImage != nullptr) && !m_pTrackOffImage->GetImagePath().empty() &&
           (m_pTrackOnImage != nullptr) && !m_pTrackOnImage->GetImagePath().empty();
}

bool Switch::HasThumbImages() const
{
    return (m_pThumbOffImage != nullptr) && !m_pThumbOffImage->GetImagePath().empty() &&
           (m_pThumbOnImage != nullptr) && !m_pThumbOnImage->GetImagePath().empty();
}

void Switch::DrawSwitchImage(IRender* pRender, Image* pImage, const UiRect& rcDest, uint8_t nAlpha) const
{
    if ((pRender == nullptr) || (pImage == nullptr) || (nAlpha == 0)) {
        return;
    }
    PaintImage(pRender, pImage, _T(""), static_cast<int32_t>(nAlpha), nullptr, &rcDest, nullptr);
}

UiColor Switch::LerpColor(UiColor colorFrom, UiColor colorTo, double t)
{
    auto lerp = [t](uint8_t from, uint8_t to) -> uint8_t {
        return (uint8_t)(from + (to - from) * t);
    };
    return UiColor(UiColor::MakeARGB(lerp(colorFrom.GetA(), colorTo.GetA()),
                                     lerp(colorFrom.GetR(), colorTo.GetR()),
                                     lerp(colorFrom.GetG(), colorTo.GetG()),
                                     lerp(colorFrom.GetB(), colorTo.GetB())));
}

int32_t Switch::GetEffectiveAnimValue() const
{
    if (!IsEnabled()) {
        //禁用时不做动画过渡，直接使用目标值
        return IsSelected() ? 100 : 0;
    }
    return m_nAnimValue;
}

UiRect Switch::GetThumbRect() const
{
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);

    // 滑块图片方形框边长等于轨道高度（阴影等留白绘制在图片内部）
    const int32_t nSize = rc.Height();
    const int32_t nCenterY = rc.CenterY();

    // 滑块方形框中心 x 随动画进度滑动
    const int32_t nAnimValue = GetEffectiveAnimValue();
    const int32_t nMinX = rc.left + m_nThumbPadding + nSize / 2;
    const int32_t nMaxX = rc.right - m_nThumbPadding - nSize / 2;
    int32_t nCenterX = nMinX;
    if (nMaxX > nMinX) {
        nCenterX = nMinX + (nMaxX - nMinX) * nAnimValue / 100;
    }

    UiRect thumbRect;
    thumbRect.left = nCenterX - nSize / 2;
    thumbRect.right = nCenterX + nSize / 2;
    thumbRect.top = nCenterY - nSize / 2;
    thumbRect.bottom = nCenterY + nSize / 2;
    return thumbRect;
}

void Switch::PaintStateColors(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }

    const bool bEnabled = IsEnabled();
    const double t = GetEffectiveAnimValue() / 100.0;

    // 轨道图片：off/on 两张按进度交叉淡入淡出
    if (HasTrackImages()) {
        if (bEnabled) {
            DrawSwitchImage(pRender, m_pTrackOffImage.get(), rc,
                            static_cast<uint8_t>((1.0 - t) * 255.0));
            DrawSwitchImage(pRender, m_pTrackOnImage.get(), rc,
                            static_cast<uint8_t>(t * 255.0));
        }
        else {
            //禁用状态固定使用未选中轨道图片
            DrawSwitchImage(pRender, m_pTrackOffImage.get(), rc, 255);
        }
        return;
    }

    // 无图片时，使用语义色自绘轨道（圆角=高度一半），颜色按进度插值
    const float fRound = (float)rc.Height() / 2.0f;
    UiColor trackColor;
    if (bEnabled) {
        trackColor = LerpColor(GetUiColor(m_strTrackOffColor), GetUiColor(m_strTrackOnColor), t);
    }
    else {
        trackColor = GetUiColor(m_strTrackDisabledColor.empty() ? m_strTrackOffColor : m_strTrackDisabledColor);
    }
    pRender->FillRoundRect(UiRectF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom), fRound, fRound, trackColor, GetAlpha());
}

void Switch::PaintStateImages(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }

    const bool bEnabled = IsEnabled();
    const double t = GetEffectiveAnimValue() / 100.0;

    // 滑块图片：方形框沿轨道平移，off/on 两张按进度交叉淡入淡出
    if (HasThumbImages()) {
        UiRect thumbRect = GetThumbRect();
        if ((thumbRect.Width() > 0) && (thumbRect.Height() > 0)) {
            if (bEnabled) {
                DrawSwitchImage(pRender, m_pThumbOffImage.get(), thumbRect,
                                static_cast<uint8_t>((1.0 - t) * 255.0));
                DrawSwitchImage(pRender, m_pThumbOnImage.get(), thumbRect,
                                static_cast<uint8_t>(t * 255.0));
            }
            else {
                //禁用状态固定使用当前选中状态对应的滑块图片
                DrawSwitchImage(pRender, IsSelected() ? m_pThumbOnImage.get() : m_pThumbOffImage.get(), thumbRect, 255);
            }
            if (m_bLoading) {
                PaintLoading(pRender, thumbRect);
            }
        }
        return;
    }

    // 无图片时，使用语义色自绘圆形滑块（直径=轨道高度-2*边距），颜色按进度插值
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }
    const int32_t nThumbMargin = Dpi().GetScaleInt(2);
    const int32_t nThumbSize = rc.Height() - nThumbMargin * 2;
    if (nThumbSize <= 0) {
        return;
    }
    const int32_t nAnimValue = GetEffectiveAnimValue();
    const int32_t nMinX = rc.left + nThumbMargin + nThumbSize / 2;
    const int32_t nMaxX = rc.right - nThumbMargin - nThumbSize / 2;
    int32_t nCenterX = nMinX;
    if (nMaxX > nMinX) {
        nCenterX = nMinX + (nMaxX - nMinX) * nAnimValue / 100;
    }
    UiRect thumbRect;
    thumbRect.left = nCenterX - nThumbSize / 2;
    thumbRect.right = nCenterX + nThumbSize / 2;
    thumbRect.top = rc.CenterY() - nThumbSize / 2;
    thumbRect.bottom = rc.CenterY() + nThumbSize / 2;

    const float fRound = (float)thumbRect.Height() / 2.0f;
    UiColor thumbColor;
    if (bEnabled) {
        thumbColor = LerpColor(GetUiColor(m_strThumbOffColor), GetUiColor(m_strThumbOnColor), t);
    }
    else {
        thumbColor = GetUiColor(m_strThumbDisabledColor.empty() ? m_strThumbOffColor : m_strThumbDisabledColor);
    }
    pRender->FillRoundRect(UiRectF((float)thumbRect.left, (float)thumbRect.top, (float)thumbRect.right, (float)thumbRect.bottom), fRound, fRound, thumbColor, GetAlpha());

    if (m_bLoading) {
        PaintLoading(pRender, thumbRect);
    }
}

void Switch::PaintText(IRender* pRender)
{
    BaseClass::PaintText(pRender);
    if (pRender == nullptr) {
        return;
    }
    if (m_strTextOn.empty() && m_strTextOff.empty() && (m_pImageOn == nullptr) && (m_pImageOff == nullptr)) {
        return;
    }

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }

    const UiRect thumbRect = GetThumbRect();
    const int32_t nTextPadding = std::max<int32_t>(2, rc.Height() / 8);
    const int32_t nGap = std::max<int32_t>(2, rc.Height() / 10);

    //选中文字在滑块左侧，未选中文字在滑块右侧
    const UiRect rcOn(rc.left + nTextPadding, rc.top, thumbRect.left - nGap, rc.bottom);
    const UiRect rcOff(thumbRect.right + nGap, rc.top, rc.right - nTextPadding, rc.bottom);
    if ((rcOn.Width() <= 0) && (rcOff.Width() <= 0)) {
        return;
    }

    const bool bEnabled = IsEnabled();
    const double t = GetEffectiveAnimValue() / 100.0;
    const uint8_t nOnAlpha = static_cast<uint8_t>(t * 255.0);
    const uint8_t nOffAlpha = static_cast<uint8_t>((1.0 - t) * 255.0);
    const UiColor onColor = GetUiColor(bEnabled ? m_strTextOnColor : _T("text_disabled"));
    const UiColor offColor = GetUiColor(bEnabled ? m_strTextOffColor : _T("text_disabled"));

    PaintSwitchContent(pRender, rcOn, m_strTextOn, m_pImageOn.get(), onColor, nOnAlpha);
    PaintSwitchContent(pRender, rcOff, m_strTextOff, m_pImageOff.get(), offColor, nOffAlpha);
}

void Switch::PaintSwitchContent(IRender* pRender, const UiRect& rcContent, const DString& strText,
                                Image* pImage, UiColor textColor, uint8_t nAlpha) const
{
    if ((pRender == nullptr) || (nAlpha == 0) || rcContent.IsEmpty()) {
        return;
    }
    if (strText.empty() && (pImage == nullptr)) {
        return;
    }

    DrawStringParam drawParam = GetDrawParam();
    drawParam.dwTextColor = textColor;
    drawParam.uFade = nAlpha;
    drawParam.uFormat = TEXT_LEFT | TEXT_VCENTER | TEXT_SINGLELINE;

    int32_t nTextWidth = 0;
    if (!strText.empty()) {
        MeasureStringParam measureParam;
        measureParam.pFont = drawParam.pFont;
        measureParam.uFormat = TEXT_SINGLELINE;
        nTextWidth = pRender->MeasureString(strText, measureParam).Width();
    }

    const bool bHasImage = (pImage != nullptr);
    const int32_t nIconSize = std::max<int32_t>(8, rcContent.Height() / 2);
    const int32_t nIconGap = 2;
    const int32_t nGap = (bHasImage && !strText.empty()) ? nIconGap : 0;
    const int32_t nTotalWidth = (bHasImage ? nIconSize : 0) + nGap + nTextWidth;

    int32_t nStartX = rcContent.CenterX() - nTotalWidth / 2;
    if (bHasImage) {
        const int32_t nIconTop = rcContent.CenterY() - nIconSize / 2;
        const uint8_t nImageAlpha = IsEnabled() ? nAlpha : std::min<uint8_t>(nAlpha, 128);
        DrawSwitchImage(pRender, pImage, UiRect(nStartX, nIconTop, nStartX + nIconSize, nIconTop + nIconSize), nImageAlpha);
        nStartX += nIconSize + nGap;
    }
    if (!strText.empty()) {
        //预留2像素余量，避免测量误差导致文字被裁剪
        drawParam.textRect = UiRect(nStartX, rcContent.top, nStartX + nTextWidth + 2, rcContent.bottom);
        pRender->DrawString(strText, drawParam);
    }
}

void Switch::PaintLoading(IRender* pRender, const UiRect& rcThumb) const
{
    if ((pRender == nullptr) || rcThumb.IsEmpty()) {
        return;
    }
    const UiColor color = GetUiColor(IsEnabled() ? m_strLoadingColor : _T("text_disabled"));

    const float fCenterX = static_cast<float>(rcThumb.CenterX());
    const float fCenterY = static_cast<float>(rcThumb.CenterY());
    const float fRadius = std::max(2.0f, static_cast<float>(rcThumb.Width()) / 4.0f);
    const float fDotRadius = std::max(1.0f, static_cast<float>(rcThumb.Width()) / 10.0f);

    //8个圆点按角度分布，透明度从头部向后递减，形成旋转指示器
    constexpr double fPi = 3.14159265358979323846;
    for (int32_t nIndex = 0; nIndex < 8; ++nIndex) {
        const double fAngle = (m_nLoadingAngle + nIndex * 45) * fPi / 180.0;
        const float fX = fCenterX + static_cast<float>(std::cos(fAngle)) * fRadius;
        const float fY = fCenterY + static_cast<float>(std::sin(fAngle)) * fRadius;
        const uint8_t nAlpha = static_cast<uint8_t>(std::max<int32_t>(79, 255 - nIndex * 22));
        pRender->FillRoundRect(UiRectF(fX - fDotRadius, fY - fDotRadius, fX + fDotRadius, fY + fDotRadius),
                               fDotRadius, fDotRadius, color, nAlpha);
    }
}

void Switch::Activate(const EventArgs* pMsg)
{
    if (m_bLoading) {
        //加载中禁止用户切换
        return;
    }
    BaseClass::Activate(pMsg);
}

void Switch::OnPrivateSetSelected()
{
    // 更新动画目标值
    const int32_t nTargetValue = IsSelected() ? 100 : 0;

    // 如果未初始化（如 XML 加载阶段）或动画时长为 0，直接跳到目标值
    if (!IsInited() || (m_nAnimationMs <= 0)) {
        m_nAnimValue = nTargetValue;
        return;
    }

    // 如果当前已在目标值，无需动画
    if (m_nAnimValue == nTargetValue) {
        return;
    }

    // 先停止旧动画
    if ((m_pAnimationPlayer != nullptr) && m_pAnimationPlayer->IsPlaying()) {
        m_pAnimationPlayer->SetCompleteCallback(nullptr);
        m_pAnimationPlayer->Stop();
    }

    m_pAnimationPlayer.reset(new AnimationPlayer);
    m_pAnimationPlayer->SetAnimationType(AnimationType::kAnimationNone);
    m_pAnimationPlayer->SetTotalMillSeconds(m_nAnimationMs);
    m_pAnimationPlayer->SetFrameIntervalMillSeconds(16);
    m_pAnimationPlayer->SetEasingFunctionType(EaseInOutQuad);
    m_pAnimationPlayer->SetStartValue(m_nAnimValue);
    m_pAnimationPlayer->SetEndValue(nTargetValue);

    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    m_pAnimationPlayer->SetPlayCallback([this, weakFlag](int32_t nValue) {
        if (weakFlag.expired()) {
            return;
        }
        m_nAnimValue = nValue;
        Invalidate();
    });
    m_pAnimationPlayer->SetCompleteCallback([this, weakFlag]() {
        if (weakFlag.expired()) {
            return;
        }
        // 动画结束，确保落在最终值
        m_nAnimValue = IsSelected() ? 100 : 0;
        Invalidate();
    });
    m_pAnimationPlayer->Start();
}

} // namespace ui
