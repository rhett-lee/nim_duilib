#include "CefMemoryDC.h"

namespace ui {

CefMemoryDC::CefMemoryDC() 
{
    m_pBitmap = nullptr;
    m_pOldBitmap = nullptr;
    m_hMemDC = nullptr;
    memset(&m_bitmapInfo, 0, sizeof(m_bitmapInfo));

    m_bitmapSize.cx = m_bitmapSize.cy = 0;
    m_bValid = false;
}

CefMemoryDC::~CefMemoryDC()
{
    DeleteDC();
}

void CefMemoryDC::DeleteDC()
{
    if (m_pOldBitmap != nullptr) {
        ::SelectObject(m_hMemDC, m_pOldBitmap);
        m_pOldBitmap = nullptr;
    }
    if (m_pBitmap != nullptr) {
        ::DeleteObject(m_pBitmap);
        m_pBitmap = nullptr;
    }
    if (m_hMemDC != nullptr)  {
        ::DeleteDC(m_hMemDC);
        m_hMemDC = nullptr;
    }
    memset(&m_bitmapInfo, 0, sizeof(m_bitmapInfo));
    m_bValid = false;
}

bool CefMemoryDC::Init(HDC src_dc, int width, int height, const LPVOID pBits/*=nullptr*/)
{
    DeleteDC();

    LPVOID pBmpBits = nullptr;
    bool bRet = CreateMemoryDC(src_dc, width, height, &pBmpBits);
    if (bRet) {
        m_bitmapSize.cx = width;
        m_bitmapSize.cy = height;
        int stride = m_bitmapSize.cx * 4;
        if (pBits) {
            memcpy(pBmpBits, pBits, stride*m_bitmapSize.cy);
        }
        else {
            memset(pBmpBits, 0, stride*m_bitmapSize.cy);
        }
    }

    return bRet;
}

bool CefMemoryDC::CreateMemoryDC(HDC src_dc, int width, int height, void** pBits)
{
    if (m_hMemDC != nullptr || src_dc == nullptr || width == 0 || height == 0) {
        return false;
    }

    m_hMemDC = ::CreateCompatibleDC(src_dc);

    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = width * height * 4;

    m_pBitmap = ::CreateDIBSection(src_dc, &bmi, DIB_RGB_COLORS, pBits, nullptr, 0);
    m_pOldBitmap = (HBITMAP)::SelectObject(m_hMemDC, m_pBitmap);

    ::GetObject(m_pBitmap, sizeof(m_bitmapInfo), &m_bitmapInfo);
    m_bValid = true;

    return true;
}

BYTE* CefMemoryDC::GetBits()
{
    ASSERT(m_bValid);
    BYTE *pPiexl = LPBYTE(m_bitmapInfo.bmBits);
    ASSERT(pPiexl != nullptr);
    return pPiexl;
}

int CefMemoryDC::GetWidth()
{
    return m_bitmapSize.cx;
}

int CefMemoryDC::GetHeight()
{
    return m_bitmapSize.cy;
}

HBITMAP CefMemoryDC::GetBitmap()
{
    ASSERT(m_pBitmap != nullptr && m_bValid);
    return m_pBitmap;
}

HDC CefMemoryDC::GetDC()
{
    ASSERT(m_hMemDC != nullptr);
    return m_hMemDC;
}

bool CefMemoryDC::IsValid()
{
    return m_bValid && m_hMemDC != nullptr && m_pBitmap != nullptr;
}

} //namespace ui
