 #include "BitmapAlpha.h"

namespace ui
{

BitmapAlpha::BitmapAlpha(uint8_t* pPiexl, int32_t nWidth, int32_t nHeight, int32_t nChannels):
    m_pPiexl(pPiexl),
    m_nWidth(nWidth),
    m_nHeight(nHeight),
    m_nChannels(nChannels)
{
    ASSERT(nChannels == 4);
    ASSERT(nWidth > 0);
    ASSERT(nHeight > 0);
    ASSERT(pPiexl != nullptr);
}

BitmapAlpha::~BitmapAlpha()
{
}

//这个函数是必须执行的，否则半透明的界面会出现异常，目前调用的情况，alpha值传入的都是0
void BitmapAlpha::ClearAlpha(const UiRect& rcDirty, uint8_t alpha) const
{
    ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) ||(m_nHeight <= 0)){
        return;
    }
    if ((rcDirty.left == 0) && (rcDirty.top == 0) && (rcDirty.Width() == m_nWidth) && (rcDirty.Height() == m_nHeight)) {
        //全部清除
        ::memset(m_pPiexl, alpha, m_nHeight * m_nWidth * m_nChannels);
        return;
    }

    uint32_t* pBmpBits = (uint32_t*)m_pPiexl;
    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);
    if (nRight > nLeft) {
        const size_t nBytes = (size_t)(nRight - nLeft) * 4;
        for (int32_t i = nTop; i < nBottom; ++i) {
            ::memset(pBmpBits + i * m_nWidth + nLeft, alpha, nBytes);
        }
    }
}

//这个函数目前影响：RichEdit控件，若不调用，其他的控件均不受影响。
void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) const
{
    // 此函数适用于GDI等API渲染位图，导致丢失alpha通道的情况，可以把alpha通道补回来
    // 但是渲染位图时，还有GDI+、AlphaBlend等API给位图设置了半透明的alpha通道时，可能导致没法正确的修正alpha通道
    ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
        return;
    }

    uint32_t * pBmpBits = (uint32_t*)m_pPiexl;
    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);

    nLeft = std::max(nLeft, rcShadowPadding.left);
    nRight = std::min(nRight, m_nWidth - rcShadowPadding.right);
    nTop = std::max(nTop, rcShadowPadding.top);
    nBottom = std::min(nBottom, m_nHeight - rcShadowPadding.bottom);

    for (int32_t i = nTop; i < nBottom; i++) {
        for (int32_t j = nLeft; j < nRight; j++) {
            uint8_t* a = (uint8_t*)(pBmpBits + i * m_nWidth + j) + 3;
            // ClearAlpha时，把alpha通道设置为某个值
            // 如果此值没有变化，则证明上面没有绘制任何内容，把alpha设为0
            if (alpha != 0 && *a == alpha) {
                *a = 0;
            }
            // 如果此值变为0，则证明上面被类似DrawText等GDI函数绘制过导致alpha被设为0，此时alpha设为255
            else if (*a == 0) {
                *a = 255;
            }
        }
    }
}

//这个函数目前影响：RichEdit控件，若不调用，其他的控件均不受影响。
void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) const
{
    // 无论什么情况，都把此区域的alpha通道设置为255
    ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
        return;
    }

    uint32_t* pBmpBits = (uint32_t*)m_pPiexl;
    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);

    nLeft = std::max(nLeft, rcShadowPadding.left);
    nRight = std::min(nRight, m_nWidth - rcShadowPadding.right);
    nTop = std::max(nTop, rcShadowPadding.top);
    nBottom = std::min(nBottom, m_nHeight - rcShadowPadding.bottom);

    for (int i = nTop; i < nBottom; ++i) {
        for (int j = nLeft; j < nRight; ++j) {            
            uint8_t* a = (uint8_t*)(pBmpBits + i * m_nWidth + j) + 3;
            if (*a != 255) {
                *a = 255;
            }
        }
    }
}

}
