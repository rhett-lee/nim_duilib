#include "Image_ICO.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Image/ImageUtil.h"

namespace ui
{
struct Image_ICO::TImpl
{
    //图片宽度
    uint32_t m_nWidth = 0;

    //图片高度
    uint32_t m_nHeight = 0;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    //各个图片帧的数据
    std::vector<AnimationFramePtr> m_frames;
};

Image_ICO::Image_ICO()
{
    m_impl = std::make_unique<TImpl>();
}

Image_ICO::~Image_ICO()
{
}

bool Image_ICO::IsDelayDecodeEnabled() const
{
    return false;
}

bool Image_ICO::IsDelayDecodeFinished() const
{
    return true;
}

uint32_t Image_ICO::GetDecodedFrameIndex() const
{
    if (!m_impl->m_frames.empty()) {
        return (uint32_t)m_impl->m_frames.size() - 1;
    }
    return 0;
}

bool Image_ICO::DelayDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> /*IsAborted*/)
{
    return false;
}

bool Image_ICO::MergeDelayDecodeData()
{
    return false;
}

bool Image_ICO::LoadImageFromMemory(const std::vector<UiImageData>& imageData,
                                    float fImageSizeScale,
                                    uint32_t nIconSize,
                                    int32_t nFrameDelayMs)
{
    ASSERT(!imageData.empty());
    if (imageData.empty()) {
        return false;
    }
    //使用nIconSize参数作为图片的大小
    m_impl->m_nWidth = ImageUtil::GetScaledImageSize(nIconSize, fImageSizeScale);
    m_impl->m_nHeight = m_impl->m_nWidth;
    m_impl->m_fImageSizeScale = fImageSizeScale;
    ASSERT((m_impl->m_nWidth > 0) && (m_impl->m_nHeight > 0));
    if ((m_impl->m_nWidth == 0) || (m_impl->m_nHeight == 0)) {
        return false;
    }
    const uint32_t nImageSize = m_impl->m_nWidth;

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }

    m_impl->m_frames.clear();
    m_impl->m_frames.resize(imageData.size());
    const int32_t nFrameCount = (int32_t)imageData.size();
    for (int i = 0; i < nFrameCount; ++i) {
        const UiImageData& icoData = imageData[i];
        AnimationFramePtr pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
        pFrameData->m_nFrameIndex = i;
        pFrameData->SetDelayMs(nFrameDelayMs);
        pFrameData->m_nOffsetX = 0;
        pFrameData->m_nOffsetY = 0;
        pFrameData->m_bDataPending = false;
        pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
        ASSERT(pFrameData->m_pBitmap != nullptr);
        if (pFrameData->m_pBitmap == nullptr) {
            return false;
        }
        ASSERT((icoData.m_imageWidth != 0) && (icoData.m_imageHeight != 0));
        if ((icoData.m_imageWidth == 0) || (icoData.m_imageHeight == 0)) {
            return false;
        }

        //图片的宽度和高度，不超过约定值(超过就缩小图片)
        if ((icoData.m_imageWidth > nImageSize) || (icoData.m_imageHeight > nImageSize)) {
            float fScaleX = static_cast<float>(nImageSize) / icoData.m_imageWidth;
            float fScaleY = static_cast<float>(nImageSize) / icoData.m_imageHeight;
            float fNewImageSizeScale = std::min(fScaleX, fScaleY);
            pFrameData->m_pBitmap->Init(icoData.m_imageWidth, icoData.m_imageHeight, icoData.m_imageData.data(), fNewImageSizeScale);
        }
        else {
            pFrameData->m_pBitmap->Init(icoData.m_imageWidth, icoData.m_imageHeight, icoData.m_imageData.data(), fImageSizeScale);
        }
        ASSERT(pFrameData->m_pBitmap->GetWidth() <= (uint32_t)m_impl->m_nWidth);
        ASSERT(pFrameData->m_pBitmap->GetHeight() <= (uint32_t)m_impl->m_nHeight);
        
        //计算偏移(使小图标居中显示)
        if (pFrameData->m_pBitmap->GetWidth() < m_impl->m_nWidth) {
            pFrameData->m_nOffsetX = (m_impl->m_nWidth - pFrameData->m_pBitmap->GetWidth()) / 2;
        }
        if (pFrameData->m_pBitmap->GetHeight() < m_impl->m_nHeight) {
            pFrameData->m_nOffsetY = (m_impl->m_nHeight - pFrameData->m_pBitmap->GetHeight()) / 2;
        }
        ASSERT(pFrameData->m_nOffsetX >= 0);
        ASSERT(pFrameData->m_nOffsetY >= 0);
        m_impl->m_frames[i] = pFrameData;
    }
    return true;
}

uint32_t Image_ICO::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_ICO::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_ICO::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

int32_t Image_ICO::GetFrameCount() const
{
    return (int32_t)m_impl->m_frames.size();
}

int32_t Image_ICO::GetLoopCount() const
{
    return -1;
}

bool Image_ICO::IsFrameDataReady(uint32_t nFrameIndex)
{
    if (nFrameIndex < (int32_t)m_impl->m_frames.size()) {
        return true;
    }
    return false;
}

int32_t Image_ICO::GetFrameDelayMs(uint32_t nFrameIndex)
{
    if (nFrameIndex < m_impl->m_frames.size()) {
        auto pAnimationFrame = m_impl->m_frames[nFrameIndex];
        if (pAnimationFrame != nullptr) {
            return pAnimationFrame->GetDelayMs();
        }
    }
    return IMAGE_ANIMATION_DELAY_MS;
}

bool Image_ICO::ReadFrameData(int32_t nFrameIndex, const UiSize& /*szDestRectSize*/, AnimationFrame* pAnimationFrame)
{
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        return false;
    }
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < (int32_t)m_impl->m_frames.size()));
    if (nFrameIndex < 0) {
        return false;
    }
    bool bRet = false;
    if (nFrameIndex < (int32_t)m_impl->m_frames.size()) {
        AnimationFramePtr pFrameData = m_impl->m_frames[nFrameIndex];
        ASSERT(pFrameData != nullptr);
        if (pFrameData != nullptr) {
            ASSERT(pFrameData->m_nFrameIndex == nFrameIndex);
            *pAnimationFrame = *pFrameData;
            pAnimationFrame->m_bDataPending = false;
            ASSERT(pAnimationFrame->m_pBitmap != nullptr);
            bRet = true;
        }
    }
    return bRet;
}

} //namespace ui
