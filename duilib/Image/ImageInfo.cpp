#include "ImageInfo.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Callback.h"
#include "duilib/Utils/PerformanceUtil.h"
#include <cmath>
#include <cstdlib>

namespace ui 
{

/** 位图缩放在UI线程内联执行的像素数上限：
*   超过该值时，把缩放放到图片解码线程执行，避免大图缩放阻塞UI线程
*/
constexpr uint32_t kInlineResizeMaxPixelCount = 512 * 512;

/** 位图尺寸与ImageInfo尺寸允许的差异(像素)：在该范围内认为尺寸一致，避免为取整差异做整图缩放
*/
constexpr int32_t kResizeSizeTolerance = 1;

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
    m_nLastFrameIndex((uint32_t)-1),
    m_fImageSizeScale(1.0f),
    m_bResizeBitmapPending(false),
    m_bResizeBitmapError(false),
    m_nResizeRequestId(0),
    m_pWeakFlag(std::make_shared<WeakFlag>())
{
}

ImageInfo::~ImageInfo()
{
    //延迟释放原图
    ReleaseImage();
}

void ImageInfo::ReleaseImage()
{
    if (m_pImageData != nullptr) {
        DString imageFullPath = m_loadParam.GetImageLoadPath().m_imageFullPath.ToString();
        GlobalManager::Instance().Image().ReleaseImage(m_pImageData, imageFullPath);
        m_pImageData.reset();
    }
}

bool ImageInfo::IsSvgImage() const
{
    return (m_imageType == ImageType::kImageSvg);
}

std::shared_ptr<IBitmap> ImageInfo::GetSvgBitmap(const UiRect& rcDest, UiRect& rcSource, Control* pControl)
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
    if ((rcSource.Width() <= 0) || (rcSource.Height() <= 0)) {
        return nullptr;
    }
    ASSERT((pSvgImage->GetWidth() > 0) && (pSvgImage->GetHeight() > 0));
    if ((pSvgImage->GetWidth() == 0) || (pSvgImage->GetHeight() == 0)) {
        return nullptr;
    }
    ASSERT((m_nImageInfoWidth > 0) && (m_nImageInfoHeight > 0));
    if ((m_nImageInfoWidth <= 0) || (m_nImageInfoHeight <= 0)) {
        return nullptr;
    }

    float fSizeScaleX = static_cast<float>(rcDest.Width()) / rcSource.Width();
    float fSizeScaleY = static_cast<float>(rcDest.Height()) / rcSource.Height();
    //取两个方向中较大的缩放比例：保证栅格化后的位图在两个方向上都不低于绘制所需的分辨率，
    //避免非等比拉伸时按较小比例栅格化、再被放大绘制而导致的模糊
    float fImageSizeScale = fSizeScaleX > fSizeScaleY ? fSizeScaleX  : fSizeScaleY ;
    std::shared_ptr<IBitmap> pBitmap = GetSvgBitmap(fImageSizeScale, pControl);
    return pBitmap;
}

std::shared_ptr<IBitmap> ImageInfo::GetSvgBitmap(float fImageSizeScale, Control* pControl)
{
    GlobalManager::Instance().AssertUIThread();
    //SVG图片：位图缓存由SVG图片对象内部按尺寸维护
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        return nullptr;
    }
    ASSERT((m_nImageInfoWidth > 0) && (m_nImageInfoHeight > 0));
    if ((m_nImageInfoWidth <= 0) || (m_nImageInfoHeight <= 0)) {
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

        SvgReplaceColorCallbackFunction svgReplaceColorCallback;
        if (pControl != nullptr) {
            std::weak_ptr<WeakFlag> controlFlag = pControl->GetWeakFlag();
            svgReplaceColorCallback = [controlFlag, pControl](const DString& strColor) {
                UiColor color;
                if (!controlFlag.expired() && (pControl != nullptr)) {
                    color = pControl->GetUiColor(strColor);
                }
                return color;
                };
        }
        pBitmap = pSvgImage->GetBitmap(UiSize(nWidth, nHeight), svgReplaceColorCallback);
    }
    return pBitmap;
}

std::shared_ptr<IBitmap> ImageInfo::GetBitmap(bool* bDecodeError, Control* pControl)
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
        std::shared_ptr<IBitmap> pBitmap = GetSvgBitmap(IMAGE_SIZE_SCALE_NONE, pControl);
        if (pBitmap == nullptr) {
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
        }
        return pBitmap;
    }
    else {
        if (bDecodeError != nullptr) {
            *bDecodeError = true;
        }
        ASSERT(0);
        return nullptr;
    }

    //设置图片内部数据(延迟解码)
    std::shared_ptr<IImage> pImageData = m_pImageData;
    ASSERT(pImageData != nullptr);
    if (pImageData == nullptr) {
        if (bDecodeError != nullptr) {
            *bDecodeError = true;
        }
        return nullptr;
    }

    ImageType imageType = pImageData->GetImageType();
    if (imageType == ImageType::kImageBitmap) {
        //异步缩放位图进行中：返回nullptr且不设置解码错误，绘制模块会登记延迟绘制，等待缩放完成后的重绘通知
        if (m_bResizeBitmapPending) {
            return nullptr;
        }
        if (m_bResizeBitmapError) {
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
            return nullptr;
        }
        std::shared_ptr<IBitmapImage> pBitmapImage = pImageData->GetImageBitmap();
        ASSERT(pBitmapImage != nullptr);
        if (pBitmapImage == nullptr) {
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
            return nullptr;
        }
        std::shared_ptr<IBitmap> pBitmap = pBitmapImage->GetBitmap(bDecodeError);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        const int32_t nBitmapWidth = (int32_t)pBitmap->GetWidth();
        const int32_t nBitmapHeight = (int32_t)pBitmap->GetHeight();
        if ((std::abs(nBitmapWidth - m_nImageInfoWidth) <= kResizeSizeTolerance) &&
            (std::abs(nBitmapHeight - m_nImageInfoHeight) <= kResizeSizeTolerance)) {
            //尺寸一致(允许取整差异时)：直接使用，避免为像素级的取整差异做整图缩放
            m_pBitmap = pBitmap;
        }
        else if (NeedAsyncResizeBitmap(pBitmap) && RequestAsyncResizeBitmap(pBitmap)) {
            //大图缩放已经提交到图片解码线程，避免在UI线程执行耗时操作
            return nullptr;
        }
        else {
            //若大小不一致，生成缓存位图后，释放原图
            m_pBitmap = ImageUtil::ResizeImageBitmap(pBitmap.get(), m_nImageInfoWidth, m_nImageInfoHeight);
            ASSERT(m_pBitmap != nullptr);
            ReleaseImage();
            if (m_pBitmap == nullptr) {
                if (bDecodeError != nullptr) {
                    *bDecodeError = true;
                }
            }
        }
        return m_pBitmap;
    }
    else {
        //未知错误
        if (bDecodeError != nullptr) {
            *bDecodeError = true;
        }
        ASSERT(0);
    }
    return nullptr;
}

bool ImageInfo::NeedAsyncResizeBitmap(const std::shared_ptr<IBitmap>& pBitmap) const
{
    GlobalManager::Instance().AssertUIThread();
    if (pBitmap == nullptr) {
        return false;
    }
    //位图像素较多时，缩放开销较大，放到图片解码线程执行
    const uint64_t nPixelCount = (uint64_t)pBitmap->GetWidth() * (uint64_t)pBitmap->GetHeight();
    return nPixelCount > kInlineResizeMaxPixelCount;
}

bool ImageInfo::RequestAsyncResizeBitmap(const std::shared_ptr<IBitmap>& pBitmap)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return false;
    }
    m_bResizeBitmapPending = true;
    m_bResizeBitmapError = false;
    const uint32_t nRequestId = ++m_nResizeRequestId;

    const int32_t nWidth = m_nImageInfoWidth;
    const int32_t nHeight = m_nImageInfoHeight;
    const DString imageKey = GetImageKey();
    std::weak_ptr<WeakFlag> weakFlag = m_pWeakFlag;
    ImageInfo* pThis = this;

    //在图片解码线程中执行缩放，完成后回到UI线程更新缓存
    auto resizeBitmapFunction = [weakFlag, pThis, nRequestId, imageKey, nWidth, nHeight, pBitmap]() {
        std::shared_ptr<IBitmap> pNewBitmap = ImageUtil::ResizeImageBitmap(pBitmap.get(), nWidth, nHeight);
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
                                                    [weakFlag, pThis, nRequestId, imageKey, pNewBitmap]() {
            if (weakFlag.expired()) {
                //ImageInfo对象已经销毁，丢弃缩放结果
                return;
            }
            pThis->OnAsyncResizeBitmapFinished(nRequestId, imageKey, pNewBitmap);
        });
    };

    //选择图片解码线程(与图片解码使用同一组线程，轮询均衡负载)，不可用时回退到工作线程
    GlobalManager& globalManager = GlobalManager::Instance();
    int32_t nThreadIdentifier = globalManager.GetNextImageDecodeThread();
    if ((nThreadIdentifier == ui::kThreadNone) && globalManager.Thread().HasThread(ui::kThreadWorker)) {
        nThreadIdentifier = ui::kThreadWorker;
    }
    size_t nTaskId = 0;
    if (nThreadIdentifier != ui::kThreadNone) {
        nTaskId = globalManager.Thread().PostTask(nThreadIdentifier, resizeBitmapFunction);
    }
    if (nTaskId == 0) {
        //没有可用的子线程：由调用方在UI线程同步处理
        m_bResizeBitmapPending = false;
        return false;
    }
    return true;
}

void ImageInfo::OnAsyncResizeBitmapFinished(uint32_t nRequestId,
                                            const DString& imageKey,
                                            const std::shared_ptr<IBitmap>& pBitmap)
{
    GlobalManager::Instance().AssertUIThread();
    if (!m_bResizeBitmapPending || (nRequestId != m_nResizeRequestId)) {
        //过期结果(图片数据已经变化)，丢弃
        return;
    }
    m_bResizeBitmapPending = false;
    if (pBitmap != nullptr) {
        m_pBitmap = pBitmap;
    }
    else {
        m_bResizeBitmapError = true;
    }
    //与同步缩放路径一致：生成缓存位图后，释放原图
    ReleaseImage();
    //通知界面重绘(此前绘制时已经登记了延迟绘制)
    GlobalManager::Instance().Image().DelayPaintImage(imageKey);
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
    ASSERT(nFrameIndex < nFrameCount);
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
#if DUILIB_PERFORMANCE_STAT_ENABLED
    //性能统计
    static size_t statNameHash = 0;
    if (statNameHash == 0) {
        DString statName = _T("ImageInfo::GetFrame");
        statNameHash = std::hash<DString>{}(statName);
        PerformanceUtilHelper::Instance().AddStat(statName);
    }
    PerformanceUtilFast statPerformance(statNameHash);
#endif //  DUILIB_PERFORMANCE_STAT_ENABLED

    GlobalManager::Instance().AssertUIThread();

    //复用上一帧数据：帧号与目标区域大小均未变化时，直接返回缓存对象，避免重复读取帧数据与创建对象
    if ((m_pLastFrame != nullptr) &&
        (m_nLastFrameIndex == nFrameIndex) &&
        (m_lastFrameDestSize == szDestRectSize)) {
        return m_pLastFrame;
    }

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
    if ((m_pPendingFrame != nullptr) &&
        (m_pPendingFrame->m_nFrameIndex == (int32_t)nFrameIndex) &&
        (m_pendingFrameDestSize == szDestRectSize)) {
        //该帧的数据仍在异步解码中，复用等待中的帧对象，避免每次绘制都重新分配
        pAnimationFrame = m_pPendingFrame;
    }
    else {
        pAnimationFrame = std::make_shared<IAnimationImage::AnimationFrame>();
    }
    if (pAnimationImage->ReadFrameData(nFrameIndex, szDestRectSize, pAnimationFrame.get())) {
        pAnimationFrame->m_nFrameIndex = nFrameIndex;
        if (pAnimationFrame->m_bDataPending) {
            //数据解码未完成，直接返回（下次仍需重新读取数据，但复用该帧对象等待解码完成）
            ASSERT(pAnimationFrame->m_pBitmap == nullptr);
            m_pPendingFrame = pAnimationFrame;
            m_pendingFrameDestSize = szDestRectSize;
            m_pLastFrame.reset();
            m_nLastFrameIndex = (uint32_t)-1;
            return pAnimationFrame;
        }
        //数据已就绪，清除等待中的帧对象缓存
        m_pPendingFrame.reset();
        ASSERT(pAnimationFrame->GetDelayMs() > 0);
        ASSERT(pAnimationFrame->m_pBitmap != nullptr);
        if (!pAnimationFrame->m_bDataError && (pAnimationFrame->m_pBitmap != nullptr)) {
            //数据读取完成，缓存该帧数据，供下次绘制时复用
            m_pLastFrame = pAnimationFrame;
            m_nLastFrameIndex = nFrameIndex;
            m_lastFrameDestSize = szDestRectSize;
        }
    }
    if (m_pLastFrame != pAnimationFrame) {
        //未缓存本次读取的帧，需要清除旧的缓存数据
        m_pLastFrame.reset();
        m_nLastFrameIndex = (uint32_t)-1;
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
    const int32_t nImageHeight = pImageData->GetHeight();
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

    //图片数据变化后，之前缓存的图片数据、位图和动画帧对象都不再有效，需要清除
    ReleaseImage(); //延迟释放旧的图片数据(必须在更新m_loadParam之前调用)
    m_pBitmap.reset();
    //使正在执行的异步缩放任务结果失效
    m_bResizeBitmapPending = false;
    m_bResizeBitmapError = false;
    ++m_nResizeRequestId;
    m_pLastFrame.reset();
    m_pPendingFrame.reset();
    m_nLastFrameIndex = (uint32_t)-1;

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
        //禁用图片的DPI自适应时，如果加载的是DPI变体图片文件，则直接使用该图片文件的实际大小，不再按DPI换算
        ASSERT(nImageFileDpiScale == 100);
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
        int32_t nImageOrgWidth = static_cast<int32_t>(std::ceil(nImageInfoWidth * 1.0f / fRealImageSizeScale));
        nImageOrgWidth = static_cast<int32_t>(std::ceil(nImageOrgWidth * 100.0f / nImageFileDpiScale));    //原图大小
        nImageOrgWidth = (int32_t)ImageUtil::GetScaledImageSize((uint32_t)nImageOrgWidth, fSizeScale); //用原图大小，重新计算ImageInfo的大小

        int32_t nImageOrgHeight = static_cast<int32_t>(std::ceil(nImageInfoHeight * 1.0f / fRealImageSizeScale));
        nImageOrgHeight = static_cast<int32_t>(std::ceil(nImageOrgHeight * 100.0f / nImageFileDpiScale));    //原图大小
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

void ImageInfo::ScaleImageSourceRect(const DpiManager& dpi, UiRect& rcSource, UiRect& rcSourceCorners)
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
    UiRect rcSourceCorners;
    ScaleImageSourceRect(dpi, rcSource, rcSourceCorners);
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
