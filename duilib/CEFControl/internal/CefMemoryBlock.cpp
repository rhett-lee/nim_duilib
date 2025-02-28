#include "CefMemoryBlock.h"

namespace ui {

CefMemoryBlock::CefMemoryBlock():
    m_pBits(nullptr),
    m_nWidth(0),
    m_nHeight(0)
{
}

CefMemoryBlock::~CefMemoryBlock()
{
    Uninit();
}

bool CefMemoryBlock::Init(int32_t width, int32_t height)
{
    ASSERT((width > 0) && (height > 0));
    if ((width <= 0) || (height <= 0)) {
        return false;
    }
    if ((m_nWidth == width) && (m_nHeight == height)) {
        //无需重新初始化
        return true;
    }
    if (m_pBits != nullptr) {
        delete[] m_pBits;
        m_pBits = nullptr;
    }
    m_pBits = new uint8_t[width * height * sizeof(uint32_t)];
    m_nWidth = width;
    m_nHeight = height;
    return true;
}

void CefMemoryBlock::Uninit()
{
    if (m_pBits != nullptr) {
        delete[] m_pBits;
        m_pBits = nullptr;
    }
    m_nWidth = 0;
    m_nHeight = 0;
}

bool CefMemoryBlock::IsValid() const
{
    return m_pBits != nullptr;
}

uint8_t* CefMemoryBlock::GetBits() const
{
    return m_pBits;
}

int32_t CefMemoryBlock::GetWidth() const
{
    return m_nWidth;
}

int32_t CefMemoryBlock::GetHeight() const
{
    return m_nHeight;
}

} //namespace ui
