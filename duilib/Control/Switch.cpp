#include "Switch.h"
#include "duilib/Core/GlobalManager.h"

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
    m_nThumbPadding(0)
{
}

Switch::~Switch()
{
}

DString Switch::GetType() const
{
    return DUI_CTR_SWITCH;
}

void Switch::SetAttribute(const DString& strName, const DString& strValue)
{
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

UiRect Switch::GetThumbRect() const
{
    UiRect rc = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rc.Deflate(rcPadding);

    // 滑块图片方形框边长等于轨道高度（阴影等留白绘制在图片内部）
    const int32_t nSize = rc.Height();
    const int32_t nCenterY = rc.CenterY();

    // 滑块方形框中心 x 随动画进度滑动
    const int32_t nMinX = rc.left + m_nThumbPadding + nSize / 2;
    const int32_t nMaxX = rc.right - m_nThumbPadding - nSize / 2;
    int32_t nCenterX = nMinX;
    if (nMaxX > nMinX) {
        nCenterX = nMinX + (nMaxX - nMinX) * m_nAnimValue / 100;
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

    const double t = m_nAnimValue / 100.0;

    // 轨道图片：off/on 两张按进度交叉淡入淡出
    if (HasTrackImages()) {
        DrawSwitchImage(pRender, m_pTrackOffImage.get(), rc,
                        static_cast<uint8_t>((1.0 - t) * 255.0));
        DrawSwitchImage(pRender, m_pTrackOnImage.get(), rc,
                        static_cast<uint8_t>(t * 255.0));
        return;
    }

    // 无图片时，使用语义色自绘轨道（圆角=高度一半），颜色按进度插值
    const float fRound = (float)rc.Height() / 2.0f;
    UiColor trackColor = LerpColor(GetUiColor(m_strTrackOffColor), GetUiColor(m_strTrackOnColor), t);
    pRender->FillRoundRect(UiRectF((float)rc.left, (float)rc.top, (float)rc.right, (float)rc.bottom), fRound, fRound, trackColor, GetAlpha());
}

void Switch::PaintStateImages(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }

    const double t = m_nAnimValue / 100.0;

    // 滑块图片：方形框沿轨道平移，off/on 两张按进度交叉淡入淡出
    if (HasThumbImages()) {
        UiRect thumbRect = GetThumbRect();
        if ((thumbRect.Width() > 0) && (thumbRect.Height() > 0)) {
            DrawSwitchImage(pRender, m_pThumbOffImage.get(), thumbRect,
                            static_cast<uint8_t>((1.0 - t) * 255.0));
            DrawSwitchImage(pRender, m_pThumbOnImage.get(), thumbRect,
                            static_cast<uint8_t>(t * 255.0));
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
    const int32_t nMinX = rc.left + nThumbMargin + nThumbSize / 2;
    const int32_t nMaxX = rc.right - nThumbMargin - nThumbSize / 2;
    int32_t nCenterX = nMinX;
    if (nMaxX > nMinX) {
        nCenterX = nMinX + (nMaxX - nMinX) * m_nAnimValue / 100;
    }
    UiRect thumbRect;
    thumbRect.left = nCenterX - nThumbSize / 2;
    thumbRect.right = nCenterX + nThumbSize / 2;
    thumbRect.top = rc.CenterY() - nThumbSize / 2;
    thumbRect.bottom = rc.CenterY() + nThumbSize / 2;

    const float fRound = (float)thumbRect.Height() / 2.0f;
    UiColor thumbColor = LerpColor(GetUiColor(m_strThumbOffColor), GetUiColor(m_strThumbOnColor), t);
    pRender->FillRoundRect(UiRectF((float)thumbRect.left, (float)thumbRect.top, (float)thumbRect.right, (float)thumbRect.bottom), fRound, fRound, thumbColor, GetAlpha());
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
