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

void BitmapAlpha::ClearAlpha(const UiRect& rcDirty, uint8_t alpha) const
{
	ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
	if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) ||(m_nHeight <= 0)){
		return;
	}

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, (int)m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, (int)m_nWidth);
	if (nRight > nLeft)	{
		for (int i = nTop; i < nBottom; ++i) {
			::memset(pBmpBits + i * m_nWidth + nLeft, alpha, (nRight - nLeft) * 4);
		}
	}
}

void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding, uint8_t alpha) const
{
	// 此函数适用于GDI等API渲染位图，导致丢失alpha通道的情况，可以把alpha通道补回来
	// 但是渲染位图时，还有GDI+、AlphaBlend等API给位图设置了半透明的alpha通道时，可能导致没法正确的修正alpha通道
	ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
	if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
		return;
	}

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, (int)m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, (int)m_nWidth);

	for (int i = nTop; i < nBottom; i++) {
		for (int j = nLeft; j < nRight; j++) {
			uint8_t* a = (uint8_t*)(pBmpBits + i * m_nWidth + j) + 3;

			if (((j >= rcShadowPadding.left && j < m_nWidth - rcShadowPadding.right)
				|| (i >= rcShadowPadding.top && i < m_nHeight - rcShadowPadding.bottom))) {

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
}

void BitmapAlpha::RestoreAlpha(const UiRect& rcDirty, const UiPadding& rcShadowPadding) const
{
	// 无论什么情况，都把此区域的alpha通道设置为255
	ASSERT((m_pPiexl != nullptr) && (m_nChannels == 4) && (m_nWidth > 0) && (m_nHeight > 0));
	if ((m_pPiexl == nullptr) || (m_nChannels != 4) || (m_nWidth <= 0) || (m_nHeight <= 0)) {
		return;
	}

	unsigned int * pBmpBits = (unsigned int *)m_pPiexl;
	int nTop = std::max((int)rcDirty.top, 0);
	int nBottom = std::min((int)rcDirty.bottom, (int)m_nHeight);
	int nLeft = std::max((int)rcDirty.left, 0);
	int nRight = std::min((int)rcDirty.right, (int)m_nWidth);

	for (int i = nTop; i < nBottom; i++) {
		for (int j = nLeft; j < nRight; j++) {
			uint8_t* a = (uint8_t*)(pBmpBits + i * m_nWidth + j) + 3;

			if (((j >= rcShadowPadding.left && j < m_nWidth - rcShadowPadding.right)
				|| (i >= rcShadowPadding.top && i < m_nHeight - rcShadowPadding.bottom))) {
					*a = 255;
			}
		}
	}
}

}