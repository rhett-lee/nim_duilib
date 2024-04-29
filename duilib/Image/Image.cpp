#include "Image.h"
#include "duilib/Image/ImageGif.h"

namespace ui 
{
Image::Image() :
	m_pControl(nullptr),
	m_pImageGif(nullptr),
	m_nCurrentFrame(0)
{
}

void Image::InitImageAttribute()
{
	m_imageAttribute.Init();
}

void Image::SetImageString(const std::wstring& strImageString)
{
	ClearImageCache();
	m_imageAttribute.InitByImageString(strImageString);
}

std::wstring Image::GetImageString() const
{
	return m_imageAttribute.sImageString.c_str();
}

bool Image::EqualToImageString(const std::wstring& imageString) const
{
	return m_imageAttribute.sImageString == imageString;
}

std::wstring Image::GetImagePath() const
{
	return m_imageAttribute.sImagePath.c_str();
}

void Image::SetImagePadding(const UiPadding& newPadding)
{
	m_imageAttribute.SetPadding(newPadding);
}

UiPadding Image::GetImagePadding() const
{
	return m_imageAttribute.GetPadding();
}

bool Image::IsImagePaintEnabled() const
{
	return m_imageAttribute.bPaintEnabled;
}

void Image::SetImagePaintEnabled(bool bEnable)
{
	m_imageAttribute.bPaintEnabled = bEnable;
}

void Image::SetImagePlayCount(int32_t nPlayCount)
{
	m_imageAttribute.nPlayCount = nPlayCount;
}

void Image::SetImageFade(uint8_t nFade)
{
	m_imageAttribute.bFade = nFade;
}

const ImageAttribute& Image::GetImageAttribute() const
{
	return m_imageAttribute;
}

ImageLoadAttribute Image::GetImageLoadAttribute() const
{
	return ImageLoadAttribute(m_imageAttribute.srcWidth.c_str(),
							  m_imageAttribute.srcHeight.c_str(),
							  m_imageAttribute.srcDpiScale,
							  m_imageAttribute.bHasSrcDpiScale,
							  m_imageAttribute.iconSize);
}

const std::shared_ptr<ImageInfo>& Image::GetImageCache() const
{
	return m_imageCache;
}

void Image::SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo)
{
	m_imageCache = imageInfo;
}

void Image::ClearImageCache()
{
	m_nCurrentFrame = 0;
	m_imageCache.reset();
}

void Image::SetCurrentFrame(uint32_t nCurrentFrame)
{
	m_nCurrentFrame = nCurrentFrame;
}

uint32_t Image::GetCurrentFrame() const
{
	return m_nCurrentFrame;
}

uint32_t Image::GetFrameCount() const
{
	if (!m_imageCache) {
		return 0;
	}
	return m_imageCache->GetFrameCount();
}

bool Image::IsMultiFrameImage() const
{
	if (!m_imageCache) {
		return false;
	}
	return m_imageCache->IsMultiFrameImage();
}

IBitmap* Image::GetCurrentBitmap() const
{
	if (!m_imageCache) {
		return nullptr;
	}
	if (m_nCurrentFrame < m_imageCache->GetFrameCount()) {
		return m_imageCache->GetBitmap(m_nCurrentFrame);
	}
	else {
		uint32_t nCurrentFrame = 0;
		if (m_imageCache->GetFrameCount() > 0) {
			nCurrentFrame = m_nCurrentFrame % m_imageCache->GetFrameCount();
		}
		return m_imageCache->GetBitmap(nCurrentFrame);
	}	
}

void Image::SetControl(Control* pControl)
{
	if (m_pControl != pControl) {
		m_pControl = pControl;
		if (m_pImageGif != nullptr) {
			m_pImageGif->SetControl(pControl);
		}
	}
}

bool Image::CheckStartGifPlay(const UiRect& rcImageRect)
{
	if (!IsMultiFrameImage() || (m_pControl == nullptr)) {
		return false;
	}
	if (m_pImageGif == nullptr) {
		m_pImageGif = new ImageGif;
		m_pImageGif->SetImage(this);
		m_pImageGif->SetControl(m_pControl);
	}
	m_pImageGif->SetImageRect(rcImageRect);
	if (m_pImageGif->IsPlayingGif()) {
		return true;
	}
	return m_pImageGif->StartGifPlay();
}

void Image::CheckStopGifPlay()
{
	if (m_pImageGif != nullptr) {
		m_pImageGif->StopGifPlay();		
	}
}

bool Image::StartGifPlay(GifFrameType nStartFrame, int32_t nPlayCount)
{
	if (!IsMultiFrameImage() || (m_pControl == nullptr)) {
		return false;
	}
	if (m_pImageGif == nullptr) {
		m_pImageGif = new ImageGif;
		m_pImageGif->SetImage(this);
		m_pImageGif->SetControl(m_pControl);
	}
	if (m_pImageGif->IsPlayingGif()) {
		m_pImageGif->StopGifPlay();
	}
	m_pImageGif->SetImageRect(UiRect());
	return m_pImageGif->StartGifPlay(nStartFrame, nPlayCount);
}

void Image::StopGifPlay(bool bTriggerEvent, GifFrameType nStopFrame)
{
	if (m_pImageGif != nullptr) {
		m_pImageGif->StopGifPlay(bTriggerEvent, nStopFrame);
	}
}

void Image::AttachGifPlayStop(const EventCallback& callback)
{
	if (m_pImageGif == nullptr) {
		m_pImageGif = new ImageGif;
		m_pImageGif->SetImage(this);
		m_pImageGif->SetControl(m_pControl);
	}
	m_pImageGif->AttachGifPlayStop(callback);
}

}