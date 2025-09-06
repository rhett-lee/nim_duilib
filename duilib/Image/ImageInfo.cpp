#include "ImageInfo.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{

ImageInfo::ImageInfo():    
    m_loadParam(_T(""), _T(""), ImageLoadParam::DpiScaleOption::kDefault),
    m_fImageSizeScale(IMAGE_SIZE_SCALE_NONE),
    m_nLoopCount(-1),
    m_nFrameCount(0),
    m_nWidth(0),
    m_nHeight(0),
    m_asyncLoadCallback(nullptr),
    m_imageType(ImageType::kImageBitmap),
    m_bHasCustomSizeScale(false),
    m_fCustomSizeScaleX(0),
    m_fCustomSizeScaleY(0)
{
}

ImageInfo::~ImageInfo()
{
}

void ImageInfo::ReleaseImage()
{
    if (m_pImageData != nullptr) {
        GlobalManager::Instance().Image().ReleaseImage(m_pImageData);
        m_pImageData.reset();
    }
}

bool ImageInfo::IsSvgImage() const
{
    return (m_imageType == ImageType::kImageSvg);
}

std::shared_ptr<IBitmap> ImageInfo::GetSvgBitmap(float fImageSizeScale)
{
    //SVG图片，无缓存
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }
    ASSERT((m_nWidth > 0) && (m_nHeight > 0));
    if ((m_nWidth <= 0) && (m_nHeight <= 0)) {
        return nullptr;
    }

    std::shared_ptr<IBitmap> pBitmap;
    if (m_imageType == ImageType::kImageSvg) {
        std::shared_ptr<ISvgImage> pSvgImage = pImageData->GetImageSvg();
        ASSERT(pSvgImage != nullptr);
        if (pSvgImage == nullptr) {
            return nullptr;
        }
        int32_t nWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)m_nWidth, fImageSizeScale);
        int32_t nHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)m_nHeight, fImageSizeScale);
        pBitmap = pSvgImage->GetBitmap(UiSize(nWidth, nHeight));
    }
    return pBitmap;
}

std::shared_ptr<IBitmap> ImageInfo::GetBitmap()
{
    if (m_imageType == ImageType::kImageBitmap) {
        //位图图片：优先使用缓存图片
        if (m_pBitmap != nullptr) {
            return m_pBitmap;
        }
    }
    else if (m_imageType == ImageType::kImageSvg) {
        //SVG图片
        return GetSvgBitmap(IMAGE_SIZE_SCALE_NONE);
    }
    else if (m_imageType == ImageType::kImageAnimation) {
        //动画图片
        ASSERT(0);
        return nullptr;
    }

    //设置图片内部数据(延迟解码)
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }

    ImageType imageType = pImageData->GetImageType();
    if (imageType == ImageType::kImageBitmap) {
        std::shared_ptr<IBitmap> pBitmap = pImageData->GetImageBitmap();
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        if (((int32_t)pBitmap->GetWidth() == m_nWidth) && ((int32_t)pBitmap->GetHeight() == m_nHeight)) {
            m_pBitmap = pBitmap;
        }
        else {
            //若大小不一致，生成缓存位图后，释放原图
            m_pBitmap = ImageUtil::ResizeImageBitmap(pBitmap.get(), m_nWidth, m_nHeight);
            ReleaseImage();
        }
        return m_pBitmap;
    }
    else {
        //未知错误
        ASSERT(0);
    }
    return nullptr;
}

std::shared_ptr<IAnimationImage::AnimationFrame> ImageInfo::GetFrame(uint32_t nFrameIndex)
{
    ASSERT(m_imageType == ImageType::kImageAnimation);
    if (m_imageType != ImageType::kImageAnimation) {
        //动画图片
        return nullptr;
    }
    const uint32_t nFrameCount = GetFrameCount();
    ASSERT(nFrameCount > 0);
    if (nFrameCount == 0) {
        return nullptr;
    }
    ASSERT(nFrameIndex <= nFrameCount);
    if (nFrameIndex >= nFrameCount) {
        return nullptr;
    }

    //查询缓存，如果缓存中有该帧，则直接返回
    if (nFrameIndex < m_frameList.size()) {
        if (m_frameList[nFrameIndex] != nullptr) {
            return m_frameList[nFrameIndex];
        }
    }
    
    //设置图片内部数据(延迟解码)
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }

    std::shared_ptr<IAnimationImage::AnimationFrame> pAnimationFrame;
    ImageType imageType = pImageData->GetImageType();
    if (imageType == ImageType::kImageAnimation) {
        std::shared_ptr<IAnimationImage> pAnimationImage = pImageData->GetImageAnimation();
        ASSERT(pAnimationImage != nullptr);
        if (pAnimationImage == nullptr) {
            return nullptr;
        }
        pAnimationFrame = std::make_shared<IAnimationImage::AnimationFrame>();
        if (pAnimationImage->ReadFrame(nFrameIndex, pAnimationFrame.get())) {
            pAnimationFrame->m_nFrameIndex = nFrameIndex;
            if (pAnimationFrame->m_bDataPending) {
                //数据解码未完成，直接返回
                ASSERT(pAnimationFrame->m_pBitmap == nullptr);
                return pAnimationFrame;
            }
            ASSERT(pAnimationFrame->GetDelayMs() > 0);         
            ASSERT(pAnimationFrame->m_pBitmap != nullptr);
            if (pAnimationFrame->m_pBitmap != nullptr) {
                //如果图片属性修改了图片大小，需要跟随调整
                uint32_t nNewWidth = pAnimationFrame->m_pBitmap->GetWidth();
                uint32_t nNewHeight = pAnimationFrame->m_pBitmap->GetHeight();
                if (m_bHasCustomSizeScale) {
                    nNewWidth = ImageUtil::GetScaledImageSize(nNewWidth, m_fCustomSizeScaleX);
                    nNewHeight = ImageUtil::GetScaledImageSize(nNewHeight, m_fCustomSizeScaleY);
                }
                if ((pAnimationFrame->m_pBitmap->GetWidth() != nNewWidth) ||
                    (pAnimationFrame->m_pBitmap->GetHeight() != nNewHeight)) {
                    pAnimationFrame->m_nOffsetX = ImageUtil::GetScaledImageOffset(pAnimationFrame->m_nOffsetX, m_fCustomSizeScaleX);
                    pAnimationFrame->m_nOffsetY = ImageUtil::GetScaledImageOffset(pAnimationFrame->m_nOffsetY, m_fCustomSizeScaleY);
                    pAnimationFrame->m_pBitmap = ImageUtil::ResizeImageBitmap(pAnimationFrame->m_pBitmap.get(), nNewWidth, nNewHeight);
                    ASSERT(pAnimationFrame->m_pBitmap != nullptr);
                }
            }
            if (pAnimationFrame->m_pBitmap != nullptr) {
                //生成缓存
                if (m_frameList.size() != nFrameCount) {
                    m_frameList.resize(nFrameCount);
                }
                m_frameList[nFrameIndex] = pAnimationFrame;
            }
        }
        else {
            ASSERT(0);
        }
    }
    else {
        //未知错误
        ASSERT(0);
    }
    return pAnimationFrame;
}

bool ImageInfo::SetImageData(const ImageLoadParam& loadParam,
                             const std::shared_ptr<IImage>& pImageData,
                             StdClosure asyncLoadCallback)
{
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return false;
    }
    const int32_t nWidth = pImageData->GetWidth();
    const int32_t nHeight = pImageData->GetHeight();
    ASSERT(nWidth > 0);
    ASSERT(nHeight > 0);
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return false;
    }

    //设置宽度和高度
    uint32_t nImageWidth = (uint32_t)nWidth;
    uint32_t nImageHeight = (uint32_t)nHeight;
    if (loadParam.CalcImageLoadSize(nImageWidth, nImageHeight, false)) {
        m_nWidth = (int32_t)nImageWidth;
        m_nHeight = (int32_t)nImageHeight;
        ASSERT(m_nWidth > 0);
        ASSERT(m_nHeight > 0);
        if ((m_nWidth <= 0) || (m_nHeight <= 0)) {
            return false;
        }
        m_bHasCustomSizeScale = true;
        m_fCustomSizeScaleX = static_cast<float>(m_nWidth) / nWidth;
        m_fCustomSizeScaleY = static_cast<float>(m_nHeight) / nHeight;
    }
    else {
        m_nWidth = nWidth;
        m_nHeight = nHeight;
        m_bHasCustomSizeScale = false;
        m_fCustomSizeScaleX = 0;
        m_fCustomSizeScaleY = 0;
    }

    m_loadParam = loadParam;
    m_pImageData = pImageData;    
    m_asyncLoadCallback = asyncLoadCallback;
    m_fImageSizeScale = pImageData->GetImageSizeScale();

    //设置图片内部数据
    ImageType imageType = pImageData->GetImageType();
    m_imageType = imageType;
    if (imageType == ImageType::kImageBitmap) {
        m_nFrameCount = 1;
        m_nLoopCount = -1;
    }
    else if (imageType == ImageType::kImageSvg) {
        m_nFrameCount = 1;
        m_nLoopCount = -1;
    }
    else if (imageType == ImageType::kImageAnimation) {
        std::shared_ptr<IAnimationImage> pAnimationImage = pImageData->GetImageAnimation();
        ASSERT(pAnimationImage != nullptr);
        if (pAnimationImage == nullptr) {
            return false;
        }
        m_nFrameCount = pAnimationImage->GetFrameCount();
        m_nLoopCount = pAnimationImage->GetLoopCount();
        ASSERT(m_nFrameCount != 0);
        if (m_nFrameCount == 0) {
            return false;
        }
        if (m_nLoopCount == 0) {
            m_nLoopCount = -1;
        }
        if (pAnimationImage->IsDecodeImageDataEnabled()) {
            //开始延迟解码(优先在子线程中解码)
            pAnimationImage->SetDecodeImageDataStarted();
            int32_t nDecodeImageThread = ThreadIdentifier::kThreadWorker;
            if (!GlobalManager::Instance().Thread().HasThread(nDecodeImageThread)) {
                nDecodeImageThread = ThreadIdentifier::kThreadUI;
            }
            auto decodeImageTask = [pAnimationImage]() {
                    //异步解码图片
                    pAnimationImage->DecodeImageData();
                };
            GlobalManager::Instance().Thread().PostTask(nDecodeImageThread, decodeImageTask);
        }
    }
    else {
        ASSERT(0);
        return false;
    }
    return true;
}

int32_t ImageInfo::GetWidth() const
{
    return m_nWidth;
}

int32_t ImageInfo::GetHeight() const
{
    return m_nHeight;
}

uint32_t ImageInfo::GetFrameCount() const
{
    return m_nFrameCount;
}

bool ImageInfo::IsMultiFrameImage() const
{
    return GetFrameCount() > 1;
}

int32_t ImageInfo::GetLoopCount() const
{
    return m_nLoopCount;
}

DString ImageInfo::GetLoadKey() const
{
    return m_loadParam.GetLoadKey(m_loadParam.GetLoadDpiScale());
}

bool ImageInfo::IsBitmapSizeDpiScaled() const
{
    return ImageUtil::NeedResizeImage(m_fImageSizeScale);
}

uint32_t ImageInfo::GetLoadDpiScale() const
{
    return m_loadParam.GetLoadDpiScale();
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
