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

Image::~Image()
{
    if (m_pImageGif != nullptr) {
        m_pImageGif->StopGifPlay();
        delete m_pImageGif;
    }
}

void Image::InitImageAttribute()
{
    m_imageAttribute.Init();
}

void Image::SetImageString(const DString& strImageString, const DpiManager& dpi)
{
    ClearImageCache();
    m_imageAttribute.InitByImageString(strImageString, dpi);
}

void Image::UpdateImageAttribute(const DString& strImageString, const DpiManager& dpi)
{
    //只修改属性，不修改图片资源路径
    DString sImagePath = m_imageAttribute.m_sImagePath.c_str();
    m_imageAttribute.ModifyAttribute(strImageString, dpi);
    m_imageAttribute.m_sImagePath = sImagePath;
}

DString Image::GetImageString() const
{
    return m_imageAttribute.m_sImageString.c_str();
}

bool Image::EqualToImageString(const DString& imageString) const
{
    return m_imageAttribute.m_sImageString == imageString;
}

DString Image::GetImagePath() const
{
    return m_imageAttribute.m_sImagePath.c_str();
}

void Image::SetImagePadding(const UiPadding& newPadding, bool bNeedDpiScale, const DpiManager& dpi)
{
    m_imageAttribute.SetImagePadding(newPadding, bNeedDpiScale, dpi);
}

UiPadding Image::GetImagePadding(const DpiManager& dpi) const
{
    return m_imageAttribute.GetImagePadding(dpi);
}

bool Image::IsImagePaintEnabled() const
{
    return m_imageAttribute.m_bPaintEnabled;
}

void Image::SetImagePaintEnabled(bool bEnable)
{
    m_imageAttribute.m_bPaintEnabled = bEnable;
}

void Image::SetImagePlayCount(int32_t nPlayCount)
{
    m_imageAttribute.m_nPlayCount = nPlayCount;
}

void Image::SetImageFade(uint8_t nFade)
{
    m_imageAttribute.m_bFade = nFade;
}

const ImageAttribute& Image::GetImageAttribute() const
{
    return m_imageAttribute;
}

ImageLoadAttribute Image::GetImageLoadAttribute() const
{
    return ImageLoadAttribute(m_imageAttribute.m_srcWidth.c_str(),
                              m_imageAttribute.m_srcHeight.c_str(),
                              m_imageAttribute.m_srcDpiScale,
                              m_imageAttribute.m_bHasSrcDpiScale,
                              m_imageAttribute.m_iconSize);
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
