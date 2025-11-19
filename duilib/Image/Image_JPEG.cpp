#include "Image_JPEG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FileUtil.h"

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO

#include "turbojpeg.h"

namespace ui
{
struct Image_JPEG::TImpl
{
    //图片文件路径
    FilePath m_imageFilePath;

    //文件数据
    std::vector<uint8_t> m_fileData;

    //解压Jpeg的句柄
    tjhandle m_tjInstance = nullptr;

    //是否支持异步线程解码图片数据
    bool m_bAsyncDecode = false;

    //是否遇到解码错误
    bool m_bDecodeError = false;

    //图片数据出错时，是否允许断言
    bool m_bAssertEnabled = true;

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
static bool FindClosestScale1(const std::vector<tjscalingfactor>& scalingFactorList,
                              float fImageSizeScale,
                              tjscalingfactor& selectedScalingfactor)
{
    if (scalingFactorList.empty()) {
        return false;
    }
    //存储所有可能的比例值
    struct TScalingfactor {
        size_t index;
        tjscalingfactor factor;
        float value;
        bool operator == (const TScalingfactor& r) const
        {
            return ImageUtil::IsSameImageScale(this->value, r.value);
        }
        bool operator < (const TScalingfactor& r) const
        {
            return this->value < r.value;
        }
    };
    std::vector<TScalingfactor> scales;
    for (size_t index = 0; index < scalingFactorList.size(); ++index) {
        const tjscalingfactor& factor = scalingFactorList[index];
        if ((factor.num < 1) || (factor.denom < 1)) {
            continue;
        }
        TScalingfactor scalingFactor;
        scalingFactor.index = index;
        scalingFactor.factor = factor;        
        scalingFactor.value = factor.num * 1.0f / factor.denom;
        scales.push_back(scalingFactor);
    }
    ASSERT(!scales.empty());

    // 首先检查是否有精确匹配的值
    if (!scales.empty()) {
        TScalingfactor dstFactor;
        dstFactor.value = fImageSizeScale;
        auto iter = std::find(scales.begin(), scales.end(), dstFactor);
        if (iter != scales.end()) {
            selectedScalingfactor = iter->factor;
            return true;
        }
    }

    // 找到所有大于等于目标值的元素
    std::vector<TScalingfactor> candidates;
    for (const TScalingfactor& scale : scales) {
        if (scale.value >= fImageSizeScale) {
            candidates.push_back(scale);
        }
    }

    // 如果有符合条件的候选值，返回最小的那个（最接近目标值）
    if (!candidates.empty()) {
        auto iter = std::min_element(candidates.begin(), candidates.end());
        if (iter != candidates.end()) {
            selectedScalingfactor = iter->factor;
            return true;
        }
    }

    // 如果所有值都小于目标值，返回最大的元素
    if (!scales.empty()) {
        auto iter = std::max_element(scales.begin(), scales.end());
        if (iter != scales.end()) {
            selectedScalingfactor = iter->factor;
            return true;
        }
    }
    return false;
}

// 查找与fImageSizeScale最接近的数值
static bool FindClosestScale2(const std::vector<tjscalingfactor>& scalingFactorList,
                              float fImageSizeScale,
                              tjscalingfactor& selectedScalingfactor)
{
    if (scalingFactorList.empty()) {
        return false;
    }
    //存储所有可能的比例值
    struct TScalingfactor {
        size_t index;
        tjscalingfactor factor;
        float value;
    };
    std::vector<TScalingfactor> scales;
    for (size_t index = 0; index < scalingFactorList.size(); ++index) {
        const tjscalingfactor& factor = scalingFactorList[index];
        if ((factor.num < 1) || (factor.denom < 1)) {
            continue;
        }

        TScalingfactor scalingFactor;
        scalingFactor.index = index;
        scalingFactor.factor = factor;
        scalingFactor.value = factor.num * 1.0f / factor.denom;
        scales.push_back(scalingFactor);
    }
    ASSERT(!scales.empty());
    if (scales.empty()) {
        return false;
    }

    // 初始化最接近的值和最小差值
    size_t closestIndex = 0;
    float closest = scales[0].value;
    float minDiff = std::abs(fImageSizeScale - closest);

    // 遍历vector查找最接近的值
    for (size_t i = 1; i < scales.size(); ++i) {
        float currentDiff = std::abs(fImageSizeScale - scales[i].value);
        if (currentDiff < minDiff) {
            minDiff = currentDiff;
            closest = scales[i].value;
            closestIndex = i;
        }
    }
    if (closestIndex < scales.size()) {
        selectedScalingfactor = scales[closestIndex].factor;
        return true;
    }
    return false;
}

bool Image_JPEG::LoadImageFile(std::vector<uint8_t>& fileData,
                               const FilePath& imageFilePath,
                               float fImageSizeScale,
                               bool bAsyncDecode,
                               const UiSize& rcMaxDestRectSize,
                               bool bAssertEnabled)
{
    ASSERT(!fileData.empty() || !imageFilePath.IsEmpty());
    if (fileData.empty() && imageFilePath.IsEmpty()) {
        return false;
    }

    //自动释放资源
    struct TAutoReleaseJpeg
    {
        std::vector<uint8_t>* pFileData = nullptr;
        tjhandle tjInstance = nullptr;
        ~TAutoReleaseJpeg()
        {
            if (pFileData != nullptr) {
                pFileData->clear();
            }
            if (tjInstance != nullptr) {
                tjDestroy(tjInstance);
                tjInstance = nullptr;
            }
        }
    };
    m_impl->m_bAssertEnabled = bAssertEnabled;
    TAutoReleaseJpeg jpegData;

    if (fileData.empty()) {
        //没有直接支持传入文件路径的函数，需要自己读入到内存，然后再处理
        FileUtil::ReadFileData(imageFilePath, fileData);
        if (bAssertEnabled) {
            ASSERT(!fileData.empty());
        }
        if (fileData.empty()) {
            m_impl->m_bDecodeError = true;
            return false;
        }
        jpegData.pFileData = &fileData; //失败时需要清空该数据
    }

    // 初始化turbojpeg解码器
    tjhandle tjInstance = tjInitDecompress();
    ASSERT(tjInstance != nullptr);
    if (tjInstance == nullptr) {
        return false;
    }
    jpegData.tjInstance = tjInstance;

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
        return false;
    }
    //解析Header成功
    if (bAssertEnabled) {
        ASSERT((width > 0) && (height > 0));
    }
    if ((width <= 0) || (height <= 0)) {
        m_impl->m_bDecodeError = true;
        return false;
    }
    if (!ImageUtil::IsValidImageScale(fImageSizeScale)) {
        fImageSizeScale = 1.0f;
    }
    //加载缩放时，只支持固定的锁定的比例，如果比例不符合要求，会导致解码失败，该比例列表可以通过API查询到
    int numscalingfactors = 0;
    tjscalingfactor* pScalingFactors = tjGetScalingFactors(&numscalingfactors);
    std::vector<tjscalingfactor> scalingFactorList;
    if ((pScalingFactors != nullptr) && (numscalingfactors > 0)) {
        for (int i = 0; i < numscalingfactors; ++i) {
            const tjscalingfactor& factor = pScalingFactors[i];
            if ((factor.num > 0) && factor.denom > 0) {
                scalingFactorList.push_back(factor);
            }
        }
    }
    tjscalingfactor selectedScalingfactor = TJUNSCALED;//默认为原始图像大小，不缩放
    ASSERT(selectedScalingfactor.num == 1);
    ASSERT(selectedScalingfactor.denom == 1);
    if (!scalingFactorList.empty()) {
        //查询所选择的缩放比
        bool bFoundImageScale = false;
        float fScale = fImageSizeScale;
        if (ImageUtil::GetBestImageScale(rcMaxDestRectSize, width, height, fImageSizeScale, fScale)) {
            if (FindClosestScale1(scalingFactorList, fScale, selectedScalingfactor)) {
                bFoundImageScale = true;
            }
        }
        if (!bFoundImageScale) {
            FindClosestScale2(scalingFactorList, fImageSizeScale, selectedScalingfactor);
        }
    }

    //最终校验，确保值为有效值
    if ((selectedScalingfactor.num < 1) || (selectedScalingfactor.denom < 1)) {
        selectedScalingfactor.num = 1;
        selectedScalingfactor.denom = 1;
    }
    m_impl->m_nWidth = TJSCALED(width, selectedScalingfactor);
    m_impl->m_nHeight = TJSCALED(height, selectedScalingfactor);
    ASSERT((m_impl->m_nWidth > 0) && (m_impl->m_nHeight > 0));
    if ((m_impl->m_nHeight <= 0) || (m_impl->m_nHeight <= 0)) {
        m_impl->m_bDecodeError = true;
        return false;
    }

    //加载成功
    jpegData.tjInstance = nullptr;
    jpegData.pFileData = nullptr;
    m_impl->m_fileData.clear();

    m_impl->m_fImageSizeScale = selectedScalingfactor.num * 1.0f / selectedScalingfactor.denom;
    m_impl->m_tjInstance = tjInstance;
    m_impl->m_fileData.swap(fileData);
    m_impl->m_bAsyncDecode = bAsyncDecode;
    m_impl->m_imageFilePath = imageFilePath;
    m_impl->m_bDecodeError = false;
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

std::shared_ptr<IBitmap> Image_JPEG::GetBitmap(bool* bDecodeError)
{
    GlobalManager::Instance().AssertUIThread();
    std::shared_ptr<IBitmap> pBitmap;
    if (m_impl->m_bAsyncDecode || (m_impl->m_pBitmap != nullptr)) {
        //异步解码, 或者已经完成解码
        pBitmap = m_impl->m_pBitmap;
    }
    else {
        //延迟解码        
        m_impl->m_pBitmap = DecodeBitmap();
        pBitmap = m_impl->m_pBitmap;        
        if (pBitmap == nullptr) {
            m_impl->m_bDecodeError = true;
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
        }
    }    
    return pBitmap;
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
        if (impl.m_bAssertEnabled) {
            ASSERT(ret == 0);
        }
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
        (m_impl->m_pDelayBitmap == nullptr) &&
        !m_impl->m_bDecodeError) {
        return true;
    }
    return false;
}

bool Image_JPEG::IsDelayDecodeFinished() const
{
    return (m_impl->m_pBitmap != nullptr) || (m_impl->m_pDelayBitmap != nullptr) || m_impl->m_bDecodeError;
}

uint32_t Image_JPEG::GetDecodedFrameIndex() const
{
    return 0;
}

bool Image_JPEG::DelayDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> /*IsAborted*/, bool* bDecodeError)
{
    bool bRet = false;
    if (IsDelayDecodeEnabled()) {
        ASSERT(m_impl->m_pDelayBitmap == nullptr);
        if (m_impl->m_pDelayBitmap == nullptr) {
            m_impl->m_pDelayBitmap = DecodeBitmap();
            bRet = m_impl->m_pDelayBitmap != nullptr;
            if (!bRet) {
                m_impl->m_bDecodeError = true;
                if (bDecodeError != nullptr) {
                    *bDecodeError = true;
                }
            }
        }
    }
    return bRet;
}

bool Image_JPEG::MergeDelayDecodeData()
{
    GlobalManager::Instance().AssertUIThread();
    bool bRet = false;
    bool bDecodeFinished = m_impl->m_bDecodeError || (m_impl->m_pBitmap != nullptr);
    if ((m_impl->m_pDelayBitmap != nullptr) && (m_impl->m_pBitmap == nullptr)) {
        m_impl->m_pBitmap = m_impl->m_pDelayBitmap;
        m_impl->m_pDelayBitmap.reset();

        bDecodeFinished = true;
        bRet = true;
    }
    if (bDecodeFinished) {
        //解码完成，释放原图资源
        if (m_impl->m_tjInstance != nullptr) {
            tjDestroy(m_impl->m_tjInstance);
            m_impl->m_tjInstance = nullptr;
        }
        std::vector<uint8_t> fileData;
        m_impl->m_fileData.swap(fileData);
    }
    return bRet;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
