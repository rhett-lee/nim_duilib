#include "Image_Bitmap.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

/** 单帧位图图片接口的实现
*/
class Image_Bitmap::BitmapImageImpl : public IBitmapImage
{
public:
    BitmapImageImpl():
        m_fImageSizeScale(IMAGE_SIZE_SCALE_NONE)
    {
    }

    virtual ~BitmapImageImpl() = default;

    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const override
    {
        if (m_pBitmap != nullptr) {
            return m_pBitmap->GetWidth();
        }
        else if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->GetWidth();
        }
        return 0;
    }

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const override
    {
        if (m_pBitmap != nullptr) {
            return m_pBitmap->GetHeight();
        }
        else if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->GetHeight();
        }
        return 0;
    }

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override
    {
        return m_fImageSizeScale;
    }

    /** 获取位图
    * @param [out] bDecodeError 返回值代表是否遇到图片解码错误
    * @return 返回位图的接口指针，如果返回nullptr并且bDecodeError为false表示图片尚未完成解码（多线程解码的情况下）
    *                          如果返回nullptr并且bDecodeError为true代表图片解码出现错误
    */
    virtual std::shared_ptr<IBitmap> GetBitmap(bool* bDecodeError) override
    {
        if (m_pBitmap != nullptr) {
            return m_pBitmap;
        }
        //延迟解码
        if (m_pAnimationImage != nullptr) {
            //单帧，加载位图图片
            IAnimationImage::AnimationFrame frame;
            if (m_pAnimationImage->ReadFrameData(0, UiSize(), &frame)) {
                if (bDecodeError != nullptr) {
                    *bDecodeError = frame.m_bDataError;
                }
                m_pBitmap = frame.m_pBitmap;
                if (m_pBitmap != nullptr) {
                    //读取完成后，释放资源
                    m_pAnimationImage.reset();
                }
            }
            else {
                if (bDecodeError != nullptr) {
                    *bDecodeError = true;
                }
                //ASSERT(0);
            }
        }
        return m_pBitmap;
    }

public:
    /** 是否支持延迟解码数据
    * @return 返回true表示需要解码，返回false表示不需要解码
    */
    virtual bool IsDelayDecodeEnabled() const override
    {
        if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->IsDelayDecodeEnabled();
        }
        return false;
    }

    /** 延迟解码图片数据是否完成
    * @return 延迟解码图片数据操作已经完成
    */
    virtual bool IsDelayDecodeFinished() const override
    {
        if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->IsDelayDecodeFinished();
        }
        return true;
    }

    /** 获取当前延迟解码完成的图片帧索引号（从0开始编号）
    */
    virtual uint32_t GetDecodedFrameIndex() const override
    {
        if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->GetDecodedFrameIndex();
        }
        return 0;
    }

    /** 延迟解码图片数据（可以在多线程中调用）
    * @param [in] nMinFrameIndex 至少需要解码到哪一帧（帧索引号，从0开始编号）
    * @param [in] IsAborted 解码终止终止测试函数，返回true表示终止，否则表示正常操作
    * @param [out] bDecodeError 返回true表示遇到图片解码错误
    * @return 返回true表示成功，返回false表示解码失败或者外部终止
    */
    virtual bool DelayDecode(uint32_t nMinFrameIndex,
                             std::function<bool(void)> IsAborted,
                             bool* bDecodeError) override
    {
        if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->DelayDecode(nMinFrameIndex, IsAborted, bDecodeError);
        }
        return false;
    }

    /** 合并延迟解码图片数据的结果
    */
    virtual bool MergeDelayDecodeData() override
    {
        if (m_pAnimationImage != nullptr) {
            return m_pAnimationImage->MergeDelayDecodeData();
        }
        return false;
    }

public:
    /** 位图数据
    */
    std::shared_ptr<IBitmap> m_pBitmap;

    /** 动画数据(只取第一帧)
    */
    std::shared_ptr<IAnimationImage> m_pAnimationImage;

    /** 原图加载时的缩放比例
    */
    float m_fImageSizeScale;
};

Image_Bitmap::Image_Bitmap():
    m_nAsyncDecodeTaskId(0)
{
}

Image_Bitmap::~Image_Bitmap()
{
}

std::unique_ptr<IImage> Image_Bitmap::MakeImage(uint32_t nWidth, uint32_t nHeight,
                                                const void* pPixelBits,
                                                float fImageSizeScale,
                                                BitmapAlphaType alphaType)
{
    
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
    if (!pBitmap->Init(nWidth, nHeight, pPixelBits, fImageSizeScale, alphaType)) {
        delete pBitmap;
        return nullptr;
    }
    Image_Bitmap* pImageBitmap = new Image_Bitmap;
    std::unique_ptr<IImage> pImage(pImageBitmap);
    BitmapImageImpl* pImageBitmapImpl = new BitmapImageImpl;
    pImageBitmap->m_pBitmapImage.reset(pImageBitmapImpl);

    pImageBitmapImpl->m_pBitmap.reset(pBitmap);
    pImageBitmapImpl->m_fImageSizeScale = fImageSizeScale;
    return pImage;
}

std::unique_ptr<IImage> Image_Bitmap::MakeImage(const std::shared_ptr<IBitmap>& pBitmap, float fImageSizeScale)
{
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return nullptr;
    }
    ASSERT((pBitmap->GetWidth() > 0) && (pBitmap->GetHeight() > 0));
    if ((pBitmap->GetWidth() <= 0) || (pBitmap->GetHeight() <= 0)) {
        return nullptr;
    }
    Image_Bitmap* pImageBitmap = new Image_Bitmap;
    std::unique_ptr<IImage> pImage(pImageBitmap);
    BitmapImageImpl* pImageBitmapImpl = new BitmapImageImpl;
    pImageBitmap->m_pBitmapImage.reset(pImageBitmapImpl);

    pImageBitmapImpl->m_pBitmap = pBitmap;
    pImageBitmapImpl->m_fImageSizeScale = fImageSizeScale;
    return pImage;
}

std::unique_ptr<IImage> Image_Bitmap::MakeImage(const std::shared_ptr<IBitmapImage>& pBitmap)
{
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        return nullptr;
    }
    ASSERT((pBitmap->GetWidth() > 0) && (pBitmap->GetHeight() > 0));
    if ((pBitmap->GetWidth() <= 0) || (pBitmap->GetHeight() <= 0)) {
        return nullptr;
    }

    Image_Bitmap* pImageBitmap = new Image_Bitmap;
    std::unique_ptr<IImage> pImage(pImageBitmap);
    pImageBitmap->m_pBitmapImage = pBitmap;
    return pImage;
}

std::unique_ptr<IImage> Image_Bitmap::MakeImage(const std::shared_ptr<IAnimationImage>& pAnimationImage)
{
    ASSERT(pAnimationImage != nullptr);
    if (pAnimationImage == nullptr) {
        return nullptr;
    }
    ASSERT((pAnimationImage->GetWidth() > 0) && (pAnimationImage->GetHeight() > 0));
    if ((pAnimationImage->GetWidth() <= 0) || (pAnimationImage->GetHeight() <= 0)) {
        return nullptr;
    }

    Image_Bitmap* pImageBitmap = new Image_Bitmap;
    std::unique_ptr<IImage> pImage(pImageBitmap);
    BitmapImageImpl* pImageBitmapImpl = new BitmapImageImpl;
    pImageBitmap->m_pBitmapImage.reset(pImageBitmapImpl);

    pImageBitmapImpl->m_pAnimationImage = pAnimationImage;
    pImageBitmapImpl->m_fImageSizeScale = pAnimationImage->GetImageSizeScale();
    return pImage;
}

int32_t Image_Bitmap::GetWidth() const
{
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->GetWidth();
    }
    return 0;
}

int32_t Image_Bitmap::GetHeight() const
{
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->GetHeight();
    }
    return 0;
}

float Image_Bitmap::GetImageSizeScale() const
{
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->GetImageSizeScale();
    }
    return 1.0f;
}

ImageType Image_Bitmap::GetImageType() const
{
    return ImageType::kImageBitmap;
}

std::shared_ptr<IBitmapImage> Image_Bitmap::GetImageBitmap() const
{
    ASSERT(m_pBitmapImage != nullptr);
    return m_pBitmapImage;
}

bool Image_Bitmap::IsAsyncDecodeEnabled() const
{
    ASSERT(m_pBitmapImage != nullptr);
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->IsDelayDecodeEnabled();
    }
    return false;
}

bool Image_Bitmap::IsAsyncDecodeFinished() const
{
    ASSERT(m_pBitmapImage != nullptr);
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->IsDelayDecodeFinished();
    }
    return false;
}

uint32_t Image_Bitmap::GetDecodedFrameIndex() const
{
    ASSERT(m_pBitmapImage != nullptr);
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->GetDecodedFrameIndex();
    }
    return 0;
}

void Image_Bitmap::SetAsyncDecodeTaskId(size_t nTaskId)
{
    m_nAsyncDecodeTaskId = nTaskId;
}

size_t Image_Bitmap::GetAsyncDecodeTaskId() const
{
    return m_nAsyncDecodeTaskId;
}

bool Image_Bitmap::AsyncDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted, bool* bDecodeError)
{
    ASSERT(m_pBitmapImage != nullptr);
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->DelayDecode(nMinFrameIndex, IsAborted, bDecodeError);
    }
    return false;
}

bool Image_Bitmap::MergeAsyncDecodeData()
{
    ASSERT(m_pBitmapImage != nullptr);
    if (m_pBitmapImage != nullptr) {
        return m_pBitmapImage->MergeDelayDecodeData();
    }
    return false;
}

} //namespace ui
