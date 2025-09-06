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

    //各个图片帧的数据
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_frames;
};

Image_ICO::Image_ICO()
{
    m_impl = std::make_unique<TImpl>();
}

Image_ICO::~Image_ICO()
{
}

bool Image_ICO::IsDecodeImageDataEnabled() const
{
    return false;
}

void Image_ICO::SetDecodeImageDataStarted()
{
}

bool Image_ICO::DecodeImageData()
{
    return true;
}

void Image_ICO::SetDecodeImageDataAborted()
{
}

bool Image_ICO::LoadImageFromMemory(const std::vector<ImageDecoder::ImageData>& imageData,
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
        const ImageDecoder::ImageData& icoData = imageData[i];
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
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
            pFrameData->m_pBitmap->Init(icoData.m_imageWidth, icoData.m_imageHeight, icoData.m_bitmapData.data(), fNewImageSizeScale);
        }
        else {
            pFrameData->m_pBitmap->Init(icoData.m_imageWidth, icoData.m_imageHeight, icoData.m_bitmapData.data(), fImageSizeScale);
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

int32_t Image_ICO::GetFrameCount() const
{
    return (int32_t)m_impl->m_frames.size();
}

int32_t Image_ICO::GetLoopCount() const
{
    return -1;
}

bool Image_ICO::ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
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
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = m_impl->m_frames[nFrameIndex];
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
