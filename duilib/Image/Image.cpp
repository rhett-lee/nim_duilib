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
    if ((m_pImagePlayer != nullptr) && m_pImagePlayer->IsAnimationPlaying()) {
        m_pImagePlayer->StopImageAnimation(AnimationImagePos::kFrameCurrent, false);
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
    bool bIconAsAnimation = m_imageAttribute.m_bIconAsAnimation;
    uint32_t nIconSize = m_imageAttribute.m_nIconSize;
    int32_t nIconFrameDelayMs = m_imageAttribute.m_nIconFrameDelayMs;
    float fPagMaxFrameRate = m_imageAttribute.m_fPagMaxFrameRate;
    return ImageLoadParam(m_imageAttribute.m_srcWidth.c_str(),
                          m_imageAttribute.m_srcHeight.c_str(),
                          nDpiScaleOption,
                          nLoadDpiScale,
                          bIconAsAnimation,
                          nIconFrameDelayMs,
                          nIconSize,
                          fPagMaxFrameRate);
}

const std::shared_ptr<ImageInfo>& Image::GetImageInfo() const
{
    return m_imageInfo;
}

void Image::SetImageInfo(const std::shared_ptr<ImageInfo>& imageInfo)
{
    m_imageInfo = imageInfo;
}

void Image::ClearImageCache()
{
    //停止播放动画
    if (m_pImagePlayer != nullptr) {
        bool bAutoPlay = m_pImagePlayer->IsAutoPlay();
        m_pImagePlayer->StopImageAnimation(AnimationImagePos::kFrameCurrent, false);
        //需要保留原来的自动播放属性，否则换肤后，加载新动画就不会自动播放了
        m_pImagePlayer->SetAutoPlay(bAutoPlay);
    }
    m_nCurrentFrame = 0;
    m_imageInfo.reset();
    m_rcDrawDestRect.Clear();
}

void Image::SetCurrentFrameIndex(uint32_t nCurrentFrame)
{
    m_nCurrentFrame = nCurrentFrame;
    const uint32_t nFrameCount = GetFrameCount();
    if ((m_nCurrentFrame >= nFrameCount) && (nFrameCount > 0)) {
        m_nCurrentFrame = m_nCurrentFrame % nFrameCount;
    }
}

uint32_t Image::GetCurrentFrameIndex() const
{
    return m_nCurrentFrame;
}

uint32_t Image::GetFrameCount() const
{
    if (!m_imageInfo) {
        return 0;
    }
    return m_imageInfo->GetFrameCount();
}

int32_t Image::GetLoopCount() const
{
    if (!m_imageInfo) {
        return -1;
    }
    return m_imageInfo->GetLoopCount();
}

bool Image::IsMultiFrameImage() const
{
    if (!m_imageInfo) {
        return false;
    }
    return m_imageInfo->IsMultiFrameImage();
}

std::shared_ptr<IAnimationImage::AnimationFrame> Image::GetCurrentFrame() const
{
    ASSERT((m_imageInfo != nullptr) && m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || !m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }
    //多帧图片
    if (m_nCurrentFrame < m_imageInfo->GetFrameCount()) {
        return m_imageInfo->GetFrame(m_nCurrentFrame);
    }
    else {
        uint32_t nCurrentFrame = 0;
        if (m_imageInfo->GetFrameCount() > 0) {
            nCurrentFrame = m_nCurrentFrame % m_imageInfo->GetFrameCount();
        }
        return m_imageInfo->GetFrame(nCurrentFrame);
    }
}

std::shared_ptr<IBitmap> Image::GetCurrentBitmap() const
{
    ASSERT((m_imageInfo != nullptr) && !m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }
    //单帧图片
    return m_imageInfo->GetBitmap();
}

std::shared_ptr<IBitmap> Image::GetCurrentBitmap(const UiRect& rcDest, UiRect& rcSource) const
{
    ASSERT((m_imageInfo != nullptr) && !m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }

    if (!m_imageInfo->IsSvgImage()) {
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
                            (rcSource.right == m_imageInfo->GetWidth()) &&
                            (rcSource.bottom == m_imageInfo->GetHeight());

    float fSizeScaleX = static_cast<float>(rcDest.Width()) / rcSource.Width();
    float fSizeScaleY = static_cast<float>(rcDest.Height()) / rcSource.Height();
    float fImageSizeScale = fSizeScaleX < fSizeScaleY ? fSizeScaleX : fSizeScaleY;

    std::shared_ptr<IBitmap> pBitmap = m_imageInfo->GetSvgBitmap(fImageSizeScale);
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

ImagePlayer* Image::InitImagePlayer()
{
    if (!IsMultiFrameImage() || (m_pControl == nullptr)) {
        return nullptr;
    }
    if (m_pImagePlayer == nullptr) {
        m_pImagePlayer = std::make_unique<ImagePlayer>();
        m_pImagePlayer->SetImage(this);
        m_pImagePlayer->SetControl(m_pControl);
    }
    return m_pImagePlayer.get();
}

void Image::SetDrawDestRect(const UiRect& rcImageRect)
{
    m_rcDrawDestRect = rcImageRect;
    if (IsMultiFrameImage()) {
        ImagePlayer* pImagePlayer = InitImagePlayer();
        ASSERT(pImagePlayer != nullptr);
        if (pImagePlayer != nullptr) {
            pImagePlayer->SetImageAnimationRect(rcImageRect);
        }
    }
}

const UiRect& Image::GetDrawDestRect() const
{
    return m_rcDrawDestRect;
}

void Image::CheckStartImageAnimation()
{
    ImagePlayer* pImagePlayer = InitImagePlayer();
    if (pImagePlayer != nullptr) {
        pImagePlayer->CheckStartImageAnimation();
    }
}

void Image::PauseImageAnimation()
{
    if ((m_pImagePlayer != nullptr) && m_pImagePlayer->IsAnimationPlaying()) {
        bool bAutoPlay = m_pImagePlayer->IsAutoPlay();
        m_pImagePlayer->StopImageAnimation(AnimationImagePos::kFrameCurrent, true);
        m_pImagePlayer->SetAutoPlay(bAutoPlay);
    }
}

bool Image::StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount)
{
    bool bRet = false;
    ImagePlayer* pImagePlayer = InitImagePlayer();
    if (pImagePlayer != nullptr) {
        if (pImagePlayer->IsAnimationPlaying()) {
            pImagePlayer->StopImageAnimation(AnimationImagePos::kFrameCurrent, false);
        }
        bRet = pImagePlayer->StartImageAnimation(nStartFrame, nPlayCount);
    }
    return bRet;
}

void Image::StopImageAnimation(AnimationImagePos nStopFrame, bool bTriggerEvent)
{
    if (m_pImagePlayer != nullptr) {
        m_pImagePlayer->StopImageAnimation(nStopFrame, bTriggerEvent);
    }
}

}
