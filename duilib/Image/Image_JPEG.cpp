#include "Image_JPEG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FileUtil.h"

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO

#include "turbojpeg.h"

namespace ui
{
struct Image_JPEG::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

    //解压Jpeg的句柄
    tjhandle m_tjInstance = nullptr;

    //是否支持异步线程解码图片数据
    bool m_bAsyncDecode = false;

    //图片宽度
    uint32_t m_nWidth = 0;

    //图片高度
    uint32_t m_nHeight = 0;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    /** 位图数据
    */
    std::shared_ptr<IBitmap> m_pBitmap;

    /** 位图数据（延迟解码）
    */
    std::shared_ptr<IBitmap> m_pDelayBitmap;
};

Image_JPEG::Image_JPEG()
{
    m_impl = std::make_unique<TImpl>();
}

Image_JPEG::~Image_JPEG()
{
    if (m_impl->m_tjInstance != nullptr) {
        tjDestroy(m_impl->m_tjInstance);
        m_impl->m_tjInstance = nullptr;
    }
}

// 查找与fImageSizeScale最接近的数值(但需要保证返回的值不小于fImageSizeScale)
static float FindClosestScale1(float fImageSizeScale)
{
    // 存储所有可能的比例值
    std::vector<float> scales = {
        1.0f / 8, 1.0f / 4, 3.0f / 8, 1.0f / 2,
        5.0f / 8, 3.0f / 4, 7.0f / 8, 1.0f / 1,
        9.0f / 8, 5.0f / 4, 11.0f / 8, 3.0f / 2,
        13.0f / 8, 7.0f / 4, 15.0f / 8, 2.0f / 1
    };

    // 首先检查是否有精确匹配的值
    auto it = std::find(scales.begin(), scales.end(), fImageSizeScale);
    if (it != scales.end()) {
        return *it;
    }

    // 找到所有大于等于目标值的元素
    std::vector<float> candidates;
    for (float scale : scales) {
        if (scale >= fImageSizeScale) {
            candidates.push_back(scale);
        }
    }

    // 如果有符合条件的候选值，返回最小的那个（最接近目标值）
    if (!candidates.empty()) {
        return *std::min_element(candidates.begin(), candidates.end());
    }

    // 如果所有值都小于目标值，返回最大的元素
    return *std::max_element(scales.begin(), scales.end());
}

// 查找与fImageSizeScale最接近的数值
static float FindClosestScale2(float fImageSizeScale)
{
    // 用vector存储数组
    std::vector<float> scales = {
        1.0f / 8, 1.0f / 4, 3.0f / 8, 1.0f / 2,
        5.0f / 8, 3.0f / 4, 7.0f / 8, 1.0f / 1,
        9.0f / 8, 5.0f / 4, 11.0f / 8, 3.0f / 2,
        13.0f / 8, 7.0f / 4, 15.0f / 8, 2.0f / 1
    };

    // 初始化最接近的值和最小差值
    float closest = scales[0];
    float minDiff = std::abs(fImageSizeScale - closest);

    // 遍历vector查找最接近的值
    for (size_t i = 1; i < scales.size(); ++i) {
        float currentDiff = std::abs(fImageSizeScale - scales[i]);
        if (currentDiff < minDiff) {
            minDiff = currentDiff;
            closest = scales[i];
        }
    }
    return closest;
}

bool Image_JPEG::LoadImageFromFile(const FilePath& filePath,
                                   float fImageSizeScale,
                                   bool bAsyncDecode,
                                   const UiSize& rcMaxDestRectSize)
{
    //没有直接支持传入文件路径的函数，需要自己读入到内存，然后再处理
    std::vector<uint8_t> fileData;
    FileUtil::ReadFileData(filePath, fileData);
    return LoadImageFromMemory(fileData, fImageSizeScale, bAsyncDecode, rcMaxDestRectSize);
}

bool Image_JPEG::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                     float fImageSizeScale,
                                     bool bAsyncDecode,
                                     const UiSize& rcMaxDestRectSize)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    // 初始化turbojpeg解码器
    tjhandle tjInstance = tjInitDecompress();
    ASSERT(tjInstance != nullptr);
    if (tjInstance == nullptr) {
        return false;
    }
    int width = 0;
    int height = 0;
    int jpegSubsamp = 0;
    int jpegColorspace = 0;
    // 解码JPEG头信息获取图像基本信息
    int ret = tjDecompressHeader3(tjInstance,
                                  fileData.data(),
                                  (unsigned long)fileData.size(),
                                  &width,
                                  &height,
                                  &jpegSubsamp,
                                  &jpegColorspace);
    if (ret != 0) {
        tjDestroy(tjInstance);
        return false;
    }
    if ((width <= 0) || (height <= 0)) {
        tjDestroy(tjInstance);
        return false;
    }
    if (!ImageUtil::IsValidImageScale(fImageSizeScale)) {
        fImageSizeScale = 1.0f;
    }
    //libjpeg API: IDCT scaling extensions in decompressor
    //libjpeg - turbo supports IDCT scaling with scaling factors of 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, 9/8, 5/4, 11/8, 3/2, 13/8, 7/4, 15/8, and 2/1 (only 1/4 and 1/2 are SIMD - accelerated.)
    //加载缩放时，只支持固定的锁定的比例，如果比例不符合要求，会导致解码失败

    //解析Header成功
    float fRealImageSizeScale = fImageSizeScale;
    bool bFoundImageScale = false;
    float fScale = fImageSizeScale;
    if (ImageUtil::GetBestImageScale(rcMaxDestRectSize, width, height, fImageSizeScale, fScale)) {
        fScale = FindClosestScale1(fScale);
        if (ImageUtil::IsValidImageScale(fScale)) {
            bFoundImageScale = true;
            fRealImageSizeScale = fScale;
        }
    }
    if (!bFoundImageScale) {
        fRealImageSizeScale = FindClosestScale2(fImageSizeScale);
        if (!ImageUtil::IsValidImageScale(fRealImageSizeScale)) {
            fRealImageSizeScale = 1.0f;
        }
    }    
    m_impl->m_nWidth = ImageUtil::GetScaledImageSize((uint32_t)width, fRealImageSizeScale);
    m_impl->m_nHeight = ImageUtil::GetScaledImageSize((uint32_t)height, fRealImageSizeScale);
    ASSERT((m_impl->m_nWidth > 0) && (m_impl->m_nHeight > 0));
    if ((m_impl->m_nHeight <= 0) || (m_impl->m_nHeight <= 0)) {
        tjDestroy(tjInstance);
        return false;
    }

    m_impl->m_fImageSizeScale = fRealImageSizeScale;
    m_impl->m_tjInstance = tjInstance;
    m_impl->m_fileData.swap(fileData);
    m_impl->m_bAsyncDecode = bAsyncDecode;
    fileData.clear();
    return true;
}

uint32_t Image_JPEG::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_JPEG::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_JPEG::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

std::shared_ptr<IBitmap> Image_JPEG::GetBitmap()
{
    GlobalManager::Instance().AssertUIThread();
    if (m_impl->m_bAsyncDecode || (m_impl->m_pBitmap != nullptr)) {
        //异步解码, 或者已经完成解码
        return m_impl->m_pBitmap;
    }
    else {
        //延迟解码        
        m_impl->m_pBitmap = DecodeBitmap();
        return m_impl->m_pBitmap;
    }
}

std::shared_ptr<IBitmap> Image_JPEG::DecodeBitmap() const
{
    std::shared_ptr<IBitmap> pJpegBitmap;
    const TImpl& impl = *m_impl;
    if ((impl.m_tjInstance != nullptr) &&
        !impl.m_fileData.empty() &&
        (impl.m_nWidth > 0) &&
        (impl.m_nHeight > 0)) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        ASSERT(pRenderFactory != nullptr);
        if (pRenderFactory == nullptr) {
            return nullptr;
        }
        IBitmap* pBitmap = pRenderFactory->CreateBitmap();
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            return nullptr;
        }
        pJpegBitmap.reset(pBitmap);
        if (!pBitmap->Init(impl.m_nWidth, impl.m_nHeight, nullptr)) {
            return nullptr;
        }
        void* pBitmapBits = pBitmap->LockPixelBits();
        if (pBitmapBits == nullptr) {
            return nullptr;
        }        

#ifdef DUILIB_BUILD_FOR_WIN
        int pixelFormat = TJPF_BGRA;
#else
        int pixelFormat = TJPF_RGBA;
#endif

        // 执行解码：从JPG内存数据解码为RGBA格式
        int ret = tjDecompress2(impl.m_tjInstance,
                                impl.m_fileData.data(),
                                (unsigned long)impl.m_fileData.size(),
                                (unsigned char*)pBitmapBits,
                                (int)impl.m_nWidth,
                                0, // 行间距，0表示使用默认值（width * bytesPerPixel）
                                (int)impl.m_nHeight,
                                pixelFormat, // 输出格式为RGBA/BGRA
                                TJFLAG_FASTDCT); // 使用快速DCT算法加速
        ASSERT(ret == 0);
        if (ret == 0) {
            pBitmap->UnLockPixelBits();
        }
        else {
            pJpegBitmap.reset();
        }
    }
    return pJpegBitmap;
}

bool Image_JPEG::IsDelayDecodeEnabled() const
{
    if (m_impl->m_bAsyncDecode &&
        (m_impl->m_tjInstance != nullptr) &&
        !m_impl->m_fileData.empty() &&
        (m_impl->m_nWidth > 0) &&
        (m_impl->m_nHeight > 0) &&
        (m_impl->m_pDelayBitmap == nullptr)) {
        return true;
    }
    return false;
}

bool Image_JPEG::IsDelayDecodeFinished() const
{
    return (m_impl->m_pBitmap != nullptr) || (m_impl->m_pDelayBitmap != nullptr);
}

uint32_t Image_JPEG::GetDecodedFrameIndex() const
{
    return 0;
}

bool Image_JPEG::DelayDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> /*IsAborted*/)
{
    if (IsDelayDecodeEnabled()) {
        ASSERT(m_impl->m_pDelayBitmap == nullptr);
        if (m_impl->m_pDelayBitmap == nullptr) {
            m_impl->m_pDelayBitmap = DecodeBitmap();
            return m_impl->m_pDelayBitmap != nullptr;
        }
    }    
    return false;
}

bool Image_JPEG::MergeDelayDecodeData()
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(m_impl->m_pBitmap == nullptr);
    if (m_impl->m_pDelayBitmap != nullptr) {
        m_impl->m_pBitmap = m_impl->m_pDelayBitmap;
        m_impl->m_pDelayBitmap.reset();

        //解码完成，释放原图资源
        if (m_impl->m_tjInstance != nullptr) {
            tjDestroy(m_impl->m_tjInstance);
            m_impl->m_tjInstance = nullptr;
        }
        std::vector<uint8_t> fileData;
        m_impl->m_fileData.swap(fileData);
        return true;
    }
    return false;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
