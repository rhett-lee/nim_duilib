#include "Bitmap_Skia.h"
#include "duilib/Image/ImageUtil.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkBitmap.h"
#include "SkiaHeaderEnd.h"

namespace ui
{

Bitmap_Skia::Bitmap_Skia()
{
    m_pSkBitmap = std::make_unique<SkBitmap>();
}

Bitmap_Skia::~Bitmap_Skia()
{
    m_pSkBitmap.reset();
}

bool Bitmap_Skia::Init(uint32_t nWidth, uint32_t nHeight, const void* pPixelBits,
                       float fImageSizeScale, BitmapAlphaType alphaType)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }

    if (ImageUtil::NeedResizeImage(fImageSizeScale)) {
        //调整图像大小
        const uint32_t nNewWidth = ImageUtil::GetScaledImageSize(nWidth, fImageSizeScale);
        const uint32_t nNewHeight = ImageUtil::GetScaledImageSize(nHeight, fImageSizeScale);
        if (pPixelBits == nullptr) {
            //无图像数据
            return InitImage(nNewWidth, nNewHeight, nullptr, alphaType);
        }
        else {
            //有图像数据，执行图片大小调整（无多余数据拷贝）
            size_t nPixelBitsLen = static_cast<size_t>(nHeight) * nWidth * 4;
            size_t nOutPixelBitsLen = static_cast<size_t>(nNewHeight) * nNewWidth * 4;

            m_pSkBitmap->reset();
            m_pSkBitmap->setInfo(SkImageInfo::Make(nNewWidth, nNewHeight, kN32_SkColorType, static_cast<SkAlphaType>(alphaType)));
            m_pSkBitmap->allocPixels();

            void* pOutPixelBits = nullptr;
            SkPixmap pixmap;
            if (m_pSkBitmap->peekPixels(&pixmap)) {
                pOutPixelBits = pixmap.writable_addr();
            }
            ASSERT(pOutPixelBits != nullptr);
            if (pOutPixelBits == nullptr) {
                m_pSkBitmap->reset();
                return false;
            }
            bool bRet = ImageUtil::ResizeImageData((const uint8_t*)pPixelBits, nPixelBitsLen, nWidth, nHeight,
                                                   (uint8_t*)pOutPixelBits, nOutPixelBitsLen, nNewWidth, nNewHeight);
            if (!bRet) {
                m_pSkBitmap->reset();
                return false;
            }
            else {
                //更新图片的透明通道数据
                UpdateAlphaFlag((uint8_t*)pOutPixelBits);
                return true;
            }            
        }
    }
    else {
        return InitImage(nWidth, nHeight, pPixelBits, alphaType);
    }
}

bool Bitmap_Skia::InitImage(uint32_t nWidth, uint32_t nHeight, const void* pPixelBits, BitmapAlphaType alphaType)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }

    m_pSkBitmap->reset();
    m_pSkBitmap->setInfo(SkImageInfo::Make(nWidth, nHeight, kN32_SkColorType, static_cast<SkAlphaType>(alphaType)));
    m_pSkBitmap->allocPixels();

    void* pBits = nullptr;
    SkPixmap pixmap;
    if (m_pSkBitmap->peekPixels(&pixmap)) {
        pBits = pixmap.writable_addr();
    }
    ASSERT(pBits != nullptr);
    if (pBits == nullptr) {
        m_pSkBitmap->reset();
        return false;
    }
    //复制图片数据到位图
    if (pPixelBits != nullptr) {
        ::memcpy(pBits, pPixelBits, nWidth * nHeight * sizeof(uint32_t));
    }
    
    //更新图片的透明通道数据
    UpdateAlphaFlag((uint8_t*)pBits);
    return true;
}

uint32_t Bitmap_Skia::GetWidth() const
{
    return (uint32_t)m_pSkBitmap->width();
}

uint32_t Bitmap_Skia::GetHeight() const
{
    return (uint32_t)m_pSkBitmap->height();
}

UiSize Bitmap_Skia::GetSize() const
{
    return UiSize(m_pSkBitmap->width(), m_pSkBitmap->height());
}

void* Bitmap_Skia::LockPixelBits()
{
    void* pPixelBits = nullptr;
    SkPixmap pixmap;
    if (m_pSkBitmap->peekPixels(&pixmap)) {
        pPixelBits = pixmap.writable_addr();
    }
    ASSERT(pPixelBits != nullptr);
    return pPixelBits;
}

void Bitmap_Skia::UnLockPixelBits()
{
    void* pPixelBits = nullptr;
    SkPixmap pixmap;
    if (m_pSkBitmap->peekPixels(&pixmap)) {
        pPixelBits = pixmap.writable_addr();
    }
    ASSERT(pPixelBits != nullptr);
    if (pPixelBits != nullptr) {
        UpdateAlphaFlag((uint8_t*)pPixelBits);
    }    
}

IBitmap* Bitmap_Skia::Clone()
{
    uint32_t nWidth = GetWidth();
    uint32_t nHeight = GetHeight();
    if ((nWidth == 0) || (nHeight == 0)) {
        return nullptr;
    }

    void* pPixelBits = nullptr;
    SkPixmap pixmap;
    if (m_pSkBitmap->peekPixels(&pixmap)) {
        pPixelBits = pixmap.writable_addr();
    }
    ASSERT(pPixelBits != nullptr);
       
    IBitmap* pBitmap = new Bitmap_Skia();
    if (!pBitmap->Init(nWidth, nHeight, pPixelBits, 1.0f, static_cast<BitmapAlphaType>(m_pSkBitmap->info().alphaType()))) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

void Bitmap_Skia::UpdateAlphaFlag(uint8_t* pPixelBits)
{
    if (pPixelBits == nullptr) {
        return;
    }
    int32_t nWidth = m_pSkBitmap->width();
    int32_t nHeight = m_pSkBitmap->height();
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return;
    }
    if (m_pSkBitmap->info().alphaType() == SkAlphaType::kOpaque_SkAlphaType) {
        //指定为不透明图片，不需要更新AlphaBitmap标志
        for (int32_t i = 0; i < nHeight; ++i) {
            for (int32_t j = 0; j < nWidth; ++j) {
                uint8_t* a = (uint8_t*)pPixelBits + (i * nWidth + j) * sizeof(uint32_t) + 3;
                *a = 255;
            }
        }
    }
}

const SkBitmap& Bitmap_Skia::GetSkBitmap() const
{
    ASSERT(m_pSkBitmap.get() != nullptr);
    return *m_pSkBitmap.get();
}

} // namespace ui
