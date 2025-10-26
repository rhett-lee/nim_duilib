#include "ImageInfo.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/PerformanceUtil.h"

namespace ui 
{

ImageInfo::ImageInfo():    
    m_nLoopCount(-1),
    m_nFrameCount(0),
    m_nImageInfoWidth(0),
    m_nImageInfoHeight(0),
    m_imageType(ImageType::kImageBitmap),
    m_bEnableImageDpiScale(true),
    m_bHasCustomSizeScale(false),
    m_fCustomSizeScaleX(0),
    m_fCustomSizeScaleY(0),
    m_nImageFileDpiScale(100),
    m_fImageSizeScale(1.0f)
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

std::shared_ptr<IBitmap> ImageInfo::GetSvgBitmap(const UiRect& rcDest, UiRect& rcSource)
{
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }
    std::shared_ptr<ISvgImage> pSvgImage;
    if (m_imageType == ImageType::kImageSvg) {
        pSvgImage = pImageData->GetImageSvg();        
    }
    ASSERT(pSvgImage != nullptr);
    if (pSvgImage == nullptr) {
        return nullptr;
    }
    ASSERT((rcSource.Width() > 0) && (rcSource.Height() > 0));
    if ((rcSource.Width() <= 0) && (rcSource.Height() <= 0)) {
        return nullptr;
    }
    ASSERT((pSvgImage->GetWidth() > 0) && (pSvgImage->GetHeight() > 0));
    if ((pSvgImage->GetWidth() <= 0) && (pSvgImage->GetHeight() <= 0)) {
        return nullptr;
    }
    ASSERT((m_nImageInfoWidth > 0) && (m_nImageInfoHeight > 0));
    if ((m_nImageInfoWidth <= 0) && (m_nImageInfoHeight <= 0)) {
        return nullptr;
    }

    float fSizeScaleX = static_cast<float>(rcDest.Width()) / rcSource.Width();
    float fSizeScaleY = static_cast<float>(rcDest.Height()) / rcSource.Height();
    float fImageSizeScale = fSizeScaleX < fSizeScaleY ? fSizeScaleX  : fSizeScaleY ;
    std::shared_ptr<IBitmap> pBitmap = GetSvgBitmap(fImageSizeScale);
    return pBitmap;
}

std::shared_ptr<IBitmap> ImageInfo::GetSvgBitmap(float fImageSizeScale)
{
    GlobalManager::Instance().AssertUIThread();
    //SVG图片，无缓存
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }
    ASSERT((m_nImageInfoWidth > 0) && (m_nImageInfoHeight > 0));
    if ((m_nImageInfoWidth <= 0) && (m_nImageInfoHeight <= 0)) {
        return nullptr;
    }

    std::shared_ptr<IBitmap> pBitmap;
    if (m_imageType == ImageType::kImageSvg) {
        std::shared_ptr<ISvgImage> pSvgImage = pImageData->GetImageSvg();
        ASSERT(pSvgImage != nullptr);
        if (pSvgImage == nullptr) {
            return nullptr;
        }
        int32_t nWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)m_nImageInfoWidth, fImageSizeScale);
        int32_t nHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)m_nImageInfoHeight, fImageSizeScale);
        pBitmap = pSvgImage->GetBitmap(UiSize(nWidth, nHeight));
    }
    return pBitmap;
}

std::shared_ptr<IBitmap> ImageInfo::GetBitmap()
{
    GlobalManager::Instance().AssertUIThread();
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
    else {
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
        std::shared_ptr<IBitmapImage> pBitmapImage = pImageData->GetImageBitmap();
        ASSERT(pBitmapImage != nullptr);
        if (pBitmapImage == nullptr) {
            return nullptr;
        }
        std::shared_ptr<IBitmap> pBitmap = pBitmapImage->GetBitmap();
        if (pBitmap == nullptr) {
            return nullptr;
        }
        if (((int32_t)pBitmap->GetWidth() == m_nImageInfoWidth) && ((int32_t)pBitmap->GetHeight() == m_nImageInfoHeight)) {
            m_pBitmap = pBitmap;
        }
        else {
            //若大小不一致，生成缓存位图后，释放原图
            m_pBitmap = ImageUtil::ResizeImageBitmap(pBitmap.get(), m_nImageInfoWidth, m_nImageInfoHeight);
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

std::shared_ptr<IAnimationImage> ImageInfo::GetAnimationImage(uint32_t nFrameIndex) const
{
    GlobalManager::Instance().AssertUIThread();
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
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }

    std::shared_ptr<IAnimationImage> pAnimationImage;
    ImageType imageType = pImageData->GetImageType();
    if (imageType == ImageType::kImageAnimation) {
        pAnimationImage = pImageData->GetImageAnimation();
        ASSERT(pAnimationImage != nullptr);
    }
    return pAnimationImage;
}

AnimationFramePtr ImageInfo::GetFrame(uint32_t nFrameIndex, const UiSize& szDestRectSize)
{
    PerformanceStat statPerformance(_T("ImageInfo::GetFrame"));
    GlobalManager::Instance().AssertUIThread();
    std::shared_ptr<IAnimationImage> pAnimationImage = GetAnimationImage(nFrameIndex);
    ASSERT(pAnimationImage != nullptr);
    if (pAnimationImage == nullptr) {
        //动画图片接口异常
        return nullptr;
    }
    const uint32_t nFrameCount = GetFrameCount();
    ASSERT(nFrameCount > 0);
    if (nFrameCount == 0) {
        //动画图片帧数异常
        return nullptr;
    }

    AnimationFramePtr pAnimationFrame;
    pAnimationFrame = std::make_shared<IAnimationImage::AnimationFrame>();
    if (pAnimationImage->ReadFrameData(nFrameIndex, szDestRectSize, pAnimationFrame.get())) {
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
            /*uint32_t nNewWidth = pAnimationFrame->m_pBitmap->GetWidth();
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
            }*/
        }
    }
    return pAnimationFrame;
}

bool ImageInfo::IsFrameDataReady(uint32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    std::shared_ptr<IAnimationImage> pAnimationImage = GetAnimationImage(nFrameIndex);
    ASSERT(pAnimationImage != nullptr);
    if (pAnimationImage != nullptr) {
        return pAnimationImage->IsFrameDataReady(nFrameIndex);
    }
    return false;
}

int32_t ImageInfo::GetFrameDelayMs(uint32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    std::shared_ptr<IAnimationImage> pAnimationImage = GetAnimationImage(nFrameIndex);
    ASSERT(pAnimationImage != nullptr);
    if (pAnimationImage != nullptr) {
        return pAnimationImage->GetFrameDelayMs(nFrameIndex);
    }
    //避免返回0，否则外部的业务逻辑有问题
    return IMAGE_ANIMATION_DELAY_MS;
}

bool ImageInfo::SetImageData(const ImageLoadParam& loadParam,
                             const std::shared_ptr<IImage>& pImageData,                             
                             bool bImageDpiScaleEnabled,
                             uint32_t nImageFileDpiScale)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return false;
    }
    ASSERT(nImageFileDpiScale > 0);
    if (nImageFileDpiScale == 0) {
        return false;
    }

    //计算ImageInfo的大小
    int32_t nImageInfoWidth = 0;
    int32_t nImageInfoHeight = 0;
    CalcImageInfoSize(loadParam, pImageData, bImageDpiScaleEnabled, nImageFileDpiScale, nImageInfoWidth, nImageInfoHeight);

    ASSERT((nImageInfoWidth > 0) && (nImageInfoHeight > 0));
    if ((nImageInfoWidth <= 0) || (nImageInfoHeight <= 0)) {
        return false;
    }

    const int32_t nImageWidth = pImageData->GetWidth();
    const int32_t nImageHeight = pImageData->GetWidth();
    ASSERT((nImageWidth > 0) && (nImageHeight > 0));
    if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
        return false;
    }
    
    //设置宽度和高度
    m_nImageInfoWidth = nImageInfoWidth;
    m_nImageInfoHeight = nImageInfoHeight;
    m_fImageSizeScale = pImageData->GetImageSizeScale();
    m_bEnableImageDpiScale = bImageDpiScaleEnabled;
    m_nImageFileDpiScale = nImageFileDpiScale;

    m_bHasCustomSizeScale = false;
    m_fCustomSizeScaleX = 1.0f;
    m_fCustomSizeScaleY = 1.0f;
    if (nImageInfoWidth != nImageWidth) {
        m_fCustomSizeScaleX = static_cast<float>(nImageInfoWidth) / nImageWidth;
        m_bHasCustomSizeScale = true;
    }
    if (nImageInfoHeight != nImageHeight) {
        m_fCustomSizeScaleY = static_cast<float>(nImageInfoHeight) / nImageHeight;
        m_bHasCustomSizeScale = true;
    }

    m_loadParam = loadParam;
    m_pImageData = pImageData;    

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
    }
    else {
        ASSERT(0);
        return false;
    }
    return true;
}

void ImageInfo::CalcImageInfoSize(const ImageLoadParam& loadParam,
                                  const std::shared_ptr<IImage>& pImageData,
                                  bool bImageDpiScaleEnabled,
                                  uint32_t nImageFileDpiScale,
                                  int32_t& nImageInfoWidth,
                                  int32_t& nImageInfoHeight) const
{
    nImageInfoWidth = 0;
    nImageInfoHeight = 0;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return;
    }
    ASSERT(nImageFileDpiScale > 0);
    if (nImageFileDpiScale == 0) {
        return;
    }
    if (!bImageDpiScaleEnabled) {
        ASSERT(nImageFileDpiScale == 100);
        if (nImageFileDpiScale != 100) {
            return;
        }
    }

    //计算ImageInfo的宽度和高度（注意：与图片的实际宽度和高度值可能不相同）
    //ImageInfo的宽度和高度: 影响布局
    //图片的宽度和高度：用于绘制
    nImageInfoWidth = pImageData->GetWidth();
    nImageInfoHeight = pImageData->GetHeight();
    ASSERT((nImageInfoWidth > 0) && (nImageInfoHeight > 0));
    if ((nImageInfoWidth <= 0) || (nImageInfoHeight <= 0)) {
        return;
    }    
    if (bImageDpiScaleEnabled && (nImageFileDpiScale != 100) && ImageUtil::IsValidImageScale(pImageData->GetImageSizeScale())) {
        const float fRealImageSizeScale = pImageData->GetImageSizeScale(); //实际加载的缩放比例(此值与加载时传入的缩放比例不一定相同)
        //举例：原图文件为"autumn.png"，如果匹配到DPI自适应图文件名为"autumn@175.png"，此时nImageFileDpiScale的值就是175
        const float fSizeScale = static_cast<float>(loadParam.GetLoadDpiScale()) / 100.0f;
        //用的是图片自适应图片（非原图），需要用原图大小来计算ImageInfo大小
        int32_t nImageOrgWidth = static_cast<int32_t>(nImageInfoWidth * 1.0f / fRealImageSizeScale + 0.5f);
        nImageOrgWidth = static_cast<int32_t>(nImageOrgWidth * 100.0f / nImageFileDpiScale + 0.5f);    //原图大小
        nImageOrgWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageOrgWidth, fSizeScale); //用原图大小，重新计算ImageInfo的大小

        int32_t nImageOrgHeight = static_cast<int32_t>(nImageInfoHeight * 1.0f / fRealImageSizeScale + 0.5f);
        nImageOrgHeight = static_cast<int32_t>(nImageOrgHeight * 100.0f / nImageFileDpiScale + 0.5f);    //原图大小
        nImageOrgHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageOrgHeight, fSizeScale); //用原图大小，重新计算ImageInfo的大小

        if ((nImageOrgWidth > 0) && (nImageOrgHeight > 0)) {
            nImageInfoWidth = nImageOrgWidth;
            nImageInfoHeight = nImageOrgHeight;
        }
    }

    //计算设置的比例, 影响加载的缩放百分比（通过width='300'或者width='300%'这种形式设置的图片属性）
    uint32_t nImageFixedWidth = 0;
    uint32_t nImageFixedHeight = 0;
    const bool bHasFixedSize = loadParam.GetImageFixedSize(nImageFixedWidth, nImageFixedHeight); //绝对数值，已经做过DPI自适应

    float fImageFixedWidthPercent = 1.0f;
    float fImageFixedHeightPercent = 1.0f;
    const bool bHasFixedPercent = loadParam.GetImageFixedPercent(fImageFixedWidthPercent, fImageFixedHeightPercent);//百分比

    if (bHasFixedSize || bHasFixedPercent) {
        //有设置图片属性：通过width='300'或者width='300%'这种形式设置的图片属性
        bool bFixedWidthSet = (nImageFixedWidth > 0) || ImageUtil::NeedResizeImage(fImageFixedWidthPercent);
        bool bFixedHeightSet = (nImageFixedHeight > 0) || ImageUtil::NeedResizeImage(fImageFixedHeightPercent);
        if (bFixedWidthSet && bFixedHeightSet) {
            //宽度和高度均设置
            if (nImageFixedWidth > 0) {
                nImageInfoWidth = nImageFixedWidth;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedWidthPercent)) {
                nImageInfoWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fImageFixedWidthPercent);
            }

            if (nImageFixedHeight > 0) {
                nImageInfoHeight = nImageFixedHeight;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedHeightPercent)) {
                nImageInfoHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fImageFixedHeightPercent);
            }
        }
        else if (bFixedWidthSet) {
            //只设置了宽度，高度同比例缩放
            int32_t nOldImageInfoWidth = nImageInfoWidth;
            if (nImageFixedWidth > 0) {
                nImageInfoWidth = nImageFixedWidth;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedWidthPercent)) {
                nImageInfoWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fImageFixedWidthPercent);
            }
            float fNewScale = static_cast<float>(nImageInfoWidth) / nOldImageInfoWidth;
            nImageInfoHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fNewScale);
        }
        else if (bFixedHeightSet) {
            //只设置了高度，宽度同比例缩放
            int32_t nOldImageInfoHeight = nImageInfoHeight;
            if (nImageFixedHeight > 0) {
                nImageInfoHeight = nImageFixedHeight;
            }
            else if (ImageUtil::NeedResizeImage(fImageFixedHeightPercent)) {
                nImageInfoHeight = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoHeight, fImageFixedHeightPercent);
            }
            float fNewScale = static_cast<float>(nImageInfoHeight) / nOldImageInfoHeight;
            nImageInfoWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageInfoWidth, fNewScale);
        }
    }
}

void ImageInfo::ScaleImageSourceRect(const DpiManager& dpi, UiRect& rcDestCorners, UiRect& rcSource, UiRect& rcSourceCorners)
{
    int32_t nImageInfoWidth = GetWidth();
    int32_t nImageInfoHeight = GetHeight();
    ASSERT((nImageInfoWidth > 0) && (nImageInfoHeight > 0));
    if ((nImageInfoWidth == 0) || (nImageInfoHeight == 0)) {
        return;
    }

    //对rcSourceCorners进行处理：对边角值进行容错处理（四个边代表边距，不代表矩形区域）
    //在XML解析加载的时候，未做DPI自适应；
    //在绘制的时候，如果图片做过DPI自适应，也要做DPI自适应，如果图片未做DPI自适应，也不需要做。    
    if ((rcSourceCorners.left < 0) || (rcSourceCorners.top < 0) ||
        (rcSourceCorners.right < 0) || (rcSourceCorners.bottom < 0)) {
        rcSourceCorners.Clear();
    }
    else if (m_bEnableImageDpiScale) {
        dpi.ScaleRect(rcSourceCorners);
    }

    //对rcDestCorners进行处理：由rcSourceCorners赋值，边角保持一致，避免绘制图片的时候四个角有变形；
    //采用九宫格绘制的时候，四个角的存在，是为了避免绘制的时候四个角出现变形
    rcDestCorners = rcSourceCorners;

    // 如果源位图已经按照DPI缩放过，那么对应的rcImageSource也需要缩放
    if ((rcSource.left < 0) || (rcSource.top < 0) ||
        (rcSource.right < 0) || (rcSource.bottom < 0) ||
        (rcSource.Width() <= 0) || (rcSource.Height() <= 0)) {
        //如果是无效值，则重置为整个图片大小
        rcSource.left = 0;
        rcSource.top = 0;
        rcSource.right = (int32_t)nImageInfoWidth;
        rcSource.bottom = (int32_t)nImageInfoHeight;
    }
    else if (m_bEnableImageDpiScale) {
        //如果外部设置此值，做DPI自适应处理
        dpi.ScaleRect(rcSource);
    }

    //图片源容错处理
    if (rcSource.right > (int32_t)nImageInfoWidth) {
        rcSource.right = (int32_t)nImageInfoWidth;
    }
    if (rcSource.bottom > (int32_t)nImageInfoHeight) {
        rcSource.bottom = (int32_t)nImageInfoHeight;
    }
}

void ImageInfo::ScaleImageSourceRect(const DpiManager& dpi, UiRect& rcSource)
{
    UiRect rcDestCorners;
    UiRect rcSourceCorners;
    ScaleImageSourceRect(dpi, rcDestCorners, rcSource, rcSourceCorners);
}

std::shared_ptr<IImage> ImageInfo::GetImageData() const
{
    return m_pImageData;
}

int32_t ImageInfo::GetWidth() const
{
    return m_nImageInfoWidth;
}

int32_t ImageInfo::GetHeight() const
{
    return m_nImageInfoHeight;
}

float ImageInfo::GetImageSizeScale() const
{
    return m_fImageSizeScale;
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

uint32_t ImageInfo::GetLoadDpiScale() const
{
    return m_loadParam.GetLoadDpiScale();
}

uint32_t ImageInfo::GetImageFileDpiScale() const
{
    return m_nImageFileDpiScale;
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
