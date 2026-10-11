#include "Progress.h"
#include "duilib/Core/GlobalManager.h"
#include <algorithm>
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
    m_easingAnimation(EaseOutQuad),
    m_strAnimationEasing(_T("EaseOutQuad")),
    m_pAnimPlayer(nullptr),
    m_bShowPercent(false),
    m_nProgressRadius(0),
    m_fBufferValue(-1),
    m_strBufferColor(),
    m_textFormatCallback(),
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
    else if ((srName == _T("buffer_value")) || (srName == _T("buffervalue"))) {
        SetBufferValue(StringUtil::StringToDouble(strValue));
    }
    else if ((srName == _T("buffer_color")) || (srName == _T("buffercolor"))) {
        SetBufferColor(strValue);
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
    else if ((srName == _T("animation_easing")) || (srName == _T("animationeasing"))) {
        SetAnimationEasingName(strValue);
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
        //min 变化后把当前值拉回新区间，避免 GetValue() 返回越界的值
        ClampCurrentValue();
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
        //max 变化后把当前值拉回新区间，避免 GetValue() 返回越界的值
        ClampCurrentValue();
        Invalidate();
    }    
}

void Progress::ClampCurrentValue()
{
    //区间可能非法（如 min > max），此时保持当前值不动，由绘制/取值的既有逻辑兜底
    if (m_nMaxValue < m_nMinValue) {
        return;
    }
    //buffer 未设置时为 -1（绘制条件要求 > min），此时不参与钳位判断，避免被当成越界值
    const bool bBufferSet = (m_fBufferValue > m_nMinValue);
    if ((m_fCurrentValue >= m_nMinValue) && (m_fCurrentValue <= m_nMaxValue) &&
        (!bBufferSet || (m_fBufferValue <= m_nMaxValue)) &&
        (m_fDisplayValue >= m_nMinValue) && (m_fDisplayValue <= m_nMaxValue)) {
        return;
    }

    double fNewValue = m_fCurrentValue;
    if (fNewValue > m_nMaxValue) {
        fNewValue = m_nMaxValue;
    }
    if (fNewValue < m_nMinValue) {
        fNewValue = m_nMinValue;
    }
    //次级进度（buffer）：仅在已设置时才钳位，保持"未设置"为负值的原有语义
    double fNewBuffer = m_fBufferValue;
    if (bBufferSet) {
        if (fNewBuffer > m_nMaxValue) {
            fNewBuffer = m_nMaxValue;
        }
        if (fNewBuffer < m_nMinValue) {
            fNewBuffer = m_nMinValue;
        }
    }
    //显示值：动画播放中时同步收敛，避免动画结束后又跳回去
    double fNewDisplay = m_fDisplayValue;
    if (fNewDisplay > m_nMaxValue) {
        fNewDisplay = m_nMaxValue;
    }
    if (fNewDisplay < m_nMinValue) {
        fNewDisplay = m_nMinValue;
    }

    const bool bValueClamped = (fNewValue != m_fCurrentValue);
    m_fCurrentValue = fNewValue;
    m_fDisplayValue = fNewDisplay;
    if (fNewBuffer != m_fBufferValue) {
        m_fBufferValue = fNewBuffer;
    }

    if (bValueClamped) {
        //值被外部区间变化钳位：同步百分比文本（动画中的显示值已随之收敛）
        UpdatePercentText();
        //正在播放的动画其终点是钳位前的旧值，会把显示值重新推回越界区间，必须停止
        if ((m_pAnimPlayer != nullptr) && m_pAnimPlayer->IsPlaying()) {
            m_pAnimPlayer->SetCompleteCallback(nullptr);
            m_pAnimPlayer->Stop();
        }
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

    //完成事件：从"未完成"跨到"完成端点"（正向到 max，倒数到 min）时触发一次
    const bool bOldCompleted = m_bReverse ? (fOldValue <= m_nMinValue) : (fOldValue >= m_nMaxValue);
    const bool bCompleted = m_bReverse ? (fValue <= m_nMinValue) : (fValue >= m_nMaxValue);
    if (bCompleted && !bOldCompleted) {
        SendEvent(kEventProgressCompleted, 0, 0);
    }

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

    //buffer 次级进度（先画，主进度覆盖其上）：视频/下载的"已缓冲 + 已播放"双进度
    if ((m_fBufferValue > m_fCurrentValue) && (m_fBufferValue > m_nMinValue)) {
        UiColor bufferColor = GetUiColor(m_strBufferColor);
        if (bufferColor.GetARGB() == 0) {
            //未指定 buffer 颜色：使用前景色的浅色版
            bufferColor = GetUiColor(m_sProgressColor.c_str());
        }
        if (bufferColor.GetARGB() != 0) {
            UiRect rcBuffer = CalcProgressRect(m_fBufferValue);
            UiRect rcBufferColor = GetRect();
            if (IsHorizontal()) {
                rcBufferColor.right = rcBufferColor.left + rcBuffer.right;
            }
            else {
                rcBufferColor.top = rcBufferColor.top + rcBuffer.top;
            }
            if (!rcBufferColor.IsEmpty()) {
                //buffer 更浅（约 40% 不透明度）
                uint8_t bufferAlpha = static_cast<uint8_t>(GetAlpha() * 40 / 100);
                DrawSolidProgressRect(pRender, rcBufferColor, bufferColor, bufferAlpha);
            }
        }
    }

    //主进度纯色
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
            DrawSolidProgressRect(pRender, rcProgressColor, dwProgressColor, GetAlpha());
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
    return CalcProgressRect(GetDisplayValue());
}

UiRect Progress::CalcProgressRect(double value) const
{
    int32_t nMax = GetMaxValue();
    int32_t nMin = GetMinValue();
    double fValue = value;

    if (nMax <= nMin) {
        nMax = nMin + 1;
    }
    if (fValue > nMax) {
        fValue = nMax;
    }
    if (fValue < nMin) {
        fValue = nMin;
    }
    //reverse（倒数）在下方按比例取反实现；此处不再做 (nMax-nMin)-fValue 的绝对值翻转，
    //那种写法在 min != 0 时会算出越界比例

    UiRect rc;
    if (IsHorizontal()) {
        //归一化为比例后再按 reverse 取反：value=min 时进度为空、value=max 时为满格
        double fRatio = (fValue - nMin) / static_cast<double>(nMax - nMin);
        fRatio = std::clamp(fRatio, 0.0, 1.0);
        if (IsReverse()) {
            fRatio = 1.0 - fRatio;
        }
        rc.right = static_cast<int>(std::ceil(fRatio * (GetRect().right - GetRect().left)));
        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        double fRatio = (fValue - nMin) / static_cast<double>(nMax - nMin);
        fRatio = std::clamp(fRatio, 0.0, 1.0);
        if (IsReverse()) {
            fRatio = 1.0 - fRatio;
        }
        //垂直进度条自下而上填充，top 由剩余高度决定
        rc.top = static_cast<int>(std::ceil((1.0 - fRatio) * (GetRect().bottom - GetRect().top)));
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

    //轨道长度与滚动块宽度：轨道长 <= 0 时无可滚动空间，直接复位避免死循环累加
    ui::UiRect rc = GetRect();
    const int32_t nTrackLen = IsHorizontal() ? (rc.right - rc.left) : (rc.bottom - rc.top);
    if (nTrackLen <= 0) {
        m_nMarqueePos = 0;
        Invalidate();
        return;
    }
    //循环周期 = 轨道长 + 滚动块宽（块走完整个轨道后从头部重新出现）
    const int32_t nCycle = nTrackLen + GetMarqueeWidth();
    if (nCycle > 0) {
        m_nMarqueePos = m_nMarqueePos % nCycle;
        if (m_nMarqueePos < 0) {
            m_nMarqueePos += nCycle;
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
            if (!rc.IsEmpty()) {
                //与主进度共用 progress_radius，避免配置了圆角后marquee 仍是直角导致风格不一致
                DrawSolidProgressRect(pRender, rc, dwProgressColor, GetAlpha());
            }
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

void Progress::SetAnimationEasing(EasingFunctionType easing)
{
    m_easingAnimation = easing;
}

void Progress::SetAnimationEasingName(const DString& strEasing)
{
    if (strEasing.empty()) {
        //空值回退为默认曲线，保持与构造函数一致
        m_easingAnimation = EaseOutQuad;
        m_strAnimationEasing = _T("EaseOutQuad");
        return;
    }
    //GetEasingFunctionType 的键与 EasingFunctionType 枚举名完全一致（首字母大小写均可），
    //无法识别时它会返回 EaseLinear，这里同步记录原始字符串，便于排查
    const EasingFunctionType easing = EasingFunctions::GetEasingFunctionType(strEasing);
    m_easingAnimation = easing;
    m_strAnimationEasing = strEasing;
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

void Progress::SetBufferValue(double fValue)
{
    if (fValue != m_fBufferValue) {
        m_fBufferValue = fValue;
        Invalidate();
    }
}

void Progress::SetBufferColor(const DString& strColor)
{
    if (m_strBufferColor != strColor) {
        m_strBufferColor = strColor;
        Invalidate();
    }
}

void Progress::SetTextFormatCallback(const TextFormatCallback& callback)
{
    m_textFormatCallback = callback;
    if (m_bShowPercent) {
        UpdatePercentText();
    }
    Invalidate();
}

void Progress::DrawSolidProgressRect(IRender* pRender, const UiRect& rcProgressColor, UiColor color, uint8_t alpha)
{
    //圆角填充（progress_radius 配置，默认 0 为直角）
    int32_t nRadius = Dpi().GetScaleInt(m_nProgressRadius);
    if (nRadius > 0) {
        int32_t nMaxRadius = std::min(rcProgressColor.Width(), rcProgressColor.Height()) / 2;
        nRadius = std::min(nRadius, nMaxRadius);
        if (nRadius > 0) {
            pRender->FillRoundRect(UiRectF::MakeFromRect(rcProgressColor), static_cast<float>(nRadius), static_cast<float>(nRadius), color, alpha);
            return;
        }
    }
    pRender->FillRect(UiRectF::MakeFromRect(rcProgressColor), color, alpha);
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
    if (m_textFormatCallback) {
        //自定义格式：由回调生成文本
        DString strText = m_textFormatCallback(GetDisplayValue(), static_cast<double>(m_nMinValue), static_cast<double>(m_nMaxValue));
        SetText(strText);
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
    m_pAnimPlayer->SetEasingFunctionType(m_easingAnimation);
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
