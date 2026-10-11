#include "Progress.h"
#include "duilib/Core/GlobalManager.h"
#include <cmath>

namespace ui
{

Progress::Progress(Window* pWindow) :
    LabelTemplate<Control>(pWindow),
    m_bHorizontal(true),
    m_bStretchForeImage(true),
    m_nMaxValue(100),
    m_nMinValue(0),
    m_fCurrentValue(0),
    m_sProgressColor(),
    m_pProgressImage(nullptr),
    m_sProgressImageModify(),
    m_bMarquee(false),
    m_nMarqueeWidth(0),
    m_nMarqueeStep(0),
    m_nMarqueeElapsed(50), // for 1s 25fps,will use 20fps default
    m_nMarqueePos(0),
    m_fDisplayValue(0),
    m_bAnimationEnabled(false),
    m_nAnimationDuration(300),
    m_pAnimPlayer(nullptr),
    m_bShowPercent(false),
    m_nProgressRadius(0),
    m_bReverse(false)
{
    SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER | TEXT_VCENTER, false);
    SetFixedHeight(UiFixedInt(12), true, true);
    SetMarqueeWidth(10, true);
    SetMarqueeStep(4, true);
}

Progress::~Progress()
{
    if (m_pProgressImage != nullptr) {
        delete m_pProgressImage;
        m_pProgressImage = nullptr;
    }
}

DString Progress::GetType() const { return DUI_CTR_PROGRESS; }

void Progress::SetAttribute(const DString& srName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if ((srName == _T("horizontal")) || (srName == _T("hor"))) {
        SetHorizontal(StringUtil::IsValueTrue(strValue));
    }
    else if (srName == _T("min")) {
        SetMinValue(StringUtil::StringToInt32(strValue));
    }
    else if (srName == _T("max")) {
        SetMaxValue(StringUtil::StringToInt32(strValue));
    }
    else if (srName == _T("value")) {
        SetValue(StringUtil::StringToDouble(strValue));
    }
    else if ((srName == _T("progress_image")) || (srName == _T("progressimage"))) {
        SetProgressImage(strValue);
    }
    else if ((srName == _T("stretch_fore_image")) || (srName == _T("is_stretch_fore")) || (srName == _T("isstretchfore"))) {
        SetStretchForeImage(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("progress_color")) || (srName == _T("progresscolor"))) {
        SetProgressColor(strValue);
    }
    else if ((srName == _T("progress_radius")) || (srName == _T("progressradius"))) {
        SetProgressRadius(StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("show_percent")) || (srName == _T("showpercent"))) {
        SetShowPercent(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("animation_enabled")) || (srName == _T("animationenabled"))) {
        SetAnimationEnabled(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("animation_duration")) || (srName == _T("animationduration"))) {
        SetAnimationDuration(StringUtil::StringToInt32(strValue));
    }
    else if (srName == _T("marquee")) {
        SetMarquee(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("marquee_width")) || (srName == _T("marqueewidth"))) {
        SetMarqueeWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if ((srName == _T("marquee_step")) || (srName == _T("marqueestep"))) {
        SetMarqueeStep(StringUtil::StringToInt32(strValue), true);
    }
    else if (srName == _T("reverse")) {
        SetReverse(StringUtil::IsValueTrue(strValue));
    }
    else {
        Label::SetAttribute(srName, strValue);
    }
}

bool Progress::IsHorizontal() const
{
    return m_bHorizontal;
}

void Progress::SetHorizontal(bool bHorizontal)
{
    if (m_bHorizontal != bHorizontal) {
        m_bHorizontal = bHorizontal;
        Invalidate();
    }    
}

int32_t Progress::GetMinValue() const
{
    return m_nMinValue;
}

void Progress::SetMinValue(int32_t nMin)
{
    if (m_nMinValue != nMin) {
        m_nMinValue = nMin;
        Invalidate();
    }
}

int32_t Progress::GetMaxValue() const
{
    return m_nMaxValue;
}

void Progress::SetMaxValue(int32_t nMax)
{
    if (m_nMaxValue != nMax) {
        m_nMaxValue = nMax;
        Invalidate();
    }    
}

double Progress::GetValue() const
{
    return m_fCurrentValue;
}

void Progress::SetValue(double fValue)
{
    if (fValue > m_nMaxValue) {
        fValue = m_nMaxValue;
    }
    if (fValue < m_nMinValue) {
        fValue = m_nMinValue;
    }
    if (fValue == m_fCurrentValue) {
        return;
    }
    const double fOldValue = m_fCurrentValue;
    m_fCurrentValue = fValue;

    //值变化动画（未初始化或动画关闭时直接到位）
    if (m_bAnimationEnabled && IsInited() && (m_nAnimationDuration > 0)) {
        StartValueAnimation(m_fDisplayValue, fValue);
    }
    else {
        m_fDisplayValue = fValue;
        UpdatePercentText();
    }

    //发送值变化事件
    SendValueChangedEvent(fValue, fOldValue);
    Invalidate();
}

bool Progress::IsStretchForeImage()
{
    return m_bStretchForeImage;
}

void Progress::SetStretchForeImage(bool bStretchForeImage /*= true*/)
{
    if (m_bStretchForeImage == bStretchForeImage) {
        return;
    }
    m_bStretchForeImage = bStretchForeImage;
    Invalidate();
}

DString Progress::GetProgressImage() const
{
    if (m_pProgressImage != nullptr) {
        return m_pProgressImage->GetImageString();
    }
    return DString();
}

void Progress::SetProgressImage(const DString& strImage)
{
    if (m_pProgressImage == nullptr) {
        m_pProgressImage = new Image;
    }
    m_pProgressImage->SetImageString(strImage, Dpi());
    Invalidate();
}

DString Progress::GetProgressColor() const
{
    return m_sProgressColor.c_str();
}

void Progress::SetProgressColor(const DString& strProgressColor)
{
    //不校验颜色是否存在，绘制时若解析不到有效颜色则跳过（宽松处理，避免断言崩溃）
    if (m_sProgressColor == strProgressColor) {
        return;
    }
    m_sProgressColor = strProgressColor;
    Invalidate();
}

void Progress::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    int32_t iValue = GetMarqueeWidth();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetMarqueeWidth(iValue, false);

    iValue = GetMarqueeStep();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetMarqueeStep(iValue, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Progress::PaintStateImages(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (m_bMarquee && !m_sProgressColor.empty()) {
        PaintMarquee(pRender);
        return;
    }

    UiRect rc = GetProgressPos();
    if (!m_sProgressColor.empty()) {
        UiColor dwProgressColor = this->GetUiColor(m_sProgressColor.c_str());
        if (dwProgressColor.GetARGB() != 0) {
            UiRect rcProgressColor = GetRect();
            if (IsHorizontal()) {
                rcProgressColor.right = rcProgressColor.left + rc.right;
            }
            else {
                rcProgressColor.top = rcProgressColor.top + rc.top;
            }
            if (rcProgressColor.IsEmpty()) {
                return;
            }
            //圆角填充（progress_radius 配置，默认 0 为直角）
            int32_t nRadius = Dpi().GetScaleInt(m_nProgressRadius);
            if (nRadius > 0) {
                int32_t nMaxRadius = std::min(rcProgressColor.Width(), rcProgressColor.Height()) / 2;
                nRadius = std::min(nRadius, nMaxRadius);
                if (nRadius > 0) {
                    pRender->FillRoundRect(UiRectF::MakeFromRect(rcProgressColor), static_cast<float>(nRadius), static_cast<float>(nRadius), dwProgressColor, GetAlpha());
                }
                else {
                    pRender->FillRect(UiRectF::MakeFromRect(rcProgressColor), dwProgressColor, GetAlpha());
                }
            }
            else {
                pRender->FillRect(UiRectF::MakeFromRect(rcProgressColor), dwProgressColor, GetAlpha());
            }
        }
    }
    if (rc.IsEmpty() || (m_pProgressImage == nullptr) || (m_pProgressImage->GetImageString().empty())) {
        return;
    }
    //加载图片资源
    LoadImageInfo(*m_pProgressImage);

    m_sProgressImageModify.clear();
    if (m_bStretchForeImage) {
        m_sProgressImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);
    }
    else {
        ui::UiRect m_rcSrc = rc;
        std::shared_ptr<ImageInfo> pProgressImageCache = m_pProgressImage->GetImageInfo();
        if (pProgressImageCache != nullptr) {
            if (m_rcSrc.right > pProgressImageCache->GetWidth()) {
                m_rcSrc.right = pProgressImageCache->GetWidth();
            }
            if (m_rcSrc.bottom > pProgressImageCache->GetHeight()) {
                m_rcSrc.bottom = pProgressImageCache->GetHeight();
            }
        }
        m_sProgressImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'")
            , rc.left, rc.top, rc.right, rc.bottom
            , m_rcSrc.left, m_rcSrc.top, m_rcSrc.right, m_rcSrc.bottom);
    }

    // 让corner的值不超过可绘制范围
    const UiRect corner = m_pProgressImage->GetImageAttribute().GetImageCorner();
    if (IsHorizontal()) {
        if (corner.left != 0 && corner.left >= rc.right) {
            DString imageModify = m_sProgressImageModify.c_str();
            imageModify += StringUtil::Printf(_T(" corner='%d,%d,%d,%d'"),
                rc.right,
                corner.top,
                0,
                corner.bottom);
            m_sProgressImageModify = imageModify;
        }
    }
    else {
        if (corner.top != 0 && corner.top >= rc.bottom) {
            DString imageModify = m_sProgressImageModify.c_str();
            imageModify += StringUtil::Printf(_T(" corner='%d,%d,%d,%d'"),
                corner.left,
                corner.bottom,
                corner.right,
                0);
            m_sProgressImageModify = imageModify;
        }
    }
    PaintImage(pRender, m_pProgressImage, m_sProgressImageModify.c_str());
}

UiRect Progress::GetProgressPos() const
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    double fValue = GetDisplayValue();

    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > nMax) {
        fValue = nMax;
    }
    if (fValue < nMin) {
        fValue = nMin;
    }
    if (IsReverse()) {
        //逆向滚动条
        fValue = (nMax - nMin) - fValue;
    }

    UiRect rc;
    if (IsHorizontal()) {
        rc.right = static_cast<int>(std::ceil(static_cast<double>((fValue - nMin) * (GetRect().right - GetRect().left)) / static_cast<double>(nMax - nMin)));
        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        rc.top = static_cast<int>(std::ceil(static_cast<double>((nMax - fValue) * (GetRect().bottom - GetRect().top)) / static_cast<double>(nMax - nMin)));
        rc.bottom = GetRect().bottom - GetRect().top;
        rc.right = GetRect().right - GetRect().left;
    }
    return rc;
}

void Progress::ClearImageCache()
{
    BaseClass::ClearImageCache();
    if (m_pProgressImage != nullptr) {
        m_pProgressImage->ClearImageCache();
    }    
}

void Progress::Play()
{
    if (!m_bMarquee || !IsVisible() || !IsEnabled()) {
        m_timer.Cancel();
        return;
    }
    m_nMarqueePos = m_nMarqueePos + GetMarqueeStep();

    ui::UiRect rc = GetRect();
    if (IsHorizontal()) {
        if (m_nMarqueePos > rc.right - rc.left) {
            m_nMarqueePos = (m_nMarqueePos - (rc.right - rc.left)) - GetMarqueeWidth();
        }
    }
    else {
        if (m_nMarqueePos > rc.bottom - rc.top) {
            m_nMarqueePos = (m_nMarqueePos - (rc.bottom - rc.top) - GetMarqueeWidth());
        }
    }

    Invalidate();
}

void Progress::PaintMarquee(IRender* pRender) 
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    if (!m_sProgressColor.empty()) {
        UiColor dwProgressColor = GetUiColor(m_sProgressColor.c_str());
        if (dwProgressColor.GetARGB() != 0) {
            ui::UiRect rc = GetRect();
            if (IsHorizontal()) {
                rc.left = std::max(m_nMarqueePos, 0) + rc.left;
                rc.right = rc.left + (m_nMarqueePos >= 0 ? GetMarqueeWidth() : (GetMarqueeWidth() + m_nMarqueePos));
            }
            else {
                rc.top = std::max(m_nMarqueePos, 0) + rc.top;
                rc.bottom = rc.top + (m_nMarqueePos >= 0 ? GetMarqueeWidth() : (GetMarqueeWidth() + m_nMarqueePos));
            }
            pRender->FillRect(UiRectF::MakeFromRect(rc), dwProgressColor);
        }
    }
}

bool Progress::IsMarquee() const
{
    return m_bMarquee;
}

void Progress::SetMarquee(bool bMarquee)
{
    if (m_bMarquee == bMarquee) {
        return;
    }

    m_bMarquee = bMarquee;
    m_nMarqueePos = 0;

    if (m_bMarquee) {
        auto playCallback = UiBind(&Progress::Play, this);
        GlobalManager::Instance().Timer().AddTimer(m_timer.GetWeakFlag(), playCallback, m_nMarqueeElapsed);
    }
    else {
        m_timer.Cancel();
    }
    Invalidate();
}

int32_t Progress::GetMarqueeWidth() const
{
    return m_nMarqueeWidth;
}

void Progress::SetMarqueeWidth(int32_t nMarqueeWidth, bool bNeedDpiScale)
{
    if (nMarqueeWidth < 0) {
        nMarqueeWidth = 10;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nMarqueeWidth);
    }
    if (m_nMarqueeWidth != nMarqueeWidth) {
        m_nMarqueeWidth = nMarqueeWidth;
        Invalidate();
    }
}

int32_t Progress::GetMarqueeStep() const
{
    return m_nMarqueeStep;
}

void Progress::SetMarqueeStep(int32_t nMarqueeStep, bool bNeedDpiScale)
{
    if (nMarqueeStep < 0) {
        nMarqueeStep = 4;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nMarqueeStep);
    }
    if (m_nMarqueeStep != nMarqueeStep) {
        m_nMarqueeStep = nMarqueeStep;
        Invalidate();
    }    
}

int32_t Progress::GetMarqueeElapsed() const
{
    return m_nMarqueeElapsed;
}

void Progress::SetMarqueeElapsed(int32_t nMarqueeElapsed)
{
    if (!IsMarquee() || nMarqueeElapsed == m_nMarqueeElapsed) {
        return;
    }

    m_nMarqueeElapsed = nMarqueeElapsed;
    m_timer.Cancel();

    auto playCallback = UiBind(&Progress::Play, this);
    GlobalManager::Instance().Timer().AddTimer(m_timer.GetWeakFlag(), playCallback, m_nMarqueeElapsed);

    Invalidate();
}

void Progress::SetReverse(bool bReverse)
{
    m_bReverse = bReverse;
}

bool Progress::IsReverse() const
{
    return m_bReverse;
}

void Progress::SetShowPercent(bool bShowPercent)
{
    if (m_bShowPercent != bShowPercent) {
        m_bShowPercent = bShowPercent;
        if (m_bShowPercent) {
            UpdatePercentText();
        }
        Invalidate();
    }
}

void Progress::SetAnimationEnabled(bool bEnabled)
{
    m_bAnimationEnabled = bEnabled;
    if (!bEnabled) {
        //关闭动画：停止旧动画并直接到位
        if ((m_pAnimPlayer != nullptr) && m_pAnimPlayer->IsPlaying()) {
            m_pAnimPlayer->SetCompleteCallback(nullptr);
            m_pAnimPlayer->Stop();
        }
        m_pAnimPlayer.reset();
        m_fDisplayValue = m_fCurrentValue;
    }
}

void Progress::SetAnimationDuration(int32_t nMs)
{
    if (nMs < 0) {
        nMs = 0;
    }
    m_nAnimationDuration = nMs;
}

void Progress::SetProgressRadius(int32_t nRadius)
{
    if (nRadius < 0) {
        nRadius = 0;
    }
    if (m_nProgressRadius != nRadius) {
        m_nProgressRadius = nRadius;
        Invalidate();
    }
}

void Progress::SendValueChangedEvent(double newValue, double oldValue)
{
    //值按整数截断传递（与 Slider 一致）；精确浮点值可从 GetValue() 读取
    SendEvent(kEventValueChanged, (WPARAM)newValue, (LPARAM)oldValue);
}

void Progress::UpdatePercentText()
{
    if (!m_bShowPercent) {
        return;
    }
    int32_t nMin = m_nMinValue;
    int32_t nMax = m_nMaxValue;
    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    double fPercent = (GetDisplayValue() - nMin) / static_cast<double>(nMax - nMin) * 100.0;
    if (fPercent < 0.0) {
        fPercent = 0.0;
    }
    if (fPercent > 100.0) {
        fPercent = 100.0;
    }
    int32_t nPercent = static_cast<int32_t>(std::lround(fPercent));
    SetText(StringUtil::Printf(_T("%d%%"), nPercent));
}

void Progress::StartValueAnimation(double fFrom, double fTo)
{
    //停止旧动画
    if ((m_pAnimPlayer != nullptr) && m_pAnimPlayer->IsPlaying()) {
        m_pAnimPlayer->SetCompleteCallback(nullptr);
        m_pAnimPlayer->Stop();
    }
    m_pAnimPlayer.reset(new AnimationPlayer);
    m_pAnimPlayer->SetAnimationType(AnimationType::kAnimationNone);
    m_pAnimPlayer->SetTotalMillSeconds(m_nAnimationDuration);
    m_pAnimPlayer->SetFrameIntervalMillSeconds(16);
    m_pAnimPlayer->SetEasingFunctionType(EaseOutQuad);
    m_pAnimPlayer->SetStartValue(0);
    m_pAnimPlayer->SetEndValue(1000);

    std::weak_ptr<WeakFlag> weakFlag = GetWeakFlag();
    m_pAnimPlayer->SetPlayCallback([this, weakFlag, fFrom, fTo](int32_t nValue) {
        if (weakFlag.expired()) {
            return;
        }
        m_fDisplayValue = fFrom + (fTo - fFrom) * static_cast<double>(nValue) / 1000.0;
        UpdatePercentText();
        Invalidate();
    });
    m_pAnimPlayer->SetCompleteCallback([this, weakFlag, fTo]() {
        if (weakFlag.expired()) {
            return;
        }
        m_fDisplayValue = fTo;
        UpdatePercentText();
        Invalidate();
    });
    m_pAnimPlayer->Start();
}

}
