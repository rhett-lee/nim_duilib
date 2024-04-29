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
}

ControlLoading::~ControlLoading()
{
}

bool ControlLoading::SetLoadingImage(const std::wstring& strImage)
{
    if (!strImage.empty()) {
        if (m_pLoadingImage == nullptr) {
            m_pLoadingImage = std::make_unique<Image>();
        }
    }
    bool bChanged = false;
    if (m_pLoadingImage != nullptr) {
        if (m_pLoadingImage->GetImageString() != strImage) {
            m_pLoadingImage->SetImageString(strImage);
            bChanged = true;
        }        
    }
    return bChanged;
}

bool ControlLoading::SetLoadingBkColor(const std::wstring& strColor)
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

	pControl->LoadImageData(*m_pLoadingImage);
	std::shared_ptr<ImageInfo> spImageInfo = m_pLoadingImage->GetImageCache();
	ASSERT(spImageInfo != nullptr);
	if (!spImageInfo) {
		return;
	}

	IBitmap* pBitmap = spImageInfo->GetBitmap(0);
	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return;
	}
	int32_t imageWidth = pBitmap->GetWidth();
	int32_t imageHeight = pBitmap->GetHeight();

	//居中
	UiRect rect = pControl->GetRect();
	UiRect rcFill = pControl->GetRect();	
	rcFill.left = rect.left + (rect.Width() - imageWidth) / 2;
	rcFill.right = rcFill.left + imageWidth;
	rcFill.top = rect.top + (rect.Height() - imageHeight) / 2;
	rcFill.bottom = rcFill.top + imageHeight;

	ui::UiRect rcDest = m_pLoadingImage->GetImageAttribute().GetDestRect();
	if (!rcDest.IsEmpty()) {
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

	wchar_t modify[64] = { 0 };
	swprintf_s(modify, L"destscale='false' dest='%d,%d,%d,%d'", rcFill.left, rcFill.top, rcFill.right, rcFill.bottom);

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
	GlobalManager::Instance().Timer().AddCancelableTimer(m_loadingImageFlag.GetWeakFlag(), 
														 nbase::Bind(&ControlLoading::Loading, this),
													     50, 
		                                                 TimerManager::REPEAT_FOREVER);
	return true;
}

void ControlLoading::StopLoading(GifFrameType frame)
{
	if (!m_bIsLoading) {
		return;
	}

	switch (frame) {
	case kGifFrameFirst:
		m_fCurrrentAngele = 0;
		break;
	case kGifFrameCurrent:
		break;
	case  kGifFrameLast:
		m_fCurrrentAngele = 360;
	}
	m_bIsLoading = false;
	m_loadingImageFlag.Cancel();
}

bool ControlLoading::IsLoading() const
{
	return m_bIsLoading;
}

}