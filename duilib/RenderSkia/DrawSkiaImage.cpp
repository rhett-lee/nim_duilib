#include "DrawSkiaImage.h"

//#include "duilib/Utils/PerformanceUtil.h"

#pragma warning (push)
#pragma warning (disable: 4505)
    #define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
    #include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

#include "SkiaHeaderBegin.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSurface.h"
#include "SkiaHeaderEnd.h"

#include <cstdint>
#include <cstring>
#include <list>
#include <mutex>
#include <unordered_map>

//定义DUILIB_HAVE_OPENCV这个宏，表示启用OpenCV，使用OpenCV对图片进行缩放，速度是最快的
//如果启用，需要做如下处理：
//1. 在duilib的工程里，添加opencv库的安装路径根目录到VC工程的include目录中
//2. 在可执行程序的工程里，需要在链接时，添加如下lib：opencv_core4110.lib;opencv_imgproc4110.lib（以上是opencv 4.11版本的库，其他版本库的lib名称会不一样）

//#define DUILIB_HAVE_OPENCV 1

#ifdef DUILIB_HAVE_OPENCV
    #pragma warning (push)
    #pragma warning (disable: 4127)
        #include "opencv2/opencv.hpp"
    #pragma warning (pop)
#endif

//#ifdef DUILIB_HAVE_OPENCV
//    #ifdef _DEBUG
//        #pragma comment (lib, "opencv_core4110d.lib")
//        #pragma comment (lib, "opencv_imgproc4110d.lib")
//    #else
//        #pragma comment (lib, "opencv_core4110.lib")
//        #pragma comment (lib, "opencv_imgproc4110.lib")
//    #endif
//#endif

namespace ui 
{

/** 图片缩放结果缓存的实现
*
* 背景：Skia 的 drawImageRect 每次绘制都会重新采样，图片从大尺寸缩到显示尺寸时开销较大
*       （首页大图缩到 100~200 px，实测约 0.05 ms/张/帧）。由于同一个图片的显示尺寸在
*       多帧之间通常不变，可以缓存"已缩放到目标尺寸"的 SkImage，绘制时按 1:1 拷贝，
*       该优化对 Raster/GL/SDL 各后端通用（GL 后端还避免了重复的光栅化上传前采样）。
*
* 失效策略：缓存 key 包含源 SkImage 的 uniqueID（内容变化即生成新的 SkImage，uniqueID 变化）、
*           目标宽高（控件尺寸/DPI 变化即变化）与采样参数，因此无需显式失效逻辑；
*           缓存按字节数 + 条目数做 LRU 淘汰，内存占用有上限。
*/
namespace
{
/** 缓存占用的总字节数上限（超过后按 LRU 淘汰） */
constexpr size_t kImageScaleCacheMaxBytes = 64 * 1024 * 1024;

/** 缓存条目数上限 */
constexpr size_t kImageScaleCacheMaxEntries = 512;

/** 准入记录的条目数上限（记录"首次出现"的key，用于避免一次性绘制的内容进入缓存） */
constexpr size_t kImageScaleCacheMaxPendingKeys = 256;

/** 单个缓存条目允许的最大字节数（超过则不缓存，避免一张大图把缓存挤空） */
constexpr size_t kImageScaleCacheMaxEntryBytes = kImageScaleCacheMaxBytes / 4u;

/** 源图片像素数小于该值时，缩放本身很快，不进入缓存（避免图标类小图污染缓存） */
constexpr int64_t kImageScaleCacheMinSrcPixels = 64 * 64;

/** 缓存key：源图片 + 目标宽高 + 采样参数 */
struct ImageScaleCacheKey
{
    uint32_t nImageID = 0;
    int32_t nWidth = 0;
    int32_t nHeight = 0;
    uint32_t nSamplingKey = 0;

    bool operator == (const ImageScaleCacheKey& r) const
    {
        return (nImageID == r.nImageID) && (nWidth == r.nWidth) &&
               (nHeight == r.nHeight) && (nSamplingKey == r.nSamplingKey);
    }
};

struct ImageScaleCacheKeyHash
{
    size_t operator()(const ImageScaleCacheKey& key) const
    {
        size_t nHash = std::hash<uint32_t>()(key.nImageID);
        nHash = nHash * 31 + std::hash<int32_t>()(key.nWidth);
        nHash = nHash * 31 + std::hash<int32_t>()(key.nHeight);
        nHash = nHash * 31 + std::hash<uint32_t>()(key.nSamplingKey);
        return nHash;
    }
};

/** 把采样参数编码成整数，用于缓存key */
uint32_t MakeSamplingKey(const SkSamplingOptions& sampling)
{
    uint32_t nKey = ((uint32_t)sampling.filter) & 0x03u;
    nKey |= (((uint32_t)sampling.mipmap) & 0x03u) << 2;
    if (sampling.useCubic) {
        nKey |= 0x10u;
        uint32_t nB = 0;
        uint32_t nC = 0;
        static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32-bit");
        ::memcpy(&nB, &sampling.cubic.B, sizeof(uint32_t));
        ::memcpy(&nC, &sampling.cubic.C, sizeof(uint32_t));
        nKey ^= (nB * 0x9E3779B9u) ^ (nC * 0x85EBCA6Bu);
    }
    return nKey;
}

struct ImageScaleCacheEntry
{
    sk_sp<SkImage> pImage;
    size_t nBytes = 0;
    std::list<ImageScaleCacheKey>::iterator lruIt;
};

/** 图片缩放结果的 LRU 缓存（支持多线程并发访问，绘制可能发生在多个UI线程） */
class ImageScaleCache
{
public:
    /** 获取进程内唯一实例（故意不析构，避免静态析构顺序问题） */
    static ImageScaleCache& Instance();

    bool IsEnabled() const;
    void Enable(bool bEnable);

    /** 查找缓存（命中时更新 LRU 顺序），未命中返回 nullptr */
    sk_sp<SkImage> Find(const ImageScaleCacheKey& key);

    /** 准入判断：同一个key首次出现时只记录、不缓存（一次性绘制的内容不产生额外开销），
    *   第二次出现时才允许创建缓存。返回 true 表示允许创建缓存。
    */
    bool Admit(const ImageScaleCacheKey& key);

    /** 插入缓存（已存在相同 key 时不重复插入） */
    void Insert(const ImageScaleCacheKey& key, const sk_sp<SkImage>& pImage);

    void Clear();

    size_t GetEntryCount() const;
    size_t GetHitCount() const;

private:
    ImageScaleCache() = default;
    ImageScaleCache(const ImageScaleCache&) = delete;
    ImageScaleCache& operator = (const ImageScaleCache&) = delete;

    /** 淘汰超出预算的缓存条目（调用方需持有锁） */
    void EvictIfNeeded();

private:
    mutable std::mutex m_mutex;
    std::unordered_map<ImageScaleCacheKey, ImageScaleCacheEntry, ImageScaleCacheKeyHash> m_entries;
    std::list<ImageScaleCacheKey> m_lruList;    //头部为最近使用
    std::unordered_map<ImageScaleCacheKey, std::list<ImageScaleCacheKey>::iterator, ImageScaleCacheKeyHash> m_pendingKeys;
    std::list<ImageScaleCacheKey> m_pendingOrder; //准入记录的FIFO顺序（头部为最近出现）
    size_t m_nTotalBytes = 0;
    size_t m_nHitCount = 0;
    bool m_bEnabled = true;
};

ImageScaleCache& ImageScaleCache::Instance()
{
    static ImageScaleCache* pCache = new ImageScaleCache();
    return *pCache;
}

bool ImageScaleCache::IsEnabled() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bEnabled;
}

void ImageScaleCache::Enable(bool bEnable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bEnabled = bEnable;
    if (!bEnable) {
        m_entries.clear();
        m_lruList.clear();
        m_pendingKeys.clear();
        m_pendingOrder.clear();
        m_nTotalBytes = 0;
    }
}

sk_sp<SkImage> ImageScaleCache::Find(const ImageScaleCacheKey& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_bEnabled) {
        return nullptr;
    }
    auto it = m_entries.find(key);
    if (it == m_entries.end()) {
        return nullptr;
    }
    ++m_nHitCount;
    m_lruList.splice(m_lruList.begin(), m_lruList, it->second.lruIt);
    return it->second.pImage;
}

bool ImageScaleCache::Admit(const ImageScaleCacheKey& key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_bEnabled) {
        return false;
    }
    auto it = m_pendingKeys.find(key);
    if (it != m_pendingKeys.end()) {
        //第二次出现：允许创建缓存，并移除准入记录
        m_pendingOrder.erase(it->second);
        m_pendingKeys.erase(it);
        return true;
    }
    m_pendingOrder.push_front(key);
    m_pendingKeys.emplace(key, m_pendingOrder.begin());
    while (m_pendingOrder.size() > kImageScaleCacheMaxPendingKeys) {
        const ImageScaleCacheKey& oldestKey = m_pendingOrder.back();
        m_pendingKeys.erase(oldestKey);
        m_pendingOrder.pop_back();
    }
    return false;
}

void ImageScaleCache::Insert(const ImageScaleCacheKey& key, const sk_sp<SkImage>& pImage)
{
    if ((pImage == nullptr) || (pImage->width() <= 0) || (pImage->height() <= 0)) {
        return;
    }
    const size_t nBytes = (size_t)pImage->width() * (size_t)pImage->height() * 4u;
    //单张图片就超过预算的 1/4 时不缓存，避免一张大图把缓存挤空
    if (nBytes > kImageScaleCacheMaxEntryBytes) {
        return;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_bEnabled) {
        return;
    }
    if (m_entries.find(key) != m_entries.end()) {
        return;
    }
    m_lruList.push_front(key);
    ImageScaleCacheEntry entry;
    entry.pImage = pImage;
    entry.nBytes = nBytes;
    entry.lruIt = m_lruList.begin();
    m_entries.emplace(key, std::move(entry));
    m_nTotalBytes += nBytes;
    EvictIfNeeded();
}

void ImageScaleCache::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_entries.clear();
    m_lruList.clear();
    m_nTotalBytes = 0;
    m_nHitCount = 0;
    m_pendingKeys.clear();
    m_pendingOrder.clear();
}

size_t ImageScaleCache::GetEntryCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_entries.size();
}

size_t ImageScaleCache::GetHitCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_nHitCount;
}

void ImageScaleCache::EvictIfNeeded()
{
    while ((!m_entries.empty()) &&
           ((m_entries.size() > kImageScaleCacheMaxEntries) || (m_nTotalBytes > kImageScaleCacheMaxBytes))) {
        const ImageScaleCacheKey& key = m_lruList.back();
        auto it = m_entries.find(key);
        if (it != m_entries.end()) {
            m_nTotalBytes -= it->second.nBytes;
            m_entries.erase(it);
        }
        m_lruList.pop_back();
    }
}

/** 判断画布当前变换是否为"无缩放且为整数平移"：
*   存在缩放/旋转时，预缩放的结果还会被二次采样，质量与直接绘制不一致，故不启用缓存。
*/
bool IsCanvasUnscaled(const SkCanvas* pSkCanvas)
{
    const SkMatrix matrix = pSkCanvas->getLocalToDeviceAs3x3();
    return matrix.isScaleTranslate() &&
           (matrix.getScaleX() == 1.0f) && (matrix.getScaleY() == 1.0f) &&
           SkScalarIsInt(matrix.getTranslateX()) && SkScalarIsInt(matrix.getTranslateY());
}

/** 把整幅源图片缩放到目标尺寸，生成新的 SkImage（失败返回 nullptr）
*   说明：缩放时使用 kSrc 混合作图，Alpha/混合等 paint 属性保留在绘制阶段应用，
*         这样同一个缓存条目可被不同的 uFade 复用。
*/
sk_sp<SkImage> CreateScaledSkImage(const sk_sp<SkImage>& skSrcImage,
                                   int32_t nDestWidth, int32_t nDestHeight,
                                   const SkSamplingOptions& sampling)
{
    sk_sp<SkSurface> pSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(nDestWidth, nDestHeight));
    if (pSurface == nullptr) {
        return nullptr;
    }
    SkCanvas* pCanvas = pSurface->getCanvas();
    if (pCanvas == nullptr) {
        return nullptr;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    const SkRect rcSrc = SkRect::MakeIWH(skSrcImage->width(), skSrcImage->height());
    const SkRect rcDst = SkRect::MakeIWH(nDestWidth, nDestHeight);
    pCanvas->drawImageRect(skSrcImage, rcSrc, rcDst, sampling, &paint, SkCanvas::kStrict_SrcRectConstraint);
    return pSurface->makeImageSnapshot();
}

} //namespace

#ifdef DUILIB_HAVE_OPENCV
//OpenCV

// 直接内存映射（零拷贝）
static cv::Mat SkImageToCvMat_Opt(const sk_sp<SkImage>& skImage)
{
    // 获取像素信息
    SkPixmap pixmap;
    if (!skImage->peekPixels(&pixmap)) {
        return cv::Mat();
    }

    // 创建共享内存的Mat
    cv::Mat viewMat(pixmap.height(), pixmap.width(), CV_8UC4, pixmap.writable_addr(), pixmap.rowBytes());

    // 通道重排（RGBA -> BGRA）
    if (skImage->colorType() == SkColorType::kRGBA_8888_SkColorType) {
        cv::mixChannels(viewMat, viewMat, { 2,0, 1,1, 0,2, 3,3 }); // 原地交换R与B通道(OpenCV默认处理的是BGRA格式)
    }    
    return viewMat; // 注意：Mat的生命周期必须短于SkImage
}

// 优化版缩放函数
static sk_sp<SkImage> SkiaResizeWithOpenCV_Opt(const sk_sp<SkImage>& srcImage, int newWidth, int newHeight)
{
    // 获取源图像内存视图
    cv::Mat srcMat = SkImageToCvMat_Opt(srcImage);
    if (srcMat.empty()) {
        return nullptr;
    }

    // 使用预分配内存执行缩放
    SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(newWidth, newHeight);
    sk_sp<SkData> dstData = SkData::MakeUninitialized(dstInfo.computeMinByteSize());
    cv::Mat dstMat(newHeight, newWidth, CV_8UC4, dstData->writable_data(), dstInfo.minRowBytes());

    // 执行缩放并转换颜色空间
    cv::resize(srcMat, dstMat, dstMat.size(), 0, 0, cv::INTER_LINEAR);

    // 通道重排（BGRA->RGBA）
    if (dstInfo.colorType() == SkColorType::kRGBA_8888_SkColorType) {
        cv::mixChannels(dstMat, dstMat, { 2,0, 1,1, 0,2, 3,3 });
    }

    // 创建共享内存的SkImage
    return SkImages::RasterFromData(dstInfo, dstData, dstInfo.minRowBytes());
}

/** 对Skia的图片进行resize操作，以适配绘制目标大小，避免绘制时缩放导致速度降低(OpenCV实现)
*/
static bool ResizeSkiaImageByOpenCV(const sk_sp<SkImage>& skImage, const UiRect& rcSrc, const UiRect& rcDest, sk_sp<SkImage>& skNewImage)
{
    if ((skImage == nullptr) || (rcSrc.left != 0) || (rcSrc.top != 0) || (rcSrc.Width() < 10) || (rcSrc.Height() < 10) || (rcDest.Width() < 10) || (rcDest.Height() < 10)) {
        return false;
    }
    if ((rcSrc.Width() == rcDest.Width()) && (rcSrc.Height() == rcDest.Height())) {
        //区域相同，无缩放
        return false;
    }
    if ((skImage->width() != rcSrc.Width()) || (skImage->height() != rcSrc.Height())) {
        //不是整个图片绘制
        return false;
    }
    SkPixmap srcPixmap;
    if (!skImage->peekPixels(&srcPixmap)) {
        return false;
    }
    if ((srcPixmap.width() != rcSrc.Width()) || (srcPixmap.height() != rcSrc.Height())) {
        //有错误
        return false;
    }

    //PerformanceUtil statPerformance(_T("Render_Skia::DrawSkiaImage::ResizeSkiaImageByOpenCV"));
    skNewImage = SkiaResizeWithOpenCV_Opt(skImage, rcDest.Width(), rcDest.Height());
    return skNewImage != nullptr;
}
#else
/** 对Skia的图片进行resize操作，以适配绘制目标大小，避免绘制时缩放导致速度降低(stb_image实现)
*/
static bool ResizeSkiaImageByStbImage(const sk_sp<SkImage>& /*skImage*/, const UiRect& /*rcSrc*/, const UiRect& /*rcDest*/, sk_sp<SkImage>& /*skNewImage*/)
{
    //屏蔽该功能，经测试发现，按目前的代码，这项开启后，绘制性能明显下降，主要因为这个函数的缩放耗时比Skia自身的缩放耗时要多很多（可能最新版本skia的缩放性能有改进）。
    return false;
    //if ((skImage == nullptr) || (rcSrc.left != 0) || (rcSrc.top != 0) || (rcSrc.Width() < 10) || (rcSrc.Height() < 10) || (rcDest.Width() < 10) || (rcDest.Height() < 10)) {
    //    return false;
    //}
    //if ((rcSrc.Width() == rcDest.Width()) && (rcSrc.Height() == rcDest.Height())) {
    //    //区域相同，无缩放
    //    return false;
    //}
    //if ((skImage->width() != rcSrc.Width()) || (skImage->height() != rcSrc.Height())) {
    //    //不是整个图片绘制
    //    return false;
    //}
    //SkPixmap srcPixmap;
    //if (!skImage->peekPixels(&srcPixmap)) {
    //    return false;
    //}
    //if ((srcPixmap.width() != rcSrc.Width()) || (srcPixmap.height() != rcSrc.Height())) {
    //    //有错误
    //    return false;
    //}
    //PerformanceUtil statPerformance(_T("Render_Skia::DrawSkiaImage::ResizeSkiaImageByStbImage"));

    //sk_sp<SkData> skData = SkData::MakeUninitialized(rcDest.Height() * rcDest.Width() * sizeof(uint32_t));
    //const unsigned char* input_pixels = (const unsigned char*)srcPixmap.addr();
    //int input_w = srcPixmap.width();
    //int input_h = srcPixmap.height();
    //int input_stride_in_bytes = 0;
    //unsigned char* output_pixels = (unsigned char*)skData->writable_data();
    //int output_w = rcDest.Width();
    //int output_h = rcDest.Height();
    //int output_stride_in_bytes = 0;
    //stbir_pixel_layout num_channels = STBIR_RGBA;
    //unsigned char* result = stbir_resize_uint8_linear(input_pixels, input_w, input_h, input_stride_in_bytes,
    //                                                  output_pixels, output_w, output_h, output_stride_in_bytes,
    //                                                  num_channels);
    //if (result != nullptr) {
    //    skNewImage = SkImages::RasterFromData(SkImageInfo::Make(SkISize::Make(output_w, output_h), skImage->imageInfo().colorInfo()), skData, rcDest.Width() * sizeof(uint32_t));
    //    if ((skNewImage->height() == rcDest.Height()) && (skNewImage->width() == rcDest.Width())) {
    //        return (skNewImage != nullptr);
    //    }
    //}
    //return false;
}
#endif //end of OpenCV


void DrawSkiaImage::DrawImage(SkCanvas* pSkCanvas,
                              const UiRect& rcDest,
                              const SkPoint& skPointOrg,
                              const sk_sp<SkImage>& skSrcImage,
                              const UiRect& rcSrc,
                              const SkPaint& skPaint,
                              const SkSamplingOptions& skSampling)
{
    if ((pSkCanvas == nullptr) || (skSrcImage == nullptr)) {
        return;
    }
    SkIRect rcSkDestI = { rcDest.left, rcDest.top, rcDest.right, rcDest.bottom };
    SkRect rcSkDest = SkRect::Make(rcSkDestI);
    rcSkDest.offset(skPointOrg);

    SkIRect rcSkSrcI = { rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom };
    SkRect rcSkSrc = SkRect::Make(rcSkSrcI);

    //快速路径：整幅源图片、需要缩放、且画布无缩放变换时，使用"缩放结果缓存"，
    //缓存命中后按 1:1 绘制，避免 Skia 每帧重复采样。
    //（九宫格/平铺使用子区域，仍走原有路径；画布有缩放时预缩放会二次采样，也不启用）
    const int32_t nDestWidth = rcDest.Width();
    const int32_t nDestHeight = rcDest.Height();
    if ((nDestWidth > 0) && (nDestHeight > 0) &&
        (rcSrc.left == 0) && (rcSrc.top == 0) &&
        (rcSrc.right == skSrcImage->width()) && (rcSrc.bottom == skSrcImage->height()) &&
        ((nDestWidth != skSrcImage->width()) || (nDestHeight != skSrcImage->height())) &&
        ((int64_t)skSrcImage->width() * skSrcImage->height() >= kImageScaleCacheMinSrcPixels) &&
        ((int64_t)nDestWidth * nDestHeight * 4 <= (int64_t)kImageScaleCacheMaxEntryBytes) &&
        SkScalarIsInt(rcSkDest.fLeft) && SkScalarIsInt(rcSkDest.fTop) &&
        IsCanvasUnscaled(pSkCanvas) &&
        ImageScaleCache::Instance().IsEnabled()) {

        ImageScaleCacheKey cacheKey;
        cacheKey.nImageID = skSrcImage->uniqueID();
        cacheKey.nWidth = nDestWidth;
        cacheKey.nHeight = nDestHeight;
        cacheKey.nSamplingKey = MakeSamplingKey(skSampling);

        sk_sp<SkImage> skScaledImage = ImageScaleCache::Instance().Find(cacheKey);
        //注意：同一个key首次出现时只记录、不缓存（一次性绘制的内容不付出额外开销），
        //      第二次出现时才创建缓存，因此动画首轮、只绘制一次的图片仍走原有绘制路径。
        if ((skScaledImage == nullptr) && ImageScaleCache::Instance().Admit(cacheKey)) {
            skScaledImage = CreateScaledSkImage(skSrcImage, nDestWidth, nDestHeight, skSampling);
            if (skScaledImage != nullptr) {
                ImageScaleCache::Instance().Insert(cacheKey, skScaledImage);
            }
        }
        if (skScaledImage != nullptr) {
            //已缩放到目标尺寸，按 1:1 绘制（paint 的 Alpha/混合等属性保持不变）
            const SkSamplingOptions blitSampling(SkFilterMode::kNearest, SkMipmapMode::kNone);
            pSkCanvas->drawImageRect(skScaledImage, rcSkDest, blitSampling, &skPaint);
            return;
        }
    }

#ifdef DUILIB_HAVE_OPENCV
    sk_sp<SkImage> skNewImage;
    if (ResizeSkiaImageByOpenCV(skSrcImage, rcSrc, rcDest, skNewImage)) {
        //PerformanceUtil statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(OpenCV)"));
        rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
        rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();
        pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, skSampling, &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
#else
    sk_sp<SkImage> skNewImage;
    if (ResizeSkiaImageByStbImage(skSrcImage, rcSrc, rcDest, skNewImage)) {
        //PerformanceUtil statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(StbImage)"));
        rcSkSrc.fRight = rcSkSrc.fLeft + skNewImage->width();
        rcSkSrc.fBottom = rcSkSrc.fTop + skNewImage->height();
        pSkCanvas->drawImageRect(skNewImage, rcSkSrc, rcSkDest, skSampling, &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
#endif
    else {
        //PerformanceUtil statPerformance(_T("Render_Skia::DrawSkiaImage::DrawImage drawImageRect(Skia Only)"));
        pSkCanvas->drawImageRect(skSrcImage, rcSkSrc, rcSkDest, skSampling, &skPaint, SkCanvas::kStrict_SrcRectConstraint);
    }
}

void DrawSkiaImage::ClearImageScaleCache()
{
    ImageScaleCache::Instance().Clear();
}

size_t DrawSkiaImage::GetImageScaleCacheCount()
{
    return ImageScaleCache::Instance().GetEntryCount();
}

size_t DrawSkiaImage::GetImageScaleCacheHitCount()
{
    return ImageScaleCache::Instance().GetHitCount();
}

void DrawSkiaImage::EnableImageScaleCache(bool bEnable)
{
    ImageScaleCache::Instance().Enable(bEnable);
}

} // namespace ui

