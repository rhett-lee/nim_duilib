#include "Bitmap_Skia.h"

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

bool Bitmap_Skia::Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight, 
                       const void* pPixelBits, BitmapAlphaType alphaType)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }

    std::vector<uint8_t> flipPixelBits;
    if (!flipHeight) {
        //避免图像是倒着的，此处对图片数据进行翻转处理（Skia似乎不支持flipHeight的情况）
        ASSERT(pPixelBits != nullptr);
        if (pPixelBits != nullptr) {
            //需要对图片数据进行垂直翻转，否则图片是倒着的
            flipPixelBits.resize(nWidth * nHeight * sizeof(uint32_t));
            FlipPixelBits((const uint8_t*)pPixelBits, nWidth, nHeight, flipPixelBits);
            pPixelBits = flipPixelBits.data();
            flipHeight = true;
        }
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

void Bitmap_Skia::FlipPixelBits(const uint8_t* pPixelBits, uint32_t nWidth, uint32_t nHeight, std::vector<uint8_t>& flipBits)
{
    ASSERT(flipBits.size() == nWidth* nHeight*4);

    const uint32_t dwEffWidth = nWidth * 4;//每行数据字节数, 按行复制数据
    for (uint32_t row = 0; row < nHeight; ++row) {
        uint8_t* dest = flipBits.data() + row * dwEffWidth;
        const uint8_t* src = pPixelBits + (nHeight - 1 - row) * dwEffWidth;
        ::memcpy(dest, src, dwEffWidth);
    }
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
    if (!pBitmap->Init(nWidth, nHeight, true, pPixelBits, static_cast<BitmapAlphaType>(m_pSkBitmap->info().alphaType()))) {
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
