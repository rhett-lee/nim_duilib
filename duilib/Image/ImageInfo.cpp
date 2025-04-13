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
    m_pFrameBitmaps(nullptr),
    m_loadDpiScale(0)
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

bool ImageInfo::SwapImageData(ImageInfo& r)
{
    //校验属性，确保属性一致才交换数据
    ASSERT(r.GetWidth() == GetWidth());
    if (r.GetWidth() != GetWidth()) {
        return false;
    }
    ASSERT(r.GetHeight() == GetHeight());
    if (r.GetHeight() != GetHeight()) {
        return false;
    }
    ASSERT(r.IsBitmapSizeDpiScaled() == IsBitmapSizeDpiScaled());
    if (r.IsBitmapSizeDpiScaled() != IsBitmapSizeDpiScaled()) {
        return false;
    }
    ASSERT(r.GetLoadDpiScale() == GetLoadDpiScale());
    if (r.GetLoadDpiScale() != GetLoadDpiScale()) {
        return false;
    }
    ASSERT(r.GetLoadKey() == GetLoadKey());
    if (r.GetLoadKey() != GetLoadKey()) {
        return false;
    }
    ASSERT(r.GetImageKey() == GetImageKey());
    if (r.GetImageKey() != GetImageKey()) {
        return false;
    }

    //交换数据
    std::swap(m_nPlayCount, r.m_nPlayCount);
    std::swap(m_nFrameCount, r.m_nFrameCount);
    std::swap(m_pFrameIntervals, r.m_pFrameIntervals);
    std::swap(m_pFrameBitmaps, r.m_pFrameBitmaps);
    return true;
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
    else if (interval < 50)    {
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

void ImageInfo::SetLoadKey(const DString& loadKey)
{
    m_loadKey = loadKey;
}

DString ImageInfo::GetLoadKey() const
{
    return m_loadKey.c_str();
}

void ImageInfo::SetLoadDpiScale(uint32_t dpiScale)
{
    m_loadDpiScale = dpiScale;
}

uint32_t ImageInfo::GetLoadDpiScale() const
{
    return m_loadDpiScale;
}

void ImageInfo::SetImageKey(const DString& imageKey)
{
    m_imageKey = imageKey;
}

DString ImageInfo::GetImageKey() const
{
    return m_imageKey.c_str();
}

}
