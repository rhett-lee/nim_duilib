#include "ImageInfo.h"

namespace ui 
{

ImageInfo::ImageInfo():
	m_bDpiScaled(false),
	m_nWidth(0),
	m_nHeight(0),
	m_nPlayCount(-1),
	m_pFrameIntervals(nullptr),
	m_nFrameCount(0),
	m_pFrameBitmaps(nullptr)
{
}

ImageInfo::~ImageInfo()
{
	if (m_pFrameBitmaps != nullptr) {
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			delete m_pFrameBitmaps[i];
		}
		delete m_pFrameBitmaps;
		m_pFrameBitmaps = nullptr;
	}
	
	if (m_pFrameIntervals != nullptr) {
		delete m_pFrameIntervals;
		m_pFrameIntervals = nullptr;
	}
}

void ImageInfo::SetImageFullPath(const std::wstring& path)
{
	m_imageFullPath = path;
}

std::wstring ImageInfo::GetImageFullPath() const
{
	return m_imageFullPath.c_str();
}

void ImageInfo::SetFrameInterval(const std::vector<int32_t>& frameIntervals)
{
	if (m_pFrameIntervals == nullptr) {
		m_pFrameIntervals = new std::vector<int32_t>;
	}
	*m_pFrameIntervals = frameIntervals;
}

void ImageInfo::SetFrameBitmap(const std::vector<IBitmap*>& frameBitmaps)
{
	if (m_pFrameBitmaps != nullptr) {
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			delete m_pFrameBitmaps[i];
		}
		delete m_pFrameBitmaps;
		m_pFrameBitmaps = nullptr;
	}
	m_nFrameCount = (uint32_t)frameBitmaps.size();
	if (m_nFrameCount > 0) {
		m_pFrameBitmaps = new IBitmap*[m_nFrameCount];
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			m_pFrameBitmaps[i] = frameBitmaps[i];
		}
	}	
}

IBitmap* ImageInfo::GetBitmap(uint32_t nIndex) const
{
	ASSERT((nIndex < m_nFrameCount) && (m_pFrameBitmaps != nullptr));
	if ((nIndex < m_nFrameCount) && (m_pFrameBitmaps != nullptr)){
		return m_pFrameBitmaps[nIndex];
	}
	return nullptr;
}

void ImageInfo::SetImageSize(int32_t nWidth, int32_t nHeight)
{
	ASSERT(nWidth > 0);
	ASSERT(nHeight > 0);
	if (nWidth > 0) {
		m_nWidth = nWidth; 
	}
	if (nHeight > 0) {
		m_nHeight = nHeight;
	}	
}

uint32_t ImageInfo::GetFrameCount() const
{
	return m_nFrameCount;
}

bool ImageInfo::IsMultiFrameImage() const
{
	return GetFrameCount() > 1;
}

int32_t ImageInfo::GetFrameInterval(uint32_t nIndex) const
{
	if (m_pFrameIntervals == nullptr) {
		return 0;
	}
	const std::vector<int32_t>& frameIntervals = *m_pFrameIntervals;
	if (nIndex >= frameIntervals.size()) {
		return 0;
	}
	int32_t interval = frameIntervals[nIndex];
	if (interval < 30) {
		interval = 100;
	}
	else if (interval < 50)	{
		interval = 50;
	}
	return interval;
}

void ImageInfo::SetPlayCount(int32_t nPlayCount)
{
	m_nPlayCount = nPlayCount;
}

int32_t ImageInfo::GetPlayCount() const
{
	return m_nPlayCount;
}

void ImageInfo::SetCacheKey(const std::wstring& cacheKey)
{
	m_cacheKey = cacheKey;
}

std::wstring ImageInfo::GetCacheKey() const
{
	return m_cacheKey.c_str();
}

}