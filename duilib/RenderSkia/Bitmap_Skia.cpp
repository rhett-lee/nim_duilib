#include "Bitmap_Skia.h"
#include "duilib/Image/ImageUtil.h"
#include <cstdint>

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
            // 安全计算 buffer 大小（避免 uint32_t 乘法溢出）
            if (nHeight > 0 && nWidth > (SIZE_MAX / 4 / (size_t)nHeight)) {
                ASSERT(!"Bitmap_Skia::Init: source image too large");
                return false;
            }
            size_t nPixelBitsLen = (size_t)nWidth * (size_t)nHeight * 4;
            if (nNewHeight > 0 && nNewWidth > (SIZE_MAX / 4 / (size_t)nNewHeight)) {
                ASSERT(!"Bitmap_Skia::Init: target image too large");
                return false;
            }
            size_t nOutPixelBitsLen = (size_t)nNewWidth * (size_t)nNewHeight * 4;

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
        // 安全计算拷贝大小（避免 uint32_t 乘法溢出）
        if (nHeight > 0 && nWidth > (SIZE_MAX / 4 / (size_t)nHeight)) {
            ASSERT(!"Bitmap_Skia::InitImage: image too large");
            m_pSkBitmap->reset();
            return false;
        }
        size_t copyLen = (size_t)nWidth * (size_t)nHeight * sizeof(uint32_t);
        ::memcpy(pBits, pPixelBits, copyLen);
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
    const int32_t nWidth = m_pSkBitmap->width();
    const int32_t nHeight = m_pSkBitmap->height();
    if ((nWidth <= 0) || (nHeight <= 0)) {
        return;
    }
    if (m_pSkBitmap->info().alphaType() == SkAlphaType::kOpaque_SkAlphaType) {
        //指定为不透明图片，将所有像素的 Alpha 通道填充为 255
        //优化：避免 i*nWidth+j 重复乘法，使用指针递增
        //同时避免内层循环索引乘法溢出
        const size_t totalPixels = (size_t)nWidth * (size_t)nHeight;
        //溢出保护（每像素 4 字节，总字节数 = totalPixels * 4）
        if (totalPixels > (SIZE_MAX / 4)) {
            ASSERT(!"Bitmap_Skia::UpdateAlphaFlag: too many pixels");
            return;
        }
        constexpr uint32_t kOpaquePixel = 0xFF000000u; // ARGB 格式：A=0xFF
        uint32_t* pPixel = reinterpret_cast<uint32_t*>(pPixelBits);
        for (size_t i = 0; i < totalPixels; ++i) {
            // 仅设置 alpha 通道（高字节），保留 RGB 不变
            pPixel[i] |= kOpaquePixel;
        }
    }
}

const SkBitmap& Bitmap_Skia::GetSkBitmap() const
{
    ASSERT(m_pSkBitmap.get() != nullptr);
    return *m_pSkBitmap.get();
}

} // namespace ui
