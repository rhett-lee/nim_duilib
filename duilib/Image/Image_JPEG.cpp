#include "Image_JPEG.h"
#include "duilib/Core/GlobalManager.h"

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

    //图片宽度
    uint32_t m_nWidth = 0;

    //图片高度
    uint32_t m_nHeight = 0;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    /** 位图数据
    */
    std::shared_ptr<IBitmap> m_pBitmap;
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

// 查找与fImageSizeScale最接近的数值
static float FindClosestScale(float fImageSizeScale)
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

bool Image_JPEG::LoadImageData(std::vector<uint8_t>& fileData, float fImageSizeScale)
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

    //解析Header成功
    //libjpeg API: IDCT scaling extensions in decompressor
    //libjpeg - turbo supports IDCT scaling with scaling factors of 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, 9/8, 5/4, 11/8, 3/2, 13/8, 7/4, 15/8, and 2/1 (only 1/4 and 1/2 are SIMD - accelerated.)
    //加载缩放时，只支持固定的锁定的比例
    float fRealImageSizeScale = FindClosestScale(fImageSizeScale);
    if (!ImageUtil::IsValidImageScale(fRealImageSizeScale)) {
        fRealImageSizeScale = 1.0f;
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
    fileData.clear();
    return true;
}

int32_t Image_JPEG::GetWidth() const
{
    return m_impl->m_nWidth;
}

int32_t Image_JPEG::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_JPEG::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

ImageType Image_JPEG::GetImageType() const
{
    return ImageType::kImageBitmap;
}

std::shared_ptr<IBitmap> Image_JPEG::GetImageBitmap() const
{
    if ((m_impl->m_pBitmap == nullptr) &&
        (m_impl->m_tjInstance != nullptr) &&
        !m_impl->m_fileData.empty() &&
        (m_impl->m_nWidth > 0) &&
        (m_impl->m_nHeight > 0)) {
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
        if (!pBitmap->Init(m_impl->m_nWidth, m_impl->m_nHeight, nullptr)) {
            delete pBitmap;
            return nullptr;
        }
        void* pBitmapBits = pBitmap->LockPixelBits();
        if (pBitmapBits == nullptr) {
            delete pBitmap;
            return nullptr;
        }
        m_impl->m_pBitmap.reset(pBitmap);

#ifdef DUILIB_BUILD_FOR_WIN
        int pixelFormat = TJPF_BGRA;
#else
        int pixelFormat = TJPF_RGBA;
#endif

        // 执行解码：从JPG内存数据解码为RGBA格式
        int ret = tjDecompress2(m_impl->m_tjInstance,
                                m_impl->m_fileData.data(),
                                (unsigned long)m_impl->m_fileData.size(),
                                (unsigned char*)pBitmapBits,
                                (int)m_impl->m_nWidth,
                                0, // 行间距，0表示使用默认值（width * bytesPerPixel）
                                (int)m_impl->m_nHeight,
                                pixelFormat, // 输出格式为RGBA/BGRA
                                TJFLAG_FASTDCT); // 使用快速DCT算法加速
        ASSERT(ret == 0);
        if (ret != 0) {
            m_impl->m_pBitmap.reset();
        }
    }
    return m_impl->m_pBitmap;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
