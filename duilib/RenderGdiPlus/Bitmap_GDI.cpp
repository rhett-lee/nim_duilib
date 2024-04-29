#include "Bitmap_GDI.h"

namespace ui
{

Bitmap_GDI::Bitmap_GDI():
    m_hBitmap(nullptr),
    m_nWidth(0),
    m_nHeight(0),
    m_bFlipHeight(true),
    m_bAlphaBitmap(false)
{
}

Bitmap_GDI::Bitmap_GDI(HBITMAP hBitmap, bool flipHeight):
    m_hBitmap(hBitmap),
    m_bFlipHeight(flipHeight),
    m_bAlphaBitmap(false),
    m_nWidth(0),
    m_nHeight(0)
{
    if (hBitmap != nullptr) {
        BITMAP bm = { 0 };
        ::GetObject(hBitmap, sizeof(bm), &bm);
        ASSERT(bm.bmBitsPixel == 32);
        m_nHeight = bm.bmHeight;
        m_nWidth = bm.bmWidth;
        UpdateAlphaFlag((const uint8_t*)bm.bmBits);
    }
}

HBITMAP Bitmap_GDI::DetachHBitmap()
{
    HBITMAP hBitmap = m_hBitmap;
    m_hBitmap = nullptr;
    m_nHeight = 0;
    m_nWidth = 0;
    m_bFlipHeight = true;
    m_bAlphaBitmap = false;
    return hBitmap;
}

Bitmap_GDI::~Bitmap_GDI()
{
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

bool Bitmap_GDI::Init(uint32_t nWidth, uint32_t nHeight, bool flipHeight, 
                      const void* pPixelBits, BitmapAlphaType /*alphaType*/)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
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
    UpdateAlphaFlag((const uint8_t*)pBits);
    return true;
}

uint32_t Bitmap_GDI::GetWidth() const
{
    return m_nWidth;
}

uint32_t Bitmap_GDI::GetHeight() const
{
    return m_nHeight;
}

UiSize Bitmap_GDI::GetSize() const
{
    return UiSize(m_nWidth, m_nHeight);
}

void* Bitmap_GDI::LockPixelBits()
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

void Bitmap_GDI::UnLockPixelBits()
{
    if (m_hBitmap != nullptr) {
        BITMAP bm = { 0 };
        ::GetObject(m_hBitmap, sizeof(bm), &bm);
        ASSERT((bm.bmBitsPixel == 32) && (bm.bmWidth == (LONG)m_nWidth) && (bm.bmHeight == (LONG)m_nHeight));
        UpdateAlphaFlag((const uint8_t*)bm.bmBits);
    }    
}

bool Bitmap_GDI::IsAlphaBitmap() const
{
    return m_bAlphaBitmap;
}

IBitmap* Bitmap_GDI::Clone()
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
    
    IBitmap* pBitmap = new Bitmap_GDI();
    if (!pBitmap->Init(nWidth, nHeight, m_bFlipHeight, bm.bmBits)) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

HBITMAP Bitmap_GDI::GetHBitmap() const
{ 
    return m_hBitmap; 
}

HBITMAP Bitmap_GDI::CreateBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits)
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

    HDC hdc = ::GetDC(NULL);
    HBITMAP hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);
    ::ReleaseDC(NULL, hdc);
    return hBitmap;
}

void Bitmap_GDI::UpdateAlphaFlag(const uint8_t* pPixelBits)
{
    m_bAlphaBitmap = false;
    if (pPixelBits == nullptr) {
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
    }
}

} // namespace ui
