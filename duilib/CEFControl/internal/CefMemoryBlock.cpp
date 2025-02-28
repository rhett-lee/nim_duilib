#include "CefMemoryBlock.h"
#include "duilib/Render/IRender.h"

namespace ui {

CefMemoryBlock::CefMemoryBlock():
    m_pBits(nullptr),
    m_nWidth(0),
    m_nHeight(0)
{
}

CefMemoryBlock::~CefMemoryBlock()
{
    Clear();
}

bool CefMemoryBlock::Init(const void* buffer, int32_t width, int32_t height)
{
    std::lock_guard<std::mutex> threadGuard(m_memMutex);
    ASSERT((width > 0) && (height > 0) && (buffer != nullptr));
    if ((width <= 0) || (height <= 0) || (buffer == nullptr)) {
        return false;
    }
    if ((m_nWidth != width) || (m_nHeight != height) || (m_pBits == nullptr)) {
        //需重新初始化
        if (m_pBits != nullptr) {
            delete[] m_pBits;
            m_pBits = nullptr;
        }
        m_pBits = new uint8_t[width * height * sizeof(uint32_t)];
        m_nWidth = width;
        m_nHeight = height;
    }
    //复制数据
    if (m_pBits != nullptr) {
        memcpy(m_pBits, buffer, width * height * sizeof(uint32_t));
        return true;
    }
    return false;
}

void CefMemoryBlock::Clear()
{
    if (m_pBits != nullptr) {
        delete[] m_pBits;
        m_pBits = nullptr;
    }
    m_nWidth = 0;
    m_nHeight = 0;
}

void  CefMemoryBlock::PaintData(IRender* pRender, const UiRect& rcPaint, int32_t left, int32_t top)
{
    std::lock_guard<std::mutex> threadGuard(m_memMutex);
    //通过直接写入数据的接口，性能最佳
    UiRect dcPaint;
    dcPaint.left = left;
    dcPaint.top = top;
    dcPaint.right = dcPaint.left + GetWidth();
    dcPaint.bottom = dcPaint.top + GetHeight();
    if (!dcPaint.IsEmpty() && !rcPaint.IsEmpty() && IsValid()) {
        bool bRet = pRender->WritePixels(GetBits(), GetWidth() * GetHeight() * sizeof(uint32_t), dcPaint);
        ASSERT_UNUSED_VARIABLE(bRet);
    }
}

bool CefMemoryBlock::IsValid() const
{
    return (m_pBits != nullptr);
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
