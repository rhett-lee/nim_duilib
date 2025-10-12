#include "Image.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/ImagePlayer.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Utils/PerformanceUtil.h"

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
    DpiScaleOption loadDpiScaleOption = m_imageAttribute.m_loadDpiScale;
    DpiScaleOption imageSizeDpiScaleOption = m_imageAttribute.m_sizeDpiScale;
    uint32_t nLoadDpiScale = (m_pControl != nullptr) ? m_pControl->Dpi().GetScale() : 100;
    bool bAsyncDecode = m_imageAttribute.m_bAsyncLoad;
    bool bIconAsAnimation = m_imageAttribute.m_bIconAsAnimation;
    uint32_t nIconSize = m_imageAttribute.m_nIconSize;
    int32_t nIconFrameDelayMs = m_imageAttribute.m_nIconFrameDelayMs;
    float fPagMaxFrameRate = m_imageAttribute.m_fPagMaxFrameRate;
    DString pagFilePwd = m_imageAttribute.m_pagFilePwd.c_str();
    return ImageLoadParam(m_imageAttribute.m_srcWidth.c_str(),
                          m_imageAttribute.m_srcHeight.c_str(),
                          loadDpiScaleOption,
                          imageSizeDpiScaleOption,
                          nLoadDpiScale,
                          bAsyncDecode,
                          bIconAsAnimation,
                          nIconFrameDelayMs,
                          nIconSize,
                          fPagMaxFrameRate,
                          pagFilePwd);
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

AnimationFramePtr Image::GetCurrentFrame(const UiRect& rcDest, UiRect& rcSource, UiRect& rcSourceCorners) const
{
    PerformanceStat statPerformance(_T("Image::GetCurrentFrame"));
    ASSERT((m_imageInfo != nullptr) && m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || !m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }
    //多帧图片
    AnimationFramePtr pAnimationFrame;
    if (m_nCurrentFrame < m_imageInfo->GetFrameCount()) {
        pAnimationFrame = m_imageInfo->GetFrame(m_nCurrentFrame, UiSize(rcDest.Width(), rcDest.Height()));
    }
    else {
        uint32_t nCurrentFrame = 0;
        if (m_imageInfo->GetFrameCount() > 0) {
            nCurrentFrame = m_nCurrentFrame % m_imageInfo->GetFrameCount();
        }
        pAnimationFrame = m_imageInfo->GetFrame(nCurrentFrame, UiSize(rcDest.Width(), rcDest.Height()));
    }
    if (pAnimationFrame != nullptr) {
        AdjustImageSourceRect(pAnimationFrame->m_pBitmap, rcSource, rcSourceCorners);
    }
    return pAnimationFrame;
}

std::shared_ptr<IBitmap> Image::GetBitmapData(UiRect& rcSource, UiRect& rcSourceCorners) const
{
    PerformanceStat statPerformance(_T("Image::GetBitmapData"));
    ASSERT((m_imageInfo != nullptr) && !m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }
    //单帧图片
    std::shared_ptr<IBitmap> pBitmap = m_imageInfo->GetBitmap();
    AdjustImageSourceRect(pBitmap, rcSource, rcSourceCorners);
    return pBitmap;
}

void Image::AdjustImageSourceRect(const std::shared_ptr<IBitmap>& pBitmap, UiRect& rcSource, UiRect& rcSourceCorners) const
{
    if (pBitmap == nullptr) {
        return;
    }
    ASSERT((pBitmap->GetWidth() > 0) && (pBitmap->GetHeight() > 0));
    if ((pBitmap->GetWidth() <= 0) || (pBitmap->GetHeight() <= 0)) {
        return;
    }
    ASSERT((m_imageInfo->GetWidth() > 0) && (m_imageInfo->GetHeight() > 0));
    if ((m_imageInfo->GetWidth() <= 0) || (m_imageInfo->GetHeight() <= 0)) {
        return;
    }
    //校验位图大小与ImageInfo大小是否一致，如果不一致，则需要调整绘制区域参数
    if ((m_imageInfo->GetWidth() == (int32_t)pBitmap->GetWidth()) &&
        (m_imageInfo->GetHeight() == (int32_t)pBitmap->GetHeight())) {
        return;
    }
    float fSizeScaleX = static_cast<float>(pBitmap->GetWidth()) / m_imageInfo->GetWidth();
    float fSizeScaleY = static_cast<float>(pBitmap->GetHeight()) / m_imageInfo->GetHeight();
    const bool bFullImage = (rcSource.left == 0) &&
                            (rcSource.top == 0) &&
                            (rcSource.right == m_imageInfo->GetWidth()) &&
                            (rcSource.bottom == m_imageInfo->GetHeight());
    if (bFullImage) {
        //完整图片
        rcSource.right = pBitmap->GetWidth();
        rcSource.bottom = pBitmap->GetHeight();

        if (ImageUtil::NeedResizeImage(fSizeScaleX)) {
            rcSourceCorners.left = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.left, fSizeScaleX);
            rcSourceCorners.right = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.right, fSizeScaleX);
        }
        if (ImageUtil::NeedResizeImage(fSizeScaleY)) {
            rcSourceCorners.top = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.top, fSizeScaleY);
            rcSourceCorners.bottom = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.bottom, fSizeScaleY);
        }
    }
    else {
        if (ImageUtil::NeedResizeImage(fSizeScaleX)) {
            //需要对rcSource修改
            rcSource.left = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.left, fSizeScaleX);
            rcSource.right = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.right, fSizeScaleX);
            ASSERT(rcSource.right > rcSource.left);
            ASSERT(rcSource.left >= 0);
            ASSERT(rcSource.right <= (int32_t)pBitmap->GetWidth());

            rcSourceCorners.left = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.left, fSizeScaleX);
            rcSourceCorners.right = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.right, fSizeScaleX);
        }
        if (ImageUtil::NeedResizeImage(fSizeScaleY)) {
            //需要对rcSource修改
            rcSource.top = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.top, fSizeScaleY);
            rcSource.bottom = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSource.bottom, fSizeScaleY);
            ASSERT(rcSource.bottom > rcSource.top);
            ASSERT(rcSource.top >= 0);
            ASSERT(rcSource.bottom <= (int32_t)pBitmap->GetHeight());

            rcSourceCorners.top = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.top, fSizeScaleY);
            rcSourceCorners.bottom = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)rcSourceCorners.bottom, fSizeScaleY);
        }
    }
}

std::shared_ptr<IBitmap> Image::GetCurrentBitmap(bool bImageStretch,
                                                 const UiRect& rcDest,
                                                 UiRect& rcSource,
                                                 UiRect& rcSourceCorners) const
{
    ASSERT((m_imageInfo != nullptr) && !m_imageInfo->IsMultiFrameImage());
    if (!m_imageInfo || m_imageInfo->IsMultiFrameImage()) {
        return nullptr;
    }
    ASSERT(!rcDest.IsEmpty() && !rcSource.IsEmpty());
    if (rcDest.IsEmpty() || rcSource.IsEmpty()) {
        return nullptr;
    }

    if (!bImageStretch) {
        //绘制时，不支持拉伸
        return GetBitmapData(rcSource, rcSourceCorners);
    }
    else if (!m_imageInfo->IsSvgImage()) {
        //不是SVG图片，不支持矢量缩放
        return GetBitmapData(rcSource, rcSourceCorners);
    }
    else if (!rcSourceCorners.IsZero()) {
        //如果设置了九宫格绘制，则按不拉伸处理(如果拉伸图片，四个角会变形)
        return GetBitmapData(rcSource, rcSourceCorners);
    }
    else if ((rcDest.Width() == rcSource.Width()) ||
             (rcDest.Height() == rcSource.Height())) {
        //如果绘制目标区域和图片源区域大小一致，无需拉伸
        return GetBitmapData(rcSource, rcSourceCorners);
    }
    else {
        //SVG图片：支持矢量缩放
        std::shared_ptr<IBitmap> pBitmap = m_imageInfo->GetSvgBitmap(rcDest, rcSource);
        if (pBitmap == nullptr) {
            pBitmap = GetBitmapData(rcSource, rcSourceCorners);
        }
        else {
            AdjustImageSourceRect(pBitmap, rcSource, rcSourceCorners);
        }
        return pBitmap;
    }    
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

void Image::RedrawImage()
{
    if (m_pControl != nullptr) {
        if (m_rcDrawDestRect.IsEmpty()) {
            m_pControl->Invalidate();
        }
        else {
            m_pControl->InvalidateRect(m_rcDrawDestRect);
        }        
    }
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
