#include "ControlLoading.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"
namespace ui 
{
ControlLoading::ControlLoading(Control* pControl):
    m_bIsLoading(false),
    m_fCurrrentAngele(0),
    m_pControl(pControl)
{
    ASSERT(pControl != nullptr);
}

ControlLoading::~ControlLoading()
{
}

bool ControlLoading::SetLoadingImage(const DString& strImage)
{
    if (!strImage.empty()) {
        if (m_pLoadingImage == nullptr) {
            m_pLoadingImage = std::make_unique<Image>();
        }
    }
    bool bChanged = false;
    if (m_pLoadingImage != nullptr) {
        if (m_pLoadingImage->GetImageString() != strImage) {
            const DpiManager& dpi = (m_pControl != nullptr) ? m_pControl->Dpi() : GlobalManager::Instance().Dpi();
            m_pLoadingImage->SetImageString(strImage, dpi);
            bChanged = true;
        }        
    }
    return bChanged;
}

bool ControlLoading::SetLoadingBkColor(const DString& strColor)
{
    if (m_strLoadingBkColor == strColor) {
        return false;
    }
    m_strLoadingBkColor = strColor;
    return true;
}

void ControlLoading::PaintLoading(IRender* pRender)
{
    Control* pControl = m_pControl;
    ASSERT((pRender != nullptr) && (pControl != nullptr));
    if ((pRender == nullptr) || (pControl == nullptr)){
        return;
    }
    if (!m_bIsLoading || (m_pLoadingImage == nullptr) || m_pLoadingImage->GetImagePath().empty()) {
        return;
    }

    pControl->LoadImageInfo(*m_pLoadingImage);
    std::shared_ptr<ImageInfo> spImageInfo = m_pLoadingImage->GetImageInfo();
    ASSERT(spImageInfo != nullptr);
    if (!spImageInfo) {
        return;
    }
    int32_t imageWidth = spImageInfo->GetWidth();
    int32_t imageHeight = spImageInfo->GetHeight();

    //居中
    UiRect rect = pControl->GetRect();
    UiRect rcFill = pControl->GetRect();    
    rcFill.left = rect.left + (rect.Width() - imageWidth) / 2;
    rcFill.right = rcFill.left + imageWidth;
    rcFill.top = rect.top + (rect.Height() - imageHeight) / 2;
    rcFill.bottom = rcFill.top + imageHeight;

    ui::UiRect rcDest = m_pLoadingImage->GetImageAttribute().GetImageDestRect(imageWidth, imageHeight, pControl->Dpi());
    if (ImageAttribute::HasValidImageRect(rcDest)) {
        rcFill = rcDest;
        rcFill.Offset(rect.left, rect.top);
    }

    if (!m_strLoadingBkColor.empty()) {
        pRender->FillRect(rcFill, pControl->GetUiColor(m_strLoadingBkColor.c_str()));
    }

    UiRect imageDestRect = rcFill;
    rcFill.Offset(-rect.left, -rect.top);

    //图片旋转矩阵
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return;
    }
    std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
    if (spMatrix != nullptr) {
        spMatrix->RotateAt((float)m_fCurrrentAngele, imageDestRect.Center());
    }

    DString modify = StringUtil::Printf(_T("destscale='false' dest='%d,%d,%d,%d'"), rcFill.left, rcFill.top, rcFill.right, rcFill.bottom);

    //绘制时需要设置裁剪区域，避免绘制超出范围（因为旋转图片后，图片区域会超出显示区域）
    AutoClip autoClip(pRender, imageDestRect, true);
    pControl->PaintImage(pRender, m_pLoadingImage.get(), modify, -1, spMatrix.get());
}

void ControlLoading::Loading()
{
    if (!m_bIsLoading) {
        return;
    }
    m_fCurrrentAngele += 10;
    if (m_fCurrrentAngele == INT32_MIN) {
        m_fCurrrentAngele = 0;
    }
    if (m_pControl != nullptr) {
        m_pControl->Invalidate();
    }    
}

bool ControlLoading::StartLoading(int32_t fStartAngle)
{
    if (fStartAngle >= 0) {
        m_fCurrrentAngele = fStartAngle;
    }
    if (m_bIsLoading) {
        return false;
    }
    m_bIsLoading = true;
    GlobalManager::Instance().Timer().AddTimer(m_loadingImageFlag.GetWeakFlag(), 
                                                         UiBind(&ControlLoading::Loading, this),
                                                         50);
    return true;
}

void ControlLoading::StopLoading(AnimationImagePos frame)
{
    if (!m_bIsLoading) {
        return;
    }

    switch (frame) {
    case AnimationImagePos::kFrameFirst:
        m_fCurrrentAngele = 0;
        break;
    case AnimationImagePos::kFrameCurrent:
        break;
    case AnimationImagePos::kFrameLast:
        m_fCurrrentAngele = 360;
        break;
    default:
        break;
    }
    m_bIsLoading = false;
    m_loadingImageFlag.Cancel();
}

bool ControlLoading::IsLoading() const
{
    return m_bIsLoading;
}

}
