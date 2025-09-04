#include "Image_Bitmap.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
Image_Bitmap::Image_Bitmap():
    m_fImageSizeScale(IMAGE_SIZE_SCALE_NONE)
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
    
    IRenderFactory* pRenderFactroy = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactroy != nullptr);
    if (pRenderFactroy == nullptr) {
        return nullptr;
    }
    IBitmap* pBitmap = pRenderFactroy->CreateBitmap();
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
    pImageBitmap->m_pBitmap.reset(pBitmap);
    pImageBitmap->m_fImageSizeScale = fImageSizeScale;
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
    pImageBitmap->m_pBitmap = pBitmap;
    pImageBitmap->m_fImageSizeScale = fImageSizeScale;
    return pImage;
}

int32_t Image_Bitmap::GetWidth() const
{
    if (m_pBitmap != nullptr) {
        return m_pBitmap->GetWidth();
    }
    return 0;
}

int32_t Image_Bitmap::GetHeight() const
{
    if (m_pBitmap != nullptr) {
        return m_pBitmap->GetHeight();
    }
    return 0;
}

float Image_Bitmap::GetImageSizeScale() const
{
    return m_fImageSizeScale;
}

ImageType Image_Bitmap::GetImageType() const
{
    return ImageType::kImageBitmap;
}

std::shared_ptr<IBitmap> Image_Bitmap::GetImageBitmap() const
{
    return m_pBitmap;
}

} //namespace ui
