#include "BitmapAlpha.h"

#include <algorithm>
#include <cstring>

namespace ui
{

BitmapAlpha::BitmapAlpha(uint8_t* pPixel, int32_t nWidth, int32_t nHeight, int32_t nChannels):
    m_pPixel(pPixel),
    m_nWidth(nWidth),
    m_nHeight(nHeight),
    m_nChannels(nChannels)
{
    ASSERT(nChannels == 4);
    ASSERT(nWidth > 0);
    ASSERT(nHeight > 0);
    ASSERT(pPixel != nullptr);
}

BitmapAlpha::~BitmapAlpha()
{
}

//这个函数是必须执行的，否则半透明的界面会出现异常，目前调用的情况，alpha值传入的都是0
void BitmapAlpha::ClearAlpha(const UiRect& rcDirty, uint8_t alpha) const
{
    ASSERT((m_pPixel != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPixel == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) ||(m_nHeight <= 0)){
        return;
    }
    //像素个数与总字节数：使用 size_t 计算，避免 int32_t 溢出（如 32768*32768*4 > INT32_MAX）
    const size_t nPixelCount = (size_t)m_nWidth * (size_t)m_nHeight;
    if ((rcDirty.left == 0) && (rcDirty.top == 0) && (rcDirty.Width() == m_nWidth) && (rcDirty.Height() == m_nHeight)) {
        //全部清除：单次 memset 已是最优，无需再优化
        ::memset(m_pPixel, alpha, nPixelCount * (size_t)m_nChannels);
        return;
    }

    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);
    if ((nRight <= nLeft) || (nBottom <= nTop)) {
        //空区域，无需处理
        return;
    }
    //性能优化：
    //1. 预计算每行的字节跨度 rowStrideBytes（size_t 类型，避免行号乘法溢出）；
    //2. 行首指针逐行递增，替代循环内 "i * rowStrideBytes" 的重复乘法。
    const size_t rowStrideBytes = (size_t)m_nWidth * (size_t)m_nChannels;
    const size_t nBytes = (size_t)(nRight - nLeft) * (size_t)m_nChannels;
    //起始位置：第 nTop 行、第 nLeft 个像素
    uint8_t* pRowStart = m_pPixel + (size_t)nTop * rowStrideBytes + (size_t)nLeft * (size_t)m_nChannels;
    for (int32_t i = nTop; i < nBottom; ++i) {
        //每行从 pRowStart 开始填充 nBytes 字节；行末下移 rowStrideBytes
        ::memset(pRowStart, alpha, nBytes);
        pRowStart += rowStrideBytes;
    }
}

//这个函数目前影响：RichEdit控件，若不调用，其他的控件均不受影响。
void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) const
{
    // 此函数适用于GDI等API渲染位图，导致丢失alpha通道的情况，可以把alpha通道补回来
    // 但是渲染位图时，还有GDI+、AlphaBlend等API给位图设置了半透明的alpha通道时，可能导致没法正确的修正alpha通道
    ASSERT((m_pPixel != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPixel == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
        return;
    }

    uint32_t* pBmpBits = (uint32_t*)m_pPixel;
    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);

    nLeft = std::max(nLeft, rcShadowPadding.left);
    nRight = std::min(nRight, m_nWidth - rcShadowPadding.right);
    nTop = std::max(nTop, rcShadowPadding.top);
    nBottom = std::min(nBottom, m_nHeight - rcShadowPadding.bottom);

    if ((nRight <= nLeft) || (nBottom <= nTop)) {
        return;
    }

    //性能优化：
    //1. 以 32 位整像素为单位读写：一次访存即可读取/写回 Alpha 字节，避免逐字节指针运算；
    //2. 行首指针逐行递增，避免每行重复计算 "i * m_nWidth"；
    //3. 保持原有的判断语义（仅修改需要的像素，不做无谓的全量写回）。
    uint32_t* pRow = pBmpBits + (size_t)nTop * (size_t)m_nWidth;
    if (alpha == 0) {
        // 常用情况：alpha == 0（ClearAlpha 时填的 alpha 就是 0）
        // 逻辑：if (*a == 0) *a = 255;
        // 说明：不做 "无条件 OR" 的改写 —— ClearAlpha 后由上层直接修改过 Alpha 的像素，
        //       必须保留其原值，不能统一置为 255。
        const uint32_t alphaMask = (uint32_t)0xFF000000;
        for (int32_t i = nTop; i < nBottom; ++i) {
            for (int32_t j = nLeft; j < nRight; ++j) {
                const uint32_t pixel = pRow[j];
                if ((pixel & alphaMask) == 0) {
                    pRow[j] = pixel | alphaMask;    //Alpha 字节: 0 -> 255，RGB 保持不变
                }
            }
            pRow += (size_t)m_nWidth;
        }
        return;
    }

    // 通用情况：alpha != 0
    // 逻辑：
    //   if (*a == alpha) *a = 0;
    //   else if (*a == 0) *a = 255;
    const uint32_t alphaMask = (uint32_t)alpha << 24;
    for (int32_t i = nTop; i < nBottom; ++i) {
        for (int32_t j = nLeft; j < nRight; ++j) {
            const uint32_t pixel = pRow[j];
            const uint32_t pixelAlpha = pixel & (uint32_t)0xFF000000;
            if (pixelAlpha == alphaMask) {
                pRow[j] = pixel & 0x00FFFFFF;       //Alpha 字节: alpha -> 0，RGB 保持不变
            }
            else if (pixelAlpha == 0) {
                pRow[j] = pixel | 0xFF000000;       //Alpha 字节: 0 -> 255，RGB 保持不变
            }
        }
        pRow += (size_t)m_nWidth;
    }
}

//这个函数目前影响：RichEdit控件，若不调用，其他的控件均不受影响。
void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) const
{
    // 无论什么情况，都把此区域的alpha通道设置为255
    ASSERT((m_pPixel != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
    if ((m_pPixel == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
        return;
    }

    uint32_t* pBmpBits = (uint32_t*)m_pPixel;
    int32_t nTop = std::max(rcDirty.top, 0);
    int32_t nBottom = std::min(rcDirty.bottom, m_nHeight);
    int32_t nLeft = std::max(rcDirty.left, 0);
    int32_t nRight = std::min(rcDirty.right, m_nWidth);

    nLeft = std::max(nLeft, rcShadowPadding.left);
    nRight = std::min(nRight, m_nWidth - rcShadowPadding.right);
    nTop = std::max(nTop, rcShadowPadding.top);
    nBottom = std::min(nBottom, m_nHeight - rcShadowPadding.bottom);

    if ((nRight <= nLeft) || (nBottom <= nTop)) {
        return;
    }

    //性能优化：
    //1. 用单条 OR 指令 (| 0xFF000000) 替代 "if (*a != 255) *a = 255;" 的条件写：
    //   分支预测开销被消除，且编译器可自动向量化（AVX2 可一次处理 8 个像素）；
    //2. 行首指针逐行递增，避免每行重复计算 "i * m_nWidth"；
    //3. 循环内使用 size_t 行偏移，避免 int32_t 乘法溢出。
    const uint32_t alphaMask = (uint32_t)0xFF000000;
    uint32_t* pRow = pBmpBits + (size_t)nTop * (size_t)m_nWidth;
    for (int32_t i = nTop; i < nBottom; ++i) {
        for (int32_t j = nLeft; j < nRight; ++j) {
            //单条 OR 指令：将 ARGB 像素的最高字节（Alpha）置为 0xFF，RGB 保持不变
            pRow[j] |= alphaMask;
        }
        pRow += (size_t)m_nWidth;
    }
}

}
