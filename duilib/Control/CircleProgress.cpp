#include "CircleProgress.h"

namespace ui
{

CircleProgress::CircleProgress(Window* pWindow) :
    Progress(pWindow),
    m_bCircular(true),
    m_bClockwise(true),
    m_nCircleWidth(1),
    m_dwBackgroundColor(0),
    m_dwForegroundColor(0),
    m_dwGradientColor(0),
    m_pIndicatorImage(nullptr)
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

void CircleProgress::SetAttribute(const DString& srName, const DString& strValue)
{
    if (srName == _T("circular")) {
        SetCircular(strValue == _T("true"));
    }
    else if ((srName == _T("circle_width")) || (srName == _T("circlewidth"))) {
        int32_t iValue = StringUtil::StringToInt32(strValue);
        SetCircleWidth(iValue, true);
    }
    else if (srName == _T("indicator")) {
        SetIndicator(strValue);
    }
    else if (srName == _T("clockwise")) {
        SetClockwiseRotation(strValue == _T("true"));
    }
    else if (srName == _T("bgcolor")) {
        SetBackgroudColor(strValue);
    }
    else if (srName == _T("fgcolor")) {
        SetForegroudColor(strValue);
    }
    else if ((srName == _T("gradient_color")) || (srName == _T("gradientcolor"))) {
        SetCircleGradientColor(strValue);
    }
    else {
        Progress::SetAttribute(srName, strValue);
    }
}

void CircleProgress::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    int32_t iValue = GetCircleWidth();
    iValue = Dpi().GetScaleInt(iValue, nOldDpiScale);
    SetCircleWidth(iValue, false);
    __super::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
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

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }
    IPen* bgPen = pRenderFactory->CreatePen(m_dwBackgroundColor, m_nCircleWidth);
    std::unique_ptr<IPen> spBgPen(bgPen);
    ASSERT(bgPen != nullptr);
    if (bgPen == nullptr) {
        return;
    }
    IPen* fgPen = pRenderFactory->CreatePen(m_dwForegroundColor, m_nCircleWidth);
    std::unique_ptr<IPen> spFgPen(fgPen);
    ASSERT(fgPen != nullptr);
    if (fgPen == nullptr) {
        return;
    }

    int direction = m_bClockwise ? 1 : -1;   //旋转方向
    int bordersize = Dpi().GetScaleInt(1); //弧度宽度目前使用1像素

    // 圆形中心
    UiPoint center;
    const UiRect& rect = GetRect();
    center.x = rect.left + (rect.right - rect.left) / 2;
    center.y = rect.top + (rect.bottom - rect.top) / 2;

    // 控件矩形内的最大正方形的边界
    int side = std::min(rect.right - rect.left, rect.bottom - rect.top);

    UiRect rcBorder;
    rcBorder.left = center.x - side / 2;
    rcBorder.top = center.y - side / 2;
    rcBorder.right = rcBorder.left + side;
    rcBorder.bottom = rcBorder.top + side;

    //加载图片

    std::shared_ptr<ImageInfo> pIndicatorImageInfo;
    if (m_pIndicatorImage != nullptr) {
        LoadImageData(*m_pIndicatorImage);
        pIndicatorImageInfo = m_pIndicatorImage->GetImageCache();
        ASSERT(pIndicatorImageInfo != nullptr);
    }    

    UiRect outer = rcBorder;
    if (pIndicatorImageInfo != nullptr) {
        outer.Inflate(-1 * pIndicatorImageInfo->GetWidth() / 2,
                      -1 * pIndicatorImageInfo->GetWidth() / 2);
    }
    else {
        outer.Inflate(-1 * m_nCircleWidth / 2, -1 * m_nCircleWidth / 2);
    }
    int inflateValue = Dpi().GetScaleInt(-1);
    outer.Inflate(inflateValue, inflateValue);

    if (m_dwGradientColor.GetARGB() == 0) {
        //不使用渐变色，直接用前景色铺满
        pRender->DrawArc(outer, 270, 360, false, bgPen);

        float sweepAngle = static_cast<float>(direction * 360 * (m_nValue - m_nMin) / (m_nMax - m_nMin));
        pRender->DrawArc(outer, 270, sweepAngle, false, fgPen);
    }
    else {
        //不使用渐变色，直接用前景色铺满
        pRender->DrawArc(outer, 270, 360, false, bgPen);

        float sweepAngle = static_cast<float>(direction * 360 * (m_nValue - m_nMin) / (m_nMax - m_nMin));
        pRender->DrawArc(outer, 270, sweepAngle, false, fgPen, &m_dwGradientColor, &rcBorder);
    }
    
    if (pIndicatorImageInfo != nullptr) {
        std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
        if ((spMatrix != nullptr) && ((m_nMax - m_nMin) != 0)){
            float angle = direction * 360 * ((float)m_nValue - m_nMin) / (m_nMax - m_nMin);
            spMatrix->RotateAt(angle, UiPoint(center.x, center.y));
        }

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
}

void CircleProgress::ClearImageCache()
{
    __super::ClearImageCache();
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

void CircleProgress::SetCircleWidth(int nCircleWidth, bool bNeedDpiScale)
{
    if (nCircleWidth < 0) {
        nCircleWidth = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(nCircleWidth);
    }
    if (m_nCircleWidth != nCircleWidth) {
        m_nCircleWidth = nCircleWidth;
        Invalidate();
    }    
}

int32_t CircleProgress::GetCircleWidth() const
{
    return m_nCircleWidth;
}

void CircleProgress::SetBackgroudColor(const DString& strColor)
{
    m_dwBackgroundColor = GlobalManager::Instance().Color().GetColor(strColor);
    ASSERT(m_dwBackgroundColor.GetARGB() != 0);
    Invalidate();
}

void CircleProgress::SetForegroudColor(const DString& strColor)
{
    m_dwForegroundColor = GlobalManager::Instance().Color().GetColor(strColor);
    ASSERT(m_dwForegroundColor.GetARGB() != 0);
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
    m_dwGradientColor = GlobalManager::Instance().Color().GetColor(strColor);
    ASSERT(m_dwGradientColor.GetARGB() != 0);
    Invalidate();
}

}
