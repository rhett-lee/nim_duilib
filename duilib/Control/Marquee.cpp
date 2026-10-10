#include "Marquee.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

Marquee::Marquee(Window* pWindow) :
    BaseClass(pWindow),
    m_direction(MarqueeDirection::kLeft),
    m_nScrollSpeed(50),
    m_nScrollInterval(30),
    m_bHoverPause(true),
    m_bPaused(false),
    m_bHovering(false),
    m_bScrolling(false),
    m_bTimerStarted(false),
    m_nTimerId(0),
    m_fOffset(0.0f),
    m_fPeriod(0.0f),
    m_fTextMainSize(0.0f),
    m_fTextCrossSize(0.0f)
{
}

Marquee::~Marquee()
{
    StopTimer();
}

DString Marquee::GetType() const
{
    return DUI_CTR_MARQUEE;
}

void Marquee::SetAttribute(const DString& strName, const DString& strValue)
{
    if ((strName == _T("scroll_direction")) || (strName == _T("scrolldirection"))) {
        if (strValue == _T("right")) {
            SetScrollDirection(MarqueeDirection::kRight);
        }
        else if (strValue == _T("up")) {
            SetScrollDirection(MarqueeDirection::kUp);
        }
        else if (strValue == _T("down")) {
            SetScrollDirection(MarqueeDirection::kDown);
        }
        else {
            SetScrollDirection(MarqueeDirection::kLeft);
        }
    }
    else if ((strName == _T("scroll_speed")) || (strName == _T("scrollspeed"))) {
        SetScrollSpeed(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("scroll_interval")) || (strName == _T("scrollinterval"))) {
        SetScrollInterval(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("hover_pause")) || (strName == _T("hoverpause"))) {
        SetHoverPause(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Marquee::SetScrollDirection(MarqueeDirection direction)
{
    if (m_direction != direction) {
        m_direction = direction;
        //滚动方向决定文本排列方向：上下滚为纵向文本，左右滚为横向文本
        bool bVertical = (direction == MarqueeDirection::kUp) || (direction == MarqueeDirection::kDown);
        SetVerticalText(bVertical);
        ResetScroll();
        Invalidate();
    }
}

void Marquee::SetScrollSpeed(int32_t nSpeed)
{
    if (nSpeed < 0) {
        nSpeed = 0;
    }
    if (m_nScrollSpeed != nSpeed) {
        m_nScrollSpeed = nSpeed;
    }
}

void Marquee::SetScrollInterval(int32_t nIntervalMs)
{
    if (nIntervalMs < 5) {
        nIntervalMs = 5;
    }
    if (nIntervalMs > 1000) {
        nIntervalMs = 1000;
    }
    if (m_nScrollInterval != nIntervalMs) {
        m_nScrollInterval = nIntervalMs;
        //重启定时器使新的间隔生效
        if (m_bTimerStarted) {
            StopTimer();
            StartTimer();
        }
    }
}

void Marquee::SetHoverPause(bool bHoverPause)
{
    if (m_bHoverPause != bHoverPause) {
        m_bHoverPause = bHoverPause;
    }
}

void Marquee::Pause()
{
    if (!m_bPaused) {
        m_bPaused = true;
    }
}

void Marquee::Resume()
{
    if (m_bPaused) {
        m_bPaused = false;
    }
}

void Marquee::SetPaused(bool bPaused)
{
    m_bPaused = bPaused;
}

void Marquee::SetText(const DString& strText)
{
    BaseClass::SetText(strText);
    ResetScroll();
}

void Marquee::SetTextId(const DString& strTextId)
{
    BaseClass::SetTextId(strTextId);
    ResetScroll();
}

void Marquee::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    //DPI 变化后文本的像素尺寸改变，清空测量缓存
    ResetScroll();
    m_strMeasuredText.clear();
    m_fTextMainSize = 0.0f;
    m_fTextCrossSize = 0.0f;
}

void Marquee::SetWindow(Window* pWindow)
{
    if (GetWindow() != pWindow) {
        StopTimer();
    }
    BaseClass::SetWindow(pWindow);
}

bool Marquee::MouseEnter(const EventArgs& msg)
{
    m_bHovering = true;
    return BaseClass::MouseEnter(msg);
}

bool Marquee::MouseLeave(const EventArgs& msg)
{
    m_bHovering = false;
    return BaseClass::MouseLeave(msg);
}

bool Marquee::IsPausedEffective() const
{
    return m_bPaused || (m_bHoverPause && m_bHovering);
}

void Marquee::StartTimer()
{
    if (m_bTimerStarted || !IsInited()) {
        return;
    }
    m_bTimerStarted = true;
    m_timerFlag.Cancel(); //先取消旧的 weakFlag，避免残留
    std::function<void()> callback = UiBind(&Marquee::OnTimerTick, this);
    TimerManager& timer = GlobalManager::Instance().Timer();
    m_nTimerId = timer.AddTimer(m_timerFlag.GetWeakFlag(), callback,
                                static_cast<uint32_t>(m_nScrollInterval), -1);
}

void Marquee::StopTimer()
{
    if (m_bTimerStarted) {
        if (m_nTimerId > 0) {
            GlobalManager::Instance().Timer().RemoveTimer(m_nTimerId);
            m_nTimerId = 0;
        }
        m_bTimerStarted = false;
    }
    m_timerFlag.Cancel();
}

void Marquee::ResetScroll()
{
    m_fOffset = 0.0f;
    m_fPeriod = 0.0f;
    //文本内容变化，测量缓存失效（下次绘制时重新测量）
    m_strMeasuredText.clear();
    m_fTextMainSize = 0.0f;
    m_fTextCrossSize = 0.0f;
    //文本变化后，滚动状态需在下一次绘制时重新判定（可能从滚动变为静态，或反之）
}

void Marquee::OnTimerTick()
{
    if (IsPausedEffective()) {
        return;
    }
    if (m_nScrollSpeed <= 0) {
        return;
    }
    const float fDelta = static_cast<float>(Dpi().GetScaleInt(m_nScrollSpeed)) *
                         static_cast<float>(m_nScrollInterval) / 1000.0f;
    m_fOffset += fDelta;
    //按周期循环（周期由 PaintText 测量后缓存）
    if ((m_fPeriod > 0.0f) && (m_fOffset >= m_fPeriod)) {
        m_fOffset -= m_fPeriod;
    }
    Invalidate();
}

void Marquee::PaintText(IRender* pRender)
{
    if (pRender == nullptr) {
        BaseClass::PaintText(pRender);
        return;
    }
    const DString text = GetText();
    if (text.empty()) {
        BaseClass::PaintText(pRender);
        return;
    }

    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    rc.Deflate(GetTextPadding());
    if ((rc.Width() <= 0) || (rc.Height() <= 0)) {
        return;
    }

    const bool bVertical = IsVerticalText();
    //测量文本尺寸（带缓存：文本未变化时复用上次测量结果，避免每帧 MeasureString）
    if (text != m_strMeasuredText) {
        MeasureStringParam measureParam = GetMeasureParam();
        UiRect rcTextSize = pRender->MeasureString(text, measureParam);
        m_strMeasuredText = text;
        m_fTextMainSize = bVertical ? static_cast<float>(rcTextSize.Height())
                                    : static_cast<float>(rcTextSize.Width());
        m_fTextCrossSize = bVertical ? static_cast<float>(rcTextSize.Width())
                                     : static_cast<float>(rcTextSize.Height());
    }
    const float fTextMainSize = m_fTextMainSize;
    const float fViewMainSize = bVertical ? static_cast<float>(rc.Height())
                                          : static_cast<float>(rc.Width());
    const bool bNeedScroll = (fTextMainSize > fViewMainSize);

    //滚动状态变化：启动/停止定时器
    if (bNeedScroll != m_bScrolling) {
        m_bScrolling = bNeedScroll;
        if (bNeedScroll) {
            StartTimer();
        }
        else {
            StopTimer();
            m_fOffset = 0.0f;
        }
    }

    //未超出：静态显示（走 Label 默认对齐绘制）
    if (!bNeedScroll) {
        BaseClass::PaintText(pRender);
        return;
    }

    //滚动绘制：双份文本首尾相接无缝循环
    const float fGap = fViewMainSize / 3.0f;
    m_fPeriod = fTextMainSize + fGap;
    //偏移归一化到 [0, period)
    if (m_fPeriod > 0.0f) {
        while (m_fOffset >= m_fPeriod) {
            m_fOffset -= m_fPeriod;
        }
        while (m_fOffset < 0.0f) {
            m_fOffset += m_fPeriod;
        }
    }

    //文本颜色
    ControlStateType stateType = kControlStateNormal;
    DString clrName = GetPaintStateTextColor(GetState(), stateType);
    UiColor textColor = GetUiColor(clrName);
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }

    DrawStringParam drawParam = GetDrawParam();
    drawParam.dwTextColor = textColor;
    drawParam.uFade = 255;

    const float fOff = m_fOffset;
    //裁剪到显示区域，避免滚动文字绘制到控件外
    std::unique_ptr<AutoClip> spClip = CreateRectClip(pRender, rc, true);

    if (bVertical) {
        //纵向文本：列宽取文本宽，水平居中于显示区
        const float fColW = m_fTextCrossSize;
        const float fX = static_cast<float>(rc.left) + (static_cast<float>(rc.Width()) - fColW) / 2.0f;
        const float fX2 = fX + fColW;
        float fY1;
        if (m_direction == MarqueeDirection::kUp) {
            fY1 = static_cast<float>(rc.top) - fOff;
        }
        else { //kDown
            fY1 = static_cast<float>(rc.top) - m_fPeriod + fOff;
        }
        const float fY2 = fY1 + m_fPeriod;
        drawParam.uFormat = TEXT_LEFT | TEXT_TOP | TEXT_VERTICAL;
        drawParam.textRect = UiRect(static_cast<int32_t>(fX), static_cast<int32_t>(fY1),
                                    static_cast<int32_t>(fX2), static_cast<int32_t>(fY1 + fTextMainSize));
        pRender->DrawString(text, drawParam);
        drawParam.textRect = UiRect(static_cast<int32_t>(fX), static_cast<int32_t>(fY2),
                                    static_cast<int32_t>(fX2), static_cast<int32_t>(fY2 + fTextMainSize));
        pRender->DrawString(text, drawParam);
    }
    else {
        //横向文本：行高取文本高，垂直居中于显示区
        const float fRowH = m_fTextCrossSize;
        const float fY = static_cast<float>(rc.top) + (static_cast<float>(rc.Height()) - fRowH) / 2.0f;
        const float fY2 = fY + fRowH;
        float fX1;
        if (m_direction == MarqueeDirection::kLeft) {
            fX1 = static_cast<float>(rc.left) - fOff;
        }
        else { //kRight
            fX1 = static_cast<float>(rc.left) - m_fPeriod + fOff;
        }
        const float fX2 = fX1 + m_fPeriod;
        drawParam.uFormat = TEXT_LEFT | TEXT_TOP | TEXT_SINGLELINE;
        drawParam.textRect = UiRect(static_cast<int32_t>(fX1), static_cast<int32_t>(fY),
                                    static_cast<int32_t>(fX1 + fTextMainSize), static_cast<int32_t>(fY2));
        pRender->DrawString(text, drawParam);
        drawParam.textRect = UiRect(static_cast<int32_t>(fX2), static_cast<int32_t>(fY),
                                    static_cast<int32_t>(fX2 + fTextMainSize), static_cast<int32_t>(fY2));
        pRender->DrawString(text, drawParam);
    }
    spClip.reset();
}

} // namespace ui
