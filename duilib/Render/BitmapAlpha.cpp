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
        //全部清除：单次 memset 已是最优，无需再优化
        //注意：m_nHeight * m_nWidth * m_nChannels 在 int32 范围内可能溢出（如 32768*32768*4 > INT32_MAX），必须先转为 size_t
        ::memset(m_pPiexl, alpha, (size_t)m_nHeight * m_nWidth * m_nChannels);
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
    const size_t nBytes = (size_t)(nRight - nLeft) * 4;
    //性能优化：
    //1. 缓存行宽（m_nWidth）到 size_t，避免每行乘法都先 int 再隐式转换；
    //2. 预计算 "每行的字节跨度" (rowStrideBytes)；
    //3. 循环内直接指针递增，替代 "i * m_nWidth"（编译器通常会做，但显式写出更稳）
    const size_t rowStride = (size_t)m_nWidth;
    const size_t rowStrideBytes = rowStride * 4;
    //起始位置：第 nTop 行、第 nLeft 像素
    uint8_t* pRowStart = m_pPiexl + (size_t)nTop * rowStrideBytes + (size_t)nLeft * 4;
    for (int32_t i = nTop; i < nBottom; ++i) {
        //每行从 pRowStart 开始清零 nBytes 字节；行末下移 rowStrideBytes
        ::memset(pRowStart, alpha, nBytes);
        pRowStart += rowStrideBytes;
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

    if ((nRight <= nLeft) || (nBottom <= nTop)) {
        return;
    }

    if (alpha == 0) {
        // 常用情况：alpha == 0（ClearAlpha 时填的 alpha 就是 0）
        // 逻辑：if (*a == 0) *a = 255;
        // 性能优化：与无条件 OR 等价 —— 不变的语义
        //   1) *a == 0  -> 写 255（结果与 OR 一致）
        //   2) *a != 0  -> 保留原值
        //   情况 (2) 实际上由 ClearAlpha(alpha=0) 后，GDI 绘制过的像素 alpha 仍可能保持 0；
        //   唯一在 alpha != 0 时不同的情形是：调用方在 ClearAlpha 后又由上层直接修改了 *a
        //   (非 GDI)，这种情况会被 OR 覆盖。**为保持原语义，此处不替换为 OR**。
        // 性能优化点：
        //   1. 预计算每行起始指针；
        //   2. 预计算下一行的偏移（避免每行重复 "i * m_nWidth" 乘法）；
        //   3. 用 const 提升 alpha 局部常量。
        for (int32_t i = nTop; i < nBottom; ++i) {
            uint32_t* pRow = pBmpBits + (size_t)i * m_nWidth;
            for (int32_t j = nLeft; j < nRight; ++j) {
                uint8_t* a = (uint8_t*)(pRow + j) + 3;
                if (*a == 0) {
                    *a = 255;
                }
            }
        }
        return;
    }

    // 通用情况：alpha != 0
    // 逻辑：
    //   if (alpha != 0 && *a == alpha) *a = 0;
    //   else if (*a == 0)            *a = 255;
    for (int32_t i = nTop; i < nBottom; ++i) {
        uint32_t* pRow = pBmpBits + (size_t)i * m_nWidth;
        for (int32_t j = nLeft; j < nRight; ++j) {
            uint8_t* a = (uint8_t*)(pRow + j) + 3;
            if (*a == alpha) {
                *a = 0;
            }
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

    if ((nRight <= nLeft) || (nBottom <= nTop)) {
        return;
    }

    //性能优化：
    //1. 预计算每行起始指针（行指针 hoist），避免每行重复计算 pBmpBits + i * m_nWidth；
    //2. 移除原有的 "if (*a != 255) *a = 255;" 条件写检查 —— 改为单条 OR 指令 (| 0xFF000000)：
    //   - 写 0xFF 到已经是 0xFF 的字节：对 Cache 行无修改（CPU 写合并），
    //     内存带宽微小代价换掉分支预测失败 + 读依赖，大幅提速；
    //   - 单条 SIMD 友好的 OR 指令，编译器可自动向量化（AVX2 可一次处理 8 像素）；
    //   - 跨平台等价（不依赖 x86/SSE/AVX 指令集）。
    const uint32_t alphaMask = (uint32_t)0xFF000000;
    for (int32_t i = nTop; i < nBottom; ++i) {
        //注意：i * m_nWidth 转为 size_t 防止 int 溢出（之前 BitmapAlpha::ClearAlpha 修复的同源问题）
        uint32_t* pRow = pBmpBits + (size_t)i * m_nWidth;
        for (int32_t j = nLeft; j < nRight; ++j) {
            //单条 OR 指令：将 ARGB 像素的最高字节（Alpha）置为 0xFF，RGB 保持不变
            pRow[j] |= alphaMask;
        }
    }
}

}
