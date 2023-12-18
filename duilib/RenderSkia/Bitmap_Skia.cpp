#include "Bitmap_Skia.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201)

#include "include/core/SkBitmap.h"

#pragma warning (pop)

namespace ui
{

Bitmap_Skia::Bitmap_Skia():
    m_hBitmap(nullptr),
    m_nWidth(0),
    m_nHeight(0),
    m_bFlipHeight(true),
    m_bAlphaBitmap(false),
    m_alphaType(kUnknown_SkAlphaType)
{
    m_pSkBitmap = std::make_unique<SkBitmap>();
}

Bitmap_Skia::Bitmap_Skia(HBITMAP hBitmap, bool flipHeight):
    m_hBitmap(hBitmap),
    m_nWidth(0),
    m_nHeight(0),
    m_bFlipHeight(flipHeight),
    m_bAlphaBitmap(false),
    m_alphaType(kPremul_SkAlphaType)
{
    ASSERT(flipHeight && "flipHeight value should be true!");
    m_pSkBitmap = std::make_unique<SkBitmap>();
    if (hBitmap != nullptr) {
        BITMAP bm = { 0 };
        ::GetObject(hBitmap, sizeof(bm), &bm);
        ASSERT(bm.bmBitsPixel == 32);
        m_nHeight = bm.bmHeight;
        m_nWidth = bm.bmWidth;
        UpdateAlphaFlag((const uint8_t*)bm.bmBits);

        m_pSkBitmap->reset();
        m_pSkBitmap->setInfo(SkImageInfo::Make(m_nWidth, m_nHeight, kN32_SkColorType, static_cast<SkAlphaType>(kPremul_SkAlphaType)));
        m_pSkBitmap->setPixels(bm.bmBits);
    }
}

HBITMAP Bitmap_Skia::DetachHBitmap()
{
    HBITMAP hBitmap = m_hBitmap;
    m_hBitmap = nullptr;
    m_nHeight = 0;
    m_nWidth = 0;
    m_bFlipHeight = true;
    m_bAlphaBitmap = false;
    m_pSkBitmap->reset();
    return hBitmap;
}

Bitmap_Skia::~Bitmap_Skia()
{
    m_pSkBitmap.reset();
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
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
            flipPixelBits.resize(nWidth * nHeight * 4);
            FlipPixelBits((const uint8_t*)pPixelBits, nWidth, nHeight, flipPixelBits);
            pPixelBits = flipPixelBits.data();
            flipHeight = true;
        }
    }

    LPVOID pBits = nullptr;
    HBITMAP hBitmap = CreateBitmap(nWidth, nHeight, flipHeight, &pBits);
    if (hBitmap == nullptr) {
        return false;
    }
    if (pBits == nullptr) {
        ::DeleteObject(hBitmap);
        return false;
    }
    if (pPixelBits != nullptr) {
        memcpy(pBits, pPixelBits, nWidth * nHeight * 4);        
    }
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
    }
    m_hBitmap = hBitmap;
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_bFlipHeight = flipHeight;
    m_alphaType = alphaType;
    UpdateAlphaFlag((const uint8_t*)pBits);
    
    m_pSkBitmap->reset();
    m_pSkBitmap->setInfo(SkImageInfo::Make(m_nWidth, m_nHeight, kN32_SkColorType, static_cast<SkAlphaType>(alphaType)));
    m_pSkBitmap->setPixels(pBits);   
    return true;
}

void Bitmap_Skia::FlipPixelBits(const uint8_t* pPixelBits, uint32_t nWidth, uint32_t nHeight, std::vector<uint8_t>& flipBits)
{
    ASSERT(flipBits.size() == nWidth* nHeight*4);

    const uint32_t dwEffWidth = nWidth * 4;//每行数据字节数, 按行复制数据
    for (uint32_t row = 0; row < nHeight; ++row) {
        uint8_t* dest = flipBits.data() + row * dwEffWidth;
        const uint8_t* src = pPixelBits + (nHeight - 1 - row) * dwEffWidth;
        memcpy(dest, src, dwEffWidth);
    }
}

uint32_t Bitmap_Skia::GetWidth() const
{
    return m_nWidth;
}

uint32_t Bitmap_Skia::GetHeight() const
{
    return m_nHeight;
}

UiSize Bitmap_Skia::GetSize() const
{
    return UiSize(m_nWidth, m_nHeight);
}

void* Bitmap_Skia::LockPixelBits()
{
    void* pPixelBits = nullptr;
    if (m_hBitmap != nullptr) {
        BITMAP bm = { 0 };
        ::GetObject(m_hBitmap, sizeof(bm), &bm);
        ASSERT(bm.bmBitsPixel == 32);
        pPixelBits = bm.bmBits;
    }
    return pPixelBits;
}

void Bitmap_Skia::UnLockPixelBits()
{
    if (m_hBitmap != nullptr) {
        BITMAP bm = { 0 };
        ::GetObject(m_hBitmap, sizeof(bm), &bm);
        ASSERT((bm.bmBitsPixel == 32) && (bm.bmWidth == (LONG)m_nWidth) && (bm.bmHeight == (LONG)m_nHeight));
        UpdateAlphaFlag((const uint8_t*)bm.bmBits);
    }    
}

bool Bitmap_Skia::IsAlphaBitmap() const
{
    return m_bAlphaBitmap;
}

IBitmap* Bitmap_Skia::Clone()
{
    uint32_t nWidth = GetWidth();
    uint32_t nHeight = GetHeight();
    if ((nWidth == 0) || (nHeight == 0)) {
        return nullptr;
    }
    BITMAP bm = { 0 };
    ::GetObject(m_hBitmap, sizeof(bm), &bm);
    ASSERT(bm.bmBitsPixel == 32);
    ASSERT(bm.bmBits != nullptr);
    
    IBitmap* pBitmap = new Bitmap_Skia();
    if (!pBitmap->Init(nWidth, nHeight, m_bFlipHeight, bm.bmBits)) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

HBITMAP Bitmap_Skia::GetHBitmap() const
{ 
    return m_hBitmap; 
}

HBITMAP Bitmap_Skia::CreateBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if (nWidth == 0 || nHeight == 0) {
        return nullptr;
    }

    BITMAPINFO bmi = { 0 };
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = nWidth;
    if (flipHeight) {
        bmi.bmiHeader.biHeight = -nHeight;//负数表示位图方向：从上到下，左上角为圆点
    }
    else {
        bmi.bmiHeader.biHeight = nHeight; //正数表示位图方向：从下到上，左下角为圆点
    }    
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = nWidth * nHeight * sizeof(DWORD);

    HBITMAP hBitmap = nullptr;
    HDC hdc = ::GetDC(NULL);
    ASSERT(hdc != nullptr);
    if (hdc != nullptr) {
        hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
    }
    ::ReleaseDC(NULL, hdc);
    return hBitmap;
}

void Bitmap_Skia::UpdateAlphaFlag(const uint8_t* pPixelBits)
{
    m_bAlphaBitmap = false;
    if (pPixelBits == nullptr) {
        return;
    }
    if (m_alphaType == kOpaque_SkAlphaType) {
        //指定为不透明图片，不需要更新AlphaBitmap标志
        return;
    }
    for (uint32_t i = 0; i < m_nHeight; ++i) {
        for (uint32_t j = 0; j < m_nWidth; j += 4) {
            uint32_t x = i * m_nWidth + j;
            if (pPixelBits[x + 3] != 255) {
                m_bAlphaBitmap = true;
                break;
            }
        }
        if (m_bAlphaBitmap) {
            break;
        }
    }
}

const SkBitmap& Bitmap_Skia::GetSkBitmap() const
{
    ASSERT(m_pSkBitmap.get() != nullptr);
    return *m_pSkBitmap.get();
}

} // namespace ui
