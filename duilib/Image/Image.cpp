#include "Image.h"

namespace ui 
{
Image::Image() :
	m_nCurrentFrame(0),
	m_bPlaying(false),
	m_nCycledCount(0)
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

void Image::ClearImageCache()
{
	m_nCurrentFrame = 0;
	m_bPlaying = false;
	m_imageCache.reset();
	m_nCycledCount = 0;
}

void Image::SetImagePlayCount(int32_t nPlayCount)
{
	m_imageAttribute.nPlayCount = nPlayCount;
}

void Image::SetImageFade(uint8_t nFade)
{
	m_imageAttribute.bFade = nFade;
}

bool Image::IncrementCurrentFrame()
{
	if (!m_imageCache) {
		return false;
	}
	m_nCurrentFrame++;
	if (m_nCurrentFrame == m_imageCache->GetFrameCount()) {
		m_nCurrentFrame = 0;
		m_nCycledCount += 1;
	}
	return true;
}

void Image::SetCurrentFrame(uint32_t nCurrentFrame)
{
	m_nCurrentFrame = nCurrentFrame;
}

IBitmap* Image::GetCurrentBitmap() const
{
	if (!m_imageCache) {
		return nullptr;
	}
	return m_imageCache->GetBitmap(m_nCurrentFrame);
}

int32_t Image::GetCurrentInterval() const
{
	if (!m_imageCache) {
		return 0;
	}
	return m_imageCache->GetFrameInterval(m_nCurrentFrame);
}

uint32_t Image::GetCurrentFrameIndex() const
{
	return m_nCurrentFrame;
}

int32_t Image::GetCycledCount() const
{
	return m_nCycledCount;
}

void Image::ClearCycledCount()
{
	m_nCycledCount = 0;
}

bool Image::ContinuePlay() const
{
	if (m_imageAttribute.nPlayCount < 0) {
		return true;
	}
	else if (m_imageAttribute.nPlayCount == 0) {
		return m_bPlaying;
	}
	else {
		return m_nCycledCount < m_imageAttribute.nPlayCount;
	}
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

}