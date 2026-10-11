#include "Slider.h"
#include <algorithm>
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

Slider::Slider(Window* pWindow) :
    Progress(pWindow),
    m_szThumb(0, 0),
    m_nStep(1),
    m_sImageModify(),
    m_thumbStateImage(),
    m_rcProgressBarPadding(),
    m_bClickToJump(true),
    m_nPageStep(0),
    m_bShowTicks(false),
    m_nTickCount(5),
    m_nTickLength(0),
    m_strTickColor(),
    m_tickPos(SliderTickPos::kBottom),
    m_bShowTickLabels(false),
    m_nTickLabelInterval(1),
    m_strTickFontId(_T("system_regular_12")),
    m_strTickLabelColor(),
    m_tickLabelCallback()
{
    SetTextStyle(TEXT_SINGLELINE | TEXT_HCENTER, false);
    m_nTickLength = 4;   //刻度线默认长度（未缩放原始值）
}

DString Slider::GetType() const { return DUI_CTR_SLIDER; }

uint32_t Slider::GetControlFlags() const
{
    //键盘可达性：可用且允许键盘操作时，纳入 Tab 键焦点循环
    return (IsEnabled() && IsKeyboardEnabled() && IsAllowTabStop()) ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void Slider::SetClickToJump(bool bEnable)
{
    m_bClickToJump = bEnable;
}

void Slider::SetPageStep(int32_t nPageStep)
{
    m_nPageStep = nPageStep;
}

int32_t Slider::GetPageStep() const
{
    if (m_nPageStep > 0) {
        return m_nPageStep;
    }
    //自动模式：总跨度的 1/10，至少为 1
    int32_t nRange = GetMaxValue() - GetMinValue();
    if (nRange <= 0) {
        return 1;
    }
    int32_t nPageStep = nRange / 10;
    if (nPageStep < 1) {
        nPageStep = 1;
    }
    if (nPageStep < m_nStep) {
        nPageStep = m_nStep;
    }
    return nPageStep;
}

void Slider::SetShowTicks(bool bShow)
{
    if (m_bShowTicks != bShow) {
        m_bShowTicks = bShow;
        Invalidate();
    }
}

void Slider::SetTickCount(int32_t nCount)
{
    if (nCount < 2) {
        nCount = 2;
    }
    if (m_nTickCount != nCount) {
        m_nTickCount = nCount;
        Invalidate();
    }
}

void Slider::SetTickLength(int32_t nLength)
{
    if (nLength < 0) {
        nLength = 0;
    }
    if (m_nTickLength != nLength) {
        //保存未缩放的原始值，绘制时按当前 DPI 缩放
        m_nTickLength = nLength;
        Invalidate();
    }
}

int32_t Slider::GetTickLength() const
{
    return m_nTickLength;
}

void Slider::SetTickColor(const DString& strColor)
{
    if (m_strTickColor != strColor) {
        m_strTickColor = strColor;
        Invalidate();
    }
}

void Slider::SetTickPos(SliderTickPos pos)
{
    if (m_tickPos != pos) {
        m_tickPos = pos;
        Invalidate();
    }
}

void Slider::SetShowTickLabels(bool bShow)
{
    if (m_bShowTickLabels != bShow) {
        m_bShowTickLabels = bShow;
        Invalidate();
    }
}

void Slider::SetTickLabelInterval(int32_t nInterval)
{
    if (nInterval < 1) {
        nInterval = 1;
    }
    if (m_nTickLabelInterval != nInterval) {
        m_nTickLabelInterval = nInterval;
        Invalidate();
    }
}

void Slider::SetTickFontId(const DString& strFontId)
{
    if (m_strTickFontId != strFontId) {
        m_strTickFontId = strFontId;
        Invalidate();
    }
}

void Slider::SetTickLabelColor(const DString& strColor)
{
    if (m_strTickLabelColor != strColor) {
        m_strTickLabelColor = strColor;
        Invalidate();
    }
}

void Slider::SetTickLabelCallback(const TickLabelCallback& callback)
{
    m_tickLabelCallback = callback;
    Invalidate();
}

double Slider::GetTickValue(int32_t nIndex) const
{
    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    const int32_t nCount = (m_nTickCount > 1) ? m_nTickCount : 1;
    if (nMax <= nMin) {
        return nMin;
    }
    return nMin + double(nMax - nMin) * (double(nIndex) / double(nCount - 1));
}

UiRect Slider::GetProgressPos() const
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
        // 使用 double 计算避免 int32 溢出，再通过 TruncateToInt32 安全转为 int32_t
        const double fValueScale = (GetRect().right - GetRect().left - szThumb.cx) / static_cast<double>(nMax - nMin);
        const int64_t nRight64 = static_cast<int64_t>((fValue - nMin) * fValueScale + szThumb.cx / 2.0 + 0.5);
        rc.right = TruncateToInt32(nRight64);
        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        const double fValueScale = (GetRect().bottom - GetRect().top - szThumb.cy) / static_cast<double>(nMax - nMin);
        const int64_t nTop64 = static_cast<int64_t>((nMax - fValue) * fValueScale + szThumb.cy / 2.0 + 0.5);
        rc.top = TruncateToInt32(nTop64);
        rc.right = GetRect().right - GetRect().left;
        rc.bottom = GetRect().bottom - GetRect().top;
    }

    return rc;
}

double Slider::ValueFromMousePt(const UiPoint& ptMouse) const
{
    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    const UiSize szThumb = GetThumbSize();
    const UiRect& rc = GetRect();
    if (IsHorizontal()) {
        if (ptMouse.x >= rc.right - szThumb.cx / 2) {
            return nMax;
        }
        if (ptMouse.x <= rc.left + szThumb.cx / 2) {
            return nMin;
        }
        return nMin + double((nMax - nMin) * (ptMouse.x - rc.left - szThumb.cx / 2)) /
            double(rc.right - rc.left - szThumb.cx);
    }
    else {
        if (ptMouse.y >= rc.bottom - szThumb.cy / 2) {
            return nMin;
        }
        if (ptMouse.y <= rc.top + szThumb.cy / 2) {
            return nMax;
        }
        return nMin + double((nMax - nMin) * (rc.bottom - ptMouse.y - szThumb.cy / 2)) /
            double(rc.bottom - rc.top - szThumb.cy);
    }
}

void Slider::StepValue(double fDelta, int32_t nStepCount)
{
    const int32_t nMin = GetMinValue();
    const int32_t nMax = GetMaxValue();
    const int32_t nStep = (m_nStep > 0) ? m_nStep : 1;
    const int32_t nCount = (nStepCount > 0) ? nStepCount : ((nStepCount < 0) ? -nStepCount : 1);
    double fValue = GetValue() + fDelta * nStep * nCount;
    if (fValue < nMin) {
        fValue = nMin;
    }
    if (fValue > nMax) {
        fValue = nMax;
    }
    SetValue(fValue);
}

bool Slider::HandleKeyDown(uint32_t vkCode, bool bShiftDown)
{
    const bool bHorizontal = IsHorizontal();
    //方向键：水平用左右、垂直用上下；PageUp/PageDown 恒为增大/减小方向
    //bBigStep 表示大步进（PageUp/PageDown，或 Shift+方向键）
    bool bIncrease = false;
    bool bArrowKey = true;
    if (vkCode == (bHorizontal ? kVK_RIGHT : kVK_UP)) {
        bIncrease = true;
    }
    else if (vkCode == (bHorizontal ? kVK_LEFT : kVK_DOWN)) {
        bIncrease = false;
    }
    else if (vkCode == kVK_PRIOR) {
        bIncrease = true;
        bArrowKey = false;
    }
    else if (vkCode == kVK_NEXT) {
        bIncrease = false;
        bArrowKey = false;
    }
    else if (vkCode == kVK_END) {
        SetValue(GetMaxValue());
        return true;
    }
    else if (vkCode == kVK_HOME) {
        SetValue(GetMinValue());
        return true;
    }
    else {
        return false;
    }

    //Shift+方向键 与 PageUp/PageDown 为大步进
    const bool bBigStep = !bArrowKey || bShiftDown;
    StepValue(bIncrease ? 1.0 : -1.0, bBigStep ? GetPageStep() : 1);
    return true;
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
    if (msg.eventType == kEventKeyDown) {
        if (IsEnabled() && IsKeyboardEnabled() &&
            HandleKeyDown(msg.vkCode, (msg.modifierKey & kShift) != 0)) {
            return;
        }
    }
    if (msg.eventType == kEventMouseButtonDown || msg.eventType == kEventMouseDoubleClick) {
        UiPoint newPtMouse(msg.ptMouse);
        newPtMouse.Offset(GetScrollOffsetInScrollBox());
        UiRect rcThumb = GetThumbRect();
        if (rcThumb.ContainsPt(newPtMouse)) {
            SetMouseFocused(true);
            return;
        }
        //点击轨道：默认按下即跳值，并进入拖动状态（可继续按住拖动）
        if (m_bClickToJump && IsEnabled() && IsMouseEnabled()) {
            SetValue(ValueFromMousePt(newPtMouse));
            SetMouseFocused(true);
        }
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        if(IsMouseFocused()) {
            SetMouseFocused(false);
        }
        //鼠标捕获结束时的最终校正（点击跳转关闭时，松开鼠标才跳值）
        if (!m_bClickToJump) {
            UiPoint ptMouse(msg.ptMouse);
            ptMouse.Offset(GetScrollOffsetInScrollBox());
            SetValue(ValueFromMousePt(ptMouse));
        }
        return;
    }
    if (msg.eventType == kEventMouseWheel) {
        int32_t detaValue = msg.eventData;
        if (detaValue > 0) {
            SetValue(GetValue() + GetChangeStep());
            return;
        }
        else {
            SetValue(GetValue() - GetChangeStep());
            return;
        }
    }
    if (msg.eventType == kEventMouseMove) {
        if (IsMouseFocused()) {
            UiPoint ptMouse(msg.ptMouse);
            ptMouse.Offset(GetScrollOffsetInScrollBox());
            SetValue(ValueFromMousePt(ptMouse));
        }
        return;
    }

    Progress::HandleEvent(msg);
}

void Slider::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("step")) {
        SetChangeStep(StringUtil::StringToInt32(strValue));
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
    else if ((strName == _T("click_to_jump")) || (strName == _T("clicktojump"))) {
        SetClickToJump(strValue == _T("true"));
    }
    else if ((strName == _T("page_step")) || (strName == _T("pagestep"))) {
        SetPageStep(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("show_ticks")) || (strName == _T("showticks"))) {
        SetShowTicks(strValue == _T("true"));
    }
    else if ((strName == _T("tick_count")) || (strName == _T("tickcount"))) {
        SetTickCount(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("tick_length")) || (strName == _T("ticklength"))) {
        SetTickLength(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("tick_color")) || (strName == _T("tickcolor"))) {
        SetTickColor(strValue);
    }
    else if ((strName == _T("tick_pos")) || (strName == _T("tickpos"))) {
        if (strValue == _T("bottom")) {
            SetTickPos(SliderTickPos::kBottom);
        }
        else if (strValue == _T("top")) {
            SetTickPos(SliderTickPos::kTop);
        }
        else if (strValue == _T("right")) {
            SetTickPos(SliderTickPos::kRight);
        }
        else if (strValue == _T("left")) {
            SetTickPos(SliderTickPos::kLeft);
        }
    }
    else if ((strName == _T("show_tick_labels")) || (strName == _T("showticklabels"))) {
        SetShowTickLabels(strValue == _T("true"));
    }
    else if ((strName == _T("tick_label_interval")) || (strName == _T("ticklabelinterval"))) {
        SetTickLabelInterval(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("tick_font")) || (strName == _T("tickfont"))) {
        SetTickFontId(strValue);
    }
    else if ((strName == _T("tick_label_color")) || (strName == _T("ticklabelcolor"))) {
        SetTickLabelColor(strValue);
    }
    else {
        Progress::SetAttribute(strName, strValue);
    }
}

void Slider::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiSize szThumbSize = GetThumbSize();
    szThumbSize = Dpi().GetScaleSize(szThumbSize, nOldDpiScale);
    SetThumbSize(szThumbSize, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void Slider::PaintBkColor(IRender* pRender)
{
    const UiRect rcOldPaintRect = GetPaintRect();
    const UiRect rcOldRect = GetRect();

    UiRect rc = GetRect();
    const UiPadding& padding = m_rcProgressBarPadding;
    rc.Deflate(padding);
    SetRect(rc);

    UiRect paintRect = GetPaintRect();
    paintRect.Deflate(padding);
    SetPaintRect(paintRect);

    Control::PaintBkColor(pRender);

    paintRect = GetPaintRect();
    paintRect.Inflate(padding);
    SetPaintRect(paintRect);

    //恢复原值
    SetRect(rcOldRect);
    SetPaintRect(rcOldPaintRect);
}

void Slider::PaintStateImages(IRender* pRender)
{
    const UiRect rcOldPaintRect = GetPaintRect();
    const UiRect rcOldRect = GetRect();

    UiRect rc = GetRect();
    const UiPadding& padding = m_rcProgressBarPadding;
    rc.Deflate(padding);
    SetRect(rc);

    Progress::PaintStateImages(pRender);

    //恢复原值
    SetRect(rcOldRect);
    SetPaintRect(rcOldPaintRect);

    UiRect rcThumb = GetThumbRect();
    rcThumb.left -= GetRect().left;
    rcThumb.top -= GetRect().top;
    rcThumb.right -= GetRect().left;
    rcThumb.bottom -= GetRect().top;

    if (IsMouseFocused()) {
        m_sImageModify.clear();
        m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStatePressed), m_sImageModify.c_str())) {
            return;
        }
    }
    else if (GetState() == kControlStateHovered) {
        m_sImageModify.clear();
        m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateHovered), m_sImageModify.c_str())) {
            return;
        }
    }

    m_sImageModify.clear();
    m_sImageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
    PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateNormal), m_sImageModify.c_str());

    //刻度线与刻度值文本（轨道之外，不影响滑块与进度条本体绘制）
    PaintTicks(pRender);
}

void Slider::PaintTicks(IRender* pRender)
{
    if ((pRender == nullptr) || !m_bShowTicks || (m_nTickCount < 2)) {
        return;
    }

    //轨道绘制区域为去掉progress_bar_padding 后的区域，刻度与之对齐
    UiRect rcTrack = GetRect();
    rcTrack.Deflate(m_rcProgressBarPadding);
    if (rcTrack.IsEmpty()) {
        return;
    }

    const UiSize szThumb = GetThumbSize();
    const bool bHorizontal = IsHorizontal();
    //轨道中心线（水平：垂直居中；垂直：水平居中）
    int32_t nTrackLength = 0;
    int32_t nThumbStart = 0;      //滑块可移动区间的起点
    int32_t nThumbSpan = 0;       //滑块可移动区间的长度
    if (bHorizontal) {
        nThumbStart = rcTrack.left + szThumb.cx / 2;
        nThumbSpan = rcTrack.right - rcTrack.left - szThumb.cx;
        nTrackLength = rcTrack.Width();
    }
    else {
        nThumbStart = rcTrack.bottom - szThumb.cy / 2;
        nThumbSpan = rcTrack.bottom - rcTrack.top - szThumb.cy;
        nTrackLength = rcTrack.Height();
    }
    if ((nThumbSpan <= 0) || (nTrackLength <= 0)) {
        return;
    }

    //刻度线颜色：默认 text_muted（次要文本色，弱化不抢主体）
    UiColor tickColor = GetUiColor(m_strTickColor.empty() ? _T("text_muted") : m_strTickColor);
    if (tickColor.IsEmpty()) {
        tickColor = GetUiColor(_T("color_gray"));
    }
    if (tickColor.IsEmpty()) {
        tickColor = UiColor(UiColors::Gray);
    }
    if (!IsEnabled()) {
        //禁用态：刻度线弱化
        tickColor = UiColor(tickColor.GetRed(), tickColor.GetGreen(), tickColor.GetBlue(), tickColor.GetAlpha() / 2);
    }

    const int32_t nTickLen = (m_nTickLength > 0) ? Dpi().GetScaleInt(m_nTickLength) : 0;
    if (nTickLen == 0) {
        return;
    }
    const int32_t nGap = Dpi().GetScaleInt(2);
    const int32_t nLabelHeight = Dpi().GetScaleInt(14);
    const bool bShowLabels = m_bShowTickLabels && !m_strTickFontId.empty();
    const int32_t nCount = m_nTickCount;

    const UiRect& rcControl = GetRect();

    //相邻刻度的中心间距：标签半宽不得超过间距的一半，否则相邻标签会重叠
    const int32_t nTickStep = (nCount > 1) ? (nThumbSpan / (nCount - 1)) : nThumbSpan;
    int32_t nLabelHalfSpan = Dpi().GetScaleInt(28);   //标签半宽（水平滑块）
    int32_t nLabelLead = nLabelHalfSpan;              //标签区宽（垂直滑块）
    if (nTickStep > 0) {
        //按实际间距收窄：留2px 间隙，且不超出一个刻度间距的一半
        nLabelHalfSpan = std::max(Dpi().GetScaleInt(12), std::min(nLabelHalfSpan, nTickStep / 2 - Dpi().GetScaleInt(2)));
        nLabelLead = nLabelHalfSpan * 2;
    }
    //垂直滑块：标签区宽也不能超出控件宽度
    const int32_t nMaxLead = std::max(nLabelHeight, rcTrack.Width() - Dpi().GetScaleInt(8));
    if (nLabelLead > nMaxLead) {
        nLabelLead = nMaxLead;
    }

    //主刻度（带标签的）稍长，次刻度略短，形成视觉层级
    const int32_t nMainTickLen = nTickLen;
    const int32_t nSubTickLen = std::max(Dpi().GetScaleInt(2), nTickLen / 2);

    for (int32_t i = 0; i < nCount; ++i) {
        //刻度位置按滑块可移动区间计算，保证与滑块中心对齐
        const int32_t nPos = nThumbStart + (int32_t)(double(nThumbSpan) * (double(i) / double(nCount - 1)) + 0.5);
        const bool bLabel = bShowLabels && ((i % m_nTickLabelInterval) == 0);
        const int32_t nThisTickLen = bLabel ? nMainTickLen : nSubTickLen;
        UiRect rcLabel;
        //首尾标签改为靠边对齐（不改变标签框宽度），避免平移后侵占相邻标签空间
        uint32_t uAlign = TEXT_HCENTER | TEXT_VCENTER;
        if (bHorizontal) {
            const int32_t nThumbTop = (rcTrack.bottom + rcTrack.top - szThumb.cy) / 2;
            const bool bBelow = (m_tickPos == SliderTickPos::kBottom);
            const int32_t nThumbEdge = bBelow ? (nThumbTop + szThumb.cy) : nThumbTop;
            const int32_t nTickStart = bBelow ? (nThumbEdge + nGap) : (nThumbEdge - nGap - nThisTickLen);
            pRender->DrawLine(UiPointF(static_cast<float>(nPos), static_cast<float>(nTickStart)),
                              UiPointF(static_cast<float>(nPos), static_cast<float>(nTickStart + nThisTickLen)),
                              tickColor, 1.0f);
            if (bLabel) {
                //标签纵向：刻度线之外，且完整落在控件内
                int32_t nLabelTop = bBelow ? (nTickStart + nTickLen) : (nTickStart - nLabelHeight);
                nLabelTop = std::clamp(nLabelTop, rcControl.top, std::max(rcControl.top, rcControl.bottom - nLabelHeight));
                const int32_t nLabelH = nLabelHalfSpan * 2;
                if (i == 0) {
                    //首个标签：文字左对齐，框从刻度向右延伸
                    uAlign = TEXT_LEFT | TEXT_VCENTER;
                    rcLabel = UiRect(nPos, nLabelTop, nPos + nLabelH, nLabelTop + nLabelHeight);
                }
                else if (i == nCount - 1) {
                    //末个标签：文字右对齐，框从刻度向左延伸
                    uAlign = TEXT_RIGHT | TEXT_VCENTER;
                    rcLabel = UiRect(nPos - nLabelH, nLabelTop, nPos, nLabelTop + nLabelHeight);
                }
                else {
                    rcLabel = UiRect(nPos - nLabelHalfSpan, nLabelTop, nPos + nLabelHalfSpan, nLabelTop + nLabelHeight);
                }
            }
        }
        else {
            const int32_t nThumbLeft = (rcTrack.right + rcTrack.left - szThumb.cx) / 2;
            const bool bRight = (m_tickPos == SliderTickPos::kRight);
            const int32_t nThumbEdge = bRight ? (nThumbLeft + szThumb.cx) : nThumbLeft;
            const int32_t nTickStart = bRight ? (nThumbEdge + nGap) : (nThumbEdge - nGap - nThisTickLen);
            pRender->DrawLine(UiPointF(static_cast<float>(nTickStart), static_cast<float>(nPos)),
                              UiPointF(static_cast<float>(nTickStart + nThisTickLen), static_cast<float>(nPos)),
                              tickColor, 1.0f);
            if (bLabel) {
                int32_t nLabelLeft = bRight ? (nTickStart + nTickLen) : (nTickStart - nTickLen - nLabelLead);
                nLabelLeft = std::clamp(nLabelLeft, rcControl.left, std::max(rcControl.left, rcControl.right - nLabelLead));
                const int32_t nLabelW = nLabelLead;
                if (i == 0) {
                    //首个标签：文字顶对齐
                    uAlign = TEXT_HCENTER | TEXT_TOP;
                    rcLabel = UiRect(nLabelLeft, nPos, nLabelLeft + nLabelW, nPos + nLabelHeight);
                }
                else if (i == nCount - 1) {
                    //末个标签：文字底对齐
                    uAlign = TEXT_HCENTER | TEXT_BOTTOM;
                    rcLabel = UiRect(nLabelLeft, nPos - nLabelHeight, nLabelLeft + nLabelW, nPos);
                }
                else {
                    rcLabel = UiRect(nLabelLeft, nPos - nLabelHeight / 2, nLabelLeft + nLabelW, nPos + nLabelHeight / 2);
                }
            }
        }
        if (bLabel) {
            DrawTickText(pRender, GetTickValue(i), i, rcLabel, uAlign);
        }
    }
}

void Slider::DrawTickText(IRender* pRender, double fValue, int32_t nIndex, const UiRect& rcLabel, uint32_t uAlign)
{
    if (pRender == nullptr) {
        return;
    }
    DString strText;
    if (m_tickLabelCallback) {
        strText = m_tickLabelCallback(fValue, nIndex);
    }
    if (strText.empty()) {
        const double fFrac = fValue - floor(fValue);
        if (fabs(fFrac) < 0.0001) {
            //整数刻度：避免浮点误差显示成 20.00
            strText = StringUtil::Printf(_T("%I64d"), (int64_t)llround(fValue));
        }
        else {
            strText = StringUtil::Printf(_T("%.2f"), fValue);
        }
    }
    if (strText.empty()) {
        return;
    }

    UiColor textColor = GetUiColor(m_strTickLabelColor.empty() ? _T("text_muted") : m_strTickLabelColor);
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }
    DrawStringParam param;
    param.textRect = rcLabel;
    param.dwTextColor = textColor;
    param.pFont = GetIFontById(m_strTickFontId);
    param.uFormat = TEXT_SINGLELINE | uAlign;
    pRender->DrawString(strText, param);
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
