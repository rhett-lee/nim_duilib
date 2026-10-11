#include "CircleProgress.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"
#include <algorithm>

namespace ui
{

CircleProgress::CircleProgress(Window* pWindow) :
    Progress(pWindow),
    m_bCircular(true),
    m_bClockwise(true),
    m_fCircleWidth(1.0f),
    m_dwBackgroundColor(0),
    m_dwForegroundColor(0),
    m_dwGradientColor(0),
    m_pIndicatorImage(nullptr),
    m_bIndeterminate(false),
    m_fIndeterminateAngle(0.0f),
    m_fStartAngle(270.0f),
    m_fSpanAngle(360.0f),
    m_fIndeterminateArc(90.0f),
    m_fIndeterminateSpeed(6.0f),
    m_nIndeterminateInterval(16),
    m_fCenterRatio(1.0f),
    m_rcCenterPadding(),
    m_centerCallback()
{
}

CircleProgress::~CircleProgress()
{
    if (m_pIndicatorImage != nullptr) {
        delete m_pIndicatorImage;
        m_pIndicatorImage = nullptr;
    }
}

DString CircleProgress::GetType() const { return DUI_CTR_CIRCLEPROGRESS; }

void CircleProgress::SetAttribute(const DString& srName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (srName == _T("circular")) {
        SetCircular(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("circle_width")) || (srName == _T("circlewidth"))) {
        int32_t iValue = StringUtil::StringToInt32(strValue);
        SetCircleWidth((float)iValue, true);
    }
    else if (srName == _T("indicator")) {
        SetIndicator(strValue);
    }
    else if (srName == _T("clockwise")) {
        SetClockwiseRotation(StringUtil::IsValueTrue(strValue));
    }
    else if (srName == _T("bgcolor")) {
        SetBackgroundColor(strValue);
    }
    else if (srName == _T("fgcolor")) {
        SetForegroundColor(strValue);
    }
    else if ((srName == _T("gradient_color")) || (srName == _T("gradientcolor"))) {
        SetCircleGradientColor(strValue);
    }
    else if ((srName == _T("indeterminate"))) {
        SetIndeterminate(StringUtil::IsValueTrue(strValue));
    }
    else if ((srName == _T("start_angle")) || (srName == _T("startangle"))) {
        SetStartAngle((float)StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("span_angle")) || (srName == _T("spanangle"))) {
        SetSpanAngle((float)StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("indeterminate_arc")) || (srName == _T("indeterminatearc"))) {
        SetIndeterminateArc((float)StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("indeterminate_speed")) || (srName == _T("indeterminatespeed"))) {
        SetIndeterminateSpeed((float)StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("indeterminate_interval")) || (srName == _T("indeterminateinterval"))) {
        SetIndeterminateInterval(StringUtil::StringToInt32(strValue));
    }
    else if ((srName == _T("center_ratio")) || (srName == _T("centerratio"))) {
        SetCenterRatio((float)StringUtil::StringToInt32(strValue) / 100.0f);
    }
    else if ((srName == _T("center_padding")) || (srName == _T("centerpadding"))) {
        UiPadding padding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), padding);
        SetCenterPadding(padding, true);
    }
    else {
        Progress::SetAttribute(srName, strValue);
    }
}

void CircleProgress::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    float fValue = GetCircleWidth();
    fValue = Dpi().GetScaleFloat(fValue, nOldDpiScale);
    SetCircleWidth(fValue, false);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void CircleProgress::PaintStateImages(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    Progress::PaintStateImages(pRender);
    if (!m_bCircular) {
        return;
    }

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
    //进度比例：先归一化再按 reverse 取反（直接做 (nMax-nMin)-fValue 的绝对值翻转
    //在 min != 0 时会算出越界比例）
    double fRatio = (fValue - nMin) / static_cast<double>(nMax - nMin);
    fRatio = std::clamp(fRatio, 0.0, 1.0);
    if (IsReverse()) {
        //逆向：value=min 时为满格，value=max 时为空
        fRatio = 1.0 - fRatio;
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }
    IPen* bgPen = pRenderFactory->CreatePen(m_dwBackgroundColor, m_fCircleWidth);
    std::unique_ptr<IPen> spBgPen(bgPen);
    ASSERT(bgPen != nullptr);
    if (bgPen == nullptr) {
        return;
    }
    IPen* fgPen = pRenderFactory->CreatePen(m_dwForegroundColor, m_fCircleWidth);
    std::unique_ptr<IPen> spFgPen(fgPen);
    ASSERT(fgPen != nullptr);
    if (fgPen == nullptr) {
        return;
    }

    int direction = m_bClockwise ? 1 : -1;   //旋转方向
    int bordersize = Dpi().GetScaleInt(1); //弧度宽度目前使用1像素

    //弧的起始角与扫过角：indeterminate 时为持续旋转的固定弧，否则按进度值
    const float fStartAngle = m_fStartAngle;
    float fSweepAngle;
    if (m_bIndeterminate) {
        fSweepAngle = m_fIndeterminateArc;
    }
    else {
        fSweepAngle = static_cast<float>(direction * m_fSpanAngle * fRatio);
    }

    //圆形中心：整圆时取控件中心；非整圆（仪表盘/缺口环）时按弧的包围盒居中，
    //使半圆表盘的上半部分完整落在控件内
    const UiRect& rect = GetRect();
    UiPoint center;
    int side;
    if (m_fSpanAngle >= 359.9f) {
        //整圆：控件内最大正方形居中
        center.x = rect.left + (rect.right - rect.left) / 2;
        center.y = rect.top + (rect.bottom - rect.top) / 2;
        side = std::min(rect.right - rect.left, rect.bottom - rect.top);
    }
    else {
        //非整圆：以弧的实际包围盒为准。采样起止角与途经的象限极值点求min/max
        const float fPi = 3.14159265f;
        float fMinX = 1.0f, fMaxX = -1.0f, fMinY = 1.0f, fMaxY = -1.0f;
        auto AccPoint = [&](float fDeg) {
            const float fRad = fDeg * fPi / 180.0f;
            const float fX = std::cos(fRad);
            const float fY = std::sin(fRad);
            fMinX = std::min(fMinX, fX); fMaxX = std::max(fMaxX, fX);
            fMinY = std::min(fMinY, fY); fMaxY = std::max(fMaxY, fY);
            };
        //起止角
        AccPoint(fStartAngle);
        AccPoint(fStartAngle + m_fSpanAngle);
        //途经的象限极值点（0/90/180/270），否则半圆、缺口弧的包围盒会被算小
        const float fStep = m_bClockwise ? 1.0f : -1.0f;
        for (int i = 0; i <= 4; ++i) {
            const float fQ = std::fmod(fStartAngle + fStep * 90.0f * i, 360.0f);
            if (fQ < 0.0f) {
                AccPoint(fQ + 360.0f);
            }
            else {
                AccPoint(fQ);
            }
        }

        const float fArcW = fMaxX - fMinX;
        const float fArcH = fMaxY - fMinY;
        const float fAvailW = (rect.right - rect.left) / 2.0f;
        const float fAvailH = (rect.bottom - rect.top) / 2.0f;
        //半径取各方向所需半径的最大值，保证弧完整落在控件内
        float fRadius = 0.0f;
        if (fArcW > 0.001f) {
            fRadius = std::max(fRadius, fAvailW / fArcW);
        }
        if (fArcH > 0.001f) {
            fRadius = std::max(fRadius, fAvailH / fArcH);
        }
        if (fRadius <= 0.0f) {
            fRadius = std::min(fAvailW, fAvailH);
        }
        //圆心落在弧包围盒的中心
        center.x = rect.left + (rect.right - rect.left) / 2 -
            static_cast<int>((fMinX + fMaxX) / 2.0f * fRadius);
        center.y = rect.top + (rect.bottom - rect.top) / 2 -
            static_cast<int>((fMinY + fMaxY) / 2.0f * fRadius);
        side = static_cast<int>(fRadius * 2.0f);
    }

    UiRect rcBorder;
    rcBorder.left = center.x - side / 2;
    rcBorder.top = center.y - side / 2;
    rcBorder.right = rcBorder.left + side;
    rcBorder.bottom = rcBorder.top + side;

    //加载图片

    std::shared_ptr<ImageInfo> pIndicatorImageInfo;
    if (m_pIndicatorImage != nullptr) {
        LoadImageInfo(*m_pIndicatorImage);
        pIndicatorImageInfo = m_pIndicatorImage->GetImageInfo();
        ASSERT(pIndicatorImageInfo != nullptr);
    }

    UiRect outer = rcBorder;
    //环的绘制半径由环宽决定；指示点不参与收缩，避免更换指示点尺寸时环跟着变形
    outer.Inflate(-1 * (int32_t)(m_fCircleWidth / 2), -1 * (int32_t)(m_fCircleWidth / 2));
    int inflateValue = Dpi().GetScaleInt(-1);
    outer.Inflate(inflateValue, inflateValue);

    //背景弧：起点与跨度均可配置（span<360 即为缺口环/仪表盘）
    pRender->DrawArc(outer, fStartAngle, m_fSpanAngle, false, bgPen);

    //前景弧：indeterminate 时为旋转的定长弧，否则按进度值扫过 span
    if (m_bIndeterminate) {
        //旋转弧：起点随旋转角前进，弧长可配置
        const float fRotateStart = m_fStartAngle + m_fIndeterminateAngle;
        if (m_dwGradientColor.GetARGB() == 0) {
            pRender->DrawArc(outer, fRotateStart, m_fIndeterminateArc, false, fgPen);
        }
        else {
            pRender->DrawArc(outer, fRotateStart, m_fIndeterminateArc, false, fgPen, &m_dwGradientColor, &rcBorder);
        }
    }
    else if (m_dwGradientColor.GetARGB() == 0) {
        //不使用渐变色，直接用前景色铺满
        pRender->DrawArc(outer, fStartAngle, fSweepAngle, false, fgPen);
    }
    else {
        //使用渐变色
        pRender->DrawArc(outer, fStartAngle, fSweepAngle, false, fgPen, &m_dwGradientColor, &rcBorder);
    }

    if (pIndicatorImageInfo != nullptr) {
        std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
        //指示点跟随进度末端：与弧的起始角+扫过角一致，而不是固定的 12 点方向
        const float fAngle = m_bIndeterminate ? (m_fStartAngle + m_fIndeterminateAngle + m_fIndeterminateArc)
            : (fStartAngle + fSweepAngle);
        if (spMatrix != nullptr) {
            const UiPoint ptScrollOffset = GetScrollOffsetInScrollBox();
            spMatrix->RotateAt(fAngle, (float)center.x - ptScrollOffset.x, (float)center.y - ptScrollOffset.y);
        }

        //先按 12 点方向摆位（弧的起始方向），再由上面的矩阵旋转到末端角度
        UiRect imageRect;
        imageRect.left = center.x - pIndicatorImageInfo->GetWidth() / 2;
        imageRect.top = outer.top + bordersize / 2 - pIndicatorImageInfo->GetHeight() / 2;
        imageRect.right = imageRect.left + pIndicatorImageInfo->GetWidth();
        imageRect.bottom = imageRect.top + pIndicatorImageInfo->GetHeight();
        imageRect.Offset(-GetRect().left, -GetRect().top);
        DString imageModify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"),
            imageRect.left, imageRect.top, imageRect.right, imageRect.bottom);
        PaintImage(pRender, m_pIndicatorImage, imageModify, -1, spMatrix.get());
    }

    //中心内容区：仪表盘场景常需自定义绘制（指针、图标、数字），默认绘制文本
    PaintCenterContent(pRender, fValue);
}

UiRect CircleProgress::GetCenterRect() const
{
    const UiRect& rect = GetRect();
    int nWidth = rect.right - rect.left;
    int nHeight = rect.bottom - rect.top;
    //按比例收缩
    const float fRatio = (m_fCenterRatio > 0.0f && m_fCenterRatio <= 1.0f) ? m_fCenterRatio : 1.0f;
    nWidth = static_cast<int>(nWidth * fRatio);
    nHeight = static_cast<int>(nHeight * fRatio);
    UiRect rcCenter((rect.right - rect.left) - nWidth, (rect.bottom - rect.top) - nHeight,
                    (rect.right - rect.left), (rect.bottom - rect.top));
    //应用内边距
    rcCenter.left += m_rcCenterPadding.left;
    rcCenter.top += m_rcCenterPadding.top;
    rcCenter.right -= m_rcCenterPadding.right;
    rcCenter.bottom -= m_rcCenterPadding.bottom;
    if (rcCenter.IsEmpty()) {
        rcCenter = rect;
    }
    return rcCenter;
}

void CircleProgress::PaintCenterContent(IRender* pRender, double fValue)
{
    if ((pRender == nullptr) || !m_centerCallback) {
        //无自定义回调时，中心文本由基类绘制流程处理（text_padding / show_percent）
        return;
    }
    m_centerCallback(pRender, GetCenterRect(), fValue);
}

void CircleProgress::ClearImageCache()
{
    BaseClass::ClearImageCache();
    if (m_pIndicatorImage != nullptr) {
        m_pIndicatorImage->ClearImageCache();
    }    
}

void CircleProgress::SetCircular(bool bCircular /*= true*/)
{
    m_bCircular = bCircular;
    Invalidate();
}

void CircleProgress::SetClockwiseRotation(bool bClockwise /*= true*/)
{
    m_bClockwise = bClockwise;
}

void CircleProgress::SetCircleWidth(float fCircleWidth, bool bNeedDpiScale)
{
    if (fCircleWidth < 0) {
        fCircleWidth = 0.0f;
    }
    if (bNeedDpiScale) {
        fCircleWidth = Dpi().GetScaleFloat(fCircleWidth);
    }
    if (m_fCircleWidth != fCircleWidth) {
        m_fCircleWidth = fCircleWidth;
        Invalidate();
    }    
}

float CircleProgress::GetCircleWidth() const
{
    return m_fCircleWidth;
}

void CircleProgress::SetBackgroundColor(const DString& strColor)
{
    m_dwBackgroundColor = GetUiColor(strColor);
    Invalidate();
}

void CircleProgress::SetForegroundColor(const DString& strColor)
{
    m_dwForegroundColor = GetUiColor(strColor);
    Invalidate();
}

void CircleProgress::SetIndicator(const DString& sIndicatorImage)
{
    if (m_pIndicatorImage == nullptr) {
        m_pIndicatorImage = new Image;
    }
    if (m_pIndicatorImage->GetImageString() != sIndicatorImage) {
        m_pIndicatorImage->ClearImageCache();
        m_pIndicatorImage->SetImageString(sIndicatorImage, Dpi());
        Invalidate();    
    }
}

void CircleProgress::SetCircleGradientColor(const DString& strColor)
{
    m_dwGradientColor = GetUiColor(strColor);
    Invalidate();
}

void CircleProgress::SetIndeterminate(bool bIndeterminate)
{
    if (m_bIndeterminate == bIndeterminate) {
        return;
    }
    m_bIndeterminate = bIndeterminate;
    m_fIndeterminateAngle = 0.0f;
    if (m_bIndeterminate) {
        auto timerCallback = UiBind(&CircleProgress::OnIndeterminateTimer, this);
        GlobalManager::Instance().Timer().AddTimer(m_indeterminateTimer.GetWeakFlag(),
            timerCallback, (uint32_t)m_nIndeterminateInterval);
    }
    else {
        m_indeterminateTimer.Cancel();
    }
    Invalidate();
}

void CircleProgress::OnIndeterminateTimer()
{
    if (!m_bIndeterminate || !IsVisible()) {
        return;
    }
    //按配置的转速旋转（默认每 16ms 转 6°，约 0.96s 一圈）
    m_fIndeterminateAngle += m_fIndeterminateSpeed;
    if (m_fIndeterminateAngle >= 360.0f) {
        m_fIndeterminateAngle -= 360.0f;
    }
    Invalidate();
}

void CircleProgress::SetStartAngle(float fAngle)
{
    //归一化到 [0, 360)
    fAngle = std::fmod(fAngle, 360.0f);
    if (fAngle < 0.0f) {
        fAngle += 360.0f;
    }
    if (m_fStartAngle != fAngle) {
        m_fStartAngle = fAngle;
        Invalidate();
    }
}

void CircleProgress::SetSpanAngle(float fSpan)
{
    //跨度限制在 (0, 360]
    if (fSpan <= 0.0f || fSpan > 360.0f) {
        fSpan = 360.0f;
    }
    if (m_fSpanAngle != fSpan) {
        m_fSpanAngle = fSpan;
        Invalidate();
    }
}

void CircleProgress::SetIndeterminateArc(float fArc)
{
    if (fArc <= 0.0f || fArc > 360.0f) {
        fArc = 360.0f;
    }
    if (m_fIndeterminateArc != fArc) {
        m_fIndeterminateArc = fArc;
        Invalidate();
    }
}

void CircleProgress::SetIndeterminateSpeed(float fSpeed)
{
    if (fSpeed <= 0.0f) {
        fSpeed = 0.1f;
    }
    if (fSpeed > 360.0f) {
        fSpeed = 360.0f;
    }
    if (m_fIndeterminateSpeed != fSpeed) {
        m_fIndeterminateSpeed = fSpeed;
        //转速变了但正在转，需要用新间隔重启定时器
        if (m_bIndeterminate) {
            m_indeterminateTimer.Cancel();
            m_fIndeterminateAngle = 0.0f;
            auto timerCallback = UiBind(&CircleProgress::OnIndeterminateTimer, this);
            GlobalManager::Instance().Timer().AddTimer(m_indeterminateTimer.GetWeakFlag(),
                timerCallback, (uint32_t)m_nIndeterminateInterval);
        }
    }
}

void CircleProgress::SetIndeterminateInterval(int32_t nInterval)
{
    if (nInterval < 1) {
        nInterval = 1;
    }
    if (m_nIndeterminateInterval != nInterval) {
        m_nIndeterminateInterval = nInterval;
        if (m_bIndeterminate) {
            m_indeterminateTimer.Cancel();
            m_fIndeterminateAngle = 0.0f;
            auto timerCallback = UiBind(&CircleProgress::OnIndeterminateTimer, this);
            GlobalManager::Instance().Timer().AddTimer(m_indeterminateTimer.GetWeakFlag(),
                timerCallback, (uint32_t)nInterval);
        }
    }
}

void CircleProgress::SetCenterRatio(float fRatio)
{
    if (fRatio <= 0.0f || fRatio > 1.0f) {
        fRatio = 1.0f;
    }
    if (m_fCenterRatio != fRatio) {
        m_fCenterRatio = fRatio;
        Invalidate();
    }
}

void CircleProgress::SetCenterPadding(const UiPadding& padding, bool bNeedDpiScale)
{
    UiPadding newPadding = padding;
    if (bNeedDpiScale) {
        Dpi().ScalePadding(newPadding);
    }
    if (!m_rcCenterPadding.Equals(newPadding)) {
        m_rcCenterPadding = newPadding;
        Invalidate();
    }
}

void CircleProgress::SetCenterCallback(const CircleCenterCallback& callback)
{
    m_centerCallback = callback;
    Invalidate();
}

}
