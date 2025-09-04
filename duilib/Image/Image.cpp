#include "Image.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/ImagePlayer.h"

namespace ui 
{
Image::Image() :
    m_pControl(nullptr),
    m_pImagePlayer(nullptr),
    m_nCurrentFrame(0)
{
}

Image::~Image()
{
    if (m_pImagePlayer != nullptr) {
        m_pImagePlayer->StopImageAnimation();
        delete m_pImagePlayer;
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
    m_imageAttribute.m_bHasPlayCount = true;
}

void Image::SetImageFade(uint8_t nFade)
{
    m_imageAttribute.m_bFade = nFade;
}

const ImageAttribute& Image::GetImageAttribute() const
{
    return m_imageAttribute;
}

ImageLoadParam Image::GetImageLoadParam() const
{
    ImageLoadParam::DpiScaleOption nDpiScaleOption = ImageLoadParam::DpiScaleOption::kDefault;
    if (m_imageAttribute.m_bHasSrcDpiScale) {
        if (m_imageAttribute.m_srcDpiScale) {
            nDpiScaleOption = ImageLoadParam::DpiScaleOption::kOn;
        }
        else {
            nDpiScaleOption = ImageLoadParam::DpiScaleOption::kOff;
        }
    }
    uint32_t nLoadDpiScale = 100;//此时未知，不需要设置
    uint32_t nIconSize = m_imageAttribute.m_iconSize;
    float fPagMaxFrameRate = m_imageAttribute.m_fPagMaxFrameRate;
    return ImageLoadParam(m_imageAttribute.m_srcWidth.c_str(),
                          m_imageAttribute.m_srcHeight.c_str(),
                          nDpiScaleOption,
                          nLoadDpiScale,
                          nIconSize,
                          fPagMaxFrameRate);
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

void Image::SetCurrentFrameIndex(uint32_t nCurrentFrame)
{
    m_nCurrentFrame = nCurrentFrame;
}

uint32_t Image::GetCurrentFrameIndex() const
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

int32_t Image::GetLoopCount() const
{
    if (!m_imageCache) {
        return -1;
    }
    return m_imageCache->GetLoopCount();
}

bool Image::IsMultiFrameImage() const
{
    if (!m_imageCache) {
        return false;
    }
    return m_imageCache->IsMultiFrameImage();
}

std::shared_ptr<IAnimationImage::AnimationFrame> Image::GetCurrentFrame() const
{
    ASSERT((m_imageCache != nullptr) && m_imageCache->IsMultiFrameImage());
    if (!m_imageCache || !m_imageCache->IsMultiFrameImage()) {
        return nullptr;
    }
    //多帧图片
    if (m_nCurrentFrame < m_imageCache->GetFrameCount()) {
        return m_imageCache->GetFrame(m_nCurrentFrame);
    }
    else {
        uint32_t nCurrentFrame = 0;
        if (m_imageCache->GetFrameCount() > 0) {
            nCurrentFrame = m_nCurrentFrame % m_imageCache->GetFrameCount();
        }
        return m_imageCache->GetFrame(nCurrentFrame);
    }
}

std::shared_ptr<IBitmap> Image::GetCurrentBitmap() const
{
    ASSERT((m_imageCache != nullptr) && !m_imageCache->IsMultiFrameImage());
    if (!m_imageCache || m_imageCache->IsMultiFrameImage()) {
        return nullptr;
    }
    //单帧图片
    return m_imageCache->GetBitmap();
}

std::shared_ptr<IBitmap> Image::GetCurrentBitmap(const UiRect& rcDest, UiRect& rcSource) const
{
    ASSERT((m_imageCache != nullptr) && !m_imageCache->IsMultiFrameImage());
    if (!m_imageCache || m_imageCache->IsMultiFrameImage()) {
        return nullptr;
    }

    if (!m_imageCache->IsSvgImage()) {
        //不是SVG图片，不支持矢量缩放
        return GetCurrentBitmap();
    }

    if (rcDest.IsEmpty()   ||
        rcSource.IsEmpty() ||
        (rcDest.Width() == rcSource.Width()) ||
        (rcDest.Height() == rcSource.Height())) {
        //不满足条件
        return GetCurrentBitmap();
    }
    const bool bFullImage = (rcSource.left == 0) &&
                            (rcSource.top == 0)  &&
                            (rcSource.right == m_imageCache->GetWidth()) &&
                            (rcSource.bottom == m_imageCache->GetHeight());

    float fSizeScaleX = static_cast<float>(rcDest.Width()) / rcSource.Width();
    float fSizeScaleY = static_cast<float>(rcDest.Height()) / rcSource.Height();
    float fImageSizeScale = fSizeScaleX < fSizeScaleY ? fSizeScaleX : fSizeScaleY;

    std::shared_ptr<IBitmap> pBitmap = m_imageCache->GetSvgBitmap(fImageSizeScale);
    if (pBitmap == nullptr) {
        pBitmap = GetCurrentBitmap();
    }
    else if (bFullImage) {
        //完整图片
        rcSource.right = pBitmap->GetWidth();
        rcSource.bottom = pBitmap->GetHeight();
    }
    else if (ImageUtil::NeedResizeImage(fImageSizeScale)) {
        //缩放后，需要对rcSource修改
        rcSource.left = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.left, fImageSizeScale);
        rcSource.top = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.top, fImageSizeScale);
        rcSource.right = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.right, fImageSizeScale);
        rcSource.bottom = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.bottom, fImageSizeScale);
        ASSERT(rcSource.right > rcSource.left);
        ASSERT(rcSource.bottom > rcSource.top);
        ASSERT(rcSource.left >= 0);
        ASSERT(rcSource.top >= 0);
        ASSERT(rcSource.right <= (int32_t)pBitmap->GetWidth());
        ASSERT(rcSource.bottom <= (int32_t)pBitmap->GetHeight());
    }
    return pBitmap;
}

void Image::SetControl(Control* pControl)
{
    if (m_pControl != pControl) {
        m_pControl = pControl;
        if (m_pImagePlayer != nullptr) {
            m_pImagePlayer->SetControl(pControl);
        }
    }
}

bool Image::CheckStartImageAnimation(const UiRect& rcImageRect)
{
    if (!IsMultiFrameImage() || (m_pControl == nullptr)) {
        return false;
    }
    if (m_pImagePlayer == nullptr) {
        m_pImagePlayer = new ImagePlayer;
        m_pImagePlayer->SetImage(this);
        m_pImagePlayer->SetControl(m_pControl);
    }
    m_pImagePlayer->SetImageRect(rcImageRect);
    if (m_pImagePlayer->IsAnimationPlaying()) {
        return true;
    }
    return m_pImagePlayer->StartImageAnimation();
}

void Image::CheckStopImageAnimation()
{
    if (m_pImagePlayer != nullptr) {
        m_pImagePlayer->StopImageAnimation();
    }
}

bool Image::StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount)
{
    if (!IsMultiFrameImage() || (m_pControl == nullptr)) {
        return false;
    }
    if (m_pImagePlayer == nullptr) {
        m_pImagePlayer = new ImagePlayer;
        m_pImagePlayer->SetImage(this);
        m_pImagePlayer->SetControl(m_pControl);
    }
    if (m_pImagePlayer->IsAnimationPlaying()) {
        m_pImagePlayer->StopImageAnimation();
    }
    m_pImagePlayer->SetImageRect(UiRect());
    return m_pImagePlayer->StartImageAnimation(nStartFrame, nPlayCount);
}

void Image::StopImageAnimation(bool bTriggerEvent, AnimationImagePos nStopFrame)
{
    if (m_pImagePlayer != nullptr) {
        m_pImagePlayer->StopImageAnimation(bTriggerEvent, nStopFrame);
    }
}

void Image::AttachImageAnimationStop(const EventCallback& callback)
{
    if (m_pImagePlayer == nullptr) {
        m_pImagePlayer = new ImagePlayer;
        m_pImagePlayer->SetImage(this);
        m_pImagePlayer->SetControl(m_pControl);
    }
    m_pImagePlayer->AttachImageAnimationStop(callback);
}

}
